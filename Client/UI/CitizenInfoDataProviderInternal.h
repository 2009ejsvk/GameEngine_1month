#pragma once

#include "CitizenInfoBuildingRuntime.h"
#include "CitizenInfoDataProvider.h"
#include "UIStringShorthand.h"

namespace CitizenInfoDataProviderInternal
{
    using CitizenInfoDataProvider::EPanelMode;
    using CitizenInfoDataProvider::FCitizenInfoBuildingRecord;
    using CitizenInfoDataProvider::FCitizenInfoCitizenRecord;
    using CitizenInfoDataProvider::FCitizenInfoSnapshot;
    using CitizenInfoDataProvider::ICitizenInfoQuerySource;
    using FBuildingUiSnapshot = CitizenInfoBuildingRuntime::FBuildingUiSnapshot;
    using UIStringShorthand::Ui;
    using UIStringShorthand::UiText;

    struct FTrackedCitizenRequest
    {
        std::string CitizenName;
        int SelectedTabIndex = 0;
    };

    struct FTrackedBuildingRequest
    {
        std::string BuildingName;
        int SelectedTabIndex = 0;
        bool ShowCustomsModeSelection = false;
        int OperationModeSelectionPageIndex = -1;
        int OverviewMetricScrollOffset = 0;
    };

    struct FBuildingPanelVisibility
    {
        bool SupportsOperationModeSelection = false;
        bool ShowOperationModeSelectionPage = false;
        bool ShowCustomsModePage = false;
        bool ShowGenericOperationModeSelectionPage = false;
        bool ShowBudgetControls = false;
        bool ShowActionButtons = false;
        bool ShowDemolishButton = false;
        bool ShowMoveButton = false;
        bool ShowFocusButton = false;
        bool ShowBuildingOverview = false;
        bool ShowBuildingWorkOverview = false;
        bool ShowHydroponicCommand = false;
        bool ShowOperationModeCommand = false;
        bool ShowOverviewWorkModeCommand = false;
        bool ShowWarehousePolicyCommand = false;
        bool ShowRuntimeUpgradeCommand = false;
        bool ShowWarehousePriorityCommand = false;
        bool ShowRepairCommand = false;
        bool ShowHarborExportCommand = false;
        bool ShowCustomsTradeCommand = false;
        bool ShowCustomsBackCommand = false;
        bool ShowOverviewWorkModeButton = false;
        bool ShowOverviewCommandButton = false;
        bool ShowBudgetText = false;
    };

    FCitizenInfoSnapshot BuildCitizenSnapshotImpl(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FCitizenIdentityProfile& IdentityProfile,
        const FNpcPoliticalProfile& PoliticalProfile,
        int TabIndex);

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshotImpl(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>& QuerySource,
        const FTrackedCitizenRequest& Request);

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshotImpl(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>& QuerySource,
        const FTrackedBuildingRequest& Request);

    std::string BuildCatalogIconTextureKey(
        const FBuildingCatalogEntry& Entry);
}
