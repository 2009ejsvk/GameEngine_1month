#pragma once

class CCitizenInfoWidget;

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
    float ActionGroupOffsetX;
    float ActionGroupOffsetY;
    float ActionGroupWidthAdd;
    float ActionGroupHeightAdd;
    float ActionIconInset;
    float ActionIconSize;
    float FooterBottomInset;
    float BodyGapAfterSection;
    float BodyGapAfterActions;
    float BodyGapBeforeActions;
    float BodyFallbackOffset;
    float BodyBottomInset;
};

struct FCitizenInfoPanelMetrics
{
    float OuterTop = 0.f;
    float Width = 0.f;
    float Height = 0.f;
};

struct FCitizenInfoInnerBounds
{
    float MarginX = 0.f;
    float Left = 0.f;
    float Top = 0.f;
    float Width = 0.f;
    float Height = 0.f;
};

struct FCitizenInfoChromeMetrics
{
    float ScrollTrackWidth = 0.f;
    float ScrollThumbHeight = 0.f;
    float CloseButtonSize = 0.f;
    float IconSize = 0.f;
    float CloseOffsetX = 0.f;
    float TitleLeft = 0.f;
};

struct FCitizenInfoRibbonMetrics
{
    float TitleHeight = 0.f;
    float SectionHeight = 0.f;
    float OffsetX = 0.f;
    float OffsetY = 0.f;
    float SectionY = 0.f;
};

struct FCitizenInfoTabMetrics
{
    int VisibleCount = 0;
    float Width = 0.f;
    float Height = 0.f;
    float Gap = 0.f;
    float TotalWidth = 0.f;
    float StartX = 0.f;
};

struct FCitizenInfoBudgetMetrics
{
    float BaseY = 0.f;
    float ButtonHeight = 0.f;
    float ButtonTop = 0.f;
    float WorkModeTop = 0.f;
    float WorkModeBoxTop = 0.f;
    float Margin = 0.f;
    float Gap = 0.f;
    float ButtonWidth = 0.f;
    float OccupancyTop = 0.f;
};

struct FCitizenInfoActionMetrics
{
    float Top = 0.f;
};

struct FCitizenInfoVisibilityFlags
{
    bool IsCitizenMode = false;
    bool TitleIconVisible = false;
    bool ShowSectionRibbon = false;
    bool ShowCitizenProfile = false;
    bool ShowCitizenPolitics = false;
    bool ShowCitizenThoughts = false;
    bool ShowWorkOverview = false;
    bool ShowCustomOverview = false;
    bool ShowResidentialWorkMode = false;
    bool ShowAnyOverview = false;
    bool ShowStatsMetricRows = false;
    bool ShowEfficiencyMetricRows = false;
    bool ShowCompactRows = false;
    bool ShowUpgradeCard = false;
    bool ShowInformationParagraphs = false;
    bool ShowOverviewCommandButton = false;
    bool ShowActions = false;
    bool ShowOperationModeSelectionPage = false;
};

struct FCitizenInfoLayoutContext
{
    FCitizenLayoutMetrics Metrics{};
    FCitizenInfoPanelMetrics Panel{};
    FCitizenInfoInnerBounds Inner{};
    FCitizenInfoChromeMetrics Chrome{};
    FCitizenInfoRibbonMetrics Ribbon{};
    FCitizenInfoTabMetrics Tabs{};
    FCitizenInfoBudgetMetrics Budget{};
    FCitizenInfoActionMetrics Actions{};
    FCitizenInfoVisibilityFlags Flags{};
};

FCitizenLayoutMetrics MakeLayoutMetrics(bool IsCitizenMode);
void LayoutOverviewMetricScrollWidgets(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context,
    float MetricsTop,
    float MetricRowH);
