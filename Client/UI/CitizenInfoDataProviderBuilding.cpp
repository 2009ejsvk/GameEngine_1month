#include "CitizenInfoDataProviderInternal.h"
#include "CitizenInfoConstants.h"
#include "CitizenInfoPresentation.h"
#include "UIEnumLabels.h"
#include "UIStrings.h"
#include "../Building/BuildingCatalog.h"
#include <algorithm>

using namespace CitizenInfoDataProviderInternal;

namespace
{
    FBuildingPanelVisibility ComputeBuildingPanelVisibility(
        const FBuildingUiSnapshot& BuildingSnapshot,
        int SelectedTabIndex,
        bool IsCustomsOffice,
        bool ShowCustomsModeSelection)
    {
        FBuildingPanelVisibility Visibility;
        const bool IsOverviewTab = SelectedTabIndex == 0;
        const bool IsStatisticsTab = SelectedTabIndex == 1;
        const bool IsUpgradesTab = SelectedTabIndex == 2;
        const bool IsInformationTab = SelectedTabIndex == 4;

        Visibility.SupportsOperationModeSelection =
            IsOverviewTab &&
            !BuildingSnapshot.Harbor &&
            !BuildingSnapshot.OperationModes.empty();
        Visibility.ShowOperationModeSelectionPage =
            Visibility.SupportsOperationModeSelection &&
            ShowCustomsModeSelection;
        Visibility.ShowCustomsModePage =
            IsCustomsOffice &&
            Visibility.ShowOperationModeSelectionPage;
        Visibility.ShowGenericOperationModeSelectionPage =
            !IsCustomsOffice &&
            Visibility.ShowOperationModeSelectionPage;
        Visibility.ShowBudgetControls = IsOverviewTab;
        Visibility.ShowActionButtons =
            IsOverviewTab &&
            !Visibility.ShowOperationModeSelectionPage;
        Visibility.ShowDemolishButton = Visibility.ShowActionButtons;
        Visibility.ShowMoveButton =
            Visibility.ShowActionButtons &&
            !IsCustomsOffice;
        Visibility.ShowFocusButton = Visibility.ShowActionButtons;
        Visibility.ShowBuildingOverview =
            IsOverviewTab &&
            BuildingSnapshot.Residential;
        Visibility.ShowBuildingWorkOverview =
            IsOverviewTab &&
            !Visibility.ShowOperationModeSelectionPage &&
            (IsCustomsOffice ||
                CitizenInfoPresentation::UseGenericBuildingWorkOverview(
                    BuildingSnapshot));
        Visibility.ShowHydroponicCommand =
            IsOverviewTab &&
            CitizenInfoBuildingRuntime::IsHydroponicFarmBuilding(
                BuildingSnapshot);
        Visibility.ShowOperationModeCommand =
            Visibility.SupportsOperationModeSelection &&
            !IsCustomsOffice;
        Visibility.ShowOverviewWorkModeCommand =
            Visibility.ShowOperationModeCommand &&
            (Visibility.ShowBuildingWorkOverview ||
                Visibility.ShowBuildingOverview);
        Visibility.ShowWarehousePolicyCommand =
            IsOverviewTab &&
            BuildingSnapshot.Warehouse &&
            BuildingSnapshot.OperationModes.empty();
        Visibility.ShowRuntimeUpgradeCommand =
            IsUpgradesTab &&
            !BuildingSnapshot.Harbor &&
            !IsCustomsOffice &&
            BuildingSnapshot.CatalogEntry &&
            !BuildingSnapshot.CatalogEntry->RuntimeUpgradeDefs.empty();
        Visibility.ShowWarehousePriorityCommand =
            IsInformationTab &&
            BuildingSnapshot.Warehouse;
        Visibility.ShowRepairCommand =
            IsStatisticsTab &&
            BuildingSnapshot.DamageLevel != EBuildingDamageLevel::None &&
            BuildingSnapshot.RepairAffordable;
        Visibility.ShowHarborExportCommand =
            IsInformationTab &&
            BuildingSnapshot.Harbor;
        Visibility.ShowCustomsTradeCommand =
            IsCustomsOffice &&
            IsOverviewTab &&
            !Visibility.ShowCustomsModePage;
        Visibility.ShowCustomsBackCommand =
            Visibility.ShowCustomsModePage;
        Visibility.ShowOverviewWorkModeButton =
            Visibility.ShowOverviewWorkModeCommand &&
            !Visibility.ShowOperationModeSelectionPage;
        Visibility.ShowOverviewCommandButton =
            Visibility.ShowCustomsTradeCommand ||
            Visibility.ShowCustomsBackCommand ||
            Visibility.ShowRepairCommand ||
            (Visibility.ShowOperationModeCommand &&
                !Visibility.ShowOverviewWorkModeCommand &&
                !Visibility.ShowOperationModeSelectionPage) ||
            Visibility.ShowWarehousePolicyCommand ||
            Visibility.ShowRuntimeUpgradeCommand ||
            Visibility.ShowWarehousePriorityCommand ||
            Visibility.ShowHydroponicCommand ||
            Visibility.ShowHarborExportCommand;
        Visibility.ShowBudgetText =
            !Visibility.ShowOperationModeSelectionPage;
        return Visibility;
    }
}

namespace CitizenInfoDataProviderInternal
{
    // Building panel-specific snapshot assembly lives here.
    FCitizenInfoSnapshot BuildTrackedBuildingSnapshotImpl(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>& QuerySource,
        const FTrackedBuildingRequest& Request)
    {
        FBuildingUiSnapshot BuildingSnapshot;

        if (!CitizenInfoBuildingRuntime::BuildBuildingUiSnapshot(
            QuerySource,
            Request.BuildingName,
            BuildingSnapshot))
        {
            return FCitizenInfoSnapshot();
        }

        FCitizenInfoSnapshot Result;
        Result.Valid = true;
        Result.Mode = EPanelMode::Building;
        Result.SelectedTabIndex =
            (std::max)(
                0,
                (std::min)(
                    CitizenInfoConstants::GBuildingTabCount - 1,
                    Request.SelectedTabIndex));
        Result.BudgetLevel = BuildingSnapshot.BudgetLevel;
        Result.Title = BuildingSnapshot.DisplayName.empty() ?
            BuildingSnapshot.ObjectName :
            BuildingSnapshot.DisplayName;
        const bool IsCustomsOffice =
            CitizenInfoBuildingRuntime::IsCustomsOfficeBuilding(
                BuildingSnapshot);
        const int OperationModeCount =
            static_cast<int>(BuildingSnapshot.OperationModes.size());
        const int VisibleOperationModeButtonCount =
            static_cast<int>(Result.BudgetButtonLabels.size());
        const FBuildingPanelVisibility Visibility =
            ComputeBuildingPanelVisibility(
                BuildingSnapshot,
                Result.SelectedTabIndex,
                IsCustomsOffice,
                Request.ShowCustomsModeSelection);
        int OperationModeSelectionPageCount = 0;
        int OperationModeSelectionPageIndex = 0;
        int OperationModeSelectionStartIndex = 0;

        if (Visibility.ShowOperationModeSelectionPage &&
            VisibleOperationModeButtonCount > 0)
        {
            OperationModeSelectionPageCount =
                (std::max)(
                    1,
                    (OperationModeCount +
                        VisibleOperationModeButtonCount - 1) /
                        VisibleOperationModeButtonCount);

            const int ActivePageIndex =
                (std::max)(
                    0,
                    (std::min)(
                        OperationModeSelectionPageCount - 1,
                        BuildingSnapshot.ActiveOperationModeIndex /
                            VisibleOperationModeButtonCount));
            OperationModeSelectionPageIndex =
                Request.OperationModeSelectionPageIndex >= 0 ?
                    (std::max)(
                        0,
                        (std::min)(
                            Request.OperationModeSelectionPageIndex,
                            OperationModeSelectionPageCount - 1)) :
                    ActivePageIndex;
            OperationModeSelectionStartIndex =
                OperationModeSelectionPageIndex *
                VisibleOperationModeButtonCount;
        }

        Result.OperationModeCount = OperationModeCount;
        Result.ShowOperationModeSelectionPage =
            Visibility.ShowOperationModeSelectionPage;
        Result.OperationModeSelectionPageIndex =
            OperationModeSelectionPageIndex;
        Result.OperationModeSelectionPageCount =
            OperationModeSelectionPageCount;
        Result.SelectedBudgetButtonIndex =
            Result.BudgetLevel - 1;

        if (BuildingSnapshot.CatalogEntry)
        {
            Result.Subtitle =
                std::wstring(GetBuildingEraDisplayName(
                    BuildingSnapshot.CatalogEntry->UnlockEra)) +
                L"  |  " +
                BuildingSnapshot.CategoryName;
        }
        else
        {
            Result.Subtitle = BuildingSnapshot.CategoryName;
        }

        if (BuildingSnapshot.DamageLevel != EBuildingDamageLevel::None)
        {
            Result.ShowHeaderNote = true;
            Result.BuildingSubtitleColor =
                BuildingSnapshot.DamageLevel == EBuildingDamageLevel::Critical ?
                    FVector4(0.82f, 0.20f, 0.18f, 1.f) :
                    FVector4(0.90f, 0.58f, 0.18f, 1.f);
            Result.HeaderNoteText =
                Ui(L"citizen_info.label.damage_status") +
                L": " +
                UIEnumLabels::GetBuildingDamageLevelDisplayName(
                    BuildingSnapshot.DamageLevel);

            if (BuildingSnapshot.RepairCost > 0)
            {
                Result.HeaderNoteText +=
                    L"  |  " +
                    Ui(L"citizen_info.label.repair_cost") +
                    L": " +
                    CitizenInfoPresentation::FormatMoney(
                        BuildingSnapshot.RepairCost);
            }

            if (BuildingSnapshot.RepairCost > 0 &&
                !BuildingSnapshot.RepairAffordable)
            {
                Result.HeaderNoteText += L"  |  예산 부족";
            }
        }

        Result.PageTitle =
            CitizenInfoPresentation::ResolveBuildingPageTitle(
                BuildingSnapshot,
                Result.SelectedTabIndex,
                Visibility.ShowOperationModeSelectionPage);
        Result.ShowTabButtons = true;
        Result.ShowBudgetControls = Visibility.ShowBudgetControls;
        Result.ShowActionButtons = Visibility.ShowActionButtons;
        Result.ShowDemolishButton = Visibility.ShowDemolishButton;
        Result.ShowMoveButton = Visibility.ShowMoveButton;
        Result.ShowFocusButton = Visibility.ShowFocusButton;
        Result.ShowBuildingOverview = Visibility.ShowBuildingOverview;
        Result.ShowBuildingWorkOverview =
            Visibility.ShowBuildingWorkOverview;
        Result.ShowBuildingMetricRows = false;
        Result.ShowBuildingUpgradeCard = false;
        Result.ShowBuildingInformationParagraphs = false;
        Result.ShowOverviewWorkModeButton =
            Visibility.ShowOverviewWorkModeButton;
        Result.ShowOverviewCommandButton =
            Visibility.ShowOverviewCommandButton;
        Result.OverviewCommandButtonText.clear();
        Result.ShowBudgetText = Visibility.ShowBudgetText;

        for (size_t Index = 0; Index < Result.BudgetButtonLabels.size(); ++Index)
        {
            const bool IsBudgetSlot = Index < 5;
            Result.BudgetButtonLabels[Index] =
                IsBudgetSlot ?
                    std::to_wstring(static_cast<int>(Index) + 1) :
                    std::wstring();
            Result.BudgetButtonEnabled[Index] =
                Result.ShowBudgetControls && IsBudgetSlot;
        }

        if (Visibility.ShowCustomsTradeCommand)
        {
            Result.OverviewCommandButtonText = L"무역 화면 열기";
        }
        else if (Visibility.ShowCustomsBackCommand)
        {
            Result.OverviewCommandButtonText = L"뒤로";
        }
        else if (Visibility.ShowRepairCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.repair_damage") +
                L": " +
                CitizenInfoPresentation::FormatMoney(
                    BuildingSnapshot.RepairCost);
        }
        else if (Visibility.ShowOperationModeCommand)
        {
            Result.OverviewCommandButtonText =
                std::wstring(L"운영 모드 선택") +
                L": " +
                (BuildingSnapshot.ActiveOperationModeText.empty() ?
                    L"-" :
                    BuildingSnapshot.ActiveOperationModeText);
        }
        else if (Visibility.ShowWarehousePolicyCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.warehouse_policy_cycle") +
                L": " +
                (BuildingSnapshot.WarehousePolicySelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.WarehousePolicySelectionText);
        }
        else if (Visibility.ShowRuntimeUpgradeCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.runtime_upgrade_cycle") +
                L": " +
                (BuildingSnapshot.ActiveRuntimeUpgradeText.empty() ?
                    L"-" :
                    BuildingSnapshot.ActiveRuntimeUpgradeText);
        }
        else if (Visibility.ShowWarehousePriorityCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.warehouse_priority_cycle") +
                L": " +
                (BuildingSnapshot.WarehousePrioritySelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.WarehousePrioritySelectionText);
        }
        else if (Visibility.ShowHarborExportCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.export_block_cycle") +
                L": " +
                (BuildingSnapshot.HarborExportSelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.HarborExportSelectionText);
        }
        else if (Visibility.ShowHydroponicCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.change_resource");
        }

        if (Visibility.ShowOperationModeSelectionPage)
        {
            Result.SelectedBudgetButtonIndex = -1;

            for (size_t Index = 0;
                Index < Result.BudgetButtonLabels.size();
                ++Index)
            {
                const int ModeIndex =
                    OperationModeSelectionStartIndex +
                    static_cast<int>(Index);

                if (ModeIndex < OperationModeCount)
                {
                    Result.BudgetButtonLabels[Index] =
                        BuildingSnapshot.OperationModes[
                            static_cast<size_t>(ModeIndex)];
                    Result.BudgetButtonEnabled[Index] = true;

                    if (ModeIndex ==
                        BuildingSnapshot.ActiveOperationModeIndex)
                    {
                        Result.SelectedBudgetButtonIndex =
                            static_cast<int>(Index);
                    }
                }
                else
                {
                    Result.BudgetButtonLabels[Index].clear();
                    Result.BudgetButtonEnabled[Index] = false;
                }
            }
        }

        const long long TotalMonthlyCost =
            static_cast<long long>(BuildingSnapshot.MonthlyWageCost) +
            static_cast<long long>(BuildingSnapshot.MonthlyUpkeepCost);
        Result.BudgetText = UIStrings::Format(
            L"citizen_info.budget.summary_template",
            {
                std::to_wstring(BuildingSnapshot.BudgetLevel),
                CitizenInfoPresentation::FormatMultiplier(
                    BuildingSnapshot.BudgetScale),
                CitizenInfoPresentation::FormatMoney(TotalMonthlyCost)
            });

        if (BuildingSnapshot.CatalogEntry)
        {
            Result.TitleIconPath = GetCatalogEntryIconPath(
                *BuildingSnapshot.CatalogEntry);

            if (Result.TitleIconPath)
            {
                Result.ShowTitleIcon = true;
                Result.TitleIconTextureKey =
                    BuildCatalogIconTextureKey(
                        *BuildingSnapshot.CatalogEntry);
            }
        }

        if (Result.SelectedTabIndex == 2 &&
            !Visibility.ShowOperationModeSelectionPage &&
            CitizenInfoPresentation::PopulateBuildingUpgradeCard(
                BuildingSnapshot,
                Result))
        {
            Result.ShowBuildingUpgradeCard = true;
            Result.UpgradeCardIconPath = Result.TitleIconPath;
            Result.UpgradeCardIconTextureKey = Result.TitleIconTextureKey;
        }

        if (Result.SelectedTabIndex == 4 &&
            !Visibility.ShowOperationModeSelectionPage &&
            CitizenInfoPresentation::PopulateBuildingInformationPanel(
                BuildingSnapshot,
                Result))
        {
            Result.ShowBuildingInformationParagraphs = true;
        }

        if (Result.ShowBuildingOverview)
        {
            CitizenInfoPresentation::PopulateResidentialOverview(
                BuildingSnapshot,
                Result);
        }

        if (Result.ShowBuildingWorkOverview)
        {
            if (IsCustomsOffice)
            {
                CitizenInfoPresentation::PopulateCustomsWorkOverview(
                    BuildingSnapshot,
                    Result);
            }
            else
            {
                CitizenInfoPresentation::PopulateGenericWorkOverview(
                    BuildingSnapshot,
                    Result,
                    Request.OverviewMetricScrollOffset);
            }
        }

        if (Result.SelectedTabIndex == 0 &&
            !Visibility.ShowOperationModeSelectionPage)
        {
            CitizenInfoPresentation::PopulateOverviewStageBadge(
                BuildingSnapshot,
                Result);
        }

        if (Result.SelectedTabIndex == 1)
        {
            CitizenInfoPresentation::PopulateBuildingStatisticsMetrics(
                BuildingSnapshot,
                IsCustomsOffice,
                Result,
                Request.OverviewMetricScrollOffset);
            Result.ShowBuildingMetricRows =
                CitizenInfoPresentation::HasOverviewMetrics(Result);
        }

        if (Result.SelectedTabIndex == 3)
        {
            CitizenInfoPresentation::PopulateBuildingEfficiencyMetrics(
                BuildingSnapshot,
                IsCustomsOffice,
                Result);
            Result.ShowBuildingMetricRows =
                CitizenInfoPresentation::HasOverviewMetrics(Result);
        }

        Result.ShowSectionRibbon =
            Result.SelectedTabIndex != 0 &&
            !Visibility.ShowOperationModeSelectionPage;

        switch (Result.SelectedTabIndex)
        {
        case 1:
            Result.BodyText =
                CitizenInfoPresentation::BuildStatisticsBody(
                    BuildingSnapshot);
            break;
        case 2:
            Result.BodyText = IsCustomsOffice ?
                CitizenInfoPresentation::BuildCustomsUpgradesBody(
                    BuildingSnapshot) :
                CitizenInfoPresentation::BuildUpgradesBody(
                    BuildingSnapshot);
            break;
        case 3:
            Result.BodyText =
                CitizenInfoPresentation::BuildEfficiencyBody(
                    BuildingSnapshot);
            break;
        case 4:
            Result.BodyText =
                CitizenInfoPresentation::BuildInformationBody(
                    BuildingSnapshot);
            break;
        case 0:
        default:
            Result.BodyText =
                Visibility.ShowOperationModeSelectionPage ?
                    CitizenInfoPresentation::BuildCustomsModeSelectionBody(
                        BuildingSnapshot) :
                Result.ShowBuildingOverview ?
                    std::wstring() :
                    CitizenInfoPresentation::BuildOverviewBody(
                        BuildingSnapshot);
            break;
        }

        return Result;
    }
}
