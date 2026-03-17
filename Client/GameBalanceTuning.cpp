#include "GameBalanceTuning.h"
#include "RuntimeConfigRegistry.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <string>

namespace GameBalanceTuning
{
    namespace Almanac
    {
        double SatisfactionClampMin = 18.0;
        double SatisfactionClampMax = 96.0;
        double SatisfactionLiftReferenceValue = 60.0;
        double SatisfactionLiftScale = 0.08;
        double SatisfactionLiftBase = 2.0;
        double SatisfactionLiftMin = 1.0;
        double SatisfactionLiftMax = 6.0;
        double SatisfactionBaselinePullScale = 0.08;
        double SatisfactionBaselinePullMin = -3.0;
        double SatisfactionBaselinePullMax = 3.0;
        double SatisfactionPoint0LiftMultiplier = 1.35;
        double SatisfactionPoint0BaselineMultiplier = 0.35;
        double SatisfactionPoint1LiftMultiplier = 0.82;
        double SatisfactionPoint1BaselineMultiplier = 0.20;
        double SatisfactionPoint2LiftMultiplier = 0.34;
        double SatisfactionPoint2BaselineMultiplier = 0.10;
        double SatisfactionTierThreshold0 = 20.0;
        double SatisfactionTierThreshold1 = 40.0;
        double SatisfactionTierThreshold2 = 55.0;
        double SatisfactionTierThreshold3 = 75.0;
    }

    namespace Politics
    {
        std::array<FWealthTierTuning, GCitizenWealthLevelCount> GWealthTiers =
        {{
            { 1.22f, 50.f },
            { 1.12f, 58.f },
            { 1.00f, 67.f },
            { 0.92f, 76.f },
            { 0.84f, 86.f }
        }};
    }
}

namespace
{
    constexpr const wchar_t* GConfigId = L"Game.Balance";

    struct FDoubleBinding
    {
        const char* Section;
        const char* Key;
        double* Value;
        double DefaultValue;
    };

    struct FWealthBinding
    {
        const char* Section;
        GameBalanceTuning::Politics::FWealthTierTuning DefaultValue;
    };

    std::array<FDoubleBinding, 20> GDoubleBindings =
    {{
        { "almanac.satisfaction", "clamp_min", &GameBalanceTuning::Almanac::SatisfactionClampMin, 18.0 },
        { "almanac.satisfaction", "clamp_max", &GameBalanceTuning::Almanac::SatisfactionClampMax, 96.0 },
        { "almanac.satisfaction", "lift_reference_value", &GameBalanceTuning::Almanac::SatisfactionLiftReferenceValue, 60.0 },
        { "almanac.satisfaction", "lift_scale", &GameBalanceTuning::Almanac::SatisfactionLiftScale, 0.08 },
        { "almanac.satisfaction", "lift_base", &GameBalanceTuning::Almanac::SatisfactionLiftBase, 2.0 },
        { "almanac.satisfaction", "lift_min", &GameBalanceTuning::Almanac::SatisfactionLiftMin, 1.0 },
        { "almanac.satisfaction", "lift_max", &GameBalanceTuning::Almanac::SatisfactionLiftMax, 6.0 },
        { "almanac.satisfaction", "baseline_pull_scale", &GameBalanceTuning::Almanac::SatisfactionBaselinePullScale, 0.08 },
        { "almanac.satisfaction", "baseline_pull_min", &GameBalanceTuning::Almanac::SatisfactionBaselinePullMin, -3.0 },
        { "almanac.satisfaction", "baseline_pull_max", &GameBalanceTuning::Almanac::SatisfactionBaselinePullMax, 3.0 },
        { "almanac.satisfaction", "point0_lift_multiplier", &GameBalanceTuning::Almanac::SatisfactionPoint0LiftMultiplier, 1.35 },
        { "almanac.satisfaction", "point0_baseline_multiplier", &GameBalanceTuning::Almanac::SatisfactionPoint0BaselineMultiplier, 0.35 },
        { "almanac.satisfaction", "point1_lift_multiplier", &GameBalanceTuning::Almanac::SatisfactionPoint1LiftMultiplier, 0.82 },
        { "almanac.satisfaction", "point1_baseline_multiplier", &GameBalanceTuning::Almanac::SatisfactionPoint1BaselineMultiplier, 0.20 },
        { "almanac.satisfaction", "point2_lift_multiplier", &GameBalanceTuning::Almanac::SatisfactionPoint2LiftMultiplier, 0.34 },
        { "almanac.satisfaction", "point2_baseline_multiplier", &GameBalanceTuning::Almanac::SatisfactionPoint2BaselineMultiplier, 0.10 },
        { "almanac.satisfaction", "tier_threshold0", &GameBalanceTuning::Almanac::SatisfactionTierThreshold0, 20.0 },
        { "almanac.satisfaction", "tier_threshold1", &GameBalanceTuning::Almanac::SatisfactionTierThreshold1, 40.0 },
        { "almanac.satisfaction", "tier_threshold2", &GameBalanceTuning::Almanac::SatisfactionTierThreshold2, 55.0 },
        { "almanac.satisfaction", "tier_threshold3", &GameBalanceTuning::Almanac::SatisfactionTierThreshold3, 75.0 }
    }};

    std::array<FWealthBinding, GCitizenWealthLevelCount> GWealthBindings =
    {{
        { "politics.wealth.broke", { 1.22f, 50.f } },
        { "politics.wealth.poor", { 1.12f, 58.f } },
        { "politics.wealth.well_off", { 1.00f, 67.f } },
        { "politics.wealth.rich", { 0.92f, 76.f } },
        { "politics.wealth.filthy_rich", { 0.84f, 86.f } }
    }};

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

    bool TryParseSectionHeader(
        const std::string& Line,
        std::string& OutSection)
    {
        if (Line.size() < 3 || Line.front() != '[' || Line.back() != ']')
            return false;

        OutSection = Line.substr(1, Line.size() - 2);
        TrimString(OutSection);
        OutSection = ToLowerCopy(OutSection);
        return !OutSection.empty();
    }

    bool TrySplitSectionAndKey(
        const std::string& CurrentSection,
        const std::string& RawKey,
        std::string& OutSection,
        std::string& OutKey)
    {
        OutSection = CurrentSection;
        OutKey = RawKey;

        const size_t DotPos = RawKey.find('.');

        if (DotPos != std::string::npos)
        {
            OutSection = RawKey.substr(0, DotPos);
            OutKey = RawKey.substr(DotPos + 1);
        }

        TrimString(OutSection);
        TrimString(OutKey);
        OutSection = ToLowerCopy(OutSection);
        OutKey = ToLowerCopy(OutKey);
        return !OutSection.empty() && !OutKey.empty();
    }

    bool TryParseDouble(const std::string& RawValue, double& OutValue)
    {
        try
        {
            size_t ParsedLength = 0;
            const double ParsedValue = std::stod(RawValue, &ParsedLength);

            if (ParsedLength != RawValue.size())
                return false;

            OutValue = ParsedValue;
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    void ResetToDefaults()
    {
        for (const FDoubleBinding& Binding : GDoubleBindings)
            *Binding.Value = Binding.DefaultValue;

        for (size_t Index = 0; Index < GWealthBindings.size(); ++Index)
        {
            GameBalanceTuning::Politics::GWealthTiers[Index] =
                GWealthBindings[Index].DefaultValue;
        }
    }

    bool ApplyDoubleValue(
        const std::string& Section,
        const std::string& Key,
        double Value)
    {
        for (const FDoubleBinding& Binding : GDoubleBindings)
        {
            if (Section == Binding.Section && Key == Binding.Key)
            {
                *Binding.Value = Value;
                return true;
            }
        }

        for (size_t Index = 0; Index < GWealthBindings.size(); ++Index)
        {
            if (Section != GWealthBindings[Index].Section)
                continue;

            if (Key == "tax_sensitivity")
            {
                GameBalanceTuning::Politics::GWealthTiers[Index].TaxSensitivity =
                    static_cast<float>(Value);
                return true;
            }

            if (Key == "expected_living_standard")
            {
                GameBalanceTuning::Politics::GWealthTiers[Index].ExpectedLivingStandard =
                    static_cast<float>(Value);
                return true;
            }

            return false;
        }

        return false;
    }

    bool LoadFile(const std::wstring& Path)
    {
        std::ifstream File(Path);

        if (!File.is_open())
            return false;

        std::string CurrentSection;
        std::string Line;

        while (std::getline(File, Line))
        {
            if (!Line.empty() && Line.back() == '\r')
                Line.pop_back();

            std::string TrimmedLine = Line;
            TrimString(TrimmedLine);

            if (TrimmedLine.empty() ||
                TrimmedLine[0] == '#' ||
                TrimmedLine[0] == ';')
            {
                continue;
            }

            std::string ParsedSection;

            if (TryParseSectionHeader(TrimmedLine, ParsedSection))
            {
                CurrentSection = ParsedSection;
                continue;
            }

            const size_t EqualsPos = TrimmedLine.find('=');

            if (EqualsPos == std::string::npos)
                continue;

            std::string RawKey = TrimmedLine.substr(0, EqualsPos);
            std::string RawValue = TrimmedLine.substr(EqualsPos + 1);
            TrimString(RawKey);
            TrimString(RawValue);

            if (RawKey.empty() || RawValue.empty())
                continue;

            std::string Section;
            std::string Key;

            if (!TrySplitSectionAndKey(CurrentSection, RawKey, Section, Key))
                continue;

            double ParsedValue = 0.0;

            if (!TryParseDouble(RawValue, ParsedValue))
                continue;

            ApplyDoubleValue(Section, Key, ParsedValue);
        }

        return true;
    }
}

namespace GameBalanceTuning
{
    namespace Politics
    {
        const FWealthTierTuning& GetWealthTier(
            ECitizenWealthLevel WealthLevel)
        {
            const size_t Index = static_cast<size_t>(GetCitizenWealthRank(WealthLevel));
            return GWealthTiers[Index];
        }
    }

    void RegisterRuntimeConfig()
    {
        RuntimeConfigRegistry::RegisterSource(
            {
                GConfigId,
                RuntimeConfigRegistry::BuildExeRelativePath(L"GameBalance.ini"),
                0.5f,
                &ResetToDefaults,
                &LoadFile,
                nullptr
            });
    }

    bool ReloadIfChanged(float DeltaTime)
    {
        RegisterRuntimeConfig();
        return RuntimeConfigRegistry::PollSource(GConfigId, DeltaTime);
    }

    unsigned long long GetRuntimeConfigGeneration()
    {
        RegisterRuntimeConfig();
        return RuntimeConfigRegistry::GetSourceGeneration(GConfigId);
    }
}
