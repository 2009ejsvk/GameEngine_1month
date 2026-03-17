#include "BuildMenuWorldQuerySource.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../UI/AlmanacDataProvider.h"
#include "../UI/AlmanacQueryService.h"
#include "../World/MainWorldUiReadAccess.h"
#include "World/World.h"
#include <vector>

namespace
{
    int CollectAliveNpcCount(CWorld* World)
    {
        if (!World)
            return 0;

        std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;
        int NpcCount = 0;

        if (World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        {
            for (size_t Index = 0; Index < OrbList.size(); ++Index)
            {
                auto Orb = OrbList[Index].lock();

                if (Orb && Orb->GetAlive())
                    ++NpcCount;
            }
        }

        return NpcCount;
    }

    class CWorldBuildMenuQuerySource final :
        public BuildMenuDataProvider::IBuildMenuQuerySource
    {
    public:
        explicit CWorldBuildMenuQuerySource(
            const std::shared_ptr<CWorld>& World)
            : mWorld(World)
            , mBuildMenuAccess(
                ResolveMainWorldBuildMenuAccess(World))
            , mAlmanacQuerySource(
                AlmanacQueryService::CreateWorldQuerySource(World))
        {
        }

        BuildMenuDataProvider::FBuildMenuStatusRecord QueryStatus() const override
        {
            BuildMenuDataProvider::FBuildMenuStatusRecord Result;
            Result.AliveNpcCount = CollectAliveNpcCount(mWorld.get());

            if (mBuildMenuAccess)
            {
                Result.HasSimulationData = true;
                Result.NationalBudget = mBuildMenuAccess->GetNationalBudget();
                Result.CurrentEra = mBuildMenuAccess->GetCurrentEra();
                Result.EraProgress = mBuildMenuAccess->GetEraProgress();
                Result.SimulationYear = mBuildMenuAccess->GetSimulationYear();
                Result.SimulationMonth = mBuildMenuAccess->GetSimulationMonth();
                Result.SimulationDay = mBuildMenuAccess->GetSimulationDay();
                Result.SimulationMonthDayCount =
                    mBuildMenuAccess->GetSimulationMonthDayCount();
                Result.SimulationMonthProgress =
                    mBuildMenuAccess->GetSimulationMonthProgress();
            }

            if (mWorld)
            {
                const AlmanacDataProvider::FAlmanacSnapshot AlmanacSnapshot =
                    AlmanacDataProvider::BuildSnapshot(mAlmanacQuerySource);
                Result.YearbookBodyText =
                    AlmanacDataProvider::BuildYearbookSummaryText(
                        AlmanacSnapshot);
            }

            return Result;
        }

    private:
        std::shared_ptr<CWorld> mWorld;
        std::shared_ptr<IMainWorldBuildMenuAccess> mBuildMenuAccess;
        std::shared_ptr<AlmanacDataProvider::IAlmanacQuerySource>
            mAlmanacQuerySource;
    };
}

namespace BuildMenuWorldQuerySource
{
    std::shared_ptr<BuildMenuDataProvider::IBuildMenuQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World)
    {
        if (!World)
            return nullptr;

        return std::make_shared<CWorldBuildMenuQuerySource>(World);
    }
}
