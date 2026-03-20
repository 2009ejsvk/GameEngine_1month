#include "CitizenInfoDataProviderInternal.h"
#include "CitizenInfoQueryService.h"
#include "../Building/BuildingCatalog.h"

namespace CitizenInfoDataProvider
{
    FCitizenInfoSnapshot BuildCitizenSnapshot(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FCitizenIdentityProfile& IdentityProfile,
        const FNpcPoliticalProfile& PoliticalProfile,
        int TabIndex)
    {
        return CitizenInfoDataProviderInternal::BuildCitizenSnapshotImpl(
            CitizenName,
            Satisfaction,
            IdentityProfile,
            PoliticalProfile,
            TabIndex);
    }

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex)
    {
        CitizenInfoDataProviderInternal::FTrackedCitizenRequest Request;
        Request.CitizenName = CitizenName;
        Request.SelectedTabIndex = SelectedCitizenTabIndex;

        return CitizenInfoDataProviderInternal::BuildTrackedCitizenSnapshotImpl(
            QuerySource,
            Request);
    }

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex)
    {
        return BuildTrackedCitizenSnapshot(
            CitizenInfoQueryService::CreateWorldQuerySource(World),
            CitizenName,
            SelectedCitizenTabIndex);
    }

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex,
        bool ShowCustomsModeSelection,
        int OperationModeSelectionPageIndex,
        int OverviewMetricScrollOffset)
    {
        CitizenInfoDataProviderInternal::FTrackedBuildingRequest Request;
        Request.BuildingName = BuildingName;
        Request.SelectedTabIndex = SelectedBuildingTabIndex;
        Request.ShowCustomsModeSelection = ShowCustomsModeSelection;
        Request.OperationModeSelectionPageIndex =
            OperationModeSelectionPageIndex;
        Request.OverviewMetricScrollOffset = OverviewMetricScrollOffset;

        return CitizenInfoDataProviderInternal::BuildTrackedBuildingSnapshotImpl(
            QuerySource,
            Request);
    }

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex,
        bool ShowCustomsModeSelection,
        int OperationModeSelectionPageIndex,
        int OverviewMetricScrollOffset)
    {
        return BuildTrackedBuildingSnapshot(
            CitizenInfoQueryService::CreateWorldQuerySource(World),
            BuildingName,
            SelectedBuildingTabIndex,
            ShowCustomsModeSelection,
            OperationModeSelectionPageIndex,
            OverviewMetricScrollOffset);
    }
}

namespace CitizenInfoDataProviderInternal
{
    std::string BuildCatalogIconTextureKey(
        const FBuildingCatalogEntry& Entry)
    {
        return
            "BuildingCatalogIcon_" +
            Entry.Id +
            "_Gen_" +
            std::to_string(::GetRuntimeConfigGeneration());
    }
}
