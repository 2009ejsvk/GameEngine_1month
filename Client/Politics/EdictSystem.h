#pragma once

#include "PoliticalTypes.h"
#include <vector>

struct FGovernmentEdictDefinition
{
    EGovernmentEdictType         Type = EGovernmentEdictType::None;
    EGovernmentEdictMode         Mode = EGovernmentEdictMode::Passive;
    EPoliticalActionType         ActionType = EPoliticalActionType::None;
    std::wstring                 DisplayName;
    std::wstring                 Summary;
    std::wstring                 EffectText;
    const wchar_t*               IconPath = nullptr;
    long long                    BaseCost = 0;
    long long                    CostPerCitizen = 0;
    long long                    MonthlyUpkeep = 0;
    int                          DurationDays = 0;
    int                          CooldownDays = 0;
    std::vector<FPoliticalSignalDef> Signals;
};

namespace EdictSystem
{
    const std::vector<FGovernmentEdictDefinition>&
        GetGovernmentEdictDefinitions();

    const FGovernmentEdictDefinition* FindGovernmentEdictDefinition(
        EGovernmentEdictType Type);

    void InitializeGovernmentEdictStates(
        std::vector<FGovernmentEdictState>& OutStates);

    long long ResolveEdictActivationCost(
        const FGovernmentEdictDefinition& Definition,
        int ActiveCitizenCount);

    long long CalculateEdictDailyUpkeep(
        const std::vector<FGovernmentEdictState>& States,
        int DaysInMonth);

    FGovernmentEdictModifiers CalculateEdictModifiers(
        const std::vector<FGovernmentEdictState>& States,
        int ActiveCitizenCount);

    FGovernmentActionRecord MakeGovernmentActionFromEdict(
        const FGovernmentEdictDefinition& Definition);
}
