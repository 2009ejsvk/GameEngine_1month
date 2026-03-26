#pragma once

#include "MainWorldAccess.h"
#include <array>
#include <memory>
#include <string>
#include <vector>

class CWorld;

class CMainWorldPoliticalDemandService
{
public:
    struct FContext
    {
        std::shared_ptr<CWorld> World;
        EBuildingEra CurrentEra = EBuildingEra::Modern;
        const FPoliticalWorldSnapshot& PoliticalSnapshot;
        FGovernmentProfile& GovernmentProfile;
        long long LastDailyExportIncome = 0;
        long long& NationalBudget;
        long long& LastDailyNetChange;
        std::array<
            TradeDiplomacyRuntime::FForeignPowerStandingState,
            TradeDiplomacyRuntime::GForeignPowerCount>&
                ForeignPowerStandingStates;
        const std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount>&
                ForeignPowerStates;
        const std::vector<FTradeRouteRuntimeState>& ActiveTradeRoutes;
    };

    struct FRefreshRequests
    {
        bool RefreshPoliticalSnapshot = false;
        bool RefreshForeignTradeDiplomacy = false;
        bool RefreshWorldMarketPrices = false;
        std::array<bool, GPoliticalFactionCount> TriggerFactionRevolts = {};
    };

public:
    void Reset();
    bool InjectScenarioDemand(FPoliticalDemandState Demand);
    bool CompleteDemand(
        EPoliticalDemandIssuerType IssuerType,
        int IssuerIndex,
        std::wstring& OutMessage,
        const FContext& Context,
        FRefreshRequests& OutRefreshRequests);
    bool RespondPoliticalDemand(
        EPoliticalDemandIssuerType IssuerType,
        int IssuerIndex,
        bool Accept,
        std::wstring& OutMessage,
        const FContext& Context,
        FRefreshRequests& OutRefreshRequests);
    FRefreshRequests Tick(const FContext& Context);

    const FPoliticalDemandNotice& GetPoliticalDemandNotice() const
    {
        return mPoliticalDemandNotice;
    }

    const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
        GetFactionDemandStates() const
    {
        return mFactionDemands;
    }

    const std::array<int, GPoliticalFactionCount>&
        GetFactionPressureDays() const
    {
        return mFactionPressureDays;
    }

    const std::array<
        FPoliticalDemandState,
        TradeDiplomacyRuntime::GForeignPowerCount>&
        GetForeignDemandStates() const
    {
        return mForeignDemands;
    }

private:
    FPoliticalDemandNotice mPoliticalDemandNotice;
    std::array<FPoliticalDemandState, GPoliticalFactionCount>
        mFactionDemands = {};
    std::array<int, GPoliticalFactionCount> mFactionPressureDays = {};
    std::array<int, GPoliticalFactionCount> mFactionDemandCooldownDays = {};
    std::array<int, GPoliticalFactionCount> mFactionDemandModifierDays = {};
    std::array<
        FPoliticalDemandState,
        TradeDiplomacyRuntime::GForeignPowerCount> mForeignDemands = {};
    std::array<
        int,
        TradeDiplomacyRuntime::GForeignPowerCount> mForeignDemandCooldownDays = {};
};
