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

    int BudgetLevel = 3;
    int BaseMonthlyWage = 0;
    int BaseMonthlyUpkeep = 0;
    FResourceInventory ResourceInventory;
    EResourceType ProducedResourceType = EResourceType::None;
    EResourceType VisitConsumptionResourceType = EResourceType::None;
    bool SupportsTeamsterPickup = false;
    bool CanExportStoredResources = false;
    float ResourceProductionAccum = 0.f;
    float HarborShipProgressMonths = 0.f;

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
        ResourceInventory.Add(Type, Amount, MaxResourceStock);
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
        return ResourceInventory.Consume(Type, Amount);
    }

    bool TryConsumeAnyExportableResource(
        int Amount,
        EResourceType& OutType)
    {
        return ResourceInventory.ConsumeAnyExportable(Amount, OutType);
    }

    bool TryConsumeExportableResources(int Amount)
    {
        return ResourceInventory.ConsumeExportableAmount(Amount);
    }
};
