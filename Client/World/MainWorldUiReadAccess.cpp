#include "MainWorld.h"
#include "MainWorldUiReadAccess.h"

class CMainWorldUiReadAccess final :
    public IMainWorldBuildMenuAccess,
    public IMainWorldHudAccess,
    public IMainWorldAlmanacAccess,
    public IMainWorldEdictReadAccess
{
public:
    explicit CMainWorldUiReadAccess(CMainWorld* Owner)
        : mOwner(Owner)
    {
    }

    long long GetNationalBudget() const override
    {
        return mOwner->GetNationalBudget();
    }

    int GetSimulationYear() const override
    {
        return mOwner->GetSimulationYear();
    }

    int GetSimulationMonth() const override
    {
        return mOwner->GetSimulationMonth();
    }

    int GetSimulationDay() const override
    {
        return mOwner->GetSimulationDay();
    }

    int GetSimulationMonthDayCount() const override
    {
        return mOwner->GetSimulationMonthDayCount();
    }

    float GetSimulationDayProgress() const override
    {
        return mOwner->GetSimulationDayProgress();
    }

    float GetSimulationMonthProgress() const override
    {
        return mOwner->GetSimulationMonthProgress();
    }

    EBuildingEra GetCurrentEra() const override
    {
        return mOwner->GetCurrentEra();
    }

    const FEraProgressState& GetEraProgress() const override
    {
        return mOwner->GetEraProgress();
    }

    const FEraTransitionState& GetEraTransitionState() const override
    {
        return mOwner->GetEraTransitionState();
    }

    void SpendBuildingCost(int BaseCost) override
    {
        mOwner->SpendBuildingCost(BaseCost);
    }

    const FTaxPolicy& GetTaxPolicy() const override
    {
        return mOwner->GetTaxPolicy();
    }

    const FPoliticalWorldSnapshot& GetPoliticalSnapshot() const override
    {
        return mOwner->GetPoliticalSnapshot();
    }

    const FElectionStatus& GetElectionStatus() const override
    {
        return mOwner->GetElectionStatus();
    }

    int GetDaysUntilNextElection() const override
    {
        return mOwner->GetDaysUntilNextElection();
    }

    double GetElectionWarningScore() const override
    {
        return mOwner->GetElectionWarningScore();
    }

    const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const override
    {
        return mOwner->GetTaxPolicyEventStatus();
    }

    const FWorldCrisisStatus& GetWorldCrisisStatus() const override
    {
        return mOwner->GetWorldCrisisStatus();
    }

    const std::array<int, GPoliticalFactionCount>&
        GetFactionDemandPressureDays() const override
    {
        return mOwner->GetFactionDemandPressureDays();
    }

    const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
        GetFactionDemandStates() const override
    {
        return mOwner->GetFactionDemandStates();
    }

    const FPoliticalDemandNotice& GetPoliticalDemandNotice() const override
    {
        return mOwner->GetPoliticalDemandNotice();
    }

    bool IsSimulationPaused() const override
    {
        return mOwner->IsSimulationPaused();
    }

    int GetSimulationSpeedMultiplier() const override
    {
        return mOwner->GetSimulationSpeedMultiplier();
    }

    void ToggleSimulationPaused() override
    {
        mOwner->ToggleSimulationPaused();
    }

    void CycleSimulationSpeedMultiplier() override
    {
        mOwner->CycleSimulationSpeedMultiplier();
    }

    bool TryExecuteEraTransition(EEraTransitionChoice Choice) override
    {
        return mOwner->TryExecuteEraTransition(Choice);
    }

    const FGovernmentProfile& GetGovernmentProfile() const override
    {
        return mOwner->GetGovernmentProfile();
    }

    const std::vector<FGovernmentEdictState>&
        GetGovernmentEdictStates() const override
    {
        return mOwner->GetGovernmentEdictStates();
    }

    const FGovernmentEdictState* GetGovernmentEdictState(
        EGovernmentEdictType Type) const override
    {
        return mOwner->GetGovernmentEdictState(Type);
    }

    long long GetLastDailyEdictCost() const override
    {
        return mOwner->GetLastDailyEdictCost();
    }

    long long GetLastDailyImportExpense() const override
    {
        return mOwner->GetLastDailyImportExpense();
    }

    long long GetLastDailyExportIncome() const override
    {
        return mOwner->GetLastDailyExportIncome();
    }

    long long GetLastDailyTaxIncome() const override
    {
        return mOwner->GetLastDailyTaxIncome();
    }

    long long GetLastDailyConsumptionTaxIncome() const override
    {
        return mOwner->GetLastDailyConsumptionTaxIncome();
    }

    long long GetLastDailyIncomeTaxIncome() const override
    {
        return mOwner->GetLastDailyIncomeTaxIncome();
    }

    long long GetLastDailyPropertyTaxIncome() const override
    {
        return mOwner->GetLastDailyPropertyTaxIncome();
    }

    double GetLastDailyTaxCollectionEfficiency() const override
    {
        return mOwner->GetLastDailyTaxCollectionEfficiency();
    }

    long long GetLastDailyNetChange() const override
    {
        return mOwner->GetLastDailyNetChange();
    }

    const FEconomyLedgerHistorySnapshot& GetEconomyLedgerHistory()
        const override
    {
        return mOwner->GetEconomyLedgerHistory();
    }

    const std::array<
        FPoliticalDemandState,
        TradeDiplomacyRuntime::GForeignPowerCount>&
        GetForeignDemandStates() const override
    {
        return mOwner->GetForeignDemandStates();
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

std::shared_ptr<CMainWorldUiReadAccess> CreateMainWorldUiReadAccessAdapter(
    CMainWorld* Owner)
{
    return std::shared_ptr<CMainWorldUiReadAccess>(
        new CMainWorldUiReadAccess(Owner));
}

std::shared_ptr<IMainWorldBuildMenuAccess>
    CMainWorld::GetBuildMenuAccessHandle() const
{
    return std::static_pointer_cast<IMainWorldBuildMenuAccess>(mAccess.UiRead);
}

IMainWorldBuildMenuAccess* CMainWorld::GetBuildMenuAccessRaw() const
{
    return static_cast<IMainWorldBuildMenuAccess*>(mAccess.UiRead.get());
}

std::shared_ptr<IMainWorldHudAccess> CMainWorld::GetHudAccessHandle() const
{
    return std::static_pointer_cast<IMainWorldHudAccess>(mAccess.UiRead);
}

IMainWorldHudAccess* CMainWorld::GetHudAccessRaw() const
{
    return static_cast<IMainWorldHudAccess*>(mAccess.UiRead.get());
}

std::shared_ptr<IMainWorldAlmanacAccess>
    CMainWorld::GetAlmanacAccessHandle() const
{
    return std::static_pointer_cast<IMainWorldAlmanacAccess>(mAccess.UiRead);
}

IMainWorldAlmanacAccess* CMainWorld::GetAlmanacAccessRaw() const
{
    return static_cast<IMainWorldAlmanacAccess*>(mAccess.UiRead.get());
}

std::shared_ptr<IMainWorldEdictReadAccess>
    CMainWorld::GetEdictReadAccessHandle() const
{
    return std::static_pointer_cast<IMainWorldEdictReadAccess>(mAccess.UiRead);
}

IMainWorldEdictReadAccess* CMainWorld::GetEdictReadAccessRaw() const
{
    return static_cast<IMainWorldEdictReadAccess*>(mAccess.UiRead.get());
}
