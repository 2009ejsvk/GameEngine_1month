#include "CitizenInfoWidget.h"
#include "CitizenInfoDataProvider.h"
#include "CitizenInfoRenderer.h"
#include "TradeWidget.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/PlacementController.h"
#include "../World/GovernmentCommandService.h"
#include "../ObjectNames.h"
#include "World/World.h"

CCitizenInfoWidget::CCitizenInfoWidget()
{
}

CCitizenInfoWidget::~CCitizenInfoWidget()
{
}

bool CCitizenInfoWidget::Init()
{
    CWidgetContainer::Init();

    mPanelMode = EPanelMode::Citizen;
    mSelectedCitizenTab = ECitizenInfoTab::Overview;
    mSelectedBuildingTab = EBuildingInfoTab::Overview;
    mTrackedCitizenName.clear();
    mTrackedBuildingName.clear();
    mCustomsModeSelectionOpen = false;
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
    mTrackedCitizenName = CitizenName;
    mTrackedBuildingName.clear();
    mSelectedCitizenTab = ECitizenInfoTab::Overview;
    mCustomsModeSelectionOpen = false;

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
    mTrackedCitizenName.clear();
    mTrackedBuildingName = BuildingObjectName;
    mSelectedBuildingTab = EBuildingInfoTab::Overview;
    mCustomsModeSelectionOpen = false;

    SetEnable(true);
    RefreshFromState();
}

void CCitizenInfoWidget::RefreshFromState()
{
    CitizenInfoDataProvider::FCitizenInfoSnapshot Snapshot;
    const int SelectedTabIndex = GetSelectedTabIndexForCurrentMode();

    if (mPanelMode == EPanelMode::Citizen)
    {
        Snapshot = CitizenInfoDataProvider::BuildTrackedCitizenSnapshot(
            mWorld.lock(),
            mTrackedCitizenName,
            SelectedTabIndex);
    }
    else
    {
        Snapshot = CitizenInfoDataProvider::BuildTrackedBuildingSnapshot(
            mWorld.lock(),
            mTrackedBuildingName,
            SelectedTabIndex,
            mCustomsModeSelectionOpen);
    }

    if (!Snapshot.Valid)
    {
        mTrackedCitizenName.clear();
        mTrackedBuildingName.clear();
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

    auto World = mWorld.lock();

    if (!World)
        return false;

    auto Building = World->FindObject<CPlacementAreaObject>(
        mTrackedBuildingName).lock();

    if (!Building || !Building->GetAlive() || !Building->GetEnable())
        return false;

    return IsCustomsOfficeBuildingId(Building->GetBuildingId());
}

bool CCitizenInfoWidget::TrySelectCustomsOperationMode(int ModeIndex)
{
    if (!mCustomsModeSelectionOpen ||
        mSelectedBuildingTab != EBuildingInfoTab::Overview ||
        !IsTrackedCustomsOffice())
    {
        return false;
    }

    auto World = mWorld.lock();

    if (!World)
        return false;

    auto Building = World->FindObject<CPlacementAreaObject>(
        mTrackedBuildingName).lock();

    if (!Building || !Building->GetAlive() || !Building->GetEnable())
        return false;

    std::wstring FeedbackMessage;

    if (!Building->SetActiveOperationMode(ModeIndex, FeedbackMessage))
        return false;

    RefreshFromState();
    return true;
}

bool CCitizenInfoWidget::OpenTradeWidget()
{
    auto World = mWorld.lock();

    if (!World)
        return false;

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return false;

    auto TradeWidget =
        UIManager->FindWidget<CTradeWidget>(GTradeWidgetName).lock();

    if (!TradeWidget)
        return false;

    TradeWidget->SetOpen(true);
    mCustomsModeSelectionOpen = false;
    SetEnable(false);
    return true;
}

void CCitizenInfoWidget::SetBuildingBudgetLevel(int Level)
{
    if (mTrackedBuildingName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Building = World->FindObject<CPlacementAreaObject>(
        mTrackedBuildingName).lock();

    if (!Building || !Building->GetAlive() || !Building->GetEnable())
        return;

    Building->SetBudgetLevel(Level);
    RefreshFromState();
}

void CCitizenInfoWidget::OnCloseButtonClick()
{
    mTrackedCitizenName.clear();
    mTrackedBuildingName.clear();
    mCustomsModeSelectionOpen = false;
    SetEnable(false);
}

void CCitizenInfoWidget::OnDemolishButtonClick()
{
    if (mTrackedBuildingName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto PlacementController =
        World->FindObject<CPlacementController>(
            GPlacementControllerName).lock();

    if (!PlacementController)
        return;

    if (!PlacementController->DemolishBuildingByName(mTrackedBuildingName))
        return;

    mTrackedBuildingName.clear();
    mCustomsModeSelectionOpen = false;
    SetEnable(false);
}

void CCitizenInfoWidget::OnMoveButtonClick()
{
    if (mTrackedBuildingName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto PlacementController =
        World->FindObject<CPlacementController>(
            GPlacementControllerName).lock();

    if (!PlacementController)
        return;

    if (!PlacementController->BeginMoveExistingBuilding(mTrackedBuildingName))
        return;

    SetEnable(false);
}

void CCitizenInfoWidget::OnCloneButtonClick()
{
}

void CCitizenInfoWidget::OnOverviewCommandButtonClick()
{
    if (mTrackedBuildingName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Building = World->FindObject<CPlacementAreaObject>(
        mTrackedBuildingName).lock();

    if (!Building || !Building->GetAlive() || !Building->GetEnable())
        return;

    if (IsTrackedCustomsOffice() &&
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

    if (!Building->IsHarbor())
    {
        std::wstring FeedbackMessage;

        if (mSelectedBuildingTab == EBuildingInfoTab::Overview)
        {
            if (Building->HasOperationModes())
            {
                if (!Building->CycleOperationMode(FeedbackMessage))
                    return;
            }
            else if (Building->HasWarehouseStorageControls())
            {
                if (!Building->CycleWarehouseStoragePolicy(FeedbackMessage))
                    return;
            }
            else
            {
                return;
            }
        }
        else if (mSelectedBuildingTab == EBuildingInfoTab::Upgrades)
        {
            if (!Building->HasRuntimeUpgrades() ||
                !Building->CycleRuntimeUpgrade(FeedbackMessage))
            {
                return;
            }
        }
        else if (mSelectedBuildingTab == EBuildingInfoTab::Information)
        {
            if (!Building->HasWarehouseStorageControls() ||
                !Building->CycleWarehousePriority(FeedbackMessage))
            {
                return;
            }
        }
        else
        {
            return;
        }

        RefreshFromState();
        return;
    }

    auto MainWorld =
        std::dynamic_pointer_cast<IGovernmentCommandService>(World);

    if (!MainWorld)
        return;

    std::wstring FeedbackMessage;
    const EBuildingInfoTab SelectedTab = mSelectedBuildingTab;

    if (SelectedTab == EBuildingInfoTab::Overview)
    {
        MainWorld->CycleAutoImportResource(FeedbackMessage);
    }
    else if (SelectedTab == EBuildingInfoTab::Statistics)
    {
        MainWorld->CycleDomesticReservePolicy(FeedbackMessage);
    }
    else if (SelectedTab == EBuildingInfoTab::Upgrades)
    {
        MainWorld->CycleImportPerResourceCap(FeedbackMessage);
    }
    else if (SelectedTab == EBuildingInfoTab::Efficiency)
    {
        MainWorld->CycleImportBudgetPolicy(FeedbackMessage);
    }
    else if (SelectedTab == EBuildingInfoTab::Information)
    {
        MainWorld->CycleExportBlockedResource(FeedbackMessage);
    }
    else
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
