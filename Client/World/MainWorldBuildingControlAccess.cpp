#include "MainWorld.h"
#include "MainWorldBuildingControlAccess.h"

class CMainWorldBuildingControlAccess final :
    public IMainWorldRuntimeRefreshAccess,
    public IMainWorldBuildingConditionAccess
{
public:
    explicit CMainWorldBuildingControlAccess(CMainWorld* Owner)
        : mOwner(Owner)
    {
    }

    void RefreshRuntimeBuildingState() override
    {
        mOwner->RefreshRuntimeBuildingState();
    }

    bool DamageBuilding(
        const std::string& BuildingName,
        EBuildingDamageLevel Level) override
    {
        return mOwner->DamageBuilding(BuildingName, Level);
    }

    bool TryRepairBuilding(
        const std::string& BuildingName,
        std::wstring& OutMessage) override
    {
        return mOwner->TryRepairBuilding(BuildingName, OutMessage);
    }

private:
    CMainWorld* mOwner = nullptr;
};

std::shared_ptr<CMainWorldBuildingControlAccess>
    CreateMainWorldBuildingControlAccessAdapter(CMainWorld* Owner)
{
    return std::shared_ptr<CMainWorldBuildingControlAccess>(
        new CMainWorldBuildingControlAccess(Owner));
}

std::shared_ptr<IMainWorldRuntimeRefreshAccess>
    CMainWorld::GetRuntimeRefreshAccessHandle() const
{
    return std::static_pointer_cast<IMainWorldRuntimeRefreshAccess>(
        mAccess.BuildingControl);
}

IMainWorldRuntimeRefreshAccess* CMainWorld::GetRuntimeRefreshAccessRaw() const
{
    return static_cast<IMainWorldRuntimeRefreshAccess*>(
        mAccess.BuildingControl.get());
}

std::shared_ptr<IMainWorldBuildingConditionAccess>
    CMainWorld::GetBuildingConditionAccessHandle() const
{
    return std::static_pointer_cast<IMainWorldBuildingConditionAccess>(
        mAccess.BuildingControl);
}

IMainWorldBuildingConditionAccess*
    CMainWorld::GetBuildingConditionAccessRaw() const
{
    return static_cast<IMainWorldBuildingConditionAccess*>(
        mAccess.BuildingControl.get());
}
