#include "PlacementAreaObject.h"
#include "../ObjectNames.h"
#include "Object/TileMapObject.h"
#include "World/World.h"

namespace
{
    const FVector4 GRoadOverlayColor(0.40f, 0.40f, 0.40f, 1.f);

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

bool CPlacementAreaObject::AcquireTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap)
{
    auto World = mWorld.lock();

    if (!World)
        return false;

    if (mTileMapObject.expired())
    {
        mTileMapObject = World->FindObject<CTileMapObject>(GTileMapObjectName);
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
            World->FindObject<CTileMapObject>(GTileMapFloorBlueName);
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
            World->FindObject<CTileMapObject>(GTileMapFloorYellowName);
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
        IsRoad() ? GRoadOverlayColor : FVector4::Blue);
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

void CPlacementAreaObject::ApplyPlacedAreaColor(
    const std::shared_ptr<class CTileMapComponent>& TileMap)
{
    SetAreaColor(
        TileMap,
        mPrimaryPlacedIndices,
        IsRoad() ? GRoadOverlayColor : FVector4::White);
    UpdatePrimaryOverlayTiles(mPrimaryPlacedIndices);

    mMarkerTileIndices.clear();

    if (IsRoad())
    {
        UpdateMarkerOverlayTiles(std::vector<int>());
        return;
    }

    if (mPlacedCenterIndex < 0 ||
        mPrimaryPlacedIndices.empty())
    {
        UpdateMarkerOverlayTiles(std::vector<int>());
        return;
    }

    auto AddMarkerUnique = [&](int MarkerIndex)
    {
        if (MarkerIndex < 0)
            return;

        if (!IsPlacedIndex(MarkerIndex))
            return;

        if (std::find(mMarkerTileIndices.begin(),
            mMarkerTileIndices.end(),
            MarkerIndex) != mMarkerTileIndices.end())
        {
            return;
        }

        mMarkerTileIndices.push_back(MarkerIndex);
    };

    const int Rotation = (mPreviewDirection % 4 + 4) % 4;

    for (size_t i = 0; i < mTemplate.MarkerAnchors.size(); ++i)
    {
        float OffsetX = mTemplate.MarkerAnchors[i].LogicalOffsetX;
        float OffsetY = mTemplate.MarkerAnchors[i].LogicalOffsetY;

        for (int r = 0; r < Rotation; ++r)
        {
            const float PrevX = OffsetX;
            OffsetX = OffsetY;
            OffsetY = -PrevX;
        }

        int MarkerIndex = -1;

        if (fabs(OffsetX) <= 0.001f &&
            fabs(OffsetY) <= 0.001f &&
            IsPlacedIndex(mPlacedCenterIndex))
        {
            MarkerIndex = mPlacedCenterIndex;
        }
        else
        {
            MarkerIndex = FindMarkerTileIndexByLogicalOffset(
                TileMap,
                mPlacedCenterIndex,
                mPrimaryPlacedIndices,
                OffsetX,
                OffsetY);
        }

        AddMarkerUnique(MarkerIndex);
    }

    if (mMarkerTileIndices.empty() &&
        !mPrimaryPlacedIndices.empty())
    {
        if (IsPlacedIndex(mPlacedCenterIndex))
            mMarkerTileIndices.push_back(mPlacedCenterIndex);
    }

    if (mMarkerTileIndices.empty() &&
        !mPrimaryPlacedIndices.empty())
    {
        const int FallbackEdgeIndex = FindMarkerTileIndexByLogicalOffset(
            TileMap, mPlacedCenterIndex, mPrimaryPlacedIndices, 0.f, 0.f);

        AddMarkerUnique(FallbackEdgeIndex);
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
    {
        Tile->SetOutLineColor(FVector4::Blue);
    }
    else
    {
        Tile->SetOutLineColor(FVector4::White);
    }
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
