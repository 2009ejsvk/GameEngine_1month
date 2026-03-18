#pragma once

#include "World/WorldSubsystem.h"
#include "../Map/PlacementAreaRuntimeState.h"
#include <string>

class CBuildingSubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    void RefreshRuntimeBuildingState();
    void RefreshBuildingRepairCosts();
    bool DamageBuilding(
        const std::string& BuildingName,
        EBuildingDamageLevel Level);
    bool TryRepairBuilding(
        const std::string& BuildingName,
        std::wstring& OutMessage);
};
