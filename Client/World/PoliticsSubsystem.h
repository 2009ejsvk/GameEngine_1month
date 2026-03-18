#pragma once

#include "World/WorldSubsystem.h"
#include "MainWorldElectionService.h"
#include "MainWorldPoliticalDemandService.h"
#include "../Politics/ConstitutionTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <memory>
#include <string>

class CPoliticsSubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    struct FRefreshSnapshotContext
    {
        const FTaxPolicyEventStatus* TaxEventStatus = nullptr;
        const FConstitutionOptionEffect* ConstitutionEffects = nullptr;
    };

    void Reset();
    void RefreshSnapshot(const FRefreshSnapshotContext& Context);
    void InitializeElectionSchedule(EBuildingEra CurrentEra, int SimulationYear);
    void TickElectionPromises(
        int SimulationYear,
        int SimulationMonth,
        int SimulationDay,
        long long LastDailyExportIncome);
    void ResolveScheduledElection(
        int SimulationYear,
        int SimulationMonth,
        int SimulationDay);
    int GetDaysUntilNextElection(
        int SimulationYear,
        int SimulationMonth,
        int SimulationDay) const;
    double GetElectionWarningScore(
        const FTaxPolicyEventStatus& TaxEventStatus,
        int SimulationYear,
        int SimulationMonth,
        int SimulationDay) const;
    void TickPoliticalDemands();
    bool RespondPoliticalDemand(
        EPoliticalDemandIssuerType IssuerType,
        int IssuerIndex,
        bool Accept,
        std::wstring& OutMessage);

    FGovernmentProfile GovernmentProfile;
    FPoliticalWorldSnapshot PoliticalSnapshot;
    std::unique_ptr<CMainWorldElectionService> ElectionService;
    std::unique_ptr<CMainWorldPoliticalDemandService> PoliticalDemandService;

private:
    CMainWorldPoliticalDemandService::FContext BuildPoliticalDemandContext();
    void ApplyPoliticalDemandRefreshRequests(
        const CMainWorldPoliticalDemandService::FRefreshRequests&
            RefreshRequests);
    void TriggerFactionRevoltConsequences(EPoliticalFaction Faction);
};
