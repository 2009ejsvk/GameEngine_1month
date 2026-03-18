#include "MainWorld.h"
#include "IWorldUIAccess.h"

namespace
{
    class CMainWorldUIAccess final :
        public IWorldUIAccess,
        public IWorldUIReadAccess,
        public IWorldUICommands
    {
    public:
        explicit CMainWorldUIAccess(CMainWorld* Owner)
            : mOwner(Owner)
        {
        }

        const IWorldUIReadAccess& Read() const override
        {
            return static_cast<const IWorldUIReadAccess&>(*this);
        }

        IWorldUICommands& Commands() override
        {
            return static_cast<IWorldUICommands&>(*this);
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

        bool IsSimulationPaused() const override
        {
            return mOwner->IsSimulationPaused();
        }

        int GetSimulationSpeedMultiplier() const override
        {
            return mOwner->GetSimulationSpeedMultiplier();
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

        const FGovernmentProfile& GetGovernmentProfile() const override
        {
            return mOwner->GetGovernmentProfile();
        }

        const FGovernmentEdictModifiers& GetEdictModifiers() const override
        {
            return mOwner->GetEdictModifiers();
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

        const FPoliticalDemandNotice& GetPoliticalDemandNotice() const override
        {
            return mOwner->GetPoliticalDemandNotice();
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

        const std::array<
            FPoliticalDemandState,
            TradeDiplomacyRuntime::GForeignPowerCount>&
            GetForeignDemandStates() const override
        {
            return mOwner->GetForeignDemandStates();
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

        const FKnowledgeState& GetKnowledgeState() const override
        {
            return mOwner->GetKnowledgeState();
        }

        int GetKnowledgePoints() const override
        {
            return mOwner->GetKnowledgePoints();
        }

        int GetDailyKnowledgeGeneration() const override
        {
            return mOwner->GetDailyKnowledgeGeneration();
        }

        bool IsResearchUnlocked(const std::wstring& Key) const override
        {
            return mOwner->IsResearchUnlocked(Key);
        }

        const FConstitutionState& GetConstitutionState() const override
        {
            return mOwner->GetConstitutionState();
        }

        const CRoadNetwork* GetRoadNetwork() const override
        {
            return mOwner->GetRoadNetwork();
        }

        const CBusRouteSystem* GetBusRouteSystem() const override
        {
            return mOwner->GetBusRouteSystem();
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

        bool TryUnlockResearch(
            const std::wstring& Key,
            int Cost) override
        {
            return mOwner->TryUnlockResearch(Key, Cost);
        }

        bool TrySelectConstitutionOption(
            EConstitutionOptionId Id) override
        {
            return mOwner->TrySelectConstitutionOption(Id);
        }

        void RebuildRoadNetwork() override
        {
            mOwner->RebuildRoadNetwork();
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
}

std::shared_ptr<IWorldUIAccess> CreateWorldUIAccessAdapter(CMainWorld* Owner)
{
    return std::shared_ptr<IWorldUIAccess>(new CMainWorldUIAccess(Owner));
}

std::shared_ptr<IWorldUIAccess> CMainWorld::GetUIAccessHandle() const
{
    return mAccess.UIFacade;
}

IWorldUIAccess* CMainWorld::GetUIAccessRaw() const
{
    return mAccess.UIFacade.get();
}

std::shared_ptr<IWorldUIAccess> ResolveWorldUIAccess(
    const std::shared_ptr<CWorld>& World)
{
    if (!World)
        return nullptr;

    if (auto Access = std::dynamic_pointer_cast<IWorldUIAccess>(World))
        return Access;

    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);
    return MainWorld ? MainWorld->GetUIAccessHandle() : nullptr;
}

IWorldUIAccess* ResolveWorldUIAccess(CWorld* World)
{
    if (!World)
        return nullptr;

    if (auto Access = dynamic_cast<IWorldUIAccess*>(World))
        return Access;

    auto MainWorld = dynamic_cast<CMainWorld*>(World);
    return MainWorld ? MainWorld->GetUIAccessRaw() : nullptr;
}
