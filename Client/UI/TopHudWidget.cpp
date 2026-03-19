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
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "../Politics/ConstitutionSystem.h"
#include "../ObjectNames.h"
#include "../World/IWorldUIAccess.h"
#include "World/World.h"
#include "World/WorldUIManager.h"
#include <Windows.h>
#include <array>
#include <cmath>
#include <sstream>

namespace
{
    constexpr size_t GConstitutionOptionSlotCount = 3;

    const std::array<EConstitutionTopic, GConstitutionTopicCount>
        GConstitutionTopics =
    {
        EConstitutionTopic::VotingRights,
        EConstitutionTopic::ArmedForces,
        EConstitutionTopic::ReligionAndState,
        EConstitutionTopic::LaborPolicy,
        EConstitutionTopic::MediaIndependence
    };

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

    std::array<EConstitutionOptionId, GConstitutionOptionSlotCount>
        GetConstitutionOptionSlots(EConstitutionTopic Topic)
    {
        std::array<EConstitutionOptionId, GConstitutionOptionSlotCount>
            Result = {};
        Result.fill(EConstitutionOptionId::None);

        const auto& Catalog = ConstitutionSystem::GetConstitutionOptionCatalog();
        size_t WriteIndex = 0;

        for (size_t Index = 0;
            Index < Catalog.size() &&
            WriteIndex < Result.size();
            ++Index)
        {
            if (Catalog[Index].Topic != Topic)
                continue;

            Result[WriteIndex] = Catalog[Index].Id;
            ++WriteIndex;
        }

        return Result;
    }

    const FConstitutionOptionDef* FindSelectedConstitutionOption(
        const FConstitutionState& State,
        EConstitutionTopic Topic)
    {
        const size_t TopicIndex = static_cast<size_t>(Topic);

        if (TopicIndex >= State.SelectedOptions.size())
            return nullptr;

        return FindConstitutionOptionDef(State.SelectedOptions[TopicIndex]);
    }

    bool IsConstitutionTopicUnlocked(
        const FConstitutionState& State,
        EConstitutionTopic Topic,
        EBuildingEra CurrentEra)
    {
        const auto& Catalog = ConstitutionSystem::GetConstitutionOptionCatalog();

        for (size_t Index = 0; Index < Catalog.size(); ++Index)
        {
            if (Catalog[Index].Topic != Topic)
                continue;

            if (IsBuildingEraUnlocked(CurrentEra, Catalog[Index].UnlockEra))
                return true;
        }

        const size_t TopicIndex = static_cast<size_t>(Topic);
        return TopicIndex < State.QueuedTopics.size() &&
            State.QueuedTopics[TopicIndex];
    }

    void AppendSignedMetric(
        std::wstring& OutText,
        const wchar_t* Label,
        int Value,
        const wchar_t* Suffix)
    {
        if (!Label || Value == 0)
            return;

        if (!OutText.empty())
            OutText += L" | ";

        OutText += Label;
        OutText += L" ";
        OutText += Value > 0 ? L"+" : L"";
        OutText += std::to_wstring(Value);

        if (Suffix)
            OutText += Suffix;
    }

    std::wstring BuildConstitutionEffectLine(
        const FConstitutionOptionDef& Option)
    {
        std::wstring Result;
        const int ImmigrationDelta =
            static_cast<int>(std::round(
                (Option.Effect.ImmigrationRateMultiplier - 1.f) * 100.f));

        AppendSignedMetric(
            Result,
            UIStrings::Get(L"constitution.effect.liberty").c_str(),
            Option.Effect.LibertyModifier,
            nullptr);
        AppendSignedMetric(
            Result,
            UIStrings::Get(L"constitution.effect.security").c_str(),
            Option.Effect.SecurityModifier,
            nullptr);
        AppendSignedMetric(
            Result,
            UIStrings::Get(L"constitution.effect.job_quality").c_str(),
            Option.Effect.JobQualityModifier,
            nullptr);
        AppendSignedMetric(
            Result,
            UIStrings::Get(L"constitution.effect.immigration").c_str(),
            ImmigrationDelta,
            L"%");

        if (Result.empty())
            Result = UIStrings::Get(L"constitution.effect.no_major_change");

        return Result;
    }

    std::wstring BuildConstitutionOptionBody(
        const FConstitutionOptionDef& Option)
    {
        std::wstring Result = BuildConstitutionEffectLine(Option);

        if (!Option.EffectSummary.empty())
        {
            Result += L"\n";
            Result += Option.EffectSummary;
        }

        return Result;
    }

    std::wstring BuildConstitutionSummaryValue(
        const FConstitutionState& State,
        EConstitutionTopic Topic,
        EBuildingEra CurrentEra)
    {
        const FConstitutionOptionDef* const Selected =
            FindSelectedConstitutionOption(State, Topic);

        if (Selected)
            return Selected->DisplayName;

        const size_t TopicIndex = static_cast<size_t>(Topic);

        if (State.PendingTopicChoice &&
            State.PendingTopic == Topic)
        {
            return UIStrings::Get(L"constitution.summary.selection_required");
        }

        if (!IsConstitutionTopicUnlocked(State, Topic, CurrentEra))
            return UIStrings::Get(L"constitution.summary.research_required");

        if (TopicIndex < State.QueuedTopics.size() &&
            State.QueuedTopics[TopicIndex])
        {
            return UIStrings::Get(L"constitution.summary.selection_required");
        }

        return UIStrings::Get(L"constitution.summary.locked");
    }

    std::wstring BuildConstitutionSummaryStatus(
        const FConstitutionState& State,
        EConstitutionTopic Topic,
        EBuildingEra CurrentEra)
    {
        if (State.PendingTopicChoice &&
            State.PendingTopic == Topic)
        {
            return UIStrings::Get(L"constitution.summary.pending");
        }

        if (FindSelectedConstitutionOption(State, Topic))
            return UIStrings::Get(L"constitution.summary.active");

        if (!IsConstitutionTopicUnlocked(State, Topic, CurrentEra))
            return UIStrings::Get(L"constitution.summary.research_required");

        return UIStrings::Get(L"constitution.summary.waiting");
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
    mConstitutionPanelOpen = false;
    mConstitutionOverviewMode = true;
    mManualEraTransitionPopupOpen = false;
    mConstitutionPopupActive = false;
    mEraTransitionPopupOpen = false;
    mConstitutionViewedTopic = EConstitutionTopic::VotingRights;
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

    CloseConstitutionPanel();
    CloseMenus(true, true, true, true, false);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto TaskWidget =
        UIManager->FindWidget<CTaskWidget>(GTaskWidgetName).lock();

    if (TaskWidget)
        TaskWidget->OpenForDemand(IssuerType, IssuerIndex);
}

bool CTopHudWidget::TryOpenEraTransitionTask()
{
    auto World = mWorld.lock();

    if (!World)
        return false;

    auto* Access = ResolveWorldUIAccess(World.get());

    if (!Access)
        return false;

    const FEraTransitionState& EraTransitionState =
        Access->Read().GetEraTransitionState();

    if (EraTransitionState.Stage != EEraTransitionStage::Available ||
        !EraTransitionState.CanStart)
    {
        return false;
    }

    CloseConstitutionPanel();
    CloseMenus(true, true, true, true, false);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return false;

    auto TaskWidget =
        UIManager->FindWidget<CTaskWidget>(GTaskWidgetName).lock();

    if (!TaskWidget)
        return false;

    mManualEraTransitionPopupOpen = false;
    mEraTransitionPopupOpen = false;
    TaskWidget->OpenEraTransitionTask();
    return true;
}

void CTopHudWidget::OpenConstitutionPanel(bool ForceOverview)
{
    CloseMenus(true, true, true, true, true);

    mManualEraTransitionPopupOpen = false;
    mEraTransitionPopupOpen = false;
    mConstitutionPanelOpen = true;

    auto* Access = ResolveWorldUIAccess(mWorld.lock().get());

    if (ForceOverview || !Access)
    {
        mConstitutionOverviewMode = true;
    }
    else
    {
        const FConstitutionState& ConstitutionState =
            Access->Read().GetConstitutionState();

        if (ConstitutionState.PendingTopicChoice)
        {
            mConstitutionOverviewMode = false;
            mConstitutionViewedTopic = ConstitutionState.PendingTopic;
        }
    }
}

void CTopHudWidget::CloseConstitutionPanel()
{
    mConstitutionPanelOpen = false;
    mConstitutionOverviewMode = true;
}

void CTopHudWidget::ShowConstitutionOverview()
{
    mConstitutionPanelOpen = true;
    mConstitutionOverviewMode = true;
}

void CTopHudWidget::ShowConstitutionTopic(EConstitutionTopic Topic)
{
    mConstitutionPanelOpen = true;
    mConstitutionOverviewMode = false;
    mConstitutionViewedTopic = Topic;
}

void CTopHudWidget::TrySelectConstitutionOption(EConstitutionOptionId OptionId)
{
    if (mGameLost || OptionId == EConstitutionOptionId::None)
        return;

    auto* Access = ResolveWorldUIAccess(mWorld.lock().get());

    if (!Access)
        return;

    if (!Access->Commands().TrySelectConstitutionOption(OptionId))
    {
        RefreshFromState();
        return;
    }

    const FConstitutionState& ConstitutionState =
        Access->Read().GetConstitutionState();

    if (ConstitutionState.PendingTopicChoice)
    {
        mConstitutionOverviewMode = false;
        mConstitutionViewedTopic = ConstitutionState.PendingTopic;
    }
    else
    {
        mConstitutionOverviewMode = true;
    }

    RefreshFromState();
}

void CTopHudWidget::TrySelectViewedConstitutionOption(size_t SlotIndex)
{
    if (SlotIndex >= GConstitutionOptionSlotCount)
        return;

    const auto OptionSlots =
        GetConstitutionOptionSlots(mConstitutionViewedTopic);
    TrySelectConstitutionOption(OptionSlots[SlotIndex]);
}

void CTopHudWidget::RefreshConstitutionPanelContent(
    IWorldUIAccess* Access,
    bool CanUseButtons)
{
    auto Panel = mConstitutionPanel.lock();
    auto TitleRibbon = mConstitutionTitleRibbon.lock();
    auto TitleText = mConstitutionTitleText.lock();
    auto SubtitleText = mConstitutionSubtitleText.lock();
    auto PendingText = mConstitutionPendingText.lock();
    auto CloseButton = mConstitutionCloseButton.lock();
    auto OverviewButton = mConstitutionOverviewButton.lock();
    auto OverviewButtonText = mConstitutionOverviewButtonText.lock();

    const bool ShowPanel = mConstitutionPanelOpen && !mGameLost;
    const bool OverviewSelected = mConstitutionOverviewMode;

    if (Panel)
        Panel->SetEnable(ShowPanel);

    if (TitleRibbon)
        TitleRibbon->SetEnable(ShowPanel);

    if (CloseButton)
    {
        CloseButton->SetEnable(ShowPanel);
        CloseButton->ButtonEnable(ShowPanel && CanUseButtons);
    }

    if (OverviewButton)
    {
        const bool Enabled = ShowPanel && CanUseButtons;
        OverviewButton->SetEnable(ShowPanel);
        OverviewButton->ButtonEnable(Enabled);
        OverviewButton->SetTint(
            EButtonState::Normal,
            OverviewSelected ?
                FVector4(1.08f, 1.02f, 0.86f, 1.f) :
                FVector4(0.94f, 0.97f, 1.02f, 0.98f));
        OverviewButton->SetTint(
            EButtonState::Hovered,
            OverviewSelected ?
                FVector4(1.14f, 1.08f, 0.90f, 1.f) :
                FVector4(1.02f, 1.04f, 1.06f, 1.f));
        OverviewButton->SetTint(
            EButtonState::Click,
            FVector4(0.88f, 0.84f, 0.72f, 1.f));
        OverviewButton->SetTint(
            EButtonState::Disable,
            FVector4(0.55f, 0.55f, 0.55f, 0.72f));
    }

    if (OverviewButtonText)
    {
        OverviewButtonText->SetText(
            UIStrings::Get(L"constitution.panel.overview_button").c_str());
        OverviewButtonText->SetEnable(ShowPanel);
        OverviewButtonText->SetTextColor(
            OverviewSelected ? 78 : 62,
            OverviewSelected ? 54 : 58,
            OverviewSelected ? 26 : 40,
            255);
    }

    if (TitleText)
        TitleText->SetEnable(ShowPanel);

    if (SubtitleText)
        SubtitleText->SetEnable(ShowPanel);

    if (PendingText)
        PendingText->SetEnable(ShowPanel);

    if (!Access)
    {
        if (TitleText)
            TitleText->SetText(
                UIStrings::Get(L"constitution.panel.title").c_str());

        if (SubtitleText)
            SubtitleText->SetText(
                UIStrings::Get(L"constitution.panel.subtitle").c_str());

        if (PendingText)
            PendingText->SetText(TEXT(""));

        return;
    }

    const FConstitutionState& ConstitutionState =
        Access->Read().GetConstitutionState();
    const EBuildingEra CurrentEra = Access->Read().GetCurrentEra();

    if (TitleText)
    {
        TitleText->SetText(
            mConstitutionOverviewMode ?
                UIStrings::Get(L"constitution.panel.title").c_str() :
                GetConstitutionTopicDisplayName(mConstitutionViewedTopic));
    }

    if (SubtitleText)
    {
        SubtitleText->SetText(
            mConstitutionOverviewMode ?
                UIStrings::Get(L"constitution.panel.subtitle").c_str() :
                UIStrings::Get(
                    L"constitution.panel.topic_subtitle").c_str());
    }

    if (PendingText)
    {
        std::wstring PendingMessage;

        if (mConstitutionOverviewMode)
        {
            if (ConstitutionState.PendingTopicChoice)
            {
                PendingMessage =
                    UIStrings::Get(L"constitution.summary.pending_prefix") +
                    std::wstring(GetConstitutionTopicDisplayName(
                        ConstitutionState.PendingTopic));
            }
            else
            {
                PendingMessage =
                    UIStrings::Get(L"constitution.summary.review_all");
            }
        }
        else
        {
            const bool TopicUnlocked = IsConstitutionTopicUnlocked(
                ConstitutionState,
                mConstitutionViewedTopic,
                CurrentEra);
            const FConstitutionOptionDef* const SelectedOption =
                FindSelectedConstitutionOption(
                    ConstitutionState,
                    mConstitutionViewedTopic);

            if (ConstitutionState.PendingTopicChoice &&
                ConstitutionState.PendingTopic == mConstitutionViewedTopic)
            {
                PendingMessage = UIStrings::Get(
                    L"constitution.panel.topic_pending");
            }
            else if (SelectedOption)
            {
                PendingMessage =
                    UIStrings::Get(L"constitution.panel.current_selection") +
                    SelectedOption->DisplayName;
            }
            else if (!TopicUnlocked)
            {
                PendingMessage = UIStrings::Get(
                    L"constitution.summary.research_required");
            }
            else
            {
                PendingMessage = UIStrings::Get(
                    L"constitution.summary.selection_required");
            }
        }

        PendingText->SetText(PendingMessage.c_str());
    }

    for (size_t Index = 0; Index < GConstitutionTopics.size(); ++Index)
    {
        const EConstitutionTopic Topic = GConstitutionTopics[Index];
        const bool TopicUnlocked = IsConstitutionTopicUnlocked(
            ConstitutionState,
            Topic,
            CurrentEra);
        const FConstitutionOptionDef* const SelectedOption =
            FindSelectedConstitutionOption(ConstitutionState, Topic);
        const bool TopicPending =
            ConstitutionState.PendingTopicChoice &&
            ConstitutionState.PendingTopic == Topic;
        const bool TopicSelected =
            !mConstitutionOverviewMode &&
            mConstitutionViewedTopic == Topic;

        if (Index < mConstitutionTopicTabButtons.size())
        {
            auto Button = mConstitutionTopicTabButtons[Index].lock();

            if (Button)
            {
                const bool Enabled = ShowPanel && CanUseButtons;
                Button->SetEnable(ShowPanel);
                Button->ButtonEnable(Enabled);

                const FVector4 NormalTint = TopicSelected ?
                    FVector4(1.08f, 1.02f, 0.86f, 1.f) :
                    TopicUnlocked ?
                        FVector4(0.94f, 0.97f, 1.02f, 0.98f) :
                        FVector4(0.80f, 0.80f, 0.80f, 0.90f);
                const FVector4 HoverTint = TopicSelected ?
                    FVector4(1.14f, 1.08f, 0.90f, 1.f) :
                    FVector4(1.02f, 1.04f, 1.06f, 1.f);

                Button->SetTint(EButtonState::Normal, NormalTint);
                Button->SetTint(EButtonState::Hovered, HoverTint);
                Button->SetTint(EButtonState::Click,
                    FVector4(0.88f, 0.84f, 0.72f, 1.f));
                Button->SetTint(EButtonState::Disable,
                    FVector4(0.55f, 0.55f, 0.55f, 0.72f));
            }
        }

        if (Index < mConstitutionTopicTabTexts.size())
        {
            auto Text = mConstitutionTopicTabTexts[Index].lock();

            if (Text)
            {
                Text->SetText(GetConstitutionTopicDisplayName(Topic));
                Text->SetEnable(ShowPanel);
                Text->SetTextColor(
                    TopicSelected ? 78 : 62,
                    TopicSelected ? 54 : 58,
                    TopicSelected ? 26 : 40,
                    255);
            }
        }

        if (Index < mConstitutionSummaryButtons.size())
        {
            auto Button = mConstitutionSummaryButtons[Index].lock();

            if (Button)
            {
                const bool Enabled = ShowPanel &&
                    mConstitutionOverviewMode &&
                    CanUseButtons;
                Button->SetEnable(ShowPanel && mConstitutionOverviewMode);
                Button->ButtonEnable(Enabled);

                const FVector4 CardTint = TopicPending ?
                    FVector4(1.08f, 0.94f, 0.48f, 1.f) :
                    SelectedOption ?
                        FVector4(1.f, 1.f, 1.f, 0.98f) :
                    TopicUnlocked ?
                        FVector4(0.96f, 0.98f, 0.96f, 0.98f) :
                        FVector4(0.90f, 0.90f, 0.90f, 0.96f);
                Button->SetTint(EButtonState::Normal, CardTint);
                Button->SetTint(EButtonState::Hovered,
                    FVector4(
                        CardTint.x + 0.04f,
                        CardTint.y + 0.03f,
                        CardTint.z + 0.03f,
                        1.f));
                Button->SetTint(EButtonState::Click,
                    FVector4(0.90f, 0.84f, 0.68f, 1.f));
                Button->SetTint(EButtonState::Disable,
                    FVector4(0.65f, 0.65f, 0.65f, 0.72f));
            }
        }

        if (Index < mConstitutionSummaryTopicTexts.size())
        {
            auto Text = mConstitutionSummaryTopicTexts[Index].lock();

            if (Text)
            {
                Text->SetText(GetConstitutionTopicDisplayName(Topic));
                Text->SetEnable(ShowPanel && mConstitutionOverviewMode);
            }
        }

        if (Index < mConstitutionSummaryValueTexts.size())
        {
            auto Text = mConstitutionSummaryValueTexts[Index].lock();

            if (Text)
            {
                Text->SetText(
                    BuildConstitutionSummaryValue(
                        ConstitutionState,
                        Topic,
                        CurrentEra)
                    .c_str());
                Text->SetEnable(ShowPanel && mConstitutionOverviewMode);
            }
        }

        if (Index < mConstitutionSummaryStatusTexts.size())
        {
            auto Text = mConstitutionSummaryStatusTexts[Index].lock();

            if (Text)
            {
                Text->SetText(
                    BuildConstitutionSummaryStatus(
                        ConstitutionState,
                        Topic,
                        CurrentEra)
                    .c_str());
                Text->SetEnable(ShowPanel && mConstitutionOverviewMode);

                if (TopicPending)
                    Text->SetTextColor(196, 38, 22, 255);
                else if (SelectedOption)
                    Text->SetTextColor(72, 108, 48, 255);
                else
                    Text->SetTextColor(132, 96, 44, 255);
            }
        }
    }

    const std::array<EConstitutionOptionId, GConstitutionOptionSlotCount>
        OptionSlots = GetConstitutionOptionSlots(mConstitutionViewedTopic);
    const bool ViewedTopicUnlocked = IsConstitutionTopicUnlocked(
        ConstitutionState,
        mConstitutionViewedTopic,
        CurrentEra);
    const bool ViewedTopicPending =
        ConstitutionState.PendingTopicChoice &&
        ConstitutionState.PendingTopic == mConstitutionViewedTopic;
    const FConstitutionOptionDef* const SelectedOption =
        FindSelectedConstitutionOption(
            ConstitutionState,
            mConstitutionViewedTopic);

    for (size_t Index = 0; Index < OptionSlots.size(); ++Index)
    {
        auto Button =
            Index < mConstitutionOptionButtons.size() ?
                mConstitutionOptionButtons[Index].lock() :
                nullptr;
        auto Title =
            Index < mConstitutionOptionTitleTexts.size() ?
                mConstitutionOptionTitleTexts[Index].lock() :
                nullptr;
        auto Summary =
            Index < mConstitutionOptionSummaryTexts.size() ?
                mConstitutionOptionSummaryTexts[Index].lock() :
                nullptr;
        auto Body =
            Index < mConstitutionOptionBodyTexts.size() ?
                mConstitutionOptionBodyTexts[Index].lock() :
                nullptr;

        const FConstitutionOptionDef* const Option =
            FindConstitutionOptionDef(OptionSlots[Index]);
        const bool Occupied = Option != nullptr;
        const bool IsSelected =
            Occupied &&
            SelectedOption &&
            SelectedOption->Id == Option->Id;
        const bool CanPick =
            ShowPanel &&
            !mConstitutionOverviewMode &&
            ViewedTopicUnlocked &&
            ViewedTopicPending &&
            Occupied &&
            CanUseButtons;

        if (Button)
        {
            Button->SetEnable(ShowPanel && !mConstitutionOverviewMode);
            Button->ButtonEnable(CanPick);

            const FVector4 NormalTint = IsSelected ?
                FVector4(1.10f, 0.95f, 0.24f, 1.f) :
                Occupied ?
                    FVector4(0.99f, 0.98f, 0.95f, 0.98f) :
                    FVector4(0.95f, 0.97f, 0.95f, 0.92f);

            Button->SetTint(EButtonState::Normal, NormalTint);
            Button->SetTint(EButtonState::Hovered,
                IsSelected ?
                    FVector4(1.12f, 0.99f, 0.36f, 1.f) :
                    FVector4(1.03f, 1.02f, 0.98f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.94f, 0.84f, 0.46f, 1.f));
            Button->SetTint(EButtonState::Disable,
                Occupied ?
                    FVector4(0.82f, 0.82f, 0.82f, 0.92f) :
                    FVector4(0.78f, 0.82f, 0.78f, 0.88f));
        }

        if (Title)
        {
            Title->SetEnable(ShowPanel && !mConstitutionOverviewMode);
            Title->SetText(
                Occupied ?
                    Option->DisplayName.c_str() :
                    UIStrings::Get(
                        L"constitution.option.placeholder_title").c_str());
        }

        if (Summary)
        {
            Summary->SetEnable(ShowPanel && !mConstitutionOverviewMode);
            Summary->SetText(
                Occupied ?
                    BuildConstitutionEffectLine(*Option).c_str() :
                    UIStrings::Get(
                        L"constitution.option.placeholder_summary").c_str());

            Summary->SetTextColor(
                IsSelected ? 88 : 118,
                IsSelected ? 54 : 84,
                IsSelected ? 14 : 54,
                255);
        }

        if (Body)
        {
            Body->SetEnable(ShowPanel && !mConstitutionOverviewMode);
            Body->SetText(
                Occupied ?
                    BuildConstitutionOptionBody(*Option).c_str() :
                    UIStrings::Get(
                        L"constitution.option.placeholder_body").c_str());
        }
    }
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
    {
        CloseMenus(true, true, true, true, true);
        OpenConstitutionPanel(false);
    }

    if (Snapshot.EraTransitionAvailable)
        mManualEraTransitionPopupOpen = false;

    mEraTransitionPopupOpen =
        !mConstitutionPanelOpen &&
        !Snapshot.EraTransitionAvailable &&
        mManualEraTransitionPopupOpen;

    if (Snapshot.GameLost && !mGameOverMenusClosed)
    {
        CloseMenus(true, true, true, true, true);
        mGameOverMenusClosed = true;
        mConstitutionPanelOpen = false;
        mManualEraTransitionPopupOpen = false;
        mConstitutionPopupActive = false;
        mEraTransitionPopupOpen = false;
    }
    else if (!Snapshot.GameLost)
    {
        mGameOverMenusClosed = false;
    }

    FTopHudRenderer::ApplySnapshot(*this, Snapshot);
    FTopHudRenderer::RefreshLayout(*this);
    RefreshConstitutionPanelContent(Access, Snapshot.CanUseButtons);
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

    CloseConstitutionPanel();
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

    CloseConstitutionPanel();
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

    if (!Access)
        return;

    if (mConstitutionPanelOpen)
    {
        CloseConstitutionPanel();
    }
    else
    {
        OpenConstitutionPanel(true);
    }

    RefreshFromState();
}

void CTopHudWidget::OnAlmanacButtonClick()
{
    if (mGameLost)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    CloseConstitutionPanel();
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

    CloseConstitutionPanel();
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

    CloseConstitutionPanel();
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

    if (TryOpenEraTransitionTask())
        mManualEraTransitionPopupOpen = false;

    mEraTransitionPopupOpen =
        mManualEraTransitionPopupOpen || mConstitutionPopupActive;

    RefreshFromState();
}

void CTopHudWidget::OnConstitutionCloseButtonClick()
{
    if (mGameLost)
        return;

    CloseConstitutionPanel();
    RefreshFromState();
}

void CTopHudWidget::OnConstitutionOverviewButtonClick()
{
    if (mGameLost)
        return;

    ShowConstitutionOverview();
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
