#pragma once

#include "UI/WidgetContainer.h"
#include "../Building/BuildingTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <array>
#include <vector>

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

    struct FDetailRowWidgets
    {
        std::weak_ptr<class CImage>     Background;
        std::weak_ptr<class CTextBlock> Label;
        std::weak_ptr<class CTextBlock> Value;
    };

private:
    std::weak_ptr<class CImage>     mPanelBackground;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CButton>    mCloseButton;
    std::vector<std::weak_ptr<class CButton>> mTabButtons;
    std::array<std::weak_ptr<CWidgetContainer>,
        static_cast<size_t>(EAlmanacPage::Count)> mPages;

    std::vector<FCardWidgets>      mOverviewCards;
    std::weak_ptr<class CTextBlock> mOverviewSummaryLeft;
    std::weak_ptr<class CTextBlock> mOverviewSummaryRight;

    std::vector<FMetricRowWidgets> mSatisfactionRows;
    std::vector<FDetailRowWidgets> mSatisfactionDetails;

    std::vector<FDetailRowWidgets> mPopulationDetails;
    std::vector<FMetricRowWidgets> mPopulationMetrics;

    std::vector<FDetailRowWidgets> mEconomyDetails;
    std::vector<FMetricRowWidgets> mEconomyMetrics;

    std::vector<FMetricRowWidgets> mResourceRows;
    std::vector<FDetailRowWidgets> mResourceDetails;
    std::weak_ptr<class CTextBlock> mResourceNotice;

    std::vector<FMetricRowWidgets> mPoliticsRows;
    std::vector<FDetailRowWidgets> mPoliticsDetails;

    std::vector<FDetailRowWidgets> mForeignDetails;
    std::vector<FMetricRowWidgets> mForeignMetrics;
    std::weak_ptr<class CTextBlock> mForeignNotice;

    std::vector<FMetricRowWidgets> mBuildingRows;
    std::vector<FDetailRowWidgets> mBuildingDetails;

    std::weak_ptr<class CImage>     mConflictHeadlineBackground;
    std::weak_ptr<class CTextBlock> mConflictHeadlineText;
    std::vector<FDetailRowWidgets>  mConflictDetails;
    std::vector<FMetricRowWidgets>  mConflictMetrics;

    bool          mOpen = false;
    EAlmanacPage  mSelectedPage = EAlmanacPage::Overview;
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
    bool IsOpen() const
    {
        return mOpen;
    }

private:
    void RefreshLayout();
    void RefreshData();
    void ApplyOpenState();
    void ApplySelectedPage();
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
