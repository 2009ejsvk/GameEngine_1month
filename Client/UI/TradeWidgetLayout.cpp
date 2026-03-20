#include "TradeWidget.h"
#include "TradeWidgetRuntime.h"
#include "UILayoutValues.h"
#include "Device.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include <algorithm>
#include <array>

using namespace TradeWidgetRuntime;

namespace
{
    namespace Layout = UIConfig::TradeWidgetLayout;
}

void CTradeWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    mLastResolutionWidth = Resolution.Width;
    mLastResolutionHeight = Resolution.Height;

    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(
        Layout::MinimumAvailableWidth,
        ScreenWidth - Layout::ScreenMargin);
    const float AvailableHeight = (std::max)(
        Layout::MinimumAvailableHeight,
        ScreenHeight - Layout::ScreenMargin);
    const float Scale = (std::min)(
        1.f,
        (std::min)(
            AvailableWidth / mPanelWidth,
            AvailableHeight / mPanelHeight));
    const float PanelWidth = mPanelWidth * Scale;
    const float PanelHeight = mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;
    const bool ShowingProductPrices = mSelectedPageIndex == 1;
    const bool ShowingPriceModifiers = mSelectedPageIndex == 2;
    const bool ShowingActiveRoutes = mSelectedPageIndex == 3;
    const bool ShowingCompletedRoutes = mSelectedPageIndex == 4;
    const float HeaderHeight = Layout::HeaderHeight * Scale;
    const float TitleWidth = PanelWidth * Layout::TitleWidthRatio;
    const float TitleLeft = PanelLeft + (PanelWidth - TitleWidth) * 0.5f;
    const float PageButtonWidth = Layout::PageButtonWidth * Scale;
    const float PageButtonHeight = Layout::PageButtonHeight * Scale;
    const float PageButtonsLeft =
        PanelLeft + (PanelWidth -
            PageButtonWidth * static_cast<float>(GTradePageCount) -
            Layout::PageButtonGap * Scale *
                static_cast<float>(GTradePageCount - 1)) * 0.5f;
    const float CloseSize = Layout::CloseButtonSize * Scale;
    const float SectionGap = Layout::SectionGap * Scale;
    const float LeftWidth = PanelWidth * Layout::LeftPanelRatio;
    const float RightWidth = PanelWidth - LeftWidth - Layout::RightPanelGap * Scale;
    const float LeftLeft = PanelLeft + Layout::LeftPanelInset * Scale;
    const float RightLeft = LeftLeft + LeftWidth + SectionGap;
    const float ContentTop = PanelTop + HeaderHeight + Layout::ContentTopOffset * Scale;
    const float ContentHeight =
        PanelHeight - HeaderHeight - Layout::ContentBottomInset * Scale;
    const float FilterHeight = Layout::FilterHeight * Scale;
    const float SortHeight = Layout::SortHeight * Scale;
    const float CompletionAutoOpenButtonWidth =
        Layout::CompletionAutoOpenButtonWidth * Scale;
    const float CompletionAutoOpenButtonGap =
        Layout::CompletionAutoOpenButtonGap * Scale;
    const float ListTop =
        (ShowingActiveRoutes || ShowingCompletedRoutes) ?
            ContentTop + Layout::RouteListTopOffset * Scale :
            ContentTop +
                FilterHeight +
                Layout::FilterToSortGap * Scale +
                SortHeight +
                Layout::FilterToSortGap * Scale;
    const float RowHeight = Layout::RowHeight * Scale;
    const float RowGap = Layout::RowGap * Scale;

    auto PanelBackground = mPanelBackground.lock();
    auto TitleRibbon = mTitleRibbon.lock();
    auto ListFrame = mListFrame.lock();
    auto DetailFrame = mDetailFrame.lock();
    auto TitleText = mTitleText.lock();
    auto CountdownText = mCountdownText.lock();
    auto CloseButton = mCloseButton.lock();
    std::array<std::shared_ptr<CButton>, GTradePageCount> PageButtons = {};
    std::array<std::shared_ptr<CTextBlock>, GTradePageCount> PageTexts = {};
    std::array<std::shared_ptr<CTextBlock>, GTradeModifierSectionCount>
        ModifierSectionTitles = {};
    auto FilterButton = mFilterButton.lock();
    auto FilterButtonText = mFilterButtonText.lock();
    auto DetailTitleText = mDetailTitleText.lock();
    auto AmountTitleText = mAmountTitleText.lock();
    auto ActionButton = mActionButton.lock();
    auto ActionButtonText = mActionButtonText.lock();
    auto CompletionAutoOpenButton = mCompletionAutoOpenButton.lock();
    auto CompletionAutoOpenButtonText = mCompletionAutoOpenButtonText.lock();
    auto FeedbackText = mFeedbackText.lock();

    if (PanelBackground)
    {
        PanelBackground->SetPos(PanelLeft, PanelTop);
        PanelBackground->SetSize(PanelWidth, PanelHeight);
    }

    if (TitleRibbon)
    {
        TitleRibbon->SetPos(TitleLeft, PanelTop + Layout::TitleRibbonTopOffset * Scale);
        TitleRibbon->SetSize(TitleWidth, HeaderHeight);
    }

    for (int Index = 0; Index < GTradePageCount; ++Index)
    {
        PageButtons[static_cast<size_t>(Index)] =
            Index < static_cast<int>(mPageButtons.size()) ?
                mPageButtons[static_cast<size_t>(Index)].lock() :
                nullptr;
        PageTexts[static_cast<size_t>(Index)] =
            Index < static_cast<int>(mPageButtonTexts.size()) ?
                mPageButtonTexts[static_cast<size_t>(Index)].lock() :
                nullptr;

        if (PageButtons[static_cast<size_t>(Index)])
        {
            PageButtons[static_cast<size_t>(Index)]->SetPos(
                PageButtonsLeft +
                    static_cast<float>(Index) *
                        (PageButtonWidth + Layout::PageButtonGap * Scale),
                PanelTop - Layout::PageButtonTopOffset * Scale);
            PageButtons[static_cast<size_t>(Index)]->SetSize(
                PageButtonWidth,
                PageButtonHeight);
        }

        if (PageTexts[static_cast<size_t>(Index)])
        {
            PageTexts[static_cast<size_t>(Index)]->SetPos(0.f, 0.f);
            PageTexts[static_cast<size_t>(Index)]->SetSize(
                PageButtonWidth,
                PageButtonHeight);
            PageTexts[static_cast<size_t>(Index)]->SetFontSize(
                Layout::PageButtonTextFontSize * Scale);
        }
    }

    for (int Index = 0; Index < GTradeModifierSectionCount; ++Index)
    {
        ModifierSectionTitles[static_cast<size_t>(Index)] =
            Index < static_cast<int>(mModifierSectionTitles.size()) ?
                mModifierSectionTitles[static_cast<size_t>(Index)].lock() :
                nullptr;
    }

    if (ListFrame)
    {
        ListFrame->SetPos(
            LeftLeft,
            (ShowingActiveRoutes || ShowingCompletedRoutes) ?
                ContentTop :
                ContentTop + FilterHeight + Layout::ListFrameFilterOffset * Scale);
        ListFrame->SetSize(
            LeftWidth,
            (ShowingActiveRoutes || ShowingCompletedRoutes) ?
                ContentHeight :
                ContentHeight - FilterHeight - Layout::ListFrameFilterOffset * Scale);
    }

    if (DetailFrame)
    {
        DetailFrame->SetPos(RightLeft, ContentTop);
        DetailFrame->SetSize(RightWidth, ContentHeight);
    }

    if (TitleText)
    {
        TitleText->SetPos(
            TitleLeft + Layout::TitleTextInsetX * Scale,
            PanelTop + Layout::TitleRibbonTopOffset * Scale);
        TitleText->SetSize(
            TitleWidth - Layout::TitleTextHorizontalPadding * Scale,
            HeaderHeight);
        TitleText->SetFontSize(Layout::TitleFontSize * Scale);
    }

    if (CountdownText)
    {
        CountdownText->SetPos(
            PanelLeft + PanelWidth - Layout::CountdownRightOffset * Scale,
            PanelTop + Layout::CountdownTopOffset * Scale);
        CountdownText->SetSize(
            Layout::CountdownWidth * Scale,
            Layout::CountdownHeight * Scale);
        CountdownText->SetFontSize(Layout::CountdownFontSize * Scale);
    }

    if (CloseButton)
    {
        CloseButton->SetPos(
            PanelLeft + PanelWidth - Layout::CloseButtonRightOffset * Scale,
            PanelTop + Layout::CloseButtonTopOffset * Scale);
        CloseButton->SetSize(CloseSize, CloseSize);
    }

    if (FilterButton)
    {
        FilterButton->SetPos(LeftLeft, ContentTop);
        FilterButton->SetSize(Layout::FilterButtonWidth * Scale, FilterHeight);
    }

    if (FilterButtonText)
    {
        FilterButtonText->SetPos(0.f, 0.f);
        FilterButtonText->SetSize(Layout::FilterButtonWidth * Scale, FilterHeight);
        FilterButtonText->SetFontSize(Layout::FilterTextFontSize * Scale);
    }

    const float SortButtonWidth =
        (LeftWidth - 3.f * (Layout::SortButtonGap * Scale)) / 4.f;

    for (int Index = 0; Index < GTradeSortCount; ++Index)
    {
        auto Button = mSortButtons[static_cast<size_t>(Index)].lock();
        auto Text = mSortButtonTexts[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        Button->SetPos(
            LeftLeft +
                (SortButtonWidth + Layout::SortButtonGap * Scale) *
                    static_cast<float>(Index),
            ContentTop + FilterHeight + Layout::FilterToSortGap * Scale);
        Button->SetSize(SortButtonWidth, SortHeight);

        if (Text)
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(SortButtonWidth, SortHeight);
            Text->SetFontSize(Layout::SortButtonTextFontSize * Scale);
        }
    }

    const float DirectionWidth = Layout::DirectionColumnWidth * Scale;
    const float PartnerWidth =
        ((ShowingActiveRoutes || ShowingCompletedRoutes) ?
            Layout::ActiveRoutePartnerWidth :
            ShowingProductPrices ?
                Layout::ProductPricePartnerWidth :
                Layout::DefaultPartnerWidth) * Scale;
    const float MarginWidth =
        ((ShowingActiveRoutes || ShowingCompletedRoutes) ?
            Layout::ActiveRouteMarginWidth :
            ShowingProductPrices ?
                Layout::ProductPriceMarginWidth :
                Layout::DefaultMarginWidth) * Scale;
    const float ResourceWidth =
        LeftWidth -
        DirectionWidth -
        PartnerWidth -
        MarginWidth -
        Layout::ResourceColumnsHorizontalPadding * Scale;

    for (int RowIndex = 0; RowIndex < GTradeVisibleProposalCount; ++RowIndex)
    {
        auto Button = mProposalRows[static_cast<size_t>(RowIndex)].Button.lock();
        auto Direction = mProposalRows[static_cast<size_t>(RowIndex)].Direction.lock();
        auto Partner = mProposalRows[static_cast<size_t>(RowIndex)].Partner.lock();
        auto Resource = mProposalRows[static_cast<size_t>(RowIndex)].Resource.lock();
        auto Margin = mProposalRows[static_cast<size_t>(RowIndex)].Margin.lock();
        float RowTop =
            ListTop + static_cast<float>(RowIndex) * (RowHeight + RowGap);

        if (ShowingPriceModifiers)
        {
            if (RowIndex < 2)
            {
                RowTop = ContentTop + Layout::PriceModifierFirstSectionTop * Scale +
                    static_cast<float>(RowIndex) *
                        (RowHeight + Layout::PriceModifierRowGap * Scale);
            }
            else if (RowIndex < 6)
            {
                RowTop = ContentTop + Layout::PriceModifierSecondSectionTop * Scale +
                    static_cast<float>(RowIndex - 2) *
                        (RowHeight + Layout::PriceModifierRowGap * Scale);
            }
            else
            {
                RowTop = ContentTop + Layout::PriceModifierThirdSectionTop * Scale +
                    static_cast<float>(RowIndex - 6) *
                        (RowHeight + Layout::PriceModifierRowGap * Scale);
            }
        }

        if (Button)
        {
            Button->SetPos(LeftLeft + Layout::ProposalButtonInsetX * Scale, RowTop);
            Button->SetSize(
                LeftWidth - Layout::ProposalButtonHorizontalPadding * Scale,
                RowHeight);
        }

        if (Direction)
        {
            Direction->SetPos(Layout::ProposalDirectionInsetX * Scale, 0.f);
            Direction->SetSize(DirectionWidth, RowHeight);
            Direction->SetFontSize(Layout::RowTextFontSize * Scale);
        }

        if (Partner)
        {
            Partner->SetPos(
                Layout::ProposalPartnerInsetX * Scale + DirectionWidth,
                0.f);
            Partner->SetSize(PartnerWidth, RowHeight);
            Partner->SetFontSize(Layout::RowTextFontSize * Scale);
        }

        if (Resource)
        {
            Resource->SetPos(
                ShowingPriceModifiers ?
                    Layout::PriceModifierResourceInsetX * Scale :
                    Layout::ProposalResourceInsetX * Scale +
                        DirectionWidth +
                        PartnerWidth,
                0.f);
            Resource->SetSize(
                ShowingPriceModifiers ?
                    LeftWidth -
                        Layout::PriceModifierResourceRightOffset * Scale :
                    ResourceWidth,
                RowHeight);
            Resource->SetFontSize(Layout::ResourceTextFontSize * Scale);
        }

        if (Margin)
        {
            Margin->SetPos(
                ShowingPriceModifiers ?
                    LeftWidth - Layout::PriceModifierMarginLeftOffset * Scale :
                    Layout::ProposalMarginInsetX * Scale +
                        DirectionWidth +
                        PartnerWidth +
                        ResourceWidth,
                0.f);
            Margin->SetSize(
                ShowingPriceModifiers ?
                    Layout::PriceModifierMarginWidth * Scale :
                    MarginWidth,
                RowHeight);
            Margin->SetFontSize(Layout::RowTextFontSize * Scale);
        }
    }

    if (ShowingPriceModifiers)
    {
        const float LeftTitleWidth =
            LeftWidth - Layout::LeftSectionTitleHorizontalPadding * Scale;
        const float RightTitleWidth =
            RightWidth - Layout::DetailTitleHorizontalPadding * Scale;
        const float RightSectionTop =
            ContentTop + Layout::PriceModifierSecondSectionTitleTop * Scale;

        if (ModifierSectionTitles[0])
        {
            ModifierSectionTitles[0]->SetPos(
                LeftLeft + Layout::SectionTitleLeftInset * Scale,
                ContentTop + Layout::ProposalDirectionInsetX * Scale);
            ModifierSectionTitles[0]->SetSize(
                LeftTitleWidth,
                Layout::SectionTitleHeight * Scale);
            ModifierSectionTitles[0]->SetFontSize(
                Layout::SectionTitleFontSize * Scale);
        }

        if (ModifierSectionTitles[1])
        {
            ModifierSectionTitles[1]->SetPos(
                RightLeft + Layout::SectionTitleRightInset * Scale,
                ContentTop + Layout::ProposalDirectionInsetX * Scale);
            ModifierSectionTitles[1]->SetSize(
                RightTitleWidth,
                Layout::SectionTitleHeight * Scale);
            ModifierSectionTitles[1]->SetFontSize(
                Layout::SectionTitleFontSize * Scale);
        }

        if (ModifierSectionTitles[2])
        {
            ModifierSectionTitles[2]->SetPos(
                LeftLeft + Layout::SectionTitleLeftInset * Scale,
                ContentTop + Layout::PriceModifierSecondSectionTitleTop * Scale);
            ModifierSectionTitles[2]->SetSize(
                LeftTitleWidth,
                Layout::SectionTitleHeight * Scale);
            ModifierSectionTitles[2]->SetFontSize(
                Layout::SectionTitleFontSize * Scale);
        }

        if (ModifierSectionTitles[3])
        {
            ModifierSectionTitles[3]->SetPos(
                RightLeft + Layout::SectionTitleRightInset * Scale,
                RightSectionTop);
            ModifierSectionTitles[3]->SetSize(
                RightTitleWidth,
                Layout::SectionTitleHeight * Scale);
            ModifierSectionTitles[3]->SetFontSize(
                Layout::SectionTitleFontSize * Scale);
        }

        if (ModifierSectionTitles[4])
        {
            ModifierSectionTitles[4]->SetPos(
                LeftLeft + Layout::SectionTitleLeftInset * Scale,
                ContentTop + Layout::PriceModifierThirdSectionTitleTop * Scale);
            ModifierSectionTitles[4]->SetSize(
                LeftTitleWidth,
                Layout::SectionTitleHeight * Scale);
            ModifierSectionTitles[4]->SetFontSize(
                Layout::SectionTitleFontSize * Scale);
        }
    }

    if (DetailTitleText)
    {
        DetailTitleText->SetPos(
            RightLeft + Layout::DetailTitleInsetX * Scale,
            ContentTop + Layout::DetailTitleTopOffset * Scale);
        DetailTitleText->SetSize(
            RightWidth - Layout::DetailTitleHorizontalPadding * Scale,
            Layout::DetailTitleHeight * Scale);
        DetailTitleText->SetFontSize(Layout::DetailTitleFontSize * Scale);
    }

    const float DetailLabelWidth = Layout::DetailLabelWidth * Scale;
    const float DetailValueWidth =
        RightWidth - DetailLabelWidth - Layout::DetailValueHorizontalPadding * Scale;
    const float DetailRowHeight = Layout::DetailRowHeight * Scale;
    const float DetailStartTop = ShowingPriceModifiers ?
        ContentTop + Layout::PriceModifierDetailStartTop * Scale :
        ContentTop + Layout::DefaultDetailStartTop * Scale;

    for (int Index = 0; Index < GTradeDetailRowCount; ++Index)
    {
        auto Label = mDetailRows[static_cast<size_t>(Index)].Label.lock();
        auto Value = mDetailRows[static_cast<size_t>(Index)].Value.lock();
        const float RowTop =
            DetailStartTop + static_cast<float>(Index) * Layout::DetailRowGap * Scale;

        if (Label)
        {
            Label->SetPos(RightLeft + Layout::DetailTitleInsetX * Scale, RowTop);
            Label->SetSize(
                ShowingPriceModifiers ?
                    RightWidth -
                        Layout::PriceModifierDetailLabelRightOffset * Scale :
                    DetailLabelWidth,
                DetailRowHeight);
            Label->SetFontSize(Layout::RowTextFontSize * Scale);
        }

        if (Value)
        {
            Value->SetPos(
                RightLeft +
                    (ShowingPriceModifiers ?
                        RightWidth -
                            Layout::PriceModifierDetailValueLeftOffset * Scale :
                        Layout::DetailTitleInsetX * Scale + DetailLabelWidth),
                RowTop);
            Value->SetSize(
                ShowingPriceModifiers ?
                    Layout::PriceModifierDetailValueWidth * Scale :
                    DetailValueWidth,
                DetailRowHeight);
            Value->SetFontSize(Layout::DetailValueFontSize * Scale);
        }
    }

    if (AmountTitleText)
    {
        AmountTitleText->SetPos(
            RightLeft + Layout::DetailTitleInsetX * Scale,
            ContentTop + Layout::AmountTitleTopOffset * Scale);
        AmountTitleText->SetSize(
            Layout::AmountTitleWidth * Scale,
            Layout::AmountTitleHeight * Scale);
        AmountTitleText->SetFontSize(Layout::AmountTitleFontSize * Scale);
    }

    const float AmountButtonWidth =
        ShowingProductPrices ?
            (RightWidth - Layout::TwoColumnAmountHorizontalPadding * Scale) / 2.f :
            (RightWidth - Layout::ThreeColumnAmountHorizontalPadding * Scale) / 3.f;

    for (int Index = 0; Index < GTradeAmountPresetCount; ++Index)
    {
        auto Button = mAmountButtons[static_cast<size_t>(Index)].lock();
        auto Text = mAmountButtonTexts[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        const float ButtonGap = Layout::AmountButtonGap * Scale;
        Button->SetPos(
            RightLeft + Layout::DetailTitleInsetX * Scale +
                static_cast<float>(Index) * (AmountButtonWidth + ButtonGap),
            ContentTop + Layout::AmountButtonTopOffset * Scale);
        Button->SetSize(AmountButtonWidth, Layout::AmountButtonHeight * Scale);

        if (Text)
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(AmountButtonWidth, Layout::AmountButtonHeight * Scale);
            Text->SetFontSize(Layout::AmountButtonTextFontSize * Scale);
        }
    }

    if (ActionButton)
    {
        ActionButton->SetPos(
            RightLeft + Layout::DetailTitleInsetX * Scale,
            ContentTop + Layout::ActionButtonTopOffset * Scale);
        ActionButton->SetSize(
            RightWidth - Layout::DetailTitleHorizontalPadding * Scale,
            Layout::ActionButtonHeight * Scale);
    }

    if (ActionButtonText)
    {
        ActionButtonText->SetPos(0.f, 0.f);
        ActionButtonText->SetSize(
            RightWidth - Layout::DetailTitleHorizontalPadding * Scale,
            Layout::ActionButtonHeight * Scale);
        ActionButtonText->SetFontSize(Layout::ActionButtonTextFontSize * Scale);
    }

    if (CompletionAutoOpenButton)
    {
        CompletionAutoOpenButton->SetPos(
            RightLeft + Layout::DetailTitleInsetX * Scale,
            ContentTop + Layout::CompletionAutoOpenButtonTopOffset * Scale);
        CompletionAutoOpenButton->SetSize(
            CompletionAutoOpenButtonWidth,
            Layout::CompletionAutoOpenButtonHeight * Scale);
    }

    if (CompletionAutoOpenButtonText)
    {
        CompletionAutoOpenButtonText->SetPos(0.f, 0.f);
        CompletionAutoOpenButtonText->SetSize(
            CompletionAutoOpenButtonWidth,
            Layout::CompletionAutoOpenButtonHeight * Scale);
        CompletionAutoOpenButtonText->SetFontSize(
            Layout::AmountButtonTextFontSize * Scale);
    }

    if (FeedbackText)
    {
        const float FeedbackLeftOffset =
            ShowingPriceModifiers ?
                Layout::DetailTitleInsetX * Scale :
                Layout::DetailTitleInsetX * Scale +
                    CompletionAutoOpenButtonWidth +
                    CompletionAutoOpenButtonGap;
        const float FeedbackRightPadding = Layout::FeedbackRightPadding * Scale;

        FeedbackText->SetPos(
            RightLeft + FeedbackLeftOffset,
            ShowingPriceModifiers ?
                ContentTop + Layout::FeedbackTopOffsetModifiers * Scale :
            ShowingCompletedRoutes ?
                ContentTop + Layout::FeedbackTopOffsetCompleted * Scale :
                ContentTop + Layout::FeedbackTopOffsetDefault * Scale);
        FeedbackText->SetSize(
            RightWidth - FeedbackLeftOffset - FeedbackRightPadding,
            ShowingPriceModifiers ?
                Layout::FeedbackHeightModifiers * Scale :
            ShowingCompletedRoutes ?
                Layout::FeedbackHeightCompleted * Scale :
                Layout::FeedbackHeightDefault * Scale);
        FeedbackText->SetFontSize(Layout::AmountButtonTextFontSize * Scale);
    }
}
