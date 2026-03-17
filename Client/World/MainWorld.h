#pragma once

#include "GovernmentCommandService.h"
#include "KnowledgeSystem.h"
#include "MainWorldAccess.h"
#include "MainWorldElectionService.h"
#include "MainWorldPoliticalDemandService.h"
#include "ScenarioRunner.h"
#include "MainWorldTradeDiplomacyState.h"
#include "MainWorldWorldCrisisService.h"
#include "../Politics/ConstitutionTypes.h"
#include "World/World.h"
#include <memory>
#include <string>
#include <vector>

class CRoadNetwork;
class CBusRouteSystem;
class CMainWorldUiReadAccess;
class CMainWorldSystemAccess;
class CMainWorldInfrastructureReadAccess;
class CMainWorldBuildingControlAccess;
class CMainWorldTradeAccessGroup;

struct FMainWorldPopulationRuntime
{
    int SpawnedNpcCount = 0;
    float NpcSpawnAccum = 0.f;
    float CitizenReassignAccum = 0.f;
};

struct FMainWorldBudgetRuntime
{
    long long NationalBudget = 0;
    long long LastDailyWageCost = 0;
    long long LastDailyUpkeepCost = 0;
    long long LastDailyExportIncome = 0;
    long long LastDailyTaxIncome = 0;
    long long LastDailyConsumptionTaxIncome = 0;
    long long LastDailyIncomeTaxIncome = 0;
    long long LastDailyPropertyTaxIncome = 0;
    long long LastDailyEdictCost = 0;
    long long LastDailyImportExpense = 0;
    long long LastDailyNetChange = 0;
    double LastDailyTaxCollectionEfficiency = 0.0;
};

struct FMainWorldSimulationRuntime
{
    int Year = 2000;
    int Month = 1;
    int Day = 1;
    float DayProgressAccum = 0.f;
    float SecondsPerSimulationDay = 2.f;
    bool Paused = false;
    int SpeedMultiplier = 1;
    float PoliticalSnapshotAccum = 0.f;
};

struct FMainWorldResultRuntime
{
    int TermStartYear = 0;
    int TermStartMonth = 1;
    int TermStartDay = 1;
    int InitialBuildingCount = 0;
    double PeakSupportPercent = 0.0;
    bool ResultShown = false;
};

struct FMainWorldPolicyRuntime
{
    int WorkerTaxPressureDays = 0;
    int PropertyTaxPressureDays = 0;
    int BudgetCrisisPressureDays = 0;
    FGovernmentProfile GovernmentProfile;
    FPoliticalWorldSnapshot PoliticalSnapshot;
    FTaxPolicyEventStatus TaxEventStatus;
    FEraProgressState EraProgress;
    FEraTransitionState EraTransition;
    std::vector<FGovernmentEdictState> GovernmentEdicts;
    FGovernmentEdictModifiers EdictModifiers;
    FKnowledgeState KnowledgeState;
    FConstitutionState ConstitutionState;
};

struct FMainWorldInfrastructureState
{
    std::shared_ptr<CRoadNetwork> RoadNetwork;
    std::shared_ptr<CBusRouteSystem> BusRouteSystem;
};

struct FMainWorldServiceSet
{
    std::unique_ptr<CMainWorldElectionService> ElectionService;
    std::unique_ptr<CMainWorldPoliticalDemandService> PoliticalDemandService;
    std::unique_ptr<CMainWorldWorldCrisisService> WorldCrisisService;
};

struct FMainWorldAccessSet
{
    std::shared_ptr<CMainWorldUiReadAccess> UiRead;
    std::shared_ptr<CMainWorldSystemAccess> SystemRead;
    std::shared_ptr<CMainWorldInfrastructureReadAccess> InfrastructureRead;
    std::shared_ptr<CMainWorldBuildingControlAccess> BuildingControl;
    std::shared_ptr<CMainWorldTradeAccessGroup> Trade;
    std::shared_ptr<IGovernmentCommandService> GovernmentCommand;
};

class CMainWorld : public CWorld
{
public:
	CMainWorld();
	virtual ~CMainWorld();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
    virtual void OnUiManagerUpdated() override;
    std::shared_ptr<IMainWorldBuildMenuAccess> GetBuildMenuAccessHandle() const;
    IMainWorldBuildMenuAccess* GetBuildMenuAccessRaw() const;
    std::shared_ptr<IMainWorldHudAccess> GetHudAccessHandle() const;
    IMainWorldHudAccess* GetHudAccessRaw() const;
    std::shared_ptr<IMainWorldAlmanacAccess> GetAlmanacAccessHandle() const;
    IMainWorldAlmanacAccess* GetAlmanacAccessRaw() const;
    std::shared_ptr<IMainWorldEdictReadAccess> GetEdictReadAccessHandle() const;
    IMainWorldEdictReadAccess* GetEdictReadAccessRaw() const;
    std::shared_ptr<IGovernmentCommandService> GetGovernmentCommandServiceHandle() const;
    IGovernmentCommandService* GetGovernmentCommandServiceRaw() const;
    std::shared_ptr<IMainWorldCitizenPolicyAccess> GetCitizenPolicyAccessHandle() const;
    IMainWorldCitizenPolicyAccess* GetCitizenPolicyAccessRaw() const;
    std::shared_ptr<IMainWorldRoadNetworkAccess> GetRoadNetworkAccessHandle() const;
    IMainWorldRoadNetworkAccess* GetRoadNetworkAccessRaw() const;
    std::shared_ptr<IMainWorldTransitAccess> GetTransitAccessHandle() const;
    IMainWorldTransitAccess* GetTransitAccessRaw() const;
    std::shared_ptr<IMainWorldRuntimeRefreshAccess> GetRuntimeRefreshAccessHandle() const;
    IMainWorldRuntimeRefreshAccess* GetRuntimeRefreshAccessRaw() const;
    std::shared_ptr<IMainWorldBuildingConditionAccess> GetBuildingConditionAccessHandle() const;
    IMainWorldBuildingConditionAccess* GetBuildingConditionAccessRaw() const;
    std::shared_ptr<IMainWorldTradeAccess> GetTradeAccessHandle() const;
    IMainWorldTradeAccess* GetTradeAccessRaw() const;
    std::shared_ptr<IMainWorldKnowledgeAccess> GetKnowledgeAccessHandle() const;
    IMainWorldKnowledgeAccess* GetKnowledgeAccessRaw() const;
    std::shared_ptr<IMainWorldConstitutionAccess> GetConstitutionAccessHandle() const;
    IMainWorldConstitutionAccess* GetConstitutionAccessRaw() const;
	long long GetNationalBudget() const
	{
		return mBudget.NationalBudget;
	}
	int GetSimulationYear() const
	{
		return mSimulation.Year;
	}
	int GetSimulationMonth() const
	{
		return mSimulation.Month;
	}
	int GetSimulationDay() const
	{
		return mSimulation.Day;
	}
	int GetSimulationMonthDayCount() const;
	float GetSimulationDayProgress() const;
	float GetSimulationMonthProgress() const;
    bool IsSimulationPaused() const
    {
        return mSimulation.Paused;
    }
    int GetSimulationSpeedMultiplier() const
    {
        return mSimulation.SpeedMultiplier;
    }
    void ToggleSimulationPaused();
    void CycleSimulationSpeedMultiplier();
    bool TryExecuteEraTransition(EEraTransitionChoice Choice);
    EBuildingEra GetCurrentEra() const
    {
        return mPolicy.EraProgress.CurrentEra;
    }
    const FEraProgressState& GetEraProgress() const
    {
        return mPolicy.EraProgress;
    }
    const FEraTransitionState& GetEraTransitionState() const
    {
        return mPolicy.EraTransition;
    }
	bool TryApplyEdict(
		EGovernmentEdictType Type,
		std::wstring& OutMessage);
	bool AdjustTaxPolicy(
		ETaxPolicyType Type,
		int DeltaPercent,
		std::wstring& OutMessage);
	bool CycleDomesticReservePolicy(
		std::wstring& OutMessage);
	bool CycleImportPerResourceCap(
		std::wstring& OutMessage);
	bool CycleImportBudgetPolicy(
		std::wstring& OutMessage);
	bool CycleExportBlockedResource(
		std::wstring& OutMessage);
	bool CycleAutoImportResource(
		std::wstring& OutMessage);
    bool ExecuteTradeProposal(
        bool ImportRoute,
        EResourceType ResourceType,
        int ForeignPowerIndex,
        int PricePerThousandUnits,
        int Amount,
        std::wstring& OutMessage);
    bool CancelTradeRoute(
        int RouteId,
        std::wstring& OutMessage);
    bool RespondPoliticalDemand(
        EPoliticalDemandIssuerType IssuerType,
        int IssuerIndex,
        bool Accept,
        std::wstring& OutMessage);
	const FGovernmentProfile& GetGovernmentProfile() const
	{
		return mPolicy.GovernmentProfile;
	}
	const FTaxPolicy& GetTaxPolicy() const
	{
		return mPolicy.GovernmentProfile.TaxPolicy;
	}
	const FPoliticalWorldSnapshot& GetPoliticalSnapshot() const
	{
		return mPolicy.PoliticalSnapshot;
	}
	const std::vector<FGovernmentEdictState>& GetGovernmentEdictStates() const
	{
		return mPolicy.GovernmentEdicts;
	}
	const FGovernmentEdictState* GetGovernmentEdictState(
		EGovernmentEdictType Type) const;
	const FGovernmentEdictModifiers& GetEdictModifiers() const
	{
		return mPolicy.EdictModifiers;
	}
	long long GetLastDailyEdictCost() const
	{
		return mBudget.LastDailyEdictCost;
	}
	long long GetLastDailyImportExpense() const
	{
		return mBudget.LastDailyImportExpense;
	}
	long long GetLastDailyExportIncome() const
	{
		return mBudget.LastDailyExportIncome;
	}
	long long GetLastDailyTaxIncome() const
	{
		return mBudget.LastDailyTaxIncome;
	}
	long long GetLastDailyConsumptionTaxIncome() const
	{
		return mBudget.LastDailyConsumptionTaxIncome;
	}
	long long GetLastDailyIncomeTaxIncome() const
	{
		return mBudget.LastDailyIncomeTaxIncome;
	}
	long long GetLastDailyPropertyTaxIncome() const
	{
		return mBudget.LastDailyPropertyTaxIncome;
	}
	double GetLastDailyTaxCollectionEfficiency() const
	{
		return mBudget.LastDailyTaxCollectionEfficiency;
	}
	long long GetLastDailyNetChange() const
	{
		return mBudget.LastDailyNetChange;
	}
	const FElectionStatus& GetElectionStatus() const
	{
		return mServices.ElectionService->GetElectionStatus();
	}
	int GetDaysUntilNextElection() const;
	double GetElectionWarningScore() const;
	const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const
	{
		return mPolicy.TaxEventStatus;
	}
    const FWorldCrisisStatus& GetWorldCrisisStatus() const
    {
        return mServices.WorldCrisisService->GetStatus();
    }
    const FPoliticalDemandNotice& GetPoliticalDemandNotice() const
    {
        return mServices.PoliticalDemandService->GetPoliticalDemandNotice();
    }
    const std::array<int, GPoliticalFactionCount>&
        GetFactionDemandPressureDays() const
    {
        return mServices.PoliticalDemandService->GetFactionPressureDays();
    }
    const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
        GetFactionDemandStates() const
    {
        return mServices.PoliticalDemandService->GetFactionDemandStates();
    }
    const std::array<
        FPoliticalDemandState,
        TradeDiplomacyRuntime::GForeignPowerCount>&
        GetForeignDemandStates() const
    {
        return mServices.PoliticalDemandService->GetForeignDemandStates();
    }
    void RebuildRoadNetwork();
    const CRoadNetwork* GetRoadNetwork() const
    {
        return mInfrastructure.RoadNetwork.get();
    }
    const CBusRouteSystem* GetBusRouteSystem() const
    {
        return mInfrastructure.BusRouteSystem.get();
    }
    void RefreshRuntimeBuildingState();
    const std::vector<FTradeRouteRuntimeState>&
        GetActiveTradeRoutes() const
    {
        return mTradeDiplomacyState.ActiveTradeRoutes;
    }
    const std::vector<FTradeRouteCompletionRecord>&
        GetCompletedTradeRoutes() const
    {
        return mTradeDiplomacyState.CompletedTradeRoutes;
    }
    int GetTradeRouteCompletionNotificationVersion() const
    {
        return mTradeDiplomacyState.TradeRouteCompletionNotificationVersion;
    }
    int GetCustomsExportTradePriceModifierPercent() const;
    int GetCustomsImportTradePriceModifierPercent() const;
    const std::array<
        TradeDiplomacyRuntime::FForeignPowerWorldState,
        TradeDiplomacyRuntime::GForeignPowerCount>&
        GetForeignPowerStates() const
    {
        return mTradeDiplomacyState.ForeignPowerStates;
    }
    const FKnowledgeState& GetKnowledgeState() const
    {
        return mPolicy.KnowledgeState;
    }
    int GetKnowledgePoints() const
    {
        return mPolicy.KnowledgeState.Points;
    }
    int GetDailyKnowledgeGeneration() const
    {
        return mPolicy.KnowledgeState.DailyGeneration;
    }
    const FConstitutionState& GetConstitutionState() const
    {
        return mPolicy.ConstitutionState;
    }
    bool IsResearchUnlocked(const std::wstring& Key) const
    {
        return ::IsResearchUnlocked(mPolicy.KnowledgeState, Key);
    }
    bool TryUnlockResearch(
        const std::wstring& Key,
        int Cost);
    bool TrySelectConstitutionOption(
        EConstitutionOptionId Id);
    bool DamageBuilding(
        const std::string& BuildingName,
        EBuildingDamageLevel Level);
    bool TryRepairBuilding(
        const std::string& BuildingName,
        std::wstring& OutMessage);

private:
	FMainWorldPopulationRuntime mPopulation;
	FMainWorldBudgetRuntime mBudget;
	FMainWorldSimulationRuntime mSimulation;
    FMainWorldResultRuntime mResultRuntime;
    FMainWorldPolicyRuntime mPolicy;
    FMainWorldInfrastructureState mInfrastructure;
    unsigned long long mLastGameConstantsGeneration = 0;
    unsigned long long mLastEdictConfigGeneration = 0;
    FMainWorldTradeDiplomacyState mTradeDiplomacyState;
    FMainWorldServiceSet mServices;
    FMainWorldAccessSet mAccess;
    CScenarioRunner mScenarioRunner;
    bool mScenarioElectionPromptPending = false;

private:
	void ResetWorldState();
    void InitializeResultTracking();
    CMainWorldPoliticalDemandService::FContext BuildPoliticalDemandContext();
    void ApplyScenarioResult(const FScenarioEvent& ScenarioEvent);
    void ShowResultWidget(bool Victory);
    void ApplyPoliticalDemandRefreshRequests(
        const CMainWorldPoliticalDemandService::FRefreshRequests&
            RefreshRequests);
    void TriggerFactionRevoltConsequences(EPoliticalFaction Faction);
	void TickPoliticalRefresh(float DeltaTime);
	void TickCitizenPopulation(float DeltaTime);
	void SpawnCitizenOrb();
	void ReassignCitizenNeeds();
	void LoadCitizenAnimation2D();
	void LoadAnimation2D();
	void LoadSound();
	void CreateUI();
	void AdvanceSimulationDate(float DeltaTime);
	void AdvanceSimulationDay();
	int GetDaysInMonth(int Year, int Month) const;
	void ApplyDailyEconomySettlement();
    void ProcessActiveTradeRoutes();
    void RecordFinishedTradeRoute(
        const FTradeRouteRuntimeState& Route,
        ETradeRouteEndReason EndReason);
    void CancelTradeRoutesForInactivePowers(EBuildingEra Era);
    void RefreshForeignTradeDiplomacy(bool ApplyIdleDecay);
    void RefreshPowerGridCoverage();
    void RefreshWorldMarketPrices();
    void RefreshBuildingPollutionExposure();
	void InitializeElectionSchedule();
    void TickElectionPromises();
	void ResolveScheduledElection();
	void TickGovernmentEdicts();
	void RefreshEdictModifiers();
	void ApplyDailyEdictCitizenEffects();
	void ApplyDailyTaxPolicyEventEffects();
	void TickTaxPolicyEvents();
    void ApplyDailyWorldCrisisEffects();
    void TickWorldCrises();
    void TickPoliticalDemands();
    void TickEraTransitionState();
    void RefreshEraProgress();
    void RefreshEraTransitionState();
    void RefreshKnowledgeGeneration();
    void ApplyDailyKnowledgeGain();
    void RefreshBuildingRepairCosts();
	void RefreshPoliticalSnapshot();
    void RebuildBusRoutes();
};
