#pragma once

#include "../Economy/TradeDiplomacyRuntime.h"
#include "../Economy/TradeRouteRuntimeState.h"
#include "../Building/BuildingTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <array>
#include <string>
#include <vector>

class CRoadNetwork;
class CBusRouteSystem;

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
    virtual const FTaxPolicy& GetTaxPolicy() const = 0;
    virtual const FPoliticalWorldSnapshot& GetPoliticalSnapshot() const = 0;
    virtual const FElectionStatus& GetElectionStatus() const = 0;
    virtual int GetDaysUntilNextElection() const = 0;
    virtual double GetElectionWarningScore() const = 0;
    virtual const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const = 0;
    virtual const FWorldCrisisStatus& GetWorldCrisisStatus() const = 0;
    virtual const FPoliticalDemandNotice& GetPoliticalDemandNotice() const = 0;
    virtual bool IsSimulationPaused() const = 0;
    virtual int GetSimulationSpeedMultiplier() const = 0;
    virtual void ToggleSimulationPaused() = 0;
    virtual void CycleSimulationSpeedMultiplier() = 0;
};

class IMainWorldAlmanacAccess
{
public:
    virtual ~IMainWorldAlmanacAccess() = default;

    virtual long long GetNationalBudget() const = 0;
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
};

class IMainWorldCitizenPolicyAccess
{
public:
    virtual ~IMainWorldCitizenPolicyAccess() = default;

    virtual const FGovernmentProfile& GetGovernmentProfile() const = 0;
    virtual const FGovernmentEdictModifiers& GetEdictModifiers() const = 0;
    virtual const FTaxPolicy& GetTaxPolicy() const = 0;
    virtual const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const = 0;
    virtual const FWorldCrisisStatus& GetWorldCrisisStatus() const = 0;
};

class IMainWorldRoadNetworkAccess
{
public:
    virtual ~IMainWorldRoadNetworkAccess() = default;

    virtual void RebuildRoadNetwork() = 0;
    virtual const CRoadNetwork* GetRoadNetwork() const = 0;
};

class IMainWorldTransitAccess
{
public:
    virtual ~IMainWorldTransitAccess() = default;

    virtual const CBusRouteSystem* GetBusRouteSystem() const = 0;
};

class IMainWorldRuntimeRefreshAccess
{
public:
    virtual ~IMainWorldRuntimeRefreshAccess() = default;

    virtual void RefreshRuntimeBuildingState() = 0;
};

class IMainWorldTradeAccess
{
public:
    virtual ~IMainWorldTradeAccess() = default;

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
