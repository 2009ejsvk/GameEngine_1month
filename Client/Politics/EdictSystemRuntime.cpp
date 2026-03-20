#include "EdictSystemRuntime.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>

namespace
{
    const FGovernmentEdictDefinition* FindEdictDefinition(
        const std::vector<FGovernmentEdictDefinition>& Definitions,
        EGovernmentEdictType Type)
    {
        for (size_t Index = 0; Index < Definitions.size(); ++Index)
        {
            if (Definitions[Index].Type == Type)
                return &Definitions[Index];
        }

        return nullptr;
    }

    void ApplyEdictEffectToModifiers(
        const FGovernmentEdictEffectDefinition& Effect,
        int ActiveCitizenCount,
        FGovernmentEdictModifiers& InOutModifiers)
    {
        if (Effect.FoodConsumptionPerVisitMin > 0)
        {
            InOutModifiers.FoodConsumptionPerVisit = (std::max)(
                InOutModifiers.FoodConsumptionPerVisit,
                Effect.FoodConsumptionPerVisitMin);
        }

        InOutModifiers.FoodGainMultiplier *= Effect.FoodGainMultiplier;
        InOutModifiers.ProductionMultiplier *= Effect.ProductionMultiplier;
        InOutModifiers.TaxRevenueMultiplier *= Effect.TaxRevenueMultiplier;
        InOutModifiers.DailyFoodDelta += Effect.DailyFoodDelta;
        InOutModifiers.DailyHousingDelta += Effect.DailyHousingDelta;
        InOutModifiers.DailyJobDelta += Effect.DailyJobDelta;
        InOutModifiers.DailyFreedomDelta += Effect.DailyFreedomDelta;
        InOutModifiers.DailySecurityDelta += Effect.DailySecurityDelta;
        InOutModifiers.DailyHealthDelta += Effect.DailyHealthDelta;
        InOutModifiers.DailyFaithDelta += Effect.DailyFaithDelta;
        InOutModifiers.DailyFunDelta += Effect.DailyFunDelta;
        InOutModifiers.ImmigrationRateMultiplier *=
            Effect.ImmigrationRateMultiplier;
        InOutModifiers.BuildingCostMultiplier *=
            Effect.BuildingCostMultiplier;
        InOutModifiers.FarmBuildingEfficiencyMultiplier *=
            Effect.FarmBuildingEfficiencyMultiplier;
        InOutModifiers.ExportPriceMultiplier *=
            Effect.ExportPriceMultiplier;
        InOutModifiers.HarborUpkeepMultiplier *=
            Effect.HarborUpkeepMultiplier;
        InOutModifiers.HarborShipProgressMultiplier *=
            Effect.HarborShipProgressMultiplier;
        InOutModifiers.PollutionPerIndustryBuildingDelta +=
            Effect.PollutionPerIndustryBuildingDelta;
        InOutModifiers.DailyBudgetDeltaPerIndustryBuilding +=
            Effect.DailyBudgetDeltaPerIndustryBuilding;
        InOutModifiers.TouristRatingModifierPercent +=
            Effect.TouristRatingModifierPercent;
        InOutModifiers.TouristChanceBonusPercent +=
            Effect.TouristChanceBonusPercent;
        InOutModifiers.DailyBudgetDelta += Effect.DailyBudgetDeltaFlat;

        for (int FactionIndex = 0;
            FactionIndex < GPoliticalFactionCount;
            ++FactionIndex)
        {
            InOutModifiers.FactionApprovalModifiers[
                static_cast<size_t>(FactionIndex)] +=
                Effect.FactionApprovalDelta[
                    static_cast<size_t>(FactionIndex)];
        }

        if (Effect.DailyBudgetDeltaPerCitizen != 0)
        {
            const long long PerCitizenMagnitude =
                std::llabs(Effect.DailyBudgetDeltaPerCitizen);
            long long Contribution =
                PerCitizenMagnitude *
                static_cast<long long>((std::max)(0, ActiveCitizenCount));
            Contribution = (std::max)(
                Contribution,
                std::llabs(Effect.DailyBudgetDeltaPerCitizenAbsMinimum));

            InOutModifiers.DailyBudgetDelta +=
                Effect.DailyBudgetDeltaPerCitizen < 0 ?
                    -Contribution :
                    Contribution;
        }
    }
}

namespace EdictSystemRuntime
{
    void InitializeGovernmentEdictStates(
        const std::vector<FGovernmentEdictDefinition>& Definitions,
        std::vector<FGovernmentEdictState>& OutStates)
    {
        OutStates.clear();
        SynchronizeGovernmentEdictStates(Definitions, OutStates);
    }

    void SynchronizeGovernmentEdictStates(
        const std::vector<FGovernmentEdictDefinition>& Definitions,
        std::vector<FGovernmentEdictState>& InOutStates)
    {
        std::vector<FGovernmentEdictState> ExistingStates = InOutStates;
        InOutStates.clear();
        InOutStates.reserve(Definitions.size());

        for (size_t DefinitionIndex = 0;
            DefinitionIndex < Definitions.size();
            ++DefinitionIndex)
        {
            FGovernmentEdictState State;
            State.Type = Definitions[DefinitionIndex].Type;

            for (size_t ExistingIndex = 0;
                ExistingIndex < ExistingStates.size();
                ++ExistingIndex)
            {
                if (ExistingStates[ExistingIndex].Type !=
                    Definitions[DefinitionIndex].Type)
                {
                    continue;
                }

                State = ExistingStates[ExistingIndex];
                break;
            }

            if (!Definitions[DefinitionIndex].Implemented)
            {
                State.Active = false;
                State.RemainingDays = 0;
                State.CooldownDays = 0;
            }

            InOutStates.push_back(State);
        }
    }

    long long ResolveEdictActivationCost(
        const FGovernmentEdictDefinition& Definition,
        int ActiveCitizenCount)
    {
        const int SafeCitizenCount = (std::max)(0, ActiveCitizenCount);
        return Definition.BaseCost +
            Definition.CostPerCitizen * static_cast<long long>(SafeCitizenCount);
    }

    long long CalculateEdictDailyUpkeep(
        const std::vector<FGovernmentEdictDefinition>& Definitions,
        const std::vector<FGovernmentEdictState>& States,
        int DaysInMonth)
    {
        const int SafeDays = (std::max)(1, DaysInMonth);
        long long Total = 0;

        for (size_t StateIndex = 0; StateIndex < States.size(); ++StateIndex)
        {
            if (!States[StateIndex].Active)
                continue;

            const FGovernmentEdictDefinition* Definition =
                FindEdictDefinition(Definitions, States[StateIndex].Type);

            if (!Definition || Definition->MonthlyUpkeep <= 0)
                continue;

            const double DailyCost =
                static_cast<double>(Definition->MonthlyUpkeep) /
                static_cast<double>(SafeDays);
            Total += static_cast<long long>(std::round(DailyCost));
        }

        return Total;
    }

    FGovernmentEdictModifiers CalculateEdictModifiers(
        const std::vector<FGovernmentEdictDefinition>& Definitions,
        const std::vector<FGovernmentEdictState>& States,
        int ActiveCitizenCount)
    {
        FGovernmentEdictModifiers Modifiers;
        const int SafeCitizenCount = (std::max)(0, ActiveCitizenCount);

        for (size_t StateIndex = 0; StateIndex < States.size(); ++StateIndex)
        {
            if (!States[StateIndex].Active)
                continue;

            const FGovernmentEdictDefinition* const Definition =
                FindEdictDefinition(Definitions, States[StateIndex].Type);

            if (!Definition || !Definition->Implemented)
                continue;

            ApplyEdictEffectToModifiers(
                Definition->Effect,
                SafeCitizenCount,
                Modifiers);
        }

        Modifiers.TouristChanceBonusPercent = (std::max)(
            -20,
            (std::min)(24, Modifiers.TouristChanceBonusPercent));

        for (int FactionIndex = 0;
            FactionIndex < GPoliticalFactionCount;
            ++FactionIndex)
        {
            int& Modifier =
                Modifiers.FactionApprovalModifiers[
                    static_cast<size_t>(FactionIndex)];
            Modifier = (std::max)(-25, (std::min)(25, Modifier));
        }

        return Modifiers;
    }

    FGovernmentActionRecord MakeGovernmentActionFromEdict(
        const FGovernmentEdictDefinition& Definition)
    {
        FGovernmentActionRecord Record;
        Record.Type = Definition.ActionType;
        Record.Label = Definition.DisplayName;
        Record.Strength = 1.f;
        Record.RemainingDays = -1;
        Record.Signals = Definition.Signals;
        return Record;
    }
}
