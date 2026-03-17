#pragma once

#include "../Economy/TradeDiplomacyRuntime.h"
#include "../Economy/TradeRouteRuntimeState.h"
#include "../Building/BuildingTypes.h"
#include <array>
#include <memory>
#include <vector>

class CMainWorld;
class CWorld;
class CMainWorldTradeAccessGroup;

class IMainWorldTradeAccess
{
public:
    virtual ~IMainWorldTradeAccess() = default;

    virtual EBuildingEra GetCurrentEra() const = 0;
    virtual const std::vector<FTradeRouteRuntimeState>&
        GetActiveTradeRoutes() const = 0;
    virtual const std::vector<FTradeRouteCompletionRecord>&
        GetCompletedTradeRoutes() const = 0;
    virtual int GetTradeRouteCompletionNotificationVersion() const = 0;
    virtual int GetCustomsExportTradePriceModifierPercent() const = 0;
    virtual int GetCustomsImportTradePriceModifierPercent() const = 0;
    virtual const std::array<
        TradeDiplomacyRuntime::FForeignPowerWorldState,
        TradeDiplomacyRuntime::GForeignPowerCount>&
        GetForeignPowerStates() const = 0;
};

std::shared_ptr<CMainWorldTradeAccessGroup> CreateMainWorldTradeAccessAdapter(
    CMainWorld* Owner);

std::shared_ptr<IMainWorldTradeAccess> ResolveMainWorldTradeAccess(
    const std::shared_ptr<CWorld>& World);
IMainWorldTradeAccess* ResolveMainWorldTradeAccess(CWorld* World);
