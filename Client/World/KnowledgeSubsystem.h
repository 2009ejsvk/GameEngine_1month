#pragma once

#include "World/WorldSubsystem.h"
#include "KnowledgeSystem.h"
#include "../Politics/ConstitutionTypes.h"
#include <string>

class CKnowledgeSubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    void Reset();
    void RefreshKnowledgeGeneration();
    void ApplyDailyKnowledgeGain();
    bool TryUnlockResearch(
        const std::wstring& Key,
        int Cost);
    bool TrySelectConstitutionOption(EConstitutionOptionId Id);

    FKnowledgeState KnowledgeState;
    FConstitutionState ConstitutionState;
};
