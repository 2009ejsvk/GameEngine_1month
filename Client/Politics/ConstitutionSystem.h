#pragma once

#include "ConstitutionTypes.h"
#include <vector>

namespace ConstitutionSystem
{
    const std::vector<FConstitutionOptionDef>& GetConstitutionOptionCatalog();

    void OnEraTransitioned(FConstitutionState& State, EBuildingEra NewEra);

    bool TrySelectConstitutionOption(
        FConstitutionState& State,
        EConstitutionOptionId OptionId);

#ifdef _DEBUG
    struct FDebugValidationStep
    {
        EBuildingEra TriggerEra = EBuildingEra::WorldWars;
        EConstitutionTopic Topic = EConstitutionTopic::VotingRights;
        EConstitutionOptionId LeftOptionId = EConstitutionOptionId::None;
        EConstitutionOptionId RightOptionId = EConstitutionOptionId::None;
        bool ExpectPendingAfterSelection = false;
        EConstitutionTopic ExpectedNextPendingTopic =
            EConstitutionTopic::VotingRights;
    };

    bool BuildDebugRightOptionValidationSteps(
        std::vector<FDebugValidationStep>& OutSteps);
#endif
}
