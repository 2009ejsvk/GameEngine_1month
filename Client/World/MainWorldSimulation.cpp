#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "../Politics/PoliticsSystem.h"
#include <algorithm>

void CMainWorld::Update(float DeltaTime)
{
    CWorld::Update(DeltaTime);

    if (mElectionStatus.GameLost)
        return;

    AdvanceSimulationDate(DeltaTime);
    TickPoliticalRefresh(DeltaTime);
    TickCitizenPopulation(DeltaTime);
}

void CMainWorld::TickPoliticalRefresh(float DeltaTime)
{
    mPoliticalSnapshotAccum += DeltaTime;

    if (mPoliticalSnapshotAccum >= MainWorldConfig::GPoliticalSnapshotInterval)
    {
        mPoliticalSnapshotAccum = 0.f;
        RefreshPoliticalSnapshot();
    }
}

int CMainWorld::GetSimulationMonthDayCount() const
{
    return GetDaysInMonth(mSimulationYear, mSimulationMonth);
}

float CMainWorld::GetSimulationDayProgress() const
{
    if (mSecondsPerSimulationDay <= 0.f)
        return 0.f;

    return Clamp<float>(
        mDayProgressAccum / mSecondsPerSimulationDay, 0.f, 1.f);
}

float CMainWorld::GetSimulationMonthProgress() const
{
    const int MonthDays = GetDaysInMonth(mSimulationYear, mSimulationMonth);

    if (MonthDays <= 0)
        return 0.f;

    const float DayProgress = GetSimulationDayProgress();
    const float CompletedDays =
        static_cast<float>((std::max)(0, mSimulationDay - 1)) +
        DayProgress;

    return Clamp<float>(
        CompletedDays / static_cast<float>(MonthDays), 0.f, 1.f);
}

void CMainWorld::AdvanceSimulationDate(float DeltaTime)
{
    if (DeltaTime <= 0.f || mSecondsPerSimulationDay <= 0.f)
        return;

    mDayProgressAccum += DeltaTime;

    while (mDayProgressAccum >= mSecondsPerSimulationDay)
    {
        mDayProgressAccum -= mSecondsPerSimulationDay;
        AdvanceSimulationDay();
    }
}

void CMainWorld::AdvanceSimulationDay()
{
    ApplyDailyEconomySettlement();
    ApplyDailyEdictCitizenEffects();
    ApplyDailyTaxPolicyEventEffects();
    TickGovernmentEdicts();
    PoliticsSystem::TickGovernmentActions(mGovernmentProfile);
    RefreshEdictModifiers();
    RefreshPoliticalSnapshot();
    TickTaxPolicyEvents();

    ++mSimulationDay;
    const int CurrentMonthDays =
        GetDaysInMonth(mSimulationYear, mSimulationMonth);

    if (mSimulationDay > CurrentMonthDays)
    {
        mSimulationDay = 1;
        ++mSimulationMonth;

        if (mSimulationMonth > 12)
        {
            mSimulationMonth = 1;
            ++mSimulationYear;
        }
    }

    ResolveScheduledElection();
}

int CMainWorld::GetDaysInMonth(int Year, int Month) const
{
    switch (Month)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return 31;
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    case 2:
    {
        const bool IsLeapYear =
            (Year % 400 == 0) || (Year % 4 == 0 && Year % 100 != 0);
        return IsLeapYear ? 29 : 28;
    }
    default:
        return 30;
    }
}
