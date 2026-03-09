#include "WorldStatsSnapshot.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <map>

namespace
{
    std::wstring Utf8ToWide(const std::string& Text)
    {
        if (Text.empty())
            return std::wstring();

        const int RequiredCount = MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), -1, nullptr, 0);

        if (RequiredCount <= 1)
            return std::wstring(Text.begin(), Text.end());

        std::wstring WideText;
        WideText.resize(RequiredCount - 1);

        MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            &WideText[0], RequiredCount - 1);

        return WideText;
    }

    template <typename TMap>
    std::vector<std::pair<std::wstring, int>> BuildTopList(
        const TMap& Source,
        size_t MaxCount)
    {
        std::vector<std::pair<std::wstring, int>> Result;
        Result.reserve(Source.size());

        for (auto Iter = Source.begin(); Iter != Source.end(); ++Iter)
        {
            if (Iter->second <= 0)
                continue;

            Result.push_back(*Iter);
        }

        std::sort(
            Result.begin(),
            Result.end(),
            [](const std::pair<std::wstring, int>& A,
                const std::pair<std::wstring, int>& B)
            {
                if (A.second != B.second)
                    return A.second > B.second;

                return A.first < B.first;
            });

        if (Result.size() > MaxCount)
            Result.resize(MaxCount);

        return Result;
    }
}

namespace WorldStats
{
    FWorldStatsSnapshot BuildSnapshot(const std::shared_ptr<CWorld>& World)
    {
        FWorldStatsSnapshot Snapshot;

        if (!World)
            return Snapshot;

        std::map<std::wstring, int> BuildingCounts;
        std::map<std::wstring, int> ResourceCounts;
        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        {
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

                ++Snapshot.TotalBuildingCount;
                Snapshot.MonthlyWageCost += Building->GetMonthlyWageCost();
                Snapshot.MonthlyUpkeepCost += Building->GetMonthlyUpkeepCost();
                Snapshot.TotalResourceStock += Building->GetResourceStock();

                if (Building->IsFoodProvider())
                    ++Snapshot.FoodProviderCount;

                if (Building->IsEntertainmentProvider())
                    ++Snapshot.EntertainmentBuildingCount;

                if (Building->IsHarbor())
                    ++Snapshot.HarborCount;

                const std::wstring BuildingName =
                    Utf8ToWide(Building->GetBuildingDisplayName());

                ++BuildingCounts[BuildingName];

                if (Building->GetResourceStock() > 0)
                    ResourceCounts[BuildingName] += Building->GetResourceStock();

                const FBuildingCatalogEntry* Entry =
                    FindBuildingCatalogEntry(Building->GetBuildingId());

                if (!Entry)
                    continue;

                const int CategoryIndex = static_cast<int>(Entry->Category);

                if (CategoryIndex >= 0 &&
                    CategoryIndex < GBuildingCategoryCount)
                {
                    ++Snapshot.BuildingCategoryCount[CategoryIndex];
                }

                if (Entry->Category == EBuildingCategory::Tourism)
                    ++Snapshot.TourismBuildingCount;

                const bool IsWorkBuilding =
                    !Building->IsResidential() &&
                    !Building->IsHarbor() &&
                    (!Building->IsEntertainmentProvider() ||
                        Building->IsFoodProvider());

                if (Entry->Residential)
                    Snapshot.ResidentialCapacity += Building->GetCapacity();

                if (IsWorkBuilding)
                    Snapshot.JobCapacity += Building->GetCapacity();
            }
        }

        Snapshot.TopBuildings = BuildTopList(BuildingCounts, 8);
        Snapshot.TopResourceBuildings = BuildTopList(ResourceCounts, 8);

        std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

        if (World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        {
            for (size_t i = 0; i < OrbList.size(); ++i)
            {
                auto Orb = OrbList[i].lock();

                if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                    continue;

                const FNpcSatisfaction& Satisfaction = Orb->GetSatisfaction();
                const FNpcPoliticalProfile& Political =
                    Orb->GetPoliticalProfile();

                ++Snapshot.ActiveCitizenCount;
                Snapshot.AverageFood += Satisfaction.Food;
                Snapshot.AverageHealth += Satisfaction.Health;
                Snapshot.AverageFun += Satisfaction.Fun;
                Snapshot.AverageFaith += Satisfaction.Faith;
                Snapshot.AverageHousing += Satisfaction.Housing;
                Snapshot.AverageJob += Satisfaction.Job;
                Snapshot.AverageFreedom += Satisfaction.Freedom;
                Snapshot.AverageSecurity += Satisfaction.Security;
                Snapshot.AverageOverall += Satisfaction.Overall;

                if (Orb->GetHomeBuilding().empty())
                    ++Snapshot.HomelessCount;
                else
                    ++Snapshot.AssignedHomeCount;

                if (Orb->GetWorkBuilding().empty())
                    ++Snapshot.UnemployedCount;
                else
                    ++Snapshot.AssignedJobCount;

                int NeutralAxisCount = 0;

                for (int AxisIndex = 0;
                    AxisIndex < static_cast<int>(EPoliticalAxis::Count);
                    ++AxisIndex)
                {
                    const EPoliticalAxis Axis =
                        static_cast<EPoliticalAxis>(AxisIndex);
                    const int StanceIndex =
                        static_cast<int>(Political.Get(Axis).Stance);

                    if (StanceIndex >= 0 && StanceIndex < 3)
                    {
                        ++Snapshot.PoliticalCount[AxisIndex][StanceIndex];

                        if (StanceIndex ==
                            static_cast<int>(EPoliticalStance::Neutral))
                        {
                            ++NeutralAxisCount;
                        }
                    }
                }

                if (NeutralAxisCount > 0)
                    ++Snapshot.AnyNeutralAxisCitizenCount;

                if (NeutralAxisCount ==
                    static_cast<int>(EPoliticalAxis::Count))
                {
                    ++Snapshot.FullyNeutralCitizenCount;
                }
            }
        }

        if (Snapshot.ActiveCitizenCount > 0)
        {
            const double Denominator =
                static_cast<double>(Snapshot.ActiveCitizenCount);
            Snapshot.AverageFood /= Denominator;
            Snapshot.AverageHealth /= Denominator;
            Snapshot.AverageFun /= Denominator;
            Snapshot.AverageFaith /= Denominator;
            Snapshot.AverageHousing /= Denominator;
            Snapshot.AverageJob /= Denominator;
            Snapshot.AverageFreedom /= Denominator;
            Snapshot.AverageSecurity /= Denominator;
            Snapshot.AverageOverall /= Denominator;
        }

        return Snapshot;
    }
}
