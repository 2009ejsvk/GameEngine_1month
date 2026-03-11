#include "PlacementAreaObject.h"
#include "Object/TileMapObject.h"
#include "World/World.h"
#include <cfloat>

bool CPlacementAreaObject::IsNavigationObstacle() const
{
    return !IsRoad();
}

void CPlacementAreaObject::GetNavigationGoalTiles(
    std::vector<int>& OutIndices)
{
    EnsurePlacementObject();

    if (IsRoad())
        return;

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

    if (IsRoad())
        return;

    if (!mTileMapPrepared ||
        mPrimaryPlacedIndices.empty())
    {
        return;
    }

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
            continue;

        OutIndices.push_back(Index);
    }
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

bool CPlacementAreaObject::GetPlacedCenterGridCoords(
    int& OutGridX, int& OutGridY) const
{
    OutGridX = 0;
    OutGridY = 0;

    if (!mTileMapPrepared || mPlacedCenterIndex < 0)
        return false;

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!const_cast<CPlacementAreaObject*>(this)->AcquireTileMap(TileMap))
        return false;

    auto CenterTile = TileMap->GetTile(mPlacedCenterIndex).lock();

    if (!CenterTile)
        return false;

    const int IndexX = CenterTile->GetIndexX();
    const int IndexY = CenterTile->GetIndexY();
    OutGridX = IndexX + ((IndexY + (IndexY & 1)) / 2);
    OutGridY = IndexX - (IndexY / 2);
    return true;
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

    static const FPlacementMarkerAnchor GOpenOffsets[4] =
    {
        {  0.5f,  0.5f },
        {  0.5f, -0.5f },
        { -0.5f, -0.5f },
        { -0.5f,  0.5f },
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

        if (TileMap->IsRoadTile(Indices[i]) &&
            !IsPlacedIndex(Indices[i]))
        {
            return false;
        }
    }

    return true;
}

void CPlacementAreaObject::RefreshAccessibilityScore()
{
    mAccessibilityScore = 0.f;

    if (IsRoad() ||
        !mTileMapPrepared ||
        mPrimaryPlacedIndices.empty())
    {
        return;
    }

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    int PerimeterTileCount = 0;
    int RoadConnectedTileCount = 0;

    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        const int TileIndex = mPrimaryPlacedIndices[i];
        bool IsPerimeterTile = false;
        bool HasAdjacentRoad = false;

        for (int Dir = 0; Dir < 8; ++Dir)
        {
            const int NeighborIndex =
                GetIsoNeighborIndexByDir(TileMap, TileIndex, Dir);

            if (NeighborIndex < 0 || !IsPlacedIndex(NeighborIndex))
                IsPerimeterTile = true;

            if (NeighborIndex >= 0 && TileMap->IsRoadTile(NeighborIndex))
                HasAdjacentRoad = true;
        }

        if (!IsPerimeterTile)
            continue;

        ++PerimeterTileCount;

        if (HasAdjacentRoad)
            ++RoadConnectedTileCount;
    }

    if (PerimeterTileCount <= 0)
        return;

    mAccessibilityScore =
        static_cast<float>(RoadConnectedTileCount) /
        static_cast<float>(PerimeterTileCount);
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
