#include "TradeDiplomacySubsystem.h"
#include "MainWorld.h"
#include "MainWorldTradeRuntime.h"
#include "WorldStatsSnapshot.h"
#include "../Economy/ResourceTradePricing.h"
#include "../Economy/TradePolicy.h"
#include "../GameConstants.h"
#include "../Map/PlacementAreaObject.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace
{
    namespace MWTrade = GameConstants::MainWorld::Trade;
    constexpr int GColonialStarterOfferCount = 5;
    constexpr std::array<EResourceType, 12> GColonialOfferRotation =
    {
        EResourceType::Banana,
        EResourceType::Corn,
        EResourceType::Pineapple,
        EResourceType::Sugar,
        EResourceType::Tobacco,
        EResourceType::Coffee,
        EResourceType::Cocoa,
        EResourceType::Cotton,
        EResourceType::Rubber,
        EResourceType::Coconuts,
        EResourceType::Logs,
        EResourceType::Fish
    };

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

    bool IsDateOnOrAfter(
        int LeftYear,
        int LeftMonth,
        int LeftDay,
        int RightYear,
        int RightMonth,
        int RightDay)
    {
        if (LeftYear != RightYear)
            return LeftYear > RightYear;

        if (LeftMonth != RightMonth)
            return LeftMonth > RightMonth;

        return LeftDay >= RightDay;
    }

    void ClearAvailableTradeOffers(FMainWorldTradeDiplomacyState& State)
    {
        State.AvailableTradeOffers.clear();
        State.TradeOfferRefreshYear = 0;
        State.TradeOfferRefreshMonth = 1;
        State.TradeOfferRefreshDay = 1;
    }

    FTradeOfferRuntimeState BuildTradeOfferState(
        bool ImportRoute,
        EResourceType Type,
        int AvailabilityUnits,
        int Score,
        int SimulationYear,
        int SimulationMonth,
        EBuildingEra CurrentEra,
        int ForcedForeignPowerIndex,
        int CustomsModifierPercent,
        int OfferId,
        const std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount>& ForeignPowers)
    {
        FTradeOfferRuntimeState Result;
        Result.OfferId = OfferId;
        Result.ImportRoute = ImportRoute;
        Result.ResourceType = Type;
        Result.MarketClass = GetResourceMarketClass(Type);
        Result.AvailabilityUnits = AvailabilityUnits;
        Result.Score = Score;

        const int BaseMaxAmount = ClampInt(
            ImportRoute ?
                RoundUpToThousand((std::max)(1000, AvailabilityUnits)) :
                RoundDownToThousand(AvailabilityUnits),
            1000,
            12000);
        const int SafePartner = ClampInt(
            ForcedForeignPowerIndex,
            0,
            TradeDiplomacyRuntime::GForeignPowerCount - 1);
        const auto& Partner = ForeignPowers[static_cast<size_t>(SafePartner)];
        const double PartnerWeight =
            ResolvePartnerWeight(Result.MarketClass, SafePartner);
        const double RelationBias =
            static_cast<double>(Partner.Relation - 50) / 100.0;
        const double TradeBias =
            static_cast<double>(Partner.TradeModifier) / 100.0;
        const double SeedBias = ComputeOfferSeed(
            SimulationYear,
            SimulationMonth,
            Type,
            SafePartner,
            ImportRoute);
        const double VolumeMultiplier = ClampDouble(
            1.0 +
                static_cast<double>(Partner.Standing) / 220.0 +
                static_cast<double>(Partner.Relation - 50) / 280.0,
            0.70,
            1.35);
        const int StandardPricePerUnit = ImportRoute ?
            ResourceTradePricing::GetImportPricePerStockUnit(Type) :
            ResourceTradePricing::GetExportPricePerStockUnit(Type);
        const int StandardPricePerThousand =
            (std::max)(1000, StandardPricePerUnit * 1000);

        Result.ForeignPowerIndex = SafePartner;
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

        Result.BasePricePerThousand = StandardPricePerThousand;
        Result.OfferPricePerThousand = ApplyTradePriceModifierPercent(
            OfferPricePerThousand,
            CustomsModifierPercent);

        if (StandardPricePerThousand > 0)
        {
            Result.MarginPercent = static_cast<int>(std::lround(
                ImportRoute ?
                    static_cast<double>(
                        StandardPricePerThousand -
                        Result.OfferPricePerThousand) * 100.0 /
                        static_cast<double>(StandardPricePerThousand) :
                    static_cast<double>(
                        Result.OfferPricePerThousand -
                        StandardPricePerThousand) * 100.0 /
                        static_cast<double>(StandardPricePerThousand)));
        }

        if (CurrentEra == EBuildingEra::Colonial)
            Result.ForeignPowerIndex = 0;

        return Result;
    }

    void ResolveNextTradeOfferRefreshDate(
        const CTradeDiplomacySubsystem::FRefreshForeignTradeContext& Context,
        const CMainWorld& World,
        int& OutYear,
        int& OutMonth,
        int& OutDay)
    {
        OutYear = Context.SimulationYear + 1;
        OutMonth = ClampInt(Context.SimulationMonth, 1, 12);
        const CSimulationSubsystem* const Simulation = World.GetSimulation();
        const int MaxDay = Simulation ?
            Simulation->GetDaysInMonth(OutYear, OutMonth) :
            31;
        OutDay = ClampInt(Context.SimulationDay, 1, MaxDay);
    }

    void RefreshColonialTradeOffersIfNeeded(
        CTradeDiplomacySubsystem& Subsystem,
        const CTradeDiplomacySubsystem::FRefreshForeignTradeContext& Context,
        const CMainWorld& World)
    {
        FMainWorldTradeDiplomacyState& State = Subsystem.State;

        if (Context.CurrentEra != EBuildingEra::Colonial)
        {
            ClearAvailableTradeOffers(State);
            return;
        }

        const bool HasRefreshDate = State.TradeOfferRefreshYear > 0;
        const bool NeedsRefresh =
            !HasRefreshDate ||
            IsDateOnOrAfter(
                Context.SimulationYear,
                Context.SimulationMonth,
                Context.SimulationDay,
                State.TradeOfferRefreshYear,
                State.TradeOfferRefreshMonth,
                State.TradeOfferRefreshDay);

        if (!NeedsRefresh)
            return;

        State.AvailableTradeOffers.clear();

        const int RotationBase = ClampInt(
            State.TradeOfferRotationIndex,
            0,
            static_cast<int>(GColonialOfferRotation.size()) - 1);
        const int CustomsModifierPercent =
            Subsystem.GetCustomsExportTradePriceModifierPercent();

        for (int OfferOffset = 0;
            OfferOffset < GColonialStarterOfferCount;
            ++OfferOffset)
        {
            const int CandidateIndex =
                (RotationBase + OfferOffset) %
                static_cast<int>(GColonialOfferRotation.size());
            const EResourceType ResourceType =
                GColonialOfferRotation[static_cast<size_t>(CandidateIndex)];
            const int AvailabilityUnits = 4000 +
                ((CandidateIndex + Context.SimulationYear) % 5) * 1000;
            const int Score =
                AvailabilityUnits *
                ResourceTradePricing::GetExportPricePerStockUnit(ResourceType);
            State.AvailableTradeOffers.push_back(
                BuildTradeOfferState(
                    false,
                    ResourceType,
                    AvailabilityUnits,
                    Score,
                    Context.SimulationYear,
                    Context.SimulationMonth,
                    Context.CurrentEra,
                    0,
                    CustomsModifierPercent,
                    State.NextTradeOfferId++,
                    State.ForeignPowerStates));
        }

        State.TradeOfferRotationIndex =
            (RotationBase + GColonialStarterOfferCount) %
            static_cast<int>(GColonialOfferRotation.size());
        ResolveNextTradeOfferRefreshDate(
            Context,
            World,
            State.TradeOfferRefreshYear,
            State.TradeOfferRefreshMonth,
            State.TradeOfferRefreshDay);
    }

    void ConsumeMatchingAvailableTradeOffer(
        FMainWorldTradeDiplomacyState& State,
        bool ImportRoute,
        EResourceType ResourceType,
        int ForeignPowerIndex,
        int OfferPricePerThousandUnits)
    {
        const auto OfferIt = std::find_if(
            State.AvailableTradeOffers.begin(),
            State.AvailableTradeOffers.end(),
            [ImportRoute,
                ResourceType,
                ForeignPowerIndex,
                OfferPricePerThousandUnits](
                    const FTradeOfferRuntimeState& Offer)
            {
                return Offer.ImportRoute == ImportRoute &&
                    Offer.ResourceType == ResourceType &&
                    Offer.ForeignPowerIndex == ForeignPowerIndex &&
                    Offer.OfferPricePerThousand == OfferPricePerThousandUnits;
            });

        if (OfferIt != State.AvailableTradeOffers.end())
            State.AvailableTradeOffers.erase(OfferIt);
    }

    std::wstring BuildExportBlockedSelectionText(
        const TradePolicy::FExportTradePolicy& Policy)
    {
        std::vector<std::wstring> BlockedResources;

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsExportableResourceType(ResourceType) ||
                !IsImmediateProductionScopeResourceType(ResourceType) ||
                TradePolicy::IsResourceExportAllowed(Policy, ResourceType))
            {
                continue;
            }

            BlockedResources.push_back(
                GetResourceTypeDisplayName(ResourceType));
        }

        if (BlockedResources.empty())
            return L"없음";

        std::wstring Result;

        for (size_t Index = 0; Index < BlockedResources.size(); ++Index)
        {
            if (Index > 0)
                Result += L", ";

            Result += BlockedResources[Index];
        }

        return Result;
    }

}

void CTradeDiplomacySubsystem::Reset()
{
    State.ActiveTradeRoutes.clear();
    State.CompletedTradeRoutes.clear();
    State.ForeignPowerStandingStates = {};
    State.ForeignPowerStates = {};
    State.AvailableTradeOffers.clear();
    State.NextTradeRouteId = 1;
    State.NextTradeRouteCompletionRecordId = 1;
    State.TradeRouteCompletionNotificationVersion = 0;
    State.NextTradeOfferId = 1;
    State.TradeOfferRefreshYear = 0;
    State.TradeOfferRefreshMonth = 1;
    State.TradeOfferRefreshDay = 1;
    State.TradeOfferRotationIndex = 0;
}

void CTradeDiplomacySubsystem::OnDayAdvanced(
    const FRefreshForeignTradeContext& Context)
{
    if (!mOwner)
        return;

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
    {
        State.ForeignPowerStates = {};
        return;
    }

    std::array<int, TradeDiplomacyRuntime::GForeignPowerCount> ActiveCounts = {};

    for (size_t RouteIndex = 0;
        RouteIndex < State.ActiveTradeRoutes.size();
        ++RouteIndex)
    {
        const int SafeIndex = TradeDiplomacyRuntime::ClampInt(
            State.ActiveTradeRoutes[RouteIndex].ForeignPowerIndex,
            0,
            TradeDiplomacyRuntime::GForeignPowerCount - 1);
        ++ActiveCounts[static_cast<size_t>(SafeIndex)];
    }

    for (int Index = 0;
        Index < TradeDiplomacyRuntime::GForeignPowerCount;
        ++Index)
    {
        auto& StandingState =
            State.ForeignPowerStandingStates[
                static_cast<size_t>(Index)];
        StandingState.ActiveContractCount =
            ActiveCounts[static_cast<size_t>(Index)];

        if (Context.ApplyIdleDecay)
            MainWorldTradeRuntime::ApplyForeignPowerIdleDecay(StandingState);
    }

    State.ForeignPowerStates =
        TradeDiplomacyRuntime::BuildForeignPowerWorldStates(
            WorldStats::BuildSnapshot(World),
            Context.GovernmentProfile,
            Context.TaxEventStatus,
            Context.GovernmentEdicts,
            Context.CurrentEra,
            State.ForeignPowerStandingStates);
    RefreshColonialTradeOffersIfNeeded(*this, Context, *mOwner);
}

void CTradeDiplomacySubsystem::RecordFinishedTradeRoute(
    const FTradeRouteRuntimeState& Route,
    ETradeRouteEndReason EndReason)
{
    FTradeRouteCompletionRecord Record;
    Record.RecordId = State.NextTradeRouteCompletionRecordId++;
    Record.RouteId = Route.RouteId;
    Record.ImportRoute = Route.ImportRoute;
    Record.ResourceType = Route.ResourceType;
    Record.MarketClass = Route.MarketClass;
    Record.ForeignPowerIndex = Route.ForeignPowerIndex;
    Record.ContractUnits = Route.ContractUnits;
    Record.FulfilledUnits = Route.FulfilledUnits;
    Record.ElapsedDays = (std::max)(
        0,
        Route.TotalDurationDays - Route.RemainingDays);
    Record.TotalDurationDays = Route.TotalDurationDays;
    Record.SettledValue =
        static_cast<long long>(Route.RoutePricePerThousandUnits) *
        static_cast<long long>(Route.FulfilledUnits) / 1000LL;
    Record.EndReason = EndReason;
    Record.CompletionRewardModifier =
        MainWorldTradeRuntime::ResolveTradeRouteCompletionRewardModifier(
            Route,
            EndReason);
    Record.SecondaryRelationModifier =
        MainWorldTradeRuntime::ResolveTradeRouteSecondaryRelationModifier(
            Route,
            EndReason);
    Record.StandingModifier =
        MainWorldTradeRuntime::ResolveTradeRouteStandingModifier(
            Route,
            EndReason);
    MainWorldTradeRuntime::ApplyTradeRouteCompletionState(
        State.ForeignPowerStandingStates[static_cast<size_t>(
            TradeDiplomacyRuntime::ClampInt(
                Route.ForeignPowerIndex,
                0,
                TradeDiplomacyRuntime::GForeignPowerCount - 1))],
        Record);

    State.CompletedTradeRoutes.insert(
        State.CompletedTradeRoutes.begin(),
        Record);

    if (State.CompletedTradeRoutes.size() >
        static_cast<size_t>(MWTrade::MaxCompletedTradeRouteRecordCount))
    {
        State.CompletedTradeRoutes.resize(
            static_cast<size_t>(MWTrade::MaxCompletedTradeRouteRecordCount));
    }

    ++State.TradeRouteCompletionNotificationVersion;
}

void CTradeDiplomacySubsystem::CancelTradeRoutesForInactivePowers(EBuildingEra Era)
{
    (void)Era;

    if (State.ActiveTradeRoutes.empty())
        return;

    for (const FTradeRouteRuntimeState& Route : State.ActiveTradeRoutes)
        RecordFinishedTradeRoute(Route, ETradeRouteEndReason::EraTransitioned);

    State.ActiveTradeRoutes.clear();
}

void CTradeDiplomacySubsystem::ProcessActiveRoutes()
{
    if (!mOwner || State.ActiveTradeRoutes.empty())
        return;

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
        return;

    std::vector<std::shared_ptr<CPlacementAreaObject>> Harbors =
        MainWorldTradeRuntime::CollectOperationalHarbors(World);
    std::vector<FTradeRouteRuntimeState> RemainingRoutes;
    RemainingRoutes.reserve(State.ActiveTradeRoutes.size());
    bool BudgetChanged = false;

    for (size_t RouteIndex = 0;
        RouteIndex < State.ActiveTradeRoutes.size();
        ++RouteIndex)
    {
        FTradeRouteRuntimeState Route = State.ActiveTradeRoutes[RouteIndex];
        const int RemainingUnits = (std::max)(
            0,
            Route.ContractUnits - Route.FulfilledUnits);

        if (RemainingUnits <= 0)
        {
            RecordFinishedTradeRoute(Route, ETradeRouteEndReason::Completed);
            continue;
        }

        int DailyTransferUnits = (std::min)(
            RemainingUnits,
            MainWorldTradeRuntime::ResolveTradeRouteDailyTransferUnits(Route));

        if (Route.ImportRoute)
        {
            if (Route.RoutePricePerThousandUnits > 0)
            {
                const long long MaxAffordableUnits =
                    mOwner->mEconomy->NationalBudget > 0 ?
                        (mOwner->mEconomy->NationalBudget * 1000LL) /
                        static_cast<long long>(Route.RoutePricePerThousandUnits) :
                        0LL;
                DailyTransferUnits = (std::min)(
                    DailyTransferUnits,
                    static_cast<int>((std::max)(0LL, MaxAffordableUnits)));
            }

            struct FHarborImportAllocation
            {
                std::shared_ptr<CPlacementAreaObject> Harbor;
                int Capacity = 0;
            };

            std::vector<FHarborImportAllocation> Allocations;
            Allocations.reserve(Harbors.size());

            for (size_t HarborIndex = 0; HarborIndex < Harbors.size(); ++HarborIndex)
            {
                FHarborImportAllocation Allocation;
                Allocation.Harbor = Harbors[HarborIndex];
                Allocation.Capacity =
                    Harbors[HarborIndex]->GetAvailableIncomingCapacity(
                        Route.ResourceType);

                if (Allocation.Capacity > 0)
                    Allocations.push_back(std::move(Allocation));
            }

            std::sort(
                Allocations.begin(),
                Allocations.end(),
                [](const FHarborImportAllocation& A,
                    const FHarborImportAllocation& B)
                {
                    return A.Capacity > B.Capacity;
                });

            int ImportedUnits = 0;
            int RemainingTransferUnits = DailyTransferUnits;

            for (size_t HarborIndex = 0;
                HarborIndex < Allocations.size() && RemainingTransferUnits > 0;
                ++HarborIndex)
            {
                const int AssignedUnits = (std::min)(
                    RemainingTransferUnits,
                    Allocations[HarborIndex].Capacity);

                if (AssignedUnits <= 0)
                    continue;

                if (!Allocations[HarborIndex].Harbor->TryAddResourceStock(
                        Route.ResourceType,
                        AssignedUnits))
                {
                    continue;
                }

                ImportedUnits += AssignedUnits;
                RemainingTransferUnits -= AssignedUnits;
            }

            if (ImportedUnits > 0)
            {
                const long long ImportCost =
                    static_cast<long long>(Route.RoutePricePerThousandUnits) *
                    static_cast<long long>(ImportedUnits) / 1000LL;
                mOwner->mEconomy->NationalBudget -= ImportCost;
                mOwner->mEconomy->LastDailyImportExpense += ImportCost;
                mOwner->mEconomy->LastDailyNetChange -= ImportCost;
                Route.FulfilledUnits += ImportedUnits;
                BudgetChanged = true;
            }
        }
        else
        {
            std::sort(
                Harbors.begin(),
                Harbors.end(),
                [&Route](const std::shared_ptr<CPlacementAreaObject>& A,
                    const std::shared_ptr<CPlacementAreaObject>& B)
                {
                    return A->GetAvailableResourceStock(Route.ResourceType) >
                        B->GetAvailableResourceStock(Route.ResourceType);
                });

            int ExportedUnits = 0;
            int RemainingTransferUnits = DailyTransferUnits;

            for (size_t HarborIndex = 0;
                HarborIndex < Harbors.size() && RemainingTransferUnits > 0;
                ++HarborIndex)
            {
                const int AvailableUnits =
                    Harbors[HarborIndex]->GetAvailableResourceStock(
                        Route.ResourceType);
                const int ExportUnits = (std::min)(
                    RemainingTransferUnits,
                    AvailableUnits);

                if (ExportUnits <= 0)
                    continue;

                if (!Harbors[HarborIndex]->TryConsumeResource(
                        Route.ResourceType,
                        ExportUnits))
                {
                    continue;
                }

                ExportedUnits += ExportUnits;
                RemainingTransferUnits -= ExportUnits;
            }

            if (ExportedUnits > 0)
            {
                const long long ExportIncome =
                    static_cast<long long>(Route.RoutePricePerThousandUnits) *
                    static_cast<long long>(ExportedUnits) / 1000LL;
                mOwner->mEconomy->NationalBudget += ExportIncome;
                mOwner->mEconomy->LastDailyExportIncome += ExportIncome;
                mOwner->mEconomy->LastDailyNetChange += ExportIncome;
                Route.FulfilledUnits += ExportedUnits;
                BudgetChanged = true;
            }
        }

        Route.RemainingDays = (std::max)(0, Route.RemainingDays - 1);

        if (Route.FulfilledUnits < Route.ContractUnits &&
            Route.RemainingDays > 0)
        {
            RemainingRoutes.push_back(std::move(Route));
        }
        else
        {
            RecordFinishedTradeRoute(
                Route,
                Route.FulfilledUnits >= Route.ContractUnits ?
                    ETradeRouteEndReason::Completed :
                    ETradeRouteEndReason::Expired);
        }
    }

    State.ActiveTradeRoutes.swap(RemainingRoutes);

    if (BudgetChanged)
    {
        mOwner->mEconomy->RefreshWorldMarketPrices(
            {
                mOwner->mPolitics->GovernmentProfile,
                mOwner->mEdictState->GovernmentEdicts,
                mOwner->mCrisis->WorldCrisisService->GetStatus(),
                State.ForeignPowerStates,
                mOwner->mSimulation->Year,
                mOwner->mSimulation->Month,
                mOwner->mSimulation->Day
            });
        mOwner->mPolitics->RefreshSnapshot(
            {
                &mOwner->mEconomy->TaxEventStatus,
                &mOwner->mKnowledgeState->ConstitutionState.ActiveEffects
            });
    }
}

bool CTradeDiplomacySubsystem::ExecuteTradeProposal(
    bool ImportRoute,
    EResourceType ResourceType,
    int ForeignPowerIndex,
    int PricePerThousandUnits,
    int Amount,
    std::wstring& OutMessage)
{
    if (!mOwner)
    {
        OutMessage = L"월드 상태를 확인할 수 없습니다.";
        return false;
    }

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
    {
        OutMessage = L"월드 상태를 확인할 수 없습니다.";
        return false;
    }

    if (static_cast<int>(State.ActiveTradeRoutes.size()) >=
        MWTrade::MaxActiveTradeRouteCount)
    {
        OutMessage = L"활성화할 수 있는 무역로가 가득 찼습니다.";
        return false;
    }

    if (!IsExportableResourceType(ResourceType))
    {
        OutMessage = L"유효하지 않은 자원 제안입니다.";
        return false;
    }

    const int SafeAmount = (std::max)(
        MWTrade::MinAmountUnits,
        (std::min)(MWTrade::MaxAmountUnits, Amount));

    if (SafeAmount < MWTrade::MinAmountUnits)
    {
        OutMessage = L"제안 물량이 너무 작습니다.";
        return false;
    }

    const int SafePricePerThousand = (std::max)(1000, PricePerThousandUnits);
    const std::vector<std::shared_ptr<CPlacementAreaObject>> Harbors =
        MainWorldTradeRuntime::CollectOperationalHarbors(World);

    if (Harbors.empty())
    {
        OutMessage = L"운영 중인 항구가 없어 무역로를 활성화할 수 없습니다.";
        return false;
    }

    FTradeRouteRuntimeState Route;
    Route.RouteId = State.NextTradeRouteId++;
    Route.ImportRoute = ImportRoute;
    Route.ResourceType = ResourceType;
    Route.MarketClass = GetResourceMarketClass(ResourceType);
    Route.ForeignPowerIndex =
        (std::max)(0, (std::min)(4, ForeignPowerIndex));
    Route.ContractUnits = SafeAmount;
    Route.FulfilledUnits = 0;
    Route.TotalDurationDays =
        MainWorldTradeRuntime::ResolveTradeRouteDurationDays(SafeAmount);
    Route.RemainingDays = Route.TotalDurationDays;
    Route.RoutePricePerThousandUnits = SafePricePerThousand;
    Route.SignedStandardPricePerThousandUnits =
        MainWorldTradeRuntime::ComputeTradeRouteSignedStandardPricePerThousand(
            ResourceType,
            ImportRoute);
    State.ActiveTradeRoutes.push_back(Route);
    ConsumeMatchingAvailableTradeOffer(
        State,
        ImportRoute,
        ResourceType,
        Route.ForeignPowerIndex,
        SafePricePerThousand);
    MainWorldTradeRuntime::ApplyTradeRouteActivationState(
        State.ForeignPowerStandingStates[static_cast<size_t>(
            Route.ForeignPowerIndex)],
        Route);

    OnDayAdvanced(
        {
            mOwner->mPolitics->GovernmentProfile,
            mOwner->mEconomy->TaxEventStatus,
            mOwner->mEdictState->GovernmentEdicts,
            mOwner->mEraState->EraProgress.CurrentEra,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day,
            false
        });
    mOwner->mEconomy->RefreshWorldMarketPrices(
        {
            mOwner->mPolitics->GovernmentProfile,
            mOwner->mEdictState->GovernmentEdicts,
            mOwner->mCrisis->WorldCrisisService->GetStatus(),
            State.ForeignPowerStates,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day
        });
    mOwner->mPolitics->RefreshSnapshot(
        {
            &mOwner->mEconomy->TaxEventStatus,
            &mOwner->mKnowledgeState->ConstitutionState.ActiveEffects
        });
    OutMessage =
        std::wstring(MainWorldTradeRuntime::GetForeignPowerName(
            ForeignPowerIndex,
            mOwner->mEraState->EraProgress.CurrentEra)) +
        L"과(와) " +
        GetResourceTypeDisplayName(ResourceType) +
        L" " +
        MainWorldTradeRuntime::FormatUnits(SafeAmount) +
        L" 단위 무역로 활성화";
    return true;
}

bool CTradeDiplomacySubsystem::CancelTradeRoute(
    int RouteId,
    std::wstring& OutMessage)
{
    const auto RouteIt = std::find_if(
        State.ActiveTradeRoutes.begin(),
        State.ActiveTradeRoutes.end(),
        [RouteId](const FTradeRouteRuntimeState& Route)
        {
            return Route.RouteId == RouteId;
        });

    if (RouteIt == State.ActiveTradeRoutes.end())
    {
        OutMessage = L"취소할 수 있는 무역 계약이 없습니다.";
        return false;
    }

    const std::wstring DirectionText =
        RouteIt->ImportRoute ? L"수입" : L"수출";
    const std::wstring ResourceName =
        GetResourceTypeDisplayName(RouteIt->ResourceType);

    RecordFinishedTradeRoute(*RouteIt, ETradeRouteEndReason::Cancelled);
    State.ActiveTradeRoutes.erase(RouteIt);
    OnDayAdvanced(
        {
            mOwner->mPolitics->GovernmentProfile,
            mOwner->mEconomy->TaxEventStatus,
            mOwner->mEdictState->GovernmentEdicts,
            mOwner->mEraState->EraProgress.CurrentEra,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day,
            false
        });
    mOwner->mEconomy->RefreshWorldMarketPrices(
        {
            mOwner->mPolitics->GovernmentProfile,
            mOwner->mEdictState->GovernmentEdicts,
            mOwner->mCrisis->WorldCrisisService->GetStatus(),
            State.ForeignPowerStates,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day
        });
    mOwner->mPolitics->RefreshSnapshot(
        {
            &mOwner->mEconomy->TaxEventStatus,
            &mOwner->mKnowledgeState->ConstitutionState.ActiveEffects
        });
    OutMessage =
        DirectionText +
        L": " +
        ResourceName +
        L" 무역 계약 취소";
    return true;
}

bool CTradeDiplomacySubsystem::CycleExportBlockedResource(std::wstring& OutMessage)
{
    if (!mOwner)
        return false;

    TradePolicy::AdvanceExportBlockedResourceSelection(
        mOwner->mPolitics->GovernmentProfile.ExportTradePolicy);
    OutMessage =
        L"수출 금지 자원: " +
        BuildExportBlockedSelectionText(
            mOwner->mPolitics->GovernmentProfile.ExportTradePolicy);
    return true;
}

int CTradeDiplomacySubsystem::GetCustomsExportTradePriceModifierPercent() const
{
    if (!mOwner)
        return 0;

    return MainWorldTradeRuntime::CollectCustomsTradeModifierSummary(
        mOwner).ExportPricePercent;
}

int CTradeDiplomacySubsystem::GetCustomsImportTradePriceModifierPercent() const
{
    if (!mOwner)
        return 0;

    return MainWorldTradeRuntime::CollectCustomsTradeModifierSummary(
        mOwner).ImportPricePercent;
}
