#include "BuildingCatalogLoader.h"
#include "Asset/PathManager.h"
#include <string>
#include <vector>

namespace
{
    std::wstring GetParentDirectoryPath(const std::wstring& Path)
    {
        if (Path.empty())
            return std::wstring();

        const size_t LastSeparator = Path.find_last_of(L"\\/");
        return LastSeparator == std::wstring::npos ?
            std::wstring() :
            Path.substr(0, LastSeparator);
    }

    std::wstring JoinPath(
        const std::wstring& BasePath,
        const wchar_t* Suffix)
    {
        if (BasePath.empty())
            return std::wstring();

        std::wstring Result = BasePath;

        if (!Suffix || !*Suffix)
            return Result;

        Result += L"\\";
        Result += Suffix;
        return Result;
    }

    std::vector<std::wstring> BuildCatalogCandidatePaths(
        const wchar_t* RepoRelativePath,
        const wchar_t* AssetRelativePath)
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
                Paths.push_back(JoinPath(RepoRoot, RepoRelativePath));
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
            Paths.push_back(JoinPath(AssetPath, AssetRelativePath));

        return Paths;
    }
}

namespace BuildingCatalogLoader
{
    std::vector<std::wstring> BuildCatalogDataCandidatePaths()
    {
        return BuildCatalogCandidatePaths(
            L"Client\\Building\\Data\\BuildingCatalog.tsv",
            L"Data\\BuildingCatalog.tsv");
    }

    std::vector<std::wstring> BuildProductionRecipeDataCandidatePaths()
    {
        return BuildCatalogCandidatePaths(
            L"Client\\Building\\Data\\BuildingProductionRecipes.tsv",
            L"Data\\BuildingProductionRecipes.tsv");
    }

    std::vector<std::wstring> BuildCatalogCostOverrideCandidatePaths()
    {
        return BuildCatalogCandidatePaths(
            L"Client\\Building\\Data\\Tropico6CostOverrides.tsv",
            L"Data\\Tropico6CostOverrides.tsv");
    }

    std::vector<std::wstring> BuildCatalogSourceMetadataOverrideCandidatePaths()
    {
        return BuildCatalogCandidatePaths(
            L"Client\\Building\\Data\\Tropico6SourceMetadataOverrides.tsv",
            L"Data\\Tropico6SourceMetadataOverrides.tsv");
    }

    std::vector<std::wstring> BuildCatalogWorkforceOverrideCandidatePaths()
    {
        return BuildCatalogCandidatePaths(
            L"Client\\Building\\Data\\Tropico6WorkforceOverrides.tsv",
            L"Data\\Tropico6WorkforceOverrides.tsv");
    }

    std::vector<std::wstring> BuildCatalogPowerOverrideCandidatePaths()
    {
        return BuildCatalogCandidatePaths(
            L"Client\\Building\\Data\\Tropico6PowerOverrides.tsv",
            L"Data\\Tropico6PowerOverrides.tsv");
    }

    std::vector<std::wstring> BuildCatalogPollutionOverrideCandidatePaths()
    {
        return BuildCatalogCandidatePaths(
            L"Client\\Building\\Data\\Tropico6PollutionOverrides.tsv",
            L"Data\\Tropico6PollutionOverrides.tsv");
    }

    std::vector<std::wstring> BuildCatalogServiceStatsOverrideCandidatePaths()
    {
        return BuildCatalogCandidatePaths(
            L"Client\\Building\\Data\\Tropico6ServiceStatsOverrides.tsv",
            L"Data\\Tropico6ServiceStatsOverrides.tsv");
    }

    std::vector<std::wstring> BuildCatalogSizeOverrideCandidatePaths()
    {
        return BuildCatalogCandidatePaths(
            L"Client\\Building\\Data\\BuildingSizeOverrides.tsv",
            L"Data\\BuildingSizeOverrides.tsv");
    }

    std::vector<std::wstring> BuildCatalogOperationModeOverrideCandidatePaths()
    {
        return BuildCatalogCandidatePaths(
            L"Client\\Building\\Data\\BuildingOperationModes.tsv",
            L"Data\\BuildingOperationModes.tsv");
    }

    std::vector<std::wstring> BuildCatalogRuntimeUpgradeOverrideCandidatePaths()
    {
        return BuildCatalogCandidatePaths(
            L"Client\\Building\\Data\\BuildingUpgrades.tsv",
            L"Data\\BuildingUpgrades.tsv");
    }
}
