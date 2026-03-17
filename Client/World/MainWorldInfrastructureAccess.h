#pragma once

#include <memory>

class CMainWorld;
class CWorld;
class CRoadNetwork;
class CBusRouteSystem;
class CMainWorldInfrastructureReadAccess;

class IMainWorldRoadNetworkAccess
{
public:
    virtual ~IMainWorldRoadNetworkAccess() = default;

    virtual void RebuildRoadNetwork() = 0;
    virtual const CRoadNetwork* GetRoadNetwork() const = 0;
};

class IMainWorldTransitAccess
{
public:
    virtual ~IMainWorldTransitAccess() = default;

    virtual const CBusRouteSystem* GetBusRouteSystem() const = 0;
};

std::shared_ptr<CMainWorldInfrastructureReadAccess>
    CreateMainWorldInfrastructureReadAccessAdapter(CMainWorld* Owner);

std::shared_ptr<IMainWorldRoadNetworkAccess> ResolveMainWorldRoadNetworkAccess(
    const std::shared_ptr<CWorld>& World);
IMainWorldRoadNetworkAccess* ResolveMainWorldRoadNetworkAccess(CWorld* World);

std::shared_ptr<IMainWorldTransitAccess> ResolveMainWorldTransitAccess(
    const std::shared_ptr<CWorld>& World);
IMainWorldTransitAccess* ResolveMainWorldTransitAccess(CWorld* World);
