#pragma once

#include "../Politics/PoliticalTypes.h"
#include "World/World.h"
#include <string>
#include <vector>

class CMainWorld :
    public CWorld
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
	bool TryApplyEdict(
		EGovernmentEdictType Type,
		std::wstring& OutMessage);
	const FGovernmentProfile& GetGovernmentProfile() const
	{
		return mGovernmentProfile;
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

private:
	int mSpawnedNpcCount = 0;
	float mNpcSpawnAccum = 0.f;
	float mCitizenReassignAccum = 0.f;
	long long mNationalBudget = 0;
	long long mLastDailyWageCost = 0;
	long long mLastDailyUpkeepCost = 0;
	long long mLastDailyExportIncome = 0;
	long long mLastDailyEdictCost = 0;
	long long mLastDailyNetChange = 0;
	int mSimulationYear = 2000;
	int mSimulationMonth = 1;
	int mSimulationDay = 1;
	float mDayProgressAccum = 0.f;
	float mSecondsPerSimulationDay = 2.f;
	float mPoliticalSnapshotAccum = 0.f;
	FGovernmentProfile mGovernmentProfile;
	FPoliticalWorldSnapshot mPoliticalSnapshot;
	std::vector<FGovernmentEdictState> mGovernmentEdicts;
	FGovernmentEdictModifiers mEdictModifiers;

private:
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
	void TickGovernmentEdicts();
	void RefreshEdictModifiers();
	void ApplyDailyEdictCitizenEffects();
	void SyncGovernmentActionFromEdict(
		EGovernmentEdictType Type,
		bool Active);
	void RefreshPoliticalSnapshot();
};
