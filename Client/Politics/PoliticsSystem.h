#pragma once

#include "ConstitutionTypes.h"
#include "PoliticalTypes.h"

class CBuildingMarkerOrb;
class CWorld;

namespace PoliticsSystem
{
    void SetDefaultGovernmentProfile(FGovernmentProfile& OutProfile);
    void TickGovernmentActions(FGovernmentProfile& InOutProfile, int DaysElapsed = 1);
    void SyncGovernmentActionFromEdict(
        FGovernmentProfile& InOutProfile,
        EGovernmentEdictType Type,
        bool Active);

    void ApplyDailyEdictCitizenEffects(
        CWorld* World,
        const FGovernmentEdictModifiers& Modifiers);

    FCitizenPoliticalEvaluation EvaluateCitizen(
        CWorld* World,
        const CBuildingMarkerOrb& Citizen,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus* TaxEventStatus = nullptr,
        const FConstitutionOptionEffect* ConstitutionEffects = nullptr);

    FPoliticalWorldSnapshot EvaluateWorld(
        CWorld* World,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus* TaxEventStatus = nullptr,
        const FConstitutionOptionEffect* ConstitutionEffects = nullptr);

    void InitializeElectionSchedule(
        FElectionStatus& OutStatus,
        int CurrentYear,
        int InitialLeadYears,
        int ElectionMonth,
        int ElectionDay);

    void ResolveScheduledElection(
        FElectionStatus& InOutStatus,
        const FPoliticalWorldSnapshot& Snapshot,
        int CurrentYear,
        int CurrentMonth,
        int CurrentDay,
        int ElectionIntervalYears,
        int ElectionMonth,
        int ElectionDay);

    int GetDaysUntilNextElection(
        const FElectionStatus& ElectionStatus,
        int CurrentYear,
        int CurrentMonth,
        int CurrentDay);

    double GetElectionWarningScore(
        const FElectionStatus& ElectionStatus,
        const FPoliticalWorldSnapshot& Snapshot,
        const FTaxPolicyEventStatus& TaxEventStatus,
        int CurrentYear,
        int CurrentMonth,
        int CurrentDay);

    const wchar_t* GetVoteIntentDisplayName(EVoteIntent VoteIntent);
}
