#include "TradeWidget.h"
#include "TradeWidgetRuntime.h"
#include "Device.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include <algorithm>
#include <array>

using namespace TradeWidgetRuntime;

void CTradeWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    mLastResolutionWidth = Resolution.Width;
    mLastResolutionHeight = Resolution.Height;

    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(640.f, ScreenWidth - 120.f);
    const float AvailableHeight = (std::max)(560.f, ScreenHeight - 120.f);
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
    const float HeaderHeight = 62.f * Scale;
    const float TitleWidth = PanelWidth * 0.42f;
    const float TitleLeft = PanelLeft + (PanelWidth - TitleWidth) * 0.5f;
    const float PageButtonWidth = 132.f * Scale;
    const float PageButtonHeight = 34.f * Scale;
    const float PageButtonsLeft =
        PanelLeft + (PanelWidth -
            PageButtonWidth * static_cast<float>(GTradePageCount) -
            12.f * Scale * static_cast<float>(GTradePageCount - 1)) * 0.5f;
    const float CloseSize = 36.f * Scale;
    const float SectionGap = 18.f * Scale;
    const float LeftWidth = PanelWidth * 0.52f;
    const float RightWidth = PanelWidth - LeftWidth - 48.f * Scale;
    const float LeftLeft = PanelLeft + 26.f * Scale;
    const float RightLeft = LeftLeft + LeftWidth + SectionGap;
    const float ContentTop = PanelTop + HeaderHeight + 20.f * Scale;
    const float ContentHeight = PanelHeight - HeaderHeight - 48.f * Scale;
    const float FilterHeight = 44.f * Scale;
    const float SortHeight = 34.f * Scale;
    const float ListTop =
        (ShowingActiveRoutes || ShowingCompletedRoutes) ?
            ContentTop + 12.f * Scale :
            ContentTop + FilterHeight + 10.f * Scale + SortHeight + 10.f * Scale;
    const float RowHeight = 46.f * Scale;
    const float RowGap = 6.f * Scale;

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
        TitleRibbon->SetPos(TitleLeft, PanelTop + 8.f * Scale);
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
                        (PageButtonWidth + 12.f * Scale),
                PanelTop - 14.f * Scale);
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
            PageTexts[static_cast<size_t>(Index)]->SetFontSize(15.f * Scale);
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
                ContentTop + FilterHeight + 8.f * Scale);
        ListFrame->SetSize(
            LeftWidth,
            (ShowingActiveRoutes || ShowingCompletedRoutes) ?
                ContentHeight :
                ContentHeight - FilterHeight - 8.f * Scale);
    }

    if (DetailFrame)
    {
        DetailFrame->SetPos(RightLeft, ContentTop);
        DetailFrame->SetSize(RightWidth, ContentHeight);
    }

    if (TitleText)
    {
        TitleText->SetPos(TitleLeft + 18.f * Scale, PanelTop + 8.f * Scale);
        TitleText->SetSize(TitleWidth - 36.f * Scale, HeaderHeight);
        TitleText->SetFontSize(30.f * Scale);
    }

    if (CountdownText)
    {
        CountdownText->SetPos(
            PanelLeft + PanelWidth - 280.f * Scale,
            PanelTop + 18.f * Scale);
        CountdownText->SetSize(214.f * Scale, 28.f * Scale);
        CountdownText->SetFontSize(15.f * Scale);
    }

    if (CloseButton)
    {
        CloseButton->SetPos(
            PanelLeft + PanelWidth - 52.f * Scale,
            PanelTop + 12.f * Scale);
        CloseButton->SetSize(CloseSize, CloseSize);
    }

    if (FilterButton)
    {
        FilterButton->SetPos(LeftLeft, ContentTop);
        FilterButton->SetSize(194.f * Scale, FilterHeight);
    }

    if (FilterButtonText)
    {
        FilterButtonText->SetPos(0.f, 0.f);
        FilterButtonText->SetSize(194.f * Scale, FilterHeight);
        FilterButtonText->SetFontSize(16.f * Scale);
    }

    const float SortButtonWidth =
        (LeftWidth - 3.f * (8.f * Scale)) / 4.f;

    for (int Index = 0; Index < GTradeSortCount; ++Index)
    {
        auto Button = mSortButtons[static_cast<size_t>(Index)].lock();
        auto Text = mSortButtonTexts[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        Button->SetPos(
            LeftLeft +
                (SortButtonWidth + 8.f * Scale) * static_cast<float>(Index),
            ContentTop + FilterHeight + 10.f * Scale);
        Button->SetSize(SortButtonWidth, SortHeight);

        if (Text)
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(SortButtonWidth, SortHeight);
            Text->SetFontSize(15.f * Scale);
        }
    }

    const float DirectionWidth = 72.f * Scale;
    const float PartnerWidth =
        ((ShowingActiveRoutes || ShowingCompletedRoutes) ?
            108.f :
            ShowingProductPrices ? 118.f : 120.f) * Scale;
    const float MarginWidth =
        ((ShowingActiveRoutes || ShowingCompletedRoutes) ?
            150.f :
            ShowingProductPrices ? 120.f : 84.f) * Scale;
    const float ResourceWidth =
        LeftWidth - DirectionWidth - PartnerWidth - MarginWidth - 48.f * Scale;

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
                RowTop = ContentTop + 66.f * Scale +
                    static_cast<float>(RowIndex) * (RowHeight + 8.f * Scale);
            }
            else if (RowIndex < 6)
            {
                RowTop = ContentTop + 256.f * Scale +
                    static_cast<float>(RowIndex - 2) * (RowHeight + 8.f * Scale);
            }
            else
            {
                RowTop = ContentTop + 508.f * Scale +
                    static_cast<float>(RowIndex - 6) * (RowHeight + 8.f * Scale);
            }
        }

        if (Button)
        {
            Button->SetPos(LeftLeft + 10.f * Scale, RowTop);
            Button->SetSize(LeftWidth - 20.f * Scale, RowHeight);
        }

        if (Direction)
        {
            Direction->SetPos(12.f * Scale, 0.f);
            Direction->SetSize(DirectionWidth, RowHeight);
            Direction->SetFontSize(15.f * Scale);
        }

        if (Partner)
        {
            Partner->SetPos(18.f * Scale + DirectionWidth, 0.f);
            Partner->SetSize(PartnerWidth, RowHeight);
            Partner->SetFontSize(15.f * Scale);
        }

        if (Resource)
        {
            Resource->SetPos(
                ShowingPriceModifiers ? 20.f * Scale :
                    24.f * Scale + DirectionWidth + PartnerWidth,
                0.f);
            Resource->SetSize(
                ShowingPriceModifiers ?
                    LeftWidth - 156.f * Scale :
                    ResourceWidth,
                RowHeight);
            Resource->SetFontSize(16.f * Scale);
        }

        if (Margin)
        {
            Margin->SetPos(
                ShowingPriceModifiers ?
                    LeftWidth - 136.f * Scale :
                    26.f * Scale + DirectionWidth + PartnerWidth + ResourceWidth,
                0.f);
            Margin->SetSize(
                ShowingPriceModifiers ? 112.f * Scale : MarginWidth,
                RowHeight);
            Margin->SetFontSize(15.f * Scale);
        }
    }

    if (ShowingPriceModifiers)
    {
        const float LeftTitleWidth = LeftWidth - 32.f * Scale;
        const float RightTitleWidth = RightWidth - 56.f * Scale;
        const float RightSectionTop = ContentTop + 212.f * Scale;

        if (ModifierSectionTitles[0])
        {
            ModifierSectionTitles[0]->SetPos(
                LeftLeft + 10.f * Scale,
                ContentTop + 12.f * Scale);
            ModifierSectionTitles[0]->SetSize(LeftTitleWidth, 28.f * Scale);
            ModifierSectionTitles[0]->SetFontSize(18.f * Scale);
        }

        if (ModifierSectionTitles[1])
        {
            ModifierSectionTitles[1]->SetPos(
                RightLeft + 28.f * Scale,
                ContentTop + 12.f * Scale);
            ModifierSectionTitles[1]->SetSize(RightTitleWidth, 28.f * Scale);
            ModifierSectionTitles[1]->SetFontSize(18.f * Scale);
        }

        if (ModifierSectionTitles[2])
        {
            ModifierSectionTitles[2]->SetPos(
                LeftLeft + 10.f * Scale,
                ContentTop + 212.f * Scale);
            ModifierSectionTitles[2]->SetSize(LeftTitleWidth, 28.f * Scale);
            ModifierSectionTitles[2]->SetFontSize(18.f * Scale);
        }

        if (ModifierSectionTitles[3])
        {
            ModifierSectionTitles[3]->SetPos(
                RightLeft + 28.f * Scale,
                RightSectionTop);
            ModifierSectionTitles[3]->SetSize(RightTitleWidth, 28.f * Scale);
            ModifierSectionTitles[3]->SetFontSize(18.f * Scale);
        }

        if (ModifierSectionTitles[4])
        {
            ModifierSectionTitles[4]->SetPos(
                LeftLeft + 10.f * Scale,
                ContentTop + 464.f * Scale);
            ModifierSectionTitles[4]->SetSize(LeftTitleWidth, 28.f * Scale);
            ModifierSectionTitles[4]->SetFontSize(18.f * Scale);
        }
    }

    if (DetailTitleText)
    {
        DetailTitleText->SetPos(
            RightLeft + 28.f * Scale,
            ContentTop + 16.f * Scale);
        DetailTitleText->SetSize(RightWidth - 56.f * Scale, 34.f * Scale);
        DetailTitleText->SetFontSize(22.f * Scale);
    }

    const float DetailLabelWidth = 150.f * Scale;
    const float DetailValueWidth = RightWidth - DetailLabelWidth - 68.f * Scale;
    const float DetailRowHeight = 28.f * Scale;
    const float DetailStartTop = ShowingPriceModifiers ?
        ContentTop + 254.f * Scale :
        ContentTop + 70.f * Scale;

    for (int Index = 0; Index < GTradeDetailRowCount; ++Index)
    {
        auto Label = mDetailRows[static_cast<size_t>(Index)].Label.lock();
        auto Value = mDetailRows[static_cast<size_t>(Index)].Value.lock();
        const float RowTop =
            DetailStartTop + static_cast<float>(Index) * 34.f * Scale;

        if (Label)
        {
            Label->SetPos(RightLeft + 28.f * Scale, RowTop);
            Label->SetSize(
                ShowingPriceModifiers ?
                    RightWidth - 180.f * Scale :
                    DetailLabelWidth,
                DetailRowHeight);
            Label->SetFontSize(15.f * Scale);
        }

        if (Value)
        {
            Value->SetPos(
                RightLeft +
                    (ShowingPriceModifiers ?
                        RightWidth - 136.f * Scale :
                        28.f * Scale + DetailLabelWidth),
                RowTop);
            Value->SetSize(
                ShowingPriceModifiers ?
                    108.f * Scale :
                    DetailValueWidth,
                DetailRowHeight);
            Value->SetFontSize(17.f * Scale);
        }
    }

    if (AmountTitleText)
    {
        AmountTitleText->SetPos(
            RightLeft + 28.f * Scale,
            ContentTop + 402.f * Scale);
        AmountTitleText->SetSize(130.f * Scale, 28.f * Scale);
        AmountTitleText->SetFontSize(17.f * Scale);
    }

    const float AmountButtonWidth =
        ShowingProductPrices ?
            (RightWidth - 66.f * Scale) / 2.f :
            (RightWidth - 76.f * Scale) / 3.f;

    for (int Index = 0; Index < GTradeAmountPresetCount; ++Index)
    {
        auto Button = mAmountButtons[static_cast<size_t>(Index)].lock();
        auto Text = mAmountButtonTexts[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        const float ButtonGap = ShowingProductPrices ? 10.f * Scale : 10.f * Scale;
        Button->SetPos(
            RightLeft + 28.f * Scale +
                static_cast<float>(Index) * (AmountButtonWidth + ButtonGap),
            ContentTop + 438.f * Scale);
        Button->SetSize(AmountButtonWidth, 40.f * Scale);

        if (Text)
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(AmountButtonWidth, 40.f * Scale);
            Text->SetFontSize(15.f * Scale);
        }
    }

    if (ActionButton)
    {
        ActionButton->SetPos(
            RightLeft + 28.f * Scale,
            ContentTop + 496.f * Scale);
        ActionButton->SetSize(RightWidth - 56.f * Scale, 46.f * Scale);
    }

    if (ActionButtonText)
    {
        ActionButtonText->SetPos(0.f, 0.f);
        ActionButtonText->SetSize(RightWidth - 56.f * Scale, 46.f * Scale);
        ActionButtonText->SetFontSize(18.f * Scale);
    }

    if (CompletionAutoOpenButton)
    {
        CompletionAutoOpenButton->SetPos(
            RightLeft + 28.f * Scale,
            ContentTop + 548.f * Scale);
        CompletionAutoOpenButton->SetSize(54.f * Scale, 40.f * Scale);
    }

    if (CompletionAutoOpenButtonText)
    {
        CompletionAutoOpenButtonText->SetPos(0.f, 0.f);
        CompletionAutoOpenButtonText->SetSize(54.f * Scale, 40.f * Scale);
        CompletionAutoOpenButtonText->SetFontSize(16.f * Scale);
    }

    if (FeedbackText)
    {
        FeedbackText->SetPos(
            RightLeft + (ShowingCompletedRoutes ? 92.f * Scale : 28.f * Scale),
            ShowingPriceModifiers ?
                ContentTop + 56.f * Scale :
            ShowingCompletedRoutes ?
                ContentTop + 552.f * Scale :
                ContentTop + 558.f * Scale);
        FeedbackText->SetSize(
            RightWidth - (ShowingCompletedRoutes ? 120.f * Scale : 56.f * Scale),
            ShowingPriceModifiers ?
                112.f * Scale :
            ShowingCompletedRoutes ?
                72.f * Scale :
                124.f * Scale);
        FeedbackText->SetFontSize(15.f * Scale);
    }
}
