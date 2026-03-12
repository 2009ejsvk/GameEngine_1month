#pragma once

#include "../Economy/TradeRouteRuntimeState.h"
#include "../Building/BuildingTypes.h"
#include "../Politics/PoliticalTypes.h"
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
    virtual bool CycleDomesticReservePolicy(
        std::wstring& OutMessage) = 0;
    virtual bool CycleImportPerResourceCap(
        std::wstring& OutMessage) = 0;
    virtual bool CycleImportBudgetPolicy(
        std::wstring& OutMessage) = 0;
    virtual bool CycleExportBlockedResource(
        std::wstring& OutMessage) = 0;
    virtual bool CycleAutoImportResource(
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
};
