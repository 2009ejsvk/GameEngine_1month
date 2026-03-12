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
        bool TaxEventActive)
    {
        FForeignTradeRuntimeData Result;

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

    inline std::array<FForeignTradeRuntimeData, GForeignPowerCount>
    BuildForeignTradeRuntimeData(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates)
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
                    TaxEventStatus.Active);
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

    inline int ComputeDiplomacyExportBiasPercent(
        EResourceType Type,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates)
    {
        const auto Powers =
            BuildForeignTradeRuntimeData(
                Snapshot,
                GovernmentProfile,
                TaxEventStatus,
                GovernmentEdictStates);
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
                static_cast<double>(Powers[static_cast<size_t>(Index)].TradeModifier) *
                Weight;
        }

        const double Bias =
            WeightedTrade * 0.45 +
            (WeightedRelation - 50.0) * 0.16;
        return ClampInt(
            static_cast<int>(std::lround(Bias)),
            -12,
            15);
    }

    inline int ComputeDiplomacyImportBiasPercent(
        EResourceType Type,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates)
    {
        const auto Powers =
            BuildForeignTradeRuntimeData(
                Snapshot,
                GovernmentProfile,
                TaxEventStatus,
                GovernmentEdictStates);
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
                static_cast<double>(Powers[static_cast<size_t>(Index)].TradeModifier) *
                Weight;
        }

        const double Bias =
            -WeightedTrade * 0.32 -
            (WeightedRelation - 50.0) * 0.12;
        return ClampInt(
            static_cast<int>(std::lround(Bias)),
            -10,
            12);
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
