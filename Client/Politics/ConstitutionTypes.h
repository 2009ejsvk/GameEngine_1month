#pragma once

#include "../Building/BuildingTypes.h"
#include "PoliticalTypes.h"
#include <array>
#include <cstddef>
#include <string>

enum class EConstitutionTopic
{
    VotingRights = 0,
    LaborPolicy,
    ReligionAndState,
    MediaIndependence,
    ArmedForces,
    Count
};

enum class EConstitutionOptionId
{
    AllCitizensVote = 0,
    WealthyCitizensVote,
    EarlyRetirement,
    LifesWork,
    Theocracy,
    OfficialSeparation,
    FreeMedia,
    StateControlledMedia,
    ProfessionalSoldiers,
    Militia,
    None
};

constexpr size_t GConstitutionTopicCount =
    static_cast<size_t>(EConstitutionTopic::Count);

struct FConstitutionOptionEffect
{
    std::array<int, GPoliticalFactionCount> FactionApprovalDeltas = {};
    float ImmigrationRateMultiplier = 1.f;
    int LibertyModifier = 0;
    int SecurityModifier = 0;
    int JobQualityModifier = 0;
};

struct FConstitutionOptionDef
{
    EConstitutionOptionId Id = EConstitutionOptionId::None;
    EConstitutionTopic Topic = EConstitutionTopic::VotingRights;
    EBuildingEra UnlockEra = EBuildingEra::WorldWars;
    std::wstring DisplayName;
    std::wstring EffectSummary;
    FConstitutionOptionEffect Effect;
};

inline std::array<EConstitutionOptionId, GConstitutionTopicCount>
MakeDefaultConstitutionSelections()
{
    std::array<EConstitutionOptionId, GConstitutionTopicCount> Result = {};
    Result.fill(EConstitutionOptionId::None);
    return Result;
}

struct FConstitutionState
{
    std::array<EConstitutionOptionId, GConstitutionTopicCount>
        SelectedOptions = MakeDefaultConstitutionSelections();
    std::array<bool, GConstitutionTopicCount> QueuedTopics = {};
    bool PendingTopicChoice = false;
    EConstitutionTopic PendingTopic = EConstitutionTopic::VotingRights;
    FConstitutionOptionEffect ActiveEffects;
};
