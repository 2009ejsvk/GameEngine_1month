#pragma once

#include "World/WorldSubsystem.h"
#include "ScenarioRunner.h"

class CScenarioSubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    void Reset();
    void InitializeResultTracking();
    void ApplyScenarioResult(const FScenarioEvent& ScenarioEvent);
    void ShowResultWidget(bool Victory);

    CScenarioRunner Runner;
    int TermStartYear = 0;
    int TermStartMonth = 1;
    int TermStartDay = 1;
    int InitialBuildingCount = 0;
    double PeakSupportPercent = 0.0;
    bool ResultShown = false;
    bool ScenarioElectionPromptPending = false;
};
