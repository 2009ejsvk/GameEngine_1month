#pragma once

#include "PoliticalTypes.h"
#include <vector>

struct FGovernmentEdictEffectDefinition
{
    int       FoodConsumptionPerVisitMin = 0;
    float     FoodGainMultiplier = 1.f;
    float     ProductionMultiplier = 1.f;
    float     TaxRevenueMultiplier = 1.f;
    float     DailyFoodDelta = 0.f;
    float     DailyHousingDelta = 0.f;
    float     DailyJobDelta = 0.f;
    float     DailyFreedomDelta = 0.f;
    float     DailySecurityDelta = 0.f;
    float     DailyHealthDelta = 0.f;
    float     DailyFaithDelta = 0.f;
    float     DailyFunDelta = 0.f;
    float     ImmigrationRateMultiplier = 1.f;
    float     BuildingCostMultiplier = 1.f;
    float     FarmBuildingEfficiencyMultiplier = 1.f;
    float     ExportPriceMultiplier = 1.f;
    float     HarborUpkeepMultiplier = 1.f;
    float     HarborShipProgressMultiplier = 1.f;
    float     PollutionPerIndustryBuildingDelta = 0.f;
    long long DailyBudgetDeltaPerIndustryBuilding = 0;
    int       TouristRatingModifierPercent = 0;
    int       TouristChanceBonusPercent = 0;
    std::array<int, GPoliticalFactionCount> FactionApprovalDelta = {};
    long long DailyBudgetDeltaFlat = 0;
    long long DailyBudgetDeltaPerCitizen = 0;
    long long DailyBudgetDeltaPerCitizenAbsMinimum = 0;
};

struct FGovernmentEdictDefinition
{
    EGovernmentEdictType         Type = EGovernmentEdictType::None;
    EEdictEra                    Era = EEdictEra::Colonial;
    EGovernmentEdictMode         Mode = EGovernmentEdictMode::Passive;
    EPoliticalActionType         ActionType = EPoliticalActionType::None;
    std::wstring                 DisplayName;
    std::wstring                 Summary;
    std::wstring                 EffectText;
    std::wstring                 IconPath;
    bool                         Implemented = false;
    long long                    BaseCost = 0;
    long long                    CostPerCitizen = 0;
    long long                    MonthlyUpkeep = 0;
    int                          DurationDays = 0;
    int                          CooldownDays = 0;
    FGovernmentEdictEffectDefinition Effect;
    std::vector<FPoliticalSignalDef> Signals;
};

namespace EdictSystem
{
    const std::vector<FGovernmentEdictDefinition>&
        GetGovernmentEdictDefinitions();

    const FGovernmentEdictDefinition* FindGovernmentEdictDefinition(
        EGovernmentEdictType Type);

    void RegisterRuntimeConfig();
    unsigned long long GetRuntimeConfigGeneration();

    void InitializeGovernmentEdictStates(
        std::vector<FGovernmentEdictState>& OutStates);
    void SynchronizeGovernmentEdictStates(
        std::vector<FGovernmentEdictState>& InOutStates);

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
