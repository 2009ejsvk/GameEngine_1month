#pragma once

#include "../Map/PlacementAreaRuntimeState.h"
#include <memory>
#include <string>

class CMainWorld;
class CWorld;
class CMainWorldBuildingControlAccess;

class IMainWorldRuntimeRefreshAccess
{
public:
    virtual ~IMainWorldRuntimeRefreshAccess() = default;

    virtual void RefreshRuntimeBuildingState() = 0;
};

class IMainWorldBuildingConditionAccess
{
public:
    virtual ~IMainWorldBuildingConditionAccess() = default;

    virtual bool DamageBuilding(
        const std::string& BuildingName,
        EBuildingDamageLevel Level) = 0;
    virtual bool TryRepairBuilding(
        const std::string& BuildingName,
        std::wstring& OutMessage) = 0;
};

std::shared_ptr<CMainWorldBuildingControlAccess>
    CreateMainWorldBuildingControlAccessAdapter(CMainWorld* Owner);

std::shared_ptr<IMainWorldRuntimeRefreshAccess>
    ResolveMainWorldRuntimeRefreshAccess(
        const std::shared_ptr<CWorld>& World);
IMainWorldRuntimeRefreshAccess* ResolveMainWorldRuntimeRefreshAccess(
    CWorld* World);

std::shared_ptr<IMainWorldBuildingConditionAccess>
    ResolveMainWorldBuildingConditionAccess(
        const std::shared_ptr<CWorld>& World);
IMainWorldBuildingConditionAccess* ResolveMainWorldBuildingConditionAccess(
    CWorld* World);
