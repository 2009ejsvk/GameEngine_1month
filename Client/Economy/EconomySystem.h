#pragma once

#include "../Politics/PoliticalTypes.h"
#include <array>
#include <string>

class CWorld;

namespace EconomySystem
{
    struct FDailyResult
    {
        long long WageCost              = 0;
        long long UpkeepCost            = 0;
        long long ImportExpense         = 0;
        long long ExportIncome          = 0;
        long long TaxIncome             = 0;
        long long ConsumptionTaxIncome  = 0;
        long long IncomeTaxIncome       = 0;
        long long PropertyTaxIncome     = 0;
        double    TaxCollectionEfficiency = 0.0;
        long long NetChange             = 0;
    };

    struct FWorldSettlementResult
    {
        FDailyResult BaseResult;
        long long AdjustedTaxIncome = 0;
        long long AdjustedConsumptionTaxIncome = 0;
        long long AdjustedIncomeTaxIncome = 0;
        long long AdjustedPropertyTaxIncome = 0;
        long long DailyEdictCost = 0;
        long long DailyTradePolicyBudgetDelta = 0;
        long long NetBudgetChange = 0;
    };

    // 하루 경제 정산을 수행하고 결과를 반환한다.
    // Budget은 직접 갱신하지 않으며 호출자가 NetChange를 반영한다.
    FDailyResult ApplyDailySettlement(
        CWorld* World,
        int DaysInMonth,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus* TaxEventStatus = nullptr,
        const FGovernmentEdictModifiers* EdictModifiers = nullptr);

    FWorldSettlementResult ApplyDailyWorldSettlement(
        CWorld* World,
        int DaysInMonth,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const std::vector<FGovernmentEdictState>& GovernmentEdicts,
        const FGovernmentEdictModifiers& EdictModifiers);

    void AdvanceHarborShipProgress(
        CWorld* World,
        int DaysInMonth,
        const FGovernmentEdictModifiers* EdictModifiers = nullptr);

    int ApplyTaxPolicyRateDelta(
        FTaxPolicy& TaxPolicy,
        ETaxPolicyType Type,
        int DeltaPercent);

    bool AdjustTaxPolicy(
        FTaxPolicy& TaxPolicy,
        ETaxPolicyType Type,
        int DeltaPercent,
        std::wstring& OutMessage);

    ETaxPolicyEventType GetRequiredTaxPolicyEventForEdict(
        EGovernmentEdictType Type);

    const wchar_t* GetTaxPolicyEventTitle(ETaxPolicyEventType Type);

    void ResolveTaxPolicyEvent(
        FTaxPolicyEventStatus& InOutTaxEventStatus,
        bool Success);

    void ApplyDailyTaxPolicyEventEffects(
        CWorld* World,
        const FTaxPolicyEventStatus& TaxEventStatus);

    void TickTaxPolicyEvents(
        const FPoliticalWorldSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        int SimulationYear,
        int SimulationMonth,
        int SimulationDay,
        long long& InOutNationalBudget,
        long long& InOutLastDailyNetChange,
        int& InOutWorkerTaxPressureDays,
        int& InOutPropertyTaxPressureDays,
        int& InOutBudgetCrisisPressureDays,
        FTaxPolicyEventStatus& InOutTaxEventStatus);
}
