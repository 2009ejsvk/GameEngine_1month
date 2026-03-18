#pragma once

#include "../Building/BuildingTypes.h"
#include <algorithm>
#include <array>
#include <string>

namespace TradePolicy
{
    constexpr int GDefaultHarborExportShipCapacityUnits = 8000;
    constexpr int GDefaultHarborImportPerResourceUnits = 1500;
    constexpr int GDefaultDailyImportBudgetCap = 0;
    constexpr double GEmergencyImportCostMultiplier = 1.5;
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

    inline EResourceType GetFirstImportableResourceType()
    {
        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (IsExportableResourceType(ResourceType) &&
                IsImmediateProductionScopeResourceType(ResourceType))
            {
                return ResourceType;
            }
        }

        return EResourceType::None;
    }

    inline bool IsPolicyResourceBundleSelectionType(EResourceType Type)
    {
        // Legacy save compatibility only. New UI cycling should stay on exact
        // exportable resources and never land on these family aliases.
        switch (Type)
        {
        case EResourceType::Crops:
        case EResourceType::AnimalProducts:
        case EResourceType::Ore:
        case EResourceType::HydroponicProduce:
        case EResourceType::FactoryLivestock:
            return true;
        default:
            return false;
        }
    }

    template <typename TVisitor>
    inline void ForEachPolicySelectionResourceType(
        EResourceType SelectionType,
        const TVisitor& Visitor);

    inline EResourceType GetFirstExactResourceTypeInPolicySelection(
        EResourceType SelectionType)
    {
        if (IsExportableResourceType(SelectionType) &&
            IsImmediateProductionScopeResourceType(SelectionType))
        {
            return SelectionType;
        }

        EResourceType FirstExactType = EResourceType::None;
        ForEachPolicySelectionResourceType(
            SelectionType,
            [&](EResourceType CandidateType)
            {
                if (FirstExactType == EResourceType::None &&
                    IsExportableResourceType(CandidateType) &&
                    IsImmediateProductionScopeResourceType(CandidateType))
                {
                    FirstExactType = CandidateType;
                }
            });
        return FirstExactType;
    }

    template <typename TVisitor>
    inline void ForEachPolicySelectionResourceType(
        EResourceType SelectionType,
        const TVisitor& Visitor)
    {
        if (SelectionType == EResourceType::None ||
            SelectionType == EResourceType::Count)
        {
            return;
        }

        switch (SelectionType)
        {
        case EResourceType::Crops:
        case EResourceType::HydroponicProduce:
            ForEachCropOutputResourceType(Visitor);
            return;
        case EResourceType::AnimalProducts:
        case EResourceType::FactoryLivestock:
            ForEachAnimalOutputResourceType(Visitor);
            return;
        case EResourceType::Ore:
            ForEachMineOutputResourceType(Visitor);
            return;
        default:
            if (IsExportableResourceType(SelectionType) &&
                IsImmediateProductionScopeResourceType(SelectionType))
            {
                Visitor(SelectionType);
            }
            return;
        }
    }

    inline bool DoesPolicySelectionMatchResourceType(
        EResourceType SelectionType,
        EResourceType ResourceType)
    {
        if (!IsExportableResourceType(ResourceType) ||
            !IsImmediateProductionScopeResourceType(ResourceType))
        {
            return false;
        }

        bool Matches = false;
        ForEachPolicySelectionResourceType(
            SelectionType,
            [&](EResourceType CandidateType)
            {
                if (CandidateType == ResourceType)
                    Matches = true;
            });
        return Matches;
    }

    inline std::wstring BuildImportPolicySelectionDisplayText(
        const FImportTradePolicy& Policy)
    {
        switch (Policy.Mode)
        {
        case EImportPolicyMode::None:
            return L"없음";
        case EImportPolicyMode::SingleResource:
            if (Policy.SelectedResourceType == EResourceType::None)
                return L"없음";

            if (IsPolicyResourceBundleSelectionType(
                    Policy.SelectedResourceType))
            {
                return std::wstring(
                           GetResourceTypeDisplayName(
                               Policy.SelectedResourceType)) +
                    L" 묶음";
            }

            {
                const EResourceType VisibleType =
                    GetFirstExactResourceTypeInPolicySelection(
                        Policy.SelectedResourceType);
                return VisibleType != EResourceType::None ?
                    std::wstring(GetResourceTypeDisplayName(VisibleType)) :
                    L"없음";
            }
        case EImportPolicyMode::AllResources:
        default:
            return L"전체";
        }
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
                !IsImmediateProductionScopeResourceType(ResourceType) ||
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
        if (!IsExportableResourceType(CurrentType) ||
            !IsImmediateProductionScopeResourceType(CurrentType))
        {
            return GetFirstImportableResourceType();
        }

        for (int ResourceIndex = static_cast<int>(CurrentType) + 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (IsExportableResourceType(ResourceType) &&
                IsImmediateProductionScopeResourceType(ResourceType))
            {
                return ResourceType;
            }
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

            if (!IsExportableResourceType(ResourceType) ||
                !IsImmediateProductionScopeResourceType(ResourceType))
            {
                continue;
            }

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
        if (!IsExportableResourceType(Type) ||
            !IsImmediateProductionScopeResourceType(Type))
        {
            return false;
        }

        switch (Policy.Mode)
        {
        case EImportPolicyMode::None:
            return false;
        case EImportPolicyMode::SingleResource:
            return DoesPolicySelectionMatchResourceType(
                Policy.SelectedResourceType,
                Type);
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
