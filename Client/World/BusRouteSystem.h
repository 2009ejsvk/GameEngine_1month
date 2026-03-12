#pragma once

#include <cfloat>
#include <memory>
#include <string>
#include <vector>

class CWorld;
class CRoadNetwork;

struct FBusStop
{
    int StopIndex = -1;
    std::string BuildingName;
    int RoadNodeIndex = -1;
    FVector3 WorldPos = FVector3::Zero;
};

struct FBusRoute
{
    std::string DepotBuildingName;
    std::vector<int> StopIndices;
    std::vector<float> SegmentDistances;
    float DispatchIntervalSeconds = 0.f;
    int Capacity = 0;
    float LoopDistance = 0.f;
};

struct FBusCommuteEstimate
{
    int RouteIndex = -1;
    float WalkDistance = 0.f;
    float RideDistance = 0.f;
    float WaitSeconds = 0.f;
    float ServiceQuality = 0.f;
    float CrowdingPenalty = 0.f;
};

class CBusRouteSystem
{
public:
    void Rebuild(
        const std::shared_ptr<CWorld>& World,
        const CRoadNetwork* RoadNetwork);

    bool HasRoutes() const
    {
        return !mRoutes.empty();
    }

    int FindNearestStop(
        const FVector3& Pos,
        float MaxWalkDistance = FLT_MAX) const;

    bool TryEstimateCommute(
        const FVector3& StartPos,
        const FVector3& EndPos,
        float MaxWalkDistance,
        float& OutWalkDistance,
        float& OutRideDistance,
        float& OutWaitSeconds) const;

    bool TryEstimateCommuteDetailed(
        const FVector3& StartPos,
        const FVector3& EndPos,
        float MaxWalkDistance,
        FBusCommuteEstimate& OutEstimate) const;

    const std::vector<FBusStop>& GetStops() const
    {
        return mStops;
    }

    const std::vector<FBusRoute>& GetRoutes() const
    {
        return mRoutes;
    }

private:
    void Clear();
    float ComputeRouteRideDistance(
        const FBusRoute& Route,
        int BoardOrderIndex,
        int AlightOrderIndex) const;
    float EstimateRouteServiceQuality(
        const FBusRoute& Route,
        float& OutCrowdingPenalty) const;

private:
    std::vector<FBusStop> mStops;
    std::vector<FBusRoute> mRoutes;
};
