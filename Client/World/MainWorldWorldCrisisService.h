#pragma once

#include "../Politics/PoliticalTypes.h"
#include "../Building/BuildingTypes.h"
#include <memory>
#include <vector>

class CWorld;

class CMainWorldWorldCrisisService
{
public:
    struct FDailyContext
    {
        std::shared_ptr<CWorld> World;
        long long& NationalBudget;
        long long& LastDailyNetChange;
        long long& LastDailyTaxIncome;
        long long& LastDailyConsumptionTaxIncome;
        long long& LastDailyIncomeTaxIncome;
        long long& LastDailyPropertyTaxIncome;
    };

    struct FTickContext
    {
        std::shared_ptr<CWorld> World;
        const FPoliticalWorldSnapshot& PoliticalSnapshot;
        const std::vector<FGovernmentEdictState>& GovernmentEdicts;
        const FTaxPolicy& TaxPolicy;
        const FTaxPolicyEventStatus& TaxEventStatus;
        int SimulationYear = 0;
        int SimulationMonth = 0;
        int SimulationDay = 0;
        long long& NationalBudget;
        long long& LastDailyNetChange;
        double LastDailyTaxCollectionEfficiency = 0.0;
        EBuildingEra CurrentEra = EBuildingEra::Colonial;
    };

public:
    void Reset();
    void ApplyDailyEffects(const FDailyContext& Context);
    void Tick(const FTickContext& Context);
    void TriggerForcedCrisis(
        EWorldCrisisType Type,
        const FTickContext& Context);
    void TriggerForcedRaid(const FTickContext& Context);

    const FWorldCrisisStatus& GetStatus() const
    {
        return mWorldCrisisStatus;
    }

    bool IsLaborStrikeWarningActive() const
    {
        return mLaborStrikeWarningActive;
    }

    bool IsLaborStrikeImminent() const
    {
        return mLaborStrikeImminent;
    }

    bool ForceEndActiveCrisis(bool Success, EBuildingEra CurrentEra);

private:
    int mRaidPressureDays = 0;
    int mLaborStrikePressureDays = 0;
    int mCrimeWavePressureDays = 0;
    int mFiscalEmergencyPressureDays = 0;
    int mActiveWorldCrisisChainDepth = 0;
    EWorldCrisisType mQueuedWorldCrisisType = EWorldCrisisType::None;
    double mQueuedWorldCrisisRisk = 0.0;
    int mQueuedWorldCrisisDelayDays = 0;
    int mQueuedWorldCrisisChainDepth = 0;
    FWorldCrisisStatus mWorldCrisisStatus;
    bool mLaborStrikeWarningActive = false;
    bool mLaborStrikeImminent = false;
};
