#pragma once

#include "../Politics/PoliticalTypes.h"
#include "../World/MainWorldAccess.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

class CWorld;

namespace AlmanacDataProvider
{
    constexpr int GBuildingCategoryCount = 8;

    struct FAlmanacSnapshot
    {
        bool HasMainWorld = false;
        int ActiveCitizenCount = 0;
        int HomelessCount = 0;
        int UnemployedCount = 0;
        int AssignedHomeCount = 0;
        int AssignedJobCount = 0;
        int ResidentialCapacity = 0;
        int JobCapacity = 0;
        int TotalBuildingCount = 0;
        int TourismBuildingCount = 0;
        int FoodProviderCount = 0;
        int EntertainmentBuildingCount = 0;
        int HarborCount = 0;
        int ActiveEdictCount = 0;
        int AnyNeutralAxisCitizenCount = 0;
        int FullyNeutralCitizenCount = 0;
        int PoliticalCount[static_cast<int>(EPoliticalAxis::Count)][3] = {};
        int BuildingCategoryCount[GBuildingCategoryCount] = {};
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
        double AverageOverall = 0.0;
        double SupportPercent = 0.0;
        double OppositionPercent = 0.0;
        double AbstainPercent = 0.0;
        double RebelRiskScore = 0.0;
        bool MartialLawActive = false;
        std::wstring RebelRiskLabel = L"낮음";
        FPoliticalWorldSnapshot PoliticalSnapshot;
        FGovernmentProfile GovernmentProfile;
        FElectionStatus ElectionStatus;
        FTaxPolicyEventStatus TaxEventStatus;
        std::vector<std::pair<std::wstring, int>> TopBuildings;
        std::vector<std::pair<std::wstring, int>> TopResourceBuildings;
        std::vector<std::wstring> ActiveEdictLines;
    };

    FAlmanacSnapshot BuildSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::shared_ptr<IMainWorldAccess>& MainWorld);

    std::wstring BuildYearbookSummaryText(
        const FAlmanacSnapshot& Snapshot);
}
