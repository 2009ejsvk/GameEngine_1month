#include "MainWorldTradeRuntime.h"
#include "World/World.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/PlacementAreaObject.h"
#include "../Economy/ResourceTradePricing.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr int GTradeRouteMinDailyTransferUnits = 150;
    constexpr int GTradeRouteMaxDailyTransferUnits = 1200;
    constexpr int GTradeRouteDefaultDurationDays = 1500;

    int ResolveTradeRouteActivationRelationModifier(
        const FTradeRouteRuntimeState& Route)
    {
        const int SizeTier = (std::max)(1, Route.ContractUnits / 1500);
        return TradeDiplomacyRuntime::ClampInt(
            1 + SizeTier / 2 + (Route.ImportRoute ? 0 : 1),
            1,
            5);
    }
}

namespace MainWorldTradeRuntime
{
    const wchar_t* GetForeignPowerName(int Index, EBuildingEra Era)
    {
        return TradeDiplomacyRuntime::GetForeignPowerName(Index, Era);
    }

    std::wstring FormatCurrency(long long Value)
    {
        const bool Negative = Value < 0;
        const unsigned long long AbsValue = Negative ?
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

    std::wstring FormatUnits(int Value)
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

    int CountActiveTradeRoutesForPower(
        const std::vector<FTradeRouteRuntimeState>& ActiveRoutes,
        int ForeignPowerIndex)
    {
        int Count = 0;

        for (size_t Index = 0; Index < ActiveRoutes.size(); ++Index)
        {
            if (ActiveRoutes[Index].ForeignPowerIndex == ForeignPowerIndex)
                ++Count;
        }

        return Count;
    }

    int ComputeTradeRouteSignedStandardPricePerThousand(
        EResourceType Type,
        bool ImportRoute)
    {
        const int PricePerUnit = ImportRoute ?
            ResourceTradePricing::GetImportPricePerStockUnit(Type) :
            ResourceTradePricing::GetExportPricePerStockUnit(Type);
        return (std::max)(1000, PricePerUnit * 1000);
    }

    int ResolveTradeRouteDurationDays(int ContractUnits)
    {
        const int CargoScaleDays =
            (std::max)(0, ContractUnits / 20);
        return (std::max)(
            540,
            (std::min)(
                GTradeRouteDefaultDurationDays,
                420 + CargoScaleDays));
    }

    int ResolveTradeRouteDailyTransferUnits(
        const FTradeRouteRuntimeState& Route)
    {
        const int RemainingUnits = (std::max)(
            0,
            Route.ContractUnits - Route.FulfilledUnits);
        const int RemainingDays = (std::max)(1, Route.RemainingDays);
        const int TimedTarget = static_cast<int>(std::ceil(
            static_cast<double>(RemainingUnits) /
            static_cast<double>((std::min)(RemainingDays, 120))));

        return (std::max)(
            GTradeRouteMinDailyTransferUnits,
            (std::min)(
                GTradeRouteMaxDailyTransferUnits,
                TimedTarget));
    }

    int ResolveTradeRouteCompletionRewardModifier(
        const FTradeRouteRuntimeState& Route,
        ETradeRouteEndReason EndReason)
    {
        const int SizeTier = (std::max)(1, Route.ContractUnits / 1500);

        switch (EndReason)
        {
        case ETradeRouteEndReason::Completed:
            return 5 + SizeTier;
        case ETradeRouteEndReason::Cancelled:
            return -(2 + SizeTier / 2);
        case ETradeRouteEndReason::Expired:
        default:
            return -(1 + SizeTier / 3);
        }
    }

    int ResolveTradeRouteSecondaryRelationModifier(
        const FTradeRouteRuntimeState& Route,
        ETradeRouteEndReason EndReason)
    {
        const int BaseModifier =
            ResolveTradeRouteCompletionRewardModifier(Route, EndReason);

        if (EndReason == ETradeRouteEndReason::Completed)
            return BaseModifier;

        return BaseModifier / 2;
    }

    int ResolveTradeRouteStandingModifier(
        const FTradeRouteRuntimeState& Route,
        ETradeRouteEndReason EndReason)
    {
        const int SizeTier = (std::max)(1, Route.ContractUnits / 1500);

        switch (EndReason)
        {
        case ETradeRouteEndReason::Completed:
            return (std::max)(2, SizeTier);
        case ETradeRouteEndReason::Cancelled:
            return -(std::max)(1, SizeTier / 2);
        case ETradeRouteEndReason::Expired:
        default:
            return -(std::max)(1, SizeTier / 3);
        }
    }

    void ApplyTradeRouteActivationState(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState,
        const FTradeRouteRuntimeState& Route)
    {
        ++InOutState.SignedContractCount;
        InOutState.LastStandingChange = 0;
        InOutState.LastRelationChange =
            ResolveTradeRouteActivationRelationModifier(Route);
        InOutState.RelationModifier = TradeDiplomacyRuntime::ClampInt(
            InOutState.RelationModifier + InOutState.LastRelationChange,
            -35,
            35);
        InOutState.IdleDays = 0;
    }

    void ApplyTradeRouteCompletionState(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState,
        const FTradeRouteCompletionRecord& Record)
    {
        if (Record.EndReason == ETradeRouteEndReason::Completed)
            ++InOutState.CompletedContractCount;
        else
            ++InOutState.FailedContractCount;

        InOutState.LastRelationChange = Record.SecondaryRelationModifier;
        InOutState.LastStandingChange = Record.StandingModifier;
        InOutState.RelationModifier = TradeDiplomacyRuntime::ClampInt(
            InOutState.RelationModifier + Record.SecondaryRelationModifier,
            -35,
            35);
        InOutState.Standing = TradeDiplomacyRuntime::ClampStanding(
            InOutState.Standing + Record.StandingModifier);
        InOutState.IdleDays = 0;
    }

    void ApplyForeignDemandStandingDelta(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState,
        int RelationDelta,
        int StandingDelta)
    {
        InOutState.LastRelationChange = RelationDelta;
        InOutState.LastStandingChange = StandingDelta;
        InOutState.RelationModifier = TradeDiplomacyRuntime::ClampInt(
            InOutState.RelationModifier + RelationDelta,
            -35,
            35);
        InOutState.Standing = TradeDiplomacyRuntime::ClampStanding(
            InOutState.Standing + StandingDelta);
        InOutState.IdleDays = 0;
    }

    void ApplyForeignPowerIdleDecay(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState)
    {
        if (InOutState.ActiveContractCount > 0)
        {
            InOutState.IdleDays = 0;
            return;
        }

        ++InOutState.IdleDays;

        if (InOutState.IdleDays % 20 == 0 && InOutState.Standing != 0)
        {
            InOutState.Standing += InOutState.Standing > 0 ? -1 : 1;
        }

        if (InOutState.IdleDays % 30 == 0 &&
            InOutState.RelationModifier != 0)
        {
            InOutState.RelationModifier +=
                InOutState.RelationModifier > 0 ? -1 : 1;
        }
    }

    std::vector<std::shared_ptr<CPlacementAreaObject>> CollectOperationalHarbors(
        const std::shared_ptr<CWorld>& World)
    {
        std::vector<std::shared_ptr<CPlacementAreaObject>> Harbors;

        if (!World)
            return Harbors;

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
            return Harbors;

        for (size_t Index = 0; Index < BuildingList.size(); ++Index)
        {
            auto Building = BuildingList[Index].lock();

            if (!Building ||
                !Building->GetAlive() ||
                !Building->GetEnable() ||
                !Building->HasPlacedArea() ||
                !Building->IsHarbor())
            {
                continue;
            }

            Harbors.push_back(Building);
        }

        return Harbors;
    }

    FCustomsTradeModifierSummary CollectCustomsTradeModifierSummary(
        CWorld* World)
    {
        FCustomsTradeModifierSummary Result;

        if (!World)
            return Result;

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
            return Result;

        for (size_t Index = 0; Index < BuildingList.size(); ++Index)
        {
            auto Building = BuildingList[Index].lock();

            if (!Building ||
                !Building->GetAlive() ||
                !Building->GetEnable() ||
                !Building->HasPlacedArea() ||
                !IsCustomsOfficeBuildingId(Building->GetBuildingId()))
            {
                continue;
            }

            const int ExportModifier =
                Building->GetTradeRouteExportPriceModifierPercent();

            if (ExportModifier > 0)
            {
                Result.ExportPricePercent = (std::max)(
                    Result.ExportPricePercent,
                    ExportModifier);
            }
            else if (Result.ExportPricePercent <= 0)
            {
                Result.ExportPricePercent = (std::min)(
                    Result.ExportPricePercent,
                    ExportModifier);
            }

            const int ImportModifier =
                Building->GetTradeRouteImportPriceModifierPercent();

            if (ImportModifier < 0)
            {
                Result.ImportPricePercent = (std::min)(
                    Result.ImportPricePercent,
                    ImportModifier);
            }
            else if (Result.ImportPricePercent >= 0)
            {
                Result.ImportPricePercent = (std::max)(
                    Result.ImportPricePercent,
                    ImportModifier);
            }
        }

        return Result;
    }
}
