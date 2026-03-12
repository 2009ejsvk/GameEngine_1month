#pragma once

#include "GovernmentCommandService.h"
#include "MainWorldAccess.h"
#include "World/World.h"
#include <string>
#include <vector>

class CRoadNetwork;
class CBusRouteSystem;

class CMainWorld :
    public CWorld,
    public IMainWorldBuildMenuAccess,
    public IMainWorldHudAccess,
    public IMainWorldAlmanacAccess,
    public IMainWorldEdictReadAccess,
    public IGovernmentCommandService,
    public IMainWorldCitizenPolicyAccess,
    public IMainWorldRoadNetworkAccess,
    public IMainWorldTransitAccess,
    public IMainWorldRuntimeRefreshAccess,
    public IMainWorldTradeAccess
{
public:
	CMainWorld();
	virtual ~CMainWorld();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	long long GetNationalBudget() const
	{
		return mNationalBudget;
	}
	int GetSimulationYear() const
	{
		return mSimulationYear;
	}
	int GetSimulationMonth() const
	{
		return mSimulationMonth;
	}
	int GetSimulationDay() const
	{
		return mSimulationDay;
	}
	int GetSimulationMonthDayCount() const;
	float GetSimulationDayProgress() const;
	float GetSimulationMonthProgress() const;
    bool IsSimulationPaused() const
    {
        return mSimulationPaused;
    }
    int GetSimulationSpeedMultiplier() const
    {
        return mSimulationSpeedMultiplier;
    }
    virtual void CycleSimulationSpeedState() override;
    EBuildingEra GetCurrentEra() const
    {
        return mEraProgress.CurrentEra;
    }
    const FEraProgressState& GetEraProgress() const
    {
        return mEraProgress;
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
	const FGovernmentProfile& GetGovernmentProfile() const
	{
		return mGovernmentProfile;
	}
	const FTaxPolicy& GetTaxPolicy() const
	{
		return mGovernmentProfile.TaxPolicy;
	}
	const FPoliticalWorldSnapshot& GetPoliticalSnapshot() const
	{
		return mPoliticalSnapshot;
	}
	const std::vector<FGovernmentEdictState>& GetGovernmentEdictStates() const
	{
		return mGovernmentEdicts;
	}
	const FGovernmentEdictState* GetGovernmentEdictState(
		EGovernmentEdictType Type) const;
	const FGovernmentEdictModifiers& GetEdictModifiers() const
	{
		return mEdictModifiers;
	}
	long long GetLastDailyEdictCost() const
	{
		return mLastDailyEdictCost;
	}
	long long GetLastDailyImportExpense() const
	{
		return mLastDailyImportExpense;
	}
	long long GetLastDailyExportIncome() const
	{
		return mLastDailyExportIncome;
	}
	long long GetLastDailyTaxIncome() const
	{
		return mLastDailyTaxIncome;
	}
	long long GetLastDailyConsumptionTaxIncome() const
	{
		return mLastDailyConsumptionTaxIncome;
	}
	long long GetLastDailyIncomeTaxIncome() const
	{
		return mLastDailyIncomeTaxIncome;
	}
	long long GetLastDailyPropertyTaxIncome() const
	{
		return mLastDailyPropertyTaxIncome;
	}
	double GetLastDailyTaxCollectionEfficiency() const
	{
		return mLastDailyTaxCollectionEfficiency;
	}
	long long GetLastDailyNetChange() const
	{
		return mLastDailyNetChange;
	}
	const FElectionStatus& GetElectionStatus() const
	{
		return mElectionStatus;
	}
	int GetDaysUntilNextElection() const;
	double GetElectionWarningScore() const;
	const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const
	{
		return mTaxEventStatus;
	}
    const FWorldCrisisStatus& GetWorldCrisisStatus() const
    {
        return mWorldCrisisStatus;
    }
    virtual void RebuildRoadNetwork() override;
    virtual const CRoadNetwork* GetRoadNetwork() const override
    {
        return mRoadNetwork.get();
    }
    virtual const CBusRouteSystem* GetBusRouteSystem() const override
    {
        return mBusRouteSystem.get();
    }
    virtual void RefreshRuntimeBuildingState() override;
    virtual const std::vector<FTradeRouteRuntimeState>&
        GetActiveTradeRoutes() const override
    {
        return mActiveTradeRoutes;
    }
    virtual const std::vector<FTradeRouteCompletionRecord>&
        GetCompletedTradeRoutes() const override
    {
        return mCompletedTradeRoutes;
    }
    virtual int GetTradeRouteCompletionNotificationVersion() const override
    {
        return mTradeRouteCompletionNotificationVersion;
    }

private:
	int mSpawnedNpcCount = 0;
	float mNpcSpawnAccum = 0.f;
	float mCitizenReassignAccum = 0.f;
	long long mNationalBudget = 0;
	long long mLastDailyWageCost = 0;
	long long mLastDailyUpkeepCost = 0;
	long long mLastDailyExportIncome = 0;
	long long mLastDailyTaxIncome = 0;
	long long mLastDailyConsumptionTaxIncome = 0;
	long long mLastDailyIncomeTaxIncome = 0;
	long long mLastDailyPropertyTaxIncome = 0;
	long long mLastDailyEdictCost = 0;
	long long mLastDailyImportExpense = 0;
	long long mLastDailyNetChange = 0;
	double mLastDailyTaxCollectionEfficiency = 0.0;
	int mSimulationYear = 2000;
	int mSimulationMonth = 1;
	int mSimulationDay = 1;
	float mDayProgressAccum = 0.f;
	float mSecondsPerSimulationDay = 2.f;
    bool mSimulationPaused = false;
    int mSimulationSpeedMultiplier = 1;
	float mPoliticalSnapshotAccum = 0.f;
    unsigned long long mLastGameConstantsGeneration = 0;
    unsigned long long mLastEdictConfigGeneration = 0;
	int mWorkerTaxPressureDays = 0;
	int mPropertyTaxPressureDays = 0;
	int mBudgetCrisisPressureDays = 0;
    int mRaidPressureDays = 0;
    int mLaborStrikePressureDays = 0;
    int mCrimeWavePressureDays = 0;
    int mFiscalEmergencyPressureDays = 0;
	FGovernmentProfile mGovernmentProfile;
	FPoliticalWorldSnapshot mPoliticalSnapshot;
	FElectionStatus mElectionStatus;
	FTaxPolicyEventStatus mTaxEventStatus;
    FWorldCrisisStatus mWorldCrisisStatus;
    FEraProgressState mEraProgress;
	std::vector<FGovernmentEdictState> mGovernmentEdicts;
	FGovernmentEdictModifiers mEdictModifiers;
    std::shared_ptr<CRoadNetwork> mRoadNetwork;
    std::shared_ptr<CBusRouteSystem> mBusRouteSystem;
    std::vector<FTradeRouteRuntimeState> mActiveTradeRoutes;
    std::vector<FTradeRouteCompletionRecord> mCompletedTradeRoutes;
    int mNextTradeRouteId = 1;
    int mNextTradeRouteCompletionRecordId = 1;
    int mTradeRouteCompletionNotificationVersion = 0;

private:
	void ResetWorldState();
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
    void RefreshPowerGridCoverage();
    void RefreshWorldMarketPrices();
    void RefreshBuildingPollutionExposure();
	void InitializeElectionSchedule();
	void ResolveScheduledElection();
	void TickGovernmentEdicts();
	void RefreshEdictModifiers();
	void ApplyDailyEdictCitizenEffects();
	void ApplyDailyTaxPolicyEventEffects();
	void TickTaxPolicyEvents();
    void ApplyDailyWorldCrisisEffects();
    void TickWorldCrises();
    void RefreshEraProgress();
	void RefreshPoliticalSnapshot();
    void RebuildBusRoutes();
};
