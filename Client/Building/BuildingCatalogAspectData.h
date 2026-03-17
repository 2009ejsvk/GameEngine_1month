#pragma once

#include "BuildingCatalogTypes.h"
#include "BuildingTypes.h"
#include "../Politics/PoliticalSignalTypes.h"
#include <array>
#include <string>
#include <vector>

struct FBuildingOperationModeEffect
{
    bool HasProducedResourceTypeOverride = false;
    EResourceType ProducedResourceTypeOverride = EResourceType::None;
    // Input-only modes are valid runtime recipe overrides. Keep this separate
    // from ProducedResourceTypeOverride so buildings like Juicery or
    // Pharmaceutical Company can swap inputs while keeping the same output.
    bool HasProductionInputTypesOverride = false;
    std::array<EResourceType, GProductionInputSlotCount>
        ProductionInputTypesOverride = {};
    std::array<int, GProductionInputSlotCount>
        ProductionInputAmountsOverride = {};
    bool HasVisitConsumptionTypeOverride = false;
    EResourceType VisitConsumptionTypeOverride = EResourceType::None;
    bool HasVisitConsumptionAcceptedTypesOverride = false;
    std::vector<EResourceType> VisitConsumptionAcceptedTypesOverride;
    float ProductionMultiplier = 1.f;
    float InputConsumptionMultiplier = 1.f;
    float ServiceThroughputMultiplier = 1.f;
    float HarborProgressMultiplier = 1.f;
    float TeamsterTransferMultiplier = 1.f;
    float ProducedPowerMultiplier = 1.f;
    float RequiredPowerMultiplier = 1.f;
    float WarehouseSlotCapacityMultiplier = 1.f;
    float StorageLossMultiplier = 1.f;
    float PollutionMultiplier = 1.f;
    float WageMultiplier = 1.f;
    float UpkeepMultiplier = 1.f;
    int TeamsterCargoLossPercent = 0;
    int ExportTradeRoutePriceDeltaPercent = 0;
    int ImportTradeRoutePriceDeltaPercent = 0;
    int ProducedPowerDeltaMW = 0;
    int RequiredPowerDeltaMW = 0;
    int WarehouseSlotCapacityDelta = 0;
    int PollutionFlatDelta = 0;
    int WageFlatDelta = 0;
    int UpkeepFlatDelta = 0;
    int CapacityDelta = 0;
    int ServiceCapacityDelta = 0;
    int PerWorkerServiceCapacityDelta = 0;
    int HousingQualityDelta = 0;
    int JobQualityDelta = 0;
    int GenericServiceQualityDelta = 0;
    float HousingQualityMultiplier = 1.f;
    float JobQualityMultiplier = 1.f;
    float GenericServiceQualityMultiplier = 1.f;

    bool HasProductionRecipeOverride() const
    {
        return (HasProducedResourceTypeOverride &&
                ProducedResourceTypeOverride != EResourceType::None) ||
            HasProductionInputTypesOverride;
    }

    bool HasVisitConsumptionOverride() const
    {
        return HasVisitConsumptionTypeOverride ||
            HasVisitConsumptionAcceptedTypesOverride;
    }

    bool HasRuntimeEffect() const
    {
        return HasProductionRecipeOverride() ||
            HasVisitConsumptionOverride() ||
            ProductionMultiplier != 1.f ||
            InputConsumptionMultiplier != 1.f ||
            ServiceThroughputMultiplier != 1.f ||
            HarborProgressMultiplier != 1.f ||
            TeamsterTransferMultiplier != 1.f ||
            ProducedPowerMultiplier != 1.f ||
            RequiredPowerMultiplier != 1.f ||
            WarehouseSlotCapacityMultiplier != 1.f ||
            StorageLossMultiplier != 1.f ||
            PollutionMultiplier != 1.f ||
            WageMultiplier != 1.f ||
            UpkeepMultiplier != 1.f ||
            TeamsterCargoLossPercent != 0 ||
            ExportTradeRoutePriceDeltaPercent != 0 ||
            ImportTradeRoutePriceDeltaPercent != 0 ||
            ProducedPowerDeltaMW != 0 ||
            RequiredPowerDeltaMW != 0 ||
            WarehouseSlotCapacityDelta != 0 ||
            PollutionFlatDelta != 0 ||
            WageFlatDelta != 0 ||
            UpkeepFlatDelta != 0 ||
            CapacityDelta != 0 ||
            ServiceCapacityDelta != 0 ||
            PerWorkerServiceCapacityDelta != 0 ||
            HousingQualityDelta != 0 ||
            JobQualityDelta != 0 ||
            GenericServiceQualityDelta != 0 ||
            HousingQualityMultiplier != 1.f ||
            JobQualityMultiplier != 1.f ||
            GenericServiceQualityMultiplier != 1.f;
    }
};

struct FBuildingOperationModeDef
{
    std::wstring DisplayName;
    std::wstring EffectSummary;
    bool HasUnlockEra = false;
    EBuildingEra UnlockEra = EBuildingEra::Colonial;
    std::wstring RequiredResearch;
    FBuildingOperationModeEffect Effect;
};

struct FBuildingRuntimeUpgradeDef
{
    std::wstring DisplayName;
    std::wstring EffectSummary;
    bool HasUnlockEra = false;
    EBuildingEra UnlockEra = EBuildingEra::Colonial;
    EBuildingCostState CostState = EBuildingCostState::None;
    int Cost = 0;
    FBuildingOperationModeEffect Effect;
};

struct FBuildingCatalogRuntimeData
{
    EResourceType ProducedResourceType = EResourceType::None;
    std::wstring ProducedResourceLabel;
    EResourceType VisitConsumptionResourceType = EResourceType::None;
    std::vector<EResourceType> VisitConsumptionAcceptedResourceTypes;
    std::array<EResourceType, GProductionInputSlotCount>
        ProductionInputTypes = {};
    std::array<int, GProductionInputSlotCount> ProductionInputAmounts = {};
    std::array<std::wstring, GProductionInputSlotCount>
        ProductionInputLabels = {};
    EBuildingProductionChainStage ProductionChainStage =
        EBuildingProductionChainStage::None;
    std::wstring SupplyChainSummary;
    bool UsesRecipeTable = false;
    bool SupportsTeamsterPickup = false;
    bool CanExportStoredResources = false;
    int BaseProducedPowerMW = 0;
    int BaseRequiredPowerMW = 0;
    int BasePollutionOutput = 0;
    int BasePollutionMitigation = 0;
    std::vector<FBuildingOperationModeDef> OperationModeDefs;
    std::vector<FBuildingRuntimeUpgradeDef> RuntimeUpgradeDefs;
};

struct FBuildingCatalogCitizenData
{
    EBuildingCategory Category = EBuildingCategory::Infrastructure;
    int HouseholdCapacity = 0;
    EBuildingLeisureClass LeisureClass = EBuildingLeisureClass::None;
    ETouristPreference PrimaryTouristPreference = ETouristPreference::None;
};

struct FBuildingCatalogPoliticalData
{
    std::vector<FPoliticalSignalDef> PoliticalSignals;
};
