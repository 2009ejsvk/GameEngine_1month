#pragma once

#include "CitizenTypes.h"
#include <memory>

class CPlacementAreaObject;
class CRoadNetwork;
class CBusRouteSystem;

enum class ECommuteMode
{
    None = 0,
    Walk,
    Transit,
    Vehicle
};

struct FCommuteProfile
{
    ECommuteMode Mode = ECommuteMode::None;
    float DirectDistanceWorld = 0.f;
    float TravelSeconds = 0.f;
    float WalkDistance = 0.f;
    float RideDistance = 0.f;
    float WaitSeconds = 0.f;
    float ServiceQuality = 0.f;
    float CrowdingPenalty = 0.f;
    float ProductivityMultiplier = 1.f;
    float JobRecoveryMultiplier = 1.f;
};

namespace CitizenCommuteCalc
{
    FCommuteProfile ResolveCommuteProfile(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding,
        const FCitizenIdentityProfile& IdentityProfile,
        const CRoadNetwork* RoadNetwork = nullptr,
        const CBusRouteSystem* BusRouteSystem = nullptr);

    float EstimateCommuteTime(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding,
        const FCitizenIdentityProfile& IdentityProfile,
        const CRoadNetwork* RoadNetwork = nullptr,
        const CBusRouteSystem* BusRouteSystem = nullptr);

    float EstimateCommutePenalty(float CommuteTimeSeconds);
}
