#include "BuildingSubsystem.h"
#include "MainWorld.h"
#include "MainWorldTradeRuntime.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/PlacementAreaObject.h"
#include "../StringUtils.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
    int GetBuildingDamageLevelRank(EBuildingDamageLevel Level)
    {
        switch (Level)
        {
        case EBuildingDamageLevel::Critical:
            return 2;
        case EBuildingDamageLevel::Damaged:
            return 1;
        case EBuildingDamageLevel::None:
        default:
            return 0;
        }
    }

    int ResolveBuildingRepairCost(
        const FBuildingCatalogEntry& Entry,
        EBuildingDamageLevel Level)
    {
        if (Level == EBuildingDamageLevel::None)
            return 0;

        int BaseCost = 1200;

        if (Entry.ConstructionCostState == EBuildingCostState::Known &&
            Entry.ConstructionCost > 0)
        {
            BaseCost = Entry.ConstructionCost;
        }
        else if (Entry.BlueprintCostState == EBuildingCostState::Known &&
            Entry.BlueprintCost > 0)
        {
            BaseCost = Entry.BlueprintCost;
        }

        switch (Level)
        {
        case EBuildingDamageLevel::Damaged:
            return (std::max)(
                250,
                static_cast<int>(roundf(static_cast<float>(BaseCost) * 0.18f)));
        case EBuildingDamageLevel::Critical:
            return (std::max)(
                600,
                static_cast<int>(roundf(static_cast<float>(BaseCost) * 0.42f)));
        case EBuildingDamageLevel::None:
        default:
            return 0;
        }
    }
}

void CMainWorld::SpendBuildingCost(int BaseCost)
{
    if (!mEconomy || !mEdictState || BaseCost <= 0)
        return;

    const float Multiplier =
        mEdictState->EdictModifiers.BuildingCostMultiplier;
    const int ActualCost = static_cast<int>(
        roundf(static_cast<float>(BaseCost) * Multiplier));

    mEconomy->ApplyConstructionExpense(ActualCost);
}

void CBuildingSubsystem::RefreshRuntimeBuildingState()
{
    if (!mOwner)
        return;

    mOwner->mInfrastructure->RefreshPowerGridCoverage();
    mOwner->mInfrastructure->RefreshBuildingPollutionExposure();
    RefreshBuildingRepairCosts();
    mOwner->mKnowledgeState->RefreshKnowledgeGeneration();
    mOwner->mPopulation->ReassignCitizenNeeds();
    mOwner->mEraState->RefreshEraProgress();
    mOwner->mPolitics->RefreshSnapshot(
        {
            &mOwner->mEconomy->TaxEventStatus,
            &mOwner->mKnowledgeState->ConstitutionState.ActiveEffects
        });
    mOwner->mTrade->OnDayAdvanced(
        {
            mOwner->mPolitics->GovernmentProfile,
            mOwner->mEconomy->TaxEventStatus,
            mOwner->mEdictState->GovernmentEdicts,
            mOwner->mEraState->EraProgress.CurrentEra,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day,
            false
        });
    mOwner->mEconomy->RefreshWorldMarketPrices(
        {
            mOwner->mPolitics->GovernmentProfile,
            mOwner->mEdictState->GovernmentEdicts,
            mOwner->mCrisis->WorldCrisisService->GetStatus(),
            mOwner->mTrade->State.ForeignPowerStates,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day
        });
}

void CBuildingSubsystem::RefreshBuildingRepairCosts()
{
    if (!mOwner)
        return;

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
        return;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return;

    for (size_t Index = 0; Index < BuildingList.size(); ++Index)
    {
        auto Building = BuildingList[Index].lock();

        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable())
        {
            continue;
        }

        const FBuildingCatalogEntry* const Entry =
            FindBuildingCatalogEntry(Building->GetBuildingId());

        Building->SetRepairCost(
            Entry ?
                ResolveBuildingRepairCost(*Entry, Building->GetDamageLevel()) :
                0);
    }
}

bool CBuildingSubsystem::DamageBuilding(
    const std::string& BuildingName,
    EBuildingDamageLevel Level)
{
    if (!mOwner ||
        BuildingName.empty() ||
        Level == EBuildingDamageLevel::None)
    {
        return false;
    }

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
        return false;

    auto Building =
        World->FindObject<CPlacementAreaObject>(BuildingName).lock();

    if (!Building ||
        !Building->GetAlive() ||
        !Building->GetEnable() ||
        !Building->HasPlacedArea() ||
        Building->IsRoad())
    {
        return false;
    }

    if (GetBuildingDamageLevelRank(Building->GetDamageLevel()) >=
        GetBuildingDamageLevelRank(Level))
    {
        return false;
    }

    Building->SetDamageLevel(Level);
    const FBuildingCatalogEntry* const Entry =
        FindBuildingCatalogEntry(Building->GetBuildingId());
    Building->SetRepairCost(
        Entry ?
            ResolveBuildingRepairCost(*Entry, Level) :
            0);
    return true;
}

bool CBuildingSubsystem::TryRepairBuilding(
    const std::string& BuildingName,
    std::wstring& OutMessage)
{
    OutMessage.clear();

    if (!mOwner)
    {
        OutMessage = L"건물을 찾을 수 없습니다.";
        return false;
    }

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World || BuildingName.empty())
    {
        OutMessage = L"건물을 찾을 수 없습니다.";
        return false;
    }

    auto Building =
        World->FindObject<CPlacementAreaObject>(BuildingName).lock();

    if (!Building ||
        !Building->GetAlive() ||
        !Building->GetEnable() ||
        !Building->HasPlacedArea())
    {
        OutMessage = L"건물을 찾을 수 없습니다.";
        return false;
    }

    if (!Building->HasBuildingDamage())
    {
        OutMessage = L"수리가 필요한 피해가 없습니다.";
        return false;
    }

    const FBuildingCatalogEntry* const Entry =
        FindBuildingCatalogEntry(Building->GetBuildingId());
    const int RepairCost =
        Building->GetRepairCost() > 0 ?
            Building->GetRepairCost() :
            (Entry ?
                ResolveBuildingRepairCost(*Entry, Building->GetDamageLevel()) :
                0);

    if (RepairCost > mOwner->mEconomy->NationalBudget)
    {
        OutMessage =
            StringUtils::Utf8ToWide(Building->GetBuildingDisplayName()) +
            L": 수리비 " +
            MainWorldTradeRuntime::FormatCurrency(RepairCost) +
            L" 필요";
        return false;
    }

    mOwner->mEconomy->ApplyRepairExpense(RepairCost);
    Building->SetDamageLevel(EBuildingDamageLevel::None);
    Building->SetRepairCost(0);
    RefreshRuntimeBuildingState();
    OutMessage =
        StringUtils::Utf8ToWide(Building->GetBuildingDisplayName()) +
        L": 긴급 수리 완료 (-" +
        MainWorldTradeRuntime::FormatCurrency(RepairCost) +
        L")";
    return true;
}
