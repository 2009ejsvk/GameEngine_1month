#include "CitizenCommuteCalc.h"
#include "../GameConstants.h"
#include "../Map/PlacementAreaObject.h"
#include "../World/BusRouteSystem.h"
#include "../World/RoadNetwork.h"
#include <algorithm>

namespace
{
    float Clamp01(float Value)
    {
        return (std::max)(0.f, (std::min)(1.f, Value));
    }

    float ResolveApproxTileWorldSpan(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding)
    {
        FVector2 TileSize = FVector2(160.f, 80.f);

        if (HomeBuilding && HomeBuilding->GetTileSize(TileSize))
            return (std::max)(1.f, (TileSize.x + TileSize.y) * 0.5f);

        if (WorkBuilding && WorkBuilding->GetTileSize(TileSize))
            return (std::max)(1.f, (TileSize.x + TileSize.y) * 0.5f);

        return 120.f;
    }

    bool TryResolveCommuteAnchor(
        const std::shared_ptr<CPlacementAreaObject>& FromBuilding,
        const std::shared_ptr<CPlacementAreaObject>& ToBuilding,
        FVector3& OutAnchor)
    {
        OutAnchor = FVector3::Zero;

        if (!FromBuilding)
            return false;

        const FVector3 ToPos =
            ToBuilding ? ToBuilding->GetWorldPos() : FromBuilding->GetWorldPos();

        if (FromBuilding->GetClosestMarkerWorldPos(ToPos, OutAnchor))
            return true;

        if (FromBuilding->GetMarkerWorldPos(OutAnchor))
            return true;

        OutAnchor = FromBuilding->GetWorldPos();
        return true;
    }

    float ResolveDirectDistanceWorld(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding)
    {
        if (!HomeBuilding || !WorkBuilding)
            return 0.f;

        FVector3 HomeAnchor = FVector3::Zero;
        FVector3 WorkAnchor = FVector3::Zero;

        if (!TryResolveCommuteAnchor(HomeBuilding, WorkBuilding, HomeAnchor) ||
            !TryResolveCommuteAnchor(WorkBuilding, HomeBuilding, WorkAnchor))
        {
            return 0.f;
        }

        HomeAnchor.z = 0.f;
        WorkAnchor.z = 0.f;
        return HomeAnchor.Distance(WorkAnchor);
    }

    bool HasRoadAccess(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        return Building &&
            Clamp01(Building->GetAccessibilityScore()) >=
                GameConstants::Citizen::CommuteRoadAccessThreshold;
    }

    float ResolveRoadConnectorDistanceWorld(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        float TileWorldSpan)
    {
        if (!Building)
            return 0.f;

        const float Access = Clamp01(Building->GetAccessibilityScore());
        const float ConnectorTiles =
            GameConstants::Citizen::CommuteRoadConnectorTiles *
            ((std::max)(0.5f, 1.25f - 0.75f * Access));
        return ConnectorTiles * TileWorldSpan;
    }

    float EstimateWalkingCommuteTime(
        float DirectDistanceWorld)
    {
        const float WalkingSpeed =
            (std::max)(1.f, GameConstants::Citizen::NpcBaseMoveSpeed);
        const float RouteDistance =
            DirectDistanceWorld *
            GameConstants::Citizen::CommuteWalkingRouteFactor;
        return RouteDistance / WalkingSpeed;
    }

    float EstimateTransitCommuteTime(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding,
        float DirectDistanceWorld,
        const CBusRouteSystem* BusRouteSystem)
    {
        (void)DirectDistanceWorld;

        if (!BusRouteSystem)
            return -1.f;

        FVector3 HomeAnchor = FVector3::Zero;
        FVector3 WorkAnchor = FVector3::Zero;

        if (!TryResolveCommuteAnchor(HomeBuilding, WorkBuilding, HomeAnchor) ||
            !TryResolveCommuteAnchor(WorkBuilding, HomeBuilding, WorkAnchor))
        {
            return -1.f;
        }

        const float TileWorldSpan =
            ResolveApproxTileWorldSpan(HomeBuilding, WorkBuilding);
        const float WalkingSpeed =
            (std::max)(1.f, GameConstants::Citizen::NpcBaseMoveSpeed);
        const float TransitSpeed =
            (std::max)(
                1.f,
                GameConstants::Citizen::NpcBaseMoveSpeed *
                    GameConstants::Citizen::CommuteTransitSpeedMultiplier);
        const float MaxWalkDistance =
            TileWorldSpan * GameConstants::Citizen::CommuteBusStopSearchTiles;
        float WalkDistance = 0.f;
        float RideDistance = 0.f;
        float WaitSeconds = 0.f;

        if (!BusRouteSystem->TryEstimateCommute(
                HomeAnchor,
                WorkAnchor,
                MaxWalkDistance,
                WalkDistance,
                RideDistance,
                WaitSeconds))
        {
            return -1.f;
        }

        return WaitSeconds +
            WalkDistance / WalkingSpeed +
            RideDistance / TransitSpeed;
    }

    float EstimateVehicleCommuteTime(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding,
        float DirectDistanceWorld,
        const CRoadNetwork* RoadNetwork)
    {
        const float TileWorldSpan =
            ResolveApproxTileWorldSpan(HomeBuilding, WorkBuilding);
        const float WalkingSpeed =
            (std::max)(1.f, GameConstants::Citizen::NpcBaseMoveSpeed);
        const float VehicleSpeed =
            (std::max)(
                1.f,
                GameConstants::Citizen::NpcBaseMoveSpeed *
                    GameConstants::Citizen::CommuteVehicleSpeedMultiplier);
        const float ConnectorDistance =
            ResolveRoadConnectorDistanceWorld(HomeBuilding, TileWorldSpan) +
            ResolveRoadConnectorDistanceWorld(WorkBuilding, TileWorldSpan);
        float RoadDistance = -1.f;

        if (RoadNetwork)
        {
            FVector3 HomeAnchor = FVector3::Zero;
            FVector3 WorkAnchor = FVector3::Zero;

            if (TryResolveCommuteAnchor(HomeBuilding, WorkBuilding, HomeAnchor) &&
                TryResolveCommuteAnchor(WorkBuilding, HomeBuilding, WorkAnchor))
            {
                RoadDistance =
                    RoadNetwork->QueryRoadDistance(HomeAnchor, WorkAnchor);
            }
        }

        if (RoadDistance <= 0.f)
        {
            RoadDistance =
                DirectDistanceWorld *
                GameConstants::Citizen::CommuteRoadRouteFactor;
        }

        return ConnectorDistance / WalkingSpeed +
            RoadDistance / VehicleSpeed;
    }
}

namespace CitizenCommuteCalc
{
    float EstimateCommuteTime(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding,
        const FCitizenIdentityProfile& IdentityProfile,
        const CRoadNetwork* RoadNetwork,
        const CBusRouteSystem* BusRouteSystem)
    {
        const float DirectDistanceWorld =
            ResolveDirectDistanceWorld(HomeBuilding, WorkBuilding);

        if (DirectDistanceWorld <= 1.f)
            return 0.f;

        switch (IdentityProfile.WealthLevel)
        {
        case ECitizenWealthLevel::Rich:
            if (HasRoadAccess(HomeBuilding) && HasRoadAccess(WorkBuilding))
            {
                return EstimateVehicleCommuteTime(
                    HomeBuilding,
                    WorkBuilding,
                    DirectDistanceWorld,
                    RoadNetwork);
            }

            if (BusRouteSystem)
            {
                const float TransitTime = EstimateTransitCommuteTime(
                    HomeBuilding,
                    WorkBuilding,
                    DirectDistanceWorld,
                    BusRouteSystem);

                if (TransitTime > 0.f)
                    return TransitTime;
            }
            break;

        case ECitizenWealthLevel::WellOff:
            if (BusRouteSystem)
            {
                const float TransitTime = EstimateTransitCommuteTime(
                    HomeBuilding,
                    WorkBuilding,
                    DirectDistanceWorld,
                    BusRouteSystem);

                if (TransitTime > 0.f)
                    return TransitTime;
            }
            break;

        default:
            break;
        }

        return EstimateWalkingCommuteTime(DirectDistanceWorld);
    }

    float EstimateCommutePenalty(float CommuteTimeSeconds)
    {
        const float GraceSeconds =
            GameConstants::Citizen::CommutePenaltyGraceSeconds;
        const float MaxSeconds =
            GameConstants::Citizen::CommutePenaltyMaxSeconds;

        if (CommuteTimeSeconds <= GraceSeconds)
            return 0.f;

        if (MaxSeconds <= GraceSeconds)
            return 100.f;

        const float Ratio =
            (CommuteTimeSeconds - GraceSeconds) /
            (MaxSeconds - GraceSeconds);
        return Clamp01(Ratio) * 100.f;
    }
}
