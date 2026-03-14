#pragma once

#include "UI/WidgetContainer.h"
#include "../Building/BuildingTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <array>
#include <vector>

class CAlmanacWidget;
class FAlmanacRenderer;

namespace AlmanacDataProvider
{
    struct FAlmanacSnapshot;
}

namespace AlmanacRendererPopulationPage
{
    void Apply(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
}

enum class EAlmanacPage
{
    Overview = 0,
    Satisfaction,
    Population,
    Economy,
    Resources,
    Politics,
    Foreign,
    Buildings,
    Conflict,
    Count
};

class CAlmanacWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;
    friend class FAlmanacRenderer;
    friend void AlmanacRendererPopulationPage::Apply(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);

protected:
    CAlmanacWidget();

public:
    virtual ~CAlmanacWidget();

public:
    struct FCardWidgets
    {
        std::weak_ptr<class CImage>     Background;
        std::weak_ptr<class CImage>     Icon;
        std::weak_ptr<class CTextBlock> Title;
        std::weak_ptr<class CTextBlock> Value;
        std::weak_ptr<class CTextBlock> Detail;
    };

    struct FMetricRowWidgets
    {
        std::weak_ptr<class CImage>       Background;
        std::weak_ptr<class CTextBlock>   Label;
        std::weak_ptr<class CProgressBar> Bar;
        std::weak_ptr<class CTextBlock>   Value;
    };

    struct FSatisfactionRowWidgets
    {
        std::weak_ptr<class CButton>      Button;
        std::weak_ptr<class CImage>       Icon;
        std::weak_ptr<class CTextBlock>   Label;
        std::weak_ptr<class CProgressBar> Bar;
        std::weak_ptr<class CTextBlock>   Value;
    };

    struct FPoliticsFactionTileWidgets
    {
        std::weak_ptr<class CButton>    Button;
        std::weak_ptr<class CImage>     Icon;
        std::weak_ptr<class CTextBlock> Label;
        std::weak_ptr<class CImage>     CountIcon;
        std::weak_ptr<class CTextBlock> CountValue;
        std::weak_ptr<class CImage>     FavorIcon;
        std::weak_ptr<class CTextBlock> FavorValue;
    };

    struct FDetailRowWidgets
    {
        std::weak_ptr<class CButton>    Button;
        std::weak_ptr<class CImage>     Background;
        std::weak_ptr<class CTextBlock> Label;
        std::weak_ptr<class CTextBlock> Value;
    };

private:
    using WImage = std::weak_ptr<class CImage>;
    using WText = std::weak_ptr<class CTextBlock>;
    using WButton = std::weak_ptr<class CButton>;
    using WProgressBar = std::weak_ptr<class CProgressBar>;
    using WContainer = std::weak_ptr<CWidgetContainer>;

    struct FChromeWidgets
    {
        WImage PanelBackground;
        WImage ContentFrame;
        WImage TitleRibbon;
        WImage TabMarker;
        WImage LeftRailTrack;
        WImage LeftRailThumb;
        WText TitleText;
        WButton CloseButton;
        std::vector<WButton> TabButtons;
        std::array<WContainer, static_cast<size_t>(EAlmanacPage::Count)> Pages;
    };

    struct FOverviewPageWidgets
    {
        std::vector<FCardWidgets> Cards;
        std::vector<WText> SectionTitles;
        WImage ElectionLeftArrow;
        WImage ElectionRightArrow;
        WText ElectionText;
        WText SummaryLeft;
        WText SummaryRight;
    };

    struct FSatisfactionPageWidgets
    {
        std::vector<FSatisfactionRowWidgets> Rows;
        WImage ListTitleBackground;
        WText ListTitle;
        WImage ChartTitleBackground;
        WImage ChartFrame;
        WImage ChartYAxisLine;
        WImage ChartXAxisLine;
        WImage ChartYAxisArrow;
        WImage ChartXAxisArrow;
        WText ChartTitle;
        WImage TooltipPanel;
        WText TooltipText;
        std::vector<WImage> ChartGridLines;
        std::vector<WImage> ChartPrimaryLines;
        std::vector<WImage> ChartSecondaryLines;
        std::vector<WText> ChartXAxisLabels;
        std::vector<WText> ChartYAxisLabels;
        std::vector<FDetailRowWidgets> Details;
    };

    struct FPopulationPageWidgets
    {
        std::vector<FDetailRowWidgets> Details;
        std::vector<FMetricRowWidgets> Metrics;
        WImage TrendTitleBackground;
        WImage TrendFrame;
        WImage TrendYAxisLine;
        WImage TrendXAxisLine;
        WImage TrendYAxisArrow;
        WImage TrendXAxisArrow;
        WText TrendTitle;
        std::vector<WImage> TrendGridLines;
        std::vector<WImage> TrendLines;
        std::vector<WImage> TrendChildBars;
        std::vector<WImage> TrendAdultBars;
        std::vector<WImage> TrendRetiredBars;
        std::vector<WImage> TrendRichBars;
        std::vector<WImage> TrendFilthyRichBars;
        std::vector<WText> TrendXAxisLabels;
        std::vector<WText> TrendYAxisLabels;
        WImage ChangeTitleBackground;
        WImage ChangeFrame;
        WImage ChangeYAxisLine;
        WImage ChangeXAxisLine;
        WImage ChangeYAxisArrow;
        WImage ChangeXAxisArrow;
        WText ChangeTitle;
        std::vector<WImage> ChangeGridLines;
        std::vector<WImage> ChangePositiveBars;
        std::vector<WImage> ChangeNegativeBars;
        std::vector<WText> ChangeXAxisLabels;
        std::vector<WText> ChangeYAxisLabels;
    };

    struct FEconomyPageWidgets
    {
        std::vector<FDetailRowWidgets> Details;
        std::vector<FMetricRowWidgets> Metrics;
        WImage TrendTitleBackground;
        WImage TrendFrame;
        WImage TrendYAxisLine;
        WImage TrendXAxisLine;
        WImage TrendYAxisArrow;
        WImage TrendXAxisArrow;
        WText TrendTitle;
        std::vector<WImage> TrendGridLines;
        std::vector<WImage> TrendLines;
        std::vector<WImage> TrendBars;
        std::vector<WImage> TrendSecondaryBars;
        std::vector<WImage> TrendTertiaryBars;
        std::vector<WText> TrendXAxisLabels;
        std::vector<WText> TrendYAxisLabels;
        WImage ChangeFrame;
        WImage ChangeYAxisLine;
        WImage ChangeXAxisLine;
        WImage ChangeYAxisArrow;
        WImage ChangeXAxisArrow;
        std::vector<WImage> ChangeGridLines;
        std::vector<WImage> ChangePositiveBars;
        std::vector<WImage> ChangeNegativeBars;
        std::vector<WText> ChangeYAxisLabels;
        WImage BreakdownTitleBackground;
        WText BreakdownTitle;
        std::vector<FDetailRowWidgets> BreakdownRows;
    };

    struct FResourcePageWidgets
    {
        WImage ListTitleBackground;
        WText ListTitle;
        WImage FilterBackground;
        WText FilterText;
        WImage FilterLeftIcon;
        WImage FilterSortIcon;
        WImage FilterSortArrow;
        std::vector<FDetailRowWidgets> Rows;
        WImage ProductionTitleBackground;
        WText ProductionTitle;
        WImage ProductionFrame;
        WImage ProductionYAxisLine;
        WImage ProductionXAxisLine;
        WImage ProductionYAxisArrow;
        WImage ProductionXAxisArrow;
        std::vector<WImage> ProductionGridLines;
        std::vector<WImage> ProductionBars;
        std::vector<WText> ProductionXAxisLabels;
        std::vector<WText> ProductionYAxisLabels;
        WImage ProductionLegendPrimarySwatch;
        WText ProductionLegendPrimaryText;
        WImage ProductionLegendSecondarySwatch;
        WText ProductionLegendSecondaryText;
        WImage DistributionTitleBackground;
        WText DistributionTitle;
        WImage DistributionFilterBackground;
        WText DistributionFilterText;
        std::vector<FMetricRowWidgets> DistributionRows;
        WImage TrackingTitleBackground;
        WText TrackingTitle;
        WText TrackingName;
        WText TrackingValue;
        std::vector<FDetailRowWidgets> Details;
        WText Notice;
    };

    struct FPoliticsPageWidgets
    {
        WImage ListTitleBackground;
        WText ListTitle;
        std::vector<FPoliticsFactionTileWidgets> FactionTiles;
        std::vector<WText> NeutralTexts;
        WImage SupportTitleBackground;
        WText SupportTitle;
        std::vector<FDetailRowWidgets> SupportRows;
        WImage ElectionLeftArrow;
        WImage ElectionRightArrow;
        WText ElectionText;
        WText FactionTitle;
        WText FactionApprovalLabel;
        WText FactionApprovalValue;
        std::vector<FDetailRowWidgets> Details;
    };

    struct FForeignPageWidgets
    {
        std::vector<FSatisfactionRowWidgets> Rows;
        WImage TitleBackground;
        WText Title;
        WText StatusLabel;
        WText StatusValue;
        std::vector<FDetailRowWidgets> Details;
        std::vector<FMetricRowWidgets> Metrics;
        WText Notice;
    };

    struct FBuildingPageWidgets
    {
        std::vector<FDetailRowWidgets> Rows;
        WText CategoryTitle;
        std::vector<FDetailRowWidgets> Details;
    };

    struct FConflictPageWidgets
    {
        WImage HeadlineBackground;
        WText HeadlineText;
        std::vector<FDetailRowWidgets> Details;
        std::vector<FMetricRowWidgets> Metrics;
    };

    struct FWidgetState
    {
        bool Open = false;
        EAlmanacPage SelectedPage = EAlmanacPage::Overview;
        int SelectedSatisfactionIndex = 0;
        int SelectedPopulationIndex = 0;
        int SelectedEconomyIndex = 0;
        int SelectedResourceIndex = 0;
        int VisibleResourceRowOffset = 0;
        int SelectedPoliticsFactionIndex = 0;
        int SelectedForeignPowerIndex = 0;
        int SelectedBuildingCategoryIndex = 0;
        float PanelWidth = 1060.f;
        float PanelHeight = 744.f;
        float DataRefreshAccum = 0.f;
        bool LayoutDirty = true;
        int LastResolutionWidth = 0;
        int LastResolutionHeight = 0;
    };

    // Primary ownership is grouped by page/concern; aliases below keep
    // existing renderer code working while callers migrate gradually.
    FChromeWidgets mChrome;
    FOverviewPageWidgets mOverviewPage;
    FSatisfactionPageWidgets mSatisfactionPage;
    FPopulationPageWidgets mPopulationPage;
    FEconomyPageWidgets mEconomyPage;
    FResourcePageWidgets mResourcePage;
    FPoliticsPageWidgets mPoliticsPage;
    FForeignPageWidgets mForeignPage;
    FBuildingPageWidgets mBuildingPage;
    FConflictPageWidgets mConflictPage;
    FWidgetState mState;

    // Compatibility aliases for existing renderer/data-provider code.
    WImage& mPanelBackground = mChrome.PanelBackground;
    WImage& mContentFrame = mChrome.ContentFrame;
    WImage& mTitleRibbon = mChrome.TitleRibbon;
    WImage& mTabMarker = mChrome.TabMarker;
    WImage& mLeftRailTrack = mChrome.LeftRailTrack;
    WImage& mLeftRailThumb = mChrome.LeftRailThumb;
    WText& mTitleText = mChrome.TitleText;
    WButton& mCloseButton = mChrome.CloseButton;
    std::vector<WButton>& mTabButtons = mChrome.TabButtons;
    std::array<WContainer, static_cast<size_t>(EAlmanacPage::Count)>& mPages = mChrome.Pages;

    std::vector<FCardWidgets>& mOverviewCards = mOverviewPage.Cards;
    std::vector<WText>& mOverviewSectionTitles = mOverviewPage.SectionTitles;
    WImage& mOverviewElectionLeftArrow = mOverviewPage.ElectionLeftArrow;
    WImage& mOverviewElectionRightArrow = mOverviewPage.ElectionRightArrow;
    WText& mOverviewElectionText = mOverviewPage.ElectionText;
    WText& mOverviewSummaryLeft = mOverviewPage.SummaryLeft;
    WText& mOverviewSummaryRight = mOverviewPage.SummaryRight;

    std::vector<FSatisfactionRowWidgets>& mSatisfactionRows = mSatisfactionPage.Rows;
    WImage& mSatisfactionListTitleBackground = mSatisfactionPage.ListTitleBackground;
    WText& mSatisfactionListTitle = mSatisfactionPage.ListTitle;
    WImage& mSatisfactionChartTitleBackground = mSatisfactionPage.ChartTitleBackground;
    WImage& mSatisfactionChartFrame = mSatisfactionPage.ChartFrame;
    WImage& mSatisfactionChartYAxisLine = mSatisfactionPage.ChartYAxisLine;
    WImage& mSatisfactionChartXAxisLine = mSatisfactionPage.ChartXAxisLine;
    WImage& mSatisfactionChartYAxisArrow = mSatisfactionPage.ChartYAxisArrow;
    WImage& mSatisfactionChartXAxisArrow = mSatisfactionPage.ChartXAxisArrow;
    WText& mSatisfactionChartTitle = mSatisfactionPage.ChartTitle;
    WImage& mSatisfactionTooltipPanel = mSatisfactionPage.TooltipPanel;
    WText& mSatisfactionTooltipText = mSatisfactionPage.TooltipText;
    std::vector<WImage>& mSatisfactionChartGridLines = mSatisfactionPage.ChartGridLines;
    std::vector<WImage>& mSatisfactionChartPrimaryLines = mSatisfactionPage.ChartPrimaryLines;
    std::vector<WImage>& mSatisfactionChartSecondaryLines = mSatisfactionPage.ChartSecondaryLines;
    std::vector<WText>& mSatisfactionChartXAxisLabels = mSatisfactionPage.ChartXAxisLabels;
    std::vector<WText>& mSatisfactionChartYAxisLabels = mSatisfactionPage.ChartYAxisLabels;
    std::vector<FDetailRowWidgets>& mSatisfactionDetails = mSatisfactionPage.Details;

    std::vector<FDetailRowWidgets>& mPopulationDetails = mPopulationPage.Details;
    std::vector<FMetricRowWidgets>& mPopulationMetrics = mPopulationPage.Metrics;
    WImage& mPopulationTrendTitleBackground = mPopulationPage.TrendTitleBackground;
    WImage& mPopulationTrendFrame = mPopulationPage.TrendFrame;
    WImage& mPopulationTrendYAxisLine = mPopulationPage.TrendYAxisLine;
    WImage& mPopulationTrendXAxisLine = mPopulationPage.TrendXAxisLine;
    WImage& mPopulationTrendYAxisArrow = mPopulationPage.TrendYAxisArrow;
    WImage& mPopulationTrendXAxisArrow = mPopulationPage.TrendXAxisArrow;
    WText& mPopulationTrendTitle = mPopulationPage.TrendTitle;
    std::vector<WImage>& mPopulationTrendGridLines = mPopulationPage.TrendGridLines;
    std::vector<WImage>& mPopulationTrendLines = mPopulationPage.TrendLines;
    std::vector<WImage>& mPopulationTrendChildBars = mPopulationPage.TrendChildBars;
    std::vector<WImage>& mPopulationTrendAdultBars = mPopulationPage.TrendAdultBars;
    std::vector<WImage>& mPopulationTrendRetiredBars = mPopulationPage.TrendRetiredBars;
    std::vector<WImage>& mPopulationTrendRichBars = mPopulationPage.TrendRichBars;
    std::vector<WImage>& mPopulationTrendFilthyRichBars = mPopulationPage.TrendFilthyRichBars;
    std::vector<WText>& mPopulationTrendXAxisLabels = mPopulationPage.TrendXAxisLabels;
    std::vector<WText>& mPopulationTrendYAxisLabels = mPopulationPage.TrendYAxisLabels;
    WImage& mPopulationChangeTitleBackground = mPopulationPage.ChangeTitleBackground;
    WImage& mPopulationChangeFrame = mPopulationPage.ChangeFrame;
    WImage& mPopulationChangeYAxisLine = mPopulationPage.ChangeYAxisLine;
    WImage& mPopulationChangeXAxisLine = mPopulationPage.ChangeXAxisLine;
    WImage& mPopulationChangeYAxisArrow = mPopulationPage.ChangeYAxisArrow;
    WImage& mPopulationChangeXAxisArrow = mPopulationPage.ChangeXAxisArrow;
    WText& mPopulationChangeTitle = mPopulationPage.ChangeTitle;
    std::vector<WImage>& mPopulationChangeGridLines = mPopulationPage.ChangeGridLines;
    std::vector<WImage>& mPopulationChangePositiveBars = mPopulationPage.ChangePositiveBars;
    std::vector<WImage>& mPopulationChangeNegativeBars = mPopulationPage.ChangeNegativeBars;
    std::vector<WText>& mPopulationChangeXAxisLabels = mPopulationPage.ChangeXAxisLabels;
    std::vector<WText>& mPopulationChangeYAxisLabels = mPopulationPage.ChangeYAxisLabels;

    std::vector<FDetailRowWidgets>& mEconomyDetails = mEconomyPage.Details;
    std::vector<FMetricRowWidgets>& mEconomyMetrics = mEconomyPage.Metrics;
    WImage& mEconomyTrendTitleBackground = mEconomyPage.TrendTitleBackground;
    WImage& mEconomyTrendFrame = mEconomyPage.TrendFrame;
    WImage& mEconomyTrendYAxisLine = mEconomyPage.TrendYAxisLine;
    WImage& mEconomyTrendXAxisLine = mEconomyPage.TrendXAxisLine;
    WImage& mEconomyTrendYAxisArrow = mEconomyPage.TrendYAxisArrow;
    WImage& mEconomyTrendXAxisArrow = mEconomyPage.TrendXAxisArrow;
    WText& mEconomyTrendTitle = mEconomyPage.TrendTitle;
    std::vector<WImage>& mEconomyTrendGridLines = mEconomyPage.TrendGridLines;
    std::vector<WImage>& mEconomyTrendLines = mEconomyPage.TrendLines;
    std::vector<WImage>& mEconomyTrendBars = mEconomyPage.TrendBars;
    std::vector<WImage>& mEconomyTrendSecondaryBars = mEconomyPage.TrendSecondaryBars;
    std::vector<WImage>& mEconomyTrendTertiaryBars = mEconomyPage.TrendTertiaryBars;
    std::vector<WText>& mEconomyTrendXAxisLabels = mEconomyPage.TrendXAxisLabels;
    std::vector<WText>& mEconomyTrendYAxisLabels = mEconomyPage.TrendYAxisLabels;
    WImage& mEconomyChangeFrame = mEconomyPage.ChangeFrame;
    WImage& mEconomyChangeYAxisLine = mEconomyPage.ChangeYAxisLine;
    WImage& mEconomyChangeXAxisLine = mEconomyPage.ChangeXAxisLine;
    WImage& mEconomyChangeYAxisArrow = mEconomyPage.ChangeYAxisArrow;
    WImage& mEconomyChangeXAxisArrow = mEconomyPage.ChangeXAxisArrow;
    std::vector<WImage>& mEconomyChangeGridLines = mEconomyPage.ChangeGridLines;
    std::vector<WImage>& mEconomyChangePositiveBars = mEconomyPage.ChangePositiveBars;
    std::vector<WImage>& mEconomyChangeNegativeBars = mEconomyPage.ChangeNegativeBars;
    std::vector<WText>& mEconomyChangeYAxisLabels = mEconomyPage.ChangeYAxisLabels;
    WImage& mEconomyBreakdownTitleBackground = mEconomyPage.BreakdownTitleBackground;
    WText& mEconomyBreakdownTitle = mEconomyPage.BreakdownTitle;
    std::vector<FDetailRowWidgets>& mEconomyBreakdownRows = mEconomyPage.BreakdownRows;

    WImage& mResourceListTitleBackground = mResourcePage.ListTitleBackground;
    WText& mResourceListTitle = mResourcePage.ListTitle;
    WImage& mResourceFilterBackground = mResourcePage.FilterBackground;
    WText& mResourceFilterText = mResourcePage.FilterText;
    WImage& mResourceFilterLeftIcon = mResourcePage.FilterLeftIcon;
    WImage& mResourceFilterSortIcon = mResourcePage.FilterSortIcon;
    WImage& mResourceFilterSortArrow = mResourcePage.FilterSortArrow;
    std::vector<FDetailRowWidgets>& mResourceRows = mResourcePage.Rows;
    WImage& mResourceProductionTitleBackground = mResourcePage.ProductionTitleBackground;
    WText& mResourceProductionTitle = mResourcePage.ProductionTitle;
    WImage& mResourceProductionFrame = mResourcePage.ProductionFrame;
    WImage& mResourceProductionYAxisLine = mResourcePage.ProductionYAxisLine;
    WImage& mResourceProductionXAxisLine = mResourcePage.ProductionXAxisLine;
    WImage& mResourceProductionYAxisArrow = mResourcePage.ProductionYAxisArrow;
    WImage& mResourceProductionXAxisArrow = mResourcePage.ProductionXAxisArrow;
    std::vector<WImage>& mResourceProductionGridLines = mResourcePage.ProductionGridLines;
    std::vector<WImage>& mResourceProductionBars = mResourcePage.ProductionBars;
    std::vector<WText>& mResourceProductionXAxisLabels = mResourcePage.ProductionXAxisLabels;
    std::vector<WText>& mResourceProductionYAxisLabels = mResourcePage.ProductionYAxisLabels;
    WImage& mResourceProductionLegendPrimarySwatch = mResourcePage.ProductionLegendPrimarySwatch;
    WText& mResourceProductionLegendPrimaryText = mResourcePage.ProductionLegendPrimaryText;
    WImage& mResourceProductionLegendSecondarySwatch = mResourcePage.ProductionLegendSecondarySwatch;
    WText& mResourceProductionLegendSecondaryText = mResourcePage.ProductionLegendSecondaryText;
    WImage& mResourceDistributionTitleBackground = mResourcePage.DistributionTitleBackground;
    WText& mResourceDistributionTitle = mResourcePage.DistributionTitle;
    WImage& mResourceDistributionFilterBackground = mResourcePage.DistributionFilterBackground;
    WText& mResourceDistributionFilterText = mResourcePage.DistributionFilterText;
    std::vector<FMetricRowWidgets>& mResourceDistributionRows = mResourcePage.DistributionRows;
    WImage& mResourceTrackingTitleBackground = mResourcePage.TrackingTitleBackground;
    WText& mResourceTrackingTitle = mResourcePage.TrackingTitle;
    WText& mResourceTrackingName = mResourcePage.TrackingName;
    WText& mResourceTrackingValue = mResourcePage.TrackingValue;
    std::vector<FDetailRowWidgets>& mResourceDetails = mResourcePage.Details;
    WText& mResourceNotice = mResourcePage.Notice;

    WImage& mPoliticsListTitleBackground = mPoliticsPage.ListTitleBackground;
    WText& mPoliticsListTitle = mPoliticsPage.ListTitle;
    std::vector<FPoliticsFactionTileWidgets>& mPoliticsFactionTiles = mPoliticsPage.FactionTiles;
    std::vector<WText>& mPoliticsNeutralTexts = mPoliticsPage.NeutralTexts;
    WImage& mPoliticsSupportTitleBackground = mPoliticsPage.SupportTitleBackground;
    WText& mPoliticsSupportTitle = mPoliticsPage.SupportTitle;
    std::vector<FDetailRowWidgets>& mPoliticsSupportRows = mPoliticsPage.SupportRows;
    WImage& mPoliticsElectionLeftArrow = mPoliticsPage.ElectionLeftArrow;
    WImage& mPoliticsElectionRightArrow = mPoliticsPage.ElectionRightArrow;
    WText& mPoliticsElectionText = mPoliticsPage.ElectionText;
    WText& mPoliticsFactionTitle = mPoliticsPage.FactionTitle;
    WText& mPoliticsFactionApprovalLabel = mPoliticsPage.FactionApprovalLabel;
    WText& mPoliticsFactionApprovalValue = mPoliticsPage.FactionApprovalValue;
    std::vector<FDetailRowWidgets>& mPoliticsDetails = mPoliticsPage.Details;

    std::vector<FSatisfactionRowWidgets>& mForeignRows = mForeignPage.Rows;
    WImage& mForeignTitleBackground = mForeignPage.TitleBackground;
    WText& mForeignTitle = mForeignPage.Title;
    WText& mForeignStatusLabel = mForeignPage.StatusLabel;
    WText& mForeignStatusValue = mForeignPage.StatusValue;
    std::vector<FDetailRowWidgets>& mForeignDetails = mForeignPage.Details;
    std::vector<FMetricRowWidgets>& mForeignMetrics = mForeignPage.Metrics;
    WText& mForeignNotice = mForeignPage.Notice;

    std::vector<FDetailRowWidgets>& mBuildingRows = mBuildingPage.Rows;
    WText& mBuildingCategoryTitle = mBuildingPage.CategoryTitle;
    std::vector<FDetailRowWidgets>& mBuildingDetails = mBuildingPage.Details;

    WImage& mConflictHeadlineBackground = mConflictPage.HeadlineBackground;
    WText& mConflictHeadlineText = mConflictPage.HeadlineText;
    std::vector<FDetailRowWidgets>& mConflictDetails = mConflictPage.Details;
    std::vector<FMetricRowWidgets>& mConflictMetrics = mConflictPage.Metrics;

    bool& mOpen = mState.Open;
    EAlmanacPage& mSelectedPage = mState.SelectedPage;
    int& mSelectedSatisfactionIndex = mState.SelectedSatisfactionIndex;
    int& mSelectedPopulationIndex = mState.SelectedPopulationIndex;
    int& mSelectedEconomyIndex = mState.SelectedEconomyIndex;
    int& mSelectedResourceIndex = mState.SelectedResourceIndex;
    int& mVisibleResourceRowOffset = mState.VisibleResourceRowOffset;
    int& mSelectedPoliticsFactionIndex = mState.SelectedPoliticsFactionIndex;
    int& mSelectedForeignPowerIndex = mState.SelectedForeignPowerIndex;
    int& mSelectedBuildingCategoryIndex = mState.SelectedBuildingCategoryIndex;
    float& mPanelWidth = mState.PanelWidth;
    float& mPanelHeight = mState.PanelHeight;
    float& mDataRefreshAccum = mState.DataRefreshAccum;
    bool& mLayoutDirty = mState.LayoutDirty;
    int& mLastResolutionWidth = mState.LastResolutionWidth;
    int& mLastResolutionHeight = mState.LastResolutionHeight;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    void ToggleOpen();
    void SetOpen(bool Open);
    void SelectSatisfactionRow(int Index);
    void SelectPopulationRow(int Index);
    void SelectEconomyRow(int Index);
    void SelectResourceRow(int Index);
    void SelectPoliticsFaction(int Index);
    void SelectForeignPower(int Index);
    void SelectBuildingCategory(int Index);
    bool IsOpen() const
    {
        return mOpen;
    }

private:
    void RefreshData();
    void SelectPage(EAlmanacPage Page);

private:
    void OnCloseButtonClick();
    void OnOverviewTabClick();
    void OnSatisfactionTabClick();
    void OnPopulationTabClick();
    void OnEconomyTabClick();
    void OnResourcesTabClick();
    void OnPoliticsTabClick();
    void OnForeignTabClick();
    void OnBuildingsTabClick();
    void OnConflictTabClick();
    void OnPoliticsDemandAcceptClick();
    void OnPoliticsDemandRejectClick();
    void OnForeignDemandAcceptClick();
    void OnForeignDemandRejectClick();
};
