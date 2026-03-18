#pragma once

#include "../Building/BuildingTypes.h"
#include "TradePolicy.h"
#include "../GameConstants.h"
#include <algorithm>

namespace TradePolicyRuntime
{
    inline bool IsExactOrLegacyBundleImportSelectionMatch(
        EResourceType SelectedResourceType,
        EResourceType InputType)
    {
        // Runtime still honors legacy family selections as exact bundles, but
        // UI cycling should stay exact-only through TradePolicy helpers.
        return TradePolicy::DoesPolicySelectionMatchResourceType(
            SelectedResourceType,
            InputType);
    }

    inline int ClampBiasPercent(int Value, int MinValue = -18, int MaxValue = 20)
    {
        return (std::max)(MinValue, (std::min)(MaxValue, Value));
    }

    inline int ComputeExportProductionBiasPercent(
        EResourceType ProducedType,
        const TradePolicy::FExportTradePolicy& ExportPolicy)
    {
        if (!IsExportableResourceType(ProducedType))
            return 0;

        const EResourceMarketClass MarketClass =
            GetPolicyFamilyMarketClass(ProducedType);
        const bool ExportAllowed =
            TradePolicy::IsResourceExportAllowed(ExportPolicy, ProducedType);
        int Bias = 0;

        if (ExportAllowed)
        {
            if (ExportPolicy.PrioritizeHighValueCargo)
            {
                switch (MarketClass)
                {
                case EResourceMarketClass::ManufacturedGoods:
                    Bias += 7;
                    break;
                case EResourceMarketClass::LuxuryGoods:
                    Bias += 10;
                    break;
                case EResourceMarketClass::RawGoods:
                    Bias += 3;
                    break;
                case EResourceMarketClass::Food:
                default:
                    Bias += 1;
                    break;
                }
            }
            else
            {
                switch (MarketClass)
                {
                case EResourceMarketClass::Food:
                    Bias += 6;
                    break;
                case EResourceMarketClass::RawGoods:
                    Bias += 5;
                    break;
                case EResourceMarketClass::ManufacturedGoods:
                    Bias += 3;
                    break;
                case EResourceMarketClass::LuxuryGoods:
                    Bias += 1;
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            switch (MarketClass)
            {
            case EResourceMarketClass::Food:
                Bias -= 1;
                break;
            case EResourceMarketClass::RawGoods:
                Bias -= 5;
                break;
            case EResourceMarketClass::ManufacturedGoods:
                Bias -= 8;
                break;
            case EResourceMarketClass::LuxuryGoods:
                Bias -= 10;
                break;
            default:
                break;
            }
        }

        return ClampBiasPercent(Bias);
    }

    inline int ComputeImportInputBiasPercent(
        EResourceType InputType,
        const TradePolicy::FImportTradePolicy& ImportPolicy)
    {
        if (InputType == EResourceType::FeedCrops)
        {
            int BestBias = (std::numeric_limits<int>::min)();

            ForEachFeedCompatibleResourceType(
                InputType,
                [&](EResourceType CandidateType)
                {
                    if (!IsExportableResourceType(CandidateType))
                        return;

                    BestBias = (std::max)(
                        BestBias,
                        ComputeImportInputBiasPercent(
                            CandidateType,
                            ImportPolicy));
                });

            return BestBias == (std::numeric_limits<int>::min)() ?
                0 :
                BestBias;
        }

        if (!IsExportableResourceType(InputType))
            return 0;

        int Bias = 0;

        switch (ImportPolicy.Mode)
        {
        case TradePolicy::EImportPolicyMode::None:
            Bias -= 8;
            break;
        case TradePolicy::EImportPolicyMode::SingleResource:
            Bias +=
                IsExactOrLegacyBundleImportSelectionMatch(
                    ImportPolicy.SelectedResourceType,
                    InputType) ?
                10 :
                -6;
            break;
        case TradePolicy::EImportPolicyMode::AllResources:
        default:
            Bias += 2;
            break;
        }

        switch (TradePolicy::GetImportMaxUnitsPerResource(ImportPolicy))
        {
        case 500:
            Bias -= 4;
            break;
        case 3000:
            Bias += 4;
            break;
        case 6000:
            Bias += 6;
            break;
        default:
            break;
        }

        const int DailyBudgetCap =
            TradePolicy::GetDailyImportBudgetCap(ImportPolicy);

        if (DailyBudgetCap <= 0)
        {
            Bias += 4;
        }
        else if (DailyBudgetCap <= 12000)
        {
            Bias -= 4;
        }
        else if (DailyBudgetCap >= 36000)
        {
            Bias += 3;
        }

        if (TradePolicy::AllowsEmergencyImports(ImportPolicy))
            Bias += 4;

        return ClampBiasPercent(Bias);
    }

    inline int ComputeBuildingProductionBiasPercent(
        EResourceType ProducedType,
        const std::array<EResourceType, GProductionInputSlotCount>& InputTypes,
        const TradePolicy::FExportTradePolicy& ExportPolicy,
        const TradePolicy::FImportTradePolicy& ImportPolicy)
    {
        (void)InputTypes;
        (void)ImportPolicy;
        return ClampBiasPercent(
            ComputeExportProductionBiasPercent(
                ProducedType,
                ExportPolicy),
            -22,
            22);
    }

    inline float ComputeBuildingProductionMultiplier(
        EResourceType ProducedType,
        const std::array<EResourceType, GProductionInputSlotCount>& InputTypes,
        const TradePolicy::FExportTradePolicy& ExportPolicy,
        const TradePolicy::FImportTradePolicy& ImportPolicy)
    {
        const int BiasPercent =
            ComputeBuildingProductionBiasPercent(
                ProducedType,
                InputTypes,
                ExportPolicy,
                ImportPolicy);
        const float Multiplier =
            1.f + static_cast<float>(BiasPercent) / 100.f;
        return (std::max)(0.72f, (std::min)(1.28f, Multiplier));
    }

    inline int ComputeExportBudgetBiasPercent(
        const TradePolicy::FExportTradePolicy& ExportPolicy)
    {
        int Bias =
            ExportPolicy.PrioritizeHighValueCargo ? 4 : 2;
        int BlockedCount = 0;

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (IsExportableResourceType(ResourceType) &&
                !TradePolicy::IsResourceExportAllowed(
                    ExportPolicy,
                    ResourceType))
            {
                ++BlockedCount;
            }
        }

        Bias -= (std::min)(2, BlockedCount);
        return ClampBiasPercent(Bias, -8, 8);
    }

    inline int ComputeImportBudgetBiasPercent(
        const TradePolicy::FImportTradePolicy& ImportPolicy)
    {
        int Bias = 0;

        switch (ImportPolicy.Mode)
        {
        case TradePolicy::EImportPolicyMode::None:
            Bias -= 2;
            break;
        case TradePolicy::EImportPolicyMode::SingleResource:
            Bias -= 4;
            break;
        case TradePolicy::EImportPolicyMode::AllResources:
        default:
            Bias += 1;
            break;
        }

        switch (TradePolicy::GetImportMaxUnitsPerResource(ImportPolicy))
        {
        case 500:
            Bias -= 2;
            break;
        case 3000:
            Bias += 1;
            break;
        case 6000:
            Bias += 2;
            break;
        default:
            break;
        }

        const int DailyBudgetCap =
            TradePolicy::GetDailyImportBudgetCap(ImportPolicy);

        if (DailyBudgetCap <= 0)
        {
            Bias += 4;
        }
        else if (DailyBudgetCap <= 12000)
        {
            Bias -= 4;
        }
        else if (DailyBudgetCap >= 36000)
        {
            Bias += 3;
        }

        if (TradePolicy::AllowsEmergencyImports(ImportPolicy))
            Bias += 6;

        return ClampBiasPercent(Bias, -10, 14);
    }

    inline long long ComputeDailyTradePolicyBudgetDelta(
        const TradePolicy::FExportTradePolicy& ExportPolicy,
        const TradePolicy::FImportTradePolicy& ImportPolicy,
        long long ExportIncome,
        long long ImportExpense)
    {
        (void)ImportPolicy;
        (void)ImportExpense;
        const int ExportBiasPercent =
            ComputeExportBudgetBiasPercent(ExportPolicy);
        const long long Delta =
            ExportIncome * static_cast<long long>(ExportBiasPercent) / 100ll;

        long long AdministrativeDelta =
            ExportPolicy.PrioritizeHighValueCargo ? 60ll : 20ll;
        long long Result = Delta + AdministrativeDelta;

        if (Result < 0)
        {
            Result = static_cast<long long>(std::llround(
                static_cast<double>(Result) *
                static_cast<double>((std::max)(
                    0.f,
                    GameConstants::Economy::
                        NegativeTradePolicyBudgetDeltaMultiplier))));
        }

        return Result;
    }
}
