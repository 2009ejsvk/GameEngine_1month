#include "EdictRenderer.h"
#include "EdictRendererInternal.h"
#include "Device.h"
#include <algorithm>

void FEdictRenderer::RefreshLayout(CEdictWidget& Widget)
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(520.f, ScreenWidth - 120.f);
    const float AvailableHeight = (std::max)(520.f, ScreenHeight - 140.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(AvailableWidth / Widget.mPanelWidth,
                AvailableHeight / Widget.mPanelHeight));
    const float PanelWidth = Widget.mPanelWidth * Scale;
    const float PanelHeight = Widget.mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;
    const float HorizontalMargin = 24.f * Scale;
    const float VerticalMargin = 18.f * Scale;
    const float HeaderTopPadding = 40.f * Scale;
    const float HeaderHeight = 48.f * Scale;
    const float TitleRibbonWidth = PanelWidth - 118.f * Scale;
    const float TitleRibbonLeft =
        PanelLeft + (PanelWidth - TitleRibbonWidth) * 0.5f;
    const float GridFrameLeft = PanelLeft + HorizontalMargin;
    const float GridFrameTop =
        PanelTop + HeaderTopPadding + HeaderHeight + 10.f * Scale;
    const float GridFrameWidth = PanelWidth - HorizontalMargin * 2.f;
    const float GridFrameHeight = 404.f * Scale;
    const float DetailFrameLeft = GridFrameLeft;
    const float DetailFrameTop =
        GridFrameTop + GridFrameHeight + 12.f * Scale;
    const float DetailFrameWidth = GridFrameWidth;
    const float DetailFrameHeight =
        PanelTop + PanelHeight - DetailFrameTop - VerticalMargin;

    auto MenuBackground = Widget.mMenuBackground.lock();
    auto MenuTitleRibbon = Widget.mMenuTitleRibbon.lock();
    auto MenuGridFrame = Widget.mMenuGridFrame.lock();
    auto MenuDetailFrame = Widget.mMenuDetailFrame.lock();
    auto DetailInfoPanel = Widget.mDetailInfoPanel.lock();
    auto TaxInfoPanel = Widget.mTaxInfoPanel.lock();
    auto ScrollTrack = Widget.mScrollTrack.lock();
    auto ScrollThumb = Widget.mScrollThumb.lock();
    auto TitleText = Widget.mTitleText.lock();
    auto PageText = Widget.mPageText.lock();
    auto CloseButton = Widget.mCloseButton.lock();
    auto PrevPageButton = Widget.mPrevPageButton.lock();
    auto NextPageButton = Widget.mNextPageButton.lock();

    if (MenuBackground)
    {
        MenuBackground->SetPos(PanelLeft, PanelTop);
        MenuBackground->SetSize(PanelWidth, PanelHeight);
    }

    if (MenuTitleRibbon)
    {
        MenuTitleRibbon->SetPos(TitleRibbonLeft, PanelTop + HeaderTopPadding);
        MenuTitleRibbon->SetSize(TitleRibbonWidth, HeaderHeight);
    }

    if (MenuGridFrame)
    {
        MenuGridFrame->SetPos(GridFrameLeft, GridFrameTop);
        MenuGridFrame->SetSize(GridFrameWidth, GridFrameHeight);
    }

    if (MenuDetailFrame)
    {
        MenuDetailFrame->SetPos(DetailFrameLeft, DetailFrameTop);
        MenuDetailFrame->SetSize(DetailFrameWidth, DetailFrameHeight);
    }

    if (TitleText)
    {
        TitleText->SetFontSize(30.f * Scale);
        TitleText->SetPos(
            TitleRibbonLeft + 32.f * Scale,
            PanelTop + HeaderTopPadding);
        TitleText->SetSize(TitleRibbonWidth - 64.f * Scale, HeaderHeight);
    }

    if (CloseButton)
    {
        CloseButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 44.f * Scale,
            PanelTop + HeaderTopPadding - 2.f * Scale);
        CloseButton->SetSize(40.f * Scale, 40.f * Scale);
    }

    const int PageCount = (std::max)(1, Widget.mPageCount);
    const bool ShowScrollBar = PageCount > 1;
    const float ScrollTrackWidth = 10.f * Scale;
    const float ScrollTrackLeft =
        GridFrameLeft + GridFrameWidth - 18.f * Scale;
    const float ScrollTrackTop = GridFrameTop + 24.f * Scale;
    const float ScrollTrackHeight = GridFrameHeight - 48.f * Scale;

    if (PrevPageButton)
    {
        PrevPageButton->SetPos(0.f, 0.f);
        PrevPageButton->SetSize(0.f, 0.f);
        PrevPageButton->SetEnable(false);
    }

    if (PageText)
    {
        PageText->SetPos(0.f, 0.f);
        PageText->SetSize(0.f, 0.f);
        PageText->SetEnable(false);
    }

    if (NextPageButton)
    {
        NextPageButton->SetPos(0.f, 0.f);
        NextPageButton->SetSize(0.f, 0.f);
        NextPageButton->SetEnable(false);
    }

    if (ScrollTrack)
    {
        ScrollTrack->SetPos(ScrollTrackLeft, ScrollTrackTop);
        ScrollTrack->SetSize(ScrollTrackWidth, ScrollTrackHeight);
        ScrollTrack->SetEnable(Widget.mOpen && ShowScrollBar);
    }

    if (ScrollThumb)
    {
        const float ThumbHeight =
            ShowScrollBar ?
            (std::max)(56.f * Scale,
                ScrollTrackHeight / static_cast<float>(PageCount)) :
            ScrollTrackHeight;
        const float ThumbTravel = (std::max)(0.f, ScrollTrackHeight - ThumbHeight);
        const float ThumbOffset =
            PageCount > 1 ?
            ThumbTravel *
            (static_cast<float>(Widget.mCurrentPage) /
                static_cast<float>(PageCount - 1)) :
            0.f;

        ScrollThumb->SetPos(ScrollTrackLeft, ScrollTrackTop + ThumbOffset);
        ScrollThumb->SetSize(ScrollTrackWidth, ThumbHeight);
        ScrollThumb->SetEnable(Widget.mOpen && ShowScrollBar);
    }

    const float CategoryGap = 12.f * Scale;
    const float CategoryWidth = 74.f * Scale;
    const float CategoryHeight = 90.f * Scale;
    const float CategoryStartX =
        PanelLeft +
        (PanelWidth -
            (CategoryWidth * static_cast<float>(GEdictCategoryCount) +
                CategoryGap * static_cast<float>(GEdictCategoryCount - 1))) *
        0.5f;
    const float CategoryTop = PanelTop - 10.f * Scale;

    for (int i = 0; i < static_cast<int>(Widget.mCategoryButtons.size()); ++i)
    {
        auto CategoryButton = Widget.mCategoryButtons[i].lock();
        auto CategoryIcon = i < static_cast<int>(Widget.mCategoryButtonIcons.size()) ?
            Widget.mCategoryButtonIcons[i].lock() : nullptr;

        if (!CategoryButton)
            continue;

        CategoryButton->SetPos(
            CategoryStartX +
            (CategoryWidth + CategoryGap) * static_cast<float>(i),
            CategoryTop);
        CategoryButton->SetSize(CategoryWidth, CategoryHeight);

        if (CategoryIcon)
        {
            CategoryIcon->SetPos(17.f * Scale, 10.f * Scale);
            CategoryIcon->SetSize(40.f * Scale, 40.f * Scale);
        }
    }

    const float SlotPaddingLeft = 18.f * Scale;
    const float SlotPaddingTop = 26.f * Scale;
    const float SlotPaddingRight = 22.f * Scale;
    const float SlotPaddingBottom = 22.f * Scale;
    const float SlotAreaLeft = GridFrameLeft + SlotPaddingLeft;
    const float SlotAreaTop = GridFrameTop + SlotPaddingTop;
    const float SlotGapX = 12.f * Scale;
    const float SlotGapY = 14.f * Scale;
    const float SlotAreaWidth =
        GridFrameWidth - SlotPaddingLeft - SlotPaddingRight -
        ScrollTrackWidth - 22.f * Scale;
    const float SlotAreaHeight =
        GridFrameHeight - SlotPaddingTop - SlotPaddingBottom;
    const float SlotWidth =
        (SlotAreaWidth - SlotGapX * (GEdictSlotColumnCount - 1)) /
        static_cast<float>(GEdictSlotColumnCount);
    const float SlotHeight =
        (SlotAreaHeight - SlotGapY * (GEdictSlotRowCount - 1)) /
        static_cast<float>(GEdictSlotRowCount);

    for (int i = 0; i < static_cast<int>(Widget.mEdictButtons.size()); ++i)
    {
        const int Row = i / GEdictSlotColumnCount;
        const int Col = i % GEdictSlotColumnCount;
        auto SlotButton = Widget.mEdictButtons[i].lock();
        auto SlotIcon = i < static_cast<int>(Widget.mEdictButtonIcons.size()) ?
            Widget.mEdictButtonIcons[i].lock() : nullptr;
        auto StarLeft = i < static_cast<int>(Widget.mEdictButtonStarLefts.size()) ?
            Widget.mEdictButtonStarLefts[i].lock() : nullptr;
        auto StarRight = i < static_cast<int>(Widget.mEdictButtonStarRights.size()) ?
            Widget.mEdictButtonStarRights[i].lock() : nullptr;
        auto SlotCheck = i < static_cast<int>(Widget.mEdictButtonChecks.size()) ?
            Widget.mEdictButtonChecks[i].lock() : nullptr;
        auto SlotText = i < static_cast<int>(Widget.mEdictButtonTexts.size()) ?
            Widget.mEdictButtonTexts[i].lock() : nullptr;

        if (!SlotButton)
            continue;

        SlotButton->SetPos(
            SlotAreaLeft +
            (SlotWidth + SlotGapX) * static_cast<float>(Col),
            SlotAreaTop + (SlotHeight + SlotGapY) * static_cast<float>(Row));
        SlotButton->SetSize(SlotWidth, SlotHeight);

        const float StarSize = 24.f * Scale;
        const float StarGap = 2.f * Scale;
        const float StarTop = 3.f * Scale;
        const float StarStartX = (SlotWidth - StarSize * 2.f - StarGap) * 0.5f;

        if (StarLeft)
        {
            StarLeft->SetPos(StarStartX, StarTop);
            StarLeft->SetSize(StarSize, StarSize);
        }

        if (StarRight)
        {
            StarRight->SetPos(StarStartX + StarSize + StarGap, StarTop);
            StarRight->SetSize(StarSize, StarSize);
        }

        if (SlotIcon)
        {
            const float IconSize = (std::min)(
                SlotWidth - 22.f * Scale,
                SlotHeight - 66.f * Scale);
            SlotIcon->SetPos(
                (SlotWidth - IconSize) * 0.5f,
                26.f * Scale);
            SlotIcon->SetSize(IconSize, IconSize);
        }

        if (SlotCheck)
        {
            const float CheckSize = 38.f * Scale;
            SlotCheck->SetPos(
                SlotWidth - CheckSize - 7.f * Scale,
                SlotHeight - CheckSize - 7.f * Scale);
            SlotCheck->SetSize(CheckSize, CheckSize);
        }

        if (SlotText)
        {
            SlotText->SetPos(10.f * Scale, SlotHeight - 44.f * Scale);
            SlotText->SetSize(SlotWidth - 20.f * Scale, 38.f * Scale);
            SlotText->SetFontSize(14.5f * Scale);
        }
    }

    auto DetailTitleText = Widget.mDetailTitleText.lock();
    auto DetailCostIcon = Widget.mDetailCostIcon.lock();
    auto DetailCostText = Widget.mDetailCostText.lock();
    auto DetailInfoText = Widget.mDetailInfoText.lock();
    auto FeedbackText = Widget.mFeedbackText.lock();
    auto RequirementText = Widget.mRequirementText.lock();
    auto DetailBodyText = Widget.mDetailBodyText.lock();
    auto ApplyButton = Widget.mApplyButton.lock();
    auto ApplyButtonText = Widget.mApplyButtonText.lock();
    auto TaxPolicyTitleText = Widget.mTaxPolicyTitleText.lock();
    auto TaxPolicySummaryText = Widget.mTaxPolicySummaryText.lock();
    const float DetailInnerLeft = DetailFrameLeft + 16.f * Scale;
    const float DetailInnerWidth = DetailFrameWidth - 32.f * Scale;
    const float DetailHeaderCenterX = DetailFrameLeft + DetailFrameWidth * 0.5f;
    const float DetailTitleTop = DetailFrameTop + 10.f * Scale;
    const float DetailTitleWidth = DetailFrameWidth * 0.52f;
    const float DetailTitleHeight = 30.f * Scale;
    const float CostIconSize = 24.f * Scale;
    const float CostTop = DetailTitleTop + DetailTitleHeight - 3.f * Scale;
    const float CostRowWidth = 140.f * Scale;
    const float CostLeft = DetailHeaderCenterX - CostRowWidth * 0.5f;
    const float DetailInfoTop = CostTop + 28.f * Scale;
    const float DetailInfoHeight = 20.f * Scale;
    const float FeedbackTop = DetailInfoTop + DetailInfoHeight + 2.f * Scale;
    const float FeedbackHeight = 22.f * Scale;
    const float ApplyButtonWidth = 140.f * Scale;
    const float ApplyButtonHeight = 38.f * Scale;
    const float ApplyButtonTop = DetailFrameTop + 14.f * Scale;
    const float ApplyButtonLeft =
        DetailFrameLeft + DetailFrameWidth - ApplyButtonWidth - 18.f * Scale;
    const float InfoPanelTop = DetailFrameTop + 74.f * Scale;
    const float InfoPanelHeight =
        DetailFrameTop + DetailFrameHeight - InfoPanelTop - 14.f * Scale;
    const float FeedbackGap = 8.f * Scale;
    const float DetailBodyLeft = DetailInnerLeft + 14.f * Scale;
    const float DetailBodyTop = FeedbackTop + FeedbackHeight + FeedbackGap;
    const float DetailBodyWidth = DetailInnerWidth - 28.f * Scale;
    const float RequirementHeight = 28.f * Scale;
    const float RequirementTop =
        InfoPanelTop + InfoPanelHeight - RequirementHeight - 8.f * Scale;
    const float DetailBodyBottom = RequirementTop - 10.f * Scale;
    const float DetailBodyHeight =
        (std::max)(0.f, DetailBodyBottom - DetailBodyTop);
    const float TaxPanelHeight = 0.f;
    const float TaxSummaryHeight = 0.f;
    const bool ShowTaxPanel = GEnableTaxPolicyPanel && TaxPanelHeight > 0.f;
    const float TaxButtonWidth = 0.f;
    const float TaxButtonHeight = 0.f;

    if (DetailTitleText)
    {
        DetailTitleText->SetFontSize(23.f * Scale);
        DetailTitleText->SetPos(
            DetailHeaderCenterX - DetailTitleWidth * 0.5f,
            DetailTitleTop);
        DetailTitleText->SetSize(DetailTitleWidth, DetailTitleHeight);
    }

    if (DetailCostIcon)
    {
        DetailCostIcon->SetPos(CostLeft, CostTop);
        DetailCostIcon->SetSize(CostIconSize, CostIconSize);
    }

    if (DetailCostText)
    {
        DetailCostText->SetPos(
            CostLeft + CostIconSize + 8.f * Scale,
            CostTop - 2.f * Scale);
        DetailCostText->SetSize(
            CostRowWidth - CostIconSize - 8.f * Scale,
            30.f * Scale);
        DetailCostText->SetFontSize(18.f * Scale);
    }

    if (DetailInfoPanel)
    {
        DetailInfoPanel->SetPos(DetailInnerLeft, InfoPanelTop);
        DetailInfoPanel->SetSize(DetailInnerWidth, InfoPanelHeight);
    }

    if (DetailInfoText)
    {
        DetailInfoText->SetPos(DetailInnerLeft, DetailInfoTop);
        DetailInfoText->SetSize(DetailInnerWidth, DetailInfoHeight);
        DetailInfoText->SetFontSize(12.5f * Scale);
    }

    if (ApplyButton)
    {
        ApplyButton->SetPos(ApplyButtonLeft, ApplyButtonTop);
        ApplyButton->SetSize(ApplyButtonWidth, ApplyButtonHeight);
    }

    if (ApplyButtonText)
        ApplyButtonText->SetFontSize(16.f * Scale);

    if (FeedbackText)
    {
        FeedbackText->SetPos(DetailInnerLeft, FeedbackTop);
        FeedbackText->SetSize(DetailInnerWidth, FeedbackHeight);
        FeedbackText->SetFontSize(12.5f * Scale);
    }

    if (DetailBodyText)
    {
        DetailBodyText->SetPos(DetailBodyLeft, DetailBodyTop);
        DetailBodyText->SetSize(DetailBodyWidth, DetailBodyHeight);
        DetailBodyText->SetFontSize(15.f * Scale);
    }

    if (RequirementText)
    {
        RequirementText->SetPos(DetailInnerLeft, RequirementTop);
        RequirementText->SetSize(DetailInnerWidth, RequirementHeight);
        RequirementText->SetFontSize(13.5f * Scale);
    }

    if (TaxInfoPanel)
    {
        TaxInfoPanel->SetPos(0.f, 0.f);
        TaxInfoPanel->SetSize(0.f, 0.f);
        TaxInfoPanel->SetEnable(Widget.mOpen && ShowTaxPanel);
    }

    if (TaxPolicyTitleText)
    {
        TaxPolicyTitleText->SetPos(0.f, 0.f);
        TaxPolicyTitleText->SetSize(0.f, 0.f);
        TaxPolicyTitleText->SetEnable(Widget.mOpen && ShowTaxPanel);
    }

    for (int i = 0; i < GTaxPolicyRowCount; ++i)
    {
        auto RowText = Widget.mTaxPolicyRowTexts[i].lock();
        auto DecreaseButton = Widget.mTaxDecreaseButtons[i].lock();
        auto IncreaseButton = Widget.mTaxIncreaseButtons[i].lock();

        if (RowText)
        {
            RowText->SetPos(0.f, 0.f);
            RowText->SetSize(0.f, 0.f);
            RowText->SetEnable(Widget.mOpen && ShowTaxPanel);
        }

        if (DecreaseButton)
        {
            DecreaseButton->SetPos(0.f, 0.f);
            DecreaseButton->SetSize(TaxButtonWidth, TaxButtonHeight);
            DecreaseButton->SetEnable(Widget.mOpen && ShowTaxPanel);
        }

        if (IncreaseButton)
        {
            IncreaseButton->SetPos(0.f, 0.f);
            IncreaseButton->SetSize(TaxButtonWidth, TaxButtonHeight);
            IncreaseButton->SetEnable(Widget.mOpen && ShowTaxPanel);
        }
    }

    if (TaxPolicySummaryText)
    {
        TaxPolicySummaryText->SetPos(0.f, 0.f);
        TaxPolicySummaryText->SetSize(0.f, 0.f);
        TaxPolicySummaryText->SetEnable(Widget.mOpen && ShowTaxPanel);
    }
}

void FEdictRenderer::ApplyOpenState(CEdictWidget& Widget)
{
    auto MenuBackground = Widget.mMenuBackground.lock();
    auto MenuTitleRibbon = Widget.mMenuTitleRibbon.lock();
    auto MenuGridFrame = Widget.mMenuGridFrame.lock();
    auto MenuDetailFrame = Widget.mMenuDetailFrame.lock();
    auto DetailInfoPanel = Widget.mDetailInfoPanel.lock();
    auto TaxInfoPanel = Widget.mTaxInfoPanel.lock();
    auto DetailCostIcon = Widget.mDetailCostIcon.lock();
    auto ScrollTrack = Widget.mScrollTrack.lock();
    auto ScrollThumb = Widget.mScrollThumb.lock();
    auto TitleText = Widget.mTitleText.lock();
    auto PageText = Widget.mPageText.lock();
    auto DetailTitleText = Widget.mDetailTitleText.lock();
    auto DetailCostText = Widget.mDetailCostText.lock();
    auto DetailInfoText = Widget.mDetailInfoText.lock();
    auto DetailBodyText = Widget.mDetailBodyText.lock();
    auto FeedbackText = Widget.mFeedbackText.lock();
    auto RequirementText = Widget.mRequirementText.lock();
    auto TaxPolicyTitleText = Widget.mTaxPolicyTitleText.lock();
    auto TaxPolicySummaryText = Widget.mTaxPolicySummaryText.lock();
    auto PrevPageButton = Widget.mPrevPageButton.lock();
    auto NextPageButton = Widget.mNextPageButton.lock();
    auto CloseButton = Widget.mCloseButton.lock();
    auto ApplyButton = Widget.mApplyButton.lock();
    auto ApplyButtonText = Widget.mApplyButtonText.lock();

    if (MenuBackground)
        MenuBackground->SetEnable(Widget.mOpen);
    if (MenuTitleRibbon)
        MenuTitleRibbon->SetEnable(Widget.mOpen);
    if (MenuGridFrame)
        MenuGridFrame->SetEnable(Widget.mOpen);
    if (MenuDetailFrame)
        MenuDetailFrame->SetEnable(Widget.mOpen);
    if (DetailInfoPanel)
        DetailInfoPanel->SetEnable(Widget.mOpen);
    if (TaxInfoPanel)
        TaxInfoPanel->SetEnable(Widget.mOpen);
    if (DetailCostIcon)
        DetailCostIcon->SetEnable(Widget.mOpen);
    if (ScrollTrack)
        ScrollTrack->SetEnable(Widget.mOpen);
    if (ScrollThumb)
        ScrollThumb->SetEnable(Widget.mOpen);
    if (TitleText)
        TitleText->SetEnable(Widget.mOpen);
    if (PageText)
        PageText->SetEnable(Widget.mOpen);
    if (DetailTitleText)
        DetailTitleText->SetEnable(Widget.mOpen);
    if (DetailCostText)
        DetailCostText->SetEnable(Widget.mOpen);
    if (DetailInfoText)
        DetailInfoText->SetEnable(Widget.mOpen);
    if (DetailBodyText)
        DetailBodyText->SetEnable(Widget.mOpen);
    if (FeedbackText)
        FeedbackText->SetEnable(Widget.mOpen);
    if (RequirementText)
        RequirementText->SetEnable(Widget.mOpen);
    if (TaxPolicyTitleText)
        TaxPolicyTitleText->SetEnable(Widget.mOpen);
    if (TaxPolicySummaryText)
        TaxPolicySummaryText->SetEnable(Widget.mOpen);
    if (PrevPageButton)
        PrevPageButton->SetEnable(Widget.mOpen);
    if (NextPageButton)
        NextPageButton->SetEnable(Widget.mOpen);
    if (CloseButton)
        CloseButton->SetEnable(Widget.mOpen);
    if (ApplyButton)
        ApplyButton->SetEnable(Widget.mOpen);
    if (ApplyButtonText)
        ApplyButtonText->SetEnable(Widget.mOpen);

    for (size_t i = 0; i < Widget.mCategoryButtons.size(); ++i)
    {
        auto Button = Widget.mCategoryButtons[i].lock();
        auto Icon = i < Widget.mCategoryButtonIcons.size() ?
            Widget.mCategoryButtonIcons[i].lock() : nullptr;

        if (Button)
            Button->SetEnable(Widget.mOpen);
        if (Icon)
            Icon->SetEnable(Widget.mOpen);
    }

    for (size_t i = 0; i < Widget.mEdictButtons.size(); ++i)
    {
        const bool HasEntry =
            i < Widget.mVisibleEntryIndices.size() &&
            Widget.mVisibleEntryIndices[i] >= 0;
        const bool SlotVisible = Widget.mOpen && HasEntry;
        auto Button = Widget.mEdictButtons[i].lock();
        auto Icon = i < Widget.mEdictButtonIcons.size() ?
            Widget.mEdictButtonIcons[i].lock() : nullptr;
        auto StarLeft = i < Widget.mEdictButtonStarLefts.size() ?
            Widget.mEdictButtonStarLefts[i].lock() : nullptr;
        auto StarRight = i < Widget.mEdictButtonStarRights.size() ?
            Widget.mEdictButtonStarRights[i].lock() : nullptr;
        auto Check = i < Widget.mEdictButtonChecks.size() ?
            Widget.mEdictButtonChecks[i].lock() : nullptr;
        auto ButtonText = i < Widget.mEdictButtonTexts.size() ?
            Widget.mEdictButtonTexts[i].lock() : nullptr;

        if (Button)
            Button->SetEnable(SlotVisible);
        if (Icon)
            Icon->SetEnable(SlotVisible && Icon->GetEnable());
        if (StarLeft)
            StarLeft->SetEnable(SlotVisible && StarLeft->GetEnable());
        if (StarRight)
            StarRight->SetEnable(SlotVisible && StarRight->GetEnable());
        if (Check)
            Check->SetEnable(SlotVisible && Check->GetEnable());
        if (ButtonText)
            ButtonText->SetEnable(SlotVisible);
    }

    for (size_t i = 0; i < Widget.mTaxPolicyRowTexts.size(); ++i)
    {
        auto RowText = Widget.mTaxPolicyRowTexts[i].lock();
        auto DecreaseButton = Widget.mTaxDecreaseButtons[i].lock();
        auto IncreaseButton = Widget.mTaxIncreaseButtons[i].lock();

        if (RowText)
            RowText->SetEnable(Widget.mOpen);
        if (DecreaseButton)
            DecreaseButton->SetEnable(Widget.mOpen);
        if (IncreaseButton)
            IncreaseButton->SetEnable(Widget.mOpen);
    }
}

bool FEdictRenderer::IsMouseOverPanel(
    const CEdictWidget& Widget,
    const FVector2& MousePos)
{
    if (!Widget.mOpen)
        return false;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(480.f, ScreenWidth - 80.f);
    const float AvailableHeight = (std::max)(420.f, ScreenHeight - 100.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(AvailableWidth / Widget.mPanelWidth,
                AvailableHeight / Widget.mPanelHeight));
    const float PanelWidth = Widget.mPanelWidth * Scale;
    const float PanelHeight = Widget.mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;

    return MousePos.x >= PanelLeft &&
        MousePos.x <= PanelLeft + PanelWidth &&
        MousePos.y >= PanelTop &&
        MousePos.y <= PanelTop + PanelHeight;
}
