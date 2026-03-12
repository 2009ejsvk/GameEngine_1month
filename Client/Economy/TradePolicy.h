#pragma once

#include "../Building/BuildingTypes.h"
#include <algorithm>
#include <array>

namespace TradePolicy
{
    constexpr int GDefaultHarborExportShipCapacityUnits = 8000;
    constexpr int GDefaultDomesticReserveBufferUnits = 1000;
    constexpr int GDefaultHarborImportPerResourceUnits = 1500;
    constexpr int GDefaultDailyImportBudgetCap = 0;
    constexpr double GEmergencyImportCostMultiplier = 1.5;
    constexpr std::array<int, 4> GDomesticReserveBufferPresets =
    {
        0,
        1000,
        3000,
        6000
    };
    constexpr std::array<int, 4> GImportPerResourceCapPresets =
    {
        500,
        1500,
        3000,
        6000
    };

    enum class EImportPolicyMode
    {
        AllResources,
        None,
        SingleResource
    };

    struct FImportBudgetPreset
    {
        int DailyBudgetCap = 0;
        bool AllowEmergencyImports = false;
    };

    constexpr std::array<FImportBudgetPreset, 4> GImportBudgetPresets =
    {{
        { 0, false },
        { 12000, false },
        { 24000, false },
        { 36000, true }
    }};

    struct FExportTradePolicy
    {
        bool PrioritizeHighValueCargo = true;
        int HarborExportShipCapacityUnits =
            GDefaultHarborExportShipCapacityUnits;
        int DomesticReserveBufferUnits =
            GDefaultDomesticReserveBufferUnits;
        std::array<bool, static_cast<size_t>(EResourceType::Count)>
            ExportAllowedByType = []()
            {
                std::array<bool, static_cast<size_t>(EResourceType::Count)>
                    Result = {};
                Result.fill(true);
                Result[0] = false;
                return Result;
            }();
    };

    struct FImportTradePolicy
    {
        EImportPolicyMode Mode = EImportPolicyMode::AllResources;
        EResourceType SelectedResourceType = EResourceType::None;
        int MaxUnitsPerResourcePerShip =
            GDefaultHarborImportPerResourceUnits;
        int DailyBudgetCap = GDefaultDailyImportBudgetCap;
        bool AllowEmergencyImports = false;
    };

    inline bool IsResourceExportAllowed(
        const FExportTradePolicy& Policy,
        EResourceType Type)
    {
        if (!IsExportableResourceType(Type))
            return false;

        const size_t ResourceIndex = static_cast<size_t>(Type);

        if (ResourceIndex >= Policy.ExportAllowedByType.size())
            return false;

        return Policy.ExportAllowedByType[ResourceIndex];
    }

    inline int GetHarborExportShipCapacityUnits(
        const FExportTradePolicy& Policy)
    {
        return (std::max)(0, Policy.HarborExportShipCapacityUnits);
    }

    inline int GetDomesticReserveBufferUnits(
        const FExportTradePolicy& Policy)
    {
        return (std::max)(0, Policy.DomesticReserveBufferUnits);
    }

    inline void AdvanceDomesticReservePolicySelection(
        FExportTradePolicy& InOutPolicy)
    {
        const int CurrentUnits =
            GetDomesticReserveBufferUnits(InOutPolicy);

        for (size_t Index = 0;
            Index < GDomesticReserveBufferPresets.size();
            ++Index)
        {
            if (GDomesticReserveBufferPresets[Index] != CurrentUnits)
                continue;

            const size_t NextIndex =
                (Index + 1) % GDomesticReserveBufferPresets.size();
            InOutPolicy.DomesticReserveBufferUnits =
                GDomesticReserveBufferPresets[NextIndex];
            return;
        }

        InOutPolicy.DomesticReserveBufferUnits =
            GDomesticReserveBufferPresets[0];
    }

    inline EResourceType GetFirstImportableResourceType()
    {
        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (IsExportableResourceType(ResourceType))
                return ResourceType;
        }

        return EResourceType::None;
    }

    inline int GetImportMaxUnitsPerResource(
        const FImportTradePolicy& Policy)
    {
        return (std::max)(0, Policy.MaxUnitsPerResourcePerShip);
    }

    inline int GetDailyImportBudgetCap(
        const FImportTradePolicy& Policy)
    {
        return (std::max)(0, Policy.DailyBudgetCap);
    }

    inline bool AllowsEmergencyImports(
        const FImportTradePolicy& Policy)
    {
        return Policy.AllowEmergencyImports;
    }

    inline void AdvanceImportResourceCapSelection(
        FImportTradePolicy& InOutPolicy)
    {
        const int CurrentCapUnits =
            GetImportMaxUnitsPerResource(InOutPolicy);

        for (size_t Index = 0;
            Index < GImportPerResourceCapPresets.size();
            ++Index)
        {
            if (GImportPerResourceCapPresets[Index] != CurrentCapUnits)
                continue;

            const size_t NextIndex =
                (Index + 1) % GImportPerResourceCapPresets.size();
            InOutPolicy.MaxUnitsPerResourcePerShip =
                GImportPerResourceCapPresets[NextIndex];
            return;
        }

        InOutPolicy.MaxUnitsPerResourcePerShip =
            GImportPerResourceCapPresets[0];
    }

    inline void AdvanceImportBudgetSelection(
        FImportTradePolicy& InOutPolicy)
    {
        const int CurrentBudgetCap =
            GetDailyImportBudgetCap(InOutPolicy);
        const bool CurrentAllowEmergency =
            AllowsEmergencyImports(InOutPolicy);

        for (size_t Index = 0;
            Index < GImportBudgetPresets.size();
            ++Index)
        {
            if (GImportBudgetPresets[Index].DailyBudgetCap !=
                    CurrentBudgetCap ||
                GImportBudgetPresets[Index].AllowEmergencyImports !=
                    CurrentAllowEmergency)
            {
                continue;
            }

            const size_t NextIndex =
                (Index + 1) % GImportBudgetPresets.size();
            InOutPolicy.DailyBudgetCap =
                GImportBudgetPresets[NextIndex].DailyBudgetCap;
            InOutPolicy.AllowEmergencyImports =
                GImportBudgetPresets[NextIndex].AllowEmergencyImports;
            return;
        }

        InOutPolicy.DailyBudgetCap =
            GImportBudgetPresets[0].DailyBudgetCap;
        InOutPolicy.AllowEmergencyImports =
            GImportBudgetPresets[0].AllowEmergencyImports;
    }

    inline EResourceType GetFirstBlockedExportResourceType(
        const FExportTradePolicy& Policy)
    {
        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsExportableResourceType(ResourceType) ||
                IsResourceExportAllowed(Policy, ResourceType))
            {
                continue;
            }

            return ResourceType;
        }

        return EResourceType::None;
    }

    inline EResourceType GetNextImportableResourceType(
        EResourceType CurrentType)
    {
        for (int ResourceIndex = static_cast<int>(CurrentType) + 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (IsExportableResourceType(ResourceType))
                return ResourceType;
        }

        return EResourceType::None;
    }

    inline EResourceType GetNextExportableResourceType(
        EResourceType CurrentType)
    {
        return GetNextImportableResourceType(CurrentType);
    }

    inline void SetAllExportableResourcesAllowed(
        FExportTradePolicy& InOutPolicy,
        bool Allowed)
    {
        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsExportableResourceType(ResourceType))
                continue;

            InOutPolicy.ExportAllowedByType[
                static_cast<size_t>(ResourceType)] = Allowed;
        }
    }

    inline void AdvanceExportBlockedResourceSelection(
        FExportTradePolicy& InOutPolicy)
    {
        const EResourceType CurrentBlockedType =
            GetFirstBlockedExportResourceType(InOutPolicy);
        const EResourceType NextBlockedType =
            CurrentBlockedType == EResourceType::None ?
                GetFirstImportableResourceType() :
                GetNextExportableResourceType(CurrentBlockedType);

        SetAllExportableResourcesAllowed(InOutPolicy, true);

        if (NextBlockedType == EResourceType::None)
            return;

        InOutPolicy.ExportAllowedByType[
            static_cast<size_t>(NextBlockedType)] = false;
    }

    inline bool IsResourceImportAllowed(
        const FImportTradePolicy& Policy,
        EResourceType Type)
    {
        if (!IsExportableResourceType(Type))
            return false;

        switch (Policy.Mode)
        {
        case EImportPolicyMode::None:
            return false;
        case EImportPolicyMode::SingleResource:
            return Type == Policy.SelectedResourceType;
        case EImportPolicyMode::AllResources:
        default:
            return true;
        }
    }

    inline void AdvanceImportPolicySelection(FImportTradePolicy& InOutPolicy)
    {
        switch (InOutPolicy.Mode)
        {
        case EImportPolicyMode::AllResources:
            InOutPolicy.Mode = EImportPolicyMode::None;
            InOutPolicy.SelectedResourceType = EResourceType::None;
            return;
        case EImportPolicyMode::None:
            InOutPolicy.Mode = EImportPolicyMode::SingleResource;
            InOutPolicy.SelectedResourceType =
                GetFirstImportableResourceType();
            return;
        case EImportPolicyMode::SingleResource:
        default:
            break;
        }

        const EResourceType NextResourceType =
            GetNextImportableResourceType(InOutPolicy.SelectedResourceType);

        if (NextResourceType == EResourceType::None)
        {
            InOutPolicy.Mode = EImportPolicyMode::AllResources;
            InOutPolicy.SelectedResourceType = EResourceType::None;
            return;
        }

        InOutPolicy.Mode = EImportPolicyMode::SingleResource;
        InOutPolicy.SelectedResourceType = NextResourceType;
    }
}
