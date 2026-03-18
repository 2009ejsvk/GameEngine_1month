#pragma once

#include "World/WorldSubsystem.h"

class CSimulationSubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    void Reset();
    void Tick(float DeltaTime);
    void TogglePaused();
    void CycleSpeedMultiplier();
    int GetDaysInMonth(int Year, int Month) const;
    float GetDayProgress() const;
    float GetMonthProgress() const;

    int Year = 2000;
    int Month = 1;
    int Day = 1;
    float DayProgressAccum = 0.f;
    float SecondsPerSimulationDay = 2.f;
    bool Paused = false;
    int SpeedMultiplier = 1;
    float PoliticalSnapshotAccum = 0.f;
};
