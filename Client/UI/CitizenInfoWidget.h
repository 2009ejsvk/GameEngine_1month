#pragma once

#include "UI/WidgetContainer.h"
#include <array>
#include <string>

struct FNpcSatisfaction;
struct FNpcPoliticalProfile;
struct FCitizenIdentityProfile;
struct FBuildingCatalogEntry;

class FCitizenInfoRenderer;

class CCitizenInfoWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;
    friend class FCitizenInfoRenderer;

protected:
    CCitizenInfoWidget();

public:
    virtual ~CCitizenInfoWidget();

private:
    enum class EPanelMode
    {
        Citizen,
        Building
    };

    enum class EBuildingInfoTab
    {
        Overview = 0,
        Statistics,
        Upgrades,
        Efficiency,
        Information,
        Count
    };

    static constexpr int GBuildingTabCount =
        static_cast<int>(EBuildingInfoTab::Count);
    static constexpr int GBudgetLevelCount = 5;
    static constexpr int GOverviewResidentSlotCount = 16;
    static constexpr int GOverviewVisitorSlotCount = 12;
    static constexpr int GOverviewMetricRowCount = 14;
    static constexpr int GCitizenActionButtonCount = 6;
    static constexpr int GCitizenPoliticsSectionCount = 3;
    static constexpr int GCitizenPoliticsSatisfactionCount = 9;
    static constexpr int GCitizenPoliticsOpinionCount = 3;
    static constexpr int GCitizenPoliticsSupportIconCount = 3;
    static constexpr int GCitizenThoughtCount = 5;
    static constexpr int GCitizenThoughtDividerCount = 4;

    EPanelMode mPanelMode = EPanelMode::Citizen;
    EBuildingInfoTab mSelectedBuildingTab = EBuildingInfoTab::Overview;

    std::weak_ptr<class CImage> mPanelImage;
    std::weak_ptr<class CImage> mInnerFrame;
    std::weak_ptr<class CImage> mTitleRibbon;
    std::weak_ptr<class CImage> mSectionRibbon;
    std::weak_ptr<class CImage> mScrollTrack;
    std::weak_ptr<class CImage> mScrollThumb;
    std::weak_ptr<class CImage> mTitleIcon;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CTextBlock> mSubtitleText;
    std::weak_ptr<class CImage> mSectionDivider;
    std::weak_ptr<class CTextBlock> mPageTitleText;
    std::weak_ptr<class CTextBlock> mBodyText;
    std::weak_ptr<class CTextBlock> mBudgetText;
    std::weak_ptr<class CButton> mCloseButton;
    std::weak_ptr<class CButton> mDemolishButton;
    std::weak_ptr<class CButton> mMoveButton;
    std::weak_ptr<class CButton> mCloneButton;
    std::weak_ptr<class CButton> mOverviewCommandButton;
    std::weak_ptr<class CTextBlock> mOverviewCommandButtonText;
    std::array<std::weak_ptr<class CButton>, GBuildingTabCount> mTabButtons;
    std::array<std::weak_ptr<class CTextBlock>, GBuildingTabCount>
        mTabButtonTexts;
    std::array<std::weak_ptr<class CImage>, GBuildingTabCount>
        mTabButtonIcons;
    std::array<std::weak_ptr<class CButton>, GBudgetLevelCount> mBudgetButtons;
    std::array<std::weak_ptr<class CTextBlock>, GBudgetLevelCount>
        mBudgetButtonTexts;
    std::weak_ptr<class CTextBlock> mOverviewWorkModeLabel;
    std::weak_ptr<class CImage> mOverviewWorkModeBackground;
    std::weak_ptr<class CTextBlock> mOverviewWorkModeText;
    std::weak_ptr<class CTextBlock> mOverviewBudgetLabel;
    std::weak_ptr<class CTextBlock> mOverviewBudgetValue;
    std::weak_ptr<class CTextBlock> mOverviewOccupancyLabel;
    std::weak_ptr<class CTextBlock> mOverviewOccupancyValue;
    std::array<std::weak_ptr<class CImage>, GOverviewResidentSlotCount>
        mOverviewResidentIcons;
    std::array<std::weak_ptr<class CImage>, GOverviewVisitorSlotCount>
        mOverviewVisitorIcons;
    std::array<std::weak_ptr<class CTextBlock>, GOverviewMetricRowCount>
        mOverviewMetricLabels;
    std::array<std::weak_ptr<class CTextBlock>, GOverviewMetricRowCount>
        mOverviewMetricValues;
    std::weak_ptr<class CImage> mUpgradeCardBackground;
    std::weak_ptr<class CImage> mUpgradeCardIcon;
    std::weak_ptr<class CTextBlock> mUpgradeCardTitle;
    std::weak_ptr<class CTextBlock> mUpgradeDescriptionText;
    std::weak_ptr<class CTextBlock> mInformationAccentText;
    std::weak_ptr<class CTextBlock> mInformationTopText;
    std::weak_ptr<class CTextBlock> mInformationBottomText;
    std::array<std::weak_ptr<class CImage>, GCitizenPoliticsSectionCount>
        mCitizenPoliticsSectionBackgrounds;
    std::array<std::weak_ptr<class CTextBlock>, GCitizenPoliticsSectionCount>
        mCitizenPoliticsSectionTitles;
    std::array<std::weak_ptr<class CTextBlock>, GCitizenPoliticsSatisfactionCount>
        mCitizenPoliticsSatisfactionLabels;
    std::array<std::weak_ptr<class CImage>, GCitizenPoliticsSatisfactionCount>
        mCitizenPoliticsSatisfactionRails;
    std::array<std::weak_ptr<class CImage>, GCitizenPoliticsSatisfactionCount>
        mCitizenPoliticsSatisfactionFills;
    std::array<std::weak_ptr<class CTextBlock>, GCitizenPoliticsOpinionCount>
        mCitizenPoliticsOpinionTexts;
    std::array<std::weak_ptr<class CImage>, GCitizenPoliticsSupportIconCount>
        mCitizenPoliticsSupportIcons;
    std::weak_ptr<class CImage> mCitizenPoliticsSupportRail;
    std::weak_ptr<class CImage> mCitizenPoliticsSupportThumb;
    std::weak_ptr<class CImage> mCitizenThoughtTitleBackground;
    std::weak_ptr<class CTextBlock> mCitizenThoughtTitleText;
    std::array<std::weak_ptr<class CTextBlock>, GCitizenThoughtCount>
        mCitizenThoughtTexts;
    std::array<std::weak_ptr<class CImage>, GCitizenThoughtDividerCount>
        mCitizenThoughtDividers;
    std::array<std::weak_ptr<class CButton>, GCitizenActionButtonCount>
        mCitizenActionButtons;
    std::array<std::weak_ptr<class CTextBlock>, GCitizenActionButtonCount>
        mCitizenActionButtonTexts;
    std::array<std::weak_ptr<class CImage>, GCitizenActionButtonCount>
        mCitizenActionButtonIcons;
    std::weak_ptr<class CTextBlock> mCitizenFooterText;
    std::string mTrackedCitizenName;
    std::string mTrackedBuildingName;
    float mPanelWidth = 360.f;
    float mPanelHeight = 720.f;
    float mPanelTop = 56.f;
    std::array<float, GCitizenPoliticsSatisfactionCount>
        mCitizenPoliticsSatisfactionFillRatios = {};
    float mCitizenPoliticsSupportRatio = 0.f;
    FVector2 mRequestedScreenPos = FVector2(0.f, 0.f);

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();

public:
    void OpenCitizen(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FVector2& ScreenPos);
    void OpenBuilding(
        const std::string& BuildingObjectName,
        const std::string& BuildingDisplayName,
        const std::string& CategoryName,
        bool IsResidential,
        int Capacity,
        const FVector2& ScreenPos);

private:
    void RefreshFromState();
    void SelectBuildingTab(EBuildingInfoTab Tab);
    void SetBuildingBudgetLevel(int Level);
    void OnCloseButtonClick();
    void OnDemolishButtonClick();
    void OnMoveButtonClick();
    void OnCloneButtonClick();
    void OnOverviewCommandButtonClick();
    void OnBudgetLevel1Click();
    void OnBudgetLevel2Click();
    void OnBudgetLevel3Click();
    void OnBudgetLevel4Click();
    void OnBudgetLevel5Click();
};
