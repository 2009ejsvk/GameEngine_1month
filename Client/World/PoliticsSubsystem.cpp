#include "PoliticsSubsystem.h"
#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "MainWorldTradeRuntime.h"
#include "WorldStatsSnapshot.h"
#include "../Map/PlacementAreaObject.h"
#include "../Politics/PoliticsSystem.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <vector>

namespace
{
    bool ShouldEnableElectionsForEra(EBuildingEra Era)
    {
        return IsBuildingEraUnlocked(Era, EBuildingEra::WorldWars);
    }

    bool IsEligibleRevoltDamageTarget(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        return Building &&
            Building->GetAlive() &&
            Building->GetEnable() &&
            Building->HasPlacedArea() &&
            !Building->IsRoad() &&
            Building->GetDamageLevel() != EBuildingDamageLevel::Critical;
    }

    bool IsFactionRevoltPriorityBuilding(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        EPoliticalFaction Faction)
    {
        if (!Building)
            return false;

        switch (Faction)
        {
        case EPoliticalFaction::Communists:
            return Building->IsResidential() ||
                Building->GetBuildingCategory() == EBuildingCategory::Housing;
        case EPoliticalFaction::Capitalists:
            return Building->GetBuildingCategory() == EBuildingCategory::Industry ||
                Building->GetBuildingCategory() ==
                    EBuildingCategory::GovernmentFinance;
        case EPoliticalFaction::Religious:
            return Building->IsFaithProvider();
        case EPoliticalFaction::Militarists:
            return Building->GetBuildingCategory() == EBuildingCategory::Military;
        case EPoliticalFaction::Environmentalists:
            return Building->IsHealthProvider() ||
                Building->GetBuildingCategory() ==
                    EBuildingCategory::PublicService ||
                Building->GetBuildingCategory() == EBuildingCategory::FoodResource;
        case EPoliticalFaction::Industrialists:
            return Building->GetBuildingCategory() == EBuildingCategory::Industry ||
                Building->CanGenerateWorkOutput();
        case EPoliticalFaction::Intellectuals:
            return Building->GetBuildingCategory() ==
                EBuildingCategory::MediaEducation;
        case EPoliticalFaction::Conservatives:
            return Building->GetBuildingCategory() ==
                    EBuildingCategory::GovernmentFinance ||
                Building->IsResidential();
        default:
            return false;
        }
    }
}

void CPoliticsSubsystem::Reset()
{
    PoliticsSystem::SetDefaultGovernmentProfile(GovernmentProfile);
    PoliticalSnapshot = FPoliticalWorldSnapshot();

    if (!ElectionService)
        ElectionService = std::make_unique<CMainWorldElectionService>();

    ElectionService->Reset();

    if (!PoliticalDemandService)
        PoliticalDemandService =
            std::make_unique<CMainWorldPoliticalDemandService>();

    PoliticalDemandService->Reset();
}

void CPoliticsSubsystem::RefreshSnapshot(const FRefreshSnapshotContext& Context)
{
    if (!mOwner)
        return;

    PoliticalSnapshot = PoliticsSystem::EvaluateWorld(
        mOwner,
        mOwner->mEraState->EraProgress.CurrentEra,
        GovernmentProfile,
        Context.TaxEventStatus,
        Context.ConstitutionEffects);

    constexpr int GWarningPressureThreshold = 10;
    std::array<int, GPoliticalFactionCount> EmptyPressureDays = {};
    std::array<FPoliticalDemandState, GPoliticalFactionCount>
        EmptyDemandStates = {};
    const auto& FactionPressureDays =
        PoliticalDemandService ?
            PoliticalDemandService->GetFactionPressureDays() :
            EmptyPressureDays;
    const auto& FactionDemandStates =
        PoliticalDemandService ?
            PoliticalDemandService->GetFactionDemandStates() :
            EmptyDemandStates;

    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        FPoliticalFactionSnapshot& FactionSnapshot =
            PoliticalSnapshot.Factions[static_cast<size_t>(Index)];
        const int PressureDays =
            (std::max)(0, FactionPressureDays[static_cast<size_t>(Index)]);
        const FPoliticalDemandState& Demand =
            FactionDemandStates[static_cast<size_t>(Index)];

        FactionSnapshot.PressureDays = PressureDays;

        if (Demand.Active)
        {
            FactionSnapshot.DemandStage = Demand.Stage;
        }
        else if (PressureDays >= GWarningPressureThreshold)
        {
            FactionSnapshot.DemandStage = EPoliticalDemandStage::Warning;
        }
        else
        {
            FactionSnapshot.DemandStage = EPoliticalDemandStage::Demand;
        }
    }

    mOwner->mScenario->PeakSupportPercent =
        (std::max)(
            mOwner->mScenario->PeakSupportPercent,
            PoliticalSnapshot.AverageSupportScore);
}

void CPoliticsSubsystem::InitializeElectionSchedule(
    EBuildingEra CurrentEra,
    int SimulationYear)
{
    if (!ElectionService ||
        !ShouldEnableElectionsForEra(CurrentEra))
    {
        return;
    }

    const FElectionStatus& ElectionStatus = ElectionService->GetElectionStatus();

    if (ElectionStatus.GameLost ||
        ElectionStatus.NextElectionYear > 0)
    {
        return;
    }

    ElectionService->InitializeSchedule(
        SimulationYear,
        MainWorldConfig::GInitialElectionLeadYears,
        MainWorldConfig::GElectionMonth,
        MainWorldConfig::GElectionDay);
}

void CPoliticsSubsystem::TickElectionPromises(
    int SimulationYear,
    int SimulationMonth,
    int SimulationDay,
    long long LastDailyExportIncome)
{
    if (!ElectionService || !mOwner)
        return;

    CMainWorldElectionService::FPromiseContext Context;
    Context.World = mOwner->mSelf.lock();
    Context.SimulationYear = SimulationYear;
    Context.SimulationMonth = SimulationMonth;
    Context.SimulationDay = SimulationDay;
    Context.LastDailyExportIncome = LastDailyExportIncome;
    ElectionService->TickPromises(Context);
}

void CPoliticsSubsystem::ResolveScheduledElection(
    int SimulationYear,
    int SimulationMonth,
    int SimulationDay)
{
    if (!ElectionService || !mOwner)
        return;

    const FElectionStatus PreviousStatus =
        ElectionService->GetElectionStatus();

    RefreshSnapshot(
        {
            &mOwner->mEconomy->TaxEventStatus,
            &mOwner->mKnowledgeState->ConstitutionState.ActiveEffects
        });
    ElectionService->ResolveScheduledElection(
        PoliticalSnapshot,
        SimulationYear,
        SimulationMonth,
        SimulationDay,
        MainWorldConfig::GElectionIntervalYears,
        MainWorldConfig::GElectionMonth,
        MainWorldConfig::GElectionDay);

    const FElectionStatus& ElectionStatus =
        ElectionService->GetElectionStatus();
    const bool RecordedNewElection =
        ElectionStatus.HasRecordedElection &&
        (!PreviousStatus.HasRecordedElection ||
            ElectionStatus.LastElectionYear != PreviousStatus.LastElectionYear ||
            ElectionStatus.LastElectionMonth != PreviousStatus.LastElectionMonth ||
            ElectionStatus.LastElectionDay != PreviousStatus.LastElectionDay);

    if (RecordedNewElection)
        mOwner->mScenario->ShowResultWidget(
            ElectionStatus.IncumbentWonLastElection);
}

int CPoliticsSubsystem::GetDaysUntilNextElection(
    int SimulationYear,
    int SimulationMonth,
    int SimulationDay) const
{
    if (!ElectionService)
        return 0;

    return ElectionService->GetDaysUntilNextElection(
        SimulationYear,
        SimulationMonth,
        SimulationDay);
}

double CPoliticsSubsystem::GetElectionWarningScore(
    const FTaxPolicyEventStatus& TaxEventStatus,
    int SimulationYear,
    int SimulationMonth,
    int SimulationDay) const
{
    if (!ElectionService)
        return 0.0;

    return ElectionService->GetElectionWarningScore(
        PoliticalSnapshot,
        TaxEventStatus,
        SimulationYear,
        SimulationMonth,
        SimulationDay);
}

CMainWorldPoliticalDemandService::FContext
    CPoliticsSubsystem::BuildPoliticalDemandContext()
{
    return
    {
        mOwner->mSelf.lock(),
        mOwner->mEraState->EraProgress.CurrentEra,
        PoliticalSnapshot,
        GovernmentProfile,
        mOwner->mEconomy->LastDailyExportIncome,
        mOwner->mEconomy->NationalBudget,
        mOwner->mEconomy->LastDailyNetChange,
        mOwner->mTrade->State.ForeignPowerStandingStates,
        mOwner->mTrade->State.ForeignPowerStates,
        mOwner->mTrade->State.ActiveTradeRoutes
    };
}

void CPoliticsSubsystem::ApplyPoliticalDemandRefreshRequests(
    const CMainWorldPoliticalDemandService::FRefreshRequests& RefreshRequests)
{
    if (!mOwner)
        return;

    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        if (!RefreshRequests.TriggerFactionRevolts[static_cast<size_t>(Index)])
            continue;

        TriggerFactionRevoltConsequences(static_cast<EPoliticalFaction>(Index));
    }

    if (RefreshRequests.RefreshPoliticalSnapshot)
    {
        RefreshSnapshot(
            {
                &mOwner->mEconomy->TaxEventStatus,
                &mOwner->mKnowledgeState->ConstitutionState.ActiveEffects
            });
    }

    if (RefreshRequests.RefreshForeignTradeDiplomacy)
    {
        mOwner->mTrade->OnDayAdvanced(
            {
                GovernmentProfile,
                mOwner->mEconomy->TaxEventStatus,
                mOwner->mEdictState->GovernmentEdicts,
                mOwner->mEraState->EraProgress.CurrentEra,
                mOwner->mSimulation->Year,
                mOwner->mSimulation->Month,
                mOwner->mSimulation->Day,
                false
            });
    }

    if (RefreshRequests.RefreshWorldMarketPrices)
    {
        mOwner->mEconomy->RefreshWorldMarketPrices(
            {
                GovernmentProfile,
                mOwner->mEdictState->GovernmentEdicts,
                mOwner->mCrisis->WorldCrisisService->GetStatus(),
                mOwner->mTrade->State.ForeignPowerStates,
                mOwner->mSimulation->Year,
                mOwner->mSimulation->Month,
                mOwner->mSimulation->Day
            });
    }
}

void CPoliticsSubsystem::TriggerFactionRevoltConsequences(
    EPoliticalFaction Faction)
{
    if (!mOwner)
        return;

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
        return;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return;

    std::vector<std::shared_ptr<CPlacementAreaObject>> PriorityTargets;
    std::vector<std::shared_ptr<CPlacementAreaObject>> FallbackTargets;

    for (size_t Index = 0; Index < BuildingList.size(); ++Index)
    {
        auto Building = BuildingList[Index].lock();

        if (!IsEligibleRevoltDamageTarget(Building))
            continue;

        if (IsFactionRevoltPriorityBuilding(Building, Faction))
            PriorityTargets.push_back(Building);
        else
            FallbackTargets.push_back(Building);
    }

    const int TotalCandidateCount = static_cast<int>(
        PriorityTargets.size() + FallbackTargets.size());
    const int TargetHitCount =
        TotalCandidateCount <= 0 ? 0 : (std::min)(2, 1 + rand() % 2);
    int AppliedHits = 0;

    auto ApplySingleHit =
        [&](std::vector<std::shared_ptr<CPlacementAreaObject>>& Targets)
        {
            while (AppliedHits < TargetHitCount && !Targets.empty())
            {
                const size_t PickIndex = static_cast<size_t>(
                    rand() % static_cast<int>(Targets.size()));
                const auto Target = Targets[PickIndex];
                Targets.erase(Targets.begin() + static_cast<int>(PickIndex));

                if (!Target)
                    continue;

                const EBuildingDamageLevel NextLevel =
                    Target->GetDamageLevel() == EBuildingDamageLevel::Damaged ?
                        EBuildingDamageLevel::Critical :
                        EBuildingDamageLevel::Damaged;

                if (mOwner->DamageBuilding(Target->GetName(), NextLevel))
                    ++AppliedHits;
            }
        };

    ApplySingleHit(PriorityTargets);
    ApplySingleHit(FallbackTargets);

    if (mOwner->mCrisis->WorldCrisisService)
    {
        const CMainWorldWorldCrisisService::FTickContext CrisisContext =
        {
            mOwner->mSelf.lock(),
            PoliticalSnapshot,
            mOwner->mEdictState->GovernmentEdicts,
            GovernmentProfile.TaxPolicy,
            mOwner->mEconomy->TaxEventStatus,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day,
            mOwner->mEconomy->NationalBudget,
            mOwner->mEconomy->LastDailyNetChange,
            mOwner->mEconomy->LastDailyTaxCollectionEfficiency,
            mOwner->mEraState->EraProgress.CurrentEra
        };
        mOwner->mCrisis->WorldCrisisService->TriggerForcedRaid(CrisisContext);
    }
}

bool CPoliticsSubsystem::RespondPoliticalDemand(
    EPoliticalDemandIssuerType IssuerType,
    int IssuerIndex,
    bool Accept,
    std::wstring& OutMessage)
{
    if (!PoliticalDemandService || !mOwner)
        return false;

    CMainWorldPoliticalDemandService::FRefreshRequests RefreshRequests;

    if (!PoliticalDemandService->RespondPoliticalDemand(
            IssuerType,
            IssuerIndex,
            Accept,
            OutMessage,
            BuildPoliticalDemandContext(),
            RefreshRequests))
    {
        return false;
    }

    ApplyPoliticalDemandRefreshRequests(RefreshRequests);
    return true;
}

bool CPoliticsSubsystem::CompletePoliticalDemand(
    EPoliticalDemandIssuerType IssuerType,
    int IssuerIndex,
    std::wstring& OutMessage)
{
    if (!PoliticalDemandService || !mOwner)
        return false;

    const long long BudgetBefore = mOwner->mEconomy->NationalBudget;
    CMainWorldPoliticalDemandService::FRefreshRequests RefreshRequests;

    if (!PoliticalDemandService->CompleteDemand(
            IssuerType,
            IssuerIndex,
            OutMessage,
            BuildPoliticalDemandContext(),
            RefreshRequests))
    {
        return false;
    }

    mOwner->mEconomy->RecordSpecialEventBudgetDelta(
        mOwner->mEconomy->NationalBudget - BudgetBefore);
    ApplyPoliticalDemandRefreshRequests(RefreshRequests);
    return true;
}

void CPoliticsSubsystem::TickPoliticalDemands()
{
    if (!PoliticalDemandService || !mOwner)
        return;

    const long long BudgetBefore = mOwner->mEconomy->NationalBudget;
    const CMainWorldPoliticalDemandService::FRefreshRequests RefreshRequests =
        PoliticalDemandService->Tick(BuildPoliticalDemandContext());
    mOwner->mEconomy->RecordSpecialEventBudgetDelta(
        mOwner->mEconomy->NationalBudget - BudgetBefore);

    ApplyPoliticalDemandRefreshRequests(RefreshRequests);
}
