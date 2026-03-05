#include "PlacementAreaObject.h"
#include "Component/SceneComponent.h"
#include "Object/TileMapObject.h"
#include "World/World.h"
#include "World/Input.h"
#include <algorithm>
#include <cmath>
#include <cfloat>

namespace
{
    struct FOverlayTileState
    {
        CTileMapComponent* TileMap = nullptr;
        std::vector<int> RefCounts;
    };

    FOverlayTileState GPrimaryOverlayState;
    FOverlayTileState GMarkerOverlayState;

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

            if (mDemolitionHoverActive &&
                !mPrimaryPlacedIndices.empty())
            {
                SetAreaColor(
                    TileMap, mPrimaryPlacedIndices, FVector4::Red);
            }
        }
    }

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
    std::shared_ptr<CTileMapComponent> TileMap;

    if (AcquireTileMap(TileMap))
    {
        for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
        {
            auto Tile = TileMap->GetTile(mPrimaryPlacedIndices[i]).lock();

            if (!Tile)
                continue;

            Tile->SetTileType(ETileType::Normal);
            Tile->SetOutLineColor(FVector4::White);
        }

        for (size_t i = 0; i < mPreviewIndices.size(); ++i)
        {
            RestoreTileColor(TileMap, mPreviewIndices[i]);
        }
    }

    mPrimaryPlacedIndices.clear();
    mMarkerTileIndices.clear();
    mPreviewIndices.clear();
    mMovePreviewActive = false;
    mPlacedCenterIndex = -1;
    mPreviewCenterIndex = -1;
    mPreviewCanPlace = false;
    mDemolitionHoverActive = false;

    UpdatePrimaryOverlayTiles(std::vector<int>());
    UpdateMarkerOverlayTiles(std::vector<int>());
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

    if (!mTileMapPrepared ||
        mPrimaryPlacedIndices.empty())
        return;

    std::vector<int> GoalTiles;
    GetNavigationGoalTiles(GoalTiles);

    auto IsGoalTile = [&](int TileIndex)
    {
        return std::find(GoalTiles.begin(), GoalTiles.end(), TileIndex) !=
            GoalTiles.end();
    };

    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        const int Index = mPrimaryPlacedIndices[i];

        if (IsGoalTile(Index))
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

void CPlacementAreaObject::RotatePreviewCW(
    const FVector2& MouseWorldPos)
{
    mPreviewDirection = (mPreviewDirection + 1) % 4;

    if (mMovePreviewActive)
        UpdatePlacementPreviewFromMouse(MouseWorldPos);
}

void CPlacementAreaObject::RotatePreviewCCW(
    const FVector2& MouseWorldPos)
{
    mPreviewDirection = (mPreviewDirection + 3) % 4;

    if (mMovePreviewActive)
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

    std::vector<int> NextPrimaryIndices;

    if (!BuildDiamondAreaIndices(TileMap, mPreviewCenterIndex, NextPrimaryIndices) ||
        (int)NextPrimaryIndices.size() != mTemplate.GetExpectedTileCount())
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

    mPrimaryPlacedIndices = NextPrimaryIndices;
    mPlacedCenterIndex = mPreviewCenterIndex;
    mPreviewIndices.clear();
    mPreviewCenterIndex = -1;
    mPreviewCanPlace = false;
    mMovePreviewActive = false;

    ApplyPlacedAreaColor(TileMap);
    SyncWorldPosFromCenter(TileMap, mPlacedCenterIndex);
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
    Template.HasDirectionalGap = false;

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
        Template.HasDirectionalGap = true;
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
    mPrimaryPlacedIndices.clear();
    mAppliedPrimaryOverlayIndices.clear();
    mAppliedMarkerOverlayIndices.clear();
    mPreviewIndices.clear();
    mPreviewCanPlace = false;
    mTileMapPrepared = false;
    mMovePreviewActive = false;
    mPlacedCenterIndex = -1;
    mPreviewCenterIndex = -1;
    mPreviewDirection = 0;
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

    if (!mAutoPlaceOnPrepare)
    {
        mPrimaryPlacedIndices.clear();
        mMarkerTileIndices.clear();
        mPreviewIndices.clear();
        mPlacedCenterIndex = -1;
        mPreviewCenterIndex = -1;
        mPreviewCanPlace = false;
        mTileMapPrepared = true;
        return;
    }

    std::vector<int> StartPrimaryIndices;
    int StartCenterIndex = -1;

    const int CenterX = Clamp<int>(
        CountX / 2 + mInitialCenterOffsetX, 0, CountX - 1);
    const int CenterY = Clamp<int>(
        CountY / 2 + mInitialCenterOffsetY, 0, CountY - 1);
    const int CenterIndex = CenterY * CountX + CenterX;

    bool Found = BuildDiamondAreaIndices(TileMap,
        CenterIndex, StartPrimaryIndices) &&
        (int)StartPrimaryIndices.size() == mTemplate.GetExpectedTileCount() &&
        IsAreaPlaceable(TileMap, StartPrimaryIndices);

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

                if (!BuildDiamondAreaIndices(TileMap, Index, StartPrimaryIndices) ||
                    (int)StartPrimaryIndices.size() != mTemplate.GetExpectedTileCount())
                {
                    continue;
                }

                if (IsAreaPlaceable(TileMap, StartPrimaryIndices))
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

        mPrimaryPlacedIndices = StartPrimaryIndices;
        mPlacedCenterIndex = StartCenterIndex;
        ApplyPlacedAreaColor(TileMap);
        SyncWorldPosFromCenter(TileMap, mPlacedCenterIndex);
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

    if (!BuildDiamondAreaIndices(TileMap, CenterIndex, mPreviewIndices) ||
        (int)mPreviewIndices.size() != mTemplate.GetExpectedTileCount())
    {
        mPreviewIndices.clear();
        return;
    }

    mPreviewCenterIndex = CenterIndex;
    mPreviewCanPlace = IsAreaPlaceable(TileMap, mPreviewIndices);

    const FVector4 PreviewColor = mPreviewCanPlace ?
        FVector4::Green : FVector4::Red;

    SetAreaColor(TileMap, mPreviewIndices, PreviewColor);

    // 프리뷰에서도 중앙 타일은 노란색(O)으로 표시한다.
    if (std::find(mPreviewIndices.begin(), mPreviewIndices.end(),
        mPreviewCenterIndex) != mPreviewIndices.end())
    {
        auto CenterTile = TileMap->GetTile(mPreviewCenterIndex).lock();

        if (CenterTile)
            CenterTile->SetOutLineColor(1.f, 1.f, 0.f, 1.f);
    }
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

    if (mTemplate.HasDirectionalGap)
    {
        const int OpenTileIndex = FindPreviewOpenTileIndex(
            TileMap, CenterIndex, OutIndices);

        if (OpenTileIndex >= 0)
        {
            auto OpenIt = std::find(
                OutIndices.begin(), OutIndices.end(), OpenTileIndex);

            if (OpenIt != OutIndices.end())
                OutIndices.erase(OpenIt);
        }
    }

    return !OutIndices.empty();
}

int CPlacementAreaObject::FindPreviewOpenTileIndex(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex,
    const std::vector<int>& CandidateIndices) const
{
    if (!TileMap || CandidateIndices.empty())
        return -1;

    // 0: 아래, 1: 오른쪽, 2: 위, 3: 왼쪽
    static const FPlacementMarkerAnchor GOpenOffsets[4] =
    {
        { 0.f, 1.f },
        { 1.f, 0.f },
        { 0.f, -1.f },
        { -1.f, 0.f }
    };

    const int Dir = (mPreviewDirection % 4 + 4) % 4;
    const FPlacementMarkerAnchor& OpenOffset = GOpenOffsets[Dir];

    return FindMarkerTileIndexByLogicalOffset(
        TileMap,
        CenterIndex,
        CandidateIndices,
        OpenOffset.LogicalOffsetX,
        OpenOffset.LogicalOffsetY);
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
    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        if (mPrimaryPlacedIndices[i] == Index)
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
    UpdatePrimaryOverlayTiles(mPrimaryPlacedIndices);

    mMarkerTileIndices.clear();

    if (mPlacedCenterIndex < 0 ||
        mPrimaryPlacedIndices.empty())
    {
        UpdateMarkerOverlayTiles(std::vector<int>());
        return;
    }

    // 중앙 타일을 노란색 마커(O)로 고정한다.
    if (IsPlacedIndex(mPlacedCenterIndex))
        mMarkerTileIndices.push_back(mPlacedCenterIndex);

    if (mMarkerTileIndices.empty() &&
        !mPrimaryPlacedIndices.empty())
    {
        const int FallbackEdgeIndex = FindMarkerTileIndexByLogicalOffset(
            TileMap, mPlacedCenterIndex, mPrimaryPlacedIndices, 0.f, 0.f);

        if (FallbackEdgeIndex >= 0)
            mMarkerTileIndices.push_back(FallbackEdgeIndex);
    }

    UpdateMarkerOverlayTiles(mMarkerTileIndices);
}

void CPlacementAreaObject::RestoreTileColor(
    const std::shared_ptr<class CTileMapComponent>& TileMap, int Index)
{
    auto Tile = TileMap->GetTile(Index).lock();

    if (!Tile)
        return;

    if (HasOverlayRef(GPrimaryOverlayState, Index) ||
        HasOverlayRef(GMarkerOverlayState, Index))
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
