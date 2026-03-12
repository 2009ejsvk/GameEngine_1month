#pragma once

#include "../Building/BuildingTypes.h"
#include "../Economy/TradeRouteRuntimeState.h"
#include <array>
#include <memory>
#include <string>
#include <vector>

class CWorld;

namespace TradeWidgetRuntime
{
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

    enum class ETradeDetailValueTone
    {
        Default = 0,
        Positive,
        Negative
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

    struct FTradeDetailRowSnapshot
    {
        std::wstring Label = L"-";
        std::wstring Value = L"-";
        ETradeDetailValueTone Tone = ETradeDetailValueTone::Default;
    };

    struct FTradeAmountButtonSnapshot
    {
        bool Visible = false;
        bool Enabled = false;
        std::wstring Text;
    };

    struct FTradeDetailSnapshot
    {
        std::wstring TitleText;
        std::array<FTradeDetailRowSnapshot, GTradeDetailRowCount> Rows = {};
        bool ShowAmountTitle = false;
        std::wstring AmountTitleText;
        std::array<FTradeAmountButtonSnapshot, GTradeAmountPresetCount>
            AmountButtons = {};
    };

    int ClampInt(int Value, int MinValue, int MaxValue);

    const wchar_t* GetFilterDisplayText(ETradeFilterType FilterType);
    std::wstring FormatCurrency(long long Value);
    std::wstring FormatSignedPercent(int Value);
    std::wstring FormatSignedInteger(int Value);
    std::wstring FormatInteger(int Value);
    std::wstring FormatTradeProgress(int CurrentValue, int TotalValue);
    std::wstring FormatRemainingTradeTime(int RemainingDays);
    long long ResolveTradeTotalPrice(const FTradeProposal& Proposal, int Amount);

    FTradeWidgetSnapshot BuildSnapshot(
        const std::shared_ptr<CWorld>& World,
        int SelectedPageIndex,
        int FilterIndex,
        int SortIndex,
        bool SortDescending,
        int SelectedProposalIndex,
        int SelectedPriceIndex,
        int SelectedActiveRouteIndex,
        int SelectedCompletedRouteIndex);

    FTradeDetailSnapshot BuildDetailSnapshot(
        const std::shared_ptr<CWorld>& World,
        const FTradeWidgetSnapshot& Snapshot,
        int SelectedPageIndex,
        int SelectedAmountIndex);
}
