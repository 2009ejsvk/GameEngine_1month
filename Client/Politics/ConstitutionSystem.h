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
}
