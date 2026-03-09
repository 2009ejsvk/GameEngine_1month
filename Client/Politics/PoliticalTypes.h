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
    IndustrialSubsidy,
    LaborTaxRelief,
    PropertyTaxRelief,
    AusterityProgram
};

enum class EGovernmentEdictType
{
    None = 0,
    FoodForThePeople,
    TaxCut,
    MartialLaw,
    FreeHousing,
    EmployeeOfTheMonth,
    LaborTaxRelief,
    PropertyTaxRelief,
    EmergencyAusterity
};

enum class EGovernmentEdictMode
{
    Passive = 0,
    Active
};

enum class ETaxPolicyType
{
    Consumption = 0,
    Income,
    Property
};

struct FTaxPolicy
{
    int ConsumptionRatePercent = 10;
    int IncomeRatePercent = 12;
    int PropertyRatePercent = 35;
};

inline const wchar_t* GetTaxPolicyDisplayName(ETaxPolicyType Type)
{
    switch (Type)
    {
    case ETaxPolicyType::Consumption:
        return L"소비세";
    case ETaxPolicyType::Income:
        return L"소득세";
    case ETaxPolicyType::Property:
        return L"재산세";
    default:
        return L"세율";
    }
}

inline int GetTaxPolicyStepPercent(ETaxPolicyType Type)
{
    switch (Type)
    {
    case ETaxPolicyType::Property:
        return 5;
    case ETaxPolicyType::Consumption:
    case ETaxPolicyType::Income:
    default:
        return 2;
    }
}

inline int GetTaxPolicyMinPercent(ETaxPolicyType Type)
{
    (void)Type;
    return 0;
}

inline int GetTaxPolicyDefaultPercent(ETaxPolicyType Type)
{
    switch (Type)
    {
    case ETaxPolicyType::Consumption:
        return 10;
    case ETaxPolicyType::Income:
        return 12;
    case ETaxPolicyType::Property:
        return 35;
    default:
        return 10;
    }
}

inline int GetTaxPolicyMaxPercent(ETaxPolicyType Type)
{
    switch (Type)
    {
    case ETaxPolicyType::Consumption:
        return 20;
    case ETaxPolicyType::Income:
        return 30;
    case ETaxPolicyType::Property:
        return 60;
    default:
        return 30;
    }
}

inline float GetTaxPolicyDeviationNormalized(
    ETaxPolicyType Type,
    int CurrentRatePercent)
{
    const int DefaultPercent = GetTaxPolicyDefaultPercent(Type);

    if (CurrentRatePercent == DefaultPercent)
        return 0.f;

    if (CurrentRatePercent > DefaultPercent)
    {
        const int PositiveRange =
            GetTaxPolicyMaxPercent(Type) - DefaultPercent;

        if (PositiveRange <= 0)
            return 0.f;

        return static_cast<float>(CurrentRatePercent - DefaultPercent) /
            static_cast<float>(PositiveRange);
    }

    const int NegativeRange =
        DefaultPercent - GetTaxPolicyMinPercent(Type);

    if (NegativeRange <= 0)
        return 0.f;

    return -static_cast<float>(DefaultPercent - CurrentRatePercent) /
        static_cast<float>(NegativeRange);
}

inline float GetTaxPolicyDeviationNormalized(
    const FTaxPolicy& TaxPolicy,
    ETaxPolicyType Type)
{
    switch (Type)
    {
    case ETaxPolicyType::Consumption:
        return GetTaxPolicyDeviationNormalized(
            Type,
            TaxPolicy.ConsumptionRatePercent);
    case ETaxPolicyType::Income:
        return GetTaxPolicyDeviationNormalized(
            Type,
            TaxPolicy.IncomeRatePercent);
    case ETaxPolicyType::Property:
        return GetTaxPolicyDeviationNormalized(
            Type,
            TaxPolicy.PropertyRatePercent);
    default:
        return 0.f;
    }
}

inline float GetCitizenTaxBurdenNormalized(
    const FTaxPolicy& TaxPolicy,
    bool IsWorker,
    bool IsResident)
{
    const float ConsumptionDeviation =
        GetTaxPolicyDeviationNormalized(
            TaxPolicy,
            ETaxPolicyType::Consumption);
    const float IncomeDeviation =
        GetTaxPolicyDeviationNormalized(
            TaxPolicy,
            ETaxPolicyType::Income);
    const float PropertyDeviation =
        GetTaxPolicyDeviationNormalized(
            TaxPolicy,
            ETaxPolicyType::Property);

    const float ConsumptionWeight = 0.40f;
    const float IncomeWeight = IsWorker ? 0.35f : 0.12f;
    const float PropertyWeight = IsResident ? 0.25f : 0.08f;
    const float TotalWeight =
        ConsumptionWeight + IncomeWeight + PropertyWeight;

    if (TotalWeight <= 0.f)
        return 0.f;

    return
        (ConsumptionDeviation * ConsumptionWeight +
            IncomeDeviation * IncomeWeight +
            PropertyDeviation * PropertyWeight) /
        TotalWeight;
}

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
    FTaxPolicy                      TaxPolicy;
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
    float     TaxRevenueMultiplier = 1.f;
    float     DailyFoodDelta = 0.f;
    float     DailyHousingDelta = 0.f;
    float     DailyJobDelta = 0.f;
    float     DailyFreedomDelta = 0.f;
    float     DailySecurityDelta = 0.f;
    long long DailyBudgetDelta = 0;
};
