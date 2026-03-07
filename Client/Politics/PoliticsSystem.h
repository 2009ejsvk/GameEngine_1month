#pragma once

#include "PoliticalTypes.h"

class CBuildingMarkerOrb;
class CWorld;

namespace PoliticsSystem
{
    void SetDefaultGovernmentProfile(FGovernmentProfile& OutProfile);
    void TickGovernmentActions(FGovernmentProfile& InOutProfile, int DaysElapsed = 1);

    FCitizenPoliticalEvaluation EvaluateCitizen(
        CWorld* World,
        const CBuildingMarkerOrb& Citizen,
        const FGovernmentProfile& GovernmentProfile);

    FPoliticalWorldSnapshot EvaluateWorld(
        CWorld* World,
        const FGovernmentProfile& GovernmentProfile);

    const wchar_t* GetVoteIntentDisplayName(EVoteIntent VoteIntent);
}
