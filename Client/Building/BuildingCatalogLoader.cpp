#include "BuildingCatalogLoader.h"
#include "Asset/PathManager.h"
#include <string>
#include <vector>

namespace
{
    std::wstring TrimTrailingSeparators(const std::wstring& Path)
    {
        std::wstring Result = Path;

        while (!Result.empty())
        {
            const wchar_t Last = Result.back();

            if (Last != L'\\' && Last != L'/')
                break;

            // Preserve a drive root such as "C:\".
            if (Result.size() == 3 &&
                Result[1] == L':' &&
                (Result[2] == L'\\' || Result[2] == L'/'))
            {
                break;
            }

            Result.pop_back();
        }

        return Result;
    }

    std::wstring GetParentDirectoryPath(const std::wstring& Path)
    {
        const std::wstring NormalizedPath =
            TrimTrailingSeparators(Path);

        if (NormalizedPath.empty())
            return std::wstring();

        const size_t LastSeparator =
            NormalizedPath.find_last_of(L"\\/");
        return LastSeparator == std::wstring::npos ?
            std::wstring() :
            NormalizedPath.substr(0, LastSeparator);
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

    void AppendUniquePath(
        std::vector<std::wstring>& Paths,
        const std::wstring& Path)
    {
        if (Path.empty())
            return;

        if (std::find(Paths.begin(), Paths.end(), Path) != Paths.end())
            return;

        Paths.push_back(Path);
    }

    std::vector<std::wstring> BuildCatalogCandidatePaths(
        const wchar_t* RepoRelativePath,
        const wchar_t* AssetRelativePath)
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring ExeDirectory =
                TrimTrailingSeparators(std::wstring(RootPath));
            const std::wstring RepoRoot =
                GetParentDirectoryPath(ExeDirectory);

            if (!RepoRoot.empty())
            {
                AppendUniquePath(
                    Paths,
                    JoinPath(RepoRoot, RepoRelativePath));
            }

            if (!ExeDirectory.empty())
            {
                // Support packaging runtime data under Binary\Data\...
                // while also handling the repo layout under Client\...
                AppendUniquePath(
                    Paths,
                    JoinPath(ExeDirectory, AssetRelativePath));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            AppendUniquePath(
                Paths,
                JoinPath(
                    TrimTrailingSeparators(std::wstring(AssetPath)),
                    AssetRelativePath));
        }

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
