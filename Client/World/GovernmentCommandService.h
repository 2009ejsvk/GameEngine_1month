#pragma once

#include "../Economy/TradeRouteRuntimeState.h"
#include "../Building/BuildingTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <memory>
#include <string>

class IGovernmentCommandService
{
public:
    virtual ~IGovernmentCommandService() = default;

    virtual bool TryApplyEdict(
        EGovernmentEdictType Type,
        std::wstring& OutMessage) = 0;
    virtual bool AdjustTaxPolicy(
        ETaxPolicyType Type,
        int DeltaPercent,
        std::wstring& OutMessage) = 0;
    virtual bool CycleExportBlockedResource(
        std::wstring& OutMessage) = 0;
    virtual bool ExecuteTradeProposal(
        bool ImportRoute,
        EResourceType ResourceType,
        int ForeignPowerIndex,
        int PricePerThousandUnits,
        int Amount,
        std::wstring& OutMessage) = 0;
    virtual bool CancelTradeRoute(
        int RouteId,
        std::wstring& OutMessage) = 0;
    virtual bool RespondPoliticalDemand(
        EPoliticalDemandIssuerType IssuerType,
        int IssuerIndex,
        bool Accept,
        std::wstring& OutMessage) = 0;
};

class CMainWorld;
class CWorld;

std::shared_ptr<IGovernmentCommandService> CreateGovernmentCommandServiceAdapter(
    CMainWorld* Owner);
std::shared_ptr<IGovernmentCommandService> ResolveGovernmentCommandService(
    const std::shared_ptr<CWorld>& World);
IGovernmentCommandService* ResolveGovernmentCommandService(CWorld* World);
