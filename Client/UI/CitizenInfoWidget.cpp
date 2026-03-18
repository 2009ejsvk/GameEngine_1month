#include "CitizenInfoWidget.h"
#include "CitizenInfoDataProvider.h"
#include "CitizenInfoInteractionService.h"
#include "CitizenInfoQueryService.h"
#include "CitizenInfoRenderer.h"
#include "UI/TextBlock.h"
#include "World/World.h"
#include <algorithm>
#include <limits>

CCitizenInfoWidget::CCitizenInfoWidget()
{
}

CCitizenInfoWidget::~CCitizenInfoWidget()
{
}

void CCitizenInfoWidget::ResetCitizenModeState()
{
    auto& CitizenState = mState.Citizen;
    CitizenState.SelectedTab = ECitizenInfoTab::Overview;
    CitizenState.TrackedCitizenName.clear();
    CitizenState.PoliticsSatisfactionFillRatios.fill(0.f);
    CitizenState.PoliticsSupportRatio = 0.f;
}

void CCitizenInfoWidget::ResetBuildingModeState()
{
    auto& BuildingState = mState.Building;
    BuildingState.SelectedTab = EBuildingInfoTab::Overview;
    BuildingState.TrackedBuildingName.clear();
    BuildingState.CustomsModeSelectionOpen = false;
    BuildingState.OperationModeCount = 0;
    BuildingState.OperationModeSelectionPageIndex = -1;
    BuildingState.OperationModeSelectionPageCount = 0;
    BuildingState.OverviewMetricScrollActive = false;
    BuildingState.OverviewMetricScrollOffset = 0;
    BuildingState.OverviewMetricScrollVisibleLineCount = 0;
    BuildingState.OverviewMetricScrollTotalLineCount = 0;
    BuildingState.OverviewMetricScrollFirstRowIndex = 0;
}

bool CCitizenInfoWidget::Init()
{
    CWidgetContainer::Init();

    mState.PanelMode = EPanelMode::Citizen;
    ResetCitizenModeState();
    ResetBuildingModeState();
    mState.PanelWidth = 360.f;
    mState.PanelHeight = 720.f;
    mState.PanelTop = 56.f;
    mState.RequestedScreenPos = FVector2(0.f, 0.f);

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

    if (mState.PanelMode == EPanelMode::Building && HasOverviewMetricScroll())
    {
        auto World = mWorld.lock();

        if (World)
        {
            auto Input = World->GetInput().lock();

            if (Input)
            {
                const int WheelDelta = Input->GetMouseWheelDelta();

                if (WheelDelta != 0 &&
                    IsMouseOverOverviewMetricArea(Input->GetMousePos()))
                {
                    MoveOverviewMetricScroll(WheelDelta < 0 ? 1 : -1);
                }
            }
        }
    }

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

    mState.RequestedScreenPos = ScreenPos;
    mState.PanelMode = EPanelMode::Citizen;
    ResetCitizenModeState();
    ResetBuildingModeState();
    mState.Citizen.TrackedCitizenName = CitizenName;

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

    mState.RequestedScreenPos = ScreenPos;
    mState.PanelMode = EPanelMode::Building;
    ResetCitizenModeState();
    ResetBuildingModeState();
    mState.Building.TrackedBuildingName = BuildingObjectName;

    SetEnable(true);
    RefreshFromState();
}

CCitizenInfoWidget::FRendererView CCitizenInfoWidget::GetRendererView()
{
    auto& Chrome = mChrome;
    auto& Building = mBuildingPanel;
    auto& Citizen = mCitizenPanel;
    auto& State = mState;

    return
    {
        *this,
        State.PanelMode,
        State.Citizen.SelectedTab,
        State.Building.SelectedTab,
        Chrome.PanelImage,
        Chrome.InnerFrame,
        Chrome.TitleRibbon,
        Chrome.SectionRibbon,
        Chrome.ScrollTrack,
        Chrome.ScrollThumb,
        Chrome.TitleIcon,
        Chrome.TitleText,
        Chrome.SubtitleText,
        Chrome.SectionDivider,
        Chrome.PageTitleText,
        Chrome.BodyText,
        Chrome.BudgetText,
        Chrome.CloseButton,
        Chrome.TabButtons,
        Chrome.TabButtonTexts,
        Chrome.TabButtonIcons,
        Building.DemolishButton,
        Building.MoveButton,
        Building.FocusButton,
        Building.OverviewCommandButton,
        Building.OverviewCommandButtonText,
        Building.BudgetButtons,
        Building.BudgetButtonTexts,
        Building.OverviewWorkModeLabel,
        Building.OverviewWorkModeBackground,
        Building.OverviewWorkModeButton,
        Building.OverviewWorkModeText,
        Building.ResidentialOverviewWorkModeLabel,
        Building.ResidentialOverviewWorkModeBackground,
        Building.ResidentialOverviewWorkModeButton,
        Building.ResidentialOverviewWorkModeText,
        Building.OverviewBudgetLabel,
        Building.OverviewBudgetValue,
        Building.OverviewOccupancyLabel,
        Building.OverviewOccupancyValue,
        Building.OverviewResidentIcons,
        Building.OverviewVisitorIcons,
        Building.OverviewMetricLabels,
        Building.OverviewMetricValues,
        Building.ResidentialOverviewBudgetLabel,
        Building.ResidentialOverviewBudgetValue,
        Building.ResidentialOverviewOccupancyLabel,
        Building.ResidentialOverviewOccupancyValue,
        Building.ResidentialOverviewResidentIcons,
        Building.ResidentialOverviewMetricLabels,
        Building.ResidentialOverviewMetricValues,
        Building.UpgradeCardBackground,
        Building.UpgradeCardIcon,
        Building.UpgradeCardTitle,
        Building.UpgradeDescriptionText,
        Building.InformationAccentText,
        Building.InformationTopText,
        Building.InformationBottomText,
        Citizen.PoliticsSectionBackgrounds,
        Citizen.PoliticsSectionTitles,
        Citizen.PoliticsSatisfactionLabels,
        Citizen.PoliticsSatisfactionRails,
        Citizen.PoliticsSatisfactionFills,
        Citizen.PoliticsOpinionTexts,
        Citizen.PoliticsSupportIcons,
        Citizen.PoliticsSupportRail,
        Citizen.PoliticsSupportThumb,
        Citizen.ThoughtTitleBackground,
        Citizen.ThoughtTitleText,
        Citizen.ThoughtTexts,
        Citizen.ThoughtDividers,
        Citizen.ActionButtons,
        Citizen.ActionButtonTexts,
        Citizen.ActionButtonIcons,
        Citizen.FooterText,
        State.PanelWidth,
        State.PanelHeight,
        State.PanelTop,
        State.Citizen.PoliticsSatisfactionFillRatios,
        State.Citizen.PoliticsSupportRatio,
        State.RequestedScreenPos,
        mWorld
    };
}

void CCitizenInfoWidget::RefreshFromState()
{
    CitizenInfoDataProvider::FCitizenInfoSnapshot Snapshot;
    const int SelectedTabIndex = GetSelectedTabIndexForCurrentMode();
    const auto QuerySource =
        CitizenInfoQueryService::CreateWorldQuerySource(mWorld.lock());

    if (mState.PanelMode == EPanelMode::Citizen)
    {
        Snapshot = CitizenInfoDataProvider::BuildTrackedCitizenSnapshot(
            QuerySource,
            mState.Citizen.TrackedCitizenName,
            SelectedTabIndex);
    }
    else
    {
        Snapshot = CitizenInfoDataProvider::BuildTrackedBuildingSnapshot(
            QuerySource,
            mState.Building.TrackedBuildingName,
            SelectedTabIndex,
            mState.Building.CustomsModeSelectionOpen,
            mState.Building.OperationModeSelectionPageIndex,
            mState.Building.OverviewMetricScrollOffset);
    }

    if (!Snapshot.Valid)
    {
        ResetCitizenModeState();
        ResetBuildingModeState();
        SetEnable(false);
        return;
    }

    SetEnable(true);
    SyncOperationModeSelectionState(Snapshot);
    SyncOverviewMetricScrollState(Snapshot);
    FCitizenInfoRenderer::ApplySnapshot(*this, Snapshot);
    FCitizenInfoRenderer::RefreshLayout(*this);
}

int CCitizenInfoWidget::GetSelectedTabIndexForCurrentMode() const
{
    if (mState.PanelMode == EPanelMode::Citizen)
        return static_cast<int>(mState.Citizen.SelectedTab);

    return static_cast<int>(mState.Building.SelectedTab);
}

bool CCitizenInfoWidget::SelectCurrentModeTab(int TabIndex)
{
    if (mState.PanelMode == EPanelMode::Citizen)
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
    if (mState.Citizen.SelectedTab == Tab)
        return false;

    auto& BuildingState = mState.Building;
    CloseOperationModeSelection();
    BuildingState.OverviewMetricScrollActive = false;
    BuildingState.OverviewMetricScrollOffset = 0;
    BuildingState.OverviewMetricScrollVisibleLineCount = 0;
    BuildingState.OverviewMetricScrollTotalLineCount = 0;
    BuildingState.OverviewMetricScrollFirstRowIndex = 0;
    mState.Citizen.SelectedTab = Tab;
    return true;
}

bool CCitizenInfoWidget::SelectBuildingTab(EBuildingInfoTab Tab)
{
    auto& BuildingState = mState.Building;

    if (BuildingState.SelectedTab == Tab)
    {
        if (Tab == EBuildingInfoTab::Overview &&
            (IsTrackedCustomsOffice() ||
                HasTrackedBuildingOperationModes()))
        {
            BuildingState.OverviewMetricScrollOffset = 0;
            if (BuildingState.CustomsModeSelectionOpen)
                CloseOperationModeSelection();
            else
                TryOpenOperationModeSelection();
            return true;
        }

        return false;
    }

    CloseOperationModeSelection();
    BuildingState.OverviewMetricScrollActive = false;
    BuildingState.OverviewMetricScrollOffset = 0;
    BuildingState.OverviewMetricScrollVisibleLineCount = 0;
    BuildingState.OverviewMetricScrollTotalLineCount = 0;
    BuildingState.OverviewMetricScrollFirstRowIndex = 0;
    BuildingState.SelectedTab = Tab;
    return true;
}

bool CCitizenInfoWidget::HasOverviewMetricScroll() const
{
    return mState.Building.OverviewMetricScrollActive;
}

int CCitizenInfoWidget::GetOverviewMetricScrollOffset() const
{
    return mState.Building.OverviewMetricScrollOffset;
}

int CCitizenInfoWidget::GetOverviewMetricScrollVisibleLineCount() const
{
    return mState.Building.OverviewMetricScrollVisibleLineCount;
}

int CCitizenInfoWidget::GetOverviewMetricScrollTotalLineCount() const
{
    return mState.Building.OverviewMetricScrollTotalLineCount;
}

int CCitizenInfoWidget::GetOverviewMetricScrollFirstRowIndex() const
{
    return mState.Building.OverviewMetricScrollFirstRowIndex;
}

bool CCitizenInfoWidget::IsMouseOverOpenPanel(const FVector2& MousePos) const
{
    if (!GetEnable())
        return false;

    const FVector3 PanelPos = GetPos();
    const FVector3 PanelSize = GetSize();

    return MousePos.x >= PanelPos.x &&
        MousePos.x <= PanelPos.x + PanelSize.x &&
        MousePos.y >= PanelPos.y &&
        MousePos.y <= PanelPos.y + PanelSize.y;
}

bool CCitizenInfoWidget::IsMouseOverOverviewMetricArea(
    const FVector2& MousePos) const
{
    if (!HasOverviewMetricScroll())
        return false;

    float Left = (std::numeric_limits<float>::max)();
    float Top = (std::numeric_limits<float>::max)();
    float Right = 0.f;
    float Bottom = 0.f;
    bool HasVisibleBounds = false;

    for (int Index = mState.Building.OverviewMetricScrollFirstRowIndex;
        Index < GOverviewMetricRowCount;
        ++Index)
    {
        if (auto Label =
            mBuildingPanel.OverviewMetricLabels[static_cast<size_t>(Index)].lock())
        {
            if (Label->GetEnable())
            {
                const FVector3 Pos = Label->GetPos();
                const FVector3 Size = Label->GetSize();
                Left = (std::min)(Left, Pos.x);
                Top = (std::min)(Top, Pos.y);
                Right = (std::max)(Right, Pos.x + Size.x);
                Bottom = (std::max)(Bottom, Pos.y + Size.y);
                HasVisibleBounds = true;
            }
        }

        if (auto Value =
            mBuildingPanel.OverviewMetricValues[static_cast<size_t>(Index)].lock())
        {
            if (Value->GetEnable())
            {
                const FVector3 Pos = Value->GetPos();
                const FVector3 Size = Value->GetSize();
                Left = (std::min)(Left, Pos.x);
                Top = (std::min)(Top, Pos.y);
                Right = (std::max)(Right, Pos.x + Size.x);
                Bottom = (std::max)(Bottom, Pos.y + Size.y);
                HasVisibleBounds = true;
            }
        }
    }

    if (!HasVisibleBounds)
        return IsMouseOverOpenPanel(MousePos);

    const FVector3 PanelPos = GetPos();
    Left += PanelPos.x;
    Right += PanelPos.x;
    Top += PanelPos.y;
    Bottom += PanelPos.y;

    return MousePos.x >= Left &&
        MousePos.x <= Right &&
        MousePos.y >= Top &&
        MousePos.y <= Bottom;
}

bool CCitizenInfoWidget::MoveOverviewMetricScroll(int DeltaLines)
{
    if (DeltaLines == 0 || !HasOverviewMetricScroll())
        return false;

    auto& BuildingState = mState.Building;
    const int MaxOffset = (std::max)(
        0,
        BuildingState.OverviewMetricScrollTotalLineCount -
            BuildingState.OverviewMetricScrollVisibleLineCount);
    const int NewOffset = (std::max)(
        0,
        (std::min)(BuildingState.OverviewMetricScrollOffset + DeltaLines, MaxOffset));

    if (NewOffset == BuildingState.OverviewMetricScrollOffset)
        return false;

    BuildingState.OverviewMetricScrollOffset = NewOffset;
    return true;
}

void CCitizenInfoWidget::CloseOperationModeSelection()
{
    auto& BuildingState = mState.Building;
    BuildingState.CustomsModeSelectionOpen = false;
    BuildingState.OperationModeSelectionPageIndex = -1;
    BuildingState.OperationModeSelectionPageCount = 0;
}

bool CCitizenInfoWidget::HasTrackedBuildingOperationModes() const
{
    return mState.Building.OperationModeCount > 0;
}

bool CCitizenInfoWidget::TryOpenOperationModeSelection()
{
    auto& BuildingState = mState.Building;

    if (mState.PanelMode != EPanelMode::Building ||
        BuildingState.SelectedTab != EBuildingInfoTab::Overview ||
        BuildingState.TrackedBuildingName.empty() ||
        BuildingState.OperationModeCount <= 0)
    {
        return false;
    }

    BuildingState.CustomsModeSelectionOpen = true;
    BuildingState.OperationModeSelectionPageIndex = -1;
    return true;
}

bool CCitizenInfoWidget::CycleOperationModeSelectionPage()
{
    auto& BuildingState = mState.Building;

    if (!BuildingState.CustomsModeSelectionOpen ||
        BuildingState.OperationModeCount <= 0)
    {
        return false;
    }

    const int PageCount = (std::max)(
        1,
        BuildingState.OperationModeSelectionPageCount);

    if (PageCount <= 1)
    {
        CloseOperationModeSelection();
        return true;
    }

    const int CurrentPageIndex = (std::max)(
        0,
        BuildingState.OperationModeSelectionPageIndex);
    BuildingState.OperationModeSelectionPageIndex =
        (CurrentPageIndex + 1) % PageCount;
    return true;
}

void CCitizenInfoWidget::SyncOverviewMetricScrollState(
    const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot)
{
    auto& BuildingState = mState.Building;
    BuildingState.OverviewMetricScrollActive = Snapshot.ShowOverviewMetricScroll;
    BuildingState.OverviewMetricScrollVisibleLineCount =
        Snapshot.OverviewMetricScrollVisibleLineCount;
    BuildingState.OverviewMetricScrollTotalLineCount =
        Snapshot.OverviewMetricScrollTotalLineCount;
    BuildingState.OverviewMetricScrollFirstRowIndex =
        Snapshot.OverviewMetricScrollFirstRowIndex;

    const int MaxOffset = (std::max)(
        0,
        Snapshot.OverviewMetricScrollTotalLineCount -
            Snapshot.OverviewMetricScrollVisibleLineCount);
    BuildingState.OverviewMetricScrollOffset = (std::max)(
        0,
        (std::min)(Snapshot.OverviewMetricScrollOffset, MaxOffset));
}

void CCitizenInfoWidget::SyncOperationModeSelectionState(
    const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot)
{
    auto& BuildingState = mState.Building;
    BuildingState.OperationModeCount = Snapshot.OperationModeCount;

    if (Snapshot.ShowOperationModeSelectionPage)
    {
        BuildingState.CustomsModeSelectionOpen = true;
        BuildingState.OperationModeSelectionPageIndex =
            Snapshot.OperationModeSelectionPageIndex;
        BuildingState.OperationModeSelectionPageCount =
            Snapshot.OperationModeSelectionPageCount;
        return;
    }

    BuildingState.CustomsModeSelectionOpen = false;
    BuildingState.OperationModeSelectionPageIndex = -1;
    BuildingState.OperationModeSelectionPageCount = 0;
}

bool CCitizenInfoWidget::IsTrackedCustomsOffice() const
{
    if (mState.Building.TrackedBuildingName.empty())
        return false;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());
    return InteractionSource &&
        InteractionSource->IsCustomsOfficeBuilding(
            mState.Building.TrackedBuildingName);
}

bool CCitizenInfoWidget::TrySelectOperationMode(int VisibleModeIndex)
{
    if (!mState.Building.CustomsModeSelectionOpen ||
        mState.Building.SelectedTab != EBuildingInfoTab::Overview ||
        !HasTrackedBuildingOperationModes())
    {
        return false;
    }

    const int PageIndex = (std::max)(
        0,
        mState.Building.OperationModeSelectionPageIndex);
    const int ModeIndex =
        PageIndex * GBudgetLevelCount + VisibleModeIndex;

    if (ModeIndex < 0 ||
        ModeIndex >= mState.Building.OperationModeCount)
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
            mState.Building.TrackedBuildingName,
            ModeIndex,
            FeedbackMessage))
    {
        return false;
    }

    CloseOperationModeSelection();
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

    CloseOperationModeSelection();
    SetEnable(false);
    return true;
}

void CCitizenInfoWidget::SetBuildingBudgetLevel(int Level)
{
    if (mState.Building.TrackedBuildingName.empty())
        return;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource ||
        !InteractionSource->SetBuildingBudgetLevel(
            mState.Building.TrackedBuildingName,
            Level))
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
    if (mState.Building.TrackedBuildingName.empty())
        return;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource ||
        !InteractionSource->DemolishBuilding(
            mState.Building.TrackedBuildingName))
    {
        return;
    }

    ResetBuildingModeState();
    SetEnable(false);
}

void CCitizenInfoWidget::OnMoveButtonClick()
{
    if (mState.Building.TrackedBuildingName.empty())
        return;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource ||
        !InteractionSource->BeginMoveBuilding(
            mState.Building.TrackedBuildingName))
    {
        return;
    }

    SetEnable(false);
}

void CCitizenInfoWidget::OnFocusButtonClick()
{
    if (mState.Building.TrackedBuildingName.empty())
        return;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource)
        return;

    InteractionSource->FocusBuilding(mState.Building.TrackedBuildingName);
}

void CCitizenInfoWidget::OnOverviewCommandButtonClick()
{
    if (mState.Building.TrackedBuildingName.empty())
        return;

    const auto InteractionSource =
        CitizenInfoInteractionService::CreateWorldInteractionSource(
            mWorld.lock());

    if (!InteractionSource)
        return;

    if (InteractionSource->IsCustomsOfficeBuilding(
            mState.Building.TrackedBuildingName) &&
        mState.Building.SelectedTab == EBuildingInfoTab::Overview)
    {
        if (mState.Building.CustomsModeSelectionOpen)
        {
            CloseOperationModeSelection();
            RefreshFromState();
            return;
        }

        OpenTradeWidget();
        return;
    }

    if (mState.Building.SelectedTab == EBuildingInfoTab::Overview &&
        HasTrackedBuildingOperationModes())
    {
        if (mState.Building.CustomsModeSelectionOpen)
        {
            if (CycleOperationModeSelectionPage())
            {
                RefreshFromState();
                return;
            }
        }
        else if (TryOpenOperationModeSelection())
        {
            RefreshFromState();
            return;
        }
    }

    std::wstring FeedbackMessage;

    if (!InteractionSource->ExecuteBuildingCommand(
            mState.Building.TrackedBuildingName,
            mState.Building.SelectedTab,
            FeedbackMessage))
    {
        return;
    }

    RefreshFromState();
}

void CCitizenInfoWidget::OnBudgetLevel1Click()
{
    if (TrySelectOperationMode(0))
        return;

    SetBuildingBudgetLevel(1);
}

void CCitizenInfoWidget::OnBudgetLevel2Click()
{
    if (TrySelectOperationMode(1))
        return;

    SetBuildingBudgetLevel(2);
}

void CCitizenInfoWidget::OnBudgetLevel3Click()
{
    if (TrySelectOperationMode(2))
        return;

    SetBuildingBudgetLevel(3);
}

void CCitizenInfoWidget::OnBudgetLevel4Click()
{
    if (TrySelectOperationMode(3))
        return;

    SetBuildingBudgetLevel(4);
}

void CCitizenInfoWidget::OnBudgetLevel5Click()
{
    if (TrySelectOperationMode(4))
        return;

    SetBuildingBudgetLevel(5);
}
