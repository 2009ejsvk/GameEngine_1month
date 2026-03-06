#pragma once

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

private:
	int mSpawnedNpcCount = 0;
	float mNpcSpawnAccum = 0.f;
	float mCitizenReassignAccum = 0.f;
	long long mNationalBudget = 0;
	long long mLastDailyWageCost = 0;
	long long mLastDailyUpkeepCost = 0;
	long long mLastDailyExportIncome = 0;
	long long mLastDailyNetChange = 0;
	int mSimulationYear = 2000;
	int mSimulationMonth = 1;
	int mSimulationDay = 1;
	float mDayProgressAccum = 0.f;
	float mSecondsPerSimulationDay = 2.f;

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
};
