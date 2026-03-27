#pragma once

#include "../Building/BuildingTypes.h"
#include "TradeDiplomacyRuntime.h"
#include "../GameConstants.h"
#include "../Politics/PoliticalTypes.h"
#include "../World/WorldStatsSnapshot.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace ResourceTradePricing
{
    constexpr int GMarketHistoryLength = 12;

    struct FTradePrice
    {
        int ExportUnitPrice = 0;
        int ImportUnitPrice = 0;
    };

    struct FResourceMarketPricePoint
    {
        int CurrentExportUnitPrice = 0;
        int CurrentImportUnitPrice = 0;
        int PreviousExportUnitPrice = 0;
        int PreviousImportUnitPrice = 0;
        int ExportPriceIndexPercent = 100;
        int ImportPriceIndexPercent = 100;
        int StorageBiasPercent = 0;
        int BalanceBiasPercent = 0;
        int TemporalBiasPercent = 0;
        int EventBiasPercent = 0;
        int DiplomacyExportBiasPercent = 0;
        int DiplomacyImportBiasPercent = 0;
        int EdictExportBiasPercent = 0;
        int EdictImportBiasPercent = 0;
        std::array<int, GMarketHistoryLength> ExportPriceHistory = {};
        std::array<int, GMarketHistoryLength> ImportPriceHistory = {};
        int HistoryCount = 0;
    };

    struct FWorldMarketPriceState
    {
        bool Initialized = false;
        int LastUpdatedYear = 0;
        int LastUpdatedMonth = 0;
        int LastUpdatedDay = 0;
        std::array<
            FResourceMarketPricePoint,
            static_cast<size_t>(EResourceType::Count)> Resources = {};
    };

    inline FTradePrice GetTradePrice(EResourceType Type)
    {
        switch (Type)
        {
        case EResourceType::Coconuts:
            return { 1, 2 };
        case EResourceType::Logs:
            return { 2, 2 };
        case EResourceType::Fish:
            return { 2, 2 };
        case EResourceType::Crops:
            return { 3, 4 };
        case EResourceType::AnimalProducts:
            return { 4, 6 };
        case EResourceType::Ore:
            return { 4, 6 };
        case EResourceType::Oil:
            return { 3, 4 };
        case EResourceType::FarmedFish:
            return { 2, 3 };
        case EResourceType::HydroponicProduce:
            return { 4, 6 };
        case EResourceType::FactoryLivestock:
            return { 5, 7 };
        case EResourceType::Banana:
            return { 2, 2 };
        case EResourceType::Cocoa:
            return { 2, 3 };
        case EResourceType::Coffee:
            return { 2, 3 };
        case EResourceType::Corn:
            return { 2, 2 };
        case EResourceType::Cotton:
            return { 2, 3 };
        case EResourceType::Pineapple:
            return { 2, 2 };
        case EResourceType::Rubber:
            return { 3, 4 };
        case EResourceType::Sugar:
            return { 2, 3 };
        case EResourceType::Tobacco:
            return { 2, 3 };
        case EResourceType::Meat:
            return { 2, 3 };
        case EResourceType::Milk:
            return { 2, 3 };
        case EResourceType::Hides:
            return { 2, 3 };
        case EResourceType::Wool:
            return { 2, 3 };
        case EResourceType::Coal:
            return { 2, 3 };
        case EResourceType::Iron:
            return { 2, 3 };
        case EResourceType::Gold:
            return { 4, 6 };
        case EResourceType::Nickel:
            return { 3, 4 };
        case EResourceType::Aluminum:
            return { 3, 4 };
        case EResourceType::Uranium:
            return { 3, 5 };
        case EResourceType::Planks:
            return { 5, 7 };
        case EResourceType::Rum:
            return { 6, 8 };
        case EResourceType::Leather:
            return { 6, 8 };
        case EResourceType::CannedGoods:
            return { 6, 9 };
        case EResourceType::Cheese:
            return { 6, 9 };
        case EResourceType::Cigars:
            return { 8, 11 };
        case EResourceType::Boats:
            return { 9, 13 };
        case EResourceType::Steel:
            return { 7, 10 };
        case EResourceType::Textiles:
            return { 7, 10 };
        case EResourceType::Weapons:
            return { 10, 14 };
        case EResourceType::Chocolate:
            return { 8, 11 };
        case EResourceType::Furniture:
            return { 8, 11 };
        case EResourceType::Jewelry:
            return { 11, 15 };
        case EResourceType::Plastic:
            return { 7, 10 };
        case EResourceType::Cars:
            return { 12, 17 };
        case EResourceType::Electronics:
            return { 12, 18 };
        case EResourceType::Apparel:
            return { 9, 13 };
        case EResourceType::Pharmaceuticals:
            return { 11, 16 };
        case EResourceType::Juice:
            return { 7, 10 };
        // Phase-2 special goods: placeholder prices only. These stay hidden
        // from the current vanilla-core trade/policy surface until the full
        // Vertical Farm / Synthetic Food Lab / DLC production loops are in.
        case EResourceType::SpecialChocolate:
            return { 10, 14 };
        case EResourceType::Goldnuts:
            return { 10, 14 };
        case EResourceType::BS:
            return { 5, 7 };
        default:
            return {};
        }
    }

    inline FTradePrice GetConfiguredTradePrice(EResourceType Type)
    {
        FTradePrice Result = GetTradePrice(Type);

        if (Result.ExportUnitPrice > 0)
        {
            Result.ExportUnitPrice = (std::max)(
                1,
                static_cast<int>(std::lround(
                    static_cast<double>(Result.ExportUnitPrice) *
                    static_cast<double>((std::max)(
                        0.f,
                        GameConstants::Economy::ExportPriceBaseMultiplier)))));
        }

        return Result;
    }

    inline int GetBaseExportPricePerStockUnit(EResourceType Type)
    {
        return GetConfiguredTradePrice(Type).ExportUnitPrice;
    }

    inline int GetBaseImportPricePerStockUnit(EResourceType Type)
    {
        return GetConfiguredTradePrice(Type).ImportUnitPrice;
    }

    inline FWorldMarketPriceState BuildDefaultWorldMarketPriceState()
    {
        FWorldMarketPriceState Result;

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);
            const FTradePrice BasePrice = GetConfiguredTradePrice(ResourceType);
            FResourceMarketPricePoint& Point =
                Result.Resources[static_cast<size_t>(ResourceType)];
            Point.CurrentExportUnitPrice = BasePrice.ExportUnitPrice;
            Point.CurrentImportUnitPrice = BasePrice.ImportUnitPrice;
            Point.PreviousExportUnitPrice = BasePrice.ExportUnitPrice;
            Point.PreviousImportUnitPrice = BasePrice.ImportUnitPrice;
            Point.ExportPriceIndexPercent =
                BasePrice.ExportUnitPrice > 0 ? 100 : 0;
            Point.ImportPriceIndexPercent =
                BasePrice.ImportUnitPrice > 0 ? 100 : 0;
            Point.ExportPriceHistory.fill(BasePrice.ExportUnitPrice);
            Point.ImportPriceHistory.fill(BasePrice.ImportUnitPrice);
            Point.HistoryCount = 1;
        }

        return Result;
    }

    inline FWorldMarketPriceState& GetActiveWorldMarketPriceState()
    {
        static FWorldMarketPriceState State =
            BuildDefaultWorldMarketPriceState();
        return State;
    }

    inline void ResetWorldMarketPriceState()
    {
        GetActiveWorldMarketPriceState() = BuildDefaultWorldMarketPriceState();
    }

    // 시나리오: 왕실 착취로 인한 럼주 수출가 보정 (0=없음, -50=−50%)
    inline int& GetScenarioRumExportBiasPercent()
    {
        static int Value = 0;
        return Value;
    }

    inline double ClampMultiplier(double Value, double Min, double Max)
    {
        return (std::max)(Min, (std::min)(Max, Value));
    }

    inline double ComputeTemporalMarketBias(
        EResourceType Type,
        int Year,
        int Month,
        int Day)
    {
        const int ResourceIndex = static_cast<int>(Type);
        const double DayStamp =
            static_cast<double>(Year * 372 + Month * 31 + Day);
        const double WaveA = std::sin(
            (DayStamp + static_cast<double>(ResourceIndex) * 11.0) * 0.173) *
            0.08;
        const double WaveB = std::cos(
            (DayStamp + static_cast<double>(ResourceIndex) * 7.0) * 0.111) *
            0.06;
        return WaveA + WaveB;
    }

    inline double ComputeStorageMarketBias(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        EResourceType Type)
    {
        const size_t ResourceIndex = static_cast<size_t>(Type);

        if (ResourceIndex >= Snapshot.ResourceTypes.size())
            return 0.0;

        const WorldStats::FResourceTypeSnapshot& ResourceSnapshot =
            Snapshot.ResourceTypes[ResourceIndex];
        const double CapacityRatio =
            ResourceSnapshot.Capacity > 0 ?
                static_cast<double>(ResourceSnapshot.AvailableStock) /
                    static_cast<double>(ResourceSnapshot.Capacity) :
                0.0;
        return ClampMultiplier(
            (0.42 - CapacityRatio) * 0.22,
            -0.14,
            0.14);
    }

    inline double ComputeProducerConsumerMarketBias(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        EResourceType Type)
    {
        const size_t ResourceIndex = static_cast<size_t>(Type);

        if (ResourceIndex >= Snapshot.ResourceTypes.size())
            return 0.0;

        const WorldStats::FResourceTypeSnapshot& ResourceSnapshot =
            Snapshot.ResourceTypes[ResourceIndex];
        const double SupplyBalance =
            static_cast<double>(ResourceSnapshot.ConsumerBuildingCount) -
            static_cast<double>(ResourceSnapshot.ProducerBuildingCount);
        const double SupplyWeight =
            static_cast<double>(
                ResourceSnapshot.ConsumerBuildingCount +
                ResourceSnapshot.ProducerBuildingCount + 2);
        return ClampMultiplier(
            (SupplyBalance / SupplyWeight) * 0.18,
            -0.10,
            0.10);
    }

    inline double ComputeEventMarketBias(
        EResourceType Type,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const FWorldCrisisStatus& WorldCrisisStatus)
    {
        static_cast<void>(Type);
        static_cast<void>(TaxEventStatus);
        static_cast<void>(WorldCrisisStatus);
        return 0.0;
    }

    inline void UpdateWorldMarketPriceState(
        FWorldMarketPriceState& InOutState,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const FWorldCrisisStatus& WorldCrisisStatus,
        const std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount>* ForeignPowerStates,
        int Year,
        int Month,
        int Day)
    {
        if (InOutState.LastUpdatedYear == Year &&
            InOutState.LastUpdatedMonth == Month &&
            InOutState.LastUpdatedDay == Day &&
            InOutState.Initialized)
        {
            return;
        }

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);
            const FTradePrice BasePrice = GetConfiguredTradePrice(ResourceType);

            if (BasePrice.ExportUnitPrice <= 0 ||
                BasePrice.ImportUnitPrice <= 0)
            {
                continue;
            }

            FResourceMarketPricePoint& Point =
                InOutState.Resources[static_cast<size_t>(ResourceType)];
            const double TemporalBias =
                ComputeTemporalMarketBias(ResourceType, Year, Month, Day);
            const double StorageBias =
                ComputeStorageMarketBias(Snapshot, ResourceType);
            const double BalanceBias =
                ComputeProducerConsumerMarketBias(Snapshot, ResourceType);
            const double EventBias =
                ComputeEventMarketBias(
                    ResourceType,
                    TaxEventStatus,
                    WorldCrisisStatus);
            const int DiplomacyExportBiasPercent = ForeignPowerStates ?
                TradeDiplomacyRuntime::ComputeDiplomacyExportBiasPercent(
                    ResourceType,
                    *ForeignPowerStates) :
                TradeDiplomacyRuntime::ComputeDiplomacyExportBiasPercent(
                    ResourceType,
                    Snapshot,
                    GovernmentProfile,
                    TaxEventStatus,
                    GovernmentEdictStates);
            const int DiplomacyImportBiasPercent = ForeignPowerStates ?
                TradeDiplomacyRuntime::ComputeDiplomacyImportBiasPercent(
                    ResourceType,
                    *ForeignPowerStates) :
                TradeDiplomacyRuntime::ComputeDiplomacyImportBiasPercent(
                    ResourceType,
                    Snapshot,
                    GovernmentProfile,
                    TaxEventStatus,
                    GovernmentEdictStates);
            const int EdictExportBiasPercent =
                TradeDiplomacyRuntime::ComputeEdictExportBiasPercent(
                    ResourceType,
                    GovernmentEdictStates);
            const int EdictImportBiasPercent =
                TradeDiplomacyRuntime::ComputeEdictImportBiasPercent(
                    ResourceType,
                    GovernmentEdictStates);
            const double ExportMultiplier = ClampMultiplier(
                1.0 +
                    TemporalBias +
                    StorageBias +
                    BalanceBias +
                    EventBias +
                    static_cast<double>(DiplomacyExportBiasPercent) / 100.0 +
                    static_cast<double>(EdictExportBiasPercent) / 100.0,
                0.65,
                1.60);
            const double ImportMultiplier = ClampMultiplier(
                1.0 +
                    TemporalBias * 0.9 +
                    StorageBias * 1.1 +
                    BalanceBias * 1.2 +
                    EventBias * 0.8 +
                    static_cast<double>(DiplomacyImportBiasPercent) / 100.0 +
                    static_cast<double>(EdictImportBiasPercent) / 100.0 +
                    ((StorageBias + BalanceBias) > 0.0 ? 0.05 : 0.02),
                0.75,
                1.85);
            const int TargetExportUnitPrice = (std::max)(
                1,
                static_cast<int>(std::lround(
                    static_cast<double>(BasePrice.ExportUnitPrice) *
                    ExportMultiplier)));
            const int TargetImportUnitPrice = (std::max)(
                TargetExportUnitPrice + 1,
                static_cast<int>(std::lround(
                    static_cast<double>(BasePrice.ImportUnitPrice) *
                    ImportMultiplier)));

            // 시나리오 럼주 착취 바이어스 적용
            int ScenarioAdjustedExportPrice = TargetExportUnitPrice;
            int ScenarioAdjustedImportPrice = TargetImportUnitPrice;
            if (ResourceType == EResourceType::Rum)
            {
                const int RumBias = GetScenarioRumExportBiasPercent();
                if (RumBias != 0)
                {
                    ScenarioAdjustedExportPrice = (std::max)(
                        1,
                        ScenarioAdjustedExportPrice * (100 + RumBias) / 100);
                    ScenarioAdjustedImportPrice = (std::max)(
                        ScenarioAdjustedExportPrice + 1,
                        ScenarioAdjustedImportPrice * (100 + RumBias) / 100);
                }
            }

            if (!InOutState.Initialized)
            {
                Point.CurrentExportUnitPrice = ScenarioAdjustedExportPrice;
                Point.CurrentImportUnitPrice = ScenarioAdjustedImportPrice;
                Point.PreviousExportUnitPrice = ScenarioAdjustedExportPrice;
                Point.PreviousImportUnitPrice = ScenarioAdjustedImportPrice;
            }
            else
            {
                Point.PreviousExportUnitPrice = Point.CurrentExportUnitPrice;
                Point.PreviousImportUnitPrice = Point.CurrentImportUnitPrice;
                Point.CurrentExportUnitPrice = (std::max)(
                    1,
                    static_cast<int>(std::lround(
                        static_cast<double>(Point.CurrentExportUnitPrice) *
                            0.68 +
                        static_cast<double>(ScenarioAdjustedExportPrice) *
                            0.32)));
                Point.CurrentImportUnitPrice = (std::max)(
                    Point.CurrentExportUnitPrice + 1,
                    static_cast<int>(std::lround(
                        static_cast<double>(Point.CurrentImportUnitPrice) *
                            0.65 +
                        static_cast<double>(ScenarioAdjustedImportPrice) *
                            0.35)));
            }

            Point.ExportPriceIndexPercent = static_cast<int>(std::lround(
                static_cast<double>(Point.CurrentExportUnitPrice) * 100.0 /
                static_cast<double>(BasePrice.ExportUnitPrice)));
            Point.ImportPriceIndexPercent = static_cast<int>(std::lround(
                static_cast<double>(Point.CurrentImportUnitPrice) * 100.0 /
                static_cast<double>(BasePrice.ImportUnitPrice)));
            Point.StorageBiasPercent = static_cast<int>(std::lround(
                StorageBias * 100.0));
            Point.BalanceBiasPercent = static_cast<int>(std::lround(
                BalanceBias * 100.0));
            Point.TemporalBiasPercent = static_cast<int>(std::lround(
                TemporalBias * 100.0));
            Point.EventBiasPercent = static_cast<int>(std::lround(
                EventBias * 100.0));
            Point.DiplomacyExportBiasPercent = DiplomacyExportBiasPercent;
            Point.DiplomacyImportBiasPercent = DiplomacyImportBiasPercent;
            Point.EdictExportBiasPercent = EdictExportBiasPercent;
            Point.EdictImportBiasPercent = EdictImportBiasPercent;

            if (!InOutState.Initialized)
            {
                Point.ExportPriceHistory.fill(Point.CurrentExportUnitPrice);
                Point.ImportPriceHistory.fill(Point.CurrentImportUnitPrice);
                Point.HistoryCount = 1;
            }
            else
            {
                for (int HistoryIndex = 0;
                    HistoryIndex < GMarketHistoryLength - 1;
                    ++HistoryIndex)
                {
                    Point.ExportPriceHistory[static_cast<size_t>(HistoryIndex)] =
                        Point.ExportPriceHistory[
                            static_cast<size_t>(HistoryIndex + 1)];
                    Point.ImportPriceHistory[static_cast<size_t>(HistoryIndex)] =
                        Point.ImportPriceHistory[
                            static_cast<size_t>(HistoryIndex + 1)];
                }

                Point.ExportPriceHistory[
                    static_cast<size_t>(GMarketHistoryLength - 1)] =
                    Point.CurrentExportUnitPrice;
                Point.ImportPriceHistory[
                    static_cast<size_t>(GMarketHistoryLength - 1)] =
                    Point.CurrentImportUnitPrice;
                Point.HistoryCount = (std::min)(
                    GMarketHistoryLength,
                    Point.HistoryCount + 1);
            }
        }

        InOutState.Initialized = true;
        InOutState.LastUpdatedYear = Year;
        InOutState.LastUpdatedMonth = Month;
        InOutState.LastUpdatedDay = Day;
    }

    inline void UpdateWorldMarketPrices(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const FWorldCrisisStatus& WorldCrisisStatus,
        int Year,
        int Month,
        int Day)
    {
        UpdateWorldMarketPriceState(
            GetActiveWorldMarketPriceState(),
            Snapshot,
            GovernmentProfile,
            GovernmentEdictStates,
            TaxEventStatus,
            WorldCrisisStatus,
            nullptr,
            Year,
            Month,
            Day);
    }

    inline void UpdateWorldMarketPrices(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const FWorldCrisisStatus& WorldCrisisStatus,
        const std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount>& ForeignPowerStates,
        int Year,
        int Month,
        int Day)
    {
        UpdateWorldMarketPriceState(
            GetActiveWorldMarketPriceState(),
            Snapshot,
            GovernmentProfile,
            GovernmentEdictStates,
            TaxEventStatus,
            WorldCrisisStatus,
            &ForeignPowerStates,
            Year,
            Month,
            Day);
    }

    inline const FResourceMarketPricePoint& GetMarketPricePoint(
        EResourceType Type)
    {
        return GetActiveWorldMarketPriceState().Resources[
            static_cast<size_t>(Type)];
    }

    inline int GetExportPricePerStockUnit(EResourceType Type)
    {
        const FTradePrice BasePrice = GetConfiguredTradePrice(Type);

        if (BasePrice.ExportUnitPrice <= 0)
            return 0;

        int MarketPrice =
            GetMarketPricePoint(Type).CurrentExportUnitPrice;

        int ScenarioBias = 0;
        if (Type == EResourceType::Rum)
            ScenarioBias = GetScenarioRumExportBiasPercent();

        // 시나리오 럼주 착취 바이어스를 즉시 반영 (lerp 대기 없이)
        if (ScenarioBias != 0 && MarketPrice > 0)
        {
            MarketPrice =
                (std::max)(1, MarketPrice * (100 + ScenarioBias) / 100);
        }

        if (MarketPrice > 0)
            return MarketPrice;

        if (ScenarioBias != 0)
        {
            return (std::max)(
                1,
                BasePrice.ExportUnitPrice * (100 + ScenarioBias) / 100);
        }

        return BasePrice.ExportUnitPrice;
    }

    inline int GetImportPricePerStockUnit(EResourceType Type)
    {
        const FTradePrice BasePrice = GetConfiguredTradePrice(Type);

        if (BasePrice.ImportUnitPrice <= 0)
            return 0;

        const int MarketPrice =
            GetMarketPricePoint(Type).CurrentImportUnitPrice;
        return MarketPrice > 0 ? MarketPrice : BasePrice.ImportUnitPrice;
    }

    inline int GetPreviousExportPricePerStockUnit(EResourceType Type)
    {
        const FTradePrice BasePrice = GetConfiguredTradePrice(Type);
        const int PreviousPrice =
            GetMarketPricePoint(Type).PreviousExportUnitPrice;
        return PreviousPrice > 0 ? PreviousPrice : BasePrice.ExportUnitPrice;
    }

    inline int GetPreviousImportPricePerStockUnit(EResourceType Type)
    {
        const FTradePrice BasePrice = GetConfiguredTradePrice(Type);
        const int PreviousPrice =
            GetMarketPricePoint(Type).PreviousImportUnitPrice;
        return PreviousPrice > 0 ? PreviousPrice : BasePrice.ImportUnitPrice;
    }

    inline int GetExportPriceIndexPercent(EResourceType Type)
    {
        const int IndexPercent =
            GetMarketPricePoint(Type).ExportPriceIndexPercent;
        return IndexPercent > 0 ? IndexPercent :
            (GetBaseExportPricePerStockUnit(Type) > 0 ? 100 : 0);
    }

    inline int GetImportPriceIndexPercent(EResourceType Type)
    {
        const int IndexPercent =
            GetMarketPricePoint(Type).ImportPriceIndexPercent;
        return IndexPercent > 0 ? IndexPercent :
            (GetBaseImportPricePerStockUnit(Type) > 0 ? 100 : 0);
    }

    inline int GetExportPriceDeltaPercent(EResourceType Type)
    {
        const int Previous = GetPreviousExportPricePerStockUnit(Type);
        const int Current = GetExportPricePerStockUnit(Type);

        if (Previous <= 0)
            return 0;

        return static_cast<int>(std::lround(
            (static_cast<double>(Current - Previous) * 100.0) /
            static_cast<double>(Previous)));
    }

    inline int GetStorageBiasPercent(EResourceType Type)
    {
        return GetMarketPricePoint(Type).StorageBiasPercent;
    }

    inline int GetBalanceBiasPercent(EResourceType Type)
    {
        return GetMarketPricePoint(Type).BalanceBiasPercent;
    }

    inline int GetTemporalBiasPercent(EResourceType Type)
    {
        return GetMarketPricePoint(Type).TemporalBiasPercent;
    }

    inline int GetEventBiasPercent(EResourceType Type)
    {
        return GetMarketPricePoint(Type).EventBiasPercent;
    }

    inline int GetDiplomacyExportBiasPercent(EResourceType Type)
    {
        return GetMarketPricePoint(Type).DiplomacyExportBiasPercent;
    }

    inline int GetDiplomacyImportBiasPercent(EResourceType Type)
    {
        return GetMarketPricePoint(Type).DiplomacyImportBiasPercent;
    }

    inline int GetEdictExportBiasPercent(EResourceType Type)
    {
        return GetMarketPricePoint(Type).EdictExportBiasPercent;
    }

    inline int GetEdictImportBiasPercent(EResourceType Type)
    {
        return GetMarketPricePoint(Type).EdictImportBiasPercent;
    }

    inline int GetPriceHistoryCount(EResourceType Type)
    {
        const int Count = GetMarketPricePoint(Type).HistoryCount;
        return (std::max)(1, (std::min)(GMarketHistoryLength, Count));
    }

    inline int GetExportPriceHistoryPoint(EResourceType Type, int Index)
    {
        if (Index < 0 || Index >= GMarketHistoryLength)
            return GetExportPricePerStockUnit(Type);

        return GetMarketPricePoint(Type).ExportPriceHistory[
            static_cast<size_t>(Index)];
    }

    inline int GetImportPriceHistoryPoint(EResourceType Type, int Index)
    {
        if (Index < 0 || Index >= GMarketHistoryLength)
            return GetImportPricePerStockUnit(Type);

        return GetMarketPricePoint(Type).ImportPriceHistory[
            static_cast<size_t>(Index)];
    }

    inline int GetImportPriceDeltaPercent(EResourceType Type)
    {
        const int Previous = GetPreviousImportPricePerStockUnit(Type);
        const int Current = GetImportPricePerStockUnit(Type);

        if (Previous <= 0)
            return 0;

        return static_cast<int>(std::lround(
            (static_cast<double>(Current - Previous) * 100.0) /
            static_cast<double>(Previous)));
    }

    inline long long ComputeExportValue(EResourceType Type, int Amount)
    {
        if (Amount <= 0)
            return 0;

        return static_cast<long long>(Amount) *
            static_cast<long long>(GetExportPricePerStockUnit(Type));
    }

    inline long long ComputeImportCost(EResourceType Type, int Amount)
    {
        if (Amount <= 0)
            return 0;

        return static_cast<long long>(Amount) *
            static_cast<long long>(GetImportPricePerStockUnit(Type));
    }
}
