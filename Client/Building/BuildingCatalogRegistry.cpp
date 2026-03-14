#include "BuildingCatalogData.h"
#include "BuildingCatalogLoader.h"
#include "../RuntimeConfigRegistry.h"
#include <Windows.h>
#include <string>
#include <vector>

namespace
{
    constexpr const wchar_t* GCatalogConfigId =
        L"Game.BuildingCatalog";
    constexpr const wchar_t* GProductionRecipeConfigId =
        L"Game.BuildingProductionRecipes";
    constexpr const wchar_t* GCostOverrideConfigId =
        L"Game.BuildingCostOverrides";
    constexpr const wchar_t* GSourceMetadataOverrideConfigId =
        L"Game.BuildingSourceMetadataOverrides";
    constexpr const wchar_t* GWorkforceOverrideConfigId =
        L"Game.BuildingWorkforceOverrides";
    constexpr const wchar_t* GPowerOverrideConfigId =
        L"Game.BuildingPowerOverrides";
    constexpr const wchar_t* GPollutionOverrideConfigId =
        L"Game.BuildingPollutionOverrides";
    constexpr const wchar_t* GServiceStatsOverrideConfigId =
        L"Game.BuildingServiceStatsOverrides";
    constexpr const wchar_t* GSizeOverrideConfigId =
        L"Game.BuildingSizeOverrides";
    constexpr const wchar_t* GOperationModeOverrideConfigId =
        L"Game.BuildingOperationModes";
    constexpr const wchar_t* GRuntimeUpgradeOverrideConfigId =
        L"Game.BuildingUpgrades";

    bool DoesCatalogSourceFileExist(const std::wstring& Path)
    {
        if (Path.empty())
            return false;

        const DWORD Attributes = GetFileAttributesW(Path.c_str());
        return Attributes != INVALID_FILE_ATTRIBUTES &&
            (Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    std::wstring ResolveCatalogWatchPath(
        const std::vector<std::wstring>& CandidatePaths)
    {
        for (size_t Index = 0; Index < CandidatePaths.size(); ++Index)
        {
            if (DoesCatalogSourceFileExist(CandidatePaths[Index]))
                return CandidatePaths[Index];
        }

        return CandidatePaths.empty() ? std::wstring() : CandidatePaths.front();
    }

    void RegisterCatalogRuntimeSource(
        const wchar_t* ConfigId,
        const std::vector<std::wstring>& CandidatePaths)
    {
        RuntimeConfigRegistry::RegisterSource(
            {
                ConfigId,
                ResolveCatalogWatchPath(CandidatePaths),
                0.5f,
                &BuildingCatalogData::ResetRuntimeDefaults,
                nullptr,
                &BuildingCatalogData::ReloadCatalogStore
            });
    }
}

void RegisterRuntimeConfig()
{
    RegisterCatalogRuntimeSource(
        GCatalogConfigId,
        BuildingCatalogLoader::BuildCatalogDataCandidatePaths());
    RegisterCatalogRuntimeSource(
        GProductionRecipeConfigId,
        BuildingCatalogLoader::BuildProductionRecipeDataCandidatePaths());
    RegisterCatalogRuntimeSource(
        GCostOverrideConfigId,
        BuildingCatalogLoader::BuildCatalogCostOverrideCandidatePaths());
    RegisterCatalogRuntimeSource(
        GSourceMetadataOverrideConfigId,
        BuildingCatalogLoader::BuildCatalogSourceMetadataOverrideCandidatePaths());
    RegisterCatalogRuntimeSource(
        GWorkforceOverrideConfigId,
        BuildingCatalogLoader::BuildCatalogWorkforceOverrideCandidatePaths());
    RegisterCatalogRuntimeSource(
        GPowerOverrideConfigId,
        BuildingCatalogLoader::BuildCatalogPowerOverrideCandidatePaths());
    RegisterCatalogRuntimeSource(
        GPollutionOverrideConfigId,
        BuildingCatalogLoader::BuildCatalogPollutionOverrideCandidatePaths());
    RegisterCatalogRuntimeSource(
        GServiceStatsOverrideConfigId,
        BuildingCatalogLoader::BuildCatalogServiceStatsOverrideCandidatePaths());
    RegisterCatalogRuntimeSource(
        GSizeOverrideConfigId,
        BuildingCatalogLoader::BuildCatalogSizeOverrideCandidatePaths());
    RegisterCatalogRuntimeSource(
        GOperationModeOverrideConfigId,
        BuildingCatalogLoader::BuildCatalogOperationModeOverrideCandidatePaths());
    RegisterCatalogRuntimeSource(
        GRuntimeUpgradeOverrideConfigId,
        BuildingCatalogLoader::BuildCatalogRuntimeUpgradeOverrideCandidatePaths());
}
