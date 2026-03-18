#include "InfrastructureSubsystem.h"
#include "BusRouteSystem.h"
#include "MainWorld.h"
#include "MainWorldInfrastructureRuntime.h"
#include "RoadNetwork.h"
#include "../ObjectNames.h"
#include "../Map/TileMapMain.h"
#include "Component/TileMapComponent.h"

void CInfrastructureSubsystem::RebuildRoadNetwork()
{
    if (!mOwner)
        return;

    if (!RoadNetwork)
        RoadNetwork = std::make_shared<CRoadNetwork>();

    auto TileMapObj = mOwner->FindObject<CTileMapObject>(GTileMapObjectName).lock();

    if (!TileMapObj)
    {
        RoadNetwork->Rebuild(std::shared_ptr<CTileMapComponent>());
        return;
    }

    RoadNetwork->Rebuild(TileMapObj->GetTileMap().lock());
    RebuildBusRoutes();
}

void CInfrastructureSubsystem::RefreshPowerGridCoverage()
{
    if (!mOwner)
        return;

    MainWorldInfrastructureRuntime::RefreshPowerGridCoverage(mOwner);
}

void CInfrastructureSubsystem::RefreshBuildingPollutionExposure()
{
    if (!mOwner)
        return;

    MainWorldInfrastructureRuntime::RefreshBuildingPollutionExposure(mOwner);
}

void CInfrastructureSubsystem::RebuildBusRoutes()
{
    if (!mOwner)
        return;

    if (!BusRouteSystem)
        BusRouteSystem = std::make_shared<CBusRouteSystem>();

    BusRouteSystem->Rebuild(mOwner->mSelf.lock(), RoadNetwork.get());
}
