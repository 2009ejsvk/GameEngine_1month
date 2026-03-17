#pragma once

#include "../Economy/TradeDiplomacyRuntime.h"
#include "../Building/BuildingTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <array>
#include <memory>
#include <vector>

class CMainWorld;
class CWorld;
class CMainWorldUiReadAccess;

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
