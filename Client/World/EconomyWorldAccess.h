#pragma once

#include "../Building/BuildingTypes.h"
#include "../Citizen/CitizenTypes.h"
#include <memory>
#include <vector>

class CWorld;

namespace EconomyWorldAccess
{
    struct FCitizenEconomySnapshot
    {
        float Security = 0.f;
        float Overall = 0.f;
        ECitizenWealthLevel WealthLevel = ECitizenWealthLevel::Poor;
        bool HasWorkBuilding = false;
        bool HasHomeBuilding = false;
    };

    class IEconomyBuildingAccess
    {
    public:
        virtual ~IEconomyBuildingAccess() = default;

        virtual bool IsOperational() const = 0;
        virtual bool IsHarbor() const = 0;
        virtual bool IsRoad() const = 0;
        virtual bool IsBusStop() const = 0;
        virtual bool IsTransportOffice() const = 0;
        virtual bool IsWarehouse() const = 0;
        virtual bool IsResidential() const = 0;
        virtual bool CanExportStoredResources() const = 0;
        virtual int GetAvailableResourceStock(EResourceType Type) const = 0;
        virtual int GetAvailableIncomingCapacity(EResourceType Type) const = 0;
        virtual EResourceType GetVisitConsumptionResourceType() const = 0;
        virtual std::vector<EResourceType>
            GetRuntimeVisitConsumptionAcceptedResourceTypes() const = 0;
        virtual EResourceType GetProducedResourceType() const = 0;
        virtual int GetResourceStock(EResourceType Type) const = 0;
        virtual int GetVisitConsumptionCompatibleResourceStock(
            EResourceType DemandType) const = 0;
        virtual int GetReservedIncomingResourceAmount(
            EResourceType Type) const = 0;
        virtual int GetVisitConsumptionCompatibleReservedIncomingResourceAmount(
            EResourceType DemandType) const = 0;
        virtual bool GetPlacedCenterGridCoords(
            int& OutGridX,
            int& OutGridY) const = 0;
        virtual int GetProductionInputCount() const = 0;
        virtual EResourceType GetProductionInputType(int SlotIndex) const = 0;
        virtual int GetTeamsterCargoLossPercent() const = 0;
        virtual bool TryConsumeResource(
            EResourceType Type,
            int Amount) = 0;
        virtual void AddResourceStock(
            EResourceType Type,
            int Amount) = 0;
        virtual long long GetDailyWageCost(int DaysInMonth) const = 0;
        virtual long long GetDailyUpkeepCost(int DaysInMonth) const = 0;
        virtual bool AdvanceHarborShipProgressAndCheckArrival(
            int DaysInMonth,
            float EdictMultiplier = 1.f) = 0;
        virtual void ApplyDailyWarehouseStorageLoss() = 0;
    };

    class IEconomyCitizenAccess
    {
    public:
        virtual ~IEconomyCitizenAccess() = default;

        virtual bool IsOperational() const = 0;
        virtual FCitizenEconomySnapshot GetEconomySnapshot() const = 0;
        virtual void ApplySatisfactionDelta(
            float FoodDelta,
            float HealthDelta,
            float FunDelta,
            float FaithDelta,
            float HousingDelta,
            float JobDelta,
            float FreedomDelta,
            float SecurityDelta) = 0;
    };

    std::vector<std::shared_ptr<IEconomyBuildingAccess>>
        CollectBuildings(CWorld* World);

    std::vector<std::shared_ptr<IEconomyCitizenAccess>>
        CollectCitizens(CWorld* World);
}
