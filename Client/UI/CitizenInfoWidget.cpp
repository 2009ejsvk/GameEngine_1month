#include "CitizenInfoWidget.h"
#include "CitizenInfoDataProvider.h"
#include "CitizenInfoRenderer.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/PlacementController.h"
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
    mSelectedBuildingTab = EBuildingInfoTab::Overview;
    mTrackedCitizenName.clear();
    mTrackedBuildingName.clear();
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
    mRequestedScreenPos = ScreenPos;
    mPanelMode = EPanelMode::Citizen;
    mTrackedCitizenName = CitizenName;
    mTrackedBuildingName.clear();
    mSelectedBuildingTab = EBuildingInfoTab::Overview;

    FNpcPoliticalProfile PoliticalProfile;
    FCitizenIdentityProfile IdentityProfile;
    auto World = mWorld.lock();

    if (World)
    {
        auto Citizen = World->FindObject<CBuildingMarkerOrb>(CitizenName).lock();

        if (Citizen && Citizen->GetAlive() && Citizen->GetEnable())
        {
            PoliticalProfile = Citizen->GetPoliticalProfile();
            IdentityProfile = Citizen->GetIdentityProfile();
        }
    }

    const auto Snapshot = CitizenInfoDataProvider::BuildCitizenSnapshot(
        CitizenName,
        Satisfaction,
        IdentityProfile,
        PoliticalProfile);

    SetEnable(true);
    FCitizenInfoRenderer::ApplySnapshot(*this, Snapshot);
    FCitizenInfoRenderer::RefreshLayout(*this);
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

    SetEnable(true);
    RefreshFromState();
}

void CCitizenInfoWidget::RefreshFromState()
{
    CitizenInfoDataProvider::FCitizenInfoSnapshot Snapshot;

    if (mPanelMode == EPanelMode::Citizen)
    {
        Snapshot = CitizenInfoDataProvider::BuildTrackedCitizenSnapshot(
            mWorld.lock(),
            mTrackedCitizenName);
    }
    else
    {
        Snapshot = CitizenInfoDataProvider::BuildTrackedBuildingSnapshot(
            mWorld.lock(),
            mTrackedBuildingName,
            static_cast<int>(mSelectedBuildingTab));
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

void CCitizenInfoWidget::SelectBuildingTab(EBuildingInfoTab Tab)
{
    if (mSelectedBuildingTab == Tab)
        return;

    mSelectedBuildingTab = Tab;
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

void CCitizenInfoWidget::OnBudgetLevel1Click()
{
    SetBuildingBudgetLevel(1);
}

void CCitizenInfoWidget::OnBudgetLevel2Click()
{
    SetBuildingBudgetLevel(2);
}

void CCitizenInfoWidget::OnBudgetLevel3Click()
{
    SetBuildingBudgetLevel(3);
}

void CCitizenInfoWidget::OnBudgetLevel4Click()
{
    SetBuildingBudgetLevel(4);
}

void CCitizenInfoWidget::OnBudgetLevel5Click()
{
    SetBuildingBudgetLevel(5);
}
