#include "MainWorld.h"
#include "GovernmentCommandService.h"

namespace
{
    template <typename TAccess, typename TSharedGetter>
    std::shared_ptr<TAccess> ResolveMainWorldSharedAccess(
        const std::shared_ptr<CWorld>& World,
        TSharedGetter Getter)
    {
        if (!World)
            return nullptr;

        if (auto Access = std::dynamic_pointer_cast<TAccess>(World))
            return Access;

        auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);
        return MainWorld ? (MainWorld.get()->*Getter)() : nullptr;
    }

    template <typename TAccess, typename TRawGetter>
    TAccess* ResolveMainWorldRawAccess(
        CWorld* World,
        TRawGetter Getter)
    {
        if (!World)
            return nullptr;

        if (auto Access = dynamic_cast<TAccess*>(World))
            return Access;

        auto MainWorld = dynamic_cast<CMainWorld*>(World);
        return MainWorld ? (MainWorld->*Getter)() : nullptr;
    }
}

class CMainWorldGovernmentCommandAccess final :
    public IGovernmentCommandService
{
public:
    explicit CMainWorldGovernmentCommandAccess(CMainWorld* Owner)
        : mOwner(Owner)
    {
    }

    bool TryApplyEdict(
        EGovernmentEdictType Type,
        std::wstring& OutMessage) override
    {
        return mOwner->TryApplyEdict(Type, OutMessage);
    }

    bool AdjustTaxPolicy(
        ETaxPolicyType Type,
        int DeltaPercent,
        std::wstring& OutMessage) override
    {
        return mOwner->AdjustTaxPolicy(Type, DeltaPercent, OutMessage);
    }

    bool CycleExportBlockedResource(
        std::wstring& OutMessage) override
    {
        return mOwner->CycleExportBlockedResource(OutMessage);
    }

    bool ExecuteTradeProposal(
        bool ImportRoute,
        EResourceType ResourceType,
        int ForeignPowerIndex,
        int PricePerThousandUnits,
        int Amount,
        std::wstring& OutMessage) override
    {
        return mOwner->ExecuteTradeProposal(
            ImportRoute,
            ResourceType,
            ForeignPowerIndex,
            PricePerThousandUnits,
            Amount,
            OutMessage);
    }

    bool CancelTradeRoute(
        int RouteId,
        std::wstring& OutMessage) override
    {
        return mOwner->CancelTradeRoute(RouteId, OutMessage);
    }

    bool RespondPoliticalDemand(
        EPoliticalDemandIssuerType IssuerType,
        int IssuerIndex,
        bool Accept,
        std::wstring& OutMessage) override
    {
        return mOwner->RespondPoliticalDemand(
            IssuerType,
            IssuerIndex,
            Accept,
            OutMessage);
    }

private:
    CMainWorld* mOwner = nullptr;
};

std::shared_ptr<IGovernmentCommandService> CreateGovernmentCommandServiceAdapter(
    CMainWorld* Owner)
{
    return std::shared_ptr<IGovernmentCommandService>(
        new CMainWorldGovernmentCommandAccess(Owner));
}

std::shared_ptr<IGovernmentCommandService>
    CMainWorld::GetGovernmentCommandServiceHandle() const
{
    return mAccess.GovernmentCommand;
}

IGovernmentCommandService* CMainWorld::GetGovernmentCommandServiceRaw() const
{
    return mAccess.GovernmentCommand.get();
}

std::shared_ptr<IGovernmentCommandService> ResolveGovernmentCommandService(
    const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IGovernmentCommandService>(
        World,
        &CMainWorld::GetGovernmentCommandServiceHandle);
}

IGovernmentCommandService* ResolveGovernmentCommandService(CWorld* World)
{
    return ResolveMainWorldRawAccess<IGovernmentCommandService>(
        World,
        &CMainWorld::GetGovernmentCommandServiceRaw);
}
