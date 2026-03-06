#include "EconomySystem.h"
#include "World/World.h"
#include "../Map/PlacementAreaObject.h"
#include <vector>

namespace
{
    constexpr int GExportPricePerStockUnit = 2;
}

EconomySystem::FDailyResult EconomySystem::ApplyDailySettlement(
    CWorld* World,
    int DaysInMonth)
{
    FDailyResult Result;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return Result;

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        auto Building = BuildingList[i].lock();

        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable() ||
            !Building->HasPlacedArea())
        {
            continue;
        }

        Result.WageCost   += Building->GetDailyWageCost(DaysInMonth);
        Result.UpkeepCost += Building->GetDailyUpkeepCost(DaysInMonth);

        if (Building->IsHarbor())
        {
            const bool ShipArrived =
                Building->AdvanceHarborShipProgressAndCheckArrival(
                    DaysInMonth);

            if (!ShipArrived)
                continue;

            const int ExportStock = Building->GetResourceStock();

            if (ExportStock > 0 &&
                Building->TryConsumeResource(ExportStock))
            {
                Result.ExportIncome += static_cast<long long>(
                    ExportStock) * GExportPricePerStockUnit;
            }
        }
    }

    Result.NetChange = Result.ExportIncome -
        Result.WageCost - Result.UpkeepCost;

    return Result;
}
