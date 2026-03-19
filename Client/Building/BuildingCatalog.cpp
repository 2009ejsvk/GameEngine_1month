#include "BuildingCatalog.h"
#include "BuildingCatalogData.h"
#include "BuildingCatalogDerived.h"
#include "BuildingCatalogLoader.h"
#include "BuildingCategoryInfo.h"
#include "../StringUtils.h"
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
    using StringUtils::Utf8ToWide;
    using StringUtils::WideToUtf8;
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
        std::vector<EResourceType> VisitConsumptionAcceptedResourceTypes;
        std::array<EResourceType, GProductionInputSlotCount>
            ProductionInputTypes = {};
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
        bool HasProducedResourceTypeOverride = false;
        EResourceType ProducedResourceTypeOverride = EResourceType::None;
        // ProducedType may stay blank for input-only modes. Treat input arrays
        // as a first-class recipe override rather than a secondary hint.
        bool HasProductionInputTypesOverride = false;
        std::array<EResourceType, GProductionInputSlotCount>
            ProductionInputTypesOverride = {};
        std::array<int, GProductionInputSlotCount>
            ProductionInputAmountsOverride = {};
        bool HasVisitConsumptionTypeOverride = false;
        EResourceType VisitConsumptionTypeOverride = EResourceType::None;
        bool HasVisitConsumptionAcceptedTypesOverride = false;
        std::vector<EResourceType> VisitConsumptionAcceptedTypesOverride;
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

    bool TryParseCatalogCostText(
        const std::wstring& Text,
        EBuildingCostState& OutState,
        int& OutCost)
    {
        const auto Trim = [](const std::wstring& Value)
        {
            size_t Start = 0;

            while (Start < Value.size() && iswspace(Value[Start]))
                ++Start;

            size_t End = Value.size();

            while (End > Start && iswspace(Value[End - 1]))
                --End;

            return Value.substr(Start, End - Start);
        };

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
        if (!Prefix || !*Prefix)
            return false;

        const auto Trim = [](const std::wstring& Value)
        {
            size_t Start = 0;

            while (Start < Value.size() && iswspace(Value[Start]))
                ++Start;

            size_t End = Value.size();

            while (End > Start && iswspace(Value[End - 1]))
                --End;

            return Value.substr(Start, End - Start);
        };

        const size_t PrefixLength = wcslen(Prefix);
        size_t Cursor = 0;

        while (Cursor <= DetailText.size())
        {
            const size_t LineEnd = DetailText.find(L'\n', Cursor);
            const size_t SliceEnd =
                LineEnd == std::wstring::npos ? DetailText.size() : LineEnd;
            std::wstring Line = DetailText.substr(Cursor, SliceEnd - Cursor);

            if (!Line.empty() && Line.back() == L'\r')
                Line.pop_back();

            Line = Trim(Line);

            if (Line.size() >= PrefixLength &&
                Line.compare(0, PrefixLength, Prefix) == 0)
            {
                return TryParseCatalogCostText(
                    Line.substr(PrefixLength),
                    OutState,
                    OutCost);
            }

            if (LineEnd == std::wstring::npos)
                break;

            Cursor = LineEnd + 1;
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

    void AppendBuildingCatalogLoadTraceLine(
        const std::wstring& Message);

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
                GBuildingWealthMaskRich |
                GBuildingWealthMaskFilthyRich;
        }

        if ((RawMask & 2) != 0)
        {
            Normalized |=
                GBuildingWealthMaskWellOff |
                GBuildingWealthMaskRich |
                GBuildingWealthMaskFilthyRich;
        }

        if ((RawMask & 4) != 0)
            Normalized |= GBuildingWealthMaskRich | GBuildingWealthMaskFilthyRich;

        if ((RawMask & 8) != 0)
            Normalized |= GBuildingWealthMaskFilthyRich;

        if ((RawMask & 16) != 0)
            Normalized |= GBuildingWealthMaskBroke;

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
            Normalized == "?�동")
        {
            OutPreference = ETouristPreference::Family;
        }
        else if (
            EqualsIgnoreCaseAscii(Normalized, "Backpacker") ||
            Normalized == "배낭?�행")
        {
            OutPreference = ETouristPreference::Backpacker;
        }
        else if (
            EqualsIgnoreCaseAscii(Normalized, "Relaxation") ||
            Normalized == "?�양")
        {
            OutPreference = ETouristPreference::Relaxation;
        }
        else if (
            EqualsIgnoreCaseAscii(Normalized, "ThrillSeeker") ||
            EqualsIgnoreCaseAscii(Normalized, "Thrill Seeker") ||
            Normalized == "?�릴중독")
        {
            OutPreference = ETouristPreference::ThrillSeeker;
        }
        else if (
            EqualsIgnoreCaseAscii(Normalized, "Celebrity") ||
            false)
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
        else if (Key == "FeedCrops")
            OutType = EResourceType::FeedCrops;
        else if (Key == "Banana")
            OutType = EResourceType::Banana;
        else if (Key == "Cocoa")
            OutType = EResourceType::Cocoa;
        else if (Key == "Coffee")
            OutType = EResourceType::Coffee;
        else if (Key == "Corn")
            OutType = EResourceType::Corn;
        else if (Key == "Cotton")
            OutType = EResourceType::Cotton;
        else if (Key == "Pineapple")
            OutType = EResourceType::Pineapple;
        else if (Key == "Rubber")
            OutType = EResourceType::Rubber;
        else if (Key == "Sugar")
            OutType = EResourceType::Sugar;
        else if (Key == "Tobacco")
            OutType = EResourceType::Tobacco;
        else if (Key == "Meat")
            OutType = EResourceType::Meat;
        else if (Key == "Milk")
            OutType = EResourceType::Milk;
        else if (Key == "Hides")
            OutType = EResourceType::Hides;
        else if (Key == "Wool")
            OutType = EResourceType::Wool;
        else if (Key == "Coal")
            OutType = EResourceType::Coal;
        else if (Key == "Iron")
            OutType = EResourceType::Iron;
        else if (Key == "Gold")
            OutType = EResourceType::Gold;
        else if (Key == "Nickel")
            OutType = EResourceType::Nickel;
        else if (Key == "Aluminum")
            OutType = EResourceType::Aluminum;
        else if (Key == "Uranium")
            OutType = EResourceType::Uranium;
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
        else if (Key == "SpecialChocolate")
            OutType = EResourceType::SpecialChocolate;
        else if (Key == "Goldnuts")
            OutType = EResourceType::Goldnuts;
        else if (Key == "BS")
            OutType = EResourceType::BS;
        else
            return false;

        return true;
    }

    std::vector<EResourceType> ParseResourceTypeList(
        const std::string& Text)
    {
        std::vector<EResourceType> Result;
        std::string CurrentToken;

        auto FlushToken = [&]()
        {
            if (CurrentToken.empty())
                return;

            EResourceType ParsedType = EResourceType::None;
            if (TryParseResourceTypeKey(CurrentToken, ParsedType) &&
                ParsedType != EResourceType::None &&
                std::find(Result.begin(), Result.end(), ParsedType) ==
                    Result.end())
            {
                Result.push_back(ParsedType);
            }

            CurrentToken.clear();
        };

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const char Ch = Text[Index];

            if (Ch == ',' || Ch == '|' || Ch == '/' || Ch == ';')
            {
                FlushToken();
                continue;
            }

            CurrentToken.push_back(Ch);
        }

        FlushToken();
        return Result;
    }

    bool LoadExternalCatalogRecords(
        std::vector<FExternalCatalogRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildingCatalogLoader::BuildCatalogDataCandidatePaths();
        AppendBuildingCatalogLoadTraceLine(
            L"=== LoadExternalCatalogRecords ===");

        for (size_t PathIndex = 0; PathIndex < CandidatePaths.size(); ++PathIndex)
        {
            std::string FileContent;
            AppendBuildingCatalogLoadTraceLine(
                L"Try: " + CandidatePaths[PathIndex]);

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
            {
                AppendBuildingCatalogLoadTraceLine(
                    L"Miss: " + CandidatePaths[PathIndex]);
                continue;
            }

            AppendBuildingCatalogLoadTraceLine(
                L"Loaded text: " + CandidatePaths[PathIndex]);

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
                AppendBuildingCatalogLoadTraceLine(
                    L"Success: " + CandidatePaths[PathIndex] +
                    L" rows=" + std::to_wstring(OutRecords.size()));
                return true;
            }
        }

        AppendBuildingCatalogLoadTraceLine(
            L"Failure: BuildingCatalog.tsv candidates exhausted");
        OutputDebugStringW(
            L"[BuildingCatalog] Failed to load BuildingCatalog.tsv\n");
        return false;
    }

    bool LoadProductionRecipeRecords(
        std::vector<FProductionRecipeRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildingCatalogLoader::BuildProductionRecipeDataCandidatePaths();

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

                        const size_t InputFieldBaseIndex = 3;
                        const size_t VisitFieldIndex =
                            InputFieldBaseIndex +
                            static_cast<size_t>(GProductionInputSlotCount) * 3;
                        const size_t VisitAcceptedFieldIndex =
                            VisitFieldIndex + 1;

                        for (int SlotIndex = 0;
                            SlotIndex < GProductionInputSlotCount;
                            ++SlotIndex)
                        {
                            const size_t BaseFieldIndex =
                                InputFieldBaseIndex +
                                static_cast<size_t>(SlotIndex) * 3;

                            if (BaseFieldIndex + 2 >= Fields.size())
                                continue;

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

                        if (VisitFieldIndex < Fields.size())
                        {
                            TryParseResourceTypeKey(
                                Fields[VisitFieldIndex],
                                Record.VisitConsumptionResourceType);
                        }

                        if (VisitAcceptedFieldIndex < Fields.size())
                        {
                            Record.VisitConsumptionAcceptedResourceTypes =
                                ParseResourceTypeList(
                                    Fields[VisitAcceptedFieldIndex]);
                        }

                        bool HasRecipeData =
                            Record.ProducedResourceType !=
                                EResourceType::None ||
                            Record.VisitConsumptionResourceType !=
                                EResourceType::None ||
                            !Record.VisitConsumptionAcceptedResourceTypes.empty();

                        for (int SlotIndex = 0;
                            SlotIndex < GProductionInputSlotCount &&
                                !HasRecipeData;
                            ++SlotIndex)
                        {
                            if (Record.ProductionInputTypes[
                                    static_cast<size_t>(SlotIndex)] !=
                                    EResourceType::None &&
                                Record.ProductionInputAmounts[
                                    static_cast<size_t>(SlotIndex)] > 0)
                            {
                                HasRecipeData = true;
                            }
                        }

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
            BuildingCatalogLoader::BuildCatalogCostOverrideCandidatePaths();

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
            BuildingCatalogLoader::BuildCatalogSourceMetadataOverrideCandidatePaths();

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
            BuildingCatalogLoader::BuildCatalogWorkforceOverrideCandidatePaths();

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
            BuildingCatalogLoader::BuildCatalogPowerOverrideCandidatePaths();

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
            BuildingCatalogLoader::BuildCatalogPollutionOverrideCandidatePaths();

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
            BuildingCatalogLoader::BuildCatalogServiceStatsOverrideCandidatePaths();

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
            BuildingCatalogLoader::BuildCatalogSizeOverrideCandidatePaths();

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
            BuildingCatalogLoader::BuildCatalogOperationModeOverrideCandidatePaths();

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

                        if (Fields.size() >= 21)
                        {
                            const std::string ProducedTypeKey =
                                UnescapeCatalogField(Fields[20]);

                            // Input-switching modes intentionally leave
                            // ProducedType empty and only fill InputN fields.
                            if (!ProducedTypeKey.empty() &&
                                TryParseResourceTypeKey(
                                    ProducedTypeKey,
                                    Record.ProducedResourceTypeOverride))
                            {
                                Record.HasProducedResourceTypeOverride = true;
                            }
                        }

                        for (int SlotIndex = 0;
                            SlotIndex < GProductionInputSlotCount;
                            ++SlotIndex)
                        {
                            const size_t TypeFieldIndex =
                                21 + static_cast<size_t>(SlotIndex) * 2;
                            const size_t AmountFieldIndex =
                                TypeFieldIndex + 1;

                            if (TypeFieldIndex >= Fields.size())
                                break;

                            const std::string TypeKey =
                                UnescapeCatalogField(Fields[TypeFieldIndex]);

                            if (TypeKey.empty())
                                continue;

                            EResourceType ParsedType = EResourceType::None;
                            if (!TryParseResourceTypeKey(TypeKey, ParsedType) ||
                                ParsedType == EResourceType::None)
                            {
                                continue;
                            }

                            int ParsedAmount = 0;
                            if (AmountFieldIndex >= Fields.size() ||
                                !TryParseCatalogIntegerField(
                                    Fields[AmountFieldIndex],
                                    ParsedAmount) ||
                                ParsedAmount <= 0)
                            {
                                continue;
                            }

                            Record.HasProductionInputTypesOverride = true;
                            Record.ProductionInputTypesOverride[
                                static_cast<size_t>(SlotIndex)] = ParsedType;
                            Record.ProductionInputAmountsOverride[
                                static_cast<size_t>(SlotIndex)] = ParsedAmount;
                        }

                        const size_t VisitConsumptionTypeFieldIndex =
                            21 +
                            static_cast<size_t>(GProductionInputSlotCount) * 2;
                        const size_t VisitAcceptedTypesFieldIndex =
                            VisitConsumptionTypeFieldIndex + 1;

                        if (VisitConsumptionTypeFieldIndex < Fields.size())
                        {
                            const std::string VisitTypeKey =
                                UnescapeCatalogField(
                                    Fields[VisitConsumptionTypeFieldIndex]);

                            if (!VisitTypeKey.empty() &&
                                TryParseResourceTypeKey(
                                    VisitTypeKey,
                                    Record.VisitConsumptionTypeOverride))
                            {
                                Record.HasVisitConsumptionTypeOverride = true;
                            }
                        }

                        if (VisitAcceptedTypesFieldIndex < Fields.size())
                        {
                            const std::string AcceptedTypesText =
                                UnescapeCatalogField(
                                    Fields[VisitAcceptedTypesFieldIndex]);

                            if (AcceptedTypesText == "-" ||
                                AcceptedTypesText == "None")
                            {
                                Record.HasVisitConsumptionAcceptedTypesOverride =
                                    true;
                                Record.VisitConsumptionAcceptedTypesOverride
                                    .clear();
                            }
                            else if (!AcceptedTypesText.empty())
                            {
                                Record.HasVisitConsumptionAcceptedTypesOverride =
                                    true;
                                Record.VisitConsumptionAcceptedTypesOverride =
                                    ParseResourceTypeList(AcceptedTypesText);
                            }
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
            BuildingCatalogLoader::BuildCatalogRuntimeUpgradeOverrideCandidatePaths();

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

    std::wstring ResolveCatalogTextureFullPathImpl(const std::wstring& Path)
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

    std::wstring ResolveBuildingCatalogLoadTracePath()
    {
        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring DebugDirectory =
                JoinPath(RootPath, L"Debug");
            return JoinPath(
                DebugDirectory,
                L"BuildingCatalogLoadTrace.log");
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
                ResolveCatalogTextureFullPathImpl(Entry.IconPath);
            const std::wstring FullSpritePath =
                ResolveCatalogTextureFullPathImpl(Entry.SpriteTexturePath);

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

    void AppendBuildingCatalogLoadTraceLine(
        const std::wstring& Message)
    {
        const std::wstring TracePath =
            ResolveBuildingCatalogLoadTracePath();
        if (TracePath.empty())
            return;

        const std::wstring TraceDirectory =
            GetParentDirectoryPath(TracePath);
        if (!EnsureDirectoryExists(TraceDirectory))
            return;

        FILE* File = nullptr;
        if (_wfopen_s(&File, TracePath.c_str(), L"ab") != 0 || !File)
            return;

        const std::string Utf8Line =
            WideToUtf8(Message) + "\r\n";
        fwrite(Utf8Line.data(), 1, Utf8Line.size(), File);
        fclose(File);
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
                    ResolveCatalogTextureFullPathImpl(Entry.IconPath);
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
                    ResolveCatalogTextureFullPathImpl(Entry.SpriteTexturePath);
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

        if (OverrideRecord.HasProducedResourceTypeOverride)
        {
            ModeDef.Effect.HasProducedResourceTypeOverride = true;
            ModeDef.Effect.ProducedResourceTypeOverride =
                OverrideRecord.ProducedResourceTypeOverride;
        }

        if (OverrideRecord.HasProductionInputTypesOverride)
        {
            ModeDef.Effect.HasProductionInputTypesOverride = true;
            ModeDef.Effect.ProductionInputTypesOverride =
                OverrideRecord.ProductionInputTypesOverride;
            ModeDef.Effect.ProductionInputAmountsOverride =
                OverrideRecord.ProductionInputAmountsOverride;
        }

        if (OverrideRecord.HasVisitConsumptionTypeOverride)
        {
            ModeDef.Effect.HasVisitConsumptionTypeOverride = true;
            ModeDef.Effect.VisitConsumptionTypeOverride =
                OverrideRecord.VisitConsumptionTypeOverride;
        }

        if (OverrideRecord.HasVisitConsumptionAcceptedTypesOverride)
        {
            ModeDef.Effect.HasVisitConsumptionAcceptedTypesOverride = true;
            ModeDef.Effect.VisitConsumptionAcceptedTypesOverride =
                OverrideRecord.VisitConsumptionAcceptedTypesOverride;
        }

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

    void ApplyDefaultOperationModeResourceBaseline(
        FBuildingCatalogEntry& Entry)
    {
        if (Entry.OperationModeDefs.empty())
            return;

        const FBuildingOperationModeEffect& DefaultEffect =
            Entry.OperationModeDefs.front().Effect;

        if (DefaultEffect.HasProducedResourceTypeOverride &&
            DefaultEffect.ProducedResourceTypeOverride !=
                EResourceType::None)
        {
            Entry.ProducedResourceType =
                DefaultEffect.ProducedResourceTypeOverride;
            Entry.ProducedResourceLabel.clear();
        }

        if (DefaultEffect.HasProductionInputTypesOverride)
        {
            // Default mode may keep the same output while changing the active
            // recipe. Mirror that at catalog baseline so new buildings start
            // with the correct exact inputs before any runtime mode switch.
            Entry.ProductionInputTypes =
                DefaultEffect.ProductionInputTypesOverride;
            Entry.ProductionInputAmounts =
                DefaultEffect.ProductionInputAmountsOverride;
            Entry.ProductionInputLabels = {};
        }

        if (DefaultEffect.HasVisitConsumptionTypeOverride)
        {
            Entry.VisitConsumptionResourceType =
                DefaultEffect.VisitConsumptionTypeOverride;
        }

        if (DefaultEffect.HasVisitConsumptionAcceptedTypesOverride)
        {
            Entry.VisitConsumptionAcceptedResourceTypes =
                DefaultEffect.VisitConsumptionAcceptedTypesOverride;
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
            BuildingCatalogData::FindBuildingCatalogEntryByCategoryLocalIndex(
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
            BuildingCatalogData::FindBuildingCatalogEntryByCategoryLocalIndex(
                Category,
                CategoryLocalIndex))
    {
        return GetCatalogEntrySpriteTexturePath(*Entry);
    }

    return GetCatalogEntrySpriteTexturePathInternal(
        BuildCatalogEntryId(Category, CategoryLocalIndex));
}

std::wstring ResolveCatalogTextureFullPath(const wchar_t* Path)
{
    return Path ?
        ResolveCatalogTextureFullPathImpl(std::wstring(Path)) :
        std::wstring();
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
                L"Details pending" :
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
            Entry.ProductionInputTypes = {};
            Entry.ProductionInputAmounts = {};
            Entry.ProductionInputLabels = {};
            Entry.VisitConsumptionResourceType = EResourceType::None;
            Entry.VisitConsumptionAcceptedResourceTypes.clear();
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
                Entry.VisitConsumptionAcceptedResourceTypes =
                    ProductionRecipe->VisitConsumptionAcceptedResourceTypes;
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

            ApplyDefaultOperationModeResourceBaseline(Entry);
            FinalizeCatalogEntryAfterRecipeLoad(Entry);

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

namespace BuildingCatalogLoader
{
    std::vector<FBuildingCatalogEntry> BuildBuildingCatalogEntries()
    {
        return ::BuildBuildingCatalogEntries();
    }
}
