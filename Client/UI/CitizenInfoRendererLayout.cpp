#include "CitizenInfoRenderer.h"
#include "CitizenInfoRendererInternal.h"
#include "CitizenInfoWidget.h"
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

struct FCitizenLayoutMetrics
{
    float PanelInnerTopOffset;
    float PanelInnerBottomInset;
    float ScrollBottomInset;
    float ScrollThumbTopOffset;
    float CloseButtonOffsetY;
    float TitleIconInsetX;
    float TitleTextInsetX;
    float TitleIconGap;
    float SubtitleOffsetY;
    float SectionRibbonOffsetY;
    float CollapsedSectionGap;
    float BudgetBaseOffsetY;
    float BudgetLabelOffsetY;
    float BudgetCustomButtonsOffsetY;
    float BudgetWorkButtonsOffsetY;
    float BudgetDefaultButtonsOffsetY;
    float BudgetCompactGap;
    float BudgetDefaultGap;
    float OccupancyGapY;
    float CompactControlHeight;
    float CompactBudgetButtonWidth;
    float SectionDividerWidth;
    float SectionDividerHeight;
    float ActionCompactIconSize;
    float ActionCompactIconOffsetY;
    float MoveCompactRightOffset;
    float FocusCompactRightOffset;
    float OverviewCommandGap;
    float CitizenActionButtonHeight;
    float CitizenActionGap;
    float ActionStackTopOffset;
    float ActionIconInset;
    float ActionIconSize;
    float FooterBottomInset;
    float BodyGapAfterSection;
    float BodyGapAfterActions;
    float BodyGapBeforeActions;
    float BodyFallbackOffset;
    float BodyBottomInset;
};

struct FCitizenInfoLayoutContext
{
    FCitizenLayoutMetrics Metrics{};
    float OuterTop = 0.f;
    float PanelWidth = 0.f;
    float PanelHeight = 0.f;
    float InnerMarginX = 0.f;
    float InnerLeft = 0.f;
    float InnerTop = 0.f;
    float InnerWidth = 0.f;
    float InnerHeight = 0.f;
    float ScrollTrackWidth = 0.f;
    float ScrollThumbHeight = 0.f;
    float TitleRibbonHeight = 0.f;
    float SectionRibbonHeight = 0.f;
    float CloseButtonSize = 0.f;
    float IconSize = 0.f;
    bool IsCitizenMode = false;
    int VisibleTabCount = 0;
    float TabWidth = 0.f;
    float TabHeight = 0.f;
    float TabGap = 0.f;
    float TotalTabsWidth = 0.f;
    float TabStartX = 0.f;
    float RibbonOffsetX = 0.f;
    float RibbonOffsetY = 0.f;
    float CloseOffsetX = 0.f;
    bool TitleIconVisible = false;
    float TitleLeft = 0.f;
    bool ShowSectionRibbon = false;
    bool ShowCitizenProfile = false;
    bool ShowCitizenPolitics = false;
    bool ShowCitizenThoughts = false;
    float SectionRibbonY = 0.f;
    bool ShowWorkOverview = false;
    bool ShowCustomOverview = false;
    bool ShowResidentialWorkMode = false;
    bool ShowAnyOverview = false;
    bool ShowCompactRows = false;
    bool ShowUpgradeCard = false;
    bool ShowInformationParagraphs = false;
    bool ShowOverviewCommandButton = false;
    float BudgetBaseY = 0.f;
    float BudgetButtonHeight = 0.f;
    float BudgetButtonTop = 0.f;
    float WorkModeTop = 0.f;
    float WorkModeBoxTop = 0.f;
    float BudgetMargin = 0.f;
    float BudgetGap = 0.f;
    float BudgetButtonWidth = 0.f;
    float OccupancyTop = 0.f;
    bool ShowActions = false;
    float ActionTop = 0.f;
};

namespace
{
constexpr float GMinimumTabStartInset = 4.f;
constexpr float GCustomOverviewMetricRowHeight = 21.f;
constexpr float GWorkOverviewMetricRowHeight = 20.f;
constexpr float GCompactMetricRowHeight = 24.f;

template <typename T>
bool IsEnabled(const std::weak_ptr<T>& Widget)
{
    if (auto Pinned = Widget.lock())
    {
        return Pinned->GetEnable();
    }

    return false;
}

FCitizenLayoutMetrics MakeLayoutMetrics(bool IsCitizenMode)
{
    const float TitleIconInsetX = IsCitizenMode ?
        6.f : UIConfig::BuildingTitleIconInsetX;
    const float TitleTextInsetX = IsCitizenMode ?
        14.f : UIConfig::BuildingTitleTextInsetX;
    const float TitleIconGap = IsCitizenMode ?
        6.f : UIConfig::BuildingTitleIconGap;
    const float SubtitleOffsetY = IsCitizenMode ?
        4.f : UIConfig::BuildingSubtitleOffsetY;
    const float CollapsedSectionGap = IsCitizenMode ?
        6.f : UIConfig::BuildingCollapsedSectionGap;
    const float BudgetLabelOffsetY = IsCitizenMode ?
        2.f : UIConfig::BuildingBudgetLabelOffsetY;
    const float BudgetCustomButtonsOffsetY = IsCitizenMode ?
        20.f : UIConfig::BuildingBudgetCustomButtonsOffsetY;
    const float BudgetWorkButtonsOffsetY = IsCitizenMode ?
        78.f : UIConfig::BuildingBudgetWorkButtonsOffsetY;
    const float BudgetDefaultButtonsOffsetY = IsCitizenMode ?
        26.f : UIConfig::BuildingBudgetDefaultButtonsOffsetY;
    const float BudgetCompactGap = IsCitizenMode ?
        6.f : UIConfig::BuildingBudgetCompactGap;
    const float BudgetDefaultGap = IsCitizenMode ?
        8.f : UIConfig::BuildingBudgetDefaultGap;
    const float OccupancyGapY = IsCitizenMode ?
        18.f : UIConfig::BuildingOccupancyGapY;
    const float CompactControlHeight = IsCitizenMode ?
        22.f : UIConfig::BuildingCompactControlHeight;
    const float CompactBudgetButtonWidth = IsCitizenMode ?
        36.f : UIConfig::BuildingCompactBudgetButtonWidth;
    const float SectionDividerWidth = IsCitizenMode ?
        172.f : UIConfig::BuildingSectionDividerWidth;
    const float SectionDividerHeight = IsCitizenMode ?
        14.f : UIConfig::BuildingSectionDividerHeight;
    const float ActionCompactIconSize = IsCitizenMode ?
        34.f : UIConfig::BuildingActionCompactIconSize;
    const float ActionCompactIconOffsetY = IsCitizenMode ?
        2.f : UIConfig::BuildingActionCompactIconOffsetY;
    const float MoveCompactRightOffset = IsCitizenMode ?
        82.f : UIConfig::BuildingMoveCompactRightOffset;
    const float FocusCompactRightOffset = IsCitizenMode ?
        40.f : UIConfig::BuildingFocusCompactRightOffset;
    const float OverviewCommandGap = IsCitizenMode ?
        10.f : UIConfig::BuildingOverviewCommandGap;
    const float CitizenActionButtonHeight = 38.f;
    const float CitizenActionGap = 4.f;
    const float ActionIconInset = 8.f;
    const float ActionIconSize = CompactControlHeight;
    const float BodyGapAfterSection = IsCitizenMode ?
        8.f : UIConfig::BuildingBodyGapAfterSection;
    const float BodyGapAfterActions = IsCitizenMode ?
        14.f : UIConfig::BuildingBodyGapAfterActions;
    const float BodyGapBeforeActions = IsCitizenMode ?
        12.f : UIConfig::BuildingBodyGapBeforeActions;
    const float BodyFallbackOffset = IsCitizenMode ?
        4.f : UIConfig::BuildingBodyFallbackOffset;

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

    const int FirstRowIndex = (std::max)(0, Owner.GetOverviewMetricScrollFirstRowIndex());
    const int VisibleLineCount =
        Owner.GetOverviewMetricScrollVisibleLineCount();
    const int TotalLineCount =
        (std::max)(
            VisibleLineCount,
            Owner.GetOverviewMetricScrollTotalLineCount());
    const int MaxOffset = (std::max)(0, TotalLineCount - VisibleLineCount);
    const float TrackLeft =
        Context.InnerMarginX * 0.5f - Context.ScrollTrackWidth * 0.5f;
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
    ScrollTrack->SetSize(Context.ScrollTrackWidth, TrackHeight);
    ScrollTrack->SetEnable(true);

    ScrollThumb->SetPos(
        TrackLeft + 0.5f,
        TrackTop + ThumbTravel * ThumbRatio);
    ScrollThumb->SetSize(Context.ScrollTrackWidth - 1.f, ThumbHeight);
    ScrollThumb->SetEnable(true);
}
} // namespace

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
    Layout.OuterTop = Widget.mPanelTop;
    Layout.PanelWidth = Widget.mPanelWidth;
    Layout.PanelHeight = Widget.mPanelHeight;
    Layout.InnerMarginX = IsCitizenMode ?
        UIConfig::CitizenPanelInnerMarginX :
        UIConfig::BuildingPanelInnerMarginX;
    Layout.InnerLeft = Layout.InnerMarginX;
    Layout.InnerTop = Layout.OuterTop + Layout.Metrics.PanelInnerTopOffset;
    Layout.InnerWidth = Layout.PanelWidth - Layout.InnerMarginX * 2.f;
    Layout.InnerHeight = Layout.PanelHeight - Layout.Metrics.PanelInnerBottomInset;
    Layout.ScrollTrackWidth = IsCitizenMode ?
        UIConfig::CitizenScrollTrackWidth :
        UIConfig::BuildingScrollTrackWidth;
    Layout.ScrollThumbHeight = IsCitizenMode ?
        UIConfig::CitizenScrollThumbHeight :
        UIConfig::BuildingScrollThumbHeight;
    Layout.TitleRibbonHeight = IsCitizenMode ?
        UIConfig::CitizenTitleRibbonHeight :
        UIConfig::BuildingTitleRibbonHeight;
    Layout.SectionRibbonHeight = IsCitizenMode ?
        UIConfig::CitizenSectionRibbonHeight :
        UIConfig::BuildingSectionRibbonHeight;
    Layout.CloseButtonSize = IsCitizenMode ?
        UIConfig::CitizenCloseButtonSize :
        UIConfig::BuildingCloseButtonSize;
    Layout.IconSize = UIConfig::BuildingIconSize;
    Layout.IsCitizenMode = IsCitizenMode;
    Layout.VisibleTabCount = Layout.IsCitizenMode ? GCitizenTabCount : GBuildingTabCount;
    Layout.TabWidth = UIConfig::BuildingTabWidth;
    Layout.TabHeight = UIConfig::BuildingTabHeight;
    Layout.TabGap = UIConfig::BuildingTabGap;
    Layout.TotalTabsWidth =
        static_cast<float>(Layout.VisibleTabCount) * Layout.TabWidth +
        static_cast<float>((std::max)(0, Layout.VisibleTabCount - 1)) *
            Layout.TabGap;
    Layout.TabStartX =
        (std::max)(
            GMinimumTabStartInset,
            (Layout.PanelWidth - Layout.TotalTabsWidth) * 0.5f);
    Layout.RibbonOffsetX = UIConfig::BuildingTitleRibbonOffsetX;
    Layout.RibbonOffsetY = UIConfig::BuildingTitleRibbonOffsetY;
    Layout.CloseOffsetX = UIConfig::BuildingCloseButtonOffsetX;
    Layout.TitleIconVisible =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        IsEnabled(Widget.mTitleIcon);
    Layout.TitleLeft = Layout.TitleIconVisible ?
        (Layout.RibbonOffsetX + Layout.IconSize + Layout.Metrics.TitleIconGap) :
        (Layout.RibbonOffsetX + Layout.Metrics.TitleTextInsetX);
    Layout.ShowSectionRibbon = IsEnabled(Widget.mSectionRibbon);
    Layout.ShowCitizenProfile =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedCitizenTab ==
            CCitizenInfoWidget::ECitizenInfoTab::Overview &&
        IsEnabled(Widget.mCitizenFooterText);
    Layout.ShowCitizenPolitics =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedCitizenTab ==
            CCitizenInfoWidget::ECitizenInfoTab::Politics &&
        IsEnabled(Widget.mCitizenPoliticsSectionTitles[0]);
    Layout.ShowCitizenThoughts =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedCitizenTab ==
            CCitizenInfoWidget::ECitizenInfoTab::Thoughts &&
        IsEnabled(Widget.mCitizenThoughtTitleText);
    Layout.SectionRibbonY =
        Layout.OuterTop +
        Layout.RibbonOffsetY +
        Layout.TitleRibbonHeight +
        Layout.Metrics.SectionRibbonOffsetY;
    Layout.ShowWorkOverview =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview &&
        IsEnabled(Widget.mOverviewWorkModeLabel);
    Layout.ShowCustomOverview =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview &&
        !Layout.ShowWorkOverview &&
        IsEnabled(Widget.mResidentialOverviewBudgetLabel);
    Layout.ShowResidentialWorkMode =
        Layout.ShowCustomOverview &&
        IsEnabled(Widget.mResidentialOverviewWorkModeLabel);
    Layout.ShowAnyOverview = Layout.ShowCustomOverview || Layout.ShowWorkOverview;
    Layout.ShowCompactRows =
        !Layout.ShowCitizenProfile &&
        !Layout.ShowCitizenPolitics &&
        !Layout.ShowCitizenThoughts &&
        !Layout.ShowAnyOverview &&
        IsEnabled(Widget.mOverviewMetricLabels[0]);
    Layout.ShowUpgradeCard = IsEnabled(Widget.mUpgradeCardBackground);
    Layout.ShowInformationParagraphs = IsEnabled(Widget.mInformationTopText);
    Layout.ShowOverviewCommandButton = IsEnabled(Widget.mOverviewCommandButton);
    Layout.BudgetBaseY =
        Layout.OuterTop +
        Layout.RibbonOffsetY +
        Layout.TitleRibbonHeight +
        Layout.Metrics.BudgetBaseOffsetY;
    Layout.BudgetButtonHeight = Layout.ShowAnyOverview ?
        Layout.Metrics.CompactControlHeight :
        UIConfig::BuildingBudgetButtonHeight;
    Layout.BudgetButtonTop = Layout.ShowCustomOverview ?
        (Layout.BudgetBaseY +
            (Layout.ShowResidentialWorkMode ?
                Layout.Metrics.BudgetWorkButtonsOffsetY :
                Layout.Metrics.BudgetCustomButtonsOffsetY)) :
        (Layout.ShowWorkOverview ?
            (Layout.BudgetBaseY + Layout.Metrics.BudgetWorkButtonsOffsetY) :
            (Layout.BudgetBaseY + Layout.Metrics.BudgetDefaultButtonsOffsetY));
    Layout.WorkModeTop = Layout.BudgetBaseY - Layout.Metrics.BudgetLabelOffsetY;
    Layout.WorkModeBoxTop =
        Layout.WorkModeTop + Layout.Metrics.CompactControlHeight;
    Layout.BudgetMargin = Layout.RibbonOffsetX;
    Layout.BudgetGap = Layout.ShowAnyOverview ?
        Layout.Metrics.BudgetCompactGap :
        Layout.Metrics.BudgetDefaultGap;
    Layout.BudgetButtonWidth = Layout.ShowAnyOverview ?
        Layout.Metrics.CompactBudgetButtonWidth :
        (Layout.PanelWidth - Layout.BudgetMargin * 2.f - Layout.BudgetGap * 4.f) /
            5.f;
    Layout.OccupancyTop =
        Layout.BudgetButtonTop +
        Layout.BudgetButtonHeight +
        Layout.Metrics.OccupancyGapY;
    Layout.ShowActions =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview;
    Layout.ActionTop =
        Layout.OuterTop +
        Layout.PanelHeight -
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
        Layout.IsCitizenMode ?
            CCitizenInfoWidget::GCitizenTabCount :
            CCitizenInfoWidget::GBuildingTabCount;
    const int SelectedTabIndex = Widget.GetSelectedTabIndexForCurrentMode();

    assert(IsPositiveFinite(Layout.PanelWidth));
    assert(IsPositiveFinite(Layout.PanelHeight));
    assert(IsPositiveFinite(Layout.InnerWidth));
    assert(IsPositiveFinite(Layout.InnerHeight));
    assert(IsPositiveFinite(Layout.ScrollTrackWidth));
    assert(IsPositiveFinite(Layout.ScrollThumbHeight));
    assert(IsPositiveFinite(Layout.TitleRibbonHeight));
    assert(IsPositiveFinite(Layout.SectionRibbonHeight));
    assert(IsPositiveFinite(Layout.CloseButtonSize));
    assert(Layout.VisibleTabCount == ExpectedVisibleTabCount);
    assert(SelectedTabIndex >= 0);
    assert(SelectedTabIndex < ExpectedVisibleTabCount);
    assert(UIConfig::CitizenPanelMinHeight > 0.f);
    assert(UIConfig::CitizenPanelRightInset >= 0.f);
    assert(Widget.mTabButtons.size() == CCitizenInfoWidget::GTabButtonCount);
    assert(Widget.mTabButtonTexts.size() == CCitizenInfoWidget::GTabButtonCount);
    assert(Widget.mTabButtonIcons.size() == CCitizenInfoWidget::GTabButtonCount);
    assert(Widget.mBudgetButtons.size() == CCitizenInfoWidget::GBudgetLevelCount);
    assert(Widget.mBudgetButtonTexts.size() == CCitizenInfoWidget::GBudgetLevelCount);
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

void FCitizenInfoRenderer::RefreshCommonLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const auto& Layout = Context.Metrics;
    const float OuterTop = Context.OuterTop;
    const float PanelWidth = Context.PanelWidth;
    const float PanelHeight = Context.PanelHeight;
    const float InnerMarginX = Context.InnerMarginX;
    const float InnerLeft = Context.InnerLeft;
    const float InnerTop = Context.InnerTop;
    const float InnerWidth = Context.InnerWidth;
    const float InnerHeight = Context.InnerHeight;
    const float ScrollTrackW = Context.ScrollTrackWidth;
    const float ScrollThumbH = Context.ScrollThumbHeight;
    const float TitleRibbonH = Context.TitleRibbonHeight;
    const float SectionRibbonH = Context.SectionRibbonHeight;
    const float CloseButtonSz = Context.CloseButtonSize;
    const float IconSz = Context.IconSize;
    const bool ShowSectionRibbon = Context.ShowSectionRibbon;
    const float SectionRibbonY = Context.SectionRibbonY;
    const bool ShowWorkOverview = Context.ShowWorkOverview;
    const bool ShowCustomOverview = Context.ShowCustomOverview;
    const bool ShowResidentialWorkMode = Context.ShowResidentialWorkMode;
    const bool ShowAnyOverview = Context.ShowAnyOverview;
    const float TitleIconOffsetY = Context.IsCitizenMode ?
        0.f : UIConfig::BuildingTitleIconOffsetY;
    const float TitleTextOffsetX = Context.IsCitizenMode ?
        0.f : UIConfig::BuildingTitleTextOffsetX;
    const float TitleTextOffsetY = Context.IsCitizenMode ?
        0.f : UIConfig::BuildingTitleTextOffsetY;
    const float TitleTextWidthAdjust = Context.IsCitizenMode ?
        0.f : UIConfig::BuildingTitleTextWidthAdjust;
    const float TitleTextHeightAdjust = Context.IsCitizenMode ?
        0.f : UIConfig::BuildingTitleTextHeightAdjust;
    // 폰트 크기
    if (auto TitleTxt = Widget.mTitleText.lock())
        TitleTxt->SetFontSize(
            Context.IsCitizenMode ?
                UIConfig::CitizenTitleFontSize :
                UIConfig::BuildingTitleFontSize);
    if (auto SubTxt = Widget.mSubtitleText.lock())
        SubTxt->SetFontSize(
            Context.IsCitizenMode ?
                UIConfig::CitizenSubtitleFontSize :
                UIConfig::BuildingSubtitleFontSize);
    if (auto BodyTxt = Widget.mBodyText.lock())
        BodyTxt->SetFontSize(
            Context.IsCitizenMode ?
                UIConfig::CitizenBodyFontSize :
                UIConfig::BuildingBodyFontSize);
    if (auto PageTitleTxt = Widget.mPageTitleText.lock())
        PageTitleTxt->SetFontSize(
            Context.IsCitizenMode ?
                UIConfig::CitizenPageTitleFontSize :
                UIConfig::BuildingPageTitleFontSize);
    if (auto BudgetTxt = Widget.mBudgetText.lock())
        BudgetTxt->SetFontSize(
            Context.IsCitizenMode ?
                UIConfig::CitizenBodyFontSize :
                UIConfig::BuildingBudgetTextFontSize);
    if (auto Text = Widget.mOverviewWorkModeLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewWorkModeLabelFontSize);
    if (auto Text = Widget.mOverviewWorkModeText.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewWorkModeValueFontSize);
    if (auto Text = Widget.mResidentialOverviewWorkModeLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewWorkModeLabelFontSize);
    if (auto Text = Widget.mResidentialOverviewWorkModeText.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewWorkModeValueFontSize);
    if (auto Text = Widget.mOverviewBudgetLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewBudgetLabelFontSize);
    if (auto Text = Widget.mOverviewBudgetValue.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewBudgetValueFontSize);
    if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewOccupancyLabelFontSize);
    if (auto Text = Widget.mOverviewOccupancyValue.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewOccupancyValueFontSize);
    if (auto Text = Widget.mResidentialOverviewBudgetLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewBudgetLabelFontSize);
    if (auto Text = Widget.mResidentialOverviewBudgetValue.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewBudgetValueFontSize);
    if (auto Text = Widget.mResidentialOverviewOccupancyLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewOccupancyLabelFontSize);
    if (auto Text = Widget.mResidentialOverviewOccupancyValue.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewOccupancyValueFontSize);
    if (auto Text = Widget.mUpgradeCardTitle.lock())
        Text->SetFontSize(UIConfig::BuildingUpgradeTitleFontSize);
    if (auto Text = Widget.mUpgradeDescriptionText.lock())
        Text->SetFontSize(UIConfig::BuildingUpgradeDescriptionFontSize);
    if (auto Text = Widget.mInformationAccentText.lock())
        Text->SetFontSize(UIConfig::BuildingInformationAccentFontSize);
    if (auto Text = Widget.mInformationTopText.lock())
        Text->SetFontSize(UIConfig::BuildingInformationBodyFontSize);
    if (auto Text = Widget.mInformationBottomText.lock())
        Text->SetFontSize(UIConfig::BuildingInformationBodyFontSize);
    if (auto Text = Widget.mOverviewCommandButtonText.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewCommandButtonFontSize);

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        if (auto Text =
            Widget.mBudgetButtonTexts[static_cast<size_t>(Index)].lock())
        {
            Text->SetFontSize(UIConfig::BuildingBudgetButtonFontSize);
        }
    }

    // 패널 배경
    if (auto PanelImage = Widget.mPanelImage.lock())
    {
        PanelImage->SetPos(0.f, OuterTop);
        PanelImage->SetSize(PanelWidth, PanelHeight);
    }

    // 내부 프레임
    if (auto InnerFrame = Widget.mInnerFrame.lock())
    {
        InnerFrame->SetPos(InnerLeft, InnerTop);
        InnerFrame->SetSize(InnerWidth, InnerHeight);
    }

    // 스크롤바
    if (auto ScrollTrack = Widget.mScrollTrack.lock())
    {
        ScrollTrack->SetPos(InnerMarginX * 0.5f - ScrollTrackW * 0.5f,
            OuterTop + TitleRibbonH + SectionRibbonH);
        ScrollTrack->SetSize(
            ScrollTrackW,
            PanelHeight - TitleRibbonH - SectionRibbonH - Layout.ScrollBottomInset);
    }
    if (auto ScrollThumb = Widget.mScrollThumb.lock())
    {
        ScrollThumb->SetPos(InnerMarginX * 0.5f - ScrollTrackW * 0.5f + 0.5f,
            OuterTop + TitleRibbonH + SectionRibbonH + Layout.ScrollThumbTopOffset);
        ScrollThumb->SetSize(ScrollTrackW - 1.f, ScrollThumbH);
    }

    // 상단 탭 (건물/NPC 공용)
    const bool IsCitizenMode =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen;
    const int VisibleTabCount = IsCitizenMode ? GCitizenTabCount : GBuildingTabCount;
    const float TabWidth = UIConfig::BuildingTabWidth;
    const float TabHeight = UIConfig::BuildingTabHeight;
    const float TabGap   = UIConfig::BuildingTabGap;
    const float TotalTabsWidth =
        static_cast<float>(VisibleTabCount) * TabWidth +
        static_cast<float>((std::max)(0, VisibleTabCount - 1)) * TabGap;
    const float TabStartX = (std::max)(
        GMinimumTabStartInset,
        (PanelWidth - TotalTabsWidth) * 0.5f);

    for (int Index = 0; Index < GBuildingTabCount; ++Index)
    {
        auto Button = Widget.mTabButtons[static_cast<size_t>(Index)].lock();
        if (!Button)
            continue;

        Button->SetPos(
            TabStartX + static_cast<float>(Index) * (TabWidth + TabGap), 0.f);
        Button->SetSize(TabWidth, TabHeight);

        if (auto Label = Widget.mTabButtonTexts[static_cast<size_t>(Index)].lock())
        {
            Label->SetFontSize(UIConfig::BuildingTabLabelFontSize);
            Label->SetPos(
                UIConfig::BuildingTabLabelOffsetX,
                UIConfig::BuildingTabLabelOffsetY);
            Label->SetSize(TabWidth, TabHeight);
        }
    }

    // 제목 리본
    const float RibbonOffsetX = UIConfig::BuildingTitleRibbonOffsetX;
    const float RibbonOffsetY = UIConfig::BuildingTitleRibbonOffsetY;

    if (auto TitleRibbon = Widget.mTitleRibbon.lock())
    {
        TitleRibbon->SetPos(RibbonOffsetX, OuterTop + RibbonOffsetY);
        TitleRibbon->SetSize(PanelWidth - RibbonOffsetX * 2.f - CloseButtonSz, TitleRibbonH);
    }

    // 닫기 버튼
    const float CloseOffsetX = UIConfig::BuildingCloseButtonOffsetX;
    if (auto CloseButton = Widget.mCloseButton.lock())
    {
        CloseButton->SetPos(
            PanelWidth - CloseOffsetX,
            OuterTop + Layout.CloseButtonOffsetY);
        CloseButton->SetSize(CloseButtonSz, CloseButtonSz);
    }

    // 아이콘 & 제목 텍스트
    const bool TitleIconVisible =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        !Widget.mTitleIcon.expired() &&
        Widget.mTitleIcon.lock()->GetEnable();
    const float TitleLeft = TitleIconVisible ?
        (RibbonOffsetX + IconSz + Layout.TitleIconGap) :
        (RibbonOffsetX + Layout.TitleTextInsetX);

    if (auto TitleIcon = Widget.mTitleIcon.lock())
    {
        TitleIcon->SetPos(
            RibbonOffsetX + Layout.TitleIconInsetX,
            OuterTop + RibbonOffsetY + (TitleRibbonH - IconSz) * 0.5f +
                TitleIconOffsetY);
        TitleIcon->SetSize(IconSz, IconSz);
    }
    if (auto TitleText = Widget.mTitleText.lock())
    {
        TitleText->SetPos(
            TitleLeft + TitleTextOffsetX,
            OuterTop + RibbonOffsetY + TitleTextOffsetY);
        TitleText->SetSize(
            PanelWidth - TitleLeft - CloseButtonSz - CloseOffsetX +
                TitleTextWidthAdjust,
            TitleRibbonH + TitleTextHeightAdjust);
    }
    if (auto SubtitleText = Widget.mSubtitleText.lock())
    {
        SubtitleText->SetPos(
            RibbonOffsetX,
            OuterTop + RibbonOffsetY + TitleRibbonH + Layout.SubtitleOffsetY);
        SubtitleText->SetSize(
            PanelWidth - RibbonOffsetX * 2.f,
            Layout.CompactControlHeight);
    }


    if (auto SectionRibbon = Widget.mSectionRibbon.lock())
    {
        SectionRibbon->SetPos(RibbonOffsetX, SectionRibbonY);
        SectionRibbon->SetSize(PanelWidth - RibbonOffsetX * 2.f, SectionRibbonH);
        SectionRibbon->SetEnable(ShowSectionRibbon);
    }
    if (auto PageTitleText = Widget.mPageTitleText.lock())
    {
        PageTitleText->SetPos(RibbonOffsetX, SectionRibbonY);
        PageTitleText->SetSize(PanelWidth - RibbonOffsetX * 2.f, SectionRibbonH);
        PageTitleText->SetEnable(ShowSectionRibbon);
    }

    if (auto ScrollTrack = Widget.mScrollTrack.lock())
    {
        const float ScrollTop =
            OuterTop + TitleRibbonH +
            (ShowSectionRibbon ? SectionRibbonH : Layout.CollapsedSectionGap);
        ScrollTrack->SetPos(
            InnerMarginX * 0.5f - ScrollTrackW * 0.5f,
            ScrollTop);
        ScrollTrack->SetSize(
            ScrollTrackW,
            PanelHeight - (ScrollTop - OuterTop) - Layout.ScrollBottomInset);
    }
    if (auto ScrollThumb = Widget.mScrollThumb.lock())
    {
        const float ScrollTop =
            OuterTop + TitleRibbonH +
            (ShowSectionRibbon ? SectionRibbonH : Layout.CollapsedSectionGap);
        ScrollThumb->SetPos(
            InnerMarginX * 0.5f - ScrollTrackW * 0.5f + 0.5f,
            ScrollTop + Layout.ScrollThumbTopOffset);
        ScrollThumb->SetSize(ScrollTrackW - 1.f, ScrollThumbH);
    }

    // 예산 컨트롤
    const float BudgetBaseY =
        OuterTop + RibbonOffsetY + TitleRibbonH + Layout.BudgetBaseOffsetY;
    if (auto BudgetText = Widget.mBudgetText.lock())
    {
        if (ShowAnyOverview)
        {
            BudgetText->SetPos(0.f, 0.f);
            BudgetText->SetSize(0.f, 0.f);
        }
        else
        {
            BudgetText->SetPos(RibbonOffsetX, BudgetBaseY);
            BudgetText->SetSize(
                PanelWidth - RibbonOffsetX * 2.f,
                Layout.CompactControlHeight);
        }
    }

    const float BudgetButtonH   = ShowAnyOverview ? Layout.CompactControlHeight :
        UIConfig::BuildingBudgetButtonHeight;
    const float BudgetButtonTop = ShowCustomOverview ?
        (BudgetBaseY +
            (ShowResidentialWorkMode ?
                Layout.BudgetWorkButtonsOffsetY :
                Layout.BudgetCustomButtonsOffsetY)) :
        (ShowWorkOverview ? (BudgetBaseY + Layout.BudgetWorkButtonsOffsetY) :
        (BudgetBaseY + Layout.BudgetDefaultButtonsOffsetY));
    const float WorkModeTop = BudgetBaseY - Layout.BudgetLabelOffsetY;
    const float WorkModeBoxTop = WorkModeTop + Layout.CompactControlHeight;
    const float BudgetMargin    = RibbonOffsetX;
    const float BudgetGap       = ShowAnyOverview ?
        Layout.BudgetCompactGap :
        Layout.BudgetDefaultGap;
    const float BudgetButtonW   = ShowAnyOverview ?
        Layout.CompactBudgetButtonWidth :
        (PanelWidth - BudgetMargin * 2.f - BudgetGap * 4.f) / 5.f;

    if (auto Text = Widget.mOverviewWorkModeLabel.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(
                BudgetMargin,
                WorkModeTop);
            Text->SetSize(
                PanelWidth * 0.5f,
                Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Background = Widget.mOverviewWorkModeBackground.lock())
    {
        if (ShowWorkOverview)
        {
            Background->SetPos(
                BudgetMargin,
                WorkModeBoxTop);
            Background->SetSize(
                PanelWidth - BudgetMargin * 2.f,
                34.f);
        }
        else
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }
    }

    if (auto Button = Widget.mOverviewWorkModeButton.lock())
    {
        if (ShowWorkOverview)
        {
            Button->SetPos(
                BudgetMargin,
                WorkModeBoxTop);
            Button->SetSize(
                PanelWidth - BudgetMargin * 2.f,
                34.f);
        }
        else
        {
            Button->SetPos(0.f, 0.f);
            Button->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewWorkModeText.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(
                BudgetMargin + 12.f,
                WorkModeBoxTop + 2.f);
            Text->SetSize(
                PanelWidth - BudgetMargin * 2.f - 24.f,
                30.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewWorkModeLabel.lock())
    {
        if (ShowResidentialWorkMode)
        {
            Text->SetPos(
                BudgetMargin,
                WorkModeTop);
            Text->SetSize(
                PanelWidth * 0.5f,
                Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Background = Widget.mResidentialOverviewWorkModeBackground.lock())
    {
        if (ShowResidentialWorkMode)
        {
            Background->SetPos(
                BudgetMargin,
                WorkModeBoxTop);
            Background->SetSize(
                PanelWidth - BudgetMargin * 2.f,
                34.f);
        }
        else
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }
    }

    if (auto Button = Widget.mResidentialOverviewWorkModeButton.lock())
    {
        if (ShowResidentialWorkMode)
        {
            Button->SetPos(
                BudgetMargin,
                WorkModeBoxTop);
            Button->SetSize(
                PanelWidth - BudgetMargin * 2.f,
                34.f);
        }
        else
        {
            Button->SetPos(0.f, 0.f);
            Button->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewWorkModeText.lock())
    {
        if (ShowResidentialWorkMode)
        {
            Text->SetPos(
                BudgetMargin + 12.f,
                WorkModeBoxTop + 2.f);
            Text->SetSize(
                PanelWidth - BudgetMargin * 2.f - 24.f,
                30.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewBudgetLabel.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(
                BudgetMargin,
                BudgetBaseY - Layout.BudgetLabelOffsetY);
            Text->SetSize(
                PanelWidth * 0.5f,
                Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewBudgetValue.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(
                PanelWidth - BudgetMargin - 120.f,
                BudgetBaseY - Layout.BudgetLabelOffsetY);
            Text->SetSize(
                120.f,
                Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        auto Button = Widget.mBudgetButtons[static_cast<size_t>(Index)].lock();
        if (!Button)
            continue;

        Button->SetPos(
            BudgetMargin + static_cast<float>(Index) * (BudgetButtonW + BudgetGap),
            BudgetButtonTop);
        Button->SetSize(BudgetButtonW, BudgetButtonH);
    }

    const float OccupancyTop =
        BudgetButtonTop + BudgetButtonH + Layout.OccupancyGapY;
    if (auto Text = Widget.mOverviewOccupancyLabel.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(BudgetMargin, OccupancyTop);
            Text->SetSize(PanelWidth * 0.5f, Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewOccupancyValue.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(PanelWidth - BudgetMargin - 120.f, OccupancyTop);
            Text->SetSize(120.f, Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewOccupancyLabel.lock())
    {
        if (ShowCustomOverview)
        {
            Text->SetPos(BudgetMargin, OccupancyTop);
            Text->SetSize(PanelWidth * 0.5f, Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewOccupancyValue.lock())
    {
        if (ShowCustomOverview)
        {
            Text->SetPos(PanelWidth - BudgetMargin - 120.f, OccupancyTop);
            Text->SetSize(120.f, Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewBudgetLabel.lock())
    {
        if (ShowCustomOverview)
        {
            Text->SetPos(
                BudgetMargin,
                BudgetBaseY - Layout.BudgetLabelOffsetY);
            Text->SetSize(
                PanelWidth * 0.5f,
                Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewBudgetValue.lock())
    {
        if (ShowCustomOverview)
        {
            Text->SetPos(
                PanelWidth - BudgetMargin - 120.f,
                BudgetBaseY - Layout.BudgetLabelOffsetY);
            Text->SetSize(
                120.f,
                Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

}

void FCitizenInfoRenderer::RefreshCitizenModeLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    if (!Context.IsCitizenMode)
    {
        return;
    }

    if (Context.ShowCitizenThoughts)
    {
        RefreshCitizenThoughtsLayout(Owner, Context);
        return;
    }

    if (Context.ShowCitizenPolitics)
    {
        RefreshCitizenPoliticsLayout(Owner, Context);
        return;
    }

    if (Context.ShowCitizenProfile)
    {
        RefreshCitizenProfileLayout(Owner, Context);
    }
}

void FCitizenInfoRenderer::RefreshCitizenThoughtsLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float OuterTop = Context.OuterTop;
    const float PanelWidth = Context.PanelWidth;
    const float RibbonOffsetY = Context.RibbonOffsetY;
    const float TitleRibbonH = Context.TitleRibbonHeight;
    const float BudgetMargin = Context.BudgetMargin;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mOverviewBudgetLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewBudgetValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Background = Widget.mOverviewWorkModeBackground.lock())
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock())
            {
                Value->SetPos(0.f, 0.f);
                Value->SetSize(0.f, 0.f);
            }
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(0.f, 0.f);
            Divider->SetSize(0.f, 0.f);
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSectionCount;
            ++Index)
        {
            if (auto Background =
                Widget.mCitizenPoliticsSectionBackgrounds[
                    static_cast<size_t>(Index)].lock())
            {
                Background->SetPos(0.f, 0.f);
                Background->SetSize(0.f, 0.f);
            }

            if (auto Title =
                Widget.mCitizenPoliticsSectionTitles[
                    static_cast<size_t>(Index)].lock())
            {
                Title->SetPos(0.f, 0.f);
                Title->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount;
            ++Index)
        {
            if (auto Label =
                Widget.mCitizenPoliticsSatisfactionLabels[
                    static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Rail =
                Widget.mCitizenPoliticsSatisfactionRails[
                    static_cast<size_t>(Index)].lock())
            {
                Rail->SetPos(0.f, 0.f);
                Rail->SetSize(0.f, 0.f);
            }

            if (auto Fill =
                Widget.mCitizenPoliticsSatisfactionFills[
                    static_cast<size_t>(Index)].lock())
            {
                Fill->SetPos(0.f, 0.f);
                Fill->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsOpinionCount;
            ++Index)
        {
            if (auto Text =
                Widget.mCitizenPoliticsOpinionTexts[
                    static_cast<size_t>(Index)].lock())
            {
                Text->SetPos(0.f, 0.f);
                Text->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSupportIconCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mCitizenPoliticsSupportIcons[
                    static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        if (auto Rail = Widget.mCitizenPoliticsSupportRail.lock())
        {
            Rail->SetPos(0.f, 0.f);
            Rail->SetSize(0.f, 0.f);
        }

        if (auto Thumb = Widget.mCitizenPoliticsSupportThumb.lock())
        {
            Thumb->SetPos(0.f, 0.f);
            Thumb->SetSize(0.f, 0.f);
        }

        const float SectionTop =
            OuterTop + RibbonOffsetY + TitleRibbonH + 42.f;
        const float SectionWidth = PanelWidth - BudgetMargin * 2.f;
        const float SectionHeight = 30.f;

        if (auto Background = Widget.mCitizenThoughtTitleBackground.lock())
        {
            Background->SetPos(BudgetMargin, SectionTop);
            Background->SetSize(SectionWidth, SectionHeight);
        }

        if (auto Text = Widget.mCitizenThoughtTitleText.lock())
        {
            Text->SetPos(BudgetMargin, SectionTop);
            Text->SetSize(SectionWidth, SectionHeight);
        }

        const float ThoughtTop = SectionTop + SectionHeight + 14.f;
        const float ThoughtWidth = SectionWidth - 6.f;
        const float ThoughtHeight = 50.f;
        const float DividerWidth = Context.Metrics.SectionDividerWidth;
        const float DividerHeight = Context.Metrics.SectionDividerHeight;
        const float ThoughtStep = 82.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenThoughtCount;
            ++Index)
        {
            if (auto Text =
                Widget.mCitizenThoughtTexts[static_cast<size_t>(Index)].lock())
            {
                Text->SetPos(
                    BudgetMargin + 2.f,
                    ThoughtTop + ThoughtStep * static_cast<float>(Index));
                Text->SetSize(ThoughtWidth, ThoughtHeight);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenThoughtDividerCount;
            ++Index)
        {
            if (auto Divider =
                Widget.mCitizenThoughtDividers[static_cast<size_t>(Index)].lock())
            {
            Divider->SetPos(
                PanelWidth * 0.5f - DividerWidth * 0.5f,
                ThoughtTop + 38.f + ThoughtStep * static_cast<float>(Index));
            Divider->SetSize(DividerWidth, DividerHeight);
            }
        }
}

void FCitizenInfoRenderer::RefreshCitizenPoliticsLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float OuterTop = Context.OuterTop;
    const float PanelWidth = Context.PanelWidth;
    const float RibbonOffsetY = Context.RibbonOffsetY;
    const float TitleRibbonH = Context.TitleRibbonHeight;
    const float BudgetMargin = Context.BudgetMargin;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mOverviewBudgetLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewBudgetValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Background = Widget.mOverviewWorkModeBackground.lock())
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock())
            {
                Value->SetPos(0.f, 0.f);
                Value->SetSize(0.f, 0.f);
            }
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(0.f, 0.f);
            Divider->SetSize(0.f, 0.f);
        }

        const float SectionWidth = PanelWidth - BudgetMargin * 2.f;
        const float SectionHeight = 30.f;
        const float SatisfactionTitleTop =
            OuterTop + RibbonOffsetY + TitleRibbonH + 42.f;
        const float SatisfactionRowsTop = SatisfactionTitleTop + SectionHeight + 10.f;
        const float SatisfactionRowH = 28.f;
        const float SatisfactionLabelW = 82.f;
        const float SatisfactionRailLeft = BudgetMargin + SatisfactionLabelW;
        const float SatisfactionRailWidth =
            PanelWidth - BudgetMargin - SatisfactionRailLeft - 4.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSectionCount;
            ++Index)
        {
            float SectionTop = 0.f;

            if (Index == 0)
                SectionTop = SatisfactionTitleTop;
            else if (Index == 1)
                SectionTop = SatisfactionRowsTop +
                    SatisfactionRowH *
                    static_cast<float>(
                        CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount) +
                    8.f;
            else
                SectionTop = SatisfactionRowsTop +
                    SatisfactionRowH *
                    static_cast<float>(
                        CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount) +
                    8.f +
                    SectionHeight +
                    8.f +
                    26.f *
                    static_cast<float>(
                        CCitizenInfoWidget::GCitizenPoliticsOpinionCount) +
                    14.f;

            if (auto Background =
                Widget.mCitizenPoliticsSectionBackgrounds[
                    static_cast<size_t>(Index)].lock())
            {
                Background->SetPos(BudgetMargin, SectionTop);
                Background->SetSize(SectionWidth, SectionHeight);
            }

            if (auto Title =
                Widget.mCitizenPoliticsSectionTitles[
                    static_cast<size_t>(Index)].lock())
            {
                Title->SetPos(BudgetMargin, SectionTop);
                Title->SetSize(SectionWidth, SectionHeight);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount;
            ++Index)
        {
            const float RowTop =
                SatisfactionRowsTop +
                SatisfactionRowH * static_cast<float>(Index);
            const float FillRatio =
                (std::max)(
                    0.f,
                    (std::min)(
                        1.f,
                        Widget.mCitizenPoliticsSatisfactionFillRatios[
                            static_cast<size_t>(Index)]));
            const float FillWidth =
                (std::max)(8.f, SatisfactionRailWidth * FillRatio);

            if (auto Label =
                Widget.mCitizenPoliticsSatisfactionLabels[
                    static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(BudgetMargin, RowTop);
                Label->SetSize(SatisfactionLabelW - 8.f, 22.f);
            }

            if (auto Rail =
                Widget.mCitizenPoliticsSatisfactionRails[
                    static_cast<size_t>(Index)].lock())
            {
                Rail->SetPos(SatisfactionRailLeft, RowTop + 5.f);
                Rail->SetSize(SatisfactionRailWidth, 13.f);
            }

            if (auto Fill =
                Widget.mCitizenPoliticsSatisfactionFills[
                    static_cast<size_t>(Index)].lock())
            {
                Fill->SetPos(SatisfactionRailLeft + 1.f, RowTop + 6.f);
                Fill->SetSize(
                    (std::min)(SatisfactionRailWidth - 2.f, FillWidth),
                    11.f);
            }
        }

        const float OpinionTitleTop =
            SatisfactionRowsTop +
            SatisfactionRowH *
                static_cast<float>(
                    CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount) +
            8.f;
        const float OpinionRowsTop = OpinionTitleTop + SectionHeight + 8.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsOpinionCount;
            ++Index)
        {
            if (auto Text =
                Widget.mCitizenPoliticsOpinionTexts[
                    static_cast<size_t>(Index)].lock())
            {
                Text->SetPos(
                    BudgetMargin + 4.f,
                    OpinionRowsTop + 26.f * static_cast<float>(Index));
                Text->SetSize(SectionWidth - 8.f, 22.f);
            }
        }

        const float SupportTitleTop =
            OpinionRowsTop +
            26.f *
                static_cast<float>(
                    CCitizenInfoWidget::GCitizenPoliticsOpinionCount) +
            14.f;
        const float SupportIconsTop = SupportTitleTop + SectionHeight + 8.f;
        const float SupportIconSpacing = SectionWidth / 3.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSupportIconCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mCitizenPoliticsSupportIcons[
                    static_cast<size_t>(Index)].lock())
            {
                const float IconLeft =
                    BudgetMargin +
                    SupportIconSpacing * static_cast<float>(Index) +
                    SupportIconSpacing * 0.5f -
                    10.f;
                Icon->SetPos(IconLeft, SupportIconsTop);
                Icon->SetSize(20.f, 20.f);
            }
        }

        const float SupportRailTop = SupportIconsTop + 26.f;
        const float SupportRailLeft = BudgetMargin + 10.f;
        const float SupportRailWidth = SectionWidth - 20.f;

        if (auto Rail = Widget.mCitizenPoliticsSupportRail.lock())
        {
            Rail->SetPos(SupportRailLeft, SupportRailTop);
            Rail->SetSize(SupportRailWidth, 10.f);
        }

        if (auto Thumb = Widget.mCitizenPoliticsSupportThumb.lock())
        {
            const float SupportRatio =
                (std::max)(0.f, (std::min)(1.f, Widget.mCitizenPoliticsSupportRatio));
            Thumb->SetPos(
                SupportRailLeft + SupportRatio * (SupportRailWidth - 10.f),
                SupportRailTop - 4.f);
            Thumb->SetSize(10.f, 18.f);
        }
}

void FCitizenInfoRenderer::RefreshCitizenProfileLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float PanelWidth = Context.PanelWidth;
    const float BudgetMargin = Context.BudgetMargin;
    const float OccupancyTop = Context.OccupancyTop;

        const std::array<FVector2, 11> PortraitPositions =
        {
            FVector2(BudgetMargin + 24.f, OccupancyTop + 10.f),
            FVector2(BudgetMargin + 94.f, OccupancyTop + 10.f),
            FVector2(BudgetMargin + 164.f, OccupancyTop + 10.f),
            FVector2(BudgetMargin + 234.f, OccupancyTop + 10.f),
            FVector2(BudgetMargin + 57.f, OccupancyTop + 62.f),
            FVector2(BudgetMargin + 235.f, OccupancyTop + 62.f),
            FVector2(BudgetMargin + 6.f, OccupancyTop + 112.f),
            FVector2(BudgetMargin + 76.f, OccupancyTop + 112.f),
            FVector2(BudgetMargin + 146.f, OccupancyTop + 112.f),
            FVector2(BudgetMargin + 216.f, OccupancyTop + 112.f),
            FVector2(BudgetMargin + 286.f, OccupancyTop + 112.f)
        };
        const float PortraitSize = 34.f;
        const float DetailTop = OccupancyTop + 170.f;
        const float DetailRowH = 28.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock();

            if (!Icon)
                continue;

            if (Index < static_cast<int>(PortraitPositions.size()))
            {
                Icon->SetPos(
                    PortraitPositions[static_cast<size_t>(Index)].x,
                    PortraitPositions[static_cast<size_t>(Index)].y);
                Icon->SetSize(PortraitSize, PortraitSize);
            }
            else
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock();
            auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock();
            const float RowTop =
                DetailTop + static_cast<float>(Index) * DetailRowH;

            if (Label)
            {
                Label->SetPos(BudgetMargin, RowTop);
                Label->SetSize(110.f, 24.f);
            }

            if (Value)
            {
                Value->SetPos(PanelWidth - BudgetMargin - 164.f, RowTop);
                Value->SetSize(164.f, 24.f);
            }
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(
                PanelWidth * 0.5f - Context.Metrics.SectionDividerWidth * 0.5f,
                DetailTop +
                    DetailRowH * 8.f +
                    6.f);
            Divider->SetSize(
                Context.Metrics.SectionDividerWidth,
                Context.Metrics.SectionDividerHeight);
        }
}

void FCitizenInfoRenderer::RefreshBuildingModeLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    if (Context.IsCitizenMode &&
        (Context.ShowCitizenThoughts ||
            Context.ShowCitizenPolitics ||
            Context.ShowCitizenProfile))
    {
        return;
    }

    if (Context.ShowCustomOverview)
    {
        RefreshBuildingCustomOverviewLayout(Owner, Context);
    }
    else if (Context.ShowWorkOverview)
    {
        RefreshBuildingWorkOverviewLayout(Owner, Context);
    }
    else if (Context.ShowCompactRows)
    {
        RefreshBuildingCompactLayout(Owner, Context);
    }
    else if (Context.ShowInformationParagraphs)
    {
        RefreshBuildingInformationLayout(Owner, Context);
    }
    else
    {
        RefreshBuildingDefaultLayout(Owner, Context);
    }
}

void FCitizenInfoRenderer::RefreshBuildingCustomOverviewLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float PanelWidth = Context.PanelWidth;
    const float BudgetMargin = Context.BudgetMargin;
    const float OccupancyTop = Context.OccupancyTop;

        const float ResidentStartX = BudgetMargin + 6.f;
        const float ResidentStartY = OccupancyTop + 26.f;
        const float ResidentIconSize = 22.f;
        const float ResidentGapX = 34.f;
        const float ResidentGapY = 24.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            auto Icon =
                Widget.mResidentialOverviewResidentIcons[
                    static_cast<size_t>(Index)].lock();

            if (!Icon)
                continue;

            const int Column = Index % 4;
            const int Row = Index / 4;
            Icon->SetPos(
                ResidentStartX + static_cast<float>(Column) * ResidentGapX,
                ResidentStartY + static_cast<float>(Row) * ResidentGapY);
            Icon->SetSize(ResidentIconSize, ResidentIconSize);
        }

        const float MetricsTop = ResidentStartY + ResidentGapY * 4.f + 10.f;
        const float MetricRowH = GCustomOverviewMetricRowHeight;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            auto Label =
                Widget.mResidentialOverviewMetricLabels[
                    static_cast<size_t>(Index)].lock();
            auto Value =
                Widget.mResidentialOverviewMetricValues[
                    static_cast<size_t>(Index)].lock();
            const float RowTop =
                MetricsTop + static_cast<float>(Index) * MetricRowH;

            if (Label)
            {
                Label->SetPos(BudgetMargin, RowTop);
                Label->SetSize(140.f, 20.f);
            }

            if (Value)
            {
                Value->SetPos(PanelWidth - BudgetMargin - 160.f, RowTop);
                Value->SetSize(160.f, 20.f);
            }
        }
}

void FCitizenInfoRenderer::RefreshBuildingWorkOverviewLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float PanelWidth = Context.PanelWidth;
    const float BudgetMargin = Context.BudgetMargin;
    const float OccupancyTop = Context.OccupancyTop;

    constexpr int GWorkOverviewResidentColumns = 6;
    constexpr int GWorkOverviewMaxVisibleResidents = 12;
    const float ResidentStartX = BudgetMargin + 6.f;
    const float ResidentStartY = OccupancyTop + 26.f;
    const float ResidentIconSize = 24.f;
    const float ResidentGapX = 26.f;
    const float ResidentGapY = 22.f;
    int VisibleResidentCount = 0;

    for (int Index = 0;
        Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
        ++Index)
    {
        auto Icon =
            Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock();

        if (!Icon || !Icon->GetEnable())
            continue;

        ++VisibleResidentCount;
    }

    VisibleResidentCount =
        (std::min)(VisibleResidentCount, GWorkOverviewMaxVisibleResidents);
    const int ResidentRowCount =
        VisibleResidentCount > 0 ?
            ((VisibleResidentCount + GWorkOverviewResidentColumns - 1) /
                GWorkOverviewResidentColumns) :
            1;
    const float MetricsTop =
        ResidentStartY +
        ResidentIconSize +
        static_cast<float>((std::max)(0, ResidentRowCount - 1)) *
            ResidentGapY +
        12.f;
    const float MetricRowH = GWorkOverviewMetricRowHeight;

    for (int Index = 0;
        Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
        ++Index)
    {
        auto Icon =
            Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock();

        if (!Icon)
            continue;

        if (Index < VisibleResidentCount)
        {
            const int Column = Index % GWorkOverviewResidentColumns;
            const int Row = Index / GWorkOverviewResidentColumns;
            Icon->SetPos(
                ResidentStartX + static_cast<float>(Column) * ResidentGapX,
                ResidentStartY + static_cast<float>(Row) * ResidentGapY);
            Icon->SetSize(ResidentIconSize, ResidentIconSize);
        }
        else
        {
            Icon->SetPos(0.f, 0.f);
            Icon->SetSize(0.f, 0.f);
        }
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GOverviewMetricRowCount;
        ++Index)
    {
        auto Label =
            Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock();
        auto Value =
            Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock();
        const bool SectionHeader =
            Value &&
            !Value->GetEnable();
        const float RowTop =
            MetricsTop + static_cast<float>(Index) * MetricRowH;

        if (Label)
        {
            Label->SetPos(BudgetMargin, RowTop);
            Label->SetSize(
                SectionHeader ?
                    (PanelWidth - BudgetMargin * 2.f) :
                    160.f,
                20.f);
        }

        if (Value)
        {
            if (SectionHeader)
            {
                Value->SetPos(0.f, 0.f);
                Value->SetSize(0.f, 0.f);
            }
            else
            {
                Value->SetPos(PanelWidth - BudgetMargin - 140.f, RowTop);
                Value->SetSize(140.f, 20.f);
            }
        }
    }

    LayoutOverviewMetricScrollWidgets(Owner, Context, MetricsTop, MetricRowH);

    const bool ShowVisitorIcons =
        !Widget.mOverviewVisitorIcons[0].expired() &&
        Widget.mOverviewVisitorIcons[0].lock()->GetEnable();

    if (ShowVisitorIcons)
    {
        const float VisitorStartX = BudgetMargin + 2.f;
        const float VisitorStartY =
            MetricsTop +
            static_cast<float>(CCitizenInfoWidget::GOverviewMetricRowCount) *
                MetricRowH +
            10.f;
        const float VisitorIconSize = 24.f;
        const float VisitorGapX = 22.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
            ++Index)
        {
            auto Icon =
                Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock();

            if (!Icon)
                continue;

            Icon->SetPos(
                VisitorStartX +
                    static_cast<float>(Index) * VisitorGapX,
                VisitorStartY);
            Icon->SetSize(VisitorIconSize, VisitorIconSize);
        }
    }
    else
    {
        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }
    }

    if (auto Divider = Widget.mSectionDivider.lock())
    {
        Divider->SetPos(0.f, 0.f);
        Divider->SetSize(0.f, 0.f);
    }
}

void FCitizenInfoRenderer::RefreshBuildingCompactLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float PanelWidth = Context.PanelWidth;
    const float BudgetMargin = Context.BudgetMargin;
    const float SectionRibbonY = Context.SectionRibbonY;
    const float SectionRibbonH = Context.SectionRibbonHeight;

        const float MetricsTop =
            SectionRibbonY + SectionRibbonH + 12.f;
        const float MetricRowH = GCompactMetricRowHeight;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mOverviewBudgetLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewBudgetValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock();
            auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock();
            const float RowTop =
                MetricsTop + static_cast<float>(Index) * MetricRowH;

            if (Label)
            {
                Label->SetPos(BudgetMargin, RowTop);
                Label->SetSize(180.f, 20.f);
            }

            if (Value)
            {
                Value->SetPos(PanelWidth - BudgetMargin - 120.f, RowTop);
                Value->SetSize(120.f, 20.f);
            }
        }

        LayoutOverviewMetricScrollWidgets(Owner, Context, MetricsTop, MetricRowH);

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(
                PanelWidth * 0.5f - Context.Metrics.SectionDividerWidth * 0.5f,
                MetricsTop + MetricRowH + 8.f);
            Divider->SetSize(
                Context.Metrics.SectionDividerWidth,
                Context.Metrics.SectionDividerHeight);
        }
}

void FCitizenInfoRenderer::RefreshBuildingInformationLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float OuterTop = Context.OuterTop;
    const float PanelWidth = Context.PanelWidth;
    const float RibbonOffsetY = Context.RibbonOffsetY;
    const float TitleRibbonH = Context.TitleRibbonHeight;
    const float SectionRibbonY = Context.SectionRibbonY;
    const float SectionRibbonH = Context.SectionRibbonHeight;
    const bool ShowSectionRibbon = Context.ShowSectionRibbon;
    const float BudgetMargin = Context.BudgetMargin;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock())
            {
                Value->SetPos(0.f, 0.f);
                Value->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mOverviewBudgetLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewBudgetValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        const float InfoBaseTop =
            ShowSectionRibbon ?
                (SectionRibbonY + SectionRibbonH) :
                (OuterTop + RibbonOffsetY + TitleRibbonH);
        const float InfoTop = InfoBaseTop + 18.f;
        const float DividerTop = InfoTop + 98.f;
        auto AccentText = Widget.mInformationAccentText.lock();
        const bool ShowAccent = AccentText && AccentText->GetEnable();

        if (AccentText)
        {
            if (ShowAccent)
            {
                AccentText->SetPos(BudgetMargin, InfoTop + 2.f);
                AccentText->SetSize(32.f, 30.f);
            }
            else
            {
                AccentText->SetPos(0.f, 0.f);
                AccentText->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mInformationTopText.lock())
        {
            const float AccentOffset = ShowAccent ? 30.f : 0.f;
            Text->SetPos(BudgetMargin + AccentOffset, InfoTop);
            Text->SetSize(
                PanelWidth - BudgetMargin * 2.f - AccentOffset,
                100.f);
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(
                PanelWidth * 0.5f - Context.Metrics.SectionDividerWidth * 0.5f,
                DividerTop);
            Divider->SetSize(
                Context.Metrics.SectionDividerWidth,
                Context.Metrics.SectionDividerHeight);
        }

        if (auto Text = Widget.mInformationBottomText.lock())
        {
            Text->SetPos(BudgetMargin, DividerTop + 28.f);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f, 110.f);
        }
}

void FCitizenInfoRenderer::RefreshBuildingDefaultLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    (void)Context;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock())
            {
                Value->SetPos(0.f, 0.f);
                Value->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mOverviewBudgetLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewBudgetValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Background = Widget.mOverviewWorkModeBackground.lock())
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(0.f, 0.f);
            Divider->SetSize(0.f, 0.f);
        }
}

void FCitizenInfoRenderer::RefreshInformationVisibilityLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    if (Context.ShowInformationParagraphs)
    {
        return;
    }

    {
        if (auto Text = Widget.mInformationAccentText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mInformationTopText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mInformationBottomText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }
}

void FCitizenInfoRenderer::RefreshUpgradeLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float PanelWidth = Context.PanelWidth;
    const float BudgetMargin = Context.BudgetMargin;
    const float SectionRibbonY = Context.SectionRibbonY;
    const float SectionRibbonH = Context.SectionRibbonHeight;

    if (Context.ShowUpgradeCard)
    {
        const float CardLeft = BudgetMargin;
        const float CardTop = SectionRibbonY + SectionRibbonH + 12.f;
        const float CardWidth = PanelWidth - BudgetMargin * 2.f;
        const float CardHeight = 46.f;

        if (auto Card = Widget.mUpgradeCardBackground.lock())
        {
            Card->SetPos(CardLeft, CardTop);
            Card->SetSize(CardWidth, CardHeight);
        }

        if (auto Icon = Widget.mUpgradeCardIcon.lock())
        {
            Icon->SetPos(CardLeft + 8.f, CardTop + 7.f);
            Icon->SetSize(30.f, 30.f);
        }

        if (auto Text = Widget.mUpgradeCardTitle.lock())
        {
            Text->SetPos(CardLeft + 44.f, CardTop + 2.f);
            Text->SetSize(CardWidth - 52.f, CardHeight - 4.f);
        }

        if (auto Text = Widget.mUpgradeDescriptionText.lock())
        {
            Text->SetPos(BudgetMargin, CardTop + CardHeight + 10.f);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f, 54.f);
        }
    }
    else
    {
        if (auto Card = Widget.mUpgradeCardBackground.lock())
        {
            Card->SetPos(0.f, 0.f);
            Card->SetSize(0.f, 0.f);
        }

        if (auto Icon = Widget.mUpgradeCardIcon.lock())
        {
            Icon->SetPos(0.f, 0.f);
            Icon->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mUpgradeCardTitle.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mUpgradeDescriptionText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

}

void FCitizenInfoRenderer::RefreshActionLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const auto& Layout = Context.Metrics;
    const float OuterTop = Context.OuterTop;
    const float PanelWidth = Context.PanelWidth;
    const float PanelHeight = Context.PanelHeight;
    const float BudgetMargin = Context.BudgetMargin;
    const float ActionTop = Context.ActionTop;
    const bool ShowActions = Context.ShowActions;
    const bool ShowAnyOverview = Context.ShowAnyOverview;
    const bool ShowInformationParagraphs = Context.ShowInformationParagraphs;
    const bool ShowOverviewCommandButton = Context.ShowOverviewCommandButton;
    const bool ShowCitizenProfile = Context.ShowCitizenProfile;
    const float ActionBtnH = UIConfig::BuildingActionButtonHeight;
    const float ActionBtnW = UIConfig::BuildingActionButtonWidth;
    auto ApplyButtonChildFontSize = [](const std::weak_ptr<CButton>& Button, float FontSize)
    {
        auto ButtonShared = Button.lock();

        if (!ButtonShared)
            return;

        const auto Child =
            std::dynamic_pointer_cast<CTextBlock>(ButtonShared->GetChild());

        if (Child)
            Child->SetFontSize(FontSize);
    };

    ApplyButtonChildFontSize(
        Widget.mDemolishButton,
        UIConfig::BuildingActionButtonFontSize);
    ApplyButtonChildFontSize(
        Widget.mMoveButton,
        UIConfig::BuildingActionButtonFontSize);
    ApplyButtonChildFontSize(
        Widget.mFocusButton,
        UIConfig::BuildingActionButtonFontSize);

    if (auto DemolishButton = Widget.mDemolishButton.lock())
    {
        DemolishButton->SetPos(BudgetMargin, ActionTop);
        DemolishButton->SetSize(
            ShowAnyOverview ? (PanelWidth - BudgetMargin * 2.f - 88.f) :
                ActionBtnW,
            ActionBtnH);
    }
    if (auto MoveButton = Widget.mMoveButton.lock())
    {
        if (ShowAnyOverview || ShowInformationParagraphs)
        {
            MoveButton->SetPos(
                PanelWidth - BudgetMargin - Layout.MoveCompactRightOffset,
                ActionTop + Layout.ActionCompactIconOffsetY);
            MoveButton->SetSize(
                Layout.ActionCompactIconSize,
                Layout.ActionCompactIconSize);
        }
        else
        {
            MoveButton->SetPos(PanelWidth - BudgetMargin - ActionBtnW * 1.1f - 6.f, ActionTop);
            MoveButton->SetSize(ActionBtnW * 0.5f, ActionBtnH);
        }
    }
    if (auto FocusButton = Widget.mFocusButton.lock())
    {
        if (ShowAnyOverview || ShowInformationParagraphs)
        {
            FocusButton->SetPos(
                PanelWidth - BudgetMargin - Layout.FocusCompactRightOffset,
                ActionTop + Layout.ActionCompactIconOffsetY);
            FocusButton->SetSize(
                Layout.ActionCompactIconSize,
                Layout.ActionCompactIconSize);
        }
        else
        {
            FocusButton->SetPos(PanelWidth - BudgetMargin - ActionBtnW * 0.55f, ActionTop);
            FocusButton->SetSize(ActionBtnW * 0.5f, ActionBtnH);
        }
    }

    if (auto Button = Widget.mOverviewCommandButton.lock())
    {
        if (ShowOverviewCommandButton)
        {
            Button->SetPos(
                BudgetMargin,
                ActionTop - ActionBtnH - Layout.OverviewCommandGap);
            Button->SetSize(PanelWidth - BudgetMargin * 2.f, ActionBtnH);
        }
        else
        {
            Button->SetPos(0.f, 0.f);
            Button->SetSize(0.f, 0.f);
        }
    }

    const float CitizenActionBtnH = Layout.CitizenActionButtonHeight;
    const float CitizenActionGap = Layout.CitizenActionGap;
    const float CitizenActionTop = ActionTop - Layout.ActionStackTopOffset;

    for (int Index = 0; Index < CCitizenInfoWidget::GCitizenActionButtonCount;
        ++Index)
    {
        const float ButtonTop =
            CitizenActionTop +
            static_cast<float>(Index) * (CitizenActionBtnH + CitizenActionGap);

        if (auto Button =
            Widget.mCitizenActionButtons[static_cast<size_t>(Index)].lock())
        {
            if (ShowCitizenProfile)
            {
                Button->SetPos(BudgetMargin, ButtonTop);
                Button->SetSize(PanelWidth - BudgetMargin * 2.f, CitizenActionBtnH);
            }
            else
            {
                Button->SetPos(0.f, 0.f);
                Button->SetSize(0.f, 0.f);
            }
        }

        if (auto Icon =
            Widget.mCitizenActionButtonIcons[static_cast<size_t>(Index)].lock())
        {
            if (ShowCitizenProfile)
            {
                Icon->SetPos(
                    BudgetMargin + Layout.ActionIconInset,
                    ButtonTop + Layout.ActionIconInset);
                Icon->SetSize(
                    Layout.ActionIconSize,
                    Layout.ActionIconSize);
            }
            else
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }
    }

    if (auto Text = Widget.mCitizenFooterText.lock())
    {
        if (ShowCitizenProfile)
        {
            Text->SetPos(
                BudgetMargin,
                OuterTop + PanelHeight - Layout.FooterBottomInset);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f, 18.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }
}

void FCitizenInfoRenderer::RefreshBodyLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const auto& Layout = Context.Metrics;
    const float OuterTop = Context.OuterTop;
    const float PanelWidth = Context.PanelWidth;
    const float PanelHeight = Context.PanelHeight;
    const float RibbonOffsetY = Context.RibbonOffsetY;
    const float TitleRibbonH = Context.TitleRibbonHeight;
    const float SectionRibbonY = Context.SectionRibbonY;
    const float SectionRibbonH = Context.SectionRibbonHeight;
    const bool ShowSectionRibbon = Context.ShowSectionRibbon;
    const bool ShowCitizenProfile = Context.ShowCitizenProfile;
    const bool ShowCitizenPolitics = Context.ShowCitizenPolitics;
    const bool ShowCitizenThoughts = Context.ShowCitizenThoughts;
    const bool ShowAnyOverview = Context.ShowAnyOverview;
    const bool ShowCompactRows = Context.ShowCompactRows;
    const bool ShowUpgradeCard = Context.ShowUpgradeCard;
    const bool ShowActions = Context.ShowActions;
    const bool ShowInformationParagraphs = Context.ShowInformationParagraphs;
    const float BudgetButtonTop = Context.BudgetButtonTop;
    const float BudgetButtonH = Context.BudgetButtonHeight;
    const float BudgetBaseY = Context.BudgetBaseY;
    const float BudgetMargin = Context.BudgetMargin;
    const float ActionTop = Context.ActionTop;
    int VisibleMetricRowCount = 0;

    if (ShowCompactRows)
    {
        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock();

            if (Label && Label->GetEnable())
                ++VisibleMetricRowCount;
        }
    }

    const float CompactMetricsBottom =
        SectionRibbonY + SectionRibbonH +
        12.f +
        static_cast<float>(VisibleMetricRowCount) * GCompactMetricRowHeight;
    const float InformationBaseTop =
        ShowSectionRibbon ?
            (SectionRibbonY + SectionRibbonH) :
            (OuterTop + RibbonOffsetY + TitleRibbonH);

    const float BodyTop =
        ShowUpgradeCard ? (SectionRibbonY + SectionRibbonH + 136.f) :
        ShowInformationParagraphs ?
            (InformationBaseTop + 266.f) :
        ShowCompactRows ? (CompactMetricsBottom + 24.f) :
        ShowSectionRibbon ? (SectionRibbonY + SectionRibbonH + Layout.BodyGapAfterSection) :
        (ShowCitizenProfile || ShowCitizenPolitics || ShowCitizenThoughts ?
            (OuterTop + PanelHeight) :
            (ShowAnyOverview ? (OuterTop + PanelHeight) :
            (ShowActions ? (BudgetButtonTop + BudgetButtonH + Layout.BodyGapAfterActions) :
                           (BudgetBaseY - Layout.BodyFallbackOffset))));
    const float BodyBottom =
        ShowActions ? (ActionTop - Layout.BodyGapBeforeActions) :
        (OuterTop + PanelHeight - Layout.BodyBottomInset);

    if (auto BodyText = Widget.mBodyText.lock())
    {
        if (ShowCitizenProfile || ShowCitizenPolitics ||
            ShowCitizenThoughts || ShowAnyOverview)
        {
            BodyText->SetPos(0.f, 0.f);
            BodyText->SetSize(0.f, 0.f);
        }
        else
        {
            BodyText->SetPos(BudgetMargin, BodyTop);
            BodyText->SetSize(
                PanelWidth - BudgetMargin * 2.f,
                (std::max)(80.f, BodyBottom - BodyTop));
        }
    }
}

