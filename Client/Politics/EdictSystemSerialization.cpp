#include "EdictSystemSerialization.h"
#include <Windows.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

namespace
{
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
        const float ParsedValue =
            static_cast<float>(strtod(Value.c_str(), &EndPtr));

        if (!EndPtr || *EndPtr != 0)
            return false;

        OutValue = ParsedValue;
        return true;
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
        std::vector<FGovernmentEdictDefinition>& Definitions,
        const std::string& SectionName)
    {
        EGovernmentEdictType Type = EGovernmentEdictType::None;

        if (!TryParseGovernmentEdictTypeKey(SectionName, Type))
            return nullptr;

        for (size_t Index = 0; Index < Definitions.size(); ++Index)
        {
            if (Definitions[Index].Type == Type)
                return &Definitions[Index];
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

        if (Key == "dailyhealthdelta" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.DailyHealthDelta = FloatValue;
            return true;
        }

        if (Key == "dailyfaithdelta" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.DailyFaithDelta = FloatValue;
            return true;
        }

        if (Key == "dailyfundelta" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.DailyFunDelta = FloatValue;
            return true;
        }

        if (Key == "immigrationratemultiplier" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.ImmigrationRateMultiplier = FloatValue;
            return true;
        }

        if (Key == "buildingcostmultiplier" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.BuildingCostMultiplier = FloatValue;
            return true;
        }

        if (Key == "farmbuildingefficiencymultiplier" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.FarmBuildingEfficiencyMultiplier = FloatValue;
            return true;
        }

        if (Key == "exportpricemultiplier" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.ExportPriceMultiplier = FloatValue;
            return true;
        }

        if (Key == "pollutionperindustrybuildingdelta" &&
            TryParseFloatValue(Value, FloatValue))
        {
            Definition.Effect.PollutionPerIndustryBuildingDelta = FloatValue;
            return true;
        }

        if (Key == "dailybudgetdeltaperindustrybuilding" &&
            TryParseLongLongValue(Value, LongLongValue))
        {
            Definition.Effect.DailyBudgetDeltaPerIndustryBuilding =
                LongLongValue;
            return true;
        }

        if (Key == "touristratingmodifierpercent" &&
            TryParseIntValue(Value, IntValue))
        {
            Definition.Effect.TouristRatingModifierPercent = IntValue;
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
}

namespace EdictSystemSerialization
{
    bool LoadEdictDefinitionsFromFile(
        const std::wstring& Path,
        std::vector<FGovernmentEdictDefinition>& InOutDefinitions)
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
                FindMutableEdictDefinition(InOutDefinitions, CurrentSection);

            if (!Definition)
                continue;

            ApplyEdictDefinitionValue(*Definition, Key, Value);
        }

        return true;
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
}
