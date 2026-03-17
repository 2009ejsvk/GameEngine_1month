#include "EconomySystem.h"
#include "ResourceTradePricing.h"
#include "TradePolicyRuntime.h"
#include "../Politics/EdictBudgetRuntime.h"
#include "../GameConstants.h"
#include "../World/EconomyWorldAccess.h"
#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace
{
    constexpr size_t GResourceTypeCount =
        static_cast<size_t>(EResourceType::Count);

    struct FAssignedHarborImportDemand
    {
        const EconomyWorldAccess::IEconomyBuildingAccess* Harbor = nullptr;
        std::array<int, GResourceTypeCount> AssignedShortageByType = {};
    };

    struct FAssignedHarborImportPlan
    {
        const EconomyWorldAccess::IEconomyBuildingAccess* Harbor = nullptr;
        std::array<int, GResourceTypeCount> AssignedNormalImportByType = {};
        std::array<int, GResourceTypeCount> AssignedEmergencyImportByType = {};
    };

    struct FAssignedHarborExportPlan
    {
        const EconomyWorldAccess::IEconomyBuildingAccess* Harbor = nullptr;
        std::array<int, GResourceTypeCount> AssignedExportByType = {};
    };

    struct FHarborTradeDemandSnapshot
    {
        int HarborCount = 0;
        std::array<int, GResourceTypeCount> ConsumerShortageByType = {};
        std::array<int, GResourceTypeCount> ConsumerReserveWeightMilliByType =
            {};
        std::array<int, GResourceTypeCount> RemainingAvailableByType = {};
        std::vector<FAssignedHarborImportDemand> AssignedImportDemandByHarbor;
        std::vector<FAssignedHarborImportPlan> AssignedImportPlanByHarbor;
        std::vector<FAssignedHarborExportPlan> AssignedExportPlanByHarbor;
    };

    struct FHarborExportCandidate
    {
        EResourceType Type = EResourceType::None;
        int ExportAmount = 0;
        int UnitPrice = 0;
    };

    struct FHarborImportCandidate
    {
        EResourceType Type = EResourceType::None;
        int Priority = 0;
        int NormalAmount = 0;
        int EmergencyAmount = 0;
        int NormalUnitPrice = 0;
        int EmergencyUnitPrice = 0;
    };

    struct FTaxEventEconomyEffects
    {
        double ExportMultiplier = 1.0;
        double ConsumptionTaxLeakage = 0.0;
        double IncomeTaxLeakage = 0.0;
        double PropertyTaxLeakage = 0.0;
        double ResidentialUpkeepMultiplier = 1.0;
        double GlobalUpkeepMultiplier = 1.0;
        double CollectionEfficiencyPenalty = 0.0;
    };

    bool IsOperationalBuilding(
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& Building)
    {
        return Building && Building->IsOperational();
    }

    void DistributeAmountByWeights(
        const std::vector<int>& Weights,
        int TotalAmount,
        std::vector<int>& OutShares)
    {
        OutShares.assign(Weights.size(), 0);

        if (Weights.empty() || TotalAmount <= 0)
            return;

        long long TotalWeight = 0;

        for (size_t Index = 0; Index < Weights.size(); ++Index)
            TotalWeight += (std::max)(0, Weights[Index]);

        if (TotalWeight <= 0)
        {
            const int BaseShare =
                TotalAmount / static_cast<int>(Weights.size());
            const int Remainder =
                TotalAmount % static_cast<int>(Weights.size());

            for (size_t Index = 0; Index < Weights.size(); ++Index)
            {
                OutShares[Index] =
                    BaseShare + (Index < static_cast<size_t>(Remainder) ? 1 : 0);
            }
            return;
        }

        struct FShareRemainder
        {
            size_t Index = 0;
            long long NumeratorRemainder = 0;
        };

        std::vector<FShareRemainder> Remainders;
        Remainders.reserve(Weights.size());
        int AssignedTotal = 0;

        for (size_t Index = 0; Index < Weights.size(); ++Index)
        {
            const long long WeightedAmount =
                static_cast<long long>((std::max)(0, Weights[Index])) *
                static_cast<long long>(TotalAmount);
            const int Share =
                static_cast<int>(WeightedAmount / TotalWeight);

            OutShares[Index] = Share;
            AssignedTotal += Share;
            Remainders.push_back(
                { Index, WeightedAmount % TotalWeight });
        }

        std::sort(
            Remainders.begin(),
            Remainders.end(),
            [](const FShareRemainder& A,
                const FShareRemainder& B)
            {
                if (A.NumeratorRemainder != B.NumeratorRemainder)
                    return A.NumeratorRemainder > B.NumeratorRemainder;

                return A.Index < B.Index;
            });

        const int RemainingAmount = (std::max)(0, TotalAmount - AssignedTotal);

        for (int Index = 0;
            Index < RemainingAmount &&
                Index < static_cast<int>(Remainders.size());
            ++Index)
        {
            ++OutShares[Remainders[static_cast<size_t>(Index)].Index];
        }
    }

    double ResolveConsumptionTaxableSpendMultiplier(
        ECitizenWealthLevel WealthLevel)
    {
        switch (WealthLevel)
        {
        case ECitizenWealthLevel::Broke:
            return 0.50;
        case ECitizenWealthLevel::Poor:
            return 0.75;
        case ECitizenWealthLevel::WellOff:
            return 1.00;
        case ECitizenWealthLevel::Rich:
            return 1.35;
        case ECitizenWealthLevel::FilthyRich:
            return 1.75;
        default:
            return 1.00;
        }
    }

    double ResolveIncomeTaxableBaseMultiplier(
        ECitizenWealthLevel WealthLevel)
    {
        switch (WealthLevel)
        {
        case ECitizenWealthLevel::Broke:
            return 0.35;
        case ECitizenWealthLevel::Poor:
            return 0.65;
        case ECitizenWealthLevel::WellOff:
            return 1.00;
        case ECitizenWealthLevel::Rich:
            return 1.45;
        case ECitizenWealthLevel::FilthyRich:
            return 2.05;
        default:
            return 1.00;
        }
    }

    double ResolvePropertyTaxableBaseMultiplier(
        ECitizenWealthLevel WealthLevel)
    {
        switch (WealthLevel)
        {
        case ECitizenWealthLevel::Broke:
            return 0.20;
        case ECitizenWealthLevel::Poor:
            return 0.55;
        case ECitizenWealthLevel::WellOff:
            return 1.00;
        case ECitizenWealthLevel::Rich:
            return 1.55;
        case ECitizenWealthLevel::FilthyRich:
            return 2.40;
        default:
            return 1.00;
        }
    }

    int ResolveEmergencyImportUnitPrice(EResourceType ResourceType);

    bool TryGetCoverageDistanceSq(
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& From,
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& To,
        float& OutDistSq)
    {
        OutDistSq = FLT_MAX;

        if (!From || !To)
            return false;

        int FromGridX = 0;
        int FromGridY = 0;
        int ToGridX = 0;
        int ToGridY = 0;

        if (!From->GetPlacedCenterGridCoords(FromGridX, FromGridY) ||
            !To->GetPlacedCenterGridCoords(ToGridX, ToGridY))
        {
            return false;
        }

        const float dx = static_cast<float>(FromGridX - ToGridX);
        const float dy = static_cast<float>(FromGridY - ToGridY);
        OutDistSq = dx * dx + dy * dy;
        return true;
    }

    bool IsWithinTeamsterCoverage(
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& Office,
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& Building)
    {
        float DistSq = FLT_MAX;

        if (!TryGetCoverageDistanceSq(Office, Building, DistSq))
            return false;

        const float CoverageRadius =
            GameConstants::Orb::TeamsterCoverageRadiusTiles;
        if (CoverageRadius <= 0.f)
            return true;

        return DistSq <= CoverageRadius * CoverageRadius;
    }

    bool IsWithinHarborDemandCoverage(
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& Harbor,
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& Building)
    {
        float DistSq = FLT_MAX;

        if (!TryGetCoverageDistanceSq(Harbor, Building, DistSq))
            return false;

        const float CoverageRadius =
            GameConstants::Orb::TeamsterCoverageRadiusTiles;
        if (CoverageRadius <= 0.f)
            return true;

        return DistSq <= CoverageRadius * CoverageRadius;
    }

    bool TryResolveSharedTeamsterRouteScore(
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& Harbor,
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& Consumer,
        const std::vector<
            std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>>& Offices,
        float& OutRouteScore)
    {
        OutRouteScore = FLT_MAX;

        if (!Harbor || !Consumer)
            return false;

        if (!IsWithinHarborDemandCoverage(Harbor, Consumer))
            return false;

        for (size_t OfficeIndex = 0; OfficeIndex < Offices.size(); ++OfficeIndex)
        {
            const auto& Office = Offices[OfficeIndex];

            if (!IsOperationalBuilding(Office) ||
                !IsWithinTeamsterCoverage(Office, Harbor) ||
                !IsWithinTeamsterCoverage(Office, Consumer))
            {
                continue;
            }

            float HarborDistSq = FLT_MAX;
            float ConsumerDistSq = FLT_MAX;

            if (!TryGetCoverageDistanceSq(Office, Harbor, HarborDistSq) ||
                !TryGetCoverageDistanceSq(Office, Consumer, ConsumerDistSq))
            {
                continue;
            }

            OutRouteScore = (std::min)(
                OutRouteScore,
                HarborDistSq + ConsumerDistSq);
        }

        return OutRouteScore < FLT_MAX;
    }

    const FAssignedHarborImportDemand* FindAssignedHarborImportDemand(
        const FHarborTradeDemandSnapshot& Snapshot,
        const EconomyWorldAccess::IEconomyBuildingAccess* Harbor)
    {
        if (!Harbor)
            return nullptr;

        for (size_t Index = 0;
            Index < Snapshot.AssignedImportDemandByHarbor.size();
            ++Index)
        {
            if (Snapshot.AssignedImportDemandByHarbor[Index].Harbor == Harbor)
                return &Snapshot.AssignedImportDemandByHarbor[Index];
        }

        return nullptr;
    }

    const FAssignedHarborImportPlan* FindAssignedHarborImportPlan(
        const FHarborTradeDemandSnapshot& Snapshot,
        const EconomyWorldAccess::IEconomyBuildingAccess* Harbor)
    {
        if (!Harbor)
            return nullptr;

        for (size_t Index = 0;
            Index < Snapshot.AssignedImportPlanByHarbor.size();
            ++Index)
        {
            if (Snapshot.AssignedImportPlanByHarbor[Index].Harbor == Harbor)
                return &Snapshot.AssignedImportPlanByHarbor[Index];
        }

        return nullptr;
    }

    const FAssignedHarborExportPlan* FindAssignedHarborExportPlan(
        const FHarborTradeDemandSnapshot& Snapshot,
        const EconomyWorldAccess::IEconomyBuildingAccess* Harbor)
    {
        if (!Harbor)
            return nullptr;

        for (size_t Index = 0;
            Index < Snapshot.AssignedExportPlanByHarbor.size();
            ++Index)
        {
            if (Snapshot.AssignedExportPlanByHarbor[Index].Harbor == Harbor)
                return &Snapshot.AssignedExportPlanByHarbor[Index];
        }

        return nullptr;
    }

    FHarborTradeDemandSnapshot BuildHarborTradeDemandSnapshot(
        const std::vector<
            std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>>&
                BuildingList)
    {
        FHarborTradeDemandSnapshot Snapshot;
        std::vector<
            std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>>
                Harbors;
        std::vector<
            std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>>
                Offices;

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            const auto& Building = BuildingList[i];

            if (!IsOperationalBuilding(Building))
                continue;

            if (Building->IsHarbor())
                Harbors.push_back(Building);

            if (Building->IsTransportOffice())
                Offices.push_back(Building);

            for (int ResourceIndex = 1;
                ResourceIndex < static_cast<int>(EResourceType::Count);
                ++ResourceIndex)
            {
                const EResourceType ResourceType =
                    static_cast<EResourceType>(ResourceIndex);

                if (!IsConcreteEconomicResourceType(ResourceType))
                    continue;

                Snapshot.RemainingAvailableByType[
                    static_cast<size_t>(ResourceType)] +=
                        Building->GetAvailableResourceStock(ResourceType);
            }
        }

        Snapshot.HarborCount = static_cast<int>(Harbors.size());
        Snapshot.AssignedImportDemandByHarbor.resize(Harbors.size());

        std::vector<std::array<int, GResourceTypeCount>>
            RemainingSupportByHarbor(Harbors.size());

        for (size_t HarborIndex = 0; HarborIndex < Harbors.size(); ++HarborIndex)
        {
            Snapshot.AssignedImportDemandByHarbor[HarborIndex].Harbor =
                Harbors[HarborIndex].get();

            for (int ResourceIndex = 1;
                ResourceIndex < static_cast<int>(EResourceType::Count);
                ++ResourceIndex)
            {
                const EResourceType ResourceType =
                    static_cast<EResourceType>(ResourceIndex);

                if (!IsConcreteEconomicResourceType(ResourceType))
                    continue;

                const int BufferedStock =
                    Harbors[HarborIndex]->GetResourceStock(ResourceType) +
                    Harbors[HarborIndex]->GetReservedIncomingResourceAmount(
                        ResourceType);
                const int FreeCapacity =
                    Harbors[HarborIndex]->GetAvailableIncomingCapacity(
                        ResourceType);
                RemainingSupportByHarbor[HarborIndex][
                    static_cast<size_t>(ResourceType)] = (std::max)(
                        0,
                        BufferedStock + FreeCapacity);
            }
        }

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            const auto& Building = BuildingList[i];

            if (!IsOperationalBuilding(Building))
                continue;

            if (Building->IsRoad() ||
                Building->IsBusStop() ||
                Building->IsHarbor() ||
                Building->IsTransportOffice() ||
                Building->IsWarehouse())
            {
                continue;
            }

            struct FReachableHarborRoute
            {
                size_t HarborIndex = 0;
                float RouteScore = FLT_MAX;
            };

            auto BuildReachableHarbors =
                [&](EResourceType ResourceType)
                -> std::vector<FReachableHarborRoute>
            {
                std::vector<FReachableHarborRoute> ReachableHarbors;

                if (ResourceType == EResourceType::None ||
                    !IsConcreteEconomicResourceType(ResourceType) ||
                    !IsExportableResourceType(ResourceType) ||
                    ResourceType == Building->GetProducedResourceType() ||
                    Harbors.empty() ||
                    Offices.empty())
                {
                    return ReachableHarbors;
                }

                const size_t ResourceIndex = static_cast<size_t>(ResourceType);

                if (ResourceIndex >= Snapshot.ConsumerShortageByType.size())
                    return ReachableHarbors;

                for (size_t HarborIndex = 0;
                    HarborIndex < Harbors.size();
                    ++HarborIndex)
                {
                    if (RemainingSupportByHarbor[HarborIndex][ResourceIndex] <= 0)
                        continue;

                    float RouteScore = FLT_MAX;

                    if (!TryResolveSharedTeamsterRouteScore(
                            Harbors[HarborIndex],
                            Building,
                            Offices,
                            RouteScore))
                    {
                        continue;
                    }

                    ReachableHarbors.push_back(
                        { HarborIndex, RouteScore });
                }

                std::sort(
                    ReachableHarbors.begin(),
                    ReachableHarbors.end(),
                    [](const FReachableHarborRoute& A,
                        const FReachableHarborRoute& B)
                    {
                        if (A.RouteScore != B.RouteScore)
                            return A.RouteScore < B.RouteScore;

                        return A.HarborIndex < B.HarborIndex;
                    });

                return ReachableHarbors;
            };

            auto AssignShortageToHarbors =
                [&](EResourceType ResourceType,
                    int Shortage,
                    const std::vector<FReachableHarborRoute>& ReachableHarbors)
            {
                if (Shortage <= 0)
                    return;

                const size_t ResourceIndex = static_cast<size_t>(ResourceType);

                if (ResourceIndex >= Snapshot.ConsumerShortageByType.size())
                    return;

                int RemainingShortage = Shortage;

                for (size_t HarborRouteIndex = 0;
                    HarborRouteIndex < ReachableHarbors.size() &&
                        RemainingShortage > 0;
                    ++HarborRouteIndex)
                {
                    const size_t HarborIndex =
                        ReachableHarbors[HarborRouteIndex].HarborIndex;
                    int& RemainingSupport =
                        RemainingSupportByHarbor[HarborIndex][ResourceIndex];
                    const int AssignedAmount = (std::min)(
                        RemainingShortage,
                        RemainingSupport);

                    if (AssignedAmount <= 0)
                        continue;

                    Snapshot.AssignedImportDemandByHarbor[HarborIndex]
                        .AssignedShortageByType[ResourceIndex] +=
                            AssignedAmount;
                    RemainingSupport -= AssignedAmount;
                    RemainingShortage -= AssignedAmount;
                }
            };

            auto AccumulateExactShortage = [&](EResourceType ResourceType)
            {
                if (ResourceType == EResourceType::None ||
                    !IsConcreteEconomicResourceType(ResourceType) ||
                    !IsExportableResourceType(ResourceType) ||
                    ResourceType == Building->GetProducedResourceType())
                {
                    return;
                }

                const size_t ResourceIndex = static_cast<size_t>(ResourceType);

                if (ResourceIndex >= Snapshot.ConsumerShortageByType.size())
                    return;

                const bool VisitConsumptionDemand =
                    ResourceType == Building->GetVisitConsumptionResourceType();
                const int CurrentStock =
                    VisitConsumptionDemand ?
                        Building->GetVisitConsumptionCompatibleResourceStock(
                            ResourceType) +
                            Building
                                ->GetVisitConsumptionCompatibleReservedIncomingResourceAmount(
                                    ResourceType) :
                        Building->GetResourceStock(ResourceType) +
                            Building->GetReservedIncomingResourceAmount(
                                ResourceType);
                const int Shortage = (std::max)(
                    0,
                    GameConstants::Economy::HarborImportTargetStockPerConsumer -
                        CurrentStock);
                const std::vector<FReachableHarborRoute> ReachableHarbors =
                    BuildReachableHarbors(ResourceType);

                if (ReachableHarbors.empty())
                    return;

                Snapshot.ConsumerReserveWeightMilliByType[ResourceIndex] +=
                    1000;
                Snapshot.ConsumerShortageByType[ResourceIndex] += Shortage;
                AssignShortageToHarbors(
                    ResourceType,
                    Shortage,
                    ReachableHarbors);
            };

            struct FAcceptedDemandTypeState
            {
                EResourceType Type = EResourceType::None;
                size_t ResourceIndex = 0;
                int BufferedStock = 0;
                int Weight = 0;
                std::vector<FReachableHarborRoute> ReachableHarbors;
            };

            auto AccumulateAcceptedExactShortage =
                [&](const std::vector<EResourceType>& AcceptedTypes)
            {
                std::vector<FAcceptedDemandTypeState> AcceptedStates;
                AcceptedStates.reserve(AcceptedTypes.size());
                int TotalBufferedStock = 0;

                for (size_t AcceptedIndex = 0;
                    AcceptedIndex < AcceptedTypes.size();
                    ++AcceptedIndex)
                {
                    const EResourceType AcceptedType =
                        AcceptedTypes[AcceptedIndex];

                    if (!IsConcreteEconomicResourceType(AcceptedType) ||
                        !IsExportableResourceType(AcceptedType) ||
                        AcceptedType == Building->GetProducedResourceType())
                    {
                        continue;
                    }

                    bool Duplicate = false;

                    for (size_t ExistingIndex = 0;
                        ExistingIndex < AcceptedStates.size();
                        ++ExistingIndex)
                    {
                        if (AcceptedStates[ExistingIndex].Type == AcceptedType)
                        {
                            Duplicate = true;
                            break;
                        }
                    }

                    if (Duplicate)
                        continue;

                    const int BufferedStock =
                        Building->GetResourceStock(AcceptedType) +
                        Building->GetReservedIncomingResourceAmount(
                            AcceptedType);
                    TotalBufferedStock += BufferedStock;

                    FAcceptedDemandTypeState State;
                    State.Type = AcceptedType;
                    State.ResourceIndex = static_cast<size_t>(AcceptedType);
                    State.BufferedStock = BufferedStock;
                    State.ReachableHarbors =
                        BuildReachableHarbors(AcceptedType);
                    AcceptedStates.push_back(std::move(State));
                }

                std::vector<int> WeightVector;
                std::vector<size_t> ActiveStateIndices;
                WeightVector.reserve(AcceptedStates.size());
                ActiveStateIndices.reserve(AcceptedStates.size());
                bool HasPositiveWeight = false;

                for (size_t StateIndex = 0;
                    StateIndex < AcceptedStates.size();
                    ++StateIndex)
                {
                    FAcceptedDemandTypeState& State =
                        AcceptedStates[StateIndex];

                    if (State.ResourceIndex >=
                            Snapshot.RemainingAvailableByType.size() ||
                        State.ReachableHarbors.empty())
                    {
                        continue;
                    }

                    State.Weight =
                        State.BufferedStock +
                        Snapshot.RemainingAvailableByType[State.ResourceIndex];
                    HasPositiveWeight |= State.Weight > 0;
                    ActiveStateIndices.push_back(StateIndex);
                }

                if (!HasPositiveWeight)
                {
                    int BestImportPrice = (std::numeric_limits<int>::max)();

                    for (size_t ActiveIndex = 0;
                        ActiveIndex < ActiveStateIndices.size();
                        ++ActiveIndex)
                    {
                        FAcceptedDemandTypeState& State =
                            AcceptedStates[
                                ActiveStateIndices[ActiveIndex]];
                        BestImportPrice = (std::min)(
                            BestImportPrice,
                            ResourceTradePricing::GetImportPricePerStockUnit(
                                State.Type));
                    }

                    for (size_t ActiveIndex = 0;
                        ActiveIndex < ActiveStateIndices.size();
                        ++ActiveIndex)
                    {
                        FAcceptedDemandTypeState& State =
                            AcceptedStates[
                                ActiveStateIndices[ActiveIndex]];
                        State.Weight =
                            ResourceTradePricing::GetImportPricePerStockUnit(
                                State.Type) == BestImportPrice ?
                                1 :
                                0;
                    }
                }

                for (size_t ActiveIndex = 0;
                    ActiveIndex < ActiveStateIndices.size();
                    ++ActiveIndex)
                {
                    WeightVector.push_back(
                        (std::max)(
                            0,
                            AcceptedStates[
                                ActiveStateIndices[ActiveIndex]].Weight));
                }

                if (WeightVector.empty())
                    return;

                std::vector<int> BufferSharesMilli;
                DistributeAmountByWeights(
                    WeightVector,
                    1000,
                    BufferSharesMilli);

                const int TotalShortage = (std::max)(
                    0,
                    GameConstants::Economy::HarborImportTargetStockPerConsumer -
                        TotalBufferedStock);
                std::vector<int> ShortageShares;
                DistributeAmountByWeights(
                    WeightVector,
                    TotalShortage,
                    ShortageShares);

                for (size_t ActiveIndex = 0;
                    ActiveIndex < ActiveStateIndices.size();
                    ++ActiveIndex)
                {
                    FAcceptedDemandTypeState& State =
                        AcceptedStates[ActiveStateIndices[ActiveIndex]];
                    const size_t ResourceIndex = State.ResourceIndex;

                    if (ResourceIndex >=
                        Snapshot.ConsumerShortageByType.size())
                    {
                        continue;
                    }

                    Snapshot.ConsumerReserveWeightMilliByType[ResourceIndex] +=
                        BufferSharesMilli[ActiveIndex];
                    Snapshot.ConsumerShortageByType[ResourceIndex] +=
                        ShortageShares[ActiveIndex];
                    AssignShortageToHarbors(
                        State.Type,
                        ShortageShares[ActiveIndex],
                        State.ReachableHarbors);
                }
            };

            auto AccumulateCompatibleInputShortage = [&](
                EResourceType ResourceType)
            {
                if (ResourceType != EResourceType::FeedCrops)
                {
                    AccumulateExactShortage(ResourceType);
                    return;
                }

                std::vector<EResourceType> AcceptedTypes;

                ForEachFeedCompatibleResourceType(
                    ResourceType,
                    [&](EResourceType AcceptedType)
                    {
                        if (AcceptedType == EResourceType::None ||
                            AcceptedType == EResourceType::Count)
                        {
                            return;
                        }

                        if (std::find(
                                AcceptedTypes.begin(),
                                AcceptedTypes.end(),
                                AcceptedType) == AcceptedTypes.end())
                        {
                            AcceptedTypes.push_back(AcceptedType);
                        }
                    });

                AccumulateAcceptedExactShortage(AcceptedTypes);
            };

            const EResourceType VisitConsumptionType =
                Building->GetVisitConsumptionResourceType();
            const std::vector<EResourceType> VisitAcceptedTypes =
                Building->GetRuntimeVisitConsumptionAcceptedResourceTypes();

            if (!VisitAcceptedTypes.empty())
            {
                AccumulateAcceptedExactShortage(VisitAcceptedTypes);
            }
            else if (IsConcreteEconomicResourceType(VisitConsumptionType))
            {
                AccumulateExactShortage(VisitConsumptionType);
            }

            for (int SlotIndex = 0;
                 SlotIndex < Building->GetProductionInputCount();
                 ++SlotIndex)
            {
                const EResourceType InputType =
                    Building->GetProductionInputType(SlotIndex);

                if (InputType == Building->GetVisitConsumptionResourceType())
                    continue;

                AccumulateCompatibleInputShortage(InputType);
            }
        }

        return Snapshot;
    }

    FHarborImportCandidate ResolveHarborImportCandidate(
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& Harbor,
        EResourceType ResourceType,
        const FHarborTradeDemandSnapshot& DemandSnapshot,
        const TradePolicy::FImportTradePolicy& ImportPolicy)
    {
        FHarborImportCandidate Candidate;
        Candidate.Type = ResourceType;

        if (!IsOperationalBuilding(Harbor) ||
            !Harbor->IsHarbor() ||
            !IsConcreteEconomicResourceType(ResourceType) ||
            !IsExportableResourceType(ResourceType) ||
            !TradePolicy::IsResourceImportAllowed(
                ImportPolicy,
                ResourceType) ||
            DemandSnapshot.HarborCount <= 0)
        {
            return Candidate;
        }

        const size_t ResourceIndex = static_cast<size_t>(ResourceType);

        if (ResourceIndex >= DemandSnapshot.ConsumerShortageByType.size())
            return Candidate;

        const FAssignedHarborImportDemand* AssignedDemand =
            FindAssignedHarborImportDemand(
                DemandSnapshot,
                Harbor.get());

        if (!AssignedDemand)
            return Candidate;

        const int AssignedShortage =
            AssignedDemand->AssignedShortageByType[ResourceIndex];

        if (AssignedShortage <= 0)
            return Candidate;

        const int BufferedStock =
            Harbor->GetResourceStock(ResourceType) +
            Harbor->GetReservedIncomingResourceAmount(ResourceType);
        const int FreeCapacity =
            Harbor->GetAvailableIncomingCapacity(ResourceType);
        const int NeededAmount = (std::min)(
            FreeCapacity,
            (std::max)(0, AssignedShortage - BufferedStock));
        const int NormalCap =
            TradePolicy::GetImportMaxUnitsPerResource(ImportPolicy);

        if (NeededAmount <= 0 || NormalCap <= 0)
            return Candidate;

        Candidate.Priority = NeededAmount;
        Candidate.NormalAmount = (std::min)(NeededAmount, NormalCap);
        Candidate.NormalUnitPrice =
            ResourceTradePricing::GetImportPricePerStockUnit(ResourceType);
        Candidate.EmergencyUnitPrice =
            ResolveEmergencyImportUnitPrice(ResourceType);

        const int RemainingNeed =
            (std::max)(0, NeededAmount - Candidate.NormalAmount);
        const bool CriticalShortage =
            RemainingNeed >= NormalCap / 2 ||
            BufferedStock <=
                GameConstants::Economy::HarborImportTargetStockPerConsumer / 4;

        if (TradePolicy::AllowsEmergencyImports(ImportPolicy) &&
            CriticalShortage &&
            RemainingNeed > 0)
        {
            Candidate.EmergencyAmount = (std::min)(RemainingNeed, NormalCap);
        }

        return Candidate;
    }

    int ResolveDomesticExportReserveAmount(
        const FHarborTradeDemandSnapshot& DemandSnapshot,
        EResourceType ResourceType,
        const TradePolicy::FExportTradePolicy& ExportPolicy)
    {
        if (!IsConcreteEconomicResourceType(ResourceType))
            return 0;

        const size_t ResourceIndex = static_cast<size_t>(ResourceType);

        if (ResourceIndex >= DemandSnapshot.ConsumerShortageByType.size())
            return 0;

        const long long BufferWeightMilli =
            ResourceIndex <
                DemandSnapshot.ConsumerReserveWeightMilliByType.size() ?
                DemandSnapshot.ConsumerReserveWeightMilliByType[ResourceIndex] :
                0;
        const long long BufferUnits = static_cast<long long>(
            TradePolicy::GetDomesticReserveBufferUnits(ExportPolicy));
        const int DistributedBuffer = static_cast<int>(
            (std::max)(
                0ll,
                (BufferUnits * BufferWeightMilli + 999ll) / 1000ll));
        return DemandSnapshot.ConsumerShortageByType[ResourceIndex] +
            DistributedBuffer;
    }

    int ResolveEmergencyImportUnitPrice(EResourceType ResourceType)
    {
        const int NormalUnitPrice =
            ResourceTradePricing::GetImportPricePerStockUnit(ResourceType);
        return (std::max)(
            NormalUnitPrice + 1,
            static_cast<int>(std::ceil(
                static_cast<double>(NormalUnitPrice) *
                TradePolicy::GEmergencyImportCostMultiplier)));
    }

    void AssignHarborExportPlans(
        const std::vector<
            std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>>&
                ArrivedExportHubs,
        const TradePolicy::FExportTradePolicy& ExportPolicy,
        double ExportMultiplier,
        FHarborTradeDemandSnapshot& InOutDemandSnapshot)
    {
        InOutDemandSnapshot.AssignedExportPlanByHarbor.clear();

        if (ArrivedExportHubs.empty() || ExportMultiplier <= 0.0)
            return;

        const int ShipCapacity =
            TradePolicy::GetHarborExportShipCapacityUnits(ExportPolicy);

        if (ShipCapacity <= 0)
            return;

        struct FExportHubAllocationState
        {
            const EconomyWorldAccess::IEconomyBuildingAccess* Harbor = nullptr;
            std::array<int, GResourceTypeCount> AssignedExportByType = {};
            int RemainingShipCapacity = 0;
            int GridX = (std::numeric_limits<int>::max)();
            int GridY = (std::numeric_limits<int>::max)();
        };

        std::vector<FExportHubAllocationState> HubStates;
        HubStates.reserve(ArrivedExportHubs.size());

        for (size_t HubIndex = 0; HubIndex < ArrivedExportHubs.size(); ++HubIndex)
        {
            const auto& Harbor = ArrivedExportHubs[HubIndex];

            if (!IsOperationalBuilding(Harbor) ||
                !Harbor->CanExportStoredResources())
            {
                continue;
            }

            FExportHubAllocationState State;
            State.Harbor = Harbor.get();
            State.RemainingShipCapacity = ShipCapacity;
            Harbor->GetPlacedCenterGridCoords(State.GridX, State.GridY);
            HubStates.push_back(State);
        }

        if (HubStates.empty())
            return;

        struct FResourceAllocationPlan
        {
            EResourceType Type = EResourceType::None;
            int UnitPrice = 0;
            // Island-wide export allowance after domestic reserve protection.
            int GlobalHeadroom = 0;
            // What export hubs can actually ship right now from their current
            // local stock this planning tick.
            int TotalCandidateAmount = 0;
        };

        std::vector<FResourceAllocationPlan> ResourcePlans;

        for (int ResourceIndex = 1;
             ResourceIndex < static_cast<int>(EResourceType::Count);
             ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsConcreteEconomicResourceType(ResourceType))
                continue;

            if (!TradePolicy::IsResourceExportAllowed(
                    ExportPolicy,
                    ResourceType))
            {
                continue;
            }

            const size_t ResourceArrayIndex =
                static_cast<size_t>(ResourceType);
            if (ResourceArrayIndex >=
                InOutDemandSnapshot.RemainingAvailableByType.size())
            {
                continue;
            }

            const int GlobalHeadroom = (std::max)(
                0,
                InOutDemandSnapshot.RemainingAvailableByType[
                    ResourceArrayIndex] -
                    ResolveDomesticExportReserveAmount(
                        InOutDemandSnapshot,
                        ResourceType,
                        ExportPolicy));

            if (GlobalHeadroom <= 0)
                continue;

            int TotalCandidateAmount = 0;

            for (size_t HubIndex = 0; HubIndex < ArrivedExportHubs.size(); ++HubIndex)
            {
                const auto& Harbor = ArrivedExportHubs[HubIndex];

                if (!IsOperationalBuilding(Harbor) ||
                    !Harbor->CanExportStoredResources())
                {
                    continue;
                }

                const int EffectiveExportStock = static_cast<int>(std::floor(
                    static_cast<double>(Harbor->GetResourceStock(ResourceType)) *
                    ExportMultiplier));
                TotalCandidateAmount += (std::max)(0, EffectiveExportStock);
            }

            if (TotalCandidateAmount <= 0)
                continue;

            // Keep national headroom and current harbor stock separate:
            // headroom limits whether exporting is allowed overall, while
            // candidate stock limits what can be loaded immediately.
            FResourceAllocationPlan Plan;
            Plan.Type = ResourceType;
            Plan.UnitPrice =
                ResourceTradePricing::GetExportPricePerStockUnit(ResourceType);
            Plan.GlobalHeadroom = GlobalHeadroom;
            Plan.TotalCandidateAmount = TotalCandidateAmount;
            ResourcePlans.push_back(Plan);
        }

        std::sort(
            ResourcePlans.begin(),
            ResourcePlans.end(),
            [&](const FResourceAllocationPlan& A,
                const FResourceAllocationPlan& B)
            {
                if (ExportPolicy.PrioritizeHighValueCargo &&
                    A.UnitPrice != B.UnitPrice)
                {
                    return A.UnitPrice > B.UnitPrice;
                }

                if (A.TotalCandidateAmount != B.TotalCandidateAmount)
                    return A.TotalCandidateAmount > B.TotalCandidateAmount;

                if (!ExportPolicy.PrioritizeHighValueCargo &&
                    A.UnitPrice != B.UnitPrice)
                {
                    return A.UnitPrice > B.UnitPrice;
                }

                return static_cast<int>(A.Type) < static_cast<int>(B.Type);
            });

        struct FHubResourceShare
        {
            size_t HubIndex = 0;
            int CandidateAmount = 0;
            int AssignedAmount = 0;
            long long RemainderNumerator = 0;
            int GridX = (std::numeric_limits<int>::max)();
            int GridY = (std::numeric_limits<int>::max)();
        };

        for (size_t ResourcePlanIndex = 0;
            ResourcePlanIndex < ResourcePlans.size();
            ++ResourcePlanIndex)
        {
            const EResourceType ResourceType =
                ResourcePlans[ResourcePlanIndex].Type;
            const int GlobalHeadroom =
                ResourcePlans[ResourcePlanIndex].GlobalHeadroom;

            if (GlobalHeadroom <= 0)
                continue;

            std::vector<FHubResourceShare> Shares;
            int TotalCandidateAmount = 0;

            for (size_t HubIndex = 0; HubIndex < ArrivedExportHubs.size(); ++HubIndex)
            {
                if (HubIndex >= HubStates.size() ||
                    HubStates[HubIndex].Harbor == nullptr ||
                    HubStates[HubIndex].RemainingShipCapacity <= 0)
                {
                    continue;
                }

                const auto& Harbor = ArrivedExportHubs[HubIndex];

                if (!IsOperationalBuilding(Harbor) ||
                    Harbor.get() != HubStates[HubIndex].Harbor)
                {
                    continue;
                }

                const int EffectiveExportStock = static_cast<int>(std::floor(
                    static_cast<double>(Harbor->GetResourceStock(ResourceType)) *
                    ExportMultiplier));
                const int CandidateAmount = (std::min)(
                    HubStates[HubIndex].RemainingShipCapacity,
                    (std::max)(0, EffectiveExportStock));

                if (CandidateAmount <= 0)
                    continue;

                FHubResourceShare Share;
                Share.HubIndex = HubIndex;
                Share.CandidateAmount = CandidateAmount;
                Share.GridX = HubStates[HubIndex].GridX;
                Share.GridY = HubStates[HubIndex].GridY;
                Shares.push_back(Share);
                TotalCandidateAmount += CandidateAmount;
            }

            if (TotalCandidateAmount <= 0)
                continue;

            // Final export assignment cannot exceed either island-wide
            // headroom or the stock already staged at export hubs.
            const int AllocatableAmount = (std::min)(
                GlobalHeadroom,
                TotalCandidateAmount);
            int AssignedTotal = 0;

            for (size_t ShareIndex = 0; ShareIndex < Shares.size(); ++ShareIndex)
            {
                const long long WeightedAmount =
                    static_cast<long long>(AllocatableAmount) *
                    static_cast<long long>(Shares[ShareIndex].CandidateAmount);
                Shares[ShareIndex].AssignedAmount = static_cast<int>(
                    WeightedAmount / static_cast<long long>(TotalCandidateAmount));
                Shares[ShareIndex].RemainderNumerator =
                    WeightedAmount % static_cast<long long>(TotalCandidateAmount);
                AssignedTotal += Shares[ShareIndex].AssignedAmount;
            }

            const int RemainingAmount =
                (std::max)(0, AllocatableAmount - AssignedTotal);

            std::sort(
                Shares.begin(),
                Shares.end(),
                [](const FHubResourceShare& A, const FHubResourceShare& B)
                {
                    if (A.RemainderNumerator != B.RemainderNumerator)
                        return A.RemainderNumerator > B.RemainderNumerator;

                    if (A.CandidateAmount != B.CandidateAmount)
                        return A.CandidateAmount > B.CandidateAmount;

                    if (A.GridX != B.GridX)
                        return A.GridX < B.GridX;

                    if (A.GridY != B.GridY)
                        return A.GridY < B.GridY;

                    return A.HubIndex < B.HubIndex;
                });

            for (int RemainingIndex = 0;
                RemainingIndex < RemainingAmount &&
                    RemainingIndex < static_cast<int>(Shares.size());
                ++RemainingIndex)
            {
                if (Shares[RemainingIndex].AssignedAmount <
                    Shares[RemainingIndex].CandidateAmount)
                {
                    ++Shares[RemainingIndex].AssignedAmount;
                }
            }

            for (size_t ShareIndex = 0; ShareIndex < Shares.size(); ++ShareIndex)
            {
                const size_t HubIndex = Shares[ShareIndex].HubIndex;
                const int AssignedAmount = Shares[ShareIndex].AssignedAmount;

                if (AssignedAmount <= 0)
                    continue;

                HubStates[HubIndex].AssignedExportByType[
                    static_cast<size_t>(ResourceType)] += AssignedAmount;
                HubStates[HubIndex].RemainingShipCapacity = (std::max)(
                    0,
                    HubStates[HubIndex].RemainingShipCapacity - AssignedAmount);
            }
        }

        InOutDemandSnapshot.AssignedExportPlanByHarbor.reserve(HubStates.size());

        for (size_t HubIndex = 0; HubIndex < HubStates.size(); ++HubIndex)
        {
            FAssignedHarborExportPlan Plan;
            Plan.Harbor = HubStates[HubIndex].Harbor;
            Plan.AssignedExportByType = HubStates[HubIndex].AssignedExportByType;
            InOutDemandSnapshot.AssignedExportPlanByHarbor.push_back(Plan);
        }
    }

    void AssignHarborImportPlans(
        const std::vector<
            std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>>&
                ArrivedHarbors,
        const TradePolicy::FImportTradePolicy& ImportPolicy,
        long long ImportBudgetCap,
        const FHarborTradeDemandSnapshot& DemandSnapshot,
        FHarborTradeDemandSnapshot& InOutDemandSnapshot)
    {
        InOutDemandSnapshot.AssignedImportPlanByHarbor.clear();

        if (ArrivedHarbors.empty())
            return;

        struct FHarborImportPlanState
        {
            const EconomyWorldAccess::IEconomyBuildingAccess* Harbor = nullptr;
            int HarborPriority = 0;
            int GridX = (std::numeric_limits<int>::max)();
            int GridY = (std::numeric_limits<int>::max)();
            std::array<int, GResourceTypeCount> AssignedNormalImportByType = {};
            std::array<int, GResourceTypeCount> AssignedEmergencyImportByType = {};
            std::vector<FHarborImportCandidate> Candidates;
        };

        std::vector<FHarborImportPlanState> HarborStates;
        HarborStates.reserve(ArrivedHarbors.size());

        for (size_t HarborIndex = 0; HarborIndex < ArrivedHarbors.size(); ++HarborIndex)
        {
            const auto& Harbor = ArrivedHarbors[HarborIndex];

            if (!IsOperationalBuilding(Harbor) || !Harbor->IsHarbor())
                continue;

            FHarborImportPlanState State;
            State.Harbor = Harbor.get();
            Harbor->GetPlacedCenterGridCoords(State.GridX, State.GridY);

            for (int ResourceIndex = 1;
                ResourceIndex < static_cast<int>(EResourceType::Count);
                ++ResourceIndex)
            {
                const EResourceType ResourceType =
                    static_cast<EResourceType>(ResourceIndex);
                const FHarborImportCandidate Candidate =
                    ResolveHarborImportCandidate(
                        Harbor,
                        ResourceType,
                        DemandSnapshot,
                        ImportPolicy);

                if (Candidate.NormalAmount <= 0 &&
                    Candidate.EmergencyAmount <= 0)
                {
                    continue;
                }

                State.HarborPriority += Candidate.Priority;
                State.Candidates.push_back(Candidate);
            }

            HarborStates.push_back(std::move(State));
        }

        if (HarborStates.empty())
            return;

        InOutDemandSnapshot.AssignedImportPlanByHarbor.reserve(HarborStates.size());

        if (ImportBudgetCap <= 0)
        {
            for (size_t HarborIndex = 0; HarborIndex < HarborStates.size(); ++HarborIndex)
            {
                for (size_t CandidateIndex = 0;
                    CandidateIndex < HarborStates[HarborIndex].Candidates.size();
                    ++CandidateIndex)
                {
                    const FHarborImportCandidate& Candidate =
                        HarborStates[HarborIndex].Candidates[CandidateIndex];
                    const size_t ResourceArrayIndex =
                        static_cast<size_t>(Candidate.Type);

                    HarborStates[HarborIndex].AssignedNormalImportByType[
                        ResourceArrayIndex] = Candidate.NormalAmount;
                    HarborStates[HarborIndex].AssignedEmergencyImportByType[
                        ResourceArrayIndex] = Candidate.EmergencyAmount;
                }
            }
        }
        else
        {
            struct FImportBudgetCandidate
            {
                size_t HarborIndex = 0;
                EResourceType Type = EResourceType::None;
                int RequestedAmount = 0;
                int UnitPrice = 0;
                int Priority = 0;
                int HarborPriority = 0;
                int GridX = (std::numeric_limits<int>::max)();
                int GridY = (std::numeric_limits<int>::max)();
            };

            auto SortCandidates =
                [](std::vector<FImportBudgetCandidate>& Candidates)
                {
                    std::sort(
                        Candidates.begin(),
                        Candidates.end(),
                        [](const FImportBudgetCandidate& A,
                            const FImportBudgetCandidate& B)
                        {
                            if (A.Priority != B.Priority)
                                return A.Priority > B.Priority;

                            if (A.HarborPriority != B.HarborPriority)
                                return A.HarborPriority > B.HarborPriority;

                            if (A.UnitPrice != B.UnitPrice)
                                return A.UnitPrice < B.UnitPrice;

                            if (A.GridX != B.GridX)
                                return A.GridX < B.GridX;

                            if (A.GridY != B.GridY)
                                return A.GridY < B.GridY;

                            return static_cast<int>(A.Type) <
                                static_cast<int>(B.Type);
                        });
                };

            auto BuildPhaseCandidates =
                [&](bool EmergencyPhase)
                {
                    std::vector<FImportBudgetCandidate> PhaseCandidates;

                    for (size_t HarborIndex = 0;
                        HarborIndex < HarborStates.size();
                        ++HarborIndex)
                    {
                        for (size_t CandidateIndex = 0;
                            CandidateIndex <
                                HarborStates[HarborIndex].Candidates.size();
                            ++CandidateIndex)
                        {
                            const FHarborImportCandidate& Candidate =
                                HarborStates[HarborIndex].Candidates[
                                    CandidateIndex];
                            const int RequestedAmount = EmergencyPhase ?
                                Candidate.EmergencyAmount :
                                Candidate.NormalAmount;
                            const int UnitPrice = EmergencyPhase ?
                                Candidate.EmergencyUnitPrice :
                                Candidate.NormalUnitPrice;

                            if (RequestedAmount <= 0 || UnitPrice <= 0)
                                continue;

                            FImportBudgetCandidate BudgetCandidate;
                            BudgetCandidate.HarborIndex = HarborIndex;
                            BudgetCandidate.Type = Candidate.Type;
                            BudgetCandidate.RequestedAmount = RequestedAmount;
                            BudgetCandidate.UnitPrice = UnitPrice;
                            BudgetCandidate.Priority = Candidate.Priority;
                            BudgetCandidate.HarborPriority =
                                HarborStates[HarborIndex].HarborPriority;
                            BudgetCandidate.GridX =
                                HarborStates[HarborIndex].GridX;
                            BudgetCandidate.GridY =
                                HarborStates[HarborIndex].GridY;
                            PhaseCandidates.push_back(BudgetCandidate);
                        }
                    }

                    SortCandidates(PhaseCandidates);
                    return PhaseCandidates;
                };

            long long RemainingBudget = ImportBudgetCap;

            auto AllocatePhase =
                [&](bool EmergencyPhase)
                {
                    std::vector<FImportBudgetCandidate> PhaseCandidates =
                        BuildPhaseCandidates(EmergencyPhase);

                    for (size_t CandidateIndex = 0;
                        CandidateIndex < PhaseCandidates.size() &&
                            RemainingBudget > 0;
                        ++CandidateIndex)
                    {
                        const FImportBudgetCandidate& Candidate =
                            PhaseCandidates[CandidateIndex];
                        const long long MaxAffordableAmount =
                            RemainingBudget /
                            static_cast<long long>(Candidate.UnitPrice);
                        const int AssignedAmount = (std::min)(
                            Candidate.RequestedAmount,
                            static_cast<int>((std::max)(
                                0ll,
                                MaxAffordableAmount)));

                        if (AssignedAmount <= 0)
                            continue;

                        const size_t ResourceArrayIndex =
                            static_cast<size_t>(Candidate.Type);

                        if (EmergencyPhase)
                        {
                            HarborStates[Candidate.HarborIndex]
                                .AssignedEmergencyImportByType[
                                    ResourceArrayIndex] += AssignedAmount;
                        }
                        else
                        {
                            HarborStates[Candidate.HarborIndex]
                                .AssignedNormalImportByType[
                                    ResourceArrayIndex] += AssignedAmount;
                        }

                        RemainingBudget -=
                            static_cast<long long>(AssignedAmount) *
                            static_cast<long long>(Candidate.UnitPrice);
                    }
                };

            AllocatePhase(false);
            AllocatePhase(true);
        }

        for (size_t HarborIndex = 0; HarborIndex < HarborStates.size(); ++HarborIndex)
        {
            FAssignedHarborImportPlan Plan;
            Plan.Harbor = HarborStates[HarborIndex].Harbor;
            Plan.AssignedNormalImportByType =
                HarborStates[HarborIndex].AssignedNormalImportByType;
            Plan.AssignedEmergencyImportByType =
                HarborStates[HarborIndex].AssignedEmergencyImportByType;
            InOutDemandSnapshot.AssignedImportPlanByHarbor.push_back(Plan);
        }
    }

    int ResolveDeliveredCargoAmount(
        int RequestedAmount,
        int LossPercent)
    {
        if (RequestedAmount <= 0)
            return 0;

        if (LossPercent <= 0 || RequestedAmount <= 1)
            return (std::max)(1, RequestedAmount);

        const int LossAmount = (std::min)(
            RequestedAmount - 1,
            static_cast<int>(
                static_cast<long long>(RequestedAmount) * LossPercent / 100ll));
        return (std::max)(1, RequestedAmount - LossAmount);
    }

    long long SettleHarborExportIncome(
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& Harbor,
        const TradePolicy::FExportTradePolicy& ExportPolicy,
        double ExportMultiplier,
        FHarborTradeDemandSnapshot& DemandSnapshot)
    {
        if (!IsOperationalBuilding(Harbor) ||
            !Harbor->CanExportStoredResources())
        {
            return 0;
        }

        const int ShipCapacity =
            TradePolicy::GetHarborExportShipCapacityUnits(ExportPolicy);

        if (ShipCapacity <= 0 || ExportMultiplier <= 0.0)
            return 0;

        const FAssignedHarborExportPlan* AssignedPlan =
            FindAssignedHarborExportPlan(
                DemandSnapshot,
                Harbor.get());

        if (!AssignedPlan)
            return 0;

        long long ExportIncome = 0;
        int RemainingShipCapacity = ShipCapacity;
        const int CargoLossPercent = Harbor->GetTeamsterCargoLossPercent();
        std::vector<FHarborExportCandidate> Candidates;

        for (int ResourceIndex = 1;
             ResourceIndex < static_cast<int>(EResourceType::Count);
             ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsConcreteEconomicResourceType(ResourceType))
                continue;

            if (!TradePolicy::IsResourceExportAllowed(
                    ExportPolicy,
                    ResourceType))
                continue;

            const size_t ResourceArrayIndex =
                static_cast<size_t>(ResourceType);
            if (ResourceArrayIndex >= AssignedPlan->AssignedExportByType.size())
                continue;

            const int ResourceStock = Harbor->GetResourceStock(ResourceType);
            const int EffectiveExportStock = static_cast<int>(std::floor(
                static_cast<double>(ResourceStock) * ExportMultiplier));
            const int ExportAmount = (std::min)(
                EffectiveExportStock,
                AssignedPlan->AssignedExportByType[ResourceArrayIndex]);

            if (ExportAmount <= 0)
                continue;

            FHarborExportCandidate Candidate;
            Candidate.Type = ResourceType;
            Candidate.ExportAmount = ExportAmount;
            Candidate.UnitPrice =
                ResourceTradePricing::GetExportPricePerStockUnit(ResourceType);
            Candidates.push_back(Candidate);
        }

        std::sort(
            Candidates.begin(),
            Candidates.end(),
            [&](const FHarborExportCandidate& A,
                const FHarborExportCandidate& B)
            {
                if (ExportPolicy.PrioritizeHighValueCargo &&
                    A.UnitPrice != B.UnitPrice)
                {
                    return A.UnitPrice > B.UnitPrice;
                }

                if (A.ExportAmount != B.ExportAmount)
                    return A.ExportAmount > B.ExportAmount;

                return static_cast<int>(A.Type) < static_cast<int>(B.Type);
            });

        for (const FHarborExportCandidate& Candidate : Candidates)
        {
            if (RemainingShipCapacity <= 0)
                break;

            const int AmountToExport = (std::min)(
                Candidate.ExportAmount,
                RemainingShipCapacity);

            if (AmountToExport <= 0)
                continue;

            if (!Harbor->TryConsumeResource(
                    Candidate.Type,
                    AmountToExport))
            {
                continue;
            }

            const int DeliveredAmount = ResolveDeliveredCargoAmount(
                AmountToExport,
                CargoLossPercent);
            ExportIncome += ResourceTradePricing::ComputeExportValue(
                Candidate.Type,
                DeliveredAmount);
            RemainingShipCapacity -= AmountToExport;

            const size_t ResourceArrayIndex =
                static_cast<size_t>(Candidate.Type);

            if (ResourceArrayIndex <
                DemandSnapshot.RemainingAvailableByType.size())
            {
                DemandSnapshot.RemainingAvailableByType[
                    ResourceArrayIndex] = (std::max)(
                        0,
                        DemandSnapshot.RemainingAvailableByType[
                            ResourceArrayIndex] - AmountToExport);
            }
        }

        return ExportIncome;
    }

    long long SettleHarborImportExpense(
        const std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>& Harbor,
        const FHarborTradeDemandSnapshot& DemandSnapshot)
    {
        if (!IsOperationalBuilding(Harbor) || !Harbor->IsHarbor())
            return 0;

        const FAssignedHarborImportPlan* AssignedPlan =
            FindAssignedHarborImportPlan(
                DemandSnapshot,
                Harbor.get());

        if (!AssignedPlan)
            return 0;

        long long ImportExpense = 0;

        for (int ResourceIndex = 1;
             ResourceIndex < static_cast<int>(EResourceType::Count);
             ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsConcreteEconomicResourceType(ResourceType))
                continue;

            const size_t ResourceArrayIndex =
                static_cast<size_t>(ResourceType);

            if (ResourceArrayIndex >=
                AssignedPlan->AssignedNormalImportByType.size())
            {
                continue;
            }

            const int NormalAmount =
                AssignedPlan->AssignedNormalImportByType[ResourceArrayIndex];
            const int EmergencyAmount =
                AssignedPlan->AssignedEmergencyImportByType[ResourceArrayIndex];

            if (NormalAmount > 0)
            {
                Harbor->AddResourceStock(ResourceType, NormalAmount);
                ImportExpense +=
                    static_cast<long long>(NormalAmount) *
                    static_cast<long long>(
                        ResourceTradePricing::GetImportPricePerStockUnit(
                            ResourceType));
            }

            if (EmergencyAmount > 0)
            {
                Harbor->AddResourceStock(ResourceType, EmergencyAmount);
                ImportExpense +=
                    static_cast<long long>(EmergencyAmount) *
                    static_cast<long long>(
                        ResolveEmergencyImportUnitPrice(ResourceType));
            }
        }

        return ImportExpense;
    }

    int* ResolveTaxRatePercent(
        FTaxPolicy& TaxPolicy,
        ETaxPolicyType Type)
    {
        switch (Type)
        {
        case ETaxPolicyType::Consumption:
            return &TaxPolicy.ConsumptionRatePercent;
        case ETaxPolicyType::Income:
            return &TaxPolicy.IncomeRatePercent;
        case ETaxPolicyType::Property:
            return &TaxPolicy.PropertyRatePercent;
        default:
            return nullptr;
        }
    }

    FTaxEventEconomyEffects ResolveTaxEventEconomyEffects(
        const FTaxPolicyEventStatus* TaxEventStatus)
    {
        FTaxEventEconomyEffects Effects;

        if (!TaxEventStatus ||
            !TaxEventStatus->Active ||
            TaxEventStatus->Type == ETaxPolicyEventType::None)
        {
            return Effects;
        }

        const double Severity = Clamp<double>(
            static_cast<double>(TaxEventStatus->DaysActive + 1) / 6.0,
            0.0,
            1.0);

        switch (TaxEventStatus->Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            Effects.IncomeTaxLeakage = 0.08 + 0.14 * Severity;
            Effects.GlobalUpkeepMultiplier = 1.03 + 0.05 * Severity;
            Effects.CollectionEfficiencyPenalty = 0.03 + 0.05 * Severity;
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            Effects.PropertyTaxLeakage = 0.18 + 0.24 * Severity;
            Effects.ResidentialUpkeepMultiplier = 1.10 + 0.18 * Severity;
            Effects.CollectionEfficiencyPenalty = 0.02 + 0.03 * Severity;
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            Effects.ExportMultiplier = 0.82 - 0.18 * Severity;
            Effects.ConsumptionTaxLeakage = 0.06 + 0.08 * Severity;
            Effects.IncomeTaxLeakage = 0.05 + 0.08 * Severity;
            Effects.PropertyTaxLeakage = 0.08 + 0.10 * Severity;
            Effects.GlobalUpkeepMultiplier = 1.08 + 0.12 * Severity;
            Effects.CollectionEfficiencyPenalty = 0.08 + 0.08 * Severity;
            break;
        default:
            break;
        }

        Effects.ExportMultiplier =
            Clamp<double>(Effects.ExportMultiplier, 0.45, 1.0);
        Effects.ConsumptionTaxLeakage =
            Clamp<double>(Effects.ConsumptionTaxLeakage, 0.0, 0.75);
        Effects.IncomeTaxLeakage =
            Clamp<double>(Effects.IncomeTaxLeakage, 0.0, 0.75);
        Effects.PropertyTaxLeakage =
            Clamp<double>(Effects.PropertyTaxLeakage, 0.0, 0.75);
        Effects.ResidentialUpkeepMultiplier =
            (std::max)(1.0, Effects.ResidentialUpkeepMultiplier);
        Effects.GlobalUpkeepMultiplier =
            (std::max)(1.0, Effects.GlobalUpkeepMultiplier);
        Effects.CollectionEfficiencyPenalty =
            Clamp<double>(Effects.CollectionEfficiencyPenalty, 0.0, 0.35);
        return Effects;
    }

    std::wstring BuildTaxPolicyEventWarningSummary(
        ETaxPolicyEventType Type,
        int DaysActive)
    {
        const bool Escalated = DaysActive >= 4;

        switch (Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            return Escalated ?
                L"자본주의자와 지식인이 근로세 경감을 최후통첩합니다." :
                L"자본주의자와 지식인이 근로세 경감을 요구합니다.";
        case ETaxPolicyEventType::PropertyTaxBacklash:
            return Escalated ?
                L"보수주의자와 자본주의자가 재산세 유예를 강하게 압박합니다." :
                L"보수주의자와 자본주의자가 재산세 유예를 요구합니다.";
        case ETaxPolicyEventType::BudgetCrisis:
            return Escalated ?
                L"보수주의자와 공산주의자가 재정 안정 대책을 최후통첩합니다." :
                L"보수주의자와 공산주의자가 재정 안정 대책을 요구합니다.";
        default:
            return L"정치 경고가 감지되었습니다.";
        }
    }

    std::wstring BuildTaxPolicyEventResolvedSummary(
        ETaxPolicyEventType Type,
        bool Success)
    {
        switch (Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            return Success ?
                L"자본주의자와 지식인이 한발 물러섰습니다." :
                L"자본주의자와 지식인의 조세 시위가 소강 상태로 돌아섰습니다.";
        case ETaxPolicyEventType::PropertyTaxBacklash:
            return Success ?
                L"보수주의자와 자본주의자의 재산권 압박이 진정되었습니다." :
                L"보수주의자와 자본주의자의 반발이 소강 상태에 들어갔습니다.";
        case ETaxPolicyEventType::BudgetCrisis:
            return Success ?
                L"보수주의자와 공산주의자의 재정 압박이 진정되었습니다." :
                L"재정 압박 연대가 일단락되었지만 경계는 남아 있습니다.";
        default:
            return Success ?
                L"정치 경고가 진정되었습니다." :
                L"정치 경고가 일단락되었습니다.";
        }
    }

    int GetTaxPolicyEventDurationDays(ETaxPolicyEventType Type)
    {
        switch (Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            return 8;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            return 7;
        case ETaxPolicyEventType::BudgetCrisis:
            return 9;
        default:
            return 0;
        }
    }

    int GetTaxPolicyEventCooldownDays(bool Success)
    {
        return Success ? 18 : 24;
    }

    void StartTaxPolicyEvent(
        FTaxPolicyEventStatus& InOutTaxEventStatus,
        ETaxPolicyEventType Type,
        int SimulationYear,
        int SimulationMonth,
        int SimulationDay,
        long long ImmediateBudgetDelta,
        long long& InOutNationalBudget,
        long long& InOutLastDailyNetChange,
        int& InOutWorkerTaxPressureDays,
        int& InOutPropertyTaxPressureDays,
        int& InOutBudgetCrisisPressureDays)
    {
        if (Type == ETaxPolicyEventType::None || InOutTaxEventStatus.Active)
            return;

        InOutTaxEventStatus = FTaxPolicyEventStatus();
        InOutTaxEventStatus.Type = Type;
        InOutTaxEventStatus.Active = true;
        InOutTaxEventStatus.RemainingDays = GetTaxPolicyEventDurationDays(Type);
        InOutTaxEventStatus.NotificationDays = 6;
        InOutTaxEventStatus.DaysActive = 0;
        InOutTaxEventStatus.TriggerYear = SimulationYear;
        InOutTaxEventStatus.TriggerMonth = SimulationMonth;
        InOutTaxEventStatus.TriggerDay = SimulationDay;
        InOutTaxEventStatus.Title = EconomySystem::GetTaxPolicyEventTitle(Type);
        InOutTaxEventStatus.Summary = BuildTaxPolicyEventWarningSummary(Type, 0);
        InOutWorkerTaxPressureDays = 0;
        InOutPropertyTaxPressureDays = 0;
        InOutBudgetCrisisPressureDays = 0;

        if (ImmediateBudgetDelta != 0)
        {
            InOutNationalBudget += ImmediateBudgetDelta;
            InOutLastDailyNetChange += ImmediateBudgetDelta;
        }
    }

    void ResolveTaxPolicyEventState(
        FTaxPolicyEventStatus& InOutTaxEventStatus,
        bool Success)
    {
        if (InOutTaxEventStatus.Type == ETaxPolicyEventType::None)
            return;

        InOutTaxEventStatus.Active = false;
        InOutTaxEventStatus.RemainingDays = 0;
        InOutTaxEventStatus.CooldownDays =
            GetTaxPolicyEventCooldownDays(Success);
        InOutTaxEventStatus.NotificationDays = 8;
        InOutTaxEventStatus.Summary = BuildTaxPolicyEventResolvedSummary(
            InOutTaxEventStatus.Type,
            Success);
        InOutTaxEventStatus.DaysActive = 0;
    }
}

EconomySystem::FDailyResult EconomySystem::ApplyDailySettlement(
    CWorld* World,
    int DaysInMonth,
    const FGovernmentProfile& GovernmentProfile,
    const FTaxPolicyEventStatus* TaxEventStatus)
{
    FDailyResult Result;
    const FTaxEventEconomyEffects EventEffects =
        ResolveTaxEventEconomyEffects(TaxEventStatus);

    const auto BuildingList = EconomyWorldAccess::CollectBuildings(World);

    if (BuildingList.empty())
        return Result;

    std::vector<bool> ShipArrivedByBuilding(BuildingList.size(), false);
    std::vector<std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>>
        ArrivedExportHubs;
    std::vector<std::shared_ptr<EconomyWorldAccess::IEconomyBuildingAccess>>
        ArrivedHarbors;
    double PropertyTaxIncome = 0.0;
    const long long ImportBudgetCap =
        TradePolicy::GetDailyImportBudgetCap(
            GovernmentProfile.ImportTradePolicy);

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        const auto& Building = BuildingList[i];

        if (!IsOperationalBuilding(Building))
        {
            continue;
        }

        Result.WageCost += Building->GetDailyWageCost(DaysInMonth);
        const double BaseDailyUpkeep =
            static_cast<double>(Building->GetDailyUpkeepCost(DaysInMonth));
        double EffectiveUpkeepMultiplier = EventEffects.GlobalUpkeepMultiplier;

        if (Building->IsResidential())
        {
            EffectiveUpkeepMultiplier *=
                EventEffects.ResidentialUpkeepMultiplier;
        }

        Result.UpkeepCost += static_cast<long long>(std::llround(
            BaseDailyUpkeep * EffectiveUpkeepMultiplier));
        PropertyTaxIncome +=
            (std::max)(
                2.0,
                BaseDailyUpkeep *
                static_cast<double>(
                    GovernmentProfile.TaxPolicy.PropertyRatePercent) / 100.0);

        if (Building->CanExportStoredResources())
        {
            ShipArrivedByBuilding[i] =
                Building->AdvanceHarborShipProgressAndCheckArrival(
                    DaysInMonth);
            if (ShipArrivedByBuilding[i])
            {
                ArrivedExportHubs.push_back(Building);

                if (Building->IsHarbor())
                    ArrivedHarbors.push_back(Building);
            }
        }
    }

    FHarborTradeDemandSnapshot HarborTradeDemand =
        BuildHarborTradeDemandSnapshot(BuildingList);
    AssignHarborExportPlans(
        ArrivedExportHubs,
        GovernmentProfile.ExportTradePolicy,
        EventEffects.ExportMultiplier,
        HarborTradeDemand);

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        const auto& Building = BuildingList[i];

        if (!IsOperationalBuilding(Building))
            continue;

        if (Building->CanExportStoredResources() && ShipArrivedByBuilding[i])
        {
            Result.ExportIncome += SettleHarborExportIncome(
                Building,
                GovernmentProfile.ExportTradePolicy,
                EventEffects.ExportMultiplier,
                HarborTradeDemand);
        }
    }

    AssignHarborImportPlans(
        ArrivedHarbors,
        GovernmentProfile.ImportTradePolicy,
        ImportBudgetCap,
        HarborTradeDemand,
        HarborTradeDemand);

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        const auto& Building = BuildingList[i];

        if (!IsOperationalBuilding(Building))
            continue;

        if (Building->IsHarbor() && ShipArrivedByBuilding[i])
        {
            Result.ImportExpense += SettleHarborImportExpense(
                Building,
                HarborTradeDemand);
        }

        if (Building->IsWarehouse())
            Building->ApplyDailyWarehouseStorageLoss();
    }

    const auto CitizenList = EconomyWorldAccess::CollectCitizens(World);
    double ConsumptionTaxIncome = 0.0;
    double IncomeTaxIncome = 0.0;
    double ResidenceTaxIncome = 0.0;
    double SecuritySum = 0.0;
    double OverallSum = 0.0;
    int ActiveCitizenCount = 0;

    for (size_t i = 0; i < CitizenList.size(); ++i)
    {
        const auto& Citizen = CitizenList[i];

        if (!Citizen || !Citizen->IsOperational())
            continue;

        ++ActiveCitizenCount;
        const EconomyWorldAccess::FCitizenEconomySnapshot Snapshot =
            Citizen->GetEconomySnapshot();
        SecuritySum += Snapshot.Security;
        OverallSum += Snapshot.Overall;
        const double ConsumptionTaxableSpendBase =
            GameConstants::Economy::DailyConsumptionSpendBase *
            ResolveConsumptionTaxableSpendMultiplier(Snapshot.WealthLevel);
        const double IncomeTaxableBase =
            GameConstants::Economy::DailyWorkerIncomeBase *
            ResolveIncomeTaxableBaseMultiplier(Snapshot.WealthLevel);
        const double PropertyTaxableBase =
            GameConstants::Economy::DailyResidenceValueBase *
            ResolvePropertyTaxableBaseMultiplier(Snapshot.WealthLevel);

        ConsumptionTaxIncome +=
            ConsumptionTaxableSpendBase *
            static_cast<double>(
                GovernmentProfile.TaxPolicy.ConsumptionRatePercent) /
            100.0;

        if (Snapshot.HasWorkBuilding)
        {
            IncomeTaxIncome +=
                IncomeTaxableBase *
                static_cast<double>(
                    GovernmentProfile.TaxPolicy.IncomeRatePercent) /
                100.0;
        }

        if (Snapshot.HasHomeBuilding)
        {
            ResidenceTaxIncome +=
                PropertyTaxableBase *
                static_cast<double>(
                    GovernmentProfile.TaxPolicy.PropertyRatePercent) /
                100.0;
        }
    }

    ConsumptionTaxIncome *= 1.0 - EventEffects.ConsumptionTaxLeakage;
    IncomeTaxIncome *= 1.0 - EventEffects.IncomeTaxLeakage;

    double CollectionEfficiency = 0.80;

    if (ActiveCitizenCount > 0)
    {
        const double Denominator = static_cast<double>(ActiveCitizenCount);
        const double AverageSecurity = SecuritySum / Denominator;
        const double AverageOverall = OverallSum / Denominator;

        CollectionEfficiency =
            0.45 +
            AverageSecurity / 200.0 +
            AverageOverall / 400.0;
        CollectionEfficiency =
            (std::max)(0.45, (std::min)(1.10, CollectionEfficiency));
    }

    PropertyTaxIncome += ResidenceTaxIncome;
    PropertyTaxIncome *= 1.0 - EventEffects.PropertyTaxLeakage;
    CollectionEfficiency -= EventEffects.CollectionEfficiencyPenalty;
    CollectionEfficiency =
        (std::max)(0.35, (std::min)(1.10, CollectionEfficiency));
    const double GrossTaxIncome =
        ConsumptionTaxIncome +
        IncomeTaxIncome +
        PropertyTaxIncome;

    Result.TaxCollectionEfficiency = CollectionEfficiency;
    Result.TaxIncome = static_cast<long long>(std::llround(
        GrossTaxIncome * CollectionEfficiency));
    Result.ConsumptionTaxIncome = static_cast<long long>(std::llround(
        ConsumptionTaxIncome * CollectionEfficiency));
    Result.IncomeTaxIncome = static_cast<long long>(std::llround(
        IncomeTaxIncome * CollectionEfficiency));
    Result.PropertyTaxIncome =
        Result.TaxIncome -
        Result.ConsumptionTaxIncome -
        Result.IncomeTaxIncome;

    Result.NetChange = Result.ExportIncome +
        Result.TaxIncome -
        Result.WageCost - Result.UpkeepCost - Result.ImportExpense;

    return Result;
}

EconomySystem::FWorldSettlementResult EconomySystem::ApplyDailyWorldSettlement(
    CWorld* World,
    int DaysInMonth,
    const FGovernmentProfile& GovernmentProfile,
    const FTaxPolicyEventStatus& TaxEventStatus,
    const std::vector<FGovernmentEdictState>& GovernmentEdicts,
    const FGovernmentEdictModifiers& EdictModifiers)
{
    FWorldSettlementResult Result;
    Result.BaseResult = ApplyDailySettlement(
        World,
        DaysInMonth,
        GovernmentProfile,
        &TaxEventStatus);

    Result.AdjustedTaxIncome = static_cast<long long>(std::llround(
        static_cast<double>(Result.BaseResult.TaxIncome) *
        static_cast<double>(EdictModifiers.TaxRevenueMultiplier)));

    if (Result.BaseResult.TaxIncome > 0 && Result.AdjustedTaxIncome > 0)
    {
        const double BaseTaxIncome =
            static_cast<double>(Result.BaseResult.TaxIncome);
        Result.AdjustedConsumptionTaxIncome = static_cast<long long>(
            std::llround(
                static_cast<double>(Result.AdjustedTaxIncome) *
                static_cast<double>(Result.BaseResult.ConsumptionTaxIncome) /
                BaseTaxIncome));
        Result.AdjustedIncomeTaxIncome = static_cast<long long>(
            std::llround(
                static_cast<double>(Result.AdjustedTaxIncome) *
                static_cast<double>(Result.BaseResult.IncomeTaxIncome) /
                BaseTaxIncome));
        Result.AdjustedPropertyTaxIncome =
            Result.AdjustedTaxIncome -
            Result.AdjustedConsumptionTaxIncome -
            Result.AdjustedIncomeTaxIncome;
    }

    const long long TaxRevenueDelta =
        Result.AdjustedTaxIncome - Result.BaseResult.TaxIncome;
    const long long DailyEdictUpkeep =
        EdictBudgetRuntime::CalculateDailyUpkeep(
            GovernmentEdicts,
            DaysInMonth);
    const long long DailyEdictBudgetDelta =
        EdictModifiers.DailyBudgetDelta;
    Result.DailyTradePolicyBudgetDelta =
        TradePolicyRuntime::ComputeDailyTradePolicyBudgetDelta(
            GovernmentProfile.ExportTradePolicy,
            GovernmentProfile.ImportTradePolicy,
            Result.BaseResult.ExportIncome,
            Result.BaseResult.ImportExpense);

    Result.DailyEdictCost = DailyEdictUpkeep - DailyEdictBudgetDelta;
    Result.NetBudgetChange =
        Result.BaseResult.NetChange +
        TaxRevenueDelta -
        DailyEdictUpkeep +
        DailyEdictBudgetDelta +
        Result.DailyTradePolicyBudgetDelta;
    return Result;
}

int EconomySystem::ApplyTaxPolicyRateDelta(
    FTaxPolicy& TaxPolicy,
    ETaxPolicyType Type,
    int DeltaPercent)
{
    int* TargetRatePercent = ResolveTaxRatePercent(TaxPolicy, Type);

    if (!TargetRatePercent)
        return 0;

    const int PreviousRatePercent = *TargetRatePercent;
    const int NewRatePercent = (std::max)(
        GetTaxPolicyMinPercent(Type),
        (std::min)(
            GetTaxPolicyMaxPercent(Type),
            PreviousRatePercent + DeltaPercent));
    *TargetRatePercent = NewRatePercent;
    return NewRatePercent - PreviousRatePercent;
}

bool EconomySystem::AdjustTaxPolicy(
    FTaxPolicy& TaxPolicy,
    ETaxPolicyType Type,
    int DeltaPercent,
    std::wstring& OutMessage)
{
    int* TargetRatePercent = ResolveTaxRatePercent(TaxPolicy, Type);

    if (!TargetRatePercent)
    {
        OutMessage = L"정의되지 않은 세율 정책입니다.";
        return false;
    }

    const int PreviousRatePercent = *TargetRatePercent;
    const int NewRatePercent = (std::max)(
        GetTaxPolicyMinPercent(Type),
        (std::min)(
            GetTaxPolicyMaxPercent(Type),
            PreviousRatePercent + DeltaPercent));

    if (PreviousRatePercent == NewRatePercent)
    {
        OutMessage =
            std::wstring(GetTaxPolicyDisplayName(Type)) +
            (DeltaPercent < 0 ?
                L"는 이미 최저 세율입니다." :
                L"는 이미 최고 세율입니다.");
        return false;
    }

    *TargetRatePercent = NewRatePercent;
    OutMessage =
        std::wstring(GetTaxPolicyDisplayName(Type)) +
        L" " +
        std::to_wstring(PreviousRatePercent) +
        L"% -> " +
        std::to_wstring(NewRatePercent) +
        L"%";
    return true;
}

ETaxPolicyEventType EconomySystem::GetRequiredTaxPolicyEventForEdict(
    EGovernmentEdictType Type)
{
    switch (Type)
    {
    case EGovernmentEdictType::LaborTaxRelief:
        return ETaxPolicyEventType::WorkerTaxStrike;
    case EGovernmentEdictType::PropertyTaxRelief:
        return ETaxPolicyEventType::PropertyTaxBacklash;
    case EGovernmentEdictType::EmergencyAusterity:
        return ETaxPolicyEventType::BudgetCrisis;
    default:
        return ETaxPolicyEventType::None;
    }
}

const wchar_t* EconomySystem::GetTaxPolicyEventTitle(ETaxPolicyEventType Type)
{
    switch (Type)
    {
    case ETaxPolicyEventType::WorkerTaxStrike:
        return L"자본주의자·지식인 조세 시위";
    case ETaxPolicyEventType::PropertyTaxBacklash:
        return L"보수주의자·자본주의자 재산권 반발";
    case ETaxPolicyEventType::BudgetCrisis:
        return L"보수주의자·공산주의자 재정 압박";
    default:
        return L"정치 경고";
    }
}

void EconomySystem::ResolveTaxPolicyEvent(
    FTaxPolicyEventStatus& InOutTaxEventStatus,
    bool Success)
{
    ResolveTaxPolicyEventState(InOutTaxEventStatus, Success);
}

void EconomySystem::ApplyDailyTaxPolicyEventEffects(
    CWorld* World,
    const FTaxPolicyEventStatus& TaxEventStatus)
{
    if (!World ||
        !TaxEventStatus.Active ||
        TaxEventStatus.Type == ETaxPolicyEventType::None)
    {
        return;
    }

    const float Escalation =
        1.0f +
        (std::min)(1.35f,
            static_cast<float>(TaxEventStatus.DaysActive) / 4.0f);
    const float ControlBreakdown =
        1.0f +
        (std::min)(0.80f,
            static_cast<float>(TaxEventStatus.DaysActive) / 6.0f);
    float FoodDelta = 0.f;
    float HealthDelta = 0.f;
    float FunDelta = 0.f;
    float FaithDelta = 0.f;
    float HousingDelta = 0.f;
    float JobDelta = 0.f;
    float FreedomDelta = 0.f;
    float SecurityDelta = 0.f;

    switch (TaxEventStatus.Type)
    {
    case ETaxPolicyEventType::WorkerTaxStrike:
        JobDelta = -0.80f * Escalation;
        FreedomDelta = -0.55f * ControlBreakdown;
        SecurityDelta = -0.25f * ControlBreakdown;
        FunDelta = -0.16f * Escalation;
        break;
    case ETaxPolicyEventType::PropertyTaxBacklash:
        HousingDelta = -0.85f * Escalation;
        FreedomDelta = -0.45f * ControlBreakdown;
        SecurityDelta = -0.20f * ControlBreakdown;
        HealthDelta = -0.10f * Escalation;
        break;
    case ETaxPolicyEventType::BudgetCrisis:
        FoodDelta = -0.70f * Escalation;
        JobDelta = -0.55f * Escalation;
        SecurityDelta = -0.40f * ControlBreakdown;
        HealthDelta = -0.18f * Escalation;
        FunDelta = -0.18f * Escalation;
        break;
    default:
        return;
    }

    const auto OrbList = EconomyWorldAccess::CollectCitizens(World);

    for (size_t i = 0; i < OrbList.size(); ++i)
    {
        const auto& Orb = OrbList[i];

        if (!Orb || !Orb->IsOperational())
            continue;

        Orb->ApplySatisfactionDelta(
            FoodDelta,
            HealthDelta,
            FunDelta,
            FaithDelta,
            HousingDelta,
            JobDelta,
            FreedomDelta,
            SecurityDelta);
    }
}

void EconomySystem::TickTaxPolicyEvents(
    const FPoliticalWorldSnapshot& Snapshot,
    const FGovernmentProfile& GovernmentProfile,
    int SimulationYear,
    int SimulationMonth,
    int SimulationDay,
    long long& InOutNationalBudget,
    long long& InOutLastDailyNetChange,
    int& InOutWorkerTaxPressureDays,
    int& InOutPropertyTaxPressureDays,
    int& InOutBudgetCrisisPressureDays,
    FTaxPolicyEventStatus& InOutTaxEventStatus)
{
    if (InOutTaxEventStatus.NotificationDays > 0)
        --InOutTaxEventStatus.NotificationDays;

    if (!InOutTaxEventStatus.Active && InOutTaxEventStatus.CooldownDays > 0)
        --InOutTaxEventStatus.CooldownDays;

    const int ActiveCitizenCount =
        (std::max)(0, Snapshot.ActiveCitizenCount);
    const double SupportPercent =
        ActiveCitizenCount > 0 ?
        static_cast<double>(Snapshot.IncumbentCount) /
            static_cast<double>(ActiveCitizenCount) * 100.0 :
        50.0;
    const float WorkerTaxBurden = GetCitizenTaxBurdenNormalized(
        GovernmentProfile.TaxPolicy,
        true,
        false);
    const float ResidentTaxBurden = GetCitizenTaxBurdenNormalized(
        GovernmentProfile.TaxPolicy,
        false,
        true);
    const float OverallTaxBurden = GetCitizenTaxBurdenNormalized(
        GovernmentProfile.TaxPolicy,
        true,
        true);
    const bool WorkerPressure =
        WorkerTaxBurden >= 0.45f ||
        (WorkerTaxBurden >= 0.30f && SupportPercent <= 48.0);
    const bool PropertyPressure =
        ResidentTaxBurden >= 0.50f ||
        (ResidentTaxBurden >= 0.35f && SupportPercent <= 50.0);
    const bool BudgetPressure =
        OverallTaxBurden <= -0.35f &&
        InOutLastDailyNetChange <= -5000;

    if (WorkerPressure)
        ++InOutWorkerTaxPressureDays;
    else
        InOutWorkerTaxPressureDays =
            (std::max)(0, InOutWorkerTaxPressureDays - 2);

    if (PropertyPressure)
        ++InOutPropertyTaxPressureDays;
    else
        InOutPropertyTaxPressureDays =
            (std::max)(0, InOutPropertyTaxPressureDays - 2);

    if (BudgetPressure)
        ++InOutBudgetCrisisPressureDays;
    else
        InOutBudgetCrisisPressureDays =
            (std::max)(0, InOutBudgetCrisisPressureDays - 2);

    if (InOutTaxEventStatus.Active)
    {
        ++InOutTaxEventStatus.DaysActive;
        InOutTaxEventStatus.Summary = BuildTaxPolicyEventWarningSummary(
            InOutTaxEventStatus.Type,
            InOutTaxEventStatus.DaysActive);

        if (InOutTaxEventStatus.RemainingDays > 0)
            --InOutTaxEventStatus.RemainingDays;

        const bool CanResolveEarly = InOutTaxEventStatus.DaysActive >= 3;

        switch (InOutTaxEventStatus.Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            if (CanResolveEarly && !WorkerPressure)
            {
                ResolveTaxPolicyEventState(InOutTaxEventStatus, true);
                return;
            }
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            if (CanResolveEarly && !PropertyPressure)
            {
                ResolveTaxPolicyEventState(InOutTaxEventStatus, true);
                return;
            }
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            if (CanResolveEarly && !BudgetPressure)
            {
                ResolveTaxPolicyEventState(InOutTaxEventStatus, true);
                return;
            }
            break;
        default:
            break;
        }

        if (InOutTaxEventStatus.RemainingDays <= 0)
        {
            ResolveTaxPolicyEventState(InOutTaxEventStatus, false);
            return;
        }

        long long DailyEventPenalty = 0;
        const float Escalation =
            1.0f +
            (std::min)(1.50f,
                static_cast<float>(InOutTaxEventStatus.DaysActive) / 4.0f);

        switch (InOutTaxEventStatus.Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            DailyEventPenalty = static_cast<long long>(std::llround(
                900.0 + 450.0 * static_cast<double>(Escalation)));
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            DailyEventPenalty = static_cast<long long>(std::llround(
                750.0 + 360.0 * static_cast<double>(Escalation)));
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            DailyEventPenalty = static_cast<long long>(std::llround(
                1400.0 + 650.0 * static_cast<double>(Escalation)));
            break;
        default:
            break;
        }

        if (DailyEventPenalty > 0)
        {
            InOutNationalBudget -= DailyEventPenalty;
            InOutLastDailyNetChange -= DailyEventPenalty;
        }

        return;
    }

    if (InOutTaxEventStatus.CooldownDays > 0)
        return;

    if (InOutBudgetCrisisPressureDays >= 4)
    {
        StartTaxPolicyEvent(
            InOutTaxEventStatus,
            ETaxPolicyEventType::BudgetCrisis,
            SimulationYear,
            SimulationMonth,
            SimulationDay,
            -6000,
            InOutNationalBudget,
            InOutLastDailyNetChange,
            InOutWorkerTaxPressureDays,
            InOutPropertyTaxPressureDays,
            InOutBudgetCrisisPressureDays);
        InOutBudgetCrisisPressureDays = 0;
        return;
    }

    if (InOutWorkerTaxPressureDays >= 5 &&
        InOutWorkerTaxPressureDays >= InOutPropertyTaxPressureDays)
    {
        StartTaxPolicyEvent(
            InOutTaxEventStatus,
            ETaxPolicyEventType::WorkerTaxStrike,
            SimulationYear,
            SimulationMonth,
            SimulationDay,
            -4000,
            InOutNationalBudget,
            InOutLastDailyNetChange,
            InOutWorkerTaxPressureDays,
            InOutPropertyTaxPressureDays,
            InOutBudgetCrisisPressureDays);
        InOutWorkerTaxPressureDays = 0;
        return;
    }

    if (InOutPropertyTaxPressureDays >= 5)
    {
        StartTaxPolicyEvent(
            InOutTaxEventStatus,
            ETaxPolicyEventType::PropertyTaxBacklash,
            SimulationYear,
            SimulationMonth,
            SimulationDay,
            -3500,
            InOutNationalBudget,
            InOutLastDailyNetChange,
            InOutWorkerTaxPressureDays,
            InOutPropertyTaxPressureDays,
            InOutBudgetCrisisPressureDays);
        InOutPropertyTaxPressureDays = 0;
    }
}
