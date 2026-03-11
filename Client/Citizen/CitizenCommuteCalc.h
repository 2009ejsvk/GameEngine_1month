#pragma once

#include "CitizenTypes.h"
#include <memory>

class CPlacementAreaObject;
class CRoadNetwork;
class CBusRouteSystem;

namespace CitizenCommuteCalc
{
    float EstimateCommuteTime(
        const std::shared_ptr<CPlacementAreaObject>& HomeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& WorkBuilding,
        const FCitizenIdentityProfile& IdentityProfile,
        const CRoadNetwork* RoadNetwork = nullptr,
        const CBusRouteSystem* BusRouteSystem = nullptr);

    float EstimateCommutePenalty(float CommuteTimeSeconds);
}
