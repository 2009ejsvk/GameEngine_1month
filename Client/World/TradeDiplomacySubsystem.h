#pragma once

#include "World/WorldSubsystem.h"
#include "MainWorldTradeDiplomacyState.h"
#include "../Politics/PoliticalTypes.h"
#include <string>

class CTradeDiplomacySubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    struct FRefreshForeignTradeContext
    {
        const FGovernmentProfile& GovernmentProfile;
        const FTaxPolicyEventStatus& TaxEventStatus;
        const std::vector<FGovernmentEdictState>& GovernmentEdicts;
        EBuildingEra CurrentEra = EBuildingEra::Colonial;
        int SimulationYear = 0;
        int SimulationMonth = 1;
        int SimulationDay = 1;
        bool ApplyIdleDecay = false;
    };

    struct FDailyTradeRouteSettlement
    {
        long long ExportIncome = 0;
        long long ImportExpense = 0;
        long long NetChange = 0;
    };

    void Reset();
    void OnDayAdvanced(const FRefreshForeignTradeContext& Context);
    FDailyTradeRouteSettlement ProcessActiveRoutes();
    void CancelTradeRoutesForInactivePowers(EBuildingEra Era);
    bool ExecuteTradeProposal(
        bool ImportRoute,
        EResourceType ResourceType,
        int ForeignPowerIndex,
        int PricePerThousandUnits,
        int Amount,
        std::wstring& OutMessage);
    bool CancelTradeRoute(
        int RouteId,
        std::wstring& OutMessage);
    bool CycleExportBlockedResource(std::wstring& OutMessage);
    int GetCustomsExportTradePriceModifierPercent() const;
    int GetCustomsImportTradePriceModifierPercent() const;

    FMainWorldTradeDiplomacyState State;

private:
    void RecordFinishedTradeRoute(
        const FTradeRouteRuntimeState& Route,
        ETradeRouteEndReason EndReason);
};
