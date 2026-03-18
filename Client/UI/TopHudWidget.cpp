#include "TopHudWidget.h"
#include "TopHudDataProvider.h"
#include "TopHudRenderer.h"
#include "AlmanacWidget.h"
#include "BuildMenuWidget.h"
#include "EdictWidget.h"
#include "TaskWidget.h"
#include "TradeWidget.h"
#include "UIStrings.h"
#include "UI/Button.h"
#include "UI/TextBlock.h"
#include "../Politics/ConstitutionSystem.h"
#include "../ObjectNames.h"
#include "../World/IWorldUIAccess.h"
#include "World/World.h"
#include "World/WorldUIManager.h"
#include <Windows.h>
#include <sstream>

namespace
{
#ifdef _DEBUG
    void OutputConstitutionUiTrace(const std::string& Text)
    {
        OutputDebugStringA("[ConstitutionUI] ");
        OutputDebugStringA(Text.c_str());
        OutputDebugStringA("\n");
    }

    void TraceConstitutionButtonHandler(
        const char* HandlerName,
        bool PopupActive,
        EConstitutionOptionId OptionId)
    {
        std::ostringstream Stream;
        Stream << (HandlerName ? HandlerName : "handler")
            << " popupActive=" << (PopupActive ? 1 : 0)
            << " optionId=" << static_cast<int>(OptionId);
        OutputConstitutionUiTrace(Stream.str());
    }

    const char* GetDebugEraName(EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::WorldWars:
            return "WorldWars";
        case EBuildingEra::ColdWar:
            return "ColdWar";
        case EBuildingEra::Modern:
            return "Modern";
        default:
            return "OtherEra";
        }
    }

    const char* GetDebugTopicName(EConstitutionTopic Topic)
    {
        switch (Topic)
        {
        case EConstitutionTopic::VotingRights:
            return "VotingRights";
        case EConstitutionTopic::LaborPolicy:
            return "LaborPolicy";
        case EConstitutionTopic::ReligionAndState:
            return "ReligionAndState";
        case EConstitutionTopic::MediaIndependence:
            return "MediaIndependence";
        case EConstitutionTopic::ArmedForces:
            return "ArmedForces";
        default:
            return "UnknownTopic";
        }
    }
#endif

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
    mConstitutionLeftOptionId = EConstitutionOptionId::None;
    mConstitutionRightOptionId = EConstitutionOptionId::None;

    FTopHudRenderer::CreateWidgets(*this);
    RefreshFromState();
    return true;
}

void CTopHudWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
    RefreshFromState();
}

void CTopHudWidget::OpenTaskWidgetForDemand(
    EPoliticalDemandIssuerType IssuerType,
    int IssuerIndex)
{
    if (mGameLost)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    CloseMenus(true, true, true, true, false);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto TaskWidget =
        UIManager->FindWidget<CTaskWidget>(GTaskWidgetName).lock();

    if (TaskWidget)
        TaskWidget->OpenForDemand(IssuerType, IssuerIndex);
}

void CTopHudWidget::RefreshFromState()
{
    auto World = mWorld.lock();
    const auto Snapshot = TopHudDataProvider::BuildSnapshot(World);
    const bool WasConstitutionPopupActive = mConstitutionPopupActive;
    mConstitutionPopupActive = false;
    mConstitutionLeftOptionId = EConstitutionOptionId::None;
    mConstitutionRightOptionId = EConstitutionOptionId::None;

    auto* Access = ResolveWorldUIAccess(World.get());

    if (Access)
    {
        const FConstitutionState& ConstitutionState =
            Access->Read().GetConstitutionState();

        if (ConstitutionState.PendingTopicChoice &&
            ResolveConstitutionOptionsForTopic(
                ConstitutionState.PendingTopic,
                mConstitutionLeftOptionId,
                mConstitutionRightOptionId))
        {
            mConstitutionPopupActive = true;
            mManualEraTransitionPopupOpen = false;
        }
    }

    if (mConstitutionPopupActive && !WasConstitutionPopupActive)
        CloseMenus(true, true, true, true, true);

    mEraTransitionPopupOpen =
        mManualEraTransitionPopupOpen || mConstitutionPopupActive;

    if (Snapshot.GameLost && !mGameOverMenusClosed)
    {
        CloseMenus(true, true, true, true, true);
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
            Access->Read().GetConstitutionState();
        const FConstitutionOptionDef* const LeftOption =
            FindConstitutionOptionDef(mConstitutionLeftOptionId);
        const FConstitutionOptionDef* const RightOption =
            FindConstitutionOptionDef(mConstitutionRightOptionId);
        auto TitleText = mEraTransitionTitleText.lock();
        auto BodyText = mEraTransitionBodyText.lock();
        auto LeftButton = mConstitutionLeftButton.lock();
        auto RightButton = mConstitutionRightButton.lock();
        auto LeftButtonText = mConstitutionLeftButtonText.lock();
        auto RightButtonText = mConstitutionRightButtonText.lock();

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

        if (LeftButton)
        {
            LeftButton->SetEnable(true);
            LeftButton->ButtonEnable(Snapshot.CanUseButtons);
        }

        if (RightButton)
        {
            RightButton->SetEnable(true);
            RightButton->ButtonEnable(Snapshot.CanUseButtons);
        }

        if (LeftButtonText && LeftOption)
            LeftButtonText->SetText(LeftOption->DisplayName.c_str());

        if (RightButtonText && RightOption)
            RightButtonText->SetText(RightOption->DisplayName.c_str());
    }

    FTopHudRenderer::RefreshLayout(*this);
}

#ifdef _DEBUG
bool CTopHudWidget::DebugValidateCurrentConstitutionRightButton(
    const ConstitutionSystem::FDebugValidationStep& Step,
    std::string& OutMessage)
{
    OutMessage.clear();

    auto World = mWorld.lock();
    auto UIManager = World ? World->GetUIManager().lock() : nullptr;
    auto* Access = ResolveWorldUIAccess(World.get());
    auto RightButton = mConstitutionRightButton.lock();
    auto RightButtonText = mConstitutionRightButtonText.lock();

    bool Success = true;
    std::ostringstream Stream;
    auto Fail =
        [&](const std::string& Reason)
        {
            if (!Stream.str().empty())
                Stream << " | ";

            Stream << Reason;
            Success = false;
        };

    if (!World)
        Fail("world_unavailable");

    if (!UIManager)
        Fail("ui_manager_unavailable");

    if (!Access)
        Fail("constitution_access_unavailable");

    if (!RightButton)
        Fail("right_button_missing");

    const FConstitutionState* BeforeState =
        Access ? &Access->Read().GetConstitutionState() :
        nullptr;

    if (!mConstitutionPopupActive)
        Fail("constitution_popup_inactive");

    if (!BeforeState || !BeforeState->PendingTopicChoice)
    {
        Fail("pending_topic_missing");
    }
    else if (BeforeState->PendingTopic != Step.Topic)
    {
        Fail(
            std::string("pending_topic_mismatch expected=") +
            GetDebugTopicName(Step.Topic) +
            " actual=" +
            GetDebugTopicName(BeforeState->PendingTopic));
    }

    if (mConstitutionRightOptionId != Step.RightOptionId)
    {
        Fail(
            std::string("right_option_mismatch expected=") +
            std::to_string(static_cast<int>(Step.RightOptionId)) +
            " actual=" +
            std::to_string(static_cast<int>(mConstitutionRightOptionId)));
    }

    const FConstitutionOptionDef* const RightOption =
        FindConstitutionOptionDef(Step.RightOptionId);

    if (!RightOption)
        Fail("right_option_definition_missing");

    if (RightButton)
    {
        if (!RightButton->GetEnable())
            Fail("right_button_disabled");

        const FVector3 ButtonPos = RightButton->GetPos();
        const FVector3 ButtonSize = RightButton->GetSize();
        const FVector2 HoverPoint(
            ButtonPos.x + ButtonSize.x * 0.5f,
            ButtonPos.y + ButtonSize.y * 0.5f);

        if (UIManager)
        {
            UIManager->CollisionMouse(0.f, HoverPoint);
            auto HoveredWidget = UIManager->GetHoveredWidget().lock();

            if (HoveredWidget != RightButton)
            {
                Fail(
                    std::string("hover_target_mismatch actual=") +
                    "Widget." +
                    UIManager->BuildWidgetPath(HoveredWidget));
            }
        }

        if (!RightButton->GetMouseOn())
            Fail("right_button_hover_missing");
    }

    if (RightButtonText && RightOption &&
        RightButtonText->GetText() != RightOption->DisplayName)
    {
        Fail("right_button_label_mismatch");
    }

    const int HandlerEntryCountBefore = mDebugPopupRightHandlerEntryCount;
    OnPopupRightButtonClick();

    if (mDebugPopupRightHandlerEntryCount != HandlerEntryCountBefore + 1 ||
        mDebugLastPopupRightHandlerOptionId != Step.RightOptionId)
    {
        Fail("right_button_click_handler_not_observed");
    }

    const FConstitutionState* AfterState =
        Access ? &Access->Read().GetConstitutionState() :
        nullptr;

    if (!AfterState)
    {
        Fail("post_click_state_unavailable");
    }
    else
    {
        const size_t TopicIndex = static_cast<size_t>(Step.Topic);

        if (AfterState->SelectedOptions[TopicIndex] != Step.RightOptionId)
            Fail("selected_option_not_applied");

        if (AfterState->QueuedTopics[TopicIndex])
            Fail("resolved_topic_still_queued");

        if (Step.ExpectPendingAfterSelection)
        {
            if (!AfterState->PendingTopicChoice)
            {
                Fail("next_topic_not_activated");
            }
            else if (AfterState->PendingTopic !=
                Step.ExpectedNextPendingTopic)
            {
                Fail(
                    std::string("next_topic_mismatch expected=") +
                    GetDebugTopicName(Step.ExpectedNextPendingTopic) +
                    " actual=" +
                    GetDebugTopicName(AfterState->PendingTopic));
            }
        }
        else if (AfterState->PendingTopicChoice)
        {
            Fail("pending_topic_choice_not_cleared");
        }
    }

    if (Success)
    {
        Stream << "ok era=" << GetDebugEraName(Step.TriggerEra)
            << " topic=" << GetDebugTopicName(Step.Topic)
            << " rightOption=" << static_cast<int>(Step.RightOptionId);

        if (Step.ExpectPendingAfterSelection)
        {
            Stream << " next=" <<
                GetDebugTopicName(Step.ExpectedNextPendingTopic);
        }
        else
        {
            Stream << " next=<none>";
        }
    }

    OutMessage = Stream.str();
    return Success;
}
#endif

void CTopHudWidget::CloseMenus(
    bool CloseBuildMenu,
    bool CloseAlmanac,
    bool CloseEdicts,
    bool CloseTrade,
    bool CloseTask)
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
    auto TaskWidget =
        UIManager->FindWidget<CTaskWidget>(GTaskWidgetName).lock();
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

    if (TaskWidget && CloseTask)
        TaskWidget->SetOpen(false);

    if (TradeWidget && CloseTrade)
        TradeWidget->SetOpen(false);
}

void CTopHudWidget::OnTaskButtonClick()
{
    if (mGameLost)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    CloseMenus(true, true, true, true, false);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto TaskWidget =
        UIManager->FindWidget<CTaskWidget>(GTaskWidgetName).lock();

    if (TaskWidget)
        TaskWidget->ToggleOpen();
}

void CTopHudWidget::OnConstructionButtonClick()
{
    if (mGameLost)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    CloseMenus(false, true, true, true, true);

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

    auto* Access = ResolveWorldUIAccess(mWorld.lock().get());

    if (!Access || mConstitutionPopupActive)
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

    CloseMenus(true, false, true, true, true);

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

    CloseMenus(true, true, false, true, true);

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

    CloseMenus(true, true, true, false, true);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto TradeWidget =
        UIManager->FindWidget<CTradeWidget>(GTradeWidgetName).lock();

    if (TradeWidget)
        TradeWidget->ToggleOpen();
}

void CTopHudWidget::OnPopupRightButtonClick()
{
    if (mGameLost)
        return;

#ifdef _DEBUG
    ++mDebugPopupRightHandlerEntryCount;
    mDebugLastPopupRightHandlerOptionId = mConstitutionRightOptionId;
    TraceConstitutionButtonHandler(
        "popup_right_handler_enter",
        mConstitutionPopupActive,
        mConstitutionRightOptionId);
#endif

    auto* Access = ResolveWorldUIAccess(mWorld.lock().get());

    if (mConstitutionPopupActive)
    {
        if (Access &&
            mConstitutionRightOptionId != EConstitutionOptionId::None)
        {
            Access->Commands().TrySelectConstitutionOption(
                mConstitutionRightOptionId);
        }

        RefreshFromState();
        return;
    }

    if (!Access)
        return;

    if (Access->Commands().TryExecuteEraTransition(
            EEraTransitionChoice::Confirm))
    {
        mManualEraTransitionPopupOpen = false;
    }

    mEraTransitionPopupOpen =
        mManualEraTransitionPopupOpen || mConstitutionPopupActive;

    RefreshFromState();
}

void CTopHudWidget::OnPopupLeftButtonClick()
{
    if (mGameLost)
        return;

#ifdef _DEBUG
    TraceConstitutionButtonHandler(
        "popup_left_handler_enter",
        mConstitutionPopupActive,
        mConstitutionLeftOptionId);
#endif

    auto* Access = ResolveWorldUIAccess(mWorld.lock().get());

    if (mConstitutionPopupActive)
    {
        if (Access &&
            mConstitutionLeftOptionId != EConstitutionOptionId::None)
        {
            Access->Commands().TrySelectConstitutionOption(
                mConstitutionLeftOptionId);
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

    auto* Access = ResolveWorldUIAccess(mWorld.lock().get());

    if (!Access)
        return;

    Access->Commands().ToggleSimulationPaused();
    RefreshFromState();
}

void CTopHudWidget::OnSpeedMultiplierButtonClick()
{
    if (mGameLost)
        return;

    auto* Access = ResolveWorldUIAccess(mWorld.lock().get());

    if (!Access)
        return;

    Access->Commands().CycleSimulationSpeedMultiplier();
    RefreshFromState();
}

void CTopHudWidget::OnAnyButtonClick()
{
}
