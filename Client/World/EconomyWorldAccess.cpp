#include "EconomyWorldAccess.h"
#include "World/World.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"

namespace EconomyWorldAccess
{
    namespace
    {
        class FPlacementAreaObjectEconomyAccess final
            : public IEconomyBuildingAccess
        {
        public:
            explicit FPlacementAreaObjectEconomyAccess(
                const std::shared_ptr<CPlacementAreaObject>& Building)
                : mBuilding(Building)
            {
            }

            bool IsOperational() const override
            {
                return mBuilding &&
                    mBuilding->GetAlive() &&
                    mBuilding->GetEnable() &&
                    mBuilding->HasPlacedArea();
            }

            bool IsHarbor() const override
            {
                return mBuilding && mBuilding->IsHarbor();
            }

            bool IsRoad() const override
            {
                return mBuilding && mBuilding->IsRoad();
            }

            bool IsBusStop() const override
            {
                return mBuilding && mBuilding->IsBusStop();
            }

            bool IsTransportOffice() const override
            {
                return mBuilding && mBuilding->IsTransportOffice();
            }

            bool IsWarehouse() const override
            {
                return mBuilding && mBuilding->IsWarehouse();
            }

            bool IsResidential() const override
            {
                return mBuilding && mBuilding->IsResidential();
            }

            bool CanExportStoredResources() const override
            {
                return mBuilding && mBuilding->CanExportStoredResources();
            }

            int GetAvailableResourceStock(EResourceType Type) const override
            {
                return mBuilding ? mBuilding->GetAvailableResourceStock(Type) : 0;
            }

            EResourceType GetVisitConsumptionResourceType() const override
            {
                return mBuilding ?
                    mBuilding->GetVisitConsumptionResourceType() :
                    EResourceType::None;
            }

            EResourceType GetProducedResourceType() const override
            {
                return mBuilding ?
                    mBuilding->GetProducedResourceType() :
                    EResourceType::None;
            }

            int GetResourceStock(EResourceType Type) const override
            {
                return mBuilding ? mBuilding->GetResourceStock(Type) : 0;
            }

            int GetReservedIncomingResourceAmount(
                EResourceType Type) const override
            {
                return mBuilding ?
                    mBuilding->GetReservedIncomingResourceAmount(Type) :
                    0;
            }

            int GetProductionInputCount() const override
            {
                return mBuilding ? mBuilding->GetProductionInputCount() : 0;
            }

            EResourceType GetProductionInputType(int SlotIndex) const override
            {
                return mBuilding ?
                    mBuilding->GetProductionInputType(SlotIndex) :
                    EResourceType::None;
            }

            int GetTeamsterCargoLossPercent() const override
            {
                return mBuilding ? mBuilding->GetTeamsterCargoLossPercent() : 0;
            }

            bool TryConsumeResource(
                EResourceType Type,
                int Amount) override
            {
                return mBuilding && mBuilding->TryConsumeResource(Type, Amount);
            }

            void AddResourceStock(
                EResourceType Type,
                int Amount) override
            {
                if (mBuilding)
                    mBuilding->AddResourceStock(Type, Amount);
            }

            long long GetDailyWageCost(int DaysInMonth) const override
            {
                return mBuilding ? mBuilding->GetDailyWageCost(DaysInMonth) : 0;
            }

            long long GetDailyUpkeepCost(int DaysInMonth) const override
            {
                return mBuilding ? mBuilding->GetDailyUpkeepCost(DaysInMonth) : 0;
            }

            bool AdvanceHarborShipProgressAndCheckArrival(
                int DaysInMonth) override
            {
                return mBuilding &&
                    mBuilding->AdvanceHarborShipProgressAndCheckArrival(
                        DaysInMonth);
            }

            void ApplyDailyWarehouseStorageLoss() override
            {
                if (mBuilding)
                    mBuilding->ApplyDailyWarehouseStorageLoss();
            }

        private:
            std::shared_ptr<CPlacementAreaObject> mBuilding;
        };

        class FBuildingMarkerOrbEconomyAccess final
            : public IEconomyCitizenAccess
        {
        public:
            explicit FBuildingMarkerOrbEconomyAccess(
                const std::shared_ptr<CBuildingMarkerOrb>& Citizen)
                : mCitizen(Citizen)
            {
            }

            bool IsOperational() const override
            {
                return mCitizen &&
                    mCitizen->GetAlive() &&
                    mCitizen->GetEnable();
            }

            FCitizenEconomySnapshot GetEconomySnapshot() const override
            {
                FCitizenEconomySnapshot Snapshot;

                if (!mCitizen)
                    return Snapshot;

                const FNpcSatisfaction& Satisfaction =
                    mCitizen->GetSatisfaction();
                Snapshot.Security = Satisfaction.Security;
                Snapshot.Overall = Satisfaction.Overall;
                Snapshot.HasWorkBuilding = !mCitizen->GetWorkBuilding().empty();
                Snapshot.HasHomeBuilding = !mCitizen->GetHomeBuilding().empty();
                return Snapshot;
            }

            void ApplySatisfactionDelta(
                float FoodDelta,
                float HealthDelta,
                float FunDelta,
                float FaithDelta,
                float HousingDelta,
                float JobDelta,
                float FreedomDelta,
                float SecurityDelta) override
            {
                if (!mCitizen)
                    return;

                mCitizen->ApplySatisfactionDelta(
                    FoodDelta,
                    HealthDelta,
                    FunDelta,
                    FaithDelta,
                    HousingDelta,
                    JobDelta,
                    FreedomDelta,
                    SecurityDelta);
            }

        private:
            std::shared_ptr<CBuildingMarkerOrb> mCitizen;
        };
    }

    std::vector<std::shared_ptr<IEconomyBuildingAccess>>
        CollectBuildings(CWorld* World)
    {
        std::vector<std::shared_ptr<IEconomyBuildingAccess>> Result;

        if (!World)
            return Result;

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
            return Result;

        Result.reserve(BuildingList.size());

        for (size_t Index = 0; Index < BuildingList.size(); ++Index)
        {
            auto Building = BuildingList[Index].lock();

            if (!Building)
                continue;

            Result.push_back(std::make_shared<FPlacementAreaObjectEconomyAccess>(
                Building));
        }

        return Result;
    }

    std::vector<std::shared_ptr<IEconomyCitizenAccess>>
        CollectCitizens(CWorld* World)
    {
        std::vector<std::shared_ptr<IEconomyCitizenAccess>> Result;

        if (!World)
            return Result;

        std::vector<std::weak_ptr<CBuildingMarkerOrb>> CitizenList;

        if (!World->FindObjectListByType<CBuildingMarkerOrb>(CitizenList))
            return Result;

        Result.reserve(CitizenList.size());

        for (size_t Index = 0; Index < CitizenList.size(); ++Index)
        {
            auto Citizen = CitizenList[Index].lock();

            if (!Citizen)
                continue;

            Result.push_back(std::make_shared<FBuildingMarkerOrbEconomyAccess>(
                Citizen));
        }

        return Result;
    }
}
