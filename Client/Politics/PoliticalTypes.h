#pragma once

#include "../Citizen/CitizenTypes.h"
#include <string>
#include <vector>

enum class EPoliticalScope
{
    Global = 0,
    Worker,
    Resident,
    Visitor
};

struct FPoliticalSignalDef
{
    EPoliticalAxis   Axis = EPoliticalAxis::Economy;
    EPoliticalStance FavoredStance = EPoliticalStance::Neutral;
    float            Strength = 0.f;
    EPoliticalScope  Scope = EPoliticalScope::Global;
};

enum class EPoliticalActionType
{
    None = 0,
    WelfareProgram,
    TaxCut,
    MartialLaw,
    HousingInitiative,
    IndustrialSubsidy
};

enum class EGovernmentEdictType
{
    None = 0,
    FoodForThePeople,
    TaxCut,
    MartialLaw,
    FreeHousing,
    EmployeeOfTheMonth
};

enum class EGovernmentEdictMode
{
    Passive = 0,
    Active
};

struct FGovernmentActionRecord
{
    EPoliticalActionType         Type = EPoliticalActionType::None;
    std::wstring                 Label;
    float                        Strength = 0.f;
    int                          RemainingDays = 0;
    std::vector<FPoliticalSignalDef> Signals;
};

struct FGovernmentProfile
{
    FNpcPoliticalProfile            Ideology;
    float                           WelfareBias = 0.f;
    float                           LibertyBias = 0.f;
    float                           Militarization = 0.f;
    std::vector<FGovernmentActionRecord> ActiveActions;
};

enum class EVoteIntent
{
    Incumbent = 0,
    Opposition,
    Abstain
};

struct FCitizenPoliticalEvaluation
{
    float       LifeScore = 0.f;
    float       GovernmentIdeologyScore = 0.f;
    float       BuildingScore = 0.f;
    float       ActionScore = 0.f;
    float       FearScore = 0.f;
    float       TotalSupportScore = 50.f;
    EVoteIntent VoteIntent = EVoteIntent::Abstain;
};

struct FPoliticalWorldSnapshot
{
    int    ActiveCitizenCount = 0;
    int    IncumbentCount = 0;
    int    OppositionCount = 0;
    int    AbstainCount = 0;
    double AverageSupportScore = 50.0;
    double AverageLifeScore = 0.0;
    double AverageGovernmentIdeologyScore = 0.0;
    double AverageBuildingScore = 0.0;
    double AverageActionScore = 0.0;
};

struct FGovernmentEdictState
{
    EGovernmentEdictType Type = EGovernmentEdictType::None;
    bool                 Active = false;
    int                  RemainingDays = 0;
    int                  CooldownDays = 0;
};

struct FGovernmentEdictModifiers
{
    int       FoodConsumptionPerVisit = 1;
    float     FoodGainMultiplier = 1.f;
    float     ProductionMultiplier = 1.f;
    float     DailyFoodDelta = 0.f;
    float     DailyHousingDelta = 0.f;
    float     DailyJobDelta = 0.f;
    float     DailyFreedomDelta = 0.f;
    float     DailySecurityDelta = 0.f;
    long long DailyBudgetDelta = 0;
};
