#pragma once

#include "../Building/BuildingTypes.h"
#include "../Citizen/CitizenTypes.h"
#include <algorithm>
#include <cmath>

struct FBuildingServiceProfile
{
    EBuildingCategory Category = EBuildingCategory::Infrastructure;
    bool Residential = false;
    bool FoodProvider = false;
    bool EntertainmentProvider = false;
    int HousingSatisfactionCap = 100;
    int JobSatisfactionCap = 100;
    int FoodSatisfactionCap = 100;
    int FunSatisfactionCap = 100;
    ECitizenEducationLevel RequiredEducationLevel =
        ECitizenEducationLevel::Uneducated;
    int Capacity = 0;

    static int ClampCapToPercent(int Value)
    {
        return (std::max)(0, (std::min)(100, Value));
    }

    void ConfigureDisplay(
        bool InResidential,
        int InCapacity,
        bool InFoodProvider,
        bool InEntertainmentProvider,
        int InHousingSatisfactionCap,
        int InJobSatisfactionCap,
        int InFoodSatisfactionCap,
        int InFunSatisfactionCap)
    {
        Residential = InResidential;
        Capacity = (std::max)(0, InCapacity);
        FoodProvider = InFoodProvider;
        EntertainmentProvider = InEntertainmentProvider;
        HousingSatisfactionCap =
            ClampCapToPercent(InHousingSatisfactionCap);
        JobSatisfactionCap =
            ClampCapToPercent(InJobSatisfactionCap);
        FoodSatisfactionCap =
            ClampCapToPercent(InFoodSatisfactionCap);
        FunSatisfactionCap =
            ClampCapToPercent(InFunSatisfactionCap);
    }
};

struct FBuildingOperationsState
{
    static constexpr int MaxResourceStock = 100000;
    static constexpr int WarehouseSlotCount = 3;
    static constexpr int WarehouseSlotCapacity =
        MaxResourceStock / WarehouseSlotCount;

    int BudgetLevel = 3;
    int BaseMonthlyWage = 0;
    int BaseMonthlyUpkeep = 0;
    FResourceInventory ResourceInventory;
    EResourceType ProducedResourceType = EResourceType::None;
    EResourceType VisitConsumptionResourceType = EResourceType::None;
    bool SupportsTeamsterPickup = false;
    bool CanExportStoredResources = false;
    bool UsesWarehouseSlots = false;
    float ResourceProductionAccum = 0.f;
    float HarborShipProgressMonths = 0.f;
    int ReservedExportPickupAmount = 0;
    std::array<EResourceType, WarehouseSlotCount> WarehouseSlotTypes =
    {
        EResourceType::None,
        EResourceType::None,
        EResourceType::None
    };
    std::array<int, static_cast<size_t>(EResourceType::Count)>
        ReservedResourcePickupAmounts = {};
    std::array<int, static_cast<size_t>(EResourceType::Count)>
        ReservedIncomingResourceAmounts = {};

    void SetBudgetLevel(int Level)
    {
        BudgetLevel = (std::max)(1, (std::min)(5, Level));
    }

    float GetBudgetSatisfactionScale() const
    {
        switch (BudgetLevel)
        {
        case 1: return 0.70f;
        case 2: return 0.85f;
        case 4: return 1.15f;
        case 5: return 1.30f;
        default: return 1.00f;
        }
    }

    int ApplyBudgetScale(int BaseCap) const
    {
        const float Scaled =
            static_cast<float>(BaseCap) * GetBudgetSatisfactionScale();
        const int Rounded = static_cast<int>(roundf(Scaled));
        return (std::max)(0, (std::min)(100, Rounded));
    }

    int ApplyEconomyScale(int BaseCost) const
    {
        const float Scaled =
            static_cast<float>(BaseCost) * GetBudgetSatisfactionScale();
        return (std::max)(0, static_cast<int>(roundf(Scaled)));
    }

    void ConfigureEconomy(
        const FBuildingServiceProfile& ServiceProfile,
        bool IsTransportOffice,
        bool IsHarbor,
        int InBaseMonthlyWage,
        int InBaseMonthlyUpkeep)
    {
        BudgetLevel = 3;
        const int SafeCapacity = (std::max)(0, ServiceProfile.Capacity);

        if (InBaseMonthlyWage < 0)
        {
            if (ServiceProfile.Residential)
            {
                BaseMonthlyWage = 0;
            }
            else
            {
                int DerivedWage = (std::max)(1, SafeCapacity) * 120;

                if (IsTransportOffice)
                    DerivedWage = (std::max)(DerivedWage, 800);

                if (IsHarbor)
                    DerivedWage = (std::max)(DerivedWage, 1000);

                BaseMonthlyWage = DerivedWage;
            }
        }
        else
        {
            BaseMonthlyWage = (std::max)(0, InBaseMonthlyWage);
        }

        if (InBaseMonthlyUpkeep < 0)
        {
            int DerivedUpkeep = ServiceProfile.Residential ?
                (80 + SafeCapacity * 4) :
                (110 + SafeCapacity * 5);

            if (IsTransportOffice)
                DerivedUpkeep += 300;

            if (IsHarbor)
                DerivedUpkeep += 450;

            if (ServiceProfile.EntertainmentProvider &&
                !ServiceProfile.FoodProvider)
            {
                DerivedUpkeep += 120;
            }

            if (ServiceProfile.FoodProvider)
                DerivedUpkeep += 90;

            BaseMonthlyUpkeep = (std::max)(0, DerivedUpkeep);
        }
        else
        {
            BaseMonthlyUpkeep = (std::max)(0, InBaseMonthlyUpkeep);
        }
    }

    void ConfigureResourceBehavior(
        EResourceType InProducedResourceType,
        EResourceType InVisitConsumptionResourceType,
        bool InSupportsTeamsterPickup,
        bool InCanExportStoredResources)
    {
        ProducedResourceType = InProducedResourceType;
        VisitConsumptionResourceType = InVisitConsumptionResourceType;
        SupportsTeamsterPickup = InSupportsTeamsterPickup;
        CanExportStoredResources = InCanExportStoredResources;
    }

    void ConfigureStorageBehavior(bool InUsesWarehouseSlots)
    {
        UsesWarehouseSlots = InUsesWarehouseSlots;

        if (!UsesWarehouseSlots)
        {
            for (int SlotIndex = 0; SlotIndex < WarehouseSlotCount; ++SlotIndex)
                WarehouseSlotTypes[SlotIndex] = EResourceType::None;
        }
    }

    int GetMonthlyWageCost() const
    {
        return ApplyEconomyScale(BaseMonthlyWage);
    }

    int GetMonthlyUpkeepCost() const
    {
        return ApplyEconomyScale(BaseMonthlyUpkeep);
    }

    int GetDailyWageCost(int DaysInMonth) const
    {
        const int SafeDays = (std::max)(1, DaysInMonth);
        const float Daily = static_cast<float>(GetMonthlyWageCost()) /
            static_cast<float>(SafeDays);
        return (std::max)(0, static_cast<int>(roundf(Daily)));
    }

    int GetDailyUpkeepCost(int DaysInMonth) const
    {
        const int SafeDays = (std::max)(1, DaysInMonth);
        const float Daily = static_cast<float>(GetMonthlyUpkeepCost()) /
            static_cast<float>(SafeDays);
        return (std::max)(0, static_cast<int>(roundf(Daily)));
    }

    bool AdvanceHarborShipProgressAndCheckArrival(
        bool IsHarbor,
        int DaysInMonth)
    {
        if (!IsHarbor)
            return false;

        const int SafeDays = (std::max)(1, DaysInMonth);
        const float DailyProgress =
            GetBudgetSatisfactionScale() /
            static_cast<float>(SafeDays);
        HarborShipProgressMonths += DailyProgress;

        constexpr float GBaseShipIntervalMonths = 3.f;

        if (HarborShipProgressMonths < GBaseShipIntervalMonths)
            return false;

        while (HarborShipProgressMonths >= GBaseShipIntervalMonths)
        {
            HarborShipProgressMonths -= GBaseShipIntervalMonths;
        }

        return true;
    }

    float GetHarborShipProgressPercent(bool IsHarbor) const
    {
        if (!IsHarbor)
            return 0.f;

        constexpr float GBaseShipIntervalMonths = 3.f;
        return (std::max)(
            0.f,
            (std::min)(
                1.f,
                HarborShipProgressMonths / GBaseShipIntervalMonths));
    }

    int GetResourceStock() const
    {
        return ResourceInventory.GetTotal();
    }

    int GetResourceStock(EResourceType Type) const
    {
        return ResourceInventory.Get(Type);
    }

    int GetExportableResourceStock() const
    {
        return ResourceInventory.GetExportableTotal();
    }

    int GetMaxTotalResourceStock() const
    {
        return UsesWarehouseSlots ?
            WarehouseSlotCapacity * WarehouseSlotCount :
            MaxResourceStock;
    }

    int GetResourceTypeCapacity(EResourceType Type) const
    {
        if (Type == EResourceType::None)
            return 0;

        return UsesWarehouseSlots ? WarehouseSlotCapacity : MaxResourceStock;
    }

    int GetWarehouseSlotType(int SlotIndex) const
    {
        if (SlotIndex < 0 || SlotIndex >= WarehouseSlotCount)
            return static_cast<int>(EResourceType::None);

        return static_cast<int>(WarehouseSlotTypes[SlotIndex]);
    }

    int FindWarehouseSlotByType(EResourceType Type) const
    {
        for (int SlotIndex = 0; SlotIndex < WarehouseSlotCount; ++SlotIndex)
        {
            if (WarehouseSlotTypes[SlotIndex] == Type)
                return SlotIndex;
        }

        return -1;
    }

    void CleanupWarehouseSlots()
    {
        if (!UsesWarehouseSlots)
            return;

        for (int SlotIndex = 0; SlotIndex < WarehouseSlotCount; ++SlotIndex)
        {
            const EResourceType SlotType = WarehouseSlotTypes[SlotIndex];

            if (SlotType == EResourceType::None)
                continue;

            const size_t ResourceIndex = static_cast<size_t>(SlotType);

            if (ResourceIndex >= ReservedIncomingResourceAmounts.size())
                continue;

            if (ResourceInventory.Get(SlotType) <= 0 &&
                ReservedIncomingResourceAmounts[ResourceIndex] <= 0)
            {
                WarehouseSlotTypes[SlotIndex] = EResourceType::None;
            }
        }
    }

    int FindFreeWarehouseSlot() const
    {
        for (int SlotIndex = 0; SlotIndex < WarehouseSlotCount; ++SlotIndex)
        {
            if (WarehouseSlotTypes[SlotIndex] == EResourceType::None)
                return SlotIndex;
        }

        return -1;
    }

    bool EnsureWarehouseSlotAssigned(EResourceType Type)
    {
        if (!UsesWarehouseSlots || Type == EResourceType::None)
            return Type != EResourceType::None;

        CleanupWarehouseSlots();

        if (FindWarehouseSlotByType(Type) >= 0)
            return true;

        const int FreeSlot = FindFreeWarehouseSlot();

        if (FreeSlot < 0)
            return false;

        WarehouseSlotTypes[FreeSlot] = Type;
        return true;
    }

    bool CanStoreResourceType(EResourceType Type) const
    {
        if (Type == EResourceType::None)
            return false;

        if (!UsesWarehouseSlots)
            return true;

        if (FindWarehouseSlotByType(Type) >= 0)
            return true;

        for (int SlotIndex = 0; SlotIndex < WarehouseSlotCount; ++SlotIndex)
        {
            const EResourceType SlotType = WarehouseSlotTypes[SlotIndex];

            if (SlotType == EResourceType::None)
                return true;

            const size_t ResourceIndex = static_cast<size_t>(SlotType);

            if (ResourceIndex >= ReservedIncomingResourceAmounts.size())
                continue;

            if (ResourceInventory.Get(SlotType) <= 0 &&
                ReservedIncomingResourceAmounts[ResourceIndex] <= 0)
            {
                return true;
            }
        }

        return false;
    }

    int GetAvailableIncomingCapacity(EResourceType Type) const
    {
        if (Type == EResourceType::None || !CanStoreResourceType(Type))
            return 0;

        const size_t ResourceIndex = static_cast<size_t>(Type);

        if (ResourceIndex >= ReservedIncomingResourceAmounts.size())
            return 0;

        return (std::max)(
            0,
            GetResourceTypeCapacity(Type) -
                GetResourceStock(Type) -
                ReservedIncomingResourceAmounts[ResourceIndex]);
    }

    int GetAvailableExportableResourceStock() const
    {
        return (std::max)(
            0,
            GetExportableResourceStock() - ReservedExportPickupAmount);
    }

    int GetAvailableResourceStock(EResourceType Type) const
    {
        const size_t Index = static_cast<size_t>(Type);

        if (Index >= ReservedResourcePickupAmounts.size())
            return 0;

        return (std::max)(
            0,
            GetResourceStock(Type) - ReservedResourcePickupAmounts[Index]);
    }

    int GetReservedIncomingResourceAmount(EResourceType Type) const
    {
        const size_t Index = static_cast<size_t>(Type);

        if (Index >= ReservedIncomingResourceAmounts.size())
            return 0;

        return ReservedIncomingResourceAmounts[Index];
    }

    EResourceType ResolvePrimaryResourceTypeForLegacy() const
    {
        if (ProducedResourceType != EResourceType::None)
            return ProducedResourceType;

        if (VisitConsumptionResourceType != EResourceType::None)
            return VisitConsumptionResourceType;

        return EResourceType::ManufacturedGoods;
    }

    void AddResourceStock(EResourceType Type, int Amount)
    {
        if (Amount <= 0 || Type == EResourceType::None)
            return;

        if (UsesWarehouseSlots && !EnsureWarehouseSlotAssigned(Type))
            return;

        ResourceInventory.Add(Type, Amount, GetResourceTypeCapacity(Type));
    }

    bool TryAddResourceStock(EResourceType Type, int Amount)
    {
        if (Amount <= 0)
            return true;

        if (Type == EResourceType::None)
            return false;

        if (UsesWarehouseSlots && !EnsureWarehouseSlotAssigned(Type))
            return false;

        const int CurrentStock = GetResourceStock(Type);
        const int Capacity = GetResourceTypeCapacity(Type);

        if (CurrentStock + Amount > Capacity)
            return false;

        ResourceInventory.Add(Type, Amount, Capacity);
        return true;
    }

    void AddProduction(float UnitsPerSec, float DeltaTime)
    {
        if (UnitsPerSec <= 0.f ||
            ProducedResourceType == EResourceType::None)
        {
            return;
        }

        ResourceProductionAccum += UnitsPerSec * DeltaTime;
        const int Whole = static_cast<int>(ResourceProductionAccum);

        if (Whole <= 0)
            return;

        ResourceProductionAccum -= static_cast<float>(Whole);
        AddResourceStock(ProducedResourceType, Whole);
    }

    bool TryConsumeResource(EResourceType Type, int Amount = 1)
    {
        const bool Consumed = ResourceInventory.Consume(Type, Amount);

        if (Consumed)
            CleanupWarehouseSlots();

        return Consumed;
    }

    bool TryConsumeAnyExportableResource(
        int Amount,
        EResourceType& OutType)
    {
        const bool Consumed =
            ResourceInventory.ConsumeAnyExportable(Amount, OutType);

        if (Consumed)
            CleanupWarehouseSlots();

        return Consumed;
    }

    bool TryConsumeExportableResources(int Amount)
    {
        const bool Consumed = ResourceInventory.ConsumeExportableAmount(Amount);

        if (Consumed)
            CleanupWarehouseSlots();

        return Consumed;
    }

    bool TryGetExportableResourceTypeForAmount(
        int Amount,
        EResourceType& OutType) const
    {
        if (Amount <= 0)
        {
            OutType = EResourceType::None;
            return true;
        }

        for (size_t Index = 0; Index < ReservedResourcePickupAmounts.size(); ++Index)
        {
            const EResourceType Type = static_cast<EResourceType>(Index);

            if (!IsExportableResourceType(Type) ||
                GetAvailableResourceStock(Type) < Amount)
            {
                continue;
            }

            OutType = Type;
            return true;
        }

        OutType = EResourceType::None;
        return false;
    }

    bool ReserveExportPickupAmount(int Amount)
    {
        if (Amount <= 0)
            return false;

        if (GetAvailableExportableResourceStock() < Amount)
            return false;

        ReservedExportPickupAmount += Amount;
        return true;
    }

    void ReleaseExportPickupAmount(int Amount)
    {
        if (Amount <= 0)
            return;

        ReservedExportPickupAmount = (std::max)(
            0,
            ReservedExportPickupAmount - Amount);
    }

    bool ReserveResourcePickupAmount(EResourceType Type, int Amount)
    {
        if (Amount <= 0 || Type == EResourceType::None)
            return false;

        const size_t Index = static_cast<size_t>(Type);

        if (Index >= ReservedResourcePickupAmounts.size() ||
            GetAvailableResourceStock(Type) < Amount)
        {
            return false;
        }

        ReservedResourcePickupAmounts[Index] += Amount;
        return true;
    }

    void ReleaseResourcePickupAmount(EResourceType Type, int Amount)
    {
        if (Amount <= 0 || Type == EResourceType::None)
            return;

        const size_t Index = static_cast<size_t>(Type);

        if (Index >= ReservedResourcePickupAmounts.size())
            return;

        ReservedResourcePickupAmounts[Index] = (std::max)(
            0,
            ReservedResourcePickupAmounts[Index] - Amount);
    }

    bool ReserveIncomingResourceAmount(EResourceType Type, int Amount)
    {
        if (Amount <= 0 || Type == EResourceType::None)
            return false;

        if (UsesWarehouseSlots && !EnsureWarehouseSlotAssigned(Type))
            return false;

        const size_t Index = static_cast<size_t>(Type);

        if (Index >= ReservedIncomingResourceAmounts.size() ||
            GetAvailableIncomingCapacity(Type) < Amount)
        {
            return false;
        }

        ReservedIncomingResourceAmounts[Index] += Amount;
        return true;
    }

    void ReleaseIncomingResourceAmount(EResourceType Type, int Amount)
    {
        if (Amount <= 0 || Type == EResourceType::None)
            return;

        const size_t Index = static_cast<size_t>(Type);

        if (Index >= ReservedIncomingResourceAmounts.size())
            return;

        ReservedIncomingResourceAmounts[Index] = (std::max)(
            0,
            ReservedIncomingResourceAmounts[Index] - Amount);
        CleanupWarehouseSlots();
    }
};
