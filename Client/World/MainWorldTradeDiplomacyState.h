#pragma once

#include "MainWorldAccess.h"
#include "../Economy/TradeOfferRuntimeState.h"
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
    std::vector<FTradeOfferRuntimeState> AvailableTradeOffers;
    int NextTradeRouteId = 1;
    int NextTradeRouteCompletionRecordId = 1;
    int TradeRouteCompletionNotificationVersion = 0;
    int NextTradeOfferId = 1;
    int TradeOfferRefreshYear = 0;
    int TradeOfferRefreshMonth = 1;
    int TradeOfferRefreshDay = 1;
    int TradeOfferRotationIndex = 0;
};
