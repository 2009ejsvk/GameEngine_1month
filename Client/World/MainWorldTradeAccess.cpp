#include "MainWorld.h"
#include "MainWorldTradeAccess.h"

class CMainWorldTradeAccessGroup final :
    public IMainWorldTradeAccess
{
public:
    explicit CMainWorldTradeAccessGroup(CMainWorld* Owner)
        : mOwner(Owner)
    {
    }

    EBuildingEra GetCurrentEra() const override
    {
        return mOwner->GetCurrentEra();
    }

    const std::vector<FTradeRouteRuntimeState>&
        GetActiveTradeRoutes() const override
    {
        return mOwner->GetActiveTradeRoutes();
    }

    const std::vector<FTradeRouteCompletionRecord>&
        GetCompletedTradeRoutes() const override
    {
        return mOwner->GetCompletedTradeRoutes();
    }

    int GetTradeRouteCompletionNotificationVersion() const override
    {
        return mOwner->GetTradeRouteCompletionNotificationVersion();
    }

    int GetCustomsExportTradePriceModifierPercent() const override
    {
        return mOwner->GetCustomsExportTradePriceModifierPercent();
    }

    int GetCustomsImportTradePriceModifierPercent() const override
    {
        return mOwner->GetCustomsImportTradePriceModifierPercent();
    }

    const std::array<
        TradeDiplomacyRuntime::FForeignPowerWorldState,
        TradeDiplomacyRuntime::GForeignPowerCount>&
        GetForeignPowerStates() const override
    {
        return mOwner->GetForeignPowerStates();
    }

private:
    CMainWorld* mOwner = nullptr;
};

std::shared_ptr<CMainWorldTradeAccessGroup> CreateMainWorldTradeAccessAdapter(
    CMainWorld* Owner)
{
    return std::shared_ptr<CMainWorldTradeAccessGroup>(
        new CMainWorldTradeAccessGroup(Owner));
}

std::shared_ptr<IMainWorldTradeAccess> CMainWorld::GetTradeAccessHandle() const
{
    return std::static_pointer_cast<IMainWorldTradeAccess>(mAccess.Trade);
}

IMainWorldTradeAccess* CMainWorld::GetTradeAccessRaw() const
{
    return static_cast<IMainWorldTradeAccess*>(mAccess.Trade.get());
}
