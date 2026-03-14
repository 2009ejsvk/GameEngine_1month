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

protected:
    CCitizenInfoWidget();

public:
    virtual ~CCitizenInfoWidget();

public:
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
        WButton FocusButton;
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

    struct FCitizenModeState
    {
        ECitizenInfoTab SelectedTab = ECitizenInfoTab::Overview;
        std::string TrackedCitizenName;
        std::array<float, GCitizenPoliticsSatisfactionCount>
            PoliticsSatisfactionFillRatios = {};
        float PoliticsSupportRatio = 0.f;
    };

    struct FBuildingModeState
    {
        EBuildingInfoTab SelectedTab = EBuildingInfoTab::Overview;
        std::string TrackedBuildingName;
        bool CustomsModeSelectionOpen = false;
    };

    struct FPanelState
    {
        EPanelMode PanelMode = EPanelMode::Citizen;
        FCitizenModeState Citizen;
        FBuildingModeState Building;
        float PanelWidth = 360.f;
        float PanelHeight = 720.f;
        float PanelTop = 56.f;
        FVector2 RequestedScreenPos = FVector2(0.f, 0.f);
    };

    struct FRendererView
    {
        CCitizenInfoWidget& Owner;
        EPanelMode& mPanelMode;
        ECitizenInfoTab& mSelectedCitizenTab;
        EBuildingInfoTab& mSelectedBuildingTab;
        WImage& mPanelImage;
        WImage& mInnerFrame;
        WImage& mTitleRibbon;
        WImage& mSectionRibbon;
        WImage& mScrollTrack;
        WImage& mScrollThumb;
        WImage& mTitleIcon;
        WText& mTitleText;
        WText& mSubtitleText;
        WImage& mSectionDivider;
        WText& mPageTitleText;
        WText& mBodyText;
        WText& mBudgetText;
        WButton& mCloseButton;
        std::array<WButton, GTabButtonCount>& mTabButtons;
        std::array<WText, GTabButtonCount>& mTabButtonTexts;
        std::array<WImage, GTabButtonCount>& mTabButtonIcons;
        WButton& mDemolishButton;
        WButton& mMoveButton;
        WButton& mFocusButton;
        WButton& mOverviewCommandButton;
        WText& mOverviewCommandButtonText;
        std::array<WButton, GBudgetLevelCount>& mBudgetButtons;
        std::array<WText, GBudgetLevelCount>& mBudgetButtonTexts;
        WText& mOverviewWorkModeLabel;
        WImage& mOverviewWorkModeBackground;
        WText& mOverviewWorkModeText;
        WText& mOverviewBudgetLabel;
        WText& mOverviewBudgetValue;
        WText& mOverviewOccupancyLabel;
        WText& mOverviewOccupancyValue;
        std::array<WImage, GOverviewResidentSlotCount>& mOverviewResidentIcons;
        std::array<WImage, GOverviewVisitorSlotCount>& mOverviewVisitorIcons;
        std::array<WText, GOverviewMetricRowCount>& mOverviewMetricLabels;
        std::array<WText, GOverviewMetricRowCount>& mOverviewMetricValues;
        WImage& mUpgradeCardBackground;
        WImage& mUpgradeCardIcon;
        WText& mUpgradeCardTitle;
        WText& mUpgradeDescriptionText;
        WText& mInformationAccentText;
        WText& mInformationTopText;
        WText& mInformationBottomText;
        std::array<WImage, GCitizenPoliticsSectionCount>&
            mCitizenPoliticsSectionBackgrounds;
        std::array<WText, GCitizenPoliticsSectionCount>&
            mCitizenPoliticsSectionTitles;
        std::array<WText, GCitizenPoliticsSatisfactionCount>&
            mCitizenPoliticsSatisfactionLabels;
        std::array<WImage, GCitizenPoliticsSatisfactionCount>&
            mCitizenPoliticsSatisfactionRails;
        std::array<WImage, GCitizenPoliticsSatisfactionCount>&
            mCitizenPoliticsSatisfactionFills;
        std::array<WText, GCitizenPoliticsOpinionCount>&
            mCitizenPoliticsOpinionTexts;
        std::array<WImage, GCitizenPoliticsSupportIconCount>&
            mCitizenPoliticsSupportIcons;
        WImage& mCitizenPoliticsSupportRail;
        WImage& mCitizenPoliticsSupportThumb;
        WImage& mCitizenThoughtTitleBackground;
        WText& mCitizenThoughtTitleText;
        std::array<WText, GCitizenThoughtCount>& mCitizenThoughtTexts;
        std::array<WImage, GCitizenThoughtDividerCount>& mCitizenThoughtDividers;
        std::array<WButton, GCitizenActionButtonCount>& mCitizenActionButtons;
        std::array<WText, GCitizenActionButtonCount>& mCitizenActionButtonTexts;
        std::array<WImage, GCitizenActionButtonCount>& mCitizenActionButtonIcons;
        WText& mCitizenFooterText;
        float& mPanelWidth;
        float& mPanelHeight;
        float& mPanelTop;
        std::array<float, GCitizenPoliticsSatisfactionCount>&
            mCitizenPoliticsSatisfactionFillRatios;
        float& mCitizenPoliticsSupportRatio;
        FVector2& mRequestedScreenPos;
        std::weak_ptr<class CWorld>& mWorld;

        template <typename T>
        std::weak_ptr<T> CreateWidget(const std::string& Name, int ZOrder = 0)
        {
            return Owner.CreateWidget<T>(Name, ZOrder);
        }

        void SetPos(float X, float Y)
        {
            Owner.SetPos(X, Y);
        }

        void SetSize(float X, float Y)
        {
            Owner.SetSize(X, Y);
        }

        int GetSelectedTabIndexForCurrentMode() const
        {
            return Owner.GetSelectedTabIndexForCurrentMode();
        }

        bool SelectCurrentModeTab(int TabIndex)
        {
            return Owner.SelectCurrentModeTab(TabIndex);
        }

        void RefreshFromState()
        {
            Owner.RefreshFromState();
        }

        void OnCloseButtonClick()
        {
            Owner.OnCloseButtonClick();
        }

        void OnDemolishButtonClick()
        {
            Owner.OnDemolishButtonClick();
        }

        void OnMoveButtonClick()
        {
            Owner.OnMoveButtonClick();
        }

        void OnFocusButtonClick()
        {
            Owner.OnFocusButtonClick();
        }

        void OnOverviewCommandButtonClick()
        {
            Owner.OnOverviewCommandButtonClick();
        }

        void OnBudgetLevel1Click()
        {
            Owner.OnBudgetLevel1Click();
        }

        void OnBudgetLevel2Click()
        {
            Owner.OnBudgetLevel2Click();
        }

        void OnBudgetLevel3Click()
        {
            Owner.OnBudgetLevel3Click();
        }

        void OnBudgetLevel4Click()
        {
            Owner.OnBudgetLevel4Click();
        }

        void OnBudgetLevel5Click()
        {
            Owner.OnBudgetLevel5Click();
        }
    };

private:

    // Primary ownership is grouped by mode/concern; aliases below keep
    // existing renderer code working while callers migrate gradually.
    FPanelChromeWidgets mChrome;
    FBuildingPanelWidgets mBuildingPanel;
    FCitizenPanelWidgets mCitizenPanel;
    FPanelState mState;

    // Compatibility aliases for existing renderer/data-provider code.
    EPanelMode& mPanelMode = mState.PanelMode;
    ECitizenInfoTab& mSelectedCitizenTab = mState.Citizen.SelectedTab;
    EBuildingInfoTab& mSelectedBuildingTab = mState.Building.SelectedTab;

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
    WButton& mFocusButton = mBuildingPanel.FocusButton;
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

    std::string& mTrackedCitizenName = mState.Citizen.TrackedCitizenName;
    std::string& mTrackedBuildingName = mState.Building.TrackedBuildingName;
    float& mPanelWidth = mState.PanelWidth;
    float& mPanelHeight = mState.PanelHeight;
    float& mPanelTop = mState.PanelTop;
    std::array<float, GCitizenPoliticsSatisfactionCount>&
        mCitizenPoliticsSatisfactionFillRatios =
            mState.Citizen.PoliticsSatisfactionFillRatios;
    float& mCitizenPoliticsSupportRatio = mState.Citizen.PoliticsSupportRatio;
    FVector2& mRequestedScreenPos = mState.RequestedScreenPos;
    bool& mCustomsModeSelectionOpen = mState.Building.CustomsModeSelectionOpen;

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
    FRendererView GetRendererView();

private:
    void ResetCitizenModeState();
    void ResetBuildingModeState();

public:
    void RefreshFromState();
    int GetSelectedTabIndexForCurrentMode() const;
    bool SelectCurrentModeTab(int TabIndex);

private:
    bool SelectCitizenTab(ECitizenInfoTab Tab);
    bool SelectBuildingTab(EBuildingInfoTab Tab);
    bool IsTrackedCustomsOffice() const;
    bool TrySelectCustomsOperationMode(int ModeIndex);
    bool OpenTradeWidget();
    void SetBuildingBudgetLevel(int Level);

public:
    void OnCloseButtonClick();
    void OnDemolishButtonClick();
    void OnMoveButtonClick();
    void OnFocusButtonClick();
    void OnOverviewCommandButtonClick();
    void OnBudgetLevel1Click();
    void OnBudgetLevel2Click();
    void OnBudgetLevel3Click();
    void OnBudgetLevel4Click();
    void OnBudgetLevel5Click();
};
