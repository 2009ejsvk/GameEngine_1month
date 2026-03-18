#pragma once

#include "PoliticalSignalTypes.h"
#include "../Economy/TradePolicy.h"
#include <array>
#include <string>
#include <vector>

enum class EPoliticalFaction
{
    Communists = 0,
    Capitalists,
    Religious,
    Militarists,
    Environmentalists,
    Industrialists,
    Intellectuals,
    Conservatives,
    Count
};

constexpr int GPoliticalFactionCount =
    static_cast<int>(EPoliticalFaction::Count);

inline constexpr EPoliticalAxis GetPoliticalFactionAxis(
    EPoliticalFaction Faction)
{
    switch (Faction)
    {
    case EPoliticalFaction::Communists:
    case EPoliticalFaction::Capitalists:
        return EPoliticalAxis::Economy;
    case EPoliticalFaction::Religious:
    case EPoliticalFaction::Militarists:
        return EPoliticalAxis::ReligionMilitarism;
    case EPoliticalFaction::Environmentalists:
    case EPoliticalFaction::Industrialists:
        return EPoliticalAxis::EnvironmentIndustry;
    case EPoliticalFaction::Intellectuals:
    case EPoliticalFaction::Conservatives:
        return EPoliticalAxis::IntellectualConservative;
    default:
        return EPoliticalAxis::Economy;
    }
}

inline constexpr EPoliticalStance GetPoliticalFactionStance(
    EPoliticalFaction Faction)
{
    switch (Faction)
    {
    case EPoliticalFaction::Communists:
    case EPoliticalFaction::Religious:
    case EPoliticalFaction::Environmentalists:
    case EPoliticalFaction::Intellectuals:
        return EPoliticalStance::Left;
    case EPoliticalFaction::Capitalists:
    case EPoliticalFaction::Militarists:
    case EPoliticalFaction::Industrialists:
    case EPoliticalFaction::Conservatives:
        return EPoliticalStance::Right;
    default:
        return EPoliticalStance::Neutral;
    }
}

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
    AusterityProgram,
    LaborNegotiation,
    WelfareSupport
};

enum class EEdictEra
{
    Colonial = 0,
    WorldWars,
    ColdWar,
    Modern
};

enum class EGovernmentEdictType
{
    None = 0,
    FoodForThePeople,
    TaxCut,
    MartialLaw,
    FreeHousing,
    EmployeeOfTheMonth,
    ChurchFee,
    NoFreeLunch,
    NothingBeatsSiesta,
    UrbanDevelopment,
    PenalColony,
    ChildAllowances,
    AdvancedBoatServices,
    Audience,
    AgriculturalSubsidies,
    BuildingPermit,
    StateLoans,
    WealthTax,
    Industrialization,
    EarlyElections,
    LiteracyProgram,
    Prohibition,
    BellsToBullets,
    MilitaryPolice,
    Volkswagen,
    RightToArms,
    NuclearTesting,
    GoodOldDays,
    ExperimentalGroundTreatment,
    MandatoryWasteSorting,
    DiplomaticSuperParty,
    HappyMeat,
    AlternativeFoodSource,
    SpellingBee,
    AssemblyBan,
    NationalDay,
    MadeInTropico,
    SocialSecurity,
    ContraceptionBan,
    LegalizedSubstances,
    CulturalDiversity,
    TaxHeaven,
    KnowledgioSinco,
    Speedway,
    CompulsoryVaccination,
    LightBulbBan,
    SeaDisposal,
    PolicyOfDetente,
    CTPA,
    SpecialTreats,
    FestivalBoost,
    MardiGras,
    MoneyLaundering,
    TourismState,
    HearTheCall,
    SoapOpera,
    LaborTaxRelief,
    PropertyTaxRelief,
    EmergencyAusterity,
    LaborNegotiation,
    EmergencyWelfareSupport
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
    TradePolicy::FExportTradePolicy ExportTradePolicy;
    TradePolicy::FImportTradePolicy ImportTradePolicy;
    float                           WelfareBias = 0.f;
    float                           LibertyBias = 0.f;
    float                           Militarization = 0.f;
    std::array<int, GPoliticalFactionCount> FactionApprovalModifiers = {};
    std::array<int, GPoliticalFactionCount> EdictFactionApprovalModifiers = {};
    std::vector<FGovernmentActionRecord> ActiveActions;
};

enum class EElectionPromiseType
{
    None = 0,
    Housing,
    Food,
    Health,
    Job,
    Freedom,
    Security,
    Faith,
    ExportIncome
};

struct FElectionPromiseState
{
    bool Active = false;
    EElectionPromiseType Type = EElectionPromiseType::None;
    int BaselineValue = 0;
    int TargetValue = 0;
    int CurrentValue = 0;
    int SuccessVoteModifierPercent = 0;
    int FailureVoteModifierPercent = 0;
    std::wstring Title;
    std::wstring Summary;
};

constexpr int GElectionPromiseCount = 2;

inline bool IsElectionPromiseMet(const FElectionPromiseState& Promise)
{
    return Promise.Active && Promise.CurrentValue >= Promise.TargetValue;
}

struct FElectionStatus
{
    bool HasRecordedElection = false;
    bool IncumbentWonLastElection = true;
    bool GameLost = false;
    int NextElectionYear = 0;
    int NextElectionMonth = 0;
    int NextElectionDay = 0;
    int ElectionsWon = 0;
    int LastElectionYear = 0;
    int LastElectionMonth = 0;
    int LastElectionDay = 0;
    int LastIncumbentVotes = 0;
    int LastOppositionVotes = 0;
    int LastAbstainVotes = 0;
    double LastVoteShare = 0.0;
    double LastTurnoutPercent = 0.0;
    bool CampaignPromisesIssued = false;
    int PromiseIssueYear = 0;
    int PromiseIssueMonth = 0;
    int PromiseIssueDay = 0;
    std::array<FElectionPromiseState, GElectionPromiseCount> ActivePromises = {};
    bool HasPromiseEvaluation = false;
    int LastPromiseMetCount = 0;
    int LastPromiseFailedCount = 0;
    int LastPromiseVoteModifierPercent = 0;
};

enum class ETaxPolicyEventType
{
    None = 0,
    WorkerTaxStrike,
    PropertyTaxBacklash,
    BudgetCrisis
};

struct FTaxPolicyEventStatus
{
    ETaxPolicyEventType Type = ETaxPolicyEventType::None;
    bool Active = false;
    int RemainingDays = 0;
    int CooldownDays = 0;
    int NotificationDays = 0;
    int DaysActive = 0;
    int TriggerYear = 0;
    int TriggerMonth = 0;
    int TriggerDay = 0;
    std::wstring Title;
    std::wstring Summary;
};

enum class EWorldCrisisType
{
    None = 0,
    Raid,
    LaborStrike,
    CrimeWave,
    FiscalEmergency
};

struct FWorldCrisisStatus
{
    EWorldCrisisType Type = EWorldCrisisType::None;
    bool Active = false;
    int RemainingDays = 0;
    int CooldownDays = 0;
    int NotificationDays = 0;
    int DaysActive = 0;
    int TriggerYear = 0;
    int TriggerMonth = 0;
    int TriggerDay = 0;
    std::wstring Title;
    std::wstring Summary;
};

enum class EPoliticalDemandIssuerType
{
    None = 0,
    Faction,
    ForeignPower
};

enum class EPoliticalDemandObjectiveType
{
    None = 0,
    Housing,
    Food,
    Faith,
    Security,
    Freedom,
    Health,
    ExportIncome,
    IncomeTaxCeiling,
    PropertyTaxCeiling,
    ActiveTradeRoutes
};

enum class EPoliticalDemandStatus
{
    None = 0,
    PendingResponse,
    Accepted,
    Completed,
    Rejected,
    Failed
};

enum class EPoliticalDemandStage
{
    Warning = 0,
    Demand,
    Ultimatum,
    Revolt
};

struct FPoliticalDemandState
{
    bool Active = false;
    EPoliticalDemandIssuerType IssuerType =
        EPoliticalDemandIssuerType::None;
    int IssuerIndex = -1;
    EPoliticalDemandObjectiveType ObjectiveType =
        EPoliticalDemandObjectiveType::None;
    EPoliticalDemandStage Stage = EPoliticalDemandStage::Demand;
    EPoliticalDemandStatus Status = EPoliticalDemandStatus::None;
    int RemainingDays = 0;
    int DurationDays = 0;
    int TargetValue = 0;
    int CurrentValue = 0;
    int ModifierDurationDays = 0;
    long long RewardBudgetDelta = 0;
    int RewardFactionApprovalDelta = 0;
    int RewardForeignRelationDelta = 0;
    int RewardForeignStandingDelta = 0;
    long long PenaltyBudgetDelta = 0;
    int PenaltyFactionApprovalDelta = 0;
    int PenaltyForeignRelationDelta = 0;
    int PenaltyForeignStandingDelta = 0;
    std::wstring Title;
    std::wstring Summary;
    std::wstring ObjectiveText;
    std::wstring RewardText;
    std::wstring PenaltyText;
};

struct FPoliticalDemandNotice
{
    bool ActiveDemand = false;
    bool Positive = false;
    int RemainingDays = 0;
    std::wstring Title;
    std::wstring Summary;
};

inline bool IsPoliticalDemandAwaitingResponse(
    const FPoliticalDemandState& State)
{
    return State.Active &&
        State.Status == EPoliticalDemandStatus::PendingResponse;
}

inline bool IsPoliticalDemandAccepted(const FPoliticalDemandState& State)
{
    return State.Active &&
        State.Status == EPoliticalDemandStatus::Accepted;
}

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
    float       FactionAlignmentScore = 0.f;
    float       FactionApprovalScore = 50.f;
    EPoliticalFaction PrimaryFaction = EPoliticalFaction::Communists;
    EVoteIntent VoteIntent = EVoteIntent::Abstain;
};

struct FPoliticalFactionSnapshot
{
    int MemberCount = 0;
    double AverageApproval = 50.0;
    double AverageLifeScore = 0.0;
    double AverageBuildingScore = 0.0;
    double AverageActionScore = 0.0;
    double AverageAlignmentScore = 0.0;
    double ApprovalModifier = 0.0;
    int PressureDays = 0;
    EPoliticalDemandStage DemandStage = EPoliticalDemandStage::Demand;
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
    std::array<FPoliticalFactionSnapshot, GPoliticalFactionCount> Factions = {};
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
    float     DailyHealthDelta = 0.f;
    float     DailyFaithDelta = 0.f;
    float     DailyFunDelta = 0.f;
    float     ImmigrationRateMultiplier = 1.f;
    float     BuildingCostMultiplier = 1.f;
    float     FarmBuildingEfficiencyMultiplier = 1.f;
    float     ExportPriceMultiplier = 1.f;
    float     PollutionPerIndustryBuildingDelta = 0.f;
    long long DailyBudgetDeltaPerIndustryBuilding = 0;
    int       TouristRatingModifierPercent = 0;
    int       TouristChanceBonusPercent = 0;
    std::array<int, GPoliticalFactionCount> FactionApprovalModifiers = {};
    long long DailyBudgetDelta = 0;
};
