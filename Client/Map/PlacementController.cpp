#include "PlacementController.h"
#include "BuildingMarkerOrb.h"
#include "PlacementBuildingVisual.h"
#include "../UI/CitizenInfoWidget.h"
#include "../World/MainWorldUiReadAccess.h"
#include "../ObjectNames.h"
#include "Component/CameraComponent.h"
#include "Component/TileMapComponent.h"
#include "Device.h"
#include "World/World.h"
#include "World/Input.h"
#include "World/WorldUIManager.h"
#include "Object/TileMapObject.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <limits>
#include <Windows.h>

namespace
{
    constexpr float GRoadPreviewAlpha = 0.9f;
    constexpr int GRoadPathDirCount = 4;
    constexpr int GRoadPathDirX[GRoadPathDirCount] = { 0, 1, 0, -1 };
    constexpr int GRoadPathDirY[GRoadPathDirCount] = { 1, 0, -1, 0 };

    bool TryGetTileGridCoords(
        const std::shared_ptr<CTileMapComponent>& TileMap,
        int TileIndex,
        int& OutGridX,
        int& OutGridY)
    {
        OutGridX = 0;
        OutGridY = 0;

        if (!TileMap)
            return false;

        auto Tile = TileMap->GetTile(TileIndex).lock();

        if (!Tile)
            return false;

        const int TileX = Tile->GetIndexX();
        const int TileY = Tile->GetIndexY();
        OutGridX = TileX + ((TileY + (TileY & 1)) / 2);
        OutGridY = TileX - (TileY / 2);
        return true;
    }

    int GetTileIndexByGrid(
        const std::shared_ptr<CTileMapComponent>& TileMap,
        int GridX,
        int GridY)
    {
        if (!TileMap)
            return -1;

        const int TileY = GridX - GridY;

        if (TileY < 0 || TileY >= TileMap->GetTileCountY())
            return -1;

        const int TileX = GridY + (TileY / 2);

        if (TileX < 0 || TileX >= TileMap->GetTileCountX())
            return -1;

        return TileY * TileMap->GetTileCountX() + TileX;
    }

    int GetRoadPathNeighborIndex(
        const std::shared_ptr<CTileMapComponent>& TileMap,
        int TileIndex,
        int DirIndex)
    {
        if (!TileMap ||
            DirIndex < 0 ||
            DirIndex >= GRoadPathDirCount)
        {
            return -1;
        }

        int GridX = 0;
        int GridY = 0;

        if (!TryGetTileGridCoords(TileMap, TileIndex, GridX, GridY))
            return -1;

        return GetTileIndexByGrid(
            TileMap,
            GridX + GRoadPathDirX[DirIndex],
            GridY + GRoadPathDirY[DirIndex]);
    }

    float EstimateRoadPathCost(
        const std::shared_ptr<CTileMapComponent>& TileMap,
        int FromTileIndex,
        int ToTileIndex)
    {
        int FromGridX = 0;
        int FromGridY = 0;
        int ToGridX = 0;
        int ToGridY = 0;

        if (!TryGetTileGridCoords(TileMap, FromTileIndex, FromGridX, FromGridY) ||
            !TryGetTileGridCoords(TileMap, ToTileIndex, ToGridX, ToGridY))
        {
            return 0.f;
        }

        return static_cast<float>(
            abs(FromGridX - ToGridX) +
            abs(FromGridY - ToGridY));
    }
}

CPlacementController::CPlacementController()
{
    SetClassType<CPlacementController>();
}

CPlacementController::CPlacementController(const CPlacementController& ref) :
    CGameObject(ref)
{
}

CPlacementController::CPlacementController(CPlacementController&& ref) noexcept :
    CGameObject(std::move(ref))
{
}

CPlacementController::~CPlacementController()
{
}

bool CPlacementController::Init()
{
    CGameObject::Init();

    auto World = mWorld.lock();
    auto Input = World->GetInput().lock();

    Input->AddBindKey("MainCameraMoveArea", VK_RBUTTON);
    Input->SetBindFunction<CPlacementController>("MainCameraMoveArea",
        EInputType::Press, this, &CPlacementController::MoveCurrentArea);

    Input->AddBindKey("MainCameraRotateCCW", 'Q');
    Input->SetBindFunction<CPlacementController>("MainCameraRotateCCW",
        EInputType::Press, this, &CPlacementController::RotateCurrentAreaCCW);

    Input->AddBindKey("MainCameraRotateCW", 'E');
    Input->SetBindFunction<CPlacementController>("MainCameraRotateCW",
        EInputType::Press, this, &CPlacementController::RotateCurrentAreaCW);

    Input->AddBindKey("MainCameraPlace", VK_LBUTTON);
    Input->SetBindFunction<CPlacementController>("MainCameraPlace",
        EInputType::Press, this, &CPlacementController::PlaceCurrentArea);
    Input->SetBindFunction<CPlacementController>("MainCameraPlace",
        EInputType::Hold, this, &CPlacementController::PaintRoadHold);
    Input->SetBindFunction<CPlacementController>("MainCameraPlace",
        EInputType::Release, this, &CPlacementController::FinishRoadBrushStroke);

    Input->AddBindKey("MainCameraCancelPlacement", VK_ESCAPE);
    Input->SetBindFunction<CPlacementController>("MainCameraCancelPlacement",
        EInputType::Press, this, &CPlacementController::CancelPlacementMode);

    return true;
}

void CPlacementController::Update(float DeltaTime)
{
    CGameObject::Update(DeltaTime);

    UpdateDemolitionHoverPreview();
    UpdateRoadPathPreview();
}

void CPlacementController::SetDemolitionMode(bool Enable)
{
    if (mDemolitionMode == Enable)
        return;

    mDemolitionMode = Enable;

    if (!mDemolitionMode)
    {
        ClearDemolitionHoverPreview();
        return;
    }

    CancelActivePlacementSession();

    auto CitizenInfoWidget = mCitizenInfoWidget.lock();

    if (CitizenInfoWidget)
        CitizenInfoWidget->SetEnable(false);

    UpdateDemolitionHoverPreview();
}

bool CPlacementController::BeginBuildPlacement(
    const FBuildingCatalogEntry& Entry,
    const std::string& SpriteTexturePath)
{
    auto World = mWorld.lock();

    if (!World)
        return false;

    const auto MainWorldAccess =
        ResolveMainWorldBuildMenuAccess(World);

    if (MainWorldAccess &&
        !IsBuildingEraUnlocked(
            MainWorldAccess->GetCurrentEra(),
            Entry.UnlockEra))
    {
        return false;
    }

    if (mDemolitionMode)
        SetDemolitionMode(false);
    else if (Entry.BuildingKind != EPlacementBuildingKind::Road)
        ClearRoadBrushMode();

    auto Input = World->GetInput().lock();

    if (!Input)
        return false;

    auto TileMapObject = World->FindObject<CTileMapObject>(GTileMapObjectName).lock();

    if (!TileMapObject)
        return false;

    auto ActivePlacementObject = mActivePlacementObject.lock();

    if (ActivePlacementObject &&
        ActivePlacementObject->IsMovePreviewActive())
    {
        ActivePlacementObject->CancelMovePreview();
    }

    int NameSuffix = 1;
    std::string PlacementName;

    do
    {
        PlacementName = GPlacedBuildingPrefix + std::to_string(NameSuffix);
        ++NameSuffix;
    } while (World->FindObject<CPlacementAreaObject>(
        PlacementName).lock());

    auto PlacementObjectWeak =
        World->CreateGameObject<CPlacementAreaObject>(PlacementName);
    auto PlacementObject = PlacementObjectWeak.lock();

    if (!PlacementObject)
        return false;

    if (Entry.BuildingKind == EPlacementBuildingKind::Road)
    {
        mRoadBrushMode = true;
        mRoadBrushEntry = Entry;
        mRoadBrushSpriteTexturePath = SpriteTexturePath;
        mLastRoadBrushTileIndex = -1;
    }

    const std::string SafeBuildingId = Entry.Id.empty() ?
        PlacementName :
        Entry.Id;

    PlacementObject->SetTileMapObject(TileMapObject);
    PlacementObject->SetAutoPlaceOnPrepare(false);
    PlacementObject->SetBuildingSpriteTexturePath(SpriteTexturePath);
    PlacementObject->SetBuildingId(SafeBuildingId);
    PlacementObject->ApplyCatalogEntry(Entry);

    auto VisualWeak = World->CreateGameObject<CBuildingVisual>(
        PlacementName + "_Visual");
    auto Visual = VisualWeak.lock();

    if (Visual)
        Visual->SetBuilding(PlacementObjectWeak);

    mActivePlacementObject = PlacementObject;

    if (Entry.BuildingKind != EPlacementBuildingKind::Road)
        PlacementObject->StartMovePreview(Input->GetMouseWorldPos());

    auto CitizenInfoWidget = mCitizenInfoWidget.lock();

    if (CitizenInfoWidget)
        CitizenInfoWidget->SetEnable(false);

    return true;
}

bool CPlacementController::BeginMoveExistingBuilding(
    const std::string& BuildingObjectName)
{
    if (BuildingObjectName.empty())
        return false;

    auto World = mWorld.lock();

    if (!World)
        return false;

    auto Input = World->GetInput().lock();

    if (!Input)
        return false;

    auto PlacementObject =
        World->FindObject<CPlacementAreaObject>(BuildingObjectName).lock();

    if (!PlacementObject ||
        !PlacementObject->GetAlive() ||
        !PlacementObject->GetEnable())
    {
        return false;
    }

    if (mDemolitionMode)
        SetDemolitionMode(false);
    else
        ClearRoadBrushMode();

    auto ActiveObject = mActivePlacementObject.lock();

    if (ActiveObject &&
        ActiveObject != PlacementObject &&
        ActiveObject->IsMovePreviewActive())
    {
        ActiveObject->CancelMovePreview();
    }

    mActivePlacementObject = PlacementObject;
    PlacementObject->StartMovePreview(Input->GetMouseWorldPos());

    auto CitizenInfoWidget = mCitizenInfoWidget.lock();

    if (CitizenInfoWidget)
        CitizenInfoWidget->SetEnable(false);

    return true;
}

bool CPlacementController::DemolishBuildingByName(
    const std::string& BuildingObjectName)
{
    if (BuildingObjectName.empty())
        return false;

    auto World = mWorld.lock();

    if (!World)
        return false;

    auto PlacementObject =
        World->FindObject<CPlacementAreaObject>(BuildingObjectName).lock();

    if (!PlacementObject ||
        !PlacementObject->GetAlive() ||
        !PlacementObject->GetEnable())
    {
        return false;
    }

    DemolishPlacementObject(PlacementObject);

    auto CitizenInfoWidget = mCitizenInfoWidget.lock();

    if (CitizenInfoWidget)
        CitizenInfoWidget->SetEnable(false);

    return true;
}

void CPlacementController::RotateCurrentAreaCCW()
{
    auto ActiveObject = mActivePlacementObject.lock();

    if (!ActiveObject || !ActiveObject->IsMovePreviewActive())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    ActiveObject->RotatePreviewCCW(Input->GetMouseWorldPos());
}

void CPlacementController::RotateCurrentAreaCW()
{
    auto ActiveObject = mActivePlacementObject.lock();

    if (!ActiveObject || !ActiveObject->IsMovePreviewActive())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    ActiveObject->RotatePreviewCW(Input->GetMouseWorldPos());
}

void CPlacementController::MoveCurrentArea()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    if (IsPointerOverActiveUi(Input->GetMousePos()))
        return;

    if (mDemolitionMode)
    {
        SetDemolitionMode(false);
        return;
    }

    if (mRoadBrushMode)
    {
        CancelActivePlacementSession();
        return;
    }

    auto ActiveObject = mActivePlacementObject.lock();

    if (ActiveObject && ActiveObject->IsMovePreviewActive())
    {
        CancelActivePlacementSession();
        return;
    }

    const FVector2 MouseWorldPos = Input->GetMouseWorldPos();

    RefreshPlacementObjects();

    auto PlacementObject = PickPlacementObject(MouseWorldPos);

    if (!PlacementObject)
        return;

    ActiveObject = mActivePlacementObject.lock();

    if (ActiveObject && ActiveObject != PlacementObject)
    {
        ActiveObject->CancelMovePreview();
    }

    mActivePlacementObject = PlacementObject;
    PlacementObject->StartMovePreview(MouseWorldPos);
}

void CPlacementController::PlaceCurrentArea()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    const FVector2 MouseWorldPos = Input->GetMouseWorldPos();
    const FVector2 MouseScreenPos = Input->GetMousePos();

    if (IsPlacementInputBlocked(MouseScreenPos))
        return;

    if (mRoadBrushMode &&
        mRoadBrushEntry.BuildingKind == EPlacementBuildingKind::Road)
    {
        int TileIndex = -1;

        if (!TryGetMouseTileIndex(MouseWorldPos, TileIndex) ||
            TileIndex < 0)
        {
            return;
        }

        if (mRoadPathStartTileIndex < 0)
        {
            std::shared_ptr<CTileMapComponent> TileMap;

            if (!TryGetTileMap(TileMap) ||
                !IsRoadPathTileTraversable(TileMap, TileIndex))
            {
                return;
            }

            mRoadPathStartTileIndex = TileIndex;
            mRoadPreviewEndTileIndex = -1;
            ApplyRoadPreviewTiles(std::vector<int>(1, TileIndex));
            return;
        }

        if (CommitRoadPathToTile(TileIndex))
            CancelActivePlacementSession();

        return;
    }

    if (TryCommitActivePlacement())
        return;

    if (mDemolitionMode)
    {
        RefreshPlacementObjects();
        auto ClickedPlacementObject = PickPlacementObject(MouseWorldPos);

        if (ClickedPlacementObject)
        {
            DemolishPlacementObject(ClickedPlacementObject);
        }

        auto CitizenInfoWidget = mCitizenInfoWidget.lock();

        if (CitizenInfoWidget)
            CitizenInfoWidget->SetEnable(false);

        return;
    }

    auto CitizenOrb = PickCitizenOrb(MouseWorldPos);

    if (CitizenOrb)
    {
        EnsureCitizenInfoWidget();
        auto CitizenInfoWidget = mCitizenInfoWidget.lock();

        if (CitizenInfoWidget)
        {
            CitizenInfoWidget->OpenCitizen(
                CitizenOrb->GetName(),
                CitizenOrb->GetSatisfaction(),
                Input->GetMousePos());
        }
        return;
    }

    RefreshPlacementObjects();
    auto ClickedPlacementObject = PickPlacementObject(MouseWorldPos);
    auto CitizenInfoWidget = mCitizenInfoWidget.lock();

    if (ClickedPlacementObject)
    {
        EnsureCitizenInfoWidget();
        CitizenInfoWidget = mCitizenInfoWidget.lock();

        if (CitizenInfoWidget)
        {
            CitizenInfoWidget->OpenBuilding(
                ClickedPlacementObject->GetName(),
                ClickedPlacementObject->GetBuildingDisplayName(),
                ClickedPlacementObject->GetBuildingCategoryName(),
                ClickedPlacementObject->IsResidential(),
                ClickedPlacementObject->GetCapacity(),
                Input->GetMousePos());
        }
    }
    else
    {
        if (CitizenInfoWidget)
            CitizenInfoWidget->SetEnable(false);
    }
}

void CPlacementController::PaintRoadHold()
{
}

void CPlacementController::FinishRoadBrushStroke()
{
}

void CPlacementController::CancelPlacementMode()
{
    if (mDemolitionMode)
    {
        SetDemolitionMode(false);
        return;
    }

    if (mRoadBrushMode)
    {
        CancelActivePlacementSession();
        return;
    }

    CancelActivePlacementSession();
}

void CPlacementController::CancelActivePlacementSession()
{
    auto ActivePlacementObject = mActivePlacementObject.lock();

    if (ActivePlacementObject &&
        ActivePlacementObject->IsMovePreviewActive())
    {
        ActivePlacementObject->CancelMovePreview();
    }

    mActivePlacementObject.reset();
    ClearRoadBrushMode();
}

void CPlacementController::UpdateDemolitionHoverPreview()
{
    if (!mDemolitionMode)
    {
        ClearDemolitionHoverPreview();
        return;
    }

    auto World = mWorld.lock();

    if (!World)
    {
        ClearDemolitionHoverPreview();
        return;
    }

    auto Input = World->GetInput().lock();

    if (!Input)
    {
        ClearDemolitionHoverPreview();
        return;
    }

    if (IsPointerOverActiveUi(Input->GetMousePos()))
    {
        ClearDemolitionHoverPreview();
        return;
    }

    RefreshPlacementObjects();

    auto HoverObject = PickPlacementObject(Input->GetMouseWorldPos());
    auto PrevHoverObject = mDemolitionHoverObject.lock();

    if (PrevHoverObject == HoverObject)
    {
        if (PrevHoverObject)
            PrevHoverObject->SetDemolitionHoverActive(true);
        return;
    }

    if (PrevHoverObject)
        PrevHoverObject->SetDemolitionHoverActive(false);

    if (HoverObject)
        HoverObject->SetDemolitionHoverActive(true);

    mDemolitionHoverObject = HoverObject;
}

void CPlacementController::ClearDemolitionHoverPreview()
{
    auto HoverObject = mDemolitionHoverObject.lock();

    if (HoverObject)
        HoverObject->SetDemolitionHoverActive(false);

    mDemolitionHoverObject.reset();
}

void CPlacementController::RefreshPlacementObjects()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    World->FindObjectListByType<CPlacementAreaObject>(
        mPlacementObjects);
}

bool CPlacementController::IsPointerOverActiveUi(
    const FVector2& MouseScreenPos) const
{
    auto World = mWorld.lock();

    if (!World)
        return false;

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return false;

    return UIManager->IsPointOverEnabledWidget(MouseScreenPos);
}

bool CPlacementController::IsPlacementInputBlocked(
    const FVector2& MouseScreenPos) const
{
    if (IsPointerOverActiveUi(MouseScreenPos))
        return true;

    auto CitizenInfoPanelWidget = mCitizenInfoWidget.lock();

    if (!CitizenInfoPanelWidget ||
        !CitizenInfoPanelWidget->GetEnable())
    {
        return false;
    }

    const FVector3& PanelPos = CitizenInfoPanelWidget->GetPos();
    const FVector3& PanelSize = CitizenInfoPanelWidget->GetSize();

    return MouseScreenPos.x >= PanelPos.x &&
        MouseScreenPos.x <= PanelPos.x + PanelSize.x &&
        MouseScreenPos.y >= PanelPos.y &&
        MouseScreenPos.y <= PanelPos.y + PanelSize.y;
}

bool CPlacementController::TryCommitActivePlacement()
{
    auto ActivePlacementObject = mActivePlacementObject.lock();

    if (!ActivePlacementObject ||
        !ActivePlacementObject->IsMovePreviewActive())
    {
        return false;
    }

    const bool WasPlaced = ActivePlacementObject->HasPlacedArea();
    const bool IsRoadBrushPlacement =
        mRoadBrushMode &&
        !WasPlaced &&
        ActivePlacementObject->IsRoad();

    if (IsRoadBrushPlacement)
    {
        auto World = mWorld.lock();

        if (World)
        {
            auto Input = World->GetInput().lock();

            if (Input)
            {
                int TileIndex = -1;

                if (TryGetMouseTileIndex(Input->GetMouseWorldPos(), TileIndex))
                {
                    // Press and Hold both fire on the first mouse-down frame.
                    mLastRoadBrushTileIndex = TileIndex;
                }
            }
        }
    }

    ActivePlacementObject->ConfirmPlacement();

    const bool IsPlaced = ActivePlacementObject->HasPlacedArea();

    if (!WasPlaced && IsPlaced)
    {
        RegisterBuildingToOrbs(ActivePlacementObject->GetName());
    }

    if (IsRoadBrushPlacement && IsPlaced)
    {
        if (!RestartRoadBrushPlacement())
            mActivePlacementObject.reset();

        return true;
    }

    if (!ActivePlacementObject->IsMovePreviewActive())
        mActivePlacementObject.reset();

    return true;
}

bool CPlacementController::RestartRoadBrushPlacement()
{
    if (!mRoadBrushMode ||
        mRoadBrushEntry.BuildingKind != EPlacementBuildingKind::Road)
    {
        return false;
    }

    const int LastRoadBrushTileIndex = mLastRoadBrushTileIndex;
    const bool Started = BeginBuildPlacement(
        mRoadBrushEntry,
        mRoadBrushSpriteTexturePath);

    if (!Started)
    {
        ClearRoadBrushMode();
        return false;
    }

    mLastRoadBrushTileIndex = LastRoadBrushTileIndex;
    return true;
}

void CPlacementController::UpdateRoadPathPreview()
{
    if (!mRoadBrushMode ||
        mRoadBrushEntry.BuildingKind != EPlacementBuildingKind::Road ||
        mRoadPathStartTileIndex < 0)
    {
        return;
    }

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input || IsPlacementInputBlocked(Input->GetMousePos()))
    {
        if (mRoadPreviewTileIndices.size() != 1 ||
            mRoadPreviewTileIndices[0] != mRoadPathStartTileIndex)
        {
            ApplyRoadPreviewTiles(
                std::vector<int>(1, mRoadPathStartTileIndex));
        }

        return;
    }

    int EndTileIndex = -1;

    if (!TryGetMouseTileIndex(Input->GetMouseWorldPos(), EndTileIndex) ||
        EndTileIndex < 0)
    {
        ApplyRoadPreviewTiles(std::vector<int>(1, mRoadPathStartTileIndex));
        mRoadPreviewEndTileIndex = -1;
        return;
    }

    if (EndTileIndex == mRoadPreviewEndTileIndex)
        return;

    std::vector<int> PathIndices;

    if (!ResolveRoadPathToTile(EndTileIndex, PathIndices) ||
        PathIndices.empty())
    {
        PathIndices.push_back(mRoadPathStartTileIndex);
    }

    ApplyRoadPreviewTiles(PathIndices);
    mRoadPreviewEndTileIndex = EndTileIndex;
}

bool CPlacementController::TryGetTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap) const
{
    OutTileMap.reset();

    auto World = mWorld.lock();

    if (!World)
        return false;

    auto TileMapObject =
        World->FindObject<CTileMapObject>(GTileMapObjectName).lock();

    if (!TileMapObject)
        return false;

    OutTileMap = TileMapObject->GetTileMap().lock();
    return OutTileMap != nullptr;
}

bool CPlacementController::TryGetRoadPreviewTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap)
{
    OutTileMap.reset();

    auto World = mWorld.lock();

    if (!World)
        return false;

    if (mRoadPreviewTileMapObject.expired())
    {
        mRoadPreviewTileMapObject =
            World->FindObject<CTileMapObject>(GTileMapRoadPreviewName);
    }

    auto RoadPreviewObj = mRoadPreviewTileMapObject.lock();

    if (!RoadPreviewObj)
        return false;

    OutTileMap = RoadPreviewObj->GetTileMap().lock();
    return OutTileMap != nullptr;
}

bool CPlacementController::TryGetMouseTileIndex(
    const FVector2& MouseWorldPos, int& OutTileIndex) const
{
    OutTileIndex = -1;

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!TryGetTileMap(TileMap))
        return false;

    OutTileIndex = TileMap->GetTileIndex(MouseWorldPos);
    return OutTileIndex >= 0;
}

bool CPlacementController::TryGetTileWorldPos(
    int TileIndex,
    FVector2& OutWorldPos) const
{
    OutWorldPos = FVector2();

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!TryGetTileMap(TileMap))
        return false;

    auto Tile = TileMap->GetTile(TileIndex).lock();

    if (!Tile)
        return false;

    const FVector2 TileCenter = Tile->GetCenter();
    OutWorldPos = TileCenter;

    auto Owner = TileMap->GetOwner().lock();

    if (Owner)
    {
        const FVector3 OwnerPos = Owner->GetWorldPos();
        OutWorldPos.x += OwnerPos.x;
        OutWorldPos.y += OwnerPos.y;
    }

    return true;
}

bool CPlacementController::ResolveRoadPathToTile(
    int EndTileIndex,
    std::vector<int>& OutPathIndices) const
{
    OutPathIndices.clear();

    if (mRoadPathStartTileIndex < 0 || EndTileIndex < 0)
        return false;

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!TryGetTileMap(TileMap))
        return false;

    if (!IsRoadPathTileTraversable(TileMap, mRoadPathStartTileIndex) ||
        !IsRoadPathTileTraversable(TileMap, EndTileIndex))
    {
        return false;
    }

    const int TileCount =
        TileMap->GetTileCountX() * TileMap->GetTileCountY();

    if (TileCount <= 0 ||
        mRoadPathStartTileIndex >= TileCount ||
        EndTileIndex >= TileCount)
    {
        return false;
    }

    std::vector<float> GScore(
        TileCount, (std::numeric_limits<float>::max)());
    std::vector<float> FScore(
        TileCount, (std::numeric_limits<float>::max)());
    std::vector<int> Parents(TileCount, -1);
    std::vector<unsigned char> Open(TileCount, 0);
    std::vector<unsigned char> Closed(TileCount, 0);

    GScore[mRoadPathStartTileIndex] = 0.f;
    FScore[mRoadPathStartTileIndex] = EstimateRoadPathCost(
        TileMap,
        mRoadPathStartTileIndex,
        EndTileIndex);
    Open[mRoadPathStartTileIndex] = 1;

    while (true)
    {
        int CurrentIndex = -1;
        float BestScore = (std::numeric_limits<float>::max)();

        for (int i = 0; i < TileCount; ++i)
        {
            if (!Open[i] || Closed[i])
                continue;

            if (FScore[i] >= BestScore)
                continue;

            BestScore = FScore[i];
            CurrentIndex = i;
        }

        if (CurrentIndex < 0)
            break;

        if (CurrentIndex == EndTileIndex)
            break;

        Open[CurrentIndex] = 0;
        Closed[CurrentIndex] = 1;

        for (int DirIndex = 0; DirIndex < GRoadPathDirCount; ++DirIndex)
        {
            const int NextIndex = GetRoadPathNeighborIndex(
                TileMap,
                CurrentIndex,
                DirIndex);

            if (NextIndex < 0 ||
                Closed[NextIndex] ||
                !IsRoadPathTileTraversable(TileMap, NextIndex))
            {
                continue;
            }

            const float CandidateScore = GScore[CurrentIndex] + 1.f;

            if (CandidateScore >= GScore[NextIndex])
                continue;

            Parents[NextIndex] = CurrentIndex;
            GScore[NextIndex] = CandidateScore;
            FScore[NextIndex] = CandidateScore +
                EstimateRoadPathCost(TileMap, NextIndex, EndTileIndex);
            Open[NextIndex] = 1;
        }
    }

    if (EndTileIndex != mRoadPathStartTileIndex &&
        Parents[EndTileIndex] < 0)
    {
        return false;
    }

    int CurrentIndex = EndTileIndex;

    while (CurrentIndex >= 0)
    {
        OutPathIndices.push_back(CurrentIndex);

        if (CurrentIndex == mRoadPathStartTileIndex)
            break;

        CurrentIndex = Parents[CurrentIndex];
    }

    if (OutPathIndices.empty() ||
        OutPathIndices.back() != mRoadPathStartTileIndex)
    {
        OutPathIndices.clear();
        return false;
    }

    std::reverse(OutPathIndices.begin(), OutPathIndices.end());
    return true;
}

bool CPlacementController::IsRoadPathTileTraversable(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int TileIndex) const
{
    if (!TileMap || TileIndex < 0)
        return false;

    auto Tile = TileMap->GetTile(TileIndex).lock();

    if (!Tile)
        return false;

    return TileMap->IsRoadTile(TileIndex) ||
        Tile->GetType() != ETileType::UnableToMove;
}

void CPlacementController::ApplyRoadPreviewTiles(
    const std::vector<int>& TileIndices)
{
    if (mRoadPreviewTileIndices == TileIndices)
        return;

    std::shared_ptr<CTileMapComponent> RoadPreviewTileMap;

    if (!TryGetRoadPreviewTileMap(RoadPreviewTileMap))
    {
        mRoadPreviewTileIndices.clear();
        return;
    }

    for (size_t i = 0; i < mRoadPreviewTileIndices.size(); ++i)
    {
        auto Tile = RoadPreviewTileMap->GetTile(
            mRoadPreviewTileIndices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetOutLineColor(1.f, 0.72f, 0.20f, 0.f);
    }

    mRoadPreviewTileIndices = TileIndices;

    for (size_t i = 0; i < mRoadPreviewTileIndices.size(); ++i)
    {
        auto Tile = RoadPreviewTileMap->GetTile(
            mRoadPreviewTileIndices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetOutLineColor(1.f, 0.72f, 0.20f, GRoadPreviewAlpha);
    }
}

bool CPlacementController::EnsureRoadPlacementObject()
{
    auto ActivePlacementObject = mActivePlacementObject.lock();

    if (ActivePlacementObject &&
        ActivePlacementObject->IsRoad() &&
        !ActivePlacementObject->HasPlacedArea())
    {
        return true;
    }

    return RestartRoadBrushPlacement();
}

bool CPlacementController::TryPlaceRoadAtTile(
    int TileIndex,
    CPlacementAreaObject::FScopedTopologyBatch* TopologyBatch)
{
    if (TileIndex < 0 || !EnsureRoadPlacementObject())
        return false;

    auto ActivePlacementObject = mActivePlacementObject.lock();

    if (!ActivePlacementObject || !ActivePlacementObject->IsRoad())
        return false;

    FVector2 TileWorldPos;

    if (!TryGetTileWorldPos(TileIndex, TileWorldPos))
        return false;

    ActivePlacementObject->StartMovePreview(TileWorldPos);
    ActivePlacementObject->ConfirmPlacement(TopologyBatch);

    const bool IsPlaced = ActivePlacementObject->HasPlacedArea();

    if (!IsPlaced)
    {
        ActivePlacementObject->CancelMovePreview();
        return false;
    }

    mActivePlacementObject.reset();
    return true;
}

bool CPlacementController::CommitRoadPathToTile(int EndTileIndex)
{
    std::vector<int> PathIndices;

    if (!ResolveRoadPathToTile(EndTileIndex, PathIndices) ||
        PathIndices.empty())
    {
        return false;
    }

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!TryGetTileMap(TileMap))
        return false;

    std::vector<int> BuildIndices;

    for (size_t i = 0; i < PathIndices.size(); ++i)
    {
        if (!TileMap->IsRoadTile(PathIndices[i]))
            BuildIndices.push_back(PathIndices[i]);
    }

    CPlacementAreaObject::FScopedTopologyBatch TopologyBatch;

    bool PlacedAny = false;

    for (size_t i = 0; i < BuildIndices.size(); ++i)
    {
        if (TryPlaceRoadAtTile(BuildIndices[i], &TopologyBatch))
            PlacedAny = true;
    }

    if (!PlacedAny && BuildIndices.empty())
        return true;

    return PlacedAny;
}

void CPlacementController::ClearRoadBrushMode()
{
    ApplyRoadPreviewTiles(std::vector<int>());
    mRoadBrushMode = false;
    mRoadBrushEntry = FBuildingCatalogEntry();
    mRoadBrushSpriteTexturePath.clear();
    mLastRoadBrushTileIndex = -1;
    mRoadPathStartTileIndex = -1;
    mRoadPreviewEndTileIndex = -1;
}

std::shared_ptr<CPlacementAreaObject> CPlacementController::PickPlacementObject(
    const FVector2& MouseWorldPos)
{
    std::shared_ptr<CPlacementAreaObject> BestObject;
    float BestDistSq = FLT_MAX;

    for (size_t i = 0; i < mPlacementObjects.size(); ++i)
    {
        auto PlacementObject = mPlacementObjects[i].lock();

        if (!PlacementObject)
            continue;

        if (!PlacementObject->ContainsPlacedTile(MouseWorldPos))
            continue;

        const float DistSq =
            PlacementObject->GetCenterDistanceSq(MouseWorldPos);

        if (!BestObject || DistSq < BestDistSq)
        {
            BestObject = PlacementObject;
            BestDistSq = DistSq;
        }
    }

    return BestObject;
}

std::shared_ptr<CBuildingMarkerOrb> CPlacementController::PickCitizenOrb(
    const FVector2& MouseWorldPos)
{
    auto World = mWorld.lock();

    if (!World)
        return std::shared_ptr<CBuildingMarkerOrb>();

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

    if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
    {
        return std::shared_ptr<CBuildingMarkerOrb>();
    }

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    float ViewWidth = static_cast<float>(Resolution.Width);
    float ViewHeight = static_cast<float>(Resolution.Height);

    auto CameraManager = World->GetCameraManager().lock();

    if (CameraManager)
    {
        auto MainCamera = CameraManager->GetMainCamera().lock();

        if (MainCamera &&
            MainCamera->GetProjectionType() ==
            ECameraProjectionType::Ortho)
        {
            ViewWidth = MainCamera->GetViewWidth();
            ViewHeight = MainCamera->GetViewHeight();
        }
    }

    const float WorldPerPixelX = ViewWidth /
        (std::max)(1.f, static_cast<float>(Resolution.Width));
    const float WorldPerPixelY = ViewHeight /
        (std::max)(1.f, static_cast<float>(Resolution.Height));
    const float SafeWorldPerPixel =
        (std::max)(WorldPerPixelX, WorldPerPixelY);
    constexpr float MinPickRadiusPixels = 14.f;

    std::shared_ptr<CBuildingMarkerOrb> BestOrb;
    float BestDistSq = FLT_MAX;

    for (size_t i = 0; i < OrbList.size(); ++i)
    {
        auto Orb = OrbList[i].lock();

        if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
            continue;

        FVector3 OrbPos = Orb->GetWorldPos();
        const float dx = OrbPos.x - MouseWorldPos.x;
        const float dy = OrbPos.y - MouseWorldPos.y;
        const float DistPxSq =
            (dx * dx) /
            ((std::max)(0.0001f, WorldPerPixelX * WorldPerPixelX)) +
            (dy * dy) /
            ((std::max)(0.0001f, WorldPerPixelY * WorldPerPixelY));

        const float BaseWorldRadius =
            (std::max)(6.f, Orb->GetOrbDiameter() * 0.5f);
        const float BaseRadiusPixels =
            BaseWorldRadius / (std::max)(0.0001f, SafeWorldPerPixel);
        const float PickRadiusPixels = (std::max)(
            MinPickRadiusPixels, BaseRadiusPixels);
        const float PickRadiusPxSq = PickRadiusPixels * PickRadiusPixels;

        if (DistPxSq > PickRadiusPxSq)
            continue;

        if (!BestOrb || DistPxSq < BestDistSq)
        {
            BestOrb = Orb;
            BestDistSq = DistPxSq;
        }
    }

    return BestOrb;
}

void CPlacementController::EnsureCitizenInfoWidget()
{
    if (!mCitizenInfoWidget.expired())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    mCitizenInfoWidget =
        UIManager->FindWidget<CCitizenInfoWidget>(GCitizenInfoWidgetName);

    if (!mCitizenInfoWidget.expired())
        return;

    mCitizenInfoWidget =
        UIManager->CreateWidget<CCitizenInfoWidget>(
            GCitizenInfoWidgetName, 200);
}

void CPlacementController::DemolishPlacementObject(
    const std::shared_ptr<class CPlacementAreaObject>& PlacementObject)
{
    if (!PlacementObject)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    const std::string BuildingObjectName = PlacementObject->GetName();

    if (BuildingObjectName.empty())
        return;

    if (mActivePlacementObject.lock() == PlacementObject)
    {
        mActivePlacementObject.reset();
    }

    auto HoverObject = mDemolitionHoverObject.lock();

    if (HoverObject == PlacementObject)
    {
        HoverObject->SetDemolitionHoverActive(false);
        mDemolitionHoverObject.reset();
    }

    PlacementObject->Destroy();

    auto Visual =
        World->FindObject<CBuildingVisual>(
            BuildingObjectName + "_Visual").lock();

    if (Visual && Visual->GetAlive())
        Visual->Destroy();

    UnregisterBuildingFromOrbs(BuildingObjectName);
    RefreshPlacementObjects();
}

void CPlacementController::RegisterBuildingToOrbs(
    const std::string& BuildingObjectName)
{
    if (BuildingObjectName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Building =
        World->FindObject<CPlacementAreaObject>(BuildingObjectName).lock();

    if (Building && (Building->IsRoad() || Building->IsBusStop()))
        return;

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

    if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        return;

    for (size_t i = 0; i < OrbList.size(); ++i)
    {
        auto Orb = OrbList[i].lock();

        if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
            continue;

        Orb->AddTargetBuildingName(BuildingObjectName);
    }
}

void CPlacementController::UnregisterBuildingFromOrbs(
    const std::string& BuildingObjectName)
{
    if (BuildingObjectName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

    if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        return;

    for (size_t i = 0; i < OrbList.size(); ++i)
    {
        auto Orb = OrbList[i].lock();

        if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
            continue;

        Orb->RemoveTargetBuildingName(BuildingObjectName);
    }
}
