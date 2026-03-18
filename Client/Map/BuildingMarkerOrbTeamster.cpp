#include "BuildingMarkerOrb.h"
#include "PlacementAreaObject.h"
#include "../StringUtils.h"
#include "../World/MainWorldUiReadAccess.h"
#include "World/World.h"
#include <cstdarg>
#include <cstdio>
#include <cfloat>
#include <limits>

namespace
{
#ifdef _DEBUG
    void DebugTeamsterLog(const char* Format, ...)
    {
        char Text[768] = {};

        va_list Args;
        va_start(Args, Format);
        vsprintf_s(Text, Format, Args);
        va_end(Args);

        OutputDebugStringA(Text);
    }

    std::string DebugResourceName(EResourceType Type)
    {
        return Type == EResourceType::None ?
            std::string("None") :
            StringUtils::WideToUtf8(GetResourceTypeDisplayName(Type));
    }

    const char* DebugExportPriorityBucketName(int PriorityBucket)
    {
        switch (PriorityBucket)
        {
        case 0:
            return "warehouse_to_harbor_staging";
        case 1:
            return "unused";
        case 2:
            return "producer_to_harbor_staging";
        case 3:
            return "producer_to_warehouse_overflow";
        default:
            break;
        }

        return "unknown";
    }

    const char* DebugExportDropoffKind(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        if (!Building)
            return "unknown";

        if (Building->CanExportStoredResources())
            return "harbor";

        if (Building->IsWarehouse())
            return "warehouse";

        return "building";
    }
#endif

    bool IsOperationalBuilding(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        return Building &&
            Building->GetAlive() &&
            Building->GetEnable() &&
            Building->HasPlacedArea();
    }

    bool TryGetCoverageDistanceSq(
        const std::shared_ptr<CPlacementAreaObject>& Office,
        const std::shared_ptr<CPlacementAreaObject>& Building,
        float& OutDistSq)
    {
        OutDistSq = FLT_MAX;

        if (!Office || !Building)
            return false;

        int OfficeGridX = 0;
        int OfficeGridY = 0;
        int BuildingGridX = 0;
        int BuildingGridY = 0;

        if (!Office->GetPlacedCenterGridCoords(OfficeGridX, OfficeGridY) ||
            !Building->GetPlacedCenterGridCoords(
                BuildingGridX, BuildingGridY))
        {
            return false;
        }

        const float dx = static_cast<float>(OfficeGridX - BuildingGridX);
        const float dy = static_cast<float>(OfficeGridY - BuildingGridY);
        OutDistSq = dx * dx + dy * dy;
        return true;
    }

    bool IsWithinTeamsterCoverage(
        const std::shared_ptr<CPlacementAreaObject>& Office,
        const std::shared_ptr<CPlacementAreaObject>& Building)
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

    int ResolveOfficeTransferUnit(
        const std::shared_ptr<CPlacementAreaObject>& Office)
    {
        return Office ?
            Office->GetTeamsterTransferUnits() :
            (std::max)(1, GameConstants::Orb::TeamsterTransferUnit);
    }

    bool TryResolveConsumerNeed(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        EResourceType& OutType,
        int& OutCurrentStock)
    {
        OutType = EResourceType::None;
        OutCurrentStock = 0;

        if (!IsOperationalBuilding(Building) ||
            Building->IsRoad() ||
            Building->IsBusStop() ||
            Building->IsHarbor() ||
            Building->IsTransportOffice() ||
            Building->IsWarehouse())
        {
            return false;
        }

        bool FoundNeed = false;
        int BestCurrentStock = 0;

        auto ConsiderNeed = [&](EResourceType Type)
        {
            if (Type == EResourceType::None ||
                Type == Building->GetProducedResourceType() ||
                (Type == Building->GetVisitConsumptionResourceType() &&
                    ShouldVisitConsumptionUseSelfProducedFoodOnly(
                        Building->GetBuildingId())))
            {
                return;
            }

            const bool VisitConsumptionDemand =
                Type == Building->GetVisitConsumptionResourceType();
            const bool CompatibleProductionInputDemand =
                !VisitConsumptionDemand &&
                Type == EResourceType::FeedCrops;
            const int CurrentStock =
                VisitConsumptionDemand ?
                    Building->GetVisitConsumptionCompatibleResourceStock(Type) +
                        Building
                            ->GetVisitConsumptionCompatibleReservedIncomingResourceAmount(
                                Type) :
                CompatibleProductionInputDemand ?
                    Building->GetProductionInputCompatibleResourceStock(Type) +
                        Building
                            ->GetProductionInputCompatibleReservedIncomingResourceAmount(
                                Type) :
                    Building->GetResourceStock(Type) +
                        Building->GetReservedIncomingResourceAmount(Type);

            if (CurrentStock >=
                GameConstants::Orb::TeamsterConsumerRestockThreshold)
            {
                return;
            }

            if (!FoundNeed || CurrentStock < BestCurrentStock)
            {
                FoundNeed = true;
                OutType = Type;
                BestCurrentStock = CurrentStock;
                OutCurrentStock = CurrentStock;
            }
        };

        ConsiderNeed(Building->GetVisitConsumptionResourceType());

        for (int SlotIndex = 0;
             SlotIndex < Building->GetProductionInputCount();
             ++SlotIndex)
        {
            const EResourceType InputType =
                Building->GetProductionInputType(SlotIndex);

            if (InputType == Building->GetVisitConsumptionResourceType())
                continue;

            ConsiderNeed(InputType);
        }

        return FoundNeed;
    }

    bool TryResolveConsumerSupply(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        EResourceType DemandType,
        EResourceType& OutSupplyType,
        int& OutAvailableAmount)
    {
        OutSupplyType = EResourceType::None;
        OutAvailableAmount = 0;

        if (!IsOperationalBuilding(Building) ||
            DemandType == EResourceType::None)
        {
            return false;
        }

        auto ConsiderSupplyType = [&](EResourceType CandidateType)
        {
            if (CandidateType == EResourceType::None ||
                CandidateType == EResourceType::Count)
            {
                return;
            }

            int AvailableAmount = 0;

            if (Building->IsWarehouse() || Building->IsHarbor())
            {
                AvailableAmount =
                    Building->GetAvailableResourceStock(CandidateType);
            }
            else if (Building->SupportsTeamsterPickup() &&
                Building->GetProducedResourceType() == CandidateType)
            {
                AvailableAmount =
                    Building->GetAvailableResourceStock(CandidateType);
            }

            if (AvailableAmount <= 0)
                return;

            if (OutSupplyType == EResourceType::None ||
                AvailableAmount > OutAvailableAmount)
            {
                OutSupplyType = CandidateType;
                OutAvailableAmount = AvailableAmount;
            }
        };

        if (DemandType == Building->GetVisitConsumptionResourceType() &&
            IsSummaryResourceType(DemandType))
        {
            const std::vector<EResourceType> AcceptedTypes =
                Building->GetRuntimeVisitConsumptionAcceptedResourceTypes();

            if (!AcceptedTypes.empty())
            {
                for (size_t Index = 0; Index < AcceptedTypes.size(); ++Index)
                    ConsiderSupplyType(AcceptedTypes[Index]);
            }
            else
            {
                ForEachFoodVisitCompatibleResourceType(
                    Building->GetBuildingId(),
                    Building->GetProducedResourceType(),
                    DemandType,
                    ConsiderSupplyType);
            }
        }
        else
        {
            if (DemandType == EResourceType::FeedCrops)
            {
                ForEachFeedCompatibleResourceType(
                    DemandType,
                    ConsiderSupplyType);
            }
            else
            {
                ConsiderSupplyType(DemandType);
            }
        }

        return OutSupplyType != EResourceType::None;
    }

    struct FTeamsterResourcePressure
    {
        int ConsumerCount = 0;
        int GlobalShortage = 0;
        int TotalAvailableStock = 0;
        int WarehouseBufferedStock = 0;
        int WarehouseFreeCapacity = 0;
    };

    struct FTeamsterExportHubPressure
    {
        int ExportHubCount = 0;
        int HarborBufferedStock = 0;
        int HarborFreeCapacity = 0;
        int IslandExportHeadroom = 0;
        int HarborStagingTarget = 0;
        int PendingStagingDemand = 0;
    };

    struct FTeamsterExportTriggerInfo
    {
        bool Triggered = false;
        EResourceType Type = EResourceType::None;
        int AvailableAmount = 0;
        int RequestedAmount = 0;
        int PendingStagingDemand = 0;
        std::string HarborName;
    };

    bool BuildingConsumesResource(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        EResourceType Type)
    {
        if (!IsOperationalBuilding(Building) ||
            Type == EResourceType::None ||
            Building->IsRoad() ||
            Building->IsBusStop() ||
            Building->IsHarbor() ||
            Building->IsTransportOffice() ||
            Building->IsWarehouse())
        {
            return false;
        }

        if (Building->GetVisitConsumptionResourceType() == Type &&
            Building->GetProducedResourceType() != Type)
        {
            return true;
        }

        return Building->UsesProductionInputResource(Type);
    }

    bool WarehouseHasAssignedSlot(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        EResourceType Type)
    {
        if (!IsOperationalBuilding(Building) ||
            !Building->IsWarehouse() ||
            Type == EResourceType::None)
        {
            return false;
        }

        for (int SlotIndex = 0;
             SlotIndex < Building->GetWarehouseSlotCount();
             ++SlotIndex)
        {
            if (Building->GetWarehouseSlotType(SlotIndex) == Type)
                return true;
        }

        return false;
    }

    FTeamsterResourcePressure BuildTeamsterResourcePressure(
        const std::vector<std::weak_ptr<CPlacementAreaObject>>& BuildingList,
        EResourceType Type)
    {
        FTeamsterResourcePressure Pressure;

        if (Type == EResourceType::None)
            return Pressure;

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            auto Building = BuildingList[i].lock();

            if (!IsOperationalBuilding(Building))
                continue;

            Pressure.TotalAvailableStock +=
                Building->GetAvailableResourceStock(Type);

            if (Building->IsWarehouse())
            {
                Pressure.WarehouseBufferedStock +=
                    Building->GetResourceStock(Type) +
                    Building->GetReservedIncomingResourceAmount(Type);
                Pressure.WarehouseFreeCapacity +=
                    Building->GetAvailableIncomingCapacity(Type);
            }

            if (!BuildingConsumesResource(Building, Type))
                continue;

            ++Pressure.ConsumerCount;

            const bool VisitConsumptionDemand =
                Type == Building->GetVisitConsumptionResourceType();
            const int CurrentStock =
                VisitConsumptionDemand ?
                    Building->GetVisitConsumptionCompatibleResourceStock(Type) +
                        Building
                            ->GetVisitConsumptionCompatibleReservedIncomingResourceAmount(
                                Type) :
                    Building->GetResourceStock(Type) +
                        Building->GetReservedIncomingResourceAmount(Type);
            Pressure.GlobalShortage += (std::max)(
                0,
                GameConstants::Orb::TeamsterConsumerTargetStock -
                    CurrentStock);
        }

        return Pressure;
    }

    const TradePolicy::FExportTradePolicy* ResolveExportTradePolicy(
        const std::shared_ptr<CWorld>& World)
    {
        auto MainWorldAccess =
            ResolveMainWorldAlmanacAccess(World);

        if (!MainWorldAccess)
            return nullptr;

        return &MainWorldAccess->GetGovernmentProfile().ExportTradePolicy;
    }

    int ResolveRequestedTransferAmount(
        int AvailableAmount,
        int TransferUnit,
        int DesiredAmount = (std::numeric_limits<int>::max)())
    {
        if (AvailableAmount <= 0 ||
            TransferUnit <= 0 ||
            DesiredAmount <= 0)
        {
            return 0;
        }

        return (std::max)(
            0,
            (std::min)(
                AvailableAmount,
                (std::min)(TransferUnit, DesiredAmount)));
    }

    FTeamsterExportHubPressure BuildTeamsterExportHubPressure(
        const std::shared_ptr<CPlacementAreaObject>& OfficeBuilding,
        const std::vector<std::weak_ptr<CPlacementAreaObject>>& BuildingList,
        EResourceType Type,
        const FTeamsterResourcePressure& ResourcePressure,
        const TradePolicy::FExportTradePolicy* ExportPolicy)
    {
        FTeamsterExportHubPressure Pressure;

        if (!OfficeBuilding || !IsExportableResourceType(Type))
            return Pressure;

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            auto Building = BuildingList[i].lock();

            if (!IsOperationalBuilding(Building) ||
                !Building->CanExportStoredResources() ||
                !IsWithinTeamsterCoverage(OfficeBuilding, Building))
            {
                continue;
            }

            ++Pressure.ExportHubCount;
            Pressure.HarborBufferedStock +=
                Building->GetResourceStock(Type) +
                Building->GetReservedIncomingResourceAmount(Type);
            Pressure.HarborFreeCapacity +=
                Building->GetAvailableIncomingCapacity(Type);
        }

        if (Pressure.ExportHubCount <= 0)
            return Pressure;

        // Keep island-wide export headroom separate from current harbor stock.
        // The first answers "may we export this nationally?", while the second
        // answers "how much should teamsters stage at export hubs right now?".
        const int IslandExportHeadroom = (std::max)(
            0,
            ResourcePressure.TotalAvailableStock);
        const int ShipCapacity = ExportPolicy ?
            TradePolicy::GetHarborExportShipCapacityUnits(*ExportPolicy) :
            TradePolicy::GDefaultHarborExportShipCapacityUnits;

        Pressure.IslandExportHeadroom = IslandExportHeadroom;

        if (IslandExportHeadroom <= 0 || ShipCapacity <= 0)
            return Pressure;

        const long long TargetStock = (std::min)(
            static_cast<long long>(IslandExportHeadroom),
            static_cast<long long>(ShipCapacity) *
                static_cast<long long>(Pressure.ExportHubCount));
        Pressure.HarborStagingTarget = static_cast<int>((std::min)(
            TargetStock,
            static_cast<long long>((std::numeric_limits<int>::max)())));
        Pressure.PendingStagingDemand = (std::min)(
            Pressure.HarborFreeCapacity,
            (std::max)(
                0,
                Pressure.HarborStagingTarget - Pressure.HarborBufferedStock));
        return Pressure;
    }

    std::string FindBestExportHubDropoffName(
        const std::shared_ptr<CPlacementAreaObject>& OfficeBuilding,
        const std::vector<std::weak_ptr<CPlacementAreaObject>>& BuildingList,
        EResourceType CargoType,
        int CargoAmount)
    {
        if (!OfficeBuilding ||
            CargoType == EResourceType::None ||
            CargoAmount <= 0)
        {
            return std::string();
        }

        std::string BestName;
        int BestBufferedStock = (std::numeric_limits<int>::max)();
        int BestIncomingCapacity = -1;
        float BestDistSq = FLT_MAX;

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            auto Building = BuildingList[i].lock();

            if (!IsOperationalBuilding(Building) ||
                !Building->CanExportStoredResources() ||
                !IsWithinTeamsterCoverage(OfficeBuilding, Building) ||
                Building->GetAvailableIncomingCapacity(CargoType) < CargoAmount)
            {
                continue;
            }

            float DistSq = FLT_MAX;

            if (!TryGetCoverageDistanceSq(OfficeBuilding, Building, DistSq))
                continue;

            const int BufferedStock =
                Building->GetResourceStock(CargoType) +
                Building->GetReservedIncomingResourceAmount(CargoType);
            const int IncomingCapacity =
                Building->GetAvailableIncomingCapacity(CargoType);

            if (BestName.empty() ||
                BufferedStock < BestBufferedStock ||
                (BufferedStock == BestBufferedStock &&
                 IncomingCapacity > BestIncomingCapacity) ||
                (BufferedStock == BestBufferedStock &&
                 IncomingCapacity == BestIncomingCapacity &&
                 DistSq < BestDistSq))
            {
                BestName = Building->GetName();
                BestBufferedStock = BufferedStock;
                BestIncomingCapacity = IncomingCapacity;
                BestDistSq = DistSq;
            }
        }

        return BestName;
    }

    std::string FindBestWarehouseDropoffName(
        const std::shared_ptr<CPlacementAreaObject>& OfficeBuilding,
        const std::vector<std::weak_ptr<CPlacementAreaObject>>& BuildingList,
        const std::string& ExcludedBuildingName,
        EResourceType CargoType,
        int CargoAmount)
    {
        if (!OfficeBuilding ||
            CargoType == EResourceType::None ||
            CargoAmount <= 0)
        {
            return std::string();
        }

        std::string BestName;
        bool BestHasAssignedSlot = false;
        int BestBufferedStock = (std::numeric_limits<int>::max)();
        int BestIncomingCapacity = -1;
        float BestDistSq = FLT_MAX;

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            auto Building = BuildingList[i].lock();

            if (!IsOperationalBuilding(Building) ||
                !Building->IsWarehouse() ||
                Building->GetName() == ExcludedBuildingName ||
                !IsWithinTeamsterCoverage(OfficeBuilding, Building) ||
                !Building->CanStoreResourceType(CargoType) ||
                Building->GetAvailableIncomingCapacity(CargoType) <
                    CargoAmount)
            {
                continue;
            }

            float DistSq = FLT_MAX;

            if (!TryGetCoverageDistanceSq(OfficeBuilding, Building, DistSq))
                continue;

            const bool HasAssignedSlot =
                WarehouseHasAssignedSlot(Building, CargoType);
            const int BufferedStock =
                Building->GetResourceStock(CargoType) +
                Building->GetReservedIncomingResourceAmount(CargoType);
            const int IncomingCapacity =
                Building->GetAvailableIncomingCapacity(CargoType);

            if (BestName.empty() ||
                (HasAssignedSlot && !BestHasAssignedSlot) ||
                (HasAssignedSlot == BestHasAssignedSlot &&
                 BufferedStock < BestBufferedStock) ||
                (HasAssignedSlot == BestHasAssignedSlot &&
                 BufferedStock == BestBufferedStock &&
                 IncomingCapacity > BestIncomingCapacity) ||
                (HasAssignedSlot == BestHasAssignedSlot &&
                 BufferedStock == BestBufferedStock &&
                 IncomingCapacity == BestIncomingCapacity &&
                 DistSq < BestDistSq))
            {
                BestName = Building->GetName();
                BestHasAssignedSlot = HasAssignedSlot;
                BestBufferedStock = BufferedStock;
                BestIncomingCapacity = IncomingCapacity;
                BestDistSq = DistSq;
            }
        }

        return BestName;
    }

    FTeamsterExportTriggerInfo TryResolveExportTrigger(
        const std::shared_ptr<CPlacementAreaObject>& OfficeBuilding,
        const std::shared_ptr<CPlacementAreaObject>& Building,
        const std::vector<std::weak_ptr<CPlacementAreaObject>>& BuildingList,
        const std::array<FTeamsterExportHubPressure,
            static_cast<size_t>(EResourceType::Count)>& ExportHubPressure,
        const TradePolicy::FExportTradePolicy* ExportPolicy,
        int TransferUnit)
    {
        FTeamsterExportTriggerInfo Result;

        if (!OfficeBuilding ||
            !IsOperationalBuilding(Building) ||
            !IsWithinTeamsterCoverage(OfficeBuilding, Building) ||
            !Building->SupportsTeamsterPickup())
        {
            return Result;
        }

        const EResourceType ProducedType = Building->GetProducedResourceType();
        if (!IsExportableResourceType(ProducedType))
            return Result;

        if (ExportPolicy &&
            !TradePolicy::IsResourceExportAllowed(
                *ExportPolicy,
                ProducedType))
        {
            return Result;
        }

        const size_t ResourceIndex = static_cast<size_t>(ProducedType);
        if (ResourceIndex >= static_cast<size_t>(EResourceType::Count))
            return Result;

        const int AvailableAmount =
            Building->GetAvailableResourceStock(ProducedType);
        if (AvailableAmount <= 0)
            return Result;

        const FTeamsterExportHubPressure& HubPressure =
            ExportHubPressure[ResourceIndex];
        if (HubPressure.PendingStagingDemand <= 0)
            return Result;

        const int RequestedAmount = ResolveRequestedTransferAmount(
            AvailableAmount,
            TransferUnit,
            HubPressure.PendingStagingDemand);
        if (RequestedAmount <= 0)
            return Result;

        const std::string HarborName = FindBestExportHubDropoffName(
            OfficeBuilding,
            BuildingList,
            ProducedType,
            RequestedAmount);
        if (HarborName.empty())
            return Result;

        Result.Triggered = true;
        Result.Type = ProducedType;
        Result.AvailableAmount = AvailableAmount;
        Result.RequestedAmount = RequestedAmount;
        Result.PendingStagingDemand = HubPressure.PendingStagingDemand;
        Result.HarborName = HarborName;
        return Result;
    }
}

void CBuildingMarkerOrb::TransitionFsm(ECitizenState NewState)
{
    auto& Delivery = mTeamsterDeliveryState;
    auto& FoodStockAvailableThisVisit =
        mCitizenProfileState.FoodStockAvailableThisVisit;
    auto& FunServiceAvailableThisVisit =
        mCitizenProfileState.FunServiceAvailableThisVisit;
    auto& HealthServiceAvailableThisVisit =
        mCitizenProfileState.HealthServiceAvailableThisVisit;
    auto& FaithServiceAvailableThisVisit =
        mCitizenProfileState.FaithServiceAvailableThisVisit;

    if (mCitizenState != NewState)
        ReleaseServiceVisitReservations();

    if (!IsTeamsterState(NewState))
        ResetTeamsterSpeed();

    mCitizenState = NewState;
    mDwellTimer = 0.f;

    switch (NewState)
    {
    case ECitizenState::GoingToWork:
        mFoodVisitBuildingName.clear();
        ReleaseTeamsterReservations();
        Delivery.ClearRoute();
        mCurrentTargetName = mWorkName;
        break;
    case ECitizenState::AtWork:
        mFoodVisitBuildingName.clear();
        ReleaseTeamsterReservations();
        Delivery.ClearRoute();
        mDwellTimer = GameConstants::Orb::AtWorkDurationSeconds;
        mCurrentTargetName.clear();
        break;
    case ECitizenState::GoingHome:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName = mHomeName;
        break;
    case ECitizenState::AtHome:
        mFoodVisitBuildingName.clear();
        mDwellTimer = GameConstants::Orb::AtHomeDurationSeconds;
        mCurrentTargetName.clear();
        break;
    case ECitizenState::GoingToFood:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName = mFoodName;
        break;
    case ECitizenState::AtFood:
    {
        const std::string VisitBuildingName = mCurrentTargetName.empty() ?
            mFoodName :
            mCurrentTargetName;
        mDwellTimer = GameConstants::Orb::AtFoodDurationSeconds;
        mCurrentTargetName.clear();
        mFoodVisitBuildingName = VisitBuildingName;
        FoodStockAvailableThisVisit = false;
        mFoodVisitReserved = false;
        {
            auto FoodWorld = mWorld.lock();
            if (FoodWorld && !mFoodVisitBuildingName.empty())
            {
                auto FoodBuilding =
                    FoodWorld->FindObject<CPlacementAreaObject>(
                        mFoodVisitBuildingName).lock();
                if (FoodBuilding)
                {
                    mFoodVisitReserved =
                        FoodBuilding->TryBeginServiceVisit(
                            EBuildingServiceType::Food);
                }
            }
        }
        break;
    }
    case ECitizenState::GoingToFun:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName = mFunName;
        break;
    case ECitizenState::AtFun:
    {
        const std::string VisitBuildingName = mCurrentTargetName.empty() ?
            mFunName :
            mCurrentTargetName;
        mFoodVisitBuildingName.clear();
        mDwellTimer = GameConstants::Orb::AtFunDurationSeconds;
        mCurrentTargetName.clear();
        mFunVisitBuildingName = VisitBuildingName;
        FunServiceAvailableThisVisit = false;
        {
            auto FunWorld = mWorld.lock();
            if (FunWorld && !mFunVisitBuildingName.empty())
            {
                auto FunBuilding =
                    FunWorld->FindObject<CPlacementAreaObject>(
                        mFunVisitBuildingName).lock();
                if (FunBuilding)
                {
                    mFunVisitReserved =
                        FunBuilding->TryBeginServiceVisit(
                            EBuildingServiceType::Fun);
                    FunServiceAvailableThisVisit =
                        mFunVisitReserved &&
                        FunBuilding->TryConsumeServiceStock(
                            EBuildingServiceType::Fun,
                            1);
                }
            }
        }
        break;
    }
    case ECitizenState::GoingToHealth:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName = mHealthName;
        break;
    case ECitizenState::AtHealth:
    {
        const std::string VisitBuildingName = mCurrentTargetName.empty() ?
            mHealthName :
            mCurrentTargetName;
        mFoodVisitBuildingName.clear();
        mDwellTimer = GameConstants::Orb::AtHealthDurationSeconds;
        mCurrentTargetName.clear();
        mHealthVisitBuildingName = VisitBuildingName;
        HealthServiceAvailableThisVisit = false;
        {
            auto HealthWorld = mWorld.lock();
            if (HealthWorld && !mHealthVisitBuildingName.empty())
            {
                auto HealthBuilding =
                    HealthWorld->FindObject<CPlacementAreaObject>(
                        mHealthVisitBuildingName).lock();
                if (HealthBuilding)
                {
                    mHealthVisitReserved =
                        HealthBuilding->TryBeginServiceVisit(
                            EBuildingServiceType::Health);
                    HealthServiceAvailableThisVisit =
                        mHealthVisitReserved &&
                        HealthBuilding->TryConsumeServiceStock(
                            EBuildingServiceType::Health,
                            1);
                }
            }
        }
        break;
    }
    case ECitizenState::GoingToFaith:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName = mFaithName;
        break;
    case ECitizenState::AtFaith:
    {
        const std::string VisitBuildingName = mCurrentTargetName.empty() ?
            mFaithName :
            mCurrentTargetName;
        mFoodVisitBuildingName.clear();
        mDwellTimer = GameConstants::Orb::AtFaithDurationSeconds;
        mCurrentTargetName.clear();
        mFaithVisitBuildingName = VisitBuildingName;
        FaithServiceAvailableThisVisit = false;
        {
            auto FaithWorld = mWorld.lock();
            if (FaithWorld && !mFaithVisitBuildingName.empty())
            {
                auto FaithBuilding =
                    FaithWorld->FindObject<CPlacementAreaObject>(
                        mFaithVisitBuildingName).lock();
                if (FaithBuilding)
                {
                    mFaithVisitReserved =
                        FaithBuilding->TryBeginServiceVisit(
                            EBuildingServiceType::Faith);
                    FaithServiceAvailableThisVisit =
                        mFaithVisitReserved &&
                        FaithBuilding->TryConsumeServiceStock(
                            EBuildingServiceType::Faith,
                            1);
                }
            }
        }
        break;
    }
    case ECitizenState::GoingToTeamsterSource:
        mFoodVisitBuildingName.clear();
        StartTeamsterSpeedBoost();
        mCurrentTargetName = Delivery.SourceName;
        break;
    case ECitizenState::GoingToTeamsterHarbor:
        mFoodVisitBuildingName.clear();
        StartTeamsterSpeedBoost();
        mCurrentTargetName = Delivery.DestinationName;
        break;
    case ECitizenState::GoingToTeamsterConsumerSource:
        mFoodVisitBuildingName.clear();
        StartTeamsterSpeedBoost();
        mCurrentTargetName = Delivery.SourceName;
        break;
    case ECitizenState::GoingToTeamsterConsumerTarget:
        mFoodVisitBuildingName.clear();
        StartTeamsterSpeedBoost();
        mCurrentTargetName = Delivery.DestinationName;
        break;
    case ECitizenState::GoingToTeamsterOffice:
        mFoodVisitBuildingName.clear();
        StartTeamsterSpeedBoost();
        mCurrentTargetName = mWorkName;
        break;
    default:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName.clear();
        break;
    }
}

void CBuildingMarkerOrb::SetHomeBuilding(const std::string& Name)
{
    mHomeName = Name;
    TryStartCoreLoop();
}

void CBuildingMarkerOrb::SetWorkBuilding(const std::string& Name)
{
    auto& Delivery = mTeamsterDeliveryState;

    if (mWorkName != Name)
    {
        ReleaseTeamsterReservations();
        Delivery.ClearRoute();
        ResetTeamsterSpeed();
    }

    mWorkName = Name;

    if (IsTeamsterState(mCitizenState))
        TransitionFsm(ECitizenState::GoingToWork);

    TryStartCoreLoop();
}

void CBuildingMarkerOrb::SetFoodBuilding(const std::string& Name)
{
    mFoodName = Name;
    TryStartCoreLoop();
}

void CBuildingMarkerOrb::SetFunBuilding(const std::string& Name)
{
    mFunName = Name;
}

void CBuildingMarkerOrb::SetHealthBuilding(const std::string& Name)
{
    mHealthName = Name;
}

void CBuildingMarkerOrb::SetFaithBuilding(const std::string& Name)
{
    mFaithName = Name;
}

void CBuildingMarkerOrb::TryStartCoreLoop()
{
    if (mCitizenState != ECitizenState::Wander)
        return;

    if (mHomeName.empty() || mWorkName.empty() || mFoodName.empty())
        return;

    TransitionFsm(ECitizenState::GoingToWork);
    mHasLockedTarget = false;
    mPathRetryAccum = 0.f;
}

bool CBuildingMarkerOrb::IsTeamsterState(ECitizenState State) const
{
    return State == ECitizenState::GoingToTeamsterSource ||
        State == ECitizenState::GoingToTeamsterHarbor ||
        State == ECitizenState::GoingToTeamsterConsumerSource ||
        State == ECitizenState::GoingToTeamsterConsumerTarget ||
        State == ECitizenState::GoingToTeamsterOffice;
}

void CBuildingMarkerOrb::StartTeamsterSpeedBoost()
{
    mTeamsterDeliveryState.SpeedBoostActive = true;
    mMoveSpeed =
        mDefaultMoveSpeed *
        GameConstants::Orb::TeamsterSpeedMultiplier;
}

void CBuildingMarkerOrb::ResetTeamsterSpeed()
{
    mTeamsterDeliveryState.SpeedBoostActive = false;
    mMoveSpeed = mDefaultMoveSpeed;
}

void CBuildingMarkerOrb::ReleaseServiceVisitReservations()
{
    auto World = mWorld.lock();

    auto ReleaseVisit = [&](const std::string& BuildingName,
                            bool& Reserved,
                            EBuildingServiceType ServiceType)
    {
        if (World && Reserved && !BuildingName.empty())
        {
            auto Building =
                World->FindObject<CPlacementAreaObject>(BuildingName).lock();

            if (Building)
                Building->EndServiceVisit(ServiceType);
        }

        Reserved = false;
    };

    ReleaseVisit(
        mFoodVisitBuildingName,
        mFoodVisitReserved,
        EBuildingServiceType::Food);
    ReleaseVisit(
        mFunVisitBuildingName,
        mFunVisitReserved,
        EBuildingServiceType::Fun);
    ReleaseVisit(
        mHealthVisitBuildingName,
        mHealthVisitReserved,
        EBuildingServiceType::Health);
    ReleaseVisit(
        mFaithVisitBuildingName,
        mFaithVisitReserved,
        EBuildingServiceType::Faith);

    mFoodVisitBuildingName.clear();
    mFunVisitBuildingName.clear();
    mHealthVisitBuildingName.clear();
    mFaithVisitBuildingName.clear();
    mCitizenProfileState.FoodStockAvailableThisVisit = false;
    mCitizenProfileState.FunServiceAvailableThisVisit = false;
    mCitizenProfileState.HealthServiceAvailableThisVisit = false;
    mCitizenProfileState.FaithServiceAvailableThisVisit = false;
}

void CBuildingMarkerOrb::ReleaseTeamsterReservations()
{
    auto World = mWorld.lock();
    auto& Delivery = mTeamsterDeliveryState;

    if (World &&
        Delivery.RequestedAmount > 0 &&
        !Delivery.SourceName.empty())
    {
        auto SourceBuilding =
            World->FindObject<CPlacementAreaObject>(Delivery.SourceName).lock();

        if (SourceBuilding)
        {
            if (Delivery.SourceReservationKind ==
                FTeamsterDeliveryState::ESourceReservationKind::Exportable)
            {
                SourceBuilding->ReleaseTeamsterExportPickup(
                    Delivery.RequestedAmount);
            }
            else if (Delivery.SourceReservationKind ==
                        FTeamsterDeliveryState::ESourceReservationKind::Typed &&
                     Delivery.RequestedType != EResourceType::None)
            {
                SourceBuilding->ReleaseTeamsterPickup(
                    Delivery.RequestedType,
                    Delivery.RequestedAmount);
            }
        }
    }

    if (World &&
        Delivery.DestinationReservationActive &&
        Delivery.HasDestinationReservation() &&
        !Delivery.DestinationName.empty())
    {
        auto DestinationBuilding =
            World->FindObject<CPlacementAreaObject>(
                Delivery.DestinationName).lock();

        if (DestinationBuilding)
        {
            DestinationBuilding->ReleaseIncomingResource(
                Delivery.ReservedDestinationType,
                Delivery.ReservedDestinationAmount);
        }
    }

    Delivery.SourceReservationKind =
        FTeamsterDeliveryState::ESourceReservationKind::None;
    Delivery.ClearDestinationReservation();
}

bool CBuildingMarkerOrb::TryStartTeamsterDelivery()
{
    if (mCitizenState != ECitizenState::AtWork)
        return false;

    auto World = mWorld.lock();

    if (!World || mWorkName.empty())
        return false;

    auto WorkBuilding =
        World->FindObject<CPlacementAreaObject>(mWorkName).lock();

    if (!WorkBuilding ||
        !WorkBuilding->GetAlive() ||
        !WorkBuilding->GetEnable() ||
        !WorkBuilding->HasPlacedArea() ||
        !WorkBuilding->IsTransportOffice())
    {
        return false;
    }

    FTeamsterDeliveryState PlannedDelivery;

    if (TryPlanTeamsterConsumerDelivery(WorkBuilding, PlannedDelivery))
    {
        mTeamsterDeliveryState = PlannedDelivery;
        TransitionFsm(ECitizenState::GoingToTeamsterConsumerSource);
        mPathRetryAccum = 0.f;
        return true;
    }

    if (!TryPlanTeamsterExportDelivery(WorkBuilding, PlannedDelivery))
        return false;

    mTeamsterDeliveryState = PlannedDelivery;
    TransitionFsm(ECitizenState::GoingToTeamsterSource);
    mPathRetryAccum = 0.f;
    return true;
}

bool CBuildingMarkerOrb::TryPlanTeamsterConsumerDelivery(
    const std::shared_ptr<CPlacementAreaObject>& OfficeBuilding,
    FTeamsterDeliveryState& OutDelivery) const
{
    OutDelivery.ClearRoute();

    auto World = mWorld.lock();

    if (!World || !OfficeBuilding)
        return false;

    const int TransferUnit = ResolveOfficeTransferUnit(OfficeBuilding);

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return false;

    const TradePolicy::FExportTradePolicy* const ExportPolicy =
        ResolveExportTradePolicy(World);
    std::array<FTeamsterResourcePressure,
        static_cast<size_t>(EResourceType::Count)> ResourcePressure = {};
    std::array<FTeamsterExportHubPressure,
        static_cast<size_t>(EResourceType::Count)> ExportHubPressure = {};

    for (size_t ResourceIndex = 1;
         ResourceIndex < static_cast<size_t>(EResourceType::Count);
         ++ResourceIndex)
    {
        ResourcePressure[ResourceIndex] = BuildTeamsterResourcePressure(
            BuildingList,
            static_cast<EResourceType>(ResourceIndex));
        ExportHubPressure[ResourceIndex] = BuildTeamsterExportHubPressure(
            OfficeBuilding,
            BuildingList,
            static_cast<EResourceType>(ResourceIndex),
            ResourcePressure[ResourceIndex],
            ExportPolicy);
    }

    auto TryFindConsumerSource = [&](
        EResourceType ResourceType,
        const std::string& ConsumerName,
        EResourceType& OutSupplyType,
        std::string& OutSourceName,
        int& OutAvailableAmount,
        float& OutSourceDistSq) -> bool
    {
        OutSupplyType = EResourceType::None;
        OutSourceName.clear();
        OutAvailableAmount = 0;
        OutSourceDistSq = FLT_MAX;

        auto ConsiderSource = [&](
            const std::shared_ptr<CPlacementAreaObject>& Building,
            bool RequireWarehouse,
            bool RequireProducer,
            bool RequireHarbor)
        {
            if (!IsOperationalBuilding(Building) ||
                Building->GetName() == ConsumerName ||
                !IsWithinTeamsterCoverage(OfficeBuilding, Building))
            {
                return;
            }

            if (RequireWarehouse && !Building->IsWarehouse())
                return;

            if (RequireProducer &&
                !Building->SupportsTeamsterPickup())
            {
                return;
            }

            if (RequireHarbor && !Building->CanExportStoredResources())
                return;

            EResourceType SupplyType = EResourceType::None;
            int AvailableAmount = 0;

            if (!TryResolveConsumerSupply(
                    Building,
                    ResourceType,
                    SupplyType,
                    AvailableAmount))
            {
                return;
            }

            if (AvailableAmount <= 0)
                return;

            float DistSq = FLT_MAX;

            if (!TryGetCoverageDistanceSq(OfficeBuilding, Building, DistSq))
                return;

            if (OutSourceName.empty() ||
                DistSq < OutSourceDistSq ||
                (DistSq == OutSourceDistSq &&
                 AvailableAmount > OutAvailableAmount))
            {
                OutSupplyType = SupplyType;
                OutSourceName = Building->GetName();
                OutAvailableAmount = AvailableAmount;
                OutSourceDistSq = DistSq;
            }
        };

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            ConsiderSource(BuildingList[i].lock(), true, false, false);
        }

        if (!OutSourceName.empty())
            return true;

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            ConsiderSource(BuildingList[i].lock(), false, true, false);
        }

        if (!OutSourceName.empty())
            return true;

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            ConsiderSource(BuildingList[i].lock(), false, false, true);
        }

        return !OutSourceName.empty();
    };

    std::string BestConsumerName;
    std::string BestSourceName;
    EResourceType BestResourceType = EResourceType::None;
    int BestCurrentStock =
        GameConstants::Orb::TeamsterConsumerRestockThreshold + 1;
    int BestRequestedAmount = 0;
    float BestConsumerDistSq = FLT_MAX;
    float BestSourceDistSq = FLT_MAX;
    std::string BestDeferredExportSourceName;
    float BestDeferredExportDistSq = FLT_MAX;
    FTeamsterExportTriggerInfo BestDeferredExportTrigger;

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        auto Building = BuildingList[i].lock();
        EResourceType ConsumerType = EResourceType::None;
        int CurrentStock = 0;
        FTeamsterExportTriggerInfo DeferredExportTrigger;

        if (!TryResolveConsumerNeed(Building, ConsumerType, CurrentStock) ||
            !IsWithinTeamsterCoverage(OfficeBuilding, Building))
        {
            // No restock target here: see if this producer should explicitly
            // stage cargo toward a harbor export hub instead.
            DeferredExportTrigger = TryResolveExportTrigger(
                OfficeBuilding,
                Building,
                BuildingList,
                ExportHubPressure,
                ExportPolicy,
                TransferUnit);

            if (DeferredExportTrigger.Triggered)
            {
                float ExportDistSq = FLT_MAX;

                if (TryGetCoverageDistanceSq(
                        OfficeBuilding,
                        Building,
                        ExportDistSq) &&
                    (BestDeferredExportSourceName.empty() ||
                     DeferredExportTrigger.PendingStagingDemand >
                        BestDeferredExportTrigger.PendingStagingDemand ||
                     (DeferredExportTrigger.PendingStagingDemand ==
                        BestDeferredExportTrigger.PendingStagingDemand &&
                      DeferredExportTrigger.RequestedAmount >
                        BestDeferredExportTrigger.RequestedAmount) ||
                     (DeferredExportTrigger.PendingStagingDemand ==
                        BestDeferredExportTrigger.PendingStagingDemand &&
                      DeferredExportTrigger.RequestedAmount ==
                        BestDeferredExportTrigger.RequestedAmount &&
                      ExportDistSq < BestDeferredExportDistSq)))
                {
                    BestDeferredExportSourceName = Building->GetName();
                    BestDeferredExportDistSq = ExportDistSq;
                    BestDeferredExportTrigger = DeferredExportTrigger;
                }
            }

            continue;
        }

        float ConsumerDistSq = FLT_MAX;

        if (!TryGetCoverageDistanceSq(OfficeBuilding, Building, ConsumerDistSq))
            continue;

        std::string SourceName;
        EResourceType SupplyType = EResourceType::None;
        int AvailableAmount = 0;
        float SourceDistSq = FLT_MAX;

        if (!TryFindConsumerSource(
                ConsumerType,
                Building->GetName(),
                SupplyType,
                SourceName,
                AvailableAmount,
                SourceDistSq))
        {
            continue;
        }

        int RequestedAmount = (std::max)(
            1,
            GameConstants::Orb::TeamsterConsumerTargetStock -
                CurrentStock);
        RequestedAmount = (std::min)(
            RequestedAmount,
            TransferUnit);
        RequestedAmount = (std::min)(RequestedAmount, AvailableAmount);

        if (RequestedAmount <= 0)
            continue;

        if (BestConsumerName.empty() ||
            CurrentStock < BestCurrentStock ||
            (CurrentStock == BestCurrentStock &&
             ConsumerDistSq < BestConsumerDistSq) ||
            (CurrentStock == BestCurrentStock &&
             ConsumerDistSq == BestConsumerDistSq &&
             SourceDistSq < BestSourceDistSq))
        {
            BestConsumerName = Building->GetName();
            BestSourceName = SourceName;
            BestResourceType = SupplyType;
            BestCurrentStock = CurrentStock;
            BestRequestedAmount = RequestedAmount;
            BestConsumerDistSq = ConsumerDistSq;
            BestSourceDistSq = SourceDistSq;
        }
    }

    if (BestConsumerName.empty() ||
        BestSourceName.empty() ||
        BestResourceType == EResourceType::None ||
        BestRequestedAmount <= 0)
    {
#ifdef _DEBUG
        if (!BestDeferredExportSourceName.empty())
        {
            DebugTeamsterLog(
                "[Teamster][ExportTrigger] office=%s source=%s resource=%s "
                "available=%d requested=%d pending_harbor_staging=%d harbor=%s\n",
                OfficeBuilding ? OfficeBuilding->GetName().c_str() : "",
                BestDeferredExportSourceName.c_str(),
                DebugResourceName(BestDeferredExportTrigger.Type).c_str(),
                BestDeferredExportTrigger.AvailableAmount,
                BestDeferredExportTrigger.RequestedAmount,
                BestDeferredExportTrigger.PendingStagingDemand,
                BestDeferredExportTrigger.HarborName.c_str());
        }
#endif
        return false;
    }

    auto SourceBuilding =
        World->FindObject<CPlacementAreaObject>(BestSourceName).lock();
    auto ConsumerBuilding =
        World->FindObject<CPlacementAreaObject>(BestConsumerName).lock();

    if (!SourceBuilding || !ConsumerBuilding)
        return false;

    // Export hubs (harbors) don't get a pickup reservation so that active
    // trade routes can still consume from them while the teamster is in
    // transit. The pickup at arrival will fail gracefully if the stock was
    // already exported, which is preferable to blocking export income.
    const bool ReserveSource = !SourceBuilding->CanExportStoredResources();

    if (ReserveSource &&
        !SourceBuilding->ReserveTeamsterPickup(
            BestResourceType,
            BestRequestedAmount))
    {
        return false;
    }

    if (!ConsumerBuilding->ReserveIncomingResource(
            BestResourceType,
            BestRequestedAmount))
    {
        if (ReserveSource)
        {
            SourceBuilding->ReleaseTeamsterPickup(
                BestResourceType,
                BestRequestedAmount);
        }
        return false;
    }

    OutDelivery.Mode = FTeamsterDeliveryState::ERouteMode::ConsumerDelivery;
    OutDelivery.SourceReservationKind =
        ReserveSource ?
            FTeamsterDeliveryState::ESourceReservationKind::Typed :
            FTeamsterDeliveryState::ESourceReservationKind::None;
    OutDelivery.SourceName = BestSourceName;
    OutDelivery.DestinationName = BestConsumerName;
    OutDelivery.RequestedType = BestResourceType;
    OutDelivery.RequestedAmount = BestRequestedAmount;
    OutDelivery.SetDestinationReservation(
        BestResourceType,
        BestRequestedAmount);
    return true;
}

bool CBuildingMarkerOrb::TryPlanTeamsterExportDelivery(
    const std::shared_ptr<CPlacementAreaObject>& OfficeBuilding,
    FTeamsterDeliveryState& OutDelivery) const
{
    OutDelivery.ClearRoute();

    auto World = mWorld.lock();

    if (!OfficeBuilding || !World)
        return false;

    const int TransferUnit = ResolveOfficeTransferUnit(OfficeBuilding);

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return false;

    const TradePolicy::FExportTradePolicy* const ExportPolicy =
        ResolveExportTradePolicy(World);
    std::array<FTeamsterResourcePressure,
        static_cast<size_t>(EResourceType::Count)> ResourcePressure = {};
    std::array<FTeamsterExportHubPressure,
        static_cast<size_t>(EResourceType::Count)> ExportHubPressure = {};

    for (size_t ResourceIndex = 1;
         ResourceIndex < static_cast<size_t>(EResourceType::Count);
         ++ResourceIndex)
    {
        ResourcePressure[ResourceIndex] = BuildTeamsterResourcePressure(
            BuildingList,
            static_cast<EResourceType>(ResourceIndex));
        ExportHubPressure[ResourceIndex] = BuildTeamsterExportHubPressure(
            OfficeBuilding,
            BuildingList,
            static_cast<EResourceType>(ResourceIndex),
            ResourcePressure[ResourceIndex],
            ExportPolicy);
    }

    std::string BestSourceName;
    std::string BestDropoffName;
    EResourceType BestCargoType = EResourceType::None;
    int BestPriorityBucket = (std::numeric_limits<int>::max)();
    int BestRequestedAmount = 0;
    float BestSourceDistSq = FLT_MAX;
    float BestDropoffDistSq = FLT_MAX;
    bool BestReserveIncoming = false;
    int BestPendingStagingDemand = 0;

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        auto SourceBuilding = BuildingList[i].lock();

        if (!IsOperationalBuilding(SourceBuilding) ||
            !IsWithinTeamsterCoverage(OfficeBuilding, SourceBuilding))
        {
            continue;
        }

        const bool SourceIsWarehouse = SourceBuilding->IsWarehouse();

        if (!SourceIsWarehouse &&
            !SourceBuilding->SupportsTeamsterPickup())
        {
            continue;
        }

        float SourceDistSq = FLT_MAX;

        if (!TryGetCoverageDistanceSq(
                OfficeBuilding,
                SourceBuilding,
                SourceDistSq))
        {
            continue;
        }

        for (size_t ResourceIndex = 1;
             ResourceIndex < static_cast<size_t>(EResourceType::Count);
             ++ResourceIndex)
        {
            const EResourceType CargoType =
                static_cast<EResourceType>(ResourceIndex);
            const int AvailableAmount =
                SourceBuilding->GetAvailableResourceStock(CargoType);

            if (!IsExportableResourceType(CargoType) ||
                AvailableAmount <= 0)
            {
                continue;
            }

            const FTeamsterResourcePressure& Pressure =
                ResourcePressure[ResourceIndex];
            const FTeamsterExportHubPressure& HubPressure =
                ExportHubPressure[ResourceIndex];
            const bool HarborExportAllowed =
                (!ExportPolicy ||
                    TradePolicy::IsResourceExportAllowed(
                        *ExportPolicy,
                        CargoType)) &&
                HubPressure.PendingStagingDemand > 0;

            std::string DropoffName;
            bool ReserveIncoming = false;
            int PriorityBucket = 0;
            int RequestedAmount = 0;
            int CandidatePendingStagingDemand = 0;

            if (SourceIsWarehouse)
            {
                if (!HarborExportAllowed)
                    continue;

                RequestedAmount = ResolveRequestedTransferAmount(
                    AvailableAmount,
                    TransferUnit,
                    HubPressure.PendingStagingDemand);

                if (RequestedAmount <= 0)
                    continue;

                DropoffName = FindBestExportHubDropoffName(
                    OfficeBuilding,
                    BuildingList,
                    CargoType,
                    RequestedAmount);

                if (DropoffName.empty())
                    continue;

                PriorityBucket = 0;
                CandidatePendingStagingDemand =
                    HubPressure.PendingStagingDemand;
            }
            else
            {
                if (HarborExportAllowed)
                {
                    RequestedAmount = ResolveRequestedTransferAmount(
                        AvailableAmount,
                        TransferUnit,
                        HubPressure.PendingStagingDemand);

                    if (RequestedAmount > 0)
                    {
                        DropoffName = FindBestExportHubDropoffName(
                            OfficeBuilding,
                            BuildingList,
                            CargoType,
                            RequestedAmount);
                    }

                    if (!DropoffName.empty())
                    {
                        PriorityBucket = 2;
                        CandidatePendingStagingDemand =
                            HubPressure.PendingStagingDemand;
                    }
                }

                if (DropoffName.empty())
                {
                    RequestedAmount = ResolveRequestedTransferAmount(
                        AvailableAmount,
                        TransferUnit);

                    if (RequestedAmount > 0)
                    {
                        DropoffName = FindBestWarehouseDropoffName(
                            OfficeBuilding,
                            BuildingList,
                            SourceBuilding->GetName(),
                            CargoType,
                            RequestedAmount);
                    }

                    if (!DropoffName.empty())
                    {
                        ReserveIncoming = true;
                        PriorityBucket = 3;
                    }
                }
            }

            if (DropoffName.empty() || RequestedAmount <= 0)
                continue;

            auto DropoffBuilding =
                World->FindObject<CPlacementAreaObject>(DropoffName).lock();

            if (!DropoffBuilding)
                continue;

            float DropoffDistSq = FLT_MAX;

            if (!TryGetCoverageDistanceSq(
                    OfficeBuilding,
                    DropoffBuilding,
                    DropoffDistSq))
            {
                continue;
            }

            if (BestSourceName.empty() ||
                PriorityBucket < BestPriorityBucket ||
                (PriorityBucket == BestPriorityBucket &&
                 RequestedAmount > BestRequestedAmount) ||
                (PriorityBucket == BestPriorityBucket &&
                 RequestedAmount == BestRequestedAmount &&
                 SourceDistSq < BestSourceDistSq) ||
                (PriorityBucket == BestPriorityBucket &&
                 RequestedAmount == BestRequestedAmount &&
                 SourceDistSq == BestSourceDistSq &&
                 DropoffDistSq < BestDropoffDistSq))
            {
                BestSourceName = SourceBuilding->GetName();
                BestDropoffName = DropoffName;
                BestCargoType = CargoType;
                BestPriorityBucket = PriorityBucket;
                BestRequestedAmount = RequestedAmount;
                BestSourceDistSq = SourceDistSq;
                BestDropoffDistSq = DropoffDistSq;
                BestReserveIncoming = ReserveIncoming;
                BestPendingStagingDemand =
                    CandidatePendingStagingDemand;
            }
        }
    }

    if (BestSourceName.empty() ||
        BestDropoffName.empty() ||
        BestCargoType == EResourceType::None ||
        BestRequestedAmount <= 0)
    {
        return false;
    }

    auto SourceBuilding =
        World->FindObject<CPlacementAreaObject>(BestSourceName).lock();
    auto DropoffBuilding =
        World->FindObject<CPlacementAreaObject>(BestDropoffName).lock();

#ifdef _DEBUG
    DebugTeamsterLog(
        "[Teamster][PlanExport] office=%s source=%s dropoff=%s "
        "dropoff_kind=%s resource=%s amount=%d reason=%s "
        "pending_harbor_staging=%d\n",
        OfficeBuilding ? OfficeBuilding->GetName().c_str() : "",
        BestSourceName.c_str(),
        BestDropoffName.c_str(),
        DebugExportDropoffKind(DropoffBuilding),
        DebugResourceName(BestCargoType).c_str(),
        BestRequestedAmount,
        DebugExportPriorityBucketName(BestPriorityBucket),
        BestPendingStagingDemand);
#endif

    if (!SourceBuilding ||
        !DropoffBuilding ||
        !SourceBuilding->ReserveTeamsterPickup(
            BestCargoType,
            BestRequestedAmount))
    {
        return false;
    }

    if (BestReserveIncoming &&
        !DropoffBuilding->ReserveIncomingResource(
            BestCargoType,
            BestRequestedAmount))
    {
        SourceBuilding->ReleaseTeamsterPickup(
            BestCargoType,
            BestRequestedAmount);
        return false;
    }

    OutDelivery.Mode = FTeamsterDeliveryState::ERouteMode::Export;
    OutDelivery.SourceReservationKind =
        FTeamsterDeliveryState::ESourceReservationKind::Typed;
    OutDelivery.SourceName = BestSourceName;
    OutDelivery.DestinationName = BestDropoffName;
    OutDelivery.RequestedAmount = BestRequestedAmount;
    OutDelivery.RequestedType = BestCargoType;
    if (BestReserveIncoming)
    {
        OutDelivery.SetDestinationReservation(
            BestCargoType,
            BestRequestedAmount);
    }
    else
    {
        OutDelivery.ClearDestinationReservation();
    }
    return true;
}

std::string CBuildingMarkerOrb::FindTeamsterSourceName() const
{
    auto World = mWorld.lock();

    if (!World || mWorkName.empty())
        return std::string();

    auto OfficeBuilding =
        World->FindObject<CPlacementAreaObject>(mWorkName).lock();

    if (!OfficeBuilding)
        return std::string();

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return std::string();

    auto FindBestSource = [&](
        bool RequireWarehouse) -> std::string
    {
        std::string BestName;
        int BestStock = 0;
        float BestDistSq = FLT_MAX;

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            auto Building = BuildingList[i].lock();

            if (!IsOperationalBuilding(Building) ||
                !IsWithinTeamsterCoverage(OfficeBuilding, Building))
            {
                continue;
            }

            if (RequireWarehouse)
            {
                if (!Building->IsWarehouse())
                    continue;
            }
            else
            {
                if (Building->IsWarehouse() ||
                    !Building->SupportsTeamsterPickup())
                {
                    continue;
                }
            }

            const int Stock =
                Building->GetAvailableExportableResourceStock();

            EResourceType ExportType = EResourceType::None;

            if (Stock < GameConstants::Orb::TeamsterTransferUnit ||
                !Building->TryGetExportableResourceTypeForAmount(
                    GameConstants::Orb::TeamsterTransferUnit,
                    ExportType))
            {
                continue;
            }

            float DistSq = FLT_MAX;

            if (!TryGetCoverageDistanceSq(OfficeBuilding, Building, DistSq))
                continue;

            if (BestName.empty() ||
                Stock > BestStock ||
                (Stock == BestStock && DistSq < BestDistSq))
            {
                BestName = Building->GetName();
                BestStock = Stock;
                BestDistSq = DistSq;
            }
        }

        return BestName;
    };

    const std::string WarehouseSource = FindBestSource(true);

    if (!WarehouseSource.empty())
        return WarehouseSource;

    return FindBestSource(false);
}

std::string CBuildingMarkerOrb::FindHarborName() const
{
    auto World = mWorld.lock();

    if (!World || mWorkName.empty())
        return std::string();

    auto OfficeBuilding =
        World->FindObject<CPlacementAreaObject>(mWorkName).lock();

    if (!OfficeBuilding)
        return std::string();

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return std::string();

    std::string BestName;
    float BestDistSq = FLT_MAX;

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        auto Building = BuildingList[i].lock();

        if (!IsOperationalBuilding(Building) ||
            !Building->CanExportStoredResources())
        {
            continue;
        }

        float DistSq = FLT_MAX;

        if (!TryGetCoverageDistanceSq(OfficeBuilding, Building, DistSq) ||
            !IsWithinTeamsterCoverage(OfficeBuilding, Building))
        {
            continue;
        }

        if (BestName.empty() || DistSq < BestDistSq)
        {
            BestName = Building->GetName();
            BestDistSq = DistSq;
        }
    }

    return BestName;
}

std::string CBuildingMarkerOrb::FindTeamsterExportDropoffName(
    const std::string& SourceName,
    EResourceType CargoType,
    int CargoAmount) const
{
    auto World = mWorld.lock();

    if (!World ||
        mWorkName.empty() ||
        CargoType == EResourceType::None ||
        CargoAmount <= 0)
    {
        return std::string();
    }

    auto OfficeBuilding =
        World->FindObject<CPlacementAreaObject>(mWorkName).lock();

    if (!OfficeBuilding)
        return std::string();

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return std::string();

    auto SourceBuilding =
        SourceName.empty() ?
            std::shared_ptr<CPlacementAreaObject>() :
            World->FindObject<CPlacementAreaObject>(SourceName).lock();
    const bool SourceIsWarehouse =
        SourceBuilding && SourceBuilding->IsWarehouse();
    const TradePolicy::FExportTradePolicy* const ExportPolicy =
        ResolveExportTradePolicy(World);
    const FTeamsterResourcePressure Pressure =
        BuildTeamsterResourcePressure(BuildingList, CargoType);
    const FTeamsterExportHubPressure HubPressure =
        BuildTeamsterExportHubPressure(
            OfficeBuilding,
            BuildingList,
            CargoType,
            Pressure,
            ExportPolicy);
    const std::string HarborName = FindBestExportHubDropoffName(
        OfficeBuilding,
        BuildingList,
        CargoType,
        CargoAmount);
    const bool HarborRouteAvailable =
        (!ExportPolicy ||
            TradePolicy::IsResourceExportAllowed(
                *ExportPolicy,
                CargoType)) &&
        !HarborName.empty();
    const bool HarborExportPreferred =
        HarborRouteAvailable &&
        HubPressure.PendingStagingDemand > 0;

    if (!SourceIsWarehouse)
    {
        if (HarborExportPreferred)
            return HarborName;

        if (HarborRouteAvailable)
            return HarborName;

        return FindBestWarehouseDropoffName(
            OfficeBuilding,
            BuildingList,
            SourceName,
            CargoType,
            CargoAmount);
    }

    if (HarborRouteAvailable)
        return HarborName;

    return std::string();
}
