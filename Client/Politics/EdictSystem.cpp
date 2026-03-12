#include "EdictSystem.h"
#include "../RuntimeConfigRegistry.h"
#include <Windows.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <initializer_list>
#include <string>

namespace
{
    constexpr const wchar_t* GConfigId = L"Game.Edicts";
    constexpr const wchar_t* GPlaceholderSummary =
        L"아이콘과 시대 배치만 연결된 참고용 칙령입니다.";
    constexpr const wchar_t* GPlaceholderEffect =
        L"실제 효과와 적용 로직은 아직 연결되지 않았습니다.";
    std::vector<FGovernmentEdictDefinition> GDefinitions;
    bool GDefinitionsInitialized = false;

    template<typename T>
    struct TKeyValueMap
    {
        T Value;
        const char* Key;
    };

    const TKeyValueMap<EGovernmentEdictType> GEdictTypeKeys[] =
    {
        { EGovernmentEdictType::FoodForThePeople, "FoodForThePeople" },
        { EGovernmentEdictType::TaxCut, "TaxCut" },
        { EGovernmentEdictType::MartialLaw, "MartialLaw" },
        { EGovernmentEdictType::FreeHousing, "FreeHousing" },
        { EGovernmentEdictType::EmployeeOfTheMonth, "EmployeeOfTheMonth" },
        { EGovernmentEdictType::ChurchFee, "ChurchFee" },
        { EGovernmentEdictType::NoFreeLunch, "NoFreeLunch" },
        { EGovernmentEdictType::NothingBeatsSiesta, "NothingBeatsSiesta" },
        { EGovernmentEdictType::UrbanDevelopment, "UrbanDevelopment" },
        { EGovernmentEdictType::PenalColony, "PenalColony" },
        { EGovernmentEdictType::ChildAllowances, "ChildAllowances" },
        { EGovernmentEdictType::AdvancedBoatServices, "AdvancedBoatServices" },
        { EGovernmentEdictType::Audience, "Audience" },
        { EGovernmentEdictType::AgriculturalSubsidies, "AgriculturalSubsidies" },
        { EGovernmentEdictType::BuildingPermit, "BuildingPermit" },
        { EGovernmentEdictType::StateLoans, "StateLoans" },
        { EGovernmentEdictType::WealthTax, "WealthTax" },
        { EGovernmentEdictType::Industrialization, "Industrialization" },
        { EGovernmentEdictType::EarlyElections, "EarlyElections" },
        { EGovernmentEdictType::LiteracyProgram, "LiteracyProgram" },
        { EGovernmentEdictType::Prohibition, "Prohibition" },
        { EGovernmentEdictType::BellsToBullets, "BellsToBullets" },
        { EGovernmentEdictType::MilitaryPolice, "MilitaryPolice" },
        { EGovernmentEdictType::Volkswagen, "Volkswagen" },
        { EGovernmentEdictType::RightToArms, "RightToArms" },
        { EGovernmentEdictType::NuclearTesting, "NuclearTesting" },
        { EGovernmentEdictType::GoodOldDays, "GoodOldDays" },
        { EGovernmentEdictType::ExperimentalGroundTreatment, "ExperimentalGroundTreatment" },
        { EGovernmentEdictType::MandatoryWasteSorting, "MandatoryWasteSorting" },
        { EGovernmentEdictType::DiplomaticSuperParty, "DiplomaticSuperParty" },
        { EGovernmentEdictType::HappyMeat, "HappyMeat" },
        { EGovernmentEdictType::AlternativeFoodSource, "AlternativeFoodSource" },
        { EGovernmentEdictType::SpellingBee, "SpellingBee" },
        { EGovernmentEdictType::AssemblyBan, "AssemblyBan" },
        { EGovernmentEdictType::NationalDay, "NationalDay" },
        { EGovernmentEdictType::MadeInTropico, "MadeInTropico" },
        { EGovernmentEdictType::SocialSecurity, "SocialSecurity" },
        { EGovernmentEdictType::ContraceptionBan, "ContraceptionBan" },
        { EGovernmentEdictType::LegalizedSubstances, "LegalizedSubstances" },
        { EGovernmentEdictType::CulturalDiversity, "CulturalDiversity" },
        { EGovernmentEdictType::TaxHeaven, "TaxHeaven" },
        { EGovernmentEdictType::KnowledgioSinco, "KnowledgioSinco" },
        { EGovernmentEdictType::Speedway, "Speedway" },
        { EGovernmentEdictType::CompulsoryVaccination, "CompulsoryVaccination" },
        { EGovernmentEdictType::LightBulbBan, "LightBulbBan" },
        { EGovernmentEdictType::SeaDisposal, "SeaDisposal" },
        { EGovernmentEdictType::PolicyOfDetente, "PolicyOfDetente" },
        { EGovernmentEdictType::CTPA, "CTPA" },
        { EGovernmentEdictType::SpecialTreats, "SpecialTreats" },
        { EGovernmentEdictType::FestivalBoost, "FestivalBoost" },
        { EGovernmentEdictType::MardiGras, "MardiGras" },
        { EGovernmentEdictType::MoneyLaundering, "MoneyLaundering" },
        { EGovernmentEdictType::TourismState, "TourismState" },
        { EGovernmentEdictType::HearTheCall, "HearTheCall" },
        { EGovernmentEdictType::SoapOpera, "SoapOpera" },
        { EGovernmentEdictType::LaborTaxRelief, "LaborTaxRelief" },
        { EGovernmentEdictType::PropertyTaxRelief, "PropertyTaxRelief" },
        { EGovernmentEdictType::EmergencyAusterity, "EmergencyAusterity" }
    };

    const TKeyValueMap<EEdictEra> GEdictEraKeys[] =
    {
        { EEdictEra::Colonial, "Colonial" },
        { EEdictEra::WorldWars, "WorldWars" },
        { EEdictEra::ColdWar, "ColdWar" },
        { EEdictEra::Modern, "Modern" }
    };

    const TKeyValueMap<EGovernmentEdictMode> GEdictModeKeys[] =
    {
        { EGovernmentEdictMode::Passive, "Passive" },
        { EGovernmentEdictMode::Active, "Active" }
    };

    const TKeyValueMap<EPoliticalActionType> GPoliticalActionKeys[] =
    {
        { EPoliticalActionType::None, "None" },
        { EPoliticalActionType::WelfareProgram, "WelfareProgram" },
        { EPoliticalActionType::TaxCut, "TaxCut" },
        { EPoliticalActionType::MartialLaw, "MartialLaw" },
        { EPoliticalActionType::HousingInitiative, "HousingInitiative" },
        { EPoliticalActionType::IndustrialSubsidy, "IndustrialSubsidy" },
        { EPoliticalActionType::LaborTaxRelief, "LaborTaxRelief" },
        { EPoliticalActionType::PropertyTaxRelief, "PropertyTaxRelief" },
        { EPoliticalActionType::AusterityProgram, "AusterityProgram" }
    };

    const TKeyValueMap<EPoliticalAxis> GPoliticalAxisKeys[] =
    {
        { EPoliticalAxis::Economy, "Economy" },
        { EPoliticalAxis::ReligionMilitarism, "ReligionMilitarism" },
        { EPoliticalAxis::EnvironmentIndustry, "EnvironmentIndustry" },
        { EPoliticalAxis::IntellectualConservative, "IntellectualConservative" }
    };

    const TKeyValueMap<EPoliticalStance> GPoliticalStanceKeys[] =
    {
        { EPoliticalStance::Left, "Left" },
        { EPoliticalStance::Neutral, "Neutral" },
        { EPoliticalStance::Right, "Right" }
    };

    const TKeyValueMap<EPoliticalScope> GPoliticalScopeKeys[] =
    {
        { EPoliticalScope::Global, "Global" },
        { EPoliticalScope::Worker, "Worker" },
        { EPoliticalScope::Resident, "Resident" },
        { EPoliticalScope::Visitor, "Visitor" }
    };

    bool TryBuildExpandedImplementedEdict(
        EGovernmentEdictType Type,
        EEdictEra Era,
        const wchar_t* DisplayName,
        const wchar_t* IconPath,
        FGovernmentEdictDefinition& OutDefinition);

    std::vector<FGovernmentEdictDefinition>
        BuildDefaultGovernmentEdictDefinitions();

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

            for (size_t Index = 0; Index < Text.size(); ++Index)
            {
                const wchar_t Character = Text[Index];
                Fallback.push_back(
                    Character >= 0 && Character <= 0x7F ?
                        static_cast<char>(Character) :
                        '?');
            }

            return Fallback;
        }

        std::string Result;
        Result.resize(RequiredBytes);
        WideCharToMultiByte(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            &Result[0], RequiredBytes, nullptr, nullptr);
        return Result;
    }

    void TrimString(std::string& Value)
    {
        Value.erase(
            Value.begin(),
            std::find_if(
                Value.begin(),
                Value.end(),
                [](unsigned char Character)
                {
                    return !std::isspace(Character);
                }));
        Value.erase(
            std::find_if(
                Value.rbegin(),
                Value.rend(),
                [](unsigned char Character)
                {
                    return !std::isspace(Character);
                }).base(),
            Value.end());
    }

    std::string ToLowerCopy(std::string Value)
    {
        std::transform(
            Value.begin(),
            Value.end(),
            Value.begin(),
            [](unsigned char Character)
            {
                return static_cast<char>(std::tolower(Character));
            });
        return Value;
    }

    std::string UnescapeIniValue(const std::string& Text)
    {
        std::string Result;
        Result.reserve(Text.size());

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const char Character = Text[Index];

            if (Character != '\\' || Index + 1 >= Text.size())
            {
                Result.push_back(Character);
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
                Result.push_back('\\');
                Result.push_back(Next);
                break;
            }
        }

        return Result;
    }

    std::string EscapeIniValue(const std::string& Text)
    {
        std::string Result;
        Result.reserve(Text.size());

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const char Character = Text[Index];

            switch (Character)
            {
            case '\n':
                Result += "\\n";
                break;
            case '\r':
                Result += "\\r";
                break;
            case '\t':
                Result += "\\t";
                break;
            default:
                Result.push_back(Character);
                break;
            }
        }

        return Result;
    }

    template<typename T, size_t N>
    const char* GetMappedKey(
        T Value,
        const TKeyValueMap<T>(&Mappings)[N],
        const char* Fallback = "")
    {
        for (size_t Index = 0; Index < N; ++Index)
        {
            if (Mappings[Index].Value == Value)
                return Mappings[Index].Key;
        }

        return Fallback;
    }

    template<typename T, size_t N>
    bool TryParseMappedKey(
        const std::string& Value,
        const TKeyValueMap<T>(&Mappings)[N],
        T& OutValue)
    {
        const std::string LowerValue = ToLowerCopy(Value);

        for (size_t Index = 0; Index < N; ++Index)
        {
            if (LowerValue == ToLowerCopy(Mappings[Index].Key))
            {
                OutValue = Mappings[Index].Value;
                return true;
            }
        }

        return false;
    }

    const char* GetGovernmentEdictTypeKey(EGovernmentEdictType Type)
    {
        return GetMappedKey(Type, GEdictTypeKeys, "None");
    }

    bool TryParseGovernmentEdictTypeKey(
        const std::string& Value,
        EGovernmentEdictType& OutType)
    {
        return TryParseMappedKey(Value, GEdictTypeKeys, OutType);
    }

    bool TryParseEdictEraKey(
        const std::string& Value,
        EEdictEra& OutEra)
    {
        return TryParseMappedKey(Value, GEdictEraKeys, OutEra);
    }

    bool TryParseEdictModeKey(
        const std::string& Value,
        EGovernmentEdictMode& OutMode)
    {
        return TryParseMappedKey(Value, GEdictModeKeys, OutMode);
    }

    bool TryParsePoliticalActionTypeKey(
        const std::string& Value,
        EPoliticalActionType& OutActionType)
    {
        return TryParseMappedKey(Value, GPoliticalActionKeys, OutActionType);
    }

    bool TryParsePoliticalAxisKey(
        const std::string& Value,
        EPoliticalAxis& OutAxis)
    {
        return TryParseMappedKey(Value, GPoliticalAxisKeys, OutAxis);
    }

    bool TryParsePoliticalStanceKey(
        const std::string& Value,
        EPoliticalStance& OutStance)
    {
        return TryParseMappedKey(Value, GPoliticalStanceKeys, OutStance);
    }

    bool TryParsePoliticalScopeKey(
        const std::string& Value,
        EPoliticalScope& OutScope)
    {
        return TryParseMappedKey(Value, GPoliticalScopeKeys, OutScope);
    }

    bool TryParseBoolValue(const std::string& Value, bool& OutValue)
    {
        const std::string LowerValue = ToLowerCopy(Value);

        if (LowerValue == "1" || LowerValue == "true" ||
            LowerValue == "yes" || LowerValue == "on")
        {
            OutValue = true;
            return true;
        }

        if (LowerValue == "0" || LowerValue == "false" ||
            LowerValue == "no" || LowerValue == "off")
        {
            OutValue = false;
            return true;
        }

        return false;
    }

    bool TryParseIntValue(const std::string& Value, int& OutValue)
    {
        char* EndPtr = nullptr;
        const long ParsedValue = strtol(Value.c_str(), &EndPtr, 10);

        if (!EndPtr || *EndPtr != 0)
            return false;

        OutValue = static_cast<int>(ParsedValue);
        return true;
    }

    bool TryParseLongLongValue(
        const std::string& Value,
        long long& OutValue)
    {
        char* EndPtr = nullptr;
        const long long ParsedValue = _strtoi64(Value.c_str(), &EndPtr, 10);

        if (!EndPtr || *EndPtr != 0)
            return false;

        OutValue = ParsedValue;
        return true;
    }

    bool TryParseFloatValue(const std::string& Value, float& OutValue)
    {
        char* EndPtr = nullptr;
        const float ParsedValue = static_cast<float>(strtod(Value.c_str(), &EndPtr));

        if (!EndPtr || *EndPtr != 0)
            return false;

        OutValue = ParsedValue;
        return true;
    }

    FGovernmentEdictEffectDefinition BuildDefaultEdictEffectDefinition(
        EGovernmentEdictType Type)
    {
        FGovernmentEdictEffectDefinition Effect;

        switch (Type)
        {
        case EGovernmentEdictType::FoodForThePeople:
            Effect.FoodConsumptionPerVisitMin = 2;
            Effect.FoodGainMultiplier = 1.18f;
            Effect.DailyFoodDelta = 3.5f;
            break;
        case EGovernmentEdictType::ChurchFee:
            Effect.TaxRevenueMultiplier = 1.10f;
            Effect.DailyFreedomDelta = -0.45f;
            Effect.DailySecurityDelta = 0.25f;
            break;
        case EGovernmentEdictType::NoFreeLunch:
            Effect.DailyBudgetDeltaFlat = 140;
            Effect.DailyFoodDelta = -1.50f;
            Effect.DailyFreedomDelta = -0.20f;
            break;
        case EGovernmentEdictType::NothingBeatsSiesta:
            Effect.ProductionMultiplier = 0.88f;
            Effect.DailyJobDelta = 0.55f;
            Effect.DailyFreedomDelta = 0.60f;
            break;
        case EGovernmentEdictType::UrbanDevelopment:
            Effect.DailyHousingDelta = 2.20f;
            Effect.DailyFreedomDelta = -0.30f;
            Effect.DailyBudgetDeltaFlat = -120;
            break;
        case EGovernmentEdictType::TaxCut:
            Effect.DailyFreedomDelta = 1.5f;
            Effect.TaxRevenueMultiplier = 0.65f;
            break;
        case EGovernmentEdictType::DiplomaticSuperParty:
            Effect.DailyFreedomDelta = 0.25f;
            Effect.DailyBudgetDeltaFlat = -80;
            break;
        case EGovernmentEdictType::AgriculturalSubsidies:
            Effect.ProductionMultiplier = 1.12f;
            Effect.DailyFoodDelta = 1.35f;
            Effect.DailyBudgetDeltaFlat = -160;
            break;
        case EGovernmentEdictType::BuildingPermit:
            Effect.TaxRevenueMultiplier = 1.10f;
            Effect.DailyFreedomDelta = -0.80f;
            Effect.DailySecurityDelta = 0.20f;
            break;
        case EGovernmentEdictType::WealthTax:
            Effect.TaxRevenueMultiplier = 1.14f;
            Effect.DailyFreedomDelta = -0.35f;
            Effect.DailyHousingDelta = 0.45f;
            break;
        case EGovernmentEdictType::Industrialization:
            Effect.ProductionMultiplier = 1.18f;
            Effect.DailyJobDelta = 0.90f;
            Effect.DailyFreedomDelta = -0.35f;
            break;
        case EGovernmentEdictType::LiteracyProgram:
            Effect.ProductionMultiplier = 1.06f;
            Effect.DailyJobDelta = 0.95f;
            Effect.DailyFreedomDelta = 0.45f;
            Effect.DailyBudgetDeltaFlat = -110;
            break;
        case EGovernmentEdictType::MartialLaw:
            Effect.DailyFreedomDelta = -4.0f;
            Effect.DailySecurityDelta = 5.0f;
            break;
        case EGovernmentEdictType::Prohibition:
            Effect.DailySecurityDelta = 2.10f;
            Effect.DailyFreedomDelta = -1.40f;
            Effect.DailyJobDelta = -0.20f;
            break;
        case EGovernmentEdictType::MilitaryPolice:
            Effect.DailySecurityDelta = 2.90f;
            Effect.DailyFreedomDelta = -2.10f;
            Effect.DailyBudgetDeltaFlat = -90;
            break;
        case EGovernmentEdictType::RightToArms:
            Effect.DailyFreedomDelta = 1.45f;
            Effect.DailySecurityDelta = -1.10f;
            break;
        case EGovernmentEdictType::FreeHousing:
            Effect.DailyHousingDelta = 3.5f;
            Effect.DailyBudgetDeltaPerCitizen = -1;
            Effect.DailyBudgetDeltaPerCitizenAbsMinimum = 80;
            break;
        case EGovernmentEdictType::EmployeeOfTheMonth:
            Effect.ProductionMultiplier = 1.35f;
            Effect.DailyJobDelta = 1.5f;
            break;
        case EGovernmentEdictType::TaxHeaven:
            Effect.TaxRevenueMultiplier = 1.06f;
            Effect.DailyFreedomDelta = -0.20f;
            Effect.DailyBudgetDeltaFlat = 110;
            break;
        case EGovernmentEdictType::PolicyOfDetente:
            Effect.DailyFreedomDelta = 0.15f;
            Effect.DailySecurityDelta = 0.20f;
            break;
        case EGovernmentEdictType::CTPA:
            Effect.ProductionMultiplier = 1.03f;
            Effect.DailyBudgetDeltaFlat = 90;
            break;
        case EGovernmentEdictType::TourismState:
            Effect.DailyFreedomDelta = 0.20f;
            Effect.DailyBudgetDeltaFlat = 120;
            break;
        case EGovernmentEdictType::LaborTaxRelief:
            Effect.DailyJobDelta = 1.25f;
            Effect.DailyFreedomDelta = 0.75f;
            break;
        case EGovernmentEdictType::PropertyTaxRelief:
            Effect.DailyHousingDelta = 1.40f;
            Effect.DailyFreedomDelta = 0.35f;
            break;
        case EGovernmentEdictType::EmergencyAusterity:
            Effect.DailyBudgetDeltaFlat = 450;
            Effect.DailyJobDelta = -1.0f;
            Effect.DailyFreedomDelta = -0.85f;
            Effect.DailySecurityDelta = 0.45f;
            break;
        default:
            break;
        }

        return Effect;
    }

    void ApplyEdictEffectToModifiers(
        const FGovernmentEdictEffectDefinition& Effect,
        int ActiveCitizenCount,
        FGovernmentEdictModifiers& InOutModifiers)
    {
        if (Effect.FoodConsumptionPerVisitMin > 0)
        {
            InOutModifiers.FoodConsumptionPerVisit = (std::max)(
                InOutModifiers.FoodConsumptionPerVisit,
                Effect.FoodConsumptionPerVisitMin);
        }

        InOutModifiers.FoodGainMultiplier *= Effect.FoodGainMultiplier;
        InOutModifiers.ProductionMultiplier *= Effect.ProductionMultiplier;
        InOutModifiers.TaxRevenueMultiplier *= Effect.TaxRevenueMultiplier;
        InOutModifiers.DailyFoodDelta += Effect.DailyFoodDelta;
        InOutModifiers.DailyHousingDelta += Effect.DailyHousingDelta;
        InOutModifiers.DailyJobDelta += Effect.DailyJobDelta;
        InOutModifiers.DailyFreedomDelta += Effect.DailyFreedomDelta;
        InOutModifiers.DailySecurityDelta += Effect.DailySecurityDelta;
        InOutModifiers.DailyBudgetDelta += Effect.DailyBudgetDeltaFlat;

        if (Effect.DailyBudgetDeltaPerCitizen != 0)
        {
            const long long PerCitizenMagnitude =
                std::llabs(Effect.DailyBudgetDeltaPerCitizen);
            long long Contribution =
                PerCitizenMagnitude *
                static_cast<long long>((std::max)(0, ActiveCitizenCount));
            Contribution = (std::max)(
                Contribution,
                std::llabs(Effect.DailyBudgetDeltaPerCitizenAbsMinimum));

            InOutModifiers.DailyBudgetDelta +=
                Effect.DailyBudgetDeltaPerCitizen < 0 ?
                    -Contribution :
                    Contribution;
        }
    }

    FPoliticalSignalDef MakeSignal(
        EPoliticalAxis Axis,
        EPoliticalStance Stance,
        float Strength,
        EPoliticalScope Scope = EPoliticalScope::Global)
    {
        FPoliticalSignalDef Signal;
        Signal.Axis = Axis;
        Signal.FavoredStance = Stance;
        Signal.Strength = Strength;
        Signal.Scope = Scope;
        return Signal;
    }

    FGovernmentEdictDefinition MakePlaceholderEdict(
        EGovernmentEdictType Type,
        EEdictEra Era,
        const wchar_t* DisplayName,
        const wchar_t* IconPath)
    {
        FGovernmentEdictDefinition Definition;

        if (TryBuildExpandedImplementedEdict(
                Type,
                Era,
                DisplayName,
                IconPath,
                Definition))
        {
            return Definition;
        }

        Definition.Type = Type;
        Definition.Era = Era;
        Definition.DisplayName = DisplayName ? DisplayName : L"칙령";
        Definition.Summary = GPlaceholderSummary;
        Definition.EffectText = GPlaceholderEffect;
        Definition.IconPath = IconPath ? IconPath : L"";
        Definition.Effect = BuildDefaultEdictEffectDefinition(Type);
        return Definition;
    }

    FGovernmentEdictDefinition MakeImplementedEdict(
        EGovernmentEdictType Type,
        EEdictEra Era,
        EGovernmentEdictMode Mode,
        EPoliticalActionType ActionType,
        const wchar_t* DisplayName,
        const wchar_t* Summary,
        const wchar_t* EffectText,
        const wchar_t* IconPath,
        long long BaseCost,
        long long CostPerCitizen,
        long long MonthlyUpkeep,
        int DurationDays,
        int CooldownDays,
        std::initializer_list<FPoliticalSignalDef> Signals = {})
    {
        FGovernmentEdictDefinition Definition;
        Definition.Type = Type;
        Definition.Era = Era;
        Definition.Mode = Mode;
        Definition.ActionType = ActionType;
        Definition.DisplayName = DisplayName ? DisplayName : L"칙령";
        Definition.Summary = Summary ? Summary : L"";
        Definition.EffectText = EffectText ? EffectText : L"";
        Definition.IconPath = IconPath ? IconPath : L"";
        Definition.Implemented = true;
        Definition.BaseCost = BaseCost;
        Definition.CostPerCitizen = CostPerCitizen;
        Definition.MonthlyUpkeep = MonthlyUpkeep;
        Definition.DurationDays = DurationDays;
        Definition.CooldownDays = CooldownDays;
        Definition.Effect = BuildDefaultEdictEffectDefinition(Type);
        Definition.Signals.assign(Signals.begin(), Signals.end());
        return Definition;
    }

    bool TryBuildExpandedImplementedEdict(
        EGovernmentEdictType Type,
        EEdictEra Era,
        const wchar_t* DisplayName,
        const wchar_t* IconPath,
        FGovernmentEdictDefinition& OutDefinition)
    {
        switch (Type)
        {
        case EGovernmentEdictType::ChurchFee:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"예배 비용을 징수해 재정을 늘리지만 자유가 낮아집니다.",
                L"세수 증가, 자유 소폭 하락, 치안 소폭 상승",
                IconPath,
                800,
                0,
                120,
                0,
                0);
            return true;
        case EGovernmentEdictType::NoFreeLunch:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"복지 부담을 줄여 예산을 아끼지만 음식 불만이 커집니다.",
                L"일일 재정 개선, 음식 만족도 하락, 자유 소폭 하락",
                IconPath,
                500,
                0,
                0,
                0,
                0);
            return true;
        case EGovernmentEdictType::NothingBeatsSiesta:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"근무 중 휴식을 보장해 여유를 늘리지만 생산량은 줄어듭니다.",
                L"생산량 감소, 직업 만족도 상승, 자유 상승",
                IconPath,
                600,
                0,
                50,
                0,
                0);
            return true;
        case EGovernmentEdictType::UrbanDevelopment:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"도시 확장에 예산을 투입해 주거 환경을 개선합니다.",
                L"주거 만족도 상승, 자유 소폭 하락, 월간 유지비 발생",
                IconPath,
                1400,
                0,
                280,
                0,
                0);
            return true;
        case EGovernmentEdictType::AgriculturalSubsidies:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"농업과 식량 공급에 보조금을 투입해 생산을 끌어올립니다.",
                L"생산량 증가, 음식 만족도 상승, 예산 부담 증가",
                IconPath,
                1800,
                0,
                520,
                0,
                0);
            return true;
        case EGovernmentEdictType::BuildingPermit:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"허가 수수료를 통해 세수를 늘리는 대신 절차적 자유를 낮춥니다.",
                L"세수 증가, 자유 하락, 치안 소폭 상승",
                IconPath,
                900,
                0,
                0,
                0,
                0);
            return true;
        case EGovernmentEdictType::WealthTax:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"부유층에 더 무거운 세금을 매겨 국가 재정을 강화합니다.",
                L"세수 증가, 자유 소폭 하락, 주거 만족도 소폭 상승",
                IconPath,
                1300,
                0,
                0,
                0,
                0);
            return true;
        case EGovernmentEdictType::Industrialization:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"산업 확장에 집중해 생산성과 고용 만족을 높입니다.",
                L"생산량 상승, 직업 만족도 상승, 자유 소폭 하락",
                IconPath,
                2600,
                0,
                450,
                0,
                0);
            return true;
        case EGovernmentEdictType::LiteracyProgram:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"기초 교육을 확장해 노동 품질과 시민 의식을 개선합니다.",
                L"생산량 소폭 상승, 직업 만족도 상승, 자유 상승",
                IconPath,
                1700,
                0,
                360,
                0,
                0);
            return true;
        case EGovernmentEdictType::Prohibition:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"음주를 강하게 제한해 치안을 높이지만 자유는 떨어집니다.",
                L"치안 상승, 자유 하락, 직업 만족도 소폭 하락",
                IconPath,
                950,
                0,
                80,
                0,
                0);
            return true;
        case EGovernmentEdictType::MilitaryPolice:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"군 병력을 동원해 질서를 강화하지만 자유 억압이 커집니다.",
                L"치안 크게 상승, 자유 하락, 월간 유지비 발생",
                IconPath,
                2400,
                0,
                620,
                0,
                0);
            return true;
        case EGovernmentEdictType::RightToArms:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"개인 무장 권리를 보장해 자유를 높이지만 치안은 흔들립니다.",
                L"자유 상승, 치안 하락",
                IconPath,
                1100,
                0,
                0,
                0,
                0);
            return true;
        default:
            break;
        }

        return false;
    }
}

namespace
{
    std::vector<FGovernmentEdictDefinition> BuildDefaultGovernmentEdictDefinitions()
    {
        std::vector<FGovernmentEdictDefinition> Definitions =
        {
            MakeImplementedEdict(
                EGovernmentEdictType::FoodForThePeople,
                EEdictEra::Colonial,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::WelfareProgram,
                L"국민에게 식량을",
                L"음식 만족도를 올리지만 식사 시 식량을 더 소비합니다.",
                L"음식 회복량 증가, 식사 시 음식 2개 소비, 공산주의자 호감 상승",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_FoodForThePeople.png"),
                500,
                0,
                0,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, 7.5f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, -4.5f)
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::ChurchFee,
                EEdictEra::Colonial,
                L"종교세",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_ChurchFee.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::NoFreeLunch,
                EEdictEra::Colonial,
                L"공짜 점심 없음",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_NoFreeLunch.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::NothingBeatsSiesta,
                EEdictEra::Colonial,
                L"의무 낮잠",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_NothingBeatsSiesta.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::UrbanDevelopment,
                EEdictEra::Colonial,
                L"도시 계획",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_urbanDevelopment.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::PenalColony,
                EEdictEra::Colonial,
                L"유형지",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_australianStyle.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::ChildAllowances,
                EEdictEra::Colonial,
                L"아동 수당",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_childAllowances.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::EmployeeOfTheMonth,
                EEdictEra::Colonial,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::IndustrialSubsidy,
                L"이달의 직원",
                L"운영비를 더 들여 생산성을 끌어올립니다.",
                L"생산량 증가, 직업 만족도 소폭 상승, 월간 운영비 추가",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_employeeOfTheMonth.png"),
                0,
                0,
                350,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::EnvironmentIndustry,
                        EPoliticalStance::Right, 8.5f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 3.0f)
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::AdvancedBoatServices,
                EEdictEra::Colonial,
                L"고급 보트 서비스",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_advancedBoatServices.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::FreeHousing,
                EEdictEra::Colonial,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::HousingInitiative,
                L"여유 주택",
                L"임대 부담을 정부가 떠안아 주거 만족도를 크게 올립니다.",
                L"주거 만족도 상승, 재정 부담 증가, 공산주의자 호감 상승",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_freeHousing.png"),
                500,
                0,
                1200,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, 9.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, -6.5f)
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::SpecialTreats,
                EEdictEra::Colonial,
                L"펜하우저 스페셜",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_specialTreats.png")),

            MakePlaceholderEdict(
                EGovernmentEdictType::Audience,
                EEdictEra::WorldWars,
                L"청문회",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_Audience.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::AgriculturalSubsidies,
                EEdictEra::WorldWars,
                L"농업보조금",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_agriculturalSubsidies.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::BuildingPermit,
                EEdictEra::WorldWars,
                L"건축 허가제",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_buildingPermit.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::StateLoans,
                EEdictEra::WorldWars,
                L"국가 대출",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_printMoney.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::WealthTax,
                EEdictEra::WorldWars,
                L"부유세",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_wealthTax.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::Industrialization,
                EEdictEra::WorldWars,
                L"산업화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_industrialization.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::EarlyElections,
                EEdictEra::WorldWars,
                L"조기선거",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_earlyElections.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::LiteracyProgram,
                EEdictEra::WorldWars,
                L"장학제도",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_literacyProgram.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::MartialLaw,
                EEdictEra::WorldWars,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::MartialLaw,
                L"계엄령",
                L"자유를 희생해 치안과 공포를 높입니다.",
                L"자유 하락, 치안 상승, 군국주의자 호감, 지식인 반발",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_martialLaw.png"),
                7500,
                0,
                0,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::ReligionMilitarism,
                        EPoliticalStance::Right, 10.0f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, -8.0f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Right, 3.5f)
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::Prohibition,
                EEdictEra::WorldWars,
                L"금주법",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_prohibition.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::BellsToBullets,
                EEdictEra::WorldWars,
                L"종을 총알로",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_bellsToBullets.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MilitaryPolice,
                EEdictEra::WorldWars,
                L"헌병대",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_militaryPolice.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::Volkswagen,
                EEdictEra::WorldWars,
                L"공짜 자동차",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_volksWagen.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::RightToArms,
                EEdictEra::WorldWars,
                L"총기 소지 권리",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_rightToArms.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::FestivalBoost,
                EEdictEra::WorldWars,
                L"축제 부스트",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_festivalboost.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MardiGras,
                EEdictEra::WorldWars,
                L"마르디 그라",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_mardiGras.png")),

            MakePlaceholderEdict(
                EGovernmentEdictType::NuclearTesting,
                EEdictEra::ColdWar,
                L"핵실험",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_nuclearTesting.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::GoodOldDays,
                EEdictEra::ColdWar,
                L"옛날이 좋았지",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_goodOldDays.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::ExperimentalGroundTreatment,
                EEdictEra::ColdWar,
                L"실험적인 토양 처리법",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_experimentalGroundTreatment.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MandatoryWasteSorting,
                EEdictEra::ColdWar,
                L"분리수거 의무화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_mandatoryWasteSorting.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::DiplomaticSuperParty,
                EEdictEra::ColdWar,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                L"외교적 슈퍼 파티",
                L"대외 이미지를 개선해 주요 강대국과의 교류 창구를 넓힙니다.",
                L"외교 무역 보정 상승, 수출 단가 소폭 상승, 수입 단가 소폭 완화, 월 유지비 발생",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_diplomaticSuperParty.png"),
                3500,
                0,
                180,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, 4.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 2.0f)
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::HappyMeat,
                EEdictEra::ColdWar,
                L"자연방목 육류",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_happyMeal.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::TaxCut,
                EEdictEra::ColdWar,
                EGovernmentEdictMode::Active,
                EPoliticalActionType::TaxCut,
                L"감세",
                L"시민 수에 비례한 비용을 지불하고 단기 지지율을 끌어올립니다.",
                L"1년간 자유/지지 상승, 자본주의자 호감, 5년 재사용 대기시간",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_taxCut.png"),
                0,
                5,
                0,
                365,
                365 * 5,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 8.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, -4.0f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, 2.5f)
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::AlternativeFoodSource,
                EEdictEra::ColdWar,
                L"대체 식량원",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_alternativeFoodSource.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::SpellingBee,
                EEdictEra::ColdWar,
                L"철자 대회",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_spellingBee.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::AssemblyBan,
                EEdictEra::ColdWar,
                L"집회 금지",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_assemblyBan.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::NationalDay,
                EEdictEra::ColdWar,
                L"국경일",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_naionalDay.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MadeInTropico,
                EEdictEra::ColdWar,
                L"트로피코산",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_MadeIn.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::SocialSecurity,
                EEdictEra::ColdWar,
                L"사회보장",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_socialSecurity.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MoneyLaundering,
                EEdictEra::ColdWar,
                L"자금 세탁",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_moneylaundering3.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::HearTheCall,
                EEdictEra::ColdWar,
                L"부름을 들어라!",
                TEXT("TROPICO_ASSET\\Blueprints\\Edict\\DLC_Future\\T_ICO_edict_HearTheCall.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::LaborTaxRelief,
                EEdictEra::ColdWar,
                EGovernmentEdictMode::Active,
                EPoliticalActionType::LaborTaxRelief,
                L"근로세 경감",
                L"근로층 세금 파업에 대응해 소득세를 즉시 낮추고 고용 불만을 진정시킵니다.",
                L"근로층 세금 파업 발생 중에만 시행 가능, 소득세 4%p 인하, 파업 즉시 진정, 4개월 동안 직업/자유 완화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_taxCut.png"),
                2500,
                6,
                0,
                120,
                360,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 6.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, -2.5f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, 1.5f)
                }),
            MakeImplementedEdict(
                EGovernmentEdictType::PropertyTaxRelief,
                EEdictEra::ColdWar,
                EGovernmentEdictMode::Active,
                EPoliticalActionType::PropertyTaxRelief,
                L"재산세 유예",
                L"주거층 재산세 반발에 대응해 재산세를 낮추고 주거 불만을 완화합니다.",
                L"재산세 반발 발생 중에만 시행 가능, 재산세 10%p 인하, 반발 즉시 진정, 4개월 동안 주거 완화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_freeHousing.png"),
                3200,
                5,
                0,
                120,
                420,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 7.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, -3.0f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Right, 2.0f)
                }),

            MakePlaceholderEdict(
                EGovernmentEdictType::ContraceptionBan,
                EEdictEra::Modern,
                L"피임 금지",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_contraceptionBan.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::LegalizedSubstances,
                EEdictEra::Modern,
                L"약물 합법화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_legalizedSubstances.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::CulturalDiversity,
                EEdictEra::Modern,
                L"다문화 정책",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_culturalDiversity.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::TaxHeaven,
                EEdictEra::Modern,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                L"조세 피난처",
                L"해외 자본과 고부가 상품 무역을 끌어들여 수출 수익을 높입니다.",
                L"가공재/사치재 수출 단가 상승, 일부 수입 단가 완화, 세수 소폭 증가, 월 유지비 발생",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_taxHeaven.png"),
                6000,
                0,
                260,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, -4.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, 6.5f)
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::KnowledgioSinco,
                EEdictEra::Modern,
                L"놀리지오 신코",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_knowledgioSinco.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::Speedway,
                EEdictEra::Modern,
                L"경주장",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_speedway.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::CompulsoryVaccination,
                EEdictEra::Modern,
                L"예방 접종 의무화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_compulsoryVaccination.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::LightBulbBan,
                EEdictEra::Modern,
                L"전구 금지",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_lightBulbBan.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::SeaDisposal,
                EEdictEra::Modern,
                L"해양처분",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_seaDisposal.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::PolicyOfDetente,
                EEdictEra::Modern,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                L"데탕트 정책",
                L"긴장을 완화하고 교역 조건을 개선해 원자재와 공산품 거래를 유리하게 만듭니다.",
                L"외교 관계 완화, 원자재/가공재 수출입 보정, 치안 압박 소폭 완화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_policyOfDetente.png"),
                4200,
                0,
                160,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::ReligionMilitarism,
                        EPoliticalStance::Right, -3.5f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, 3.0f)
                }),
            MakeImplementedEdict(
                EGovernmentEdictType::CTPA,
                EEdictEra::Modern,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                L"카리브해 무역 조약 연합",
                L"지역 무역 협정에 참여해 전반적인 수출입 조건을 개선합니다.",
                L"전반적 수출입 우대, 제조·사치재 수출 보너스, 월 협정 비용 발생",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_CTPA.png"),
                5200,
                0,
                220,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 4.0f),
                    MakeSignal(EPoliticalAxis::EnvironmentIndustry,
                        EPoliticalStance::Right, 2.0f)
                }),
            MakeImplementedEdict(
                EGovernmentEdictType::TourismState,
                EEdictEra::Modern,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                L"관광 국가",
                L"관광 친화 노선으로 외화를 끌어들이지만 소비재 수요가 함께 늘어납니다.",
                L"사치재 수출 단가 상승, 관광 소비재 수입 비용 상승, 월 유지비 발생",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_tourismstate.png"),
                4800,
                0,
                190,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 2.5f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, 2.5f)
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::SoapOpera,
                EEdictEra::Modern,
                L"우주 연속극",
                TEXT("TROPICO_ASSET\\Blueprints\\Edict\\DLC_Future\\T_ICO_edict_SoapOpera.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::EmergencyAusterity,
                EEdictEra::Modern,
                EGovernmentEdictMode::Active,
                EPoliticalActionType::AusterityProgram,
                L"긴축 예산",
                L"국고 위기 경보에 대응해 긴급 자금을 투입하고 단기 긴축 체제로 전환합니다.",
                L"국고 위기 발생 중에만 시행 가능, 즉시 자금 투입, 위기 즉시 진정, 4개월 동안 일일 재정 개선과 자유/직업 압박",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_printMoney.png"),
                4500,
                4,
                0,
                120,
                480,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 2.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, -4.5f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Right, 5.5f)
                })
        };

        return Definitions;
    }

    void ResetEdictDefinitionsToDefaults()
    {
        GDefinitions = BuildDefaultGovernmentEdictDefinitions();
        GDefinitionsInitialized = true;
    }

    bool TryParseSectionHeader(
        const std::string& Line,
        std::string& OutSection)
    {
        if (Line.size() < 3 || Line.front() != '[' || Line.back() != ']')
            return false;

        OutSection = Line.substr(1, Line.size() - 2);
        TrimString(OutSection);
        return !OutSection.empty();
    }

    FGovernmentEdictDefinition* FindMutableEdictDefinition(
        const std::string& SectionName)
    {
        EGovernmentEdictType Type = EGovernmentEdictType::None;

        if (!TryParseGovernmentEdictTypeKey(SectionName, Type))
            return nullptr;

        for (size_t Index = 0; Index < GDefinitions.size(); ++Index)
        {
            if (GDefinitions[Index].Type == Type)
                return &GDefinitions[Index];
        }

        return nullptr;
    }

    bool TryParseSignalPropertyKey(
        const std::string& Key,
        size_t& OutSignalIndex,
        std::string& OutPropertyKey)
    {
        const std::string LowerKey = ToLowerCopy(Key);

        if (LowerKey.size() <= 6 ||
            LowerKey.compare(0, 6, "signal") != 0)
        {
            return false;
        }

        size_t Cursor = 6;

        while (Cursor < LowerKey.size() &&
            std::isdigit(static_cast<unsigned char>(LowerKey[Cursor])))
        {
            ++Cursor;
        }

        if (Cursor == 6 || Cursor >= LowerKey.size())
            return false;

        int ParsedIndex = 0;

        if (!TryParseIntValue(LowerKey.substr(6, Cursor - 6), ParsedIndex) ||
            ParsedIndex < 0)
        {
            return false;
        }

        OutSignalIndex = static_cast<size_t>(ParsedIndex);
        OutPropertyKey = LowerKey.substr(Cursor);
        return !OutPropertyKey.empty();
    }

    bool ApplyEdictDefinitionValue(
        FGovernmentEdictDefinition& Definition,
        const std::string& RawKey,
        const std::string& RawValue)
    {
        std::string Key = RawKey;
        TrimString(Key);
        Key = ToLowerCopy(Key);

        std::string Value = RawValue;
        TrimString(Value);

        if (Key == "displayname")
        {
            Definition.DisplayName = Utf8ToWide(UnescapeIniValue(Value));
            return true;
        }

        if (Key == "summary")
        {
            Definition.Summary = Utf8ToWide(UnescapeIniValue(Value));
            return true;
        }

        if (Key == "effecttext")
        {
            Definition.EffectText = Utf8ToWide(UnescapeIniValue(Value));
            return true;
        }

        if (Key == "iconpath")
        {
            Definition.IconPath = Utf8ToWide(UnescapeIniValue(Value));
            return true;
        }

        bool BoolValue = false;
        int IntValue = 0;
        long long LongLongValue = 0;
        float FloatValue = 0.f;

        if (Key == "implemented" &&
            TryParseBoolValue(Value, BoolValue))
        {
            Definition.Implemented = BoolValue;
            return true;
        }

        EEdictEra Era = Definition.Era;
        if (Key == "era" && TryParseEdictEraKey(Value, Era))
        {
            Definition.Era = Era;
            return true;
        }

        EGovernmentEdictMode Mode = Definition.Mode;
        if (Key == "mode" && TryParseEdictModeKey(Value, Mode))
        {
            Definition.Mode = Mode;
            return true;
        }

        EPoliticalActionType ActionType = Definition.ActionType;
        if (Key == "actiontype" &&
            TryParsePoliticalActionTypeKey(Value, ActionType))
        {
            Definition.ActionType = ActionType;
            return true;
        }

        if (Key == "basecost" && TryParseLongLongValue(Value, LongLongValue))
        {
            Definition.BaseCost = LongLongValue;
            return true;
        }

        if (Key == "costpercitizen" &&
            TryParseLongLongValue(Value, LongLongValue))
        {
            Definition.CostPerCitizen = LongLongValue;
            return true;
        }

        if (Key == "monthlyupkeep" &&
            TryParseLongLongValue(Value, LongLongValue))
        {
            Definition.MonthlyUpkeep = LongLongValue;
            return true;
        }

        if (Key == "durationdays" && TryParseIntValue(Value, IntValue))
        {
            Definition.DurationDays = IntValue;
            return true;
        }

        if (Key == "cooldowndays" && TryParseIntValue(Value, IntValue))
        {
            Definition.CooldownDays = IntValue;
            return true;
        }

        if (Key == "foodconsumptionpervisitmin" &&
            TryParseIntValue(Value, IntValue))
        {
            Definition.Effect.FoodConsumptionPerVisitMin = IntValue;
            return true;
        }

        if (Key == "dailybudgetdeltaflat" &&
            TryParseLongLongValue(Value, LongLongValue))
        {
            Definition.Effect.DailyBudgetDeltaFlat = LongLongValue;
            return true;
        }

        if (Key == "dailybudgetdeltapercitizen" &&
            TryParseLongLongValue(Value, LongLongValue))
        {
            Definition.Effect.DailyBudgetDeltaPerCitizen = LongLongValue;
            return true;
        }

        if (Key == "dailybudgetdeltapercitizenabsminimum" &&
            TryParseLongLongValue(Value, LongLongValue))
        {
            Definition.Effect.DailyBudgetDeltaPerCitizenAbsMinimum =
                LongLongValue;
            return true;
        }

        if (Key == "foodgainmultiplier" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.FoodGainMultiplier = FloatValue;
            return true;
        }

        if (Key == "productionmultiplier" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.ProductionMultiplier = FloatValue;
            return true;
        }

        if (Key == "taxrevenuemultiplier" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.TaxRevenueMultiplier = FloatValue;
            return true;
        }

        if (Key == "dailyfooddelta" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.DailyFoodDelta = FloatValue;
            return true;
        }

        if (Key == "dailyhousingdelta" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.DailyHousingDelta = FloatValue;
            return true;
        }

        if (Key == "dailyjobdelta" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.DailyJobDelta = FloatValue;
            return true;
        }

        if (Key == "dailyfreedomdelta" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.DailyFreedomDelta = FloatValue;
            return true;
        }

        if (Key == "dailysecuritydelta" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.DailySecurityDelta = FloatValue;
            return true;
        }

        if (Key == "signalcount" && TryParseIntValue(Value, IntValue))
        {
            Definition.Signals.resize(
                static_cast<size_t>((std::max)(0, IntValue)));
            return true;
        }

        size_t SignalIndex = 0;
        std::string SignalPropertyKey;

        if (!TryParseSignalPropertyKey(Key, SignalIndex, SignalPropertyKey))
            return false;

        if (Definition.Signals.size() <= SignalIndex)
            Definition.Signals.resize(SignalIndex + 1);

        FPoliticalSignalDef& Signal = Definition.Signals[SignalIndex];

        if (SignalPropertyKey == "axis")
        {
            EPoliticalAxis Axis = Signal.Axis;

            if (!TryParsePoliticalAxisKey(Value, Axis))
                return false;

            Signal.Axis = Axis;
            return true;
        }

        if (SignalPropertyKey == "stance")
        {
            EPoliticalStance Stance = Signal.FavoredStance;

            if (!TryParsePoliticalStanceKey(Value, Stance))
                return false;

            Signal.FavoredStance = Stance;
            return true;
        }

        if (SignalPropertyKey == "scope")
        {
            EPoliticalScope Scope = Signal.Scope;

            if (!TryParsePoliticalScopeKey(Value, Scope))
                return false;

            Signal.Scope = Scope;
            return true;
        }

        if (SignalPropertyKey == "strength" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Signal.Strength = FloatValue;
            return true;
        }

        return false;
    }

    bool LoadEdictDefinitionsFromFile(const std::wstring& Path)
    {
        std::ifstream File(Path);

        if (!File.is_open())
            return false;

        std::string CurrentSection;
        std::string Line;
        bool FirstLine = true;

        while (std::getline(File, Line))
        {
            if (!Line.empty() && Line.back() == '\r')
                Line.pop_back();

            if (FirstLine)
            {
                FirstLine = false;

                if (Line.size() >= 3 &&
                    static_cast<unsigned char>(Line[0]) == 0xEF &&
                    static_cast<unsigned char>(Line[1]) == 0xBB &&
                    static_cast<unsigned char>(Line[2]) == 0xBF)
                {
                    Line.erase(0, 3);
                }
            }

            std::string TrimmedLine = Line;
            TrimString(TrimmedLine);

            if (TrimmedLine.empty() ||
                TrimmedLine[0] == ';' ||
                TrimmedLine[0] == '#')
            {
                continue;
            }

            std::string ParsedSection;

            if (TryParseSectionHeader(TrimmedLine, ParsedSection))
            {
                CurrentSection = ParsedSection;
                continue;
            }

            const size_t EqPos = TrimmedLine.find('=');

            if (EqPos == std::string::npos || CurrentSection.empty())
                continue;

            std::string Key = TrimmedLine.substr(0, EqPos);
            std::string Value = TrimmedLine.substr(EqPos + 1);
            TrimString(Key);
            TrimString(Value);

            FGovernmentEdictDefinition* const Definition =
                FindMutableEdictDefinition(CurrentSection);

            if (!Definition)
                continue;

            ApplyEdictDefinitionValue(*Definition, Key, Value);
        }

        return true;
    }

    void AppendEdictIniLine(
        std::string& InOutBuffer,
        const char* Key,
        const std::string& Value)
    {
        InOutBuffer += Key ? Key : "";
        InOutBuffer += '=';
        InOutBuffer += EscapeIniValue(Value);
        InOutBuffer += "\r\n";
    }

    void WriteEdictDefinitionsToFile(
        const std::wstring& Path,
        const std::vector<FGovernmentEdictDefinition>& Definitions)
    {
        std::string Buffer;
        Buffer.reserve(32768);

        for (size_t Index = 0; Index < Definitions.size(); ++Index)
        {
            const FGovernmentEdictDefinition& Definition = Definitions[Index];
            Buffer += '[';
            Buffer += GetGovernmentEdictTypeKey(Definition.Type);
            Buffer += "]\r\n";
            AppendEdictIniLine(
                Buffer,
                "Implemented",
                Definition.Implemented ? "true" : "false");
            AppendEdictIniLine(
                Buffer,
                "Era",
                GetMappedKey(Definition.Era, GEdictEraKeys, "Colonial"));
            AppendEdictIniLine(
                Buffer,
                "Mode",
                GetMappedKey(Definition.Mode, GEdictModeKeys, "Passive"));
            AppendEdictIniLine(
                Buffer,
                "ActionType",
                GetMappedKey(
                    Definition.ActionType,
                    GPoliticalActionKeys,
                    "None"));
            AppendEdictIniLine(
                Buffer,
                "DisplayName",
                WideToUtf8(Definition.DisplayName));
            AppendEdictIniLine(
                Buffer,
                "Summary",
                WideToUtf8(Definition.Summary));
            AppendEdictIniLine(
                Buffer,
                "EffectText",
                WideToUtf8(Definition.EffectText));
            AppendEdictIniLine(
                Buffer,
                "IconPath",
                WideToUtf8(Definition.IconPath));
            AppendEdictIniLine(
                Buffer,
                "BaseCost",
                std::to_string(Definition.BaseCost));
            AppendEdictIniLine(
                Buffer,
                "CostPerCitizen",
                std::to_string(Definition.CostPerCitizen));
            AppendEdictIniLine(
                Buffer,
                "MonthlyUpkeep",
                std::to_string(Definition.MonthlyUpkeep));
            AppendEdictIniLine(
                Buffer,
                "DurationDays",
                std::to_string(Definition.DurationDays));
            AppendEdictIniLine(
                Buffer,
                "CooldownDays",
                std::to_string(Definition.CooldownDays));
            AppendEdictIniLine(
                Buffer,
                "FoodConsumptionPerVisitMin",
                std::to_string(Definition.Effect.FoodConsumptionPerVisitMin));
            AppendEdictIniLine(
                Buffer,
                "FoodGainMultiplier",
                std::to_string(Definition.Effect.FoodGainMultiplier));
            AppendEdictIniLine(
                Buffer,
                "ProductionMultiplier",
                std::to_string(Definition.Effect.ProductionMultiplier));
            AppendEdictIniLine(
                Buffer,
                "TaxRevenueMultiplier",
                std::to_string(Definition.Effect.TaxRevenueMultiplier));
            AppendEdictIniLine(
                Buffer,
                "DailyFoodDelta",
                std::to_string(Definition.Effect.DailyFoodDelta));
            AppendEdictIniLine(
                Buffer,
                "DailyHousingDelta",
                std::to_string(Definition.Effect.DailyHousingDelta));
            AppendEdictIniLine(
                Buffer,
                "DailyJobDelta",
                std::to_string(Definition.Effect.DailyJobDelta));
            AppendEdictIniLine(
                Buffer,
                "DailyFreedomDelta",
                std::to_string(Definition.Effect.DailyFreedomDelta));
            AppendEdictIniLine(
                Buffer,
                "DailySecurityDelta",
                std::to_string(Definition.Effect.DailySecurityDelta));
            AppendEdictIniLine(
                Buffer,
                "DailyBudgetDeltaFlat",
                std::to_string(Definition.Effect.DailyBudgetDeltaFlat));
            AppendEdictIniLine(
                Buffer,
                "DailyBudgetDeltaPerCitizen",
                std::to_string(Definition.Effect.DailyBudgetDeltaPerCitizen));
            AppendEdictIniLine(
                Buffer,
                "DailyBudgetDeltaPerCitizenAbsMinimum",
                std::to_string(
                    Definition.Effect.DailyBudgetDeltaPerCitizenAbsMinimum));
            AppendEdictIniLine(
                Buffer,
                "SignalCount",
                std::to_string(Definition.Signals.size()));

            for (size_t SignalIndex = 0;
                SignalIndex < Definition.Signals.size();
                ++SignalIndex)
            {
                const FPoliticalSignalDef& Signal =
                    Definition.Signals[SignalIndex];
                const std::string Prefix =
                    "Signal" + std::to_string(SignalIndex);
                AppendEdictIniLine(
                    Buffer,
                    (Prefix + "Axis").c_str(),
                    GetMappedKey(
                        Signal.Axis,
                        GPoliticalAxisKeys,
                        "Economy"));
                AppendEdictIniLine(
                    Buffer,
                    (Prefix + "Stance").c_str(),
                    GetMappedKey(
                        Signal.FavoredStance,
                        GPoliticalStanceKeys,
                        "Neutral"));
                AppendEdictIniLine(
                    Buffer,
                    (Prefix + "Strength").c_str(),
                    std::to_string(Signal.Strength));
                AppendEdictIniLine(
                    Buffer,
                    (Prefix + "Scope").c_str(),
                    GetMappedKey(
                        Signal.Scope,
                        GPoliticalScopeKeys,
                        "Global"));
            }

            Buffer += "\r\n";
        }

        FILE* File = nullptr;

        if (_wfopen_s(&File, Path.c_str(), L"wb") != 0 || !File)
            return;

        static const unsigned char Bom[] = { 0xEF, 0xBB, 0xBF };
        fwrite(Bom, 1, sizeof(Bom), File);
        fwrite(Buffer.data(), 1, Buffer.size(), File);
        fclose(File);
    }

    void EnsureDefaultEdictConfigFileExists(const std::wstring& Path)
    {
        const DWORD Attributes = GetFileAttributesW(Path.c_str());

        if (Attributes != INVALID_FILE_ATTRIBUTES &&
            (Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            return;
        }

        if (!GDefinitionsInitialized)
            ResetEdictDefinitionsToDefaults();

        WriteEdictDefinitionsToFile(Path, GDefinitions);
    }
}

namespace EdictSystem
{
    const std::vector<FGovernmentEdictDefinition>&
        GetGovernmentEdictDefinitions()
    {
        if (!GDefinitionsInitialized)
        {
            GDefinitions = BuildDefaultGovernmentEdictDefinitions();
            GDefinitionsInitialized = true;
        }

        return GDefinitions;
    }

    void RegisterRuntimeConfig()
    {
        const std::wstring ConfigPath =
            RuntimeConfigRegistry::BuildExeRelativePath(L"Edicts.ini");
        EnsureDefaultEdictConfigFileExists(ConfigPath);
        RuntimeConfigRegistry::RegisterSource(
            {
                GConfigId,
                ConfigPath,
                0.5f,
                &ResetEdictDefinitionsToDefaults,
                &LoadEdictDefinitionsFromFile,
                nullptr
            });
    }

    unsigned long long GetRuntimeConfigGeneration()
    {
        return RuntimeConfigRegistry::GetSourceGeneration(GConfigId);
    }

    const FGovernmentEdictDefinition* FindGovernmentEdictDefinition(
        EGovernmentEdictType Type)
    {
        const auto& Definitions = GetGovernmentEdictDefinitions();

        for (size_t i = 0; i < Definitions.size(); ++i)
        {
            if (Definitions[i].Type == Type)
                return &Definitions[i];
        }

        return nullptr;
    }

    void InitializeGovernmentEdictStates(
        std::vector<FGovernmentEdictState>& OutStates)
    {
        OutStates.clear();

        SynchronizeGovernmentEdictStates(OutStates);
    }

    void SynchronizeGovernmentEdictStates(
        std::vector<FGovernmentEdictState>& InOutStates)
    {
        const auto& Definitions = GetGovernmentEdictDefinitions();
        std::vector<FGovernmentEdictState> ExistingStates = InOutStates;
        InOutStates.clear();
        InOutStates.reserve(Definitions.size());

        for (size_t i = 0; i < Definitions.size(); ++i)
        {
            FGovernmentEdictState State;
            State.Type = Definitions[i].Type;

            for (size_t ExistingIndex = 0;
                ExistingIndex < ExistingStates.size();
                ++ExistingIndex)
            {
                if (ExistingStates[ExistingIndex].Type != Definitions[i].Type)
                    continue;

                State = ExistingStates[ExistingIndex];
                break;
            }

            if (!Definitions[i].Implemented)
            {
                State.Active = false;
                State.RemainingDays = 0;
                State.CooldownDays = 0;
            }

            InOutStates.push_back(State);
        }
    }

    long long ResolveEdictActivationCost(
        const FGovernmentEdictDefinition& Definition,
        int ActiveCitizenCount)
    {
        const int SafeCitizenCount = (std::max)(0, ActiveCitizenCount);
        return Definition.BaseCost +
            Definition.CostPerCitizen * static_cast<long long>(SafeCitizenCount);
    }

    long long CalculateEdictDailyUpkeep(
        const std::vector<FGovernmentEdictState>& States,
        int DaysInMonth)
    {
        const int SafeDays = (std::max)(1, DaysInMonth);
        long long Total = 0;

        for (size_t i = 0; i < States.size(); ++i)
        {
            if (!States[i].Active)
                continue;

            const FGovernmentEdictDefinition* Definition =
                FindGovernmentEdictDefinition(States[i].Type);

            if (!Definition || Definition->MonthlyUpkeep <= 0)
                continue;

            const double DailyCost =
                static_cast<double>(Definition->MonthlyUpkeep) /
                static_cast<double>(SafeDays);
            Total += static_cast<long long>(std::round(DailyCost));
        }

        return Total;
    }

    FGovernmentEdictModifiers CalculateEdictModifiers(
        const std::vector<FGovernmentEdictState>& States,
        int ActiveCitizenCount)
    {
        FGovernmentEdictModifiers Modifiers;
        const int SafeCitizenCount = (std::max)(0, ActiveCitizenCount);

        for (size_t i = 0; i < States.size(); ++i)
        {
            if (!States[i].Active)
                continue;

            const FGovernmentEdictDefinition* const Definition =
                FindGovernmentEdictDefinition(States[i].Type);

            if (!Definition || !Definition->Implemented)
                continue;

            ApplyEdictEffectToModifiers(
                Definition->Effect,
                SafeCitizenCount,
                Modifiers);
        }

        return Modifiers;
    }

    FGovernmentActionRecord MakeGovernmentActionFromEdict(
        const FGovernmentEdictDefinition& Definition)
    {
        FGovernmentActionRecord Record;
        Record.Type = Definition.ActionType;
        Record.Label = Definition.DisplayName;
        Record.Strength = 1.f;
        Record.RemainingDays = -1;
        Record.Signals = Definition.Signals;
        return Record;
    }
}
