#pragma once

#include "World/WorldSubsystem.h"
#include <memory>

class CRoadNetwork;
class CBusRouteSystem;

class CInfrastructureSubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    void RebuildRoadNetwork();
    void RefreshPowerGridCoverage();
    void RefreshBuildingPollutionExposure();

private:
    void RebuildBusRoutes();

public:
    std::shared_ptr<CRoadNetwork> RoadNetwork;
    std::shared_ptr<CBusRouteSystem> BusRouteSystem;
};
