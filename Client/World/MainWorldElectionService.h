#pragma once

#include "../Politics/PoliticalTypes.h"
#include <memory>

class CWorld;

class CMainWorldElectionService
{
public:
    struct FPromiseContext
    {
        std::shared_ptr<CWorld> World;
        int SimulationYear = 0;
        int SimulationMonth = 0;
        int SimulationDay = 0;
        long long LastDailyExportIncome = 0;
    };

public:
    void Reset();
    void InitializeSchedule(
        int SimulationYear,
        int InitialElectionLeadYears,
        int ElectionMonth,
        int ElectionDay);
    void TickPromises(const FPromiseContext& Context);
    void ResolveScheduledElection(
        const FPoliticalWorldSnapshot& PoliticalSnapshot,
        int SimulationYear,
        int SimulationMonth,
        int SimulationDay,
        int ElectionIntervalYears,
        int ElectionMonth,
        int ElectionDay);
    int GetDaysUntilNextElection(
        int SimulationYear,
        int SimulationMonth,
        int SimulationDay) const;
    double GetElectionWarningScore(
        const FPoliticalWorldSnapshot& PoliticalSnapshot,
        const FTaxPolicyEventStatus& TaxEventStatus,
        int SimulationYear,
        int SimulationMonth,
        int SimulationDay) const;

    const FElectionStatus& GetElectionStatus() const
    {
        return mElectionStatus;
    }

    bool IsGameLost() const
    {
        return mElectionStatus.GameLost;
    }

private:
    FElectionStatus mElectionStatus;
};
