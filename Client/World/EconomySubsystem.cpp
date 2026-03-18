#include "EconomySubsystem.h"
#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "../Economy/EconomySystem.h"
#include "../Economy/ResourceTradePricing.h"
#include "../Economy/TradePolicyRuntime.h"
#include "WorldStatsSnapshot.h"
#include <cmath>

namespace
{
    template <typename T, size_t N>
    void PushHistoryEntry(
        std::array<T, N>& History,
        int& InOutCount,
        const T& Entry)
    {
        if (InOutCount < static_cast<int>(N))
        {
            History[static_cast<size_t>(InOutCount)] = Entry;
            ++InOutCount;
            return;
        }

        for (size_t Index = 1; Index < N; ++Index)
            History[Index - 1] = History[Index];

        History[N - 1] = Entry;
        InOutCount = static_cast<int>(N);
    }

    int ComputeTourismRating(const WorldStats::FWorldStatsSnapshot& Snapshot)
    {
        const int CurrentTouristCount = (std::max)(0, Snapshot.ActiveTouristCount);
        const int TourismVisitCapacity = (std::max)(0, Snapshot.TourismVisitCapacity);
        const int TourismVisitOccupancy = (std::max)(0, Snapshot.TourismVisitOccupancy);
        const int TouristPreferenceMatchedCount =
            (std::max)(0, Snapshot.TouristPreferenceMatchedCount);
        const int TouristPreferenceMatchedPercent =
            CurrentTouristCount > 0 ?
                (std::min)(
                    100,
                    (std::max)(
                        0,
                        static_cast<int>(std::lround(
                            static_cast<double>(TouristPreferenceMatchedCount) *
                            100.0 /
                            static_cast<double>(CurrentTouristCount))))) :
                0;
        const int TourismVisitUtilizationPercent =
            TourismVisitCapacity > 0 ?
                (std::min)(
                    100,
                    (std::max)(
                        0,
                        static_cast<int>(std::lround(
                            static_cast<double>(TourismVisitOccupancy) *
                            100.0 /
                            static_cast<double>(TourismVisitCapacity))))) :
                0;

        int TourismProfileDiversityCount = 0;
        for (int PreferenceIndex = 1;
            PreferenceIndex < GTouristPreferenceCount;
            ++PreferenceIndex)
        {
            if (Snapshot.TouristProfileCount[PreferenceIndex] > 0)
                ++TourismProfileDiversityCount;
        }

        if (Snapshot.TourismBuildingCount <= 0 && CurrentTouristCount <= 0)
            return 0;

        return (std::min)(
            99,
            (std::max)(
                0,
                18 +
                    (std::min)(28, Snapshot.TourismBuildingCount * 4) +
                    (std::min)(18, TourismProfileDiversityCount * 3) +
                    (std::min)(22, TouristPreferenceMatchedPercent / 3) +
                    (std::min)(14, TourismVisitUtilizationPercent / 6) +
                    (std::min)(8, Snapshot.HarborCount * 2)));
    }

    void AddMonthlyTotals(
        FEconomyLedgerPeriodRecord& InOutMonth,
        const FEconomyLedgerPeriodRecord& DayRecord)
    {
        InOutMonth.ExportIncome += DayRecord.ExportIncome;
        InOutMonth.ImportExpense += DayRecord.ImportExpense;
        InOutMonth.ConsumptionTaxIncome += DayRecord.ConsumptionTaxIncome;
        InOutMonth.IncomeTaxIncome += DayRecord.IncomeTaxIncome;
        InOutMonth.PropertyTaxIncome += DayRecord.PropertyTaxIncome;
        InOutMonth.WageCost += DayRecord.WageCost;
        InOutMonth.UpkeepCost += DayRecord.UpkeepCost;
        InOutMonth.EdictUpkeepCost += DayRecord.EdictUpkeepCost;
        InOutMonth.TaxBudgetDelta += DayRecord.TaxBudgetDelta;
        InOutMonth.TradePolicyBudgetDelta += DayRecord.TradePolicyBudgetDelta;
        InOutMonth.ConstructionExpense += DayRecord.ConstructionExpense;
        InOutMonth.RepairExpense += DayRecord.RepairExpense;
        InOutMonth.EdictActivationExpense += DayRecord.EdictActivationExpense;
        InOutMonth.SpecialEventBudgetDelta += DayRecord.SpecialEventBudgetDelta;
        InOutMonth.OtherBudgetDelta += DayRecord.OtherBudgetDelta;
        InOutMonth.NetBudgetChange += DayRecord.NetBudgetChange;
        InOutMonth.NationalBudget = DayRecord.NationalBudget;
        InOutMonth.ActiveCitizenCount = DayRecord.ActiveCitizenCount;
        InOutMonth.AssignedJobCount = DayRecord.AssignedJobCount;
        InOutMonth.JobCapacity = DayRecord.JobCapacity;
        InOutMonth.UnemployedCount = DayRecord.UnemployedCount;
        InOutMonth.WorkVacancyCount = DayRecord.WorkVacancyCount;
        InOutMonth.ActiveTouristCount = DayRecord.ActiveTouristCount;
        InOutMonth.TourismVisitCapacity = DayRecord.TourismVisitCapacity;
        InOutMonth.TourismVisitOccupancy = DayRecord.TourismVisitOccupancy;
        InOutMonth.TourismRating = DayRecord.TourismRating;
    }
}

void CEconomySubsystem::Reset()
{
    NationalBudget = MainWorldConfig::GInitialNationalBudget;
    LastDailyWageCost = 0;
    LastDailyUpkeepCost = 0;
    LastDailyExportIncome = 0;
    LastDailyBaseTaxIncome = 0;
    LastDailyTaxIncome = 0;
    LastDailyTaxBudgetDelta = 0;
    LastDailyConsumptionTaxIncome = 0;
    LastDailyIncomeTaxIncome = 0;
    LastDailyPropertyTaxIncome = 0;
    LastDailyEdictCost = 0;
    LastDailyImportExpense = 0;
    LastDailyTradePolicyBudgetDelta = 0;
    LastDailyNetChange = 0;
    LastDailyTaxCollectionEfficiency = 0.0;
    WorkerTaxPressureDays = 0;
    PropertyTaxPressureDays = 0;
    BudgetCrisisPressureDays = 0;
    TaxEventStatus = FTaxPolicyEventStatus();
    mCurrentDayAdjustments = FDirectBudgetAdjustments();
    mLedgerHistory = FEconomyLedgerHistorySnapshot();
    mCurrentMonthTaxEfficiencySum = 0.0;
}

void CEconomySubsystem::OnDayAdvanced(const FSettlementContext& Context)
{
    if (!mOwner)
        return;

    const int DaysInMonth = mOwner->mSimulation->GetDaysInMonth(
        Context.SimulationYear,
        Context.SimulationMonth);
    const auto Result = EconomySystem::ApplyDailyWorldSettlement(
        mOwner,
        DaysInMonth,
        Context.GovernmentProfile,
        TaxEventStatus,
        Context.GovernmentEdicts,
        Context.EdictModifiers);
    LastDailyWageCost = Result.BaseResult.WageCost;
    LastDailyUpkeepCost = Result.BaseResult.UpkeepCost;
    LastDailyExportIncome = Result.BaseResult.ExportIncome;
    LastDailyBaseTaxIncome = Result.BaseResult.TaxIncome;
    LastDailyTaxIncome = Result.AdjustedTaxIncome;
    LastDailyTaxBudgetDelta =
        Result.AdjustedTaxIncome - Result.BaseResult.TaxIncome;
    LastDailyConsumptionTaxIncome = Result.AdjustedConsumptionTaxIncome;
    LastDailyIncomeTaxIncome = Result.AdjustedIncomeTaxIncome;
    LastDailyPropertyTaxIncome = Result.AdjustedPropertyTaxIncome;
    LastDailyEdictCost = Result.DailyEdictCost;
    LastDailyImportExpense = Result.BaseResult.ImportExpense;
    LastDailyTradePolicyBudgetDelta = Result.DailyTradePolicyBudgetDelta;
    LastDailyTaxCollectionEfficiency =
        Result.BaseResult.TaxCollectionEfficiency;
    LastDailyNetChange = Result.NetBudgetChange;
    NationalBudget += LastDailyNetChange;
}

void CEconomySubsystem::ApplyDailyTaxPolicyEventEffects()
{
    if (!mOwner)
        return;

    EconomySystem::ApplyDailyTaxPolicyEventEffects(
        mOwner,
        TaxEventStatus);
}

void CEconomySubsystem::RefreshWorldMarketPrices(
    const FWorldMarketContext& Context)
{
    if (!mOwner)
        return;

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
        return;

    ResourceTradePricing::UpdateWorldMarketPrices(
        WorldStats::BuildSnapshot(World),
        Context.GovernmentProfile,
        Context.GovernmentEdicts,
        TaxEventStatus,
        Context.WorldCrisisStatus,
        Context.ForeignPowerStates,
        Context.SimulationYear,
        Context.SimulationMonth,
        Context.SimulationDay);
}

void CEconomySubsystem::TickTaxEvents(const FTaxEventsContext& Context)
{
    const long long BudgetBefore = NationalBudget;
    EconomySystem::TickTaxPolicyEvents(
        Context.PoliticalSnapshot,
        Context.GovernmentProfile,
        Context.SimulationYear,
        Context.SimulationMonth,
        Context.SimulationDay,
        NationalBudget,
        LastDailyNetChange,
        WorkerTaxPressureDays,
        PropertyTaxPressureDays,
        BudgetCrisisPressureDays,
        TaxEventStatus);
    RecordSpecialEventBudgetDelta(NationalBudget - BudgetBefore);
}

void CEconomySubsystem::ApplyConstructionExpense(long long Cost)
{
    if (Cost <= 0)
        return;

    mCurrentDayAdjustments.ConstructionExpense += Cost;
    ApplyImmediateBudgetDelta(-Cost);
}

void CEconomySubsystem::ApplyRepairExpense(long long Cost)
{
    if (Cost <= 0)
        return;

    mCurrentDayAdjustments.RepairExpense += Cost;
    ApplyImmediateBudgetDelta(-Cost);
}

void CEconomySubsystem::ApplyEdictActivationExpense(long long Cost)
{
    if (Cost <= 0)
        return;

    mCurrentDayAdjustments.EdictActivationExpense += Cost;
    ApplyImmediateBudgetDelta(-Cost);
}

void CEconomySubsystem::ApplySpecialEventBudgetDelta(long long Delta)
{
    if (Delta == 0)
        return;

    RecordSpecialEventBudgetDelta(Delta);
    ApplyImmediateBudgetDelta(Delta);
}

void CEconomySubsystem::ApplyOtherBudgetDelta(long long Delta)
{
    if (Delta == 0)
        return;

    RecordOtherBudgetDelta(Delta);
    ApplyImmediateBudgetDelta(Delta);
}

void CEconomySubsystem::RecordSpecialEventBudgetDelta(long long Delta)
{
    if (Delta == 0)
        return;

    mCurrentDayAdjustments.SpecialEventBudgetDelta += Delta;
}

void CEconomySubsystem::RecordOtherBudgetDelta(long long Delta)
{
    if (Delta == 0)
        return;

    mCurrentDayAdjustments.OtherBudgetDelta += Delta;
}

void CEconomySubsystem::RefreshTradePolicyBudgetDelta(
    const FGovernmentProfile& GovernmentProfile)
{
    const long long TotalTradePolicyBudgetDelta =
        TradePolicyRuntime::ComputeDailyTradePolicyBudgetDelta(
            GovernmentProfile.ExportTradePolicy,
            GovernmentProfile.ImportTradePolicy,
            LastDailyExportIncome,
            LastDailyImportExpense);
    const long long DeltaDifference =
        TotalTradePolicyBudgetDelta - LastDailyTradePolicyBudgetDelta;

    if (DeltaDifference != 0)
        ApplyImmediateBudgetDelta(DeltaDifference);

    LastDailyTradePolicyBudgetDelta = TotalTradePolicyBudgetDelta;
}

void CEconomySubsystem::FinalizeDayLedger(
    const WorldStats::FWorldStatsSnapshot& Snapshot,
    bool EndOfMonth)
{
    FEconomyLedgerPeriodRecord Record;
    Record.NationalBudget = NationalBudget;
    Record.ExportIncome = LastDailyExportIncome;
    Record.ImportExpense = LastDailyImportExpense;
    Record.ConsumptionTaxIncome = LastDailyConsumptionTaxIncome;
    Record.IncomeTaxIncome = LastDailyIncomeTaxIncome;
    Record.PropertyTaxIncome = LastDailyPropertyTaxIncome;
    Record.WageCost = LastDailyWageCost;
    Record.UpkeepCost = LastDailyUpkeepCost;
    Record.EdictUpkeepCost = LastDailyEdictCost;
    Record.TaxBudgetDelta = LastDailyTaxBudgetDelta;
    Record.TradePolicyBudgetDelta = LastDailyTradePolicyBudgetDelta;
    Record.ConstructionExpense = mCurrentDayAdjustments.ConstructionExpense;
    Record.RepairExpense = mCurrentDayAdjustments.RepairExpense;
    Record.EdictActivationExpense =
        mCurrentDayAdjustments.EdictActivationExpense;
    Record.SpecialEventBudgetDelta =
        mCurrentDayAdjustments.SpecialEventBudgetDelta;
    Record.OtherBudgetDelta = mCurrentDayAdjustments.OtherBudgetDelta;
    Record.TaxCollectionEfficiency = LastDailyTaxCollectionEfficiency;
    Record.ActiveCitizenCount = Snapshot.ActiveCitizenCount;
    Record.AssignedJobCount = Snapshot.AssignedJobCount;
    Record.JobCapacity = Snapshot.JobCapacity;
    Record.UnemployedCount = Snapshot.UnemployedCount;
    Record.WorkVacancyCount =
        (std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount);
    Record.ActiveTouristCount = Snapshot.ActiveTouristCount;
    Record.TourismVisitCapacity = Snapshot.TourismVisitCapacity;
    Record.TourismVisitOccupancy = Snapshot.TourismVisitOccupancy;
    Record.TourismRating = ComputeTourismRating(Snapshot);
    Record.NetBudgetChange =
        Record.GetTotalIncome() - Record.GetTotalExpense();
    LastDailyNetChange = Record.NetBudgetChange;

    PushDailyHistory(Record);
    AddMonthlyTotals(mLedgerHistory.CurrentMonthToDate, Record);
    mLedgerHistory.CurrentMonthDayCount += 1;
    mCurrentMonthTaxEfficiencySum += Record.TaxCollectionEfficiency;

    if (mLedgerHistory.CurrentMonthDayCount > 0)
    {
        mLedgerHistory.CurrentMonthToDate.TaxCollectionEfficiency =
            mCurrentMonthTaxEfficiencySum /
            static_cast<double>(mLedgerHistory.CurrentMonthDayCount);
    }

    if (EndOfMonth)
    {
        PushMonthlyHistory(mLedgerHistory.CurrentMonthToDate);
        mLedgerHistory.CurrentMonthToDate = FEconomyLedgerPeriodRecord();
        mLedgerHistory.CurrentMonthDayCount = 0;
        mCurrentMonthTaxEfficiencySum = 0.0;
    }

    mCurrentDayAdjustments = FDirectBudgetAdjustments();
}

void CEconomySubsystem::ApplyImmediateBudgetDelta(long long Delta)
{
    if (Delta == 0)
        return;

    NationalBudget += Delta;
    LastDailyNetChange += Delta;
}

void CEconomySubsystem::PushDailyHistory(
    const FEconomyLedgerPeriodRecord& Record)
{
    PushHistoryEntry(
        mLedgerHistory.DailyHistory,
        mLedgerHistory.DailyHistoryCount,
        Record);
}

void CEconomySubsystem::PushMonthlyHistory(
    const FEconomyLedgerPeriodRecord& Record)
{
    PushHistoryEntry(
        mLedgerHistory.MonthlyHistory,
        mLedgerHistory.MonthlyHistoryCount,
        Record);
}

bool CEconomySubsystem::AdjustTaxPolicy(
    ETaxPolicyType Type,
    int DeltaPercent,
    std::wstring& OutMessage)
{
    if (!mOwner)
    {
        OutMessage = L"월드 상태를 확인할 수 없습니다.";
        return false;
    }

    const bool Adjusted = EconomySystem::AdjustTaxPolicy(
        mOwner->mPolitics->GovernmentProfile.TaxPolicy,
        Type,
        DeltaPercent,
        OutMessage);

    if (!Adjusted)
        return false;

    mOwner->mTrade->OnDayAdvanced(
        {
            mOwner->mPolitics->GovernmentProfile,
            TaxEventStatus,
            mOwner->mEdictState->GovernmentEdicts,
            mOwner->mEraState->EraProgress.CurrentEra,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day,
            false
        });
    RefreshWorldMarketPrices(
        {
            mOwner->mPolitics->GovernmentProfile,
            mOwner->mEdictState->GovernmentEdicts,
            mOwner->mCrisis->WorldCrisisService->GetStatus(),
            mOwner->mTrade->State.ForeignPowerStates,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day
        });
    return true;
}
