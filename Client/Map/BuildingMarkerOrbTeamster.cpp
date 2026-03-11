#include "BuildingMarkerOrb.h"
#include "PlacementAreaObject.h"
#include "World/World.h"
#include <cfloat>

namespace
{
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
        return DistSq <= CoverageRadius * CoverageRadius;
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

        OutType = Building->GetVisitConsumptionResourceType();

        if (OutType == EResourceType::None)
            return false;

        // 자가 생산 가능한 시설은 우선순위에서 제외한다.
        if (Building->GetProducedResourceType() == OutType)
            return false;

        OutCurrentStock =
            Building->GetResourceStock(OutType) +
            Building->GetReservedIncomingResourceAmount(OutType);
        return OutCurrentStock <
            GameConstants::Orb::TeamsterConsumerRestockThreshold;
    }

    int ResolveConsumerSupplyAmount(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        EResourceType Type)
    {
        if (!IsOperationalBuilding(Building) ||
            Type == EResourceType::None)
        {
            return 0;
        }

        if (Building->IsWarehouse() || Building->IsHarbor())
            return Building->GetAvailableResourceStock(Type);

        if (!Building->SupportsTeamsterPickup() ||
            Building->GetProducedResourceType() != Type)
        {
            return 0;
        }

        return Building->GetAvailableResourceStock(Type);
    }
}

void CBuildingMarkerOrb::TransitionFsm(ECitizenState NewState)
{
    auto& Delivery = mTeamsterDeliveryState;
    auto& FoodStockAvailableThisVisit =
        mCitizenProfileState.FoodStockAvailableThisVisit;

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
        mDwellTimer = GAtWorkDuration;
        mCurrentTargetName.clear();
        break;
    case ECitizenState::GoingHome:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName = mHomeName;
        break;
    case ECitizenState::AtHome:
        mFoodVisitBuildingName.clear();
        mDwellTimer = GAtHomeDuration;
        mCurrentTargetName.clear();
        break;
    case ECitizenState::GoingToFood:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName = mFoodName;
        break;
    case ECitizenState::AtFood:
        mDwellTimer = GAtFoodDuration;
        mCurrentTargetName.clear();
        if (mFoodVisitBuildingName.empty())
            mFoodVisitBuildingName = mFoodName;
        FoodStockAvailableThisVisit = false;
        {
            auto FoodWorld = mWorld.lock();
            if (FoodWorld && !mFoodVisitBuildingName.empty())
            {
                auto FoodBuilding =
                    FoodWorld->FindObject<CPlacementAreaObject>(
                        mFoodVisitBuildingName).lock();
                if (FoodBuilding)
                {
                    FoodStockAvailableThisVisit =
                        FoodBuilding->TryConsumeResource(
                            EResourceType::Food,
                            1);
                }
            }
        }
        break;
    case ECitizenState::GoingToFun:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName = mFunName;
        break;
    case ECitizenState::AtFun:
        mFoodVisitBuildingName.clear();
        mDwellTimer = GAtFunDuration;
        mCurrentTargetName.clear();
        break;
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
    mMoveSpeed = mDefaultMoveSpeed * GTeamsterSpeedMultiplier;
}

void CBuildingMarkerOrb::ResetTeamsterSpeed()
{
    mTeamsterDeliveryState.SpeedBoostActive = false;
    mMoveSpeed = mDefaultMoveSpeed;
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
        Delivery.RequestedAmount > 0 &&
        Delivery.RequestedType != EResourceType::None &&
        !Delivery.DestinationName.empty())
    {
        auto DestinationBuilding =
            World->FindObject<CPlacementAreaObject>(
                Delivery.DestinationName).lock();

        if (DestinationBuilding)
        {
            DestinationBuilding->ReleaseIncomingResource(
                Delivery.RequestedType,
                Delivery.RequestedAmount);
        }
    }

    Delivery.SourceReservationKind =
        FTeamsterDeliveryState::ESourceReservationKind::None;
    Delivery.DestinationReservationActive = false;
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

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return false;

    auto TryFindConsumerSource = [&](
        EResourceType ResourceType,
        const std::string& ConsumerName,
        std::string& OutSourceName,
        int& OutAvailableAmount,
        float& OutSourceDistSq) -> bool
    {
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
                (!Building->SupportsTeamsterPickup() ||
                 Building->GetProducedResourceType() != ResourceType))
            {
                return;
            }

            if (RequireHarbor && !Building->IsHarbor())
                return;

            const int AvailableAmount =
                ResolveConsumerSupplyAmount(Building, ResourceType);

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
    int BestCurrentStock = GTeamsterConsumerRestockThreshold + 1;
    int BestRequestedAmount = 0;
    float BestConsumerDistSq = FLT_MAX;
    float BestSourceDistSq = FLT_MAX;

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        auto Building = BuildingList[i].lock();
        EResourceType ConsumerType = EResourceType::None;
        int CurrentStock = 0;

        if (!TryResolveConsumerNeed(Building, ConsumerType, CurrentStock) ||
            !IsWithinTeamsterCoverage(OfficeBuilding, Building))
        {
            continue;
        }

        float ConsumerDistSq = FLT_MAX;

        if (!TryGetCoverageDistanceSq(OfficeBuilding, Building, ConsumerDistSq))
            continue;

        std::string SourceName;
        int AvailableAmount = 0;
        float SourceDistSq = FLT_MAX;

        if (!TryFindConsumerSource(
                ConsumerType,
                Building->GetName(),
                SourceName,
                AvailableAmount,
                SourceDistSq))
        {
            continue;
        }

        int RequestedAmount = (std::max)(
            1,
            GTeamsterConsumerTargetStock - CurrentStock);
        RequestedAmount = (std::min)(RequestedAmount, GTeamsterTransferUnit);
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
            BestResourceType = ConsumerType;
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
        return false;
    }

    auto SourceBuilding =
        World->FindObject<CPlacementAreaObject>(BestSourceName).lock();
    auto ConsumerBuilding =
        World->FindObject<CPlacementAreaObject>(BestConsumerName).lock();

    if (!SourceBuilding ||
        !ConsumerBuilding ||
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
        SourceBuilding->ReleaseTeamsterPickup(
            BestResourceType,
            BestRequestedAmount);
        return false;
    }

    OutDelivery.Mode = FTeamsterDeliveryState::ERouteMode::ConsumerDelivery;
    OutDelivery.SourceReservationKind =
        FTeamsterDeliveryState::ESourceReservationKind::Typed;
    OutDelivery.SourceName = BestSourceName;
    OutDelivery.DestinationName = BestConsumerName;
    OutDelivery.RequestedType = BestResourceType;
    OutDelivery.RequestedAmount = BestRequestedAmount;
    OutDelivery.DestinationReservationActive = true;
    return true;
}

bool CBuildingMarkerOrb::TryPlanTeamsterExportDelivery(
    const std::shared_ptr<CPlacementAreaObject>& OfficeBuilding,
    FTeamsterDeliveryState& OutDelivery) const
{
    OutDelivery.ClearRoute();

    if (!OfficeBuilding)
        return false;

    const std::string SourceName = FindTeamsterSourceName();

    if (SourceName.empty())
        return false;

    auto World = mWorld.lock();

    if (!World)
        return false;

    auto SourceBuilding =
        World->FindObject<CPlacementAreaObject>(SourceName).lock();

    EResourceType CargoType = EResourceType::None;

    if (!SourceBuilding ||
        !SourceBuilding->TryGetExportableResourceTypeForAmount(
            GTeamsterTransferUnit,
            CargoType) ||
        CargoType == EResourceType::None)
    {
        return false;
    }

    const std::string DropoffName =
        FindTeamsterExportDropoffName(SourceName, CargoType);

    if (DropoffName.empty())
        return false;

    auto DropoffBuilding =
        World->FindObject<CPlacementAreaObject>(DropoffName).lock();

    if (!SourceBuilding->ReserveTeamsterPickup(
            CargoType,
            GTeamsterTransferUnit))
    {
        return false;
    }

    if (DropoffBuilding &&
        DropoffBuilding->IsWarehouse() &&
        !DropoffBuilding->ReserveIncomingResource(
            CargoType,
            GTeamsterTransferUnit))
    {
        SourceBuilding->ReleaseTeamsterPickup(
            CargoType,
            GTeamsterTransferUnit);
        return false;
    }

    OutDelivery.Mode = FTeamsterDeliveryState::ERouteMode::Export;
    OutDelivery.SourceReservationKind =
        FTeamsterDeliveryState::ESourceReservationKind::Typed;
    OutDelivery.SourceName = SourceName;
    OutDelivery.DestinationName = DropoffName;
    OutDelivery.RequestedAmount = GTeamsterTransferUnit;
    OutDelivery.RequestedType = CargoType;
    OutDelivery.DestinationReservationActive =
        DropoffBuilding && DropoffBuilding->IsWarehouse();
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

            if (Stock < GTeamsterTransferUnit ||
                !Building->TryGetExportableResourceTypeForAmount(
                    GTeamsterTransferUnit,
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
            !Building->IsHarbor())
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
    EResourceType CargoType) const
{
    auto World = mWorld.lock();

    if (!World ||
        mWorkName.empty() ||
        SourceName.empty() ||
        CargoType == EResourceType::None)
    {
        return std::string();
    }

    auto OfficeBuilding =
        World->FindObject<CPlacementAreaObject>(mWorkName).lock();

    if (!OfficeBuilding)
        return std::string();

    auto SourceBuilding =
        World->FindObject<CPlacementAreaObject>(SourceName).lock();

    if (SourceBuilding && !SourceBuilding->IsWarehouse())
    {
        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        {
            std::string BestWarehouseName;
            float BestDistSq = FLT_MAX;

            for (size_t i = 0; i < BuildingList.size(); ++i)
            {
                auto Building = BuildingList[i].lock();

                if (!IsOperationalBuilding(Building) ||
                    !Building->IsWarehouse() ||
                    Building->GetName() == SourceName ||
                    !Building->CanStoreResourceType(CargoType) ||
                    Building->GetAvailableIncomingCapacity(CargoType) <
                        GTeamsterTransferUnit)
                {
                    continue;
                }

                float DistSq = FLT_MAX;

                if (!TryGetCoverageDistanceSq(
                        OfficeBuilding,
                        Building,
                        DistSq) ||
                    !IsWithinTeamsterCoverage(OfficeBuilding, Building))
                {
                    continue;
                }

                if (BestWarehouseName.empty() || DistSq < BestDistSq)
                {
                    BestWarehouseName = Building->GetName();
                    BestDistSq = DistSq;
                }
            }

            if (!BestWarehouseName.empty())
                return BestWarehouseName;
        }
    }

    return FindHarborName();
}
