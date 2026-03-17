#include "TopHudWidget.h"
#include "TopHudDataProvider.h"
#include "TopHudRenderer.h"
#include "AlmanacWidget.h"
#include "BuildMenuWidget.h"
#include "EdictWidget.h"
#include "TradeWidget.h"
#include "UIStrings.h"
#include "UI/Button.h"
#include "UI/TextBlock.h"
#include "../Politics/ConstitutionSystem.h"
#include "../ObjectNames.h"
#include "../World/MainWorldAccess.h"
#include "World/World.h"
#include "World/WorldUIManager.h"

namespace
{
    const wchar_t* GetConstitutionTopicDisplayName(EConstitutionTopic Topic)
    {
        switch (Topic)
        {
        case EConstitutionTopic::VotingRights:
            return UIStrings::Get(
                L"constitution.topic.voting_rights").c_str();
        case EConstitutionTopic::LaborPolicy:
            return UIStrings::Get(
                L"constitution.topic.labor_policy").c_str();
        case EConstitutionTopic::ReligionAndState:
            return UIStrings::Get(
                L"constitution.topic.religion_and_state").c_str();
        case EConstitutionTopic::MediaIndependence:
            return UIStrings::Get(
                L"constitution.topic.media_independence").c_str();
        case EConstitutionTopic::ArmedForces:
            return UIStrings::Get(
                L"constitution.topic.armed_forces").c_str();
        case EConstitutionTopic::Count:
        default:
            return UIStrings::Get(
                L"constitution.topic.default").c_str();
        }
    }

    const FConstitutionOptionDef* FindConstitutionOptionDef(
        EConstitutionOptionId Id)
    {
        const auto& Catalog = ConstitutionSystem::GetConstitutionOptionCatalog();

        for (size_t Index = 0; Index < Catalog.size(); ++Index)
        {
            if (Catalog[Index].Id == Id)
                return &Catalog[Index];
        }

        return nullptr;
    }

    bool ResolveConstitutionOptionsForTopic(
        EConstitutionTopic Topic,
        EConstitutionOptionId& OutLeftOptionId,
        EConstitutionOptionId& OutRightOptionId)
    {
        OutLeftOptionId = EConstitutionOptionId::None;
        OutRightOptionId = EConstitutionOptionId::None;

        const auto& Catalog = ConstitutionSystem::GetConstitutionOptionCatalog();

        for (size_t Index = 0; Index < Catalog.size(); ++Index)
        {
            if (Catalog[Index].Topic != Topic)
                continue;

            if (OutLeftOptionId == EConstitutionOptionId::None)
            {
                OutLeftOptionId = Catalog[Index].Id;
                continue;
            }

            OutRightOptionId = Catalog[Index].Id;
            break;
        }

        return OutLeftOptionId != EConstitutionOptionId::None &&
            OutRightOptionId != EConstitutionOptionId::None;
    }

    std::wstring BuildConstitutionPopupBody(
        EConstitutionTopic Topic,
        const FConstitutionOptionDef* LeftOption,
        const FConstitutionOptionDef* RightOption)
    {
        std::wstring Result =
            std::wstring(GetConstitutionTopicDisplayName(Topic)) +
            L" " +
            UIStrings::Get(L"constitution.popup.body_prompt") +
            L"\n\n";

        if (LeftOption)
        {
            Result += LeftOption->DisplayName;

            if (!LeftOption->EffectSummary.empty())
            {
                Result += L"\n";
                Result += LeftOption->EffectSummary;
            }
        }

        Result += L"\n\n";

        if (RightOption)
        {
            Result += RightOption->DisplayName;

            if (!RightOption->EffectSummary.empty())
            {
                Result += L"\n";
                Result += RightOption->EffectSummary;
            }
        }

        return Result;
    }
}

CTopHudWidget::CTopHudWidget()
{
}

CTopHudWidget::~CTopHudWidget()
{
}

bool CTopHudWidget::Init()
{
    CWidgetContainer::Init();

    mMonthProgress = 0.f;
    mGameLost = false;
    mGameOverMenusClosed = false;
    mManualEraTransitionPopupOpen = false;
    mConstitutionPopupActive = false;
    mEraTransitionPopupOpen = false;
    mConstitutionConfirmOptionId = EConstitutionOptionId::None;
    mConstitutionCancelOptionId = EConstitutionOptionId::None;

    FTopHudRenderer::CreateWidgets(*this);
    RefreshFromState();
    return true;
}

void CTopHudWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
    RefreshFromState();
}

void CTopHudWidget::RefreshFromState()
{
    const auto Snapshot = TopHudDataProvider::BuildSnapshot(mWorld.lock());
    const bool WasConstitutionPopupActive = mConstitutionPopupActive;
    mConstitutionPopupActive = false;
    mConstitutionConfirmOptionId = EConstitutionOptionId::None;
    mConstitutionCancelOptionId = EConstitutionOptionId::None;

    auto ConstitutionAccess =
        ResolveMainWorldConstitutionAccess(mWorld.lock());

    if (ConstitutionAccess)
    {
        const FConstitutionState& ConstitutionState =
            ConstitutionAccess->GetConstitutionState();

        if (ConstitutionState.PendingTopicChoice &&
            ResolveConstitutionOptionsForTopic(
                ConstitutionState.PendingTopic,
                mConstitutionConfirmOptionId,
                mConstitutionCancelOptionId))
        {
            mConstitutionPopupActive = true;
            mManualEraTransitionPopupOpen = false;
        }
    }

    if (mConstitutionPopupActive && !WasConstitutionPopupActive)
        CloseMenus(true, true, true, true);

    mEraTransitionPopupOpen =
        mManualEraTransitionPopupOpen || mConstitutionPopupActive;

    if (Snapshot.GameLost && !mGameOverMenusClosed)
    {
        CloseMenus(true, true, true, true);
        mGameOverMenusClosed = true;
        mManualEraTransitionPopupOpen = false;
        mConstitutionPopupActive = false;
        mEraTransitionPopupOpen = false;
    }
    else if (!Snapshot.GameLost)
    {
        mGameOverMenusClosed = false;
    }

    FTopHudRenderer::ApplySnapshot(*this, Snapshot);

    if (mConstitutionPopupActive)
    {
        const FConstitutionState& ConstitutionState =
            ConstitutionAccess->GetConstitutionState();
        const FConstitutionOptionDef* const LeftOption =
            FindConstitutionOptionDef(mConstitutionConfirmOptionId);
        const FConstitutionOptionDef* const RightOption =
            FindConstitutionOptionDef(mConstitutionCancelOptionId);
        auto TitleText = mEraTransitionTitleText.lock();
        auto BodyText = mEraTransitionBodyText.lock();
        auto ConfirmButton = mEraTransitionConfirmButton.lock();
        auto CancelButton = mEraTransitionCancelButton.lock();
        auto ConfirmButtonText = mEraTransitionConfirmButtonText.lock();
        auto CancelButtonText = mEraTransitionCancelButtonText.lock();

        if (TitleText)
            TitleText->SetText(
                (UIStrings::Get(L"constitution.popup.title_prefix") +
                    GetConstitutionTopicDisplayName(
                        ConstitutionState.PendingTopic))
                .c_str());

        if (BodyText)
            BodyText->SetText(
                BuildConstitutionPopupBody(
                    ConstitutionState.PendingTopic,
                    LeftOption,
                    RightOption)
                .c_str());

        if (ConfirmButton)
        {
            ConfirmButton->SetEnable(true);
            ConfirmButton->ButtonEnable(Snapshot.CanUseButtons);
        }

        if (CancelButton)
        {
            CancelButton->SetEnable(true);
            CancelButton->ButtonEnable(Snapshot.CanUseButtons);
        }

        if (ConfirmButtonText && LeftOption)
            ConfirmButtonText->SetText(LeftOption->DisplayName.c_str());

        if (CancelButtonText && RightOption)
            CancelButtonText->SetText(RightOption->DisplayName.c_str());
    }

    FTopHudRenderer::RefreshLayout(*this);
}

void CTopHudWidget::CloseMenus(
    bool CloseBuildMenu,
    bool CloseAlmanac,
    bool CloseEdicts,
    bool CloseTrade)
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto BuildMenu =
        UIManager->FindWidget<CBuildMenuWidget>(GBuildMenuWidgetName).lock();
    auto AlmanacWidget =
        UIManager->FindWidget<CAlmanacWidget>(GAlmanacWidgetName).lock();
    auto EdictWidget =
        UIManager->FindWidget<CEdictWidget>(GEdictWidgetName).lock();
    auto TradeWidget =
        UIManager->FindWidget<CTradeWidget>(GTradeWidgetName).lock();

    if (BuildMenu)
    {
        if (CloseBuildMenu)
            BuildMenu->SetBuildMenuOpen(false);

        if (CloseAlmanac)
            BuildMenu->SetAlmanacOpen(false);
    }

    if (AlmanacWidget && CloseAlmanac)
        AlmanacWidget->SetOpen(false);

    if (EdictWidget && CloseEdicts)
        EdictWidget->SetOpen(false);

    if (TradeWidget && CloseTrade)
        TradeWidget->SetOpen(false);
}

void CTopHudWidget::OnConstructionButtonClick()
{
    if (mGameLost)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    CloseMenus(false, true, true, true);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto BuildMenu =
        UIManager->FindWidget<CBuildMenuWidget>(GBuildMenuWidgetName).lock();

    if (BuildMenu)
        BuildMenu->ToggleBuildMenu();
}

void CTopHudWidget::OnEraTransitionButtonClick()
{
    if (mGameLost)
        return;

    auto HudAccess =
        ResolveMainWorldHudAccess(mWorld.lock());

    if (!HudAccess || mConstitutionPopupActive)
        return;

    mManualEraTransitionPopupOpen = !mManualEraTransitionPopupOpen;
    mEraTransitionPopupOpen =
        mManualEraTransitionPopupOpen || mConstitutionPopupActive;
    RefreshFromState();
}

void CTopHudWidget::OnAlmanacButtonClick()
{
    if (mGameLost)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    CloseMenus(true, false, true, true);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto AlmanacWidget =
        UIManager->FindWidget<CAlmanacWidget>(GAlmanacWidgetName).lock();

    if (AlmanacWidget)
        AlmanacWidget->ToggleOpen();
}

void CTopHudWidget::OnEdictsButtonClick()
{
    if (mGameLost)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    CloseMenus(true, true, false, true);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto EdictWidget =
        UIManager->FindWidget<CEdictWidget>(GEdictWidgetName).lock();

    if (EdictWidget)
        EdictWidget->ToggleOpen();
}

void CTopHudWidget::OnTradeButtonClick()
{
    if (mGameLost)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    CloseMenus(true, true, true, false);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto TradeWidget =
        UIManager->FindWidget<CTradeWidget>(GTradeWidgetName).lock();

    if (TradeWidget)
        TradeWidget->ToggleOpen();
}

void CTopHudWidget::OnEraTransitionConfirmButtonClick()
{
    if (mGameLost)
        return;

    if (mConstitutionPopupActive)
    {
        auto ConstitutionAccess =
            ResolveMainWorldConstitutionAccess(mWorld.lock());

        if (ConstitutionAccess &&
            mConstitutionConfirmOptionId != EConstitutionOptionId::None)
        {
            ConstitutionAccess->TrySelectConstitutionOption(
                mConstitutionConfirmOptionId);
        }

        RefreshFromState();
        return;
    }

    auto HudAccess =
        ResolveMainWorldHudAccess(mWorld.lock());

    if (!HudAccess)
        return;

    if (HudAccess->TryExecuteEraTransition(EEraTransitionChoice::Confirm))
        mManualEraTransitionPopupOpen = false;

    mEraTransitionPopupOpen =
        mManualEraTransitionPopupOpen || mConstitutionPopupActive;

    RefreshFromState();
}

void CTopHudWidget::OnEraTransitionCancelButtonClick()
{
    if (mConstitutionPopupActive)
    {
        auto ConstitutionAccess =
            ResolveMainWorldConstitutionAccess(mWorld.lock());

        if (ConstitutionAccess &&
            mConstitutionCancelOptionId != EConstitutionOptionId::None)
        {
            ConstitutionAccess->TrySelectConstitutionOption(
                mConstitutionCancelOptionId);
        }

        RefreshFromState();
        return;
    }

    mManualEraTransitionPopupOpen = false;
    mEraTransitionPopupOpen = false;
    RefreshFromState();
}

void CTopHudWidget::OnSpeedStateButtonClick()
{
    if (mGameLost)
        return;

    auto HudAccess =
        ResolveMainWorldHudAccess(mWorld.lock());

    if (!HudAccess)
        return;

    HudAccess->ToggleSimulationPaused();
    RefreshFromState();
}

void CTopHudWidget::OnSpeedMultiplierButtonClick()
{
    if (mGameLost)
        return;

    auto HudAccess =
        ResolveMainWorldHudAccess(mWorld.lock());

    if (!HudAccess)
        return;

    HudAccess->CycleSimulationSpeedMultiplier();
    RefreshFromState();
}

void CTopHudWidget::OnAnyButtonClick()
{
}
