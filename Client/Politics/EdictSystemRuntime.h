#pragma once

#include "EdictSystem.h"
#include <vector>

namespace EdictSystemRuntime
{
    void InitializeGovernmentEdictStates(
        const std::vector<FGovernmentEdictDefinition>& Definitions,
        std::vector<FGovernmentEdictState>& OutStates);

    void SynchronizeGovernmentEdictStates(
        const std::vector<FGovernmentEdictDefinition>& Definitions,
        std::vector<FGovernmentEdictState>& InOutStates);

    long long ResolveEdictActivationCost(
        const FGovernmentEdictDefinition& Definition,
        int ActiveCitizenCount);

    long long CalculateEdictDailyUpkeep(
        const std::vector<FGovernmentEdictDefinition>& Definitions,
        const std::vector<FGovernmentEdictState>& States,
        int DaysInMonth);

    FGovernmentEdictModifiers CalculateEdictModifiers(
        const std::vector<FGovernmentEdictDefinition>& Definitions,
        const std::vector<FGovernmentEdictState>& States,
        int ActiveCitizenCount);

    FGovernmentActionRecord MakeGovernmentActionFromEdict(
        const FGovernmentEdictDefinition& Definition);
}
