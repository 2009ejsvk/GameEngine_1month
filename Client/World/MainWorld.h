#pragma once

#include "GovernmentCommandService.h"
#include "KnowledgeSystem.h"
#include "BuildingSubsystem.h"
#include "CrisisSubsystem.h"
#include "EconomySubsystem.h"
#include "EdictSubsystem.h"
#include "EraSubsystem.h"
#include "InfrastructureSubsystem.h"
#include "MainWorldAccess.h"
#include "MainWorldElectionService.h"
#include "MainWorldPoliticalDemandService.h"
#include "KnowledgeSubsystem.h"
#include "PoliticsSubsystem.h"
#include "PopulationSubsystem.h"
#include "ScenarioSubsystem.h"
#include "ScenarioRunner.h"
#include "SimulationSubsystem.h"
#include "TradeDiplomacySubsystem.h"
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
class IWorldUIAccess;

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

struct FMainWorldPoliticsState
{
    FGovernmentProfile GovernmentProfile;
    FPoliticalWorldSnapshot PoliticalSnapshot;
};

struct FMainWorldEdictState
{
    std::vector<FGovernmentEdictState> GovernmentEdicts;
    FGovernmentEdictModifiers EdictModifiers;
};

struct FMainWorldEraState
{
    FEraProgressState EraProgress;
    FEraTransitionState EraTransition;
};

struct FMainWorldTaxPolicyState
{
    int WorkerTaxPressureDays = 0;
    int PropertyTaxPressureDays = 0;
    int BudgetCrisisPressureDays = 0;
    FTaxPolicyEventStatus TaxEventStatus;
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
    std::shared_ptr<IWorldUIAccess> UIFacade;
};

class CMainWorld : public CWorld
{
    friend class CSimulationSubsystem;
    friend class CEraSubsystem;
    friend class CEdictSubsystem;
    friend class CKnowledgeSubsystem;
    friend class CCrisisSubsystem;
    friend class CTradeDiplomacySubsystem;
    friend class CEconomySubsystem;
    friend class CPoliticsSubsystem;
    friend class CPopulationSubsystem;
    friend class CInfrastructureSubsystem;
    friend class CBuildingSubsystem;
    friend class CScenarioSubsystem;

public:
	CMainWorld();
	virtual ~CMainWorld();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
    virtual void PostUpdate(float DeltaTime) override;
    virtual void OnUiManagerUpdated() override;
    virtual bool IsSimulationPaused() const override
    {
        return mSimulation && mSimulation->Paused;
    }
    virtual int GetSimulationSpeedMultiplier() const override
    {
        return mSimulation ? mSimulation->SpeedMultiplier : 1;
    }
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
    std::shared_ptr<IWorldUIAccess> GetUIAccessHandle() const;
    IWorldUIAccess* GetUIAccessRaw() const;
    CSimulationSubsystem* GetSimulation() const
    {
        return mSimulation.get();
    }
    CEconomySubsystem* GetEconomy() const
    {
        return mEconomy.get();
    }
    CTradeDiplomacySubsystem* GetTrade() const
    {
        return mTrade.get();
    }
    CPoliticsSubsystem* GetPolitics() const
    {
        return mPolitics.get();
    }
    CEdictSubsystem* GetEdicts() const
    {
        return mEdictState.get();
    }
    CEraSubsystem* GetEra() const
    {
        return mEraState.get();
    }
    CKnowledgeSubsystem* GetKnowledge() const
    {
        return mKnowledgeState.get();
    }
    CCrisisSubsystem* GetCrisis() const
    {
        return mCrisis.get();
    }
    CPopulationSubsystem* GetPopulation() const
    {
        return mPopulation.get();
    }
    CInfrastructureSubsystem* GetInfrastructure() const
    {
        return mInfrastructure.get();
    }
    CBuildingSubsystem* GetBuildings() const
    {
        return mBuildings.get();
    }
    CScenarioSubsystem* GetScenario() const
    {
        return mScenario.get();
    }

    // Legacy wrapper getters remain for existing UI callers.
	long long GetNationalBudget() const
	{
		return mEconomy->NationalBudget;
	}
	void SpendBuildingCost(int BaseCost);
	int GetSimulationYear() const
	{
		return mSimulation->Year;
	}
	int GetSimulationMonth() const
	{
		return mSimulation->Month;
	}
	int GetSimulationDay() const
	{
		return mSimulation->Day;
	}
	int GetSimulationMonthDayCount() const
	{
		return mSimulation->GetDaysInMonth(
            mSimulation->Year,
            mSimulation->Month);
	}
	float GetSimulationDayProgress() const
	{
		return mSimulation->GetDayProgress();
	}
	float GetSimulationMonthProgress() const
	{
		return mSimulation->GetMonthProgress();
	}
    void ToggleSimulationPaused()
    {
        mSimulation->TogglePaused();
    }
    void CycleSimulationSpeedMultiplier()
    {
        mSimulation->CycleSpeedMultiplier();
    }
    bool TryExecuteEraTransition(EEraTransitionChoice Choice);
    bool TryExecutePeacePayment(std::wstring& OutMessage);
    EBuildingEra GetCurrentEra() const
    {
        return mEraState->EraProgress.CurrentEra;
    }
    const FEraProgressState& GetEraProgress() const
    {
        return mEraState->EraProgress;
    }
    const FEraTransitionState& GetEraTransitionState() const
    {
        return mEraState->EraTransition;
    }
	bool TryApplyEdict(
		EGovernmentEdictType Type,
		std::wstring& OutMessage);
	bool AdjustTaxPolicy(
		ETaxPolicyType Type,
		int DeltaPercent,
		std::wstring& OutMessage);
	bool CycleExportBlockedResource(
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
		return mPolitics->GovernmentProfile;
	}
	const FTaxPolicy& GetTaxPolicy() const
	{
		return mPolitics->GovernmentProfile.TaxPolicy;
	}
	const FPoliticalWorldSnapshot& GetPoliticalSnapshot() const
	{
		return mPolitics->PoliticalSnapshot;
	}
	const std::vector<FGovernmentEdictState>& GetGovernmentEdictStates() const
	{
		return mEdictState->GovernmentEdicts;
	}
	const FGovernmentEdictState* GetGovernmentEdictState(
		EGovernmentEdictType Type) const;
	const FGovernmentEdictModifiers& GetEdictModifiers() const
	{
		return mEdictState->EdictModifiers;
	}
	long long GetLastDailyEdictCost() const
	{
		return mEconomy->LastDailyEdictCost;
	}
	long long GetLastDailyImportExpense() const
	{
		return mEconomy->LastDailyImportExpense;
	}
	long long GetLastDailyExportIncome() const
	{
		return mEconomy->LastDailyExportIncome;
	}
	long long GetLastDailyTaxIncome() const
	{
		return mEconomy->LastDailyTaxIncome;
	}
	long long GetLastDailyConsumptionTaxIncome() const
	{
		return mEconomy->LastDailyConsumptionTaxIncome;
	}
	long long GetLastDailyIncomeTaxIncome() const
	{
		return mEconomy->LastDailyIncomeTaxIncome;
	}
	long long GetLastDailyPropertyTaxIncome() const
	{
		return mEconomy->LastDailyPropertyTaxIncome;
	}
	double GetLastDailyTaxCollectionEfficiency() const
	{
		return mEconomy->LastDailyTaxCollectionEfficiency;
	}
	long long GetLastDailyNetChange() const
	{
		return mEconomy->LastDailyNetChange;
	}
    const FEconomyLedgerHistorySnapshot& GetEconomyLedgerHistory() const
    {
        return mEconomy->GetLedgerHistory();
    }
	const FElectionStatus& GetElectionStatus() const
	{
		return mPolitics->ElectionService->GetElectionStatus();
	}
	int GetDaysUntilNextElection() const;
	double GetElectionWarningScore() const;
	const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const
	{
		return mEconomy->TaxEventStatus;
	}
    const FWorldCrisisStatus& GetWorldCrisisStatus() const
    {
        return mCrisis->WorldCrisisService->GetStatus();
    }
    const FPoliticalDemandNotice& GetPoliticalDemandNotice() const
    {
        return mPolitics->PoliticalDemandService->GetPoliticalDemandNotice();
    }
    const std::array<int, GPoliticalFactionCount>&
        GetFactionDemandPressureDays() const
    {
        return mPolitics->PoliticalDemandService->GetFactionPressureDays();
    }
    const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
        GetFactionDemandStates() const
    {
        return mPolitics->PoliticalDemandService->GetFactionDemandStates();
    }
    const std::array<
        FPoliticalDemandState,
        TradeDiplomacyRuntime::GForeignPowerCount>&
        GetForeignDemandStates() const
    {
        return mPolitics->PoliticalDemandService->GetForeignDemandStates();
    }
    void RebuildRoadNetwork();
    const CRoadNetwork* GetRoadNetwork() const
    {
        return mInfrastructure->RoadNetwork.get();
    }
    const CBusRouteSystem* GetBusRouteSystem() const
    {
        return mInfrastructure->BusRouteSystem.get();
    }
    void RefreshRuntimeBuildingState();
    const std::vector<FTradeRouteRuntimeState>&
        GetActiveTradeRoutes() const
    {
        return mTrade->State.ActiveTradeRoutes;
    }
    const std::vector<FTradeRouteCompletionRecord>&
        GetCompletedTradeRoutes() const
    {
        return mTrade->State.CompletedTradeRoutes;
    }
    const std::vector<FTradeOfferRuntimeState>&
        GetAvailableTradeOffers() const
    {
        return mTrade->State.AvailableTradeOffers;
    }
    int GetTradeRouteCompletionNotificationVersion() const
    {
        return mTrade->State.TradeRouteCompletionNotificationVersion;
    }
    int GetCustomsExportTradePriceModifierPercent() const;
    int GetCustomsImportTradePriceModifierPercent() const;
    const std::array<
        TradeDiplomacyRuntime::FForeignPowerWorldState,
        TradeDiplomacyRuntime::GForeignPowerCount>&
        GetForeignPowerStates() const
    {
        return mTrade->State.ForeignPowerStates;
    }
    const FKnowledgeState& GetKnowledgeState() const
    {
        return mKnowledgeState->KnowledgeState;
    }
    int GetKnowledgePoints() const
    {
        return mKnowledgeState->KnowledgeState.Points;
    }
    int GetDailyKnowledgeGeneration() const
    {
        return mKnowledgeState->KnowledgeState.DailyGeneration;
    }
    const FConstitutionState& GetConstitutionState() const
    {
        return mKnowledgeState->ConstitutionState;
    }
    bool IsResearchUnlocked(const std::wstring& Key) const
    {
        return ::IsResearchUnlocked(mKnowledgeState->KnowledgeState, Key);
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
	std::unique_ptr<CPopulationSubsystem> mPopulation;
	std::unique_ptr<CEconomySubsystem> mEconomy;
	std::unique_ptr<CSimulationSubsystem> mSimulation;
    std::unique_ptr<CScenarioSubsystem> mScenario;
    std::unique_ptr<CPoliticsSubsystem> mPolitics;
    std::unique_ptr<CEdictSubsystem> mEdictState;
    std::unique_ptr<CEraSubsystem> mEraState;
    std::unique_ptr<CCrisisSubsystem> mCrisis;
    std::unique_ptr<CKnowledgeSubsystem> mKnowledgeState;
    std::unique_ptr<CInfrastructureSubsystem> mInfrastructure;
    std::unique_ptr<CBuildingSubsystem> mBuildings;
    unsigned long long mLastGameConstantsGeneration = 0;
    unsigned long long mLastEdictConfigGeneration = 0;
    std::unique_ptr<CTradeDiplomacySubsystem> mTrade;
    FMainWorldAccessSet mAccess;

private:
	void ResetWorldState();
#ifdef _DEBUG
    void RunDebugConstitutionValidationIfRequested();
#endif
	void TickPoliticalRefresh(float DeltaTime);
	void CreateUI();
	void AdvanceSimulationDay();
};
