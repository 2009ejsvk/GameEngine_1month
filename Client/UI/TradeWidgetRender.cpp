#include "TradeWidget.h"
#include "TradeWidgetRuntime.h"
#include "TropicoUiStyle.h"
#include "../World/GovernmentCommandService.h"
#include "../World/MainWorldAccess.h"
#include "Device.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "World/World.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

using namespace TradeWidgetRuntime;

namespace
{
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

    void ConfigureTitleText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(30.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(110, 78, 26, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(250, 240, 214, 180);
    }

    void ConfigureHeaderText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(15.f);
        Text->SetAlignH(ETextAlignH::Right);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(108, 94, 67, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(244, 234, 202, 180);
    }

    void ConfigureButtonText(
        const std::shared_ptr<CTextBlock>& Text,
        float FontSize)
    {
        if (!Text)
            return;

        Text->SetFontSize(FontSize);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(88, 60, 16, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(250, 240, 220, 180);
    }

    void ConfigureBodyLabelText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(15.f);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(119, 99, 58, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(246, 236, 214, 160);
    }

    void ConfigureBodyValueText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(17.f);
        Text->SetAlignH(ETextAlignH::Right);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(58, 46, 26, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(250, 240, 220, 120);
    }

    void ConfigureRowText(
        const std::shared_ptr<CTextBlock>& Text,
        ETextAlignH Align,
        float FontSize)
    {
        if (!Text)
            return;

        Text->SetFontSize(FontSize);
        Text->SetAlignH(Align);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(72, 56, 32, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(245, 234, 208, 140);
    }

    #if 0
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
        int ModifierPercent);

    FTradeProposal BuildTradeProposal(
        const std::shared_ptr<CWorld>& World,
        bool ImportRoute,
        EResourceType Type,
        int AvailabilityUnits,
        int Score,
        int SimulationYear,
        int SimulationMonth,
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
        int BestPartner = 0;

        for (int Index = 0;
            Index < TradeDiplomacyRuntime::GForeignPowerCount;
            ++Index)
        {
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

        Result.ForeignPowerIndex = BestPartner;
        Result.PartnerName = GetForeignPowerName(BestPartner);

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
            std::dynamic_pointer_cast<IMainWorldTradeAccess>(World);
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
            std::dynamic_pointer_cast<IMainWorldTradeAccess>(World);
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
            std::dynamic_pointer_cast<IMainWorldTradeAccess>(World);

        if (!TradeAccess)
            return Result;

        const auto& Routes = TradeAccess->GetActiveTradeRoutes();
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
            View.PartnerName = GetForeignPowerName(Route.ForeignPowerIndex);
            View.CategoryName =
                GetResourceMarketClassDisplayName(View.MarketClass);

            if (View.StandardPricePerThousand > 0)
            {
                const int DeltaValue = Route.ImportRoute ?
                    View.RoutePricePerThousand - View.StandardPricePerThousand :
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
            std::dynamic_pointer_cast<IMainWorldTradeAccess>(World);

        if (!TradeAccess)
            return Result;

        const auto& Records = TradeAccess->GetCompletedTradeRoutes();
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
            View.PartnerName = GetForeignPowerName(Record.ForeignPowerIndex);
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
            std::dynamic_pointer_cast<IMainWorldTradeAccess>(World);

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
        const ETradePageType PageType =
            static_cast<ETradePageType>(ClampInt(
                SelectedPageIndex,
                0,
                static_cast<int>(ETradePageType::Count) - 1));

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

        auto ReadAccess = std::dynamic_pointer_cast<IMainWorldAlmanacAccess>(World);
        auto HudAccess = std::dynamic_pointer_cast<IMainWorldHudAccess>(World);

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

    #endif

    void ConfigureTradeButton(
        const std::shared_ptr<CButton>& Button,
        const std::string& TextureKeyBase)
    {
        if (!Button)
            return;

        ApplyButtonTextureSet(
            Button,
            TextureKeyBase,
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureDefaultButtonStyle(Button);
    }

    void ConfigureProposalRowButton(const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        ApplyButtonTextureSet(
            Button,
            "TradeProposalRow",
            GSlotCardTexture,
            GSlotCardHoverTexture,
            GSlotCardSelectedTexture,
            GSlotCardDisabledTexture);
        ConfigureIconSlotButtonStyle(Button);
    }

    #if 0
    long long ResolveTradeTotalPrice(
        const FTradeProposal& Proposal,
        int Amount)
    {
        const long long SafeAmount =
            static_cast<long long>((std::max)(0, Amount));
        return static_cast<long long>(Proposal.OfferPricePerThousand) *
            SafeAmount / 1000LL;
    }
    #endif
}

void CTradeWidget::RefreshFromState()
{
    const FTradeWidgetSnapshot Snapshot = BuildSnapshot(
        mWorld.lock(),
        mSelectedPageIndex,
        mSelectedFilterIndex,
        mSelectedSortIndex,
        mSortDescending,
        mSelectedProposalIndex,
        mSelectedPriceIndex,
        mSelectedActiveRouteIndex,
        mSelectedCompletedRouteIndex);
    const bool ShowingProductPrices = mSelectedPageIndex == 1;
    const bool ShowingPriceModifiers = mSelectedPageIndex == 2;
    const bool ShowingActiveRoutes = mSelectedPageIndex == 3;
    const bool ShowingCompletedRoutes = mSelectedPageIndex == 4;

    if (ShowingPriceModifiers)
    {
    }
    else if (ShowingProductPrices)
    {
        if (Snapshot.HasSelectedPrice)
        {
            mSelectedPriceIndex = ClampInt(
                mSelectedPriceIndex,
                0,
                static_cast<int>(Snapshot.VisiblePrices.size()) - 1);
        }
        else
        {
            mSelectedPriceIndex = 0;
        }
    }
    else if (ShowingActiveRoutes)
    {
        if (Snapshot.HasSelectedRoute)
        {
            mSelectedActiveRouteIndex = ClampInt(
                mSelectedActiveRouteIndex,
                0,
                static_cast<int>(Snapshot.VisibleRoutes.size()) - 1);
        }
        else
        {
            mSelectedActiveRouteIndex = 0;
        }
    }
    else if (ShowingCompletedRoutes)
    {
        if (Snapshot.HasSelectedCompletedRoute)
        {
            mSelectedCompletedRouteIndex = ClampInt(
                mSelectedCompletedRouteIndex,
                0,
                static_cast<int>(
                    Snapshot.VisibleCompletedRoutes.size()) - 1);
        }
        else
        {
            mSelectedCompletedRouteIndex = 0;
        }
    }
    else
    {
        if (Snapshot.HasSelectedProposal)
        {
            mSelectedProposalIndex = ClampInt(
                mSelectedProposalIndex,
                0,
                static_cast<int>(Snapshot.VisibleProposals.size()) - 1);
        }
        else
        {
            mSelectedProposalIndex = 0;
            mSelectedAmountIndex = 0;
        }
    }

    auto PanelBackground = mPanelBackground.lock();
    auto TitleRibbon = mTitleRibbon.lock();
    auto ListFrame = mListFrame.lock();
    auto DetailFrame = mDetailFrame.lock();
    auto TitleText = mTitleText.lock();
    auto CountdownText = mCountdownText.lock();
    auto CloseButton = mCloseButton.lock();
    std::array<std::shared_ptr<CButton>, GTradePageCount> PageButtons = {};
    std::array<std::shared_ptr<CTextBlock>, GTradePageCount> PageTexts = {};
    std::array<std::shared_ptr<CTextBlock>, GTradeModifierSectionCount>
        ModifierSectionTitles = {};
    auto FilterButton = mFilterButton.lock();
    auto FilterButtonText = mFilterButtonText.lock();
    auto DetailTitleText = mDetailTitleText.lock();
    auto AmountTitleText = mAmountTitleText.lock();
    auto ActionButton = mActionButton.lock();
    auto ActionButtonText = mActionButtonText.lock();
    auto CompletionAutoOpenButton = mCompletionAutoOpenButton.lock();
    auto CompletionAutoOpenButtonText = mCompletionAutoOpenButtonText.lock();
    auto FeedbackText = mFeedbackText.lock();

    if (PanelBackground)
        PanelBackground->SetEnable(mOpen);
    if (TitleRibbon)
        TitleRibbon->SetEnable(mOpen);
    if (ListFrame)
        ListFrame->SetEnable(mOpen);
    if (DetailFrame)
        DetailFrame->SetEnable(mOpen);
    if (TitleText)
    {
        TitleText->SetEnable(mOpen);
        TitleText->SetText(Snapshot.TitleText.c_str());
    }
    if (CountdownText)
    {
        CountdownText->SetEnable(mOpen);
        CountdownText->SetText(Snapshot.CountdownText.c_str());
    }
    if (CloseButton)
        CloseButton->SetEnable(mOpen);
    for (int Index = 0; Index < GTradePageCount; ++Index)
    {
        PageButtons[static_cast<size_t>(Index)] =
            Index < static_cast<int>(mPageButtons.size()) ?
                mPageButtons[static_cast<size_t>(Index)].lock() :
                nullptr;
        PageTexts[static_cast<size_t>(Index)] =
            Index < static_cast<int>(mPageButtonTexts.size()) ?
                mPageButtonTexts[static_cast<size_t>(Index)].lock() :
                nullptr;

        if (PageButtons[static_cast<size_t>(Index)])
        {
            PageButtons[static_cast<size_t>(Index)]->SetEnable(mOpen);
            PageButtons[static_cast<size_t>(Index)]->ButtonEnable(mOpen);
            PageButtons[static_cast<size_t>(Index)]->SetTint(
                EButtonState::Normal,
                Index == mSelectedPageIndex ?
                    FVector4(1.08f, 1.00f, 0.82f, 1.f) :
                    FVector4(0.96f, 0.96f, 0.96f, 1.f));
        }

        if (PageTexts[static_cast<size_t>(Index)])
        {
            PageTexts[static_cast<size_t>(Index)]->SetEnable(mOpen);
            PageTexts[static_cast<size_t>(Index)]->SetText(
                Snapshot.PageTexts[static_cast<size_t>(Index)].c_str());
        }
    }
    for (int Index = 0; Index < GTradeModifierSectionCount; ++Index)
    {
        ModifierSectionTitles[static_cast<size_t>(Index)] =
            Index < static_cast<int>(mModifierSectionTitles.size()) ?
                mModifierSectionTitles[static_cast<size_t>(Index)].lock() :
                nullptr;
        if (ModifierSectionTitles[static_cast<size_t>(Index)])
        {
            ModifierSectionTitles[static_cast<size_t>(Index)]->SetEnable(
                mOpen && ShowingPriceModifiers);
        }
    }
    if (FilterButton)
    {
        FilterButton->SetEnable(
            mOpen &&
            !ShowingActiveRoutes &&
            !ShowingPriceModifiers &&
            !ShowingCompletedRoutes);
        FilterButton->ButtonEnable(
            mOpen &&
            !ShowingActiveRoutes &&
            !ShowingPriceModifiers &&
            !ShowingCompletedRoutes);
    }
    if (FilterButtonText)
    {
        FilterButtonText->SetEnable(
            mOpen &&
            !ShowingActiveRoutes &&
            !ShowingPriceModifiers &&
            !ShowingCompletedRoutes);
        FilterButtonText->SetText(Snapshot.FilterText.c_str());
    }

    for (int Index = 0; Index < GTradeSortCount; ++Index)
    {
        auto Button =
            Index < static_cast<int>(mSortButtons.size()) ?
                mSortButtons[static_cast<size_t>(Index)].lock() :
                nullptr;
        auto Text =
            Index < static_cast<int>(mSortButtonTexts.size()) ?
                mSortButtonTexts[static_cast<size_t>(Index)].lock() :
                nullptr;

        if (Button)
        {
            Button->SetEnable(
                mOpen &&
                !ShowingActiveRoutes &&
                !ShowingPriceModifiers &&
                !ShowingCompletedRoutes);
            Button->ButtonEnable(
                mOpen &&
                !ShowingActiveRoutes &&
                !ShowingPriceModifiers &&
                !ShowingCompletedRoutes);
            Button->SetTint(
                EButtonState::Normal,
                Index == mSelectedSortIndex ?
                    FVector4(1.08f, 1.00f, 0.82f, 1.f) :
                    FVector4(0.96f, 0.96f, 0.96f, 1.f));
        }

        if (Text)
        {
            Text->SetEnable(
                mOpen &&
                !ShowingActiveRoutes &&
                !ShowingPriceModifiers &&
                !ShowingCompletedRoutes);
            Text->SetText(Snapshot.SortTexts[static_cast<size_t>(Index)].c_str());
        }
    }

    if (ShowingPriceModifiers)
    {
        const wchar_t* SectionNames[GTradeModifierSectionCount] =
        {
            L"일반 수출 가격",
            L"환경 이벤트",
            L"수출 무역로 가격",
            L"개인 수출 가격",
            L"수입 무역로 가격"
        };

        for (int Index = 0; Index < GTradeModifierSectionCount; ++Index)
        {
            if (ModifierSectionTitles[static_cast<size_t>(Index)])
            {
                ModifierSectionTitles[static_cast<size_t>(Index)]->SetText(
                    SectionNames[Index]);
            }
        }

        auto ApplyModifierRow =
            [&](int RowIndex,
                const std::wstring& LabelText,
                int PercentValue,
                bool FavorPositive)
        {
            if (RowIndex < 0 || RowIndex >= GTradeVisibleProposalCount)
                return;

            auto Button = mProposalRows[static_cast<size_t>(RowIndex)].Button.lock();
            auto Direction =
                mProposalRows[static_cast<size_t>(RowIndex)].Direction.lock();
            auto Partner =
                mProposalRows[static_cast<size_t>(RowIndex)].Partner.lock();
            auto Resource =
                mProposalRows[static_cast<size_t>(RowIndex)].Resource.lock();
            auto Margin =
                mProposalRows[static_cast<size_t>(RowIndex)].Margin.lock();

            const bool Visible = !LabelText.empty();

            if (Button)
            {
                Button->SetEnable(mOpen && Visible);
                Button->ButtonEnable(false);
                Button->SetTint(
                    EButtonState::Normal,
                    FVector4(0.98f, 0.98f, 0.98f, 1.f));
            }

            if (Direction)
            {
                Direction->SetEnable(false);
                Direction->SetText(L"");
            }

            if (Partner)
            {
                Partner->SetEnable(false);
                Partner->SetText(L"");
            }

            if (Resource)
            {
                Resource->SetEnable(Visible);
                Resource->SetText(LabelText.c_str());
                Resource->SetTextColor(72, 56, 32, 255);
            }

            if (Margin)
            {
                Margin->SetEnable(Visible);
                Margin->SetText(FormatSignedPercent(PercentValue).c_str());
                const bool Favorable =
                    FavorPositive ? PercentValue >= 0 : PercentValue <= 0;
                Margin->SetTextColor(
                    Favorable ? 46 : 138,
                    Favorable ? 110 : 68,
                    Favorable ? 58 : 54,
                    255);
            }
        };

        for (int RowIndex = 0; RowIndex < GTradeVisibleProposalCount; ++RowIndex)
            ApplyModifierRow(RowIndex, L"", 0, true);

        int LeftRowIndex = 0;
        const auto& GeneralLines = Snapshot.ModifierPage.GeneralExportLines;
        const auto& ExportRouteLines = Snapshot.ModifierPage.ExportRouteLines;
        const auto& ImportRouteLines = Snapshot.ModifierPage.ImportRouteLines;

        for (size_t Index = 0;
            Index < GeneralLines.size() && LeftRowIndex < 1;
            ++Index, ++LeftRowIndex)
        {
            ApplyModifierRow(
                LeftRowIndex,
                GeneralLines[Index].Label,
                GeneralLines[Index].Percent,
                true);
        }

        ApplyModifierRow(
            1,
            L"전체",
            Snapshot.ModifierPage.GeneralExportTotalPercent,
            true);

        LeftRowIndex = 2;

        for (size_t Index = 0;
            Index < ExportRouteLines.size() && LeftRowIndex < 5;
            ++Index, ++LeftRowIndex)
        {
            ApplyModifierRow(
                LeftRowIndex,
                ExportRouteLines[Index].Label,
                ExportRouteLines[Index].Percent,
                true);
        }

        ApplyModifierRow(
            5,
            L"전체",
            Snapshot.ModifierPage.ExportRouteTotalPercent,
            true);

        LeftRowIndex = 6;

        for (size_t Index = 0;
            Index < ImportRouteLines.size() && LeftRowIndex < 8;
            ++Index, ++LeftRowIndex)
        {
            ApplyModifierRow(
                LeftRowIndex,
                ImportRouteLines[Index].Label,
                ImportRouteLines[Index].Percent,
                false);
        }

        ApplyModifierRow(
            8,
            L"전체",
            Snapshot.ModifierPage.ImportRouteTotalPercent,
            false);
        ApplyModifierRow(9, L"", 0, false);

        if (DetailTitleText)
        {
            DetailTitleText->SetEnable(false);
            DetailTitleText->SetText(L"");
        }

        for (int Index = 0; Index < GTradeDetailRowCount; ++Index)
        {
            auto Label = mDetailRows[static_cast<size_t>(Index)].Label.lock();
            auto Value = mDetailRows[static_cast<size_t>(Index)].Value.lock();
            const bool Visible =
                Index < static_cast<int>(
                    Snapshot.ModifierPage.PersonalExportLines.size());

            if (Label)
            {
                Label->SetEnable(mOpen && Visible);
                Label->SetText(
                    Visible ?
                        Snapshot.ModifierPage.PersonalExportLines[
                            static_cast<size_t>(Index)].Label.c_str() :
                        L"");
            }

            if (Value)
            {
                Value->SetEnable(mOpen && Visible);

                if (Visible)
                {
                    const int Percent =
                        Snapshot.ModifierPage.PersonalExportLines[
                            static_cast<size_t>(Index)].Percent;
                    Value->SetText(FormatSignedPercent(Percent).c_str());
                    Value->SetTextColor(
                        Percent >= 0 ? 46 : 138,
                        Percent >= 0 ? 110 : 68,
                        Percent >= 0 ? 58 : 54,
                        255);
                }
                else
                {
                    Value->SetText(L"");
                }
            }
        }

        if (AmountTitleText)
        {
            AmountTitleText->SetEnable(false);
            AmountTitleText->SetText(L"");
        }

        for (int Index = 0; Index < GTradeAmountPresetCount; ++Index)
        {
            auto Button = mAmountButtons[static_cast<size_t>(Index)].lock();
            auto Text = mAmountButtonTexts[static_cast<size_t>(Index)].lock();

            if (Button)
            {
                Button->SetEnable(false);
                Button->ButtonEnable(false);
            }

            if (Text)
            {
                Text->SetEnable(false);
                Text->SetText(L"");
            }
        }

        if (ActionButton)
        {
            ActionButton->SetEnable(false);
            ActionButton->ButtonEnable(false);
        }

        if (ActionButtonText)
        {
            ActionButtonText->SetEnable(false);
            ActionButtonText->SetText(L"");
        }

        if (FeedbackText)
        {
            FeedbackText->SetEnable(mOpen);
            FeedbackText->SetText(
                Snapshot.ModifierPage.EventSummary.c_str());
        }

        RefreshLayout();
        return;
    }

    const int ProposalCount =
        ShowingActiveRoutes ?
            static_cast<int>(Snapshot.VisibleRoutes.size()) :
        ShowingCompletedRoutes ?
            static_cast<int>(Snapshot.VisibleCompletedRoutes.size()) :
        ShowingProductPrices ?
            static_cast<int>(Snapshot.VisiblePrices.size()) :
            static_cast<int>(Snapshot.VisibleProposals.size());

    for (int RowIndex = 0; RowIndex < GTradeVisibleProposalCount; ++RowIndex)
    {
        auto Button =
            mProposalRows[static_cast<size_t>(RowIndex)].Button.lock();
        auto Direction =
            mProposalRows[static_cast<size_t>(RowIndex)].Direction.lock();
        auto Partner =
            mProposalRows[static_cast<size_t>(RowIndex)].Partner.lock();
        auto Resource =
            mProposalRows[static_cast<size_t>(RowIndex)].Resource.lock();
        auto Margin =
            mProposalRows[static_cast<size_t>(RowIndex)].Margin.lock();
        const bool EnableRow = mOpen && RowIndex < ProposalCount;

        if (Button)
        {
            Button->SetEnable(EnableRow);
            Button->ButtonEnable(EnableRow);

            if (EnableRow)
            {
                const bool Selected = ShowingActiveRoutes ?
                    RowIndex == mSelectedActiveRouteIndex :
                    ShowingCompletedRoutes ?
                        RowIndex == mSelectedCompletedRouteIndex :
                    ShowingProductPrices ?
                        RowIndex == mSelectedPriceIndex :
                        RowIndex == mSelectedProposalIndex;
                Button->SetTint(
                    EButtonState::Normal,
                    Selected ?
                        FVector4(1.08f, 1.00f, 0.82f, 1.f) :
                        FVector4(0.98f, 0.98f, 0.98f, 1.f));
            }
        }

        if (!EnableRow)
        {
            if (Direction)
            {
                Direction->SetEnable(false);
                Direction->SetText(L"");
            }
            if (Partner)
            {
                Partner->SetEnable(false);
                Partner->SetText(L"");
            }
            if (Resource)
            {
                Resource->SetEnable(false);
                Resource->SetText(L"");
            }
            if (Margin)
            {
                Margin->SetEnable(false);
                Margin->SetText(L"");
            }

            continue;
        }

        if (ShowingProductPrices)
        {
            const FTradePriceItem& PriceItem =
                Snapshot.VisiblePrices[static_cast<size_t>(RowIndex)];
            const bool Selected = RowIndex == mSelectedPriceIndex;

            if (Direction)
            {
                Direction->SetEnable(true);
                Direction->SetText(
                    FormatSignedPercent(PriceItem.ExportDeltaPercent).c_str());
                Direction->SetTextColor(
                    PriceItem.ExportDeltaPercent >= 0 ? 46 : 138,
                    PriceItem.ExportDeltaPercent >= 0 ? 110 : 68,
                    PriceItem.ExportDeltaPercent >= 0 ? 58 : 54,
                    255);
                Direction->SetShadowTextColor(
                    Selected ? 255 : 245,
                    Selected ? 248 : 235,
                    Selected ? 214 : 204,
                    140);
            }

            if (Partner)
            {
                Partner->SetEnable(true);
                Partner->SetText(PriceItem.CategoryName.c_str());
                Partner->SetTextColor(
                    Selected ? 88 : 72,
                    Selected ? 62 : 56,
                    Selected ? 22 : 32,
                    255);
            }

            if (Resource)
            {
                Resource->SetEnable(true);
                Resource->SetText(GetResourceTypeDisplayName(PriceItem.ResourceType));
                Resource->SetTextColor(
                    Selected ? 88 : 72,
                    Selected ? 62 : 56,
                    Selected ? 22 : 32,
                    255);
            }

            if (Margin)
            {
                Margin->SetEnable(true);
                Margin->SetText(
                    FormatCurrency(PriceItem.ExportPricePerThousand).c_str());
                Margin->SetTextColor(58, 46, 26, 255);
            }
        }
        else if (ShowingActiveRoutes)
        {
            const FActiveTradeRouteView& Route =
                Snapshot.VisibleRoutes[static_cast<size_t>(RowIndex)];
            const bool Selected = RowIndex == mSelectedActiveRouteIndex;

            if (Direction)
            {
                Direction->SetEnable(true);
                Direction->SetText(Route.ImportRoute ? L"수입" : L"수출");
                Direction->SetTextColor(
                    Route.ImportRoute ? 70 : 42,
                    Route.ImportRoute ? 97 : 106,
                    Route.ImportRoute ? 150 : 64,
                    255);
                Direction->SetShadowTextColor(
                    Selected ? 255 : 245,
                    Selected ? 248 : 235,
                    Selected ? 214 : 204,
                    140);
            }

            if (Partner)
            {
                Partner->SetEnable(true);
                Partner->SetText(Route.PartnerName.c_str());
                Partner->SetTextColor(
                    Selected ? 88 : 72,
                    Selected ? 62 : 56,
                    Selected ? 22 : 32,
                    255);
            }

            if (Resource)
            {
                Resource->SetEnable(true);
                Resource->SetText(GetResourceTypeDisplayName(Route.ResourceType));
                Resource->SetTextColor(
                    Selected ? 88 : 72,
                    Selected ? 62 : 56,
                    Selected ? 22 : 32,
                    255);
            }

            if (Margin)
            {
                Margin->SetEnable(true);
                Margin->SetText(
                    FormatTradeProgress(
                        Route.FulfilledUnits,
                        Route.ContractUnits).c_str());
                Margin->SetTextColor(58, 46, 26, 255);
            }
        }
        else if (ShowingCompletedRoutes)
        {
            const FCompletedTradeRouteView& Route =
                Snapshot.VisibleCompletedRoutes[static_cast<size_t>(RowIndex)];
            const bool Selected = RowIndex == mSelectedCompletedRouteIndex;

            if (Direction)
            {
                Direction->SetEnable(true);
                Direction->SetText(Route.ImportRoute ? L"수입" : L"수출");
                Direction->SetTextColor(
                    Route.ImportRoute ? 70 : 42,
                    Route.ImportRoute ? 97 : 106,
                    Route.ImportRoute ? 150 : 64,
                    255);
                Direction->SetShadowTextColor(
                    Selected ? 255 : 245,
                    Selected ? 248 : 235,
                    Selected ? 214 : 204,
                    140);
            }

            if (Partner)
            {
                Partner->SetEnable(true);
                Partner->SetText(Route.PartnerName.c_str());
                Partner->SetTextColor(
                    Selected ? 88 : 72,
                    Selected ? 62 : 56,
                    Selected ? 22 : 32,
                    255);
            }

            if (Resource)
            {
                Resource->SetEnable(true);
                Resource->SetText(GetResourceTypeDisplayName(Route.ResourceType));
                Resource->SetTextColor(
                    Selected ? 88 : 72,
                    Selected ? 62 : 56,
                    Selected ? 22 : 32,
                    255);
            }

            if (Margin)
            {
                Margin->SetEnable(true);
                Margin->SetText(
                    GetTradeRouteEndReasonDisplayName(Route.EndReason));
                Margin->SetTextColor(
                    Route.EndReason == ETradeRouteEndReason::Completed ? 46 : 138,
                    Route.EndReason == ETradeRouteEndReason::Completed ? 110 : 68,
                    Route.EndReason == ETradeRouteEndReason::Completed ? 58 : 54,
                    255);
            }
        }
        else
        {
            const FTradeProposal& Proposal =
                Snapshot.VisibleProposals[static_cast<size_t>(RowIndex)];
            const bool Selected = RowIndex == mSelectedProposalIndex;

            if (Direction)
            {
                Direction->SetEnable(true);
                Direction->SetText(Proposal.ImportRoute ? L"수입" : L"수출");
                Direction->SetTextColor(
                    Proposal.ImportRoute ? 70 : 42,
                    Proposal.ImportRoute ? 97 : 106,
                    Proposal.ImportRoute ? 150 : 64,
                    255);
                Direction->SetShadowTextColor(
                    Selected ? 255 : 245,
                    Selected ? 248 : 235,
                    Selected ? 214 : 204,
                    140);
            }

            if (Partner)
            {
                Partner->SetEnable(true);
                Partner->SetText(Proposal.PartnerName.c_str());
                Partner->SetTextColor(
                    Selected ? 88 : 72,
                    Selected ? 62 : 56,
                    Selected ? 22 : 32,
                    255);
            }

            if (Resource)
            {
                Resource->SetEnable(true);
                Resource->SetText(GetResourceTypeDisplayName(Proposal.ResourceType));
                Resource->SetTextColor(
                    Selected ? 88 : 72,
                    Selected ? 62 : 56,
                    Selected ? 22 : 32,
                    255);
            }

            if (Margin)
            {
                Margin->SetEnable(true);
                Margin->SetText(FormatSignedPercent(Proposal.MarginPercent).c_str());
                Margin->SetTextColor(
                    Proposal.MarginPercent >= 0 ? 46 : 138,
                    Proposal.MarginPercent >= 0 ? 110 : 68,
                    Proposal.MarginPercent >= 0 ? 58 : 54,
                    255);
            }
        }
    }

    const FTradeProposal* SelectedProposal =
        !ShowingProductPrices &&
        !ShowingActiveRoutes &&
        !ShowingCompletedRoutes &&
        Snapshot.HasSelectedProposal ?
            &Snapshot.SelectedProposal :
            nullptr;
    const FTradePriceItem* SelectedPrice =
        ShowingProductPrices && Snapshot.HasSelectedPrice ?
            &Snapshot.SelectedPrice :
            nullptr;
    const FActiveTradeRouteView* SelectedRoute =
        ShowingActiveRoutes && Snapshot.HasSelectedRoute ?
            &Snapshot.SelectedRoute :
            nullptr;
    const FCompletedTradeRouteView* SelectedCompletedRoute =
        ShowingCompletedRoutes && Snapshot.HasSelectedCompletedRoute ?
            &Snapshot.SelectedCompletedRoute :
            nullptr;

    const FTradeDetailSnapshot DetailSnapshot = BuildDetailSnapshot(
        mWorld.lock(),
        Snapshot,
        mSelectedPageIndex,
        mSelectedAmountIndex);

    if (DetailTitleText)
    {
        DetailTitleText->SetEnable(mOpen);
        DetailTitleText->SetText(DetailSnapshot.TitleText.c_str());
    }

    for (int Index = 0; Index < GTradeDetailRowCount; ++Index)
    {
        auto Label = mDetailRows[static_cast<size_t>(Index)].Label.lock();
        auto Value = mDetailRows[static_cast<size_t>(Index)].Value.lock();
        const auto& Row = DetailSnapshot.Rows[static_cast<size_t>(Index)];

        if (Label)
        {
            Label->SetEnable(mOpen);
            Label->SetText(Row.Label.c_str());
        }

        if (Value)
        {
            Value->SetEnable(mOpen);
            Value->SetText(Row.Value.c_str());

            switch (Row.Tone)
            {
            case ETradeDetailValueTone::Positive:
                Value->SetTextColor(48, 114, 62, 255);
                break;
            case ETradeDetailValueTone::Negative:
                Value->SetTextColor(138, 68, 54, 255);
                break;
            case ETradeDetailValueTone::Default:
            default:
                Value->SetTextColor(58, 46, 26, 255);
                break;
            }
        }
    }

    if (ShowingProductPrices)
    {
        if (AmountTitleText)
        {
            AmountTitleText->SetEnable(mOpen && DetailSnapshot.ShowAmountTitle);
            AmountTitleText->SetText(
                DetailSnapshot.ShowAmountTitle ?
                    DetailSnapshot.AmountTitleText.c_str() :
                    L"");
        }

        for (int Index = 0; Index < GTradeAmountPresetCount; ++Index)
        {
            auto Button = mAmountButtons[static_cast<size_t>(Index)].lock();
            auto Text = mAmountButtonTexts[static_cast<size_t>(Index)].lock();
            const auto& AmountSnapshot =
                DetailSnapshot.AmountButtons[static_cast<size_t>(Index)];

            if (Button)
            {
                Button->SetEnable(mOpen && AmountSnapshot.Visible);
                Button->ButtonEnable(AmountSnapshot.Enabled);
                Button->SetTint(
                    EButtonState::Normal,
                    FVector4(0.96f, 0.96f, 0.96f, 1.f));
            }

            if (Text)
            {
                Text->SetEnable(mOpen && AmountSnapshot.Visible);
                Text->SetText(AmountSnapshot.Text.c_str());
            }
        }
    }

    if (AmountTitleText && !ShowingProductPrices)
    {
        AmountTitleText->SetEnable(
            mOpen &&
            !ShowingActiveRoutes &&
            !ShowingCompletedRoutes);
        AmountTitleText->SetText(
            (ShowingActiveRoutes || ShowingCompletedRoutes) ?
                L"" :
                L"계약 물량");
    }

    for (int Index = 0;
        Index < GTradeAmountPresetCount && !ShowingProductPrices;
        ++Index)
    {
        auto Button = mAmountButtons[static_cast<size_t>(Index)].lock();
        auto Text = mAmountButtonTexts[static_cast<size_t>(Index)].lock();
        const int Amount = GTradeAmountPresets[static_cast<size_t>(Index)];
        const bool Enabled =
            mOpen &&
            !ShowingActiveRoutes &&
            !ShowingCompletedRoutes &&
            SelectedProposal &&
            Amount <= SelectedProposal->MaxAmount;

        if (Button)
        {
            Button->SetEnable(
                mOpen &&
                !ShowingActiveRoutes &&
                !ShowingCompletedRoutes);
            Button->ButtonEnable(Enabled);
            Button->SetTint(
                EButtonState::Normal,
                Index == mSelectedAmountIndex && Enabled ?
                    FVector4(1.08f, 1.00f, 0.82f, 1.f) :
                    FVector4(0.96f, 0.96f, 0.96f, 1.f));
        }

        if (Text)
        {
            Text->SetEnable(
                mOpen &&
                !ShowingActiveRoutes &&
                !ShowingCompletedRoutes);
            Text->SetText(FormatInteger(Amount).c_str());
        }
    }

    if (ActionButton)
    {
        ActionButton->SetEnable(
            mOpen &&
            !ShowingProductPrices &&
            !ShowingCompletedRoutes);
        ActionButton->ButtonEnable(
            mOpen &&
            !ShowingProductPrices &&
            !ShowingCompletedRoutes &&
            (ShowingActiveRoutes ?
                SelectedRoute != nullptr :
                SelectedProposal != nullptr));
    }

    if (ActionButtonText)
    {
        ActionButtonText->SetEnable(
            mOpen &&
            !ShowingProductPrices &&
            !ShowingCompletedRoutes);
        ActionButtonText->SetText(
            ShowingActiveRoutes ?
                (SelectedRoute ? L"무역 계약 취소" : L"취소 불가") :
                (SelectedProposal ?
                    (SelectedProposal->ImportRoute ? L"수입 계약 활성화" : L"수출 계약 활성화") :
                    L"체결 불가"));
    }

    if (CompletionAutoOpenButton)
    {
        CompletionAutoOpenButton->SetEnable(mOpen && ShowingCompletedRoutes);
        CompletionAutoOpenButton->ButtonEnable(mOpen && ShowingCompletedRoutes);
        CompletionAutoOpenButton->SetTint(
            EButtonState::Normal,
            mAutoOpenCompletionPage ?
                FVector4(1.08f, 1.00f, 0.82f, 1.f) :
                FVector4(0.96f, 0.96f, 0.96f, 1.f));
    }

    if (CompletionAutoOpenButtonText)
    {
        CompletionAutoOpenButtonText->SetEnable(mOpen && ShowingCompletedRoutes);
        CompletionAutoOpenButtonText->SetText(
            mAutoOpenCompletionPage ? L"[x]" : L"[ ]");
    }

    if (FeedbackText)
    {
        FeedbackText->SetEnable(mOpen);

        if (!mFeedbackMessage.empty())
        {
            FeedbackText->SetText(mFeedbackMessage.c_str());
        }
        else if (SelectedRoute)
        {
            FeedbackText->SetText(
                SelectedRoute->ImportRoute ?
                    L"예산과 항구 저장 여유가 있는 만큼 매일 자동 수입됩니다. 계약 총량을 채우거나 기간이 끝나면 자동 종료됩니다." :
                    L"항구 재고가 확보되는 만큼 매일 자동 선적됩니다. 계약 총량을 채우거나 기간이 끝나면 자동 종료됩니다.");
        }
        else if (ShowingCompletedRoutes)
        {
            FeedbackText->SetText(
                L"무역로가 완료/취소되었을 경우 해당 탭의 무역 화면을 자동으로 엽니다.");
        }
        else if (SelectedPrice)
        {
            FeedbackText->SetText(
                L"좌측 가격은 현재 수출 시세 기준입니다. 우측 수정치는 세계 시장, 외교, 칙령, 이벤트 영향을 반영합니다.");
        }
        else if (SelectedProposal)
        {
            FeedbackText->SetText(
                SelectedProposal->ImportRoute ?
                    L"계약을 활성화하면 예산과 항구 저장 여유가 있는 만큼 매일 자동 수입됩니다." :
                    L"계약을 활성화하면 항구 재고가 확보되는 만큼 매일 자동 선적됩니다.");
        }
        else
        {
            FeedbackText->SetText(
                ShowingActiveRoutes ?
                    L"현재 활성화된 무역 계약이 없습니다." :
                ShowingProductPrices ?
                    L"현재 필터 조건에서 표시할 상품이 없습니다." :
                    L"현재 필터 조건에서 제안 가능한 무역로가 없습니다.");
        }
    }

    RefreshLayout();
}

