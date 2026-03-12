#include "CitizenInfoRenderer.h"
#include "CitizenInfoRendererInternal.h"
#include "CitizenInfoWidget.h"
#include "UILayoutConfig.h"
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
    float ActionCompactIconSize;
    float ActionCompactIconOffsetY;
    float MoveCompactRightOffset;
    float CloneCompactRightOffset;
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
constexpr float GCompactBudgetButtonWidth = 36.f;
constexpr float GCompactControlHeight = 22.f;
constexpr float GSectionDividerWidth = 172.f;
constexpr float GSectionDividerHeight = 14.f;

template <typename T>
bool IsEnabled(const std::weak_ptr<T>& Widget)
{
    if (auto Pinned = Widget.lock())
    {
        return Pinned->GetEnable();
    }

    return false;
}

FCitizenLayoutMetrics MakeCitizenLayoutMetrics()
{
    const float TitleIconInsetX = 6.f;
    const float TitleTextInsetX = 14.f;
    const float TitleIconGap = 6.f;
    const float SubtitleOffsetY = 4.f;
    const float CollapsedSectionGap = 6.f;
    const float BudgetLabelOffsetY = 2.f;
    const float BudgetCustomButtonsOffsetY = 20.f;
    const float BudgetWorkButtonsOffsetY = 78.f;
    const float BudgetDefaultButtonsOffsetY = 26.f;
    const float BudgetCompactGap = 6.f;
    const float BudgetDefaultGap = 8.f;
    const float OccupancyGapY = 18.f;
    const float ActionCompactIconSize = 34.f;
    const float ActionCompactIconOffsetY = 2.f;
    const float MoveCompactRightOffset = 82.f;
    const float CloneCompactRightOffset = 40.f;
    const float OverviewCommandGap = 10.f;
    const float CitizenActionButtonHeight = 38.f;
    const float CitizenActionGap = 4.f;
    const float ActionIconInset = 8.f;
    const float ActionIconSize = GCompactControlHeight;
    const float BodyGapAfterSection = 8.f;
    const float BodyGapAfterActions = 14.f;
    const float BodyGapBeforeActions = 12.f;
    const float BodyFallbackOffset = 4.f;

    return
    {
        UIConfig::CitizenPanelInnerTopOffset,
        UIConfig::CitizenPanelInnerBottomInset,
        UIConfig::CitizenScrollBottomInset,
        UIConfig::CitizenScrollThumbTopOffset,
        UIConfig::CitizenCloseButtonOffsetY,
        TitleIconInsetX,
        TitleTextInsetX,
        TitleIconGap,
        SubtitleOffsetY,
        UIConfig::CitizenSectionRibbonOffsetY,
        CollapsedSectionGap,
        UIConfig::CitizenBudgetBaseOffsetY,
        BudgetLabelOffsetY,
        BudgetCustomButtonsOffsetY,
        BudgetWorkButtonsOffsetY,
        BudgetDefaultButtonsOffsetY,
        BudgetCompactGap,
        BudgetDefaultGap,
        OccupancyGapY,
        ActionCompactIconSize,
        ActionCompactIconOffsetY,
        MoveCompactRightOffset,
        CloneCompactRightOffset,
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
        UIConfig::CitizenBodyBottomInset
    };
}
} // namespace

void FCitizenInfoRenderer::RefreshLayout(CCitizenInfoWidget& Widget)
{
    (void)Widget.mRequestedScreenPos;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);

    Widget.mPanelWidth = (std::min)(
        UIConfig::CitizenPanelMaxWidth,
        (std::max)(
            UIConfig::CitizenPanelMinWidth,
            ScreenWidth * UIConfig::CitizenPanelWidthRatio));
    Widget.mPanelTop = UIConfig::CitizenPanelTopOffset;
    Widget.mPanelHeight = (std::max)(
        UIConfig::CitizenPanelMinHeight,
        ScreenHeight - Widget.mPanelTop - UIConfig::CitizenPanelBottomMargin);

    const float PanelLeft =
        ScreenWidth - Widget.mPanelWidth - UIConfig::CitizenPanelRightInset;
    Widget.SetPos(PanelLeft, 0.f);
    Widget.SetSize(Widget.mPanelWidth, Widget.mPanelTop + Widget.mPanelHeight);

    FCitizenInfoLayoutContext Layout;
    Layout.Metrics = MakeCitizenLayoutMetrics();
    Layout.OuterTop = Widget.mPanelTop;
    Layout.PanelWidth = Widget.mPanelWidth;
    Layout.PanelHeight = Widget.mPanelHeight;
    Layout.InnerMarginX = UIConfig::CitizenPanelInnerMarginX;
    Layout.InnerLeft = Layout.InnerMarginX;
    Layout.InnerTop = Layout.OuterTop + Layout.Metrics.PanelInnerTopOffset;
    Layout.InnerWidth = Layout.PanelWidth - Layout.InnerMarginX * 2.f;
    Layout.InnerHeight = Layout.PanelHeight - Layout.Metrics.PanelInnerBottomInset;
    Layout.ScrollTrackWidth = UIConfig::CitizenScrollTrackWidth;
    Layout.ScrollThumbHeight = UIConfig::CitizenScrollThumbHeight;
    Layout.TitleRibbonHeight = UIConfig::CitizenTitleRibbonHeight;
    Layout.SectionRibbonHeight = UIConfig::CitizenSectionRibbonHeight;
    Layout.CloseButtonSize = UIConfig::CitizenCloseButtonSize;
    Layout.IconSize = UIConfig::BuildingIconSize;
    Layout.IsCitizenMode =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen;
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
        IsEnabled(Widget.mOverviewBudgetLabel);
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
        GCompactControlHeight :
        UIConfig::BuildingBudgetButtonHeight;
    Layout.BudgetButtonTop = Layout.ShowCustomOverview ?
        (Layout.BudgetBaseY + Layout.Metrics.BudgetCustomButtonsOffsetY) :
        (Layout.ShowWorkOverview ?
            (Layout.BudgetBaseY + Layout.Metrics.BudgetWorkButtonsOffsetY) :
            (Layout.BudgetBaseY + Layout.Metrics.BudgetDefaultButtonsOffsetY));
    Layout.WorkModeTop = Layout.BudgetBaseY - Layout.Metrics.BudgetLabelOffsetY;
    Layout.WorkModeBoxTop = Layout.WorkModeTop + GCompactControlHeight;
    Layout.BudgetMargin = Layout.RibbonOffsetX;
    Layout.BudgetGap = Layout.ShowAnyOverview ?
        Layout.Metrics.BudgetCompactGap :
        Layout.Metrics.BudgetDefaultGap;
    Layout.BudgetButtonWidth = Layout.ShowAnyOverview ?
        GCompactBudgetButtonWidth :
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

    ValidateLayoutForDebug(Widget, Layout);
    RefreshCommonLayout(Widget, Layout);
    RefreshCitizenModeLayout(Widget, Layout);
    RefreshBuildingModeLayout(Widget, Layout);
    RefreshInformationVisibilityLayout(Widget, Layout);
    RefreshUpgradeLayout(Widget, Layout);
    RefreshActionLayout(Widget, Layout);
    RefreshBodyLayout(Widget, Layout);
}

void FCitizenInfoRenderer::ValidateLayoutForDebug(
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Layout)
{
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
        Widget.mOverviewVisitorIcons.size() ==
        CCitizenInfoWidget::GOverviewVisitorSlotCount);
    assert(
        Widget.mOverviewMetricLabels.size() ==
        CCitizenInfoWidget::GOverviewMetricRowCount);
    assert(
        Widget.mOverviewMetricValues.size() ==
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
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
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
    const bool ShowAnyOverview = Context.ShowAnyOverview;
    // 폰트 크기
    if (auto TitleTxt = Widget.mTitleText.lock())
        TitleTxt->SetFontSize(UIConfig::CitizenTitleFontSize);
    if (auto SubTxt = Widget.mSubtitleText.lock())
        SubTxt->SetFontSize(UIConfig::CitizenSubtitleFontSize);
    if (auto BodyTxt = Widget.mBodyText.lock())
        BodyTxt->SetFontSize(UIConfig::CitizenBodyFontSize);

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
            OuterTop + RibbonOffsetY + (TitleRibbonH - IconSz) * 0.5f);
        TitleIcon->SetSize(IconSz, IconSz);
    }
    if (auto TitleText = Widget.mTitleText.lock())
    {
        TitleText->SetPos(TitleLeft, OuterTop + RibbonOffsetY);
        TitleText->SetSize(PanelWidth - TitleLeft - CloseButtonSz - CloseOffsetX, TitleRibbonH);
    }
    if (auto SubtitleText = Widget.mSubtitleText.lock())
    {
        SubtitleText->SetPos(
            RibbonOffsetX,
            OuterTop + RibbonOffsetY + TitleRibbonH + Layout.SubtitleOffsetY);
        SubtitleText->SetSize(
            PanelWidth - RibbonOffsetX * 2.f,
            GCompactControlHeight);
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
                GCompactControlHeight);
        }
    }

    const float BudgetButtonH   = ShowAnyOverview ? GCompactControlHeight :
        UIConfig::BuildingBudgetButtonHeight;
    const float BudgetButtonTop = ShowCustomOverview ?
        (BudgetBaseY + Layout.BudgetCustomButtonsOffsetY) :
        (ShowWorkOverview ? (BudgetBaseY + Layout.BudgetWorkButtonsOffsetY) :
        (BudgetBaseY + Layout.BudgetDefaultButtonsOffsetY));
    const float WorkModeTop = BudgetBaseY - Layout.BudgetLabelOffsetY;
    const float WorkModeBoxTop = WorkModeTop + GCompactControlHeight;
    const float BudgetMargin    = RibbonOffsetX;
    const float BudgetGap       = ShowAnyOverview ?
        Layout.BudgetCompactGap :
        Layout.BudgetDefaultGap;
    const float BudgetButtonW   = ShowAnyOverview ?
        GCompactBudgetButtonWidth :
        (PanelWidth - BudgetMargin * 2.f - BudgetGap * 4.f) / 5.f;

    if (auto Text = Widget.mOverviewWorkModeLabel.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(BudgetMargin, WorkModeTop);
            Text->SetSize(PanelWidth * 0.5f, GCompactControlHeight);
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
            Background->SetPos(BudgetMargin, WorkModeBoxTop);
            Background->SetSize(PanelWidth - BudgetMargin * 2.f, 34.f);
        }
        else
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewWorkModeText.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(BudgetMargin + 12.f, WorkModeBoxTop + 2.f);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f - 24.f, 30.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewBudgetLabel.lock())
    {
        if (ShowAnyOverview)
        {
            Text->SetPos(BudgetMargin, BudgetBaseY - Layout.BudgetLabelOffsetY);
            Text->SetSize(PanelWidth * 0.5f, GCompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewBudgetValue.lock())
    {
        if (ShowAnyOverview)
        {
            Text->SetPos(
                PanelWidth - BudgetMargin - 120.f,
                BudgetBaseY - Layout.BudgetLabelOffsetY);
            Text->SetSize(120.f, GCompactControlHeight);
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
        if (ShowAnyOverview)
        {
            Text->SetPos(BudgetMargin, OccupancyTop);
            Text->SetSize(PanelWidth * 0.5f, GCompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewOccupancyValue.lock())
    {
        if (ShowAnyOverview)
        {
            Text->SetPos(PanelWidth - BudgetMargin - 120.f, OccupancyTop);
            Text->SetSize(120.f, GCompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

}

void FCitizenInfoRenderer::RefreshCitizenModeLayout(
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
    if (!Context.IsCitizenMode)
    {
        return;
    }

    if (Context.ShowCitizenThoughts)
    {
        RefreshCitizenThoughtsLayout(Widget, Context);
        return;
    }

    if (Context.ShowCitizenPolitics)
    {
        RefreshCitizenPoliticsLayout(Widget, Context);
        return;
    }

    if (Context.ShowCitizenProfile)
    {
        RefreshCitizenProfileLayout(Widget, Context);
    }
}

void FCitizenInfoRenderer::RefreshCitizenThoughtsLayout(
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
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
        const float DividerWidth = GSectionDividerWidth;
        const float DividerHeight = GSectionDividerHeight;
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
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
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
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
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
                PanelWidth * 0.5f - 86.f,
                DetailTop +
                    DetailRowH * 8.f +
                    6.f);
            Divider->SetSize(GSectionDividerWidth, GSectionDividerHeight);
        }
}

void FCitizenInfoRenderer::RefreshBuildingModeLayout(
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
    if (Context.IsCitizenMode &&
        (Context.ShowCitizenThoughts ||
            Context.ShowCitizenPolitics ||
            Context.ShowCitizenProfile))
    {
        return;
    }

    if (Context.ShowCustomOverview)
    {
        RefreshBuildingCustomOverviewLayout(Widget, Context);
    }
    else if (Context.ShowWorkOverview)
    {
        RefreshBuildingWorkOverviewLayout(Widget, Context);
    }
    else if (Context.ShowCompactRows)
    {
        RefreshBuildingCompactLayout(Widget, Context);
    }
    else if (Context.ShowInformationParagraphs)
    {
        RefreshBuildingInformationLayout(Widget, Context);
    }
    else
    {
        RefreshBuildingDefaultLayout(Widget, Context);
    }
}

void FCitizenInfoRenderer::RefreshBuildingCustomOverviewLayout(
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
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
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock();

            if (!Icon)
                continue;

            const int Column = Index % 4;
            const int Row = Index / 4;
            Icon->SetPos(
                ResidentStartX + static_cast<float>(Column) * ResidentGapX,
                ResidentStartY + static_cast<float>(Row) * ResidentGapY);
            Icon->SetSize(ResidentIconSize, ResidentIconSize);
        }

        const float MetricsTop = ResidentStartY + ResidentGapY * 4.f + 14.f;
        const float MetricRowH = 24.f;

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
                Label->SetSize(140.f, 22.f);
            }

            if (Value)
            {
                Value->SetPos(PanelWidth - BudgetMargin - 160.f, RowTop);
                Value->SetSize(160.f, 22.f);
            }
        }
}

void FCitizenInfoRenderer::RefreshBuildingWorkOverviewLayout(
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
    const float PanelWidth = Context.PanelWidth;
    const float BudgetMargin = Context.BudgetMargin;
    const float OccupancyTop = Context.OccupancyTop;

    constexpr int GWorkOverviewResidentColumns = 6;
    constexpr int GWorkOverviewMaxVisibleResidents = 12;
    const float ResidentStartX = BudgetMargin + 6.f;
    const float ResidentStartY = OccupancyTop + 26.f;
    const float ResidentIconSize = 24.f;
    const float ResidentGapX = 26.f;
    const float ResidentGapY = 24.f;
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
        18.f;
    const float MetricRowH = 23.f;

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
                22.f);
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
                Value->SetSize(140.f, 22.f);
            }
        }
    }

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
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
    const float PanelWidth = Context.PanelWidth;
    const float BudgetMargin = Context.BudgetMargin;
    const float SectionRibbonY = Context.SectionRibbonY;
    const float SectionRibbonH = Context.SectionRibbonHeight;

        const float MetricsTop =
            SectionRibbonY + SectionRibbonH + 12.f;
        const float MetricRowH = 30.f;

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
                Label->SetSize(180.f, 24.f);
            }

            if (Value)
            {
                Value->SetPos(PanelWidth - BudgetMargin - 120.f, RowTop);
                Value->SetSize(120.f, 24.f);
            }
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(
                PanelWidth * 0.5f - 86.f,
                MetricsTop + MetricRowH + 8.f);
            Divider->SetSize(GSectionDividerWidth, GSectionDividerHeight);
        }
}

void FCitizenInfoRenderer::RefreshBuildingInformationLayout(
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
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

        const float InfoTop = OuterTop + RibbonOffsetY + TitleRibbonH + 18.f;
        const float DividerTop = InfoTop + 98.f;

        if (auto Text = Widget.mInformationAccentText.lock())
        {
            Text->SetPos(BudgetMargin, InfoTop + 2.f);
            Text->SetSize(32.f, 30.f);
        }

        if (auto Text = Widget.mInformationTopText.lock())
        {
            Text->SetPos(BudgetMargin + 30.f, InfoTop);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f - 30.f, 100.f);
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(PanelWidth * 0.5f - 86.f, DividerTop);
            Divider->SetSize(GSectionDividerWidth, GSectionDividerHeight);
        }

        if (auto Text = Widget.mInformationBottomText.lock())
        {
            Text->SetPos(BudgetMargin, DividerTop + 28.f);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f, 110.f);
        }
}

void FCitizenInfoRenderer::RefreshBuildingDefaultLayout(
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
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
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
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
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
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
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
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
    if (auto CloneButton = Widget.mCloneButton.lock())
    {
        if (ShowAnyOverview || ShowInformationParagraphs)
        {
            CloneButton->SetPos(
                PanelWidth - BudgetMargin - Layout.CloneCompactRightOffset,
                ActionTop + Layout.ActionCompactIconOffsetY);
            CloneButton->SetSize(
                Layout.ActionCompactIconSize,
                Layout.ActionCompactIconSize);
        }
        else
        {
            CloneButton->SetPos(PanelWidth - BudgetMargin - ActionBtnW * 0.55f, ActionTop);
            CloneButton->SetSize(ActionBtnW * 0.5f, ActionBtnH);
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
    CCitizenInfoWidget& Widget,
    const FCitizenInfoLayoutContext& Context)
{
    const auto& Layout = Context.Metrics;
    const float OuterTop = Context.OuterTop;
    const float PanelWidth = Context.PanelWidth;
    const float PanelHeight = Context.PanelHeight;
    const float SectionRibbonY = Context.SectionRibbonY;
    const float SectionRibbonH = Context.SectionRibbonHeight;
    const bool ShowSectionRibbon = Context.ShowSectionRibbon;
    const bool ShowCitizenProfile = Context.ShowCitizenProfile;
    const bool ShowCitizenPolitics = Context.ShowCitizenPolitics;
    const bool ShowCitizenThoughts = Context.ShowCitizenThoughts;
    const bool ShowAnyOverview = Context.ShowAnyOverview;
    const bool ShowActions = Context.ShowActions;
    const bool ShowInformationParagraphs = Context.ShowInformationParagraphs;
    const float BudgetButtonTop = Context.BudgetButtonTop;
    const float BudgetButtonH = Context.BudgetButtonHeight;
    const float BudgetBaseY = Context.BudgetBaseY;
    const float BudgetMargin = Context.BudgetMargin;
    const float ActionTop = Context.ActionTop;

    const float BodyTop =
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
            ShowCitizenThoughts || ShowAnyOverview || ShowInformationParagraphs)
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

