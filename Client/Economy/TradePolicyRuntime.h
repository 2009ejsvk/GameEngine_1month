#pragma once

#include "../Building/BuildingTypes.h"
#include "TradePolicy.h"
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

            switch (TradePolicy::GetDomesticReserveBufferUnits(ExportPolicy))
            {
            case 0:
                if (MarketClass == EResourceMarketClass::Food)
                    Bias -= 3;
                else if (MarketClass == EResourceMarketClass::RawGoods)
                    Bias += 2;
                else if (MarketClass == EResourceMarketClass::ManufacturedGoods)
                    Bias += 3;
                else if (MarketClass == EResourceMarketClass::LuxuryGoods)
                    Bias += 4;
                break;
            case 3000:
                if (MarketClass == EResourceMarketClass::Food)
                    Bias += 4;
                else if (MarketClass == EResourceMarketClass::RawGoods)
                    Bias += 1;
                else if (MarketClass == EResourceMarketClass::ManufacturedGoods)
                    Bias -= 1;
                else if (MarketClass == EResourceMarketClass::LuxuryGoods)
                    Bias -= 2;
                break;
            case 6000:
                if (MarketClass == EResourceMarketClass::Food)
                    Bias += 8;
                else if (MarketClass == EResourceMarketClass::RawGoods)
                    Bias += 2;
                else if (MarketClass == EResourceMarketClass::ManufacturedGoods)
                    Bias -= 3;
                else if (MarketClass == EResourceMarketClass::LuxuryGoods)
                    Bias -= 5;
                break;
            default:
                break;
            }
        }
        else
        {
            switch (MarketClass)
            {
            case EResourceMarketClass::Food:
                Bias +=
                    TradePolicy::GetDomesticReserveBufferUnits(ExportPolicy) >=
                        3000 ?
                    4 :
                    -1;
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
        int Bias =
            ComputeExportProductionBiasPercent(
                ProducedType,
                ExportPolicy);
        int InputBiasTotal = 0;
        int InputCount = 0;

        auto AccumulateInputBias = [&](EResourceType InputType)
        {
            if (InputType == EResourceType::None)
                return;

            InputBiasTotal +=
                ComputeImportInputBiasPercent(
                    InputType,
                    ImportPolicy);
            ++InputCount;
        };

        for (int SlotIndex = 0;
            SlotIndex < GProductionInputSlotCount;
            ++SlotIndex)
        {
            AccumulateInputBias(
                InputTypes[static_cast<size_t>(SlotIndex)]);
        }

        if (InputCount > 0)
        {
            Bias +=
                static_cast<int>((InputBiasTotal + (InputCount / 2)) / InputCount);

            if (ImportPolicy.Mode == TradePolicy::EImportPolicyMode::None)
                Bias -= 2;
        }

        return ClampBiasPercent(Bias, -22, 22);
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
        const int ReserveBufferUnits =
            TradePolicy::GetDomesticReserveBufferUnits(ExportPolicy);

        if (ReserveBufferUnits <= 0)
        {
            Bias += 2;
        }
        else if (ReserveBufferUnits >= 6000)
        {
            Bias -= 4;
        }
        else if (ReserveBufferUnits >= 3000)
        {
            Bias -= 1;
        }

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
        const int ExportBiasPercent =
            ComputeExportBudgetBiasPercent(ExportPolicy);
        const int ImportBiasPercent =
            ComputeImportBudgetBiasPercent(ImportPolicy);
        long long Delta =
            ExportIncome * static_cast<long long>(ExportBiasPercent) / 100ll;
        Delta -=
            ImportExpense * static_cast<long long>(ImportBiasPercent) / 100ll;

        long long AdministrativeDelta =
            ExportPolicy.PrioritizeHighValueCargo ? 60ll : 20ll;
        const int ReserveBufferUnits =
            TradePolicy::GetDomesticReserveBufferUnits(ExportPolicy);

        if (ReserveBufferUnits >= 6000)
            AdministrativeDelta -= 140ll;
        else if (ReserveBufferUnits >= 3000)
            AdministrativeDelta -= 60ll;
        else if (ReserveBufferUnits <= 0)
            AdministrativeDelta += 40ll;

        switch (ImportPolicy.Mode)
        {
        case TradePolicy::EImportPolicyMode::None:
            AdministrativeDelta += 110ll;
            break;
        case TradePolicy::EImportPolicyMode::SingleResource:
            AdministrativeDelta += 50ll;
            break;
        case TradePolicy::EImportPolicyMode::AllResources:
        default:
            AdministrativeDelta -= 20ll;
            break;
        }

        const int DailyBudgetCap =
            TradePolicy::GetDailyImportBudgetCap(ImportPolicy);

        if (DailyBudgetCap <= 0)
            AdministrativeDelta -= 80ll;
        else if (DailyBudgetCap <= 12000)
            AdministrativeDelta += 90ll;
        else if (DailyBudgetCap <= 24000)
            AdministrativeDelta += 20ll;
        else
            AdministrativeDelta -= 40ll;

        if (TradePolicy::AllowsEmergencyImports(ImportPolicy))
            AdministrativeDelta -= 160ll;

        return Delta + AdministrativeDelta;
    }
}
