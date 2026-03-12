#include "AlmanacQueryService.h"
#include "../World/MainWorldAccess.h"
#include "../World/WorldStatsSnapshot.h"
#include "World/World.h"

namespace
{
    class CWorldAlmanacQuerySource final :
        public AlmanacDataProvider::IAlmanacQuerySource
    {
    public:
        explicit CWorldAlmanacQuerySource(
            const std::shared_ptr<CWorld>& World)
            : mWorld(World)
            , mMainWorldAccess(
                std::dynamic_pointer_cast<IMainWorldAlmanacAccess>(World))
        {
        }

    public:
        AlmanacDataProvider::FAlmanacMainWorldRecord QueryMainWorldRecord()
            const override
        {
            AlmanacDataProvider::FAlmanacMainWorldRecord Result;

            if (!mMainWorldAccess)
                return Result;

            Result.Available = true;
            Result.NationalBudget = mMainWorldAccess->GetNationalBudget();
            Result.DailyExportIncome =
                mMainWorldAccess->GetLastDailyExportIncome();
            Result.DailyTaxIncome =
                mMainWorldAccess->GetLastDailyTaxIncome();
            Result.DailyConsumptionTaxIncome =
                mMainWorldAccess->GetLastDailyConsumptionTaxIncome();
            Result.DailyIncomeTaxIncome =
                mMainWorldAccess->GetLastDailyIncomeTaxIncome();
            Result.DailyPropertyTaxIncome =
                mMainWorldAccess->GetLastDailyPropertyTaxIncome();
            Result.DailyEdictCost =
                mMainWorldAccess->GetLastDailyEdictCost();
            Result.DailyImportExpense =
                mMainWorldAccess->GetLastDailyImportExpense();
            Result.DailyNetChange =
                mMainWorldAccess->GetLastDailyNetChange();
            Result.TaxCollectionEfficiency =
                mMainWorldAccess->GetLastDailyTaxCollectionEfficiency();
            Result.PoliticalSnapshot =
                mMainWorldAccess->GetPoliticalSnapshot();
            Result.GovernmentProfile =
                mMainWorldAccess->GetGovernmentProfile();
            Result.ElectionStatus =
                mMainWorldAccess->GetElectionStatus();
            Result.DaysUntilNextElection =
                mMainWorldAccess->GetDaysUntilNextElection();
            Result.ElectionWarningScore =
                mMainWorldAccess->GetElectionWarningScore();
            Result.TaxEventStatus =
                mMainWorldAccess->GetTaxPolicyEventStatus();
            Result.WorldCrisisStatus =
                mMainWorldAccess->GetWorldCrisisStatus();
            Result.GovernmentEdictStates =
                mMainWorldAccess->GetGovernmentEdictStates();
            return Result;
        }

        void QueryWorldStats(
            WorldStats::FWorldStatsSnapshot& OutSnapshot) const override
        {
            OutSnapshot = mWorld ?
                WorldStats::BuildSnapshot(mWorld) :
                WorldStats::FWorldStatsSnapshot();
        }

    private:
        std::shared_ptr<CWorld> mWorld;
        std::shared_ptr<IMainWorldAlmanacAccess> mMainWorldAccess;
    };
}

namespace AlmanacQueryService
{
    std::shared_ptr<AlmanacDataProvider::IAlmanacQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World)
    {
        if (!World)
            return nullptr;

        return std::make_shared<CWorldAlmanacQuerySource>(World);
    }
}
