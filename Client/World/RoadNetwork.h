#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

class CTileMapComponent;

struct FRoadNetworkEdge
{
    int ToNodeIndex = -1;
    float Length = 0.f;
};

struct FRoadNetworkNode
{
    int TileIndex = -1;
    int GridX = 0;
    int GridY = 0;
    FVector3 WorldPos = FVector3::Zero;
    std::vector<FRoadNetworkEdge> Edges;
};

class CRoadNetwork
{
public:
    void Rebuild(const std::shared_ptr<CTileMapComponent>& TileMap);

    float QueryRoadDistance(
        const FVector2& StartPos,
        const FVector2& EndPos) const;
    float QueryRoadDistance(
        const FVector3& StartPos,
        const FVector3& EndPos) const;

    int FindNearestRoadNode(const FVector2& Pos) const;
    int FindNearestRoadNode(const FVector3& Pos) const;

    const std::vector<FRoadNetworkNode>& GetNodes() const
    {
        return mNodes;
    }

    bool IsValidNodeIndex(int NodeIndex) const
    {
        return NodeIndex >= 0 &&
            NodeIndex < static_cast<int>(mNodes.size());
    }

private:
    void Clear();

private:
    std::weak_ptr<CTileMapComponent> mTileMap;
    std::vector<FRoadNetworkNode> mNodes;
    std::unordered_map<int, int> mNodeIndexByTile;
    int mTileCountX = 0;
    int mTileCountY = 0;
};
