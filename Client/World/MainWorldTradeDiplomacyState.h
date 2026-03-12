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
    std::array<
        FPoliticalDemandState,
        TradeDiplomacyRuntime::GForeignPowerCount> ForeignPowerDemands = {};
    std::array<
        int,
        TradeDiplomacyRuntime::GForeignPowerCount> ForeignDemandCooldownDays = {};
    int NextTradeRouteId = 1;
    int NextTradeRouteCompletionRecordId = 1;
    int TradeRouteCompletionNotificationVersion = 0;
};
