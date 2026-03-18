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
        float ApprovalLerpRate = 0.08f;
        float ApprovalMaxDecreasePerDay = 4.0f;
        float ApprovalMaxIncreasePerDay = 3.0f;
    }

    namespace Economy
    {
        int HarborImportTargetStockPerConsumer = 1000;
        int HarborImportMaxPerResourcePerShip = 1500;
        float ProductionInputBufferSeconds = 8.f;
        float ProductionMaxBufferedUnits = 3.f;
        float BuildingWageBaseMultiplier = 0.85f;
        float BuildingUpkeepBaseMultiplier = 0.65f;
        float ExportPriceBaseMultiplier = 1.50f;
        float NegativeTradePolicyBudgetDeltaMultiplier = 0.35f;
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
        float TeamsterCoverageRadiusTiles = 0.f;
        int TeamsterTransferUnit = 1000;
        int TeamsterConsumerRestockThreshold = 250;
        int TeamsterConsumerTargetStock = 1000;
        float PoliticalShiftIntervalSeconds = 12.f;
    }

    namespace MainWorld
    {
        // Era-based demand threshold multipliers
        // Indices: 0=Colonial, 1=WorldWars, 2=ColdWar, 3=Modern
        float EraDemandThresholdMultipliers[4] = { 0.75f, 0.85f, 0.95f, 1.00f };
        float EraCrisisCooldownMultipliers[4]  = { 1.75f, 1.40f, 1.15f, 1.00f };
        float EraFactionMemberMinMultipliers[4] = { 1.67f, 1.33f, 1.00f, 1.00f };

        namespace Trade
        {
            int MaxActiveTradeRouteCount = 10;
            int MaxCompletedTradeRouteRecordCount = 12;
            int MinAmountUnits = 1000;
            int MaxAmountUnits = 24000;
            int MinDailyTransferUnits = 150;
            int MaxDailyTransferUnits = 1200;
            int DefaultDurationDays = 1500;
        }

        namespace PoliticalDemand
        {
            int MaxActiveFactionDemandCount = 2;
            int MaxActiveForeignDemandCount = 1;
            int FactionDurationDays = 90;
            int ForeignDurationDays = 105;
            int FactionCooldownDays = 90;
            int ForeignCooldownDays = 105;
            int FactionModifierDurationDays = 120;
            int NoticeDurationDays = 10;
            int CampaignPromiseLeadDays = 240;
            int FactionMemberMinCount = 6;
            float FactionApprovalThreshold = 60.f;

            FHousingDemandTuning Communists =
            {
                46, 1, 58, 72, 10, 1200, 18, 9, -12, 1.2f, 1.0f, 1.6f
            };
            FTaxCeilingDemandTuning Capitalists =
            {
                12, 12, 2000, 8, -10, 1.1f, 1.8f
            };
            FScoreDemandTuning Religious =
            {
                46, 58, 72, 10, 1500, 8, -11, 1.1f, 1.2f
            };
            FScoreDemandTuning Militarists =
            {
                48, 60, 76, 10, 1700, 8, -11, 1.1f, 1.3f
            };
            FScoreDemandTuning Environmentalists =
            {
                48, 60, 74, 9, 1600, 9, -12, 1.1f, 1.2f
            };
            FExportIncomeDemandTuning Industrialists =
            {
                5200, 5200, 1800, 2200, 8, -10, 1.0f, 700.f
            };
            FScoreDemandTuning Intellectuals =
            {
                48, 60, 76, 10, 1500, 9, -12, 1.15f, 1.3f
            };
            FTaxCeilingDemandTuning Conservatives =
            {
                35, 35, 1800, 8, -10, 1.0f, 1.5f
            };

            FForeignGlobalTuning ForeignGlobal =
            {
                72, 10, 0.7f, 0.8f, 2400, 9, 4, -10, -4
            };
            FForeignTradeRouteDemandTuning China =
            {
                2, 2, 6.0f
            };
            FForeignScoreDemandTuning Russia =
            {
                48, 60, 1.4f
            };
            FForeignScoreDemandTuning UnitedStates =
            {
                48, 60, 1.4f
            };
            FForeignScoreDemandTuning MiddleEast =
            {
                46, 58, 1.3f
            };
            FForeignScoreDemandTuning EuropeanUnion =
            {
                48, 60, 1.4f
            };
        }

        namespace ElectionPromise
        {
            FHousingPromiseTuning Housing =
            {
                8, 4, 5, 62.f, 1.25f, 2.6f
            };
            FScorePromiseTuning Food =
            {
                7, 3, 4, 60.f, 1.20f
            };
            FScorePromiseTuning Health =
            {
                7, 3, 4, 60.f, 1.15f
            };
            FJobPromiseTuning Job =
            {
                8, 4, 5, 60.f, 1.05f, 55.f
            };
            FScorePromiseTuning Freedom =
            {
                8, 3, 4, 60.f, 1.10f
            };
            FScorePromiseTuning Security =
            {
                8, 3, 4, 60.f, 1.10f
            };
            FScorePromiseTuning Faith =
            {
                7, 3, 4, 58.f, 1.05f
            };
            FExportIncomePromiseTuning ExportIncome =
            {
                1400, 5, 3, 4, 5600, 240.f
            };
        }

        namespace TradeDiplomacy
        {
            int StandingIdleDecayIntervalDays = 20;
            int RelationIdleDecayIntervalDays = 30;
        }

        namespace WorldCrisis
        {
            int RaidDurationDays = 7;
            int LaborStrikeDurationDays = 6;
            int CrimeWaveDurationDays = 9;
            int FiscalEmergencyDurationDays = 8;
            int SuccessCooldownDays = 20;
            int FailureCooldownDays = 28;
            int StartNotificationDays = 6;
            int ResolvedNotificationDays = 8;

            float SeverityDurationDivisorDays = 5.f;
            float FollowupSuccessBaseCarryover = 0.04f;
            float FollowupFailureBaseCarryover = 0.12f;
            float FollowupSeverityCarryoverWeight = 0.10f;
            float LightPressureSeverityThreshold = 0.35f;
            float HeavyPressureSeverityThreshold = 0.72f;

            FPressureRangeTuning SecurityCollapse =
            {
                58.f, 30.f
            };
            FPressureRangeTuning ResidentialSecurityCollapse =
            {
                56.f, 26.f
            };
            FPressureRangeTuning FreedomCollapse =
            {
                56.f, 28.f
            };
            FPressureRangeTuning ResidentialFreedomCollapse =
            {
                54.f, 24.f
            };
            FPressureRangeTuning JobStress =
            {
                56.f, 32.f
            };
            FPressureRangeTuning FoodStress =
            {
                54.f, 28.f
            };
            FPressureRangeTuning HousingStress =
            {
                54.f, 28.f
            };
            FPressureRangeTuning ResidentialPollutionStress =
            {
                28.f, 68.f
            };
            FPressureRangeTuning BudgetDeficit =
            {
                5000.f, 70000.f
            };
            FPressureRangeTuning NetLossPressure =
            {
                1200.f, 9000.f
            };
            FPressureRangeTuning TaxCollectionBreakdown =
            {
                0.82f, 0.48f
            };

            FImmediateBudgetTuning RaidImmediateBudget =
            {
                900, 1600.f
            };
            FImmediateBudgetTuning LaborStrikeImmediateBudget =
            {
                150, 300.f
            };
            FImmediateBudgetTuning CrimeWaveImmediateBudget =
            {
                700, 1200.f
            };
            FImmediateBudgetTuning FiscalEmergencyImmediateBudget =
            {
                1600, 2600.f
            };

            FRaidRiskTuning RaidRisk =
            {
                0.26f, 0.14f, 0.14f, 0.08f, 0.20f, 0.08f, 0.06f, 0.04f, 0.12f
            };
            FLaborStrikeRiskTuning LaborStrikeRisk =
            {
                0.34f, 0.26f, 0.16f, 0.12f, 0.12f
            };
            FCrimeWaveRiskTuning CrimeWaveRisk =
            {
                0.28f, 0.18f, 0.14f, 0.12f, 0.10f, 0.10f, 0.08f, 0.10f
            };
            FFiscalEmergencyRiskTuning FiscalEmergencyRisk =
            {
                0.42f, 0.26f, 0.20f, 0.04f, 0.08f, 0.04f, 80
            };

            FPressureTransferTuning RaidPressureTransfer =
            {
                1, 0
            };
            FPressureTransferTuning LaborStrikePressureTransfer =
            {
                1, 0
            };
            FPressureTransferTuning CrimeWavePressureTransfer =
            {
                1, 0
            };
            FPressureTransferTuning FiscalEmergencyPressureTransfer =
            {
                1, 0
            };
        }
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
        Politics::ApprovalLerpRate = 0.08f;
        Politics::ApprovalMaxDecreasePerDay = 4.0f;
        Politics::ApprovalMaxIncreasePerDay = 3.0f;

        Economy::HarborImportTargetStockPerConsumer = 1000;
        Economy::HarborImportMaxPerResourcePerShip = 1500;
        Economy::ProductionInputBufferSeconds = 8.f;
        Economy::ProductionMaxBufferedUnits = 3.f;
        Economy::BuildingWageBaseMultiplier = 0.85f;
        Economy::BuildingUpkeepBaseMultiplier = 0.65f;
        Economy::ExportPriceBaseMultiplier = 1.50f;
        Economy::NegativeTradePolicyBudgetDeltaMultiplier = 0.35f;
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
        Orb::TeamsterCoverageRadiusTiles = 0.f;
        Orb::TeamsterTransferUnit = 1000;
        Orb::TeamsterConsumerRestockThreshold = 250;
        Orb::TeamsterConsumerTargetStock = 1000;
        Orb::PoliticalShiftIntervalSeconds =
            Politics::CitizenPoliticalShiftIntervalSeconds;

        MainWorld::EraDemandThresholdMultipliers[0] = 0.75f;
        MainWorld::EraDemandThresholdMultipliers[1] = 0.85f;
        MainWorld::EraDemandThresholdMultipliers[2] = 0.95f;
        MainWorld::EraDemandThresholdMultipliers[3] = 1.00f;

        MainWorld::EraCrisisCooldownMultipliers[0] = 1.75f;
        MainWorld::EraCrisisCooldownMultipliers[1] = 1.40f;
        MainWorld::EraCrisisCooldownMultipliers[2] = 1.15f;
        MainWorld::EraCrisisCooldownMultipliers[3] = 1.00f;

        MainWorld::EraFactionMemberMinMultipliers[0] = 1.67f;
        MainWorld::EraFactionMemberMinMultipliers[1] = 1.33f;
        MainWorld::EraFactionMemberMinMultipliers[2] = 1.00f;
        MainWorld::EraFactionMemberMinMultipliers[3] = 1.00f;

        MainWorld::Trade::MaxActiveTradeRouteCount = 10;
        MainWorld::Trade::MaxCompletedTradeRouteRecordCount = 12;
        MainWorld::Trade::MinAmountUnits = 1000;
        MainWorld::Trade::MaxAmountUnits = 24000;
        MainWorld::Trade::MinDailyTransferUnits = 150;
        MainWorld::Trade::MaxDailyTransferUnits = 1200;
        MainWorld::Trade::DefaultDurationDays = 1500;

        MainWorld::PoliticalDemand::MaxActiveFactionDemandCount = 2;
        MainWorld::PoliticalDemand::MaxActiveForeignDemandCount = 1;
        MainWorld::PoliticalDemand::FactionDurationDays = 90;
        MainWorld::PoliticalDemand::ForeignDurationDays = 105;
        MainWorld::PoliticalDemand::FactionCooldownDays = 90;
        MainWorld::PoliticalDemand::ForeignCooldownDays = 105;
        MainWorld::PoliticalDemand::FactionModifierDurationDays = 120;
        MainWorld::PoliticalDemand::NoticeDurationDays = 10;
        MainWorld::PoliticalDemand::CampaignPromiseLeadDays = 240;
        MainWorld::PoliticalDemand::FactionMemberMinCount = 6;
        MainWorld::PoliticalDemand::FactionApprovalThreshold = 60.f;

        MainWorld::PoliticalDemand::Communists =
        {
            46, 1, 58, 72, 10, 1200, 18, 9, -12, 1.2f, 1.0f, 1.6f
        };
        MainWorld::PoliticalDemand::Capitalists =
        {
            12, 12, 2000, 8, -10, 1.1f, 1.8f
        };
        MainWorld::PoliticalDemand::Religious =
        {
            46, 58, 72, 10, 1500, 8, -11, 1.1f, 1.2f
        };
        MainWorld::PoliticalDemand::Militarists =
        {
            48, 60, 76, 10, 1700, 8, -11, 1.1f, 1.3f
        };
        MainWorld::PoliticalDemand::Environmentalists =
        {
            48, 60, 74, 9, 1600, 9, -12, 1.1f, 1.2f
        };
        MainWorld::PoliticalDemand::Industrialists =
        {
            5200, 5200, 1800, 2200, 8, -10, 1.0f, 700.f
        };
        MainWorld::PoliticalDemand::Intellectuals =
        {
            48, 60, 76, 10, 1500, 9, -12, 1.15f, 1.3f
        };
        MainWorld::PoliticalDemand::Conservatives =
        {
            35, 35, 1800, 8, -10, 1.0f, 1.5f
        };
        MainWorld::PoliticalDemand::ForeignGlobal =
        {
            72, 10, 0.7f, 0.8f, 2400, 9, 4, -10, -4
        };
        MainWorld::PoliticalDemand::China =
        {
            2, 2, 6.0f
        };
        MainWorld::PoliticalDemand::Russia =
        {
            48, 60, 1.4f
        };
        MainWorld::PoliticalDemand::UnitedStates =
        {
            48, 60, 1.4f
        };
        MainWorld::PoliticalDemand::MiddleEast =
        {
            46, 58, 1.3f
        };
        MainWorld::PoliticalDemand::EuropeanUnion =
        {
            48, 60, 1.4f
        };

        MainWorld::ElectionPromise::Housing =
        {
            8, 4, 5, 62.f, 1.25f, 2.6f
        };
        MainWorld::ElectionPromise::Food =
        {
            7, 3, 4, 60.f, 1.20f
        };
        MainWorld::ElectionPromise::Health =
        {
            7, 3, 4, 60.f, 1.15f
        };
        MainWorld::ElectionPromise::Job =
        {
            8, 4, 5, 60.f, 1.05f, 55.f
        };
        MainWorld::ElectionPromise::Freedom =
        {
            8, 3, 4, 60.f, 1.10f
        };
        MainWorld::ElectionPromise::Security =
        {
            8, 3, 4, 60.f, 1.10f
        };
        MainWorld::ElectionPromise::Faith =
        {
            7, 3, 4, 58.f, 1.05f
        };
        MainWorld::ElectionPromise::ExportIncome =
        {
            1400, 5, 3, 4, 5600, 240.f
        };

        MainWorld::TradeDiplomacy::StandingIdleDecayIntervalDays = 20;
        MainWorld::TradeDiplomacy::RelationIdleDecayIntervalDays = 30;

        MainWorld::WorldCrisis::RaidDurationDays = 7;
        MainWorld::WorldCrisis::LaborStrikeDurationDays = 6;
        MainWorld::WorldCrisis::CrimeWaveDurationDays = 9;
        MainWorld::WorldCrisis::FiscalEmergencyDurationDays = 8;
        MainWorld::WorldCrisis::SuccessCooldownDays = 20;
        MainWorld::WorldCrisis::FailureCooldownDays = 28;
        MainWorld::WorldCrisis::StartNotificationDays = 6;
        MainWorld::WorldCrisis::ResolvedNotificationDays = 8;

        MainWorld::WorldCrisis::SeverityDurationDivisorDays = 5.f;
        MainWorld::WorldCrisis::FollowupSuccessBaseCarryover = 0.04f;
        MainWorld::WorldCrisis::FollowupFailureBaseCarryover = 0.12f;
        MainWorld::WorldCrisis::FollowupSeverityCarryoverWeight = 0.10f;
        MainWorld::WorldCrisis::LightPressureSeverityThreshold = 0.35f;
        MainWorld::WorldCrisis::HeavyPressureSeverityThreshold = 0.72f;

        MainWorld::WorldCrisis::SecurityCollapse =
        {
            58.f, 30.f
        };
        MainWorld::WorldCrisis::ResidentialSecurityCollapse =
        {
            56.f, 26.f
        };
        MainWorld::WorldCrisis::FreedomCollapse =
        {
            56.f, 28.f
        };
        MainWorld::WorldCrisis::ResidentialFreedomCollapse =
        {
            54.f, 24.f
        };
        MainWorld::WorldCrisis::JobStress =
        {
            56.f, 32.f
        };
        MainWorld::WorldCrisis::FoodStress =
        {
            54.f, 28.f
        };
        MainWorld::WorldCrisis::HousingStress =
        {
            54.f, 28.f
        };
        MainWorld::WorldCrisis::ResidentialPollutionStress =
        {
            28.f, 68.f
        };
        MainWorld::WorldCrisis::BudgetDeficit =
        {
            5000.f, 70000.f
        };
        MainWorld::WorldCrisis::NetLossPressure =
        {
            1200.f, 9000.f
        };
        MainWorld::WorldCrisis::TaxCollectionBreakdown =
        {
            0.82f, 0.48f
        };

        MainWorld::WorldCrisis::RaidImmediateBudget =
        {
            900, 1600.f
        };
        MainWorld::WorldCrisis::LaborStrikeImmediateBudget =
        {
            150, 300.f
        };
        MainWorld::WorldCrisis::CrimeWaveImmediateBudget =
        {
            700, 1200.f
        };
        MainWorld::WorldCrisis::FiscalEmergencyImmediateBudget =
        {
            1600, 2600.f
        };

        MainWorld::WorldCrisis::RaidRisk =
        {
            0.26f, 0.14f, 0.14f, 0.08f, 0.20f, 0.08f, 0.06f, 0.04f, 0.12f
        };
        MainWorld::WorldCrisis::LaborStrikeRisk =
        {
            0.34f, 0.26f, 0.16f, 0.12f, 0.12f
        };
        MainWorld::WorldCrisis::CrimeWaveRisk =
        {
            0.28f, 0.18f, 0.14f, 0.12f, 0.10f, 0.10f, 0.08f, 0.10f
        };
        MainWorld::WorldCrisis::FiscalEmergencyRisk =
        {
            0.42f, 0.26f, 0.20f, 0.04f, 0.08f, 0.04f, 80
        };

        MainWorld::WorldCrisis::RaidPressureTransfer =
        {
            1, 0
        };
        MainWorld::WorldCrisis::LaborStrikePressureTransfer =
        {
            1, 0
        };
        MainWorld::WorldCrisis::CrimeWavePressureTransfer =
        {
            1, 0
        };
        MainWorld::WorldCrisis::FiscalEmergencyPressureTransfer =
        {
            1, 0
        };
    }

    bool ApplyScoreDemandIntValue(
        GameConstants::MainWorld::PoliticalDemand::FScoreDemandTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "ignoreatorabovevalue")
            Tuning.IgnoreAtOrAboveValue = Value;
        else if (Key == "targetmin")
            Tuning.TargetMin = Value;
        else if (Key == "targetmax")
            Tuning.TargetMax = Value;
        else if (Key == "targetdelta")
            Tuning.TargetDelta = Value;
        else if (Key == "rewardbudgetdelta")
            Tuning.RewardBudgetDelta = Value;
        else if (Key == "rewardapprovaldelta")
            Tuning.RewardApprovalDelta = Value;
        else if (Key == "penaltyapprovaldelta")
            Tuning.PenaltyApprovalDelta = Value;
        else
            return false;

        return true;
    }

    bool ApplyScoreDemandFloatValue(
        GameConstants::MainWorld::PoliticalDemand::FScoreDemandTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "approvalpriorityweight")
            Tuning.ApprovalPriorityWeight = Value;
        else if (Key == "deficitpriorityweight")
            Tuning.DeficitPriorityWeight = Value;
        else
            return false;

        return true;
    }

    bool ApplyHousingDemandIntValue(
        GameConstants::MainWorld::PoliticalDemand::FHousingDemandTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "ignoreatorabovehousing")
            Tuning.IgnoreAtOrAboveHousing = Value;
        else if (Key == "ignoreatorbelowhomeless")
            Tuning.IgnoreAtOrBelowHomeless = Value;
        else if (Key == "targetmin")
            Tuning.TargetMin = Value;
        else if (Key == "targetmax")
            Tuning.TargetMax = Value;
        else if (Key == "targetdelta")
            Tuning.TargetDelta = Value;
        else if (Key == "rewardbudgetbase")
            Tuning.RewardBudgetBase = Value;
        else if (Key == "rewardbudgetpermember")
            Tuning.RewardBudgetPerMember = Value;
        else if (Key == "rewardapprovaldelta")
            Tuning.RewardApprovalDelta = Value;
        else if (Key == "penaltyapprovaldelta")
            Tuning.PenaltyApprovalDelta = Value;
        else
            return false;

        return true;
    }

    bool ApplyHousingDemandFloatValue(
        GameConstants::MainWorld::PoliticalDemand::FHousingDemandTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "approvalpriorityweight")
            Tuning.ApprovalPriorityWeight = Value;
        else if (Key == "housingdeficitpriorityweight")
            Tuning.HousingDeficitPriorityWeight = Value;
        else if (Key == "homelesspriorityweight")
            Tuning.HomelessPriorityWeight = Value;
        else
            return false;

        return true;
    }

    bool ApplyTaxCeilingDemandIntValue(
        GameConstants::MainWorld::PoliticalDemand::FTaxCeilingDemandTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "ignoreatorbelowvalue")
            Tuning.IgnoreAtOrBelowValue = Value;
        else if (Key == "targetvalue")
            Tuning.TargetValue = Value;
        else if (Key == "rewardbudgetdelta")
            Tuning.RewardBudgetDelta = Value;
        else if (Key == "rewardapprovaldelta")
            Tuning.RewardApprovalDelta = Value;
        else if (Key == "penaltyapprovaldelta")
            Tuning.PenaltyApprovalDelta = Value;
        else
            return false;

        return true;
    }

    bool ApplyTaxCeilingDemandFloatValue(
        GameConstants::MainWorld::PoliticalDemand::FTaxCeilingDemandTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "approvalpriorityweight")
            Tuning.ApprovalPriorityWeight = Value;
        else if (Key == "excesspriorityweight")
            Tuning.ExcessPriorityWeight = Value;
        else
            return false;

        return true;
    }

    bool ApplyExportIncomeDemandIntValue(
        GameConstants::MainWorld::PoliticalDemand::FExportIncomeDemandTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "ignoreatorabovevalue")
            Tuning.IgnoreAtOrAboveValue = Value;
        else if (Key == "targetmin")
            Tuning.TargetMin = Value;
        else if (Key == "targetdelta")
            Tuning.TargetDelta = Value;
        else if (Key == "rewardbudgetdelta")
            Tuning.RewardBudgetDelta = Value;
        else if (Key == "rewardapprovaldelta")
            Tuning.RewardApprovalDelta = Value;
        else if (Key == "penaltyapprovaldelta")
            Tuning.PenaltyApprovalDelta = Value;
        else
            return false;

        return true;
    }

    bool ApplyExportIncomeDemandFloatValue(
        GameConstants::MainWorld::PoliticalDemand::FExportIncomeDemandTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "approvalpriorityweight")
            Tuning.ApprovalPriorityWeight = Value;
        else if (Key == "deficitdivisor")
            Tuning.DeficitDivisor = Value;
        else
            return false;

        return true;
    }

    bool ApplyForeignGlobalIntValue(
        GameConstants::MainWorld::PoliticalDemand::FForeignGlobalTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "relationpressurestart")
            Tuning.RelationPressureStart = Value;
        else if (Key == "standingpressurestart")
            Tuning.StandingPressureStart = Value;
        else if (Key == "rewardbudgetdelta")
            Tuning.RewardBudgetDelta = Value;
        else if (Key == "rewardrelationdelta")
            Tuning.RewardRelationDelta = Value;
        else if (Key == "rewardstandingdelta")
            Tuning.RewardStandingDelta = Value;
        else if (Key == "penaltyrelationdelta")
            Tuning.PenaltyRelationDelta = Value;
        else if (Key == "penaltystandingdelta")
            Tuning.PenaltyStandingDelta = Value;
        else
            return false;

        return true;
    }

    bool ApplyForeignGlobalFloatValue(
        GameConstants::MainWorld::PoliticalDemand::FForeignGlobalTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "relationpriorityweight")
            Tuning.RelationPriorityWeight = Value;
        else if (Key == "standingpriorityweight")
            Tuning.StandingPriorityWeight = Value;
        else
            return false;

        return true;
    }

    bool ApplyForeignTradeRouteDemandIntValue(
        GameConstants::MainWorld::PoliticalDemand::FForeignTradeRouteDemandTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "ignoreatoraboveactiveroutes")
            Tuning.IgnoreAtOrAboveActiveRoutes = Value;
        else if (Key == "targetvalue")
            Tuning.TargetValue = Value;
        else
            return false;

        return true;
    }

    bool ApplyForeignTradeRouteDemandFloatValue(
        GameConstants::MainWorld::PoliticalDemand::FForeignTradeRouteDemandTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key != "priorityweight")
            return false;

        Tuning.PriorityWeight = Value;
        return true;
    }

    bool ApplyForeignScoreDemandIntValue(
        GameConstants::MainWorld::PoliticalDemand::FForeignScoreDemandTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "ignoreatorabovevalue")
            Tuning.IgnoreAtOrAboveValue = Value;
        else if (Key == "targetvalue")
            Tuning.TargetValue = Value;
        else
            return false;

        return true;
    }

    bool ApplyForeignScoreDemandFloatValue(
        GameConstants::MainWorld::PoliticalDemand::FForeignScoreDemandTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key != "priorityweight")
            return false;

        Tuning.PriorityWeight = Value;
        return true;
    }

    bool ApplyScorePromiseIntValue(
        GameConstants::MainWorld::ElectionPromise::FScorePromiseTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "targetdelta")
            Tuning.TargetDelta = Value;
        else if (Key == "successvotemodifierpercent")
            Tuning.SuccessVoteModifierPercent = Value;
        else if (Key == "failurevotemodifierpercent")
            Tuning.FailureVoteModifierPercent = Value;
        else
            return false;

        return true;
    }

    bool ApplyScorePromiseFloatValue(
        GameConstants::MainWorld::ElectionPromise::FScorePromiseTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "prioritythresholdvalue")
            Tuning.PriorityThresholdValue = Value;
        else if (Key == "prioritydeficitweight")
            Tuning.PriorityDeficitWeight = Value;
        else
            return false;

        return true;
    }

    bool ApplyHousingPromiseIntValue(
        GameConstants::MainWorld::ElectionPromise::FHousingPromiseTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "targetdelta")
            Tuning.TargetDelta = Value;
        else if (Key == "successvotemodifierpercent")
            Tuning.SuccessVoteModifierPercent = Value;
        else if (Key == "failurevotemodifierpercent")
            Tuning.FailureVoteModifierPercent = Value;
        else
            return false;

        return true;
    }

    bool ApplyHousingPromiseFloatValue(
        GameConstants::MainWorld::ElectionPromise::FHousingPromiseTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "prioritythresholdvalue")
            Tuning.PriorityThresholdValue = Value;
        else if (Key == "prioritydeficitweight")
            Tuning.PriorityDeficitWeight = Value;
        else if (Key == "homelesshouseholdpriorityweight")
            Tuning.HomelessHouseholdPriorityWeight = Value;
        else
            return false;

        return true;
    }

    bool ApplyJobPromiseIntValue(
        GameConstants::MainWorld::ElectionPromise::FJobPromiseTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "targetdelta")
            Tuning.TargetDelta = Value;
        else if (Key == "successvotemodifierpercent")
            Tuning.SuccessVoteModifierPercent = Value;
        else if (Key == "failurevotemodifierpercent")
            Tuning.FailureVoteModifierPercent = Value;
        else
            return false;

        return true;
    }

    bool ApplyJobPromiseFloatValue(
        GameConstants::MainWorld::ElectionPromise::FJobPromiseTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "prioritythresholdvalue")
            Tuning.PriorityThresholdValue = Value;
        else if (Key == "prioritydeficitweight")
            Tuning.PriorityDeficitWeight = Value;
        else if (Key == "unemploymentratiopriorityweight")
            Tuning.UnemploymentRatioPriorityWeight = Value;
        else
            return false;

        return true;
    }

    bool ApplyExportIncomePromiseIntValue(
        GameConstants::MainWorld::ElectionPromise::FExportIncomePromiseTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "targetmindelta")
            Tuning.TargetMinDelta = Value;
        else if (Key == "targetscaledivisor")
            Tuning.TargetScaleDivisor = Value;
        else if (Key == "successvotemodifierpercent")
            Tuning.SuccessVoteModifierPercent = Value;
        else if (Key == "failurevotemodifierpercent")
            Tuning.FailureVoteModifierPercent = Value;
        else if (Key == "priorityreferencevalue")
            Tuning.PriorityReferenceValue = Value;
        else
            return false;

        return true;
    }

    bool ApplyExportIncomePromiseFloatValue(
        GameConstants::MainWorld::ElectionPromise::FExportIncomePromiseTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key != "prioritydivisor")
            return false;

        Tuning.PriorityDivisor = Value;
        return true;
    }

    bool ApplyPressureRangeFloatValue(
        GameConstants::MainWorld::WorldCrisis::FPressureRangeTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "startvalue")
            Tuning.StartValue = Value;
        else if (Key == "fullvalue")
            Tuning.FullValue = Value;
        else
            return false;

        return true;
    }

    bool ApplyImmediateBudgetIntValue(
        GameConstants::MainWorld::WorldCrisis::FImmediateBudgetTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key != "basepenalty")
            return false;

        Tuning.BasePenalty = Value;
        return true;
    }

    bool ApplyImmediateBudgetFloatValue(
        GameConstants::MainWorld::WorldCrisis::FImmediateBudgetTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key != "riskpenaltyscale")
            return false;

        Tuning.RiskPenaltyScale = Value;
        return true;
    }

    bool ApplyRaidRiskFloatValue(
        GameConstants::MainWorld::WorldCrisis::FRaidRiskTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "securitycollapseweight")
            Tuning.SecurityCollapseWeight = Value;
        else if (Key == "residentialsecuritycollapseweight")
            Tuning.ResidentialSecurityCollapseWeight = Value;
        else if (Key == "freedomcollapseweight")
            Tuning.FreedomCollapseWeight = Value;
        else if (Key == "residentialfreedomcollapseweight")
            Tuning.ResidentialFreedomCollapseWeight = Value;
        else if (Key == "oppositionratioweight")
            Tuning.OppositionRatioWeight = Value;
        else if (Key == "foodstressweight")
            Tuning.FoodStressWeight = Value;
        else if (Key == "housingstressweight")
            Tuning.HousingStressWeight = Value;
        else if (Key == "residentialpollutionstressweight")
            Tuning.ResidentialPollutionStressWeight = Value;
        else if (Key == "martiallawreduction")
            Tuning.MartialLawReduction = Value;
        else
            return false;

        return true;
    }

    bool ApplyLaborStrikeRiskFloatValue(
        GameConstants::MainWorld::WorldCrisis::FLaborStrikeRiskTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "unemploymentratioweight")
            Tuning.UnemploymentRatioWeight = Value;
        else if (Key == "jobstressweight")
            Tuning.JobStressWeight = Value;
        else if (Key == "incometaxpressureweight")
            Tuning.IncomeTaxPressureWeight = Value;
        else if (Key == "oppositionratioweight")
            Tuning.OppositionRatioWeight = Value;
        else if (Key == "workertaxstrikebonus")
            Tuning.WorkerTaxStrikeBonus = Value;
        else
            return false;

        return true;
    }

    bool ApplyCrimeWaveRiskFloatValue(
        GameConstants::MainWorld::WorldCrisis::FCrimeWaveRiskTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "securitycollapseweight")
            Tuning.SecurityCollapseWeight = Value;
        else if (Key == "residentialsecuritycollapseweight")
            Tuning.ResidentialSecurityCollapseWeight = Value;
        else if (Key == "homelessratioweight")
            Tuning.HomelessRatioWeight = Value;
        else if (Key == "unemploymentratioweight")
            Tuning.UnemploymentRatioWeight = Value;
        else if (Key == "housingstressweight")
            Tuning.HousingStressWeight = Value;
        else if (Key == "residentialpollutionstressweight")
            Tuning.ResidentialPollutionStressWeight = Value;
        else if (Key == "netlosspressureweight")
            Tuning.NetLossPressureWeight = Value;
        else if (Key == "martiallawreduction")
            Tuning.MartialLawReduction = Value;
        else
            return false;

        return true;
    }

    bool ApplyFiscalEmergencyRiskIntValue(
        GameConstants::MainWorld::WorldCrisis::FFiscalEmergencyRiskTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key != "largepopulationthreshold")
            return false;

        Tuning.LargePopulationThreshold = Value;
        return true;
    }

    bool ApplyFiscalEmergencyRiskFloatValue(
        GameConstants::MainWorld::WorldCrisis::FFiscalEmergencyRiskTuning& Tuning,
        const std::string& Key,
        float Value)
    {
        if (Key == "budgetdeficitweight")
            Tuning.BudgetDeficitWeight = Value;
        else if (Key == "netlosspressureweight")
            Tuning.NetLossPressureWeight = Value;
        else if (Key == "taxcollectionbreakdownweight")
            Tuning.TaxCollectionBreakdownWeight = Value;
        else if (Key == "residentialpollutionstressweight")
            Tuning.ResidentialPollutionStressWeight = Value;
        else if (Key == "budgetcrisisbonus")
            Tuning.BudgetCrisisBonus = Value;
        else if (Key == "largepopulationbonus")
            Tuning.LargePopulationBonus = Value;
        else
            return false;

        return true;
    }

    bool ApplyPressureTransferIntValue(
        GameConstants::MainWorld::WorldCrisis::FPressureTransferTuning& Tuning,
        const std::string& Key,
        int Value)
    {
        if (Key == "primarybasedays")
            Tuning.PrimaryBaseDays = Value;
        else if (Key == "secondarybasedays")
            Tuning.SecondaryBaseDays = Value;
        else
            return false;

        return true;
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
            else if (Key == "buildingwagebasemultiplier")
                Economy::BuildingWageBaseMultiplier = Value;
            else if (Key == "buildingupkeepbasemultiplier")
                Economy::BuildingUpkeepBaseMultiplier = Value;
            else if (Key == "exportpricebasemultiplier")
                Economy::ExportPriceBaseMultiplier = Value;
            else if (Key == "negativetradepolicybudgetdeltamultiplier")
                Economy::NegativeTradePolicyBudgetDeltaMultiplier = Value;
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

        if (Section == "mainworldpoliticaldemand")
        {
            if (Key != "factionapprovalthreshold")
                return false;

            MainWorld::PoliticalDemand::FactionApprovalThreshold = Value;
            return true;
        }

        if (Section == "mainworldfactioncommunists")
        {
            return ApplyHousingDemandFloatValue(
                MainWorld::PoliticalDemand::Communists,
                Key,
                Value);
        }

        if (Section == "mainworldfactioncapitalists")
        {
            return ApplyTaxCeilingDemandFloatValue(
                MainWorld::PoliticalDemand::Capitalists,
                Key,
                Value);
        }

        if (Section == "mainworldfactionreligious")
        {
            return ApplyScoreDemandFloatValue(
                MainWorld::PoliticalDemand::Religious,
                Key,
                Value);
        }

        if (Section == "mainworldfactionmilitarists")
        {
            return ApplyScoreDemandFloatValue(
                MainWorld::PoliticalDemand::Militarists,
                Key,
                Value);
        }

        if (Section == "mainworldfactionenvironmentalists")
        {
            return ApplyScoreDemandFloatValue(
                MainWorld::PoliticalDemand::Environmentalists,
                Key,
                Value);
        }

        if (Section == "mainworldfactionindustrialists")
        {
            return ApplyExportIncomeDemandFloatValue(
                MainWorld::PoliticalDemand::Industrialists,
                Key,
                Value);
        }

        if (Section == "mainworldfactionintellectuals")
        {
            return ApplyScoreDemandFloatValue(
                MainWorld::PoliticalDemand::Intellectuals,
                Key,
                Value);
        }

        if (Section == "mainworldfactionconservatives")
        {
            return ApplyTaxCeilingDemandFloatValue(
                MainWorld::PoliticalDemand::Conservatives,
                Key,
                Value);
        }

        if (Section == "mainworldforeignglobal")
        {
            return ApplyForeignGlobalFloatValue(
                MainWorld::PoliticalDemand::ForeignGlobal,
                Key,
                Value);
        }

        if (Section == "mainworldforeignchina")
        {
            return ApplyForeignTradeRouteDemandFloatValue(
                MainWorld::PoliticalDemand::China,
                Key,
                Value);
        }

        if (Section == "mainworldforeignrussia")
        {
            return ApplyForeignScoreDemandFloatValue(
                MainWorld::PoliticalDemand::Russia,
                Key,
                Value);
        }

        if (Section == "mainworldforeignunitedstates")
        {
            return ApplyForeignScoreDemandFloatValue(
                MainWorld::PoliticalDemand::UnitedStates,
                Key,
                Value);
        }

        if (Section == "mainworldforeignmiddleeast")
        {
            return ApplyForeignScoreDemandFloatValue(
                MainWorld::PoliticalDemand::MiddleEast,
                Key,
                Value);
        }

        if (Section == "mainworldforeigneuropeanunion")
        {
            return ApplyForeignScoreDemandFloatValue(
                MainWorld::PoliticalDemand::EuropeanUnion,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisehousing")
        {
            return ApplyHousingPromiseFloatValue(
                MainWorld::ElectionPromise::Housing,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisefood")
        {
            return ApplyScorePromiseFloatValue(
                MainWorld::ElectionPromise::Food,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisehealth")
        {
            return ApplyScorePromiseFloatValue(
                MainWorld::ElectionPromise::Health,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisejob")
        {
            return ApplyJobPromiseFloatValue(
                MainWorld::ElectionPromise::Job,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisefreedom")
        {
            return ApplyScorePromiseFloatValue(
                MainWorld::ElectionPromise::Freedom,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisesecurity")
        {
            return ApplyScorePromiseFloatValue(
                MainWorld::ElectionPromise::Security,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisefaith")
        {
            return ApplyScorePromiseFloatValue(
                MainWorld::ElectionPromise::Faith,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromiseexportincome")
        {
            return ApplyExportIncomePromiseFloatValue(
                MainWorld::ElectionPromise::ExportIncome,
                Key,
                Value);
        }

        if (Section == "mainworldworldcrisis")
        {
            if (Key == "severitydurationdivisordays")
                MainWorld::WorldCrisis::SeverityDurationDivisorDays = Value;
            else if (Key == "followupsuccessbasecarryover")
                MainWorld::WorldCrisis::FollowupSuccessBaseCarryover = Value;
            else if (Key == "followupfailurebasecarryover")
                MainWorld::WorldCrisis::FollowupFailureBaseCarryover = Value;
            else if (Key == "followupseveritycarryoverweight")
                MainWorld::WorldCrisis::FollowupSeverityCarryoverWeight = Value;
            else if (Key == "lightpressureseveritythreshold")
                MainWorld::WorldCrisis::LightPressureSeverityThreshold = Value;
            else if (Key == "heavypressureseveritythreshold")
                MainWorld::WorldCrisis::HeavyPressureSeverityThreshold = Value;
            else
                return false;

            return true;
        }

        if (Section == "mainworldworldcrisissecuritycollapse")
            return ApplyPressureRangeFloatValue(
                MainWorld::WorldCrisis::SecurityCollapse,
                Key,
                Value);

        if (Section == "mainworldworldcrisisresidentialsecuritycollapse")
            return ApplyPressureRangeFloatValue(
                MainWorld::WorldCrisis::ResidentialSecurityCollapse,
                Key,
                Value);

        if (Section == "mainworldworldcrisisfreedomcollapse")
            return ApplyPressureRangeFloatValue(
                MainWorld::WorldCrisis::FreedomCollapse,
                Key,
                Value);

        if (Section == "mainworldworldcrisisresidentialfreedomcollapse")
            return ApplyPressureRangeFloatValue(
                MainWorld::WorldCrisis::ResidentialFreedomCollapse,
                Key,
                Value);

        if (Section == "mainworldworldcrisisjobstress")
            return ApplyPressureRangeFloatValue(
                MainWorld::WorldCrisis::JobStress,
                Key,
                Value);

        if (Section == "mainworldworldcrisisfoodstress")
            return ApplyPressureRangeFloatValue(
                MainWorld::WorldCrisis::FoodStress,
                Key,
                Value);

        if (Section == "mainworldworldcrisishousingstress")
            return ApplyPressureRangeFloatValue(
                MainWorld::WorldCrisis::HousingStress,
                Key,
                Value);

        if (Section == "mainworldworldcrisisresidentialpollutionstress")
            return ApplyPressureRangeFloatValue(
                MainWorld::WorldCrisis::ResidentialPollutionStress,
                Key,
                Value);

        if (Section == "mainworldworldcrisisbudgetdeficit")
            return ApplyPressureRangeFloatValue(
                MainWorld::WorldCrisis::BudgetDeficit,
                Key,
                Value);

        if (Section == "mainworldworldcrisisnetlosspressure")
            return ApplyPressureRangeFloatValue(
                MainWorld::WorldCrisis::NetLossPressure,
                Key,
                Value);

        if (Section == "mainworldworldcrisistaxcollectionbreakdown")
            return ApplyPressureRangeFloatValue(
                MainWorld::WorldCrisis::TaxCollectionBreakdown,
                Key,
                Value);

        if (Section == "mainworldworldcrisisraidimmediatebudget")
            return ApplyImmediateBudgetFloatValue(
                MainWorld::WorldCrisis::RaidImmediateBudget,
                Key,
                Value);

        if (Section == "mainworldworldcrisislaborstrikeimmediatebudget")
            return ApplyImmediateBudgetFloatValue(
                MainWorld::WorldCrisis::LaborStrikeImmediateBudget,
                Key,
                Value);

        if (Section == "mainworldworldcrisiscrimewaveimmediatebudget")
            return ApplyImmediateBudgetFloatValue(
                MainWorld::WorldCrisis::CrimeWaveImmediateBudget,
                Key,
                Value);

        if (Section == "mainworldworldcrisisfiscalemergencyimmediatebudget")
            return ApplyImmediateBudgetFloatValue(
                MainWorld::WorldCrisis::FiscalEmergencyImmediateBudget,
                Key,
                Value);

        if (Section == "mainworldworldcrisisraidrisk")
            return ApplyRaidRiskFloatValue(
                MainWorld::WorldCrisis::RaidRisk,
                Key,
                Value);

        if (Section == "mainworldworldcrisislaborstrikerisk")
            return ApplyLaborStrikeRiskFloatValue(
                MainWorld::WorldCrisis::LaborStrikeRisk,
                Key,
                Value);

        if (Section == "mainworldworldcrisiscrimewaverisk")
            return ApplyCrimeWaveRiskFloatValue(
                MainWorld::WorldCrisis::CrimeWaveRisk,
                Key,
                Value);

        if (Section == "mainworldworldcrisisfiscalemergencyrisk")
            return ApplyFiscalEmergencyRiskFloatValue(
                MainWorld::WorldCrisis::FiscalEmergencyRisk,
                Key,
                Value);

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

        if (Section == "mainworldtrade")
        {
            if (Key == "maxactivetraderoutecount")
                MainWorld::Trade::MaxActiveTradeRouteCount = Value;
            else if (Key == "maxcompletedtraderouterecordcount")
                MainWorld::Trade::MaxCompletedTradeRouteRecordCount = Value;
            else if (Key == "minamountunits")
                MainWorld::Trade::MinAmountUnits = Value;
            else if (Key == "maxamountunits")
                MainWorld::Trade::MaxAmountUnits = Value;
            else if (Key == "mindailytransferunits")
                MainWorld::Trade::MinDailyTransferUnits = Value;
            else if (Key == "maxdailytransferunits")
                MainWorld::Trade::MaxDailyTransferUnits = Value;
            else if (Key == "defaultdurationdays")
                MainWorld::Trade::DefaultDurationDays = Value;
            else
                return false;

            return true;
        }

        if (Section == "mainworldtradediplomacy")
        {
            if (Key == "standingidledecayintervaldays")
                MainWorld::TradeDiplomacy::StandingIdleDecayIntervalDays = Value;
            else if (Key == "relationidledecayintervaldays")
                MainWorld::TradeDiplomacy::RelationIdleDecayIntervalDays = Value;
            else
                return false;

            return true;
        }

        if (Section == "mainworldpoliticaldemand")
        {
            if (Key == "maxactivefactiondemandcount")
                MainWorld::PoliticalDemand::MaxActiveFactionDemandCount = Value;
            else if (Key == "maxactiveforeigndemandcount")
                MainWorld::PoliticalDemand::MaxActiveForeignDemandCount = Value;
            else if (Key == "factiondurationdays")
                MainWorld::PoliticalDemand::FactionDurationDays = Value;
            else if (Key == "foreigndurationdays")
                MainWorld::PoliticalDemand::ForeignDurationDays = Value;
            else if (Key == "factioncooldowndays")
                MainWorld::PoliticalDemand::FactionCooldownDays = Value;
            else if (Key == "foreigncooldowndays")
                MainWorld::PoliticalDemand::ForeignCooldownDays = Value;
            else if (Key == "factionmodifierdurationdays")
                MainWorld::PoliticalDemand::FactionModifierDurationDays = Value;
            else if (Key == "noticedurationdays")
                MainWorld::PoliticalDemand::NoticeDurationDays = Value;
            else if (Key == "campaignpromiseleaddays")
                MainWorld::PoliticalDemand::CampaignPromiseLeadDays = Value;
            else if (Key == "factionmembermincount")
                MainWorld::PoliticalDemand::FactionMemberMinCount = Value;
            else
                return false;

            return true;
        }

        if (Section == "mainworldfactioncommunists")
        {
            return ApplyHousingDemandIntValue(
                MainWorld::PoliticalDemand::Communists,
                Key,
                Value);
        }

        if (Section == "mainworldfactioncapitalists")
        {
            return ApplyTaxCeilingDemandIntValue(
                MainWorld::PoliticalDemand::Capitalists,
                Key,
                Value);
        }

        if (Section == "mainworldfactionreligious")
        {
            return ApplyScoreDemandIntValue(
                MainWorld::PoliticalDemand::Religious,
                Key,
                Value);
        }

        if (Section == "mainworldfactionmilitarists")
        {
            return ApplyScoreDemandIntValue(
                MainWorld::PoliticalDemand::Militarists,
                Key,
                Value);
        }

        if (Section == "mainworldfactionenvironmentalists")
        {
            return ApplyScoreDemandIntValue(
                MainWorld::PoliticalDemand::Environmentalists,
                Key,
                Value);
        }

        if (Section == "mainworldfactionindustrialists")
        {
            return ApplyExportIncomeDemandIntValue(
                MainWorld::PoliticalDemand::Industrialists,
                Key,
                Value);
        }

        if (Section == "mainworldfactionintellectuals")
        {
            return ApplyScoreDemandIntValue(
                MainWorld::PoliticalDemand::Intellectuals,
                Key,
                Value);
        }

        if (Section == "mainworldfactionconservatives")
        {
            return ApplyTaxCeilingDemandIntValue(
                MainWorld::PoliticalDemand::Conservatives,
                Key,
                Value);
        }

        if (Section == "mainworldforeignglobal")
        {
            return ApplyForeignGlobalIntValue(
                MainWorld::PoliticalDemand::ForeignGlobal,
                Key,
                Value);
        }

        if (Section == "mainworldforeignchina")
        {
            return ApplyForeignTradeRouteDemandIntValue(
                MainWorld::PoliticalDemand::China,
                Key,
                Value);
        }

        if (Section == "mainworldforeignrussia")
        {
            return ApplyForeignScoreDemandIntValue(
                MainWorld::PoliticalDemand::Russia,
                Key,
                Value);
        }

        if (Section == "mainworldforeignunitedstates")
        {
            return ApplyForeignScoreDemandIntValue(
                MainWorld::PoliticalDemand::UnitedStates,
                Key,
                Value);
        }

        if (Section == "mainworldforeignmiddleeast")
        {
            return ApplyForeignScoreDemandIntValue(
                MainWorld::PoliticalDemand::MiddleEast,
                Key,
                Value);
        }

        if (Section == "mainworldforeigneuropeanunion")
        {
            return ApplyForeignScoreDemandIntValue(
                MainWorld::PoliticalDemand::EuropeanUnion,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisehousing")
        {
            return ApplyHousingPromiseIntValue(
                MainWorld::ElectionPromise::Housing,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisefood")
        {
            return ApplyScorePromiseIntValue(
                MainWorld::ElectionPromise::Food,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisehealth")
        {
            return ApplyScorePromiseIntValue(
                MainWorld::ElectionPromise::Health,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisejob")
        {
            return ApplyJobPromiseIntValue(
                MainWorld::ElectionPromise::Job,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisefreedom")
        {
            return ApplyScorePromiseIntValue(
                MainWorld::ElectionPromise::Freedom,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisesecurity")
        {
            return ApplyScorePromiseIntValue(
                MainWorld::ElectionPromise::Security,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromisefaith")
        {
            return ApplyScorePromiseIntValue(
                MainWorld::ElectionPromise::Faith,
                Key,
                Value);
        }

        if (Section == "mainworldelectionpromiseexportincome")
        {
            return ApplyExportIncomePromiseIntValue(
                MainWorld::ElectionPromise::ExportIncome,
                Key,
                Value);
        }

        if (Section == "mainworldworldcrisis")
        {
            if (Key == "raiddurationdays")
                MainWorld::WorldCrisis::RaidDurationDays = Value;
            else if (Key == "laborstrikedurationdays")
                MainWorld::WorldCrisis::LaborStrikeDurationDays = Value;
            else if (Key == "crimewavedurationdays")
                MainWorld::WorldCrisis::CrimeWaveDurationDays = Value;
            else if (Key == "fiscalemergencydurationdays")
                MainWorld::WorldCrisis::FiscalEmergencyDurationDays = Value;
            else if (Key == "successcooldowndays")
                MainWorld::WorldCrisis::SuccessCooldownDays = Value;
            else if (Key == "failurecooldowndays")
                MainWorld::WorldCrisis::FailureCooldownDays = Value;
            else if (Key == "startnotificationdays")
                MainWorld::WorldCrisis::StartNotificationDays = Value;
            else if (Key == "resolvednotificationdays")
                MainWorld::WorldCrisis::ResolvedNotificationDays = Value;
            else
                return false;

            return true;
        }

        if (Section == "mainworldworldcrisisraidimmediatebudget")
            return ApplyImmediateBudgetIntValue(
                MainWorld::WorldCrisis::RaidImmediateBudget,
                Key,
                Value);

        if (Section == "mainworldworldcrisislaborstrikeimmediatebudget")
            return ApplyImmediateBudgetIntValue(
                MainWorld::WorldCrisis::LaborStrikeImmediateBudget,
                Key,
                Value);

        if (Section == "mainworldworldcrisiscrimewaveimmediatebudget")
            return ApplyImmediateBudgetIntValue(
                MainWorld::WorldCrisis::CrimeWaveImmediateBudget,
                Key,
                Value);

        if (Section == "mainworldworldcrisisfiscalemergencyimmediatebudget")
            return ApplyImmediateBudgetIntValue(
                MainWorld::WorldCrisis::FiscalEmergencyImmediateBudget,
                Key,
                Value);

        if (Section == "mainworldworldcrisisfiscalemergencyrisk")
            return ApplyFiscalEmergencyRiskIntValue(
                MainWorld::WorldCrisis::FiscalEmergencyRisk,
                Key,
                Value);

        if (Section == "mainworldworldcrisisraidpressuretransfer")
            return ApplyPressureTransferIntValue(
                MainWorld::WorldCrisis::RaidPressureTransfer,
                Key,
                Value);

        if (Section == "mainworldworldcrisislaborstrikepressuretransfer")
            return ApplyPressureTransferIntValue(
                MainWorld::WorldCrisis::LaborStrikePressureTransfer,
                Key,
                Value);

        if (Section == "mainworldworldcrisiscrimewavepressuretransfer")
            return ApplyPressureTransferIntValue(
                MainWorld::WorldCrisis::CrimeWavePressureTransfer,
                Key,
                Value);

        if (Section == "mainworldworldcrisisfiscalemergencypressuretransfer")
            return ApplyPressureTransferIntValue(
                MainWorld::WorldCrisis::FiscalEmergencyPressureTransfer,
                Key,
                Value);

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
