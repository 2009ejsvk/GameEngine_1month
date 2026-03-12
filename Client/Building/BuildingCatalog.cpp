#include "BuildingCatalog.h"
#include "BuildingCatalogDerived.h"
#include "BuildingCategoryInfo.h"
#include "../RuntimeConfigRegistry.h"
#include "Asset/PathManager.h"
#include <Windows.h>
#include <algorithm>
#include <cctype>
#include <cwchar>
#include <cwctype>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace
{
    std::string WideToUtf8(const std::wstring& Text)
    {
        if (Text.empty())
            return std::string();

        const int RequiredBytes = WideCharToMultiByte(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            nullptr, 0, nullptr, nullptr);

        if (RequiredBytes <= 0)
        {
            std::string Fallback;
            Fallback.reserve(Text.size());

            for (size_t i = 0; i < Text.size(); ++i)
            {
                const wchar_t Ch = Text[i];

                if (Ch >= 0 && Ch <= 0x7f)
                    Fallback.push_back(static_cast<char>(Ch));
                else
                    Fallback.push_back('?');
            }

            return Fallback;
        }

        std::string Utf8;
        Utf8.resize(RequiredBytes);
        WideCharToMultiByte(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            &Utf8[0], RequiredBytes, nullptr, nullptr);
        return Utf8;
    }
} // namespace

namespace
{
    std::string TrimAsciiWhitespace(
        const std::string& Text)
    {
        size_t Begin = 0;
        size_t End = Text.size();

        while (Begin < End &&
            (Text[Begin] == ' ' || Text[Begin] == '\t' ||
                Text[Begin] == '\r' || Text[Begin] == '\n'))
        {
            ++Begin;
        }

        while (End > Begin &&
            (Text[End - 1] == ' ' || Text[End - 1] == '\t' ||
                Text[End - 1] == '\r' || Text[End - 1] == '\n'))
        {
            --End;
        }

        return Text.substr(Begin, End - Begin);
    }

    bool EqualsIgnoreCaseAscii(
        const std::string& A,
        const char* B)
    {
        const size_t Length = A.size();

        for (size_t Index = 0; Index < Length; ++Index)
        {
            const char Left = static_cast<char>(
                std::tolower(static_cast<unsigned char>(A[Index])));
            const char Right = static_cast<char>(
                std::tolower(static_cast<unsigned char>(B[Index])));

            if (Right == 0 || Left != Right)
                return false;
        }

        return B[Length] == 0;
    }

    bool TryParsePlacementTemplateTypeKey(
        const std::string& Key,
        EPlacementTemplateType& OutTemplateType)
    {
        const std::string Normalized = TrimAsciiWhitespace(Key);

        if (Normalized.empty())
            return false;

        if (EqualsIgnoreCaseAscii(Normalized, "SingleTileMarker") ||
            EqualsIgnoreCaseAscii(Normalized, "1x1"))
        {
            OutTemplateType = EPlacementTemplateType::SingleTileMarker;
        }
        else if (
            EqualsIgnoreCaseAscii(
                Normalized,
                "Diamond3x3SingleMarker") ||
            EqualsIgnoreCaseAscii(Normalized, "3x3"))
        {
            OutTemplateType =
                EPlacementTemplateType::Diamond3x3SingleMarker;
        }
        else if (
            EqualsIgnoreCaseAscii(
                Normalized,
                "Diamond5x5TwoMarker") ||
            EqualsIgnoreCaseAscii(Normalized, "5x5TwoMarker"))
        {
            OutTemplateType =
                EPlacementTemplateType::Diamond5x5TwoMarker;
        }
        else if (
            EqualsIgnoreCaseAscii(
                Normalized,
                "Diamond5x5FourMarker") ||
            EqualsIgnoreCaseAscii(Normalized, "5x5FourMarker"))
        {
            OutTemplateType =
                EPlacementTemplateType::Diamond5x5FourMarker;
        }
        else if (
            EqualsIgnoreCaseAscii(
                Normalized,
                "Diamond7x7ThreeMarker") ||
            EqualsIgnoreCaseAscii(Normalized, "7x7ThreeMarker"))
        {
            OutTemplateType =
                EPlacementTemplateType::Diamond7x7ThreeMarker;
        }
        else
        {
            return false;
        }

        return true;
    }

    struct FExternalCatalogRecord
    {
        EBuildingCategory Category = EBuildingCategory::Infrastructure;
        int LocalIndex = 0;
        std::wstring DisplayName;
        EBuildingCostState BlueprintCostState =
            EBuildingCostState::None;
        int BlueprintCost = 0;
        EBuildingCostState ConstructionCostState =
            EBuildingCostState::None;
        int ConstructionCost = 0;
        bool HasTemplateType = false;
        EPlacementTemplateType TemplateType =
            EPlacementTemplateType::Diamond3x3SingleMarker;
        std::wstring IconPath;
        std::wstring SpriteTexturePath;
        std::wstring DetailText;
    };

    struct FProductionRecipeRecord
    {
        std::string BuildingId;
        EResourceType ProducedResourceType = EResourceType::None;
        std::wstring ProducedResourceLabel;
        EResourceType VisitConsumptionResourceType = EResourceType::None;
        std::array<EResourceType, GProductionInputSlotCount>
            ProductionInputTypes =
            {
                EResourceType::None,
                EResourceType::None
            };
        std::array<int, GProductionInputSlotCount>
            ProductionInputAmounts = {};
        std::array<std::wstring, GProductionInputSlotCount>
            ProductionInputLabels = {};
    };

    struct FCatalogCostOverrideRecord
    {
        std::string BuildingId;
        EBuildingCostState BlueprintCostState =
            EBuildingCostState::None;
        int BlueprintCost = 0;
        EBuildingCostState ConstructionCostState =
            EBuildingCostState::None;
        int ConstructionCost = 0;
        std::wstring SourceAssetName;
        std::wstring SourceDisplayName;
    };

    struct FCatalogSourceMetadataOverrideRecord
    {
        std::string BuildingId;
        bool HasUnlockEra = false;
        EBuildingEra UnlockEra = EBuildingEra::Colonial;
        bool HasBuildMenuCategoryOverride = false;
        EBuildingCategory BuildMenuCategoryOverride =
            EBuildingCategory::Infrastructure;
        std::wstring SourceAssetName;
        std::wstring SourceDisplayName;
    };

    struct FCatalogWorkforceOverrideRecord
    {
        std::string BuildingId;
        bool HasCapacity = false;
        int Capacity = 0;
        bool HasRequiredEducationLevel = false;
        ECitizenEducationLevel RequiredEducationLevel =
            ECitizenEducationLevel::Uneducated;
        std::wstring SourceAssetName;
        std::wstring SourceDisplayName;
    };

    struct FCatalogPowerOverrideRecord
    {
        std::string BuildingId;
        bool HasProducedPowerMW = false;
        int ProducedPowerMW = 0;
        bool HasRequiredPowerMW = false;
        int RequiredPowerMW = 0;
        std::wstring SourceDisplayName;
    };

    struct FCatalogPollutionOverrideRecord
    {
        std::string BuildingId;
        bool HasPollutionOutput = false;
        int PollutionOutput = 0;
        bool HasPollutionMitigation = false;
        int PollutionMitigation = 0;
        std::wstring SourceDisplayName;
    };

    struct FCatalogServiceStatsOverrideRecord
    {
        std::string BuildingId;
        bool HasCapacity = false;
        int Capacity = 0;
        bool HasServiceCapacity = false;
        int ServiceCapacity = 0;
        bool HasHouseholdCapacity = false;
        int HouseholdCapacity = 0;
        bool HasBaseJobQuality = false;
        int BaseJobQuality = 0;
        bool HasBaseServiceQuality = false;
        int BaseServiceQuality = 0;
        bool HasBaseHousingQuality = false;
        int BaseHousingQuality = 0;
        bool HasAllowedWealthMask = false;
        unsigned int AllowedWealthMask = GBuildingWealthMaskAll;
        bool HasTouristPreference = false;
        ETouristPreference TouristPreference =
            ETouristPreference::None;
        bool HasSize = false;
        int SizeX = 1;
        int SizeY = 1;
        std::wstring SourceDisplayName;
    };

    struct FCatalogSizeOverrideRecord
    {
        std::string BuildingId;
        int SizeX = 1;
        int SizeY = 1;
        std::wstring SourceDisplayName;
    };

    struct FCatalogOperationModeOverrideRecord
    {
        std::string BuildingId;
        int ModeIndex = 0;
        std::wstring DisplayName;
        std::wstring EffectSummary;
        bool HasProductionMultiplier = false;
        float ProductionMultiplier = 1.f;
        bool HasInputConsumptionMultiplier = false;
        float InputConsumptionMultiplier = 1.f;
        bool HasServiceThroughputMultiplier = false;
        float ServiceThroughputMultiplier = 1.f;
        bool HasPollutionMultiplier = false;
        float PollutionMultiplier = 1.f;
        bool HasWageMultiplier = false;
        float WageMultiplier = 1.f;
        bool HasUpkeepMultiplier = false;
        float UpkeepMultiplier = 1.f;
        bool HasExportPriceDeltaPercent = false;
        int ExportPriceDeltaPercent = 0;
        bool HasImportPriceDeltaPercent = false;
        int ImportPriceDeltaPercent = 0;
        bool HasCapacityDelta = false;
        int CapacityDelta = 0;
        bool HasServiceCapacityDelta = false;
        int ServiceCapacityDelta = 0;
        bool HasHousingQualityDelta = false;
        int HousingQualityDelta = 0;
        bool HasJobQualityDelta = false;
        int JobQualityDelta = 0;
        bool HasGenericServiceQualityDelta = false;
        int GenericServiceQualityDelta = 0;
        bool HasUnlockEra = false;
        EBuildingEra UnlockEra = EBuildingEra::Colonial;
        std::wstring RequiredResearch;
        std::wstring SourceDisplayName;
    };

    struct FCatalogRuntimeUpgradeOverrideRecord
    {
        std::string BuildingId;
        int UpgradeIndex = 0;
        std::wstring DisplayName;
        std::wstring EffectSummary;
        bool HasUnlockEra = false;
        EBuildingEra UnlockEra = EBuildingEra::Colonial;
        EBuildingCostState CostState = EBuildingCostState::None;
        int Cost = 0;
        bool HasProductionMultiplier = false;
        float ProductionMultiplier = 1.f;
        bool HasInputConsumptionMultiplier = false;
        float InputConsumptionMultiplier = 1.f;
        bool HasUpkeepMultiplier = false;
        float UpkeepMultiplier = 1.f;
        bool HasWarehouseSlotCapacityMultiplier = false;
        float WarehouseSlotCapacityMultiplier = 1.f;
        bool HasCapacityDelta = false;
        int CapacityDelta = 0;
        bool HasServiceCapacityDelta = false;
        int ServiceCapacityDelta = 0;
        bool HasPerWorkerServiceCapacityDelta = false;
        int PerWorkerServiceCapacityDelta = 0;
        bool HasHousingQualityDelta = false;
        int HousingQualityDelta = 0;
        bool HasJobQualityDelta = false;
        int JobQualityDelta = 0;
        bool HasGenericServiceQualityDelta = false;
        int GenericServiceQualityDelta = 0;
        bool HasRequiredPowerDeltaMW = false;
        int RequiredPowerDeltaMW = 0;
        bool HasPollutionMultiplier = false;
        float PollutionMultiplier = 1.f;
        bool HasUpkeepFlatDelta = false;
        int UpkeepFlatDelta = 0;
        bool HasWarehouseSlotCapacityDelta = false;
        int WarehouseSlotCapacityDelta = 0;
        std::wstring SourceDisplayName;
    };

    bool TryParseCatalogCostText(
        const std::wstring& Text,
        EBuildingCostState& OutState,
        int& OutCost);

    bool TryExtractDetailCost(
        const std::wstring& DetailText,
        const wchar_t* Prefix,
        EBuildingCostState& OutState,
        int& OutCost);

    bool TryParseSignedInteger(
        const std::wstring& Text,
        int& OutValue);

    bool TryParseFirstFloat(
        const std::wstring& Text,
        double& OutValue);

    std::wstring Utf8ToWide(const std::string& Text)
    {
        if (Text.empty())
            return std::wstring();

        const int RequiredCount = MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            nullptr, 0);

        if (RequiredCount <= 0)
            return std::wstring(Text.begin(), Text.end());

        std::wstring WideText;
        WideText.resize(RequiredCount);
        MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            &WideText[0], RequiredCount);
        return WideText;
    }

    std::wstring TrimTrailingSeparators(const std::wstring& Path)
    {
        std::wstring Result = Path;

        while (!Result.empty() &&
            (Result.back() == L'\\' || Result.back() == L'/'))
        {
            Result.pop_back();
        }

        return Result;
    }

    std::wstring GetParentDirectoryPath(const std::wstring& Path)
    {
        const std::wstring TrimmedPath = TrimTrailingSeparators(Path);
        const size_t SlashPos = TrimmedPath.find_last_of(L"\\/");

        if (SlashPos == std::wstring::npos)
            return std::wstring();

        return TrimmedPath.substr(0, SlashPos);
    }

    std::wstring JoinPath(
        const std::wstring& BasePath,
        const wchar_t* Suffix)
    {
        if (BasePath.empty())
            return std::wstring(Suffix ? Suffix : L"");

        std::wstring Result = TrimTrailingSeparators(BasePath);

        if (!Suffix || !*Suffix)
            return Result;

        Result += L"\\";
        Result += Suffix;
        return Result;
    }

    std::vector<std::wstring> BuildCatalogDataCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\BuildingCatalog.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\BuildingCatalog.tsv"));
        }

        return Paths;
    }

    std::vector<std::wstring> BuildProductionRecipeDataCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\BuildingProductionRecipes.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\BuildingProductionRecipes.tsv"));
        }

        return Paths;
    }

    std::vector<std::wstring> BuildCatalogCostOverrideCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\Tropico6CostOverrides.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\Tropico6CostOverrides.tsv"));
        }

        return Paths;
    }

    std::vector<std::wstring> BuildCatalogSourceMetadataOverrideCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\Tropico6SourceMetadataOverrides.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\Tropico6SourceMetadataOverrides.tsv"));
        }

        return Paths;
    }

    std::vector<std::wstring> BuildCatalogWorkforceOverrideCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\Tropico6WorkforceOverrides.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\Tropico6WorkforceOverrides.tsv"));
        }

        return Paths;
    }

    std::vector<std::wstring> BuildCatalogPowerOverrideCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\Tropico6PowerOverrides.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\Tropico6PowerOverrides.tsv"));
        }

        return Paths;
    }

    std::vector<std::wstring> BuildCatalogPollutionOverrideCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\Tropico6PollutionOverrides.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\Tropico6PollutionOverrides.tsv"));
        }

        return Paths;
    }

    std::vector<std::wstring> BuildCatalogServiceStatsOverrideCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\Tropico6ServiceStatsOverrides.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\Tropico6ServiceStatsOverrides.tsv"));
        }

        return Paths;
    }

    std::vector<std::wstring> BuildCatalogSizeOverrideCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\BuildingSizeOverrides.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\BuildingSizeOverrides.tsv"));
        }

        return Paths;
    }

    std::vector<std::wstring> BuildCatalogOperationModeOverrideCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\BuildingOperationModes.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\BuildingOperationModes.tsv"));
        }

        return Paths;
    }

    std::vector<std::wstring> BuildCatalogRuntimeUpgradeOverrideCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\BuildingUpgrades.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\BuildingUpgrades.tsv"));
        }

        return Paths;
    }

    bool LoadUtf8TextFile(
        const std::wstring& FullPath,
        std::string& OutText)
    {
        OutText.clear();

        FILE* File = nullptr;
        _wfopen_s(&File, FullPath.c_str(), L"rb");

        if (!File)
            return false;

        fseek(File, 0, SEEK_END);
        const long FileSize = ftell(File);
        fseek(File, 0, SEEK_SET);

        if (FileSize < 0)
        {
            fclose(File);
            return false;
        }

        OutText.resize(static_cast<size_t>(FileSize));

        if (FileSize > 0)
        {
            const size_t ReadSize = fread(
                &OutText[0],
                1,
                static_cast<size_t>(FileSize),
                File);

            if (ReadSize != static_cast<size_t>(FileSize))
            {
                fclose(File);
                OutText.clear();
                return false;
            }
        }

        fclose(File);

        if (OutText.size() >= 3 &&
            static_cast<unsigned char>(OutText[0]) == 0xEF &&
            static_cast<unsigned char>(OutText[1]) == 0xBB &&
            static_cast<unsigned char>(OutText[2]) == 0xBF)
        {
            OutText.erase(0, 3);
        }

        return true;
    }

    std::vector<std::string> SplitTabSeparatedLine(const std::string& Line)
    {
        std::vector<std::string> Fields;
        std::string Current;

        for (size_t Index = 0; Index < Line.size(); ++Index)
        {
            const char Ch = Line[Index];

            if (Ch == '\t')
            {
                Fields.push_back(Current);
                Current.clear();
                continue;
            }

            Current.push_back(Ch);
        }

        Fields.push_back(Current);
        return Fields;
    }

    std::string UnescapeCatalogField(const std::string& Text)
    {
        std::string Result;
        Result.reserve(Text.size());

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const char Ch = Text[Index];

            if (Ch != '\\' || Index + 1 >= Text.size())
            {
                Result.push_back(Ch);
                continue;
            }

            const char Next = Text[++Index];

            switch (Next)
            {
            case 'n':
                Result.push_back('\n');
                break;
            case 'r':
                Result.push_back('\r');
                break;
            case 't':
                Result.push_back('\t');
                break;
            case '\\':
                Result.push_back('\\');
                break;
            default:
                Result.push_back(Next);
                break;
            }
        }

        return Result;
    }

    bool TryParseBuildingCategoryKey(
        const std::string& Key,
        EBuildingCategory& OutCategory)
    {
        if (Key == "Infrastructure")
            OutCategory = EBuildingCategory::Infrastructure;
        else if (Key == "FoodResource")
            OutCategory = EBuildingCategory::FoodResource;
        else if (Key == "Industry")
            OutCategory = EBuildingCategory::Industry;
        else if (Key == "Housing")
            OutCategory = EBuildingCategory::Housing;
        else if (Key == "Entertainment")
            OutCategory = EBuildingCategory::Entertainment;
        else if (Key == "MediaEducation")
            OutCategory = EBuildingCategory::MediaEducation;
        else if (Key == "Tourism")
            OutCategory = EBuildingCategory::Tourism;
        else if (Key == "PublicService")
            OutCategory = EBuildingCategory::PublicService;
        else if (Key == "LuxuryEntertainment")
            OutCategory = EBuildingCategory::LuxuryEntertainment;
        else if (Key == "Military")
            OutCategory = EBuildingCategory::Military;
        else if (Key == "GovernmentFinance")
            OutCategory = EBuildingCategory::GovernmentFinance;
        else
            return false;

        return true;
    }

    bool TryParseBuildingEraKey(
        const std::string& Key,
        EBuildingEra& OutEra)
    {
        if (Key == "Colonial")
            OutEra = EBuildingEra::Colonial;
        else if (Key == "WorldWars" || Key == "World Wars")
            OutEra = EBuildingEra::WorldWars;
        else if (Key == "ColdWar" || Key == "Cold War")
            OutEra = EBuildingEra::ColdWar;
        else if (
            Key == "Modern" ||
            Key == "ModernTimes" ||
            Key == "Modern Times")
        {
            OutEra = EBuildingEra::Modern;
        }
        else
        {
            return false;
        }

        return true;
    }

    bool TryParseEducationLevelKey(
        const std::string& Key,
        ECitizenEducationLevel& OutLevel)
    {
        if (
            Key.empty() ||
            Key == "Uneducated" ||
            Key == "None")
        {
            OutLevel = ECitizenEducationLevel::Uneducated;
        }
        else if (Key == "HighSchool")
        {
            OutLevel = ECitizenEducationLevel::HighSchool;
        }
        else if (Key == "College")
        {
            OutLevel = ECitizenEducationLevel::College;
        }
        else
        {
            return false;
        }

        return true;
    }

    bool TryParseCatalogIntegerField(
        const std::string& Field,
        int& OutValue)
    {
        const std::wstring ValueText = Utf8ToWide(
            UnescapeCatalogField(Field));

        if (ValueText.empty())
            return false;

        return TryParseSignedInteger(ValueText, OutValue);
    }

    bool TryParseCatalogFloatField(
        const std::string& Field,
        float& OutValue)
    {
        const std::wstring ValueText = Utf8ToWide(
            UnescapeCatalogField(Field));

        if (ValueText.empty())
            return false;

        double ParsedValue = 0.0;

        if (!TryParseFirstFloat(ValueText, ParsedValue))
            return false;

        OutValue = static_cast<float>(ParsedValue);
        return true;
    }

    unsigned int NormalizeAllowedWealthMaskOverride(int RawMask)
    {
        if (RawMask <= 0)
            return GBuildingWealthMaskAll;

        unsigned int Normalized = GBuildingWealthMaskNone;

        if ((RawMask & 1) != 0)
        {
            Normalized |=
                GBuildingWealthMaskPoor |
                GBuildingWealthMaskWellOff |
                GBuildingWealthMaskRich;
        }

        if ((RawMask & 2) != 0)
            Normalized |= GBuildingWealthMaskWellOff | GBuildingWealthMaskRich;

        if ((RawMask & 4) != 0)
            Normalized |= GBuildingWealthMaskRich;

        // 현재 엔진은 부/중산층/서민 3단계만 사용하므로
        // "더럽게 부유"는 최고 티어인 Rich 로 접어서 반영한다.
        if ((RawMask & 8) != 0)
            Normalized |= GBuildingWealthMaskRich;

        return Normalized == GBuildingWealthMaskNone ?
            GBuildingWealthMaskAll :
            Normalized;
    }

    bool TryParseTouristPreferenceKey(
        const std::string& Key,
        ETouristPreference& OutPreference)
    {
        const std::string Normalized = TrimAsciiWhitespace(Key);

        if (Normalized.empty() ||
            EqualsIgnoreCaseAscii(Normalized, "None"))
        {
            OutPreference = ETouristPreference::None;
        }
        else if (
            EqualsIgnoreCaseAscii(Normalized, "Cultural") ||
            Normalized == "문화")
        {
            OutPreference = ETouristPreference::Cultural;
        }
        else if (
            EqualsIgnoreCaseAscii(Normalized, "Family") ||
            Normalized == "아동")
        {
            OutPreference = ETouristPreference::Family;
        }
        else if (
            EqualsIgnoreCaseAscii(Normalized, "Backpacker") ||
            Normalized == "배낭여행")
        {
            OutPreference = ETouristPreference::Backpacker;
        }
        else if (
            EqualsIgnoreCaseAscii(Normalized, "Relaxation") ||
            Normalized == "휴양")
        {
            OutPreference = ETouristPreference::Relaxation;
        }
        else if (
            EqualsIgnoreCaseAscii(Normalized, "ThrillSeeker") ||
            EqualsIgnoreCaseAscii(Normalized, "Thrill Seeker") ||
            Normalized == "스릴중독")
        {
            OutPreference = ETouristPreference::ThrillSeeker;
        }
        else if (
            EqualsIgnoreCaseAscii(Normalized, "Celebrity") ||
            Normalized == "유명인")
        {
            OutPreference = ETouristPreference::Celebrity;
        }
        else
        {
            return false;
        }

        return true;
    }

    bool TryParseResourceTypeKey(
        const std::string& Key,
        EResourceType& OutType)
    {
        if (Key.empty() || Key == "-" || Key == "None")
        {
            OutType = EResourceType::None;
            return true;
        }

        if (Key == "Coconuts")
            OutType = EResourceType::Coconuts;
        else if (Key == "Logs")
            OutType = EResourceType::Logs;
        else if (Key == "Fish")
            OutType = EResourceType::Fish;
        else if (Key == "Crops")
            OutType = EResourceType::Crops;
        else if (Key == "AnimalProducts")
            OutType = EResourceType::AnimalProducts;
        else if (Key == "Ore")
            OutType = EResourceType::Ore;
        else if (Key == "Oil")
            OutType = EResourceType::Oil;
        else if (Key == "FarmedFish")
            OutType = EResourceType::FarmedFish;
        else if (Key == "HydroponicProduce")
            OutType = EResourceType::HydroponicProduce;
        else if (Key == "FactoryLivestock")
            OutType = EResourceType::FactoryLivestock;
        else if (Key == "Planks")
            OutType = EResourceType::Planks;
        else if (Key == "Rum")
            OutType = EResourceType::Rum;
        else if (Key == "Leather")
            OutType = EResourceType::Leather;
        else if (Key == "CannedGoods")
            OutType = EResourceType::CannedGoods;
        else if (Key == "Cheese")
            OutType = EResourceType::Cheese;
        else if (Key == "Cigars")
            OutType = EResourceType::Cigars;
        else if (Key == "Boats")
            OutType = EResourceType::Boats;
        else if (Key == "Steel")
            OutType = EResourceType::Steel;
        else if (Key == "Textiles")
            OutType = EResourceType::Textiles;
        else if (Key == "Weapons")
            OutType = EResourceType::Weapons;
        else if (Key == "Chocolate")
            OutType = EResourceType::Chocolate;
        else if (Key == "Furniture")
            OutType = EResourceType::Furniture;
        else if (Key == "Jewelry")
            OutType = EResourceType::Jewelry;
        else if (Key == "Plastic")
            OutType = EResourceType::Plastic;
        else if (Key == "Cars")
            OutType = EResourceType::Cars;
        else if (Key == "Electronics")
            OutType = EResourceType::Electronics;
        else if (Key == "Apparel")
            OutType = EResourceType::Apparel;
        else if (Key == "Pharmaceuticals")
            OutType = EResourceType::Pharmaceuticals;
        else if (Key == "Juice")
            OutType = EResourceType::Juice;
        else
            return false;

        return true;
    }

    bool LoadExternalCatalogRecords(
        std::vector<FExternalCatalogRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildCatalogDataCandidatePaths();

        for (size_t PathIndex = 0; PathIndex < CandidatePaths.size(); ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (Fields.size() >= 4)
                    {
                        EBuildingCategory Category;
                        char* EndPtr = nullptr;
                        const long ParsedIndex =
                            strtol(Fields[1].c_str(), &EndPtr, 10);

                        if (TryParseBuildingCategoryKey(Fields[0], Category) &&
                            EndPtr && *EndPtr == 0 &&
                            ParsedIndex >= 0)
                        {
                            FExternalCatalogRecord Record;
                            Record.Category = Category;
                            Record.LocalIndex =
                                static_cast<int>(ParsedIndex);
                            Record.DisplayName = Utf8ToWide(
                                UnescapeCatalogField(Fields[2]));

                            if (Fields.size() >= 9)
                            {
                                TryParseCatalogCostText(
                                    Utf8ToWide(UnescapeCatalogField(Fields[3])),
                                    Record.BlueprintCostState,
                                    Record.BlueprintCost);
                                TryParseCatalogCostText(
                                    Utf8ToWide(UnescapeCatalogField(Fields[4])),
                                    Record.ConstructionCostState,
                                    Record.ConstructionCost);

                                EPlacementTemplateType ParsedTemplateType =
                                    EPlacementTemplateType::
                                        Diamond3x3SingleMarker;

                                if (TryParsePlacementTemplateTypeKey(
                                        UnescapeCatalogField(Fields[5]),
                                        ParsedTemplateType))
                                {
                                    Record.HasTemplateType = true;
                                    Record.TemplateType = ParsedTemplateType;
                                }

                                Record.IconPath = Utf8ToWide(
                                    UnescapeCatalogField(Fields[6]));
                                Record.SpriteTexturePath = Utf8ToWide(
                                    UnescapeCatalogField(Fields[7]));
                                Record.DetailText = Utf8ToWide(
                                    UnescapeCatalogField(Fields[8]));
                            }
                            else if (Fields.size() >= 8)
                            {
                                TryParseCatalogCostText(
                                    Utf8ToWide(UnescapeCatalogField(Fields[3])),
                                    Record.BlueprintCostState,
                                    Record.BlueprintCost);
                                TryParseCatalogCostText(
                                    Utf8ToWide(UnescapeCatalogField(Fields[4])),
                                    Record.ConstructionCostState,
                                    Record.ConstructionCost);

                                EPlacementTemplateType ParsedTemplateType =
                                    EPlacementTemplateType::
                                        Diamond3x3SingleMarker;

                                if (TryParsePlacementTemplateTypeKey(
                                        UnescapeCatalogField(Fields[5]),
                                        ParsedTemplateType))
                                {
                                    Record.HasTemplateType = true;
                                    Record.TemplateType = ParsedTemplateType;
                                }

                                Record.IconPath = Utf8ToWide(
                                    UnescapeCatalogField(Fields[6]));
                                Record.DetailText = Utf8ToWide(
                                    UnescapeCatalogField(Fields[7]));
                            }
                            else if (Fields.size() >= 7)
                            {
                                TryParseCatalogCostText(
                                    Utf8ToWide(UnescapeCatalogField(Fields[3])),
                                    Record.BlueprintCostState,
                                    Record.BlueprintCost);
                                TryParseCatalogCostText(
                                    Utf8ToWide(UnescapeCatalogField(Fields[4])),
                                    Record.ConstructionCostState,
                                    Record.ConstructionCost);

                                EPlacementTemplateType ParsedTemplateType =
                                    EPlacementTemplateType::
                                        Diamond3x3SingleMarker;

                                if (TryParsePlacementTemplateTypeKey(
                                        UnescapeCatalogField(Fields[5]),
                                        ParsedTemplateType))
                                {
                                    Record.HasTemplateType = true;
                                    Record.TemplateType = ParsedTemplateType;
                                }

                                Record.DetailText = Utf8ToWide(
                                    UnescapeCatalogField(Fields[6]));
                            }
                            else if (Fields.size() >= 6)
                            {
                                TryParseCatalogCostText(
                                    Utf8ToWide(UnescapeCatalogField(Fields[3])),
                                    Record.BlueprintCostState,
                                    Record.BlueprintCost);
                                TryParseCatalogCostText(
                                    Utf8ToWide(UnescapeCatalogField(Fields[4])),
                                    Record.ConstructionCostState,
                                    Record.ConstructionCost);
                                Record.DetailText = Utf8ToWide(
                                    UnescapeCatalogField(Fields[5]));
                            }
                            else
                            {
                                Record.DetailText = Utf8ToWide(
                                    UnescapeCatalogField(Fields[3]));
                            }

                            if (Record.BlueprintCostState ==
                                EBuildingCostState::None)
                            {
                                TryExtractDetailCost(
                                    Record.DetailText,
                                    L"설계도 비용:",
                                    Record.BlueprintCostState,
                                    Record.BlueprintCost);
                            }

                            if (Record.ConstructionCostState ==
                                EBuildingCostState::None)
                            {
                                TryExtractDetailCost(
                                    Record.DetailText,
                                    L"건설 비용:",
                                    Record.ConstructionCostState,
                                    Record.ConstructionCost);
                            }
                            OutRecords.push_back(Record);
                        }
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            if (!OutRecords.empty())
            {
                std::stable_sort(
                    OutRecords.begin(),
                    OutRecords.end(),
                    [](const FExternalCatalogRecord& A,
                        const FExternalCatalogRecord& B)
                    {
                        if (A.Category != B.Category)
                        {
                            return static_cast<int>(A.Category) <
                                static_cast<int>(B.Category);
                        }

                        return A.LocalIndex < B.LocalIndex;
                    });
                return true;
            }
        }

        OutputDebugStringW(
            L"[BuildingCatalog] Failed to load BuildingCatalog.tsv\n");
        return false;
    }

    bool LoadProductionRecipeRecords(
        std::vector<FProductionRecipeRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildProductionRecipeDataCandidatePaths();

        for (size_t PathIndex = 0;
            PathIndex < CandidatePaths.size();
            ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (Fields.size() >= 3 && !Fields[0].empty())
                    {
                        FProductionRecipeRecord Record;
                        Record.BuildingId = Fields[0];
                        if (!TryParseResourceTypeKey(
                                Fields[1],
                                Record.ProducedResourceType))
                        {
                            continue;
                        }

                        Record.ProducedResourceLabel = Utf8ToWide(
                            UnescapeCatalogField(Fields[2]));

                        for (int SlotIndex = 0;
                            SlotIndex < GProductionInputSlotCount;
                            ++SlotIndex)
                        {
                            const size_t BaseFieldIndex =
                                3 + static_cast<size_t>(SlotIndex) * 3;

                            if (BaseFieldIndex >= Fields.size())
                                break;

                            EResourceType InputType =
                                EResourceType::None;

                            if (!TryParseResourceTypeKey(
                                    Fields[BaseFieldIndex],
                                    InputType))
                            {
                                continue;
                            }

                            int InputAmount = 0;

                            if (BaseFieldIndex + 1 < Fields.size() &&
                                !Fields[BaseFieldIndex + 1].empty())
                            {
                                char* EndPtr = nullptr;
                                const long ParsedAmount = strtol(
                                    Fields[BaseFieldIndex + 1].c_str(),
                                    &EndPtr,
                                    10);

                                if (EndPtr && *EndPtr == 0 &&
                                    ParsedAmount > 0)
                                {
                                    InputAmount =
                                        static_cast<int>(ParsedAmount);
                                }
                            }

                            if (InputType == EResourceType::None ||
                                InputAmount <= 0)
                            {
                                continue;
                            }

                            Record.ProductionInputTypes[SlotIndex] =
                                InputType;
                            Record.ProductionInputAmounts[SlotIndex] =
                                InputAmount;

                            if (BaseFieldIndex + 2 < Fields.size())
                            {
                                Record.ProductionInputLabels[SlotIndex] =
                                    Utf8ToWide(
                                        UnescapeCatalogField(
                                            Fields[BaseFieldIndex + 2]));
                            }
                        }

                        if (Fields.size() >= 10)
                        {
                            TryParseResourceTypeKey(
                                Fields[9],
                                Record.VisitConsumptionResourceType);
                        }

                        const bool HasRecipeData =
                            Record.ProducedResourceType !=
                                EResourceType::None ||
                            Record.VisitConsumptionResourceType !=
                                EResourceType::None ||
                            Record.ProductionInputTypes[0] !=
                                EResourceType::None ||
                            Record.ProductionInputTypes[1] !=
                                EResourceType::None;

                        if (HasRecipeData)
                            OutRecords.push_back(std::move(Record));
                    }
                }
                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            if (!OutRecords.empty())
            {
                std::stable_sort(
                    OutRecords.begin(),
                    OutRecords.end(),
                    [](const FProductionRecipeRecord& A,
                        const FProductionRecipeRecord& B)
                    {
                        return A.BuildingId < B.BuildingId;
                    });
                return true;
            }
        }

        return false;
    }

    bool LoadCatalogCostOverrideRecords(
        std::vector<FCatalogCostOverrideRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildCatalogCostOverrideCandidatePaths();

        for (size_t PathIndex = 0;
            PathIndex < CandidatePaths.size();
            ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (!Fields.empty() && !Fields[0].empty())
                    {
                        FCatalogCostOverrideRecord Record;
                        Record.BuildingId = Fields[0];

                        if (Fields.size() >= 2)
                        {
                            TryParseCatalogCostText(
                                Utf8ToWide(UnescapeCatalogField(Fields[1])),
                                Record.BlueprintCostState,
                                Record.BlueprintCost);
                        }

                        if (Fields.size() >= 3)
                        {
                            TryParseCatalogCostText(
                                Utf8ToWide(UnescapeCatalogField(Fields[2])),
                                Record.ConstructionCostState,
                                Record.ConstructionCost);
                        }

                        if (Fields.size() >= 4)
                        {
                            Record.SourceAssetName = Utf8ToWide(
                                UnescapeCatalogField(Fields[3]));
                        }

                        if (Fields.size() >= 5)
                        {
                            Record.SourceDisplayName = Utf8ToWide(
                                UnescapeCatalogField(Fields[4]));
                        }

                        if (Record.BlueprintCostState !=
                                EBuildingCostState::None ||
                            Record.ConstructionCostState !=
                                EBuildingCostState::None)
                        {
                            OutRecords.push_back(std::move(Record));
                        }
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            std::stable_sort(
                OutRecords.begin(),
                OutRecords.end(),
                [](const FCatalogCostOverrideRecord& A,
                    const FCatalogCostOverrideRecord& B)
                {
                    return A.BuildingId < B.BuildingId;
                });
            return true;
        }

        return false;
    }

    bool LoadCatalogSourceMetadataOverrideRecords(
        std::vector<FCatalogSourceMetadataOverrideRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildCatalogSourceMetadataOverrideCandidatePaths();

        for (size_t PathIndex = 0;
            PathIndex < CandidatePaths.size();
            ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (!Fields.empty() && !Fields[0].empty())
                    {
                        FCatalogSourceMetadataOverrideRecord Record;
                        Record.BuildingId = Fields[0];

                        if (Fields.size() >= 2)
                        {
                            Record.HasUnlockEra = TryParseBuildingEraKey(
                                UnescapeCatalogField(Fields[1]),
                                Record.UnlockEra);
                        }

                        if (Fields.size() >= 3)
                        {
                            Record.HasBuildMenuCategoryOverride =
                                TryParseBuildingCategoryKey(
                                    UnescapeCatalogField(Fields[2]),
                                    Record.BuildMenuCategoryOverride);
                        }

                        if (Fields.size() >= 4)
                        {
                            Record.SourceAssetName = Utf8ToWide(
                                UnescapeCatalogField(Fields[3]));
                        }

                        if (Fields.size() >= 5)
                        {
                            Record.SourceDisplayName = Utf8ToWide(
                                UnescapeCatalogField(Fields[4]));
                        }

                        if (Record.HasUnlockEra ||
                            Record.HasBuildMenuCategoryOverride)
                        {
                            OutRecords.push_back(std::move(Record));
                        }
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            std::stable_sort(
                OutRecords.begin(),
                OutRecords.end(),
                [](const FCatalogSourceMetadataOverrideRecord& A,
                    const FCatalogSourceMetadataOverrideRecord& B)
                {
                    return A.BuildingId < B.BuildingId;
                });
            return true;
        }

        return false;
    }

    bool LoadCatalogWorkforceOverrideRecords(
        std::vector<FCatalogWorkforceOverrideRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildCatalogWorkforceOverrideCandidatePaths();

        for (size_t PathIndex = 0;
            PathIndex < CandidatePaths.size();
            ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (!Fields.empty() && !Fields[0].empty())
                    {
                        FCatalogWorkforceOverrideRecord Record;
                        Record.BuildingId = Fields[0];

                        if (Fields.size() >= 2)
                        {
                            const std::wstring CapacityText = Utf8ToWide(
                                UnescapeCatalogField(Fields[1]));
                            wchar_t* EndPtr = nullptr;
                            const long ParsedCapacity = wcstol(
                                CapacityText.c_str(),
                                &EndPtr,
                                10);

                            if (EndPtr != CapacityText.c_str())
                            {
                                Record.HasCapacity = true;
                                Record.Capacity =
                                    (std::max)(0, static_cast<int>(ParsedCapacity));
                            }
                        }

                        if (Fields.size() >= 3)
                        {
                            const std::string EducationKey =
                                UnescapeCatalogField(Fields[2]);
                            if (!EducationKey.empty())
                            {
                                Record.HasRequiredEducationLevel =
                                    TryParseEducationLevelKey(
                                        EducationKey,
                                        Record.RequiredEducationLevel);
                            }
                        }

                        if (Fields.size() >= 4)
                        {
                            Record.SourceAssetName = Utf8ToWide(
                                UnescapeCatalogField(Fields[3]));
                        }

                        if (Fields.size() >= 5)
                        {
                            Record.SourceDisplayName = Utf8ToWide(
                                UnescapeCatalogField(Fields[4]));
                        }

                        if (Record.HasCapacity ||
                            Record.HasRequiredEducationLevel)
                        {
                            OutRecords.push_back(std::move(Record));
                        }
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            std::stable_sort(
                OutRecords.begin(),
                OutRecords.end(),
                [](const FCatalogWorkforceOverrideRecord& A,
                    const FCatalogWorkforceOverrideRecord& B)
                {
                    return A.BuildingId < B.BuildingId;
                });
            return true;
        }

        return false;
    }

    bool LoadCatalogPowerOverrideRecords(
        std::vector<FCatalogPowerOverrideRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildCatalogPowerOverrideCandidatePaths();

        for (size_t PathIndex = 0;
            PathIndex < CandidatePaths.size();
            ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (!Fields.empty() && !Fields[0].empty())
                    {
                        FCatalogPowerOverrideRecord Record;
                        Record.BuildingId = Fields[0];

                        if (Fields.size() >= 2)
                        {
                            const std::string ValueText =
                                UnescapeCatalogField(Fields[1]);
                            if (!ValueText.empty())
                            {
                                char* EndPtr = nullptr;
                                const long ParsedValue = strtol(
                                    ValueText.c_str(),
                                    &EndPtr,
                                    10);
                                if (EndPtr != ValueText.c_str())
                                {
                                    Record.HasProducedPowerMW = true;
                                    Record.ProducedPowerMW =
                                        (std::max)(0, static_cast<int>(ParsedValue));
                                }
                            }
                        }

                        if (Fields.size() >= 3)
                        {
                            const std::string ValueText =
                                UnescapeCatalogField(Fields[2]);
                            if (!ValueText.empty())
                            {
                                char* EndPtr = nullptr;
                                const long ParsedValue = strtol(
                                    ValueText.c_str(),
                                    &EndPtr,
                                    10);
                                if (EndPtr != ValueText.c_str())
                                {
                                    Record.HasRequiredPowerMW = true;
                                    Record.RequiredPowerMW =
                                        (std::max)(0, static_cast<int>(ParsedValue));
                                }
                            }
                        }

                        if (Fields.size() >= 4)
                        {
                            Record.SourceDisplayName = Utf8ToWide(
                                UnescapeCatalogField(Fields[3]));
                        }

                        if (Record.HasProducedPowerMW ||
                            Record.HasRequiredPowerMW)
                        {
                            OutRecords.push_back(std::move(Record));
                        }
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            std::stable_sort(
                OutRecords.begin(),
                OutRecords.end(),
                [](const FCatalogPowerOverrideRecord& A,
                    const FCatalogPowerOverrideRecord& B)
                {
                    return A.BuildingId < B.BuildingId;
                });
            return true;
        }

        return false;
    }

    bool LoadCatalogPollutionOverrideRecords(
        std::vector<FCatalogPollutionOverrideRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildCatalogPollutionOverrideCandidatePaths();

        for (size_t PathIndex = 0;
            PathIndex < CandidatePaths.size();
            ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (!Fields.empty() && !Fields[0].empty())
                    {
                        FCatalogPollutionOverrideRecord Record;
                        Record.BuildingId = Fields[0];

                        if (Fields.size() >= 2)
                        {
                            const std::string ValueText =
                                UnescapeCatalogField(Fields[1]);
                            if (!ValueText.empty())
                            {
                                char* EndPtr = nullptr;
                                const long ParsedValue = strtol(
                                    ValueText.c_str(),
                                    &EndPtr,
                                    10);
                                if (EndPtr != ValueText.c_str())
                                {
                                    Record.HasPollutionOutput = true;
                                    Record.PollutionOutput =
                                        (std::max)(0, static_cast<int>(ParsedValue));
                                }
                            }
                        }

                        if (Fields.size() >= 3)
                        {
                            const std::string ValueText =
                                UnescapeCatalogField(Fields[2]);
                            if (!ValueText.empty())
                            {
                                char* EndPtr = nullptr;
                                const long ParsedValue = strtol(
                                    ValueText.c_str(),
                                    &EndPtr,
                                    10);
                                if (EndPtr != ValueText.c_str())
                                {
                                    Record.HasPollutionMitigation = true;
                                    Record.PollutionMitigation =
                                        (std::max)(0, static_cast<int>(ParsedValue));
                                }
                            }
                        }

                        if (Fields.size() >= 4)
                        {
                            Record.SourceDisplayName = Utf8ToWide(
                                UnescapeCatalogField(Fields[3]));
                        }

                        if (Record.HasPollutionOutput ||
                            Record.HasPollutionMitigation)
                        {
                            OutRecords.push_back(std::move(Record));
                        }
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            std::stable_sort(
                OutRecords.begin(),
                OutRecords.end(),
                [](const FCatalogPollutionOverrideRecord& A,
                    const FCatalogPollutionOverrideRecord& B)
                {
                    return A.BuildingId < B.BuildingId;
                });
            return true;
        }

        return false;
    }

    bool LoadCatalogServiceStatsOverrideRecords(
        std::vector<FCatalogServiceStatsOverrideRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildCatalogServiceStatsOverrideCandidatePaths();

        for (size_t PathIndex = 0;
            PathIndex < CandidatePaths.size();
            ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (!Fields.empty() && !Fields[0].empty())
                    {
                        FCatalogServiceStatsOverrideRecord Record;
                        Record.BuildingId = Fields[0];

                        int ParsedValue = 0;

                        if (Fields.size() >= 2 &&
                            TryParseCatalogIntegerField(
                                Fields[1],
                                ParsedValue))
                        {
                            Record.HasCapacity = true;
                            Record.Capacity = (std::max)(0, ParsedValue);
                        }

                        if (Fields.size() >= 3 &&
                            TryParseCatalogIntegerField(
                                Fields[2],
                                ParsedValue))
                        {
                            Record.HasServiceCapacity = true;
                            Record.ServiceCapacity = (std::max)(0, ParsedValue);
                        }

                        if (Fields.size() >= 4 &&
                            TryParseCatalogIntegerField(
                                Fields[3],
                                ParsedValue))
                        {
                            Record.HasHouseholdCapacity = true;
                            Record.HouseholdCapacity = (std::max)(0, ParsedValue);
                        }

                        if (Fields.size() >= 5 &&
                            TryParseCatalogIntegerField(
                                Fields[4],
                                ParsedValue))
                        {
                            Record.HasBaseJobQuality = true;
                            Record.BaseJobQuality =
                                (std::max)(0, (std::min)(100, ParsedValue));
                        }

                        if (Fields.size() >= 6 &&
                            TryParseCatalogIntegerField(
                                Fields[5],
                                ParsedValue))
                        {
                            Record.HasBaseServiceQuality = true;
                            Record.BaseServiceQuality =
                                (std::max)(0, (std::min)(100, ParsedValue));
                        }

                        if (Fields.size() >= 7 &&
                            TryParseCatalogIntegerField(
                                Fields[6],
                                ParsedValue))
                        {
                            Record.HasBaseHousingQuality = true;
                            Record.BaseHousingQuality =
                                (std::max)(0, (std::min)(100, ParsedValue));
                        }

                        if (Fields.size() >= 8 &&
                            TryParseCatalogIntegerField(
                                Fields[7],
                                ParsedValue))
                        {
                            Record.HasAllowedWealthMask = true;
                            Record.AllowedWealthMask =
                                NormalizeAllowedWealthMaskOverride(ParsedValue);
                        }

                        if (Fields.size() >= 9)
                        {
                            const std::string TouristPreferenceKey =
                                UnescapeCatalogField(Fields[8]);
                            if (!TouristPreferenceKey.empty())
                            {
                                Record.HasTouristPreference =
                                    TryParseTouristPreferenceKey(
                                        TouristPreferenceKey,
                                        Record.TouristPreference);
                            }
                        }

                        int ParsedSizeX = 0;
                        int ParsedSizeY = 0;
                        if (Fields.size() >= 11 &&
                            TryParseCatalogIntegerField(
                                Fields[9],
                                ParsedSizeX) &&
                            TryParseCatalogIntegerField(
                                Fields[10],
                                ParsedSizeY))
                        {
                            Record.HasSize = ParsedSizeX > 0 && ParsedSizeY > 0;
                            Record.SizeX = (std::max)(1, ParsedSizeX);
                            Record.SizeY = (std::max)(1, ParsedSizeY);
                        }

                        if (Fields.size() >= 12)
                        {
                            Record.SourceDisplayName = Utf8ToWide(
                                UnescapeCatalogField(Fields[11]));
                        }

                        if (Record.HasCapacity ||
                            Record.HasServiceCapacity ||
                            Record.HasHouseholdCapacity ||
                            Record.HasBaseJobQuality ||
                            Record.HasBaseServiceQuality ||
                            Record.HasBaseHousingQuality ||
                            Record.HasAllowedWealthMask ||
                            Record.HasTouristPreference ||
                            Record.HasSize)
                        {
                            OutRecords.push_back(std::move(Record));
                        }
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            std::stable_sort(
                OutRecords.begin(),
                OutRecords.end(),
                [](const FCatalogServiceStatsOverrideRecord& A,
                    const FCatalogServiceStatsOverrideRecord& B)
                {
                    return A.BuildingId < B.BuildingId;
                });
            return true;
        }

        return false;
    }

    bool LoadCatalogSizeOverrideRecords(
        std::vector<FCatalogSizeOverrideRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildCatalogSizeOverrideCandidatePaths();

        for (size_t PathIndex = 0;
            PathIndex < CandidatePaths.size();
            ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (Fields.size() >= 3 && !Fields[0].empty())
                    {
                        int ParsedSizeX = 0;
                        int ParsedSizeY = 0;
                        if (!TryParseCatalogIntegerField(
                                Fields[1],
                                ParsedSizeX) ||
                            !TryParseCatalogIntegerField(
                                Fields[2],
                                ParsedSizeY) ||
                            ParsedSizeX <= 0 ||
                            ParsedSizeY <= 0)
                        {
                            continue;
                        }

                        FCatalogSizeOverrideRecord Record;
                        Record.BuildingId = Fields[0];
                        Record.SizeX = ParsedSizeX;
                        Record.SizeY = ParsedSizeY;

                        if (Fields.size() >= 4)
                        {
                            Record.SourceDisplayName = Utf8ToWide(
                                UnescapeCatalogField(Fields[3]));
                        }

                        OutRecords.push_back(std::move(Record));
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            std::stable_sort(
                OutRecords.begin(),
                OutRecords.end(),
                [](const FCatalogSizeOverrideRecord& A,
                    const FCatalogSizeOverrideRecord& B)
                {
                    return A.BuildingId < B.BuildingId;
                });
            return true;
        }

        return false;
    }

    bool LoadCatalogOperationModeOverrideRecords(
        std::vector<FCatalogOperationModeOverrideRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildCatalogOperationModeOverrideCandidatePaths();

        for (size_t PathIndex = 0;
            PathIndex < CandidatePaths.size();
            ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (Fields.size() >= 3 && !Fields[0].empty())
                    {
                        char* EndPtr = nullptr;
                        const long ParsedModeIndex =
                            strtol(Fields[1].c_str(), &EndPtr, 10);

                        if (!(EndPtr && *EndPtr == 0 && ParsedModeIndex >= 0))
                            continue;

                        FCatalogOperationModeOverrideRecord Record;
                        Record.BuildingId = Fields[0];
                        Record.ModeIndex = static_cast<int>(ParsedModeIndex);
                        Record.DisplayName = Utf8ToWide(
                            UnescapeCatalogField(Fields[2]));

                        if (Fields.size() >= 4)
                        {
                            Record.EffectSummary = Utf8ToWide(
                                UnescapeCatalogField(Fields[3]));
                        }

                        float ParsedFloatValue = 0.f;
                        int ParsedIntValue = 0;

                        if (Fields.size() >= 5 &&
                            TryParseCatalogFloatField(
                                Fields[4],
                                ParsedFloatValue))
                        {
                            Record.HasProductionMultiplier = true;
                            Record.ProductionMultiplier =
                                (std::max)(0.f, ParsedFloatValue);
                        }

                        if (Fields.size() >= 6 &&
                            TryParseCatalogFloatField(
                                Fields[5],
                                ParsedFloatValue))
                        {
                            Record.HasInputConsumptionMultiplier = true;
                            Record.InputConsumptionMultiplier =
                                (std::max)(0.f, ParsedFloatValue);
                        }

                        if (Fields.size() >= 7 &&
                            TryParseCatalogFloatField(
                                Fields[6],
                                ParsedFloatValue))
                        {
                            Record.HasServiceThroughputMultiplier = true;
                            Record.ServiceThroughputMultiplier =
                                (std::max)(0.f, ParsedFloatValue);
                        }

                        if (Fields.size() >= 8 &&
                            TryParseCatalogFloatField(
                                Fields[7],
                                ParsedFloatValue))
                        {
                            Record.HasPollutionMultiplier = true;
                            Record.PollutionMultiplier =
                                (std::max)(0.f, ParsedFloatValue);
                        }

                        if (Fields.size() >= 9 &&
                            TryParseCatalogFloatField(
                                Fields[8],
                                ParsedFloatValue))
                        {
                            Record.HasWageMultiplier = true;
                            Record.WageMultiplier =
                                (std::max)(0.f, ParsedFloatValue);
                        }

                        if (Fields.size() >= 10 &&
                            TryParseCatalogFloatField(
                                Fields[9],
                                ParsedFloatValue))
                        {
                            Record.HasUpkeepMultiplier = true;
                            Record.UpkeepMultiplier =
                                (std::max)(0.f, ParsedFloatValue);
                        }

                        if (Fields.size() >= 11 &&
                            TryParseCatalogIntegerField(
                                Fields[10],
                                ParsedIntValue))
                        {
                            Record.HasExportPriceDeltaPercent = true;
                            Record.ExportPriceDeltaPercent = ParsedIntValue;
                        }

                        if (Fields.size() >= 12 &&
                            TryParseCatalogIntegerField(
                                Fields[11],
                                ParsedIntValue))
                        {
                            Record.HasImportPriceDeltaPercent = true;
                            Record.ImportPriceDeltaPercent = ParsedIntValue;
                        }

                        if (Fields.size() >= 13 &&
                            TryParseCatalogIntegerField(
                                Fields[12],
                                ParsedIntValue))
                        {
                            Record.HasCapacityDelta = true;
                            Record.CapacityDelta = ParsedIntValue;
                        }

                        if (Fields.size() >= 14 &&
                            TryParseCatalogIntegerField(
                                Fields[13],
                                ParsedIntValue))
                        {
                            Record.HasServiceCapacityDelta = true;
                            Record.ServiceCapacityDelta = ParsedIntValue;
                        }

                        if (Fields.size() >= 15 &&
                            TryParseCatalogIntegerField(
                                Fields[14],
                                ParsedIntValue))
                        {
                            Record.HasHousingQualityDelta = true;
                            Record.HousingQualityDelta = ParsedIntValue;
                        }

                        if (Fields.size() >= 16 &&
                            TryParseCatalogIntegerField(
                                Fields[15],
                                ParsedIntValue))
                        {
                            Record.HasJobQualityDelta = true;
                            Record.JobQualityDelta = ParsedIntValue;
                        }

                        if (Fields.size() >= 17 &&
                            TryParseCatalogIntegerField(
                                Fields[16],
                                ParsedIntValue))
                        {
                            Record.HasGenericServiceQualityDelta = true;
                            Record.GenericServiceQualityDelta = ParsedIntValue;
                        }

                        if (Fields.size() >= 18)
                        {
                            Record.RequiredResearch = Utf8ToWide(
                                UnescapeCatalogField(Fields[17]));
                        }

                        if (Fields.size() >= 19)
                        {
                            const std::string EraKey =
                                UnescapeCatalogField(Fields[18]);
                            if (!EraKey.empty())
                            {
                                Record.HasUnlockEra =
                                    TryParseBuildingEraKey(
                                        EraKey,
                                        Record.UnlockEra);
                            }
                        }

                        if (Fields.size() >= 20)
                        {
                            Record.SourceDisplayName = Utf8ToWide(
                                UnescapeCatalogField(Fields[19]));
                        }

                        OutRecords.push_back(std::move(Record));
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            std::stable_sort(
                OutRecords.begin(),
                OutRecords.end(),
                [](const FCatalogOperationModeOverrideRecord& A,
                    const FCatalogOperationModeOverrideRecord& B)
                {
                    if (A.BuildingId != B.BuildingId)
                        return A.BuildingId < B.BuildingId;

                    return A.ModeIndex < B.ModeIndex;
                });
            return true;
        }

        return false;
    }

    bool LoadCatalogRuntimeUpgradeOverrideRecords(
        std::vector<FCatalogRuntimeUpgradeOverrideRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildCatalogRuntimeUpgradeOverrideCandidatePaths();

        for (size_t PathIndex = 0;
            PathIndex < CandidatePaths.size();
            ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (Fields.size() >= 3 && !Fields[0].empty())
                    {
                        char* EndPtr = nullptr;
                        const long ParsedUpgradeIndex =
                            strtol(Fields[1].c_str(), &EndPtr, 10);

                        if (!(EndPtr && *EndPtr == 0 &&
                                ParsedUpgradeIndex >= 0))
                        {
                            continue;
                        }

                        FCatalogRuntimeUpgradeOverrideRecord Record;
                        Record.BuildingId = Fields[0];
                        Record.UpgradeIndex =
                            static_cast<int>(ParsedUpgradeIndex);
                        Record.DisplayName = Utf8ToWide(
                            UnescapeCatalogField(Fields[2]));

                        if (Fields.size() >= 4)
                        {
                            Record.EffectSummary = Utf8ToWide(
                                UnescapeCatalogField(Fields[3]));
                        }

                        if (Fields.size() >= 5)
                        {
                            const std::string EraKey =
                                UnescapeCatalogField(Fields[4]);
                            if (!EraKey.empty())
                            {
                                Record.HasUnlockEra =
                                    TryParseBuildingEraKey(
                                        EraKey,
                                        Record.UnlockEra);
                            }
                        }

                        if (Fields.size() >= 6)
                        {
                            const std::wstring CostText = Utf8ToWide(
                                UnescapeCatalogField(Fields[5]));
                            if (!CostText.empty())
                            {
                                TryParseCatalogCostText(
                                    CostText,
                                    Record.CostState,
                                    Record.Cost);
                            }
                        }

                        float ParsedFloatValue = 0.f;
                        int ParsedIntValue = 0;

                        if (Fields.size() >= 7 &&
                            TryParseCatalogFloatField(
                                Fields[6],
                                ParsedFloatValue))
                        {
                            Record.HasProductionMultiplier = true;
                            Record.ProductionMultiplier =
                                (std::max)(0.f, ParsedFloatValue);
                        }

                        if (Fields.size() >= 8 &&
                            TryParseCatalogFloatField(
                                Fields[7],
                                ParsedFloatValue))
                        {
                            Record.HasInputConsumptionMultiplier = true;
                            Record.InputConsumptionMultiplier =
                                (std::max)(0.f, ParsedFloatValue);
                        }

                        if (Fields.size() >= 9 &&
                            TryParseCatalogFloatField(
                                Fields[8],
                                ParsedFloatValue))
                        {
                            Record.HasUpkeepMultiplier = true;
                            Record.UpkeepMultiplier =
                                (std::max)(0.f, ParsedFloatValue);
                        }

                        if (Fields.size() >= 10 &&
                            TryParseCatalogFloatField(
                                Fields[9],
                                ParsedFloatValue))
                        {
                            Record.HasWarehouseSlotCapacityMultiplier = true;
                            Record.WarehouseSlotCapacityMultiplier =
                                (std::max)(0.f, ParsedFloatValue);
                        }

                        if (Fields.size() >= 11 &&
                            TryParseCatalogIntegerField(
                                Fields[10],
                                ParsedIntValue))
                        {
                            Record.HasCapacityDelta = true;
                            Record.CapacityDelta = ParsedIntValue;
                        }

                        if (Fields.size() >= 12 &&
                            TryParseCatalogIntegerField(
                                Fields[11],
                                ParsedIntValue))
                        {
                            Record.HasServiceCapacityDelta = true;
                            Record.ServiceCapacityDelta = ParsedIntValue;
                        }

                        if (Fields.size() >= 13 &&
                            TryParseCatalogIntegerField(
                                Fields[12],
                                ParsedIntValue))
                        {
                            Record.HasPerWorkerServiceCapacityDelta = true;
                            Record.PerWorkerServiceCapacityDelta =
                                ParsedIntValue;
                        }

                        if (Fields.size() >= 14 &&
                            TryParseCatalogIntegerField(
                                Fields[13],
                                ParsedIntValue))
                        {
                            Record.HasHousingQualityDelta = true;
                            Record.HousingQualityDelta = ParsedIntValue;
                        }

                        if (Fields.size() >= 15 &&
                            TryParseCatalogIntegerField(
                                Fields[14],
                                ParsedIntValue))
                        {
                            Record.HasJobQualityDelta = true;
                            Record.JobQualityDelta = ParsedIntValue;
                        }

                        if (Fields.size() >= 16 &&
                            TryParseCatalogIntegerField(
                                Fields[15],
                                ParsedIntValue))
                        {
                            Record.HasGenericServiceQualityDelta = true;
                            Record.GenericServiceQualityDelta =
                                ParsedIntValue;
                        }

                        if (Fields.size() >= 17 &&
                            TryParseCatalogIntegerField(
                                Fields[16],
                                ParsedIntValue))
                        {
                            Record.HasRequiredPowerDeltaMW = true;
                            Record.RequiredPowerDeltaMW = ParsedIntValue;
                        }

                        if (Fields.size() >= 18 &&
                            TryParseCatalogFloatField(
                                Fields[17],
                                ParsedFloatValue))
                        {
                            Record.HasPollutionMultiplier = true;
                            Record.PollutionMultiplier =
                                (std::max)(0.f, ParsedFloatValue);
                        }

                        if (Fields.size() >= 19 &&
                            TryParseCatalogIntegerField(
                                Fields[18],
                                ParsedIntValue))
                        {
                            Record.HasUpkeepFlatDelta = true;
                            Record.UpkeepFlatDelta = ParsedIntValue;
                        }

                        if (Fields.size() >= 20 &&
                            TryParseCatalogIntegerField(
                                Fields[19],
                                ParsedIntValue))
                        {
                            Record.HasWarehouseSlotCapacityDelta = true;
                            Record.WarehouseSlotCapacityDelta =
                                ParsedIntValue;
                        }

                        if (Fields.size() >= 21)
                        {
                            Record.SourceDisplayName = Utf8ToWide(
                                UnescapeCatalogField(Fields[20]));
                        }

                        OutRecords.push_back(std::move(Record));
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            std::stable_sort(
                OutRecords.begin(),
                OutRecords.end(),
                [](const FCatalogRuntimeUpgradeOverrideRecord& A,
                    const FCatalogRuntimeUpgradeOverrideRecord& B)
                {
                    if (A.BuildingId != B.BuildingId)
                        return A.BuildingId < B.BuildingId;

                    return A.UpgradeIndex < B.UpgradeIndex;
                });
            return true;
        }

        return false;
    }

    bool IsAbsolutePath(const std::wstring& Path)
    {
        if (Path.size() >= 2 && Path[1] == L':')
            return true;

        if (Path.size() >= 2 &&
            ((Path[0] == L'\\' && Path[1] == L'\\') ||
                (Path[0] == L'/' && Path[1] == L'/')))
        {
            return true;
        }

        return false;
    }

    std::wstring ResolveTextureFullPath(const std::wstring& Path)
    {
        if (Path.empty())
            return std::wstring();

        if (IsAbsolutePath(Path))
            return Path;

        if (const TCHAR* TexturePath = CPathManager::FindPath("Texture"))
            return JoinPath(TexturePath, Path.c_str());

        return Path;
    }

    bool DoesFileExist(const std::wstring& FullPath)
    {
        if (FullPath.empty())
            return false;

        const DWORD Attributes = GetFileAttributesW(FullPath.c_str());
        return Attributes != INVALID_FILE_ATTRIBUTES &&
            (Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    void LogCatalogValidationMessage(const std::wstring& Message);

    const wchar_t* GetPlacementTemplateTypeKey(
        EPlacementTemplateType Type)
    {
        switch (Type)
        {
        case EPlacementTemplateType::SingleTileMarker:
            return L"SingleTileMarker";
        case EPlacementTemplateType::Diamond3x3SingleMarker:
            return L"Diamond3x3SingleMarker";
        case EPlacementTemplateType::Diamond5x5TwoMarker:
            return L"Diamond5x5TwoMarker";
        case EPlacementTemplateType::Diamond5x5FourMarker:
            return L"Diamond5x5FourMarker";
        case EPlacementTemplateType::Diamond7x7ThreeMarker:
            return L"Diamond7x7ThreeMarker";
        default:
            return L"Unknown";
        }
    }

    const wchar_t* GetPlacementBuildingKindKey(
        EPlacementBuildingKind Kind)
    {
        switch (Kind)
        {
        case EPlacementBuildingKind::Structure:
            return L"Structure";
        case EPlacementBuildingKind::Road:
            return L"Road";
        case EPlacementBuildingKind::TransportOffice:
            return L"TransportOffice";
        case EPlacementBuildingKind::Harbor:
            return L"Harbor";
        default:
            return L"Unknown";
        }
    }

    std::wstring SanitizeAuditTsvField(const std::wstring& Text)
    {
        std::wstring Result = Text;

        for (size_t Index = 0; Index < Result.size(); ++Index)
        {
            wchar_t& Ch = Result[Index];

            if (Ch == L'\t' || Ch == L'\r' || Ch == L'\n')
                Ch = L' ';
        }

        return Result;
    }

    std::wstring SanitizeAuditTsvField(const std::string& Text)
    {
        return SanitizeAuditTsvField(Utf8ToWide(Text));
    }

    std::wstring ResolveBuildingCatalogVisualAuditDumpPath()
    {
        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring DebugDirectory =
                JoinPath(RootPath, L"Debug");
            return JoinPath(
                DebugDirectory,
                L"BuildingCatalogVisualAudit.tsv");
        }

        return std::wstring();
    }

    bool EnsureDirectoryExists(const std::wstring& DirectoryPath)
    {
        if (DirectoryPath.empty())
            return false;

        const DWORD Attributes = GetFileAttributesW(DirectoryPath.c_str());
        if (Attributes != INVALID_FILE_ATTRIBUTES)
            return (Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        return CreateDirectoryW(DirectoryPath.c_str(), nullptr) != 0;
    }

    void WriteBuildingCatalogVisualAuditDump(
        const std::vector<FBuildingCatalogEntry>& Entries)
    {
        const std::wstring DumpPath =
            ResolveBuildingCatalogVisualAuditDumpPath();
        if (DumpPath.empty())
        {
            LogCatalogValidationMessage(
                L"검수 덤프 경로를 찾지 못해 BuildingCatalogVisualAudit.tsv 생성 생략");
            return;
        }

        const std::wstring DumpDirectory =
            GetParentDirectoryPath(DumpPath);
        if (!EnsureDirectoryExists(DumpDirectory))
        {
            LogCatalogValidationMessage(
                L"검수 덤프 디렉터리 생성 실패: " + DumpDirectory);
            return;
        }

        std::string Buffer;
        Buffer.reserve(Entries.size() * 160);
        Buffer +=
            "DisplayName\tbuildingId\tIconPath\tSpritePath\tTemplateType\t"
            "BuildingKind\tIconExists\tSpriteExists\r\n";

        for (const FBuildingCatalogEntry& Entry : Entries)
        {
            const std::wstring FullIconPath =
                ResolveTextureFullPath(Entry.IconPath);
            const std::wstring FullSpritePath =
                ResolveTextureFullPath(Entry.SpriteTexturePath);

            Buffer += WideToUtf8(SanitizeAuditTsvField(Entry.DisplayName));
            Buffer += "\t";
            Buffer += WideToUtf8(SanitizeAuditTsvField(Entry.Id));
            Buffer += "\t";
            Buffer += WideToUtf8(SanitizeAuditTsvField(Entry.IconPath));
            Buffer += "\t";
            Buffer += WideToUtf8(
                SanitizeAuditTsvField(Entry.SpriteTexturePath));
            Buffer += "\t";
            Buffer += WideToUtf8(
                GetPlacementTemplateTypeKey(Entry.TemplateType));
            Buffer += "\t";
            Buffer += WideToUtf8(
                GetPlacementBuildingKindKey(Entry.BuildingKind));
            Buffer += "\t";
            Buffer += DoesFileExist(FullIconPath) ? "true" : "false";
            Buffer += "\t";
            Buffer += DoesFileExist(FullSpritePath) ? "true" : "false";
            Buffer += "\r\n";
        }

        FILE* File = nullptr;
        if (_wfopen_s(&File, DumpPath.c_str(), L"wb") != 0 || !File)
        {
            LogCatalogValidationMessage(
                L"검수 덤프 파일 생성 실패: " + DumpPath);
            return;
        }

        static const unsigned char Bom[] = { 0xEF, 0xBB, 0xBF };
        fwrite(Bom, 1, sizeof(Bom), File);
        fwrite(Buffer.data(), 1, Buffer.size(), File);
        fclose(File);

        LogCatalogValidationMessage(
            L"검수 덤프 생성: " + DumpPath +
            L" (rows=" + std::to_wstring(Entries.size()) + L")");
    }

    void LogCatalogValidationMessage(const std::wstring& Message)
    {
        std::wstring Output = L"[BuildingCatalog] ";
        Output += Message;
        Output += L"\n";
        OutputDebugStringW(Output.c_str());
    }

    void ValidateBuildingCatalogEntries(
        const std::vector<FBuildingCatalogEntry>& Entries)
    {
        std::unordered_set<std::string> UniqueIds;
        UniqueIds.reserve(Entries.size());

        int NonEmptyIconPathCount = 0;
        int NonEmptySpritePathCount = 0;
        int DuplicateIdCount = 0;
        int EmptyIconPathCount = 0;
        int EmptySpritePathCount = 0;
        int MissingIconFileCount = 0;
        int MissingSpriteFileCount = 0;

        for (const FBuildingCatalogEntry& Entry : Entries)
        {
            const auto InsertResult = UniqueIds.insert(Entry.Id);
            if (!InsertResult.second)
            {
                ++DuplicateIdCount;
                LogCatalogValidationMessage(
                    L"중복 건물 ID: " + Utf8ToWide(Entry.Id));
            }

            if (Entry.IconPath.empty())
            {
                ++EmptyIconPathCount;
                LogCatalogValidationMessage(
                    L"빈 IconPath: " + Utf8ToWide(Entry.Id));
            }
            else
            {
                ++NonEmptyIconPathCount;

                const std::wstring FullIconPath =
                    ResolveTextureFullPath(Entry.IconPath);
                if (!DoesFileExist(FullIconPath))
                {
                    ++MissingIconFileCount;
                    LogCatalogValidationMessage(
                        L"IconPath 파일 없음: " + Utf8ToWide(Entry.Id) +
                        L" -> " + Entry.IconPath);
                }
            }

            if (Entry.SpriteTexturePath.empty())
            {
                ++EmptySpritePathCount;
                LogCatalogValidationMessage(
                    L"빈 SpriteTexturePath: " + Utf8ToWide(Entry.Id));
            }
            else
            {
                ++NonEmptySpritePathCount;

                const std::wstring FullSpritePath =
                    ResolveTextureFullPath(Entry.SpriteTexturePath);
                if (!DoesFileExist(FullSpritePath))
                {
                    ++MissingSpriteFileCount;
                    LogCatalogValidationMessage(
                        L"SpriteTexturePath 파일 없음: " +
                        Utf8ToWide(Entry.Id) + L" -> " +
                        Entry.SpriteTexturePath);
                }
            }
        }

        if (NonEmptyIconPathCount != static_cast<int>(Entries.size()))
        {
            LogCatalogValidationMessage(
                L"건물 수와 IconPath 수 불일치: buildings=" +
                std::to_wstring(Entries.size()) +
                L", iconPaths=" + std::to_wstring(NonEmptyIconPathCount));
        }

        if (NonEmptySpritePathCount != static_cast<int>(Entries.size()))
        {
            LogCatalogValidationMessage(
                L"건물 수와 SpriteTexturePath 수 불일치: buildings=" +
                std::to_wstring(Entries.size()) +
                L", spritePaths=" +
                std::to_wstring(NonEmptySpritePathCount));
        }

        LogCatalogValidationMessage(
            L"검증 요약: buildings=" + std::to_wstring(Entries.size()) +
            L", duplicateIds=" + std::to_wstring(DuplicateIdCount) +
            L", emptyIconPath=" + std::to_wstring(EmptyIconPathCount) +
            L", emptySpriteTexturePath=" +
            std::to_wstring(EmptySpritePathCount) +
            L", missingIconFiles=" + std::to_wstring(MissingIconFileCount) +
            L", missingSpriteFiles=" +
            std::to_wstring(MissingSpriteFileCount));
    }

    const FProductionRecipeRecord* FindProductionRecipeRecord(
        const std::vector<FProductionRecipeRecord>& Records,
        const std::string& BuildingId)
    {
        const auto It = std::find_if(
            Records.begin(),
            Records.end(),
            [&](const FProductionRecipeRecord& Record)
            {
                return Record.BuildingId == BuildingId;
            });

        return It != Records.end() ? &(*It) : nullptr;
    }

    const FCatalogCostOverrideRecord* FindCatalogCostOverrideRecord(
        const std::vector<FCatalogCostOverrideRecord>& Records,
        const std::string& BuildingId)
    {
        const auto It = std::find_if(
            Records.begin(),
            Records.end(),
            [&](const FCatalogCostOverrideRecord& Record)
            {
                return Record.BuildingId == BuildingId;
            });

        return It != Records.end() ? &(*It) : nullptr;
    }

    const FCatalogSourceMetadataOverrideRecord*
        FindCatalogSourceMetadataOverrideRecord(
            const std::vector<FCatalogSourceMetadataOverrideRecord>& Records,
            const std::string& BuildingId)
    {
        const auto It = std::find_if(
            Records.begin(),
            Records.end(),
            [&](const FCatalogSourceMetadataOverrideRecord& Record)
            {
                return Record.BuildingId == BuildingId;
            });

        return It != Records.end() ? &(*It) : nullptr;
    }

    const FCatalogWorkforceOverrideRecord* FindCatalogWorkforceOverrideRecord(
        const std::vector<FCatalogWorkforceOverrideRecord>& Records,
        const std::string& BuildingId)
    {
        const auto It = std::find_if(
            Records.begin(),
            Records.end(),
            [&](const FCatalogWorkforceOverrideRecord& Record)
            {
                return Record.BuildingId == BuildingId;
            });

        return It != Records.end() ? &(*It) : nullptr;
    }

    const FCatalogPowerOverrideRecord* FindCatalogPowerOverrideRecord(
        const std::vector<FCatalogPowerOverrideRecord>& Records,
        const std::string& BuildingId)
    {
        const auto It = std::find_if(
            Records.begin(),
            Records.end(),
            [&](const FCatalogPowerOverrideRecord& Record)
            {
                return Record.BuildingId == BuildingId;
            });

        return It != Records.end() ? &(*It) : nullptr;
    }

    const FCatalogPollutionOverrideRecord*
        FindCatalogPollutionOverrideRecord(
            const std::vector<FCatalogPollutionOverrideRecord>& Records,
            const std::string& BuildingId)
    {
        const auto It = std::find_if(
            Records.begin(),
            Records.end(),
            [&](const FCatalogPollutionOverrideRecord& Record)
            {
                return Record.BuildingId == BuildingId;
            });

        return It != Records.end() ? &(*It) : nullptr;
    }

    const FCatalogServiceStatsOverrideRecord*
        FindCatalogServiceStatsOverrideRecord(
            const std::vector<FCatalogServiceStatsOverrideRecord>& Records,
            const std::string& BuildingId)
    {
        const auto It = std::find_if(
            Records.begin(),
            Records.end(),
            [&](const FCatalogServiceStatsOverrideRecord& Record)
            {
                return Record.BuildingId == BuildingId;
            });

        return It != Records.end() ? &(*It) : nullptr;
    }

    const FCatalogSizeOverrideRecord* FindCatalogSizeOverrideRecord(
        const std::vector<FCatalogSizeOverrideRecord>& Records,
        const std::string& BuildingId)
    {
        const auto It = std::find_if(
            Records.begin(),
            Records.end(),
            [&](const FCatalogSizeOverrideRecord& Record)
            {
                return Record.BuildingId == BuildingId;
            });

        return It != Records.end() ? &(*It) : nullptr;
    }

    std::vector<const FCatalogOperationModeOverrideRecord*>
        FindCatalogOperationModeOverrideRecords(
            const std::vector<FCatalogOperationModeOverrideRecord>& Records,
            const std::string& BuildingId)
    {
        std::vector<const FCatalogOperationModeOverrideRecord*> Result;

        for (const FCatalogOperationModeOverrideRecord& Record : Records)
        {
            if (Record.BuildingId == BuildingId)
                Result.push_back(&Record);
        }

        return Result;
    }

    std::vector<const FCatalogRuntimeUpgradeOverrideRecord*>
        FindCatalogRuntimeUpgradeOverrideRecords(
            const std::vector<FCatalogRuntimeUpgradeOverrideRecord>& Records,
            const std::string& BuildingId)
    {
        std::vector<const FCatalogRuntimeUpgradeOverrideRecord*> Result;

        for (const FCatalogRuntimeUpgradeOverrideRecord& Record : Records)
        {
            if (Record.BuildingId == BuildingId)
                Result.push_back(&Record);
        }

        return Result;
    }

    bool ShouldApplyCostOverride(EBuildingCostState CurrentState)
    {
        return CurrentState == EBuildingCostState::None ||
            CurrentState == EBuildingCostState::Unknown;
    }

    void ApplyCatalogCostOverride(
        const FCatalogCostOverrideRecord& OverrideRecord,
        FBuildingCatalogEntry& Entry)
    {
        if (ShouldApplyCostOverride(Entry.BlueprintCostState) &&
            OverrideRecord.BlueprintCostState !=
                EBuildingCostState::None)
        {
            Entry.BlueprintCostState =
                OverrideRecord.BlueprintCostState;
            Entry.BlueprintCost =
                OverrideRecord.BlueprintCost;
        }

        if (ShouldApplyCostOverride(Entry.ConstructionCostState) &&
            OverrideRecord.ConstructionCostState !=
                EBuildingCostState::None)
        {
            Entry.ConstructionCostState =
                OverrideRecord.ConstructionCostState;
            Entry.ConstructionCost =
                OverrideRecord.ConstructionCost;
        }
    }

    void ApplyCatalogSourceMetadataOverride(
        const FCatalogSourceMetadataOverrideRecord& OverrideRecord,
        FBuildingCatalogEntry& Entry)
    {
        if (OverrideRecord.HasUnlockEra)
            Entry.UnlockEra = OverrideRecord.UnlockEra;

        if (OverrideRecord.HasBuildMenuCategoryOverride)
        {
            Entry.HasBuildMenuCategoryOverride = true;
            Entry.BuildMenuCategoryOverride =
                OverrideRecord.BuildMenuCategoryOverride;
        }
    }

    void ApplyCatalogWorkforceOverride(
        const FCatalogWorkforceOverrideRecord& OverrideRecord,
        FBuildingCatalogEntry& Entry)
    {
        if (OverrideRecord.HasCapacity)
            Entry.Capacity = OverrideRecord.Capacity;

        if (OverrideRecord.HasRequiredEducationLevel)
        {
            Entry.RequiredEducationLevel =
                OverrideRecord.RequiredEducationLevel;
        }
    }

    void ApplyCatalogPowerOverride(
        const FCatalogPowerOverrideRecord& OverrideRecord,
        FBuildingCatalogEntry& Entry)
    {
        if (OverrideRecord.HasProducedPowerMW)
            Entry.BaseProducedPowerMW = OverrideRecord.ProducedPowerMW;

        if (OverrideRecord.HasRequiredPowerMW)
            Entry.BaseRequiredPowerMW = OverrideRecord.RequiredPowerMW;
    }

    void ApplyCatalogPollutionOverride(
        const FCatalogPollutionOverrideRecord& OverrideRecord,
        FBuildingCatalogEntry& Entry)
    {
        if (OverrideRecord.HasPollutionOutput)
            Entry.BasePollutionOutput = OverrideRecord.PollutionOutput;

        if (OverrideRecord.HasPollutionMitigation)
        {
            Entry.BasePollutionMitigation =
                OverrideRecord.PollutionMitigation;
        }
    }

    void ApplyBaseQualityMetadata(FBuildingCatalogEntry& Entry)
    {
        if (Entry.BaseHousingQuality > 0)
            Entry.HousingSatisfactionCap = Entry.BaseHousingQuality;

        if (Entry.BaseJobQuality > 0)
            Entry.JobSatisfactionCap = Entry.BaseJobQuality;

        if (Entry.BaseServiceQuality <= 0)
            return;

        if (Entry.FoodProvider)
            Entry.FoodSatisfactionCap = Entry.BaseServiceQuality;
        if (Entry.EntertainmentProvider)
            Entry.FunSatisfactionCap = Entry.BaseServiceQuality;
        if (Entry.HealthProvider)
            Entry.HealthSatisfactionCap = Entry.BaseServiceQuality;
        if (Entry.FaithProvider)
            Entry.FaithSatisfactionCap = Entry.BaseServiceQuality;
    }

    void ApplyCatalogServiceStatsOverride(
        const FCatalogServiceStatsOverrideRecord& OverrideRecord,
        FBuildingCatalogEntry& Entry)
    {
        if (OverrideRecord.HasCapacity)
            Entry.Capacity = OverrideRecord.Capacity;

        if (OverrideRecord.HasServiceCapacity)
        {
            Entry.ServiceCapacity = OverrideRecord.ServiceCapacity;
            Entry.ServiceCapacityUsesHouseholds = false;
        }

        if (OverrideRecord.HasHouseholdCapacity)
            Entry.HouseholdCapacity = OverrideRecord.HouseholdCapacity;

        if (OverrideRecord.HasBaseHousingQuality)
            Entry.BaseHousingQuality = OverrideRecord.BaseHousingQuality;

        if (OverrideRecord.HasBaseJobQuality)
            Entry.BaseJobQuality = OverrideRecord.BaseJobQuality;

        if (OverrideRecord.HasBaseServiceQuality)
            Entry.BaseServiceQuality = OverrideRecord.BaseServiceQuality;

        if (OverrideRecord.HasAllowedWealthMask)
            Entry.AllowedWealthMask = OverrideRecord.AllowedWealthMask;

        if (OverrideRecord.HasTouristPreference)
        {
            Entry.PrimaryTouristPreference =
                OverrideRecord.TouristPreference;
        }

        if (OverrideRecord.HasSize)
        {
            Entry.BuildingSizeX = OverrideRecord.SizeX;
            Entry.BuildingSizeY = OverrideRecord.SizeY;
        }

        ApplyBaseQualityMetadata(Entry);
    }

    void ApplyCatalogSizeOverride(
        const FCatalogSizeOverrideRecord& OverrideRecord,
        FBuildingCatalogEntry& Entry)
    {
        Entry.BuildingSizeX = (std::max)(1, OverrideRecord.SizeX);
        Entry.BuildingSizeY = (std::max)(1, OverrideRecord.SizeY);
    }

    void ApplyCatalogOperationModeOverride(
        const FBuildingCatalogEntry& Entry,
        const FCatalogOperationModeOverrideRecord& OverrideRecord,
        std::vector<FBuildingOperationModeDef>& ModeDefs)
    {
        if (OverrideRecord.ModeIndex < 0)
            return;

        const size_t SafeModeIndex =
            static_cast<size_t>(OverrideRecord.ModeIndex);

        if (ModeDefs.size() <= SafeModeIndex)
            ModeDefs.resize(SafeModeIndex + 1);

        FBuildingOperationModeDef& ModeDef = ModeDefs[SafeModeIndex];

        if (!OverrideRecord.DisplayName.empty())
            ModeDef.DisplayName = OverrideRecord.DisplayName;

        if (!OverrideRecord.EffectSummary.empty())
        {
            ModeDef.EffectSummary = OverrideRecord.EffectSummary;
            ModeDef.Effect = FBuildingOperationModeEffect();

            const std::vector<std::wstring> Clauses =
                SplitOperationModeSummaryClauses(ModeDef.EffectSummary);
            for (size_t ClauseIndex = 0;
                ClauseIndex < Clauses.size();
                ++ClauseIndex)
            {
                ApplyOperationModeEffectClause(
                    Entry,
                    Clauses[ClauseIndex],
                    ModeDef.Effect);
            }
        }

        if (OverrideRecord.HasUnlockEra)
        {
            ModeDef.HasUnlockEra = true;
            ModeDef.UnlockEra = OverrideRecord.UnlockEra;
        }

        if (!OverrideRecord.RequiredResearch.empty())
            ModeDef.RequiredResearch = OverrideRecord.RequiredResearch;

        if (OverrideRecord.HasProductionMultiplier)
        {
            ModeDef.Effect.ProductionMultiplier =
                OverrideRecord.ProductionMultiplier;
        }

        if (OverrideRecord.HasInputConsumptionMultiplier)
        {
            ModeDef.Effect.InputConsumptionMultiplier =
                OverrideRecord.InputConsumptionMultiplier;
        }

        if (OverrideRecord.HasServiceThroughputMultiplier)
        {
            ModeDef.Effect.ServiceThroughputMultiplier =
                OverrideRecord.ServiceThroughputMultiplier;
        }

        if (OverrideRecord.HasPollutionMultiplier)
        {
            ModeDef.Effect.PollutionMultiplier =
                OverrideRecord.PollutionMultiplier;
        }

        if (OverrideRecord.HasWageMultiplier)
            ModeDef.Effect.WageMultiplier = OverrideRecord.WageMultiplier;

        if (OverrideRecord.HasUpkeepMultiplier)
            ModeDef.Effect.UpkeepMultiplier = OverrideRecord.UpkeepMultiplier;

        if (OverrideRecord.HasExportPriceDeltaPercent)
        {
            ModeDef.Effect.ExportTradeRoutePriceDeltaPercent =
                OverrideRecord.ExportPriceDeltaPercent;
        }

        if (OverrideRecord.HasImportPriceDeltaPercent)
        {
            ModeDef.Effect.ImportTradeRoutePriceDeltaPercent =
                OverrideRecord.ImportPriceDeltaPercent;
        }

        if (OverrideRecord.HasCapacityDelta)
            ModeDef.Effect.CapacityDelta = OverrideRecord.CapacityDelta;

        if (OverrideRecord.HasServiceCapacityDelta)
        {
            ModeDef.Effect.ServiceCapacityDelta =
                OverrideRecord.ServiceCapacityDelta;
        }

        if (OverrideRecord.HasHousingQualityDelta)
        {
            ModeDef.Effect.HousingQualityDelta =
                OverrideRecord.HousingQualityDelta;
        }

        if (OverrideRecord.HasJobQualityDelta)
            ModeDef.Effect.JobQualityDelta = OverrideRecord.JobQualityDelta;

        if (OverrideRecord.HasGenericServiceQualityDelta)
        {
            ModeDef.Effect.GenericServiceQualityDelta =
                OverrideRecord.GenericServiceQualityDelta;
        }
    }

    void ApplyCatalogRuntimeUpgradeOverride(
        const FCatalogRuntimeUpgradeOverrideRecord& OverrideRecord,
        std::vector<FBuildingRuntimeUpgradeDef>& UpgradeDefs)
    {
        if (OverrideRecord.UpgradeIndex < 0)
            return;

        const size_t SafeUpgradeIndex =
            static_cast<size_t>(OverrideRecord.UpgradeIndex);

        if (UpgradeDefs.size() <= SafeUpgradeIndex)
            UpgradeDefs.resize(SafeUpgradeIndex + 1);

        FBuildingRuntimeUpgradeDef& UpgradeDef =
            UpgradeDefs[SafeUpgradeIndex];

        if (!OverrideRecord.DisplayName.empty())
            UpgradeDef.DisplayName = OverrideRecord.DisplayName;

        if (!OverrideRecord.EffectSummary.empty())
            UpgradeDef.EffectSummary = OverrideRecord.EffectSummary;

        if (OverrideRecord.HasUnlockEra)
        {
            UpgradeDef.HasUnlockEra = true;
            UpgradeDef.UnlockEra = OverrideRecord.UnlockEra;
        }

        if (OverrideRecord.CostState != EBuildingCostState::None)
        {
            UpgradeDef.CostState = OverrideRecord.CostState;
            UpgradeDef.Cost = OverrideRecord.Cost;
        }

        if (OverrideRecord.HasProductionMultiplier)
        {
            UpgradeDef.Effect.ProductionMultiplier =
                OverrideRecord.ProductionMultiplier;
        }

        if (OverrideRecord.HasInputConsumptionMultiplier)
        {
            UpgradeDef.Effect.InputConsumptionMultiplier =
                OverrideRecord.InputConsumptionMultiplier;
        }

        if (OverrideRecord.HasUpkeepMultiplier)
        {
            UpgradeDef.Effect.UpkeepMultiplier =
                OverrideRecord.UpkeepMultiplier;
        }

        if (OverrideRecord.HasWarehouseSlotCapacityMultiplier)
        {
            UpgradeDef.Effect.WarehouseSlotCapacityMultiplier =
                OverrideRecord.WarehouseSlotCapacityMultiplier;
        }

        if (OverrideRecord.HasCapacityDelta)
            UpgradeDef.Effect.CapacityDelta = OverrideRecord.CapacityDelta;

        if (OverrideRecord.HasServiceCapacityDelta)
        {
            UpgradeDef.Effect.ServiceCapacityDelta =
                OverrideRecord.ServiceCapacityDelta;
        }

        if (OverrideRecord.HasPerWorkerServiceCapacityDelta)
        {
            UpgradeDef.Effect.PerWorkerServiceCapacityDelta =
                OverrideRecord.PerWorkerServiceCapacityDelta;
        }

        if (OverrideRecord.HasHousingQualityDelta)
        {
            UpgradeDef.Effect.HousingQualityDelta =
                OverrideRecord.HousingQualityDelta;
        }

        if (OverrideRecord.HasJobQualityDelta)
            UpgradeDef.Effect.JobQualityDelta = OverrideRecord.JobQualityDelta;

        if (OverrideRecord.HasGenericServiceQualityDelta)
        {
            UpgradeDef.Effect.GenericServiceQualityDelta =
                OverrideRecord.GenericServiceQualityDelta;
        }

        if (OverrideRecord.HasRequiredPowerDeltaMW)
        {
            UpgradeDef.Effect.RequiredPowerDeltaMW =
                OverrideRecord.RequiredPowerDeltaMW;
        }

        if (OverrideRecord.HasPollutionMultiplier)
        {
            UpgradeDef.Effect.PollutionMultiplier =
                OverrideRecord.PollutionMultiplier;
        }

        if (OverrideRecord.HasUpkeepFlatDelta)
        {
            UpgradeDef.Effect.UpkeepFlatDelta =
                OverrideRecord.UpkeepFlatDelta;
        }

        if (OverrideRecord.HasWarehouseSlotCapacityDelta)
        {
            UpgradeDef.Effect.WarehouseSlotCapacityDelta =
                OverrideRecord.WarehouseSlotCapacityDelta;
        }
    }

    void AddPoliticalSignal(
        FBuildingCatalogEntry& Entry,
        EPoliticalAxis Axis,
        EPoliticalStance FavoredStance,
        float Strength,
        EPoliticalScope Scope = EPoliticalScope::Global)
    {
        if (Strength <= 0.f)
            return;

        FPoliticalSignalDef Signal;
        Signal.Axis = Axis;
        Signal.FavoredStance = FavoredStance;
        Signal.Strength = Strength;
        Signal.Scope = Scope;
        Entry.PoliticalSignals.push_back(Signal);
    }

    std::vector<std::wstring> SplitDetailLines(const std::wstring& Text)
    {
        std::vector<std::wstring> Lines;
        std::wstring CurrentLine;

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const wchar_t Ch = Text[Index];

            if (Ch == L'\r')
                continue;

            if (Ch == L'\n')
            {
                Lines.push_back(CurrentLine);
                CurrentLine.clear();
                continue;
            }

            CurrentLine.push_back(Ch);
        }

        if (!CurrentLine.empty() || Text.empty())
            Lines.push_back(CurrentLine);

        return Lines;
    }

    std::wstring Trim(const std::wstring& Text)
    {
        size_t Start = 0;

        while (Start < Text.size() && iswspace(Text[Start]))
            ++Start;

        size_t End = Text.size();

        while (End > Start && iswspace(Text[End - 1]))
            --End;

        return Text.substr(Start, End - Start);
    }

    bool TryExtractDetailLineValue(
        const std::wstring& DetailText,
        const wchar_t* Prefix,
        std::wstring& OutValue)
    {
        OutValue.clear();

        if (!Prefix || !*Prefix)
            return false;

        const std::vector<std::wstring> Lines =
            SplitDetailLines(DetailText);

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::wstring Line = Trim(Lines[Index]);

            if (Line.find(Prefix) != 0)
                continue;

            OutValue = Trim(Line.substr(wcslen(Prefix)));
            return true;
        }

        return false;
    }

    bool TryParseSignedInteger(
        const std::wstring& Text,
        int& OutValue)
    {
        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const wchar_t Ch = Text[Index];

            if ((Ch == L'+' || Ch == L'-') &&
                Index + 1 < Text.size() &&
                iswdigit(Text[Index + 1]))
            {
                wchar_t* EndPtr = nullptr;
                const long Value = wcstol(Text.c_str() + Index, &EndPtr, 10);

                if (EndPtr != Text.c_str() + Index)
                {
                    OutValue = static_cast<int>(Value);
                    return true;
                }
            }

            if (!iswdigit(Ch))
                continue;

            wchar_t* EndPtr = nullptr;
            const long Value = wcstol(Text.c_str() + Index, &EndPtr, 10);

            if (EndPtr != Text.c_str() + Index)
            {
                OutValue = static_cast<int>(Value);
                return true;
            }
        }

        return false;
    }

    bool TryParseWorkerCapacity(
        const std::wstring& DetailText,
        int& OutCapacity)
    {
        std::wstring WorkerLine;
        if (!TryExtractDetailLineValue(
                DetailText,
                L"필요 인력:",
                WorkerLine))
        {
            return false;
        }

        if (WorkerLine.find(L"업그레이드") != std::wstring::npos)
        {
            OutCapacity = 0;
            return true;
        }

        if (WorkerLine.find(L"미기재") != std::wstring::npos)
            return false;

        if (WorkerLine.find(L"없음") != std::wstring::npos)
        {
            OutCapacity = 0;
            return true;
        }

        int ParsedValue = 0;
        if (!TryParseSignedInteger(WorkerLine, ParsedValue))
            return false;

        OutCapacity = (std::max)(0, ParsedValue);
        return true;
    }

    bool TryParseDetailCapacityValue(
        const std::wstring& DetailText,
        const wchar_t* Prefix,
        int& OutCapacity)
    {
        std::wstring CapacityLine;
        if (!TryExtractDetailLineValue(DetailText, Prefix, CapacityLine))
            return false;

        if (CapacityLine.find(L"미기재") != std::wstring::npos)
            return false;

        if (CapacityLine.find(L"없음") != std::wstring::npos)
        {
            OutCapacity = 0;
            return true;
        }

        int ParsedValue = 0;
        if (!TryParseSignedInteger(CapacityLine, ParsedValue))
            return false;

        OutCapacity = (std::max)(0, ParsedValue);
        return true;
    }

    bool TryParseHouseholdCapacity(
        const std::wstring& DetailText,
        int& OutCapacity)
    {
        return TryParseDetailCapacityValue(
            DetailText,
            L"수용 가구:",
            OutCapacity);
    }

    bool TryParseServiceCapacity(
        const std::wstring& DetailText,
        int& OutCapacity,
        bool& OutUsesHouseholds)
    {
        OutUsesHouseholds = false;

        if (TryParseDetailCapacityValue(
                DetailText,
                L"수용 인원:",
                OutCapacity))
        {
            return true;
        }

        if (TryParseDetailCapacityValue(
                DetailText,
                L"수용 가구:",
                OutCapacity))
        {
            OutUsesHouseholds = true;
            return true;
        }

        return false;
    }

    bool TryParseDetailQualityValue(
        const std::wstring& DetailText,
        const wchar_t* Prefix,
        int& OutValue)
    {
        std::wstring QualityLine;
        if (!TryExtractDetailLineValue(DetailText, Prefix, QualityLine))
            return false;

        if (QualityLine.find(L"미기재") != std::wstring::npos)
            return false;

        int ParsedValue = 0;
        if (!TryParseSignedInteger(QualityLine, ParsedValue))
            return false;

        OutValue = (std::max)(0, (std::min)(100, ParsedValue));
        return true;
    }

    bool TryParseBuildingSize(
        const std::wstring& DetailText,
        int& OutSizeX,
        int& OutSizeY)
    {
        std::wstring SizeLine;
        if (!TryExtractDetailLineValue(DetailText, L"크기:", SizeLine))
            return false;

        if (SizeLine.find(L"미기재") != std::wstring::npos)
            return false;

        for (size_t Index = 0; Index < SizeLine.size(); ++Index)
        {
            if (!iswdigit(SizeLine[Index]))
                continue;

            wchar_t* EndPtr = nullptr;
            const long ParsedX = wcstol(SizeLine.c_str() + Index, &EndPtr, 10);
            if (EndPtr == SizeLine.c_str() + Index)
                continue;

            size_t Cursor = static_cast<size_t>(EndPtr - SizeLine.c_str());
            while (Cursor < SizeLine.size() && iswspace(SizeLine[Cursor]))
                ++Cursor;

            if (Cursor >= SizeLine.size() ||
                (SizeLine[Cursor] != L'x' &&
                    SizeLine[Cursor] != L'X' &&
                    SizeLine[Cursor] != L'×'))
            {
                continue;
            }

            ++Cursor;
            while (Cursor < SizeLine.size() && iswspace(SizeLine[Cursor]))
                ++Cursor;

            if (Cursor >= SizeLine.size() || !iswdigit(SizeLine[Cursor]))
                continue;

            wchar_t* EndPtrY = nullptr;
            const long ParsedY = wcstol(SizeLine.c_str() + Cursor, &EndPtrY, 10);
            if (EndPtrY == SizeLine.c_str() + Cursor)
                continue;

            OutSizeX = (std::max)(1, static_cast<int>(ParsedX));
            OutSizeY = (std::max)(1, static_cast<int>(ParsedY));
            return true;
        }

        return false;
    }

    bool TryExtractWealthRequirementValue(
        const std::wstring& DetailText,
        std::wstring& OutValue)
    {
        static const wchar_t* Prefixes[] =
        {
            L"재산 요구치:",
            L"필요 재산:",
            L"관광객 재산:",
            L"요구 재산:"
        };

        for (size_t Index = 0;
            Index < sizeof(Prefixes) / sizeof(Prefixes[0]);
            ++Index)
        {
            if (TryExtractDetailLineValue(
                    DetailText,
                    Prefixes[Index],
                    OutValue))
            {
                return true;
            }
        }

        OutValue.clear();
        return false;
    }

    unsigned int ParseAllowedWealthMaskFromValue(const std::wstring& Value)
    {
        const std::wstring TrimmedValue = Trim(Value);
        if (TrimmedValue.empty() ||
            TrimmedValue.find(L"미기재") != std::wstring::npos)
        {
            return GBuildingWealthMaskAll;
        }

        unsigned int Mask = GBuildingWealthMaskNone;

        if (TrimmedValue.find(L"파산") != std::wstring::npos ||
            TrimmedValue.find(L"가난") != std::wstring::npos)
        {
            Mask |= GBuildingWealthMaskPoor;
        }

        if (TrimmedValue.find(L"유복") != std::wstring::npos)
            Mask |= GBuildingWealthMaskWellOff;

        if (TrimmedValue.find(L"부유") != std::wstring::npos ||
            TrimmedValue.find(L"더럽게 부유") != std::wstring::npos)
        {
            Mask |= GBuildingWealthMaskRich;
        }

        if (TrimmedValue.find(L"이상") != std::wstring::npos)
        {
            if (Mask & GBuildingWealthMaskPoor)
                Mask |= GBuildingWealthMaskWellOff | GBuildingWealthMaskRich;
            else if (Mask & GBuildingWealthMaskWellOff)
                Mask |= GBuildingWealthMaskRich;
        }

        return Mask == GBuildingWealthMaskNone ?
            GBuildingWealthMaskAll :
            Mask;
    }

    unsigned int ParseAllowedWealthMask(const std::wstring& DetailText)
    {
        std::wstring WealthRequirementValue;
        if (!TryExtractWealthRequirementValue(
                DetailText,
                WealthRequirementValue))
        {
            return GBuildingWealthMaskAll;
        }

        return ParseAllowedWealthMaskFromValue(WealthRequirementValue);
    }

    bool TryParseCatalogCostText(
        const std::wstring& Text,
        EBuildingCostState& OutState,
        int& OutCost)
    {
        const std::wstring Trimmed = Trim(Text);

        if (Trimmed.empty())
        {
            OutState = EBuildingCostState::None;
            OutCost = 0;
            return false;
        }

        if (Trimmed.find(L"미기재") != std::wstring::npos)
        {
            OutState = EBuildingCostState::Unknown;
            OutCost = 0;
            return true;
        }

        if (Trimmed.find(L"없음") != std::wstring::npos ||
            Trimmed.find(L"무료") != std::wstring::npos)
        {
            OutState = EBuildingCostState::Known;
            OutCost = 0;
            return true;
        }

        int ParsedCost = 0;

        if (TryParseSignedInteger(Trimmed, ParsedCost))
        {
            OutState = EBuildingCostState::Known;
            OutCost = ParsedCost;
            return true;
        }

        OutState = EBuildingCostState::Unknown;
        OutCost = 0;
        return true;
    }

    bool TryExtractDetailCost(
        const std::wstring& DetailText,
        const wchar_t* Prefix,
        EBuildingCostState& OutState,
        int& OutCost)
    {
        if (!Prefix)
            return false;

        const std::vector<std::wstring> Lines = SplitDetailLines(DetailText);

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::wstring Line = Trim(Lines[Index]);

            if (Line.size() < wcslen(Prefix) ||
                Line.compare(0, wcslen(Prefix), Prefix) != 0)
            {
                continue;
            }

            return TryParseCatalogCostText(
                Line.substr(wcslen(Prefix)),
                OutState,
                OutCost);
        }

        return false;
    }

    bool TryParseFirstFloat(
        const std::wstring& Text,
        double& OutValue)
    {
        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const wchar_t Ch = Text[Index];
            const bool StartsNumber =
                iswdigit(Ch) ||
                ((Ch == L'+' || Ch == L'-') &&
                    Index + 1 < Text.size() &&
                    iswdigit(Text[Index + 1]));

            if (!StartsNumber)
                continue;

            wchar_t* EndPtr = nullptr;
            const double Value = wcstod(Text.c_str() + Index, &EndPtr);

            if (EndPtr != Text.c_str() + Index)
            {
                OutValue = Value;
                return true;
            }
        }

        return false;
    }

    std::vector<std::wstring> SplitCommaClauses(const std::wstring& Text)
    {
        std::vector<std::wstring> Clauses;
        std::wstring CurrentClause;
        int ParenthesesDepth = 0;

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const wchar_t Ch = Text[Index];

            if (Ch == L'(')
                ++ParenthesesDepth;
            else if (Ch == L')' && ParenthesesDepth > 0)
                --ParenthesesDepth;

            if (Ch == L',' && ParenthesesDepth == 0)
            {
                const std::wstring TrimmedClause = Trim(CurrentClause);

                if (!TrimmedClause.empty())
                    Clauses.push_back(TrimmedClause);

                CurrentClause.clear();
                continue;
            }

            CurrentClause.push_back(Ch);
        }

        const std::wstring TrimmedClause = Trim(CurrentClause);

        if (!TrimmedClause.empty())
            Clauses.push_back(TrimmedClause);

        return Clauses;
    }

    bool TryParseUpgradeUnlockEraClause(
        const std::wstring& Clause,
        EBuildingEra& OutEra)
    {
        std::wstring CanonicalClause;

        for (const wchar_t Ch : Trim(Clause))
        {
            if (!iswspace(Ch))
                CanonicalClause.push_back(Ch);
        }

        if (CanonicalClause == L"식민지" ||
            CanonicalClause == L"식민지시대")
        {
            OutEra = EBuildingEra::Colonial;
            return true;
        }

        if (CanonicalClause == L"세계대전" ||
            CanonicalClause == L"세계대전시대")
        {
            OutEra = EBuildingEra::WorldWars;
            return true;
        }

        if (CanonicalClause == L"냉전" ||
            CanonicalClause == L"냉전시대")
        {
            OutEra = EBuildingEra::ColdWar;
            return true;
        }

        if (CanonicalClause == L"현대" ||
            CanonicalClause == L"현대시대")
        {
            OutEra = EBuildingEra::Modern;
            return true;
        }

        return false;
    }

    bool TryParseUpgradeCostClause(
        const std::wstring& Clause,
        EBuildingCostState& OutState,
        int& OutCost)
    {
        const std::wstring TrimmedClause = Trim(Clause);

        if (TrimmedClause.empty())
            return false;

        if (TrimmedClause.find(L'+') != std::wstring::npos ||
            TrimmedClause.find(L'-') != std::wstring::npos ||
            TrimmedClause.find(L'%') != std::wstring::npos ||
            TrimmedClause.find(L':') != std::wstring::npos)
        {
            return false;
        }

        std::wstring CanonicalClause;

        for (const wchar_t Ch : TrimmedClause)
        {
            if (iswspace(Ch) || Ch == L'$' || Ch == L',')
                continue;

            CanonicalClause.push_back(Ch);
        }

        if (CanonicalClause.empty())
            return false;

        if (CanonicalClause == L"미기재")
        {
            OutState = EBuildingCostState::Unknown;
            OutCost = 0;
            return true;
        }

        if (CanonicalClause == L"없음" ||
            CanonicalClause == L"무료")
        {
            OutState = EBuildingCostState::Known;
            OutCost = 0;
            return true;
        }

        if (!std::all_of(
                CanonicalClause.begin(),
                CanonicalClause.end(),
                [](const wchar_t Ch)
                {
                    return iswdigit(Ch) != 0;
                }))
        {
            return false;
        }

        return TryParseCatalogCostText(
            CanonicalClause,
            OutState,
            OutCost);
    }

    std::wstring JoinCommaClauses(const std::vector<std::wstring>& Clauses)
    {
        std::wstring Result;

        for (size_t Index = 0; Index < Clauses.size(); ++Index)
        {
            const std::wstring Clause = Trim(Clauses[Index]);

            if (Clause.empty())
                continue;

            if (!Result.empty())
                Result += L", ";

            Result += Clause;
        }

        return Result;
    }

    void ApplyPercentMultiplier(float& TargetMultiplier, int Percent)
    {
        const float RelativeMultiplier = (std::max)(
            0.f,
            1.f + static_cast<float>(Percent) / 100.f);
        TargetMultiplier *= RelativeMultiplier;
    }

    int ResolveContextualSignedPercent(
        const std::wstring& Clause,
        int ParsedInteger)
    {
        if (ParsedInteger == 0)
            return 0;

        if (Clause.find(L"감소") != std::wstring::npos ||
            Clause.find(L"인하") != std::wstring::npos ||
            Clause.find(L"하락") != std::wstring::npos ||
            Clause.find(L"절감") != std::wstring::npos)
        {
            return -std::abs(ParsedInteger);
        }

        if (Clause.find(L"증가") != std::wstring::npos ||
            Clause.find(L"상승") != std::wstring::npos ||
            Clause.find(L"인상") != std::wstring::npos ||
            Clause.find(L"추가") != std::wstring::npos)
        {
            return std::abs(ParsedInteger);
        }

        return ParsedInteger;
    }

    bool ProvidesRuntimeService(const FBuildingCatalogEntry& Entry)
    {
        return Entry.FoodProvider ||
            Entry.EntertainmentProvider ||
            Entry.HealthProvider ||
            Entry.FaithProvider;
    }

    void ApplyQualityEffect(
        const std::wstring& Clause,
        int ParsedInteger,
        bool HasInteger,
        int& OutDelta,
        float& OutMultiplier)
    {
        if (!HasInteger)
            return;

        if (Clause.find(L"%") != std::wstring::npos)
            ApplyPercentMultiplier(OutMultiplier, ParsedInteger);
        else
            OutDelta += ParsedInteger;
    }

    void ApplyOperationModeClause(
        const FBuildingCatalogEntry& Entry,
        const std::wstring& RawClause,
        FBuildingOperationModeEffect& OutEffect)
    {
        const std::wstring Clause = Trim(RawClause);

        if (Clause.empty())
            return;

        int ParsedInteger = 0;
        const bool HasInteger = TryParseSignedInteger(Clause, ParsedInteger);
        double ParsedFloat = 0.0;
        const bool HasFloat = TryParseFirstFloat(Clause, ParsedFloat);

        if (Clause.find(L"화물선 속도") != std::wstring::npos &&
            HasInteger &&
            Clause.find(L"%") != std::wstring::npos)
        {
            ApplyPercentMultiplier(
                OutEffect.HarborProgressMultiplier,
                ParsedInteger);
            return;
        }

        if (Clause.find(L"적하량") != std::wstring::npos &&
            HasInteger &&
            Clause.find(L"%") != std::wstring::npos)
        {
            ApplyPercentMultiplier(
                OutEffect.TeamsterTransferMultiplier,
                ParsedInteger);
            return;
        }

        if (Clause.find(L"화물 손실") != std::wstring::npos &&
            HasInteger &&
            Clause.find(L"%") != std::wstring::npos)
        {
            OutEffect.TeamsterCargoLossPercent = (std::max)(
                OutEffect.TeamsterCargoLossPercent,
                std::abs(ParsedInteger));
            return;
        }

        if ((Clause.find(L"수출 가격") != std::wstring::npos ||
                Clause.find(L"수출 시세") != std::wstring::npos) &&
            HasInteger &&
            Clause.find(L"%") != std::wstring::npos)
        {
            OutEffect.ExportTradeRoutePriceDeltaPercent +=
                ResolveContextualSignedPercent(Clause, ParsedInteger);
            return;
        }

        if ((Clause.find(L"수입 무역로") != std::wstring::npos ||
                Clause.find(L"수입 가격") != std::wstring::npos) &&
            HasInteger &&
            Clause.find(L"%") != std::wstring::npos)
        {
            OutEffect.ImportTradeRoutePriceDeltaPercent +=
                ResolveContextualSignedPercent(Clause, ParsedInteger);
            return;
        }

        if ((Clause.find(L"생산 전력") != std::wstring::npos ||
                Clause.find(L"발전량") != std::wstring::npos) &&
            HasInteger)
        {
            if (Clause.find(L"%") != std::wstring::npos)
            {
                ApplyPercentMultiplier(
                    OutEffect.ProducedPowerMultiplier,
                    ParsedInteger);
            }
            else
            {
                OutEffect.ProducedPowerDeltaMW += ParsedInteger;
            }

            return;
        }

        if ((Clause.find(L"전력") != std::wstring::npos ||
                Clause.find(L"MW") != std::wstring::npos) &&
            HasInteger)
        {
            if (Clause.find(L"%") != std::wstring::npos)
            {
                ApplyPercentMultiplier(
                    OutEffect.RequiredPowerMultiplier,
                    ParsedInteger);
            }
            else
            {
                OutEffect.RequiredPowerDeltaMW += ParsedInteger;
            }

            return;
        }

        if ((Clause.find(L"슬롯당 보관량") != std::wstring::npos ||
                Clause.find(L"슬롯 보관량") != std::wstring::npos ||
                Clause.find(L"보관량") != std::wstring::npos ||
                Clause.find(L"저장량") != std::wstring::npos) &&
            HasInteger)
        {
            if (Clause.find(L"%") != std::wstring::npos)
            {
                ApplyPercentMultiplier(
                    OutEffect.WarehouseSlotCapacityMultiplier,
                    ParsedInteger);
            }
            else
            {
                OutEffect.WarehouseSlotCapacityDelta += ParsedInteger;
            }

            return;
        }

        if ((Clause.find(L"보관 손실") != std::wstring::npos ||
                Clause.find(L"보관 중 손실") != std::wstring::npos ||
                Clause.find(L"장기 보관") != std::wstring::npos ||
                Clause.find(L"부패") != std::wstring::npos) &&
            HasInteger &&
            Clause.find(L"%") != std::wstring::npos)
        {
            ApplyPercentMultiplier(
                OutEffect.StorageLossMultiplier,
                ParsedInteger);
            return;
        }

        if (Clause.find(L"공해") != std::wstring::npos)
        {
            if (HasInteger && Clause.find(L"%") != std::wstring::npos)
                ApplyPercentMultiplier(OutEffect.PollutionMultiplier, ParsedInteger);
            else if (HasInteger)
                OutEffect.PollutionFlatDelta += ParsedInteger;

            return;
        }

        if (Clause.find(L"유지비") != std::wstring::npos)
        {
            if (HasInteger && Clause.find(L"%") != std::wstring::npos)
                ApplyPercentMultiplier(OutEffect.UpkeepMultiplier, ParsedInteger);
            else if (HasInteger)
                OutEffect.UpkeepFlatDelta += ParsedInteger;

            return;
        }

        if (Clause.find(L"임금") != std::wstring::npos ||
            Clause.find(L"급여") != std::wstring::npos)
        {
            if (HasInteger && Clause.find(L"%") != std::wstring::npos)
                ApplyPercentMultiplier(OutEffect.WageMultiplier, ParsedInteger);
            else if (HasInteger)
                OutEffect.WageFlatDelta += ParsedInteger;

            return;
        }

        if (Clause.find(L"직업 품질") != std::wstring::npos)
        {
            ApplyQualityEffect(
                Clause,
                ParsedInteger,
                HasInteger,
                OutEffect.JobQualityDelta,
                OutEffect.JobQualityMultiplier);
            return;
        }

        if (Clause.find(L"주거 품질") != std::wstring::npos)
        {
            ApplyQualityEffect(
                Clause,
                ParsedInteger,
                HasInteger,
                OutEffect.HousingQualityDelta,
                OutEffect.HousingQualityMultiplier);
            return;
        }

        if (Clause.find(L"서비스 품질") != std::wstring::npos)
        {
            ApplyQualityEffect(
                Clause,
                ParsedInteger,
                HasInteger,
                OutEffect.GenericServiceQualityDelta,
                OutEffect.GenericServiceQualityMultiplier);
            return;
        }

        if (Clause.find(L"노동자당 방문객 슬롯") != std::wstring::npos)
        {
            if (HasInteger)
                OutEffect.PerWorkerServiceCapacityDelta += ParsedInteger;

            return;
        }

        if (Clause.find(L"방문객 슬롯") != std::wstring::npos ||
            Clause.find(L"서비스 슬롯") != std::wstring::npos)
        {
            if (HasInteger)
                OutEffect.ServiceCapacityDelta += ParsedInteger;

            return;
        }

        if (Clause.find(L"숙박 슬롯") != std::wstring::npos ||
            (Clause.find(L"방 슬롯") != std::wstring::npos &&
                Clause.find(L"방문객") == std::wstring::npos))
        {
            if (HasInteger)
                OutEffect.ServiceCapacityDelta += ParsedInteger;

            return;
        }

        if (Clause.find(L"가구수") != std::wstring::npos ||
            Clause.find(L"일자리") != std::wstring::npos)
        {
            if (HasInteger)
                OutEffect.CapacityDelta += ParsedInteger;

            return;
        }

        if ((Clause.find(L"소모") != std::wstring::npos ||
                Clause.find(L"투입량") != std::wstring::npos) &&
            HasInteger &&
            Clause.find(L"%") != std::wstring::npos)
        {
            ApplyPercentMultiplier(
                OutEffect.InputConsumptionMultiplier,
                ParsedInteger);
            return;
        }

        if (Clause.find(L"효율") != std::wstring::npos &&
            HasInteger &&
            Clause.find(L"%") != std::wstring::npos)
        {
            if (Entry.BuildingKind == EPlacementBuildingKind::Harbor)
            {
                ApplyPercentMultiplier(
                    OutEffect.HarborProgressMultiplier,
                    ParsedInteger);
            }

            if (Entry.ProducedResourceType != EResourceType::None)
                ApplyPercentMultiplier(OutEffect.ProductionMultiplier, ParsedInteger);

            if (ProvidesRuntimeService(Entry))
            {
                ApplyPercentMultiplier(
                    OutEffect.ServiceThroughputMultiplier,
                    ParsedInteger);
            }

            return;
        }

        if ((Clause.find(L"산출량") != std::wstring::npos ||
                Clause.find(L"생산량") != std::wstring::npos) &&
            HasInteger &&
            Clause.find(L"%") != std::wstring::npos)
        {
            ApplyPercentMultiplier(
                OutEffect.ProductionMultiplier,
                ParsedInteger);
            return;
        }

        if (Entry.ProducedResourceType != EResourceType::None &&
            Clause.find(L"생산") != std::wstring::npos &&
            HasFloat &&
            Clause.find(L"%") == std::wstring::npos &&
            ParsedFloat > 0.0)
        {
            OutEffect.ProductionMultiplier *= static_cast<float>(ParsedFloat);
        }
    }

    std::vector<FBuildingOperationModeDef> ExtractOperationModeDefs(
        const FBuildingCatalogEntry& Entry)
    {
        std::vector<FBuildingOperationModeDef> Result;
        const std::vector<std::wstring> Lines =
            SplitDetailLines(Entry.DetailText);
        bool Capture = false;

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::wstring Line = Trim(Lines[Index]);

            if (Line.find(L"운영 모드:") == 0)
            {
                Capture = true;
                continue;
            }

            if (!Capture)
                continue;

            if (Line.empty())
                break;

            if (Line[0] != L'-')
                break;

            FBuildingOperationModeDef ModeDef;
            const std::wstring RawModeText = Trim(Line.substr(1));
            const size_t OpenParen = RawModeText.find(L'(');
            const size_t CloseParen = RawModeText.find_last_of(L')');

            if (OpenParen != std::wstring::npos &&
                CloseParen != std::wstring::npos &&
                CloseParen > OpenParen)
            {
                ModeDef.DisplayName = Trim(
                    RawModeText.substr(0, OpenParen));
                ModeDef.EffectSummary = Trim(
                    RawModeText.substr(
                        OpenParen + 1,
                        CloseParen - OpenParen - 1));
            }
            else
            {
                ModeDef.DisplayName = RawModeText;
            }

            if (ModeDef.DisplayName.empty())
                ModeDef.DisplayName = RawModeText;

            const std::vector<std::wstring> Clauses =
                SplitCommaClauses(ModeDef.EffectSummary);

            for (size_t ClauseIndex = 0;
                ClauseIndex < Clauses.size();
                ++ClauseIndex)
            {
                ApplyOperationModeClause(
                    Entry,
                    Clauses[ClauseIndex],
                    ModeDef.Effect);
            }

            Result.push_back(std::move(ModeDef));
        }

        return Result;
    }

    bool TryBuildRuntimeUpgradeDef(
        const FBuildingCatalogEntry& Entry,
        const std::wstring& RawUpgradeText,
        FBuildingRuntimeUpgradeDef& OutDef)
    {
        const std::wstring UpgradeText = Trim(RawUpgradeText);

        if (UpgradeText.empty())
            return false;

        const size_t OpenParen = UpgradeText.find(L'(');
        const size_t CloseParen = UpgradeText.find_last_of(L')');

        if (OpenParen == std::wstring::npos ||
            CloseParen == std::wstring::npos ||
            CloseParen <= OpenParen)
        {
            return false;
        }

        std::wstring DisplayName = Trim(UpgradeText.substr(0, OpenParen));
        const size_t ScopeSep = DisplayName.find_last_of(L':');

        if (ScopeSep != std::wstring::npos)
            DisplayName = Trim(DisplayName.substr(ScopeSep + 1));

        if (DisplayName.empty())
            return false;

        FBuildingRuntimeUpgradeDef Def;
        Def.DisplayName = DisplayName;
        const std::wstring EffectText = Trim(
            UpgradeText.substr(OpenParen + 1, CloseParen - OpenParen - 1));
        const std::vector<std::wstring> Clauses =
            SplitCommaClauses(EffectText);
        std::vector<std::wstring> EffectClauses;

        for (size_t ClauseIndex = 0;
            ClauseIndex < Clauses.size();
            ++ClauseIndex)
        {
            const std::wstring Clause = Trim(Clauses[ClauseIndex]);

            if (Clause.empty())
                continue;

            EBuildingEra ParsedEra = EBuildingEra::Colonial;

            if (!Def.HasUnlockEra &&
                TryParseUpgradeUnlockEraClause(Clause, ParsedEra))
            {
                Def.HasUnlockEra = true;
                Def.UnlockEra = ParsedEra;
                continue;
            }

            if (Def.CostState == EBuildingCostState::None)
            {
                EBuildingCostState ParsedCostState =
                    EBuildingCostState::None;
                int ParsedCost = 0;

                if (TryParseUpgradeCostClause(
                        Clause,
                        ParsedCostState,
                        ParsedCost))
                {
                    Def.CostState = ParsedCostState;
                    Def.Cost = ParsedCost;
                    continue;
                }
            }

            EffectClauses.push_back(Clause);
        }

        Def.EffectSummary = JoinCommaClauses(EffectClauses);

        for (size_t ClauseIndex = 0;
            ClauseIndex < EffectClauses.size();
            ++ClauseIndex)
        {
            ApplyOperationModeClause(
                Entry,
                EffectClauses[ClauseIndex],
                Def.Effect);
        }

        if (!Def.Effect.HasRuntimeEffect())
            return false;

        OutDef = std::move(Def);
        return true;
    }

    std::vector<FBuildingRuntimeUpgradeDef> ExtractRuntimeUpgradeDefs(
        const FBuildingCatalogEntry& Entry)
    {
        std::vector<FBuildingRuntimeUpgradeDef> Result;
        const std::vector<std::wstring> UpgradeLines =
            ExtractUpgradeHintsFromDetail(Entry.DetailText);

        for (size_t LineIndex = 0; LineIndex < UpgradeLines.size(); ++LineIndex)
        {
            const std::vector<std::wstring> Candidates =
                SplitCommaClauses(UpgradeLines[LineIndex]);

            for (size_t CandidateIndex = 0;
                CandidateIndex < Candidates.size();
                ++CandidateIndex)
            {
                FBuildingRuntimeUpgradeDef Def;

                if (TryBuildRuntimeUpgradeDef(
                        Entry,
                        Candidates[CandidateIndex],
                        Def))
                {
                    Result.push_back(std::move(Def));
                }
            }
        }

        return Result;
    }

    bool HasProductionInputs(const FBuildingCatalogEntry& Entry)
    {
        for (int SlotIndex = 0;
            SlotIndex < GProductionInputSlotCount;
            ++SlotIndex)
        {
            const size_t Index = static_cast<size_t>(SlotIndex);

            if (Entry.ProductionInputTypes[Index] != EResourceType::None &&
                Entry.ProductionInputAmounts[Index] > 0)
            {
                return true;
            }
        }

        return false;
    }

    std::vector<std::wstring> BuildProductionInputDisplayLabels(
        const FBuildingCatalogEntry& Entry)
    {
        std::vector<std::wstring> InputLabels;

        for (int SlotIndex = 0;
            SlotIndex < GProductionInputSlotCount;
            ++SlotIndex)
        {
            const std::wstring InputLabel =
                GetBuildingProductionInputDisplayName(Entry, SlotIndex);

            if (InputLabel.empty())
                continue;

            std::wstring DisplayLabel = InputLabel;
            const size_t Index = static_cast<size_t>(SlotIndex);

            if (Entry.ProductionInputLabels[Index].empty() &&
                Entry.ProductionInputAmounts[Index] > 1)
            {
                DisplayLabel += L" x";
                DisplayLabel +=
                    std::to_wstring(Entry.ProductionInputAmounts[Index]);
            }

            InputLabels.push_back(std::move(DisplayLabel));
        }

        return InputLabels;
    }

    std::vector<std::wstring> BuildProductionDemandDisplayLabels(
        const FBuildingCatalogEntry& Entry)
    {
        std::vector<std::wstring> DemandLabels =
            BuildProductionInputDisplayLabels(Entry);

        if (Entry.ProducedResourceType == EResourceType::None &&
            Entry.VisitConsumptionResourceType != EResourceType::None)
        {
            const std::wstring VisitLabel =
                GetResourceTypeDisplayName(
                    Entry.VisitConsumptionResourceType);

            if (!VisitLabel.empty() &&
                std::find(
                    DemandLabels.begin(),
                    DemandLabels.end(),
                    VisitLabel) == DemandLabels.end())
            {
                DemandLabels.insert(DemandLabels.begin(), VisitLabel);
            }
        }

        return DemandLabels;
    }

    std::wstring BuildProductionConsumerDisplayLabel(
        const FBuildingCatalogEntry& Entry)
    {
        const std::wstring OutputLabel =
            GetBuildingProducedResourceDisplayName(Entry);

        if (!OutputLabel.empty())
            return OutputLabel;

        return Entry.DisplayName;
    }

    void AppendUniqueLabel(
        std::vector<std::wstring>& Labels,
        const std::wstring& Label)
    {
        if (Label.empty())
            return;

        const auto ExistingIt = std::find(
            Labels.begin(), Labels.end(), Label);

        if (ExistingIt == Labels.end())
            Labels.push_back(Label);
    }

    std::wstring JoinLabels(
        const std::vector<std::wstring>& Labels,
        const wchar_t* Separator)
    {
        std::wstring Result;

        for (size_t Index = 0; Index < Labels.size(); ++Index)
        {
            if (Labels[Index].empty())
                continue;

            if (!Result.empty() && Separator)
                Result += Separator;

            Result += Labels[Index];
        }

        return Result;
    }

    void PopulateProductionChainMetadata(
        std::vector<FBuildingCatalogEntry>& Entries)
    {
        std::vector<std::vector<std::wstring>> DownstreamResourceLabels(
            static_cast<size_t>(EResourceType::Count));

        for (const FBuildingCatalogEntry& ConsumerEntry : Entries)
        {
            const std::wstring ConsumerLabel =
                BuildProductionConsumerDisplayLabel(ConsumerEntry);

            if (ConsumerLabel.empty())
                continue;

            auto AppendDownstreamDemand = [&](EResourceType ResourceType)
            {
                if (ResourceType == EResourceType::None)
                    return;

                const size_t ResourceIndex =
                    static_cast<size_t>(ResourceType);

                if (ResourceIndex >= DownstreamResourceLabels.size())
                    return;

                AppendUniqueLabel(
                    DownstreamResourceLabels[ResourceIndex],
                    ConsumerLabel);
            };

            if (ConsumerEntry.VisitConsumptionResourceType !=
                    EResourceType::None &&
                ConsumerEntry.VisitConsumptionResourceType !=
                    ConsumerEntry.ProducedResourceType)
            {
                AppendDownstreamDemand(
                    ConsumerEntry.VisitConsumptionResourceType);
            }

            for (int SlotIndex = 0;
                SlotIndex < GProductionInputSlotCount;
                ++SlotIndex)
            {
                const size_t Index = static_cast<size_t>(SlotIndex);
                const EResourceType InputType =
                    ConsumerEntry.ProductionInputTypes[Index];

                if (InputType == EResourceType::None ||
                    ConsumerEntry.ProductionInputAmounts[Index] <= 0 ||
                    InputType == ConsumerEntry.VisitConsumptionResourceType)
                {
                    continue;
                }

                AppendDownstreamDemand(InputType);
            }
        }

        for (FBuildingCatalogEntry& Entry : Entries)
        {
            Entry.ProductionChainStage =
                FBuildingCatalogEntry::EProductionChainStage::None;
            Entry.SupplyChainSummary.clear();

            const std::wstring OutputLabel =
                GetBuildingProducedResourceDisplayName(Entry);
            const std::vector<std::wstring> DemandLabels =
                BuildProductionDemandDisplayLabels(Entry);

            if (Entry.ProducedResourceType == EResourceType::None ||
                OutputLabel.empty())
            {
                if (!DemandLabels.empty() && !Entry.DisplayName.empty())
                {
                    Entry.SupplyChainSummary =
                        JoinLabels(DemandLabels, L" + ") +
                        L" -> " +
                        Entry.DisplayName;
                }
                continue;
            }

            const bool EntryHasInputs = HasProductionInputs(Entry);
            const std::vector<std::wstring>& DownstreamLabels =
                DownstreamResourceLabels[
                    static_cast<size_t>(Entry.ProducedResourceType)];

            if (!EntryHasInputs)
            {
                Entry.ProductionChainStage =
                    FBuildingCatalogEntry::EProductionChainStage::Primary;
            }
            else if (!DownstreamLabels.empty())
            {
                Entry.ProductionChainStage =
                    FBuildingCatalogEntry::EProductionChainStage::Intermediate;
            }
            else
            {
                Entry.ProductionChainStage =
                    FBuildingCatalogEntry::EProductionChainStage::Final;
            }

            std::wstring Summary;

            if (!DemandLabels.empty())
            {
                Summary += JoinLabels(DemandLabels, L" + ");
                Summary += L" -> ";
            }

            Summary += OutputLabel;

            if (!DownstreamLabels.empty())
            {
                Summary += L" -> ";
                Summary += JoinLabels(DownstreamLabels, L", ");
            }
            else if (!EntryHasInputs)
            {
                Summary += L" 생산";
            }

            Entry.SupplyChainSummary = std::move(Summary);
        }
    }

    std::string BuildCatalogEntryId(
        EBuildingCategory Category,
        int CategoryLocalIndex)
    {
        const int CategoryIndex = static_cast<int>(Category);

        if (CategoryIndex < 0 || CategoryLocalIndex < 0)
            return std::string();

        return "build_" + std::to_string(CategoryIndex + 1) +
            "_" + std::to_string(CategoryLocalIndex + 1);
    }

    void LogMissingCatalogVisualPathOnce(
        const std::string& EntryId,
        const wchar_t* FieldName)
    {
        static std::vector<std::wstring> GLoggedWarnings;

        std::wstring WarningKey = Utf8ToWide(EntryId);
        WarningKey += L":";
        WarningKey += FieldName ? FieldName : L"Unknown";

        if (std::find(
                GLoggedWarnings.begin(),
                GLoggedWarnings.end(),
                WarningKey) != GLoggedWarnings.end())
        {
            return;
        }

        GLoggedWarnings.push_back(WarningKey);

        std::wstring Message =
            L"[BuildingCatalog] Missing visual path data: id=";
        Message += Utf8ToWide(EntryId);
        Message += L" field=";
        Message += FieldName ? FieldName : L"Unknown";
        Message += L"\n";
        OutputDebugStringW(Message.c_str());
    }

    const wchar_t* GetCatalogEntryIconPathInternal(const std::string& EntryId)
    {
        if (!EntryId.empty())
            LogMissingCatalogVisualPathOnce(EntryId, L"IconPath");

        return nullptr;
    }

    const wchar_t* GetCatalogEntrySpriteTexturePathInternal(
        const std::string& EntryId)
    {
        if (!EntryId.empty())
            LogMissingCatalogVisualPathOnce(EntryId, L"SpriteTexturePath");

        return nullptr;
    }
} // namespace

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
    constexpr size_t GRetiredCatalogSnapshotLimit = 4;

    std::shared_ptr<const std::vector<FBuildingCatalogEntry>>
        GCurrentBuildingCatalog;
    std::vector<std::shared_ptr<const std::vector<FBuildingCatalogEntry>>>
        GRetiredBuildingCatalogs;
    unsigned long long GBuildingCatalogGeneration = 0;

    std::vector<FBuildingCatalogEntry> BuildBuildingCatalogEntries();

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

    void ResetBuildingCatalogRuntimeDefaults()
    {
    }

    void ReplaceBuildingCatalogStore(
        std::vector<FBuildingCatalogEntry>&& Entries)
    {
        if (GCurrentBuildingCatalog)
        {
            GRetiredBuildingCatalogs.push_back(GCurrentBuildingCatalog);

            while (GRetiredBuildingCatalogs.size() >
                GRetiredCatalogSnapshotLimit)
            {
                GRetiredBuildingCatalogs.erase(
                    GRetiredBuildingCatalogs.begin());
            }
        }

        GCurrentBuildingCatalog =
            std::make_shared<const std::vector<FBuildingCatalogEntry>>(
                std::move(Entries));
        ++GBuildingCatalogGeneration;
    }

    void ReloadBuildingCatalogStore()
    {
        ReplaceBuildingCatalogStore(BuildBuildingCatalogEntries());
    }

    const std::vector<FBuildingCatalogEntry>& ResolveCurrentBuildingCatalog()
    {
        if (!GCurrentBuildingCatalog)
            ReloadBuildingCatalogStore();

        return *GCurrentBuildingCatalog;
    }

    const FBuildingCatalogEntry* FindCatalogEntryByCategoryLocalIndex(
        EBuildingCategory Category,
        int CategoryLocalIndex)
    {
        const auto& Catalog = ResolveCurrentBuildingCatalog();
        const auto It = std::find_if(
            Catalog.begin(),
            Catalog.end(),
            [&](const FBuildingCatalogEntry& Entry)
            {
                return Entry.Category == Category &&
                    Entry.CategoryLocalIndex == CategoryLocalIndex;
            });

        return It != Catalog.end() ? &(*It) : nullptr;
    }
}

const wchar_t* GetCatalogEntryIconPath(
    const FBuildingCatalogEntry& Entry)
{
    if (!Entry.IconPath.empty())
        return Entry.IconPath.c_str();

    return GetCatalogEntryIconPathInternal(Entry.Id);
}

const wchar_t* GetCatalogEntryIconPath(
    EBuildingCategory Category,
    int CategoryLocalIndex)
{
    if (const FBuildingCatalogEntry* Entry =
            FindCatalogEntryByCategoryLocalIndex(
                Category,
                CategoryLocalIndex))
    {
        return GetCatalogEntryIconPath(*Entry);
    }

    return GetCatalogEntryIconPathInternal(
        BuildCatalogEntryId(Category, CategoryLocalIndex));
}

const wchar_t* GetCatalogEntrySpriteTexturePath(
    const FBuildingCatalogEntry& Entry)
{
    if (!Entry.SpriteTexturePath.empty())
        return Entry.SpriteTexturePath.c_str();

    if (!Entry.IconPath.empty())
        return Entry.IconPath.c_str();

    return GetCatalogEntrySpriteTexturePathInternal(Entry.Id);
}

const wchar_t* GetCatalogEntrySpriteTexturePath(
    EBuildingCategory Category,
    int CategoryLocalIndex)
{
    if (const FBuildingCatalogEntry* Entry =
            FindCatalogEntryByCategoryLocalIndex(
                Category,
                CategoryLocalIndex))
    {
        return GetCatalogEntrySpriteTexturePath(*Entry);
    }

    return GetCatalogEntrySpriteTexturePathInternal(
        BuildCatalogEntryId(Category, CategoryLocalIndex));
}

namespace
{
    std::vector<FBuildingCatalogEntry> BuildBuildingCatalogEntries()
    {
        std::vector<FBuildingCatalogEntry> Entries;
        std::vector<FExternalCatalogRecord> SourceRecords;
        std::vector<FProductionRecipeRecord> ProductionRecipeRecords;
        std::vector<FCatalogCostOverrideRecord> CostOverrideRecords;
        std::vector<FCatalogSourceMetadataOverrideRecord>
            SourceMetadataOverrideRecords;
        std::vector<FCatalogWorkforceOverrideRecord>
            WorkforceOverrideRecords;
        std::vector<FCatalogPowerOverrideRecord> PowerOverrideRecords;
        std::vector<FCatalogPollutionOverrideRecord>
            PollutionOverrideRecords;
        std::vector<FCatalogServiceStatsOverrideRecord>
            ServiceStatsOverrideRecords;
        std::vector<FCatalogSizeOverrideRecord> SizeOverrideRecords;
        std::vector<FCatalogOperationModeOverrideRecord>
            OperationModeOverrideRecords;
        std::vector<FCatalogRuntimeUpgradeOverrideRecord>
            RuntimeUpgradeOverrideRecords;

        if (!LoadExternalCatalogRecords(SourceRecords))
            return Entries;

        LoadProductionRecipeRecords(ProductionRecipeRecords);
        LoadCatalogCostOverrideRecords(CostOverrideRecords);
        LoadCatalogSourceMetadataOverrideRecords(SourceMetadataOverrideRecords);
        LoadCatalogWorkforceOverrideRecords(WorkforceOverrideRecords);
        LoadCatalogPowerOverrideRecords(PowerOverrideRecords);
        LoadCatalogPollutionOverrideRecords(PollutionOverrideRecords);
        LoadCatalogServiceStatsOverrideRecords(ServiceStatsOverrideRecords);
        LoadCatalogSizeOverrideRecords(SizeOverrideRecords);
        LoadCatalogOperationModeOverrideRecords(OperationModeOverrideRecords);
        LoadCatalogRuntimeUpgradeOverrideRecords(RuntimeUpgradeOverrideRecords);

        Entries.reserve(SourceRecords.size());

        for (size_t RecordIndex = 0;
            RecordIndex < SourceRecords.size();
            ++RecordIndex)
        {
            const FExternalCatalogRecord& Record = SourceRecords[RecordIndex];
            const int CategoryIndex = static_cast<int>(Record.Category);
            const int i = Record.LocalIndex;

            if (!BuildingCategoryInfo::IsValidCategoryIndex(CategoryIndex) ||
                i < 0)
            {
                continue;
            }

            FBuildingCatalogEntry Entry;
            Entry.Id = "build_" + std::to_string(CategoryIndex + 1) +
                "_" + std::to_string(i + 1);
            Entry.DisplayName = Record.DisplayName;
            Entry.CategoryName =
                BuildingCategoryInfo::GetDisplayName(CategoryIndex);
            Entry.DetailText =
                Record.DetailText.empty() ?
                L"세부 데이터 준비 중" :
                Record.DetailText;
            Entry.BlueprintCostState = Record.BlueprintCostState;
            Entry.BlueprintCost = Record.BlueprintCost;
            Entry.ConstructionCostState = Record.ConstructionCostState;
            Entry.ConstructionCost = Record.ConstructionCost;
            if (const FCatalogCostOverrideRecord* CostOverride =
                    FindCatalogCostOverrideRecord(
                        CostOverrideRecords,
                        Entry.Id))
            {
                ApplyCatalogCostOverride(*CostOverride, Entry);
            }
            Entry.Category = Record.Category;
            Entry.CategoryLocalIndex = i;
            Entry.IconPath = Record.IconPath;
            Entry.SpriteTexturePath = Record.SpriteTexturePath;
            Entry.TemplateType =
                Record.HasTemplateType ?
                Record.TemplateType :
                ResolveLegacyTemplateTypeByBuildingId(Entry.Id);

            if (Entry.IconPath.empty())
            {
                const wchar_t* LegacyIconPath =
                    GetCatalogEntryIconPathInternal(Entry.Id);

                if (LegacyIconPath)
                    Entry.IconPath = LegacyIconPath;
            }

            if (Entry.SpriteTexturePath.empty())
                Entry.SpriteTexturePath = Entry.IconPath;

            InitializeDerivedCatalogEntry(Entry);
            if (const FCatalogPowerOverrideRecord* PowerOverride =
                    FindCatalogPowerOverrideRecord(
                        PowerOverrideRecords,
                        Entry.Id))
            {
                ApplyCatalogPowerOverride(*PowerOverride, Entry);
            }
            if (const FCatalogPollutionOverrideRecord* PollutionOverride =
                    FindCatalogPollutionOverrideRecord(
                        PollutionOverrideRecords,
                        Entry.Id))
            {
                ApplyCatalogPollutionOverride(*PollutionOverride, Entry);
            }

            if (const FCatalogSourceMetadataOverrideRecord* SourceOverride =
                    FindCatalogSourceMetadataOverrideRecord(
                        SourceMetadataOverrideRecords,
                        Entry.Id))
            {
                ApplyCatalogSourceMetadataOverride(*SourceOverride, Entry);
            }
            Entry.CategoryName = BuildingCategoryInfo::GetDisplayName(
                Entry.HasBuildMenuCategoryOverride ?
                    Entry.BuildMenuCategoryOverride :
                    Entry.Category);
            if (const FCatalogWorkforceOverrideRecord* WorkforceOverride =
                    FindCatalogWorkforceOverrideRecord(
                        WorkforceOverrideRecords,
                        Entry.Id))
            {
                ApplyCatalogWorkforceOverride(*WorkforceOverride, Entry);
            }
            if (const FCatalogServiceStatsOverrideRecord* ServiceStatsOverride =
                    FindCatalogServiceStatsOverrideRecord(
                        ServiceStatsOverrideRecords,
                        Entry.Id))
            {
                ApplyCatalogServiceStatsOverride(
                    *ServiceStatsOverride,
                    Entry);
            }
            else
            {
                ApplyBaseQualityMetadata(Entry);
            }

            if (const FCatalogSizeOverrideRecord* SizeOverride =
                    FindCatalogSizeOverrideRecord(
                        SizeOverrideRecords,
                        Entry.Id))
            {
                ApplyCatalogSizeOverride(*SizeOverride, Entry);
            }

            if (!Entry.Residential && Entry.Capacity <= 0)
            {
                Entry.JobSatisfactionCap = 0;
                Entry.BaseJobQuality = 0;
                Entry.RequiredEducationLevel =
                    ECitizenEducationLevel::Uneducated;
            }
            Entry.ProducedResourceType = EResourceType::None;
            Entry.ProducedResourceLabel.clear();
            Entry.ProductionInputTypes =
            {
                EResourceType::None,
                EResourceType::None
            };
            Entry.ProductionInputAmounts = {};
            Entry.ProductionInputLabels = {};
            Entry.VisitConsumptionResourceType = EResourceType::None;
            const FProductionRecipeRecord* const ProductionRecipe =
                FindProductionRecipeRecord(
                    ProductionRecipeRecords,
                    Entry.Id);

            if (ProductionRecipe)
            {
                Entry.ProducedResourceType =
                    ProductionRecipe->ProducedResourceType;
                Entry.ProducedResourceLabel =
                    ProductionRecipe->ProducedResourceLabel;
                Entry.ProductionInputTypes =
                    ProductionRecipe->ProductionInputTypes;
                Entry.ProductionInputAmounts =
                    ProductionRecipe->ProductionInputAmounts;
                Entry.ProductionInputLabels =
                    ProductionRecipe->ProductionInputLabels;
                Entry.VisitConsumptionResourceType =
                    ProductionRecipe->VisitConsumptionResourceType;
            }

            if (Entry.UsesRecipeTable &&
                !ProductionRecipe)
            {
                std::wstring Warning =
                    L"[BuildingCatalog] Missing production recipe: ";
                Warning += Utf8ToWide(Entry.Id);
                Warning += L"\n";
                OutputDebugStringW(Warning.c_str());
            }

            FinalizeCatalogEntryAfterRecipeLoad(Entry);
            Entry.OperationModeDefs =
                ExtractOperationModeDefsFromEntry(Entry);
            Entry.RuntimeUpgradeDefs =
                ExtractRuntimeUpgradeDefsFromEntry(Entry);
            Entry.UpgradeHints =
                ExtractUpgradeHintsFromDetail(Entry.DetailText);

            const std::vector<const FCatalogOperationModeOverrideRecord*>
                OperationModeOverrides =
                    FindCatalogOperationModeOverrideRecords(
                        OperationModeOverrideRecords,
                        Entry.Id);
            for (const FCatalogOperationModeOverrideRecord* OverrideRecord :
                OperationModeOverrides)
            {
                if (!OverrideRecord)
                    continue;

                ApplyCatalogOperationModeOverride(
                    Entry,
                    *OverrideRecord,
                    Entry.OperationModeDefs);
            }

            const std::vector<const FCatalogRuntimeUpgradeOverrideRecord*>
                RuntimeUpgradeOverrides =
                    FindCatalogRuntimeUpgradeOverrideRecords(
                        RuntimeUpgradeOverrideRecords,
                        Entry.Id);
            if (!RuntimeUpgradeOverrides.empty())
            {
                Entry.RuntimeUpgradeDefs.clear();

                for (const FCatalogRuntimeUpgradeOverrideRecord* OverrideRecord :
                    RuntimeUpgradeOverrides)
                {
                    if (!OverrideRecord)
                        continue;

                    ApplyCatalogRuntimeUpgradeOverride(
                        *OverrideRecord,
                        Entry.RuntimeUpgradeDefs);
                }
            }

            Entries.push_back(Entry);
        }

        ApplyCatalogUiBehaviorFlags(Entries);
        ::PopulateProductionChainMetadata(Entries);
        ValidateBuildingCatalogEntries(Entries);
        WriteBuildingCatalogVisualAuditDump(Entries);

        return Entries;
    }
}

const std::vector<FBuildingCatalogEntry>& GetBuildingCatalog()
{
    return ResolveCurrentBuildingCatalog();
}

void RegisterRuntimeConfig()
{
    RuntimeConfigRegistry::RegisterSource(
        {
            GCatalogConfigId,
            ResolveCatalogWatchPath(BuildCatalogDataCandidatePaths()),
            0.5f,
            &ResetBuildingCatalogRuntimeDefaults,
            nullptr,
            &ReloadBuildingCatalogStore
        });
    RuntimeConfigRegistry::RegisterSource(
        {
            GProductionRecipeConfigId,
            ResolveCatalogWatchPath(BuildProductionRecipeDataCandidatePaths()),
            0.5f,
            &ResetBuildingCatalogRuntimeDefaults,
            nullptr,
            &ReloadBuildingCatalogStore
        });
    RuntimeConfigRegistry::RegisterSource(
        {
            GCostOverrideConfigId,
            ResolveCatalogWatchPath(
                BuildCatalogCostOverrideCandidatePaths()),
            0.5f,
            &ResetBuildingCatalogRuntimeDefaults,
            nullptr,
            &ReloadBuildingCatalogStore
        });
    RuntimeConfigRegistry::RegisterSource(
        {
            GSourceMetadataOverrideConfigId,
            ResolveCatalogWatchPath(
                BuildCatalogSourceMetadataOverrideCandidatePaths()),
            0.5f,
            &ResetBuildingCatalogRuntimeDefaults,
            nullptr,
            &ReloadBuildingCatalogStore
        });
    RuntimeConfigRegistry::RegisterSource(
        {
            GWorkforceOverrideConfigId,
            ResolveCatalogWatchPath(
                BuildCatalogWorkforceOverrideCandidatePaths()),
            0.5f,
            &ResetBuildingCatalogRuntimeDefaults,
            nullptr,
            &ReloadBuildingCatalogStore
        });
    RuntimeConfigRegistry::RegisterSource(
        {
            GPowerOverrideConfigId,
            ResolveCatalogWatchPath(
                BuildCatalogPowerOverrideCandidatePaths()),
            0.5f,
            &ResetBuildingCatalogRuntimeDefaults,
            nullptr,
            &ReloadBuildingCatalogStore
        });
    RuntimeConfigRegistry::RegisterSource(
        {
            GPollutionOverrideConfigId,
            ResolveCatalogWatchPath(
                BuildCatalogPollutionOverrideCandidatePaths()),
            0.5f,
            &ResetBuildingCatalogRuntimeDefaults,
            nullptr,
            &ReloadBuildingCatalogStore
        });
    RuntimeConfigRegistry::RegisterSource(
        {
            GServiceStatsOverrideConfigId,
            ResolveCatalogWatchPath(
                BuildCatalogServiceStatsOverrideCandidatePaths()),
            0.5f,
            &ResetBuildingCatalogRuntimeDefaults,
            nullptr,
            &ReloadBuildingCatalogStore
        });
    RuntimeConfigRegistry::RegisterSource(
        {
            GSizeOverrideConfigId,
            ResolveCatalogWatchPath(
                BuildCatalogSizeOverrideCandidatePaths()),
            0.5f,
            &ResetBuildingCatalogRuntimeDefaults,
            nullptr,
            &ReloadBuildingCatalogStore
        });
    RuntimeConfigRegistry::RegisterSource(
        {
            GOperationModeOverrideConfigId,
            ResolveCatalogWatchPath(
                BuildCatalogOperationModeOverrideCandidatePaths()),
            0.5f,
            &ResetBuildingCatalogRuntimeDefaults,
            nullptr,
            &ReloadBuildingCatalogStore
        });
    RuntimeConfigRegistry::RegisterSource(
        {
            GRuntimeUpgradeOverrideConfigId,
            ResolveCatalogWatchPath(
                BuildCatalogRuntimeUpgradeOverrideCandidatePaths()),
            0.5f,
            &ResetBuildingCatalogRuntimeDefaults,
            nullptr,
            &ReloadBuildingCatalogStore
        });
}

unsigned long long GetRuntimeConfigGeneration()
{
    return GBuildingCatalogGeneration;
}

const FBuildingCatalogEntry* FindBuildingCatalogEntry(const std::string& EntryId)
{
    const auto& Catalog = GetBuildingCatalog();
    const auto It = std::find_if(
        Catalog.begin(), Catalog.end(),
        [&](const FBuildingCatalogEntry& Entry)
        {
            return Entry.Id == EntryId;
        });

    return It != Catalog.end() ? &(*It) : nullptr;
}

bool IsCustomsOfficeCatalogEntry(const FBuildingCatalogEntry& Entry)
{
    return Entry.IsCustomsOffice;
}

bool IsCustomsOfficeBuildingId(const std::string& EntryId)
{
    const FBuildingCatalogEntry* const Entry =
        FindBuildingCatalogEntry(EntryId);
    return Entry && IsCustomsOfficeCatalogEntry(*Entry);
}

EBuildingCategory GetEffectiveBuildMenuCategory(
    const FBuildingCatalogEntry& Entry)
{
    if (Entry.HasBuildMenuCategoryOverride)
        return Entry.BuildMenuCategoryOverride;

    if (Entry.Category == EBuildingCategory::Entertainment &&
        Entry.CategoryLocalIndex >= 12)
    {
        return EBuildingCategory::LuxuryEntertainment;
    }

    if (Entry.Category == EBuildingCategory::PublicService &&
        Entry.IsCustomsOffice)
    {
        return EBuildingCategory::GovernmentFinance;
    }

    return Entry.Category;
}

std::string GetCatalogEntryIconPathUtf8(const FBuildingCatalogEntry& Entry)
{
    const wchar_t* IconPath = GetCatalogEntryIconPath(Entry);

    if (!IconPath)
        return std::string();

    return WideToUtf8(std::wstring(IconPath));
}

std::string GetCatalogEntryIconPathUtf8(EBuildingCategory Category, int CategoryLocalIndex)
{
    const wchar_t* IconPath = GetCatalogEntryIconPath(Category, CategoryLocalIndex);

    if (!IconPath)
        return std::string();

    return WideToUtf8(std::wstring(IconPath));
}

std::string GetCatalogEntrySpriteTexturePathUtf8(
    const FBuildingCatalogEntry& Entry)
{
    const wchar_t* SpriteTexturePath =
        GetCatalogEntrySpriteTexturePath(Entry);

    if (!SpriteTexturePath)
        return std::string();

    return WideToUtf8(std::wstring(SpriteTexturePath));
}

std::string GetCatalogEntrySpriteTexturePathUtf8(
    EBuildingCategory Category,
    int CategoryLocalIndex)
{
    const wchar_t* SpriteTexturePath =
        GetCatalogEntrySpriteTexturePath(Category, CategoryLocalIndex);

    if (!SpriteTexturePath)
        return std::string();

    return WideToUtf8(std::wstring(SpriteTexturePath));
}

std::wstring GetBuildingProducedResourceDisplayName(
    const FBuildingCatalogEntry& Entry)
{
    if (Entry.ProducedResourceType == EResourceType::None)
        return std::wstring();

    if (!Entry.ProducedResourceLabel.empty())
        return Entry.ProducedResourceLabel;

    return std::wstring(GetResourceTypeDisplayName(Entry.ProducedResourceType));
}

std::wstring GetBuildingProductionInputDisplayName(
    const FBuildingCatalogEntry& Entry,
    int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= GProductionInputSlotCount)
        return std::wstring();

    const size_t Index = static_cast<size_t>(SlotIndex);
    const EResourceType InputType = Entry.ProductionInputTypes[Index];
    const int InputAmount = Entry.ProductionInputAmounts[Index];

    if (InputType == EResourceType::None || InputAmount <= 0)
        return std::wstring();

    if (!Entry.ProductionInputLabels[Index].empty())
        return Entry.ProductionInputLabels[Index];

    return std::wstring(GetResourceTypeDisplayName(InputType));
}

const wchar_t* GetProductionChainStageDisplayName(
    FBuildingCatalogEntry::EProductionChainStage Stage)
{
    switch (Stage)
    {
    case FBuildingCatalogEntry::EProductionChainStage::Primary:
        return L"1차 자원";
    case FBuildingCatalogEntry::EProductionChainStage::Intermediate:
        return L"중간재";
    case FBuildingCatalogEntry::EProductionChainStage::Final:
        return L"완제품";
    default:
        break;
    }

    return L"";
}

std::wstring BuildProductionChainSummary(
    const FBuildingCatalogEntry& Entry)
{
    if (!Entry.SupplyChainSummary.empty())
        return Entry.SupplyChainSummary;

    const std::wstring OutputLabel =
        GetBuildingProducedResourceDisplayName(Entry);
    const std::vector<std::wstring> DemandLabels =
        BuildProductionDemandDisplayLabels(Entry);

    if (OutputLabel.empty())
    {
        if (DemandLabels.empty() || Entry.DisplayName.empty())
            return std::wstring();

        return JoinLabels(DemandLabels, L" + ") +
            L" -> " +
            Entry.DisplayName;
    }

    if (DemandLabels.empty())
        return OutputLabel + L" 생산";

    return JoinLabels(DemandLabels, L" + ") +
        L" -> " +
        OutputLabel;
}

std::wstring GetOperationModeDisplayName(
    const FBuildingCatalogEntry& Entry,
    int ModeIndex)
{
    if (ModeIndex < 0 ||
        ModeIndex >= static_cast<int>(Entry.OperationModeDefs.size()))
    {
        return std::wstring();
    }

    return Entry.OperationModeDefs[static_cast<size_t>(ModeIndex)].DisplayName;
}

namespace
{
    std::wstring BuildOperationModeSummary(
        const FBuildingOperationModeDef& ModeDef)
    {
        std::vector<std::wstring> Segments;

        if (ModeDef.HasUnlockEra)
        {
            Segments.push_back(
                std::wstring(GetBuildingEraDisplayName(ModeDef.UnlockEra)));
        }

        if (!ModeDef.RequiredResearch.empty())
            Segments.push_back(ModeDef.RequiredResearch);

        if (!ModeDef.EffectSummary.empty())
            Segments.push_back(ModeDef.EffectSummary);

        std::wstring Result;
        for (size_t Index = 0; Index < Segments.size(); ++Index)
        {
            if (Segments[Index].empty())
                continue;

            if (!Result.empty())
                Result += L" / ";

            Result += Segments[Index];
        }

        return Result;
    }
}

std::wstring GetOperationModeEffectSummary(
    const FBuildingCatalogEntry& Entry,
    int ModeIndex)
{
    if (ModeIndex < 0 ||
        ModeIndex >= static_cast<int>(Entry.OperationModeDefs.size()))
    {
        return std::wstring();
    }

    return BuildOperationModeSummary(
        Entry.OperationModeDefs[static_cast<size_t>(ModeIndex)]);
}

std::wstring GetRuntimeUpgradeDisplayName(
    const FBuildingCatalogEntry& Entry,
    int UpgradeIndex)
{
    if (UpgradeIndex < 0 ||
        UpgradeIndex >= static_cast<int>(Entry.RuntimeUpgradeDefs.size()))
    {
        return std::wstring();
    }

    return Entry.RuntimeUpgradeDefs[static_cast<size_t>(UpgradeIndex)].
        DisplayName;
}

std::wstring GetRuntimeUpgradeEffectSummary(
    const FBuildingCatalogEntry& Entry,
    int UpgradeIndex)
{
    if (UpgradeIndex < 0 ||
        UpgradeIndex >= static_cast<int>(Entry.RuntimeUpgradeDefs.size()))
    {
        return std::wstring();
    }

    return Entry.RuntimeUpgradeDefs[static_cast<size_t>(UpgradeIndex)].
        EffectSummary;
}
