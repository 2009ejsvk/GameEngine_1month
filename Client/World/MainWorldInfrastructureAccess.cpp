#include "MainWorld.h"
#include "MainWorldInfrastructureAccess.h"

class CMainWorldInfrastructureReadAccess final :
    public IMainWorldRoadNetworkAccess,
    public IMainWorldTransitAccess
{
public:
    explicit CMainWorldInfrastructureReadAccess(CMainWorld* Owner)
        : mOwner(Owner)
    {
    }

    void RebuildRoadNetwork() override
    {
        mOwner->RebuildRoadNetwork();
    }

    const CRoadNetwork* GetRoadNetwork() const override
    {
        return mOwner->GetRoadNetwork();
    }

    const CBusRouteSystem* GetBusRouteSystem() const override
    {
        return mOwner->GetBusRouteSystem();
    }

private:
    CMainWorld* mOwner = nullptr;
};

std::shared_ptr<CMainWorldInfrastructureReadAccess>
    CreateMainWorldInfrastructureReadAccessAdapter(CMainWorld* Owner)
{
    return std::shared_ptr<CMainWorldInfrastructureReadAccess>(
        new CMainWorldInfrastructureReadAccess(Owner));
}

std::shared_ptr<IMainWorldRoadNetworkAccess>
    CMainWorld::GetRoadNetworkAccessHandle() const
{
    return std::static_pointer_cast<IMainWorldRoadNetworkAccess>(
        mAccess.InfrastructureRead);
}

IMainWorldRoadNetworkAccess* CMainWorld::GetRoadNetworkAccessRaw() const
{
    return static_cast<IMainWorldRoadNetworkAccess*>(
        mAccess.InfrastructureRead.get());
}

std::shared_ptr<IMainWorldTransitAccess> CMainWorld::GetTransitAccessHandle()
    const
{
    return std::static_pointer_cast<IMainWorldTransitAccess>(
        mAccess.InfrastructureRead);
}

IMainWorldTransitAccess* CMainWorld::GetTransitAccessRaw() const
{
    return static_cast<IMainWorldTransitAccess*>(
        mAccess.InfrastructureRead.get());
}
