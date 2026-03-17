#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "MainWorldTradeRuntime.h"
#include "WorldStatsSnapshot.h"
#include "RuntimeConfigRegistry.h"
#include "../ObjectNames.h"
#include "../UI/EventWidget.h"
#include "../UI/UILayoutApplier.h"
#include "../UI/UILayoutLoader.h"
#include "../GameConstants.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Politics/EdictSystem.h"
#include "../Politics/PoliticsSystem.h"
#include "World/WorldUIManager.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr int GScenarioForeignDemandDurationDays = 365;
    constexpr int GScenarioReligiousDemandDurationDays = 60;

    std::wstring GetScenarioForeignPowerName(
        int ForeignPowerIndex,
        EBuildingEra CurrentEra)
    {
        return MainWorldTradeRuntime::GetForeignPowerName(
            ForeignPowerIndex,
            CurrentEra);
    }

    FPoliticalDemandState BuildScenarioUsaDemand(
        const std::vector<FTradeRouteRuntimeState>& ActiveTradeRoutes,
        EBuildingEra CurrentEra)
    {
        FPoliticalDemandState Demand;
        const std::wstring PowerName =
            GetScenarioForeignPowerName(0, CurrentEra);
        Demand.Active = true;
        Demand.IssuerType = EPoliticalDemandIssuerType::ForeignPower;
        Demand.IssuerIndex = 0;
        Demand.ObjectiveType = EPoliticalDemandObjectiveType::ActiveTradeRoutes;
        Demand.Stage = EPoliticalDemandStage::Demand;
        Demand.Status = EPoliticalDemandStatus::PendingResponse;
        Demand.DurationDays = GScenarioForeignDemandDurationDays;
        Demand.RemainingDays = Demand.DurationDays;
        Demand.TargetValue = 2;
        Demand.CurrentValue =
            MainWorldTradeRuntime::CountActiveTradeRoutesForPower(
                ActiveTradeRoutes,
                Demand.IssuerIndex);
        Demand.PenaltyForeignRelationDelta = -20;
        Demand.Title = PowerName + L"의 제안";
        Demand.Summary =
            PowerName +
            L" 대사가 도착했습니다. 관계 인정을 위해 " +
            PowerName +
            L"과의 무역로를 2개 이상 개설하라고 요구합니다.";
        Demand.ObjectiveText = PowerName + L"과 활성 무역로 2개 이상";
        Demand.RewardText = L"목표 활성화";
        Demand.PenaltyText = PowerName + L" 외교관계 -20";
        return Demand;
    }

    FPoliticalDemandState BuildScenarioUssrDemand(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        EBuildingEra CurrentEra)
    {
        FPoliticalDemandState Demand;
        const std::wstring PowerName =
            GetScenarioForeignPowerName(1, CurrentEra);
        Demand.Active = true;
        Demand.IssuerType = EPoliticalDemandIssuerType::ForeignPower;
        Demand.IssuerIndex = 1;
        Demand.ObjectiveType = EPoliticalDemandObjectiveType::Housing;
        Demand.Stage = EPoliticalDemandStage::Demand;
        Demand.Status = EPoliticalDemandStatus::PendingResponse;
        Demand.DurationDays = GScenarioForeignDemandDurationDays;
        Demand.RemainingDays = Demand.DurationDays;
        Demand.TargetValue = 70;
        Demand.CurrentValue =
            static_cast<int>(std::lround(Snapshot.AverageHousing));
        Demand.PenaltyForeignRelationDelta = -20;
        Demand.Title = PowerName + L"의 제안";
        Demand.Summary =
            PowerName +
            L" 대사가 지원을 제안했습니다. 평균 주거 만족도를 70 "
            L"이상으로 유지하면 지원을 약속합니다.";
        Demand.ObjectiveText = L"평균 주거 70 이상";
        Demand.RewardText = L"목표 활성화";
        Demand.PenaltyText = PowerName + L" 외교관계 -20";
        return Demand;
    }

    FPoliticalDemandState BuildScenarioReligiousDemand(
        const WorldStats::FWorldStatsSnapshot& Snapshot)
    {
        FPoliticalDemandState Demand;
        Demand.Active = true;
        Demand.IssuerType = EPoliticalDemandIssuerType::Faction;
        Demand.IssuerIndex = static_cast<int>(EPoliticalFaction::Religious);
        Demand.ObjectiveType = EPoliticalDemandObjectiveType::Faith;
        Demand.Stage = EPoliticalDemandStage::Demand;
        Demand.Status = EPoliticalDemandStatus::PendingResponse;
        Demand.DurationDays = GScenarioReligiousDemandDurationDays;
        Demand.RemainingDays = Demand.DurationDays;
        Demand.TargetValue = 65;
        Demand.CurrentValue =
            static_cast<int>(std::lround(Snapshot.AverageFaith));
        Demand.ModifierDurationDays = 90;
        Demand.PenaltyFactionApprovalDelta = -30;
        Demand.Title = L"종교 파벌의 요구";
        Demand.Summary =
            L"신앙 만족도가 낮습니다. 교회 건설이나 종교 서비스 확충으로 "
            L"평균 신앙을 65 이상까지 끌어올리십시오.";
        Demand.ObjectiveText = L"평균 신앙 65 이상";
        Demand.RewardText = L"신앙 안정";
        Demand.PenaltyText = L"종교 파벌 지지 하락";
        return Demand;
    }

    void ShowScenarioEventWidget(
        const std::weak_ptr<CWorldUIManager>& WeakUiManager,
        EPoliticalDemandIssuerType IssuerType,
        int IssuerIndex,
        const std::wstring& Title,
        const std::wstring& Body,
        const std::wstring& AcceptConsequence,
        const std::wstring& RejectConsequence)
    {
        auto UiManager = WeakUiManager.lock();

        if (!UiManager)
            return;

        auto EventWidget =
            UiManager->FindWidget<CEventWidget>(GEventWidgetName).lock();

        if (!EventWidget)
            return;

        FEventWidgetState& State = EventWidget->GetMutableState();
        State.Visible = true;
        State.IssuerType = IssuerType;
        State.IssuerIndex = IssuerIndex;
        State.Title = Title;
        State.Body = Body;
        State.AcceptConsequence = AcceptConsequence;
        State.RejectConsequence = RejectConsequence;
    }
}

void CMainWorld::ToggleSimulationPaused()
{
    mSimulation.Paused = !mSimulation.Paused;
}

void CMainWorld::CycleSimulationSpeedMultiplier()
{
    if (mSimulation.SpeedMultiplier >= 4)
    {
        mSimulation.SpeedMultiplier = 1;
        return;
    }

    if (mSimulation.SpeedMultiplier >= 2)
    {
        mSimulation.SpeedMultiplier = 4;
        return;
    }

    mSimulation.SpeedMultiplier = 2;
}

void CMainWorld::OnUiManagerUpdated()
{
    UILayoutApplier::ApplyWidgetOverrides(GetUIManager().lock());
}

void CMainWorld::Update(float DeltaTime)
{
    UILayoutLoader::ReloadIfChanged(DeltaTime);
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
        EdictSystem::SynchronizeGovernmentEdictStates(mPolicy.GovernmentEdicts);
        mPolicy.GovernmentProfile.ActiveActions.clear();

        for (size_t Index = 0; Index < mPolicy.GovernmentEdicts.size(); ++Index)
        {
            PoliticsSystem::SyncGovernmentActionFromEdict(
                mPolicy.GovernmentProfile,
                mPolicy.GovernmentEdicts[Index].Type,
                mPolicy.GovernmentEdicts[Index].Active);
        }

        RefreshEdictModifiers();
        RefreshPoliticalSnapshot();
        RefreshForeignTradeDiplomacy(false);
    }

    const float SimulationDeltaTime =
        mSimulation.Paused ?
        0.f :
        DeltaTime * static_cast<float>((std::max)(1, mSimulation.SpeedMultiplier));

    CWorld::Update(SimulationDeltaTime);

    if (mServices.ElectionService->IsGameLost())
    {
        ShowResultWidget(false);
        return;
    }

    AdvanceSimulationDate(SimulationDeltaTime);
    TickPoliticalRefresh(SimulationDeltaTime);
    TickCitizenPopulation(SimulationDeltaTime);
}

void CMainWorld::TickPoliticalRefresh(float DeltaTime)
{
    mSimulation.PoliticalSnapshotAccum += DeltaTime;

    if (mSimulation.PoliticalSnapshotAccum >= MainWorldConfig::GPoliticalSnapshotInterval)
    {
        mSimulation.PoliticalSnapshotAccum = 0.f;
        RefreshEraProgress();
        RefreshPoliticalSnapshot();
    }
}

int CMainWorld::GetSimulationMonthDayCount() const
{
    return GetDaysInMonth(mSimulation.Year, mSimulation.Month);
}

float CMainWorld::GetSimulationDayProgress() const
{
    if (mSimulation.SecondsPerSimulationDay <= 0.f)
        return 0.f;

    return Clamp<float>(
        mSimulation.DayProgressAccum / mSimulation.SecondsPerSimulationDay, 0.f, 1.f);
}

float CMainWorld::GetSimulationMonthProgress() const
{
    const int MonthDays = GetDaysInMonth(mSimulation.Year, mSimulation.Month);

    if (MonthDays <= 0)
        return 0.f;

    const float DayProgress = GetSimulationDayProgress();
    const float CompletedDays =
        static_cast<float>((std::max)(0, mSimulation.Day - 1)) +
        DayProgress;

    return Clamp<float>(
        CompletedDays / static_cast<float>(MonthDays), 0.f, 1.f);
}

void CMainWorld::AdvanceSimulationDate(float DeltaTime)
{
    if (DeltaTime <= 0.f || mSimulation.SecondsPerSimulationDay <= 0.f)
        return;

    mSimulation.DayProgressAccum += DeltaTime;

    while (mSimulation.DayProgressAccum >= mSimulation.SecondsPerSimulationDay)
    {
        mSimulation.DayProgressAccum -= mSimulation.SecondsPerSimulationDay;
        AdvanceSimulationDay();
    }
}

void CMainWorld::AdvanceSimulationDay()
{
    RefreshPowerGridCoverage();
    RefreshBuildingPollutionExposure();
    RefreshKnowledgeGeneration();
    RefreshForeignTradeDiplomacy(false);
    RefreshWorldMarketPrices();
    ApplyDailyEconomySettlement();
    ProcessActiveTradeRoutes();
    RefreshForeignTradeDiplomacy(true);
    ApplyDailyEdictCitizenEffects();
    ApplyDailyTaxPolicyEventEffects();
    ApplyDailyWorldCrisisEffects();
    TickGovernmentEdicts();
    PoliticsSystem::TickGovernmentActions(mPolicy.GovernmentProfile);
    RefreshEdictModifiers();
    ApplyDailyKnowledgeGain();
    TickEraTransitionState();
    RefreshEraProgress();
    RefreshPoliticalSnapshot();
    TickTaxPolicyEvents();
    TickWorldCrises();
    TickPoliticalDemands();
    RefreshForeignTradeDiplomacy(false);

    ++mSimulation.Day;
    const int CurrentMonthDays =
        GetDaysInMonth(mSimulation.Year, mSimulation.Month);

    if (mSimulation.Day > CurrentMonthDays)
    {
        mSimulation.Day = 1;
        ++mSimulation.Month;

        if (mSimulation.Month > 12)
        {
            mSimulation.Month = 1;
            ++mSimulation.Year;
        }
    }

    const FScenarioEvent ScenarioResult =
        mScenarioRunner.Tick(
            mSimulation.Year,
            mSimulation.Month,
            mSimulation.Day);
    ApplyScenarioResult(ScenarioResult);
    TickElectionPromises();
    ResolveScheduledElection();
}

void CMainWorld::ApplyScenarioResult(const FScenarioEvent& ScenarioEvent)
{
    if (!ScenarioEvent.IsValid())
        return;

    const std::shared_ptr<CWorld> World = mSelf.lock();

    if (!World)
        return;

    switch (ScenarioEvent.Type)
    {
    case EScenarioEvent::ForeignDemand_USA:
        if (mServices.PoliticalDemandService)
        {
            const std::wstring PowerName = GetScenarioForeignPowerName(
                0,
                mPolicy.EraProgress.CurrentEra);
            const bool Injected =
                mServices.PoliticalDemandService->InjectScenarioDemand(
                    BuildScenarioUsaDemand(
                        mTradeDiplomacyState.ActiveTradeRoutes,
                        mPolicy.EraProgress.CurrentEra));

            if (Injected)
            {
                ShowScenarioEventWidget(
                    mUIManager,
                    EPoliticalDemandIssuerType::ForeignPower,
                    0,
                    PowerName + L"의 제안",
                    PowerName + L" 대사가 도착했습니다.\n"
                    L"수출 무역로 2개를 개설하면\n"
                    L"관계를 인정하겠습니다.",
                    L"수락 시: 무역로 목표가 활성화됩니다.",
                    L"거부 시: " + PowerName + L" 외교관계 -20");
            }
        }
        break;
    case EScenarioEvent::ForeignDemand_USSR:
        if (mServices.PoliticalDemandService)
        {
            const std::wstring PowerName = GetScenarioForeignPowerName(
                1,
                mPolicy.EraProgress.CurrentEra);
            const WorldStats::FWorldStatsSnapshot Snapshot =
                WorldStats::BuildSnapshot(World);
            const bool Injected =
                mServices.PoliticalDemandService->InjectScenarioDemand(
                    BuildScenarioUssrDemand(
                        Snapshot,
                        mPolicy.EraProgress.CurrentEra));

            if (Injected)
            {
                ShowScenarioEventWidget(
                    mUIManager,
                    EPoliticalDemandIssuerType::ForeignPower,
                    1,
                    PowerName + L"의 제안",
                    PowerName + L" 대사가 지원을 제안합니다.\n"
                    L"주거 만족도 70 이상을 유지하면\n"
                    L"지원금을 제공하겠습니다.",
                    L"수락 시: 주거 목표가 활성화됩니다.",
                    L"거부 시: " + PowerName + L" 외교관계 -20");
            }
        }
        break;
    case EScenarioEvent::Crisis_LaborStrike:
        if (mServices.WorldCrisisService)
        {
            const CMainWorldWorldCrisisService::FTickContext Context =
            {
                World,
                mPolicy.PoliticalSnapshot,
                mPolicy.GovernmentEdicts,
                mPolicy.GovernmentProfile.TaxPolicy,
                mPolicy.TaxEventStatus,
                mSimulation.Year,
                mSimulation.Month,
                mSimulation.Day,
                mBudget.NationalBudget,
                mBudget.LastDailyNetChange,
                mBudget.LastDailyTaxCollectionEfficiency
            };
            mServices.WorldCrisisService->TriggerForcedCrisis(
                EWorldCrisisType::LaborStrike,
                Context);
        }
        break;
    case EScenarioEvent::FactionDemand_Religious:
        if (mServices.PoliticalDemandService)
        {
            const WorldStats::FWorldStatsSnapshot Snapshot =
                WorldStats::BuildSnapshot(World);
            const bool Injected =
                mServices.PoliticalDemandService->InjectScenarioDemand(
                    BuildScenarioReligiousDemand(Snapshot));

            if (Injected)
            {
                ShowScenarioEventWidget(
                    mUIManager,
                    EPoliticalDemandIssuerType::Faction,
                    static_cast<int>(EPoliticalFaction::Religious),
                    L"종교 파벌의 요구",
                    L"신앙 만족도가 낮습니다.\n"
                    L"교회를 건설하거나 종교 서비스를 확충해\n"
                    L"평균 신앙 65 이상을 달성하십시오.",
                    L"수락 시: 신앙 목표가 활성화됩니다.",
                    L"거부 시: 종교 파벌 압박이 심화됩니다.");
            }
        }
        break;
    case EScenarioEvent::ElectionPromptPopup:
        mScenarioElectionPromptPending = true;
        break;
    case EScenarioEvent::None:
    default:
        break;
    }
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

