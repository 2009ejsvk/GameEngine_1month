#pragma once

#include "PoliticalTypes.h"
#include <vector>

namespace EdictBudgetRuntime
{
    long long CalculateDailyUpkeep(
        const std::vector<FGovernmentEdictState>& States,
        int DaysInMonth);
}
