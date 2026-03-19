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
#include <Windows.h>
#include <algorithm>
#include <cmath>

namespace
{
    constexpr int GScenarioForeignDemandDurationDays = 365;
    constexpr int GScenarioReligiousDemandDurationDays = 60;
    constexpr float GExportSettlementPopupSeconds = 8.f;

    template <typename T>
    std::shared_ptr<T> LockOrWarn(
        const std::weak_ptr<T>& Weak,
        const char* Context)
    {
        auto Locked = Weak.lock();

        if (!Locked)
        {
            OutputDebugStringA("[MainWorld] expired: ");
            OutputDebugStringA(Context);
            OutputDebugStringA("\n");
        }

        return Locked;
    }

    std::wstring FormatSignedCurrency(long long Value)
    {
        const bool Positive = Value >= 0;
        const unsigned long long AbsoluteValue = Positive ?
            static_cast<unsigned long long>(Value) :
            static_cast<unsigned long long>(-Value);
        std::wstring Digits = std::to_wstring(AbsoluteValue);

        for (int Index = static_cast<int>(Digits.size()) - 3;
            Index > 0;
            Index -= 3)
        {
            Digits.insert(static_cast<size_t>(Index), 1, L',');
        }

        return std::wstring(Positive ? L"+$" : L"-$") + Digits;
    }

    std::wstring FormatAbsoluteCurrency(long long Value)
    {
        if (Value < 0)
            Value = -Value;

        return MainWorldTradeRuntime::FormatCurrency(Value);
    }

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
        auto UiManager = LockOrWarn(
            WeakUiManager,
            "ShowScenarioEventWidget.UiManager");

        if (!UiManager)
            return;

        auto EventWidget =
            LockOrWarn(
                UiManager->FindWidget<CEventWidget>(GEventWidgetName),
                "ShowScenarioEventWidget.EventWidget");

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
        State.ShowAcceptButton = true;
        State.ShowRejectButton = true;
        State.AutoCloseSeconds = 0.f;
    }

    void ShowExportSettlementWidget(
        const std::weak_ptr<CWorldUIManager>& WeakUiManager,
        long long ExportIncome,
        long long TaxIncome,
        long long WageCost,
        long long UpkeepCost,
        long long ImportExpense,
        long long EdictCost,
        long long TradePolicyBudgetDelta,
        long long NetBudgetChange)
    {
        if (ExportIncome <= 0)
            return;

        auto UiManager = LockOrWarn(
            WeakUiManager,
            "ShowExportSettlementWidget.UiManager");

        if (!UiManager)
            return;

        auto EventWidget =
            LockOrWarn(
                UiManager->FindWidget<CEventWidget>(GEventWidgetName),
                "ShowExportSettlementWidget.EventWidget");

        if (!EventWidget)
            return;

        FEventWidgetState& State = EventWidget->GetMutableState();

        if (State.Visible &&
            State.IssuerType != EPoliticalDemandIssuerType::None)
        {
            return;
        }

        State.Visible = true;
        State.IssuerType = EPoliticalDemandIssuerType::None;
        State.IssuerIndex = -1;
        State.Title = L"수출 발생 / 일일 예산 정산";
        State.Body =
            L"총수출 " +
            FormatAbsoluteCurrency(ExportIncome) +
            L" + 총세금 " +
            FormatAbsoluteCurrency(TaxIncome) +
            L"\n- 임금 " +
            FormatAbsoluteCurrency(WageCost) +
            L" - 유지비 " +
            FormatAbsoluteCurrency(UpkeepCost) +
            L" - 총수입비 " +
            FormatAbsoluteCurrency(ImportExpense) +
            L" - 순칙령비 " +
            FormatAbsoluteCurrency(EdictCost);
        State.AcceptConsequence =
            L"무역정책보정 " +
            FormatSignedCurrency(TradePolicyBudgetDelta);
        State.RejectConsequence =
            L"오늘 순변동 " +
            FormatSignedCurrency(NetBudgetChange);
        State.ShowAcceptButton = false;
        State.ShowRejectButton = false;
        State.AutoCloseSeconds = GExportSettlementPopupSeconds;
    }
}

void CMainWorld::OnUiManagerUpdated()
{
    UILayoutApplier::ApplyWidgetOverrides(
        LockOrWarn(
            GetUIManager(),
            "CMainWorld::OnUiManagerUpdated.UiManager"));
}

void CMainWorld::Update(float DeltaTime)
{
    mEconomy->WorkerTaxPressureDays = 0;
    mEconomy->PropertyTaxPressureDays = 0;
    mEconomy->BudgetCrisisPressureDays = 0;
    mEconomy->TaxEventStatus = FTaxPolicyEventStatus();

    if (mCrisis && mCrisis->WorldCrisisService)
        mCrisis->WorldCrisisService->Reset();

    UILayoutLoader::ReloadIfChanged(DeltaTime);
    RuntimeConfigRegistry::PollAll(DeltaTime);

    const unsigned long long GameConstantsGeneration =
        GameConstants::GetRuntimeConfigGeneration();

    if (GameConstantsGeneration != mLastGameConstantsGeneration)
    {
        mLastGameConstantsGeneration = GameConstantsGeneration;
        RebuildRoadNetwork();
        RefreshRuntimeBuildingState();
        mPolitics->RefreshSnapshot(
            {
                &mEconomy->TaxEventStatus,
                &mKnowledgeState->ConstitutionState.ActiveEffects
            });
        mTrade->OnDayAdvanced(
            {
                mPolitics->GovernmentProfile,
                mEconomy->TaxEventStatus,
                mEdictState->GovernmentEdicts,
                mEraState->EraProgress.CurrentEra,
                mSimulation->Year,
                mSimulation->Month,
                mSimulation->Day,
                false
            });
        mEconomy->RefreshWorldMarketPrices(
            {
                mPolitics->GovernmentProfile,
                mEdictState->GovernmentEdicts,
                mCrisis->WorldCrisisService->GetStatus(),
                mTrade->State.ForeignPowerStates,
                mSimulation->Year,
                mSimulation->Month,
                mSimulation->Day
            });

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
        EdictSystem::SynchronizeGovernmentEdictStates(mEdictState->GovernmentEdicts);
        mPolitics->GovernmentProfile.ActiveActions.clear();

        for (size_t Index = 0; Index < mEdictState->GovernmentEdicts.size(); ++Index)
        {
            PoliticsSystem::SyncGovernmentActionFromEdict(
                mPolitics->GovernmentProfile,
                mEdictState->GovernmentEdicts[Index].Type,
                mEdictState->GovernmentEdicts[Index].Active);
        }

        mEdictState->RefreshEdictModifiers();
        mPolitics->RefreshSnapshot(
            {
                &mEconomy->TaxEventStatus,
                &mKnowledgeState->ConstitutionState.ActiveEffects
            });
        mTrade->OnDayAdvanced(
            {
                mPolitics->GovernmentProfile,
                mEconomy->TaxEventStatus,
                mEdictState->GovernmentEdicts,
                mEraState->EraProgress.CurrentEra,
                mSimulation->Year,
                mSimulation->Month,
                mSimulation->Day,
                false
            });
    }

    const float SimulationDeltaTime =
        mSimulation->Paused ?
        0.f :
        DeltaTime * static_cast<float>((std::max)(1, mSimulation->SpeedMultiplier));

    CWorld::Update(SimulationDeltaTime);

    if (mPolitics->ElectionService->IsGameLost())
    {
        mScenario->ShowResultWidget(false);
        return;
    }

    mSimulation->Tick(SimulationDeltaTime);
    TickPoliticalRefresh(SimulationDeltaTime);
    mPopulation->Tick(SimulationDeltaTime);
}

void CMainWorld::TickPoliticalRefresh(float DeltaTime)
{
    mSimulation->PoliticalSnapshotAccum += DeltaTime;

    if (mSimulation->PoliticalSnapshotAccum >= MainWorldConfig::GPoliticalSnapshotInterval)
    {
        mSimulation->PoliticalSnapshotAccum = 0.f;
        mEraState->RefreshEraProgress();
        mPolitics->RefreshSnapshot(
            {
                &mEconomy->TaxEventStatus,
                &mKnowledgeState->ConstitutionState.ActiveEffects
            });
    }
}

void CMainWorld::AdvanceSimulationDay()
{
    mEconomy->WorkerTaxPressureDays = 0;
    mEconomy->PropertyTaxPressureDays = 0;
    mEconomy->BudgetCrisisPressureDays = 0;
    mEconomy->TaxEventStatus = FTaxPolicyEventStatus();

    if (mCrisis && mCrisis->WorldCrisisService)
        mCrisis->WorldCrisisService->Reset();

    mInfrastructure->RefreshPowerGridCoverage();
    mInfrastructure->RefreshBuildingPollutionExposure();
    mKnowledgeState->RefreshKnowledgeGeneration();
    mTrade->OnDayAdvanced(
        {
            mPolitics->GovernmentProfile,
            mEconomy->TaxEventStatus,
            mEdictState->GovernmentEdicts,
            mEraState->EraProgress.CurrentEra,
            mSimulation->Year,
            mSimulation->Month,
            mSimulation->Day,
            false
        });
    mEconomy->RefreshWorldMarketPrices(
        {
            mPolitics->GovernmentProfile,
            mEdictState->GovernmentEdicts,
            mCrisis->WorldCrisisService->GetStatus(),
            mTrade->State.ForeignPowerStates,
            mSimulation->Year,
            mSimulation->Month,
            mSimulation->Day
        });
    mEconomy->OnDayAdvanced(
        {
            mPolitics->GovernmentProfile,
            mEdictState->GovernmentEdicts,
            mEdictState->EdictModifiers,
            mSimulation->Year,
            mSimulation->Month
        });
    mTrade->ProcessActiveRoutes();
    mEconomy->RefreshTradePolicyBudgetDelta(
        mPolitics->GovernmentProfile);
    ShowExportSettlementWidget(
        GetUIManager(),
        mEconomy->LastDailyExportIncome,
        mEconomy->LastDailyTaxIncome,
        mEconomy->LastDailyWageCost,
        mEconomy->LastDailyUpkeepCost,
        mEconomy->LastDailyImportExpense,
        mEconomy->LastDailyEdictCost,
        mEconomy->LastDailyTradePolicyBudgetDelta,
        mEconomy->LastDailyNetChange);
    mTrade->OnDayAdvanced(
        {
            mPolitics->GovernmentProfile,
            mEconomy->TaxEventStatus,
            mEdictState->GovernmentEdicts,
            mEraState->EraProgress.CurrentEra,
            mSimulation->Year,
            mSimulation->Month,
            mSimulation->Day,
            true
        });
    mEdictState->ApplyDailyEdictCitizenEffects();
    mEconomy->ApplyDailyTaxPolicyEventEffects();
    mCrisis->ApplyDailyWorldCrisisEffects();
    mEdictState->TickGovernmentEdicts();
    PoliticsSystem::TickGovernmentActions(mPolitics->GovernmentProfile);
    mEdictState->RefreshEdictModifiers();
    mKnowledgeState->ApplyDailyKnowledgeGain();
    mEraState->TickEraTransitionState();
    mEraState->RefreshEraProgress();
    mPolitics->RefreshSnapshot(
        {
            &mEconomy->TaxEventStatus,
            &mKnowledgeState->ConstitutionState.ActiveEffects
        });
    mEconomy->TickTaxEvents(
        {
            mPolitics->PoliticalSnapshot,
            mPolitics->GovernmentProfile,
            mSimulation->Year,
            mSimulation->Month,
            mSimulation->Day
        });
    mCrisis->TickWorldCrises();
    mPolitics->TickPoliticalDemands();
    mTrade->OnDayAdvanced(
        {
            mPolitics->GovernmentProfile,
            mEconomy->TaxEventStatus,
            mEdictState->GovernmentEdicts,
            mEraState->EraProgress.CurrentEra,
            mSimulation->Year,
            mSimulation->Month,
            mSimulation->Day,
            false
        });

    const std::shared_ptr<CWorld> World = mSelf.lock();
    const int CurrentMonthDays =
        mSimulation->GetDaysInMonth(mSimulation->Year, mSimulation->Month);

    if (World)
    {
        mEconomy->FinalizeDayLedger(
            WorldStats::BuildSnapshot(World),
            mSimulation->Day >= CurrentMonthDays);
    }

    ++mSimulation->Day;

    if (mSimulation->Day > CurrentMonthDays)
    {
        mSimulation->Day = 1;
        ++mSimulation->Month;

        if (mSimulation->Month > 12)
        {
            mSimulation->Month = 1;
            ++mSimulation->Year;
        }
    }

    const FScenarioEvent ScenarioResult =
        mScenario->Runner.Tick(
            mSimulation->Year,
            mSimulation->Month,
            mSimulation->Day);
    mScenario->ApplyScenarioResult(ScenarioResult);
    mPolitics->TickElectionPromises(
        mSimulation->Year,
        mSimulation->Month,
        mSimulation->Day,
        mEconomy->LastDailyExportIncome);
    mPolitics->ResolveScheduledElection(
        mSimulation->Year,
        mSimulation->Month,
        mSimulation->Day);
}

void CMainWorld::PostUpdate(float DeltaTime)
{
    CWorld::PostUpdate(DeltaTime);
}
