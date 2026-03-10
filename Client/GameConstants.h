#pragma once

namespace GameConstants
{
    namespace Citizen
    {
        constexpr float NpcBaseMoveSpeed = 140.f;
        constexpr float NpcMoveSpeedVariance = 21.f;
    }

    namespace Politics
    {
        constexpr float CitizenPoliticalShiftIntervalSeconds = 12.f;
    }

    namespace Economy
    {
        constexpr int ExportPricePerStockUnit = 2;
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
        constexpr float TeamsterSpeedMultiplier = 5.f;
        constexpr int TeamsterTransferUnit = 1000;
        constexpr float PoliticalShiftIntervalSeconds =
            Politics::CitizenPoliticalShiftIntervalSeconds;
    }
}
