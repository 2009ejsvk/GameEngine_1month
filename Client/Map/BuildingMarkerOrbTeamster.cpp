#include "BuildingMarkerOrb.h"
#include "PlacementAreaObject.h"
#include "World/World.h"

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
        Delivery.ClearCargo();
        mCurrentTargetName = mWorkName;
        break;
    case ECitizenState::AtWork:
        mFoodVisitBuildingName.clear();
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
        mCurrentTargetName = Delivery.HarborName;
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

bool CBuildingMarkerOrb::TryStartTeamsterDelivery()
{
    auto& Delivery = mTeamsterDeliveryState;

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

    const std::string SourceName = FindTeamsterSourceName();

    if (SourceName.empty())
        return false;

    const std::string HarborName = FindHarborName();

    if (HarborName.empty())
        return false;

    Delivery.SourceName = SourceName;
    Delivery.HarborName = HarborName;
    Delivery.ClearCargo();

    TransitionFsm(ECitizenState::GoingToTeamsterSource);
    mPathRetryAccum = 0.f;
    return true;
}

std::string CBuildingMarkerOrb::FindTeamsterSourceName() const
{
    auto World = mWorld.lock();

    if (!World)
        return std::string();

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return std::string();

    std::string BestName;
    int BestStock = 0;

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        auto Building = BuildingList[i].lock();

        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable() ||
            !Building->HasPlacedArea() ||
            !Building->SupportsTeamsterPickup())
        {
            continue;
        }

        const int Stock = Building->GetExportableResourceStock();

        if (Stock < GTeamsterTransferUnit)
            continue;

        if (BestName.empty() || Stock > BestStock)
        {
            BestName = Building->GetName();
            BestStock = Stock;
        }
    }

    return BestName;
}

std::string CBuildingMarkerOrb::FindHarborName() const
{
    auto World = mWorld.lock();

    if (!World)
        return std::string();

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return std::string();

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        auto Building = BuildingList[i].lock();

        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable() ||
            !Building->HasPlacedArea() ||
            !Building->IsHarbor())
        {
            continue;
        }

        return Building->GetName();
    }

    return std::string();
}
