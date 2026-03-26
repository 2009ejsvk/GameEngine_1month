#include "BuildingMarkerOrb.h"
#include "PlacementAreaObject.h"
#include "../ObjectNames.h"
#include "Component/CameraComponent.h"
#include "Component/MeshComponent.h"
#include "Component/ObjectMovementComponent.h"
#include "Object/TileMapObject.h"
#include "World/CameraManager.h"
#include "World/World.h"
#include "../StringUtils.h"
#include "Device.h"
#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>

namespace
{
#ifdef _DEBUG
    void DebugOrbLog(const char* Format, ...)
    {
        char Text[512] = {};

        va_list Args;
        va_start(Args, Format);
        vsprintf_s(Text, Format, Args);
        va_end(Args);

        OutputDebugStringA(Text);
    }

    std::string DebugTeamsterResourceName(EResourceType Type)
    {
        return Type == EResourceType::None ?
            std::string("None") :
            StringUtils::WideToUtf8(GetResourceTypeDisplayName(Type));
    }

    const char* DebugTeamsterDropoffKind(
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
}

void CBuildingMarkerOrb::CancelCurrentPath()
{
    auto Movement = mMovement.lock();

    if (!Movement)
        return;

    Movement->AdvancePathRequestId();
    Movement->StartPathPoint();
    Movement->SetPathTargetObjectName("");
    mHasLockedTarget   = false;
    mWaitingForPath    = false;
    mWaitingPathAccum  = 0.f;
}

ECitizenState CBuildingMarkerOrb::NormalizeResumeState(
    ECitizenState State) const
{
    switch (State)
    {
    case ECitizenState::GoingHome:
    case ECitizenState::AtHome:
        return ECitizenState::GoingHome;
    case ECitizenState::GoingToWork:
    case ECitizenState::AtWork:
        return ECitizenState::GoingToWork;
    default:
        return ECitizenState::GoingToWork;
    }
}

ECitizenState CBuildingMarkerOrb::ResolveStateAfterService() const
{
    if (mHomeName.empty() || mWorkName.empty() || mFoodName.empty())
        return ECitizenState::Wander;

    const ECitizenState ResumeState =
        NormalizeResumeState(mResumeStateAfterService);

    if (ResumeState == ECitizenState::GoingHome && !mHomeName.empty())
        return ECitizenState::GoingHome;

    if (!mWorkName.empty())
        return ECitizenState::GoingToWork;

    return ECitizenState::GoingHome;
}

bool CBuildingMarkerOrb::TryInterruptByNeed()
{
    const auto& Satisfaction = mCitizenProfileState.Satisfaction;

    if (mCitizenState == ECitizenState::Wander)
        return false;

    if (IsTeamsterState(mCitizenState))
        return false;

    const bool IsFoodState =
        mCitizenState == ECitizenState::GoingToFood ||
        mCitizenState == ECitizenState::AtFood;
    const bool IsFunState =
        mCitizenState == ECitizenState::GoingToFun ||
        mCitizenState == ECitizenState::AtFun;
    const bool IsHealthState =
        mCitizenState == ECitizenState::GoingToHealth ||
        mCitizenState == ECitizenState::AtHealth;
    const bool IsFaithState =
        mCitizenState == ECitizenState::GoingToFaith ||
        mCitizenState == ECitizenState::AtFaith;

    if (!IsFoodState &&
        !mFoodName.empty() &&
        Satisfaction.Food <= GameConstants::Orb::FoodInterruptThreshold)
    {
        mResumeStateAfterService = NormalizeResumeState(mCitizenState);
        CancelCurrentPath();
        TransitionFsm(ECitizenState::GoingToFood);
        mPathRetryAccum = 0.f;
        return true;
    }

    if (!IsHealthState &&
        !IsFoodState &&
        !mHealthName.empty() &&
        Satisfaction.Health <=
            GameConstants::Orb::HealthInterruptThreshold)
    {
        mResumeStateAfterService = NormalizeResumeState(mCitizenState);
        CancelCurrentPath();
        TransitionFsm(ECitizenState::GoingToHealth);
        mPathRetryAccum = 0.f;
        return true;
    }

    if (!IsFaithState &&
        !IsFoodState &&
        !IsHealthState &&
        !mFaithName.empty() &&
        Satisfaction.Faith <=
            GameConstants::Orb::FaithInterruptThreshold)
    {
        mResumeStateAfterService = NormalizeResumeState(mCitizenState);
        CancelCurrentPath();
        TransitionFsm(ECitizenState::GoingToFaith);
        mPathRetryAccum = 0.f;
        return true;
    }

    if (!IsFunState &&
        !IsFoodState &&
        !IsHealthState &&
        !IsFaithState &&
        !mFunName.empty() &&
        Satisfaction.Fun <= GameConstants::Orb::FunInterruptThreshold)
    {
        mResumeStateAfterService = NormalizeResumeState(mCitizenState);
        CancelCurrentPath();
        TransitionFsm(ECitizenState::GoingToFun);
        mPathRetryAccum = 0.f;
        return true;
    }

    return false;
}

std::string CBuildingMarkerOrb::ResolveTargetByState() const
{
    const auto& Delivery = mTeamsterDeliveryState;

    switch (mCitizenState)
    {
    case ECitizenState::GoingToWork:
        return mWorkName;
    case ECitizenState::GoingHome:
        return mHomeName;
    case ECitizenState::GoingToFood:
        return mFoodName;
    case ECitizenState::GoingToFun:
        return mFunName;
    case ECitizenState::GoingToHealth:
        return mHealthName;
    case ECitizenState::GoingToFaith:
        return mFaithName;
    case ECitizenState::GoingToTeamsterSource:
        return Delivery.SourceName;
    case ECitizenState::GoingToTeamsterHarbor:
        return Delivery.DestinationName;
    case ECitizenState::GoingToTeamsterConsumerSource:
        return Delivery.SourceName;
    case ECitizenState::GoingToTeamsterConsumerTarget:
        return Delivery.DestinationName;
    case ECitizenState::GoingToTeamsterOffice:
        return mWorkName;
    default:
        return PickRandomTargetName(std::string());
    }
}

bool CBuildingMarkerOrb::TickDwellState(float DeltaTime)
{
    const bool IsDwelling =
        mCitizenState == ECitizenState::AtWork  ||
        mCitizenState == ECitizenState::AtHome  ||
        mCitizenState == ECitizenState::AtFood  ||
        mCitizenState == ECitizenState::AtFun   ||
        mCitizenState == ECitizenState::AtHealth ||
        mCitizenState == ECitizenState::AtFaith;

    if (!IsDwelling)
        return false;

    if (mCitizenState == ECitizenState::AtWork &&
        TryStartTeamsterDelivery())
    {
        return true;
    }

    if ((mCitizenState == ECitizenState::AtWork ||
         mCitizenState == ECitizenState::AtHome) &&
        TryInterruptByNeed())
    {
        return true;
    }

    mDwellTimer -= DeltaTime;

    if (mDwellTimer <= 0.f)
    {
        CancelCurrentPath();

        if (mCitizenState == ECitizenState::AtWork)
            TransitionFsm(ECitizenState::GoingHome);
        else if (mCitizenState == ECitizenState::AtHome)
            TransitionFsm(ECitizenState::GoingToWork);
        else
            TransitionFsm(ResolveStateAfterService());
    }

    return true;
}

bool CBuildingMarkerOrb::TickInitialPosition(float CurrentZ)
{
    std::vector<std::pair<std::string, FVector3>> MarkerList;

    if (!CollectTargetMarkers(MarkerList))
    {
        FVector3 FallbackStartPos = FVector3::Zero;

        if (TryGetFallbackStartPos(FallbackStartPos))
        {
            SetWorldPos(FallbackStartPos);
            mCurrentTargetName.clear();
            mHasStartPos = true;
#ifdef _DEBUG
            mDebugMissingMarkerLogged = false;
#endif
            return true;
        }

#ifdef _DEBUG
        if (!mDebugMissingMarkerLogged)
        {
            DebugOrbLog(
                "[Orb] Marker unavailable. no valid target marker\n");
            mDebugMissingMarkerLogged = true;
        }
#endif
        return true;
    }

#ifdef _DEBUG
    mDebugMissingMarkerLogged = false;
#endif

    for (size_t i = 0; i < MarkerList.size(); ++i)
        MarkerList[i].second.z = CurrentZ;

    const int StartIndex = rand() % (int)MarkerList.size();
    const std::string& StartName = MarkerList[StartIndex].first;
    SetWorldPos(MarkerList[StartIndex].second);

    mCurrentTargetName = (mCitizenState == ECitizenState::Wander)
        ? PickRandomTargetName(StartName)
        : ResolveTargetByState();

    if (mCurrentTargetName.empty())
        mCurrentTargetName = StartName;

    mPathRetryAccum = -((float)(rand() % 20) / 100.f);

#ifdef _DEBUG
    DebugOrbLog(
        "[Orb] Start at %s marker=(%.1f, %.1f) target=%s delay=%.2f\n",
        StartName.c_str(),
        MarkerList[StartIndex].second.x,
        MarkerList[StartIndex].second.y,
        mCurrentTargetName.c_str(),
        -mPathRetryAccum);
#endif

    mHasStartPos = true;
    return true;
}

void CBuildingMarkerOrb::HandleMissingTarget()
{
    auto World = mWorld.lock();
    auto& Delivery = mTeamsterDeliveryState;
    const bool MissingDeliveryTargetWithCargo =
        World &&
        Delivery.HasLoadedCargo() &&
        (mCitizenState == ECitizenState::GoingToTeamsterHarbor ||
         mCitizenState == ECitizenState::GoingToTeamsterConsumerTarget);

    CancelCurrentPath();
    ReleaseTeamsterReservations();

    if (MissingDeliveryTargetWithCargo)
    {
        const std::string RecoveryName =
            FindTeamsterExportDropoffName(
                Delivery.SourceName,
                Delivery.CargoType,
                Delivery.CarryAmount);

        if (!RecoveryName.empty() && RecoveryName != mCurrentTargetName)
        {
            auto RecoveryBuilding =
                World->FindObject<CPlacementAreaObject>(RecoveryName).lock();
            const bool NeedsReservation =
                RecoveryBuilding && RecoveryBuilding->IsWarehouse();

            if (RecoveryBuilding &&
                (!NeedsReservation ||
                 RecoveryBuilding->ReserveIncomingResource(
                    Delivery.CargoType,
                    Delivery.CarryAmount)))
            {
                Delivery.DestinationName = RecoveryName;
                if (NeedsReservation)
                {
                    Delivery.SetDestinationReservation(
                        Delivery.CargoType,
                        Delivery.CarryAmount);
                }
                else
                {
                    Delivery.ClearDestinationReservation();
                }
#ifdef _DEBUG
                if (Delivery.Mode == FTeamsterDeliveryState::ERouteMode::Export)
                {
                    DebugOrbLog(
                        "[Teamster][RecoverExport] office=%s missing=%s reroute=%s "
                        "dropoff_kind=%s resource=%s amount=%d\n",
                        mWorkName.c_str(),
                        mCurrentTargetName.c_str(),
                        RecoveryName.c_str(),
                        DebugTeamsterDropoffKind(RecoveryBuilding),
                        DebugTeamsterResourceName(Delivery.CargoType).c_str(),
                        Delivery.CarryAmount);
                }
#endif
                ResetTeamsterSpeed();
                TransitionFsm(ECitizenState::GoingToTeamsterHarbor);
                RequestMoveTo(RecoveryName);
                mPathRetryAccum = 0.f;
                return;
            }
        }

        if (!Delivery.SourceName.empty())
        {
            auto SourceBuilding =
                World->FindObject<CPlacementAreaObject>(
                    Delivery.SourceName).lock();

            if (SourceBuilding &&
                SourceBuilding->TryAddResourceStock(
                    Delivery.CargoType,
                    Delivery.CarryAmount))
            {
#ifdef _DEBUG
                if (Delivery.Mode == FTeamsterDeliveryState::ERouteMode::Export)
                {
                    DebugOrbLog(
                        "[Teamster][RecoverExportToSource] office=%s source=%s "
                        "resource=%s amount=%d stock_after=%d\n",
                        mWorkName.c_str(),
                        Delivery.SourceName.c_str(),
                        DebugTeamsterResourceName(Delivery.CargoType).c_str(),
                        Delivery.CarryAmount,
                        SourceBuilding->GetResourceStock(Delivery.CargoType));
                }
#endif
                Delivery.ClearCargo();
                ResetTeamsterSpeed();
                TransitionFsm(ECitizenState::GoingToTeamsterOffice);
                RequestMoveTo(mWorkName);
                mPathRetryAccum = 0.f;
                return;
            }
        }
    }

    switch (mCitizenState)
    {
    case ECitizenState::GoingToWork:
    case ECitizenState::AtWork:
        mWorkName.clear();
        Delivery.ClearRoute();
        ResetTeamsterSpeed();
        break;
    case ECitizenState::GoingHome:
    case ECitizenState::AtHome:
        mHomeName.clear();
        break;
    case ECitizenState::GoingToFood:
    case ECitizenState::AtFood:
        mFoodName.clear();
        break;
    case ECitizenState::GoingToFun:
    case ECitizenState::AtFun:
        mFunName.clear();
        break;
    case ECitizenState::GoingToHealth:
    case ECitizenState::AtHealth:
        mHealthName.clear();
        break;
    case ECitizenState::GoingToFaith:
    case ECitizenState::AtFaith:
        mFaithName.clear();
        break;
    case ECitizenState::GoingToTeamsterSource:
        Delivery.SourceName.clear();
        Delivery.ClearCargo();
        ResetTeamsterSpeed();
        break;
    case ECitizenState::GoingToTeamsterHarbor:
        Delivery.DestinationName.clear();
        Delivery.ClearCargo();
        ResetTeamsterSpeed();
        break;
    case ECitizenState::GoingToTeamsterConsumerSource:
        Delivery.SourceName.clear();
        Delivery.ClearCargo();
        ResetTeamsterSpeed();
        break;
    case ECitizenState::GoingToTeamsterConsumerTarget:
        Delivery.DestinationName.clear();
        Delivery.ClearCargo();
        ResetTeamsterSpeed();
        break;
    case ECitizenState::GoingToTeamsterOffice:
        mWorkName.clear();
        Delivery.ClearRoute();
        ResetTeamsterSpeed();
        break;
    default:
        break;
    }

    if (mHomeName.empty() || mWorkName.empty() || mFoodName.empty())
    {
        TransitionFsm(ECitizenState::Wander);
    }
    else if (mCitizenState == ECitizenState::GoingToFood ||
             mCitizenState == ECitizenState::AtFood      ||
             mCitizenState == ECitizenState::GoingToFun  ||
             mCitizenState == ECitizenState::AtFun       ||
             mCitizenState == ECitizenState::GoingToHealth ||
             mCitizenState == ECitizenState::AtHealth    ||
             mCitizenState == ECitizenState::GoingToFaith ||
             mCitizenState == ECitizenState::AtFaith)
    {
        TransitionFsm(ResolveStateAfterService());
    }
    else if (IsTeamsterState(mCitizenState))
    {
        ResetTeamsterSpeed();
        TransitionFsm(ECitizenState::GoingToWork);
    }

    mCurrentTargetName = (mCitizenState == ECitizenState::Wander)
        ? PickRandomTargetName(std::string())
        : ResolveTargetByState();

    if (mCurrentTargetName.empty())
        return;

    RequestMoveTo(mCurrentTargetName);
    mPathRetryAccum = 0.f;
}

void CBuildingMarkerOrb::HandleArrival(float Dist)
{
    auto World = mWorld.lock();
    auto& Delivery = mTeamsterDeliveryState;
    auto ResolveTransportCargoAmount =
        [&](int RequestedAmount) -> int
    {
        if (!World || RequestedAmount <= 0 || mWorkName.empty())
            return (std::max)(0, RequestedAmount);

        auto OfficeBuilding =
            World->FindObject<CPlacementAreaObject>(mWorkName).lock();

        if (!OfficeBuilding)
            return (std::max)(0, RequestedAmount);

        const int LossPercent =
            OfficeBuilding->GetTeamsterCargoLossPercent();

        if (LossPercent <= 0 || RequestedAmount <= 1)
            return (std::max)(1, RequestedAmount);

        const int LossAmount = (std::min)(
            RequestedAmount - 1,
            static_cast<int>(
                static_cast<long long>(RequestedAmount) * LossPercent / 100ll));
        return (std::max)(1, RequestedAmount - LossAmount);
    };

    auto TryRedirectCargoToStorage =
        [&](const std::string& ExcludedDestinationName) -> bool
    {
        if (!World ||
            !Delivery.HasLoadedCargo())
        {
            return false;
        }

        const std::string RecoveryName =
            FindTeamsterExportDropoffName(
                Delivery.SourceName,
                Delivery.CargoType,
                Delivery.CarryAmount);

        if (RecoveryName.empty() ||
            RecoveryName == ExcludedDestinationName)
        {
            return false;
        }

        auto RecoveryBuilding =
            World->FindObject<CPlacementAreaObject>(RecoveryName).lock();

        if (!RecoveryBuilding)
            return false;

        const bool NeedsReservation = RecoveryBuilding->IsWarehouse();

        if (NeedsReservation &&
            !RecoveryBuilding->ReserveIncomingResource(
                Delivery.CargoType,
                Delivery.CarryAmount))
        {
            return false;
        }

        Delivery.DestinationName = RecoveryName;
        if (NeedsReservation)
        {
            Delivery.SetDestinationReservation(
                Delivery.CargoType,
                Delivery.CarryAmount);
        }
        else
        {
            Delivery.ClearDestinationReservation();
        }
#ifdef _DEBUG
        if (Delivery.Mode == FTeamsterDeliveryState::ERouteMode::Export)
        {
            DebugOrbLog(
                "[Teamster][RerouteExport] office=%s from=%s to=%s "
                "dropoff_kind=%s resource=%s amount=%d\n",
                mWorkName.c_str(),
                ExcludedDestinationName.c_str(),
                RecoveryName.c_str(),
                DebugTeamsterDropoffKind(RecoveryBuilding),
                DebugTeamsterResourceName(Delivery.CargoType).c_str(),
                Delivery.CarryAmount);
        }
#endif
        TransitionFsm(ECitizenState::GoingToTeamsterHarbor);
        mPathRetryAccum = -((float)(rand() % 10) / 100.f);
        return true;
    };

    auto TryRestoreCargoToSource = [&]() -> bool
    {
        if (!World ||
            Delivery.CarryAmount <= 0 ||
            Delivery.CargoType == EResourceType::None ||
            Delivery.SourceName.empty())
        {
            return false;
        }

        auto SourceBuilding =
            World->FindObject<CPlacementAreaObject>(Delivery.SourceName).lock();

        if (!SourceBuilding ||
            !SourceBuilding->TryAddResourceStock(
                Delivery.CargoType,
                Delivery.CarryAmount))
        {
            return false;
        }

#ifdef _DEBUG
        if (Delivery.Mode == FTeamsterDeliveryState::ERouteMode::Export)
        {
            DebugOrbLog(
                "[Teamster][RestoreExport] office=%s source=%s resource=%s "
                "amount=%d stock_after=%d\n",
                mWorkName.c_str(),
                Delivery.SourceName.c_str(),
                DebugTeamsterResourceName(Delivery.CargoType).c_str(),
                Delivery.CarryAmount,
                SourceBuilding->GetResourceStock(Delivery.CargoType));
        }
#endif

        Delivery.ClearCargo();
        return true;
    };

    CancelCurrentPath();

    if (mCitizenState == ECitizenState::GoingToWork)
    {
        TransitionFsm(ECitizenState::AtWork);
    }
    else if (mCitizenState == ECitizenState::GoingHome)
    {
        TransitionFsm(ECitizenState::AtHome);
    }
    else if (mCitizenState == ECitizenState::GoingToFood)
    {
        TransitionFsm(ECitizenState::AtFood);
    }
    else if (mCitizenState == ECitizenState::GoingToFun)
    {
        TransitionFsm(ECitizenState::AtFun);
    }
    else if (mCitizenState == ECitizenState::GoingToHealth)
    {
        TransitionFsm(ECitizenState::AtHealth);
    }
    else if (mCitizenState == ECitizenState::GoingToFaith)
    {
        TransitionFsm(ECitizenState::AtFaith);
    }
    else if (mCitizenState == ECitizenState::GoingToTeamsterSource)
    {
        bool LoadedCargo = false;

        if (World && !mCurrentTargetName.empty())
        {
            auto SourceBuilding =
                World->FindObject<CPlacementAreaObject>(
                    mCurrentTargetName).lock();

            if (Delivery.SourceReservationKind ==
                FTeamsterDeliveryState::ESourceReservationKind::Typed &&
                SourceBuilding &&
                Delivery.HasPendingPickupRequest())
            {
                SourceBuilding->ReleaseTeamsterPickup(
                    Delivery.RequestedType,
                    Delivery.RequestedAmount);
                Delivery.SourceReservationKind =
                    FTeamsterDeliveryState::ESourceReservationKind::None;
            }

            if (SourceBuilding &&
                (SourceBuilding->SupportsTeamsterPickup() ||
                 SourceBuilding->IsWarehouse()) &&
                Delivery.HasPendingPickupRequest() &&
                SourceBuilding->TryConsumeResource(
                    Delivery.RequestedType,
                    (std::max)(1, Delivery.RequestedAmount)))
            {
                Delivery.MarkCargoLoadedFromRequest(
                    ResolveTransportCargoAmount(
                        (std::max)(1, Delivery.RequestedAmount)));
                LoadedCargo = true;
            }
        }

        if (LoadedCargo)
        {
            if (Delivery.DestinationName.empty())
            {
                Delivery.DestinationName =
                    FindTeamsterExportDropoffName(
                        mCurrentTargetName,
                        Delivery.CargoType,
                        Delivery.CarryAmount);
            }

            bool HasValidDestination = !Delivery.DestinationName.empty();

            if (HasValidDestination &&
                !Delivery.HasDestinationReservation())
            {
                auto DestinationBuilding =
                    World->FindObject<CPlacementAreaObject>(
                        Delivery.DestinationName).lock();

                if (!DestinationBuilding)
                {
                    HasValidDestination = false;
                    Delivery.DestinationName.clear();
                }
                else if (DestinationBuilding->IsWarehouse())
                {
                    HasValidDestination =
                        DestinationBuilding->ReserveIncomingResource(
                            Delivery.CargoType,
                            Delivery.CarryAmount);
                    if (HasValidDestination)
                    {
                        Delivery.SetDestinationReservation(
                            Delivery.CargoType,
                            Delivery.CarryAmount);
                    }
                    else
                    {
                        Delivery.ClearDestinationReservation();
                    }

                    if (!HasValidDestination)
                        Delivery.DestinationName.clear();
                }
            }

            #ifdef _DEBUG
            if (Delivery.Mode == FTeamsterDeliveryState::ERouteMode::Export)
            {
                DebugOrbLog(
                    "[Teamster][PickupExport] office=%s source=%s resource=%s "
                    "amount=%d next=%s\n",
                    mWorkName.c_str(),
                    mCurrentTargetName.c_str(),
                    DebugTeamsterResourceName(Delivery.CargoType).c_str(),
                    Delivery.CarryAmount,
                    Delivery.DestinationName.c_str());
            }
            #endif

            if (HasValidDestination)
                TransitionFsm(ECitizenState::GoingToTeamsterHarbor);
            else if (TryRedirectCargoToStorage(std::string()))
                return;
            else if (TryRestoreCargoToSource())
                TransitionFsm(ECitizenState::GoingToTeamsterOffice);
            else
            {
                Delivery.ClearCargo();
                TransitionFsm(ECitizenState::GoingToTeamsterOffice);
            }
        }
        else
        {
            Delivery.ClearCargo();
            TransitionFsm(ECitizenState::GoingToTeamsterOffice);
        }
    }
    else if (mCitizenState == ECitizenState::GoingToTeamsterHarbor)
    {
        if (World &&
            Delivery.HasDestinationReservation() &&
            Delivery.HasLoadedCargo() &&
            !mCurrentTargetName.empty())
        {
            auto ReservedTarget =
                World->FindObject<CPlacementAreaObject>(
                    mCurrentTargetName).lock();

            if (ReservedTarget)
            {
                ReservedTarget->ReleaseIncomingResource(
                    Delivery.ReservedDestinationType,
                    Delivery.ReservedDestinationAmount);
            }

            Delivery.ClearDestinationReservation();
        }

        if (World &&
            Delivery.HasLoadedCargo() &&
            !mCurrentTargetName.empty())
        {
            auto DestinationBuilding =
                World->FindObject<CPlacementAreaObject>(
                    mCurrentTargetName).lock();

            if (DestinationBuilding &&
                Delivery.CargoType != EResourceType::None)
            {
                if (DestinationBuilding->TryAddResourceStock(
                        Delivery.CargoType,
                        Delivery.CarryAmount))
                {
#ifdef _DEBUG
                    if (Delivery.Mode == FTeamsterDeliveryState::ERouteMode::Export)
                    {
                        DebugOrbLog(
                            "[Teamster][DropoffExport] office=%s destination=%s "
                            "dropoff_kind=%s resource=%s amount=%d stock_after=%d\n",
                            mWorkName.c_str(),
                            mCurrentTargetName.c_str(),
                            DebugTeamsterDropoffKind(DestinationBuilding),
                            DebugTeamsterResourceName(Delivery.CargoType).c_str(),
                            Delivery.CarryAmount,
                            DestinationBuilding->GetResourceStock(
                                Delivery.CargoType));
                    }
#endif
                    Delivery.ClearCargo();
                    TransitionFsm(ECitizenState::GoingToTeamsterOffice);
                    mPathRetryAccum = -((float)(rand() % 10) / 100.f);
                    return;
                }
                else if (TryRedirectCargoToStorage(mCurrentTargetName))
                {
                    return;
                }
            }
        }

        if (TryRestoreCargoToSource())
        {
            TransitionFsm(ECitizenState::GoingToTeamsterOffice);
            mPathRetryAccum = -((float)(rand() % 10) / 100.f);
            return;
        }

        Delivery.ClearCargo();
        TransitionFsm(ECitizenState::GoingToTeamsterOffice);
    }
    else if (mCitizenState == ECitizenState::GoingToTeamsterConsumerSource)
    {
        bool LoadedCargo = false;

        if (World &&
            Delivery.HasPendingPickupRequest() &&
            !mCurrentTargetName.empty())
        {
            auto SourceBuilding =
                World->FindObject<CPlacementAreaObject>(
                    mCurrentTargetName).lock();

            if (SourceBuilding)
            {
                if (Delivery.SourceReservationKind ==
                    FTeamsterDeliveryState::ESourceReservationKind::Typed)
                {
                    SourceBuilding->ReleaseTeamsterPickup(
                        Delivery.RequestedType,
                        Delivery.RequestedAmount);
                    Delivery.SourceReservationKind =
                        FTeamsterDeliveryState::ESourceReservationKind::None;
                }

                const int AvailableAmount =
                    SourceBuilding->GetResourceStock(Delivery.RequestedType);
                const int RequestedAmount =
                    (std::max)(1, Delivery.RequestedAmount);
                const int PickupAmount =
                    (std::min)(RequestedAmount, AvailableAmount);

                if (PickupAmount > 0 &&
                    SourceBuilding->TryConsumeResource(
                        Delivery.RequestedType,
                        PickupAmount))
                {
                    Delivery.MarkCargoLoadedFromRequest(
                        ResolveTransportCargoAmount(PickupAmount));
                    LoadedCargo = true;
                }
            }
        }

        if (LoadedCargo && !Delivery.DestinationName.empty())
        {
            TransitionFsm(ECitizenState::GoingToTeamsterConsumerTarget);
        }
        else
        {
            // Pickup failed — release the consumer building's incoming
            // reservation before returning to the office.
            if (World &&
                Delivery.HasDestinationReservation() &&
                !Delivery.DestinationName.empty())
            {
                auto DestBuilding =
                    World->FindObject<CPlacementAreaObject>(
                        Delivery.DestinationName).lock();

                if (DestBuilding)
                {
                    DestBuilding->ReleaseIncomingResource(
                        Delivery.ReservedDestinationType,
                        Delivery.ReservedDestinationAmount);
                }
            }

            TransitionFsm(ECitizenState::GoingToTeamsterOffice);
        }
    }
    else if (mCitizenState == ECitizenState::GoingToTeamsterConsumerTarget)
    {
        if (World &&
            Delivery.HasDestinationReservation() &&
            !mCurrentTargetName.empty())
        {
            auto ReservedTarget =
                World->FindObject<CPlacementAreaObject>(
                    mCurrentTargetName).lock();

            if (ReservedTarget)
            {
                ReservedTarget->ReleaseIncomingResource(
                    Delivery.ReservedDestinationType,
                    Delivery.ReservedDestinationAmount);
            }

            Delivery.ClearDestinationReservation();
        }

        bool Delivered = false;

        if (World &&
            Delivery.HasLoadedCargo() &&
            !mCurrentTargetName.empty())
        {
            auto ConsumerBuilding =
                World->FindObject<CPlacementAreaObject>(
                    mCurrentTargetName).lock();

            if (ConsumerBuilding &&
                (ConsumerBuilding->GetVisitConsumptionResourceType() ==
                    Delivery.CargoType ||
                 ConsumerBuilding->UsesProductionInputResource(
                    Delivery.CargoType)) &&
                ConsumerBuilding->TryAddResourceStock(
                    Delivery.CargoType,
                    Delivery.CarryAmount))
            {
                Delivered = true;
            }
        }

        if (!Delivered &&
            TryRedirectCargoToStorage(mCurrentTargetName))
        {
            return;
        }

        if (!Delivered)
            TryRestoreCargoToSource();

        Delivery.ClearCargo();

        if (!TryRerouteTeamsterAfterDelivery())
            TransitionFsm(ECitizenState::GoingToTeamsterOffice);
    }
    else if (mCitizenState == ECitizenState::GoingToTeamsterOffice)
    {
        ResetTeamsterSpeed();
        Delivery.ClearCargo();
        TransitionFsm(ECitizenState::AtWork);
    }
    else
    {
        // Wander 모드: 랜덤 타겟 선택
        mCurrentTargetName = PickRandomTargetName(mCurrentTargetName);

        if (mCurrentTargetName.empty())
            return;

#ifdef _DEBUG
        DebugOrbLog(
            "[Orb] Arrived. switch target=%s dist=%.2f arrival=%.2f\n",
            mCurrentTargetName.c_str(),
            Dist, mArrivalDistance);
#endif
    }

    // 도착 후 짧은 대기로 정지 체감 감소
    mPathRetryAccum = -((float)(rand() % 10) / 100.f);
}

void CBuildingMarkerOrb::Update(float DeltaTime)
{
    CGameObject::Update(DeltaTime);

    UpdateSatisfaction(DeltaTime);
    UpdatePoliticalProfile(DeltaTime);

    if (mCitizenProfileState.Satisfaction.Health <=
        GameConstants::Orb::HealthRemovalThreshold)
    {
        Destroy();
        return;
    }

    RefreshBuildings();
    UpdateScaleFromTileSize();

    auto World    = mWorld.lock();
    auto Movement = mMovement.lock();

    if (!World || !Movement)
    {
#ifdef _DEBUG
        if (!mDebugMissingDependencyLogged)
        {
            DebugOrbLog(
                "[Orb] Missing dependency World=%d Move=%d\n",
                World ? 1 : 0,
                Movement ? 1 : 0);
            mDebugMissingDependencyLogged = true;
        }
#endif
        return;
    }

#ifdef _DEBUG
    mDebugMissingDependencyLogged = false;
#endif

    // 화면 바깥 오브 렌더링 컬링
    auto Mesh = mMeshComponent.lock();

    if (Mesh)
    {
        auto CameraMgr = World->GetCameraManager().lock();

        if (!CameraMgr)
        {
            Mesh->SetEnable(false);
            return;
        }

        const FVector3 CamPos = CameraMgr->GetMainCameraWorldPos();
        float ViewWidth  = (float)CDevice::GetInst()->GetResolution().Width;
        float ViewHeight = (float)CDevice::GetInst()->GetResolution().Height;

        auto MainCamera = CameraMgr->GetMainCamera().lock();

        if (MainCamera &&
            MainCamera->GetProjectionType() ==
            ECameraProjectionType::Ortho)
        {
            ViewWidth  = MainCamera->GetViewWidth();
            ViewHeight = MainCamera->GetViewHeight();
        }

        const float Margin = (std::max)(mOrbDiameter * 2.f, 32.f);
        const FVector3 Pos = GetWorldPos();
        const bool Visible =
            fabsf(Pos.x - CamPos.x) <= (ViewWidth  * 0.5f + Margin) &&
            fabsf(Pos.y - CamPos.y) <= (ViewHeight * 0.5f + Margin);

        Mesh->SetEnable(Visible);
    }

    const float CurrentZ = GetWorldPos().z;

    {
        const float EffectiveSpeed =
            World->IsSimulationPaused()
            ? 0.f
            : mMoveSpeed * static_cast<float>(
                (std::max)(1, World->GetSimulationSpeedMultiplier()));
        Movement->SetSpeed(EffectiveSpeed);
    }
    mPathRetryAccum += DeltaTime;

    // 체류 상태 (At* 상태): 타이머 소모 후 다음 상태 전환
    if (TickDwellState(DeltaTime))
        return;

    TryInterruptByNeed();

    // 첫 프레임: 시작 위치 배정
    if (!mHasStartPos)
    {
        TickInitialPosition(CurrentZ);
        return;
    }

    if (mCurrentTargetName.empty())
    {
        CancelCurrentPath();
        mCurrentTargetName = ResolveTargetByState();

        if (mCurrentTargetName.empty())
            return;

        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;
        return;
    }

    auto TargetBuilding =
        World->FindObject<CPlacementAreaObject>(mCurrentTargetName).lock();

    // 목표 건물이 사라진 경우: 상태 정리 후 재경로
    if (!TargetBuilding)
    {
        HandleMissingTarget();
        return;
    }

    FVector3 TargetMarker = FVector3::Zero;

    if (!TargetBuilding->GetClosestMarkerWorldPos(
        GetWorldPos(), TargetMarker))
    {
#ifdef _DEBUG
        if (!mDebugMissingMarkerLogged)
        {
            DebugOrbLog(
                "[Orb] Marker unavailable. target=%s\n",
                mCurrentTargetName.c_str());
            mDebugMissingMarkerLogged = true;
        }
#endif
        CancelCurrentPath();

        mCurrentTargetName = (mCitizenState == ECitizenState::Wander)
            ? PickRandomTargetName(std::string())
            : ResolveTargetByState();

        if (mCurrentTargetName.empty())
            return;

        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;
        return;
    }

#ifdef _DEBUG
    mDebugMissingMarkerLogged = false;
#endif

    TargetMarker.z = CurrentZ;

    // 건물 이동 감지: 잠금 좌표와 현재 closest 마커가 멀어지면 경로 재시도
    if (mHasLockedTarget)
    {
        FVector3 ClosestToLocked;

        if (TargetBuilding->GetClosestMarkerWorldPos(
            mLockedTargetPos, ClosestToLocked))
        {
            ClosestToLocked.z = mLockedTargetPos.z;

            if (mLockedTargetPos.Distance(ClosestToLocked) > 1.f)
            {
                mHasLockedTarget = false;
                CancelCurrentPath();
                const float Jitter =
                    (float)(rand() % 100) / 100.f *
                    mPathRetryInterval * 0.2f;
                mPathRetryAccum =
                    mPathRetryInterval * 0.75f + Jitter;
            }
        }
        else
        {
            mHasLockedTarget = false;
            CancelCurrentPath();
            mPathRetryAccum = mPathRetryInterval;
        }
    }

    FVector3 ArrivalRef = mHasLockedTarget ? mLockedTargetPos : TargetMarker;
    ArrivalRef.z = CurrentZ;

    FVector3 Current = GetWorldPos();
    Current.z = CurrentZ;

    const float Dist = Current.Distance(ArrivalRef);

    bool ArrivedAtBuilding = (Dist <= mArrivalDistance);

    if (!ArrivedAtBuilding && mHasLockedTarget)
        ArrivedAtBuilding = Current.Distance(TargetMarker) <= mArrivalDistance;

    if (ArrivedAtBuilding)
    {
        HandleArrival(Dist);
        return;
    }

    // 경로 대기 상태 관리
    if (mWaitingForPath && !Movement->GetVelocity().IsZero())
    {
        mWaitingForPath   = false;
        mWaitingPathAccum = 0.f;
    }

    if (mWaitingForPath)
    {
        mWaitingPathAccum += DeltaTime;

        if (mWaitingPathAccum >= 0.35f)
        {
            mWaitingForPath   = false;
            mWaitingPathAccum = 0.f;
            mPathRetryAccum   = mPathRetryInterval;
        }
    }
    else
    {
        mWaitingPathAccum = 0.f;
    }

    const FVector3 Velocity = Movement->GetVelocity();
    UpdateSpriteAnimationFromVelocity(Velocity);

    if (!Velocity.IsZero())
    {
        mStallAccum      = 0.f;
        mLastProgressPos = Current;
    }
    else
    {
        if (Current.Distance(mLastProgressPos) > 2.f)
        {
            mStallAccum      = 0.f;
            mLastProgressPos = Current;
        }
        else
        {
            mStallAccum += DeltaTime;
        }
    }

    // 정체 해소: 일정 시간 이상 이동 없으면 타겟 또는 경로 재시도
    if (mStallAccum >= 1.5f)
    {
        CancelCurrentPath();

        if (mCitizenState != ECitizenState::Wander)
        {
            RequestMoveTo(mCurrentTargetName);
        }
        else
        {
            const std::string PrevTarget = mCurrentTargetName;
            mCurrentTargetName = PickRandomTargetName(mCurrentTargetName);

            if (mCurrentTargetName.empty())
                mCurrentTargetName = PrevTarget;

            RequestMoveTo(mCurrentTargetName);
        }

        mPathRetryAccum  = 0.f;
        mStallAccum      = 0.f;
        mLastProgressPos = Current;
        return;
    }

    const float EffectiveRetryInterval = mWaitingForPath
        ? mPathRetryInterval * 1.2f
        : mPathRetryInterval;

    if (mPathRetryAccum >= EffectiveRetryInterval &&
        Movement->GetVelocity().IsZero())
    {
        mWaitingForPath = false;
        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;
    }

#ifdef _DEBUG
    mDebugStatusLogAccum += DeltaTime;

    if (mDebugStatusLogAccum >= 1.f)
    {
        mDebugStatusLogAccum = 0.f;

        const FVector3 Pos           = GetWorldPos();
        const std::string& PathTarget = Movement->GetPathTargetObjectName();

        DebugOrbLog(
            "[Orb] Pos=(%.1f, %.1f) target=%s pathTarget=%s dist=%.1f\n",
            Pos.x, Pos.y,
            mCurrentTargetName.c_str(),
            PathTarget.empty() ? "<none>" : PathTarget.c_str(),
            Dist);
    }
#endif
}

void CBuildingMarkerOrb::RefreshBuildings()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    if (mTileMapObject.expired())
    {
        mTileMapObject = World->FindObject<CTileMapObject>(GTileMapObjectName);
    }
}

bool CBuildingMarkerOrb::TryGetFallbackStartPos(FVector3& OutPos)
{
    auto TileMapObject = mTileMapObject.lock();

    if (!TileMapObject)
        return false;

    auto TileMap = TileMapObject->GetTileMap().lock();

    if (!TileMap)
        return false;

    const int CountX = TileMap->GetTileCountX();
    const int CountY = TileMap->GetTileCountY();

    if (CountX <= 0 || CountY <= 0)
        return false;

    const int TileCount = CountX * CountY;
    int ChosenIndex = -1;
    const int MaxAttempts = (std::min)(TileCount, 64);

    for (int Attempt = 0; Attempt < MaxAttempts; ++Attempt)
    {
        const int CandidateIndex = rand() % TileCount;
        auto CandidateTile = TileMap->GetTile(CandidateIndex).lock();

        if (!CandidateTile)
            continue;

        if (CandidateTile->GetType() == ETileType::UnableToMove)
            continue;

        ChosenIndex = CandidateIndex;
        break;
    }

    if (ChosenIndex < 0)
    {
        ChosenIndex = (CountY / 2) * CountX + (CountX / 2);
    }

    auto SpawnTile = TileMap->GetTile(ChosenIndex).lock();

    if (!SpawnTile)
        return false;

    const FVector2 Center = SpawnTile->GetCenter();
    const FVector3 TileMapWorldPos = TileMapObject->GetWorldPos();
    const float CurrentZ = GetWorldPos().z;
    OutPos = FVector3(
        Center.x + TileMapWorldPos.x,
        Center.y + TileMapWorldPos.y,
        CurrentZ);

    return true;
}

void CBuildingMarkerOrb::UpdateScaleFromTileSize()
{
    if (mScaleInitialized)
        return;

    auto World = mWorld.lock();
    FVector2 TileSize;
    bool SizeFound = false;

    if (!World)
        return;

    std::vector<std::string> TargetNames;

    if (!mRandomTargetNames.empty())
    {
        TargetNames = mRandomTargetNames;
    }

    else
    {
        if (!mBuildingAName.empty())
            TargetNames.push_back(mBuildingAName);

        if (!mBuildingBName.empty() &&
            mBuildingBName != mBuildingAName)
        {
            TargetNames.push_back(mBuildingBName);
        }
    }

    std::vector<std::string> UniqueNames;

    for (size_t i = 0; i < TargetNames.size(); ++i)
    {
        const std::string& Name = TargetNames[i];

        if (Name.empty())
            continue;

        if (std::find(UniqueNames.begin(), UniqueNames.end(), Name) ==
            UniqueNames.end())
        {
            UniqueNames.push_back(Name);
        }
    }

    for (size_t i = 0; i < UniqueNames.size(); ++i)
    {
        auto Building = World->FindObject<CPlacementAreaObject>(
            UniqueNames[i]).lock();

        if (!Building)
            continue;

        if (Building->GetTileSize(TileSize))
        {
            SizeFound = true;
            break;
        }
    }

    if (!SizeFound)
        return;

    float Diameter = (std::min)(TileSize.x, TileSize.y) * 1.f;

    if (Diameter < 1.f)
        Diameter = 1.f;

    mOrbDiameter = Diameter;

    mArrivalDistance = Diameter * 2.5f;

    if (mArrivalDistance < 4.f)
        mArrivalDistance = 4.f;

    auto Mesh = mMeshComponent.lock();

    if (Mesh)
    {
        Mesh->SetRelativeScale(Diameter, Diameter);
        Mesh->SetRenderSortYBias(-Diameter * 0.5f);
        Mesh->SetRenderSortPriority(1);
    }

    mScaleInitialized = true;
}

bool CBuildingMarkerOrb::CollectTargetMarkers(
    std::vector<std::pair<std::string, FVector3>>& OutMarkers)
{
    OutMarkers.clear();

    auto World = mWorld.lock();

    if (!World)
        return false;

    std::vector<std::string> TargetNames;

    if (!mRandomTargetNames.empty())
    {
        TargetNames = mRandomTargetNames;
    }

    else
    {
        if (!mBuildingAName.empty())
            TargetNames.push_back(mBuildingAName);

        if (!mBuildingBName.empty() &&
            mBuildingBName != mBuildingAName)
        {
            TargetNames.push_back(mBuildingBName);
        }
    }

    std::vector<std::string> UniqueNames;

    for (size_t i = 0; i < TargetNames.size(); ++i)
    {
        const std::string& Name = TargetNames[i];

        if (Name.empty())
            continue;

        if (std::find(UniqueNames.begin(), UniqueNames.end(), Name) ==
            UniqueNames.end())
        {
            UniqueNames.push_back(Name);
        }
    }

    for (size_t i = 0; i < UniqueNames.size(); ++i)
    {
        auto Building = World->FindObject<CPlacementAreaObject>(
            UniqueNames[i]).lock();

        if (!Building)
            continue;

        FVector3 MarkerPos;

        if (!Building->GetClosestMarkerWorldPos(
            GetWorldPos(), MarkerPos))
            continue;

        OutMarkers.emplace_back(UniqueNames[i], MarkerPos);
    }

    return !OutMarkers.empty();
}

std::string CBuildingMarkerOrb::PickRandomTargetName(
    const std::string& ExcludeName) const
{
    std::vector<std::string> TargetNames;

    if (!mRandomTargetNames.empty())
    {
        TargetNames = mRandomTargetNames;
    }

    else
    {
        if (!mBuildingAName.empty())
            TargetNames.push_back(mBuildingAName);

        if (!mBuildingBName.empty() &&
            mBuildingBName != mBuildingAName)
        {
            TargetNames.push_back(mBuildingBName);
        }
    }

    std::vector<std::string> UniqueNames;

    for (size_t i = 0; i < TargetNames.size(); ++i)
    {
        const std::string& Name = TargetNames[i];

        if (Name.empty())
            continue;

        if (std::find(UniqueNames.begin(), UniqueNames.end(), Name) ==
            UniqueNames.end())
        {
            UniqueNames.push_back(Name);
        }
    }

    if (UniqueNames.empty())
        return std::string();

    std::vector<std::string> CandidateNames;

    for (size_t i = 0; i < UniqueNames.size(); ++i)
    {
        if (UniqueNames[i] != ExcludeName)
            CandidateNames.push_back(UniqueNames[i]);
    }

    if (CandidateNames.empty())
        CandidateNames = UniqueNames;

    const int PickIndex = rand() % (int)CandidateNames.size();

    return CandidateNames[PickIndex];
}

void CBuildingMarkerOrb::RequestMoveTo(
    const std::string& TargetBuildingName)
{
    auto Movement = mMovement.lock();
    auto World = mWorld.lock();

    if (!Movement || !World || TargetBuildingName.empty())
        return;

    auto TargetBuilding =
        World->FindObject<CPlacementAreaObject>(TargetBuildingName).lock();

    if (TargetBuilding)
    {
        FVector3 MarkerPos;

        if (TargetBuilding->GetClosestMarkerWorldPos(
            GetWorldPos(), MarkerPos))
        {
            // 첫 요청에만 목표 위치를 잠근다.
            // 잠금이 살아있으면 기존 좌표를 재사용해 매 retry마다
            // 가장 가까운 마커가 바뀌는 것(circling 원인)을 방지한다.
            if (!mHasLockedTarget)
            {
                mLockedTargetPos = MarkerPos;
                mHasLockedTarget = true;
            }

            const FVector3& NavTarget = mLockedTargetPos;

#ifdef _DEBUG
            DebugOrbLog("[Orb] RequestMoveTo target=%s pos=(%.1f,%.1f) locked=%d\n",
                TargetBuildingName.c_str(), NavTarget.x, NavTarget.y,
                mHasLockedTarget ? 1 : 0);
#endif
            const bool Requested = Movement->MovePath(NavTarget);
            mWaitingForPath = Requested;
            mWaitingPathAccum = 0.f;

            if (!Requested)
            {
                // 큐 포화로 요청이 반려되면 빠르게 재시도한다.
                mPathRetryAccum = mPathRetryInterval * 0.9f;
#ifdef _DEBUG
                DebugOrbLog(
                    "[Orb] RequestMoveTo enqueue failed target=%s\n",
                    TargetBuildingName.c_str());
#endif
            }
            return;
        }
    }

    // Fallback: marker 좌표 확보에 실패했을 때만 기존 오브젝트 타겟 경로를 사용한다.
#ifdef _DEBUG
    DebugOrbLog("[Orb] RequestMoveTo fallback target=%s\n",
        TargetBuildingName.c_str());
#endif
    const bool Requested =
        Movement->MovePathToObject(TargetBuildingName);
    mWaitingForPath = Requested;
    mWaitingPathAccum = 0.f;

    if (!Requested)
    {
        mPathRetryAccum = mPathRetryInterval * 0.9f;
#ifdef _DEBUG
        DebugOrbLog(
            "[Orb] RequestMoveTo fallback enqueue failed target=%s\n",
            TargetBuildingName.c_str());
#endif
    }
}

void CBuildingMarkerOrb::RemoveTargetBuildingName(
    const std::string& BuildingName)
{
    if (BuildingName.empty())
        return;

    mRandomTargetNames.erase(
        std::remove(mRandomTargetNames.begin(),
            mRandomTargetNames.end(), BuildingName),
        mRandomTargetNames.end());

    if (mBuildingAName == BuildingName)
        mBuildingAName.clear();

    if (mBuildingBName == BuildingName)
        mBuildingBName.clear();

    if (mCurrentTargetName == BuildingName)
    {
        mCurrentTargetName.clear();
        mHasLockedTarget = false;
        mWaitingForPath = false;
        mWaitingPathAccum = 0.f;
        mStallAccum = 0.f;

        auto Movement = mMovement.lock();

        if (Movement)
        {
            Movement->AdvancePathRequestId();
            Movement->StartPathPoint();
            Movement->SetPathTargetObjectName("");
        }
    }

    if (mBuildingAName.empty() && !mRandomTargetNames.empty())
        mBuildingAName = mRandomTargetNames[0];

    if (mBuildingBName.empty())
    {
        for (size_t i = 0; i < mRandomTargetNames.size(); ++i)
        {
            if (mRandomTargetNames[i] != mBuildingAName)
            {
                mBuildingBName = mRandomTargetNames[i];
                break;
            }
        }
    }

    // 핵심 건물(집/직장/음식)이 철거되면 Wander로 폴백
    if (mHomeName == BuildingName) { mHomeName.clear(); }
    if (mWorkName == BuildingName) { mWorkName.clear(); }
    if (mFoodName == BuildingName) { mFoodName.clear(); }
    if (mFoodVisitBuildingName == BuildingName)
    {
        mFoodVisitBuildingName.clear();
        mCitizenProfileState.FoodStockAvailableThisVisit = false;
        mFoodVisitReserved = false;
    }
    if (mFunName == BuildingName) { mFunName.clear(); }
    if (mFunVisitBuildingName == BuildingName)
    {
        mFunVisitBuildingName.clear();
        mCitizenProfileState.FunServiceAvailableThisVisit = false;
        mFunVisitReserved = false;
    }
    if (mHealthName == BuildingName) { mHealthName.clear(); }
    if (mHealthVisitBuildingName == BuildingName)
    {
        mHealthVisitBuildingName.clear();
        mCitizenProfileState.HealthServiceAvailableThisVisit = false;
        mHealthVisitReserved = false;
    }
    if (mFaithName == BuildingName) { mFaithName.clear(); }
    if (mFaithVisitBuildingName == BuildingName)
    {
        mFaithVisitBuildingName.clear();
        mCitizenProfileState.FaithServiceAvailableThisVisit = false;
        mFaithVisitReserved = false;
    }
    if (mTeamsterDeliveryState.SourceName == BuildingName ||
        mTeamsterDeliveryState.DestinationName == BuildingName)
    {
        ReleaseTeamsterReservations();
    }
    if (mTeamsterDeliveryState.SourceName == BuildingName)
    {
        mTeamsterDeliveryState.SourceName.clear();
        mTeamsterDeliveryState.ClearCargo();
    }
    if (mTeamsterDeliveryState.DestinationName == BuildingName)
    {
        mTeamsterDeliveryState.DestinationName.clear();
        mTeamsterDeliveryState.ClearCargo();
    }

    if ((mTeamsterDeliveryState.SourceName.empty() ||
        mTeamsterDeliveryState.DestinationName.empty()) &&
        IsTeamsterState(mCitizenState))
    {
        ResetTeamsterSpeed();
        TransitionFsm(ECitizenState::GoingToWork);
    }

    if ((mHomeName.empty() || mWorkName.empty() || mFoodName.empty()) &&
        mCitizenState != ECitizenState::Wander)
    {
        ResetTeamsterSpeed();
        mCitizenState = ECitizenState::Wander;
        mCurrentTargetName.clear();
        mHasLockedTarget = false;
    }
    else if (((mCitizenState == ECitizenState::GoingToFun ||
        mCitizenState == ECitizenState::AtFun) &&
        mFunName.empty()) ||
        ((mCitizenState == ECitizenState::GoingToHealth ||
          mCitizenState == ECitizenState::AtHealth) &&
         mHealthName.empty()) ||
        ((mCitizenState == ECitizenState::GoingToFaith ||
          mCitizenState == ECitizenState::AtFaith) &&
         mFaithName.empty()))
    {
        TransitionFsm(ECitizenState::GoingToWork);
    }
}
