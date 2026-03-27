#include "ScenarioSubsystem.h"
#include "MainWorld.h"
#include "MainWorldTradeRuntime.h"
#include "TradeDiplomacySubsystem.h"
#include "WorldStatsSnapshot.h"
#include "../ObjectNames.h"
#include "../UI/EventWidget.h"
#include "../UI/ResultWidget.h"
#include "../UI/TaskWidget.h"
#include "World/WorldUIManager.h"
#include "../Economy/ResourceTradePricing.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <string>
#include <vector>

namespace
{
    constexpr int GScenarioDemandDurationDays = 3650; // 사실상 무제한
    constexpr long long GIndependencePaymentAmount = 10000LL;
    constexpr int GScenarioRumTradeAvailability = 4000;
    constexpr int GSmugglersRumScenarioTag = 1001;
    constexpr int GCrownRumScenarioTag = 1002;

    int ResolveScenarioRumBasePricePerThousand()
    {
        int& ScenarioBiasPercent =
            ResourceTradePricing::GetScenarioRumExportBiasPercent();
        const int SavedBiasPercent = ScenarioBiasPercent;
        ScenarioBiasPercent = 0;
        const int BasePricePerThousand =
            MainWorldTradeRuntime::ComputeTradeRouteSignedStandardPricePerThousand(
                EResourceType::Rum,
                false);
        ScenarioBiasPercent = SavedBiasPercent;
        return BasePricePerThousand;
    }

    void InjectScenarioRumExportOffer(
        CTradeDiplomacySubsystem& Trade,
        int ForeignPowerIndex,
        int ScenarioTag,
        int OfferPricePerThousand)
    {
        const int BasePricePerThousand = ResolveScenarioRumBasePricePerThousand();

        FTradeOfferRuntimeState Offer;
        Offer.OfferId = Trade.State.NextTradeOfferId++;
        Offer.ScenarioTag = ScenarioTag;
        Offer.ImportRoute = false;
        Offer.ResourceType = EResourceType::Rum;
        Offer.MarketClass = GetResourceMarketClass(EResourceType::Rum);
        Offer.ForeignPowerIndex = ForeignPowerIndex;
        Offer.BasePricePerThousand = BasePricePerThousand;
        Offer.OfferPricePerThousand = (std::max)(1000, OfferPricePerThousand);
        Offer.MarginPercent =
            BasePricePerThousand > 0 ?
                static_cast<int>(std::lround(
                    static_cast<double>(
                        Offer.OfferPricePerThousand - BasePricePerThousand) *
                    100.0 /
                    static_cast<double>(BasePricePerThousand))) :
                0;
        Offer.MaxAmount = GScenarioRumTradeAvailability;
        Offer.AvailabilityUnits = GScenarioRumTradeAvailability;
        Offer.Score =
            GScenarioRumTradeAvailability *
            ResourceTradePricing::GetExportPricePerStockUnit(EResourceType::Rum);

        Trade.State.AvailableTradeOffers.push_back(Offer);
    }

    bool HasCompletedScenarioRumRoute(
        const CTradeDiplomacySubsystem& Trade,
        int ForeignPowerIndex,
        int ScenarioTag)
    {
        for (const FTradeRouteCompletionRecord& Record :
            Trade.State.CompletedTradeRoutes)
        {
            if (!Record.ImportRoute &&
                Record.ResourceType == EResourceType::Rum &&
                Record.ForeignPowerIndex == ForeignPowerIndex &&
                (Record.ScenarioTag == ScenarioTag ||
                    Record.ScenarioTag == 0) &&
                Record.EndReason == ETradeRouteEndReason::Completed)
            {
                return true;
            }
        }

        return false;
    }

    bool HasActiveScenarioRumRoute(
        const CTradeDiplomacySubsystem& Trade,
        int ScenarioTag)
    {
        for (const FTradeRouteRuntimeState& Route : Trade.State.ActiveTradeRoutes)
        {
            if (!Route.ImportRoute &&
                Route.ResourceType == EResourceType::Rum &&
                Route.ScenarioTag == ScenarioTag)
            {
                return true;
            }
        }

        return false;
    }

    bool HasScenarioRumOffer(
        const CTradeDiplomacySubsystem& Trade,
        int ScenarioTag)
    {
        for (const FTradeOfferRuntimeState& Offer : Trade.State.AvailableTradeOffers)
        {
            if (!Offer.ImportRoute &&
                Offer.ResourceType == EResourceType::Rum &&
                Offer.ScenarioTag == ScenarioTag)
            {
                return true;
            }
        }

        return false;
    }

    void RemoveScenarioRumOffers(
        CTradeDiplomacySubsystem& Trade,
        int ScenarioTag)
    {
        auto& Offers = Trade.State.AvailableTradeOffers;
        Offers.erase(
            std::remove_if(
                Offers.begin(),
                Offers.end(),
                [ScenarioTag](const FTradeOfferRuntimeState& Offer)
                {
                    return !Offer.ImportRoute &&
                        Offer.ResourceType == EResourceType::Rum &&
                        Offer.ScenarioTag == ScenarioTag;
                }),
            Offers.end());
    }

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

}

// ──────────────────────────────────────────────────────────────
//  과제 UI 열기 헬퍼 (팝업 대신 TaskWidget 직접 표시)
// ──────────────────────────────────────────────────────────────
void CScenarioSubsystem::ShowEventWidget(
    int IssuerIndex,
    const std::wstring& /*Title*/,
    const std::wstring& /*Body*/,
    const std::wstring& /*AcceptConsequence*/,
    const std::wstring& /*RejectConsequence*/)
{
    auto UiManager = mOwner ? mOwner->GetUIManager().lock() : nullptr;

    if (!UiManager)
        return;

    auto TaskWidgetPtr =
        UiManager->FindWidget<CTaskWidget>(GTaskWidgetName).lock();

    if (!TaskWidgetPtr)
        return;

    TaskWidgetPtr->OpenForDemand(
        EPoliticalDemandIssuerType::ForeignPower,
        IssuerIndex);
}

void CScenarioSubsystem::RefreshScenarioWorldMarketPrices()
{
    if (!mOwner || !mOwner->mEconomy || !mOwner->mPolitics ||
        !mOwner->mEdictState || !mOwner->mCrisis || !mOwner->mTrade ||
        !mOwner->mSimulation)
    {
        return;
    }

    mOwner->mEconomy->RefreshWorldMarketPrices(
        {
            mOwner->mPolitics->GovernmentProfile,
            mOwner->mEdictState->GovernmentEdicts,
            mOwner->mCrisis->WorldCrisisService->GetStatus(),
            mOwner->mTrade->State.ForeignPowerStates,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day
        });
}

// ──────────────────────────────────────────────────────────────
//  밀수 무역로 보상
// ──────────────────────────────────────────────────────────────

void CScenarioSubsystem::OpenSmugglersRumRoute()
{
    if (!mOwner)
        return;

    CTradeDiplomacySubsystem* Trade = mOwner->GetTrade();

    if (!Trade)
        return;

    // Colonial 시대에는 기본 제안 생성기가 파트너를 왕실(0)로 고정하므로
    // 시나리오 전용 밀수 무역로는 수동 주입한다.
    InjectScenarioRumExportOffer(
        *Trade,
        1,
        GSmugglersRumScenarioTag,
        ResolveScenarioRumBasePricePerThousand());
    SmugglersRumOfferInjected = true;
}

void CScenarioSubsystem::OpenCrownRumRoute()
{
    if (!mOwner)
        return;

    CTradeDiplomacySubsystem* Trade = mOwner->GetTrade();

    if (!Trade)
        return;

    const int BasePricePerThousand = ResolveScenarioRumBasePricePerThousand();
    const int DiscountedPricePerThousand =
        (std::max)(1000, BasePricePerThousand / 2);

    InjectScenarioRumExportOffer(
        *Trade,
        0,
        GCrownRumScenarioTag,
        DiscountedPricePerThousand);
    CrownRumOfferInjected = true;
}

// ──────────────────────────────────────────────────────────────
//  Demand 주입 헬퍼
// ──────────────────────────────────────────────────────────────

void CScenarioSubsystem::InjectSmugglersOfferDemand()
{
    if (!mOwner || !mOwner->mPolitics->PoliticalDemandService)
        return;

    FPoliticalDemandState Demand;
    Demand.Active = true;
    Demand.IssuerType = EPoliticalDemandIssuerType::ForeignPower;
    Demand.IssuerIndex = 1; // 밀수업자
    Demand.ObjectiveType = EPoliticalDemandObjectiveType::RumProducerBuilding;
    Demand.Stage = EPoliticalDemandStage::Demand;
    Demand.Status = EPoliticalDemandStatus::PendingResponse;
    Demand.DurationDays = GScenarioDemandDurationDays;
    Demand.RemainingDays = GScenarioDemandDurationDays;
    Demand.TargetValue = 1;
    Demand.CurrentValue = 0;
    Demand.Title = L"밀수업자의 제안";
    Demand.Summary =
        L"낡은 배를 탄 수상한 남자가 항구에 나타났습니다.\n"
        L"\"이 섬에서 럼주를 만들면 내가 팔아주지.\n"
        L"럼주 증류소를 하나만 지어봐.\"";
    Demand.ObjectiveText = L"럼주 증류소 1개 건설";
    Demand.RewardText = L"밀수 판로 개척";
    Demand.PenaltyText = L"없음";
    mOwner->mPolitics->PoliticalDemandService->InjectScenarioDemand(Demand);

    ShowEventWidget(
        1,
        L"밀수업자의 제안",
        L"낡은 배를 탄 수상한 남자가 항구에 나타났습니다.\n"
        L"\"이 섬에서 럼주를 만들면 내가 팔아주지.\n"
        L"럼주 증류소를 하나만 지어봐.\"",
        L"수락: 럼주 증류소를 짓기 시작합니다.",
        L"거부: 제안을 무시합니다.");
}

void CScenarioSubsystem::InjectPenultimoFarmDemand()
{
    if (!mOwner || !mOwner->mPolitics->PoliticalDemandService)
        return;

    FPoliticalDemandState Demand;
    Demand.Active = true;
    Demand.IssuerType = EPoliticalDemandIssuerType::ForeignPower;
    Demand.IssuerIndex = 2; // 패놀티모 (broker 초상화)
    Demand.ObjectiveType = EPoliticalDemandObjectiveType::SugarProducerBuilding;
    Demand.Stage = EPoliticalDemandStage::Demand;
    Demand.Status = EPoliticalDemandStatus::PendingResponse;
    Demand.DurationDays = GScenarioDemandDurationDays;
    Demand.RemainingDays = GScenarioDemandDurationDays;
    Demand.TargetValue = 1;
    Demand.CurrentValue = 0;
    Demand.Title = L"패놀티모의 조언";
    Demand.SpeakerOverrideName = L"페눌티모";
    Demand.Summary =
        L"패놀티모가 조용히 다가왔습니다.\n"
        L"\"각하, 럼주는 설탕으로 만들죠.\n"
        L"설탕 농장을 먼저 지어야 증류소가 돌아갑니다.\"";
    Demand.ObjectiveText = L"설탕 농장 1개 건설";
    Demand.RewardText = L"럼주 생산 기반 확보";
    Demand.PenaltyText = L"없음";
    mOwner->mPolitics->PoliticalDemandService->InjectScenarioDemand(Demand);

    ShowEventWidget(
        1,
        L"패놀티모의 조언",
        L"패놀티모가 조용히 다가왔습니다.\n"
        L"\"각하, 럼주는 설탕으로 만들죠.\n"
        L"설탕 농장을 먼저 지어야 증류소가 돌아갑니다.\"",
        L"알겠습니다: 설탕 농장을 짓겠습니다.",
        L"나중에 생각해보죠.");
}

void CScenarioSubsystem::InjectSmugglersRumSaleDemand()
{
    if (!mOwner || !mOwner->mPolitics->PoliticalDemandService)
        return;

    FPoliticalDemandState Demand;
    Demand.Active = true;
    Demand.IssuerType = EPoliticalDemandIssuerType::ForeignPower;
    Demand.IssuerIndex = 1; // 밀수업자
    Demand.ObjectiveType = EPoliticalDemandObjectiveType::None; // 완료는 TickPhase에서 직접 판정
    Demand.Stage = EPoliticalDemandStage::Demand;
    Demand.Status = EPoliticalDemandStatus::PendingResponse;
    Demand.DurationDays = GScenarioDemandDurationDays;
    Demand.RemainingDays = GScenarioDemandDurationDays;
    Demand.TargetValue = 0;
    Demand.CurrentValue = 0;
    Demand.Title = L"밀수 판로 개척";
    Demand.Summary =
        L"밀수업자가 다시 찾아왔습니다.\n"
        L"\"증류소도 생겼겠다, 이제 럼주를 나한테 팔아봐.\n"
        L"무역로를 열고 계약한 물량을 전부 넘겨주면 돼.\"";
    Demand.ObjectiveText = L"밀수업자와 럼주 무역로 개설 후 수출 완료";
    Demand.RewardText = L"럼주 밀수 완료";
    Demand.PenaltyText = L"없음";
    mOwner->mPolitics->PoliticalDemandService->InjectScenarioDemand(Demand);

    ShowEventWidget(
        1,
        L"밀수 판로 개척",
        L"밀수업자가 다시 찾아왔습니다.\n"
        L"\"증류소도 생겼겠다, 이제 럼주를 나한테 팔아봐.\n"
        L"무역로를 열고 계약한 물량을 전부 넘겨주면 돼.\"",
        L"수락: 밀수업자와 럼주 무역로를 열고 수출합니다.",
        L"나중에 하지.");
}

void CScenarioSubsystem::InjectCrownExploitationDemand()
{
    if (!mOwner || !mOwner->mPolitics->PoliticalDemandService)
        return;

    const std::wstring PowerName = MainWorldTradeRuntime::GetForeignPowerName(
        0, mOwner->mEraState->EraProgress.CurrentEra);

    FPoliticalDemandState Demand;
    Demand.Active = true;
    Demand.IssuerType = EPoliticalDemandIssuerType::ForeignPower;
    Demand.IssuerIndex = 0; // 왕실
    Demand.ObjectiveType = EPoliticalDemandObjectiveType::None; // 완료는 TickPhase에서 직접 판정
    Demand.Stage = EPoliticalDemandStage::Demand;
    Demand.Status = EPoliticalDemandStatus::PendingResponse;
    Demand.DurationDays = GScenarioDemandDurationDays;
    Demand.RemainingDays = GScenarioDemandDurationDays;
    Demand.TargetValue = 0;
    Demand.CurrentValue = 0;
    Demand.PenaltyForeignRelationDelta = -25;
    Demand.Title = PowerName + L"의 명령";
    Demand.Summary =
        PowerName + L" 총독이 서한을 보냈습니다.\n"
        L"\"그 럼주, 우리에게만 팔아라.\n"
        L"왕실 럼주 무역로를 열고 계약 물량을 전부 넘겨라.\"\n"
        L"(주의: 수락 후 계약을 이행하는 동안 럼주 판매가격이 -50% 적용됩니다)";
    Demand.ObjectiveText =
        PowerName + L"과 럼주 무역로 개설 후 수출 완료";
    Demand.RewardText = L"왕실 관계 유지";
    Demand.PenaltyText = PowerName + L" 외교관계 -25";
    mOwner->mPolitics->PoliticalDemandService->InjectScenarioDemand(Demand);
    OpenCrownRumRoute();

    ShowEventWidget(
        0,
        PowerName + L"의 명령",
        PowerName + L" 총독이 서한을 보냈습니다.\n"
        L"\"그 럼주, 우리에게만 팔아라.\n"
        L"왕실 럼주 무역로를 열고 계약 물량을 전부 넘겨라.\"",
        L"수락: 왕실 럼주 계약을 맺고 전량 납품합니다. (가격 -50%)",
        L"거부: 왕실 외교관계 -25");
}

void CScenarioSubsystem::InjectIndependencePrepDemand()
{
    if (!mOwner || !mOwner->mPolitics->PoliticalDemandService)
        return;

    FPoliticalDemandState Demand;
    Demand.Active = true;
    Demand.IssuerType = EPoliticalDemandIssuerType::ForeignPower;
    Demand.IssuerIndex = 1; // 혁명군 측
    Demand.ObjectiveType = EPoliticalDemandObjectiveType::MilitaryWorkers;
    Demand.Stage = EPoliticalDemandStage::Demand;
    Demand.Status = EPoliticalDemandStatus::PendingResponse;
    Demand.DurationDays = GScenarioDemandDurationDays;
    Demand.RemainingDays = GScenarioDemandDurationDays;
    Demand.TargetValue = 8;
    Demand.CurrentValue = 0;
    Demand.Title = L"독립을 준비하라";
    Demand.Summary =
        L"왕실의 착취가 도를 넘었습니다.\n"
        L"패놀티모가 말합니다.\n"
        L"\"각하, 이대로 당하면 안 됩니다.\n"
        L"요새를 짓고 군사 8명을 모아 저항을 준비하십시오.\"";
    Demand.ObjectiveText = L"군사 건물 인원 8명";
    Demand.RewardText = L"독립 준비 완료";
    Demand.PenaltyText = L"없음";
    mOwner->mPolitics->PoliticalDemandService->InjectScenarioDemand(Demand);

    ShowEventWidget(
        1,
        L"독립을 준비하라",
        L"왕실의 착취가 도를 넘었습니다.\n"
        L"\"각하, 이대로 당하면 안 됩니다.\n"
        L"요새를 짓고 군사 8명을 모아 저항을 준비하십시오.\"",
        L"알겠습니다: 군사를 모읍니다.",
        L"아직 이릅니다.");
}

void CScenarioSubsystem::InjectPeacePaymentDemand()
{
    if (!mOwner || !mOwner->mPolitics->PoliticalDemandService)
        return;

    const std::wstring PowerName = MainWorldTradeRuntime::GetForeignPowerName(
        0, mOwner->mEraState->EraProgress.CurrentEra);

    FPoliticalDemandState Demand;
    Demand.Active = true;
    Demand.IssuerType = EPoliticalDemandIssuerType::ForeignPower;
    Demand.IssuerIndex = 0; // 왕실
    Demand.ObjectiveType = EPoliticalDemandObjectiveType::None;
    Demand.Stage = EPoliticalDemandStage::Demand;
    Demand.Status = EPoliticalDemandStatus::PendingResponse;
    Demand.DurationDays = GScenarioDemandDurationDays;
    Demand.RemainingDays = GScenarioDemandDurationDays;
    Demand.TargetValue = static_cast<int>(GIndependencePaymentAmount);
    Demand.CurrentValue = 0;
    Demand.Title = PowerName + L"의 협상";
    Demand.Summary =
        PowerName +
        L" 총독이 백기를 들었습니다.\n"
        L"\"우리도 더 이상 전쟁은 원치 않는다.\n"
        L"배상금 $10,000을 지불하면 독립을 인정하겠다.\"";
    Demand.ObjectiveText = L"국고 $10,000 확보 후 지불 버튼 클릭";
    Demand.RewardText = L"시나리오 승리 / 더 하고 싶다면 다음 시대로 전환";
    Demand.PenaltyText = L"없음";
    mOwner->mPolitics->PoliticalDemandService->InjectScenarioDemand(Demand);

    ShowEventWidget(
        0,
        PowerName + L"의 협상",
        PowerName +
        L" 총독이 백기를 들었습니다.\n"
        L"\"우리도 더 이상 전쟁은 원치 않는다.\n"
        L"배상금 $10,000을 지불하면 독립을 인정하겠다.\"",
        L"수락: 배상금을 모아 지불합니다.",
        L"거부: 전쟁을 계속합니다.");
}

bool CScenarioSubsystem::TryExecutePeacePayment(std::wstring& OutMessage)
{
    if (Phase != EScenarioPhase::PeacePayment || !mOwner)
    {
        OutMessage = L"배상금 지불 단계가 아닙니다.";
        return false;
    }

    if (mOwner->mEconomy->NationalBudget <
        static_cast<long long>(GIndependencePaymentAmount))
    {
        OutMessage = L"국고가 부족합니다. $10,000 이상 필요합니다.";
        return false;
    }

    std::wstring CompletionMsg;
    mOwner->mPolitics->CompletePoliticalDemand(
        EPoliticalDemandIssuerType::ForeignPower,
        0,
        CompletionMsg);
    mOwner->mEconomy->NationalBudget -= GIndependencePaymentAmount;
    if (mOwner->mEconomy->NationalBudget < 0)
        mOwner->mEconomy->NationalBudget = 0;
    ResourceTradePricing::GetScenarioRumExportBiasPercent() = 0;
    EnterPhase(EScenarioPhase::EraTransitionReady);

    mOwner->mEraState->RefreshEraTransitionState();
    OutMessage = L"시나리오 승리! 더 하고 싶다면 다음 시대로 전환하세요.";
    return true;
}

// ──────────────────────────────────────────────────────────────
//  디버그: 현재 페이즈 강제 스킵
// ──────────────────────────────────────────────────────────────

void CScenarioSubsystem::DebugSkipPhase()
{
    if (!mOwner || ResultShown)
        return;

    // 현재 페이즈의 부수 작업을 정리한 뒤 다음 페이즈로 진입한다.
    switch (Phase)
    {
    case EScenarioPhase::Intro:
        // Intro → SmugglersOffer: 부수 작업 없음
        break;

    case EScenarioPhase::SmugglersOffer:
        // TickPhase에서 조건 충족 시 OpenSmugglersRumRoute()를 호출하므로 여기서도 동일하게 처리
        if (!SmugglersRumOfferInjected)
            OpenSmugglersRumRoute();
        break;

    case EScenarioPhase::SmugglersRumSale:
    {
        SmugglersRumOfferInjected = false;
        std::wstring Msg;
        mOwner->mPolitics->CompletePoliticalDemand(
            EPoliticalDemandIssuerType::ForeignPower, 1, Msg);
        break;
    }

    case EScenarioPhase::CrownExploitation:
    {
        CrownRumExploitationActive = false;
        CrownRumOfferInjected = false;
        ResourceTradePricing::GetScenarioRumExportBiasPercent() = 0;
        if (CTradeDiplomacySubsystem* Trade = mOwner->GetTrade())
            RemoveScenarioRumOffers(*Trade, GCrownRumScenarioTag);
        std::wstring Msg;
        mOwner->mPolitics->CompletePoliticalDemand(
            EPoliticalDemandIssuerType::ForeignPower, 0, Msg);
        break;
    }

    case EScenarioPhase::IndependencePrep:
    {
        std::wstring Msg;
        mOwner->mPolitics->CompletePoliticalDemand(
            EPoliticalDemandIssuerType::ForeignPower, 1, Msg);
        break;
    }

    case EScenarioPhase::PeacePayment:
        // TryExecutePeacePayment가 예산을 체크하므로 직접 처리
        ResourceTradePricing::GetScenarioRumExportBiasPercent() = 0;
        EnterPhase(EScenarioPhase::EraTransitionReady);
        mOwner->mEraState->RefreshEraTransitionState();
        mOwner->mEraState->TryExecuteEraTransition(EEraTransitionChoice::Confirm);
        return; // EnterPhase 이미 완료

    case EScenarioPhase::EraTransitionReady:
        // 이미 마지막 페이즈
        return;

    default:
        break;
    }

    const int Next = static_cast<int>(Phase) + 1;
    if (Next <= static_cast<int>(EScenarioPhase::EraTransitionReady))
        EnterPhase(static_cast<EScenarioPhase>(Next));
}

// ──────────────────────────────────────────────────────────────
//  Phase 전환
// ──────────────────────────────────────────────────────────────

void CScenarioSubsystem::EnterPhase(EScenarioPhase NewPhase)
{
    Phase = NewPhase;

    switch (NewPhase)
    {
    case EScenarioPhase::SmugglersOffer:
        InjectSmugglersOfferDemand();
        break;

    case EScenarioPhase::PenultimoFarm:
        InjectPenultimoFarmDemand();
        break;

    case EScenarioPhase::SmugglersRumSale:
        InjectSmugglersRumSaleDemand();
        break;

    case EScenarioPhase::CrownExploitation:
        InjectCrownExploitationDemand();
        break;

    case EScenarioPhase::IndependencePrep:
        InjectIndependencePrepDemand();
        break;

    case EScenarioPhase::PeacePayment:
        InjectPeacePaymentDemand();
        break;

    case EScenarioPhase::EraTransitionReady:
        EraTransitionUnlocked = true;
        break;

    default:
        break;
    }
}

// ──────────────────────────────────────────────────────────────
//  매 틱 Phase 진행 체크
// ──────────────────────────────────────────────────────────────

void CScenarioSubsystem::TickPhase()
{
    if (!mOwner || ResultShown)
        return;

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
        return;

    // PeakSupportPercent 갱신
    const double CurrentSupport =
        mOwner->mPolitics->PoliticalSnapshot.AverageSupportScore;
    if (CurrentSupport > PeakSupportPercent)
        PeakSupportPercent = CurrentSupport;

    const auto& ForeignDemands =
        mOwner->mPolitics->PoliticalDemandService->GetForeignDemandStates();

    const auto IsDemandAccepted = [&](int IssuerIndex) -> bool
    {
        if (IssuerIndex < 0 ||
            IssuerIndex >=
                static_cast<int>(ForeignDemands.size()))
        {
            return false;
        }
        return IsPoliticalDemandAccepted(
            ForeignDemands[static_cast<size_t>(IssuerIndex)]);
    };

    const auto IsDemandSatisfied = [&](int IssuerIndex) -> bool
    {
        if (IssuerIndex < 0 ||
            IssuerIndex >=
                static_cast<int>(ForeignDemands.size()))
        {
            return false;
        }
        const FPoliticalDemandState& D =
            ForeignDemands[static_cast<size_t>(IssuerIndex)];
        return D.Active && D.CurrentValue >= D.TargetValue;
    };

    switch (Phase)
    {
    case EScenarioPhase::Intro:
        // 게임 시작 직후 SmugglersOffer로 진입
        EnterPhase(EScenarioPhase::SmugglersOffer);
        break;

    case EScenarioPhase::SmugglersOffer:
    {
        // TickPhase가 TickPoliticalDemands보다 먼저 실행된다.
        // 수요 상태(D.CurrentValue)에 의존하지 않고 실제 월드 스냅샷으로 직접 검사.
        const FPoliticalDemandState& D1 = ForeignDemands[1];
        const bool ReadyToCheck =
            IsDemandAccepted(1) || // 아직 Active+Accepted 상태
            !D1.Active;            // 수요가 만료되거나 완료로 초기화된 경우
        if (ReadyToCheck)
        {
            const WorldStats::FWorldStatsSnapshot Snap =
                WorldStats::BuildSnapshot(World);
            if (Snap.ResourceTypes[
                    static_cast<size_t>(EResourceType::Rum)]
                    .ProducerBuildingCount >= 1)
            {
                OpenSmugglersRumRoute();
                EnterPhase(EScenarioPhase::PenultimoFarm);
            }
        }
        break;
    }

    case EScenarioPhase::PenultimoFarm:
    {
        // SmugglersOffer와 동일한 레이스 컨디션 패턴 방지
        const FPoliticalDemandState& D1b = ForeignDemands[2];
        const bool ReadyToCheckFarm =
            IsDemandAccepted(2) ||
            !D1b.Active;
        if (ReadyToCheckFarm)
        {
            const WorldStats::FWorldStatsSnapshot Snap =
                WorldStats::BuildSnapshot(World);
            if (Snap.ResourceTypes[
                    static_cast<size_t>(EResourceType::Sugar)]
                    .ProducerBuildingCount >= 1)
            {
                EnterPhase(EScenarioPhase::SmugglersRumSale);
            }
        }
        break;
    }

    case EScenarioPhase::SmugglersRumSale:
    {
        bool SmugglersRouteCompleted = false;
        if (CTradeDiplomacySubsystem* Trade = mOwner->GetTrade())
        {
            SmugglersRouteCompleted =
                HasCompletedScenarioRumRoute(
                    *Trade,
                    1,
                    GSmugglersRumScenarioTag);
        }
        if (SmugglersRouteCompleted)
        {
            std::wstring CompletionMessage;
            mOwner->mPolitics->CompletePoliticalDemand(
                EPoliticalDemandIssuerType::ForeignPower,
                1,
                CompletionMessage);
            SmugglersRumOfferInjected = false;
            EnterPhase(EScenarioPhase::CrownExploitation);
        }
        break;
    }

    case EScenarioPhase::CrownExploitation:
        if (ForeignDemands[0].Active)
        {
            if (!CrownRumExploitationActive)
            {
                const bool DemandAccepted = IsDemandAccepted(0);
                CrownRumExploitationActive = DemandAccepted;
                ResourceTradePricing::GetScenarioRumExportBiasPercent() =
                    DemandAccepted ? -50 : 0;
                RefreshScenarioWorldMarketPrices();
            }

            CTradeDiplomacySubsystem* Trade = mOwner->GetTrade();
            if (Trade &&
                HasCompletedScenarioRumRoute(
                    *Trade,
                    0,
                    GCrownRumScenarioTag))
            {
                std::wstring CompletionMessage;
                mOwner->mPolitics->CompletePoliticalDemand(
                    EPoliticalDemandIssuerType::ForeignPower,
                    0,
                    CompletionMessage);
                CrownRumOfferInjected = false;
                CrownRumExploitationActive = false;
                ResourceTradePricing::GetScenarioRumExportBiasPercent() = 0;
                RefreshScenarioWorldMarketPrices();
                EnterPhase(EScenarioPhase::IndependencePrep);
            }
        }
        else if (CrownRumExploitationActive && !ForeignDemands[0].Active)
        {
            CrownRumExploitationActive = false;
            CrownRumOfferInjected = false;
            ResourceTradePricing::GetScenarioRumExportBiasPercent() = 0;
            RefreshScenarioWorldMarketPrices();
            if (CTradeDiplomacySubsystem* Trade = mOwner->GetTrade())
                RemoveScenarioRumOffers(*Trade, GCrownRumScenarioTag);
        }
        else if (!ForeignDemands[0].Active)
        {
            CrownRumOfferInjected = false;
            if (CTradeDiplomacySubsystem* Trade = mOwner->GetTrade())
                RemoveScenarioRumOffers(*Trade, GCrownRumScenarioTag);
        }
        break;

    case EScenarioPhase::IndependencePrep:
    {
        const FPoliticalDemandState& D1c = ForeignDemands[1];
        const bool ReadyToCheckMilitary =
            IsDemandAccepted(1) ||
            !D1c.Active;
        if (ReadyToCheckMilitary)
        {
            const WorldStats::FWorldStatsSnapshot Snap =
                WorldStats::BuildSnapshot(World);
            if (Snap.MilitaryWorkerCount >= 8)
            {
                if (D1c.Active)
                {
                    std::wstring CompletionMessage;
                    mOwner->mPolitics->CompletePoliticalDemand(
                        EPoliticalDemandIssuerType::ForeignPower,
                        1,
                        CompletionMessage);
                }

                EnterPhase(EScenarioPhase::PeacePayment);
            }
        }
        break;
    }

    case EScenarioPhase::PeacePayment:
        // 지불은 TryExecutePeacePayment()에서 사용자가 버튼 클릭 시 처리
        break;

    case EScenarioPhase::EraTransitionReady:
        // EraTransitionUnlocked = true 이므로 EraSubsystem이 전환 허용
        break;
    }

    // ──────────────────────────────────────────────────────────────
    // 시나리오 럼주 제안 유지: 연간 무역 갱신이 AvailableTradeOffers를 지워도
    // 목표 무역로가 완료될 때까지 제안을 다시 넣어준다.
    // ──────────────────────────────────────────────────────────────
    const auto MaintainScenarioRumOffer =
        [&](bool& InOutOfferInjected,
            int ForeignPowerIndex,
            int ScenarioTag,
            auto OpenRoute)
        {
            if (!InOutOfferInjected || !mOwner)
                return;

            CTradeDiplomacySubsystem* Trade = mOwner->GetTrade();

            if (!Trade)
                return;

            if (HasCompletedScenarioRumRoute(
                    *Trade,
                    ForeignPowerIndex,
                    ScenarioTag))
            {
                InOutOfferInjected = false;
                return;
            }

            if (HasActiveScenarioRumRoute(*Trade, ScenarioTag) ||
                HasScenarioRumOffer(*Trade, ScenarioTag))
            {
                return;
            }

            OpenRoute();
        };

    if (SmugglersRumOfferInjected)
    {
        MaintainScenarioRumOffer(
            SmugglersRumOfferInjected,
            1,
            GSmugglersRumScenarioTag,
            [this]()
            {
                OpenSmugglersRumRoute();
            });
    }

    if (CrownRumOfferInjected)
    {
        MaintainScenarioRumOffer(
            CrownRumOfferInjected,
            0,
            GCrownRumScenarioTag,
            [this]()
            {
                OpenCrownRumRoute();
            });
    }
}

// ──────────────────────────────────────────────────────────────
//  기존 인터페이스
// ──────────────────────────────────────────────────────────────

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

    Phase = EScenarioPhase::Intro;
    EraTransitionUnlocked = false;
    CrownRumExploitationActive = false;
    ResourceTradePricing::GetScenarioRumExportBiasPercent() = 0;

    InitialBuildingCount = 0;
    PeakSupportPercent = 0.0;
    ResultShown = false;
    ScenarioElectionPromptPending = false;
    SmugglersRumOfferInjected = false;
    CrownRumOfferInjected = false;
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

void CScenarioSubsystem::NotifyEraTransitioned(EBuildingEra NewEra)
{
    if (!mOwner || ResultShown)
        return;

    if (NewEra == EBuildingEra::WorldWars)
        ShowResultWidget(true);
}

void CScenarioSubsystem::ShowResultWidget(bool Victory)
{
    if (!mOwner || ResultShown)
        return;

    auto UiManager = mOwner->GetUIManager().lock();
    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!UiManager || !World)
        return;

    auto ResultWidgetPtr =
        UiManager->FindWidget<CResultWidget>(GResultWidgetName).lock();

    if (!ResultWidgetPtr)
        return;

    if (auto EventWidget =
            UiManager->FindWidget<CEventWidget>(GEventWidgetName).lock())
    {
        EventWidget->GetMutableState().Visible = false;
    }

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
    FResultWidgetState& State = ResultWidgetPtr->GetMutableState();
    State.Visible = true;
    State.Victory = Victory;

    if (Victory)
    {
        State.Title = L"독립 선언!";
        State.Summary = L"럼주 밀수의 이익으로 트로피코는 마침내\u3000독립을 선언했습니다.";
        State.DetailPrimary =
            L"독립 선언 연도: " +
            std::to_wstring(mOwner->mSimulation->Year) + L"년";
        State.DetailSecondary = TenureText;
        State.DetailTertiary =
            L"임기 중 건설: " + std::to_wstring(BuildingsBuilt) + L"개 건물";
        State.DetailQuaternary =
            L"최고 지지율: " + FormatPercentText(PeakSupportPercent, 0);
    }
    else
    {
        State.Title = L"쿠데타 발생";
        State.Summary = L"지지율 붕괴로 군부가 관저를 점령했습니다.";
        State.DetailPrimary =
            L"최종 지지율: " +
            FormatPercentText(
                mOwner->mPolitics->PoliticalSnapshot.AverageSupportScore, 0);
        State.DetailSecondary = TenureText;
        State.DetailTertiary =
            L"임기 중 건설: " + std::to_wstring(BuildingsBuilt) + L"개 건물";
        State.DetailQuaternary =
            L"최고 지지율: " + FormatPercentText(PeakSupportPercent, 0);
    }

    mOwner->mSimulation->Paused = true;
    ResultShown = true;

    (void)ElectionStatus;
}
