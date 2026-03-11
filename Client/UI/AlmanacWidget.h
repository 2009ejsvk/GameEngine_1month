#pragma once

#include "UI/WidgetContainer.h"
#include "../Building/BuildingTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <array>
#include <vector>

class FAlmanacRenderer;

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
    std::weak_ptr<class CImage>     mPanelBackground;
    std::weak_ptr<class CImage>     mContentFrame;
    std::weak_ptr<class CImage>     mTitleRibbon;
    std::weak_ptr<class CImage>     mTabMarker;
    std::weak_ptr<class CImage>     mLeftRailTrack;
    std::weak_ptr<class CImage>     mLeftRailThumb;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CButton>    mCloseButton;
    std::vector<std::weak_ptr<class CButton>> mTabButtons;
    std::array<std::weak_ptr<CWidgetContainer>,
        static_cast<size_t>(EAlmanacPage::Count)> mPages;

    std::vector<FCardWidgets>      mOverviewCards;
    std::vector<std::weak_ptr<class CTextBlock>> mOverviewSectionTitles;
    std::weak_ptr<class CImage>     mOverviewElectionLeftArrow;
    std::weak_ptr<class CImage>     mOverviewElectionRightArrow;
    std::weak_ptr<class CTextBlock> mOverviewElectionText;
    std::weak_ptr<class CTextBlock> mOverviewSummaryLeft;
    std::weak_ptr<class CTextBlock> mOverviewSummaryRight;

    std::vector<FSatisfactionRowWidgets> mSatisfactionRows;
    std::weak_ptr<class CImage>     mSatisfactionListTitleBackground;
    std::weak_ptr<class CTextBlock> mSatisfactionListTitle;
    std::weak_ptr<class CImage>     mSatisfactionChartTitleBackground;
    std::weak_ptr<class CImage>     mSatisfactionChartFrame;
    std::weak_ptr<class CImage>     mSatisfactionChartYAxisLine;
    std::weak_ptr<class CImage>     mSatisfactionChartXAxisLine;
    std::weak_ptr<class CImage>     mSatisfactionChartYAxisArrow;
    std::weak_ptr<class CImage>     mSatisfactionChartXAxisArrow;
    std::weak_ptr<class CTextBlock> mSatisfactionChartTitle;
    std::weak_ptr<class CImage>     mSatisfactionTooltipPanel;
    std::weak_ptr<class CTextBlock> mSatisfactionTooltipText;
    std::vector<std::weak_ptr<class CImage>> mSatisfactionChartGridLines;
    std::vector<std::weak_ptr<class CImage>> mSatisfactionChartPrimaryLines;
    std::vector<std::weak_ptr<class CImage>> mSatisfactionChartSecondaryLines;
    std::vector<std::weak_ptr<class CTextBlock>> mSatisfactionChartXAxisLabels;
    std::vector<std::weak_ptr<class CTextBlock>> mSatisfactionChartYAxisLabels;
    std::vector<FDetailRowWidgets> mSatisfactionDetails;

    std::vector<FDetailRowWidgets> mPopulationDetails;
    std::vector<FMetricRowWidgets> mPopulationMetrics;
    std::weak_ptr<class CImage>     mPopulationTrendTitleBackground;
    std::weak_ptr<class CImage>     mPopulationTrendFrame;
    std::weak_ptr<class CImage>     mPopulationTrendYAxisLine;
    std::weak_ptr<class CImage>     mPopulationTrendXAxisLine;
    std::weak_ptr<class CImage>     mPopulationTrendYAxisArrow;
    std::weak_ptr<class CImage>     mPopulationTrendXAxisArrow;
    std::weak_ptr<class CTextBlock> mPopulationTrendTitle;
    std::vector<std::weak_ptr<class CImage>> mPopulationTrendGridLines;
    std::vector<std::weak_ptr<class CImage>> mPopulationTrendLines;
    std::vector<std::weak_ptr<class CImage>> mPopulationTrendChildBars;
    std::vector<std::weak_ptr<class CImage>> mPopulationTrendAdultBars;
    std::vector<std::weak_ptr<class CImage>> mPopulationTrendRetiredBars;
    std::vector<std::weak_ptr<class CImage>> mPopulationTrendRichBars;
    std::vector<std::weak_ptr<class CImage>> mPopulationTrendFilthyRichBars;
    std::vector<std::weak_ptr<class CTextBlock>> mPopulationTrendXAxisLabels;
    std::vector<std::weak_ptr<class CTextBlock>> mPopulationTrendYAxisLabels;
    std::weak_ptr<class CImage>     mPopulationChangeTitleBackground;
    std::weak_ptr<class CImage>     mPopulationChangeFrame;
    std::weak_ptr<class CImage>     mPopulationChangeYAxisLine;
    std::weak_ptr<class CImage>     mPopulationChangeXAxisLine;
    std::weak_ptr<class CImage>     mPopulationChangeYAxisArrow;
    std::weak_ptr<class CImage>     mPopulationChangeXAxisArrow;
    std::weak_ptr<class CTextBlock> mPopulationChangeTitle;
    std::vector<std::weak_ptr<class CImage>> mPopulationChangeGridLines;
    std::vector<std::weak_ptr<class CImage>> mPopulationChangePositiveBars;
    std::vector<std::weak_ptr<class CImage>> mPopulationChangeNegativeBars;
    std::vector<std::weak_ptr<class CTextBlock>> mPopulationChangeXAxisLabels;
    std::vector<std::weak_ptr<class CTextBlock>> mPopulationChangeYAxisLabels;

    std::vector<FDetailRowWidgets> mEconomyDetails;
    std::vector<FMetricRowWidgets> mEconomyMetrics;
    std::weak_ptr<class CImage>     mEconomyTrendTitleBackground;
    std::weak_ptr<class CImage>     mEconomyTrendFrame;
    std::weak_ptr<class CImage>     mEconomyTrendYAxisLine;
    std::weak_ptr<class CImage>     mEconomyTrendXAxisLine;
    std::weak_ptr<class CImage>     mEconomyTrendYAxisArrow;
    std::weak_ptr<class CImage>     mEconomyTrendXAxisArrow;
    std::weak_ptr<class CTextBlock> mEconomyTrendTitle;
    std::vector<std::weak_ptr<class CImage>> mEconomyTrendGridLines;
    std::vector<std::weak_ptr<class CImage>> mEconomyTrendLines;
    std::vector<std::weak_ptr<class CImage>> mEconomyTrendBars;
    std::vector<std::weak_ptr<class CImage>> mEconomyTrendSecondaryBars;
    std::vector<std::weak_ptr<class CImage>> mEconomyTrendTertiaryBars;
    std::vector<std::weak_ptr<class CTextBlock>> mEconomyTrendXAxisLabels;
    std::vector<std::weak_ptr<class CTextBlock>> mEconomyTrendYAxisLabels;
    std::weak_ptr<class CImage>     mEconomyChangeFrame;
    std::weak_ptr<class CImage>     mEconomyChangeYAxisLine;
    std::weak_ptr<class CImage>     mEconomyChangeXAxisLine;
    std::weak_ptr<class CImage>     mEconomyChangeYAxisArrow;
    std::weak_ptr<class CImage>     mEconomyChangeXAxisArrow;
    std::vector<std::weak_ptr<class CImage>> mEconomyChangeGridLines;
    std::vector<std::weak_ptr<class CImage>> mEconomyChangePositiveBars;
    std::vector<std::weak_ptr<class CImage>> mEconomyChangeNegativeBars;
    std::vector<std::weak_ptr<class CTextBlock>> mEconomyChangeYAxisLabels;
    std::weak_ptr<class CImage>     mEconomyBreakdownTitleBackground;
    std::weak_ptr<class CTextBlock> mEconomyBreakdownTitle;
    std::vector<FDetailRowWidgets>  mEconomyBreakdownRows;

    std::weak_ptr<class CImage>     mResourceListTitleBackground;
    std::weak_ptr<class CTextBlock> mResourceListTitle;
    std::weak_ptr<class CImage>     mResourceFilterBackground;
    std::weak_ptr<class CTextBlock> mResourceFilterText;
    std::weak_ptr<class CImage>     mResourceFilterLeftIcon;
    std::weak_ptr<class CImage>     mResourceFilterSortIcon;
    std::weak_ptr<class CImage>     mResourceFilterSortArrow;
    std::vector<FDetailRowWidgets>  mResourceRows;
    std::weak_ptr<class CImage>     mResourceProductionTitleBackground;
    std::weak_ptr<class CTextBlock> mResourceProductionTitle;
    std::weak_ptr<class CImage>     mResourceProductionFrame;
    std::weak_ptr<class CImage>     mResourceProductionYAxisLine;
    std::weak_ptr<class CImage>     mResourceProductionXAxisLine;
    std::weak_ptr<class CImage>     mResourceProductionYAxisArrow;
    std::weak_ptr<class CImage>     mResourceProductionXAxisArrow;
    std::vector<std::weak_ptr<class CImage>> mResourceProductionGridLines;
    std::vector<std::weak_ptr<class CImage>> mResourceProductionBars;
    std::vector<std::weak_ptr<class CTextBlock>> mResourceProductionXAxisLabels;
    std::vector<std::weak_ptr<class CTextBlock>> mResourceProductionYAxisLabels;
    std::weak_ptr<class CImage>     mResourceProductionLegendPrimarySwatch;
    std::weak_ptr<class CTextBlock> mResourceProductionLegendPrimaryText;
    std::weak_ptr<class CImage>     mResourceProductionLegendSecondarySwatch;
    std::weak_ptr<class CTextBlock> mResourceProductionLegendSecondaryText;
    std::weak_ptr<class CImage>     mResourceDistributionTitleBackground;
    std::weak_ptr<class CTextBlock> mResourceDistributionTitle;
    std::weak_ptr<class CImage>     mResourceDistributionFilterBackground;
    std::weak_ptr<class CTextBlock> mResourceDistributionFilterText;
    std::vector<FMetricRowWidgets>  mResourceDistributionRows;
    std::weak_ptr<class CImage>     mResourceTrackingTitleBackground;
    std::weak_ptr<class CTextBlock> mResourceTrackingTitle;
    std::weak_ptr<class CTextBlock> mResourceTrackingName;
    std::weak_ptr<class CTextBlock> mResourceTrackingValue;
    std::vector<FDetailRowWidgets>  mResourceDetails;
    std::weak_ptr<class CTextBlock> mResourceNotice;

    std::weak_ptr<class CImage>     mPoliticsListTitleBackground;
    std::weak_ptr<class CTextBlock> mPoliticsListTitle;
    std::vector<FPoliticsFactionTileWidgets> mPoliticsFactionTiles;
    std::vector<std::weak_ptr<class CTextBlock>> mPoliticsNeutralTexts;
    std::weak_ptr<class CImage>     mPoliticsSupportTitleBackground;
    std::weak_ptr<class CTextBlock> mPoliticsSupportTitle;
    std::vector<FDetailRowWidgets>  mPoliticsSupportRows;
    std::weak_ptr<class CImage>     mPoliticsElectionLeftArrow;
    std::weak_ptr<class CImage>     mPoliticsElectionRightArrow;
    std::weak_ptr<class CTextBlock> mPoliticsElectionText;
    std::weak_ptr<class CTextBlock> mPoliticsFactionTitle;
    std::weak_ptr<class CTextBlock> mPoliticsFactionApprovalLabel;
    std::weak_ptr<class CTextBlock> mPoliticsFactionApprovalValue;
    std::vector<FDetailRowWidgets>  mPoliticsDetails;

    std::vector<FSatisfactionRowWidgets> mForeignRows;
    std::weak_ptr<class CImage>     mForeignTitleBackground;
    std::weak_ptr<class CTextBlock> mForeignTitle;
    std::weak_ptr<class CTextBlock> mForeignStatusLabel;
    std::weak_ptr<class CTextBlock> mForeignStatusValue;
    std::vector<FDetailRowWidgets>  mForeignDetails;
    std::vector<FMetricRowWidgets>  mForeignMetrics;
    std::weak_ptr<class CTextBlock> mForeignNotice;

    std::vector<FDetailRowWidgets> mBuildingRows;
    std::weak_ptr<class CTextBlock> mBuildingCategoryTitle;
    std::vector<FDetailRowWidgets> mBuildingDetails;

    std::weak_ptr<class CImage>     mConflictHeadlineBackground;
    std::weak_ptr<class CTextBlock> mConflictHeadlineText;
    std::vector<FDetailRowWidgets>  mConflictDetails;
    std::vector<FMetricRowWidgets>  mConflictMetrics;

    bool          mOpen = false;
    EAlmanacPage  mSelectedPage = EAlmanacPage::Overview;
    int           mSelectedSatisfactionIndex = 0;
    int           mSelectedPopulationIndex = 0;
    int           mSelectedEconomyIndex = 0;
    int           mSelectedResourceIndex = 0;
    int           mSelectedPoliticsFactionIndex = 0;
    int           mSelectedForeignPowerIndex = 0;
    int           mSelectedBuildingCategoryIndex = 0;
    float         mPanelWidth = 1060.f;
    float         mPanelHeight = 744.f;
    float         mDataRefreshAccum = 0.f;
    bool          mLayoutDirty = true;
    int           mLastResolutionWidth = 0;
    int           mLastResolutionHeight = 0;

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
};
