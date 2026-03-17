#pragma once

#include "../Politics/PoliticalTypes.h"
#include "../World/WorldStatsSnapshot.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <vector>

namespace TradeDiplomacyRuntime
{
    constexpr int GForeignPowerCount = 5;

    struct FForeignTradeRuntimeData
    {
        int Relation = 0;
        int TradeModifier = 0;
    };

    struct FForeignPowerStandingState
    {
        int Standing = 0;
        int RelationModifier = 0;
        int ActiveContractCount = 0;
        int SignedContractCount = 0;
        int CompletedContractCount = 0;
        int FailedContractCount = 0;
        int IdleDays = 0;
        int LastStandingChange = 0;
        int LastRelationChange = 0;
    };

    struct FForeignPowerWorldState
    {
        int Relation = 0;
        int TradeModifier = 0;
        int Standing = 0;
        int RelationModifier = 0;
        int ActiveContractCount = 0;
        int SignedContractCount = 0;
        int CompletedContractCount = 0;
        int FailedContractCount = 0;
        int LastStandingChange = 0;
        int LastRelationChange = 0;
    };

    struct FForeignTradeContext
    {
        double HarborStrength = 0.0;
        double TradeStrength = 0.0;
        double IndustryStrength = 0.0;
        double TourismStrength = 0.0;
        double FaithStrength = 0.0;
        double FoodStrength = 0.0;
        double EducationStrength = 0.0;
        double WelfareStrength = 0.0;
        double LibertyStrength = 0.0;
        double SecurityStrength = 0.0;
        double PowerStability = 0.0;
        double MarketPreference = 0.0;
        double StatePreference = 0.0;
        double ReligionPreference = 0.0;
        double MilitaryPreference = 0.0;
        double EnvironmentPreference = 0.0;
        double IndustryPreference = 0.0;
        double IntellectualPreference = 0.0;
        double ConservativePreference = 0.0;
    };

    inline double Clamp01(double Value)
    {
        return (std::max)(0.0, (std::min)(1.0, Value));
    }

    inline int ClampInt(int Value, int MinValue, int MaxValue)
    {
        return (std::max)(MinValue, (std::min)(MaxValue, Value));
    }

    inline int ClampStanding(int Value)
    {
        return ClampInt(Value, -100, 100);
    }

    inline const wchar_t* GetForeignPowerStatusText(int Relation)
    {
        if (Relation >= 82)
            return L"우호적";
        if (Relation >= 64)
            return L"호의적";
        if (Relation >= 46)
            return L"신중";
        if (Relation >= 28)
            return L"경계";

        return L"냉각";
    }

    inline const wchar_t* GetForeignPowerName(int Index, EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::Colonial:
            switch (Index)
            {
            case 0: return L"왕실";
            case 1: return L"밀수업자";
            default: return L"해외";
            }
        case EBuildingEra::WorldWars:
            switch (Index)
            {
            case 0: return L"연합국";
            case 1: return L"추축국";
            default: return L"해외";
            }
        case EBuildingEra::ColdWar:
            switch (Index)
            {
            case 0: return L"서방 진영";
            case 1: return L"동구권";
            default: return L"해외";
            }
        case EBuildingEra::Modern:
        default:
            switch (Index)
            {
            case 0: return L"미국";
            case 1: return L"러시아";
            case 2: return L"중국";
            case 3: return L"유럽연합";
            case 4: return L"중동";
            default: return L"해외";
            }
        }
    }

    inline bool IsForeignPowerActiveForEra(int Index, EBuildingEra Era)
    {
        if (Index < 0 || Index >= GForeignPowerCount)
            return false;

        switch (Era)
        {
        case EBuildingEra::Colonial:
        case EBuildingEra::WorldWars:
        case EBuildingEra::ColdWar:
            return Index < 2;
        case EBuildingEra::Modern:
        default:
            return true;
        }
    }

    inline bool HasActiveEdict(
        const std::vector<FGovernmentEdictState>& States,
        EGovernmentEdictType Type)
    {
        for (size_t Index = 0; Index < States.size(); ++Index)
        {
            if (States[Index].Type == Type && States[Index].Active)
                return true;
        }

        return false;
    }

    inline double NormalizeBias(float Value)
    {
        return Clamp01(0.5 + static_cast<double>(Value) * 0.5);
    }

    inline double GetGovernmentAffinity(
        const FNpcPoliticalChoice& Choice,
        EPoliticalStance Target)
    {
        const double SupportWeight =
            Choice.Support == EPoliticalSupportLevel::Weak ? 0.18 :
            Choice.Support == EPoliticalSupportLevel::Strong ? 0.40 :
            0.28;

        if (Choice.Stance == Target)
            return Clamp01(0.55 + SupportWeight);
        if (Choice.Stance == EPoliticalStance::Neutral)
            return 0.50;

        return Clamp01(0.28 - SupportWeight * 0.35);
    }

    inline FForeignTradeContext BuildForeignTradeContext(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        bool MartialLawActive)
    {
        FForeignTradeContext Context;
        const int IndustryCategoryIndex =
            static_cast<int>(EBuildingCategory::Industry);
        long long TotalHarborExportableStock = 0;

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            TotalHarborExportableStock +=
                Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)].
                    HarborExportableStock;
        }

        const int HighEducationCount =
            Snapshot.EducationCount[1] + Snapshot.EducationCount[2];
        const double CitizenCount =
            static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount));
        const double ExportStrength = Clamp01(
            static_cast<double>((std::max)(0LL, TotalHarborExportableStock)) /
            16000.0);
        const double ResourceStrength = Clamp01(
            static_cast<double>((std::max)(0LL, Snapshot.TotalResourceStock)) /
            14000.0);

        Context.HarborStrength =
            Clamp01(static_cast<double>(Snapshot.HarborCount) / 6.0);
        Context.TradeStrength =
            Clamp01(Context.HarborStrength * 0.45 + ExportStrength * 0.55);
        Context.IndustryStrength =
            Clamp01(
                static_cast<double>(
                    Snapshot.BuildingCategoryCount[IndustryCategoryIndex]) / 8.0 +
                ResourceStrength * 0.35 +
                Clamp01(
                    static_cast<double>(Snapshot.JobCapacity) / 2500.0) * 0.15);
        Context.TourismStrength =
            Clamp01(
                static_cast<double>(Snapshot.TourismBuildingCount) / 8.0 +
                static_cast<double>(Snapshot.EntertainmentBuildingCount) / 14.0 +
                Clamp01(Snapshot.AverageFun / 100.0) * 0.25);
        Context.FaithStrength =
            Clamp01(static_cast<double>(Snapshot.FaithBuildingCount) / 6.0);
        Context.FoodStrength =
            Clamp01(
                static_cast<double>(Snapshot.FoodProviderCount) / 12.0 +
                Clamp01(Snapshot.AverageFood / 100.0) * 0.40);
        Context.EducationStrength =
            Clamp01(
                static_cast<double>(HighEducationCount) / CitizenCount * 1.2 +
                static_cast<double>(Snapshot.FreedomInfluenceBuildingCount) /
                    8.0 * 0.35);
        Context.WelfareStrength =
            Clamp01(
                NormalizeBias(GovernmentProfile.WelfareBias) * 0.50 +
                Clamp01(Snapshot.AverageHealth / 100.0) * 0.25 +
                Clamp01(Snapshot.AverageHousing / 100.0) * 0.25);
        Context.LibertyStrength =
            Clamp01(
                NormalizeBias(GovernmentProfile.LibertyBias) * 0.60 +
                Clamp01(Snapshot.AverageFreedom / 100.0) * 0.40);
        Context.SecurityStrength =
            Clamp01(
                NormalizeBias(GovernmentProfile.Militarization) * 0.35 +
                Clamp01(Snapshot.AverageSecurity / 100.0) * 0.35 +
                Clamp01(
                    static_cast<double>(Snapshot.SecurityInfluenceBuildingCount) /
                    8.0) * 0.20 +
                (MartialLawActive ? 0.10 : 0.0));
        Context.PowerStability =
            Snapshot.TotalRequiredPowerMW > 0 ?
                Clamp01(
                    static_cast<double>((std::max)(
                        0,
                        Snapshot.TotalProducedPowerMW)) /
                    static_cast<double>(Snapshot.TotalRequiredPowerMW)) :
                (Snapshot.DisconnectedConsumerCount > 0 ? 0.25 : 1.0);

        Context.MarketPreference =
            GetGovernmentAffinity(
                GovernmentProfile.Ideology.Economy,
                EPoliticalStance::Left);
        Context.StatePreference =
            GetGovernmentAffinity(
                GovernmentProfile.Ideology.Economy,
                EPoliticalStance::Right);
        Context.ReligionPreference =
            GetGovernmentAffinity(
                GovernmentProfile.Ideology.ReligionMilitarism,
                EPoliticalStance::Left);
        Context.MilitaryPreference =
            GetGovernmentAffinity(
                GovernmentProfile.Ideology.ReligionMilitarism,
                EPoliticalStance::Right);
        Context.EnvironmentPreference =
            GetGovernmentAffinity(
                GovernmentProfile.Ideology.EnvironmentIndustry,
                EPoliticalStance::Left);
        Context.IndustryPreference =
            GetGovernmentAffinity(
                GovernmentProfile.Ideology.EnvironmentIndustry,
                EPoliticalStance::Right);
        Context.IntellectualPreference =
            GetGovernmentAffinity(
                GovernmentProfile.Ideology.IntellectualConservative,
                EPoliticalStance::Left);
        Context.ConservativePreference =
            GetGovernmentAffinity(
                GovernmentProfile.Ideology.IntellectualConservative,
                EPoliticalStance::Right);

        return Context;
    }

    inline FForeignTradeRuntimeData BuildForeignPowerRuntimeData(
        int Index,
        const FForeignTradeContext& Context,
        bool MartialLawActive,
        bool TaxEventActive,
        EBuildingEra Era = EBuildingEra::Modern)
    {
        FForeignTradeRuntimeData Result;

        if (!IsForeignPowerActiveForEra(Index, Era))
            return Result;

        switch (Index)
        {
        case 0:
            Result.TradeModifier = ClampInt(
                static_cast<int>(std::lround(
                    Context.TradeStrength * 15.0 +
                    Context.HarborStrength * 5.0)),
                -20,
                20);
            Result.Relation = ClampInt(
                static_cast<int>(std::lround(
                    38.0 +
                    Context.TradeStrength * 26.0 +
                    Context.IndustryPreference * 18.0 +
                    Context.SecurityStrength * 10.0 +
                    Context.StatePreference * 8.0 -
                    Context.EnvironmentPreference * 6.0)),
                0,
                100);
            break;
        case 1:
            Result.TradeModifier = ClampInt(
                static_cast<int>(std::lround(
                    Context.TradeStrength * 8.0 +
                    Context.PowerStability * 4.0)),
                -20,
                20);
            Result.Relation = ClampInt(
                static_cast<int>(std::lround(
                    35.0 +
                    Context.SecurityStrength * 20.0 +
                    Context.MilitaryPreference * 18.0 +
                    Context.PowerStability * 10.0 +
                    Context.StatePreference * 8.0 -
                    Context.LibertyStrength * 6.0)),
                0,
                100);
            break;
        case 2:
            Result.TradeModifier = ClampInt(
                static_cast<int>(std::lround(
                    Context.TradeStrength * 9.0 +
                    Context.TourismStrength * 5.0)),
                -20,
                20);
            Result.Relation = ClampInt(
                static_cast<int>(std::lround(
                    36.0 +
                    Context.MarketPreference * 18.0 +
                    Context.LibertyStrength * 22.0 +
                    Context.TourismStrength * 16.0 +
                    Context.EducationStrength * 12.0 -
                    (MartialLawActive ? 12.0 : 0.0) -
                    (TaxEventActive ? 6.0 : 0.0))),
                0,
                100);
            break;
        case 3:
            Result.TradeModifier = ClampInt(
                static_cast<int>(std::lround(
                    Context.TradeStrength * 7.0 +
                    Context.FoodStrength * 4.0)),
                -20,
                20);
            Result.Relation = ClampInt(
                static_cast<int>(std::lround(
                    34.0 +
                    Context.ReligionPreference * 22.0 +
                    Context.FaithStrength * 18.0 +
                    Context.SecurityStrength * 10.0 +
                    Context.FoodStrength * 8.0 +
                    Context.ConservativePreference * 8.0 -
                    Context.LibertyStrength * 4.0)),
                0,
                100);
            break;
        default:
            Result.TradeModifier = ClampInt(
                static_cast<int>(std::lround(
                    Context.TradeStrength * 8.0 +
                    Context.EducationStrength * 4.0)),
                -20,
                20);
            Result.Relation = ClampInt(
                static_cast<int>(std::lround(
                    40.0 +
                    Context.EnvironmentPreference * 18.0 +
                    Context.IntellectualPreference * 18.0 +
                    Context.LibertyStrength * 16.0 +
                    Context.EducationStrength * 12.0 +
                    Context.WelfareStrength * 10.0 -
                    (MartialLawActive ? 14.0 : 0.0))),
                0,
                100);
            break;
        }

        return Result;
    }

    inline FForeignPowerWorldState BuildForeignPowerWorldState(
        const FForeignTradeRuntimeData& RuntimeData,
        const FForeignPowerStandingState& StandingState)
    {
        FForeignPowerWorldState Result;
        Result.Standing = ClampStanding(StandingState.Standing);
        Result.RelationModifier =
            ClampInt(StandingState.RelationModifier, -35, 35);
        Result.ActiveContractCount =
            ClampInt(StandingState.ActiveContractCount, 0, 32);
        Result.SignedContractCount =
            (std::max)(0, StandingState.SignedContractCount);
        Result.CompletedContractCount =
            (std::max)(0, StandingState.CompletedContractCount);
        Result.FailedContractCount =
            (std::max)(0, StandingState.FailedContractCount);
        Result.LastStandingChange =
            ClampInt(StandingState.LastStandingChange, -20, 20);
        Result.LastRelationChange =
            ClampInt(StandingState.LastRelationChange, -20, 20);

        const int CompletionBalance =
            Result.CompletedContractCount - Result.FailedContractCount;
        const int StandingRelationBonus = ClampInt(
            static_cast<int>(std::lround(
                static_cast<double>(Result.Standing) * 0.22)),
            -22,
            22);
        const int StandingTradeBonus = ClampInt(
            static_cast<int>(std::lround(
                static_cast<double>(Result.Standing) * 0.12)),
            -12,
            12);
        const int ActiveBonus =
            ClampInt(Result.ActiveContractCount * 2, 0, 8);
        const int ReliabilityRelationBonus =
            ClampInt(CompletionBalance * 2, -10, 10);
        const int ReliabilityTradeBonus =
            ClampInt(CompletionBalance, -6, 6);

        Result.Relation = ClampInt(
            RuntimeData.Relation +
                Result.RelationModifier +
                StandingRelationBonus +
                ActiveBonus +
                ReliabilityRelationBonus,
            0,
            100);
        Result.TradeModifier = ClampInt(
            RuntimeData.TradeModifier +
                StandingTradeBonus +
                ActiveBonus +
                ReliabilityTradeBonus +
                ClampInt(
                    static_cast<int>(std::lround(
                        static_cast<double>(Result.Relation - 50) * 0.10)),
                    -6,
                    6),
            -25,
            25);

        return Result;
    }

    inline void ApplyEdictForeignPolicyBias(
        int Index,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates,
        FForeignTradeRuntimeData& InOutData)
    {
        int RelationDelta = 0;
        int TradeDelta = 0;

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::PolicyOfDetente))
        {
            RelationDelta += (Index == 2 || Index == 4) ? 6 : 4;
            TradeDelta += (Index == 0 || Index == 2) ? 2 : 1;
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::DiplomaticSuperParty))
        {
            RelationDelta += (Index == 2 || Index == 4) ? 7 : 4;
            TradeDelta += Index == 2 ? 3 : 1;
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::TourismState))
        {
            if (Index == 2 || Index == 4)
            {
                RelationDelta += 6;
                TradeDelta += 2;
            }
            else if (Index == 3)
            {
                RelationDelta += 2;
            }
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::CTPA))
        {
            if (Index == 0 || Index == 2)
            {
                RelationDelta += 3;
                TradeDelta += 4;
            }
            else if (Index == 4)
            {
                TradeDelta += 1;
            }
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::TaxHeaven))
        {
            if (Index == 0 || Index == 2)
            {
                RelationDelta += 3;
                TradeDelta += 3;
            }
            else if (Index == 4)
            {
                RelationDelta -= 2;
            }
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::TaxCut) &&
            (Index == 0 || Index == 2))
        {
            RelationDelta += 2;
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::MartialLaw))
        {
            if (Index == 1 || Index == 3)
            {
                RelationDelta += 4;
            }
            else if (Index == 2 || Index == 4)
            {
                RelationDelta -= 8;
                TradeDelta -= 3;
            }
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::MilitaryPolice))
        {
            if (Index == 1)
            {
                RelationDelta += 3;
            }
            else if (Index == 2 || Index == 4)
            {
                RelationDelta -= 5;
                TradeDelta -= 2;
            }
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::RightToArms))
        {
            if (Index == 1)
            {
                RelationDelta += 4;
            }
            else if (Index == 2 || Index == 4)
            {
                RelationDelta -= 2;
            }
        }

        if ((HasActiveEdict(
                 GovernmentEdictStates,
                 EGovernmentEdictType::FreeHousing) ||
                HasActiveEdict(
                    GovernmentEdictStates,
                    EGovernmentEdictType::FoodForThePeople)) &&
            Index == 4)
        {
            RelationDelta += 2;
        }

        if ((HasActiveEdict(
                 GovernmentEdictStates,
                 EGovernmentEdictType::ChurchFee) ||
                HasActiveEdict(
                    GovernmentEdictStates,
                    EGovernmentEdictType::Prohibition)) &&
            Index == 3)
        {
            RelationDelta += 3;
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::EmergencyAusterity))
        {
            if (Index == 0 || Index == 1)
            {
                RelationDelta += 1;
                TradeDelta += 1;
            }
            else if (Index == 2 || Index == 4)
            {
                RelationDelta -= 4;
                TradeDelta -= 2;
            }
        }

        InOutData.Relation = ClampInt(
            InOutData.Relation + RelationDelta,
            0,
            100);
        InOutData.TradeModifier = ClampInt(
            InOutData.TradeModifier + TradeDelta,
            -25,
            25);
    }

    inline std::array<FForeignTradeRuntimeData, GForeignPowerCount>
    BuildForeignTradeRuntimeData(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates,
        EBuildingEra Era = EBuildingEra::Modern)
    {
        const bool MartialLawActive =
            HasActiveEdict(GovernmentEdictStates, EGovernmentEdictType::MartialLaw);
        const FForeignTradeContext Context =
            BuildForeignTradeContext(
                Snapshot,
                GovernmentProfile,
                MartialLawActive);
        std::array<FForeignTradeRuntimeData, GForeignPowerCount> Result = {};

        for (int Index = 0; Index < GForeignPowerCount; ++Index)
        {
            Result[static_cast<size_t>(Index)] =
                BuildForeignPowerRuntimeData(
                    Index,
                    Context,
                    MartialLawActive,
                    TaxEventStatus.Active,
                    Era);
            ApplyEdictForeignPolicyBias(
                Index,
                GovernmentEdictStates,
                Result[static_cast<size_t>(Index)]);
        }

        return Result;
    }

    inline std::array<FForeignPowerWorldState, GForeignPowerCount>
    BuildForeignPowerWorldStates(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates,
        EBuildingEra Era = EBuildingEra::Modern,
        const std::array<FForeignPowerStandingState, GForeignPowerCount>&
            StandingStates =
                std::array<FForeignPowerStandingState, GForeignPowerCount>())
    {
        const auto RuntimeData =
            BuildForeignTradeRuntimeData(
                Snapshot,
                GovernmentProfile,
                TaxEventStatus,
                GovernmentEdictStates,
                Era);
        std::array<FForeignPowerWorldState, GForeignPowerCount> Result = {};

        for (int Index = 0; Index < GForeignPowerCount; ++Index)
        {
            if (!IsForeignPowerActiveForEra(Index, Era))
                continue;

            Result[static_cast<size_t>(Index)] =
                BuildForeignPowerWorldState(
                    RuntimeData[static_cast<size_t>(Index)],
                    StandingStates[static_cast<size_t>(Index)]);
        }

        return Result;
    }

    inline void ResolveTradeWeights(
        EResourceMarketClass MarketClass,
        std::array<double, GForeignPowerCount>& OutWeights)
    {
        switch (MarketClass)
        {
        case EResourceMarketClass::Food:
            OutWeights = { 0.15, 0.10, 0.15, 0.35, 0.25 };
            break;
        case EResourceMarketClass::RawGoods:
            OutWeights = { 0.40, 0.30, 0.05, 0.15, 0.10 };
            break;
        case EResourceMarketClass::ManufacturedGoods:
            OutWeights = { 0.35, 0.20, 0.20, 0.05, 0.20 };
            break;
        case EResourceMarketClass::LuxuryGoods:
            OutWeights = { 0.10, 0.00, 0.35, 0.20, 0.35 };
            break;
        default:
            OutWeights = { 0.20, 0.20, 0.20, 0.20, 0.20 };
            break;
        }
    }

    template <typename TForeignPowerState>
    inline int ComputeDiplomacyBiasPercent(
        EResourceType Type,
        const std::array<TForeignPowerState, GForeignPowerCount>& Powers,
        bool ImportRoute)
    {
        std::array<double, GForeignPowerCount> Weights = {};
        ResolveTradeWeights(GetResourceMarketClass(Type), Weights);

        double WeightedRelation = 0.0;
        double WeightedTrade = 0.0;

        for (int Index = 0; Index < GForeignPowerCount; ++Index)
        {
            const double Weight = Weights[static_cast<size_t>(Index)];
            WeightedRelation +=
                static_cast<double>(Powers[static_cast<size_t>(Index)].Relation) *
                Weight;
            WeightedTrade +=
                static_cast<double>(
                    Powers[static_cast<size_t>(Index)].TradeModifier) *
                Weight;
        }

        const double Bias = ImportRoute ?
            -WeightedTrade * 0.32 -
                (WeightedRelation - 50.0) * 0.12 :
            WeightedTrade * 0.45 +
                (WeightedRelation - 50.0) * 0.16;
        return ClampInt(
            static_cast<int>(std::lround(Bias)),
            ImportRoute ? -10 : -12,
            ImportRoute ? 12 : 15);
    }

    inline int ComputeDiplomacyExportBiasPercent(
        EResourceType Type,
        const std::array<FForeignPowerWorldState, GForeignPowerCount>& Powers)
    {
        return ComputeDiplomacyBiasPercent(Type, Powers, false);
    }

    inline int ComputeDiplomacyImportBiasPercent(
        EResourceType Type,
        const std::array<FForeignPowerWorldState, GForeignPowerCount>& Powers)
    {
        return ComputeDiplomacyBiasPercent(Type, Powers, true);
    }

    inline int ComputeDiplomacyExportBiasPercent(
        EResourceType Type,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates,
        EBuildingEra Era = EBuildingEra::Modern)
    {
        const auto Powers =
            BuildForeignTradeRuntimeData(
                Snapshot,
                GovernmentProfile,
                TaxEventStatus,
                GovernmentEdictStates,
                Era);
        return ComputeDiplomacyBiasPercent(Type, Powers, false);
    }

    inline int ComputeDiplomacyImportBiasPercent(
        EResourceType Type,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates,
        EBuildingEra Era = EBuildingEra::Modern)
    {
        const auto Powers =
            BuildForeignTradeRuntimeData(
                Snapshot,
                GovernmentProfile,
                TaxEventStatus,
                GovernmentEdictStates,
                Era);
        return ComputeDiplomacyBiasPercent(Type, Powers, true);
    }

    inline int ComputeEdictExportBiasPercent(
        EResourceType Type,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates)
    {
        const EResourceMarketClass MarketClass = GetResourceMarketClass(Type);
        int Bias = 0;

        if (HasActiveEdict(GovernmentEdictStates, EGovernmentEdictType::CTPA))
        {
            Bias += 6;

            if (MarketClass == EResourceMarketClass::ManufacturedGoods)
                Bias += 2;
            else if (MarketClass == EResourceMarketClass::LuxuryGoods)
                Bias += 3;
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::PolicyOfDetente))
        {
            if (MarketClass == EResourceMarketClass::RawGoods ||
                MarketClass == EResourceMarketClass::ManufacturedGoods)
            {
                Bias += 4;
            }
            else if (MarketClass == EResourceMarketClass::LuxuryGoods)
            {
                Bias += 2;
            }
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::DiplomaticSuperParty))
        {
            Bias += 3;
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::TaxHeaven))
        {
            if (MarketClass == EResourceMarketClass::ManufacturedGoods)
                Bias += 4;
            else if (MarketClass == EResourceMarketClass::LuxuryGoods)
                Bias += 6;
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::TourismState))
        {
            if (MarketClass == EResourceMarketClass::LuxuryGoods)
                Bias += 4;
            else if (MarketClass == EResourceMarketClass::Food)
                Bias += 1;
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::TaxCut) &&
            MarketClass == EResourceMarketClass::LuxuryGoods)
        {
            Bias += 2;
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::EmergencyAusterity))
        {
            Bias -= 6;
        }

        return ClampInt(Bias, -12, 18);
    }

    inline int ComputeEdictImportBiasPercent(
        EResourceType Type,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates)
    {
        const EResourceMarketClass MarketClass = GetResourceMarketClass(Type);
        int Bias = 0;

        if (HasActiveEdict(GovernmentEdictStates, EGovernmentEdictType::CTPA))
        {
            Bias -= 5;

            if (MarketClass == EResourceMarketClass::ManufacturedGoods)
                Bias -= 2;
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::PolicyOfDetente))
        {
            if (MarketClass == EResourceMarketClass::RawGoods ||
                MarketClass == EResourceMarketClass::ManufacturedGoods)
            {
                Bias -= 3;
            }
            else if (MarketClass == EResourceMarketClass::LuxuryGoods)
            {
                Bias -= 2;
            }
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::DiplomaticSuperParty))
        {
            Bias -= 2;
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::TaxHeaven))
        {
            if (MarketClass == EResourceMarketClass::ManufacturedGoods ||
                MarketClass == EResourceMarketClass::LuxuryGoods)
            {
                Bias -= 2;
            }
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::TourismState))
        {
            if (MarketClass == EResourceMarketClass::Food ||
                MarketClass == EResourceMarketClass::LuxuryGoods)
            {
                Bias += 3;
            }
        }

        if (HasActiveEdict(
                GovernmentEdictStates,
                EGovernmentEdictType::EmergencyAusterity))
        {
            Bias += 8;
        }

        return ClampInt(Bias, -12, 16);
    }
}
