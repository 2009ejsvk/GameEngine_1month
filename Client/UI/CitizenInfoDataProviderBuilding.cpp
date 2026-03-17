#include "CitizenInfoDataProviderInternal.h"
#include "CitizenInfoConstants.h"
#include "CitizenInfoPresentation.h"
#include "CitizenInfoQueryService.h"
#include "UIStrings.h"
#include "../Building/BuildingCatalog.h"
#include <algorithm>

using namespace CitizenInfoDataProviderInternal;

namespace CitizenInfoDataProvider
{
    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex,
        bool ShowCustomsModeSelection,
        int OverviewMetricScrollOffset)
    {
        FBuildingUiSnapshot BuildingSnapshot;

        if (!CitizenInfoBuildingRuntime::BuildBuildingUiSnapshot(
            QuerySource,
            BuildingName,
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
                    SelectedBuildingTabIndex));
        Result.BudgetLevel = BuildingSnapshot.BudgetLevel;
        Result.Title = BuildingSnapshot.DisplayName.empty() ?
            BuildingSnapshot.ObjectName :
            BuildingSnapshot.DisplayName;
        const bool IsCustomsOffice =
            CitizenInfoBuildingRuntime::IsCustomsOfficeBuilding(
                BuildingSnapshot);
        const bool ShowCustomsModePage =
            IsCustomsOffice &&
            Result.SelectedTabIndex == 0 &&
            ShowCustomsModeSelection;

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
                GetDamageLevelDisplayName(BuildingSnapshot.DamageLevel);

            if (BuildingSnapshot.RepairCost > 0)
            {
                Result.HeaderNoteText +=
                    L"  |  " +
                    Ui(L"citizen_info.label.repair_cost") +
                    L": " +
                    FormatMoney(BuildingSnapshot.RepairCost);
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
                ShowCustomsModePage);
        Result.ShowTabButtons = true;
        Result.ShowBudgetControls = Result.SelectedTabIndex == 0;
        Result.ShowActionButtons =
            Result.SelectedTabIndex == 0 &&
            !ShowCustomsModePage;
        Result.ShowDemolishButton = Result.ShowActionButtons;
        Result.ShowMoveButton =
            Result.ShowActionButtons &&
            !IsCustomsOffice;
        Result.ShowFocusButton = Result.ShowActionButtons;
        Result.ShowBuildingOverview =
            Result.SelectedTabIndex == 0 &&
            BuildingSnapshot.Residential;
        Result.ShowBuildingWorkOverview =
            Result.SelectedTabIndex == 0 &&
            !ShowCustomsModePage &&
            (IsCustomsOffice ||
                CitizenInfoPresentation::UseGenericBuildingWorkOverview(
                    BuildingSnapshot));
        Result.ShowBuildingMetricRows = false;
        Result.ShowBuildingUpgradeCard = false;
        Result.ShowBuildingInformationParagraphs = false;
        const bool ShowHydroponicCommand =
            Result.SelectedTabIndex == 0 &&
            CitizenInfoBuildingRuntime::IsHydroponicFarmBuilding(
                BuildingSnapshot);
        const bool ShowOperationModeCommand =
            Result.SelectedTabIndex == 0 &&
            !BuildingSnapshot.Harbor &&
            !IsCustomsOffice &&
            !BuildingSnapshot.OperationModes.empty();
        const bool ShowOverviewWorkModeCommand =
            ShowOperationModeCommand &&
            (Result.ShowBuildingWorkOverview ||
                Result.ShowBuildingOverview);
        const bool ShowWarehousePolicyCommand =
            Result.SelectedTabIndex == 0 &&
            BuildingSnapshot.Warehouse &&
            BuildingSnapshot.OperationModes.empty();
        const bool ShowRuntimeUpgradeCommand =
            Result.SelectedTabIndex == 2 &&
            !BuildingSnapshot.Harbor &&
            !IsCustomsOffice &&
            BuildingSnapshot.CatalogEntry &&
            !BuildingSnapshot.CatalogEntry->RuntimeUpgradeDefs.empty();
        const bool ShowWarehousePriorityCommand =
            Result.SelectedTabIndex == 4 &&
            BuildingSnapshot.Warehouse;
        const bool ShowRepairCommand =
            Result.SelectedTabIndex == 1 &&
            BuildingSnapshot.DamageLevel != EBuildingDamageLevel::None &&
            BuildingSnapshot.RepairAffordable;
        const bool ShowHarborImportCommand =
            Result.SelectedTabIndex == 0 &&
            BuildingSnapshot.Harbor;
        const bool ShowHarborReserveCommand =
            Result.SelectedTabIndex == 1 &&
            BuildingSnapshot.Harbor;
        const bool ShowHarborImportCapCommand =
            Result.SelectedTabIndex == 2 &&
            BuildingSnapshot.Harbor;
        const bool ShowHarborImportBudgetCommand =
            Result.SelectedTabIndex == 3 &&
            BuildingSnapshot.Harbor;
        const bool ShowHarborExportCommand =
            Result.SelectedTabIndex == 4 &&
            BuildingSnapshot.Harbor;
        const bool ShowCustomsTradeCommand =
            IsCustomsOffice &&
            Result.SelectedTabIndex == 0 &&
            !ShowCustomsModePage;
        const bool ShowCustomsBackCommand =
            ShowCustomsModePage;
        Result.ShowOverviewWorkModeButton =
            ShowOverviewWorkModeCommand;
        Result.ShowOverviewCommandButton =
            ShowCustomsTradeCommand ||
            ShowCustomsBackCommand ||
            ShowRepairCommand ||
            (ShowOperationModeCommand &&
                !ShowOverviewWorkModeCommand) ||
            ShowWarehousePolicyCommand ||
            ShowRuntimeUpgradeCommand ||
            ShowWarehousePriorityCommand ||
            ShowHydroponicCommand ||
            ShowHarborImportCommand ||
            ShowHarborReserveCommand ||
            ShowHarborImportCapCommand ||
            ShowHarborImportBudgetCommand ||
            ShowHarborExportCommand;
        Result.OverviewCommandButtonText.clear();
        Result.ShowBudgetText = !ShowCustomsModePage;

        for (size_t Index = 0; Index < Result.BudgetButtonLabels.size(); ++Index)
        {
            Result.BudgetButtonLabels[Index] =
                std::to_wstring(static_cast<int>(Index) + 1);
            Result.BudgetButtonEnabled[Index] = Result.ShowBudgetControls;
        }

        if (ShowCustomsTradeCommand)
        {
            Result.OverviewCommandButtonText = L"무역 화면 열기";
        }
        else if (ShowCustomsBackCommand)
        {
            Result.OverviewCommandButtonText = L"뒤로";
        }
        else if (ShowRepairCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.repair_damage") +
                L": " +
                FormatMoney(BuildingSnapshot.RepairCost);
        }
        else if (ShowOperationModeCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.operation_mode_cycle") +
                L": " +
                (BuildingSnapshot.ActiveOperationModeText.empty() ?
                    L"-" :
                    BuildingSnapshot.ActiveOperationModeText);
        }
        else if (ShowWarehousePolicyCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.warehouse_policy_cycle") +
                L": " +
                (BuildingSnapshot.WarehousePolicySelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.WarehousePolicySelectionText);
        }
        else if (ShowRuntimeUpgradeCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.runtime_upgrade_cycle") +
                L": " +
                (BuildingSnapshot.ActiveRuntimeUpgradeText.empty() ?
                    L"-" :
                    BuildingSnapshot.ActiveRuntimeUpgradeText);
        }
        else if (ShowWarehousePriorityCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.warehouse_priority_cycle") +
                L": " +
                (BuildingSnapshot.WarehousePrioritySelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.WarehousePrioritySelectionText);
        }
        else if (ShowHarborImportCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.auto_import_cycle") +
                L": " +
                (BuildingSnapshot.HarborImportSelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.HarborImportSelectionText);
        }
        else if (ShowHarborReserveCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.domestic_reserve_cycle") +
                L": " +
                (BuildingSnapshot.HarborDomesticReserveSelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.HarborDomesticReserveSelectionText);
        }
        else if (ShowHarborImportCapCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.import_cap_cycle") +
                L": " +
                (BuildingSnapshot.HarborImportCapSelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.HarborImportCapSelectionText);
        }
        else if (ShowHarborImportBudgetCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.import_budget_cycle") +
                L": " +
                (BuildingSnapshot.HarborImportBudgetSelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.HarborImportBudgetSelectionText);
        }
        else if (ShowHarborExportCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.export_block_cycle") +
                L": " +
                (BuildingSnapshot.HarborExportSelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.HarborExportSelectionText);
        }
        else if (ShowHydroponicCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.change_resource");
        }

        if (ShowCustomsModePage)
        {
            Result.BudgetLevel =
                (std::max)(1, BuildingSnapshot.ActiveOperationModeIndex + 1);

            for (size_t Index = 0;
                Index < Result.BudgetButtonLabels.size();
                ++Index)
            {
                if (Index < BuildingSnapshot.OperationModes.size())
                {
                    Result.BudgetButtonLabels[Index] =
                        BuildingSnapshot.OperationModes[Index];
                    Result.BudgetButtonEnabled[Index] = true;
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
            !ShowCustomsModePage &&
            CitizenInfoPresentation::PopulateBuildingUpgradeCard(
                BuildingSnapshot,
                Result))
        {
            Result.ShowBuildingUpgradeCard = true;
            Result.UpgradeCardIconPath = Result.TitleIconPath;
            Result.UpgradeCardIconTextureKey = Result.TitleIconTextureKey;
        }

        if (Result.SelectedTabIndex == 4 &&
            !ShowCustomsModePage &&
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
                    OverviewMetricScrollOffset);
            }
        }

        if (Result.SelectedTabIndex == 0 &&
            !ShowCustomsModePage)
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
                OverviewMetricScrollOffset);
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
            !ShowCustomsModePage;

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
                ShowCustomsModePage ?
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

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex,
        bool ShowCustomsModeSelection,
        int OverviewMetricScrollOffset)
    {
        return BuildTrackedBuildingSnapshot(
            CitizenInfoQueryService::CreateWorldQuerySource(World),
            BuildingName,
            SelectedBuildingTabIndex,
            ShowCustomsModeSelection,
            OverviewMetricScrollOffset);
    }
}
