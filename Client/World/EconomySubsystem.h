#pragma once

#include "World/WorldSubsystem.h"
#include "MainWorldUiReadAccess.h"
#include "MainWorldWorldCrisisService.h"
#include "../Economy/TradeDiplomacyRuntime.h"
#include "../Politics/PoliticalTypes.h"
#include <array>
#include <string>
#include <vector>

namespace WorldStats
{
    struct FWorldStatsSnapshot;
}

class CEconomySubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    struct FDirectBudgetAdjustments
    {
        long long ConstructionExpense = 0;
        long long RepairExpense = 0;
        long long EdictActivationExpense = 0;
        long long SpecialEventBudgetDelta = 0;
        long long OtherBudgetDelta = 0;
    };

    struct FSettlementContext
    {
        const FGovernmentProfile& GovernmentProfile;
        const std::vector<FGovernmentEdictState>& GovernmentEdicts;
        const FGovernmentEdictModifiers& EdictModifiers;
        int SimulationYear = 0;
        int SimulationMonth = 1;
    };

    struct FWorldMarketContext
    {
        const FGovernmentProfile& GovernmentProfile;
        const std::vector<FGovernmentEdictState>& GovernmentEdicts;
        const FWorldCrisisStatus& WorldCrisisStatus;
        const std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount>& ForeignPowerStates;
        int SimulationYear = 0;
        int SimulationMonth = 1;
        int SimulationDay = 1;
    };

    struct FTaxEventsContext
    {
        const FPoliticalWorldSnapshot& PoliticalSnapshot;
        const FGovernmentProfile& GovernmentProfile;
        int SimulationYear = 0;
        int SimulationMonth = 1;
        int SimulationDay = 1;
    };

    void Reset();
    void OnDayAdvanced(const FSettlementContext& Context);
    void ApplyDailyTaxPolicyEventEffects();
    void RefreshWorldMarketPrices(const FWorldMarketContext& Context);
    void TickTaxEvents(const FTaxEventsContext& Context);
    void ApplyConstructionExpense(long long Cost);
    void ApplyRepairExpense(long long Cost);
    void ApplyEdictActivationExpense(long long Cost);
    void ApplySpecialEventBudgetDelta(long long Delta);
    void ApplyOtherBudgetDelta(long long Delta);
    void RecordSpecialEventBudgetDelta(long long Delta);
    void RecordOtherBudgetDelta(long long Delta);
    void RefreshTradePolicyBudgetDelta(
        const FGovernmentProfile& GovernmentProfile);
    void FinalizeDayLedger(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        bool EndOfMonth);
    const FEconomyLedgerHistorySnapshot& GetLedgerHistory() const
    {
        return mLedgerHistory;
    }
    bool AdjustTaxPolicy(
        ETaxPolicyType Type,
        int DeltaPercent,
        std::wstring& OutMessage);

    long long NationalBudget = 0;
    long long LastDailyWageCost = 0;
    long long LastDailyUpkeepCost = 0;
    long long LastDailyExportIncome = 0;
    long long LastDailyBaseTaxIncome = 0;
    long long LastDailyTaxIncome = 0;
    long long LastDailyTaxBudgetDelta = 0;
    long long LastDailyConsumptionTaxIncome = 0;
    long long LastDailyIncomeTaxIncome = 0;
    long long LastDailyPropertyTaxIncome = 0;
    long long LastDailyEdictCost = 0;
    long long LastDailyImportExpense = 0;
    long long LastDailyTradePolicyBudgetDelta = 0;
    long long LastDailyNetChange = 0;
    double LastDailyTaxCollectionEfficiency = 0.0;
    int WorkerTaxPressureDays = 0;
    int PropertyTaxPressureDays = 0;
    int BudgetCrisisPressureDays = 0;
    FTaxPolicyEventStatus TaxEventStatus;

private:
    void ApplyImmediateBudgetDelta(long long Delta);
    void PushDailyHistory(const FEconomyLedgerPeriodRecord& Record);
    void PushMonthlyHistory(const FEconomyLedgerPeriodRecord& Record);

private:
    FDirectBudgetAdjustments mCurrentDayAdjustments;
    FEconomyLedgerHistorySnapshot mLedgerHistory;
    double mCurrentMonthTaxEfficiencySum = 0.0;
};
