#include "PlacementAreaObject.h"
#include "Component/SceneComponent.h"
#include "Component/MeshComponent.h"
#include "Object/TileMapObject.h"
#include "World/World.h"
#include "World/WorldAssetManager.h"
#include "World/Input.h"
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <unordered_map>

namespace
{
    constexpr int PlacementDuplicateOffsetY = 4;

    struct FOverlayTileState
    {
        CTileMapComponent* TileMap = nullptr;
        std::vector<int> RefCounts;
    };

    FOverlayTileState GPrimaryOverlayState;
    FOverlayTileState GMarkerOverlayState;
    FOverlayTileState GWallOverlayState;
    FOverlayTileState GCeilingOverlayState;

    void EnsureOverlayState(
        FOverlayTileState& State,
        const std::shared_ptr<CTileMapComponent>& TileMap)
    {
        if (!TileMap)
            return;

        const int TileCount =
            TileMap->GetTileCountX() * TileMap->GetTileCountY();

        if (TileCount <= 0)
            return;

        if (State.TileMap != TileMap.get() ||
            (int)State.RefCounts.size() != TileCount)
        {
            State.TileMap = TileMap.get();
            State.RefCounts.clear();
            State.RefCounts.resize(TileCount, 0);
        }
    }

    bool HasOverlayRef(
        const FOverlayTileState& State, int TileIndex)
    {
        if (TileIndex < 0 || TileIndex >= (int)State.RefCounts.size())
            return false;

        return State.RefCounts[TileIndex] > 0;
    }

    void UpdateOverlayTileRefs(
        FOverlayTileState& State,
        const std::shared_ptr<CTileMapComponent>& TileMap,
        std::vector<int>& InOutAppliedIndices,
        const std::vector<int>& NextIndices,
        const FVector4& VisibleColor)
    {
        if (InOutAppliedIndices == NextIndices)
            return;

        EnsureOverlayState(State, TileMap);

        if (State.TileMap != TileMap.get() ||
            State.RefCounts.empty())
        {
            InOutAppliedIndices.clear();
            return;
        }

        const int TileCount = static_cast<int>(State.RefCounts.size());

        for (size_t i = 0; i < InOutAppliedIndices.size(); ++i)
        {
            const int Index = InOutAppliedIndices[i];

            if (Index < 0 || Index >= TileCount)
                continue;

            int& RefCount = State.RefCounts[Index];

            if (RefCount <= 0)
                continue;

            --RefCount;

            if (RefCount > 0)
                continue;

            RefCount = 0;
            auto Tile = TileMap->GetTile(Index).lock();

            if (!Tile)
                continue;

            Tile->SetOutLineColor(VisibleColor.x, VisibleColor.y,
                VisibleColor.z, 0.f);
        }

        InOutAppliedIndices = NextIndices;

        for (size_t i = 0; i < InOutAppliedIndices.size(); ++i)
        {
            const int Index = InOutAppliedIndices[i];

            if (Index < 0 || Index >= TileCount)
                continue;

            int& RefCount = State.RefCounts[Index];
            ++RefCount;

            if (RefCount != 1)
                continue;

            auto Tile = TileMap->GetTile(Index).lock();

            if (!Tile)
                continue;

            Tile->SetOutLineColor(VisibleColor);
        }
    }
}

CPlacementAreaObject::CPlacementAreaObject()
{
    SetClassType<CPlacementAreaObject>();
    mTemplate = CreateTemplateByType(
        EPlacementTemplateType::Diamond3x3SingleMarker);
}

CPlacementAreaObject::CPlacementAreaObject(
    const CPlacementAreaObject& ref) :
    CGameObject(ref)
{
}

CPlacementAreaObject::CPlacementAreaObject(
    CPlacementAreaObject&& ref) noexcept :
    CGameObject(std::move(ref))
{
}

CPlacementAreaObject::~CPlacementAreaObject()
{
}

bool CPlacementAreaObject::Init()
{
    CGameObject::Init();

    CreateComponent<CSceneComponent>("Root");
    mWallMeshComponent = CreateComponent<CMeshComponent>("WallMesh");

    auto WallMesh = mWallMeshComponent.lock();

    if (WallMesh)
    {
        WallMesh->SetShader("MaterialColor2D");
        WallMesh->SetRenderLayer("MapWall");
        WallMesh->SetEnable(false);
    }

    return true;
}

void CPlacementAreaObject::Update(float DeltaTime)
{
    CGameObject::Update(DeltaTime);

    EnsurePlacementObject();

    if (mTileMapPrepared)
    {
        std::shared_ptr<CTileMapComponent> TileMap;

        if (AcquireTileMap(TileMap))
        {
            ApplyPlacedAreaColor(TileMap);
        }
    }

    RefreshWallMeshAnchor();

    if (!mMovePreviewActive)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    UpdatePlacementPreviewFromMouse(Input->GetMouseWorldPos());
}

void CPlacementAreaObject::Destroy()
{
    UpdatePrimaryOverlayTiles(std::vector<int>());
    UpdateMarkerOverlayTiles(std::vector<int>());
    UpdateCeilingOverlayTiles(std::vector<int>());
    ClearWallMesh();
    CGameObject::Destroy();
}

bool CPlacementAreaObject::IsNavigationObstacle() const
{
    return true;
}

void CPlacementAreaObject::SetPlacementTemplateType(
    EPlacementTemplateType Type)
{
    SetPlacementTemplate(CreateTemplateByType(Type));
}

void CPlacementAreaObject::SetPlacementTemplate(
    const FPlacementTemplate& Template)
{
    mTemplate = Template;
    EnsureTemplateValidity();
    ResetPlacementState();
}

void CPlacementAreaObject::GetNavigationGoalTiles(
    std::vector<int>& OutIndices)
{
    EnsurePlacementObject();

    if (!mTileMapPrepared)
        return;

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    if (mMarkerTileIndices.empty())
    {
        ApplyPlacedAreaColor(TileMap);
    }

    for (size_t i = 0; i < mMarkerTileIndices.size(); ++i)
    {
        const int MarkerIndex = mMarkerTileIndices[i];

        if (!IsPlacedIndex(MarkerIndex))
            continue;

        if (std::find(OutIndices.begin(), OutIndices.end(),
            MarkerIndex) == OutIndices.end())
        {
            OutIndices.push_back(MarkerIndex);
        }
    }
}

void CPlacementAreaObject::GetNavigationBlockedTiles(
    std::vector<int>& OutIndices)
{
    EnsurePlacementObject();

    if (!mTileMapPrepared || mPrimaryPlacedIndices.empty())
        return;

    std::vector<int> GoalTiles;
    GetNavigationGoalTiles(GoalTiles);

    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        const int Index = mPrimaryPlacedIndices[i];

        if (std::find(GoalTiles.begin(), GoalTiles.end(), Index) !=
            GoalTiles.end())
        {
            continue;
        }

        OutIndices.push_back(Index);
    }
}

void CPlacementAreaObject::StartMovePreview(
    const FVector2& MouseWorldPos)
{
    EnsurePlacementObject();

    if (!mTileMapPrepared)
        return;

    mMovePreviewActive = true;
    UpdatePlacementPreviewFromMouse(MouseWorldPos);
}

void CPlacementAreaObject::ConfirmPlacement()
{
    EnsurePlacementObject();

    if (!mTileMapPrepared ||
        !mPreviewCanPlace ||
        mPreviewIndices.empty())
    {
        return;
    }

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    mMarkerTileIndices.clear();

    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(mPrimaryPlacedIndices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetTileType(ETileType::Normal);
        Tile->SetOutLineColor(FVector4::White);
    }

    for (size_t i = 0; i < mExtendedPlacedIndices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(mExtendedPlacedIndices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetTileType(ETileType::Normal);
        Tile->SetOutLineColor(FVector4::White);
    }

    std::vector<int> NextPrimaryIndices;
    std::vector<int> NextExtendedIndices;
    std::vector<int> NextPlacedIndices;

    if (!BuildPlacementAreaGroups(
        TileMap, mPreviewCenterIndex,
        NextPrimaryIndices, NextExtendedIndices, NextPlacedIndices))
    {
        return;
    }

    for (size_t i = 0; i < NextPrimaryIndices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(NextPrimaryIndices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetTileType(ETileType::UnableToMove);
    }

    for (size_t i = 0; i < NextExtendedIndices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(NextExtendedIndices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetTileType(ETileType::Normal);
    }

    mPlacedIndices = NextPlacedIndices;
    mPrimaryPlacedIndices = NextPrimaryIndices;
    mExtendedPlacedIndices = NextExtendedIndices;
    UpdateCeilingOverlayTiles(mExtendedPlacedIndices);
    mPlacedCenterIndex = mPreviewCenterIndex;
    mPreviewIndices.clear();
    mPreviewCenterIndex = -1;
    mPreviewCanPlace = false;
    mMovePreviewActive = false;

    ApplyPlacedAreaColor(TileMap);
    SyncWorldPosFromCenter(TileMap, mPlacedCenterIndex);
    RebuildWallMesh(TileMap);
}

void CPlacementAreaObject::CancelMovePreview()
{
    ClearPreview();
    mMovePreviewActive = false;
}

bool CPlacementAreaObject::ContainsPlacedTile(
    const FVector2& MouseWorldPos)
{
    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return false;

    const int TileIndex = TileMap->GetTileIndex(MouseWorldPos);

    if (TileIndex < 0)
        return false;

    return IsPlacedIndex(TileIndex);
}

bool CPlacementAreaObject::ContainsExtendedPlacedTileIndex(int TileIndex)
{
    EnsurePlacementObject();
    return IsExtendedPlacedIndex(TileIndex);
}

float CPlacementAreaObject::GetCenterDistanceSq(
    const FVector2& MouseWorldPos) const
{
    const FVector3 Pos = GetWorldPos();
    const float dx = Pos.x - MouseWorldPos.x;
    const float dy = Pos.y - MouseWorldPos.y;

    return dx * dx + dy * dy;
}

bool CPlacementAreaObject::GetMarkerWorldPos(FVector3& OutWorldPos)
{
    std::vector<FVector3> MarkerWorldPosList;

    if (!GetMarkerWorldPositions(MarkerWorldPosList) ||
        MarkerWorldPosList.empty())
    {
        return false;
    }

    OutWorldPos = MarkerWorldPosList[0];

    return true;
}

bool CPlacementAreaObject::GetMarkerWorldPositions(
    std::vector<FVector3>& OutWorldPosList)
{
    OutWorldPosList.clear();

    EnsurePlacementObject();

    if (!mTileMapPrepared)
        return false;

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return false;

    if (mMarkerTileIndices.empty())
    {
        ApplyPlacedAreaColor(TileMap);
    }

    if (mMarkerTileIndices.empty())
        return false;

    auto TileMapObj = mTileMapObject.lock();

    if (!TileMapObj)
        return false;

    const FVector3 TileMapWorldPos = TileMapObj->GetWorldPos();

    for (size_t i = 0; i < mMarkerTileIndices.size(); ++i)
    {
        auto MarkerTile = TileMap->GetTile(mMarkerTileIndices[i]).lock();

        if (!MarkerTile)
            continue;

        const FVector2 MarkerCenter = MarkerTile->GetCenter();
        OutWorldPosList.push_back(FVector3(
            MarkerCenter.x + TileMapWorldPos.x,
            MarkerCenter.y + TileMapWorldPos.y,
            0.f));
    }

    return !OutWorldPosList.empty();
}

bool CPlacementAreaObject::GetClosestMarkerWorldPos(
    const FVector3& RefWorldPos, FVector3& OutWorldPos)
{
    std::vector<FVector3> MarkerWorldPosList;

    if (!GetMarkerWorldPositions(MarkerWorldPosList) ||
        MarkerWorldPosList.empty())
    {
        return false;
    }

    float BestDistSq = FLT_MAX;
    int BestIndex = -1;

    for (size_t i = 0; i < MarkerWorldPosList.size(); ++i)
    {
        const FVector3 Delta = MarkerWorldPosList[i] - RefWorldPos;
        const float DistSq = Delta.x * Delta.x +
            Delta.y * Delta.y +
            Delta.z * Delta.z;

        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            BestIndex = (int)i;
        }
    }

    if (BestIndex < 0)
        return false;

    OutWorldPos = MarkerWorldPosList[BestIndex];

    return true;
}

bool CPlacementAreaObject::GetTileSize(FVector2& OutTileSize)
{
    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return false;

    OutTileSize = TileMap->GetTileSize();

    return true;
}

FPlacementTemplate CPlacementAreaObject::CreateTemplateByType(
    EPlacementTemplateType Type)
{
    FPlacementTemplate Template;
    Template.Type = Type;
    Template.AreaColor = FVector4::Blue;

    switch (Type)
    {
    case EPlacementTemplateType::Diamond5x5TwoMarker:
        Template.DiamondRadius = 2;
        Template.MarkerAnchors.push_back({ 1.f, 0.5f });
        Template.MarkerAnchors.push_back({ -1.f, -0.5f });
        break;

    case EPlacementTemplateType::Diamond5x5FourMarker:
        Template.DiamondRadius = 2;
        Template.MarkerAnchors.push_back({ 1.f, 0.5f });
        Template.MarkerAnchors.push_back({ -1.f, -0.5f });
        Template.MarkerAnchors.push_back({ 0.5f, -1.f });
        Template.MarkerAnchors.push_back({ -0.5f, 1.f });
        break;

    case EPlacementTemplateType::Diamond7x7ThreeMarker:
        Template.DiamondRadius = 3;
        Template.MarkerAnchors.push_back({ 1.5f, 1.f });
        Template.MarkerAnchors.push_back({ -1.5f, -1.f });
        Template.MarkerAnchors.push_back({ 0.f, 0.f });
        break;

    case EPlacementTemplateType::Diamond3x3SingleMarker:
    default:
        Template.DiamondRadius = 1;
        Template.MarkerAnchors.push_back({ 0.5f, 0.5f });
        break;
    }

    return Template;
}

void CPlacementAreaObject::EnsureTemplateValidity()
{
    if (mTemplate.DiamondRadius < 1)
        mTemplate.DiamondRadius = 1;

    if (mTemplate.MarkerAnchors.empty())
    {
        mTemplate.MarkerAnchors.push_back({ 0.5f, 0.5f });
    }
}

void CPlacementAreaObject::ResetPlacementState()
{
    UpdatePrimaryOverlayTiles(std::vector<int>());
    UpdateMarkerOverlayTiles(std::vector<int>());
    UpdateCeilingOverlayTiles(std::vector<int>());
    ClearWallMesh();
    mPlacedIndices.clear();
    mPrimaryPlacedIndices.clear();
    mExtendedPlacedIndices.clear();
    mAppliedPrimaryOverlayIndices.clear();
    mAppliedMarkerOverlayIndices.clear();
    mAppliedWallOverlayIndices.clear();
    mAppliedCeilingOverlayIndices.clear();
    mPreviewIndices.clear();
    mPreviewCanPlace = false;
    mTileMapPrepared = false;
    mMovePreviewActive = false;
    mPlacedCenterIndex = -1;
    mPreviewCenterIndex = -1;
    mMarkerTileIndices.clear();
}

void CPlacementAreaObject::EnsurePlacementObject()
{
    if (mTileMapPrepared)
        return;

    EnsureTemplateValidity();

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    const int CountX = TileMap->GetTileCountX();
    const int CountY = TileMap->GetTileCountY();

    if (CountX <= 0 || CountY <= 0)
        return;

    // 여러 배치 오브젝트가 있어도 동일 타일맵에 대해서만 1회 초기화한다.
    static CTileMapComponent* sInitializedTileMap = nullptr;

    if (sInitializedTileMap != TileMap.get())
    {
        const int TileCount = CountX * CountY;

        for (int i = 0; i < TileCount; ++i)
        {
            auto Tile = TileMap->GetTile(i).lock();

            if (!Tile)
                continue;

            if (Tile->GetType() == ETileType::UnableToMove)
                Tile->SetOutLineColor(FVector4::Blue);

            else
                Tile->SetOutLineColor(FVector4::White);
        }

        sInitializedTileMap = TileMap.get();
    }

    std::vector<int> StartIndices;
    std::vector<int> StartPrimaryIndices;
    std::vector<int> StartExtendedIndices;
    int StartCenterIndex = -1;

    const int CenterX = Clamp<int>(
        CountX / 2 + mInitialCenterOffsetX, 0, CountX - 1);
    const int CenterY = Clamp<int>(
        CountY / 2 + mInitialCenterOffsetY, 0, CountY - 1);
    const int CenterIndex = CenterY * CountX + CenterX;

    bool Found = BuildPlacementAreaGroups(TileMap,
        CenterIndex,
        StartPrimaryIndices, StartExtendedIndices, StartIndices) &&
        IsAreaPlaceable(TileMap, StartIndices);

    if (Found)
    {
        StartCenterIndex = CenterIndex;
    }

    if (!Found)
    {
        for (int y = 0; y < CountY && !Found; ++y)
        {
            for (int x = 0; x < CountX; ++x)
            {
                const int Index = y * CountX + x;

                if (!BuildPlacementAreaGroups(TileMap,
                    Index,
                    StartPrimaryIndices,
                    StartExtendedIndices,
                    StartIndices))
                {
                    continue;
                }

                if (IsAreaPlaceable(TileMap, StartIndices))
                {
                    Found = true;
                    StartCenterIndex = Index;
                    break;
                }
            }
        }
    }

    if (Found)
    {
        for (size_t i = 0; i < StartPrimaryIndices.size(); ++i)
        {
            auto Tile = TileMap->GetTile(StartPrimaryIndices[i]).lock();

            if (!Tile)
                continue;

            Tile->SetTileType(ETileType::UnableToMove);
        }

        for (size_t i = 0; i < StartExtendedIndices.size(); ++i)
        {
            auto Tile = TileMap->GetTile(StartExtendedIndices[i]).lock();

            if (!Tile)
                continue;

            Tile->SetTileType(ETileType::Normal);
        }

        mPlacedIndices = StartIndices;
        mPrimaryPlacedIndices = StartPrimaryIndices;
        mExtendedPlacedIndices = StartExtendedIndices;
        UpdateCeilingOverlayTiles(mExtendedPlacedIndices);
        mPlacedCenterIndex = StartCenterIndex;
        ApplyPlacedAreaColor(TileMap);
        SyncWorldPosFromCenter(TileMap, mPlacedCenterIndex);
        RebuildWallMesh(TileMap);
    }

    else
    {
        ClearWallMesh();
    }

    mPreviewIndices.clear();
    mPreviewCenterIndex = -1;
    mPreviewCanPlace = false;
    mTileMapPrepared = true;
}

void CPlacementAreaObject::UpdatePlacementPreviewFromMouse(
    const FVector2& MouseWorldPos)
{
    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    ClearPreview();

    const int CenterIndex = TileMap->GetTileIndex(MouseWorldPos);

    if (CenterIndex < 0)
        return;

    if (!BuildPlacementAreaIndices(TileMap,
        CenterIndex, mPreviewIndices))
    {
        return;
    }

    mPreviewCenterIndex = CenterIndex;
    mPreviewCanPlace = IsAreaPlaceable(TileMap, mPreviewIndices);

    const FVector4 PreviewColor = mPreviewCanPlace ?
        FVector4::Green : FVector4::Red;

    SetAreaColor(TileMap, mPreviewIndices, PreviewColor);
}

void CPlacementAreaObject::ClearPreview()
{
    if (mPreviewIndices.empty())
    {
        mPreviewCanPlace = false;
        mPreviewCenterIndex = -1;
        return;
    }

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
    {
        mPreviewIndices.clear();
        mPreviewCanPlace = false;
        mPreviewCenterIndex = -1;
        return;
    }

    for (size_t i = 0; i < mPreviewIndices.size(); ++i)
    {
        RestoreTileColor(TileMap, mPreviewIndices[i]);
    }

    mPreviewIndices.clear();
    mPreviewCanPlace = false;
    mPreviewCenterIndex = -1;
}

bool CPlacementAreaObject::AcquireTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap)
{
    auto World = mWorld.lock();

    if (!World)
        return false;

    if (mTileMapObject.expired())
    {
        mTileMapObject = World->FindObject<CTileMapObject>("TileMap");
    }

    auto TileMapObj = mTileMapObject.lock();

    if (!TileMapObj)
        return false;

    OutTileMap = TileMapObj->GetTileMap().lock();

    return OutTileMap != nullptr;
}

bool CPlacementAreaObject::AcquireBlueOverlayTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap)
{
    auto World = mWorld.lock();

    if (!World)
        return false;

    if (mBlueOverlayTileMapObject.expired())
    {
        mBlueOverlayTileMapObject =
            World->FindObject<CTileMapObject>("TileMapFloorBlue");
    }

    auto BlueOverlayObj = mBlueOverlayTileMapObject.lock();

    if (!BlueOverlayObj)
        return false;

    OutTileMap = BlueOverlayObj->GetTileMap().lock();

    return OutTileMap != nullptr;
}

bool CPlacementAreaObject::AcquireYellowOverlayTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap)
{
    auto World = mWorld.lock();

    if (!World)
        return false;

    if (mYellowOverlayTileMapObject.expired())
    {
        mYellowOverlayTileMapObject =
            World->FindObject<CTileMapObject>("TileMapFloorYellow");
    }

    auto YellowOverlayObj = mYellowOverlayTileMapObject.lock();

    if (!YellowOverlayObj)
        return false;

    OutTileMap = YellowOverlayObj->GetTileMap().lock();

    return OutTileMap != nullptr;
}

bool CPlacementAreaObject::AcquireWallTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap)
{
    auto World = mWorld.lock();

    if (!World)
        return false;

    if (mWallTileMapObject.expired())
    {
        mWallTileMapObject =
            World->FindObject<CTileMapObject>("TileMapWall");
    }

    auto WallTileMapObj = mWallTileMapObject.lock();

    if (!WallTileMapObj)
        return false;

    OutTileMap = WallTileMapObj->GetTileMap().lock();

    return OutTileMap != nullptr;
}

bool CPlacementAreaObject::AcquireCeilingTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap)
{
    auto World = mWorld.lock();

    if (!World)
        return false;

    if (mCeilingTileMapObject.expired())
    {
        mCeilingTileMapObject =
            World->FindObject<CTileMapObject>("TileMapCeiling");
    }

    auto CeilingTileMapObj = mCeilingTileMapObject.lock();

    if (!CeilingTileMapObj)
        return false;

    OutTileMap = CeilingTileMapObj->GetTileMap().lock();

    return OutTileMap != nullptr;
}

void CPlacementAreaObject::UpdatePrimaryOverlayTiles(
    const std::vector<int>& NextIndices)
{
    std::shared_ptr<CTileMapComponent> BlueOverlayTileMap;

    if (!AcquireBlueOverlayTileMap(BlueOverlayTileMap))
    {
        mAppliedPrimaryOverlayIndices.clear();
        return;
    }

    UpdateOverlayTileRefs(
        GPrimaryOverlayState,
        BlueOverlayTileMap,
        mAppliedPrimaryOverlayIndices,
        NextIndices,
        FVector4::Blue);
}

void CPlacementAreaObject::UpdateMarkerOverlayTiles(
    const std::vector<int>& NextIndices)
{
    std::shared_ptr<CTileMapComponent> YellowOverlayTileMap;

    if (!AcquireYellowOverlayTileMap(YellowOverlayTileMap))
    {
        mAppliedMarkerOverlayIndices.clear();
        return;
    }

    UpdateOverlayTileRefs(
        GMarkerOverlayState,
        YellowOverlayTileMap,
        mAppliedMarkerOverlayIndices,
        NextIndices,
        FVector4(1.f, 1.f, 0.f, 1.f));
}

void CPlacementAreaObject::UpdateWallOverlayTiles(
    const std::vector<int>& NextIndices)
{
    std::shared_ptr<CTileMapComponent> WallTileMap;

    if (!AcquireWallTileMap(WallTileMap))
    {
        mAppliedWallOverlayIndices.clear();
        return;
    }

    UpdateOverlayTileRefs(
        GWallOverlayState,
        WallTileMap,
        mAppliedWallOverlayIndices,
        NextIndices,
        FVector4(0.42f, 0.57f, 0.86f, 1.f));
}

void CPlacementAreaObject::UpdateCeilingOverlayTiles(
    const std::vector<int>& NextIndices)
{
    std::shared_ptr<CTileMapComponent> CeilingTileMap;

    if (!AcquireCeilingTileMap(CeilingTileMap))
    {
        mAppliedCeilingOverlayIndices.clear();
        return;
    }

    UpdateOverlayTileRefs(
        GCeilingOverlayState,
        CeilingTileMap,
        mAppliedCeilingOverlayIndices,
        NextIndices,
        FVector4::Green);
}

void CPlacementAreaObject::BuildWallIndices(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    std::vector<int>& OutIndices) const
{
    OutIndices.clear();

    if (!TileMap ||
        mPrimaryPlacedIndices.empty() ||
        mExtendedPlacedIndices.empty())
    {
        return;
    }

    const int CountX = TileMap->GetTileCountX();
    const int CountY = TileMap->GetTileCountY();
    const int TileCount = CountX * CountY;
    const int WallHeight = std::abs(PlacementDuplicateOffsetY);

    if (CountX <= 0 ||
        CountY <= 0 ||
        TileCount <= 0 ||
        WallHeight <= 1)
    {
        return;
    }

    const int VerticalStep = PlacementDuplicateOffsetY > 0 ? 1 : -1;
    std::vector<unsigned char> PrimaryMask(TileCount, 0);
    std::vector<unsigned char> ExtendedMask(TileCount, 0);
    std::vector<unsigned char> AddedMask(TileCount, 0);

    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        const int Index = mPrimaryPlacedIndices[i];

        if (Index < 0 || Index >= TileCount)
            continue;

        PrimaryMask[Index] = 1;
    }

    for (size_t i = 0; i < mExtendedPlacedIndices.size(); ++i)
    {
        const int Index = mExtendedPlacedIndices[i];

        if (Index < 0 || Index >= TileCount)
            continue;

        ExtendedMask[Index] = 1;
    }

    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        const int PrimaryIndex = mPrimaryPlacedIndices[i];

        if (PrimaryIndex < 0 || PrimaryIndex >= TileCount)
            continue;

        if (!IsPlacedEdgeTile(TileMap, PrimaryIndex))
            continue;

        auto PrimaryTile = TileMap->GetTile(PrimaryIndex).lock();

        if (!PrimaryTile)
            continue;

        const int PrimaryY = PrimaryTile->GetIndexY();
        const float PrimaryLogicalX = PrimaryTile->GetIndexX() +
            ((PrimaryY % 2 == 0) ? 0.f : 0.5f);

        for (int Layer = 1; Layer < WallHeight; ++Layer)
        {
            const int WallY = PrimaryY + VerticalStep * Layer;

            if (WallY < 0 || WallY >= CountY)
                break;

            const float WallOffsetX = (WallY % 2 == 0) ? 0.f : 0.5f;
            const float WallXFloat = PrimaryLogicalX - WallOffsetX;
            const int WallX = static_cast<int>(std::round(WallXFloat));

            if (WallX < 0 || WallX >= CountX)
                continue;

            const int WallIndex = WallY * CountX + WallX;

            if (WallIndex < 0 || WallIndex >= TileCount)
                continue;

            if (PrimaryMask[WallIndex] || ExtendedMask[WallIndex])
                continue;

            if (AddedMask[WallIndex])
                continue;

            AddedMask[WallIndex] = 1;
            OutIndices.push_back(WallIndex);
        }
    }
}

bool CPlacementAreaObject::BuildWallMeshGeometry(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    std::vector<FVertexColor>& OutVertices,
    std::vector<unsigned int>& OutIndices) const
{
    OutVertices.clear();
    OutIndices.clear();

    if (!TileMap ||
        mPrimaryPlacedIndices.empty() ||
        mExtendedPlacedIndices.empty())
    {
        return false;
    }

    const int CountX = TileMap->GetTileCountX();
    const int CountY = TileMap->GetTileCountY();
    const int TileCount = CountX * CountY;

    if (CountX <= 0 || CountY <= 0 || TileCount <= 0)
        return false;

    std::vector<unsigned char> ExtendedMask(TileCount, 0);

    for (size_t i = 0; i < mExtendedPlacedIndices.size(); ++i)
    {
        const int Index = mExtendedPlacedIndices[i];

        if (Index < 0 || Index >= TileCount)
            continue;

        ExtendedMask[Index] = 1;
    }

    FVector2 ExtrudeVector;
    bool HasExtrudeVector = false;

    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        const int FloorIndex = mPrimaryPlacedIndices[i];

        if (FloorIndex < 0 || FloorIndex >= TileCount)
            continue;

        auto FloorTile = TileMap->GetTile(FloorIndex).lock();

        if (!FloorTile)
            continue;

        const int PairX = FloorTile->GetIndexX();
        const int PairY = FloorTile->GetIndexY() +
            PlacementDuplicateOffsetY;

        if (PairX < 0 || PairX >= CountX ||
            PairY < 0 || PairY >= CountY)
        {
            continue;
        }

        const int PairIndex = PairY * CountX + PairX;

        if (PairIndex < 0 || PairIndex >= TileCount ||
            !ExtendedMask[PairIndex])
        {
            continue;
        }

        auto CeilingTile = TileMap->GetTile(PairIndex).lock();

        if (!CeilingTile)
            continue;

        ExtrudeVector = CeilingTile->GetCenter() -
            FloorTile->GetCenter();

        if (ExtrudeVector.Length() <= 0.001f)
            continue;

        HasExtrudeVector = true;
        break;
    }

    if (!HasExtrudeVector)
        return false;

    struct FEdgeKey
    {
        int x0 = 0;
        int y0 = 0;
        int x1 = 0;
        int y1 = 0;

        bool operator==(const FEdgeKey& Other) const
        {
            return x0 == Other.x0 &&
                y0 == Other.y0 &&
                x1 == Other.x1 &&
                y1 == Other.y1;
        }
    };

    struct FEdgeKeyHasher
    {
        size_t operator()(const FEdgeKey& Key) const
        {
            size_t Hash = std::hash<int>()(Key.x0);
            Hash ^= std::hash<int>()(Key.y0) + 0x9e3779b9 +
                (Hash << 6) + (Hash >> 2);
            Hash ^= std::hash<int>()(Key.x1) + 0x9e3779b9 +
                (Hash << 6) + (Hash >> 2);
            Hash ^= std::hash<int>()(Key.y1) + 0x9e3779b9 +
                (Hash << 6) + (Hash >> 2);
            return Hash;
        }
    };

    struct FEdgeData
    {
        FVector2 Start;
        FVector2 End;
        int Count = 0;
    };

    auto QuantizePoint = [](const FVector2& Point)
    {
        return std::pair<int, int>(
            static_cast<int>(std::round(Point.x * 1000.f)),
            static_cast<int>(std::round(Point.y * 1000.f)));
    };

    auto MakeEdgeKey = [&](const FVector2& Start, const FVector2& End)
    {
        auto A = QuantizePoint(Start);
        auto B = QuantizePoint(End);

        FEdgeKey Key;

        if (A < B)
        {
            Key.x0 = A.first;
            Key.y0 = A.second;
            Key.x1 = B.first;
            Key.y1 = B.second;
        }

        else
        {
            Key.x0 = B.first;
            Key.y0 = B.second;
            Key.x1 = A.first;
            Key.y1 = A.second;
        }

        return Key;
    };

    std::unordered_map<FEdgeKey, FEdgeData, FEdgeKeyHasher> EdgeMap;

    auto AddEdge = [&](const FVector2& Start, const FVector2& End)
    {
        const FEdgeKey Key = MakeEdgeKey(Start, End);
        auto& Edge = EdgeMap[Key];

        if (Edge.Count == 0)
        {
            Edge.Start = Start;
            Edge.End = End;
        }

        ++Edge.Count;
    };

    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        const int Index = mPrimaryPlacedIndices[i];
        auto Tile = TileMap->GetTile(Index).lock();

        if (!Tile)
            continue;

        const FVector2 Center = Tile->GetCenter();
        const FVector2 TileSize = Tile->GetSize();
        const float HalfWidth = TileSize.x * 0.5f;
        const float HalfHeight = TileSize.y * 0.5f;

        if (HalfWidth <= 0.f || HalfHeight <= 0.f)
            continue;

        const FVector2 Top(Center.x, Center.y - HalfHeight);
        const FVector2 Right(Center.x + HalfWidth, Center.y);
        const FVector2 Bottom(Center.x, Center.y + HalfHeight);
        const FVector2 Left(Center.x - HalfWidth, Center.y);

        AddEdge(Top, Right);
        AddEdge(Right, Bottom);
        AddEdge(Bottom, Left);
        AddEdge(Left, Top);
    }

    const FVector4 BottomColor(0.34f, 0.47f, 0.71f, 1.f);
    const FVector4 TopColor(0.48f, 0.62f, 0.88f, 1.f);

    for (auto Iter = EdgeMap.begin(); Iter != EdgeMap.end(); ++Iter)
    {
        const FEdgeData& Edge = Iter->second;

        if (Edge.Count != 1)
            continue;

        const FVector2 V0 = Edge.Start;
        const FVector2 V1 = Edge.End;
        const FVector2 V2 = Edge.End + ExtrudeVector;
        const FVector2 V3 = Edge.Start + ExtrudeVector;

        const int BaseIndex = static_cast<int>(OutVertices.size());

        OutVertices.push_back(FVertexColor(
            V0.x, V0.y, 0.f,
            BottomColor.x, BottomColor.y, BottomColor.z, BottomColor.w));
        OutVertices.push_back(FVertexColor(
            V1.x, V1.y, 0.f,
            BottomColor.x, BottomColor.y, BottomColor.z, BottomColor.w));
        OutVertices.push_back(FVertexColor(
            V2.x, V2.y, 0.f,
            TopColor.x, TopColor.y, TopColor.z, TopColor.w));
        OutVertices.push_back(FVertexColor(
            V3.x, V3.y, 0.f,
            TopColor.x, TopColor.y, TopColor.z, TopColor.w));

        // 양면 렌더링 보장을 위해 양쪽 winding을 모두 넣는다.
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 0));
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 1));
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 2));
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 0));
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 2));
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 3));
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 0));
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 2));
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 1));
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 0));
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 3));
        OutIndices.push_back(static_cast<unsigned int>(BaseIndex + 2));
    }

    return !OutVertices.empty() && !OutIndices.empty();
}

void CPlacementAreaObject::RefreshWallMeshAnchor()
{
    auto WallMesh = mWallMeshComponent.lock();
    auto TileMapObj = mTileMapObject.lock();

    if (!WallMesh || !TileMapObj)
        return;

    const FVector3 RelativePos = TileMapObj->GetWorldPos() - GetWorldPos();
    WallMesh->SetRelativePos(RelativePos);
}

void CPlacementAreaObject::RebuildWallMesh(
    const std::shared_ptr<class CTileMapComponent>& TileMap)
{
    auto WallMesh = mWallMeshComponent.lock();

    if (!WallMesh)
        return;

    RefreshWallMeshAnchor();

    std::vector<FVertexColor> Vertices;
    std::vector<unsigned int> Indices;

    if (!BuildWallMeshGeometry(TileMap, Vertices, Indices))
    {
        WallMesh->SetEnable(false);
        return;
    }

    auto World = mWorld.lock();

    if (!World)
    {
        WallMesh->SetEnable(false);
        return;
    }

    auto AssetManager = World->GetWorldAssetManager().lock();

    if (!AssetManager)
    {
        WallMesh->SetEnable(false);
        return;
    }

    ++mWallMeshVersion;
    mWallMeshName = mName + "_WallMesh_" +
        std::to_string(mWallMeshVersion);

    if (!AssetManager->CreateMesh(
        mWallMeshName,
        false,
        Vertices.data(),
        sizeof(FVertexColor),
        static_cast<int>(Vertices.size()),
        D3D11_USAGE_IMMUTABLE,
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
        Indices.data(),
        sizeof(unsigned int),
        static_cast<int>(Indices.size()),
        DXGI_FORMAT_R32_UINT,
        D3D11_USAGE_IMMUTABLE))
    {
        WallMesh->SetEnable(false);
        return;
    }

    WallMesh->SetShader("MaterialColor2D");
    WallMesh->SetMesh(mWallMeshName);
    WallMesh->SetRenderLayer("MapWall");
    WallMesh->SetEnable(true);
}

void CPlacementAreaObject::ClearWallMesh()
{
    auto WallMesh = mWallMeshComponent.lock();

    if (WallMesh)
        WallMesh->SetEnable(false);

    mWallMeshName.clear();
}

bool CPlacementAreaObject::BuildPlacementAreaIndices(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex, std::vector<int>& OutIndices) const
{
    std::vector<int> PrimaryIndices;
    std::vector<int> ExtendedIndices;

    return BuildPlacementAreaGroups(
        TileMap, CenterIndex,
        PrimaryIndices, ExtendedIndices, OutIndices);
}

bool CPlacementAreaObject::BuildPlacementAreaGroups(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex,
    std::vector<int>& OutPrimaryIndices,
    std::vector<int>& OutExtendedIndices,
    std::vector<int>& OutMergedIndices) const
{
    OutPrimaryIndices.clear();
    OutExtendedIndices.clear();
    OutMergedIndices.clear();

    const int ExpectedCount = mTemplate.GetExpectedTileCount();

    if (ExpectedCount <= 0)
        return false;

    if (!BuildDiamondAreaIndices(TileMap, CenterIndex, OutPrimaryIndices) ||
        (int)OutPrimaryIndices.size() != ExpectedCount)
    {
        return false;
    }

    auto CenterTile = TileMap->GetTile(CenterIndex).lock();

    if (!CenterTile)
        return false;

    const int CountX = TileMap->GetTileCountX();
    const int CountY = TileMap->GetTileCountY();
    const int ExtendedCenterX = CenterTile->GetIndexX();
    const int ExtendedCenterY = CenterTile->GetIndexY() +
        PlacementDuplicateOffsetY;

    if (ExtendedCenterX < 0 || ExtendedCenterX >= CountX ||
        ExtendedCenterY < 0 || ExtendedCenterY >= CountY)
    {
        return false;
    }

    const int ExtendedCenterIndex = ExtendedCenterY * CountX +
        ExtendedCenterX;

    if (!BuildDiamondAreaIndices(TileMap,
        ExtendedCenterIndex, OutExtendedIndices) ||
        (int)OutExtendedIndices.size() != ExpectedCount)
    {
        return false;
    }

    OutMergedIndices = OutPrimaryIndices;
    OutMergedIndices.reserve(
        OutPrimaryIndices.size() + OutExtendedIndices.size());

    for (size_t i = 0; i < OutExtendedIndices.size(); ++i)
    {
        const int Index = OutExtendedIndices[i];

        if (std::find(OutMergedIndices.begin(),
            OutMergedIndices.end(), Index) == OutMergedIndices.end())
        {
            OutMergedIndices.push_back(Index);
        }
    }

    return !OutMergedIndices.empty();
}

bool CPlacementAreaObject::BuildDiamondAreaIndices(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex, std::vector<int>& OutIndices) const
{
    OutIndices.clear();

    auto CenterTile = TileMap->GetTile(CenterIndex).lock();

    if (!CenterTile)
        return false;

    const int CountX = TileMap->GetTileCountX();
    const int CountY = TileMap->GetTileCountY();
    const int CenterX = CenterTile->GetIndexX();
    const int CenterY = CenterTile->GetIndexY();

    const int Radius = mTemplate.DiamondRadius;
    const int SearchRange = Radius * 2;
    const float CenterLogicalX = CenterX +
        (CenterY % 2 == 0 ? 0.f : 0.5f);
    const float CenterLogicalY = CenterY * 0.5f;
    const float DiamondRadius = (float)Radius;

    for (int y = CenterY - SearchRange; y <= CenterY + SearchRange; ++y)
    {
        if (y < 0 || y >= CountY)
            continue;

        for (int x = CenterX - SearchRange; x <= CenterX + SearchRange; ++x)
        {
            if (x < 0 || x >= CountX)
                continue;

            const float LogicalX = x + (y % 2 == 0 ? 0.f : 0.5f);
            const float LogicalY = y * 0.5f;
            const float DistX = fabs(LogicalX - CenterLogicalX);
            const float DistY = fabs(LogicalY - CenterLogicalY);

            if (DistX + DistY > DiamondRadius)
                continue;

            OutIndices.push_back(y * CountX + x);
        }
    }

    return !OutIndices.empty();
}

bool CPlacementAreaObject::IsAreaPlaceable(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    const std::vector<int>& Indices) const
{
    if (Indices.empty())
        return false;

    for (size_t i = 0; i < Indices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(Indices[i]).lock();

        if (!Tile)
            return false;

        if (Tile->GetType() == ETileType::UnableToMove &&
            !IsPlacedIndex(Indices[i]))
        {
            return false;
        }
    }

    return true;
}

void CPlacementAreaObject::SetAreaColor(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    const std::vector<int>& Indices, const FVector4& Color)
{
    for (size_t i = 0; i < Indices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(Indices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetOutLineColor(Color);
    }
}

bool CPlacementAreaObject::IsPlacedIndex(int Index) const
{
    for (size_t i = 0; i < mPlacedIndices.size(); ++i)
    {
        if (mPlacedIndices[i] == Index)
            return true;
    }

    return false;
}

bool CPlacementAreaObject::IsPrimaryPlacedIndex(int Index) const
{
    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        if (mPrimaryPlacedIndices[i] == Index)
            return true;
    }

    return false;
}

bool CPlacementAreaObject::IsExtendedPlacedIndex(int Index) const
{
    for (size_t i = 0; i < mExtendedPlacedIndices.size(); ++i)
    {
        if (mExtendedPlacedIndices[i] == Index)
            return true;
    }

    return false;
}

int CPlacementAreaObject::FindMarkerTileIndexByLogicalOffset(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex, const std::vector<int>& Indices,
    float TargetOffsetX, float TargetOffsetY) const
{
    auto CenterTile = TileMap->GetTile(CenterIndex).lock();

    if (!CenterTile)
        return -1;

    const float CenterLogicalX = CenterTile->GetIndexX() +
        (CenterTile->GetIndexY() % 2 == 0 ? 0.f : 0.5f);
    const float CenterLogicalY = CenterTile->GetIndexY() * 0.5f;
    const float TargetLogicalX = CenterLogicalX + TargetOffsetX;
    const float TargetLogicalY = CenterLogicalY + TargetOffsetY;

    int BestIndex = -1;
    float BestScore = FLT_MAX;
    auto IsInArea = [&](int TileIndex)
    {
        return std::find(Indices.begin(), Indices.end(), TileIndex) !=
            Indices.end();
    };

    for (size_t i = 0; i < Indices.size(); ++i)
    {
        const int CandidateIndex = Indices[i];
        bool IsEdge = false;

        for (int Dir = 0; Dir < 8; ++Dir)
        {
            const int Neighbor = GetIsoNeighborIndexByDir(
                TileMap, CandidateIndex, Dir);

            if (Neighbor < 0 || !IsInArea(Neighbor))
            {
                IsEdge = true;
                break;
            }
        }

        if (!IsEdge)
            continue;

        auto Tile = TileMap->GetTile(Indices[i]).lock();

        if (!Tile)
            continue;

        const float LogicalX = Tile->GetIndexX() +
            (Tile->GetIndexY() % 2 == 0 ? 0.f : 0.5f);
        const float LogicalY = Tile->GetIndexY() * 0.5f;
        const float Score = fabs(LogicalX - TargetLogicalX) +
            fabs(LogicalY - TargetLogicalY);

        if (Score < BestScore)
        {
            BestScore = Score;
            BestIndex = Indices[i];
        }
    }

    return BestIndex;
}

void CPlacementAreaObject::ApplyPlacedAreaColor(
    const std::shared_ptr<class CTileMapComponent>& TileMap)
{
    SetAreaColor(TileMap, mPrimaryPlacedIndices, FVector4::White);
    SetAreaColor(TileMap, mExtendedPlacedIndices, FVector4::White);
    UpdatePrimaryOverlayTiles(mPrimaryPlacedIndices);

    mMarkerTileIndices.clear();

    if (mPlacedCenterIndex < 0 ||
        mPlacedIndices.empty())
    {
        UpdateMarkerOverlayTiles(std::vector<int>());
        return;
    }

    std::vector<int> MarkerSourceIndices = mPrimaryPlacedIndices;

    if (MarkerSourceIndices.empty())
    {
        MarkerSourceIndices = mPlacedIndices;
    }

    for (size_t i = 0; i < mTemplate.MarkerAnchors.size(); ++i)
    {
        const FPlacementMarkerAnchor& Marker = mTemplate.MarkerAnchors[i];
        const int MarkerIndex = FindMarkerTileIndexByLogicalOffset(
            TileMap, mPlacedCenterIndex, MarkerSourceIndices,
            Marker.LogicalOffsetX, Marker.LogicalOffsetY);

        if (MarkerIndex < 0 || !IsPlacedIndex(MarkerIndex))
            continue;

        if (std::find(mMarkerTileIndices.begin(),
            mMarkerTileIndices.end(), MarkerIndex) ==
            mMarkerTileIndices.end())
        {
            mMarkerTileIndices.push_back(MarkerIndex);
        }
    }

    if (mMarkerTileIndices.empty() &&
        !mPlacedIndices.empty())
    {
        const int FallbackEdgeIndex = FindMarkerTileIndexByLogicalOffset(
            TileMap, mPlacedCenterIndex, MarkerSourceIndices, 0.f, 0.f);

        if (FallbackEdgeIndex >= 0)
            mMarkerTileIndices.push_back(FallbackEdgeIndex);
    }

    std::vector<int> VisibleMarkerTileIndices;

    for (size_t i = 0; i < mMarkerTileIndices.size(); ++i)
    {
        const int MarkerIndex = mMarkerTileIndices[i];

        // 겹침 구간은 초록(확장 타일) 표시를 우선한다.
        if (IsExtendedPlacedIndex(MarkerIndex))
            continue;

        VisibleMarkerTileIndices.push_back(MarkerIndex);
    }

    UpdateMarkerOverlayTiles(VisibleMarkerTileIndices);
}

void CPlacementAreaObject::RestoreTileColor(
    const std::shared_ptr<class CTileMapComponent>& TileMap, int Index)
{
    auto Tile = TileMap->GetTile(Index).lock();

    if (!Tile)
        return;

    if (HasOverlayRef(GPrimaryOverlayState, Index) ||
        HasOverlayRef(GMarkerOverlayState, Index) ||
        HasOverlayRef(GWallOverlayState, Index) ||
        HasOverlayRef(GCeilingOverlayState, Index))
    {
        Tile->SetOutLineColor(FVector4::White);
    }

    else if (Tile->GetType() == ETileType::UnableToMove)
        Tile->SetOutLineColor(FVector4::Blue);

    else
        Tile->SetOutLineColor(FVector4::White);
}

void CPlacementAreaObject::SyncWorldPosFromCenter(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex)
{
    auto CenterTile = TileMap->GetTile(CenterIndex).lock();

    if (!CenterTile)
        return;

    auto TileMapObj = mTileMapObject.lock();

    if (!TileMapObj)
        return;

    const FVector2 Center = CenterTile->GetCenter();
    const FVector3 TileMapWorldPos = TileMapObj->GetWorldPos();

    SetWorldPos(Center.x + TileMapWorldPos.x,
        Center.y + TileMapWorldPos.y, GetWorldPos().z);
}

bool CPlacementAreaObject::IsPlacedEdgeTile(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int TileIndex) const
{
    if (!IsPlacedIndex(TileIndex))
        return false;

    for (int i = 0; i < 8; ++i)
    {
        const int Neighbor = GetIsoNeighborIndexByDir(
            TileMap, TileIndex, i);

        if (Neighbor < 0 || !IsPlacedIndex(Neighbor))
            return true;
    }

    return false;
}

int CPlacementAreaObject::GetIsoNeighborIndexByDir(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int TileIndex, int DirIndex) const
{
    if (!TileMap || DirIndex < 0 || DirIndex >= 8)
        return -1;

    auto Tile = TileMap->GetTile(TileIndex).lock();

    if (!Tile)
        return -1;

    const int x = Tile->GetIndexX();
    const int y = Tile->GetIndexY();
    const int GridX = x + ((y + (y & 1)) / 2);
    const int GridY = x - (y / 2);
    const int DirX[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
    const int DirY[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    const int NextGridX = GridX + DirX[DirIndex];
    const int NextGridY = GridY + DirY[DirIndex];
    const int NextY = NextGridX - NextGridY;

    if (NextY < 0 || NextY >= TileMap->GetTileCountY())
        return -1;

    const int NextX = NextGridY + (NextY / 2);

    if (NextX < 0 || NextX >= TileMap->GetTileCountX())
        return -1;

    return NextY * TileMap->GetTileCountX() + NextX;
}
