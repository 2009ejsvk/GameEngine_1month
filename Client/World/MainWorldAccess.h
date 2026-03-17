#pragma once

#include "MainWorldTradeAccess.h"
#include "MainWorldBuildingControlAccess.h"
#include "MainWorldInfrastructureAccess.h"
#include "MainWorldSystemAccess.h"
#include "MainWorldUiReadAccess.h"
#include "../Economy/TradeDiplomacyRuntime.h"
#include "../Economy/TradeRouteRuntimeState.h"
#include "../Building/BuildingTypes.h"
#include "../Map/PlacementAreaRuntimeState.h"
#include "../Politics/ConstitutionTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <array>
#include <memory>
#include <string>
#include <vector>

class CRoadNetwork;
class CBusRouteSystem;
class CWorld;

#define DECLARE_MAIN_WORLD_ACCESS_RESOLVER(AccessType, ResolverName) \
    std::shared_ptr<AccessType> Resolve##ResolverName( \
        const std::shared_ptr<CWorld>& World); \
    AccessType* Resolve##ResolverName(CWorld* World);

#undef DECLARE_MAIN_WORLD_ACCESS_RESOLVER
