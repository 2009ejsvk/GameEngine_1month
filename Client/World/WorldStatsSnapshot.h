#pragma once

#include "../Building/BuildingTypes.h"
#include "../Citizen/CitizenTypes.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

class CWorld;

namespace WorldStats
{
    constexpr int GBuildingCategoryCount =
        static_cast<int>(EBuildingCategory::Count);

    struct FWorldStatsSnapshot
    {
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
        int AnyNeutralAxisCitizenCount = 0;
        int FullyNeutralCitizenCount = 0;
        int PoliticalCount[static_cast<int>(EPoliticalAxis::Count)][3] = {};
        int BuildingCategoryCount[GBuildingCategoryCount] = {};
        long long MonthlyWageCost = 0;
        long long MonthlyUpkeepCost = 0;
        long long TotalResourceStock = 0;
        double AverageFood = 0.0;
        double AverageHealth = 0.0;
        double AverageFun = 0.0;
        double AverageFaith = 0.0;
        double AverageHousing = 0.0;
        double AverageJob = 0.0;
        double AverageFreedom = 0.0;
        double AverageSecurity = 0.0;
        double AverageOverall = 0.0;
        std::vector<std::pair<std::wstring, int>> TopBuildings;
        std::vector<std::pair<std::wstring, int>> TopResourceBuildings;
    };

    FWorldStatsSnapshot BuildSnapshot(const std::shared_ptr<CWorld>& World);
}
