#include "KnowledgeSubsystem.h"
#include "MainWorld.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/PlacementAreaObject.h"
#include "../Politics/ConstitutionSystem.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
    bool TryResolveKnowledgeProducerBaseDailyGeneration(
        const FBuildingCatalogEntry& Entry,
        int& OutBaseDailyGeneration)
    {
        OutBaseDailyGeneration = 0;

        if (Entry.Id == "build_6_2")
        {
            OutBaseDailyGeneration = 3;
            return true;
        }

        if (Entry.Id == "build_6_3")
        {
            OutBaseDailyGeneration = 4;
            return true;
        }

        if (Entry.Id == "build_6_4")
        {
            OutBaseDailyGeneration = 6;
            return true;
        }

        if (Entry.Id == "build_6_11")
        {
            OutBaseDailyGeneration = 10;
            return true;
        }

        return false;
    }

    int ResolveKnowledgeGenerationForBuilding(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable() ||
            !Building->HasPlacedArea())
        {
            return 0;
        }

        const FBuildingCatalogEntry* const Entry =
            FindBuildingCatalogEntry(Building->GetBuildingId());

        if (!Entry)
            return 0;

        int BaseDailyGeneration = 0;

        if (!TryResolveKnowledgeProducerBaseDailyGeneration(
                *Entry,
                BaseDailyGeneration))
        {
            return 0;
        }

        const int WorkerCapacity = Building->GetCapacity();
        const int CurrentWorkers = Building->GetCurrentWorkerOccupancy();

        if (WorkerCapacity <= 0 || CurrentWorkers <= 0)
            return 0;

        const float WorkerRatio = (std::max)(
            0.f,
            (std::min)(
                1.f,
                static_cast<float>(CurrentWorkers) /
                    static_cast<float>(WorkerCapacity)));
        const float EffectiveGeneration =
            static_cast<float>(BaseDailyGeneration) *
            WorkerRatio *
            (std::max)(0.f, Building->GetBudgetSatisfactionScale()) *
            (std::max)(0.f, Building->GetPowerSupplyRatio());
        return (std::max)(
            0,
            static_cast<int>(roundf(EffectiveGeneration)));
    }
}

void CKnowledgeSubsystem::Reset()
{
    KnowledgeState.Reset();
    ConstitutionState = FConstitutionState();
}

void CKnowledgeSubsystem::RefreshKnowledgeGeneration()
{
    KnowledgeState.DailyGeneration = 0;

    if (!mOwner)
        return;

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
        return;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return;

    int TotalDailyGeneration = 0;

    for (size_t Index = 0; Index < BuildingList.size(); ++Index)
    {
        const auto Building = BuildingList[Index].lock();
        TotalDailyGeneration += ResolveKnowledgeGenerationForBuilding(Building);
    }

    KnowledgeState.DailyGeneration = (std::max)(0, TotalDailyGeneration);
}

void CKnowledgeSubsystem::ApplyDailyKnowledgeGain()
{
    if (KnowledgeState.DailyGeneration <= 0)
        return;

    KnowledgeState.Points += (std::max)(0, KnowledgeState.DailyGeneration);
}

bool CKnowledgeSubsystem::TryUnlockResearch(
    const std::wstring& Key,
    int Cost)
{
    if (Key.empty())
        return false;

    if (::IsResearchUnlocked(KnowledgeState, Key))
        return true;

    if (!CanUnlockResearch(KnowledgeState, Key, Cost))
        return false;

    return ::TryUnlockResearch(KnowledgeState, Key, Cost);
}

bool CKnowledgeSubsystem::TrySelectConstitutionOption(EConstitutionOptionId Id)
{
    if (!mOwner)
        return false;

    if (!ConstitutionSystem::TrySelectConstitutionOption(
            ConstitutionState,
            Id))
    {
        return false;
    }

    mOwner->mPolitics->RefreshSnapshot(
        {
            &mOwner->mEconomy->TaxEventStatus,
            &ConstitutionState.ActiveEffects
        });
    return true;
}
