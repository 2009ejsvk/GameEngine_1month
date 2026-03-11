#pragma once

namespace GameConstants
{
    namespace Citizen
    {
        constexpr float NpcBaseMoveSpeed = 140.f;
        constexpr float NpcMoveSpeedVariance = 21.f;
        constexpr float CommuteWalkingRouteFactor = 1.22f;
        constexpr float CommuteRoadRouteFactor = 1.08f;
        constexpr float CommuteTransitSpeedMultiplier = 1.8f;
        constexpr float CommuteVehicleSpeedMultiplier = 3.0f;
        constexpr float CommuteTransitWaitSeconds = 18.f;
        constexpr float CommuteRoadConnectorTiles = 1.25f;
        constexpr float CommuteRoadAccessThreshold = 0.15f;
        constexpr float CommutePenaltyGraceSeconds = 18.f;
        constexpr float CommutePenaltyMaxSeconds = 90.f;
        constexpr float CommuteJobPenaltyWeight = 0.55f;
        constexpr float CommuteBusStopSearchTiles = 6.f;
        constexpr float CommuteBusStopRoadLinkTiles = 1.75f;
        constexpr float CommuteBusAverageWaitFactor = 0.5f;
        constexpr float BusDispatchSeconds = 24.f;
        constexpr int BusRouteCapacity = 20;
    }

    namespace Politics
    {
        constexpr float CitizenPoliticalShiftIntervalSeconds = 12.f;
    }

    namespace Economy
    {
        constexpr int ExportPricePerStockUnit = 2;
        constexpr int ImportPricePerStockUnit = 3;
        constexpr int HarborImportTargetStockPerConsumer = 1000;
        constexpr int HarborImportMaxPerResourcePerShip = 1500;
        constexpr double DailyConsumptionSpendBase = 30.0;
        constexpr double DailyWorkerIncomeBase = 66.6666666667;
        constexpr double DailyResidenceValueBase = 11.4285714286;
    }

    namespace Orb
    {
        constexpr float AtWorkDurationSeconds = 15.f;
        constexpr float AtHomeDurationSeconds = 10.f;
        constexpr float AtFoodDurationSeconds = 4.f;
        constexpr float AtFunDurationSeconds = 6.f;
        constexpr float FoodInterruptThreshold = 25.f;
        constexpr float FunInterruptThreshold = 30.f;
        constexpr float HealthRemovalThreshold = 5.f;
        constexpr float TeamsterSpeedMultiplier = 3.f;
        constexpr float TeamsterCoverageRadiusTiles = 25.f;
        constexpr int TeamsterTransferUnit = 1000;
        constexpr int TeamsterConsumerRestockThreshold = 250;
        constexpr int TeamsterConsumerTargetStock = 1000;
        constexpr float PoliticalShiftIntervalSeconds =
            Politics::CitizenPoliticalShiftIntervalSeconds;
    }
}
