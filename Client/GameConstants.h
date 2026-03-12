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
}
