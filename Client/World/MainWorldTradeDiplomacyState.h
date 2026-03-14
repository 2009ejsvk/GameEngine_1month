#pragma once

#include "MainWorldAccess.h"
#include <array>
#include <vector>

struct FMainWorldTradeDiplomacyState
{
    std::vector<FTradeRouteRuntimeState> ActiveTradeRoutes;
    std::vector<FTradeRouteCompletionRecord> CompletedTradeRoutes;
    std::array<
        TradeDiplomacyRuntime::FForeignPowerStandingState,
        TradeDiplomacyRuntime::GForeignPowerCount> ForeignPowerStandingStates = {};
    std::array<
        TradeDiplomacyRuntime::FForeignPowerWorldState,
        TradeDiplomacyRuntime::GForeignPowerCount> ForeignPowerStates = {};
    int NextTradeRouteId = 1;
    int NextTradeRouteCompletionRecordId = 1;
    int TradeRouteCompletionNotificationVersion = 0;
};
