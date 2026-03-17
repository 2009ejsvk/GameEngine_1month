#include "TradeWidgetRuntime.h"
#include "../Economy/ResourceTradePricing.h"
#include "../Economy/TradeDiplomacyRuntime.h"
#include "../World/MainWorldTradeAccess.h"
#include "../World/MainWorldUiReadAccess.h"
#include "../World/MainWorldTradeRuntime.h"
#include "../World/WorldStatsSnapshot.h"
#include "World/World.h"
#include <algorithm>
#include <cmath>

namespace
{
    using namespace TradeWidgetRuntime;

    int RoundDownToThousand(int Value)
    {
        if (Value < 1000)
            return 0;

        return (Value / 1000) * 1000;
    }

    int RoundUpToThousand(int Value)
    {
        if (Value <= 0)
            return 0;

        return ((Value + 999) / 1000) * 1000;
    }

    double ClampDouble(double Value, double MinValue, double MaxValue)
    {
        return (std::max)(MinValue, (std::min)(MaxValue, Value));
    }

    ETradePageType ResolvePageType(int SelectedPageIndex)
    {
        return static_cast<ETradePageType>(ClampInt(
            SelectedPageIndex,
            0,
            static_cast<int>(ETradePageType::Count) - 1));
    }

    double ResolvePartnerWeight(
        EResourceMarketClass MarketClass,
        int ForeignPowerIndex)
    {
        static const std::array<double, 5> FoodWeights =
        {
            0.15, 0.10, 0.15, 0.35, 0.25
        };
        static const std::array<double, 5> RawWeights =
        {
            0.40, 0.30, 0.05, 0.15, 0.10
        };
        static const std::array<double, 5> ProcessedWeights =
        {
            0.35, 0.20, 0.20, 0.05, 0.20
        };
        static const std::array<double, 5> LuxuryWeights =
        {
            0.10, 0.00, 0.35, 0.20, 0.35
        };
        static const std::array<double, 5> DefaultWeights =
        {
            0.20, 0.20, 0.20, 0.20, 0.20
        };

        const int SafeIndex = ClampInt(ForeignPowerIndex, 0, 4);

        switch (MarketClass)
        {
        case EResourceMarketClass::Food:
            return FoodWeights[static_cast<size_t>(SafeIndex)];
        case EResourceMarketClass::RawGoods:
            return RawWeights[static_cast<size_t>(SafeIndex)];
        case EResourceMarketClass::ManufacturedGoods:
            return ProcessedWeights[static_cast<size_t>(SafeIndex)];
        case EResourceMarketClass::LuxuryGoods:
            return LuxuryWeights[static_cast<size_t>(SafeIndex)];
        default:
            return DefaultWeights[static_cast<size_t>(SafeIndex)];
        }
    }

    bool MatchesFilter(ETradeFilterType FilterType, EResourceType Type)
    {
        switch (FilterType)
        {
        case ETradeFilterType::All:
            return true;
        case ETradeFilterType::Food:
            return GetResourceMarketClass(Type) == EResourceMarketClass::Food;
        case ETradeFilterType::ConsumerGoods:
            return Type == EResourceType::CannedGoods ||
                Type == EResourceType::Cheese ||
                Type == EResourceType::Textiles ||
                Type == EResourceType::Plastic ||
                Type == EResourceType::Apparel ||
                Type == EResourceType::Juice;
        case ETradeFilterType::LuxuryGoods:
            return GetResourceMarketClass(Type) == EResourceMarketClass::LuxuryGoods;
        case ETradeFilterType::Minerals:
            return Type == EResourceType::Ore ||
                IsMineOutputResourceType(Type) ||
                Type == EResourceType::Goldnuts ||
                Type == EResourceType::Oil ||
                Type == EResourceType::Steel ||
                Type == EResourceType::Jewelry;
        case ETradeFilterType::ProcessedResources:
            return GetResourceMarketClass(Type) ==
                    EResourceMarketClass::ManufacturedGoods ||
                Type == EResourceType::CannedGoods ||
                Type == EResourceType::Cheese;
        case ETradeFilterType::RawMaterials:
            return GetResourceMarketClass(Type) == EResourceMarketClass::RawGoods;
        case ETradeFilterType::LocalResources:
            return Type == EResourceType::Coconuts ||
                Type == EResourceType::Logs ||
                Type == EResourceType::Fish ||
                Type == EResourceType::Crops ||
                Type == EResourceType::AnimalProducts ||
                Type == EResourceType::Ore ||
                Type == EResourceType::Goldnuts ||
                Type == EResourceType::BS ||
                IsCropOutputResourceType(Type) ||
                IsAnimalOutputResourceType(Type) ||
                IsMineOutputResourceType(Type);
        case ETradeFilterType::PlantationGoods:
            return Type == EResourceType::Coconuts ||
                Type == EResourceType::Crops ||
                Type == EResourceType::HydroponicProduce ||
                Type == EResourceType::BS ||
                IsCropOutputResourceType(Type) ||
                Type == EResourceType::Rum ||
                Type == EResourceType::Cigars ||
                Type == EResourceType::Chocolate ||
                Type == EResourceType::SpecialChocolate ||
                Type == EResourceType::Juice;
        default:
            return true;
        }
    }

    int BuildMonthsUntilNextProposal(int CurrentMonth)
    {
        if (CurrentMonth <= 1)
            return 12;

        return (std::max)(1, 13 - CurrentMonth);
    }

    double ComputeOfferSeed(
        int SimulationYear,
        int SimulationMonth,
        EResourceType Type,
        int ForeignPowerIndex,
        bool ImportRoute)
    {
        const double Value = std::sin(
            (static_cast<double>(SimulationYear) * 37.0) +
            (static_cast<double>(SimulationMonth) * 11.0) +
            (static_cast<double>(static_cast<int>(Type)) * 9.0) +
            (static_cast<double>(ForeignPowerIndex) * 4.0) +
            (ImportRoute ? 17.0 : 0.0));
        return Value * 0.06;
    }

    int ApplyTradePriceModifierPercent(
        int PricePerThousand,
        int ModifierPercent)
    {
        if (PricePerThousand <= 0 || ModifierPercent == 0)
            return (std::max)(1000, PricePerThousand);

        return (std::max)(
            1000,
            static_cast<int>(std::lround(
                static_cast<double>(PricePerThousand) *
                (100.0 + static_cast<double>(ModifierPercent)) /
                100.0 / 50.0)) * 50);
    }

    FTradeProposal BuildTradeProposal(
        const std::shared_ptr<CWorld>& World,
        bool ImportRoute,
        EResourceType Type,
        int AvailabilityUnits,
        int Score,
        int SimulationYear,
        int SimulationMonth,
        EBuildingEra CurrentEra,
        const std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount>& ForeignPowers)
    {
        FTradeProposal Result;
        Result.ImportRoute = ImportRoute;
        Result.ResourceType = Type;
        Result.MarketClass = GetResourceMarketClass(Type);
        Result.CategoryName = GetResourceMarketClassDisplayName(Result.MarketClass);
        Result.AvailabilityUnits = AvailabilityUnits;
        const int BaseMaxAmount = ClampInt(
            ImportRoute ?
                RoundUpToThousand((std::max)(1000, AvailabilityUnits)) :
                RoundDownToThousand(AvailabilityUnits),
            1000,
            12000);
        Result.MaxAmount = BaseMaxAmount;
        Result.Score = Score;

        double BestScore = -1000000.0;
        int BestPartner = -1;

        for (int Index = 0;
            Index < TradeDiplomacyRuntime::GForeignPowerCount;
            ++Index)
        {
            if (!TradeDiplomacyRuntime::IsForeignPowerActiveForEra(
                    Index,
                    CurrentEra))
            {
                continue;
            }

            const auto& Partner = ForeignPowers[static_cast<size_t>(Index)];
            const double Weight = ResolvePartnerWeight(Result.MarketClass, Index);
            const double CandidateScore =
                static_cast<double>(Partner.Relation) * 0.72 +
                static_cast<double>(Partner.TradeModifier) * 5.25 +
                Weight * 28.0 +
                (ImportRoute ? Weight * 10.0 : Weight * 16.0);

            if (CandidateScore > BestScore)
            {
                BestScore = CandidateScore;
                BestPartner = Index;
            }
        }

        if (BestPartner < 0)
            BestPartner = 0;

        Result.ForeignPowerIndex = BestPartner;
        Result.PartnerName = MainWorldTradeRuntime::GetForeignPowerName(
            BestPartner,
            CurrentEra);

        const auto& Partner = ForeignPowers[static_cast<size_t>(BestPartner)];
        Result.Relation = Partner.Relation;
        Result.Standing = Partner.Standing;
        Result.TradeModifier = Partner.TradeModifier;
        const int StandardPricePerUnit = ImportRoute ?
            ResourceTradePricing::GetImportPricePerStockUnit(Type) :
            ResourceTradePricing::GetExportPricePerStockUnit(Type);
        const int StandardPricePerThousand =
            (std::max)(1000, StandardPricePerUnit * 1000);
        const double PartnerWeight =
            ResolvePartnerWeight(Result.MarketClass, BestPartner);
        const double RelationBias =
            static_cast<double>(Partner.Relation - 50) / 100.0;
        const double TradeBias =
            static_cast<double>(Partner.TradeModifier) / 100.0;
        const double SeedBias = ComputeOfferSeed(
            SimulationYear,
            SimulationMonth,
            Type,
            BestPartner,
            ImportRoute);
        const double VolumeMultiplier = ClampDouble(
            1.0 +
                static_cast<double>(Partner.Standing) / 220.0 +
                static_cast<double>(Partner.Relation - 50) / 280.0,
            0.70,
            1.35);
        Result.MaxAmount = ClampInt(
            ImportRoute ?
                RoundUpToThousand(static_cast<int>(std::lround(
                    static_cast<double>(BaseMaxAmount) * VolumeMultiplier))) :
                RoundDownToThousand(static_cast<int>(std::lround(
                    static_cast<double>(BaseMaxAmount) * VolumeMultiplier))),
            1000,
            16000);
        const double OfferMultiplier = ImportRoute ?
            ClampDouble(
                1.0 -
                    TradeBias * 0.34 -
                    RelationBias * 0.18 -
                    PartnerWeight * 0.14 +
                    SeedBias,
                0.78,
                1.18) :
            ClampDouble(
                1.0 +
                    TradeBias * 0.44 +
                    RelationBias * 0.24 +
                    PartnerWeight * 0.18 +
                    SeedBias,
                0.82,
                1.30);
        const int OfferPricePerThousand = (std::max)(
            1000,
            static_cast<int>(std::lround(
                static_cast<double>(StandardPricePerThousand) *
                OfferMultiplier / 50.0)) * 50);
        const auto TradeAccess =
            ResolveMainWorldTradeAccess(World);
        const int CustomsModifierPercent =
            TradeAccess ?
                (ImportRoute ?
                    TradeAccess->GetCustomsImportTradePriceModifierPercent() :
                    TradeAccess->GetCustomsExportTradePriceModifierPercent()) :
                0;

        Result.BasePricePerThousand = StandardPricePerThousand;
        Result.OfferPricePerThousand = ApplyTradePriceModifierPercent(
            OfferPricePerThousand,
            CustomsModifierPercent);

        if (StandardPricePerThousand > 0)
        {
            Result.MarginPercent = static_cast<int>(std::lround(
                ImportRoute ?
                    static_cast<double>(
                        StandardPricePerThousand - Result.OfferPricePerThousand) *
                        100.0 /
                        static_cast<double>(StandardPricePerThousand) :
                    static_cast<double>(
                        Result.OfferPricePerThousand - StandardPricePerThousand) *
                        100.0 /
                        static_cast<double>(StandardPricePerThousand)));
        }

        return Result;
    }

    std::vector<FTradeProposal> BuildTradeProposals(
        const std::shared_ptr<CWorld>& World,
        int SimulationYear,
        int SimulationMonth,
        const FGovernmentProfile& GovernmentProfile,
        const std::vector<FGovernmentEdictState>& GovernmentEdictStates,
        const FTaxPolicyEventStatus& TaxEventStatus)
    {
        std::vector<FTradeProposal> Result;

        if (!World)
            return Result;

        const WorldStats::FWorldStatsSnapshot Snapshot =
            WorldStats::BuildSnapshot(World);
        const auto TradeAccess =
            ResolveMainWorldTradeAccess(World);
        const EBuildingEra CurrentEra =
            TradeAccess ? TradeAccess->GetCurrentEra() : EBuildingEra::Modern;
        std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount> ForeignPowers = {};

        if (TradeAccess)
        {
            ForeignPowers = TradeAccess->GetForeignPowerStates();
        }
        else
        {
            ForeignPowers =
                TradeDiplomacyRuntime::BuildForeignPowerWorldStates(
                    Snapshot,
                    GovernmentProfile,
                    TaxEventStatus,
                    GovernmentEdictStates,
                    CurrentEra,
                    std::array<
                        TradeDiplomacyRuntime::FForeignPowerStandingState,
                        TradeDiplomacyRuntime::GForeignPowerCount>());
        }

        struct FMetric
        {
            EResourceType Type = EResourceType::None;
            int AvailabilityUnits = 0;
            int Score = 0;
        };

        std::vector<FMetric> ExportMetrics;
        std::vector<FMetric> ImportMetrics;

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsImmediateProductionScopeResourceType(ResourceType))
                continue;

            const auto& ResourceSnapshot =
                Snapshot.ResourceTypes[static_cast<size_t>(ResourceType)];
            const int ExportableUnits = RoundDownToThousand(
                ResourceSnapshot.HarborExportableStock);
            const int ImportUnits = ClampInt(
                RoundUpToThousand((std::max)(
                    ResourceSnapshot.ShortagePressure,
                    ResourceSnapshot.ConsumerBuildingCount * 250)),
                0,
                12000);

            if (ExportableUnits >= 1000)
            {
                const int ExportScore =
                    ExportableUnits *
                    ResourceTradePricing::GetExportPricePerStockUnit(ResourceType);
                ExportMetrics.push_back(
                    { ResourceType, ExportableUnits, ExportScore });
            }

            if (ImportUnits >= 1000)
            {
                const int ImportScore =
                    ResourceSnapshot.ShortagePressure * 6 +
                    ResourceSnapshot.ConsumerBuildingCount * 400 +
                    (std::max)(0, ResourceSnapshot.ProducerBuildingCount * -50);
                ImportMetrics.push_back(
                    { ResourceType, ImportUnits, ImportScore });
            }
        }

        auto SortMetrics = [](std::vector<FMetric>& Metrics)
        {
            std::sort(
                Metrics.begin(),
                Metrics.end(),
                [](const FMetric& A, const FMetric& B)
                {
                    if (A.Score != B.Score)
                        return A.Score > B.Score;

                    return static_cast<int>(A.Type) <
                        static_cast<int>(B.Type);
                });
        };

        SortMetrics(ExportMetrics);
        SortMetrics(ImportMetrics);

        const size_t ExportCount = (std::min)(
            static_cast<size_t>(6),
            ExportMetrics.size());
        const size_t ImportCount = (std::min)(
            static_cast<size_t>(6),
            ImportMetrics.size());

        for (size_t Index = 0; Index < ExportCount; ++Index)
        {
            Result.push_back(
                BuildTradeProposal(
                    World,
                    false,
                    ExportMetrics[Index].Type,
                    ExportMetrics[Index].AvailabilityUnits,
                    ExportMetrics[Index].Score,
                    SimulationYear,
                    SimulationMonth,
                    CurrentEra,
                    ForeignPowers));
        }

        for (size_t Index = 0; Index < ImportCount; ++Index)
        {
            Result.push_back(
                BuildTradeProposal(
                    World,
                    true,
                    ImportMetrics[Index].Type,
                    ImportMetrics[Index].AvailabilityUnits,
                    ImportMetrics[Index].Score,
                    SimulationYear,
                    SimulationMonth,
                    CurrentEra,
                    ForeignPowers));
        }

        return Result;
    }

    std::vector<FActiveTradeRouteView> BuildActiveTradeRouteViews(
        const std::shared_ptr<CWorld>& World)
    {
        std::vector<FActiveTradeRouteView> Result;

        if (!World)
            return Result;

        auto TradeAccess =
            ResolveMainWorldTradeAccess(World);

        if (!TradeAccess)
            return Result;

        const auto& Routes = TradeAccess->GetActiveTradeRoutes();
        const EBuildingEra CurrentEra = TradeAccess->GetCurrentEra();
        Result.reserve(Routes.size());

        for (size_t Index = 0; Index < Routes.size(); ++Index)
        {
            const FTradeRouteRuntimeState& Route = Routes[Index];
            FActiveTradeRouteView View;
            View.RouteId = Route.RouteId;
            View.ImportRoute = Route.ImportRoute;
            View.ResourceType = Route.ResourceType;
            View.MarketClass = Route.MarketClass;
            View.ForeignPowerIndex = Route.ForeignPowerIndex;
            View.ContractUnits = Route.ContractUnits;
            View.FulfilledUnits = Route.FulfilledUnits;
            View.RemainingDays = Route.RemainingDays;
            View.TotalDurationDays = Route.TotalDurationDays;
            View.RoutePricePerThousand = Route.RoutePricePerThousandUnits;
            View.StandardPricePerThousand = (std::max)(
                1000,
                (Route.ImportRoute ?
                    ResourceTradePricing::GetImportPricePerStockUnit(
                        Route.ResourceType) :
                    ResourceTradePricing::GetExportPricePerStockUnit(
                        Route.ResourceType)) * 1000);
            View.PartnerName =
                MainWorldTradeRuntime::GetForeignPowerName(
                    Route.ForeignPowerIndex,
                    CurrentEra);
            View.CategoryName =
                GetResourceMarketClassDisplayName(View.MarketClass);

            if (View.StandardPricePerThousand > 0)
            {
                const int DeltaValue =
                    View.RoutePricePerThousand - View.StandardPricePerThousand;
                View.DeltaPercent = static_cast<int>(std::lround(
                    static_cast<double>(DeltaValue) * 100.0 /
                    static_cast<double>(View.StandardPricePerThousand)));
            }

            Result.push_back(std::move(View));
        }

        std::sort(
            Result.begin(),
            Result.end(),
            [](const FActiveTradeRouteView& A,
                const FActiveTradeRouteView& B)
            {
                if (A.ImportRoute != B.ImportRoute)
                    return A.ImportRoute > B.ImportRoute;

                if (A.RemainingDays != B.RemainingDays)
                    return A.RemainingDays > B.RemainingDays;

                return A.RouteId > B.RouteId;
            });

        if (Result.size() > GTradeVisibleProposalCount)
            Result.resize(GTradeVisibleProposalCount);

        return Result;
    }

    std::vector<FCompletedTradeRouteView> BuildCompletedTradeRouteViews(
        const std::shared_ptr<CWorld>& World)
    {
        std::vector<FCompletedTradeRouteView> Result;

        if (!World)
            return Result;

        auto TradeAccess =
            ResolveMainWorldTradeAccess(World);

        if (!TradeAccess)
            return Result;

        const auto& Records = TradeAccess->GetCompletedTradeRoutes();
        const EBuildingEra CurrentEra = TradeAccess->GetCurrentEra();
        Result.reserve(Records.size());

        for (size_t Index = 0; Index < Records.size(); ++Index)
        {
            const FTradeRouteCompletionRecord& Record = Records[Index];
            FCompletedTradeRouteView View;
            View.RecordId = Record.RecordId;
            View.RouteId = Record.RouteId;
            View.ImportRoute = Record.ImportRoute;
            View.ResourceType = Record.ResourceType;
            View.MarketClass = Record.MarketClass;
            View.ForeignPowerIndex = Record.ForeignPowerIndex;
            View.ContractUnits = Record.ContractUnits;
            View.FulfilledUnits = Record.FulfilledUnits;
            View.ElapsedDays = Record.ElapsedDays;
            View.TotalDurationDays = Record.TotalDurationDays;
            View.SettledValue = Record.SettledValue;
            View.EndReason = Record.EndReason;
            View.CompletionRewardModifier = Record.CompletionRewardModifier;
            View.SecondaryRelationModifier = Record.SecondaryRelationModifier;
            View.StandingModifier = Record.StandingModifier;
            View.PartnerName =
                MainWorldTradeRuntime::GetForeignPowerName(
                    Record.ForeignPowerIndex,
                    CurrentEra);
            View.CategoryName =
                GetResourceMarketClassDisplayName(Record.MarketClass);
            Result.push_back(std::move(View));
        }

        if (Result.size() > GTradeVisibleProposalCount)
            Result.resize(GTradeVisibleProposalCount);

        return Result;
    }

    std::vector<FTradePriceItem> BuildTradePriceItems(
        int FilterIndex,
        int SortIndex,
        bool SortDescending)
    {
        std::vector<FTradePriceItem> Result;
        Result.reserve(static_cast<size_t>(EResourceType::Count));

        const ETradeFilterType FilterType =
            static_cast<ETradeFilterType>(ClampInt(
                FilterIndex,
                0,
                static_cast<int>(ETradeFilterType::Count) - 1));

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsImmediateProductionScopeResourceType(ResourceType))
                continue;

            if (!MatchesFilter(FilterType, ResourceType))
                continue;

            FTradePriceItem Item;
            Item.ResourceType = ResourceType;
            Item.MarketClass = GetResourceMarketClass(ResourceType);
            Item.ExportPricePerThousand =
                ResourceTradePricing::GetExportPricePerStockUnit(ResourceType) *
                1000;
            Item.ImportPricePerThousand =
                ResourceTradePricing::GetImportPricePerStockUnit(ResourceType) *
                1000;
            Item.ExportIndexPercent =
                ResourceTradePricing::GetExportPriceIndexPercent(ResourceType);
            Item.ImportIndexPercent =
                ResourceTradePricing::GetImportPriceIndexPercent(ResourceType);
            Item.ExportDeltaPercent =
                ResourceTradePricing::GetExportPriceDeltaPercent(ResourceType);
            Item.ImportDeltaPercent =
                ResourceTradePricing::GetImportPriceDeltaPercent(ResourceType);
            Item.CategoryName =
                GetResourceMarketClassDisplayName(Item.MarketClass);
            Result.push_back(std::move(Item));
        }

        auto Compare = [SortIndex, SortDescending](
            const FTradePriceItem& A,
            const FTradePriceItem& B)
        {
            int Comparison = 0;

            switch (SortIndex)
            {
            case 1:
                Comparison =
                    A.ExportPricePerThousand - B.ExportPricePerThousand;
                break;
            case 2:
                Comparison =
                    A.ImportPricePerThousand - B.ImportPricePerThousand;
                break;
            case 3:
                Comparison =
                    A.ExportIndexPercent - B.ExportIndexPercent;
                break;
            case 0:
            default:
                if (GetResourceTypeDisplayName(A.ResourceType) !=
                    GetResourceTypeDisplayName(B.ResourceType))
                {
                    Comparison =
                        std::wstring(GetResourceTypeDisplayName(A.ResourceType)) <
                            std::wstring(GetResourceTypeDisplayName(B.ResourceType)) ?
                                -1 :
                                1;
                }
                break;
            }

            if (Comparison == 0)
            {
                Comparison =
                    static_cast<int>(A.ResourceType) -
                    static_cast<int>(B.ResourceType);
            }

            return SortDescending ? Comparison > 0 : Comparison < 0;
        };

        std::sort(Result.begin(), Result.end(), Compare);

        if (Result.size() > GTradeVisibleProposalCount)
            Result.resize(GTradeVisibleProposalCount);

        return Result;
    }

    int ComputeTaxEventExportModifierPercent(
        const FTaxPolicyEventStatus& TaxEventStatus)
    {
        if (!TaxEventStatus.Active)
            return 0;

        const double Severity = ClampDouble(
            static_cast<double>(TaxEventStatus.DaysActive + 1) / 6.0,
            0.0,
            1.0);
        double ExportMultiplier = 1.0;

        switch (TaxEventStatus.Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            ExportMultiplier = 0.86 - 0.18 * Severity;
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            ExportMultiplier = 0.82 - 0.18 * Severity;
            break;
        default:
            ExportMultiplier = 1.0;
            break;
        }

        return static_cast<int>(std::lround((ExportMultiplier - 1.0) * 100.0));
    }

    std::wstring BuildTradeModifierEventSummary(
        const FTaxPolicyEventStatus& TaxEventStatus,
        const FWorldCrisisStatus& WorldCrisisStatus)
    {
        if (WorldCrisisStatus.Active)
        {
            return WorldCrisisStatus.Title +
                L" 지속시간: " +
                FormatRemainingTradeTime(WorldCrisisStatus.RemainingDays);
        }

        if (TaxEventStatus.Active)
        {
            return TaxEventStatus.Title +
                L" 지속시간: " +
                FormatRemainingTradeTime(TaxEventStatus.RemainingDays);
        }

        return L"이벤트 없음";
    }

    FTradeModifierPageSnapshot BuildTradeModifierPageSnapshot(
        const std::shared_ptr<CWorld>& World,
        const FTaxPolicyEventStatus& TaxEventStatus,
        const FWorldCrisisStatus& WorldCrisisStatus)
    {
        FTradeModifierPageSnapshot Snapshot;
        Snapshot.EventSummary = BuildTradeModifierEventSummary(
            TaxEventStatus,
            WorldCrisisStatus);

        const int GlobalExportModifierPercent =
            ComputeTaxEventExportModifierPercent(TaxEventStatus);

        if (GlobalExportModifierPercent != 0)
        {
            Snapshot.GeneralExportLines.push_back(
                { TaxEventStatus.Title.empty() ? L"세금 이벤트" : TaxEventStatus.Title,
                    GlobalExportModifierPercent });
        }

        Snapshot.GeneralExportTotalPercent = GlobalExportModifierPercent;

        auto TradeAccess =
            ResolveMainWorldTradeAccess(World);

        if (TradeAccess)
        {
            const int CustomsExportModifierPercent =
                TradeAccess->GetCustomsExportTradePriceModifierPercent();
            const int CustomsImportModifierPercent =
                TradeAccess->GetCustomsImportTradePriceModifierPercent();
            const auto& ActiveRoutes = TradeAccess->GetActiveTradeRoutes();
            long long ExportWeightSum = 0;
            long long ExportPercentSum = 0;
            long long ImportWeightSum = 0;
            long long ImportPercentSum = 0;

            for (size_t Index = 0; Index < ActiveRoutes.size(); ++Index)
            {
                const FTradeRouteRuntimeState& Route = ActiveRoutes[Index];
                const int StandardPrice = Route.ImportRoute ?
                    ResourceTradePricing::GetImportPricePerStockUnit(
                        Route.ResourceType) * 1000 :
                    ResourceTradePricing::GetExportPricePerStockUnit(
                        Route.ResourceType) * 1000;

                if (StandardPrice <= 0)
                    continue;

                const int DeltaPercent = static_cast<int>(std::lround(
                    static_cast<double>(
                        Route.RoutePricePerThousandUnits - StandardPrice) * 100.0 /
                    static_cast<double>(StandardPrice)));
                const int Weight = (std::max)(1000, Route.ContractUnits);
                const std::wstring Label =
                    std::wstring(GetResourceTypeDisplayName(Route.ResourceType)) +
                    L" - 계약 시세";

                if (Route.ImportRoute)
                {
                    if (Snapshot.ImportRouteLines.size() < 2)
                    {
                        Snapshot.ImportRouteLines.push_back(
                            { Label, DeltaPercent });
                    }

                    ImportWeightSum += Weight;
                    ImportPercentSum += static_cast<long long>(DeltaPercent) * Weight;
                }
                else
                {
                    if (Snapshot.ExportRouteLines.size() < 3)
                    {
                        Snapshot.ExportRouteLines.push_back(
                            { Label, DeltaPercent });
                    }

                    ExportWeightSum += Weight;
                    ExportPercentSum += static_cast<long long>(DeltaPercent) * Weight;
                }
            }

            if (ExportWeightSum > 0)
            {
                Snapshot.ExportRouteTotalPercent = static_cast<int>(std::lround(
                    static_cast<double>(ExportPercentSum) /
                    static_cast<double>(ExportWeightSum)));
            }

            if (ImportWeightSum > 0)
            {
                Snapshot.ImportRouteTotalPercent = static_cast<int>(std::lround(
                    static_cast<double>(ImportPercentSum) /
                    static_cast<double>(ImportWeightSum)));
            }

            if (CustomsExportModifierPercent != 0)
            {
                Snapshot.ExportRouteLines.push_back(
                    { L"세관 운영 모드", CustomsExportModifierPercent });
                Snapshot.ExportRouteTotalPercent +=
                    CustomsExportModifierPercent;
            }

            if (CustomsImportModifierPercent != 0)
            {
                Snapshot.ImportRouteLines.push_back(
                    { L"세관 운영 모드", CustomsImportModifierPercent });
                Snapshot.ImportRouteTotalPercent +=
                    CustomsImportModifierPercent;
            }
        }

        struct FPersonalModifierEntry
        {
            std::wstring Label;
            int Percent = 0;
            int SortKey = 0;
        };

        std::vector<FPersonalModifierEntry> PersonalEntries;

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsImmediateProductionScopeResourceType(ResourceType))
                continue;

            const int DiplomacyPercent =
                ResourceTradePricing::GetDiplomacyExportBiasPercent(ResourceType);
            const int EdictPercent =
                ResourceTradePricing::GetEdictExportBiasPercent(ResourceType);
            const int CombinedPercent = DiplomacyPercent + EdictPercent;

            if (CombinedPercent == 0)
                continue;

            std::wstring Reason = L"외교 보정";

            if (EdictPercent != 0 && std::abs(EdictPercent) >= std::abs(DiplomacyPercent))
                Reason = L"칙령 보정";

            PersonalEntries.push_back(
                {
                    std::wstring(GetResourceTypeDisplayName(ResourceType)) +
                        L" - " + Reason,
                    CombinedPercent,
                    std::abs(CombinedPercent) * 1000 +
                        ResourceTradePricing::GetExportPricePerStockUnit(ResourceType)
                });
        }

        std::sort(
            PersonalEntries.begin(),
            PersonalEntries.end(),
            [](const FPersonalModifierEntry& A,
                const FPersonalModifierEntry& B)
            {
                if (A.SortKey != B.SortKey)
                    return A.SortKey > B.SortKey;

                return A.Label < B.Label;
            });

        if (PersonalEntries.size() > static_cast<size_t>(GTradeDetailRowCount))
            PersonalEntries.resize(static_cast<size_t>(GTradeDetailRowCount));

        for (size_t Index = 0; Index < PersonalEntries.size(); ++Index)
        {
            Snapshot.PersonalExportLines.push_back(
                { PersonalEntries[Index].Label, PersonalEntries[Index].Percent });
        }

        return Snapshot;
    }

    void SortTradeProposals(
        std::vector<FTradeProposal>& Proposals,
        int SortIndex,
        bool Descending)
    {
        auto Compare = [SortIndex, Descending](
            const FTradeProposal& A,
            const FTradeProposal& B)
        {
            auto ResolveDirectionOrder = [](const FTradeProposal& Proposal)
            {
                return Proposal.ImportRoute ? 0 : 1;
            };

            auto ResolveNameOrder = [](const std::wstring& Left,
                const std::wstring& Right)
            {
                if (Left == Right)
                    return 0;

                return Left < Right ? -1 : 1;
            };

            int Comparison = 0;

            switch (SortIndex)
            {
            case 0:
                Comparison =
                    ResolveDirectionOrder(A) - ResolveDirectionOrder(B);
                break;
            case 1:
                Comparison = ResolveNameOrder(A.PartnerName, B.PartnerName);
                break;
            case 2:
                Comparison = ResolveNameOrder(
                    GetResourceTypeDisplayName(A.ResourceType),
                    GetResourceTypeDisplayName(B.ResourceType));
                break;
            case 3:
            default:
                Comparison = A.MarginPercent - B.MarginPercent;
                break;
            }

            if (Comparison == 0)
                Comparison = A.Score - B.Score;

            if (Comparison == 0)
            {
                Comparison =
                    static_cast<int>(A.ResourceType) -
                    static_cast<int>(B.ResourceType);
            }

            return Descending ? Comparison > 0 : Comparison < 0;
        };

        std::sort(Proposals.begin(), Proposals.end(), Compare);
    }

    void SetDetailRow(
        FTradeDetailSnapshot& Snapshot,
        int Index,
        const std::wstring& Label,
        const std::wstring& Value,
        ETradeDetailValueTone Tone = ETradeDetailValueTone::Default)
    {
        if (Index < 0 || Index >= GTradeDetailRowCount)
            return;

        Snapshot.Rows[static_cast<size_t>(Index)].Label = Label;
        Snapshot.Rows[static_cast<size_t>(Index)].Value = Value;
        Snapshot.Rows[static_cast<size_t>(Index)].Tone = Tone;
    }

    ETradeDetailValueTone ResolveSignedTone(int Value)
    {
        if (Value > 0)
            return ETradeDetailValueTone::Positive;
        if (Value < 0)
            return ETradeDetailValueTone::Negative;
        return ETradeDetailValueTone::Default;
    }
}

namespace TradeWidgetRuntime
{
    int ClampInt(int Value, int MinValue, int MaxValue)
    {
        return (std::max)(MinValue, (std::min)(MaxValue, Value));
    }

    const wchar_t* GetFilterDisplayText(ETradeFilterType FilterType)
    {
        switch (FilterType)
        {
        case ETradeFilterType::Food:
            return L"음식";
        case ETradeFilterType::ConsumerGoods:
            return L"소비재";
        case ETradeFilterType::LuxuryGoods:
            return L"사치품";
        case ETradeFilterType::Minerals:
            return L"광물";
        case ETradeFilterType::ProcessedResources:
            return L"가공된 자원";
        case ETradeFilterType::RawMaterials:
            return L"원자재";
        case ETradeFilterType::LocalResources:
            return L"지역 자원";
        case ETradeFilterType::PlantationGoods:
            return L"대규모 농장 상품";
        case ETradeFilterType::All:
        default:
            return L"모든 상품";
        }
    }

    std::wstring FormatCurrency(long long Value)
    {
        bool Negative = Value < 0;
        unsigned long long AbsValue = Negative ?
            static_cast<unsigned long long>(-Value) :
            static_cast<unsigned long long>(Value);
        std::wstring Digits = std::to_wstring(AbsValue);

        for (int Index = static_cast<int>(Digits.size()) - 3;
            Index > 0;
            Index -= 3)
        {
            Digits.insert(static_cast<size_t>(Index), 1, L',');
        }

        if (Negative)
            Digits.insert(Digits.begin(), L'-');

        return L"$" + Digits;
    }

    std::wstring FormatSignedPercent(int Value)
    {
        if (Value > 0)
            return L"+" + std::to_wstring(Value) + L"%";
        if (Value < 0)
            return std::to_wstring(Value) + L"%";
        return L"0%";
    }

    std::wstring FormatSignedInteger(int Value)
    {
        if (Value > 0)
            return L"+" + std::to_wstring(Value);

        return std::to_wstring(Value);
    }

    std::wstring FormatInteger(int Value)
    {
        std::wstring Digits = std::to_wstring((std::max)(0, Value));

        for (int Index = static_cast<int>(Digits.size()) - 3;
            Index > 0;
            Index -= 3)
        {
            Digits.insert(static_cast<size_t>(Index), 1, L',');
        }

        return Digits;
    }

    std::wstring FormatTradeProgress(int CurrentValue, int TotalValue)
    {
        return FormatInteger(CurrentValue) +
            L" / " +
            FormatInteger(TotalValue);
    }

    std::wstring FormatRemainingTradeTime(int RemainingDays)
    {
        const int SafeDays = (std::max)(0, RemainingDays);
        const int Months = SafeDays / 30;
        const int Days = SafeDays % 30;

        return std::to_wstring(Months) +
            L"개월 " +
            std::to_wstring(Days) +
            L"일";
    }

    long long ResolveTradeTotalPrice(
        const FTradeProposal& Proposal,
        int Amount)
    {
        const long long SafeAmount =
            static_cast<long long>((std::max)(0, Amount));
        return static_cast<long long>(Proposal.OfferPricePerThousand) *
            SafeAmount / 1000LL;
    }

    FTradeWidgetSnapshot BuildSnapshot(
        const std::shared_ptr<CWorld>& World,
        int SelectedPageIndex,
        int FilterIndex,
        int SortIndex,
        bool SortDescending,
        int SelectedProposalIndex,
        int SelectedPriceIndex,
        int SelectedActiveRouteIndex,
        int SelectedCompletedRouteIndex)
    {
        FTradeWidgetSnapshot Snapshot;
        const ETradePageType PageType = ResolvePageType(SelectedPageIndex);

        Snapshot.PageTexts[0] = L"무역로 제안";
        Snapshot.PageTexts[1] = L"상품 및 가격";
        Snapshot.PageTexts[2] = L"가격 수정치";
        Snapshot.PageTexts[3] = L"활성화된 무역로";
        Snapshot.PageTexts[4] = L"무역로 이행 완료";
        Snapshot.TitleText =
            PageType == ETradePageType::CompletedRoutes ?
                L"무역로 이행 완료" :
            PageType == ETradePageType::ActiveRoutes ?
                L"활성화된 무역로" :
            PageType == ETradePageType::PriceModifiers ?
                L"가격 수정치" :
            PageType == ETradePageType::ProductPrices ?
                L"상품 및 가격" :
                L"무역로 제안";

        auto ReadAccess = ResolveMainWorldAlmanacAccess(World);
        auto HudAccess = ResolveMainWorldHudAccess(World);

        if (!ReadAccess || !HudAccess)
            return Snapshot;

        const int SimulationMonth = HudAccess->GetSimulationMonth();
        Snapshot.FilterText =
            std::wstring(GetFilterDisplayText(
                static_cast<ETradeFilterType>(
                    ClampInt(
                        FilterIndex,
                        0,
                        static_cast<int>(ETradeFilterType::Count) - 1)))) +
            L"  v";

        const wchar_t* SortLabels[GTradeSortCount] =
        {
            PageType == ETradePageType::ProductPrices ? L"상품" : L"구분",
            PageType == ETradePageType::ProductPrices ? L"수출가" : L"국가",
            PageType == ETradePageType::ProductPrices ? L"수입가" : L"상품",
            PageType == ETradePageType::ProductPrices ? L"시장" : L"차익"
        };

        for (int Index = 0; Index < GTradeSortCount; ++Index)
        {
            Snapshot.SortTexts[static_cast<size_t>(Index)] =
                std::wstring(SortLabels[Index]) +
                (SortIndex == Index ?
                    (SortDescending ? L" v" : L" ^") :
                    L"");
        }

        if (PageType == ETradePageType::ActiveRoutes)
        {
            Snapshot.VisibleRoutes = BuildActiveTradeRouteViews(World);
            Snapshot.CountdownText =
                L"활성 계약 " +
                std::to_wstring(Snapshot.VisibleRoutes.size()) +
                L"건";

            if (!Snapshot.VisibleRoutes.empty())
            {
                const int SafeIndex = ClampInt(
                    SelectedActiveRouteIndex,
                    0,
                    static_cast<int>(Snapshot.VisibleRoutes.size()) - 1);
                Snapshot.SelectedRoute =
                    Snapshot.VisibleRoutes[static_cast<size_t>(SafeIndex)];
                Snapshot.HasSelectedRoute = true;
            }

            return Snapshot;
        }

        if (PageType == ETradePageType::CompletedRoutes)
        {
            Snapshot.VisibleCompletedRoutes =
                BuildCompletedTradeRouteViews(World);
            Snapshot.CountdownText =
                L"최근 종료 계약 " +
                std::to_wstring(Snapshot.VisibleCompletedRoutes.size()) +
                L"건";

            if (!Snapshot.VisibleCompletedRoutes.empty())
            {
                const int SafeIndex = ClampInt(
                    SelectedCompletedRouteIndex,
                    0,
                    static_cast<int>(
                        Snapshot.VisibleCompletedRoutes.size()) - 1);
                Snapshot.SelectedCompletedRoute =
                    Snapshot.VisibleCompletedRoutes[
                        static_cast<size_t>(SafeIndex)];
                Snapshot.HasSelectedCompletedRoute = true;
            }

            return Snapshot;
        }

        if (PageType == ETradePageType::ProductPrices)
        {
            Snapshot.CountdownText = L"시장 시세 갱신: 일일";
            Snapshot.VisiblePrices = BuildTradePriceItems(
                FilterIndex,
                SortIndex,
                SortDescending);

            if (!Snapshot.VisiblePrices.empty())
            {
                const int SafeIndex = ClampInt(
                    SelectedPriceIndex,
                    0,
                    static_cast<int>(Snapshot.VisiblePrices.size()) - 1);
                Snapshot.SelectedPrice =
                    Snapshot.VisiblePrices[static_cast<size_t>(SafeIndex)];
                Snapshot.HasSelectedPrice = true;
            }

            return Snapshot;
        }

        if (PageType == ETradePageType::PriceModifiers)
        {
            Snapshot.CountdownText = L"가격 수정치";
            Snapshot.ModifierPage = BuildTradeModifierPageSnapshot(
                World,
                HudAccess->GetTaxPolicyEventStatus(),
                HudAccess->GetWorldCrisisStatus());
            return Snapshot;
        }

        Snapshot.CountdownText =
            L"신규 제안 발생까지 " +
            std::to_wstring(BuildMonthsUntilNextProposal(SimulationMonth)) +
            L"개월";

        std::vector<FTradeProposal> Proposals = BuildTradeProposals(
            World,
            HudAccess->GetSimulationYear(),
            HudAccess->GetSimulationMonth(),
            ReadAccess->GetGovernmentProfile(),
            ReadAccess->GetGovernmentEdictStates(),
            HudAccess->GetTaxPolicyEventStatus());

        const ETradeFilterType FilterType =
            static_cast<ETradeFilterType>(
                ClampInt(
                    FilterIndex,
                    0,
                    static_cast<int>(ETradeFilterType::Count) - 1));
        Proposals.erase(
            std::remove_if(
                Proposals.begin(),
                Proposals.end(),
                [FilterType](const FTradeProposal& Proposal)
                {
                    return !MatchesFilter(FilterType, Proposal.ResourceType);
                }),
            Proposals.end());

        SortTradeProposals(
            Proposals,
            ClampInt(SortIndex, 0, GTradeSortCount - 1),
            SortDescending);

        if (Proposals.size() > GTradeVisibleProposalCount)
            Proposals.resize(GTradeVisibleProposalCount);

        Snapshot.VisibleProposals = Proposals;

        if (!Snapshot.VisibleProposals.empty())
        {
            const int SafeIndex = ClampInt(
                SelectedProposalIndex,
                0,
                static_cast<int>(Snapshot.VisibleProposals.size()) - 1);
            Snapshot.SelectedProposal =
                Snapshot.VisibleProposals[static_cast<size_t>(SafeIndex)];
            Snapshot.HasSelectedProposal = true;
        }

        return Snapshot;
    }

    FTradeDetailSnapshot BuildDetailSnapshot(
        const std::shared_ptr<CWorld>& World,
        const FTradeWidgetSnapshot& Snapshot,
        int SelectedPageIndex,
        int SelectedAmountIndex)
    {
        FTradeDetailSnapshot Result;
        const ETradePageType PageType = ResolvePageType(SelectedPageIndex);
        const bool ShowingProductPrices = PageType == ETradePageType::ProductPrices;
        const bool ShowingActiveRoutes = PageType == ETradePageType::ActiveRoutes;
        const bool ShowingCompletedRoutes = PageType == ETradePageType::CompletedRoutes;

        if (Snapshot.HasSelectedPrice)
        {
            const FTradePriceItem& SelectedPrice = Snapshot.SelectedPrice;
            const EResourceType Type = SelectedPrice.ResourceType;
            const int StorageBias =
                ResourceTradePricing::GetStorageBiasPercent(Type);
            const int BalanceBias =
                ResourceTradePricing::GetBalanceBiasPercent(Type);
            const int TemporalBias =
                ResourceTradePricing::GetTemporalBiasPercent(Type);
            const int EventBias =
                ResourceTradePricing::GetEventBiasPercent(Type);
            const int DiplomacyBias =
                ResourceTradePricing::GetDiplomacyExportBiasPercent(Type);
            const int EdictBias =
                ResourceTradePricing::GetEdictExportBiasPercent(Type);
            int RelatedImportOffers = 0;
            int RelatedExportOffers = 0;
            int ActiveImportRoutes = 0;
            int ActiveExportRoutes = 0;

            Result.TitleText = GetResourceTypeDisplayName(SelectedPrice.ResourceType);

            auto TradeAccess =
                ResolveMainWorldTradeAccess(World);

            if (TradeAccess)
            {
                const auto& ActiveRoutes = TradeAccess->GetActiveTradeRoutes();

                for (size_t Index = 0; Index < ActiveRoutes.size(); ++Index)
                {
                    if (ActiveRoutes[Index].ResourceType != Type)
                        continue;

                    if (ActiveRoutes[Index].ImportRoute)
                        ++ActiveImportRoutes;
                    else
                        ++ActiveExportRoutes;
                }
            }

            auto ReadAccess =
                ResolveMainWorldAlmanacAccess(World);
            auto HudAccess =
                ResolveMainWorldHudAccess(World);

            if (ReadAccess && HudAccess)
            {
                const std::vector<FTradeProposal> AllProposals =
                    BuildTradeProposals(
                        World,
                        HudAccess->GetSimulationYear(),
                        HudAccess->GetSimulationMonth(),
                        ReadAccess->GetGovernmentProfile(),
                        ReadAccess->GetGovernmentEdictStates(),
                        HudAccess->GetTaxPolicyEventStatus());

                for (size_t Index = 0; Index < AllProposals.size(); ++Index)
                {
                    if (AllProposals[Index].ResourceType != Type)
                        continue;

                    if (AllProposals[Index].ImportRoute)
                        ++RelatedImportOffers;
                    else
                        ++RelatedExportOffers;
                }
            }

            SetDetailRow(Result, 0, L"범주", SelectedPrice.CategoryName);
            SetDetailRow(
                Result,
                1,
                L"현재 수출가 (1,000)",
                FormatCurrency(SelectedPrice.ExportPricePerThousand));
            SetDetailRow(
                Result,
                2,
                L"현재 수입가 (1,000)",
                FormatCurrency(SelectedPrice.ImportPricePerThousand));
            SetDetailRow(
                Result,
                3,
                L"기준 대비",
                L"수출 " +
                    std::to_wstring(SelectedPrice.ExportIndexPercent) +
                    L"% | 수입 " +
                    std::to_wstring(SelectedPrice.ImportIndexPercent) +
                    L"%");
            SetDetailRow(
                Result,
                4,
                L"전일 변동",
                L"수출 " +
                    FormatSignedPercent(SelectedPrice.ExportDeltaPercent) +
                    L" | 수입 " +
                    FormatSignedPercent(SelectedPrice.ImportDeltaPercent));
            SetDetailRow(
                Result,
                5,
                L"재고 압박",
                FormatSignedPercent(StorageBias),
                ResolveSignedTone(StorageBias));
            SetDetailRow(
                Result,
                6,
                L"수급 균형",
                FormatSignedPercent(BalanceBias),
                ResolveSignedTone(BalanceBias));
            SetDetailRow(
                Result,
                7,
                L"외교 / 칙령",
                L"외교 " +
                    FormatSignedPercent(DiplomacyBias) +
                    L" | 칙령 " +
                    FormatSignedPercent(EdictBias));
            SetDetailRow(
                Result,
                8,
                L"이벤트 / 시장 파동",
                L"이벤트 " +
                    FormatSignedPercent(EventBias) +
                    L" | 파동 " +
                    FormatSignedPercent(TemporalBias));

            Result.ShowAmountTitle = true;
            Result.AmountTitleText = L"무역로";
            Result.AmountButtons[0].Visible = true;
            Result.AmountButtons[0].Text =
                std::wstring(L"관련 제안\n수입: ") +
                std::to_wstring(RelatedImportOffers) +
                L"  수출: " +
                std::to_wstring(RelatedExportOffers);
            Result.AmountButtons[1].Visible = true;
            Result.AmountButtons[1].Text =
                std::wstring(L"체결한 계약\n수입: ") +
                std::to_wstring(ActiveImportRoutes) +
                L"  수출: " +
                std::to_wstring(ActiveExportRoutes);
            return Result;
        }

        if (Snapshot.HasSelectedRoute)
        {
            const FActiveTradeRouteView& SelectedRoute = Snapshot.SelectedRoute;
            Result.TitleText =
                std::wstring(SelectedRoute.ImportRoute ? L"수입: " : L"수출: ") +
                GetResourceTypeDisplayName(SelectedRoute.ResourceType);
            SetDetailRow(Result, 0, L"범주", SelectedRoute.CategoryName);
            SetDetailRow(Result, 1, L"무역국", SelectedRoute.PartnerName);
            SetDetailRow(
                Result,
                2,
                L"진행량",
                FormatTradeProgress(
                    SelectedRoute.FulfilledUnits,
                    SelectedRoute.ContractUnits));
            SetDetailRow(
                Result,
                3,
                L"남은 기간",
                FormatRemainingTradeTime(SelectedRoute.RemainingDays));
            SetDetailRow(
                Result,
                4,
                SelectedRoute.ImportRoute ? L"현재 비용" : L"현재 수익",
                FormatCurrency(
                    static_cast<long long>(SelectedRoute.RoutePricePerThousand) *
                    static_cast<long long>(SelectedRoute.FulfilledUnits) / 1000LL));
            SetDetailRow(
                Result,
                5,
                L"표준 단가 (1,000)",
                FormatCurrency(SelectedRoute.StandardPricePerThousand));
            SetDetailRow(
                Result,
                6,
                L"계약 단가 (1,000)",
                FormatCurrency(SelectedRoute.RoutePricePerThousand));
            const bool FavorableDelta = SelectedRoute.ImportRoute ?
                SelectedRoute.DeltaPercent <= 0 :
                SelectedRoute.DeltaPercent >= 0;
            SetDetailRow(
                Result,
                7,
                L"편차",
                std::to_wstring(std::abs(SelectedRoute.DeltaPercent)) +
                    L"% 표준 " +
                    (SelectedRoute.DeltaPercent >= 0 ? L"이상" : L"이하"),
                FavorableDelta ?
                    ETradeDetailValueTone::Positive :
                    ETradeDetailValueTone::Negative);
            SetDetailRow(
                Result,
                8,
                L"계약 총량",
                FormatInteger(SelectedRoute.ContractUnits) + L" 단위");
            return Result;
        }

        if (Snapshot.HasSelectedCompletedRoute)
        {
            const FCompletedTradeRouteView& SelectedRoute =
                Snapshot.SelectedCompletedRoute;
            Result.TitleText =
                std::wstring(SelectedRoute.ImportRoute ? L"수입: " : L"수출: ") +
                GetResourceTypeDisplayName(SelectedRoute.ResourceType);
            SetDetailRow(Result, 0, L"범주", SelectedRoute.CategoryName);
            SetDetailRow(Result, 1, L"무역국", SelectedRoute.PartnerName);
            SetDetailRow(
                Result,
                2,
                L"이행량",
                FormatTradeProgress(
                    SelectedRoute.FulfilledUnits,
                    SelectedRoute.ContractUnits));
            SetDetailRow(
                Result,
                3,
                L"지속 기간",
                FormatRemainingTradeTime(SelectedRoute.ElapsedDays));
            SetDetailRow(
                Result,
                4,
                L"종료 사유",
                GetTradeRouteEndReasonDisplayName(SelectedRoute.EndReason));
            SetDetailRow(
                Result,
                5,
                SelectedRoute.ImportRoute ? L"비용" : L"수익",
                FormatCurrency(SelectedRoute.SettledValue));
            SetDetailRow(
                Result,
                6,
                L"완료 보상",
                FormatSignedInteger(SelectedRoute.CompletionRewardModifier),
                ResolveSignedTone(SelectedRoute.CompletionRewardModifier));
            SetDetailRow(
                Result,
                7,
                L"관계 변화",
                FormatSignedInteger(SelectedRoute.SecondaryRelationModifier),
                ResolveSignedTone(SelectedRoute.SecondaryRelationModifier));
            SetDetailRow(
                Result,
                8,
                L"standing 변화",
                FormatSignedInteger(SelectedRoute.StandingModifier),
                ResolveSignedTone(SelectedRoute.StandingModifier));
            return Result;
        }

        if (Snapshot.HasSelectedProposal)
        {
            const FTradeProposal& SelectedProposal = Snapshot.SelectedProposal;
            const int SelectedAmount =
                GTradeAmountPresets[static_cast<size_t>(ClampInt(
                    SelectedAmountIndex,
                    0,
                    GTradeAmountPresetCount - 1))];

            Result.TitleText =
                SelectedProposal.PartnerName +
                L" | " +
                (SelectedProposal.ImportRoute ? L"수입 제안" : L"수출 제안");
            SetDetailRow(
                Result,
                0,
                L"거래 유형",
                SelectedProposal.ImportRoute ? L"수입 계약" : L"수출 계약");
            SetDetailRow(
                Result,
                1,
                L"상품",
                GetResourceTypeDisplayName(SelectedProposal.ResourceType));
            SetDetailRow(Result, 2, L"분류", SelectedProposal.CategoryName);
            SetDetailRow(
                Result,
                3,
                L"무역국 / standing",
                SelectedProposal.PartnerName +
                    L" / " +
                    FormatSignedInteger(SelectedProposal.Standing));
            SetDetailRow(
                Result,
                4,
                L"기준 단가 (1,000)",
                FormatCurrency(SelectedProposal.BasePricePerThousand));
            SetDetailRow(
                Result,
                5,
                L"제안 단가 (1,000)",
                FormatCurrency(SelectedProposal.OfferPricePerThousand));
            SetDetailRow(
                Result,
                6,
                L"차익",
                FormatSignedPercent(SelectedProposal.MarginPercent),
                SelectedProposal.MarginPercent >= 0 ?
                    ETradeDetailValueTone::Positive :
                    ETradeDetailValueTone::Negative);
            SetDetailRow(
                Result,
                7,
                L"최대 물량",
                FormatInteger(SelectedProposal.MaxAmount) + L" 단위");
            SetDetailRow(
                Result,
                8,
                L"총 계약금",
                FormatCurrency(ResolveTradeTotalPrice(
                    SelectedProposal,
                    (std::min)(SelectedAmount, SelectedProposal.MaxAmount))));
            return Result;
        }

        Result.TitleText =
            ShowingCompletedRoutes ? L"이행 완료 기록 없음" :
            ShowingActiveRoutes ? L"활성 계약 없음" :
            ShowingProductPrices ? L"상품 없음" :
            L"제안 없음";
        return Result;
    }
}
