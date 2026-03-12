#include "GameConstants.h"
#include "RuntimeConfigRegistry.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>

namespace GameConstants
{
    namespace Citizen
    {
        float NpcBaseMoveSpeed = 140.f;
        float NpcMoveSpeedVariance = 21.f;
        float CommuteWalkingRouteFactor = 1.22f;
        float CommuteRoadRouteFactor = 1.08f;
        float CommuteTransitSpeedMultiplier = 1.8f;
        float CommuteVehicleSpeedMultiplier = 3.0f;
        float CommuteTransitWaitSeconds = 18.f;
        float CommuteRoadConnectorTiles = 1.25f;
        float CommuteRoadAccessThreshold = 0.15f;
        float CommutePenaltyGraceSeconds = 18.f;
        float CommutePenaltyMaxSeconds = 90.f;
        float CommuteJobPenaltyWeight = 0.55f;
        float CommuteBusStopSearchTiles = 6.f;
        float CommuteBusStopRoadLinkTiles = 1.75f;
        float CommuteBusAverageWaitFactor = 0.5f;
        float BusDispatchSeconds = 24.f;
        int BusRouteCapacity = 20;
    }

    namespace Politics
    {
        float CitizenPoliticalShiftIntervalSeconds = 12.f;
    }

    namespace Economy
    {
        int HarborImportTargetStockPerConsumer = 1000;
        int HarborImportMaxPerResourcePerShip = 1500;
        float ProductionInputBufferSeconds = 8.f;
        float ProductionMaxBufferedUnits = 3.f;
        double DailyConsumptionSpendBase = 30.0;
        double DailyWorkerIncomeBase = 66.6666666667;
        double DailyResidenceValueBase = 11.4285714286;
    }

    namespace Orb
    {
        float AtWorkDurationSeconds = 15.f;
        float AtHomeDurationSeconds = 10.f;
        float AtFoodDurationSeconds = 4.f;
        float AtFunDurationSeconds = 6.f;
        float AtHealthDurationSeconds = 5.f;
        float AtFaithDurationSeconds = 5.f;
        float FoodInterruptThreshold = 25.f;
        float FunInterruptThreshold = 30.f;
        float HealthInterruptThreshold = 35.f;
        float FaithInterruptThreshold = 32.f;
        float HealthRemovalThreshold = 5.f;
        int ServiceStockPerCapacity = 3;
        float ServiceStockRegenPerCapacityPerSecond = 0.25f;
        float TeamsterSpeedMultiplier = 3.f;
        float TeamsterCoverageRadiusTiles = 25.f;
        int TeamsterTransferUnit = 1000;
        int TeamsterConsumerRestockThreshold = 250;
        int TeamsterConsumerTargetStock = 1000;
        float PoliticalShiftIntervalSeconds = 12.f;
    }
}

namespace
{
    constexpr const wchar_t* GConfigId = L"Game.Constants";

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
        return !OutKey.empty();
    }

    void ResetToDefaults()
    {
        using namespace GameConstants;

        Citizen::NpcBaseMoveSpeed = 140.f;
        Citizen::NpcMoveSpeedVariance = 21.f;
        Citizen::CommuteWalkingRouteFactor = 1.22f;
        Citizen::CommuteRoadRouteFactor = 1.08f;
        Citizen::CommuteTransitSpeedMultiplier = 1.8f;
        Citizen::CommuteVehicleSpeedMultiplier = 3.0f;
        Citizen::CommuteTransitWaitSeconds = 18.f;
        Citizen::CommuteRoadConnectorTiles = 1.25f;
        Citizen::CommuteRoadAccessThreshold = 0.15f;
        Citizen::CommutePenaltyGraceSeconds = 18.f;
        Citizen::CommutePenaltyMaxSeconds = 90.f;
        Citizen::CommuteJobPenaltyWeight = 0.55f;
        Citizen::CommuteBusStopSearchTiles = 6.f;
        Citizen::CommuteBusStopRoadLinkTiles = 1.75f;
        Citizen::CommuteBusAverageWaitFactor = 0.5f;
        Citizen::BusDispatchSeconds = 24.f;
        Citizen::BusRouteCapacity = 20;

        Politics::CitizenPoliticalShiftIntervalSeconds = 12.f;

        Economy::HarborImportTargetStockPerConsumer = 1000;
        Economy::HarborImportMaxPerResourcePerShip = 1500;
        Economy::ProductionInputBufferSeconds = 8.f;
        Economy::ProductionMaxBufferedUnits = 3.f;
        Economy::DailyConsumptionSpendBase = 30.0;
        Economy::DailyWorkerIncomeBase = 66.6666666667;
        Economy::DailyResidenceValueBase = 11.4285714286;

        Orb::AtWorkDurationSeconds = 15.f;
        Orb::AtHomeDurationSeconds = 10.f;
        Orb::AtFoodDurationSeconds = 4.f;
        Orb::AtFunDurationSeconds = 6.f;
        Orb::AtHealthDurationSeconds = 5.f;
        Orb::AtFaithDurationSeconds = 5.f;
        Orb::FoodInterruptThreshold = 25.f;
        Orb::FunInterruptThreshold = 30.f;
        Orb::HealthInterruptThreshold = 35.f;
        Orb::FaithInterruptThreshold = 32.f;
        Orb::HealthRemovalThreshold = 5.f;
        Orb::ServiceStockPerCapacity = 3;
        Orb::ServiceStockRegenPerCapacityPerSecond = 0.25f;
        Orb::TeamsterSpeedMultiplier = 3.f;
        Orb::TeamsterCoverageRadiusTiles = 25.f;
        Orb::TeamsterTransferUnit = 1000;
        Orb::TeamsterConsumerRestockThreshold = 250;
        Orb::TeamsterConsumerTargetStock = 1000;
        Orb::PoliticalShiftIntervalSeconds =
            Politics::CitizenPoliticalShiftIntervalSeconds;
    }

    bool ApplyFloatValue(
        const std::string& Section,
        const std::string& Key,
        float Value)
    {
        using namespace GameConstants;

        if (Section == "citizen")
        {
            if (Key == "npcbasemovespeed")
                Citizen::NpcBaseMoveSpeed = Value;
            else if (Key == "npcmovespeedvariance")
                Citizen::NpcMoveSpeedVariance = Value;
            else if (Key == "commutewalkingroutefactor")
                Citizen::CommuteWalkingRouteFactor = Value;
            else if (Key == "commuteroadroutefactor")
                Citizen::CommuteRoadRouteFactor = Value;
            else if (Key == "commutetransitspeedmultiplier")
                Citizen::CommuteTransitSpeedMultiplier = Value;
            else if (Key == "commutevehiclespeedmultiplier")
                Citizen::CommuteVehicleSpeedMultiplier = Value;
            else if (Key == "commutetransitwaitseconds")
                Citizen::CommuteTransitWaitSeconds = Value;
            else if (Key == "commuteroadconnectortiles")
                Citizen::CommuteRoadConnectorTiles = Value;
            else if (Key == "commuteroadaccessthreshold")
                Citizen::CommuteRoadAccessThreshold = Value;
            else if (Key == "commutepenaltygraceseconds")
                Citizen::CommutePenaltyGraceSeconds = Value;
            else if (Key == "commutepenaltymaxseconds")
                Citizen::CommutePenaltyMaxSeconds = Value;
            else if (Key == "commutejobpenaltyweight")
                Citizen::CommuteJobPenaltyWeight = Value;
            else if (Key == "commutebusstopsearchtiles")
                Citizen::CommuteBusStopSearchTiles = Value;
            else if (Key == "commutebusstoproadlinktiles")
                Citizen::CommuteBusStopRoadLinkTiles = Value;
            else if (Key == "commutebusaveragewaitfactor")
                Citizen::CommuteBusAverageWaitFactor = Value;
            else if (Key == "busdispatchseconds")
                Citizen::BusDispatchSeconds = Value;
            else
                return false;

            return true;
        }

        if (Section == "politics")
        {
            if (Key != "citizenpoliticalshiftintervalseconds")
                return false;

            Politics::CitizenPoliticalShiftIntervalSeconds = Value;
            return true;
        }

        if (Section == "economy")
        {
            if (Key == "productioninputbufferseconds")
                Economy::ProductionInputBufferSeconds = Value;
            else if (Key == "productionmaxbufferedunits")
                Economy::ProductionMaxBufferedUnits = Value;
            else
                return false;

            return true;
        }

        if (Section == "orb")
        {
            if (Key == "atworkdurationseconds")
                Orb::AtWorkDurationSeconds = Value;
            else if (Key == "athomedurationseconds")
                Orb::AtHomeDurationSeconds = Value;
            else if (Key == "atfooddurationseconds")
                Orb::AtFoodDurationSeconds = Value;
            else if (Key == "atfundurationseconds")
                Orb::AtFunDurationSeconds = Value;
            else if (Key == "athealthdurationseconds")
                Orb::AtHealthDurationSeconds = Value;
            else if (Key == "atfaithdurationseconds")
                Orb::AtFaithDurationSeconds = Value;
            else if (Key == "foodinterruptthreshold")
                Orb::FoodInterruptThreshold = Value;
            else if (Key == "funinterruptthreshold")
                Orb::FunInterruptThreshold = Value;
            else if (Key == "healthinterruptthreshold")
                Orb::HealthInterruptThreshold = Value;
            else if (Key == "faithinterruptthreshold")
                Orb::FaithInterruptThreshold = Value;
            else if (Key == "healthremovalthreshold")
                Orb::HealthRemovalThreshold = Value;
            else if (Key == "servicestockregenpercapacitypersecond")
                Orb::ServiceStockRegenPerCapacityPerSecond = Value;
            else if (Key == "teamsterspeedmultiplier")
                Orb::TeamsterSpeedMultiplier = Value;
            else if (Key == "teamstercoverageradiustiles")
                Orb::TeamsterCoverageRadiusTiles = Value;
            else if (Key == "politicalshiftintervalseconds")
                Orb::PoliticalShiftIntervalSeconds = Value;
            else
                return false;

            return true;
        }

        return false;
    }

    bool ApplyIntValue(
        const std::string& Section,
        const std::string& Key,
        int Value)
    {
        using namespace GameConstants;

        if (Section == "citizen")
        {
            if (Key != "busroutecapacity")
                return false;

            Citizen::BusRouteCapacity = Value;
            return true;
        }

        if (Section == "economy")
        {
            if (Key == "harborimporttargetstockperconsumer")
                Economy::HarborImportTargetStockPerConsumer = Value;
            else if (Key == "harborimportmaxperresourcepership")
                Economy::HarborImportMaxPerResourcePerShip = Value;
            else
                return false;

            return true;
        }

        if (Section == "orb")
        {
            if (Key == "servicestockpercapacity")
                Orb::ServiceStockPerCapacity = Value;
            else if (Key == "teamstertransferunit")
                Orb::TeamsterTransferUnit = Value;
            else if (Key == "teamsterconsumerrestockthreshold")
                Orb::TeamsterConsumerRestockThreshold = Value;
            else if (Key == "teamsterconsumertargetstock")
                Orb::TeamsterConsumerTargetStock = Value;
            else
                return false;

            return true;
        }

        return false;
    }

    bool ApplyDoubleValue(
        const std::string& Section,
        const std::string& Key,
        double Value)
    {
        using namespace GameConstants;

        if (Section != "economy")
            return false;

        if (Key == "dailyconsumptionspendbase")
            Economy::DailyConsumptionSpendBase = Value;
        else if (Key == "dailyworkerincomebase")
            Economy::DailyWorkerIncomeBase = Value;
        else if (Key == "dailyresidencevaluebase")
            Economy::DailyResidenceValueBase = Value;
        else
            return false;

        return true;
    }

    bool LoadFile(const std::wstring& Path)
    {
        std::ifstream File(Path);

        if (!File.is_open())
            return false;

        std::string CurrentSection;
        std::string Line;
        bool OrbPoliticalShiftExplicit = false;

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

            const size_t EqPos = TrimmedLine.find('=');

            if (EqPos == std::string::npos)
                continue;

            std::string RawKey = TrimmedLine.substr(0, EqPos);
            std::string RawValue = TrimmedLine.substr(EqPos + 1);
            TrimString(RawKey);
            TrimString(RawValue);

            if (RawKey.empty() || RawValue.empty())
                continue;

            std::string Section;
            std::string Key;

            if (!TrySplitSectionAndKey(CurrentSection, RawKey, Section, Key) ||
                Section.empty())
            {
                continue;
            }

            if (Section == "orb" &&
                Key == "politicalshiftintervalseconds")
            {
                OrbPoliticalShiftExplicit = true;
            }

            try
            {
                size_t ParsedLength = 0;
                const int IntValue = std::stoi(RawValue, &ParsedLength);

                if (ParsedLength == RawValue.size() &&
                    ApplyIntValue(Section, Key, IntValue))
                {
                    continue;
                }
            }
            catch (...)
            {
            }

            try
            {
                size_t ParsedLength = 0;
                const double DoubleValue = std::stod(RawValue, &ParsedLength);

                if (ParsedLength != RawValue.size())
                    continue;

                if (ApplyFloatValue(
                        Section,
                        Key,
                        static_cast<float>(DoubleValue)))
                {
                    continue;
                }

                if (ApplyDoubleValue(Section, Key, DoubleValue))
                    continue;
            }
            catch (...)
            {
            }
        }

        if (!OrbPoliticalShiftExplicit)
        {
            GameConstants::Orb::PoliticalShiftIntervalSeconds =
                GameConstants::Politics::CitizenPoliticalShiftIntervalSeconds;
        }

        return true;
    }
}

namespace GameConstants
{
    void RegisterRuntimeConfig()
    {
        RuntimeConfigRegistry::RegisterSource(
            {
                GConfigId,
                RuntimeConfigRegistry::BuildExeRelativePath(
                    L"GameConstants.ini"),
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
