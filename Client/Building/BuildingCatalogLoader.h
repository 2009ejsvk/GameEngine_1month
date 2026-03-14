#pragma once

#include "BuildingCatalog.h"
#include <string>
#include <vector>

namespace BuildingCatalogLoader
{
    std::vector<std::wstring> BuildCatalogDataCandidatePaths();
    std::vector<std::wstring> BuildProductionRecipeDataCandidatePaths();
    std::vector<std::wstring> BuildCatalogCostOverrideCandidatePaths();
    std::vector<std::wstring> BuildCatalogSourceMetadataOverrideCandidatePaths();
    std::vector<std::wstring> BuildCatalogWorkforceOverrideCandidatePaths();
    std::vector<std::wstring> BuildCatalogPowerOverrideCandidatePaths();
    std::vector<std::wstring> BuildCatalogPollutionOverrideCandidatePaths();
    std::vector<std::wstring> BuildCatalogServiceStatsOverrideCandidatePaths();
    std::vector<std::wstring> BuildCatalogSizeOverrideCandidatePaths();
    std::vector<std::wstring> BuildCatalogOperationModeOverrideCandidatePaths();
    std::vector<std::wstring> BuildCatalogRuntimeUpgradeOverrideCandidatePaths();

    std::vector<FBuildingCatalogEntry> BuildBuildingCatalogEntries();
}
