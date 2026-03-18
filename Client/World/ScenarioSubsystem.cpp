#include "ScenarioSubsystem.h"
#include "MainWorld.h"
#include "MainWorldTradeRuntime.h"
#include "WorldStatsSnapshot.h"
#include "../ObjectNames.h"
#include "../UI/EventWidget.h"
#include "../UI/ResultWidget.h"
#include "World/WorldUIManager.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <string>
#include <vector>

namespace
{
    constexpr int GScenarioForeignDemandDurationDays = 365;
    constexpr int GScenarioReligiousDemandDurationDays = 60;

    std::wstring FormatPercentText(double Value, int DecimalPlaces)
    {
        wchar_t Buffer[64] = {};

        if (DecimalPlaces <= 0)
        {
            swprintf_s(Buffer, L"%d%%", static_cast<int>(std::lround(Value)));
            return Buffer;
        }

        swprintf_s(Buffer, L"%.1f%%", Value);
        return Buffer;
    }

    std::wstring BuildTenureText(
        int StartYear,
        int StartMonth,
        int StartDay,
        int CurrentYear,
        int CurrentMonth,
        int CurrentDay)
    {
        int TotalMonths =
            (CurrentYear - StartYear) * 12 +
            (CurrentMonth - StartMonth);

        if (CurrentDay < StartDay)
            --TotalMonths;

        TotalMonths = (std::max)(0, TotalMonths);
        const int Years = TotalMonths / 12;
        const int Months = TotalMonths % 12;

        return
            L"재임 기간: " +
            std::to_wstring(Years) +
            L"년 " +
            std::to_wstring(Months) +
            L"개월";
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

void CScenarioSubsystem::Reset()
{
    if (mOwner)
    {
        TermStartYear = mOwner->mSimulation->Year;
        TermStartMonth = mOwner->mSimulation->Month;
        TermStartDay = mOwner->mSimulation->Day;
    }
    else
    {
        TermStartYear = 0;
        TermStartMonth = 1;
        TermStartDay = 1;
    }

    InitialBuildingCount = 0;
    PeakSupportPercent = 0.0;
    ResultShown = false;
    ScenarioElectionPromptPending = false;
    Runner.Init();
}

void CScenarioSubsystem::InitializeResultTracking()
{
    if (!mOwner)
        return;

    TermStartYear = mOwner->mSimulation->Year;
    TermStartMonth = mOwner->mSimulation->Month;
    TermStartDay = mOwner->mSimulation->Day;
    ResultShown = false;
    PeakSupportPercent =
        (std::max)(0.0, mOwner->mPolitics->PoliticalSnapshot.AverageSupportScore);

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
    {
        InitialBuildingCount = 0;
        return;
    }

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(World);
    InitialBuildingCount =
        (std::max)(0, Snapshot.TotalBuildingCount);
}

void CScenarioSubsystem::ApplyScenarioResult(const FScenarioEvent& ScenarioEvent)
{
    if (!mOwner || !ScenarioEvent.IsValid())
        return;

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
        return;

    switch (ScenarioEvent.Type)
    {
    case EScenarioEvent::ForeignDemand_USA:
        if (mOwner->mPolitics->PoliticalDemandService)
        {
            const std::wstring PowerName = GetScenarioForeignPowerName(
                0,
                mOwner->mEraState->EraProgress.CurrentEra);
            const bool Injected =
                mOwner->mPolitics->PoliticalDemandService->InjectScenarioDemand(
                    BuildScenarioUsaDemand(
                        mOwner->mTrade->State.ActiveTradeRoutes,
                        mOwner->mEraState->EraProgress.CurrentEra));

            if (Injected)
            {
                ShowScenarioEventWidget(
                    mOwner->GetUIManager(),
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
        if (mOwner->mPolitics->PoliticalDemandService)
        {
            const std::wstring PowerName = GetScenarioForeignPowerName(
                1,
                mOwner->mEraState->EraProgress.CurrentEra);
            const WorldStats::FWorldStatsSnapshot Snapshot =
                WorldStats::BuildSnapshot(World);
            const bool Injected =
                mOwner->mPolitics->PoliticalDemandService->InjectScenarioDemand(
                    BuildScenarioUssrDemand(
                        Snapshot,
                        mOwner->mEraState->EraProgress.CurrentEra));

            if (Injected)
            {
                ShowScenarioEventWidget(
                    mOwner->GetUIManager(),
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
        if (mOwner->mCrisis->WorldCrisisService)
        {
            const CMainWorldWorldCrisisService::FTickContext Context =
            {
                World,
                mOwner->mPolitics->PoliticalSnapshot,
                mOwner->mEdictState->GovernmentEdicts,
                mOwner->mPolitics->GovernmentProfile.TaxPolicy,
                mOwner->mEconomy->TaxEventStatus,
                mOwner->mSimulation->Year,
                mOwner->mSimulation->Month,
                mOwner->mSimulation->Day,
                mOwner->mEconomy->NationalBudget,
                mOwner->mEconomy->LastDailyNetChange,
                mOwner->mEconomy->LastDailyTaxCollectionEfficiency,
                mOwner->mEraState->EraProgress.CurrentEra
            };
            mOwner->mCrisis->WorldCrisisService->TriggerForcedCrisis(
                EWorldCrisisType::LaborStrike,
                Context);
        }
        break;
    case EScenarioEvent::FactionDemand_Religious:
        if (mOwner->mPolitics->PoliticalDemandService)
        {
            const WorldStats::FWorldStatsSnapshot Snapshot =
                WorldStats::BuildSnapshot(World);
            const bool Injected =
                mOwner->mPolitics->PoliticalDemandService->InjectScenarioDemand(
                    BuildScenarioReligiousDemand(Snapshot));

            if (Injected)
            {
                ShowScenarioEventWidget(
                    mOwner->GetUIManager(),
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
        ScenarioElectionPromptPending = true;
        break;
    case EScenarioEvent::None:
    default:
        break;
    }
}

void CScenarioSubsystem::ShowResultWidget(bool Victory)
{
    if (!mOwner || ResultShown)
        return;

    auto UiManager = mOwner->GetUIManager().lock();
    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!UiManager || !World)
        return;

    auto ResultWidget =
        UiManager->FindWidget<CResultWidget>(GResultWidgetName).lock();

    if (!ResultWidget)
        return;

    if (auto EventWidget = UiManager->FindWidget<CEventWidget>(GEventWidgetName).lock())
        EventWidget->GetMutableState().Visible = false;

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(World);
    const FElectionStatus& ElectionStatus =
        mOwner->mPolitics->ElectionService->GetElectionStatus();
    const int BuildingsBuilt =
        (std::max)(0, Snapshot.TotalBuildingCount - InitialBuildingCount);
    const std::wstring TenureText =
        BuildTenureText(
            TermStartYear,
            TermStartMonth,
            TermStartDay,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day);
    FResultWidgetState& State = ResultWidget->GetMutableState();
    State.Visible = true;
    State.Victory = Victory;

    if (Victory)
    {
        State.Title = L"재선 성공!";
        State.Summary = L"트로피코 시민들이 다시 한번 당신을 선택했습니다.";
        State.DetailPrimary =
            L"득표율: " + FormatPercentText(ElectionStatus.LastVoteShare, 1);
        State.DetailSecondary = L"임기 연장: 4년";
        State.DetailTertiary =
            L"임기 중 건설: " +
            std::to_wstring(BuildingsBuilt) +
            L"개 건물";
        State.DetailQuaternary =
            L"최고 지지율: " +
            FormatPercentText(PeakSupportPercent, 0);
    }
    else
    {
        State.Title = L"쿠데타 발생";
        State.Summary = L"지지율 붕괴로 군부가 관저를 점령했습니다.";
        State.DetailPrimary =
            L"최종 지지율: " +
            FormatPercentText(mOwner->mPolitics->PoliticalSnapshot.AverageSupportScore, 0);
        State.DetailSecondary = TenureText;
        State.DetailTertiary =
            L"임기 중 건설: " +
            std::to_wstring(BuildingsBuilt) +
            L"개 건물";
        State.DetailQuaternary =
            L"최고 지지율: " +
            FormatPercentText(PeakSupportPercent, 0);
    }

    mOwner->mSimulation->Paused = true;
    ResultShown = true;
}
