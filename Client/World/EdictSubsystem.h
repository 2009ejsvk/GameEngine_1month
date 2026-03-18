#pragma once

#include "World/WorldSubsystem.h"
#include "../Politics/PoliticalTypes.h"
#include <string>
#include <vector>

class CEdictSubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    void Reset();
    void TickGovernmentEdicts();
    void RefreshEdictModifiers();
    void ApplyDailyEdictCitizenEffects();
    bool TryApplyEdict(
        EGovernmentEdictType Type,
        std::wstring& OutMessage);

    std::vector<FGovernmentEdictState> GovernmentEdicts;
    FGovernmentEdictModifiers EdictModifiers;
};
