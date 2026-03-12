#pragma once

#include "CitizenInfoConstants.h"
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

    using ECitizenInfoTab = CitizenInfoConstants::ECitizenInfoTab;
    using EBuildingInfoTab = CitizenInfoConstants::EBuildingInfoTab;
    static constexpr int GCitizenTabCount = CitizenInfoConstants::GCitizenTabCount;
    static constexpr int GBuildingTabCount = CitizenInfoConstants::GBuildingTabCount;
    static constexpr int GTabButtonCount = CitizenInfoConstants::GTabButtonCount;
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

    using WImage = std::weak_ptr<class CImage>;
    using WText = std::weak_ptr<class CTextBlock>;
    using WButton = std::weak_ptr<class CButton>;

    struct FPanelChromeWidgets
    {
        WImage PanelImage;
        WImage InnerFrame;
        WImage TitleRibbon;
        WImage SectionRibbon;
        WImage ScrollTrack;
        WImage ScrollThumb;
        WImage TitleIcon;
        WText TitleText;
        WText SubtitleText;
        WImage SectionDivider;
        WText PageTitleText;
        WText BodyText;
        WText BudgetText;
        WButton CloseButton;
        std::array<WButton, GTabButtonCount> TabButtons;
        std::array<WText, GTabButtonCount> TabButtonTexts;
        std::array<WImage, GTabButtonCount> TabButtonIcons;
    };

    struct FBuildingPanelWidgets
    {
        WButton DemolishButton;
        WButton MoveButton;
        WButton CloneButton;
        WButton OverviewCommandButton;
        WText OverviewCommandButtonText;
        std::array<WButton, GBudgetLevelCount> BudgetButtons;
        std::array<WText, GBudgetLevelCount> BudgetButtonTexts;
        WText OverviewWorkModeLabel;
        WImage OverviewWorkModeBackground;
        WText OverviewWorkModeText;
        WText OverviewBudgetLabel;
        WText OverviewBudgetValue;
        WText OverviewOccupancyLabel;
        WText OverviewOccupancyValue;
        std::array<WImage, GOverviewResidentSlotCount> OverviewResidentIcons;
        std::array<WImage, GOverviewVisitorSlotCount> OverviewVisitorIcons;
        std::array<WText, GOverviewMetricRowCount> OverviewMetricLabels;
        std::array<WText, GOverviewMetricRowCount> OverviewMetricValues;
        WImage UpgradeCardBackground;
        WImage UpgradeCardIcon;
        WText UpgradeCardTitle;
        WText UpgradeDescriptionText;
        WText InformationAccentText;
        WText InformationTopText;
        WText InformationBottomText;
    };

    struct FCitizenPanelWidgets
    {
        std::array<WImage, GCitizenPoliticsSectionCount>
            PoliticsSectionBackgrounds;
        std::array<WText, GCitizenPoliticsSectionCount>
            PoliticsSectionTitles;
        std::array<WText, GCitizenPoliticsSatisfactionCount>
            PoliticsSatisfactionLabels;
        std::array<WImage, GCitizenPoliticsSatisfactionCount>
            PoliticsSatisfactionRails;
        std::array<WImage, GCitizenPoliticsSatisfactionCount>
            PoliticsSatisfactionFills;
        std::array<WText, GCitizenPoliticsOpinionCount>
            PoliticsOpinionTexts;
        std::array<WImage, GCitizenPoliticsSupportIconCount>
            PoliticsSupportIcons;
        WImage PoliticsSupportRail;
        WImage PoliticsSupportThumb;
        WImage ThoughtTitleBackground;
        WText ThoughtTitleText;
        std::array<WText, GCitizenThoughtCount> ThoughtTexts;
        std::array<WImage, GCitizenThoughtDividerCount> ThoughtDividers;
        std::array<WButton, GCitizenActionButtonCount> ActionButtons;
        std::array<WText, GCitizenActionButtonCount> ActionButtonTexts;
        std::array<WImage, GCitizenActionButtonCount> ActionButtonIcons;
        WText FooterText;
    };

    struct FPanelState
    {
        EPanelMode PanelMode = EPanelMode::Citizen;
        ECitizenInfoTab SelectedCitizenTab = ECitizenInfoTab::Overview;
        EBuildingInfoTab SelectedBuildingTab = EBuildingInfoTab::Overview;
        std::string TrackedCitizenName;
        std::string TrackedBuildingName;
        float PanelWidth = 360.f;
        float PanelHeight = 720.f;
        float PanelTop = 56.f;
        std::array<float, GCitizenPoliticsSatisfactionCount>
            CitizenPoliticsSatisfactionFillRatios = {};
        float CitizenPoliticsSupportRatio = 0.f;
        FVector2 RequestedScreenPos = FVector2(0.f, 0.f);
    };

    // Primary ownership is grouped by mode/concern; aliases below keep
    // existing renderer code working while callers migrate gradually.
    FPanelChromeWidgets mChrome;
    FBuildingPanelWidgets mBuildingPanel;
    FCitizenPanelWidgets mCitizenPanel;
    FPanelState mState;

    // Compatibility aliases for existing renderer/data-provider code.
    EPanelMode& mPanelMode = mState.PanelMode;
    ECitizenInfoTab& mSelectedCitizenTab = mState.SelectedCitizenTab;
    EBuildingInfoTab& mSelectedBuildingTab = mState.SelectedBuildingTab;

    WImage& mPanelImage = mChrome.PanelImage;
    WImage& mInnerFrame = mChrome.InnerFrame;
    WImage& mTitleRibbon = mChrome.TitleRibbon;
    WImage& mSectionRibbon = mChrome.SectionRibbon;
    WImage& mScrollTrack = mChrome.ScrollTrack;
    WImage& mScrollThumb = mChrome.ScrollThumb;
    WImage& mTitleIcon = mChrome.TitleIcon;
    WText& mTitleText = mChrome.TitleText;
    WText& mSubtitleText = mChrome.SubtitleText;
    WImage& mSectionDivider = mChrome.SectionDivider;
    WText& mPageTitleText = mChrome.PageTitleText;
    WText& mBodyText = mChrome.BodyText;
    WText& mBudgetText = mChrome.BudgetText;
    WButton& mCloseButton = mChrome.CloseButton;
    std::array<WButton, GTabButtonCount>& mTabButtons = mChrome.TabButtons;
    std::array<WText, GTabButtonCount>& mTabButtonTexts = mChrome.TabButtonTexts;
    std::array<WImage, GTabButtonCount>& mTabButtonIcons = mChrome.TabButtonIcons;

    WButton& mDemolishButton = mBuildingPanel.DemolishButton;
    WButton& mMoveButton = mBuildingPanel.MoveButton;
    WButton& mCloneButton = mBuildingPanel.CloneButton;
    WButton& mOverviewCommandButton = mBuildingPanel.OverviewCommandButton;
    WText& mOverviewCommandButtonText = mBuildingPanel.OverviewCommandButtonText;
    std::array<WButton, GBudgetLevelCount>& mBudgetButtons = mBuildingPanel.BudgetButtons;
    std::array<WText, GBudgetLevelCount>& mBudgetButtonTexts = mBuildingPanel.BudgetButtonTexts;
    WText& mOverviewWorkModeLabel = mBuildingPanel.OverviewWorkModeLabel;
    WImage& mOverviewWorkModeBackground = mBuildingPanel.OverviewWorkModeBackground;
    WText& mOverviewWorkModeText = mBuildingPanel.OverviewWorkModeText;
    WText& mOverviewBudgetLabel = mBuildingPanel.OverviewBudgetLabel;
    WText& mOverviewBudgetValue = mBuildingPanel.OverviewBudgetValue;
    WText& mOverviewOccupancyLabel = mBuildingPanel.OverviewOccupancyLabel;
    WText& mOverviewOccupancyValue = mBuildingPanel.OverviewOccupancyValue;
    std::array<WImage, GOverviewResidentSlotCount>& mOverviewResidentIcons =
        mBuildingPanel.OverviewResidentIcons;
    std::array<WImage, GOverviewVisitorSlotCount>& mOverviewVisitorIcons =
        mBuildingPanel.OverviewVisitorIcons;
    std::array<WText, GOverviewMetricRowCount>& mOverviewMetricLabels =
        mBuildingPanel.OverviewMetricLabels;
    std::array<WText, GOverviewMetricRowCount>& mOverviewMetricValues =
        mBuildingPanel.OverviewMetricValues;
    WImage& mUpgradeCardBackground = mBuildingPanel.UpgradeCardBackground;
    WImage& mUpgradeCardIcon = mBuildingPanel.UpgradeCardIcon;
    WText& mUpgradeCardTitle = mBuildingPanel.UpgradeCardTitle;
    WText& mUpgradeDescriptionText = mBuildingPanel.UpgradeDescriptionText;
    WText& mInformationAccentText = mBuildingPanel.InformationAccentText;
    WText& mInformationTopText = mBuildingPanel.InformationTopText;
    WText& mInformationBottomText = mBuildingPanel.InformationBottomText;

    std::array<WImage, GCitizenPoliticsSectionCount>& mCitizenPoliticsSectionBackgrounds =
        mCitizenPanel.PoliticsSectionBackgrounds;
    std::array<WText, GCitizenPoliticsSectionCount>& mCitizenPoliticsSectionTitles =
        mCitizenPanel.PoliticsSectionTitles;
    std::array<WText, GCitizenPoliticsSatisfactionCount>& mCitizenPoliticsSatisfactionLabels =
        mCitizenPanel.PoliticsSatisfactionLabels;
    std::array<WImage, GCitizenPoliticsSatisfactionCount>& mCitizenPoliticsSatisfactionRails =
        mCitizenPanel.PoliticsSatisfactionRails;
    std::array<WImage, GCitizenPoliticsSatisfactionCount>& mCitizenPoliticsSatisfactionFills =
        mCitizenPanel.PoliticsSatisfactionFills;
    std::array<WText, GCitizenPoliticsOpinionCount>& mCitizenPoliticsOpinionTexts =
        mCitizenPanel.PoliticsOpinionTexts;
    std::array<WImage, GCitizenPoliticsSupportIconCount>& mCitizenPoliticsSupportIcons =
        mCitizenPanel.PoliticsSupportIcons;
    WImage& mCitizenPoliticsSupportRail = mCitizenPanel.PoliticsSupportRail;
    WImage& mCitizenPoliticsSupportThumb = mCitizenPanel.PoliticsSupportThumb;
    WImage& mCitizenThoughtTitleBackground = mCitizenPanel.ThoughtTitleBackground;
    WText& mCitizenThoughtTitleText = mCitizenPanel.ThoughtTitleText;
    std::array<WText, GCitizenThoughtCount>& mCitizenThoughtTexts =
        mCitizenPanel.ThoughtTexts;
    std::array<WImage, GCitizenThoughtDividerCount>& mCitizenThoughtDividers =
        mCitizenPanel.ThoughtDividers;
    std::array<WButton, GCitizenActionButtonCount>& mCitizenActionButtons =
        mCitizenPanel.ActionButtons;
    std::array<WText, GCitizenActionButtonCount>& mCitizenActionButtonTexts =
        mCitizenPanel.ActionButtonTexts;
    std::array<WImage, GCitizenActionButtonCount>& mCitizenActionButtonIcons =
        mCitizenPanel.ActionButtonIcons;
    WText& mCitizenFooterText = mCitizenPanel.FooterText;

    std::string& mTrackedCitizenName = mState.TrackedCitizenName;
    std::string& mTrackedBuildingName = mState.TrackedBuildingName;
    float& mPanelWidth = mState.PanelWidth;
    float& mPanelHeight = mState.PanelHeight;
    float& mPanelTop = mState.PanelTop;
    std::array<float, GCitizenPoliticsSatisfactionCount>&
        mCitizenPoliticsSatisfactionFillRatios =
            mState.CitizenPoliticsSatisfactionFillRatios;
    float& mCitizenPoliticsSupportRatio = mState.CitizenPoliticsSupportRatio;
    FVector2& mRequestedScreenPos = mState.RequestedScreenPos;

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
    bool mCustomsModeSelectionOpen = false;

private:
    void RefreshFromState();
    int GetSelectedTabIndexForCurrentMode() const;
    bool SelectCurrentModeTab(int TabIndex);
    bool SelectCitizenTab(ECitizenInfoTab Tab);
    bool SelectBuildingTab(EBuildingInfoTab Tab);
    bool IsTrackedCustomsOffice() const;
    bool TrySelectCustomsOperationMode(int ModeIndex);
    bool OpenTradeWidget();
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
