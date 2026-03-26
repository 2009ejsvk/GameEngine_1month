#pragma once

#include "World/WorldSubsystem.h"
#include "ScenarioRunner.h"
#include "../Building/BuildingTypes.h"

class CScenarioSubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    void Reset();
    void InitializeResultTracking();
    void TickPhase();
    void ShowResultWidget(bool Victory);
    void NotifyEraTransitioned(EBuildingEra NewEra);

    EScenarioPhase Phase = EScenarioPhase::Intro;
    bool EraTransitionUnlocked = false;
    bool CrownRumExploitationActive = false;

    int TermStartYear = 0;
    int TermStartMonth = 1;
    int TermStartDay = 1;
    int InitialBuildingCount = 0;
    double PeakSupportPercent = 0.0;
    bool ResultShown = false;
    bool ScenarioElectionPromptPending = false;
    bool SmugglersRumOfferInjected = false;
    bool CrownRumOfferInjected = false;

private:
    void EnterPhase(EScenarioPhase NewPhase);
    void OpenSmugglersRumRoute();
    void OpenCrownRumRoute();
    void InjectSmugglersOfferDemand();
    void InjectPenultimoFarmDemand();
    void InjectSmugglersRumSaleDemand();
    void InjectCrownExploitationDemand();
    void InjectIndependencePrepDemand();
    void InjectPeacePaymentDemand();

public:
    bool TryExecutePeacePayment(std::wstring& OutMessage);
    void DebugSkipPhase();

private:
    void ShowEventWidget(
        int IssuerIndex,
        const std::wstring& Title,
        const std::wstring& Body,
        const std::wstring& AcceptConsequence,
        const std::wstring& RejectConsequence);
};
