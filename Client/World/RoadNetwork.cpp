#include "RoadNetwork.h"
#include "Component/TileMapComponent.h"
#include "Object/GameObject.h"
#include <algorithm>
#include <cfloat>
#include <limits>
#include <unordered_set>

namespace
{
    constexpr int GRoadDirCount = 4;
    constexpr int GRoadDirX[GRoadDirCount] = { 0, 1, 0, -1 };
    constexpr int GRoadDirY[GRoadDirCount] = { 1, 0, -1, 0 };

    long long PackStepKey(int FromIndex, int ToIndex)
    {
        return (static_cast<long long>(FromIndex) << 32) |
            static_cast<unsigned int>(ToIndex);
    }

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

        const int IndexX = Tile->GetIndexX();
        const int IndexY = Tile->GetIndexY();
        OutGridX = IndexX + ((IndexY + (IndexY & 1)) / 2);
        OutGridY = IndexX - (IndexY / 2);
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

    int StepRoadTileIndex(
        const std::shared_ptr<CTileMapComponent>& TileMap,
        int FromIndex,
        int DirIndex)
    {
        if (!TileMap ||
            DirIndex < 0 ||
            DirIndex >= GRoadDirCount)
        {
            return -1;
        }

        int GridX = 0;
        int GridY = 0;

        if (!TryGetTileGridCoords(TileMap, FromIndex, GridX, GridY))
            return -1;

        const int NextIndex = GetTileIndexByGrid(
            TileMap,
            GridX + GRoadDirX[DirIndex],
            GridY + GRoadDirY[DirIndex]);

        if (NextIndex < 0 || !TileMap->IsRoadTile(NextIndex))
            return -1;

        return NextIndex;
    }

    FVector3 GetTileWorldCenter(
        const std::shared_ptr<CTileMapComponent>& TileMap,
        int TileIndex)
    {
        FVector3 Center = FVector3::Zero;

        if (!TileMap)
            return Center;

        auto Tile = TileMap->GetTile(TileIndex).lock();

        if (!Tile)
            return Center;

        const FVector2 TileCenter = Tile->GetCenter();
        Center.x = TileCenter.x;
        Center.y = TileCenter.y;

        auto Owner = TileMap->GetOwner().lock();

        if (Owner)
        {
            const FVector3 OwnerPos = Owner->GetWorldPos();
            Center.x += OwnerPos.x;
            Center.y += OwnerPos.y;
            Center.z = OwnerPos.z;
        }

        return Center;
    }

    float GetTileStepDistance(
        const std::shared_ptr<CTileMapComponent>& TileMap,
        int FromIndex,
        int ToIndex)
    {
        return GetTileWorldCenter(TileMap, FromIndex).Distance(
            GetTileWorldCenter(TileMap, ToIndex));
    }

    void CollectRoadNeighbors(
        const std::shared_ptr<CTileMapComponent>& TileMap,
        int TileIndex,
        std::vector<int>& OutNeighborIndices,
        std::vector<int>* OutDirIndices = nullptr)
    {
        OutNeighborIndices.clear();

        if (OutDirIndices)
            OutDirIndices->clear();

        if (!TileMap || !TileMap->IsRoadTile(TileIndex))
            return;

        for (int DirIndex = 0; DirIndex < GRoadDirCount; ++DirIndex)
        {
            const int NextIndex =
                StepRoadTileIndex(TileMap, TileIndex, DirIndex);

            if (NextIndex < 0)
                continue;

            OutNeighborIndices.push_back(NextIndex);

            if (OutDirIndices)
                OutDirIndices->push_back(DirIndex);
        }
    }

    bool AreOppositeRoadDirs(int DirA, int DirB)
    {
        return ((DirA + 2) % GRoadDirCount) == DirB;
    }

    bool IsRoadGraphNodeTile(
        const std::shared_ptr<CTileMapComponent>& TileMap,
        int TileIndex)
    {
        if (!TileMap || !TileMap->IsRoadTile(TileIndex))
            return false;

        std::vector<int> NeighborIndices;
        std::vector<int> DirIndices;
        CollectRoadNeighbors(
            TileMap,
            TileIndex,
            NeighborIndices,
            &DirIndices);

        if (NeighborIndices.size() != 2)
            return true;

        return !AreOppositeRoadDirs(DirIndices[0], DirIndices[1]);
    }

    void AddUniqueEdge(
        std::vector<FRoadNetworkEdge>& Edges,
        int ToNodeIndex,
        float Length)
    {
        if (ToNodeIndex < 0 || Length <= 0.f)
            return;

        for (size_t i = 0; i < Edges.size(); ++i)
        {
            if (Edges[i].ToNodeIndex != ToNodeIndex)
                continue;

            if (Length < Edges[i].Length)
                Edges[i].Length = Length;

            return;
        }

        FRoadNetworkEdge Edge;
        Edge.ToNodeIndex = ToNodeIndex;
        Edge.Length = Length;
        Edges.push_back(Edge);
    }
}

void CRoadNetwork::Clear()
{
    mTileMap.reset();
    mNodes.clear();
    mNodeIndexByTile.clear();
    mTileCountX = 0;
    mTileCountY = 0;
}

void CRoadNetwork::Rebuild(
    const std::shared_ptr<CTileMapComponent>& TileMap)
{
    Clear();
    mTileMap = TileMap;

    if (!TileMap)
        return;

    mTileCountX = TileMap->GetTileCountX();
    mTileCountY = TileMap->GetTileCountY();
    const int TileCount = mTileCountX * mTileCountY;

    if (TileCount <= 0)
        return;

    std::vector<int> NodeTileIndices;
    NodeTileIndices.reserve((std::max)(4, TileCount / 16));

    for (int TileIndex = 0; TileIndex < TileCount; ++TileIndex)
    {
        if (!TileMap->IsRoadTile(TileIndex))
            continue;

        if (IsRoadGraphNodeTile(TileMap, TileIndex))
            NodeTileIndices.push_back(TileIndex);
    }

    if (NodeTileIndices.empty())
        return;

    mNodes.reserve(NodeTileIndices.size());

    for (size_t i = 0; i < NodeTileIndices.size(); ++i)
    {
        FRoadNetworkNode Node;
        Node.TileIndex = NodeTileIndices[i];
        TryGetTileGridCoords(
            TileMap,
            Node.TileIndex,
            Node.GridX,
            Node.GridY);
        Node.WorldPos = GetTileWorldCenter(TileMap, Node.TileIndex);
        mNodeIndexByTile.emplace(
            Node.TileIndex,
            static_cast<int>(mNodes.size()));
        mNodes.push_back(Node);
    }

    std::unordered_set<long long> ProcessedSteps;

    for (size_t NodeIndex = 0; NodeIndex < mNodes.size(); ++NodeIndex)
    {
        const int StartTileIndex = mNodes[NodeIndex].TileIndex;
        std::vector<int> NeighborIndices;
        CollectRoadNeighbors(
            TileMap,
            StartTileIndex,
            NeighborIndices);

        for (size_t NeighborIdx = 0;
             NeighborIdx < NeighborIndices.size();
             ++NeighborIdx)
        {
            const int FirstStepTile = NeighborIndices[NeighborIdx];
            const long long FirstStepKey =
                PackStepKey(StartTileIndex, FirstStepTile);

            if (ProcessedSteps.find(FirstStepKey) != ProcessedSteps.end())
                continue;

            ProcessedSteps.insert(FirstStepKey);
            ProcessedSteps.insert(PackStepKey(FirstStepTile, StartTileIndex));

            float Length = GetTileStepDistance(
                TileMap,
                StartTileIndex,
                FirstStepTile);
            int PrevTileIndex = StartTileIndex;
            int CurrentTileIndex = FirstStepTile;

            while (true)
            {
                auto TargetNodeIt =
                    mNodeIndexByTile.find(CurrentTileIndex);

                if (TargetNodeIt != mNodeIndexByTile.end())
                {
                    const int TargetNodeIndex = TargetNodeIt->second;

                    if (TargetNodeIndex != static_cast<int>(NodeIndex))
                    {
                        AddUniqueEdge(
                            mNodes[NodeIndex].Edges,
                            TargetNodeIndex,
                            Length);
                        AddUniqueEdge(
                            mNodes[TargetNodeIndex].Edges,
                            static_cast<int>(NodeIndex),
                            Length);
                    }

                    break;
                }

                std::vector<int> NextNeighborIndices;
                CollectRoadNeighbors(
                    TileMap,
                    CurrentTileIndex,
                    NextNeighborIndices);

                int NextTileIndex = -1;

                for (size_t i = 0; i < NextNeighborIndices.size(); ++i)
                {
                    if (NextNeighborIndices[i] == PrevTileIndex)
                        continue;

                    NextTileIndex = NextNeighborIndices[i];
                    break;
                }

                if (NextTileIndex < 0)
                    break;

                const long long NextStepKey =
                    PackStepKey(CurrentTileIndex, NextTileIndex);

                if (ProcessedSteps.find(NextStepKey) != ProcessedSteps.end())
                    break;

                ProcessedSteps.insert(NextStepKey);
                ProcessedSteps.insert(
                    PackStepKey(NextTileIndex, CurrentTileIndex));
                Length += GetTileStepDistance(
                    TileMap,
                    CurrentTileIndex,
                    NextTileIndex);
                PrevTileIndex = CurrentTileIndex;
                CurrentTileIndex = NextTileIndex;
            }
        }
    }
}

int CRoadNetwork::FindNearestRoadNode(const FVector2& Pos) const
{
    return FindNearestRoadNode(FVector3(Pos.x, Pos.y, 0.f));
}

int CRoadNetwork::FindNearestRoadNode(const FVector3& Pos) const
{
    if (mNodes.empty())
        return -1;

    const float TargetX = Pos.x;
    const float TargetY = Pos.y;
    float BestDistSq = FLT_MAX;
    int BestNodeIndex = -1;

    for (size_t i = 0; i < mNodes.size(); ++i)
    {
        const float dx = mNodes[i].WorldPos.x - TargetX;
        const float dy = mNodes[i].WorldPos.y - TargetY;
        const float DistSq = dx * dx + dy * dy;

        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            BestNodeIndex = static_cast<int>(i);
        }
    }

    return BestNodeIndex;
}

float CRoadNetwork::QueryRoadDistance(
    const FVector2& StartPos,
    const FVector2& EndPos) const
{
    return QueryRoadDistance(
        FVector3(StartPos.x, StartPos.y, 0.f),
        FVector3(EndPos.x, EndPos.y, 0.f));
}

float CRoadNetwork::QueryRoadDistance(
    const FVector3& StartPos,
    const FVector3& EndPos) const
{
    if (mNodes.empty())
        return -1.f;

    const int StartNodeIndex = FindNearestRoadNode(StartPos);
    const int EndNodeIndex = FindNearestRoadNode(EndPos);

    if (!IsValidNodeIndex(StartNodeIndex) ||
        !IsValidNodeIndex(EndNodeIndex))
    {
        return -1.f;
    }

    const FVector3 StartAnchor(
        StartPos.x,
        StartPos.y,
        mNodes[StartNodeIndex].WorldPos.z);
    const FVector3 EndAnchor(
        EndPos.x,
        EndPos.y,
        mNodes[EndNodeIndex].WorldPos.z);
    const float StartConnectorDistance =
        StartAnchor.Distance(mNodes[StartNodeIndex].WorldPos);
    const float EndConnectorDistance =
        EndAnchor.Distance(mNodes[EndNodeIndex].WorldPos);

    if (StartNodeIndex == EndNodeIndex)
        return StartConnectorDistance + EndConnectorDistance;

    const float Infinity = (std::numeric_limits<float>::max)();
    std::vector<float> Distances(mNodes.size(), Infinity);
    std::vector<unsigned char> Closed(mNodes.size(), 0);
    Distances[StartNodeIndex] = 0.f;

    while (true)
    {
        int CurrentNodeIndex = -1;
        float BestDistance = Infinity;

        for (size_t i = 0; i < Distances.size(); ++i)
        {
            if (Closed[i])
                continue;

            if (Distances[i] < BestDistance)
            {
                BestDistance = Distances[i];
                CurrentNodeIndex = static_cast<int>(i);
            }
        }

        if (CurrentNodeIndex < 0)
            break;

        if (CurrentNodeIndex == EndNodeIndex)
            break;

        Closed[CurrentNodeIndex] = 1;

        const std::vector<FRoadNetworkEdge>& Edges =
            mNodes[CurrentNodeIndex].Edges;

        for (size_t EdgeIndex = 0; EdgeIndex < Edges.size(); ++EdgeIndex)
        {
            const FRoadNetworkEdge& Edge = Edges[EdgeIndex];

            if (!IsValidNodeIndex(Edge.ToNodeIndex))
                continue;

            const float CandidateDistance =
                Distances[CurrentNodeIndex] + Edge.Length;

            if (CandidateDistance < Distances[Edge.ToNodeIndex])
                Distances[Edge.ToNodeIndex] = CandidateDistance;
        }
    }

    if (Distances[EndNodeIndex] >= Infinity)
        return -1.f;

    return StartConnectorDistance +
        Distances[EndNodeIndex] +
        EndConnectorDistance;
}
