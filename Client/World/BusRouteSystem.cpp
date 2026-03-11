#include "BusRouteSystem.h"
#include "RoadNetwork.h"
#include "../GameConstants.h"
#include "../Map/PlacementAreaObject.h"
#include "World/World.h"
#include <algorithm>
#include <cfloat>

namespace
{
    bool TryResolveBuildingAnchor(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        FVector3& OutWorldPos)
    {
        OutWorldPos = FVector3::Zero;

        if (!Building)
            return false;

        if (Building->GetMarkerWorldPos(OutWorldPos))
            return true;

        OutWorldPos = Building->GetWorldPos();
        return true;
    }

    float ResolveTileWorldSpan(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        FVector2 TileSize = FVector2(160.f, 80.f);

        if (Building && Building->GetTileSize(TileSize))
            return (std::max)(1.f, (TileSize.x + TileSize.y) * 0.5f);

        return 120.f;
    }

    float ResolveRoadLinkLimitWorld(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        return ResolveTileWorldSpan(Building) *
            GameConstants::Citizen::CommuteBusStopRoadLinkTiles;
    }

    int FindRoadNodeIfNearby(
        const CRoadNetwork* RoadNetwork,
        const FVector3& WorldPos,
        float MaxDistance)
    {
        if (!RoadNetwork || MaxDistance <= 0.f)
            return -1;

        const int NodeIndex = RoadNetwork->FindNearestRoadNode(WorldPos);

        if (!RoadNetwork->IsValidNodeIndex(NodeIndex))
            return -1;

        const FVector3 NodePos =
            RoadNetwork->GetNodes()[static_cast<size_t>(NodeIndex)].WorldPos;

        return NodePos.Distance(WorldPos) <= MaxDistance ?
            NodeIndex :
            -1;
    }

    bool TryAddBusStop(
        std::vector<FBusStop>& Stops,
        const std::shared_ptr<CPlacementAreaObject>& Building,
        const CRoadNetwork* RoadNetwork)
    {
        if (!Building || !Building->IsBusStop())
            return false;

        FVector3 StopPos = FVector3::Zero;

        if (!TryResolveBuildingAnchor(Building, StopPos))
            return false;

        const int RoadNodeIndex = FindRoadNodeIfNearby(
            RoadNetwork,
            StopPos,
            ResolveRoadLinkLimitWorld(Building));

        if (RoadNodeIndex < 0)
            return false;

        FBusStop Stop;
        Stop.StopIndex = static_cast<int>(Stops.size());
        Stop.BuildingName = Building->GetName();
        Stop.RoadNodeIndex = RoadNodeIndex;
        Stop.WorldPos = StopPos;
        Stops.push_back(Stop);
        return true;
    }

    bool TryResolveDepotAnchor(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        FVector3& OutWorldPos)
    {
        return TryResolveBuildingAnchor(Building, OutWorldPos);
    }

    int FindNearestReachableStop(
        const CRoadNetwork* RoadNetwork,
        const FVector3& FromPos,
        const std::vector<FBusStop>& Stops,
        const std::vector<int>& CandidateStopIndices)
    {
        int BestStopIndex = -1;
        float BestDistance = FLT_MAX;

        for (size_t i = 0; i < CandidateStopIndices.size(); ++i)
        {
            const int StopIndex = CandidateStopIndices[i];

            if (StopIndex < 0 ||
                StopIndex >= static_cast<int>(Stops.size()))
            {
                continue;
            }

            const float Distance =
                RoadNetwork->QueryRoadDistance(
                    FromPos,
                    Stops[static_cast<size_t>(StopIndex)].WorldPos);

            if (Distance < 0.f)
                continue;

            if (Distance < BestDistance)
            {
                BestDistance = Distance;
                BestStopIndex = StopIndex;
            }
        }

        return BestStopIndex;
    }
}

void CBusRouteSystem::Clear()
{
    mStops.clear();
    mRoutes.clear();
}

void CBusRouteSystem::Rebuild(
    const std::shared_ptr<CWorld>& World,
    const CRoadNetwork* RoadNetwork)
{
    Clear();

    if (!World || !RoadNetwork)
        return;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return;

    std::vector<std::shared_ptr<CPlacementAreaObject>> BusGarages;

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        auto Building = BuildingList[i].lock();

        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable() ||
            !Building->HasPlacedArea())
        {
            continue;
        }

        if (Building->IsBusStop())
        {
            TryAddBusStop(mStops, Building, RoadNetwork);
            continue;
        }

        if (Building->IsBusGarage())
            BusGarages.push_back(Building);
    }

    if (mStops.size() < 2 || BusGarages.empty())
        return;

    for (size_t GarageIdx = 0; GarageIdx < BusGarages.size(); ++GarageIdx)
    {
        const auto& Garage = BusGarages[GarageIdx];
        FVector3 DepotPos = FVector3::Zero;

        if (!TryResolveDepotAnchor(Garage, DepotPos))
            continue;

        if (FindRoadNodeIfNearby(
                RoadNetwork,
                DepotPos,
                ResolveRoadLinkLimitWorld(Garage)) < 0)
        {
            continue;
        }

        std::vector<int> RemainingStopIndices;
        RemainingStopIndices.reserve(mStops.size());

        for (size_t StopIdx = 0; StopIdx < mStops.size(); ++StopIdx)
        {
            const float Distance = RoadNetwork->QueryRoadDistance(
                DepotPos,
                mStops[StopIdx].WorldPos);

            if (Distance >= 0.f)
                RemainingStopIndices.push_back(static_cast<int>(StopIdx));
        }

        if (RemainingStopIndices.size() < 2)
            continue;

        FBusRoute Route;
        Route.DepotBuildingName = Garage->GetName();
        Route.DispatchIntervalSeconds =
            GameConstants::Citizen::BusDispatchSeconds;
        Route.Capacity = GameConstants::Citizen::BusRouteCapacity;

        FVector3 CurrentPos = DepotPos;

        // 임시 자동 노선 생성:
        // 노선 편집 UI가 없으므로 차고지에서 접근 가능한 정류장을
        // 가장 가까운 순서로 연결해 하나의 순환 노선을 만든다.
        while (!RemainingStopIndices.empty())
        {
            const int NextStopIndex = FindNearestReachableStop(
                RoadNetwork,
                CurrentPos,
                mStops,
                RemainingStopIndices);

            if (NextStopIndex < 0)
                break;

            Route.StopIndices.push_back(NextStopIndex);
            CurrentPos = mStops[static_cast<size_t>(NextStopIndex)].WorldPos;
            RemainingStopIndices.erase(
                std::remove(
                    RemainingStopIndices.begin(),
                    RemainingStopIndices.end(),
                    NextStopIndex),
                RemainingStopIndices.end());
        }

        if (Route.StopIndices.size() < 2)
            continue;

        Route.SegmentDistances.reserve(Route.StopIndices.size());

        for (size_t StopOrder = 0;
             StopOrder < Route.StopIndices.size();
             ++StopOrder)
        {
            const int FromStopIndex = Route.StopIndices[StopOrder];
            const int ToStopIndex =
                Route.StopIndices[
                    (StopOrder + 1) % Route.StopIndices.size()];
            float SegmentDistance = RoadNetwork->QueryRoadDistance(
                mStops[static_cast<size_t>(FromStopIndex)].WorldPos,
                mStops[static_cast<size_t>(ToStopIndex)].WorldPos);

            if (SegmentDistance < 0.f)
            {
                SegmentDistance =
                    mStops[static_cast<size_t>(FromStopIndex)].WorldPos.Distance(
                        mStops[static_cast<size_t>(ToStopIndex)].WorldPos);
            }

            Route.SegmentDistances.push_back((std::max)(0.f, SegmentDistance));
            Route.LoopDistance += Route.SegmentDistances.back();
        }

        mRoutes.push_back(std::move(Route));
    }
}

int CBusRouteSystem::FindNearestStop(
    const FVector3& Pos,
    float MaxWalkDistance) const
{
    int BestStopIndex = -1;
    float BestDistance = FLT_MAX;

    for (size_t i = 0; i < mStops.size(); ++i)
    {
        const float Distance = mStops[i].WorldPos.Distance(Pos);

        if (Distance > MaxWalkDistance)
            continue;

        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            BestStopIndex = static_cast<int>(i);
        }
    }

    return BestStopIndex;
}

float CBusRouteSystem::ComputeRouteRideDistance(
    const FBusRoute& Route,
    int BoardOrderIndex,
    int AlightOrderIndex) const
{
    if (Route.StopIndices.empty() ||
        Route.SegmentDistances.size() != Route.StopIndices.size())
    {
        return -1.f;
    }

    if (BoardOrderIndex < 0 ||
        BoardOrderIndex >= static_cast<int>(Route.StopIndices.size()) ||
        AlightOrderIndex < 0 ||
        AlightOrderIndex >= static_cast<int>(Route.StopIndices.size()))
    {
        return -1.f;
    }

    if (BoardOrderIndex == AlightOrderIndex)
        return 0.f;

    float ForwardDistance = 0.f;
    int CurrentIndex = BoardOrderIndex;

    while (CurrentIndex != AlightOrderIndex)
    {
        ForwardDistance +=
            Route.SegmentDistances[static_cast<size_t>(CurrentIndex)];
        CurrentIndex =
            (CurrentIndex + 1) % static_cast<int>(Route.StopIndices.size());
    }

    if (Route.LoopDistance <= 0.f)
        return ForwardDistance;

    return (std::min)(ForwardDistance, Route.LoopDistance - ForwardDistance);
}

bool CBusRouteSystem::TryEstimateCommute(
    const FVector3& StartPos,
    const FVector3& EndPos,
    float MaxWalkDistance,
    float& OutWalkDistance,
    float& OutRideDistance,
    float& OutWaitSeconds) const
{
    OutWalkDistance = 0.f;
    OutRideDistance = 0.f;
    OutWaitSeconds = 0.f;

    bool FoundRoute = false;
    float BestScore = FLT_MAX;

    for (size_t RouteIdx = 0; RouteIdx < mRoutes.size(); ++RouteIdx)
    {
        const FBusRoute& Route = mRoutes[RouteIdx];

        if (Route.StopIndices.size() < 2)
            continue;

        for (size_t BoardOrder = 0;
             BoardOrder < Route.StopIndices.size();
             ++BoardOrder)
        {
            const int BoardStopIndex = Route.StopIndices[BoardOrder];
            const float StartWalkDistance =
                StartPos.Distance(
                    mStops[static_cast<size_t>(BoardStopIndex)].WorldPos);

            if (StartWalkDistance > MaxWalkDistance)
                continue;

            for (size_t AlightOrder = 0;
                 AlightOrder < Route.StopIndices.size();
                 ++AlightOrder)
            {
                const int AlightStopIndex = Route.StopIndices[AlightOrder];
                const float EndWalkDistance =
                    EndPos.Distance(
                        mStops[static_cast<size_t>(AlightStopIndex)].WorldPos);

                if (EndWalkDistance > MaxWalkDistance)
                    continue;

                const float RideDistance = ComputeRouteRideDistance(
                    Route,
                    static_cast<int>(BoardOrder),
                    static_cast<int>(AlightOrder));

                if (RideDistance <= 0.f)
                    continue;

                const float Score =
                    StartWalkDistance + EndWalkDistance + RideDistance;

                if (Score >= BestScore)
                    continue;

                BestScore = Score;
                OutWalkDistance = StartWalkDistance + EndWalkDistance;
                OutRideDistance = RideDistance;
                OutWaitSeconds =
                    Route.DispatchIntervalSeconds *
                    GameConstants::Citizen::CommuteBusAverageWaitFactor;
                FoundRoute = true;
            }
        }
    }

    return FoundRoute;
}
