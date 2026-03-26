#pragma once

#include "CitizenInfoConstants.h"
#include "CitizenInfoState.h"
#include "UI/WidgetContainer.h"

struct FNpcSatisfaction;
struct FNpcPoliticalProfile;
struct FCitizenIdentityProfile;
struct FBuildingCatalogEntry;
namespace CitizenInfoDataProvider
{
    struct FCitizenInfoSnapshot;
}

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
    using EPanelMode = ECitizenInfoPanelMode;
    using ECitizenInfoTab = CitizenInfoConstants::ECitizenInfoTab;
    using EBuildingInfoTab = CitizenInfoConstants::EBuildingInfoTab;
    static constexpr int GCitizenTabCount = CitizenInfoConstants::GCitizenTabCount;
    static constexpr int GBuildingTabCount = CitizenInfoConstants::GBuildingTabCount;
    static constexpr int GTabButtonCount = CitizenInfoConstants::GTabButtonCount;
    static constexpr int GBudgetLevelCount = 12;
    static constexpr int GBudgetDisplayCount = 5;
    static constexpr int GOverviewResidentSlotCount = 16;
    static constexpr int GOverviewVisitorSlotCount = 12;
    static constexpr int GCitizenProfileSlotCount = 12;
    static constexpr int GOverviewMetricRowCount = 30;
    static constexpr int GCitizenMetricRowCount = GOverviewMetricRowCount;
    static constexpr int GHarborCargoStartIndex = 7; // rows 8+ (1-indexed) → HarborCargoLabel/Value
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
        std::array<WText, GTabButtonCount> SubtitleTexts;
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
        WButton OverviewWorkModeButton;
        WText OverviewWorkModeText;
        WText ResidentialOverviewWorkModeLabel;
        WImage ResidentialOverviewWorkModeBackground;
        WButton ResidentialOverviewWorkModeButton;
        WText ResidentialOverviewWorkModeText;
        WText OverviewBudgetLabel;
        WText OverviewBudgetValue;
        WText OverviewOccupancyLabel;
        WText OverviewOccupancyValue;
        std::array<WImage, GOverviewResidentSlotCount> OverviewResidentIcons;
        std::array<WImage, GOverviewVisitorSlotCount> OverviewVisitorIcons;
        std::array<WImage, GOverviewMetricRowCount> OverviewMetricIcons;
        std::array<WText, GOverviewMetricRowCount> OverviewMetricLabels;
        std::array<WImage, GOverviewMetricRowCount> OverviewMetricValueBgs;
        std::array<WText, GOverviewMetricRowCount> OverviewMetricValues;
        WText ResidentialOverviewBudgetLabel;
        WText ResidentialOverviewBudgetValue;
        WText ResidentialOverviewOccupancyLabel;
        WText ResidentialOverviewOccupancyValue;
        std::array<WImage, GOverviewResidentSlotCount>
            ResidentialOverviewResidentIcons;
        std::array<WText, GOverviewMetricRowCount>
            ResidentialOverviewMetricLabels;
        std::array<WText, GOverviewMetricRowCount>
            ResidentialOverviewMetricValues;
        std::array<WText, GOverviewMetricRowCount> StatsMetricLabels;
        std::array<WText, GOverviewMetricRowCount> StatsMetricValues;
        std::array<WText, GOverviewMetricRowCount> EfficiencyMetricLabels;
        std::array<WText, GOverviewMetricRowCount> EfficiencyMetricValues;
        WText StatsBodyText;
        WText UpgradeBodyText;
        WText EfficiencyBodyText;
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
        std::array<WText, GCitizenTabCount> CitizenSubtitleTexts;
        std::array<WText, GCitizenMetricRowCount> CitizenMetricLabels;
        std::array<WText, GCitizenMetricRowCount> CitizenMetricValues;
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
        std::array<WImage, GCitizenProfileSlotCount> ProfileIcons;
        WImage ThoughtTitleBackground;
        WText ThoughtTitleText;
        std::array<WText, GCitizenThoughtCount> ThoughtTexts;
        std::array<WImage, GCitizenThoughtDividerCount> ThoughtDividers;
        std::array<WButton, GCitizenActionButtonCount> ActionButtons;
        std::array<WText, GCitizenActionButtonCount> ActionButtonTexts;
        std::array<WImage, GCitizenActionButtonCount> ActionButtonIcons;
        std::array<WImage, GCitizenActionButtonCount> ActionButtonLabelBgs;
        WText FooterText;
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
        std::array<WText, GTabButtonCount>& mSubtitleTexts;
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
        WButton& mOverviewWorkModeButton;
        WText& mOverviewWorkModeText;
        WText& mResidentialOverviewWorkModeLabel;
        WImage& mResidentialOverviewWorkModeBackground;
        WButton& mResidentialOverviewWorkModeButton;
        WText& mResidentialOverviewWorkModeText;
        WText& mOverviewBudgetLabel;
        WText& mOverviewBudgetValue;
        WText& mOverviewOccupancyLabel;
        WText& mOverviewOccupancyValue;
        std::array<WImage, GOverviewResidentSlotCount>& mOverviewResidentIcons;
        std::array<WImage, GOverviewVisitorSlotCount>& mOverviewVisitorIcons;
        std::array<WImage, GOverviewMetricRowCount>& mOverviewMetricIcons;
        std::array<WText, GOverviewMetricRowCount>& mOverviewMetricLabels;
        std::array<WImage, GOverviewMetricRowCount>& mOverviewMetricValueBgs;
        std::array<WText, GOverviewMetricRowCount>& mOverviewMetricValues;
        WText& mResidentialOverviewBudgetLabel;
        WText& mResidentialOverviewBudgetValue;
        WText& mResidentialOverviewOccupancyLabel;
        WText& mResidentialOverviewOccupancyValue;
        std::array<WImage, GOverviewResidentSlotCount>&
            mResidentialOverviewResidentIcons;
        std::array<WText, GOverviewMetricRowCount>&
            mResidentialOverviewMetricLabels;
        std::array<WText, GOverviewMetricRowCount>&
            mResidentialOverviewMetricValues;
        std::array<WText, GOverviewMetricRowCount>& mStatsMetricLabels;
        std::array<WText, GOverviewMetricRowCount>& mStatsMetricValues;
        std::array<WText, GOverviewMetricRowCount>& mEfficiencyMetricLabels;
        std::array<WText, GOverviewMetricRowCount>& mEfficiencyMetricValues;
        WText& mStatsBodyText;
        WText& mUpgradeBodyText;
        WText& mEfficiencyBodyText;
        WImage& mUpgradeCardBackground;
        WImage& mUpgradeCardIcon;
        WText& mUpgradeCardTitle;
        WText& mUpgradeDescriptionText;
        WText& mInformationAccentText;
        WText& mInformationTopText;
        WText& mInformationBottomText;
        std::array<WText, GCitizenTabCount>& mCitizenSubtitleTexts;
        std::array<WText, GCitizenMetricRowCount>& mCitizenMetricLabels;
        std::array<WText, GCitizenMetricRowCount>& mCitizenMetricValues;
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
        std::array<WImage, GCitizenProfileSlotCount>& mCitizenProfileIcons;
        WImage& mCitizenThoughtTitleBackground;
        WText& mCitizenThoughtTitleText;
        std::array<WText, GCitizenThoughtCount>& mCitizenThoughtTexts;
        std::array<WImage, GCitizenThoughtDividerCount>& mCitizenThoughtDividers;
        std::array<WButton, GCitizenActionButtonCount>& mCitizenActionButtons;
        std::array<WText, GCitizenActionButtonCount>& mCitizenActionButtonTexts;
        std::array<WImage, GCitizenActionButtonCount>& mCitizenActionButtonIcons;
        std::array<WImage, GCitizenActionButtonCount>& mCitizenActionButtonLabelBgs;
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

        void OnBudgetButtonClick(int Index)
        {
            Owner.OnBudgetButtonClick(Index);
        }
    };

private:
    // Primary ownership stays grouped by mode/concern.
    FPanelChromeWidgets mChrome;
    FBuildingPanelWidgets mBuildingPanel;
    FCitizenPanelWidgets mCitizenPanel;
    FCitizenInfoState mState;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
    const FCitizenInfoState& GetState() const
    {
        return mState;
    }
    FCitizenInfoState& GetMutableState()
    {
        return mState;
    }

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
    bool HasOverviewMetricScroll() const;
    int GetOverviewMetricScrollOffset() const;
    int GetOverviewMetricScrollVisibleLineCount() const;
    int GetOverviewMetricScrollTotalLineCount() const;
    int GetOverviewMetricScrollFirstRowIndex() const;
    bool IsMouseOverOpenPanel(const FVector2& MousePos) const;

private:
    bool SelectCitizenTab(ECitizenInfoTab Tab);
    bool SelectBuildingTab(EBuildingInfoTab Tab);
    bool IsMouseOverOverviewMetricArea(const FVector2& MousePos) const;
    bool MoveOverviewMetricScroll(int DeltaLines);
    void CloseOperationModeSelection();
    bool HasTrackedBuildingOperationModes() const;
    bool TryOpenOperationModeSelection();
    bool CycleOperationModeSelectionPage();
    void SyncOverviewMetricScrollState(
        const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot);
    void SyncOperationModeSelectionState(
        const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot);
    bool IsTrackedCustomsOffice() const;
    bool TrySelectOperationMode(int VisibleModeIndex);
    bool OpenTradeWidget();
    void SetBuildingBudgetLevel(int Level);

public:
    void OnCloseButtonClick();
    void OnDemolishButtonClick();
    void OnMoveButtonClick();
    void OnFocusButtonClick();
    void OnOverviewCommandButtonClick();
    void OnBudgetButtonClick(int Index);
};
