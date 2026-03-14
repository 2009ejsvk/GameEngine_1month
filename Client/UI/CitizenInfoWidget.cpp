#include "CitizenInfoWidget.h"
#include "CitizenInfoDataProvider.h"
#include "CitizenInfoInteractionService.h"
#include "CitizenInfoQueryService.h"
#include "CitizenInfoRenderer.h"

CCitizenInfoWidget::CCitizenInfoWidget()
{
}

CCitizenInfoWidget::~CCitizenInfoWidget()
{
}

void CCitizenInfoWidget::ResetCitizenModeState()
{
    mSelectedCitizenTab = ECitizenInfoTab::Overview;
    mTrackedCitizenName.clear();
    mCitizenPoliticsSatisfactionFillRatios.fill(0.f);
    mCitizenPoliticsSupportRatio = 0.f;
}

void CCitizenInfoWidget::ResetBuildingModeState()
{
    mSelectedBuildingTab = EBuildingInfoTab::Overview;
    mTrackedBuildingName.clear();
    mCustomsModeSelectionOpen = false;
}

bool CCitizenInfoWidget::Init()
{
    CWidgetContainer::Init();

    mPanelMode = EPanelMode::Citizen;
    ResetCitizenModeState();
    ResetBuildingModeState();
    mPanelWidth = 360.f;
    mPanelHeight = 720.f;
    mPanelTop = 56.f;
    mRequestedScreenPos = FVector2(0.f, 0.f);

    FCitizenInfoRenderer::CreateWidgets(*this);
    FCitizenInfoRenderer::RefreshLayout(*this);
    SetEnable(false);
    return true;
}

void CCitizenInfoWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
    (void)DeltaTime;

    if (!GetEnable())
        return;

    RefreshFromState();
}

void CCitizenInfoWidget::Render()
{
    CWidgetContainer::Render();
}

void CCitizenInfoWidget::OpenCitizen(
    const std::string& CitizenName,
    const FNpcSatisfaction& Satisfaction,
    const FVector2& ScreenPos)
{
    (void)Satisfaction;

    mRequestedScreenPos = ScreenPos;
    mPanelMode = EPanelMode::Citizen;
    ResetCitizenModeState();
    ResetBuildingModeState();
    mTrackedCitizenName = CitizenName;

    SetEnable(true);
    RefreshFromState();
}

void CCitizenInfoWidget::OpenBuilding(
    const std::string& BuildingObjectName,
    const std::string& BuildingDisplayName,
    const std::string& CategoryName,
    bool IsResidential,
    int Capacity,
    const FVector2& ScreenPos)
{
    (void)BuildingDisplayName;
    (void)CategoryName;
    (void)IsResidential;
    (void)Capacity;

    mRequestedScreenPos = ScreenPos;
    mPanelMode = EPanelMode::Building;
    ResetCitizenModeState();
    ResetBuildingModeState();
    mTrackedBuildingName = BuildingObjectName;

    SetEnable(true);
    RefreshFromState();
}

CCitizenInfoWidget::FRendererView CCitizenInfoWidget::GetRendererView()
{
    return
    {
        *this,
        mPanelMode,
        mSelectedCitizenTab,
        mSelectedBuildingTab,
        mPanelImage,
        mInnerFrame,
        mTitleRibbon,
        mSectionRibbon,
        mScrollTrack,
        mScrollThumb,
        mTitleIcon,
        mTitleText,
        mSubtitleText,
        mSectionDivider,
        mPageTitleText,
        mBodyText,
        mBudgetText,
        mCloseButton,
        mTabButtons,
        mTabButtonTexts,
        mTabButtonIcons,
        mDemolishButton,
        mMoveButton,
        mFocusButton,
        mOverviewCommandButton,
        mOverviewCommandButtonText,
        mBudgetButtons,
        mBudgetButtonTexts,
        mOverviewWorkModeLabel,
        mOverviewWorkModeBackground,
        mOverviewWorkModeText,
        mOverviewBudgetLabel,
        mOverviewBudgetValue,
        mOverviewOccupancyLabel,
        mOverviewOccupancyValue,
        mOverviewResidentIcons,
        mOverviewVisitorIcons,
        mOverviewMetricLabels,
        mOverviewMetricValues,
        mUpgradeCardBackground,
        mUpgradeCardIcon,
        mUpgradeCardTitle,
        mUpgradeDescriptionText,
        mInformationAccentText,
        mInformationTopText,
        mInformationBottomText,
        mCitizenPoliticsSectionBackgrounds,
        mCitizenPoliticsSectionTitles,
        mCitizenPoliticsSatisfactionLabels,
        mCitizenPoliticsSatisfactionRails,
        mCitizenPoliticsSatisfactionFills,
        mCitizenPoliticsOpinionTexts,
        mCitizenPoliticsSupportIcons,
        mCitizenPoliticsSupportRail,
        mCitizenPoliticsSupportThumb,
        mCitizenThoughtTitleBackground,
        mCitizenThoughtTitleText,
        mCitizenThoughtTexts,
        mCitizenThoughtDividers,
        mCitizenActionButtons,
        mCitizenActionButtonTexts,
        mCitizenActionButtonIcons,
        mCitizenFooterText,
        mPanelWidth,
        mPanelHeight,
        mPanelTop,
        mCitizenPoliticsSatisfactionFillRatios,
        mCitizenPoliticsSupportRatio,
        mRequestedScreenPos,
        mWorld
    };
}

void CCitizenInfoWidget::RefreshFromState()
{
    CitizenInfoDataProvider::FCitizenInfoSnapshot Snapshot;
    const int SelectedTabIndex = GetSelectedTabIndexForCurrentMode();
    const auto QuerySource =
        CitizenInfoQueryService::CreateWorldQuerySource(mWorld.lock());

    if (mPanelMode == EPanelMode::Citizen)
    {
        Snapshot = CitizenInfoDataProvider::BuildTrackedCitizenSnapshot(
            QuerySource,
            mTrackedCitizenName,
            SelectedTabIndex);
    }
    else
    {
        Snapshot = CitizenInfoDataProvider::BuildTrackedBuildingSnapshot(
            QuerySource,
            mTrackedBuildingName,
            SelectedTabIndex,
            mCustomsModeSelectionOpen);
    }

    if (!Snapshot.Valid)
    {
        ResetCitizenModeState();
        ResetBuildingModeState();
        SetEnable(false);
        return;
    }

    SetEnable(true);
    FCitizenInfoRenderer::ApplySnapshot(*this, Snapshot);
    FCitizenInfoRenderer::RefreshLayout(*this);
}

int CCitizenInfoWidget::GetSelectedTabIndexForCurrentMode() const
{
    if (mPanelMode == EPanelMode::Citizen)
        return static_cast<int>(mSelectedCitizenTab);

    return static_cast<int>(mSelectedBuildingTab);
}

bool CCitizenInfoWidget::SelectCurrentModeTab(int TabIndex)
{
    if (mPanelMode == EPanelMode::Citizen)
    {
        if (TabIndex < 0 || TabIndex >= GCitizenTabCount)
            return false;

        return SelectCitizenTab(static_cast<ECitizenInfoTab>(TabIndex));
    }

    if (TabIndex < 0 || TabIndex >= GBuildingTabCount)
        return false;

    return SelectBuildingTab(static_cast<EBuildingInfoTab>(TabIndex));
}

bool CCitizenInfoWidget::SelectCitizenTab(ECitizenInfoTab Tab)
{
    if (mSelectedCitizenTab == Tab)
        return false;

    mCustomsModeSelectionOpen = false;
    mSelectedCitizenTab = Tab;
    return true;
}

bool CCitizenInfoWidget::SelectBuildingTab(EBuildingInfoTab Tab)
{
    if (mSelectedBuildingTab == Tab)
    {
        if (Tab == EBuildingInfoTab::Overview &&
            IsTrackedCustomsOffice())
        {
            mCustomsModeSelectionOpen = !mCustomsModeSelectionOpen;
            return true;
        }

        return false;
    }

    mCustomsModeSelectionOpen = false;
    mSelectedBuildingTab = Tab;
    return true;
}

bool CCitizenInfoWidget::IsTrackedCustomsOffice() const
{
    if (mTrackedBuildingName.empty())
        return false;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());
    return InteractionSource &&
        InteractionSource->IsCustomsOfficeBuilding(mTrackedBuildingName);
}

bool CCitizenInfoWidget::TrySelectCustomsOperationMode(int ModeIndex)
{
    if (!mCustomsModeSelectionOpen ||
        mSelectedBuildingTab != EBuildingInfoTab::Overview ||
        !IsTrackedCustomsOffice())
    {
        return false;
    }

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource)
        return false;

    std::wstring FeedbackMessage;

    if (!InteractionSource->SetBuildingOperationMode(
            mTrackedBuildingName,
            ModeIndex,
            FeedbackMessage))
    {
        return false;
    }

    RefreshFromState();
    return true;
}

bool CCitizenInfoWidget::OpenTradeWidget()
{
    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource || !InteractionSource->OpenTradeWidget())
        return false;

    mCustomsModeSelectionOpen = false;
    SetEnable(false);
    return true;
}

void CCitizenInfoWidget::SetBuildingBudgetLevel(int Level)
{
    if (mTrackedBuildingName.empty())
        return;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource ||
        !InteractionSource->SetBuildingBudgetLevel(mTrackedBuildingName, Level))
    {
        return;
    }

    RefreshFromState();
}

void CCitizenInfoWidget::OnCloseButtonClick()
{
    ResetCitizenModeState();
    ResetBuildingModeState();
    SetEnable(false);
}

void CCitizenInfoWidget::OnDemolishButtonClick()
{
    if (mTrackedBuildingName.empty())
        return;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource ||
        !InteractionSource->DemolishBuilding(mTrackedBuildingName))
    {
        return;
    }

    ResetBuildingModeState();
    SetEnable(false);
}

void CCitizenInfoWidget::OnMoveButtonClick()
{
    if (mTrackedBuildingName.empty())
        return;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource ||
        !InteractionSource->BeginMoveBuilding(mTrackedBuildingName))
    {
        return;
    }

    SetEnable(false);
}

void CCitizenInfoWidget::OnFocusButtonClick()
{
    if (mTrackedBuildingName.empty())
        return;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource)
        return;

    InteractionSource->FocusBuilding(mTrackedBuildingName);
}

void CCitizenInfoWidget::OnOverviewCommandButtonClick()
{
    if (mTrackedBuildingName.empty())
        return;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource)
        return;

    if (InteractionSource->IsCustomsOfficeBuilding(mTrackedBuildingName) &&
        mSelectedBuildingTab == EBuildingInfoTab::Overview)
    {
        if (mCustomsModeSelectionOpen)
        {
            mCustomsModeSelectionOpen = false;
            RefreshFromState();
            return;
        }

        OpenTradeWidget();
        return;
    }

    std::wstring FeedbackMessage;

    if (!InteractionSource->ExecuteBuildingCommand(
            mTrackedBuildingName,
            mSelectedBuildingTab,
            FeedbackMessage))
    {
        return;
    }

    RefreshFromState();
}

void CCitizenInfoWidget::OnBudgetLevel1Click()
{
    if (TrySelectCustomsOperationMode(0))
        return;

    SetBuildingBudgetLevel(1);
}

void CCitizenInfoWidget::OnBudgetLevel2Click()
{
    if (TrySelectCustomsOperationMode(1))
        return;

    SetBuildingBudgetLevel(2);
}

void CCitizenInfoWidget::OnBudgetLevel3Click()
{
    if (TrySelectCustomsOperationMode(2))
        return;

    SetBuildingBudgetLevel(3);
}

void CCitizenInfoWidget::OnBudgetLevel4Click()
{
    if (TrySelectCustomsOperationMode(3))
        return;

    SetBuildingBudgetLevel(4);
}

void CCitizenInfoWidget::OnBudgetLevel5Click()
{
    if (TrySelectCustomsOperationMode(4))
        return;

    SetBuildingBudgetLevel(5);
}
