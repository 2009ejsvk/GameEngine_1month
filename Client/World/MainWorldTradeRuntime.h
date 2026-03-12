#pragma once

#include "../Economy/TradeRouteRuntimeState.h"
#include "../Economy/TradeDiplomacyRuntime.h"
#include <memory>
#include <string>
#include <vector>

class CWorld;
class CPlacementAreaObject;

namespace MainWorldTradeRuntime
{
    struct FCustomsTradeModifierSummary
    {
        int ExportPricePercent = 0;
        int ImportPricePercent = 0;
    };

    const wchar_t* GetForeignPowerName(int Index);
    std::wstring FormatCurrency(long long Value);
    std::wstring FormatUnits(int Value);
    int CountActiveTradeRoutesForPower(
        const std::vector<FTradeRouteRuntimeState>& ActiveRoutes,
        int ForeignPowerIndex);
    int ComputeTradeRouteSignedStandardPricePerThousand(
        EResourceType Type,
        bool ImportRoute);
    int ResolveTradeRouteDurationDays(int ContractUnits);
    int ResolveTradeRouteDailyTransferUnits(
        const FTradeRouteRuntimeState& Route);
    int ResolveTradeRouteCompletionRewardModifier(
        const FTradeRouteRuntimeState& Route,
        ETradeRouteEndReason EndReason);
    int ResolveTradeRouteSecondaryRelationModifier(
        const FTradeRouteRuntimeState& Route,
        ETradeRouteEndReason EndReason);
    int ResolveTradeRouteStandingModifier(
        const FTradeRouteRuntimeState& Route,
        ETradeRouteEndReason EndReason);
    void ApplyTradeRouteActivationState(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState,
        const FTradeRouteRuntimeState& Route);
    void ApplyTradeRouteCompletionState(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState,
        const FTradeRouteCompletionRecord& Record);
    void ApplyForeignDemandStandingDelta(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState,
        int RelationDelta,
        int StandingDelta);
    void ApplyForeignPowerIdleDecay(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState);
    std::vector<std::shared_ptr<CPlacementAreaObject>> CollectOperationalHarbors(
        const std::shared_ptr<CWorld>& World);
    FCustomsTradeModifierSummary CollectCustomsTradeModifierSummary(
        CWorld* World);
}
