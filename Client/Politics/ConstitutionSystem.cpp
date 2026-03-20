#include "ConstitutionSystem.h"
#include "../UI/UIStrings.h"
#include <algorithm>

namespace
{
    constexpr size_t GetTopicIndex(EConstitutionTopic Topic)
    {
        return static_cast<size_t>(Topic);
    }

    void AddFactionApproval(
        FConstitutionOptionEffect& Effect,
        EPoliticalFaction Faction,
        int Delta)
    {
        const int Index = static_cast<int>(Faction);

        if (Index < 0 || Index >= GPoliticalFactionCount)
            return;

        Effect.FactionApprovalDeltas[static_cast<size_t>(Index)] += Delta;
    }

    FConstitutionOptionDef MakeOption(
        EConstitutionOptionId Id,
        EConstitutionTopic Topic,
        EBuildingEra UnlockEra,
        const wchar_t* DisplayName,
        const wchar_t* EffectSummary,
        FConstitutionOptionEffect Effect)
    {
        FConstitutionOptionDef Option;
        Option.Id = Id;
        Option.Topic = Topic;
        Option.UnlockEra = UnlockEra;
        Option.DisplayName = DisplayName ? DisplayName : L"";
        Option.EffectSummary = EffectSummary ? EffectSummary : L"";
        Option.Effect = Effect;
        return Option;
    }

    std::vector<FConstitutionOptionDef> BuildConstitutionOptionCatalog()
    {
        std::vector<FConstitutionOptionDef> Catalog;
        Catalog.reserve(10);

        {
            FConstitutionOptionEffect Effect;
            Effect.ImmigrationRateMultiplier = 1.08f;
            Effect.LibertyModifier = 10;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 8);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, 6);
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, -5);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, -4);
            Catalog.push_back(MakeOption(
                EConstitutionOptionId::AllCitizensVote,
                EConstitutionTopic::VotingRights,
                EBuildingEra::WorldWars,
                UIStrings::Get(
                    L"constitution.option.all_citizens_vote.name").c_str(),
                UIStrings::Get(
                    L"constitution.option.all_citizens_vote.summary").c_str(),
                Effect));
        }

        {
            FConstitutionOptionEffect Effect;
            Effect.ImmigrationRateMultiplier = 0.96f;
            Effect.LibertyModifier = -6;
            Effect.SecurityModifier = 2;
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 8);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, 6);
            AddFactionApproval(Effect, EPoliticalFaction::Communists, -7);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, -4);
            Catalog.push_back(MakeOption(
                EConstitutionOptionId::WealthyCitizensVote,
                EConstitutionTopic::VotingRights,
                EBuildingEra::WorldWars,
                UIStrings::Get(
                    L"constitution.option.wealthy_citizens_vote.name").c_str(),
                UIStrings::Get(
                    L"constitution.option.wealthy_citizens_vote.summary").c_str(),
                Effect));
        }

        {
            FConstitutionOptionEffect Effect;
            Effect.ImmigrationRateMultiplier = 1.03f;
            Effect.JobQualityModifier = 6;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 6);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, 3);
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, -4);
            AddFactionApproval(Effect, EPoliticalFaction::Industrialists, -3);
            Catalog.push_back(MakeOption(
                EConstitutionOptionId::EarlyRetirement,
                EConstitutionTopic::LaborPolicy,
                EBuildingEra::WorldWars,
                UIStrings::Get(
                    L"constitution.option.early_retirement.name").c_str(),
                UIStrings::Get(
                    L"constitution.option.early_retirement.summary").c_str(),
                Effect));
        }

        {
            FConstitutionOptionEffect Effect;
            Effect.ImmigrationRateMultiplier = 0.97f;
            Effect.JobQualityModifier = -4;
            Effect.SecurityModifier = 1;
            AddFactionApproval(Effect, EPoliticalFaction::Industrialists, 6);
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Communists, -5);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, -2);
            Catalog.push_back(MakeOption(
                EConstitutionOptionId::LifesWork,
                EConstitutionTopic::LaborPolicy,
                EBuildingEra::WorldWars,
                UIStrings::Get(
                    L"constitution.option.lifes_work.name").c_str(),
                UIStrings::Get(
                    L"constitution.option.lifes_work.summary").c_str(),
                Effect));
        }

        {
            FConstitutionOptionEffect Effect;
            Effect.ImmigrationRateMultiplier = 0.94f;
            Effect.LibertyModifier = -8;
            Effect.SecurityModifier = 3;
            AddFactionApproval(Effect, EPoliticalFaction::Religious, 9);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, -7);
            Catalog.push_back(MakeOption(
                EConstitutionOptionId::Theocracy,
                EConstitutionTopic::ReligionAndState,
                EBuildingEra::ColdWar,
                UIStrings::Get(
                    L"constitution.option.theocracy.name").c_str(),
                UIStrings::Get(
                    L"constitution.option.theocracy.summary").c_str(),
                Effect));
        }

        {
            FConstitutionOptionEffect Effect;
            Effect.ImmigrationRateMultiplier = 1.05f;
            Effect.LibertyModifier = 6;
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, 7);
            AddFactionApproval(Effect, EPoliticalFaction::Environmentalists, 2);
            AddFactionApproval(Effect, EPoliticalFaction::Religious, -8);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, -2);
            Catalog.push_back(MakeOption(
                EConstitutionOptionId::OfficialSeparation,
                EConstitutionTopic::ReligionAndState,
                EBuildingEra::ColdWar,
                UIStrings::Get(
                    L"constitution.option.official_separation.name").c_str(),
                UIStrings::Get(
                    L"constitution.option.official_separation.summary").c_str(),
                Effect));
        }

        {
            FConstitutionOptionEffect Effect;
            Effect.ImmigrationRateMultiplier = 1.06f;
            Effect.LibertyModifier = 9;
            Effect.SecurityModifier = -2;
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, 8);
            AddFactionApproval(Effect, EPoliticalFaction::Environmentalists, 3);
            AddFactionApproval(Effect, EPoliticalFaction::Militarists, -3);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, -5);
            Catalog.push_back(MakeOption(
                EConstitutionOptionId::FreeMedia,
                EConstitutionTopic::MediaIndependence,
                EBuildingEra::ColdWar,
                UIStrings::Get(
                    L"constitution.option.free_media.name").c_str(),
                UIStrings::Get(
                    L"constitution.option.free_media.summary").c_str(),
                Effect));
        }

        {
            FConstitutionOptionEffect Effect;
            Effect.ImmigrationRateMultiplier = 0.95f;
            Effect.LibertyModifier = -10;
            Effect.SecurityModifier = 4;
            AddFactionApproval(Effect, EPoliticalFaction::Militarists, 6);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, -8);
            Catalog.push_back(MakeOption(
                EConstitutionOptionId::StateControlledMedia,
                EConstitutionTopic::MediaIndependence,
                EBuildingEra::ColdWar,
                UIStrings::Get(
                    L"constitution.option.state_controlled_media.name").c_str(),
                UIStrings::Get(
                    L"constitution.option.state_controlled_media.summary").c_str(),
                Effect));
        }

        {
            FConstitutionOptionEffect Effect;
            Effect.ImmigrationRateMultiplier = 1.01f;
            Effect.LibertyModifier = -2;
            Effect.SecurityModifier = 8;
            AddFactionApproval(Effect, EPoliticalFaction::Militarists, 8);
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 2);
            AddFactionApproval(Effect, EPoliticalFaction::Communists, -3);
            Catalog.push_back(MakeOption(
                EConstitutionOptionId::ProfessionalSoldiers,
                EConstitutionTopic::ArmedForces,
                EBuildingEra::WorldWars,
                UIStrings::Get(
                    L"constitution.option.professional_soldiers.name").c_str(),
                UIStrings::Get(
                    L"constitution.option.professional_soldiers.summary").c_str(),
                Effect));
        }

        {
            FConstitutionOptionEffect Effect;
            Effect.ImmigrationRateMultiplier = 1.02f;
            Effect.LibertyModifier = 2;
            Effect.SecurityModifier = -4;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Militarists, -5);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, -2);
            Catalog.push_back(MakeOption(
                EConstitutionOptionId::Militia,
                EConstitutionTopic::ArmedForces,
                EBuildingEra::WorldWars,
                UIStrings::Get(
                    L"constitution.option.militia.name").c_str(),
                UIStrings::Get(
                    L"constitution.option.militia.summary").c_str(),
                Effect));
        }

        return Catalog;
    }

    const FConstitutionOptionDef* FindConstitutionOptionDef(
        EConstitutionOptionId OptionId)
    {
        const auto& Catalog = ConstitutionSystem::GetConstitutionOptionCatalog();

        for (size_t Index = 0; Index < Catalog.size(); ++Index)
        {
            if (Catalog[Index].Id == OptionId)
                return &Catalog[Index];
        }

        return nullptr;
    }

    bool ResolveTopicOptions(
        EConstitutionTopic Topic,
        EConstitutionOptionId& OutLeftOptionId,
        EConstitutionOptionId& OutRightOptionId)
    {
        OutLeftOptionId = EConstitutionOptionId::None;
        OutRightOptionId = EConstitutionOptionId::None;

        const auto& Catalog = ConstitutionSystem::GetConstitutionOptionCatalog();

        for (size_t Index = 0; Index < Catalog.size(); ++Index)
        {
            if (Catalog[Index].Topic != Topic)
                continue;

            if (OutLeftOptionId == EConstitutionOptionId::None)
            {
                OutLeftOptionId = Catalog[Index].Id;
                continue;
            }

            OutRightOptionId = Catalog[Index].Id;
            break;
        }

        return OutLeftOptionId != EConstitutionOptionId::None &&
            OutRightOptionId != EConstitutionOptionId::None;
    }

    bool IsTopicUnlockedInEra(
        EConstitutionTopic Topic,
        EBuildingEra Era)
    {
        const auto& Catalog = ConstitutionSystem::GetConstitutionOptionCatalog();

        for (size_t Index = 0; Index < Catalog.size(); ++Index)
        {
            const FConstitutionOptionDef& Option = Catalog[Index];

            if (Option.Topic != Topic)
                continue;

            if (IsBuildingEraUnlocked(Era, Option.UnlockEra))
                return true;
        }

        return false;
    }

    void QueuePendingTopicsForEra(
        FConstitutionState& State,
        EBuildingEra Era)
    {
        std::array<bool, GConstitutionTopicCount> TopicVisited = {};
        const auto& Catalog = ConstitutionSystem::GetConstitutionOptionCatalog();

        for (size_t Index = 0; Index < Catalog.size(); ++Index)
        {
            const FConstitutionOptionDef& Option = Catalog[Index];
            const size_t TopicIndex = GetTopicIndex(Option.Topic);

            if (TopicVisited[TopicIndex])
                continue;

            TopicVisited[TopicIndex] = true;

            if (IsTopicUnlockedInEra(Option.Topic, Era))
                State.QueuedTopics[TopicIndex] = true;
        }
    }

    bool TryActivateNextQueuedTopic(FConstitutionState& State)
    {
        std::array<bool, GConstitutionTopicCount> TopicVisited = {};
        const auto& Catalog = ConstitutionSystem::GetConstitutionOptionCatalog();

        for (size_t Index = 0; Index < Catalog.size(); ++Index)
        {
            const FConstitutionOptionDef& Option = Catalog[Index];
            const size_t TopicIndex = GetTopicIndex(Option.Topic);

            if (TopicVisited[TopicIndex])
                continue;

            TopicVisited[TopicIndex] = true;

            if (!State.QueuedTopics[TopicIndex])
                continue;

            State.PendingTopicChoice = true;
            State.PendingTopic = Option.Topic;
            return true;
        }

        State.PendingTopicChoice = false;
        State.PendingTopic = EConstitutionTopic::VotingRights;
        return false;
    }

    void RecalculateActiveEffects(FConstitutionState& State)
    {
        State.ActiveEffects = FConstitutionOptionEffect();

        for (size_t TopicIndex = 0;
            TopicIndex < State.SelectedOptions.size();
            ++TopicIndex)
        {
            const EConstitutionOptionId OptionId =
                State.SelectedOptions[TopicIndex];

            if (OptionId == EConstitutionOptionId::None)
                continue;

            const FConstitutionOptionDef* const Option =
                FindConstitutionOptionDef(OptionId);

            if (!Option)
                continue;

            State.ActiveEffects.ImmigrationRateMultiplier *=
                Option->Effect.ImmigrationRateMultiplier;
            State.ActiveEffects.LibertyModifier +=
                Option->Effect.LibertyModifier;
            State.ActiveEffects.SecurityModifier +=
                Option->Effect.SecurityModifier;
            State.ActiveEffects.JobQualityModifier +=
                Option->Effect.JobQualityModifier;

            for (int FactionIndex = 0;
                FactionIndex < GPoliticalFactionCount;
                ++FactionIndex)
            {
                State.ActiveEffects.FactionApprovalDeltas[
                    static_cast<size_t>(FactionIndex)] +=
                    Option->Effect.FactionApprovalDeltas[
                        static_cast<size_t>(FactionIndex)];
            }
        }

        State.ActiveEffects.ImmigrationRateMultiplier = (std::max)(
            0.5f,
            (std::min)(1.5f, State.ActiveEffects.ImmigrationRateMultiplier));
        State.ActiveEffects.LibertyModifier = (std::max)(
            -25,
            (std::min)(25, State.ActiveEffects.LibertyModifier));
        State.ActiveEffects.SecurityModifier = (std::max)(
            -25,
            (std::min)(25, State.ActiveEffects.SecurityModifier));
        State.ActiveEffects.JobQualityModifier = (std::max)(
            -25,
            (std::min)(25, State.ActiveEffects.JobQualityModifier));

        for (int FactionIndex = 0;
            FactionIndex < GPoliticalFactionCount;
            ++FactionIndex)
        {
            int& Delta =
                State.ActiveEffects.FactionApprovalDeltas[
                    static_cast<size_t>(FactionIndex)];
            Delta = (std::max)(-25, (std::min)(25, Delta));
        }
    }
}

namespace ConstitutionSystem
{
    const std::vector<FConstitutionOptionDef>& GetConstitutionOptionCatalog()
    {
        static const std::vector<FConstitutionOptionDef> GCatalog =
            BuildConstitutionOptionCatalog();
        return GCatalog;
    }

    void OnEraTransitioned(FConstitutionState& State, EBuildingEra NewEra)
    {
        RecalculateActiveEffects(State);
        QueuePendingTopicsForEra(State, NewEra);

        if (!State.PendingTopicChoice)
            TryActivateNextQueuedTopic(State);
    }

    bool TrySelectConstitutionOption(
        FConstitutionState& State,
        EConstitutionOptionId OptionId)
    {
        const FConstitutionOptionDef* const Option =
            FindConstitutionOptionDef(OptionId);

        if (!Option ||
            Option->Id == EConstitutionOptionId::None ||
            !State.PendingTopicChoice ||
            Option->Topic != State.PendingTopic)
        {
            return false;
        }

        const size_t TopicIndex = GetTopicIndex(Option->Topic);
        State.SelectedOptions[TopicIndex] = OptionId;
        State.QueuedTopics[TopicIndex] = false;
        State.PendingTopicChoice = false;
        RecalculateActiveEffects(State);
        TryActivateNextQueuedTopic(State);

        return true;
    }

#ifdef _DEBUG
    bool BuildDebugRightOptionValidationSteps(
        std::vector<FDebugValidationStep>& OutSteps)
    {
        OutSteps.clear();

        FConstitutionState State;
        constexpr EBuildingEra ValidationEras[] =
        {
            EBuildingEra::WorldWars,
            EBuildingEra::ColdWar,
            EBuildingEra::Modern
        };

        for (const EBuildingEra Era : ValidationEras)
        {
            OnEraTransitioned(State, Era);

            while (State.PendingTopicChoice)
            {
                FDebugValidationStep Step;
                Step.TriggerEra = Era;
                Step.Topic = State.PendingTopic;

                if (!ResolveTopicOptions(
                        Step.Topic,
                        Step.LeftOptionId,
                        Step.RightOptionId))
                {
                    OutSteps.clear();
                    return false;
                }

                FConstitutionState AfterSelection = State;

                if (!TrySelectConstitutionOption(
                        AfterSelection,
                        Step.RightOptionId))
                {
                    OutSteps.clear();
                    return false;
                }

                Step.ExpectPendingAfterSelection =
                    AfterSelection.PendingTopicChoice;
                Step.ExpectedNextPendingTopic =
                    AfterSelection.PendingTopic;
                OutSteps.push_back(Step);
                State = AfterSelection;
            }
        }

        return !OutSteps.empty();
    }
#endif
}
