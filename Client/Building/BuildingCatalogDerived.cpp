#include "BuildingCatalogDerived.h"
#include "BuildingCatalog.h"
#include "../StringUtils.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cwchar>
#include <cwctype>
#include <initializer_list>
#include <vector>

namespace
{
    using StringUtils::SplitLines;
    using StringUtils::Trim;

    constexpr std::array<int, 12> GHousingCaps =
    {
        10, 20, 30, 75, 50, 25, 60, 40, 45, 70, 85, 90
    };

    constexpr std::array<int, 25> GEntertainmentFunCaps =
    {
        45, 70, 40, 55, 68, 60, 58, 52, 65, 72,
        78, 80, 60, 75, 65, 70, 70, 60, 80, 65,
        65, 65, 75, 70, 95
    };

    bool NameContains(
        const std::wstring& Name,
        const wchar_t* Pattern)
    {
        return Pattern && Name.find(Pattern) != std::wstring::npos;
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

    bool TryExtractDetailLineValue(
        const std::wstring& DetailText,
        const wchar_t* Prefix,
        std::wstring& OutValue)
    {
        OutValue.clear();

        if (!Prefix || !*Prefix)
            return false;

        const std::vector<std::wstring> Lines = SplitLines(DetailText);

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

    void ResolveCategoryDefaultFlags(
        EBuildingCategory Category,
        bool& OutResidential,
        bool& OutFoodProvider,
        bool& OutEntertainmentProvider)
    {
        OutResidential = false;
        OutFoodProvider = false;
        OutEntertainmentProvider = false;

        switch (Category)
        {
        case EBuildingCategory::Housing:
            OutResidential = true;
            break;
        case EBuildingCategory::Entertainment:
        case EBuildingCategory::LuxuryEntertainment:
            OutEntertainmentProvider = true;
            break;
        default:
            break;
        }
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
        std::wstring RichParseText = TrimmedValue;
        bool HasBroke = false;
        bool HasPoor = false;
        bool HasWellOff = false;
        bool HasRich = false;
        bool HasFilthyRich = false;

        if (TrimmedValue.find(L"파산") != std::wstring::npos ||
            TrimmedValue.find(L"무일푼") != std::wstring::npos)
        {
            HasBroke = true;
        }

        if (TrimmedValue.find(L"가난") != std::wstring::npos)
            HasPoor = true;

        if (TrimmedValue.find(L"유복") != std::wstring::npos)
            HasWellOff = true;

        const size_t FilthyRichPos = TrimmedValue.find(L"더럽게 부유");
        if (FilthyRichPos != std::wstring::npos)
        {
            HasFilthyRich = true;
            RichParseText.erase(FilthyRichPos, wcslen(L"더럽게 부유"));
        }

        if (RichParseText.find(L"부유") != std::wstring::npos)
            HasRich = true;

        if (TrimmedValue.find(L"이상") != std::wstring::npos)
        {
            if (HasBroke)
            {
                HasPoor = true;
                HasWellOff = true;
                HasRich = true;
                HasFilthyRich = true;
            }
            else if (HasPoor)
            {
                HasWellOff = true;
                HasRich = true;
                HasFilthyRich = true;
            }
            else if (HasWellOff)
            {
                HasRich = true;
                HasFilthyRich = true;
            }
            else if (HasRich)
            {
                HasFilthyRich = true;
            }
        }

        // Single-tier requirements are treated as minimum wealth bands,
        // except "파산", which remains a strict lowest-tier requirement.
        if (!HasBroke && HasPoor && !HasWellOff && !HasRich && !HasFilthyRich)
        {
            HasWellOff = true;
            HasRich = true;
            HasFilthyRich = true;
        }
        else if (HasWellOff && !HasRich && !HasFilthyRich)
        {
            HasRich = true;
            HasFilthyRich = true;
        }
        else if (HasRich && !HasFilthyRich)
        {
            HasFilthyRich = true;
        }

        if (HasBroke)
            Mask |= GBuildingWealthMaskBroke;
        if (HasPoor)
            Mask |= GBuildingWealthMaskPoor;
        if (HasWellOff)
            Mask |= GBuildingWealthMaskWellOff;
        if (HasRich)
            Mask |= GBuildingWealthMaskRich;
        if (HasFilthyRich)
            Mask |= GBuildingWealthMaskFilthyRich;

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

    bool IsCatalogIdOneOf(
        const std::string& Id,
        std::initializer_list<const char*> Candidates)
    {
        for (const char* Candidate : Candidates)
        {
            if (Candidate && Id == Candidate)
                return true;
        }

        return false;
    }

    EBuildingEra ParseUnlockEra(const std::wstring& DetailText)
    {
        if (DetailText.find(L"식민지 시대") != std::wstring::npos)
            return EBuildingEra::Colonial;

        if (DetailText.find(L"세계대전 시대") != std::wstring::npos)
            return EBuildingEra::WorldWars;

        if (DetailText.find(L"냉전 시대") != std::wstring::npos)
            return EBuildingEra::ColdWar;

        if (DetailText.find(L"현대 시대") != std::wstring::npos)
            return EBuildingEra::Modern;

        return EBuildingEra::Colonial;
    }

    EBuildingHousingClass ResolveHousingClass(const std::string& Id)
    {
        if (IsCatalogIdOneOf(Id,
            {
                "build_4_1",
                "build_4_3",
                "build_4_6",
                "build_4_8",
                "build_4_9"
            }))
        {
            return EBuildingHousingClass::Collective;
        }

        if (IsCatalogIdOneOf(Id,
            {
                "build_4_2",
                "build_4_4",
                "build_4_5",
                "build_4_11",
                "build_4_12"
            }))
        {
            return EBuildingHousingClass::Elite;
        }

        if (IsCatalogIdOneOf(Id, { "build_4_7", "build_4_10" }))
            return EBuildingHousingClass::Standard;

        return EBuildingHousingClass::None;
    }

    EBuildingLeisureClass ResolveLeisureClass(
        EBuildingCategory Category,
        const std::string& Id)
    {
        if (Category != EBuildingCategory::Entertainment)
            return EBuildingLeisureClass::None;

        if (IsCatalogIdOneOf(Id, { "build_5_13", "build_5_14", "build_5_24" }))
            return EBuildingLeisureClass::Cultural;

        if (IsCatalogIdOneOf(Id,
            {
                "build_5_16",
                "build_5_17",
                "build_5_18",
                "build_5_19",
                "build_5_20",
                "build_5_25"
            }))
        {
            return EBuildingLeisureClass::Luxury;
        }

        return EBuildingLeisureClass::General;
    }

    ETouristPreference ParsePrimaryTouristPreference(
        const std::wstring& DetailText)
    {
        const size_t MarkerPos = DetailText.find(L"선호 관광객:");

        if (MarkerPos == std::wstring::npos)
            return ETouristPreference::None;

        const std::wstring TouristLine =
            DetailText.substr(MarkerPos);

        if (TouristLine.find(L"문화") != std::wstring::npos)
            return ETouristPreference::Cultural;
        if (TouristLine.find(L"아동") != std::wstring::npos)
            return ETouristPreference::Family;
        if (TouristLine.find(L"배낭여행") != std::wstring::npos)
            return ETouristPreference::Backpacker;
        if (TouristLine.find(L"휴양") != std::wstring::npos)
            return ETouristPreference::Relaxation;
        if (TouristLine.find(L"스릴중독") != std::wstring::npos)
            return ETouristPreference::ThrillSeeker;
        if (TouristLine.find(L"유명인") != std::wstring::npos)
            return ETouristPreference::Celebrity;

        return ETouristPreference::None;
    }

    bool ResolveHealthProvider(const FBuildingCatalogEntry& Entry)
    {
        if (Entry.DetailText.find(L"보건 제공") != std::wstring::npos)
            return true;

        return IsCatalogIdOneOf(
            Entry.Id,
            { "build_8_12" });
    }

    bool ResolveFaithProvider(const FBuildingCatalogEntry& Entry)
    {
        if (Entry.DetailText.find(L"신앙 제공") != std::wstring::npos)
            return true;

        return false;
    }

    int ResolveHealthSatisfactionCap(const FBuildingCatalogEntry& Entry)
    {
        if (IsCatalogIdOneOf(Entry.Id, { "build_8_4" }))
            return 62;
        if (IsCatalogIdOneOf(Entry.Id, { "build_8_7" }))
            return 80;
        if (IsCatalogIdOneOf(Entry.Id, { "build_8_12" }))
            return 72;

        return Entry.HealthProvider ? 60 : 100;
    }

    int ResolveFaithSatisfactionCap(const FBuildingCatalogEntry& Entry)
    {
        if (IsCatalogIdOneOf(Entry.Id, { "build_8_1" }))
            return 56;
        if (IsCatalogIdOneOf(Entry.Id, { "build_8_3" }))
            return 68;
        if (IsCatalogIdOneOf(Entry.Id, { "build_8_6" }))
            return 82;
        return Entry.FaithProvider ? 60 : 100;
    }

    EPlacementBuildingKind ResolvePlacementBuildingKind(
        const std::string& BuildingId)
    {
        if (BuildingId == "build_1_1")
            return EPlacementBuildingKind::Road;
        if (BuildingId == "build_1_3")
            return EPlacementBuildingKind::Harbor;
        if (BuildingId == "build_1_5")
            return EPlacementBuildingKind::TransportOffice;

        return EPlacementBuildingKind::Structure;
    }

    void ApplyCatalogIdentityFlags(
        FBuildingCatalogEntry& Entry)
    {
        Entry.IsWarehouse =
            IsCatalogIdOneOf(Entry.Id, { "build_1_7", "build_1_8" });
        Entry.IsBusGarage =
            IsCatalogIdOneOf(Entry.Id, { "build_1_12" });
        Entry.IsBusStop =
            IsCatalogIdOneOf(Entry.Id, { "build_1_13" });
        Entry.IsExportStorageHub =
            IsCatalogIdOneOf(Entry.Id, { "build_1_7" });
        Entry.IsDemolish =
            IsCatalogIdOneOf(Entry.Id, { "build_1_2" });
        Entry.IsHiddenFromBuildMenu =
            IsCatalogIdOneOf(Entry.Id, { "build_4_1" });
        Entry.BuildingKind = ResolvePlacementBuildingKind(Entry.Id);
    }

    bool ResolveUsesRecipeTable(
        const FBuildingCatalogEntry& Entry)
    {
        if (Entry.Residential ||
            Entry.BuildingKind == EPlacementBuildingKind::Road ||
            Entry.BuildingKind == EPlacementBuildingKind::Harbor ||
            Entry.BuildingKind == EPlacementBuildingKind::TransportOffice ||
            Entry.IsWarehouse ||
            Entry.IsBusGarage)
        {
            return false;
        }

        if (Entry.Category == EBuildingCategory::Industry)
            return true;

        if (Entry.Category == EBuildingCategory::FoodResource)
            return !IsCatalogIdOneOf(Entry.Id, { "build_2_5" });

        return Entry.FoodProvider;
    }

    void ResolveDefaultBuildingSizeFromTemplate(
        EPlacementTemplateType TemplateType,
        int& OutSizeX,
        int& OutSizeY)
    {
        switch (TemplateType)
        {
        case EPlacementTemplateType::SingleTileMarker:
            OutSizeX = 1;
            OutSizeY = 1;
            return;
        case EPlacementTemplateType::Diamond5x5TwoMarker:
        case EPlacementTemplateType::Diamond5x5FourMarker:
            OutSizeX = 5;
            OutSizeY = 5;
            return;
        case EPlacementTemplateType::Diamond7x7ThreeMarker:
            OutSizeX = 7;
            OutSizeY = 7;
            return;
        case EPlacementTemplateType::Diamond3x3SingleMarker:
        default:
            OutSizeX = 3;
            OutSizeY = 3;
            return;
        }
    }

    int ResolveServiceCapacity(const FBuildingCatalogEntry& Entry)
    {
        const bool ProvidesAnyService =
            Entry.FoodProvider ||
            Entry.EntertainmentProvider ||
            Entry.HealthProvider ||
            Entry.FaithProvider;

        if (!ProvidesAnyService)
            return 0;

        int DerivedCapacity = (std::max)(4, Entry.Capacity / 2);

        if (Entry.Category == EBuildingCategory::Entertainment)
            DerivedCapacity = (std::max)(DerivedCapacity, 8);
        else if (Entry.Category == EBuildingCategory::PublicService)
            DerivedCapacity = (std::max)(DerivedCapacity, 6);
        else if (Entry.Category == EBuildingCategory::FoodResource)
            DerivedCapacity = (std::max)(DerivedCapacity, 5);

        if (Entry.FoodProvider && Entry.EntertainmentProvider)
            DerivedCapacity += 2;

        return DerivedCapacity;
    }

    std::wstring ExtractBasePollutionSummaryText(
        const std::wstring& DetailText)
    {
        std::wstring Result;
        const std::vector<std::wstring> Lines =
            SplitLines(DetailText);

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::wstring Line = Trim(Lines[Index]);

            if (Line.find(L"효과:") != 0 &&
                Line.find(L"비고:") != 0)
            {
                continue;
            }

            if (!Result.empty())
                Result += L'\n';

            Result += Line;
        }

        return Result;
    }

    int ResolveBasePollutionOutput(const FBuildingCatalogEntry& Entry)
    {
        const std::wstring Text =
            ExtractBasePollutionSummaryText(Entry.DetailText);

        if (Text.find(L"많은 공해 배출") != std::wstring::npos)
            return 32;

        if (Text.find(L"건물 자체는 공해 배출") != std::wstring::npos)
            return 18;

        if (Text.find(L"적은 공해 배출") != std::wstring::npos ||
            Text.find(L"적은 공해") != std::wstring::npos)
        {
            return 8;
        }

        if (Text.find(L"공해 배출") != std::wstring::npos)
            return 18;

        return 0;
    }

    int ResolveBasePollutionMitigation(const FBuildingCatalogEntry& Entry)
    {
        const std::wstring Text =
            ExtractBasePollutionSummaryText(Entry.DetailText);

        if (Text.find(L"범위 내 다른 건물 공해 감소") != std::wstring::npos)
            return 20;

        if (Text.find(L"주변 공해 감소") != std::wstring::npos)
            return 12;

        if (Text.find(L"공해 감소") != std::wstring::npos)
            return 12;

        return 0;
    }

    EResourceType ResolveVisitConsumptionResourceType(
        const FBuildingCatalogEntry& Entry)
    {
        if (Entry.FoodProvider)
        {
            if (Entry.ProducedResourceType != EResourceType::None &&
                IsFoodResourceType(Entry.ProducedResourceType))
            {
                return Entry.ProducedResourceType;
            }

            for (int SlotIndex = 0;
                SlotIndex < GProductionInputSlotCount;
                ++SlotIndex)
            {
                const EResourceType InputType =
                    Entry.ProductionInputTypes[static_cast<size_t>(SlotIndex)];

                if (IsFoodResourceType(InputType))
                    return InputType;
            }

            return EResourceType::Crops;
        }

        for (int SlotIndex = 0;
            SlotIndex < GProductionInputSlotCount;
            ++SlotIndex)
        {
            const size_t Index = static_cast<size_t>(SlotIndex);

            if (Entry.ProductionInputTypes[Index] != EResourceType::None &&
                Entry.ProductionInputAmounts[Index] > 0)
            {
                return Entry.ProductionInputTypes[Index];
            }
        }

        return EResourceType::None;
    }

    void AssignPoliticalSignals(FBuildingCatalogEntry& Entry)
    {
        Entry.PoliticalSignals.clear();

        const std::wstring& Name = Entry.DisplayName;

        switch (Entry.Category)
        {
        case EBuildingCategory::Infrastructure:
            if (Name == L"항구" || Name == L"화물창고" || Name == L"창고")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 5.0f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Right, 2.5f);
            }
            else if (Name == L"운송업자 사무소" || Name == L"건설 사무소")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 3.0f, EPoliticalScope::Worker);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Right, 2.0f);
            }
            else if (NameContains(Name, L"풍력") || NameContains(Name, L"태양광"))
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Left, 3.5f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Left, 1.0f);
            }
            else if (NameContains(Name, L"발전소") || NameContains(Name, L"원자력"))
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Right, 3.5f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 1.5f);
            }
            break;

        case EBuildingCategory::FoodResource:
            if (NameContains(Name, L"광산") ||
                NameContains(Name, L"석유") ||
                NameContains(Name, L"정유") ||
                Name == L"벌목소")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Right, 4.0f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 2.0f, EPoliticalScope::Worker);
            }
            else
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Right, 2.0f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Left, 1.5f);
            }
            break;

        case EBuildingCategory::Industry:
            AddPoliticalSignal(
                Entry, EPoliticalAxis::Economy,
                EPoliticalStance::Left, 3.5f);
            AddPoliticalSignal(
                Entry, EPoliticalAxis::EnvironmentIndustry,
                EPoliticalStance::Right, 3.5f);
            AddPoliticalSignal(
                Entry, EPoliticalAxis::Economy,
                EPoliticalStance::Left, 1.5f, EPoliticalScope::Worker);
            break;

        case EBuildingCategory::Housing:
            if (Entry.HousingClass == EBuildingHousingClass::Collective)
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Right, 4.0f, EPoliticalScope::Resident);
            }
            else if (Entry.HousingClass == EBuildingHousingClass::Elite)
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 3.5f, EPoliticalScope::Resident);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right, 2.0f, EPoliticalScope::Resident);
            }
            break;

        case EBuildingCategory::Entertainment:
            if (Entry.LeisureClass == EBuildingLeisureClass::Luxury)
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 2.5f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right, 1.5f);
            }
            else if (Entry.LeisureClass == EBuildingLeisureClass::Cultural)
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Left, 3.0f);
            }
            break;

        case EBuildingCategory::MediaEducation:
            if (Name == L"영묘")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::ReligionMilitarism,
                    EPoliticalStance::Left, 2.5f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right, 1.5f);
            }
            else if (Name == L"격려용 광고판" || Name == L"격려용 동상")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right, 2.0f);
            }
            else if (Name == L"핵 개발 프로그램")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::ReligionMilitarism,
                    EPoliticalStance::Right, 3.0f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Right, 2.0f);
            }
            else
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Left, 3.5f);
            }
            break;

        case EBuildingCategory::Tourism:
            AddPoliticalSignal(
                Entry, EPoliticalAxis::Economy,
                EPoliticalStance::Left, 2.5f);
            AddPoliticalSignal(
                Entry, EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Right, 1.0f);
            break;

        case EBuildingCategory::PublicService:
            if (Name == L"예배당" || Name == L"교회" || Name == L"성당")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::ReligionMilitarism,
                    EPoliticalStance::Left, 3.5f);
            }
            else if (Name == L"소방서")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::ReligionMilitarism,
                    EPoliticalStance::Right, 2.0f);
            }
            else if (Name == L"쇼핑몰")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 3.0f);
            }
            else if (Name == L"쓰레기장" || Name == L"폐기물 처리장")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Left, 2.5f);
            }
            else
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Right, 2.5f);
            }
            break;

        default:
            break;
        }
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

    std::vector<std::wstring> BuildAcceptedResourceDisplayLabels(
        const std::vector<EResourceType>& AcceptedTypes)
    {
        std::vector<std::wstring> AcceptedLabels;

        for (size_t Index = 0; Index < AcceptedTypes.size(); ++Index)
        {
            const EResourceType AcceptedType = AcceptedTypes[Index];

            if (AcceptedType == EResourceType::None ||
                AcceptedType == EResourceType::Count ||
                IsSummaryResourceType(AcceptedType))
            {
                continue;
            }

            const std::wstring Label =
                std::wstring(GetResourceTypeDisplayName(AcceptedType));

            if (Label.empty())
                continue;

            if (std::find(
                    AcceptedLabels.begin(),
                    AcceptedLabels.end(),
                    Label) == AcceptedLabels.end())
            {
                AcceptedLabels.push_back(Label);
            }
        }

        return AcceptedLabels;
    }

    std::wstring BuildVisitConsumptionDemandLabel(
        EResourceType VisitType,
        const std::vector<EResourceType>& AcceptedTypes)
    {
        const std::vector<std::wstring> AcceptedLabels =
            BuildAcceptedResourceDisplayLabels(AcceptedTypes);

        if (!AcceptedLabels.empty())
        {
            std::wstring JoinedAcceptedLabels;

            for (size_t Index = 0; Index < AcceptedLabels.size(); ++Index)
            {
                if (!JoinedAcceptedLabels.empty())
                    JoinedAcceptedLabels += L"/";

                JoinedAcceptedLabels += AcceptedLabels[Index];
            }

            return L"소비 품목(" + JoinedAcceptedLabels + L")";
        }

        return std::wstring(GetResourceTypeDisplayName(VisitType));
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
                BuildVisitConsumptionDemandLabel(
                    Entry.VisitConsumptionResourceType,
                    Entry.VisitConsumptionAcceptedResourceTypes);

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
}

EPlacementTemplateType ResolveLegacyTemplateTypeByBuildingId(
    const std::string& BuildingId)
{
    struct FTemplateRule
    {
        const char* BuildingId;
        EPlacementTemplateType TemplateType;
    };

    static const std::array<FTemplateRule, 2> GRules =
    {
        FTemplateRule{ "build_1_1", EPlacementTemplateType::SingleTileMarker },
        FTemplateRule{ "build_1_12", EPlacementTemplateType::SingleTileMarker }
    };

    for (const FTemplateRule& Rule : GRules)
    {
        if (BuildingId == Rule.BuildingId)
            return Rule.TemplateType;
    }

    return EPlacementTemplateType::Diamond3x3SingleMarker;
}

void InitializeDerivedCatalogEntry(
    FBuildingCatalogEntry& Entry)
{
    Entry.IsCustomsOffice =
        Entry.DisplayName == L"세관";

    if (Entry.IsCustomsOffice)
    {
        Entry.HasBuildMenuCategoryOverride = true;
        Entry.BuildMenuCategoryOverride =
            EBuildingCategory::GovernmentFinance;
    }

    ResolveCategoryDefaultFlags(
        Entry.Category,
        Entry.Residential,
        Entry.FoodProvider,
        Entry.EntertainmentProvider);
    ApplyCatalogIdentityFlags(Entry);

    if (Entry.Residential)
    {
        Entry.Capacity =
            20 + (Entry.CategoryLocalIndex % 5) * 8 +
            (Entry.CategoryLocalIndex / 5) * 4;
    }
    else
    {
        Entry.Capacity =
            15 + (Entry.CategoryLocalIndex % 6) * 6 +
            (Entry.CategoryLocalIndex / 6) * 3;
    }

    if (Entry.BuildingKind == EPlacementBuildingKind::Road)
    {
        Entry.Capacity = 0;
        Entry.JobSatisfactionCap = 0;
    }

    if (Entry.IsBusGarage)
    {
        Entry.Capacity = 0;
        Entry.JobSatisfactionCap = 0;
    }

    if (Entry.IsCustomsOffice)
        Entry.Capacity = 5;

    Entry.HousingSatisfactionCap = 100;
    Entry.JobSatisfactionCap = 100;
    Entry.FoodSatisfactionCap = 100;
    Entry.FunSatisfactionCap = 100;
    Entry.HealthSatisfactionCap = 100;
    Entry.FaithSatisfactionCap = 100;

    if (Entry.Category == EBuildingCategory::Infrastructure)
    {
        Entry.JobSatisfactionCap = (std::min)(
            85,
            45 + (Entry.CategoryLocalIndex % 7) * 5 +
                (Entry.CategoryLocalIndex / 7) * 3);
    }
    else if (Entry.Category == EBuildingCategory::FoodResource)
    {
        Entry.FoodSatisfactionCap = (std::min)(
            82,
            35 + (Entry.CategoryLocalIndex % 7) * 6 +
                (Entry.CategoryLocalIndex / 7) * 4);
        Entry.JobSatisfactionCap = (std::min)(
            70,
            40 + (Entry.CategoryLocalIndex % 5) * 5 +
                (Entry.CategoryLocalIndex / 5) * 2);
    }
    else if (Entry.Category == EBuildingCategory::Industry)
    {
        Entry.JobSatisfactionCap = (std::min)(
            90,
            50 + (Entry.CategoryLocalIndex % 8) * 5 +
                (Entry.CategoryLocalIndex / 8) * 4);
    }
    else if (Entry.Category == EBuildingCategory::Housing)
    {
        if (Entry.CategoryLocalIndex >= 0 &&
            Entry.CategoryLocalIndex < static_cast<int>(GHousingCaps.size()))
        {
            Entry.HousingSatisfactionCap =
                GHousingCaps[static_cast<size_t>(Entry.CategoryLocalIndex)];
        }
    }
    else if (Entry.Category == EBuildingCategory::Entertainment)
    {
        if (Entry.CategoryLocalIndex >= 0 &&
            Entry.CategoryLocalIndex <
                static_cast<int>(GEntertainmentFunCaps.size()))
        {
            Entry.FunSatisfactionCap =
                GEntertainmentFunCaps[
                    static_cast<size_t>(Entry.CategoryLocalIndex)];
        }
    }
    else if (Entry.Category == EBuildingCategory::PublicService)
    {
        Entry.JobSatisfactionCap = (std::min)(
            88,
            48 + (Entry.CategoryLocalIndex % 6) * 5 +
                (Entry.CategoryLocalIndex / 6) * 4);
    }

    if (Entry.BuildingKind == EPlacementBuildingKind::Road)
        Entry.JobSatisfactionCap = 0;

    if (Entry.IsBusGarage)
        Entry.JobSatisfactionCap = 0;

    if (Entry.Category == EBuildingCategory::Entertainment &&
        IsCatalogIdOneOf(
            Entry.Id,
            { "build_5_6", "build_5_8", "build_5_19" }))
    {
        Entry.FoodProvider = true;

        if (Entry.Id == "build_5_6")
            Entry.FoodSatisfactionCap = 65;
        else if (Entry.Id == "build_5_19")
            Entry.FoodSatisfactionCap = 75;
        else
            Entry.FoodSatisfactionCap = 55;
    }
    else if (Entry.Category == EBuildingCategory::PublicService &&
        IsCatalogIdOneOf(
            Entry.Id,
            { "build_8_2", "build_8_10" }))
    {
        Entry.FoodProvider = true;
        Entry.FoodSatisfactionCap =
            Entry.Id == "build_8_10" ? 70 : 55;
    }

    if (Entry.Category == EBuildingCategory::PublicService &&
        IsCatalogIdOneOf(
            Entry.Id,
            { "build_8_12" }))
    {
        Entry.EntertainmentProvider = true;
        Entry.FunSatisfactionCap = 68;
    }

    if (Entry.Category == EBuildingCategory::FoodResource &&
        IsCatalogIdOneOf(
            Entry.Id,
            {
                "build_2_1",
                "build_2_3",
                "build_2_4",
                "build_2_6",
                "build_2_9",
                "build_2_11",
                "build_2_12"
            }))
    {
        Entry.FoodProvider = true;
    }

    Entry.HealthProvider = ResolveHealthProvider(Entry);
    Entry.FaithProvider = ResolveFaithProvider(Entry);
    Entry.HealthSatisfactionCap = ResolveHealthSatisfactionCap(Entry);
    Entry.FaithSatisfactionCap = ResolveFaithSatisfactionCap(Entry);
    Entry.BaseHousingQuality = 0;
    Entry.BaseJobQuality = 0;
    Entry.BaseServiceQuality = 0;
    ResolveDefaultBuildingSizeFromTemplate(
        Entry.TemplateType,
        Entry.BuildingSizeX,
        Entry.BuildingSizeY);
    Entry.BaseProducedPowerMW = 0;
    Entry.BaseRequiredPowerMW = 0;
    Entry.BasePollutionOutput = ResolveBasePollutionOutput(Entry);
    Entry.BasePollutionMitigation = ResolveBasePollutionMitigation(Entry);
    Entry.UnlockEra = ParseUnlockEra(Entry.DetailText);
    Entry.AllowedWealthMask = ParseAllowedWealthMask(Entry.DetailText);
    Entry.RequiredEducationLevel =
        ECitizenEducationLevel::Uneducated;
    Entry.ServiceCapacity = ResolveServiceCapacity(Entry);
    Entry.ServiceCapacityUsesHouseholds = false;

    Entry.HousingClass = ResolveHousingClass(Entry.Id);
    Entry.LeisureClass =
        ResolveLeisureClass(Entry.Category, Entry.Id);
    Entry.PrimaryTouristPreference =
        ParsePrimaryTouristPreference(Entry.DetailText);
    Entry.UsesRecipeTable = ResolveUsesRecipeTable(Entry);
}

void FinalizeCatalogEntryAfterRecipeLoad(
    FBuildingCatalogEntry& Entry)
{
    if (Entry.VisitConsumptionResourceType == EResourceType::None)
    {
        Entry.VisitConsumptionResourceType =
            ResolveVisitConsumptionResourceType(Entry);
    }

    Entry.CanExportStoredResources =
        Entry.BuildingKind == EPlacementBuildingKind::Harbor ||
        Entry.IsExportStorageHub;
    Entry.SupportsTeamsterPickup =
        Entry.ProducedResourceType != EResourceType::None &&
        !Entry.Residential &&
        !Entry.CanExportStoredResources &&
        Entry.BuildingKind !=
            EPlacementBuildingKind::TransportOffice;
    Entry.SupportsImmigration =
        Entry.DetailText.find(L"이민/이주 처리") !=
        std::wstring::npos;

    AssignPoliticalSignals(Entry);
}

std::vector<std::wstring> ExtractUpgradeHintsFromDetail(
    const std::wstring& DetailText)
{
    std::vector<std::wstring> Result;
    const std::vector<std::wstring> Lines =
        SplitLines(DetailText);
    bool Capture = false;

    for (size_t Index = 0; Index < Lines.size(); ++Index)
    {
        const std::wstring& Line = Lines[Index];

        if (Line.find(L"업그레이드") == 0)
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

        Result.push_back(Line.substr(1));
    }

    return Result;
}

std::vector<std::wstring> SplitOperationModeSummaryClauses(
    const std::wstring& Text)
{
    return SplitCommaClauses(Text);
}

void ApplyOperationModeEffectClause(
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

std::vector<FBuildingOperationModeDef> ExtractOperationModeDefsFromEntry(
    const FBuildingCatalogEntry& Entry)
{
    std::vector<FBuildingOperationModeDef> Result;
    const std::vector<std::wstring> Lines =
        SplitLines(Entry.DetailText);
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
        std::vector<std::wstring> EffectClauses;

        for (size_t ClauseIndex = 0;
            ClauseIndex < Clauses.size();
            ++ClauseIndex)
        {
            const std::wstring Clause = Trim(Clauses[ClauseIndex]);

            if (Clause.empty())
                continue;

            EBuildingEra ParsedUnlockEra = EBuildingEra::Colonial;

            if (TryParseUpgradeUnlockEraClause(Clause, ParsedUnlockEra))
            {
                ModeDef.HasUnlockEra = true;
                ModeDef.UnlockEra = ParsedUnlockEra;
                continue;
            }

            if (Clause.find(L"연구 필요") != std::wstring::npos)
            {
                if (ModeDef.RequiredResearch.empty())
                    ModeDef.RequiredResearch = L"연구 필요";

                continue;
            }

            EffectClauses.push_back(Clause);
            ApplyOperationModeEffectClause(
                Entry,
                Clause,
                ModeDef.Effect);
        }

        ModeDef.EffectSummary = JoinLabels(EffectClauses, L", ");
        Result.push_back(std::move(ModeDef));
    }

    return Result;
}

std::vector<FBuildingRuntimeUpgradeDef> ExtractRuntimeUpgradeDefsFromEntry(
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
            const std::wstring UpgradeText = Trim(Candidates[CandidateIndex]);

            if (UpgradeText.empty())
                continue;

            const size_t OpenParen = UpgradeText.find(L'(');
            const size_t CloseParen = UpgradeText.find_last_of(L')');

            if (OpenParen == std::wstring::npos ||
                CloseParen == std::wstring::npos ||
                CloseParen <= OpenParen)
            {
                continue;
            }

            std::wstring DisplayName = Trim(UpgradeText.substr(0, OpenParen));
            const size_t ScopeSep = DisplayName.find_last_of(L':');

            if (ScopeSep != std::wstring::npos)
                DisplayName = Trim(DisplayName.substr(ScopeSep + 1));

            if (DisplayName.empty())
                continue;

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
                ApplyOperationModeEffectClause(
                    Entry,
                    EffectClauses[ClauseIndex],
                    Def.Effect);
            }

            if (!Def.Effect.HasRuntimeEffect())
                continue;

            Result.push_back(std::move(Def));
        }
    }

    return Result;
}

void ApplyCatalogUiBehaviorFlags(
    std::vector<FBuildingCatalogEntry>& Entries)
{
    for (auto& Entry : Entries)
    {
        if (Entry.IsDemolish)
        {
            Entry.IsDemolish = true;
            Entry.Capacity = 0;
            Entry.HouseholdCapacity = 0;
            Entry.ServiceCapacity = 0;
            Entry.ServiceCapacityUsesHouseholds = false;
            Entry.JobSatisfactionCap = 0;
            Entry.RequiredEducationLevel =
                ECitizenEducationLevel::Uneducated;
        }
    }
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
            ForEachSupplyChainDemandKeyResourceType(
                ResourceType,
                [&](EResourceType KeyType)
                {
                    const size_t ResourceIndex =
                        static_cast<size_t>(KeyType);

                    if (ResourceIndex >= DownstreamResourceLabels.size())
                        return;

                    AppendUniqueLabel(
                        DownstreamResourceLabels[ResourceIndex],
                        ConsumerLabel);
                });
        };

        if (ConsumerEntry.VisitConsumptionResourceType !=
                EResourceType::None &&
            ConsumerEntry.VisitConsumptionResourceType !=
                ConsumerEntry.ProducedResourceType)
        {
            const EResourceType VisitType =
                ConsumerEntry.VisitConsumptionResourceType;

            if (!ConsumerEntry.VisitConsumptionAcceptedResourceTypes.empty())
            {
                for (size_t AcceptedIndex = 0;
                    AcceptedIndex <
                        ConsumerEntry.VisitConsumptionAcceptedResourceTypes.size();
                    ++AcceptedIndex)
                {
                    AppendDownstreamDemand(
                        ConsumerEntry.VisitConsumptionAcceptedResourceTypes[
                            AcceptedIndex]);
                }
            }
            else if (ConsumerEntry.FoodProvider &&
                IsSummaryResourceType(VisitType))
            {
                ForEachFoodVisitCompatibleResourceType(
                    ConsumerEntry.Id,
                    ConsumerEntry.ProducedResourceType,
                    VisitType,
                    AppendDownstreamDemand);
            }
            else
            {
                AppendDownstreamDemand(VisitType);
            }
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

            const bool FeedInput =
                IsFeedInputDemandResourceType(InputType);

            if (FeedInput)
            {
                ForEachFeedCompatibleResourceType(
                    InputType,
                    AppendDownstreamDemand);
            }
            else
            {
                ForEachIndustrialInputCompatibleResourceType(
                    InputType,
                    AppendDownstreamDemand);
            }
        }
    }

    for (FBuildingCatalogEntry& Entry : Entries)
    {
        Entry.ProductionChainStage =
            EBuildingProductionChainStage::None;
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
                EBuildingProductionChainStage::Primary;
        }
        else if (!DownstreamLabels.empty())
        {
            Entry.ProductionChainStage =
                EBuildingProductionChainStage::Intermediate;
        }
        else
        {
            Entry.ProductionChainStage =
                EBuildingProductionChainStage::Final;
        }

        std::wstring Summary;

        if (!DemandLabels.empty())
        {
            Summary += JoinLabels(DemandLabels, L" + ");
            Summary += L" -> ";
        }

        Summary += OutputLabel;

        if (EntryHasInputs && !DownstreamLabels.empty())
        {
            Summary += L" -> ";
            Summary += JoinLabels(DownstreamLabels, L", ");
        }
        else if (!EntryHasInputs)
        {
            // 1차 생산자는 하류 공정을 직접 수행하는 것처럼 보이지 않도록
            // 산출물만 요약하고 "생산"으로 마무리한다.
            Summary += L" 생산";
        }

        Entry.SupplyChainSummary = std::move(Summary);
    }
}

