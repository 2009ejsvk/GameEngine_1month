#include "EdictBudgetRuntime.h"
#include "EdictSystem.h"

namespace EdictBudgetRuntime
{
    long long CalculateDailyUpkeep(
        const std::vector<FGovernmentEdictState>& States,
        int DaysInMonth)
    {
        return EdictSystem::CalculateEdictDailyUpkeep(
            States,
            DaysInMonth);
    }
}
