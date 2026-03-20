#include "CitizenInfoRenderer.h"
#include "CitizenInfoRendererInternal.h"
#include "CitizenInfoWidget.h"
#include "CitizenInfoLayoutMetrics.h"
#include "UILayoutValues.h"
#include "Device.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>

using namespace CitizenInfoRendererInternal;

namespace
{
template <typename T>
bool IsEnabled(const std::weak_ptr<T>& Widget)
{
    if (auto Pinned = Widget.lock())
    {
        return Pinned->GetEnable();
    }

    return false;
}
}

FCitizenLayoutMetrics MakeLayoutMetrics(bool IsCitizenMode)
{
    const float TitleIconInsetX = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::TitleIconInsetX :
        UIConfig::BuildingTitleIconInsetX;
    const float TitleTextInsetX = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::TitleTextInsetX :
        UIConfig::BuildingTitleTextInsetX;
    const float TitleIconGap = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::TitleIconGap :
        UIConfig::BuildingTitleIconGap;
    const float SubtitleOffsetY = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::SubtitleOffsetY :
        UIConfig::BuildingSubtitleOffsetY;
    const float CollapsedSectionGap = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::CollapsedSectionGap :
        UIConfig::BuildingCollapsedSectionGap;
    const float BudgetLabelOffsetY = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::BudgetLabelOffsetY :
        UIConfig::BuildingBudgetLabelOffsetY;
    const float BudgetCustomButtonsOffsetY = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::BudgetCustomButtonsOffsetY :
        UIConfig::BuildingBudgetCustomButtonsOffsetY;
    const float BudgetWorkButtonsOffsetY = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::BudgetWorkButtonsOffsetY :
        UIConfig::BuildingBudgetWorkButtonsOffsetY;
    const float BudgetDefaultButtonsOffsetY = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::BudgetDefaultButtonsOffsetY :
        UIConfig::BuildingBudgetDefaultButtonsOffsetY;
    const float BudgetCompactGap = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::BudgetCompactGap :
        UIConfig::BuildingBudgetCompactGap;
    const float BudgetDefaultGap = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::BudgetDefaultGap :
        UIConfig::BuildingBudgetDefaultGap;
    const float OccupancyGapY = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::OccupancyGapY :
        UIConfig::BuildingOccupancyGapY;
    const float CompactControlHeight = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::CompactControlHeight :
        UIConfig::BuildingCompactControlHeight;
    const float CompactBudgetButtonWidth = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::CompactBudgetButtonWidth :
        UIConfig::BuildingCompactBudgetButtonWidth;
    const float SectionDividerWidth = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::SectionDividerWidth :
        UIConfig::BuildingSectionDividerWidth;
    const float SectionDividerHeight = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::SectionDividerHeight :
        UIConfig::BuildingSectionDividerHeight;
    const float ActionCompactIconSize = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::ActionCompactIconSize :
        UIConfig::BuildingActionCompactIconSize;
    const float ActionCompactIconOffsetY = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::ActionCompactIconOffsetY :
        UIConfig::BuildingActionCompactIconOffsetY;
    const float MoveCompactRightOffset = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::MoveCompactRightOffset :
        UIConfig::BuildingMoveCompactRightOffset;
    const float FocusCompactRightOffset = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::FocusCompactRightOffset :
        UIConfig::BuildingFocusCompactRightOffset;
    const float OverviewCommandGap = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::OverviewCommandGap :
        UIConfig::BuildingOverviewCommandGap;
    const float CitizenActionButtonHeight =
        UIConfig::CitizenInfoLayoutDefaults::ActionButtonHeight;
    const float CitizenActionGap =
        UIConfig::CitizenInfoLayoutDefaults::ActionGap;
    const float ActionIconInset =
        UIConfig::CitizenInfoLayoutDefaults::ActionIconInset;
    const float ActionIconSize = CompactControlHeight;
    const float BodyGapAfterSection = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::BodyGapAfterSection :
        UIConfig::BuildingBodyGapAfterSection;
    const float BodyGapAfterActions = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::BodyGapAfterActions :
        UIConfig::BuildingBodyGapAfterActions;
    const float BodyGapBeforeActions = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::BodyGapBeforeActions :
        UIConfig::BuildingBodyGapBeforeActions;
    const float BodyFallbackOffset = IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::BodyFallbackOffset :
        UIConfig::BuildingBodyFallbackOffset;

    return
    {
        IsCitizenMode ?
            UIConfig::CitizenPanelInnerTopOffset :
            UIConfig::BuildingPanelInnerMarginTop,
        IsCitizenMode ?
            UIConfig::CitizenPanelInnerBottomInset :
            UIConfig::BuildingPanelInnerBottomInset,
        IsCitizenMode ?
            UIConfig::CitizenScrollBottomInset :
            UIConfig::BuildingScrollBottomInset,
        IsCitizenMode ?
            UIConfig::CitizenScrollThumbTopOffset :
            UIConfig::BuildingScrollThumbTopOffset,
        IsCitizenMode ?
            UIConfig::CitizenCloseButtonOffsetY :
            UIConfig::BuildingCloseButtonOffsetY,
        TitleIconInsetX,
        TitleTextInsetX,
        TitleIconGap,
        SubtitleOffsetY,
        IsCitizenMode ?
            UIConfig::CitizenSectionRibbonOffsetY :
            UIConfig::BuildingSectionRibbonOffsetY,
        CollapsedSectionGap,
        IsCitizenMode ?
            UIConfig::CitizenBudgetBaseOffsetY :
            UIConfig::BuildingBudgetBaseOffsetY,
        BudgetLabelOffsetY,
        BudgetCustomButtonsOffsetY,
        BudgetWorkButtonsOffsetY,
        BudgetDefaultButtonsOffsetY,
        BudgetCompactGap,
        BudgetDefaultGap,
        OccupancyGapY,
        CompactControlHeight,
        CompactBudgetButtonWidth,
        SectionDividerWidth,
        SectionDividerHeight,
        ActionCompactIconSize,
        ActionCompactIconOffsetY,
        MoveCompactRightOffset,
        FocusCompactRightOffset,
        OverviewCommandGap,
        CitizenActionButtonHeight,
        CitizenActionGap,
        UIConfig::CitizenActionStackTopOffset,
        ActionIconInset,
        ActionIconSize,
        UIConfig::CitizenFooterBottomInset,
        BodyGapAfterSection,
        BodyGapAfterActions,
        BodyGapBeforeActions,
        BodyFallbackOffset,
        IsCitizenMode ?
            UIConfig::CitizenBodyBottomInset :
            UIConfig::BuildingBodyBottomInset
    };
}

void LayoutOverviewMetricScrollWidgets(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context,
    float MetricsTop,
    float MetricRowH)
{
    auto Widget = Owner.GetRendererView();
    auto ScrollTrack = Widget.mScrollTrack.lock();
    auto ScrollThumb = Widget.mScrollThumb.lock();

    if (!ScrollTrack || !ScrollThumb)
        return;

    if (Widget.mPanelMode != CCitizenInfoWidget::EPanelMode::Building ||
        !Owner.HasOverviewMetricScroll() ||
        Owner.GetOverviewMetricScrollVisibleLineCount() <= 0 ||
        Owner.GetOverviewMetricScrollTotalLineCount() <= 0 ||
        Owner.GetOverviewMetricScrollFirstRowIndex() >=
            CCitizenInfoWidget::GOverviewMetricRowCount)
    {
        ScrollTrack->SetPos(0.f, 0.f);
        ScrollTrack->SetSize(0.f, 0.f);
        ScrollTrack->SetEnable(false);
        ScrollThumb->SetPos(0.f, 0.f);
        ScrollThumb->SetSize(0.f, 0.f);
        ScrollThumb->SetEnable(false);
        return;
    }

    const int FirstRowIndex =
        (std::max)(0, Owner.GetOverviewMetricScrollFirstRowIndex());
    const int VisibleLineCount =
        Owner.GetOverviewMetricScrollVisibleLineCount();
    const int TotalLineCount =
        (std::max)(
            VisibleLineCount,
            Owner.GetOverviewMetricScrollTotalLineCount());
    const int MaxOffset = (std::max)(0, TotalLineCount - VisibleLineCount);
    const float TrackLeft =
        Context.Inner.MarginX * 0.5f - Context.Chrome.ScrollTrackWidth * 0.5f;
    const float TrackTop =
        MetricsTop + static_cast<float>(FirstRowIndex) * MetricRowH + 2.f;
    const float TrackHeight = (std::max)(
        24.f,
        static_cast<float>(VisibleLineCount) * MetricRowH - 4.f);
    const float ThumbHeight = (std::max)(
        18.f,
        TrackHeight *
            (static_cast<float>(VisibleLineCount) /
                static_cast<float>(TotalLineCount)));
    const float ThumbTravel = (std::max)(0.f, TrackHeight - ThumbHeight);
    const float ThumbRatio =
        MaxOffset > 0 ?
            static_cast<float>(Owner.GetOverviewMetricScrollOffset()) /
                static_cast<float>(MaxOffset) :
            0.f;

    ScrollTrack->SetPos(TrackLeft, TrackTop);
    ScrollTrack->SetSize(Context.Chrome.ScrollTrackWidth, TrackHeight);
    ScrollTrack->SetEnable(true);

    ScrollThumb->SetPos(
        TrackLeft + 0.5f,
        TrackTop + ThumbTravel * ThumbRatio);
    ScrollThumb->SetSize(Context.Chrome.ScrollTrackWidth - 1.f, ThumbHeight);
    ScrollThumb->SetEnable(true);
}

void FCitizenInfoRenderer::RefreshLayout(CCitizenInfoWidget& Owner)
{
    auto Widget = Owner.GetRendererView();
    (void)Widget.mRequestedScreenPos;
    const bool IsCitizenMode =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);

    Widget.mPanelWidth = (std::min)(
        IsCitizenMode ?
            UIConfig::CitizenPanelMaxWidth :
            UIConfig::BuildingPanelMaxWidth,
        (std::max)(
            IsCitizenMode ?
                UIConfig::CitizenPanelMinWidth :
                UIConfig::BuildingPanelMinWidth,
            ScreenWidth * (IsCitizenMode ?
                UIConfig::CitizenPanelWidthRatio :
                UIConfig::BuildingPanelWidthRatio)));
    Widget.mPanelTop = IsCitizenMode ?
        UIConfig::CitizenPanelTopOffset :
        UIConfig::BuildingPanelTopOffset;
    Widget.mPanelHeight = (std::max)(
        IsCitizenMode ?
            UIConfig::CitizenPanelMinHeight :
            UIConfig::BuildingPanelMinHeight,
        ScreenHeight - Widget.mPanelTop - (IsCitizenMode ?
            UIConfig::CitizenPanelBottomMargin :
            UIConfig::BuildingPanelBottomMargin));

    const float PanelLeft =
        ScreenWidth - Widget.mPanelWidth - (IsCitizenMode ?
            UIConfig::CitizenPanelRightInset :
            UIConfig::BuildingPanelRightInset);
    Widget.SetPos(PanelLeft, 0.f);
    Widget.SetSize(Widget.mPanelWidth, Widget.mPanelTop + Widget.mPanelHeight);

    FCitizenInfoLayoutContext Layout;
    Layout.Metrics = MakeLayoutMetrics(IsCitizenMode);
    Layout.Panel.OuterTop = Widget.mPanelTop;
    Layout.Panel.Width = Widget.mPanelWidth;
    Layout.Panel.Height = Widget.mPanelHeight;
    Layout.Inner.MarginX = IsCitizenMode ?
        UIConfig::CitizenPanelInnerMarginX :
        UIConfig::BuildingPanelInnerMarginX;
    Layout.Inner.Left = Layout.Inner.MarginX;
    Layout.Inner.Top =
        Layout.Panel.OuterTop + Layout.Metrics.PanelInnerTopOffset;
    Layout.Inner.Width = Layout.Panel.Width - Layout.Inner.MarginX * 2.f;
    Layout.Inner.Height =
        Layout.Panel.Height - Layout.Metrics.PanelInnerBottomInset;
    Layout.Chrome.ScrollTrackWidth = IsCitizenMode ?
        UIConfig::CitizenScrollTrackWidth :
        UIConfig::BuildingScrollTrackWidth;
    Layout.Chrome.ScrollThumbHeight = IsCitizenMode ?
        UIConfig::CitizenScrollThumbHeight :
        UIConfig::BuildingScrollThumbHeight;
    Layout.Ribbon.TitleHeight = IsCitizenMode ?
        UIConfig::CitizenTitleRibbonHeight :
        UIConfig::BuildingTitleRibbonHeight;
    Layout.Ribbon.SectionHeight = IsCitizenMode ?
        UIConfig::CitizenSectionRibbonHeight :
        UIConfig::BuildingSectionRibbonHeight;
    Layout.Chrome.CloseButtonSize = IsCitizenMode ?
        UIConfig::CitizenCloseButtonSize :
        UIConfig::BuildingCloseButtonSize;
    Layout.Chrome.IconSize = UIConfig::BuildingIconSize;
    Layout.Flags.IsCitizenMode = IsCitizenMode;
    Layout.Tabs.VisibleCount =
        Layout.Flags.IsCitizenMode ? GCitizenTabCount : GBuildingTabCount;
    Layout.Tabs.Width = UIConfig::BuildingTabWidth;
    Layout.Tabs.Height = UIConfig::BuildingTabHeight;
    Layout.Tabs.Gap = UIConfig::BuildingTabGap;
    Layout.Tabs.TotalWidth =
        static_cast<float>(Layout.Tabs.VisibleCount) * Layout.Tabs.Width +
        static_cast<float>((std::max)(0, Layout.Tabs.VisibleCount - 1)) *
            Layout.Tabs.Gap;
    Layout.Tabs.StartX =
        (std::max)(
            UIConfig::BuildingMinimumTabStartInset,
            (Layout.Panel.Width - Layout.Tabs.TotalWidth) * 0.5f);
    Layout.Ribbon.OffsetX = UIConfig::BuildingTitleRibbonOffsetX;
    Layout.Ribbon.OffsetY = UIConfig::BuildingTitleRibbonOffsetY;
    Layout.Chrome.CloseOffsetX = UIConfig::BuildingCloseButtonOffsetX;
    Layout.Flags.TitleIconVisible =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        IsEnabled(Widget.mTitleIcon);
    Layout.Chrome.TitleLeft = Layout.Flags.TitleIconVisible ?
        (Layout.Ribbon.OffsetX + Layout.Chrome.IconSize +
            Layout.Metrics.TitleIconGap) :
        (Layout.Ribbon.OffsetX + Layout.Metrics.TitleTextInsetX);
    Layout.Flags.ShowSectionRibbon = IsEnabled(Widget.mSectionRibbon);
    Layout.Flags.ShowCitizenProfile =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedCitizenTab ==
            CCitizenInfoWidget::ECitizenInfoTab::Overview &&
        IsEnabled(Widget.mCitizenFooterText);
    Layout.Flags.ShowCitizenPolitics =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedCitizenTab ==
            CCitizenInfoWidget::ECitizenInfoTab::Politics &&
        IsEnabled(Widget.mCitizenPoliticsSectionTitles[0]);
    Layout.Flags.ShowCitizenThoughts =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedCitizenTab ==
            CCitizenInfoWidget::ECitizenInfoTab::Thoughts &&
        IsEnabled(Widget.mCitizenThoughtTitleText);
    Layout.Ribbon.SectionY =
        Layout.Panel.OuterTop +
        Layout.Ribbon.OffsetY +
        Layout.Ribbon.TitleHeight +
        Layout.Metrics.SectionRibbonOffsetY;
    Layout.Flags.ShowWorkOverview =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview &&
        IsEnabled(Widget.mOverviewWorkModeLabel);
    Layout.Flags.ShowCustomOverview =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview &&
        !Layout.Flags.ShowWorkOverview &&
        IsEnabled(Widget.mResidentialOverviewBudgetLabel);
    Layout.Flags.ShowResidentialWorkMode =
        Layout.Flags.ShowCustomOverview &&
        IsEnabled(Widget.mResidentialOverviewWorkModeLabel);
    Layout.Flags.ShowOperationModeSelectionPage =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview &&
        Owner.GetState().Building.CustomsModeSelectionOpen;
    Layout.Flags.ShowAnyOverview =
        Layout.Flags.ShowCustomOverview || Layout.Flags.ShowWorkOverview;
    Layout.Flags.ShowStatsMetricRows =
        !Layout.Flags.ShowCitizenProfile &&
        !Layout.Flags.ShowCitizenPolitics &&
        !Layout.Flags.ShowCitizenThoughts &&
        !Layout.Flags.ShowAnyOverview &&
        IsEnabled(Widget.mStatsMetricLabels[0]);
    Layout.Flags.ShowEfficiencyMetricRows =
        !Layout.Flags.ShowCitizenProfile &&
        !Layout.Flags.ShowCitizenPolitics &&
        !Layout.Flags.ShowCitizenThoughts &&
        !Layout.Flags.ShowAnyOverview &&
        IsEnabled(Widget.mEfficiencyMetricLabels[0]);
    Layout.Flags.ShowCompactRows =
        Layout.Flags.ShowStatsMetricRows ||
        Layout.Flags.ShowEfficiencyMetricRows;
    Layout.Flags.ShowUpgradeCard = IsEnabled(Widget.mUpgradeCardBackground);
    Layout.Flags.ShowInformationParagraphs =
        IsEnabled(Widget.mInformationTopText);
    Layout.Flags.ShowOverviewCommandButton =
        IsEnabled(Widget.mOverviewCommandButton);
    Layout.Budget.BaseY =
        Layout.Panel.OuterTop +
        Layout.Ribbon.OffsetY +
        Layout.Ribbon.TitleHeight +
        Layout.Metrics.BudgetBaseOffsetY;
    Layout.Budget.ButtonHeight = UIConfig::BuildingBudgetButtonHeight;
    Layout.Budget.ButtonTop =
        Layout.Budget.BaseY + Layout.Metrics.BudgetWorkButtonsOffsetY;
    Layout.Budget.WorkModeTop =
        Layout.Budget.BaseY - Layout.Metrics.BudgetLabelOffsetY;
    Layout.Budget.WorkModeBoxTop =
        Layout.Budget.WorkModeTop + Layout.Metrics.CompactControlHeight;
    Layout.Budget.Margin = Layout.Ribbon.OffsetX;
    Layout.Budget.Gap = Layout.Flags.ShowAnyOverview ?
        Layout.Metrics.BudgetCompactGap :
        Layout.Metrics.BudgetDefaultGap;
    Layout.Budget.ButtonWidth = Layout.Flags.ShowAnyOverview ?
        Layout.Metrics.CompactBudgetButtonWidth :
        (Layout.Panel.Width - Layout.Budget.Margin * 2.f -
            Layout.Budget.Gap * 4.f) /
            5.f;
    Layout.Budget.OccupancyTop =
        Layout.Budget.ButtonTop +
        Layout.Budget.ButtonHeight +
        Layout.Metrics.OccupancyGapY;
    Layout.Flags.ShowActions =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview;
    Layout.Actions.Top =
        Layout.Panel.OuterTop +
        Layout.Panel.Height -
        UIConfig::BuildingActionButtonBottomMargin;

    ValidateLayoutForDebug(Owner, Layout);
    RefreshCommonLayout(Owner, Layout);
    RefreshCitizenModeLayout(Owner, Layout);
    RefreshBuildingModeLayout(Owner, Layout);
    RefreshInformationVisibilityLayout(Owner, Layout);
    RefreshUpgradeLayout(Owner, Layout);
    RefreshActionLayout(Owner, Layout);
    RefreshBodyLayout(Owner, Layout);
}

void FCitizenInfoRenderer::ValidateLayoutForDebug(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Layout)
{
    auto Widget = Owner.GetRendererView();
#ifndef NDEBUG
    auto IsPositiveFinite = [](float Value)
    {
        return std::isfinite(Value) && Value > 0.f;
    };
    auto IsRatio = [](float Value)
    {
        return std::isfinite(Value) &&
            Value >= -0.01f &&
            Value <= 1.01f;
    };

    const int ExpectedVisibleTabCount =
        Layout.Flags.IsCitizenMode ?
            CCitizenInfoWidget::GCitizenTabCount :
            CCitizenInfoWidget::GBuildingTabCount;
    const int SelectedTabIndex = Widget.GetSelectedTabIndexForCurrentMode();

    assert(IsPositiveFinite(Layout.Panel.Width));
    assert(IsPositiveFinite(Layout.Panel.Height));
    assert(IsPositiveFinite(Layout.Inner.Width));
    assert(IsPositiveFinite(Layout.Inner.Height));
    assert(IsPositiveFinite(Layout.Chrome.ScrollTrackWidth));
    assert(IsPositiveFinite(Layout.Chrome.ScrollThumbHeight));
    assert(IsPositiveFinite(Layout.Ribbon.TitleHeight));
    assert(IsPositiveFinite(Layout.Ribbon.SectionHeight));
    assert(IsPositiveFinite(Layout.Chrome.CloseButtonSize));
    assert(Layout.Tabs.VisibleCount == ExpectedVisibleTabCount);
    assert(SelectedTabIndex >= 0);
    assert(SelectedTabIndex < ExpectedVisibleTabCount);
    assert(UIConfig::CitizenPanelMinHeight > 0.f);
    assert(UIConfig::CitizenPanelRightInset >= 0.f);
    assert(Widget.mTabButtons.size() == CCitizenInfoWidget::GTabButtonCount);
    assert(
        Widget.mTabButtonTexts.size() == CCitizenInfoWidget::GTabButtonCount);
    assert(
        Widget.mTabButtonIcons.size() == CCitizenInfoWidget::GTabButtonCount);
    assert(
        Widget.mBudgetButtons.size() == CCitizenInfoWidget::GBudgetLevelCount);
    assert(
        Widget.mBudgetButtonTexts.size() ==
            CCitizenInfoWidget::GBudgetLevelCount);
    assert(
        Widget.mOverviewResidentIcons.size() ==
        CCitizenInfoWidget::GOverviewResidentSlotCount);
    assert(
        Widget.mResidentialOverviewResidentIcons.size() ==
        CCitizenInfoWidget::GOverviewResidentSlotCount);
    assert(
        Widget.mOverviewVisitorIcons.size() ==
        CCitizenInfoWidget::GOverviewVisitorSlotCount);
    assert(
        Widget.mOverviewMetricLabels.size() ==
        CCitizenInfoWidget::GOverviewMetricRowCount);
    assert(
        Widget.mOverviewMetricValues.size() ==
        CCitizenInfoWidget::GOverviewMetricRowCount);
    assert(
        Widget.mResidentialOverviewMetricLabels.size() ==
        CCitizenInfoWidget::GOverviewMetricRowCount);
    assert(
        Widget.mResidentialOverviewMetricValues.size() ==
        CCitizenInfoWidget::GOverviewMetricRowCount);
    assert(
        Widget.mCitizenPoliticsSectionBackgrounds.size() ==
        CCitizenInfoWidget::GCitizenPoliticsSectionCount);
    assert(
        Widget.mCitizenPoliticsSectionTitles.size() ==
        CCitizenInfoWidget::GCitizenPoliticsSectionCount);
    assert(
        Widget.mCitizenPoliticsSatisfactionLabels.size() ==
        CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount);
    assert(
        Widget.mCitizenPoliticsSatisfactionRails.size() ==
        CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount);
    assert(
        Widget.mCitizenPoliticsSatisfactionFills.size() ==
        CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount);
    assert(
        Widget.mCitizenPoliticsOpinionTexts.size() ==
        CCitizenInfoWidget::GCitizenPoliticsOpinionCount);
    assert(
        Widget.mCitizenPoliticsSupportIcons.size() ==
        CCitizenInfoWidget::GCitizenPoliticsSupportIconCount);
    assert(
        Widget.mCitizenThoughtTexts.size() ==
        CCitizenInfoWidget::GCitizenThoughtCount);
    assert(
        Widget.mCitizenThoughtDividers.size() ==
        CCitizenInfoWidget::GCitizenThoughtDividerCount);
    assert(
        Widget.mCitizenActionButtons.size() ==
        CCitizenInfoWidget::GCitizenActionButtonCount);
    assert(
        Widget.mCitizenActionButtonTexts.size() ==
        CCitizenInfoWidget::GCitizenActionButtonCount);
    assert(
        Widget.mCitizenActionButtonIcons.size() ==
        CCitizenInfoWidget::GCitizenActionButtonCount);

    for (float Ratio : Widget.mCitizenPoliticsSatisfactionFillRatios)
    {
        assert(IsRatio(Ratio));
    }

    assert(IsRatio(Widget.mCitizenPoliticsSupportRatio));
#else
    (void)Widget;
    (void)Layout;
#endif
}
