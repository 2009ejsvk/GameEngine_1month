#include "TradeWidget.h"
#include "TradeWidgetRuntime.h"
#include "TropicoUiStyle.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "World/World.h"
#include <string>

using namespace TradeWidgetRuntime;

namespace
{
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

    void ConfigureTitleText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(30.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(110, 78, 26, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(250, 240, 214, 180);
    }

    void ConfigureHeaderText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(15.f);
        Text->SetAlignH(ETextAlignH::Right);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(108, 94, 67, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(244, 234, 202, 180);
    }

    void ConfigureButtonText(
        const std::shared_ptr<CTextBlock>& Text,
        float FontSize)
    {
        if (!Text)
            return;

        Text->SetFontSize(FontSize);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(88, 60, 16, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(250, 240, 220, 180);
    }

    void ConfigureBodyLabelText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(15.f);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(119, 99, 58, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(246, 236, 214, 160);
    }

    void ConfigureBodyValueText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(17.f);
        Text->SetAlignH(ETextAlignH::Right);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(58, 46, 26, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(250, 240, 220, 120);
    }

    void ConfigureRowText(
        const std::shared_ptr<CTextBlock>& Text,
        ETextAlignH Align,
        float FontSize)
    {
        if (!Text)
            return;

        Text->SetFontSize(FontSize);
        Text->SetAlignH(Align);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(72, 56, 32, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(245, 234, 208, 140);
    }

    void ConfigureTradeButton(
        const std::shared_ptr<CButton>& Button,
        const std::string& TextureKeyBase)
    {
        if (!Button)
            return;

        ApplyButtonTextureSet(
            Button,
            TextureKeyBase,
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureDefaultButtonStyle(Button);
    }

    void ConfigureProposalRowButton(const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        ApplyButtonTextureSet(
            Button,
            "TradeProposalRow",
            GSlotCardTexture,
            GSlotCardHoverTexture,
            GSlotCardSelectedTexture,
            GSlotCardDisabledTexture);
        ConfigureIconSlotButtonStyle(Button);
    }
}

CTradeWidget::CTradeWidget()
{
}

CTradeWidget::~CTradeWidget()
{
}

bool CTradeWidget::Init()
{
    CWidgetContainer::Init();

    auto PanelBackground =
        CreateWidget<CImage>("TradeWidget_Background", 6).lock();
    auto TitleRibbon =
        CreateWidget<CImage>("TradeWidget_TitleRibbon", 7).lock();
    auto ListFrame =
        CreateWidget<CImage>("TradeWidget_ListFrame", 7).lock();
    auto DetailFrame =
        CreateWidget<CImage>("TradeWidget_DetailFrame", 7).lock();
    auto TitleText =
        CreateWidget<CTextBlock>("TradeWidget_Title", 8).lock();
    auto CountdownText =
        CreateWidget<CTextBlock>("TradeWidget_Countdown", 8).lock();
    auto CloseButton =
        CreateWidget<CButton>("TradeWidget_CloseButton", 8).lock();
    mPageButtons.resize(GTradePageCount);
    mPageButtonTexts.resize(GTradePageCount);
    mModifierSectionTitles.resize(GTradeModifierSectionCount);
    auto FilterButton =
        CreateWidget<CButton>("TradeWidget_FilterButton", 8).lock();
    auto DetailTitleText =
        CreateWidget<CTextBlock>("TradeWidget_DetailTitle", 8).lock();
    auto AmountTitleText =
        CreateWidget<CTextBlock>("TradeWidget_AmountTitle", 8).lock();
    auto ActionButton =
        CreateWidget<CButton>("TradeWidget_ActionButton", 8).lock();
    auto CompletionAutoOpenButton =
        CreateWidget<CButton>("TradeWidget_CompletionAutoOpenButton", 8).lock();
    auto FeedbackText =
        CreateWidget<CTextBlock>("TradeWidget_Feedback", 8).lock();

    if (PanelBackground)
    {
        PanelBackground->SetTexture(
            "TradeWidget_BackgroundTex",
            GMainMenuPanelTexture);
        PanelBackground->SetTint(1.f, 1.f, 1.f, 1.f);
        mPanelBackground = PanelBackground;
    }

    if (TitleRibbon)
    {
        TitleRibbon->SetTexture(
            "TradeWidget_TitleRibbonTex",
            GMenuTitleRibbonTexture);
        TitleRibbon->SetTint(1.f, 1.f, 1.f, 1.f);
        mTitleRibbon = TitleRibbon;
    }

    if (ListFrame)
    {
        ListFrame->SetTexture(
            "TradeWidget_ListFrameTex",
            GMenuGridFrameTexture);
        ListFrame->SetTint(1.f, 1.f, 1.f, 1.f);
        mListFrame = ListFrame;
    }

    if (DetailFrame)
    {
        DetailFrame->SetTexture(
            "TradeWidget_DetailFrameTex",
            GMenuDetailFrameTexture);
        DetailFrame->SetTint(1.f, 1.f, 1.f, 1.f);
        mDetailFrame = DetailFrame;
    }

    if (TitleText)
    {
        ConfigureTitleText(TitleText);
        mTitleText = TitleText;
    }

    if (CountdownText)
    {
        ConfigureHeaderText(CountdownText);
        mCountdownText = CountdownText;
    }

    if (CloseButton)
    {
        ApplyButtonTextureSet(
            CloseButton,
            "TradeWidget_CloseButton",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        ConfigureIconSlotButtonStyle(CloseButton);
        CloseButton->SetEventCallback<CTradeWidget>(
            EButtonEventState::Click,
            this,
            &CTradeWidget::OnCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_CloseText",
            mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            ConfigureButtonText(CloseText, 20.f);
            CloseButton->SetChild(CloseText);
        }

        mCloseButton = CloseButton;
    }

    for (int Index = 0; Index < GTradePageCount; ++Index)
    {
        auto PageButton = CreateWidget<CButton>(
            "TradeWidget_PageButton_" + std::to_string(Index + 1),
            8).lock();

        if (!PageButton)
            continue;

        ConfigureTradeButton(
            PageButton,
            "TradeWidget_PageButton_" + std::to_string(Index + 1));

        auto PageContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_PageContent_" + std::to_string(Index + 1),
            mWorld);
        auto PageText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_PageText_" + std::to_string(Index + 1),
            mWorld);

        if (PageContent && PageText)
        {
            ConfigureButtonText(PageText, 15.f);
            PageContent->AddWidget(PageText);
            PageButton->SetChild(PageContent);
            mPageButtonTexts[static_cast<size_t>(Index)] = PageText;
        }

        PageButton->SetEventCallback(
            EButtonEventState::Click,
            [this, Index]()
            {
                OnPageButtonClick(Index);
            });
        mPageButtons[static_cast<size_t>(Index)] = PageButton;
    }

    for (int Index = 0; Index < GTradeModifierSectionCount; ++Index)
    {
        auto SectionTitle = CreateWidget<CTextBlock>(
            "TradeWidget_ModifierSection_" + std::to_string(Index + 1),
            8).lock();

        if (!SectionTitle)
            continue;

        SectionTitle->SetFontSize(18.f);
        SectionTitle->SetAlignH(ETextAlignH::Center);
        SectionTitle->SetAlignV(ETextAlignV::Middle);
        SectionTitle->SetTextColor(96, 83, 55, 255);
        SectionTitle->EnableShadow(true);
        SectionTitle->SetShadowOffset(1.f, 1.f);
        SectionTitle->SetShadowTextColor(244, 234, 202, 150);
        mModifierSectionTitles[static_cast<size_t>(Index)] = SectionTitle;
    }

    if (FilterButton)
    {
        ConfigureTradeButton(FilterButton, "TradeWidget_FilterButton");
        auto FilterContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_FilterContent",
            mWorld);
        auto FilterText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_FilterText",
            mWorld);

        if (FilterContent && FilterText)
        {
            ConfigureButtonText(FilterText, 16.f);
            FilterContent->AddWidget(FilterText);
            FilterButton->SetChild(FilterContent);
            mFilterButtonText = FilterText;
        }

        FilterButton->SetEventCallback<CTradeWidget>(
            EButtonEventState::Click,
            this,
            &CTradeWidget::OnFilterButtonClick);
        mFilterButton = FilterButton;
    }

    mSortButtons.resize(GTradeSortCount);
    mSortButtonTexts.resize(GTradeSortCount);

    for (int Index = 0; Index < GTradeSortCount; ++Index)
    {
        auto SortButton = CreateWidget<CButton>(
            "TradeWidget_SortButton_" + std::to_string(Index + 1),
            8).lock();

        if (!SortButton)
            continue;

        ConfigureTradeButton(
            SortButton,
            "TradeWidget_SortButton_" + std::to_string(Index + 1));

        auto SortContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_SortContent_" + std::to_string(Index + 1),
            mWorld);
        auto SortText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_SortText_" + std::to_string(Index + 1),
            mWorld);

        if (SortContent && SortText)
        {
            ConfigureButtonText(SortText, 15.f);
            SortContent->AddWidget(SortText);
            SortButton->SetChild(SortContent);
            mSortButtonTexts[static_cast<size_t>(Index)] = SortText;
        }

        SortButton->SetEventCallback(
            EButtonEventState::Click,
            [this, Index]()
            {
                OnSortButtonClick(Index);
            });
        mSortButtons[static_cast<size_t>(Index)] = SortButton;
    }

    mProposalRows.resize(GTradeVisibleProposalCount);

    for (int RowIndex = 0; RowIndex < GTradeVisibleProposalCount; ++RowIndex)
    {
        auto RowButton = CreateWidget<CButton>(
            "TradeWidget_ProposalRow_" + std::to_string(RowIndex + 1),
            8).lock();

        if (!RowButton)
            continue;

        ConfigureProposalRowButton(RowButton);
        RowButton->SetEventCallback(
            EButtonEventState::Click,
            [this, RowIndex]()
            {
                OnProposalButtonClick(RowIndex);
            });

        auto Content = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_ProposalContent_" + std::to_string(RowIndex + 1),
            mWorld);
        auto DirectionText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_ProposalDirection_" + std::to_string(RowIndex + 1),
            mWorld);
        auto PartnerText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_ProposalPartner_" + std::to_string(RowIndex + 1),
            mWorld);
        auto ResourceText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_ProposalResource_" + std::to_string(RowIndex + 1),
            mWorld);
        auto MarginText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_ProposalMargin_" + std::to_string(RowIndex + 1),
            mWorld);

        if (Content && DirectionText && PartnerText && ResourceText && MarginText)
        {
            ConfigureRowText(DirectionText, ETextAlignH::Center, 15.f);
            ConfigureRowText(PartnerText, ETextAlignH::Center, 15.f);
            ConfigureRowText(ResourceText, ETextAlignH::Left, 16.f);
            ConfigureRowText(MarginText, ETextAlignH::Right, 15.f);
            Content->AddWidget(DirectionText);
            Content->AddWidget(PartnerText);
            Content->AddWidget(ResourceText);
            Content->AddWidget(MarginText);
            RowButton->SetChild(Content);
            mProposalRows[static_cast<size_t>(RowIndex)].Direction =
                DirectionText;
            mProposalRows[static_cast<size_t>(RowIndex)].Partner =
                PartnerText;
            mProposalRows[static_cast<size_t>(RowIndex)].Resource =
                ResourceText;
            mProposalRows[static_cast<size_t>(RowIndex)].Margin =
                MarginText;
        }

        mProposalRows[static_cast<size_t>(RowIndex)].Button = RowButton;
    }

    if (DetailTitleText)
    {
        DetailTitleText->SetFontSize(22.f);
        DetailTitleText->SetAlignH(ETextAlignH::Left);
        DetailTitleText->SetAlignV(ETextAlignV::Middle);
        DetailTitleText->SetTextColor(92, 67, 23, 255);
        DetailTitleText->EnableShadow(true);
        DetailTitleText->SetShadowOffset(1.f, 1.f);
        DetailTitleText->SetShadowTextColor(248, 236, 204, 160);
        mDetailTitleText = DetailTitleText;
    }

    mDetailRows.resize(GTradeDetailRowCount);

    for (int Index = 0; Index < GTradeDetailRowCount; ++Index)
    {
        auto Label = CreateWidget<CTextBlock>(
            "TradeWidget_DetailLabel_" + std::to_string(Index + 1),
            8).lock();
        auto Value = CreateWidget<CTextBlock>(
            "TradeWidget_DetailValue_" + std::to_string(Index + 1),
            8).lock();

        if (Label)
            ConfigureBodyLabelText(Label);

        if (Value)
            ConfigureBodyValueText(Value);

        mDetailRows[static_cast<size_t>(Index)].Label = Label;
        mDetailRows[static_cast<size_t>(Index)].Value = Value;
    }

    if (AmountTitleText)
    {
        AmountTitleText->SetText(TEXT("계약 물량"));
        AmountTitleText->SetFontSize(17.f);
        AmountTitleText->SetAlignH(ETextAlignH::Left);
        AmountTitleText->SetAlignV(ETextAlignV::Middle);
        AmountTitleText->SetTextColor(98, 77, 41, 255);
        AmountTitleText->EnableShadow(true);
        AmountTitleText->SetShadowOffset(1.f, 1.f);
        AmountTitleText->SetShadowTextColor(248, 236, 204, 150);
        mAmountTitleText = AmountTitleText;
    }

    mAmountButtons.resize(GTradeAmountPresetCount);
    mAmountButtonTexts.resize(GTradeAmountPresetCount);

    for (int Index = 0; Index < GTradeAmountPresetCount; ++Index)
    {
        auto AmountButton = CreateWidget<CButton>(
            "TradeWidget_AmountButton_" + std::to_string(Index + 1),
            8).lock();

        if (!AmountButton)
            continue;

        ConfigureTradeButton(
            AmountButton,
            "TradeWidget_AmountButton_" + std::to_string(Index + 1));

        auto AmountContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_AmountContent_" + std::to_string(Index + 1),
            mWorld);
        auto AmountText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_AmountText_" + std::to_string(Index + 1),
            mWorld);

        if (AmountContent && AmountText)
        {
            ConfigureButtonText(AmountText, 15.f);
            AmountContent->AddWidget(AmountText);
            AmountButton->SetChild(AmountContent);
            mAmountButtonTexts[static_cast<size_t>(Index)] = AmountText;
        }

        AmountButton->SetEventCallback(
            EButtonEventState::Click,
            [this, Index]()
            {
                OnAmountButtonClick(Index);
            });
        mAmountButtons[static_cast<size_t>(Index)] = AmountButton;
    }

    if (ActionButton)
    {
        ConfigureTradeButton(ActionButton, "TradeWidget_ActionButton");

        auto ActionContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_ActionContent",
            mWorld);
        auto ActionText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_ActionText",
            mWorld);

        if (ActionContent && ActionText)
        {
            ConfigureButtonText(ActionText, 18.f);
            ActionContent->AddWidget(ActionText);
            ActionButton->SetChild(ActionContent);
            mActionButtonText = ActionText;
        }

        ActionButton->SetEventCallback<CTradeWidget>(
            EButtonEventState::Click,
            this,
            &CTradeWidget::OnActionButtonClick);
        mActionButton = ActionButton;
    }

    if (CompletionAutoOpenButton)
    {
        ConfigureTradeButton(
            CompletionAutoOpenButton,
            "TradeWidget_CompletionAutoOpenButton");

        auto CompletionAutoOpenContent =
            CWidget::CreateStaticWidget<CWidgetContainer>(
                "TradeWidget_CompletionAutoOpenContent",
                mWorld);
        auto CompletionAutoOpenText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "TradeWidget_CompletionAutoOpenText",
                mWorld);

        if (CompletionAutoOpenContent && CompletionAutoOpenText)
        {
            ConfigureButtonText(CompletionAutoOpenText, 18.f);
            CompletionAutoOpenContent->AddWidget(CompletionAutoOpenText);
            CompletionAutoOpenButton->SetChild(CompletionAutoOpenContent);
            mCompletionAutoOpenButtonText = CompletionAutoOpenText;
        }

        CompletionAutoOpenButton->SetEventCallback<CTradeWidget>(
            EButtonEventState::Click,
            this,
            &CTradeWidget::OnCompletionAutoOpenButtonClick);
        mCompletionAutoOpenButton = CompletionAutoOpenButton;
    }

    if (FeedbackText)
    {
        FeedbackText->SetFontSize(15.f);
        FeedbackText->SetAlignH(ETextAlignH::Left);
        FeedbackText->SetAlignV(ETextAlignV::Top);
        FeedbackText->SetTextColor(90, 76, 55, 255);
        FeedbackText->EnableShadow(true);
        FeedbackText->SetShadowOffset(1.f, 1.f);
        FeedbackText->SetShadowTextColor(250, 240, 220, 120);
        mFeedbackText = FeedbackText;
    }

    RefreshLayout();
    RefreshFromState();
    return true;
}
