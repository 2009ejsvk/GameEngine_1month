#pragma once

#include "PlacementAreaComponents.h"

struct FPlacementBuildingRoleState
{
    bool Warehouse = false;
    bool BusGarage = false;
    bool BusStop = false;
};

enum class EBuildingDamageLevel
{
    None = 0,
    Damaged,
    Critical
};

struct FPlacementAreaRuntimeState
{
    int ActiveOperationModeIndex = 0;
    int ActiveRuntimeUpgradeIndex = -1;
    EWarehouseStoragePolicy WarehouseStoragePolicy =
        EWarehouseStoragePolicy::Balanced;
    EResourceType PreferredWarehouseResourceType =
        EResourceType::None;
    float PowerSupplyRatio = 1.f;
    int LocalPollutionExposure = 0;
    int LocalFreedomSupport = 0;
    int LocalSecuritySupport = 0;
    float AccessibilityScore = 0.f;
    EBuildingDamageLevel DamageLevel = EBuildingDamageLevel::None;
    int RepairCost = 0;
    FPlacementBuildingRoleState Roles;

    void ResetForCatalog(const FPlacementBuildingRoleState& InRoles)
    {
        Roles = InRoles;
        ActiveOperationModeIndex = 0;
        ActiveRuntimeUpgradeIndex = -1;
        WarehouseStoragePolicy = EWarehouseStoragePolicy::Balanced;
        PreferredWarehouseResourceType = EResourceType::None;
        PowerSupplyRatio = 1.f;
        LocalPollutionExposure = 0;
        LocalFreedomSupport = 0;
        LocalSecuritySupport = 0;
        AccessibilityScore = 0.f;
        DamageLevel = EBuildingDamageLevel::None;
        RepairCost = 0;
    }
};
