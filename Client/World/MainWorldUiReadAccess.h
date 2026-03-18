#pragma once

#include "../Economy/TradeDiplomacyRuntime.h"
#include "../Building/BuildingTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <algorithm>
#include <array>
#include <memory>
#include <vector>

class CMainWorld;
class CWorld;
class CMainWorldUiReadAccess;

constexpr int GEconomyLedgerDailyHistoryLength = 24;
constexpr int GEconomyLedgerMonthlyHistoryLength = 24;

struct FEconomyLedgerPeriodRecord
{
    long long NationalBudget = 0;
    long long ExportIncome = 0;
    long long ImportExpense = 0;
    long long ConsumptionTaxIncome = 0;
    long long IncomeTaxIncome = 0;
    long long PropertyTaxIncome = 0;
    long long WageCost = 0;
    long long UpkeepCost = 0;
    long long EdictUpkeepCost = 0;
    long long TaxBudgetDelta = 0;
    long long TradePolicyBudgetDelta = 0;
    long long ConstructionExpense = 0;
    long long RepairExpense = 0;
    long long EdictActivationExpense = 0;
    long long SpecialEventBudgetDelta = 0;
    long long OtherBudgetDelta = 0;
    long long NetBudgetChange = 0;
    double TaxCollectionEfficiency = 0.0;
    int ActiveCitizenCount = 0;
    int AssignedJobCount = 0;
    int JobCapacity = 0;
    int UnemployedCount = 0;
    int WorkVacancyCount = 0;
    int ActiveTouristCount = 0;
    int TourismVisitCapacity = 0;
    int TourismVisitOccupancy = 0;
    int TourismRating = 0;

    long long GetTotalTaxIncome() const
    {
        return ConsumptionTaxIncome +
            IncomeTaxIncome +
            PropertyTaxIncome;
    }

    long long GetTotalIncome() const
    {
        return ExportIncome +
            GetTotalTaxIncome() +
            TradePolicyBudgetDelta +
            (std::max)(0LL, SpecialEventBudgetDelta) +
            (std::max)(0LL, OtherBudgetDelta);
    }

    long long GetTotalExpense() const
    {
        return ImportExpense +
            WageCost +
            UpkeepCost +
            EdictUpkeepCost +
            ConstructionExpense +
            RepairExpense +
            EdictActivationExpense +
            (std::max)(0LL, -SpecialEventBudgetDelta) +
            (std::max)(0LL, -OtherBudgetDelta);
    }

    int GetJobOccupancyPercent() const
    {
        return JobCapacity > 0 ?
            static_cast<int>(
                (std::max)(
                    0LL,
                    (static_cast<long long>(AssignedJobCount) * 100LL) /
                        static_cast<long long>(JobCapacity))) :
            0;
    }

    int GetUnemploymentPercent() const
    {
        return ActiveCitizenCount > 0 ?
            static_cast<int>(
                (std::max)(
                    0LL,
                    (static_cast<long long>(UnemployedCount) * 100LL) /
                        static_cast<long long>(ActiveCitizenCount))) :
            0;
    }

    int GetTourismUtilizationPercent() const
    {
        return TourismVisitCapacity > 0 ?
            static_cast<int>(
                (std::max)(
                    0LL,
                    (static_cast<long long>(TourismVisitOccupancy) * 100LL) /
                        static_cast<long long>(TourismVisitCapacity))) :
            0;
    }
};

struct FEconomyLedgerHistorySnapshot
{
    std::array<
        FEconomyLedgerPeriodRecord,
        GEconomyLedgerDailyHistoryLength> DailyHistory = {};
    std::array<
        FEconomyLedgerPeriodRecord,
        GEconomyLedgerMonthlyHistoryLength> MonthlyHistory = {};
    FEconomyLedgerPeriodRecord CurrentMonthToDate;
    int DailyHistoryCount = 0;
    int MonthlyHistoryCount = 0;
    int CurrentMonthDayCount = 0;
};

class IMainWorldBuildMenuAccess
{
public:
    virtual ~IMainWorldBuildMenuAccess() = default;

    virtual long long GetNationalBudget() const = 0;
    virtual int GetSimulationYear() const = 0;
    virtual int GetSimulationMonth() const = 0;
    virtual int GetSimulationDay() const = 0;
    virtual int GetSimulationMonthDayCount() const = 0;
    virtual float GetSimulationDayProgress() const = 0;
    virtual float GetSimulationMonthProgress() const = 0;
    virtual EBuildingEra GetCurrentEra() const = 0;
    virtual const FEraProgressState& GetEraProgress() const = 0;
    virtual const FEraTransitionState& GetEraTransitionState() const = 0;
    virtual void SpendBuildingCost(int BaseCost) = 0;
};

class IMainWorldHudAccess
{
public:
    virtual ~IMainWorldHudAccess() = default;

    virtual long long GetNationalBudget() const = 0;
    virtual int GetSimulationYear() const = 0;
    virtual int GetSimulationMonth() const = 0;
    virtual int GetSimulationDay() const = 0;
    virtual float GetSimulationMonthProgress() const = 0;
    virtual EBuildingEra GetCurrentEra() const = 0;
    virtual const FEraProgressState& GetEraProgress() const = 0;
    virtual const FEraTransitionState& GetEraTransitionState() const = 0;
    virtual const FTaxPolicy& GetTaxPolicy() const = 0;
    virtual const FPoliticalWorldSnapshot& GetPoliticalSnapshot() const = 0;
    virtual const FElectionStatus& GetElectionStatus() const = 0;
    virtual int GetDaysUntilNextElection() const = 0;
    virtual double GetElectionWarningScore() const = 0;
    virtual const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const = 0;
    virtual const FWorldCrisisStatus& GetWorldCrisisStatus() const = 0;
    virtual const std::array<int, GPoliticalFactionCount>&
        GetFactionDemandPressureDays() const = 0;
    virtual const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
        GetFactionDemandStates() const = 0;
    virtual const FPoliticalDemandNotice& GetPoliticalDemandNotice() const = 0;
    virtual bool IsSimulationPaused() const = 0;
    virtual int GetSimulationSpeedMultiplier() const = 0;
    virtual void ToggleSimulationPaused() = 0;
    virtual void CycleSimulationSpeedMultiplier() = 0;
    virtual bool TryExecuteEraTransition(EEraTransitionChoice Choice) = 0;
};

class IMainWorldAlmanacAccess
{
public:
    virtual ~IMainWorldAlmanacAccess() = default;

    virtual long long GetNationalBudget() const = 0;
    virtual EBuildingEra GetCurrentEra() const = 0;
    virtual const FGovernmentProfile& GetGovernmentProfile() const = 0;
    virtual const FPoliticalWorldSnapshot& GetPoliticalSnapshot() const = 0;
    virtual const std::vector<FGovernmentEdictState>&
        GetGovernmentEdictStates() const = 0;
    virtual long long GetLastDailyEdictCost() const = 0;
    virtual long long GetLastDailyImportExpense() const = 0;
    virtual long long GetLastDailyExportIncome() const = 0;
    virtual long long GetLastDailyTaxIncome() const = 0;
    virtual long long GetLastDailyConsumptionTaxIncome() const = 0;
    virtual long long GetLastDailyIncomeTaxIncome() const = 0;
    virtual long long GetLastDailyPropertyTaxIncome() const = 0;
    virtual double GetLastDailyTaxCollectionEfficiency() const = 0;
    virtual long long GetLastDailyNetChange() const = 0;
    virtual const FEconomyLedgerHistorySnapshot&
        GetEconomyLedgerHistory() const = 0;
    virtual const FElectionStatus& GetElectionStatus() const = 0;
    virtual int GetDaysUntilNextElection() const = 0;
    virtual double GetElectionWarningScore() const = 0;
    virtual const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const = 0;
    virtual const FWorldCrisisStatus& GetWorldCrisisStatus() const = 0;
    virtual const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
        GetFactionDemandStates() const = 0;
    virtual const std::array<
        FPoliticalDemandState,
        TradeDiplomacyRuntime::GForeignPowerCount>&
        GetForeignDemandStates() const = 0;
    virtual const FPoliticalDemandNotice& GetPoliticalDemandNotice() const = 0;
    virtual const std::array<
        TradeDiplomacyRuntime::FForeignPowerWorldState,
        TradeDiplomacyRuntime::GForeignPowerCount>&
        GetForeignPowerStates() const = 0;
};

class IMainWorldEdictReadAccess
{
public:
    virtual ~IMainWorldEdictReadAccess() = default;

    virtual long long GetNationalBudget() const = 0;
    virtual const FTaxPolicy& GetTaxPolicy() const = 0;
    virtual const FPoliticalWorldSnapshot& GetPoliticalSnapshot() const = 0;
    virtual const FGovernmentEdictState* GetGovernmentEdictState(
        EGovernmentEdictType Type) const = 0;
    virtual long long GetLastDailyTaxIncome() const = 0;
    virtual const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const = 0;
    virtual EBuildingEra GetCurrentEra() const = 0;
    virtual const FEraProgressState& GetEraProgress() const = 0;
    virtual const FEraTransitionState& GetEraTransitionState() const = 0;
};

std::shared_ptr<CMainWorldUiReadAccess> CreateMainWorldUiReadAccessAdapter(
    CMainWorld* Owner);

std::shared_ptr<IMainWorldBuildMenuAccess> ResolveMainWorldBuildMenuAccess(
    const std::shared_ptr<CWorld>& World);
IMainWorldBuildMenuAccess* ResolveMainWorldBuildMenuAccess(CWorld* World);

std::shared_ptr<IMainWorldHudAccess> ResolveMainWorldHudAccess(
    const std::shared_ptr<CWorld>& World);
IMainWorldHudAccess* ResolveMainWorldHudAccess(CWorld* World);

std::shared_ptr<IMainWorldAlmanacAccess> ResolveMainWorldAlmanacAccess(
    const std::shared_ptr<CWorld>& World);
IMainWorldAlmanacAccess* ResolveMainWorldAlmanacAccess(CWorld* World);

std::shared_ptr<IMainWorldEdictReadAccess> ResolveMainWorldEdictReadAccess(
    const std::shared_ptr<CWorld>& World);
IMainWorldEdictReadAccess* ResolveMainWorldEdictReadAccess(CWorld* World);
