#include "SimulationSubsystem.h"
#include "MainWorld.h"
#include "MainWorldConfig.h"
#include <algorithm>

namespace
{
    float ClampToUnitRange(float Value)
    {
        if (Value < 0.f)
            return 0.f;

        if (Value > 1.f)
            return 1.f;

        return Value;
    }
}

void CSimulationSubsystem::Reset()
{
    Year = MainWorldConfig::GSimulationStartYear;
    Month = MainWorldConfig::GSimulationStartMonth;
    Day = MainWorldConfig::GSimulationStartDay;
    DayProgressAccum = 0.f;
    SecondsPerSimulationDay = MainWorldConfig::GSecondsPerSimulationDay;
    Paused = false;
    SpeedMultiplier = 1;
    PoliticalSnapshotAccum = 0.f;
}

void CSimulationSubsystem::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.f || SecondsPerSimulationDay <= 0.f || !mOwner)
        return;

    DayProgressAccum += DeltaTime;

    while (DayProgressAccum >= SecondsPerSimulationDay)
    {
        DayProgressAccum -= SecondsPerSimulationDay;
        mOwner->AdvanceSimulationDay();
    }
}

void CSimulationSubsystem::TogglePaused()
{
    Paused = !Paused;
}

void CSimulationSubsystem::CycleSpeedMultiplier()
{
    if (SpeedMultiplier >= 4)
    {
        SpeedMultiplier = 1;
        return;
    }

    if (SpeedMultiplier >= 2)
    {
        SpeedMultiplier = 4;
        return;
    }

    SpeedMultiplier = 2;
}

int CSimulationSubsystem::GetDaysInMonth(int YearValue, int MonthValue) const
{
    switch (MonthValue)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return 31;
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    case 2:
    {
        const bool IsLeapYear =
            (YearValue % 400 == 0) || (YearValue % 4 == 0 && YearValue % 100 != 0);
        return IsLeapYear ? 29 : 28;
    }
    default:
        return 30;
    }
}

float CSimulationSubsystem::GetDayProgress() const
{
    if (SecondsPerSimulationDay <= 0.f)
        return 0.f;

    return ClampToUnitRange(
        DayProgressAccum / SecondsPerSimulationDay);
}

float CSimulationSubsystem::GetMonthProgress() const
{
    const int MonthDays = GetDaysInMonth(Year, Month);

    if (MonthDays <= 0)
        return 0.f;

    const float CompletedDays =
        static_cast<float>((std::max)(0, Day - 1)) +
        GetDayProgress();

    return ClampToUnitRange(
        CompletedDays / static_cast<float>(MonthDays));
}
