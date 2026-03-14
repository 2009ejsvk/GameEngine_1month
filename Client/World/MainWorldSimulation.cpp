#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "RuntimeConfigRegistry.h"
#include "../UI/UILayoutConfig.h"
#include "../GameConstants.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Politics/EdictSystem.h"
#include "../Politics/PoliticsSystem.h"
#include <algorithm>

void CMainWorld::ToggleSimulationPaused()
{
    mSimulationPaused = !mSimulationPaused;
}

void CMainWorld::CycleSimulationSpeedMultiplier()
{
    if (mSimulationSpeedMultiplier >= 4)
    {
        mSimulationSpeedMultiplier = 1;
        return;
    }

    if (mSimulationSpeedMultiplier >= 2)
    {
        mSimulationSpeedMultiplier = 4;
        return;
    }

    mSimulationSpeedMultiplier = 2;
}

void CMainWorld::OnUiManagerUpdated()
{
    UIConfig::ApplyWidgetOverrides(GetUIManager().lock());
}

void CMainWorld::Update(float DeltaTime)
{
    RuntimeConfigRegistry::PollAll(DeltaTime);

    const unsigned long long GameConstantsGeneration =
        GameConstants::GetRuntimeConfigGeneration();

    if (GameConstantsGeneration != mLastGameConstantsGeneration)
    {
        mLastGameConstantsGeneration = GameConstantsGeneration;
        RebuildRoadNetwork();
        RefreshRuntimeBuildingState();
        RefreshPoliticalSnapshot();
        RefreshForeignTradeDiplomacy(false);
        RefreshWorldMarketPrices();

        std::vector<std::weak_ptr<CBuildingMarkerOrb>> CitizenList;

        if (FindObjectListByType<CBuildingMarkerOrb>(CitizenList))
        {
            for (size_t i = 0; i < CitizenList.size(); ++i)
            {
                auto Citizen = CitizenList[i].lock();

                if (Citizen && Citizen->GetAlive() && Citizen->GetEnable())
                    Citizen->RefreshMoveSpeedFromGameConstants();
            }
        }
    }

    const unsigned long long EdictConfigGeneration =
        EdictSystem::GetRuntimeConfigGeneration();

    if (EdictConfigGeneration != mLastEdictConfigGeneration)
    {
        mLastEdictConfigGeneration = EdictConfigGeneration;
        EdictSystem::SynchronizeGovernmentEdictStates(mGovernmentEdicts);
        mGovernmentProfile.ActiveActions.clear();

        for (size_t Index = 0; Index < mGovernmentEdicts.size(); ++Index)
        {
            PoliticsSystem::SyncGovernmentActionFromEdict(
                mGovernmentProfile,
                mGovernmentEdicts[Index].Type,
                mGovernmentEdicts[Index].Active);
        }

        RefreshEdictModifiers();
        RefreshPoliticalSnapshot();
        RefreshForeignTradeDiplomacy(false);
    }

    const float SimulationDeltaTime =
        mSimulationPaused ?
        0.f :
        DeltaTime * static_cast<float>((std::max)(1, mSimulationSpeedMultiplier));

    CWorld::Update(SimulationDeltaTime);

    if (mElectionService->IsGameLost())
        return;

    AdvanceSimulationDate(SimulationDeltaTime);
    TickPoliticalRefresh(SimulationDeltaTime);
    TickCitizenPopulation(SimulationDeltaTime);
}

void CMainWorld::TickPoliticalRefresh(float DeltaTime)
{
    mPoliticalSnapshotAccum += DeltaTime;

    if (mPoliticalSnapshotAccum >= MainWorldConfig::GPoliticalSnapshotInterval)
    {
        mPoliticalSnapshotAccum = 0.f;
        RefreshEraProgress();
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
    RefreshPowerGridCoverage();
    RefreshBuildingPollutionExposure();
    RefreshForeignTradeDiplomacy(false);
    RefreshWorldMarketPrices();
    ApplyDailyEconomySettlement();
    ProcessActiveTradeRoutes();
    RefreshForeignTradeDiplomacy(true);
    ApplyDailyEdictCitizenEffects();
    ApplyDailyTaxPolicyEventEffects();
    ApplyDailyWorldCrisisEffects();
    TickGovernmentEdicts();
    PoliticsSystem::TickGovernmentActions(mGovernmentProfile);
    RefreshEdictModifiers();
    RefreshEraProgress();
    RefreshPoliticalSnapshot();
    TickTaxPolicyEvents();
    TickWorldCrises();
    TickPoliticalDemands();
    RefreshForeignTradeDiplomacy(false);

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

    TickElectionPromises();
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
