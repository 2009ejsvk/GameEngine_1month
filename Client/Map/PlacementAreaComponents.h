#pragma once

#include "../Building/BuildingTypes.h"
#include "../Citizen/CitizenTypes.h"
#include "../GameConstants.h"
#include <algorithm>
#include <limits>
#include <cmath>

struct FBuildingServiceProfile
{
    EBuildingCategory Category = EBuildingCategory::Infrastructure;
    bool Residential = false;
    bool FoodProvider = false;
    bool EntertainmentProvider = false;
    bool HealthProvider = false;
    bool FaithProvider = false;
    int HousingSatisfactionCap = 100;
    int JobSatisfactionCap = 100;
    int FoodSatisfactionCap = 100;
    int FunSatisfactionCap = 100;
    int HealthSatisfactionCap = 100;
    int FaithSatisfactionCap = 100;
    ECitizenEducationLevel RequiredEducationLevel =
        ECitizenEducationLevel::Uneducated;
    unsigned int AllowedWealthMask = GBuildingWealthMaskAll;
    int Capacity = 0;
    int HouseholdCapacity = 0;
    int ServiceCapacity = 0;
    EBuildingLeisureClass LeisureClass = EBuildingLeisureClass::None;
    ETouristPreference PrimaryTouristPreference = ETouristPreference::None;

    static int ClampCapToPercent(int Value)
    {
        return (std::max)(0, (std::min)(100, Value));
    }

    void ConfigureDisplay(
        bool InResidential,
        int InCapacity,
        bool InFoodProvider,
        bool InEntertainmentProvider,
        bool InHealthProvider,
        bool InFaithProvider,
        int InHousingSatisfactionCap,
        int InJobSatisfactionCap,
        int InFoodSatisfactionCap,
        int InFunSatisfactionCap,
        int InHealthSatisfactionCap,
        int InFaithSatisfactionCap,
        int InServiceCapacity)
    {
        Residential = InResidential;
        Capacity = (std::max)(0, InCapacity);
        FoodProvider = InFoodProvider;
        EntertainmentProvider = InEntertainmentProvider;
        HealthProvider = InHealthProvider;
        FaithProvider = InFaithProvider;
        HousingSatisfactionCap =
            ClampCapToPercent(InHousingSatisfactionCap);
        JobSatisfactionCap =
            ClampCapToPercent(InJobSatisfactionCap);
        FoodSatisfactionCap =
            ClampCapToPercent(InFoodSatisfactionCap);
        FunSatisfactionCap =
            ClampCapToPercent(InFunSatisfactionCap);
        HealthSatisfactionCap =
            ClampCapToPercent(InHealthSatisfactionCap);
        FaithSatisfactionCap =
            ClampCapToPercent(InFaithSatisfactionCap);
        ServiceCapacity = (std::max)(0, InServiceCapacity);
    }
};

enum class EWarehouseStoragePolicy
{
    Balanced = 0,
    Dedicated
};

struct FBuildingOperationsState
{
    static constexpr int MaxResourceStock = 100000;
    static constexpr int WarehouseSlotCount = 3;
    static constexpr int WarehouseSlotCapacity =
        MaxResourceStock / WarehouseSlotCount;
    static constexpr long long ProductionAccumScale = 1000000ll;
    static constexpr long long ProductionAccumFractionalCap =
        95ll * ProductionAccumScale / 100ll;
    static constexpr long long StorageLossPercentScale = 1000000ll;
    static constexpr long long StorageLossAccumUnitScale =
        100ll * StorageLossPercentScale;

    int BudgetLevel = 3;
    int BaseMonthlyWage = 0;
    int BaseMonthlyUpkeep = 0;
    FResourceInventory ResourceInventory;
    EResourceType ProducedResourceType = EResourceType::None;
    EResourceType VisitConsumptionResourceType = EResourceType::None;
    std::array<EResourceType, GProductionInputSlotCount>
        ProductionInputTypes = {};
    std::array<int, GProductionInputSlotCount> ProductionInputAmounts = {};
    bool SupportsTeamsterPickup = false;
    bool CanExportStoredResources = false;
    bool UsesWarehouseSlots = false;
    int WarehouseSlotCapacityUnits = WarehouseSlotCapacity;
    EWarehouseStoragePolicy WarehouseStoragePolicy =
        EWarehouseStoragePolicy::Balanced;
    EResourceType PreferredWarehouseResourceType =
        EResourceType::None;
    long long ResourceProductionAccumScaled = 0ll;
    float LastProductionEfficiency = 1.f;
    int LastDailyStorageLoss = 0;
    // Assigned workforce count used for production and per-worker scaling.
    int CurrentWorkerOccupancy = 0;
    // On-site workers currently in AtWork state for UI and observability.
    int WorkingNowOccupancy = 0;
    std::array<int, GBuildingServiceTypeCount> ActiveServiceVisitors = {};
    std::array<int, GBuildingServiceTypeCount> ServiceVisitCaps = {};
    std::array<int, GBuildingServiceTypeCount> ServiceStocks = {};
    std::array<int, GBuildingServiceTypeCount> ServiceStockCaps = {};
    std::array<float, GBuildingServiceTypeCount> ServiceStockAccums = {};
    float HarborShipProgressMonths = 0.f;
    bool HarborShipArrivedThisTick = false;
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
    std::array<long long, static_cast<size_t>(EResourceType::Count)>
        StorageLossAccums = {};

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

    static long long ResolveScaledStorageLossRate(float LossPercent)
    {
        if (LossPercent <= 0.f)
            return 0;

        return static_cast<long long>(std::llround(
            static_cast<double>(LossPercent) *
            static_cast<double>(StorageLossPercentScale)));
    }

    static long long ResolveScaledProductionUnits(double Units)
    {
        if (Units <= 0.0)
            return 0ll;

        const double ScaledUnits =
            Units * static_cast<double>(ProductionAccumScale);
        const double MaxScaledUnits = static_cast<double>(
            (std::numeric_limits<long long>::max)());

        if (ScaledUnits >= MaxScaledUnits)
            return (std::numeric_limits<long long>::max)();

        return static_cast<long long>(std::llround(ScaledUnits));
    }

    static long long ResolveScaledProductionBufferCap()
    {
        return ResolveScaledProductionUnits(
            static_cast<double>((std::max)(
                0.f,
                GameConstants::Economy::ProductionMaxBufferedUnits)));
    }

    void ClampResourceProductionAccumToFractional()
    {
        ResourceProductionAccumScaled = (std::max)(
            0ll,
            (std::min)(
                ResourceProductionAccumScaled,
                ProductionAccumFractionalCap));
    }

    static constexpr size_t GetServiceIndex(EBuildingServiceType Type)
    {
        return static_cast<size_t>(Type);
    }

    static constexpr bool IsValidServiceType(EBuildingServiceType Type)
    {
        return Type >= EBuildingServiceType::Food &&
            Type < EBuildingServiceType::Count;
    }

    static constexpr bool ServiceConsumesStock(EBuildingServiceType Type)
    {
        return Type != EBuildingServiceType::Food;
    }

    bool ProvidesService(EBuildingServiceType Type) const
    {
        if (!IsValidServiceType(Type))
            return false;

        const size_t Index = GetServiceIndex(Type);
        return ServiceVisitCaps[Index] > 0;
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

    void ConfigureServiceBehavior(const FBuildingServiceProfile& ServiceProfile)
    {
        for (size_t Index = 0; Index < ActiveServiceVisitors.size(); ++Index)
        {
            ActiveServiceVisitors[Index] = 0;
            ServiceVisitCaps[Index] = 0;
            ServiceStocks[Index] = 0;
            ServiceStockCaps[Index] = 0;
            ServiceStockAccums[Index] = 0.f;
        }

        auto ConfigureService = [&](EBuildingServiceType Type, bool Enabled)
        {
            if (!Enabled)
                return;

            const size_t Index = GetServiceIndex(Type);
            const int VisitCap = (std::max)(1, ServiceProfile.ServiceCapacity);
            ServiceVisitCaps[Index] = VisitCap;

            if (!ServiceConsumesStock(Type))
                return;

            ServiceStockCaps[Index] =
                VisitCap * GameConstants::Orb::ServiceStockPerCapacity;
            ServiceStocks[Index] = ServiceStockCaps[Index];
        };

        ConfigureService(
            EBuildingServiceType::Food,
            ServiceProfile.FoodProvider);
        ConfigureService(
            EBuildingServiceType::Fun,
            ServiceProfile.EntertainmentProvider);
        ConfigureService(
            EBuildingServiceType::Health,
            ServiceProfile.HealthProvider);
        ConfigureService(
            EBuildingServiceType::Faith,
            ServiceProfile.FaithProvider);
    }

    void ConfigureResourceBehavior(
        EResourceType InProducedResourceType,
        EResourceType InVisitConsumptionResourceType,
        bool InSupportsTeamsterPickup,
        bool InCanExportStoredResources,
        const std::array<EResourceType, GProductionInputSlotCount>&
            InProductionInputTypes = {},
        const std::array<int, GProductionInputSlotCount>&
            InProductionInputAmounts = {})
    {
        ProducedResourceType = InProducedResourceType;
        VisitConsumptionResourceType = InVisitConsumptionResourceType;
        ProductionInputTypes = InProductionInputTypes;
        ProductionInputAmounts = InProductionInputAmounts;
        SupportsTeamsterPickup = InSupportsTeamsterPickup;
        CanExportStoredResources = InCanExportStoredResources;

        for (int SlotIndex = 0;
             SlotIndex < GProductionInputSlotCount;
             ++SlotIndex)
        {
            if (ProductionInputTypes[SlotIndex] == EResourceType::None ||
                ProductionInputAmounts[SlotIndex] <= 0)
            {
                ProductionInputTypes[SlotIndex] = EResourceType::None;
                ProductionInputAmounts[SlotIndex] = 0;
            }
        }
    }

    int ResolveEffectiveServiceVisitCap(
        size_t Index,
        float ServiceThroughputMultiplier,
        int ServiceCapacityDelta) const
    {
        if (Index >= ServiceVisitCaps.size())
            return 0;

        const int BaseVisitCap = ServiceVisitCaps[Index];

        if (BaseVisitCap <= 0)
            return 0;

        const float SafeMultiplier =
            (std::max)(0.f, ServiceThroughputMultiplier);
        const int ScaledVisitCap = static_cast<int>(roundf(
            static_cast<float>(BaseVisitCap) * SafeMultiplier));
        return (std::max)(0, ScaledVisitCap + ServiceCapacityDelta);
    }

    void TickServiceStock(
        float DeltaTime,
        float ServiceThroughputMultiplier = 1.f,
        int ServiceCapacityDelta = 0)
    {
        if (DeltaTime <= 0.f)
            return;

        for (size_t Index = 0; Index < ServiceStocks.size(); ++Index)
        {
            const int EffectiveVisitCap = ResolveEffectiveServiceVisitCap(
                Index,
                ServiceThroughputMultiplier,
                ServiceCapacityDelta);
            const int EffectiveStockCap =
                EffectiveVisitCap *
                GameConstants::Orb::ServiceStockPerCapacity;

            ServiceStockCaps[Index] = EffectiveStockCap;

            if (EffectiveStockCap <= 0)
            {
                ServiceStocks[Index] = 0;
                ServiceStockAccums[Index] = 0.f;
                continue;
            }

            ServiceStocks[Index] = (std::min)(
                ServiceStocks[Index],
                EffectiveStockCap);

            const float RegenRate =
                static_cast<float>(EffectiveVisitCap) *
                GameConstants::Orb::ServiceStockRegenPerCapacityPerSecond *
                GetBudgetSatisfactionScale();

            if (RegenRate <= 0.f)
                continue;

            ServiceStockAccums[Index] =
                (std::min)(
                    static_cast<float>(EffectiveStockCap),
                    ServiceStockAccums[Index] + RegenRate * DeltaTime);
            const int Whole = static_cast<int>(ServiceStockAccums[Index]);

            if (Whole <= 0)
                continue;

            const int AddAmount =
                (std::min)(Whole, EffectiveStockCap - ServiceStocks[Index]);

            if (AddAmount <= 0)
            {
                ServiceStockAccums[Index] = 0.f;
                continue;
            }

            ServiceStocks[Index] += AddAmount;
            ServiceStockAccums[Index] -= static_cast<float>(AddAmount);
        }
    }

    void ConfigureStorageBehavior(bool InUsesWarehouseSlots)
    {
        UsesWarehouseSlots = InUsesWarehouseSlots;
        WarehouseSlotCapacityUnits = WarehouseSlotCapacity;
        WarehouseStoragePolicy = EWarehouseStoragePolicy::Balanced;
        PreferredWarehouseResourceType = EResourceType::None;
        LastDailyStorageLoss = 0;
        StorageLossAccums.fill(0ll);

        if (!UsesWarehouseSlots)
        {
            for (int SlotIndex = 0; SlotIndex < WarehouseSlotCount; ++SlotIndex)
                WarehouseSlotTypes[SlotIndex] = EResourceType::None;
        }
    }

    void ConfigureWarehouseRuntime(
        int InSlotCapacityUnits,
        EWarehouseStoragePolicy InPolicy,
        EResourceType InPreferredResourceType)
    {
        WarehouseSlotCapacityUnits = (std::max)(1000, InSlotCapacityUnits);
        WarehouseStoragePolicy = InPolicy;
        PreferredWarehouseResourceType = InPreferredResourceType;
        CleanupWarehouseSlots();
    }

    int GetWarehouseSlotCapacityUnits() const
    {
        return UsesWarehouseSlots ? WarehouseSlotCapacityUnits : MaxResourceStock;
    }

    EWarehouseStoragePolicy GetWarehouseStoragePolicy() const
    {
        return WarehouseStoragePolicy;
    }

    EResourceType GetPreferredWarehouseResourceType() const
    {
        return PreferredWarehouseResourceType;
    }

    int GetLastDailyStorageLoss() const
    {
        return LastDailyStorageLoss;
    }

    int GetMonthlyWageCost() const
    {
        const float Scaled =
            static_cast<float>(BaseMonthlyWage) *
            GetBudgetSatisfactionScale() *
            (std::max)(0.f, GameConstants::Economy::BuildingWageBaseMultiplier);
        return (std::max)(0, static_cast<int>(roundf(Scaled)));
    }

    int GetMonthlyUpkeepCost() const
    {
        const float Scaled =
            static_cast<float>(BaseMonthlyUpkeep) *
            GetBudgetSatisfactionScale() *
            (std::max)(0.f, GameConstants::Economy::BuildingUpkeepBaseMultiplier);
        return (std::max)(0, static_cast<int>(roundf(Scaled)));
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
        int DaysInMonth,
        float HarborProgressMultiplier = 1.f)
    {
        HarborShipArrivedThisTick = false;

        if (!IsHarbor)
            return false;

        const int SafeDays = (std::max)(1, DaysInMonth);
        const float DailyProgress =
            GetBudgetSatisfactionScale() *
            (std::max)(0.f, HarborProgressMultiplier) /
            static_cast<float>(SafeDays);
        HarborShipProgressMonths += DailyProgress;

        constexpr float GBaseShipIntervalMonths = 3.f;

        if (HarborShipProgressMonths < GBaseShipIntervalMonths)
            return false;

        while (HarborShipProgressMonths >= GBaseShipIntervalMonths)
        {
            HarborShipProgressMonths -= GBaseShipIntervalMonths;
        }

        HarborShipArrivedThisTick = true;
        return true;
    }

    bool GetHarborShipArrivedThisTick(bool IsHarbor) const
    {
        return IsHarbor && HarborShipArrivedThisTick;
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
            GetWarehouseSlotCapacityUnits() * WarehouseSlotCount :
            MaxResourceStock;
    }

    int GetResourceTypeCapacity(EResourceType Type) const
    {
        if (Type == EResourceType::None)
            return 0;

        return UsesWarehouseSlots ? GetWarehouseSlotCapacityUnits() : MaxResourceStock;
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

    bool PreservesWarehouseAssignments() const
    {
        return WarehouseStoragePolicy == EWarehouseStoragePolicy::Dedicated;
    }

    bool HasAssignedPreferredWarehouseSlot() const
    {
        if (!UsesWarehouseSlots ||
            PreferredWarehouseResourceType == EResourceType::None)
        {
            return false;
        }

        for (int SlotIndex = 0; SlotIndex < WarehouseSlotCount; ++SlotIndex)
        {
            const EResourceType SlotType = WarehouseSlotTypes[SlotIndex];

            if (SlotType == EResourceType::None)
                continue;

            if (SlotType == PreferredWarehouseResourceType)
            {
                return true;
            }
        }

        return false;
    }

    bool IsWarehouseSlotReusable(int SlotIndex) const
    {
        if (SlotIndex < 0 || SlotIndex >= WarehouseSlotCount)
            return false;

        const EResourceType SlotType = WarehouseSlotTypes[SlotIndex];

        if (SlotType == EResourceType::None)
            return true;

        if (PreservesWarehouseAssignments())
            return false;

        const size_t ResourceIndex = static_cast<size_t>(SlotType);

        if (ResourceIndex >= ReservedIncomingResourceAmounts.size())
            return false;

        return ResourceInventory.Get(SlotType) <= 0 &&
            ReservedIncomingResourceAmounts[ResourceIndex] <= 0;
    }

    int CountAssignableWarehouseSlots(EResourceType Type) const
    {
        int Count = 0;

        for (int SlotIndex = 0; SlotIndex < WarehouseSlotCount; ++SlotIndex)
        {
            if (IsWarehouseSlotReusable(SlotIndex))
                ++Count;
        }

        if (PreferredWarehouseResourceType == EResourceType::None ||
            Type == PreferredWarehouseResourceType ||
            HasAssignedPreferredWarehouseSlot())
        {
            return Count;
        }

        return (std::max)(0, Count - 1);
    }

    int FindAssignableWarehouseSlot(EResourceType Type) const
    {
        if (CountAssignableWarehouseSlots(Type) <= 0)
            return -1;

        for (int SlotIndex = 0; SlotIndex < WarehouseSlotCount; ++SlotIndex)
        {
            if (IsWarehouseSlotReusable(SlotIndex))
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
                if (PreservesWarehouseAssignments())
                    continue;

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

        const int FreeSlot = FindAssignableWarehouseSlot(Type);

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

        return FindWarehouseSlotByType(Type) >= 0 ||
            CountAssignableWarehouseSlots(Type) > 0;
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

    int GetReservedResourcePickupAmount(EResourceType Type) const
    {
        const size_t Index = static_cast<size_t>(Type);

        if (Index >= ReservedResourcePickupAmounts.size())
            return 0;

        return ReservedResourcePickupAmounts[Index];
    }

    int GetReservedExportPickupAmount() const
    {
        return ReservedExportPickupAmount;
    }

    int GetProductionInputCount() const
    {
        int Count = 0;

        for (int SlotIndex = 0;
             SlotIndex < GProductionInputSlotCount;
             ++SlotIndex)
        {
            if (ProductionInputTypes[SlotIndex] != EResourceType::None &&
                ProductionInputAmounts[SlotIndex] > 0)
            {
                ++Count;
            }
        }

        return Count;
    }

    EResourceType GetProductionInputType(int SlotIndex) const
    {
        if (SlotIndex < 0 || SlotIndex >= GProductionInputSlotCount)
            return EResourceType::None;

        return ProductionInputTypes[SlotIndex];
    }

    int GetProductionInputCompatibleResourceStock(
        EResourceType DemandType) const
    {
        if (DemandType == EResourceType::None ||
            DemandType == EResourceType::Count)
        {
            return 0;
        }

        if (DemandType != EResourceType::FeedCrops)
            return GetResourceStock(DemandType);

        return GetResourceStock(EResourceType::Corn) +
            GetResourceStock(EResourceType::Sugar);
    }

    int GetProductionInputAmount(int SlotIndex) const
    {
        if (SlotIndex < 0 || SlotIndex >= GProductionInputSlotCount)
            return 0;

        return ProductionInputAmounts[SlotIndex];
    }

    bool UsesProductionInputResource(EResourceType Type) const
    {
        if (Type == EResourceType::None)
            return false;

        for (int SlotIndex = 0;
             SlotIndex < GProductionInputSlotCount;
             ++SlotIndex)
        {
            if (ProductionInputTypes[SlotIndex] == Type &&
                ProductionInputAmounts[SlotIndex] > 0)
            {
                return true;
            }
        }

        return false;
    }

    float GetLastProductionEfficiency() const
    {
        return LastProductionEfficiency;
    }

    bool TryConsumeProductionInputCompatibleResource(
        EResourceType DemandType,
        int Amount)
    {
        if (Amount <= 0)
            return true;

        if (DemandType != EResourceType::FeedCrops)
            return TryConsumeResource(DemandType, Amount);

        const int CornStock = GetResourceStock(EResourceType::Corn);
        const int SugarStock = GetResourceStock(EResourceType::Sugar);

        if (CornStock + SugarStock < Amount)
            return false;

        const EResourceType PrimaryType =
            CornStock >= SugarStock ?
                EResourceType::Corn :
                EResourceType::Sugar;
        const EResourceType SecondaryType =
            PrimaryType == EResourceType::Corn ?
                EResourceType::Sugar :
                EResourceType::Corn;
        const int PrimaryAmount = (std::min)(
            Amount,
            GetResourceStock(PrimaryType));

        if (PrimaryAmount > 0 &&
            !TryConsumeResource(PrimaryType, PrimaryAmount))
        {
            return false;
        }

        const int RemainingAmount = Amount - PrimaryAmount;
        return RemainingAmount <= 0 ||
            TryConsumeResource(SecondaryType, RemainingAmount);
    }

    void SetCurrentWorkerOccupancy(int Occupancy)
    {
        CurrentWorkerOccupancy = (std::max)(0, Occupancy);
    }

    int GetCurrentWorkerOccupancy() const
    {
        return (std::max)(0, CurrentWorkerOccupancy);
    }

    void SetWorkingNowOccupancy(int Occupancy)
    {
        WorkingNowOccupancy = (std::max)(0, Occupancy);
    }

    int GetWorkingNowOccupancy() const
    {
        return (std::max)(0, WorkingNowOccupancy);
    }

    int GetServiceVisitCapacity(
        EBuildingServiceType Type,
        float ServiceThroughputMultiplier = 1.f,
        int ServiceCapacityDelta = 0) const
    {
        if (!IsValidServiceType(Type))
            return 0;

        return ResolveEffectiveServiceVisitCap(
            GetServiceIndex(Type),
            ServiceThroughputMultiplier,
            ServiceCapacityDelta);
    }

    int GetActiveServiceVisitorCount(EBuildingServiceType Type) const
    {
        if (!IsValidServiceType(Type))
            return 0;

        return ActiveServiceVisitors[GetServiceIndex(Type)];
    }

    int GetServiceStock(EBuildingServiceType Type) const
    {
        if (!IsValidServiceType(Type))
            return 0;

        return ServiceStocks[GetServiceIndex(Type)];
    }

    bool TryBeginServiceVisit(
        EBuildingServiceType Type,
        float ServiceThroughputMultiplier = 1.f,
        int ServiceCapacityDelta = 0)
    {
        if (!ProvidesService(Type))
            return false;

        const size_t Index = GetServiceIndex(Type);
        const int EffectiveVisitCap = ResolveEffectiveServiceVisitCap(
            Index,
            ServiceThroughputMultiplier,
            ServiceCapacityDelta);

        if (EffectiveVisitCap <= 0 ||
            ActiveServiceVisitors[Index] >= EffectiveVisitCap)
        {
            return false;
        }

        ++ActiveServiceVisitors[Index];
        return true;
    }

    void EndServiceVisit(EBuildingServiceType Type)
    {
        if (!IsValidServiceType(Type))
            return;

        const size_t Index = GetServiceIndex(Type);
        ActiveServiceVisitors[Index] = (std::max)(
            0,
            ActiveServiceVisitors[Index] - 1);
    }

    bool TryConsumeServiceStock(EBuildingServiceType Type, int Amount = 1)
    {
        if (Amount <= 0)
            return true;

        if (!ProvidesService(Type))
            return false;

        if (!ServiceConsumesStock(Type))
            return true;

        const size_t Index = GetServiceIndex(Type);

        if (ServiceStocks[Index] < Amount)
            return false;

        ServiceStocks[Index] -= Amount;
        return true;
    }

    EResourceType ResolvePrimaryResourceTypeForLegacy() const
    {
        if (ProducedResourceType != EResourceType::None)
            return ProducedResourceType;

        if (VisitConsumptionResourceType != EResourceType::None)
            return VisitConsumptionResourceType;

        return EResourceType::None;
    }

    static float GetBaseWarehouseStorageLossPercent(EResourceType Type)
    {
        switch (Type)
        {
        case EResourceType::Fish:
        case EResourceType::FarmedFish:
            return 0.55f;
        case EResourceType::HydroponicProduce:
        case EResourceType::Juice:
            return 0.40f;
        case EResourceType::Banana:
        case EResourceType::Corn:
        case EResourceType::Pineapple:
        case EResourceType::Meat:
        case EResourceType::Milk:
        case EResourceType::Coconuts:
        case EResourceType::Crops:
        case EResourceType::AnimalProducts:
        case EResourceType::FactoryLivestock:
            return 0.30f;
        case EResourceType::BS:
            return 0.12f;
        case EResourceType::Cheese:
            return 0.12f;
        case EResourceType::CannedGoods:
            return 0.05f;
        default:
            break;
        }

        switch (GetResourceMarketClass(Type))
        {
        case EResourceMarketClass::Food:
            return 0.22f;
        case EResourceMarketClass::RawGoods:
            return 0.08f;
        case EResourceMarketClass::ManufacturedGoods:
            return 0.03f;
        case EResourceMarketClass::LuxuryGoods:
            return 0.01f;
        default:
            return 0.f;
        }
    }

    int ApplyDailyWarehouseStorageLoss(float LossMultiplier = 1.f)
    {
        LastDailyStorageLoss = 0;

        if (!UsesWarehouseSlots)
            return 0;

        const float SafeLossMultiplier = (std::max)(0.f, LossMultiplier);

        for (size_t Index = 0; Index < StorageLossAccums.size(); ++Index)
        {
            const EResourceType Type = static_cast<EResourceType>(Index);
            const int CurrentStock = ResourceInventory.Get(Type);

            if (CurrentStock <= 0)
            {
                StorageLossAccums[Index] = 0ll;
                continue;
            }

            const float DailyLossPercent =
                GetBaseWarehouseStorageLossPercent(Type) *
                SafeLossMultiplier;
            const long long ScaledLossRate =
                ResolveScaledStorageLossRate(DailyLossPercent);

            if (ScaledLossRate <= 0)
                continue;

            StorageLossAccums[Index] +=
                static_cast<long long>(CurrentStock) * ScaledLossRate;
            const long long WholeLossLong =
                StorageLossAccums[Index] / StorageLossAccumUnitScale;
            const int WholeLoss = static_cast<int>((std::min)(
                WholeLossLong,
                static_cast<long long>((std::numeric_limits<int>::max)())));

            if (WholeLoss <= 0)
                continue;

            const int AppliedLoss = (std::min)(CurrentStock, WholeLoss);

            if (AppliedLoss <= 0)
                continue;

            if (ResourceInventory.Consume(Type, AppliedLoss))
            {
                StorageLossAccums[Index] -=
                    static_cast<long long>(AppliedLoss) *
                    StorageLossAccumUnitScale;
                LastDailyStorageLoss += AppliedLoss;
            }
        }

        if (LastDailyStorageLoss > 0)
            CleanupWarehouseSlots();

        return LastDailyStorageLoss;
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

    void AddProduction(
        float UnitsPerSec,
        float DeltaTime,
        int CurrentWorkers,
        int WorkerCapacity,
        float ProductionMultiplier = 1.f,
        float InputConsumptionMultiplier = 1.f)
    {
        const int SafeCurrentWorkers = (std::max)(0, CurrentWorkers);
        const int SafeWorkerCapacity = (std::max)(0, WorkerCapacity);

        if (SafeCurrentWorkers <= 0 || SafeWorkerCapacity <= 0)
        {
            LastProductionEfficiency = 0.f;
            return;
        }

        const float WorkforceCoverage = (std::min)(
            1.f,
            static_cast<float>(SafeCurrentWorkers) /
                static_cast<float>(SafeWorkerCapacity));
        const float EffectiveOutputPerSec =
            UnitsPerSec *
            (std::max)(0.f, ProductionMultiplier) *
            WorkforceCoverage;

        if (EffectiveOutputPerSec <= 0.f ||
            ProducedResourceType == EResourceType::None)
        {
            LastProductionEfficiency = 0.f;
            return;
        }

        const float EffectiveInputMultiplier =
            (std::max)(0.f, InputConsumptionMultiplier);
        const int InputCount = GetProductionInputCount();

        const int OutputCapacityLeft = (std::max)(
            0,
            GetResourceTypeCapacity(ProducedResourceType) -
                GetResourceStock(ProducedResourceType));

        if (OutputCapacityLeft <= 0)
        {
            ClampResourceProductionAccumToFractional();
            LastProductionEfficiency = 0.f;
            return;
        }

        int MaxUnitsByInputs = (std::numeric_limits<int>::max)();

        if (InputCount > 0)
        {
            for (int SlotIndex = 0;
                 SlotIndex < GProductionInputSlotCount;
                 ++SlotIndex)
            {
                const EResourceType InputType =
                    ProductionInputTypes[SlotIndex];
                const int InputAmount = ProductionInputAmounts[SlotIndex];
                const float EffectiveInputAmount =
                    static_cast<float>(InputAmount) *
                    EffectiveInputMultiplier;

                if (InputType == EResourceType::None ||
                    InputAmount <= 0 ||
                    EffectiveInputAmount <= 0.f)
                {
                    continue;
                }

                MaxUnitsByInputs = (std::min)(
                    MaxUnitsByInputs,
                    static_cast<int>(floorf(
                        static_cast<float>(
                            GetProductionInputCompatibleResourceStock(
                                InputType)) /
                        EffectiveInputAmount)));
            }

            if (MaxUnitsByInputs <= 0)
            {
                ClampResourceProductionAccumToFractional();
                LastProductionEfficiency = 0.f;
                return;
            }
        }

        LastProductionEfficiency =
            WorkforceCoverage * (std::max)(0.f, ProductionMultiplier);
        ResourceProductionAccumScaled = (std::min)(
            ResourceProductionAccumScaled +
                ResolveScaledProductionUnits(
                    static_cast<double>(EffectiveOutputPerSec) *
                    static_cast<double>(DeltaTime)),
            ResolveScaledProductionBufferCap());
        const int WholeRequested = static_cast<int>((std::min)(
            ResourceProductionAccumScaled / ProductionAccumScale,
            static_cast<long long>((std::numeric_limits<int>::max)())));

        if (WholeRequested <= 0)
            return;

        int Whole = WholeRequested;
        Whole = (std::min)(Whole, MaxUnitsByInputs);
        Whole = (std::min)(Whole, OutputCapacityLeft);

        const float InputCoverage =
            WholeRequested > 0 ?
                (std::max)(
                    0.f,
                    (std::min)(
                        1.f,
                        static_cast<float>(Whole) /
                            static_cast<float>(WholeRequested))) :
                1.f;
        LastProductionEfficiency =
            WorkforceCoverage * InputCoverage * (std::max)(0.f, ProductionMultiplier);

        if (Whole <= 0)
        {
            ClampResourceProductionAccumToFractional();
            LastProductionEfficiency = 0.f;
            return;
        }

        for (int SlotIndex = 0;
             SlotIndex < GProductionInputSlotCount;
             ++SlotIndex)
        {
            const EResourceType InputType =
                ProductionInputTypes[SlotIndex];
            const int InputAmount = ProductionInputAmounts[SlotIndex];
            const float EffectiveInputAmount =
                static_cast<float>(InputAmount) *
                EffectiveInputMultiplier;
            const int RequiredAmount = static_cast<int>(roundf(
                static_cast<float>(Whole) * EffectiveInputAmount));

            if (InputType == EResourceType::None ||
                InputAmount <= 0 ||
                EffectiveInputAmount <= 0.f ||
                RequiredAmount <= 0)
            {
                continue;
            }

            if (!TryConsumeProductionInputCompatibleResource(
                    InputType,
                    RequiredAmount))
            {
                ClampResourceProductionAccumToFractional();
                LastProductionEfficiency = 0.f;
                return;
            }
        }

        ResourceProductionAccumScaled = (std::max)(
            0ll,
            ResourceProductionAccumScaled -
                static_cast<long long>(Whole) * ProductionAccumScale);
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
