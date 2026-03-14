#pragma once

namespace GameConstants
{
    void RegisterRuntimeConfig();
    bool ReloadIfChanged(float DeltaTime);
    unsigned long long GetRuntimeConfigGeneration();

    namespace Citizen
    {
        extern float NpcBaseMoveSpeed;
        extern float NpcMoveSpeedVariance;
        extern float CommuteWalkingRouteFactor;
        extern float CommuteRoadRouteFactor;
        extern float CommuteTransitSpeedMultiplier;
        extern float CommuteVehicleSpeedMultiplier;
        extern float CommuteTransitWaitSeconds;
        extern float CommuteRoadConnectorTiles;
        extern float CommuteRoadAccessThreshold;
        extern float CommutePenaltyGraceSeconds;
        extern float CommutePenaltyMaxSeconds;
        extern float CommuteJobPenaltyWeight;
        extern float CommuteBusStopSearchTiles;
        extern float CommuteBusStopRoadLinkTiles;
        extern float CommuteBusAverageWaitFactor;
        extern float BusDispatchSeconds;
        extern int BusRouteCapacity;
    }

    namespace Politics
    {
        extern float CitizenPoliticalShiftIntervalSeconds;
    }

    namespace Economy
    {
        extern int HarborImportTargetStockPerConsumer;
        extern int HarborImportMaxPerResourcePerShip;
        extern float ProductionInputBufferSeconds;
        extern float ProductionMaxBufferedUnits;
        extern double DailyConsumptionSpendBase;
        extern double DailyWorkerIncomeBase;
        extern double DailyResidenceValueBase;
    }

    namespace Orb
    {
        extern float AtWorkDurationSeconds;
        extern float AtHomeDurationSeconds;
        extern float AtFoodDurationSeconds;
        extern float AtFunDurationSeconds;
        extern float AtHealthDurationSeconds;
        extern float AtFaithDurationSeconds;
        extern float FoodInterruptThreshold;
        extern float FunInterruptThreshold;
        extern float HealthInterruptThreshold;
        extern float FaithInterruptThreshold;
        extern float HealthRemovalThreshold;
        extern int ServiceStockPerCapacity;
        extern float ServiceStockRegenPerCapacityPerSecond;
        extern float TeamsterSpeedMultiplier;
        extern float TeamsterCoverageRadiusTiles;
        extern int TeamsterTransferUnit;
        extern int TeamsterConsumerRestockThreshold;
        extern int TeamsterConsumerTargetStock;
        extern float PoliticalShiftIntervalSeconds;
    }

    namespace MainWorld
    {
        namespace Trade
        {
            extern int MaxActiveTradeRouteCount;
            extern int MaxCompletedTradeRouteRecordCount;
            extern int MinAmountUnits;
            extern int MaxAmountUnits;
            extern int MinDailyTransferUnits;
            extern int MaxDailyTransferUnits;
            extern int DefaultDurationDays;
        }

        namespace PoliticalDemand
        {
            struct FScoreDemandTuning
            {
                int IgnoreAtOrAboveValue;
                int TargetMin;
                int TargetMax;
                int TargetDelta;
                int RewardBudgetDelta;
                int RewardApprovalDelta;
                int PenaltyApprovalDelta;
                float ApprovalPriorityWeight;
                float DeficitPriorityWeight;
            };

            struct FHousingDemandTuning
            {
                int IgnoreAtOrAboveHousing;
                int IgnoreAtOrBelowHomeless;
                int TargetMin;
                int TargetMax;
                int TargetDelta;
                int RewardBudgetBase;
                int RewardBudgetPerMember;
                int RewardApprovalDelta;
                int PenaltyApprovalDelta;
                float ApprovalPriorityWeight;
                float HousingDeficitPriorityWeight;
                float HomelessPriorityWeight;
            };

            struct FTaxCeilingDemandTuning
            {
                int IgnoreAtOrBelowValue;
                int TargetValue;
                int RewardBudgetDelta;
                int RewardApprovalDelta;
                int PenaltyApprovalDelta;
                float ApprovalPriorityWeight;
                float ExcessPriorityWeight;
            };

            struct FExportIncomeDemandTuning
            {
                int IgnoreAtOrAboveValue;
                int TargetMin;
                int TargetDelta;
                int RewardBudgetDelta;
                int RewardApprovalDelta;
                int PenaltyApprovalDelta;
                float ApprovalPriorityWeight;
                float DeficitDivisor;
            };

            struct FForeignGlobalTuning
            {
                int RelationPressureStart;
                int StandingPressureStart;
                float RelationPriorityWeight;
                float StandingPriorityWeight;
                int RewardBudgetDelta;
                int RewardRelationDelta;
                int RewardStandingDelta;
                int PenaltyRelationDelta;
                int PenaltyStandingDelta;
            };

            struct FForeignTradeRouteDemandTuning
            {
                int IgnoreAtOrAboveActiveRoutes;
                int TargetValue;
                float PriorityWeight;
            };

            struct FForeignScoreDemandTuning
            {
                int IgnoreAtOrAboveValue;
                int TargetValue;
                float PriorityWeight;
            };

            extern int MaxActiveFactionDemandCount;
            extern int MaxActiveForeignDemandCount;
            extern int FactionDurationDays;
            extern int ForeignDurationDays;
            extern int FactionCooldownDays;
            extern int ForeignCooldownDays;
            extern int FactionModifierDurationDays;
            extern int NoticeDurationDays;
            extern int CampaignPromiseLeadDays;
            extern int FactionMemberMinCount;
            extern float FactionApprovalThreshold;

            extern FHousingDemandTuning Communists;
            extern FTaxCeilingDemandTuning Capitalists;
            extern FScoreDemandTuning Religious;
            extern FScoreDemandTuning Militarists;
            extern FScoreDemandTuning Environmentalists;
            extern FExportIncomeDemandTuning Industrialists;
            extern FScoreDemandTuning Intellectuals;
            extern FTaxCeilingDemandTuning Conservatives;

            extern FForeignGlobalTuning ForeignGlobal;
            extern FForeignTradeRouteDemandTuning China;
            extern FForeignScoreDemandTuning Russia;
            extern FForeignScoreDemandTuning UnitedStates;
            extern FForeignScoreDemandTuning MiddleEast;
            extern FForeignScoreDemandTuning EuropeanUnion;
        }

        namespace ElectionPromise
        {
            struct FScorePromiseTuning
            {
                int TargetDelta;
                int SuccessVoteModifierPercent;
                int FailureVoteModifierPercent;
                float PriorityThresholdValue;
                float PriorityDeficitWeight;
            };

            struct FHousingPromiseTuning
            {
                int TargetDelta;
                int SuccessVoteModifierPercent;
                int FailureVoteModifierPercent;
                float PriorityThresholdValue;
                float PriorityDeficitWeight;
                float HomelessHouseholdPriorityWeight;
            };

            struct FJobPromiseTuning
            {
                int TargetDelta;
                int SuccessVoteModifierPercent;
                int FailureVoteModifierPercent;
                float PriorityThresholdValue;
                float PriorityDeficitWeight;
                float UnemploymentRatioPriorityWeight;
            };

            struct FExportIncomePromiseTuning
            {
                int TargetMinDelta;
                int TargetScaleDivisor;
                int SuccessVoteModifierPercent;
                int FailureVoteModifierPercent;
                int PriorityReferenceValue;
                float PriorityDivisor;
            };

            extern FHousingPromiseTuning Housing;
            extern FScorePromiseTuning Food;
            extern FScorePromiseTuning Health;
            extern FJobPromiseTuning Job;
            extern FScorePromiseTuning Freedom;
            extern FScorePromiseTuning Security;
            extern FScorePromiseTuning Faith;
            extern FExportIncomePromiseTuning ExportIncome;
        }

        namespace TradeDiplomacy
        {
            extern int StandingIdleDecayIntervalDays;
            extern int RelationIdleDecayIntervalDays;
        }

        namespace WorldCrisis
        {
            struct FPressureRangeTuning
            {
                float StartValue;
                float FullValue;
            };

            struct FImmediateBudgetTuning
            {
                int BasePenalty;
                float RiskPenaltyScale;
            };

            struct FRaidRiskTuning
            {
                float SecurityCollapseWeight;
                float ResidentialSecurityCollapseWeight;
                float FreedomCollapseWeight;
                float ResidentialFreedomCollapseWeight;
                float OppositionRatioWeight;
                float FoodStressWeight;
                float HousingStressWeight;
                float ResidentialPollutionStressWeight;
                float MartialLawReduction;
            };

            struct FLaborStrikeRiskTuning
            {
                float UnemploymentRatioWeight;
                float JobStressWeight;
                float IncomeTaxPressureWeight;
                float OppositionRatioWeight;
                float WorkerTaxStrikeBonus;
            };

            struct FCrimeWaveRiskTuning
            {
                float SecurityCollapseWeight;
                float ResidentialSecurityCollapseWeight;
                float HomelessRatioWeight;
                float UnemploymentRatioWeight;
                float HousingStressWeight;
                float ResidentialPollutionStressWeight;
                float NetLossPressureWeight;
                float MartialLawReduction;
            };

            struct FFiscalEmergencyRiskTuning
            {
                float BudgetDeficitWeight;
                float NetLossPressureWeight;
                float TaxCollectionBreakdownWeight;
                float ResidentialPollutionStressWeight;
                float BudgetCrisisBonus;
                float LargePopulationBonus;
                int LargePopulationThreshold;
            };

            struct FPressureTransferTuning
            {
                int PrimaryBaseDays;
                int SecondaryBaseDays;
            };

            extern int RaidDurationDays;
            extern int LaborStrikeDurationDays;
            extern int CrimeWaveDurationDays;
            extern int FiscalEmergencyDurationDays;
            extern int SuccessCooldownDays;
            extern int FailureCooldownDays;
            extern int StartNotificationDays;
            extern int ResolvedNotificationDays;

            extern float SeverityDurationDivisorDays;
            extern float FollowupSuccessBaseCarryover;
            extern float FollowupFailureBaseCarryover;
            extern float FollowupSeverityCarryoverWeight;
            extern float LightPressureSeverityThreshold;
            extern float HeavyPressureSeverityThreshold;

            extern FPressureRangeTuning SecurityCollapse;
            extern FPressureRangeTuning ResidentialSecurityCollapse;
            extern FPressureRangeTuning FreedomCollapse;
            extern FPressureRangeTuning ResidentialFreedomCollapse;
            extern FPressureRangeTuning JobStress;
            extern FPressureRangeTuning FoodStress;
            extern FPressureRangeTuning HousingStress;
            extern FPressureRangeTuning ResidentialPollutionStress;
            extern FPressureRangeTuning BudgetDeficit;
            extern FPressureRangeTuning NetLossPressure;
            extern FPressureRangeTuning TaxCollectionBreakdown;

            extern FImmediateBudgetTuning RaidImmediateBudget;
            extern FImmediateBudgetTuning LaborStrikeImmediateBudget;
            extern FImmediateBudgetTuning CrimeWaveImmediateBudget;
            extern FImmediateBudgetTuning FiscalEmergencyImmediateBudget;

            extern FRaidRiskTuning RaidRisk;
            extern FLaborStrikeRiskTuning LaborStrikeRisk;
            extern FCrimeWaveRiskTuning CrimeWaveRisk;
            extern FFiscalEmergencyRiskTuning FiscalEmergencyRisk;

            extern FPressureTransferTuning RaidPressureTransfer;
            extern FPressureTransferTuning LaborStrikePressureTransfer;
            extern FPressureTransferTuning CrimeWavePressureTransfer;
            extern FPressureTransferTuning FiscalEmergencyPressureTransfer;
        }
    }
}
