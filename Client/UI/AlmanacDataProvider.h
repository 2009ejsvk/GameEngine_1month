#pragma once

#include "../Building/BuildingTypes.h"
#include "../Building/BuildingCategoryInfo.h"
#include "../Citizen/CitizenTypes.h"
#include "../Politics/PoliticalTypes.h"
#include "../World/MainWorldAccess.h"
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class CWorld;
namespace WorldStats
{
    struct FWorldStatsSnapshot;
}

namespace AlmanacDataProvider
{
    struct FAlmanacResourceTypeSnapshot
    {
        EResourceType Type = EResourceType::None;
        int TotalStock = 0;
        int AvailableStock = 0;
        int ReservedPickup = 0;
        int ReservedIncoming = 0;
        int Capacity = 0;
        int AvailableIncomingCapacity = 0;
        int ShortagePressure = 0;
        int ProducerAvailableStock = 0;
        int WarehouseBufferedStock = 0;
        int ConsumerCoveredStock = 0;
        int HarborExportableStock = 0;
        int HarborReservedPickup = 0;
        int ProducerBuildingCount = 0;
        int ConsumerBuildingCount = 0;
        int StorageBuildingCount = 0;
        int WarehouseBuildingCount = 0;
        int HarborBuildingCount = 0;
        std::vector<std::pair<std::wstring, int>> TopStockBuildings;
        std::vector<std::pair<std::wstring, int>> TopProducerBuildings;
        std::vector<std::pair<std::wstring, int>> TopWarehouseBuildings;
        std::vector<std::pair<std::wstring, int>> TopConsumerBuildings;
        std::vector<std::pair<std::wstring, int>> TopHarborBuildings;
        std::vector<std::pair<std::wstring, int>> TopShortageBuildings;
        std::vector<std::pair<std::wstring, int>> TopReservedBuildings;
        std::vector<std::pair<std::wstring, int>> TopOverflowBuildings;
    };

    struct FAlmanacBuildingCategorySnapshot
    {
        EBuildingCategory Category = EBuildingCategory::Infrastructure;
        int Count = 0;
        std::vector<std::pair<std::wstring, int>> TopBuildings;
    };

    struct FAlmanacMainWorldRecord
    {
        bool Available = false;
        long long NationalBudget = 0;
        long long DailyExportIncome = 0;
        long long DailyTaxIncome = 0;
        long long DailyConsumptionTaxIncome = 0;
        long long DailyIncomeTaxIncome = 0;
        long long DailyPropertyTaxIncome = 0;
        long long DailyEdictCost = 0;
        long long DailyImportExpense = 0;
        long long DailyNetChange = 0;
        double TaxCollectionEfficiency = 0.0;
        int DaysUntilNextElection = -1;
        double ElectionWarningScore = 0.0;
        FPoliticalWorldSnapshot PoliticalSnapshot;
        FGovernmentProfile GovernmentProfile;
        FElectionStatus ElectionStatus;
        FTaxPolicyEventStatus TaxEventStatus;
        FWorldCrisisStatus WorldCrisisStatus;
        FPoliticalDemandNotice PoliticalDemandNotice;
        std::array<FPoliticalDemandState, GPoliticalFactionCount>
            FactionDemandStates = {};
        std::array<
            FPoliticalDemandState,
            TradeDiplomacyRuntime::GForeignPowerCount> ForeignDemandStates = {};
        std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount> ForeignPowerStates = {};
        std::vector<FGovernmentEdictState> GovernmentEdictStates;
    };

    struct FAlmanacSnapshot
    {
        bool HasMainWorld = false;
        int ActiveCitizenCount = 0;
        int ActiveTouristCount = 0;
        int ActiveHouseholdCount = 0;
        int HomelessCount = 0;
        int HomelessHouseholdCount = 0;
        int UnemployedCount = 0;
        int AssignedHomeCount = 0;
        int AssignedJobCount = 0;
        int ResidentialCapacity = 0;
        int JobCapacity = 0;
        int TotalBuildingCount = 0;
        int TourismBuildingCount = 0;
        int FoodProviderCount = 0;
        int EntertainmentBuildingCount = 0;
        int FaithBuildingCount = 0;
        int ResidentialVacancyBuildingCount = 0;
        int WorkVacancyBuildingCount = 0;
        int FreedomInfluenceBuildingCount = 0;
        int SecurityInfluenceBuildingCount = 0;
        int HarborCount = 0;
        int TourismVisitCapacity = 0;
        int TourismVisitOccupancy = 0;
        int TouristPreferenceMatchedCount = 0;
        int TotalProducedPowerMW = 0;
        int TotalRequiredPowerMW = 0;
        int DisconnectedConsumerCount = 0;
        int ActiveEdictCount = 0;
        int AnyNeutralAxisCitizenCount = 0;
        int FullyNeutralCitizenCount = 0;
        int HomelessWealthCount[GCitizenWealthLevelCount] = {};
        int CitizenWealthCount[GCitizenWealthLevelCount] = {};
        int TouristProfileCount[GTouristPreferenceCount] = {};
        int OverallSatisfactionCitizenCount[5] = {};
        int EducationCount[3] = {};
        int UnemployedEducationCount[3] = {};
        int WorkVacancyEducationCount[3] = {};
        int ResidentialVacancyWealthCount[5] = {};
        int PoliticalCount[static_cast<int>(EPoliticalAxis::Count)][3] = {};
        int BuildingCategoryCount[BuildingCategoryInfo::GBuildingCategoryCount] = {};
        long long NationalBudget = 0;
        long long MonthlyWageCost = 0;
        long long MonthlyUpkeepCost = 0;
        long long TotalResourceStock = 0;
        long long DailyExportIncome = 0;
        long long DailyTaxIncome = 0;
        long long DailyConsumptionTaxIncome = 0;
        long long DailyIncomeTaxIncome = 0;
        long long DailyPropertyTaxIncome = 0;
        long long DailyEdictCost = 0;
        long long DailyImportExpense = 0;
        long long DailyNetChange = 0;
        double TaxCollectionEfficiency = 0.0;
        int DaysUntilNextElection = -1;
        double ElectionWarningScore = 0.0;
        double AverageFood = 0.0;
        double AverageHealth = 0.0;
        double AverageFun = 0.0;
        double AverageFaith = 0.0;
        double AverageHousing = 0.0;
        double AverageJob = 0.0;
        double AverageFreedom = 0.0;
        double AverageSecurity = 0.0;
        double AverageResidentialFreedom = 0.0;
        double AverageResidentialSecurity = 0.0;
        double AverageResidentialPollution = 0.0;
        double AverageOverall = 0.0;
        double SupportPercent = 0.0;
        double OppositionPercent = 0.0;
        double AbstainPercent = 0.0;
        double RebelRiskScore = 0.0;
        bool MartialLawActive = false;
        std::wstring RebelRiskLabel;
        std::array<
            FAlmanacResourceTypeSnapshot,
            static_cast<size_t>(EResourceType::Count)> ResourceTypes = {};
        std::array<
            FAlmanacBuildingCategorySnapshot,
            BuildingCategoryInfo::GBuildingCategoryCount> BuildingCategories = {};
        FPoliticalWorldSnapshot PoliticalSnapshot;
        FGovernmentProfile GovernmentProfile;
        FElectionStatus ElectionStatus;
        FTaxPolicyEventStatus TaxEventStatus;
        FWorldCrisisStatus WorldCrisisStatus;
        FPoliticalDemandNotice PoliticalDemandNotice;
        std::array<FPoliticalDemandState, GPoliticalFactionCount>
            FactionDemandStates = {};
        std::array<
            FPoliticalDemandState,
            TradeDiplomacyRuntime::GForeignPowerCount> ForeignDemandStates = {};
        std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount> ForeignPowerStates = {};
        std::vector<std::pair<std::wstring, int>> TopBuildings;
        std::vector<std::pair<std::wstring, int>> TopResourceBuildings;
        std::vector<std::wstring> ActiveEdictLines;
    };

    class IAlmanacQuerySource
    {
    public:
        virtual ~IAlmanacQuerySource() = default;

        virtual FAlmanacMainWorldRecord QueryMainWorldRecord() const = 0;
        virtual void QueryWorldStats(
            WorldStats::FWorldStatsSnapshot& OutSnapshot) const = 0;
    };

    FAlmanacSnapshot BuildSnapshot(
        const std::shared_ptr<IAlmanacQuerySource>& QuerySource);

    FAlmanacSnapshot BuildSnapshot(
        const std::shared_ptr<CWorld>& World);

    std::wstring BuildYearbookSummaryText(
        const FAlmanacSnapshot& Snapshot);
}
