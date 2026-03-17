#include "CitizenCommuteCalc.h"
#include "../GameConstants.h"
#include "../Map/PlacementAreaObject.h"
#include "../World/BusRouteSystem.h"
#include "../World/RoadNetwork.h"
#include <algorithm>

namespace
{
    bool IsAffluentCitizen(ECitizenWealthLevel WealthLevel)
    {
        return GetCitizenWealthRank(WealthLevel) >=
            GetCitizenWealthRank(ECitizenWealthLevel::Rich);
    }

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

    float ResolveCommutePenaltyNormalized(float CommuteTimeSeconds)
    {
        const float GraceSeconds =
            GameConstants::Citizen::CommutePenaltyGraceSeconds;
        const float MaxSeconds =
            GameConstants::Citizen::CommutePenaltyMaxSeconds;

        if (CommuteTimeSeconds <= GraceSeconds)
            return 0.f;

        if (MaxSeconds <= GraceSeconds)
            return 1.f;

        return Clamp01(
            (CommuteTimeSeconds - GraceSeconds) /
            (MaxSeconds - GraceSeconds));
    }

    float ResolveVehicleServiceQuality(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding)
    {
        const float HomeAccess =
            HomeBuilding ?
                Clamp01(HomeBuilding->GetAccessibilityScore()) :
                0.5f;
        const float WorkAccess =
            WorkBuilding ?
                Clamp01(WorkBuilding->GetAccessibilityScore()) :
                0.5f;
        return Clamp01((HomeAccess + WorkAccess) * 0.5f);
    }

    FCommuteProfile BuildCommuteProfile(
        ECommuteMode Mode,
        float DirectDistanceWorld,
        float TravelSeconds,
        float WalkDistance,
        float RideDistance,
        float WaitSeconds,
        float ServiceQuality,
        float CrowdingPenalty)
    {
        FCommuteProfile Profile;
        Profile.Mode = Mode;
        Profile.DirectDistanceWorld = DirectDistanceWorld;
        Profile.TravelSeconds = (std::max)(0.f, TravelSeconds);
        Profile.WalkDistance = (std::max)(0.f, WalkDistance);
        Profile.RideDistance = (std::max)(0.f, RideDistance);
        Profile.WaitSeconds = (std::max)(0.f, WaitSeconds);
        Profile.ServiceQuality = Clamp01(ServiceQuality);
        Profile.CrowdingPenalty = Clamp01(CrowdingPenalty);

        const float PenaltyRatio =
            ResolveCommutePenaltyNormalized(Profile.TravelSeconds);
        float Productivity =
            1.f - PenaltyRatio * 0.50f - Profile.CrowdingPenalty * 0.10f;
        float JobRecovery =
            1.f - PenaltyRatio * 0.42f - Profile.CrowdingPenalty * 0.08f;

        switch (Mode)
        {
        case ECommuteMode::Transit:
            Productivity += Profile.ServiceQuality * 0.10f;
            JobRecovery += Profile.ServiceQuality * 0.08f;
            break;
        case ECommuteMode::Vehicle:
            Productivity += 0.08f + Profile.ServiceQuality * 0.06f;
            JobRecovery += 0.06f + Profile.ServiceQuality * 0.05f;
            break;
        case ECommuteMode::Walk:
            Productivity -= PenaltyRatio * 0.08f;
            JobRecovery -= PenaltyRatio * 0.06f;
            break;
        default:
            break;
        }

        Profile.ProductivityMultiplier =
            Clamp<float>(Productivity, 0.45f, 1.12f);
        Profile.JobRecoveryMultiplier =
            Clamp<float>(JobRecovery, 0.50f, 1.10f);
        return Profile;
    }

    float ScoreCommuteChoice(
        const FCommuteProfile& Profile,
        ECitizenWealthLevel WealthLevel)
    {
        float Score =
            Profile.TravelSeconds -
            Profile.ProductivityMultiplier * 18.f -
            Profile.ServiceQuality * 6.f +
            Profile.CrowdingPenalty * 12.f;

        switch (WealthLevel)
        {
        case ECitizenWealthLevel::FilthyRich:
            if (Profile.Mode == ECommuteMode::Vehicle)
                Score -= 11.f;
            else if (Profile.Mode == ECommuteMode::Walk)
                Score += 18.f;
            break;
        case ECitizenWealthLevel::Rich:
            if (Profile.Mode == ECommuteMode::Vehicle)
                Score -= 8.f;
            else if (Profile.Mode == ECommuteMode::Walk)
                Score += 14.f;
            break;
        case ECitizenWealthLevel::WellOff:
            if (Profile.Mode == ECommuteMode::Transit)
                Score -= 4.f;
            else if (Profile.Mode == ECommuteMode::Walk)
                Score += 5.f;
            break;
        case ECitizenWealthLevel::Broke:
            if (Profile.Mode == ECommuteMode::Transit)
                Score -= 4.f;
            else if (Profile.Mode == ECommuteMode::Vehicle)
                Score += 6.f;
            break;
        case ECitizenWealthLevel::Poor:
        default:
            if (Profile.Mode == ECommuteMode::Transit)
                Score -= 3.f;
            break;
        }

        return Score;
    }

    bool TryBuildTransitCommuteProfile(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding,
        float DirectDistanceWorld,
        const CBusRouteSystem* BusRouteSystem,
        FCommuteProfile& OutProfile)
    {
        (void)DirectDistanceWorld;

        if (!BusRouteSystem)
            return false;

        FVector3 HomeAnchor = FVector3::Zero;
        FVector3 WorkAnchor = FVector3::Zero;

        if (!TryResolveCommuteAnchor(HomeBuilding, WorkBuilding, HomeAnchor) ||
            !TryResolveCommuteAnchor(WorkBuilding, HomeBuilding, WorkAnchor))
        {
            return false;
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
        FBusCommuteEstimate Estimate;

        if (!BusRouteSystem->TryEstimateCommuteDetailed(
                HomeAnchor,
                WorkAnchor,
                MaxWalkDistance,
                Estimate))
        {
            return false;
        }

        OutProfile = BuildCommuteProfile(
            ECommuteMode::Transit,
            DirectDistanceWorld,
            Estimate.WaitSeconds +
                Estimate.WalkDistance / WalkingSpeed +
                Estimate.RideDistance / TransitSpeed,
            Estimate.WalkDistance,
            Estimate.RideDistance,
            Estimate.WaitSeconds,
            Estimate.ServiceQuality,
            Estimate.CrowdingPenalty);
        return true;
    }

    bool TryBuildVehicleCommuteProfile(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding,
        float DirectDistanceWorld,
        const CRoadNetwork* RoadNetwork,
        FCommuteProfile& OutProfile)
    {
        if (!HasRoadAccess(HomeBuilding) || !HasRoadAccess(WorkBuilding))
            return false;

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

        OutProfile = BuildCommuteProfile(
            ECommuteMode::Vehicle,
            DirectDistanceWorld,
            ConnectorDistance / WalkingSpeed +
                RoadDistance / VehicleSpeed,
            ConnectorDistance,
            RoadDistance,
            0.f,
            ResolveVehicleServiceQuality(HomeBuilding, WorkBuilding),
            0.f);
        return true;
    }
}

namespace CitizenCommuteCalc
{
    FCommuteProfile ResolveCommuteProfile(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding,
        const FCitizenIdentityProfile& IdentityProfile,
        const CRoadNetwork* RoadNetwork,
        const CBusRouteSystem* BusRouteSystem)
    {
        const float DirectDistanceWorld =
            ResolveDirectDistanceWorld(HomeBuilding, WorkBuilding);

        if (DirectDistanceWorld <= 1.f)
            return FCommuteProfile();

        const float WalkDistance =
            DirectDistanceWorld *
            GameConstants::Citizen::CommuteWalkingRouteFactor;
        FCommuteProfile BestProfile = BuildCommuteProfile(
            ECommuteMode::Walk,
            DirectDistanceWorld,
            EstimateWalkingCommuteTime(DirectDistanceWorld),
            WalkDistance,
            0.f,
            0.f,
            0.30f,
            0.f);
        float BestScore =
            ScoreCommuteChoice(BestProfile, IdentityProfile.WealthLevel);

        FCommuteProfile TransitProfile;

        if (TryBuildTransitCommuteProfile(
                HomeBuilding,
                WorkBuilding,
                DirectDistanceWorld,
                BusRouteSystem,
                TransitProfile))
        {
            const float TransitScore =
                ScoreCommuteChoice(TransitProfile, IdentityProfile.WealthLevel);

            if (TransitScore < BestScore)
            {
                BestProfile = TransitProfile;
                BestScore = TransitScore;
            }
        }

        FCommuteProfile VehicleProfile;

        if (IsAffluentCitizen(IdentityProfile.WealthLevel) &&
            TryBuildVehicleCommuteProfile(
                HomeBuilding,
                WorkBuilding,
                DirectDistanceWorld,
                RoadNetwork,
                VehicleProfile))
        {
            const float VehicleScore =
                ScoreCommuteChoice(VehicleProfile, IdentityProfile.WealthLevel);

            if (VehicleScore < BestScore)
                BestProfile = VehicleProfile;
        }

        return BestProfile;
    }

    float EstimateCommuteTime(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding,
        const FCitizenIdentityProfile& IdentityProfile,
        const CRoadNetwork* RoadNetwork,
        const CBusRouteSystem* BusRouteSystem)
    {
        return ResolveCommuteProfile(
            HomeBuilding,
            WorkBuilding,
            IdentityProfile,
            RoadNetwork,
            BusRouteSystem).TravelSeconds;
    }

    float EstimateCommutePenalty(float CommuteTimeSeconds)
    {
        return ResolveCommutePenaltyNormalized(CommuteTimeSeconds) * 100.f;
    }
}
