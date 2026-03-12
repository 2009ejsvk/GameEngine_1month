#include "TradeWidget.h"
#include "TropicoUiStyle.h"
#include "../Economy/ResourceTradePricing.h"
#include "../Economy/TradeDiplomacyRuntime.h"
#include "../World/GovernmentCommandService.h"
#include "../World/MainWorldAccess.h"
#include "../World/WorldStatsSnapshot.h"
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

namespace
{
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

    constexpr int GTradePageCount = 5;
    constexpr int GTradeSortCount = 4;
    constexpr int GTradeVisibleProposalCount = 10;
    constexpr int GTradeDetailRowCount = 9;
    constexpr int GTradeAmountPresetCount = 3;
    constexpr int GTradeModifierSectionCount = 5;
    constexpr std::array<int, GTradeAmountPresetCount> GTradeAmountPresets =
    {
        1000,
        3000,
        6000
    };

    enum class ETradePageType
    {
        Proposals = 0,
        ProductPrices,
        PriceModifiers,
        ActiveRoutes,
        CompletedRoutes,
        Count
    };

    enum class ETradeFilterType
    {
        All = 0,
        Food,
        ConsumerGoods,
        LuxuryGoods,
        Minerals,
        ProcessedResources,
        RawMaterials,
        LocalResources,
        PlantationGoods,
        Count
    };

    struct FTradeProposal
    {
        bool ImportRoute = false;
        EResourceType ResourceType = EResourceType::None;
        EResourceMarketClass MarketClass = EResourceMarketClass::None;
        int ForeignPowerIndex = 0;
        int Relation = 0;
        int Standing = 0;
        int TradeModifier = 0;
        int BasePricePerThousand = 0;
        int OfferPricePerThousand = 0;
        int MarginPercent = 0;
        int MaxAmount = 0;
        int AvailabilityUnits = 0;
        int Score = 0;
        std::wstring PartnerName;
        std::wstring CategoryName;
    };

    struct FActiveTradeRouteView
    {
        int RouteId = 0;
        bool ImportRoute = false;
        EResourceType ResourceType = EResourceType::None;
        EResourceMarketClass MarketClass = EResourceMarketClass::None;
        int ForeignPowerIndex = 0;
        int ContractUnits = 0;
        int FulfilledUnits = 0;
        int RemainingDays = 0;
        int TotalDurationDays = 0;
        int RoutePricePerThousand = 0;
        int StandardPricePerThousand = 0;
        int DeltaPercent = 0;
        std::wstring PartnerName;
        std::wstring CategoryName;
    };

    struct FTradePriceItem
    {
        EResourceType ResourceType = EResourceType::None;
        EResourceMarketClass MarketClass = EResourceMarketClass::None;
        int ExportPricePerThousand = 0;
        int ImportPricePerThousand = 0;
        int ExportIndexPercent = 0;
        int ImportIndexPercent = 0;
        int ExportDeltaPercent = 0;
        int ImportDeltaPercent = 0;
        std::wstring CategoryName;
    };

    struct FCompletedTradeRouteView
    {
        int RecordId = 0;
        int RouteId = 0;
        bool ImportRoute = false;
        EResourceType ResourceType = EResourceType::None;
        EResourceMarketClass MarketClass = EResourceMarketClass::None;
        int ForeignPowerIndex = 0;
        int ContractUnits = 0;
        int FulfilledUnits = 0;
        int ElapsedDays = 0;
        int TotalDurationDays = 0;
        long long SettledValue = 0;
        ETradeRouteEndReason EndReason = ETradeRouteEndReason::Completed;
        int CompletionRewardModifier = 0;
        int SecondaryRelationModifier = 0;
        int StandingModifier = 0;
        std::wstring PartnerName;
        std::wstring CategoryName;
    };

    struct FTradeModifierLine
    {
        std::wstring Label;
        int Percent = 0;
    };

    struct FTradeModifierPageSnapshot
    {
        std::wstring EventSummary;
        std::vector<FTradeModifierLine> GeneralExportLines;
        int GeneralExportTotalPercent = 0;
        std::vector<FTradeModifierLine> ExportRouteLines;
        int ExportRouteTotalPercent = 0;
        std::vector<FTradeModifierLine> ImportRouteLines;
        int ImportRouteTotalPercent = 0;
        std::vector<FTradeModifierLine> PersonalExportLines;
    };

    struct FTradeWidgetSnapshot
    {
        std::wstring TitleText;
        std::wstring CountdownText;
        std::array<std::wstring, GTradePageCount> PageTexts = {};
        std::wstring FilterText;
        std::array<std::wstring, GTradeSortCount> SortTexts = {};
        std::vector<FTradeProposal> VisibleProposals;
        FTradeProposal SelectedProposal;
        bool HasSelectedProposal = false;
        std::vector<FActiveTradeRouteView> VisibleRoutes;
        FActiveTradeRouteView SelectedRoute;
        bool HasSelectedRoute = false;
        std::vector<FCompletedTradeRouteView> VisibleCompletedRoutes;
        FCompletedTradeRouteView SelectedCompletedRoute;
        bool HasSelectedCompletedRoute = false;
        std::vector<FTradePriceItem> VisiblePrices;
        FTradePriceItem SelectedPrice;
        bool HasSelectedPrice = false;
        FTradeModifierPageSnapshot ModifierPage;
    };

    const wchar_t* GetForeignPowerName(int Index)
    {
        static const wchar_t* Names[TradeDiplomacyRuntime::GForeignPowerCount] =
        {
            L"중국",
            L"러시아",
            L"미국",
            L"중동",
            L"유럽연합"
        };

        if (Index < 0 ||
            Index >= TradeDiplomacyRuntime::GForeignPowerCount)
        {
            return L"해외";
        }

        return Names[Index];
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

    int ClampInt(int Value, int MinValue, int MaxValue)
    {
        return (std::max)(MinValue, (std::min)(MaxValue, Value));
    }

    double ClampDouble(double Value, double MinValue, double MaxValue)
    {
        return (std::max)(MinValue, (std::min)(MaxValue, Value));
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
                Type == EResourceType::Ore;
        case ETradeFilterType::PlantationGoods:
            return Type == EResourceType::Coconuts ||
                Type == EResourceType::Crops ||
                Type == EResourceType::Rum ||
                Type == EResourceType::Cigars ||
                Type == EResourceType::Chocolate ||
                Type == EResourceType::Juice;
        default:
            return true;
        }
    }

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

    long long ResolveTradeTotalPrice(
        const FTradeProposal& Proposal,
        int Amount)
    {
        const long long SafeAmount =
            static_cast<long long>((std::max)(0, Amount));
        return static_cast<long long>(Proposal.OfferPricePerThousand) *
            SafeAmount / 1000LL;
    }
}

CTradeWidget::CTradeWidget()
{
}

CTradeWidget::~CTradeWidget()
{
}

bool CTradeWidget::Init()
{
    CWidgetContainer::Init();

    auto PanelBackground =
        CreateWidget<CImage>("TradeWidget_Background", 6).lock();
    auto TitleRibbon =
        CreateWidget<CImage>("TradeWidget_TitleRibbon", 7).lock();
    auto ListFrame =
        CreateWidget<CImage>("TradeWidget_ListFrame", 7).lock();
    auto DetailFrame =
        CreateWidget<CImage>("TradeWidget_DetailFrame", 7).lock();
    auto TitleText =
        CreateWidget<CTextBlock>("TradeWidget_Title", 8).lock();
    auto CountdownText =
        CreateWidget<CTextBlock>("TradeWidget_Countdown", 8).lock();
    auto CloseButton =
        CreateWidget<CButton>("TradeWidget_CloseButton", 8).lock();
    mPageButtons.resize(GTradePageCount);
    mPageButtonTexts.resize(GTradePageCount);
    mModifierSectionTitles.resize(GTradeModifierSectionCount);
    auto FilterButton =
        CreateWidget<CButton>("TradeWidget_FilterButton", 8).lock();
    auto DetailTitleText =
        CreateWidget<CTextBlock>("TradeWidget_DetailTitle", 8).lock();
    auto AmountTitleText =
        CreateWidget<CTextBlock>("TradeWidget_AmountTitle", 8).lock();
    auto ActionButton =
        CreateWidget<CButton>("TradeWidget_ActionButton", 8).lock();
    auto CompletionAutoOpenButton =
        CreateWidget<CButton>("TradeWidget_CompletionAutoOpenButton", 8).lock();
    auto FeedbackText =
        CreateWidget<CTextBlock>("TradeWidget_Feedback", 8).lock();

    if (PanelBackground)
    {
        PanelBackground->SetTexture(
            "TradeWidget_BackgroundTex",
            GMainMenuPanelTexture);
        PanelBackground->SetTint(1.f, 1.f, 1.f, 1.f);
        mPanelBackground = PanelBackground;
    }

    if (TitleRibbon)
    {
        TitleRibbon->SetTexture(
            "TradeWidget_TitleRibbonTex",
            GMenuTitleRibbonTexture);
        TitleRibbon->SetTint(1.f, 1.f, 1.f, 1.f);
        mTitleRibbon = TitleRibbon;
    }

    if (ListFrame)
    {
        ListFrame->SetTexture(
            "TradeWidget_ListFrameTex",
            GMenuGridFrameTexture);
        ListFrame->SetTint(1.f, 1.f, 1.f, 1.f);
        mListFrame = ListFrame;
    }

    if (DetailFrame)
    {
        DetailFrame->SetTexture(
            "TradeWidget_DetailFrameTex",
            GMenuDetailFrameTexture);
        DetailFrame->SetTint(1.f, 1.f, 1.f, 1.f);
        mDetailFrame = DetailFrame;
    }

    if (TitleText)
    {
        ConfigureTitleText(TitleText);
        mTitleText = TitleText;
    }

    if (CountdownText)
    {
        ConfigureHeaderText(CountdownText);
        mCountdownText = CountdownText;
    }

    if (CloseButton)
    {
        ApplyButtonTextureSet(
            CloseButton,
            "TradeWidget_CloseButton",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        ConfigureIconSlotButtonStyle(CloseButton);
        CloseButton->SetEventCallback<CTradeWidget>(
            EButtonEventState::Click,
            this,
            &CTradeWidget::OnCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_CloseText",
            mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            ConfigureButtonText(CloseText, 20.f);
            CloseButton->SetChild(CloseText);
        }

        mCloseButton = CloseButton;
    }

    for (int Index = 0; Index < GTradePageCount; ++Index)
    {
        auto PageButton = CreateWidget<CButton>(
            "TradeWidget_PageButton_" + std::to_string(Index + 1),
            8).lock();

        if (!PageButton)
            continue;

        ConfigureTradeButton(
            PageButton,
            "TradeWidget_PageButton_" + std::to_string(Index + 1));

        auto PageContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_PageContent_" + std::to_string(Index + 1),
            mWorld);
        auto PageText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_PageText_" + std::to_string(Index + 1),
            mWorld);

        if (PageContent && PageText)
        {
            ConfigureButtonText(PageText, 15.f);
            PageContent->AddWidget(PageText);
            PageButton->SetChild(PageContent);
            mPageButtonTexts[static_cast<size_t>(Index)] = PageText;
        }

        PageButton->SetEventCallback(
            EButtonEventState::Click,
            [this, Index]()
            {
                OnPageButtonClick(Index);
            });
        mPageButtons[static_cast<size_t>(Index)] = PageButton;
    }

    for (int Index = 0; Index < GTradeModifierSectionCount; ++Index)
    {
        auto SectionTitle = CreateWidget<CTextBlock>(
            "TradeWidget_ModifierSection_" + std::to_string(Index + 1),
            8).lock();

        if (!SectionTitle)
            continue;

        SectionTitle->SetFontSize(18.f);
        SectionTitle->SetAlignH(ETextAlignH::Center);
        SectionTitle->SetAlignV(ETextAlignV::Middle);
        SectionTitle->SetTextColor(96, 83, 55, 255);
        SectionTitle->EnableShadow(true);
        SectionTitle->SetShadowOffset(1.f, 1.f);
        SectionTitle->SetShadowTextColor(244, 234, 202, 150);
        mModifierSectionTitles[static_cast<size_t>(Index)] = SectionTitle;
    }

    if (FilterButton)
    {
        ConfigureTradeButton(FilterButton, "TradeWidget_FilterButton");
        auto FilterContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_FilterContent",
            mWorld);
        auto FilterText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_FilterText",
            mWorld);

        if (FilterContent && FilterText)
        {
            ConfigureButtonText(FilterText, 16.f);
            FilterContent->AddWidget(FilterText);
            FilterButton->SetChild(FilterContent);
            mFilterButtonText = FilterText;
        }

        FilterButton->SetEventCallback<CTradeWidget>(
            EButtonEventState::Click,
            this,
            &CTradeWidget::OnFilterButtonClick);
        mFilterButton = FilterButton;
    }

    mSortButtons.resize(GTradeSortCount);
    mSortButtonTexts.resize(GTradeSortCount);

    for (int Index = 0; Index < GTradeSortCount; ++Index)
    {
        auto SortButton = CreateWidget<CButton>(
            "TradeWidget_SortButton_" + std::to_string(Index + 1),
            8).lock();

        if (!SortButton)
            continue;

        ConfigureTradeButton(
            SortButton,
            "TradeWidget_SortButton_" + std::to_string(Index + 1));

        auto SortContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_SortContent_" + std::to_string(Index + 1),
            mWorld);
        auto SortText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_SortText_" + std::to_string(Index + 1),
            mWorld);

        if (SortContent && SortText)
        {
            ConfigureButtonText(SortText, 15.f);
            SortContent->AddWidget(SortText);
            SortButton->SetChild(SortContent);
            mSortButtonTexts[static_cast<size_t>(Index)] = SortText;
        }

        SortButton->SetEventCallback(
            EButtonEventState::Click,
            [this, Index]()
            {
                OnSortButtonClick(Index);
            });
        mSortButtons[static_cast<size_t>(Index)] = SortButton;
    }

    mProposalRows.resize(GTradeVisibleProposalCount);

    for (int RowIndex = 0; RowIndex < GTradeVisibleProposalCount; ++RowIndex)
    {
        auto RowButton = CreateWidget<CButton>(
            "TradeWidget_ProposalRow_" + std::to_string(RowIndex + 1),
            8).lock();

        if (!RowButton)
            continue;

        ConfigureProposalRowButton(RowButton);
        RowButton->SetEventCallback(
            EButtonEventState::Click,
            [this, RowIndex]()
            {
                OnProposalButtonClick(RowIndex);
            });

        auto Content = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_ProposalContent_" + std::to_string(RowIndex + 1),
            mWorld);
        auto DirectionText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_ProposalDirection_" + std::to_string(RowIndex + 1),
            mWorld);
        auto PartnerText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_ProposalPartner_" + std::to_string(RowIndex + 1),
            mWorld);
        auto ResourceText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_ProposalResource_" + std::to_string(RowIndex + 1),
            mWorld);
        auto MarginText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_ProposalMargin_" + std::to_string(RowIndex + 1),
            mWorld);

        if (Content && DirectionText && PartnerText && ResourceText && MarginText)
        {
            ConfigureRowText(DirectionText, ETextAlignH::Center, 15.f);
            ConfigureRowText(PartnerText, ETextAlignH::Center, 15.f);
            ConfigureRowText(ResourceText, ETextAlignH::Left, 16.f);
            ConfigureRowText(MarginText, ETextAlignH::Right, 15.f);
            Content->AddWidget(DirectionText);
            Content->AddWidget(PartnerText);
            Content->AddWidget(ResourceText);
            Content->AddWidget(MarginText);
            RowButton->SetChild(Content);
            mProposalRows[static_cast<size_t>(RowIndex)].Direction =
                DirectionText;
            mProposalRows[static_cast<size_t>(RowIndex)].Partner =
                PartnerText;
            mProposalRows[static_cast<size_t>(RowIndex)].Resource =
                ResourceText;
            mProposalRows[static_cast<size_t>(RowIndex)].Margin =
                MarginText;
        }

        mProposalRows[static_cast<size_t>(RowIndex)].Button = RowButton;
    }

    if (DetailTitleText)
    {
        DetailTitleText->SetFontSize(22.f);
        DetailTitleText->SetAlignH(ETextAlignH::Left);
        DetailTitleText->SetAlignV(ETextAlignV::Middle);
        DetailTitleText->SetTextColor(92, 67, 23, 255);
        DetailTitleText->EnableShadow(true);
        DetailTitleText->SetShadowOffset(1.f, 1.f);
        DetailTitleText->SetShadowTextColor(248, 236, 204, 160);
        mDetailTitleText = DetailTitleText;
    }

    mDetailRows.resize(GTradeDetailRowCount);

    for (int Index = 0; Index < GTradeDetailRowCount; ++Index)
    {
        auto Label = CreateWidget<CTextBlock>(
            "TradeWidget_DetailLabel_" + std::to_string(Index + 1),
            8).lock();
        auto Value = CreateWidget<CTextBlock>(
            "TradeWidget_DetailValue_" + std::to_string(Index + 1),
            8).lock();

        if (Label)
            ConfigureBodyLabelText(Label);

        if (Value)
            ConfigureBodyValueText(Value);

        mDetailRows[static_cast<size_t>(Index)].Label = Label;
        mDetailRows[static_cast<size_t>(Index)].Value = Value;
    }

    if (AmountTitleText)
    {
        AmountTitleText->SetText(TEXT("계약 물량"));
        AmountTitleText->SetFontSize(17.f);
        AmountTitleText->SetAlignH(ETextAlignH::Left);
        AmountTitleText->SetAlignV(ETextAlignV::Middle);
        AmountTitleText->SetTextColor(98, 77, 41, 255);
        AmountTitleText->EnableShadow(true);
        AmountTitleText->SetShadowOffset(1.f, 1.f);
        AmountTitleText->SetShadowTextColor(248, 236, 204, 150);
        mAmountTitleText = AmountTitleText;
    }

    mAmountButtons.resize(GTradeAmountPresetCount);
    mAmountButtonTexts.resize(GTradeAmountPresetCount);

    for (int Index = 0; Index < GTradeAmountPresetCount; ++Index)
    {
        auto AmountButton = CreateWidget<CButton>(
            "TradeWidget_AmountButton_" + std::to_string(Index + 1),
            8).lock();

        if (!AmountButton)
            continue;

        ConfigureTradeButton(
            AmountButton,
            "TradeWidget_AmountButton_" + std::to_string(Index + 1));

        auto AmountContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_AmountContent_" + std::to_string(Index + 1),
            mWorld);
        auto AmountText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_AmountText_" + std::to_string(Index + 1),
            mWorld);

        if (AmountContent && AmountText)
        {
            ConfigureButtonText(AmountText, 15.f);
            AmountContent->AddWidget(AmountText);
            AmountButton->SetChild(AmountContent);
            mAmountButtonTexts[static_cast<size_t>(Index)] = AmountText;
        }

        AmountButton->SetEventCallback(
            EButtonEventState::Click,
            [this, Index]()
            {
                OnAmountButtonClick(Index);
            });
        mAmountButtons[static_cast<size_t>(Index)] = AmountButton;
    }

    if (ActionButton)
    {
        ConfigureTradeButton(ActionButton, "TradeWidget_ActionButton");

        auto ActionContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "TradeWidget_ActionContent",
            mWorld);
        auto ActionText = CWidget::CreateStaticWidget<CTextBlock>(
            "TradeWidget_ActionText",
            mWorld);

        if (ActionContent && ActionText)
        {
            ConfigureButtonText(ActionText, 18.f);
            ActionContent->AddWidget(ActionText);
            ActionButton->SetChild(ActionContent);
            mActionButtonText = ActionText;
        }

        ActionButton->SetEventCallback<CTradeWidget>(
            EButtonEventState::Click,
            this,
            &CTradeWidget::OnActionButtonClick);
        mActionButton = ActionButton;
    }

    if (CompletionAutoOpenButton)
    {
        ConfigureTradeButton(
            CompletionAutoOpenButton,
            "TradeWidget_CompletionAutoOpenButton");

        auto CompletionAutoOpenContent =
            CWidget::CreateStaticWidget<CWidgetContainer>(
                "TradeWidget_CompletionAutoOpenContent",
                mWorld);
        auto CompletionAutoOpenText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "TradeWidget_CompletionAutoOpenText",
                mWorld);

        if (CompletionAutoOpenContent && CompletionAutoOpenText)
        {
            ConfigureButtonText(CompletionAutoOpenText, 18.f);
            CompletionAutoOpenContent->AddWidget(CompletionAutoOpenText);
            CompletionAutoOpenButton->SetChild(CompletionAutoOpenContent);
            mCompletionAutoOpenButtonText = CompletionAutoOpenText;
        }

        CompletionAutoOpenButton->SetEventCallback<CTradeWidget>(
            EButtonEventState::Click,
            this,
            &CTradeWidget::OnCompletionAutoOpenButtonClick);
        mCompletionAutoOpenButton = CompletionAutoOpenButton;
    }

    if (FeedbackText)
    {
        FeedbackText->SetFontSize(15.f);
        FeedbackText->SetAlignH(ETextAlignH::Left);
        FeedbackText->SetAlignV(ETextAlignV::Top);
        FeedbackText->SetTextColor(90, 76, 55, 255);
        FeedbackText->EnableShadow(true);
        FeedbackText->SetShadowOffset(1.f, 1.f);
        FeedbackText->SetShadowTextColor(250, 240, 220, 120);
        mFeedbackText = FeedbackText;
    }

    RefreshLayout();
    RefreshFromState();
    return true;
}

void CTradeWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    auto TradeAccess =
        std::dynamic_pointer_cast<IMainWorldTradeAccess>(mWorld.lock());

    if (TradeAccess)
    {
        const int NotificationVersion =
            TradeAccess->GetTradeRouteCompletionNotificationVersion();

        if (NotificationVersion > mLastSeenCompletionNotificationVersion)
        {
            if (mAutoOpenCompletionPage)
            {
                SetOpen(true);
                mSelectedPageIndex =
                    static_cast<int>(ETradePageType::CompletedRoutes);
                mSelectedCompletedRouteIndex = 0;
                mFeedbackMessage.clear();
            }

            mLastSeenCompletionNotificationVersion = NotificationVersion;
        }
    }

    if (!mOpen)
        return;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();

    if (mLastResolutionWidth != Resolution.Width ||
        mLastResolutionHeight != Resolution.Height)
    {
        RefreshLayout();
    }

    RefreshFromState();
}

void CTradeWidget::ToggleOpen()
{
    SetOpen(!mOpen);
}

void CTradeWidget::SetOpen(bool Open)
{
    if (mOpen == Open)
        return;

    mOpen = Open;

    if (mOpen)
    {
        mSelectedProposalIndex = 0;
        mSelectedPriceIndex = 0;
        mSelectedActiveRouteIndex = 0;
        mSelectedCompletedRouteIndex = 0;
        mSelectedAmountIndex = 0;
        mFeedbackMessage.clear();
        RefreshLayout();
    }

    RefreshFromState();
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

    if (DetailTitleText)
    {
        DetailTitleText->SetEnable(mOpen);

        if (SelectedPrice)
        {
            DetailTitleText->SetText(
                GetResourceTypeDisplayName(SelectedPrice->ResourceType));
        }
        else if (SelectedRoute)
        {
            DetailTitleText->SetText(
                (std::wstring(
                    SelectedRoute->ImportRoute ? L"수입: " : L"수출: ") +
                    std::wstring(
                        GetResourceTypeDisplayName(SelectedRoute->ResourceType)))
                    .c_str());
        }
        else if (SelectedCompletedRoute)
        {
            DetailTitleText->SetText(
                (std::wstring(
                    SelectedCompletedRoute->ImportRoute ? L"수입: " : L"수출: ") +
                    std::wstring(
                        GetResourceTypeDisplayName(
                            SelectedCompletedRoute->ResourceType)))
                    .c_str());
        }
        else if (SelectedProposal)
        {
            DetailTitleText->SetText(
                (SelectedProposal->PartnerName +
                    L" | " +
                    (SelectedProposal->ImportRoute ? L"수입 제안" : L"수출 제안"))
                    .c_str());
        }
        else
        {
            DetailTitleText->SetText(
                ShowingCompletedRoutes ? L"이행 완료 기록 없음" :
                ShowingActiveRoutes ? L"활성 계약 없음" :
                ShowingProductPrices ? L"상품 없음" :
                L"제안 없음");
        }
    }

    std::array<std::wstring, GTradeDetailRowCount> DetailLabels = {};
    std::array<std::wstring, GTradeDetailRowCount> DetailValues = {};

    if (SelectedPrice)
    {
        const EResourceType Type = SelectedPrice->ResourceType;
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

        auto TradeAccess =
            std::dynamic_pointer_cast<IMainWorldTradeAccess>(mWorld.lock());

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
            std::dynamic_pointer_cast<IMainWorldAlmanacAccess>(mWorld.lock());
        auto HudAccess =
            std::dynamic_pointer_cast<IMainWorldHudAccess>(mWorld.lock());

        if (ReadAccess && HudAccess)
        {
            const std::vector<FTradeProposal> AllProposals =
                BuildTradeProposals(
                    mWorld.lock(),
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

        DetailLabels =
        {
            L"범주",
            L"현재 수출가 (1,000)",
            L"현재 수입가 (1,000)",
            L"기준 대비",
            L"전일 변동",
            L"재고 압박",
            L"수급 균형",
            L"외교 / 칙령",
            L"이벤트 / 시장 파동"
        };
        DetailValues[0] = SelectedPrice->CategoryName;
        DetailValues[1] =
            FormatCurrency(SelectedPrice->ExportPricePerThousand);
        DetailValues[2] =
            FormatCurrency(SelectedPrice->ImportPricePerThousand);
        DetailValues[3] =
            L"수출 " +
            std::to_wstring(SelectedPrice->ExportIndexPercent) +
            L"% | 수입 " +
            std::to_wstring(SelectedPrice->ImportIndexPercent) +
            L"%";
        DetailValues[4] =
            L"수출 " +
            FormatSignedPercent(SelectedPrice->ExportDeltaPercent) +
            L" | 수입 " +
            FormatSignedPercent(SelectedPrice->ImportDeltaPercent);
        DetailValues[5] = FormatSignedPercent(StorageBias);
        DetailValues[6] = FormatSignedPercent(BalanceBias);
        DetailValues[7] =
            L"외교 " +
            FormatSignedPercent(DiplomacyBias) +
            L" | 칙령 " +
            FormatSignedPercent(EdictBias);
        DetailValues[8] =
            L"이벤트 " +
            FormatSignedPercent(EventBias) +
            L" | 파동 " +
            FormatSignedPercent(TemporalBias);

        if (AmountTitleText)
        {
            AmountTitleText->SetEnable(mOpen);
            AmountTitleText->SetText(L"무역로");
        }

        for (int Index = 0; Index < GTradeAmountPresetCount; ++Index)
        {
            auto Button = mAmountButtons[static_cast<size_t>(Index)].lock();
            auto Text = mAmountButtonTexts[static_cast<size_t>(Index)].lock();

            if (Button)
            {
                const bool Visible = Index < 2;
                Button->SetEnable(mOpen && Visible);
                Button->ButtonEnable(false);
                Button->SetTint(
                    EButtonState::Normal,
                    FVector4(0.96f, 0.96f, 0.96f, 1.f));
            }

            if (Text)
            {
                Text->SetEnable(mOpen && Index < 2);

                if (Index == 0)
                {
                    Text->SetText(
                        (std::wstring(L"관련 제안\n수입: ") +
                            std::to_wstring(RelatedImportOffers) +
                            L"  수출: " +
                            std::to_wstring(RelatedExportOffers)).c_str());
                }
                else if (Index == 1)
                {
                    Text->SetText(
                        (std::wstring(L"체결한 계약\n수입: ") +
                            std::to_wstring(ActiveImportRoutes) +
                            L"  수출: " +
                            std::to_wstring(ActiveExportRoutes)).c_str());
                }
            }
        }
    }
    else if (SelectedRoute)
    {
        DetailLabels =
        {
            L"범주",
            L"무역국",
            L"진행량",
            L"남은 기간",
            SelectedRoute->ImportRoute ? L"현재 비용" : L"현재 수익",
            L"표준 단가 (1,000)",
            L"계약 단가 (1,000)",
            L"편차",
            L"계약 총량"
        };
        DetailValues[0] = SelectedRoute->CategoryName;
        DetailValues[1] = SelectedRoute->PartnerName;
        DetailValues[2] = FormatTradeProgress(
            SelectedRoute->FulfilledUnits,
            SelectedRoute->ContractUnits);
        DetailValues[3] =
            FormatRemainingTradeTime(SelectedRoute->RemainingDays);
        DetailValues[4] = FormatCurrency(
            static_cast<long long>(SelectedRoute->RoutePricePerThousand) *
            static_cast<long long>(SelectedRoute->FulfilledUnits) / 1000LL);
        DetailValues[5] =
            FormatCurrency(SelectedRoute->StandardPricePerThousand);
        DetailValues[6] =
            FormatCurrency(SelectedRoute->RoutePricePerThousand);
        DetailValues[7] =
            std::to_wstring(std::abs(SelectedRoute->DeltaPercent)) +
            L"% 표준 " +
            (SelectedRoute->DeltaPercent >= 0 ? L"이상" : L"이하");
        DetailValues[8] =
            FormatInteger(SelectedRoute->ContractUnits) + L" 단위";
    }
    else if (SelectedCompletedRoute)
    {
        DetailLabels =
        {
            L"범주",
            L"무역국",
            L"이행량",
            L"지속 기간",
            L"종료 사유",
            SelectedCompletedRoute->ImportRoute ? L"비용" : L"수익",
            L"완료 보상",
            L"관계 변화",
            L"standing 변화"
        };
        DetailValues[0] = SelectedCompletedRoute->CategoryName;
        DetailValues[1] = SelectedCompletedRoute->PartnerName;
        DetailValues[2] = FormatTradeProgress(
            SelectedCompletedRoute->FulfilledUnits,
            SelectedCompletedRoute->ContractUnits);
        DetailValues[3] =
            FormatRemainingTradeTime(SelectedCompletedRoute->ElapsedDays);
        DetailValues[4] =
            GetTradeRouteEndReasonDisplayName(SelectedCompletedRoute->EndReason);
        DetailValues[5] = FormatCurrency(SelectedCompletedRoute->SettledValue);
        DetailValues[6] =
            FormatSignedInteger(
                SelectedCompletedRoute->CompletionRewardModifier);
        DetailValues[7] =
            FormatSignedInteger(
                SelectedCompletedRoute->SecondaryRelationModifier);
        DetailValues[8] =
            FormatSignedInteger(SelectedCompletedRoute->StandingModifier);
    }
    else if (SelectedProposal)
    {
        DetailLabels =
        {
            L"거래 유형",
            L"상품",
            L"분류",
            L"무역국 / standing",
            L"기준 단가 (1,000)",
            L"제안 단가 (1,000)",
            L"차익",
            L"최대 물량",
            L"총 계약금"
        };
        const int SelectedAmount =
            GTradeAmountPresets[static_cast<size_t>(ClampInt(
                mSelectedAmountIndex,
                0,
                GTradeAmountPresetCount - 1))];

        DetailValues[0] =
            SelectedProposal->ImportRoute ? L"수입 계약" : L"수출 계약";
        DetailValues[1] =
            GetResourceTypeDisplayName(SelectedProposal->ResourceType);
        DetailValues[2] = SelectedProposal->CategoryName;
        DetailValues[3] =
            SelectedProposal->PartnerName +
            L" / " +
            FormatSignedInteger(SelectedProposal->Standing);
        DetailValues[4] =
            FormatCurrency(SelectedProposal->BasePricePerThousand);
        DetailValues[5] =
            FormatCurrency(SelectedProposal->OfferPricePerThousand);
        DetailValues[6] =
            FormatSignedPercent(SelectedProposal->MarginPercent);
        DetailValues[7] =
            FormatInteger(SelectedProposal->MaxAmount) + L" 단위";
        DetailValues[8] =
            FormatCurrency(ResolveTradeTotalPrice(
                *SelectedProposal,
                (std::min)(SelectedAmount, SelectedProposal->MaxAmount)));
    }
    else
    {
        for (int Index = 0; Index < GTradeDetailRowCount; ++Index)
        {
            DetailLabels[static_cast<size_t>(Index)] = L"-";
            DetailValues[static_cast<size_t>(Index)] = L"-";
        }

        if (ShowingProductPrices)
        {
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
        }
    }

    for (int Index = 0; Index < GTradeDetailRowCount; ++Index)
    {
        auto Label = mDetailRows[static_cast<size_t>(Index)].Label.lock();
        auto Value = mDetailRows[static_cast<size_t>(Index)].Value.lock();

        if (Label)
        {
            Label->SetEnable(mOpen);
            Label->SetText(DetailLabels[static_cast<size_t>(Index)].c_str());
        }

        if (Value)
        {
            Value->SetEnable(mOpen);
            Value->SetText(DetailValues[static_cast<size_t>(Index)].c_str());

            if (!ShowingActiveRoutes &&
                !ShowingCompletedRoutes &&
                Index == 6 &&
                SelectedProposal)
            {
                Value->SetTextColor(
                    SelectedProposal->MarginPercent >= 0 ? 48 : 138,
                    SelectedProposal->MarginPercent >= 0 ? 114 : 68,
                    SelectedProposal->MarginPercent >= 0 ? 62 : 54,
                    255);
            }
            else if (ShowingActiveRoutes &&
                Index == 7 &&
                SelectedRoute)
            {
                const bool FavorableDelta = SelectedRoute->ImportRoute ?
                    SelectedRoute->DeltaPercent <= 0 :
                    SelectedRoute->DeltaPercent >= 0;
                Value->SetTextColor(
                    FavorableDelta ? 48 : 138,
                    FavorableDelta ? 114 : 68,
                    FavorableDelta ? 62 : 54,
                    255);
            }
            else if (ShowingCompletedRoutes &&
                SelectedCompletedRoute &&
                Index >= 6)
            {
                const int ModifierValue =
                    Index == 6 ?
                        SelectedCompletedRoute->CompletionRewardModifier :
                    Index == 7 ?
                        SelectedCompletedRoute->SecondaryRelationModifier :
                        SelectedCompletedRoute->StandingModifier;
                Value->SetTextColor(
                    ModifierValue >= 0 ? 48 : 138,
                    ModifierValue >= 0 ? 114 : 68,
                    ModifierValue >= 0 ? 62 : 54,
                    255);
            }
            else
            {
                Value->SetTextColor(58, 46, 26, 255);
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

void CTradeWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    mLastResolutionWidth = Resolution.Width;
    mLastResolutionHeight = Resolution.Height;

    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(640.f, ScreenWidth - 120.f);
    const float AvailableHeight = (std::max)(560.f, ScreenHeight - 120.f);
    const float Scale = (std::min)(
        1.f,
        (std::min)(
            AvailableWidth / mPanelWidth,
            AvailableHeight / mPanelHeight));
    const float PanelWidth = mPanelWidth * Scale;
    const float PanelHeight = mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;
    const bool ShowingProductPrices = mSelectedPageIndex == 1;
    const bool ShowingPriceModifiers = mSelectedPageIndex == 2;
    const bool ShowingActiveRoutes = mSelectedPageIndex == 3;
    const bool ShowingCompletedRoutes = mSelectedPageIndex == 4;
    const float HeaderHeight = 62.f * Scale;
    const float TitleWidth = PanelWidth * 0.42f;
    const float TitleLeft = PanelLeft + (PanelWidth - TitleWidth) * 0.5f;
    const float PageButtonWidth = 132.f * Scale;
    const float PageButtonHeight = 34.f * Scale;
    const float PageButtonsLeft =
        PanelLeft + (PanelWidth -
            PageButtonWidth * static_cast<float>(GTradePageCount) -
            12.f * Scale * static_cast<float>(GTradePageCount - 1)) * 0.5f;
    const float CloseSize = 36.f * Scale;
    const float SectionGap = 18.f * Scale;
    const float LeftWidth = PanelWidth * 0.52f;
    const float RightWidth = PanelWidth - LeftWidth - 48.f * Scale;
    const float LeftLeft = PanelLeft + 26.f * Scale;
    const float RightLeft = LeftLeft + LeftWidth + SectionGap;
    const float ContentTop = PanelTop + HeaderHeight + 20.f * Scale;
    const float ContentHeight = PanelHeight - HeaderHeight - 48.f * Scale;
    const float FilterHeight = 44.f * Scale;
    const float SortHeight = 34.f * Scale;
    const float ListTop =
        (ShowingActiveRoutes || ShowingCompletedRoutes) ?
            ContentTop + 12.f * Scale :
            ContentTop + FilterHeight + 10.f * Scale + SortHeight + 10.f * Scale;
    const float RowHeight = 46.f * Scale;
    const float RowGap = 6.f * Scale;

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
    {
        PanelBackground->SetPos(PanelLeft, PanelTop);
        PanelBackground->SetSize(PanelWidth, PanelHeight);
    }

    if (TitleRibbon)
    {
        TitleRibbon->SetPos(TitleLeft, PanelTop + 8.f * Scale);
        TitleRibbon->SetSize(TitleWidth, HeaderHeight);
    }

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
            PageButtons[static_cast<size_t>(Index)]->SetPos(
                PageButtonsLeft +
                    static_cast<float>(Index) *
                        (PageButtonWidth + 12.f * Scale),
                PanelTop - 14.f * Scale);
            PageButtons[static_cast<size_t>(Index)]->SetSize(
                PageButtonWidth,
                PageButtonHeight);
        }

        if (PageTexts[static_cast<size_t>(Index)])
        {
            PageTexts[static_cast<size_t>(Index)]->SetPos(0.f, 0.f);
            PageTexts[static_cast<size_t>(Index)]->SetSize(
                PageButtonWidth,
                PageButtonHeight);
            PageTexts[static_cast<size_t>(Index)]->SetFontSize(15.f * Scale);
        }
    }

    for (int Index = 0; Index < GTradeModifierSectionCount; ++Index)
    {
        ModifierSectionTitles[static_cast<size_t>(Index)] =
            Index < static_cast<int>(mModifierSectionTitles.size()) ?
                mModifierSectionTitles[static_cast<size_t>(Index)].lock() :
                nullptr;
    }

    if (ListFrame)
    {
        ListFrame->SetPos(
            LeftLeft,
            (ShowingActiveRoutes || ShowingCompletedRoutes) ?
                ContentTop :
                ContentTop + FilterHeight + 8.f * Scale);
        ListFrame->SetSize(
            LeftWidth,
            (ShowingActiveRoutes || ShowingCompletedRoutes) ?
                ContentHeight :
                ContentHeight - FilterHeight - 8.f * Scale);
    }

    if (DetailFrame)
    {
        DetailFrame->SetPos(RightLeft, ContentTop);
        DetailFrame->SetSize(RightWidth, ContentHeight);
    }

    if (TitleText)
    {
        TitleText->SetPos(TitleLeft + 18.f * Scale, PanelTop + 8.f * Scale);
        TitleText->SetSize(TitleWidth - 36.f * Scale, HeaderHeight);
        TitleText->SetFontSize(30.f * Scale);
    }

    if (CountdownText)
    {
        CountdownText->SetPos(
            PanelLeft + PanelWidth - 280.f * Scale,
            PanelTop + 18.f * Scale);
        CountdownText->SetSize(214.f * Scale, 28.f * Scale);
        CountdownText->SetFontSize(15.f * Scale);
    }

    if (CloseButton)
    {
        CloseButton->SetPos(
            PanelLeft + PanelWidth - 52.f * Scale,
            PanelTop + 12.f * Scale);
        CloseButton->SetSize(CloseSize, CloseSize);
    }

    if (FilterButton)
    {
        FilterButton->SetPos(LeftLeft, ContentTop);
        FilterButton->SetSize(194.f * Scale, FilterHeight);
    }

    if (FilterButtonText)
    {
        FilterButtonText->SetPos(0.f, 0.f);
        FilterButtonText->SetSize(194.f * Scale, FilterHeight);
        FilterButtonText->SetFontSize(16.f * Scale);
    }

    const float SortButtonWidth =
        (LeftWidth - 3.f * (8.f * Scale)) / 4.f;

    for (int Index = 0; Index < GTradeSortCount; ++Index)
    {
        auto Button = mSortButtons[static_cast<size_t>(Index)].lock();
        auto Text = mSortButtonTexts[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        Button->SetPos(
            LeftLeft +
                (SortButtonWidth + 8.f * Scale) * static_cast<float>(Index),
            ContentTop + FilterHeight + 10.f * Scale);
        Button->SetSize(SortButtonWidth, SortHeight);

        if (Text)
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(SortButtonWidth, SortHeight);
            Text->SetFontSize(15.f * Scale);
        }
    }

    const float DirectionWidth = 72.f * Scale;
    const float PartnerWidth =
        ((ShowingActiveRoutes || ShowingCompletedRoutes) ?
            108.f :
            ShowingProductPrices ? 118.f : 120.f) * Scale;
    const float MarginWidth =
        ((ShowingActiveRoutes || ShowingCompletedRoutes) ?
            150.f :
            ShowingProductPrices ? 120.f : 84.f) * Scale;
    const float ResourceWidth =
        LeftWidth - DirectionWidth - PartnerWidth - MarginWidth - 48.f * Scale;

    for (int RowIndex = 0; RowIndex < GTradeVisibleProposalCount; ++RowIndex)
    {
        auto Button = mProposalRows[static_cast<size_t>(RowIndex)].Button.lock();
        auto Direction = mProposalRows[static_cast<size_t>(RowIndex)].Direction.lock();
        auto Partner = mProposalRows[static_cast<size_t>(RowIndex)].Partner.lock();
        auto Resource = mProposalRows[static_cast<size_t>(RowIndex)].Resource.lock();
        auto Margin = mProposalRows[static_cast<size_t>(RowIndex)].Margin.lock();
        float RowTop =
            ListTop + static_cast<float>(RowIndex) * (RowHeight + RowGap);

        if (ShowingPriceModifiers)
        {
            if (RowIndex < 2)
            {
                RowTop = ContentTop + 66.f * Scale +
                    static_cast<float>(RowIndex) * (RowHeight + 8.f * Scale);
            }
            else if (RowIndex < 6)
            {
                RowTop = ContentTop + 256.f * Scale +
                    static_cast<float>(RowIndex - 2) * (RowHeight + 8.f * Scale);
            }
            else
            {
                RowTop = ContentTop + 508.f * Scale +
                    static_cast<float>(RowIndex - 6) * (RowHeight + 8.f * Scale);
            }
        }

        if (Button)
        {
            Button->SetPos(LeftLeft + 10.f * Scale, RowTop);
            Button->SetSize(LeftWidth - 20.f * Scale, RowHeight);
        }

        if (Direction)
        {
            Direction->SetPos(12.f * Scale, 0.f);
            Direction->SetSize(DirectionWidth, RowHeight);
            Direction->SetFontSize(15.f * Scale);
        }

        if (Partner)
        {
            Partner->SetPos(18.f * Scale + DirectionWidth, 0.f);
            Partner->SetSize(PartnerWidth, RowHeight);
            Partner->SetFontSize(15.f * Scale);
        }

        if (Resource)
        {
            Resource->SetPos(
                ShowingPriceModifiers ? 20.f * Scale :
                    24.f * Scale + DirectionWidth + PartnerWidth,
                0.f);
            Resource->SetSize(
                ShowingPriceModifiers ?
                    LeftWidth - 156.f * Scale :
                    ResourceWidth,
                RowHeight);
            Resource->SetFontSize(16.f * Scale);
        }

        if (Margin)
        {
            Margin->SetPos(
                ShowingPriceModifiers ?
                    LeftWidth - 136.f * Scale :
                    26.f * Scale + DirectionWidth + PartnerWidth + ResourceWidth,
                0.f);
            Margin->SetSize(
                ShowingPriceModifiers ? 112.f * Scale : MarginWidth,
                RowHeight);
            Margin->SetFontSize(15.f * Scale);
        }
    }

    if (ShowingPriceModifiers)
    {
        const float LeftTitleWidth = LeftWidth - 32.f * Scale;
        const float RightTitleWidth = RightWidth - 56.f * Scale;
        const float RightSectionTop = ContentTop + 212.f * Scale;

        if (ModifierSectionTitles[0])
        {
            ModifierSectionTitles[0]->SetPos(
                LeftLeft + 10.f * Scale,
                ContentTop + 12.f * Scale);
            ModifierSectionTitles[0]->SetSize(LeftTitleWidth, 28.f * Scale);
            ModifierSectionTitles[0]->SetFontSize(18.f * Scale);
        }

        if (ModifierSectionTitles[1])
        {
            ModifierSectionTitles[1]->SetPos(
                RightLeft + 28.f * Scale,
                ContentTop + 12.f * Scale);
            ModifierSectionTitles[1]->SetSize(RightTitleWidth, 28.f * Scale);
            ModifierSectionTitles[1]->SetFontSize(18.f * Scale);
        }

        if (ModifierSectionTitles[2])
        {
            ModifierSectionTitles[2]->SetPos(
                LeftLeft + 10.f * Scale,
                ContentTop + 212.f * Scale);
            ModifierSectionTitles[2]->SetSize(LeftTitleWidth, 28.f * Scale);
            ModifierSectionTitles[2]->SetFontSize(18.f * Scale);
        }

        if (ModifierSectionTitles[3])
        {
            ModifierSectionTitles[3]->SetPos(
                RightLeft + 28.f * Scale,
                RightSectionTop);
            ModifierSectionTitles[3]->SetSize(RightTitleWidth, 28.f * Scale);
            ModifierSectionTitles[3]->SetFontSize(18.f * Scale);
        }

        if (ModifierSectionTitles[4])
        {
            ModifierSectionTitles[4]->SetPos(
                LeftLeft + 10.f * Scale,
                ContentTop + 464.f * Scale);
            ModifierSectionTitles[4]->SetSize(LeftTitleWidth, 28.f * Scale);
            ModifierSectionTitles[4]->SetFontSize(18.f * Scale);
        }
    }

    if (DetailTitleText)
    {
        DetailTitleText->SetPos(
            RightLeft + 28.f * Scale,
            ContentTop + 16.f * Scale);
        DetailTitleText->SetSize(RightWidth - 56.f * Scale, 34.f * Scale);
        DetailTitleText->SetFontSize(22.f * Scale);
    }

    const float DetailLabelWidth = 150.f * Scale;
    const float DetailValueWidth = RightWidth - DetailLabelWidth - 68.f * Scale;
    const float DetailRowHeight = 28.f * Scale;
    const float DetailStartTop = ShowingPriceModifiers ?
        ContentTop + 254.f * Scale :
        ContentTop + 70.f * Scale;

    for (int Index = 0; Index < GTradeDetailRowCount; ++Index)
    {
        auto Label = mDetailRows[static_cast<size_t>(Index)].Label.lock();
        auto Value = mDetailRows[static_cast<size_t>(Index)].Value.lock();
        const float RowTop =
            DetailStartTop + static_cast<float>(Index) * 34.f * Scale;

        if (Label)
        {
            Label->SetPos(RightLeft + 28.f * Scale, RowTop);
            Label->SetSize(
                ShowingPriceModifiers ?
                    RightWidth - 180.f * Scale :
                    DetailLabelWidth,
                DetailRowHeight);
            Label->SetFontSize(15.f * Scale);
        }

        if (Value)
        {
            Value->SetPos(
                RightLeft +
                    (ShowingPriceModifiers ?
                        RightWidth - 136.f * Scale :
                        28.f * Scale + DetailLabelWidth),
                RowTop);
            Value->SetSize(
                ShowingPriceModifiers ?
                    108.f * Scale :
                    DetailValueWidth,
                DetailRowHeight);
            Value->SetFontSize(17.f * Scale);
        }
    }

    if (AmountTitleText)
    {
        AmountTitleText->SetPos(
            RightLeft + 28.f * Scale,
            ContentTop + 402.f * Scale);
        AmountTitleText->SetSize(130.f * Scale, 28.f * Scale);
        AmountTitleText->SetFontSize(17.f * Scale);
    }

    const float AmountButtonWidth =
        ShowingProductPrices ?
            (RightWidth - 66.f * Scale) / 2.f :
            (RightWidth - 76.f * Scale) / 3.f;

    for (int Index = 0; Index < GTradeAmountPresetCount; ++Index)
    {
        auto Button = mAmountButtons[static_cast<size_t>(Index)].lock();
        auto Text = mAmountButtonTexts[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        const float ButtonGap = ShowingProductPrices ? 10.f * Scale : 10.f * Scale;
        Button->SetPos(
            RightLeft + 28.f * Scale +
                static_cast<float>(Index) * (AmountButtonWidth + ButtonGap),
            ContentTop + 438.f * Scale);
        Button->SetSize(AmountButtonWidth, 40.f * Scale);

        if (Text)
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(AmountButtonWidth, 40.f * Scale);
            Text->SetFontSize(15.f * Scale);
        }
    }

    if (ActionButton)
    {
        ActionButton->SetPos(
            RightLeft + 28.f * Scale,
            ContentTop + 496.f * Scale);
        ActionButton->SetSize(RightWidth - 56.f * Scale, 46.f * Scale);
    }

    if (ActionButtonText)
    {
        ActionButtonText->SetPos(0.f, 0.f);
        ActionButtonText->SetSize(RightWidth - 56.f * Scale, 46.f * Scale);
        ActionButtonText->SetFontSize(18.f * Scale);
    }

    if (CompletionAutoOpenButton)
    {
        CompletionAutoOpenButton->SetPos(
            RightLeft + 28.f * Scale,
            ContentTop + 548.f * Scale);
        CompletionAutoOpenButton->SetSize(54.f * Scale, 40.f * Scale);
    }

    if (CompletionAutoOpenButtonText)
    {
        CompletionAutoOpenButtonText->SetPos(0.f, 0.f);
        CompletionAutoOpenButtonText->SetSize(54.f * Scale, 40.f * Scale);
        CompletionAutoOpenButtonText->SetFontSize(16.f * Scale);
    }

    if (FeedbackText)
    {
        FeedbackText->SetPos(
            RightLeft + (ShowingCompletedRoutes ? 92.f * Scale : 28.f * Scale),
            ShowingPriceModifiers ?
                ContentTop + 56.f * Scale :
            ShowingCompletedRoutes ?
                ContentTop + 552.f * Scale :
                ContentTop + 558.f * Scale);
        FeedbackText->SetSize(
            RightWidth - (ShowingCompletedRoutes ? 120.f * Scale : 56.f * Scale),
            ShowingPriceModifiers ?
                112.f * Scale :
            ShowingCompletedRoutes ?
                72.f * Scale :
                124.f * Scale);
        FeedbackText->SetFontSize(15.f * Scale);
    }
}

void CTradeWidget::OnCloseButtonClick()
{
    SetOpen(false);
}

void CTradeWidget::OnPageButtonClick(int PageIndex)
{
    mSelectedPageIndex = ClampInt(PageIndex, 0, GTradePageCount - 1);
    mSelectedProposalIndex = 0;
    mSelectedPriceIndex = 0;
    mSelectedActiveRouteIndex = 0;
    mSelectedCompletedRouteIndex = 0;
    mSelectedAmountIndex = 0;
    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTradeWidget::OnFilterButtonClick()
{
    mSelectedFilterIndex =
        (mSelectedFilterIndex + 1) %
        static_cast<int>(ETradeFilterType::Count);
    mSelectedProposalIndex = 0;
    mSelectedAmountIndex = 0;
    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTradeWidget::OnSortButtonClick(int SortIndex)
{
    const int SafeSortIndex = ClampInt(SortIndex, 0, GTradeSortCount - 1);

    if (mSelectedSortIndex == SafeSortIndex)
        mSortDescending = !mSortDescending;
    else
        mSortDescending = true;

    mSelectedSortIndex = SafeSortIndex;
    mSelectedProposalIndex = 0;
    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTradeWidget::OnProposalButtonClick(int RowIndex)
{
    if (mSelectedPageIndex == 4)
    {
        mSelectedCompletedRouteIndex = ClampInt(
            RowIndex,
            0,
            GTradeVisibleProposalCount - 1);
    }
    else if (mSelectedPageIndex == 3)
    {
        mSelectedActiveRouteIndex = ClampInt(
            RowIndex,
            0,
            GTradeVisibleProposalCount - 1);
    }
    else if (mSelectedPageIndex == 1)
    {
        mSelectedPriceIndex = ClampInt(
            RowIndex,
            0,
            GTradeVisibleProposalCount - 1);
    }
    else if (mSelectedPageIndex == 2)
    {
    }
    else
    {
        mSelectedProposalIndex = ClampInt(
            RowIndex,
            0,
            GTradeVisibleProposalCount - 1);
        mSelectedAmountIndex = 0;
    }

    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTradeWidget::OnAmountButtonClick(int AmountIndex)
{
    mSelectedAmountIndex = ClampInt(
        AmountIndex,
        0,
        GTradeAmountPresetCount - 1);
    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTradeWidget::OnActionButtonClick()
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

    if (ShowingProductPrices || ShowingPriceModifiers || ShowingCompletedRoutes)
    {
        mFeedbackMessage.clear();
        RefreshFromState();
        return;
    }

    if (ShowingActiveRoutes)
    {
        if (!Snapshot.HasSelectedRoute)
        {
            mFeedbackMessage = L"취소할 수 있는 무역 계약이 없습니다.";
            RefreshFromState();
            return;
        }

        auto World = mWorld.lock();
        auto CommandService =
            std::dynamic_pointer_cast<IGovernmentCommandService>(World);

        if (!CommandService)
        {
            mFeedbackMessage = L"행정 명령을 확인할 수 없습니다.";
            RefreshFromState();
            return;
        }

        std::wstring ResponseMessage;
        CommandService->CancelTradeRoute(
            Snapshot.SelectedRoute.RouteId,
            ResponseMessage);
        mFeedbackMessage = ResponseMessage;
        RefreshFromState();
        return;
    }

    if (!Snapshot.HasSelectedProposal)
    {
        mFeedbackMessage = L"체결 가능한 제안이 없습니다.";
        RefreshFromState();
        return;
    }

    const FTradeProposal& Proposal = Snapshot.SelectedProposal;
    const int Amount = GTradeAmountPresets[static_cast<size_t>(ClampInt(
        mSelectedAmountIndex,
        0,
        GTradeAmountPresetCount - 1))];

    if (Amount > Proposal.MaxAmount)
    {
        mFeedbackMessage = L"선택한 물량이 현재 제안 한도를 초과합니다.";
        RefreshFromState();
        return;
    }

    auto World = mWorld.lock();
    auto CommandService =
        std::dynamic_pointer_cast<IGovernmentCommandService>(World);

    if (!CommandService)
    {
        mFeedbackMessage = L"행정 명령을 확인할 수 없습니다.";
        RefreshFromState();
        return;
    }

    std::wstring ResponseMessage;
    CommandService->ExecuteTradeProposal(
        Proposal.ImportRoute,
        Proposal.ResourceType,
        Proposal.ForeignPowerIndex,
        Proposal.OfferPricePerThousand,
        Amount,
        ResponseMessage);
    mFeedbackMessage = ResponseMessage;
    mSelectedPageIndex = 3;
    mSelectedActiveRouteIndex = 0;
    RefreshFromState();
}

void CTradeWidget::OnCompletionAutoOpenButtonClick()
{
    mAutoOpenCompletionPage = !mAutoOpenCompletionPage;
    mFeedbackMessage.clear();
    RefreshFromState();
}
