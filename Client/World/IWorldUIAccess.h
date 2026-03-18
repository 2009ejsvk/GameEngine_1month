#pragma once

#include "GovernmentCommandService.h"
#include "MainWorldBuildingControlAccess.h"
#include "MainWorldInfrastructureAccess.h"
#include "MainWorldSystemAccess.h"
#include "MainWorldTradeAccess.h"
#include "MainWorldUiReadAccess.h"
#include <memory>

class CMainWorld;
class CWorld;

class IWorldUIReadAccess
{
public:
    virtual ~IWorldUIReadAccess() = default;

    virtual long long GetNationalBudget() const = 0;
    virtual int GetSimulationYear() const = 0;
    virtual int GetSimulationMonth() const = 0;
    virtual int GetSimulationDay() const = 0;
    virtual int GetSimulationMonthDayCount() const = 0;
    virtual float GetSimulationDayProgress() const = 0;
    virtual float GetSimulationMonthProgress() const = 0;
    virtual bool IsSimulationPaused() const = 0;
    virtual int GetSimulationSpeedMultiplier() const = 0;
    virtual EBuildingEra GetCurrentEra() const = 0;
    virtual const FEraProgressState& GetEraProgress() const = 0;
    virtual const FEraTransitionState& GetEraTransitionState() const = 0;
    virtual const FGovernmentProfile& GetGovernmentProfile() const = 0;
    virtual const FGovernmentEdictModifiers& GetEdictModifiers() const = 0;
    virtual const FTaxPolicy& GetTaxPolicy() const = 0;
    virtual const FPoliticalWorldSnapshot& GetPoliticalSnapshot() const = 0;
    virtual const FElectionStatus& GetElectionStatus() const = 0;
    virtual int GetDaysUntilNextElection() const = 0;
    virtual double GetElectionWarningScore() const = 0;
    virtual const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const = 0;
    virtual const FWorldCrisisStatus& GetWorldCrisisStatus() const = 0;
    virtual const FPoliticalDemandNotice& GetPoliticalDemandNotice() const = 0;
    virtual const std::array<int, GPoliticalFactionCount>&
        GetFactionDemandPressureDays() const = 0;
    virtual const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
        GetFactionDemandStates() const = 0;
    virtual const std::array<
        FPoliticalDemandState,
        TradeDiplomacyRuntime::GForeignPowerCount>&
        GetForeignDemandStates() const = 0;
    virtual const std::vector<FGovernmentEdictState>&
        GetGovernmentEdictStates() const = 0;
    virtual const FGovernmentEdictState* GetGovernmentEdictState(
        EGovernmentEdictType Type) const = 0;
    virtual long long GetLastDailyEdictCost() const = 0;
    virtual long long GetLastDailyImportExpense() const = 0;
    virtual long long GetLastDailyExportIncome() const = 0;
    virtual long long GetLastDailyTaxIncome() const = 0;
    virtual long long GetLastDailyConsumptionTaxIncome() const = 0;
    virtual long long GetLastDailyIncomeTaxIncome() const = 0;
    virtual long long GetLastDailyPropertyTaxIncome() const = 0;
    virtual double GetLastDailyTaxCollectionEfficiency() const = 0;
    virtual long long GetLastDailyNetChange() const = 0;
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
    virtual const FKnowledgeState& GetKnowledgeState() const = 0;
    virtual int GetKnowledgePoints() const = 0;
    virtual int GetDailyKnowledgeGeneration() const = 0;
    virtual bool IsResearchUnlocked(const std::wstring& Key) const = 0;
    virtual const FConstitutionState& GetConstitutionState() const = 0;
    virtual const CRoadNetwork* GetRoadNetwork() const = 0;
    virtual const CBusRouteSystem* GetBusRouteSystem() const = 0;
};

class IWorldUICommands
{
public:
    virtual ~IWorldUICommands() = default;

    virtual bool TryApplyEdict(
        EGovernmentEdictType Type,
        std::wstring& OutMessage) = 0;
    virtual bool AdjustTaxPolicy(
        ETaxPolicyType Type,
        int DeltaPercent,
        std::wstring& OutMessage) = 0;
    virtual bool CycleExportBlockedResource(
        std::wstring& OutMessage) = 0;
    virtual bool ExecuteTradeProposal(
        bool ImportRoute,
        EResourceType ResourceType,
        int ForeignPowerIndex,
        int PricePerThousandUnits,
        int Amount,
        std::wstring& OutMessage) = 0;
    virtual bool CancelTradeRoute(
        int RouteId,
        std::wstring& OutMessage) = 0;
    virtual bool RespondPoliticalDemand(
        EPoliticalDemandIssuerType IssuerType,
        int IssuerIndex,
        bool Accept,
        std::wstring& OutMessage) = 0;
    virtual void ToggleSimulationPaused() = 0;
    virtual void CycleSimulationSpeedMultiplier() = 0;
    virtual bool TryExecuteEraTransition(EEraTransitionChoice Choice) = 0;
    virtual bool TryUnlockResearch(
        const std::wstring& Key,
        int Cost) = 0;
    virtual bool TrySelectConstitutionOption(
        EConstitutionOptionId Id) = 0;
    virtual void RebuildRoadNetwork() = 0;
    virtual void RefreshRuntimeBuildingState() = 0;
    virtual bool DamageBuilding(
        const std::string& BuildingName,
        EBuildingDamageLevel Level) = 0;
    virtual bool TryRepairBuilding(
        const std::string& BuildingName,
        std::wstring& OutMessage) = 0;
};

class IWorldUIAccess
{
public:
    virtual ~IWorldUIAccess() = default;

    virtual const IWorldUIReadAccess& Read() const = 0;
    virtual IWorldUICommands& Commands() = 0;
};

std::shared_ptr<IWorldUIAccess> ResolveWorldUIAccess(
    const std::shared_ptr<CWorld>& World);
IWorldUIAccess* ResolveWorldUIAccess(CWorld* World);
