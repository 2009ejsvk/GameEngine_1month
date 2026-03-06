#include "PlacementController.h"
#include "BuildingMarkerOrb.h"
#include "PlacementBuildingVisual.h"
#include "../UI/CitizenInfoWidget.h"
#include "../ObjectNames.h"
#include "Component/CameraComponent.h"
#include "Device.h"
#include "World/World.h"
#include "World/Input.h"
#include "World/WorldUIManager.h"
#include "Object/TileMapObject.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <Windows.h>

namespace
{
    std::string WideToUtf8(const std::wstring& Wide)
    {
        if (Wide.empty())
            return {};

        const int Len = WideCharToMultiByte(
            CP_UTF8, 0,
            Wide.c_str(), static_cast<int>(Wide.size()),
            nullptr, 0, nullptr, nullptr);

        if (Len <= 0)
            return {};

        std::string Result(static_cast<size_t>(Len), '\0');
        WideCharToMultiByte(
            CP_UTF8, 0,
            Wide.c_str(), static_cast<int>(Wide.size()),
            &Result[0], Len, nullptr, nullptr);

        return Result;
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

    return true;
}

void CPlacementController::Update(float DeltaTime)
{
    CGameObject::Update(DeltaTime);

    UpdateDemolitionHoverPreview();
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

    auto ActivePlacementObject = mActivePlacementObject.lock();

    if (ActivePlacementObject &&
        ActivePlacementObject->IsMovePreviewActive())
    {
        ActivePlacementObject->CancelMovePreview();
    }

    mActivePlacementObject.reset();

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

    if (mDemolitionMode)
        SetDemolitionMode(false);

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

    const std::string SafeBuildingId = Entry.Id.empty() ?
        PlacementName :
        Entry.Id;

    PlacementObject->SetTileMapObject(TileMapObject);
    PlacementObject->SetAutoPlaceOnPrepare(false);
    PlacementObject->SetBuildingId(SafeBuildingId);
    PlacementObject->SetBuildingSpriteTexturePath(SpriteTexturePath);
    PlacementObject->SetBuildingKind(Entry.BuildingKind);
    PlacementObject->SetBuildingDisplayInfo(
        WideToUtf8(Entry.DisplayName),
        WideToUtf8(Entry.CategoryName),
        Entry.Residential,
        Entry.Capacity,
        Entry.FoodProvider,
        Entry.EntertainmentProvider,
        Entry.HousingSatisfactionCap,
        Entry.JobSatisfactionCap,
        Entry.FoodSatisfactionCap,
        Entry.FunSatisfactionCap);
    PlacementObject->SetPlacementTemplateType(Entry.TemplateType);

    auto VisualWeak = World->CreateGameObject<CBuildingVisual>(
        PlacementName + "_Visual");
    auto Visual = VisualWeak.lock();

    if (Visual)
        Visual->SetBuilding(PlacementObjectWeak);

    mActivePlacementObject = PlacementObject;
    PlacementObject->StartMovePreview(Input->GetMouseWorldPos());

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
    if (mDemolitionMode)
    {
        SetDemolitionMode(false);
        return;
    }

    auto ActiveObject = mActivePlacementObject.lock();

    if (ActiveObject && ActiveObject->IsMovePreviewActive())
    {
        ActiveObject->CancelMovePreview();
        mActivePlacementObject.reset();
        return;
    }

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

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

    auto CitizenInfoPanelWidget = mCitizenInfoWidget.lock();

    if (CitizenInfoPanelWidget && CitizenInfoPanelWidget->GetEnable())
    {
        const FVector3& PanelPos = CitizenInfoPanelWidget->GetPos();
        const FVector3& PanelSize = CitizenInfoPanelWidget->GetSize();

        const bool InsideCitizenPanel =
            MouseScreenPos.x >= PanelPos.x &&
            MouseScreenPos.x <= PanelPos.x + PanelSize.x &&
            MouseScreenPos.y >= PanelPos.y &&
            MouseScreenPos.y <= PanelPos.y + PanelSize.y;

        if (InsideCitizenPanel)
            return;
    }

    auto ActivePlacementObject = mActivePlacementObject.lock();

    if (ActivePlacementObject &&
        ActivePlacementObject->IsMovePreviewActive())
    {
        const bool WasPlaced = ActivePlacementObject->HasPlacedArea();
        ActivePlacementObject->ConfirmPlacement();

        const bool IsPlaced = ActivePlacementObject->HasPlacedArea();

        if (!WasPlaced && IsPlaced)
        {
            RegisterBuildingToOrbs(ActivePlacementObject->GetName());
        }

        if (!ActivePlacementObject->IsMovePreviewActive())
            mActivePlacementObject.reset();
        return;
    }

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
