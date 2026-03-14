#include "MainWorldElectionService.h"
#include "MainWorldTradeRuntime.h"
#include "WorldStatsSnapshot.h"
#include "../GameConstants.h"
#include "../Politics/PoliticsSystem.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace
{
    namespace MWDemand = GameConstants::MainWorld::PoliticalDemand;
    namespace MWPromise = GameConstants::MainWorld::ElectionPromise;

    struct FElectionPromiseCandidate
    {
        FElectionPromiseState Promise;
        double Priority = 0.0;
    };

    const wchar_t* GetElectionPromiseName(EElectionPromiseType Type)
    {
        switch (Type)
        {
        case EElectionPromiseType::Housing:
            return L"주거 개선";
        case EElectionPromiseType::Food:
            return L"식량 안정";
        case EElectionPromiseType::Health:
            return L"보건 확충";
        case EElectionPromiseType::Job:
            return L"고용 확대";
        case EElectionPromiseType::Freedom:
            return L"자유 보장";
        case EElectionPromiseType::Security:
            return L"치안 강화";
        case EElectionPromiseType::Faith:
            return L"신앙 지원";
        case EElectionPromiseType::ExportIncome:
            return L"수출 확대";
        case EElectionPromiseType::None:
        default:
            return L"공약";
        }
    }

    int EvaluateElectionPromiseCurrentValue(
        const FElectionPromiseState& Promise,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        long long LastDailyExportIncome)
    {
        switch (Promise.Type)
        {
        case EElectionPromiseType::Housing:
            return static_cast<int>(std::lround(Snapshot.AverageHousing));
        case EElectionPromiseType::Food:
            return static_cast<int>(std::lround(Snapshot.AverageFood));
        case EElectionPromiseType::Health:
            return static_cast<int>(std::lround(Snapshot.AverageHealth));
        case EElectionPromiseType::Job:
            return static_cast<int>(std::lround(Snapshot.AverageJob));
        case EElectionPromiseType::Freedom:
            return static_cast<int>(std::lround(Snapshot.AverageFreedom));
        case EElectionPromiseType::Security:
            return static_cast<int>(std::lround(Snapshot.AverageSecurity));
        case EElectionPromiseType::Faith:
            return static_cast<int>(std::lround(Snapshot.AverageFaith));
        case EElectionPromiseType::ExportIncome:
            return (std::max)(0, static_cast<int>(LastDailyExportIncome));
        case EElectionPromiseType::None:
        default:
            return 0;
        }
    }

    FElectionPromiseState BuildElectionPromiseState(
        EElectionPromiseType Type,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        long long LastDailyExportIncome)
    {
        FElectionPromiseState Promise;
        Promise.Active = true;
        Promise.Type = Type;
        Promise.Title = std::wstring(GetElectionPromiseName(Type)) + L" 공약";
        Promise.BaselineValue =
            EvaluateElectionPromiseCurrentValue(
                Promise,
                Snapshot,
                LastDailyExportIncome);
        Promise.CurrentValue = Promise.BaselineValue;

        switch (Type)
        {
        case EElectionPromiseType::Housing:
        {
            const auto& Tuning = MWPromise::Housing;
            Promise.TargetValue =
                (std::min)(100, Promise.BaselineValue + Tuning.TargetDelta);
            Promise.SuccessVoteModifierPercent =
                Tuning.SuccessVoteModifierPercent;
            Promise.FailureVoteModifierPercent =
                Tuning.FailureVoteModifierPercent;
            Promise.Summary =
                L"평균 주거 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        }
        case EElectionPromiseType::Food:
        {
            const auto& Tuning = MWPromise::Food;
            Promise.TargetValue =
                (std::min)(100, Promise.BaselineValue + Tuning.TargetDelta);
            Promise.SuccessVoteModifierPercent =
                Tuning.SuccessVoteModifierPercent;
            Promise.FailureVoteModifierPercent =
                Tuning.FailureVoteModifierPercent;
            Promise.Summary =
                L"평균 식량 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        }
        case EElectionPromiseType::Health:
        {
            const auto& Tuning = MWPromise::Health;
            Promise.TargetValue =
                (std::min)(100, Promise.BaselineValue + Tuning.TargetDelta);
            Promise.SuccessVoteModifierPercent =
                Tuning.SuccessVoteModifierPercent;
            Promise.FailureVoteModifierPercent =
                Tuning.FailureVoteModifierPercent;
            Promise.Summary =
                L"평균 보건 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        }
        case EElectionPromiseType::Job:
        {
            const auto& Tuning = MWPromise::Job;
            Promise.TargetValue =
                (std::min)(100, Promise.BaselineValue + Tuning.TargetDelta);
            Promise.SuccessVoteModifierPercent =
                Tuning.SuccessVoteModifierPercent;
            Promise.FailureVoteModifierPercent =
                Tuning.FailureVoteModifierPercent;
            Promise.Summary =
                L"평균 직업 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        }
        case EElectionPromiseType::Freedom:
        {
            const auto& Tuning = MWPromise::Freedom;
            Promise.TargetValue =
                (std::min)(100, Promise.BaselineValue + Tuning.TargetDelta);
            Promise.SuccessVoteModifierPercent =
                Tuning.SuccessVoteModifierPercent;
            Promise.FailureVoteModifierPercent =
                Tuning.FailureVoteModifierPercent;
            Promise.Summary =
                L"평균 자유 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        }
        case EElectionPromiseType::Security:
        {
            const auto& Tuning = MWPromise::Security;
            Promise.TargetValue =
                (std::min)(100, Promise.BaselineValue + Tuning.TargetDelta);
            Promise.SuccessVoteModifierPercent =
                Tuning.SuccessVoteModifierPercent;
            Promise.FailureVoteModifierPercent =
                Tuning.FailureVoteModifierPercent;
            Promise.Summary =
                L"평균 치안 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        }
        case EElectionPromiseType::Faith:
        {
            const auto& Tuning = MWPromise::Faith;
            Promise.TargetValue =
                (std::min)(100, Promise.BaselineValue + Tuning.TargetDelta);
            Promise.SuccessVoteModifierPercent =
                Tuning.SuccessVoteModifierPercent;
            Promise.FailureVoteModifierPercent =
                Tuning.FailureVoteModifierPercent;
            Promise.Summary =
                L"평균 신앙 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        }
        case EElectionPromiseType::ExportIncome:
        {
            const auto& Tuning = MWPromise::ExportIncome;
            const int BaselineIncome = (std::max)(0, Promise.BaselineValue);
            Promise.TargetValue = BaselineIncome +
                (std::max)(
                    Tuning.TargetMinDelta,
                    BaselineIncome /
                        (std::max)(1, Tuning.TargetScaleDivisor));
            Promise.SuccessVoteModifierPercent =
                Tuning.SuccessVoteModifierPercent;
            Promise.FailureVoteModifierPercent =
                Tuning.FailureVoteModifierPercent;
            Promise.Summary =
                L"일일 수출 " +
                MainWorldTradeRuntime::FormatCurrency(Promise.TargetValue) +
                L" 이상";
            break;
        }
        case EElectionPromiseType::None:
        default:
            Promise.Active = false;
            break;
        }

        return Promise;
    }

    double EvaluateElectionPromisePriority(
        EElectionPromiseType Type,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        long long LastDailyExportIncome)
    {
        const double CitizenCount =
            static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount));
        const double UnemploymentRatio =
            static_cast<double>(Snapshot.UnemployedCount) / CitizenCount;

        switch (Type)
        {
        case EElectionPromiseType::Housing:
        {
            const auto& Tuning = MWPromise::Housing;
            return
                (std::max)(
                    0.0,
                    static_cast<double>(Tuning.PriorityThresholdValue) -
                        Snapshot.AverageHousing) *
                    static_cast<double>(Tuning.PriorityDeficitWeight) +
                static_cast<double>(Snapshot.HomelessHouseholdCount) *
                    static_cast<double>(Tuning.HomelessHouseholdPriorityWeight);
        }
        case EElectionPromiseType::Food:
        {
            const auto& Tuning = MWPromise::Food;
            return (std::max)(
                0.0,
                static_cast<double>(Tuning.PriorityThresholdValue) -
                    Snapshot.AverageFood) *
                static_cast<double>(Tuning.PriorityDeficitWeight);
        }
        case EElectionPromiseType::Health:
        {
            const auto& Tuning = MWPromise::Health;
            return (std::max)(
                0.0,
                static_cast<double>(Tuning.PriorityThresholdValue) -
                    Snapshot.AverageHealth) *
                static_cast<double>(Tuning.PriorityDeficitWeight);
        }
        case EElectionPromiseType::Job:
        {
            const auto& Tuning = MWPromise::Job;
            return
                (std::max)(
                    0.0,
                    static_cast<double>(Tuning.PriorityThresholdValue) -
                        Snapshot.AverageJob) *
                    static_cast<double>(Tuning.PriorityDeficitWeight) +
                UnemploymentRatio *
                    static_cast<double>(Tuning.UnemploymentRatioPriorityWeight);
        }
        case EElectionPromiseType::Freedom:
        {
            const auto& Tuning = MWPromise::Freedom;
            return (std::max)(
                0.0,
                static_cast<double>(Tuning.PriorityThresholdValue) -
                    Snapshot.AverageFreedom) *
                static_cast<double>(Tuning.PriorityDeficitWeight);
        }
        case EElectionPromiseType::Security:
        {
            const auto& Tuning = MWPromise::Security;
            return (std::max)(
                0.0,
                static_cast<double>(Tuning.PriorityThresholdValue) -
                    Snapshot.AverageSecurity) *
                static_cast<double>(Tuning.PriorityDeficitWeight);
        }
        case EElectionPromiseType::Faith:
        {
            const auto& Tuning = MWPromise::Faith;
            return (std::max)(
                0.0,
                static_cast<double>(Tuning.PriorityThresholdValue) -
                    Snapshot.AverageFaith) *
                static_cast<double>(Tuning.PriorityDeficitWeight);
        }
        case EElectionPromiseType::ExportIncome:
        {
            const auto& Tuning = MWPromise::ExportIncome;
            return static_cast<double>((std::max)(
                0,
                Tuning.PriorityReferenceValue -
                    static_cast<int>(LastDailyExportIncome))) /
                static_cast<double>((std::max)(1.0f, Tuning.PriorityDivisor));
        }
        case EElectionPromiseType::None:
        default:
            return 0.0;
        }
    }

    int CountActiveElectionPromises(const FElectionStatus& ElectionStatus)
    {
        int Count = 0;

        for (int Index = 0; Index < GElectionPromiseCount; ++Index)
        {
            if (ElectionStatus.ActivePromises[static_cast<size_t>(Index)].Active)
                ++Count;
        }

        return Count;
    }
}

void CMainWorldElectionService::Reset()
{
    mElectionStatus = FElectionStatus();
}

void CMainWorldElectionService::InitializeSchedule(
    int SimulationYear,
    int InitialElectionLeadYears,
    int ElectionMonth,
    int ElectionDay)
{
    PoliticsSystem::InitializeElectionSchedule(
        mElectionStatus,
        SimulationYear,
        InitialElectionLeadYears,
        ElectionMonth,
        ElectionDay);
}

void CMainWorldElectionService::TickPromises(const FPromiseContext& Context)
{
    if (mElectionStatus.GameLost || !Context.World)
        return;

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(Context.World);

    for (int Index = 0; Index < GElectionPromiseCount; ++Index)
    {
        FElectionPromiseState& Promise =
            mElectionStatus.ActivePromises[static_cast<size_t>(Index)];

        if (!Promise.Active)
            continue;

        Promise.CurrentValue =
            EvaluateElectionPromiseCurrentValue(
                Promise,
                Snapshot,
                Context.LastDailyExportIncome);
    }

    const int DaysUntilElection = GetDaysUntilNextElection(
        Context.SimulationYear,
        Context.SimulationMonth,
        Context.SimulationDay);

    if (DaysUntilElection <= 0 ||
        DaysUntilElection > MWDemand::CampaignPromiseLeadDays ||
        mElectionStatus.CampaignPromisesIssued ||
        CountActiveElectionPromises(mElectionStatus) > 0)
    {
        return;
    }

    static const EElectionPromiseType GPromiseTypes[] =
    {
        EElectionPromiseType::Housing,
        EElectionPromiseType::Food,
        EElectionPromiseType::Health,
        EElectionPromiseType::Job,
        EElectionPromiseType::Freedom,
        EElectionPromiseType::Security,
        EElectionPromiseType::Faith,
        EElectionPromiseType::ExportIncome
    };

    std::vector<FElectionPromiseCandidate> Candidates;
    Candidates.reserve(sizeof(GPromiseTypes) / sizeof(GPromiseTypes[0]));

    for (EElectionPromiseType Type : GPromiseTypes)
    {
        FElectionPromiseCandidate Candidate;
        Candidate.Promise = BuildElectionPromiseState(
            Type,
            Snapshot,
            Context.LastDailyExportIncome);
        Candidate.Priority = EvaluateElectionPromisePriority(
            Type,
            Snapshot,
            Context.LastDailyExportIncome);
        Candidates.push_back(Candidate);
    }

    std::sort(
        Candidates.begin(),
        Candidates.end(),
        [](const FElectionPromiseCandidate& A,
            const FElectionPromiseCandidate& B)
        {
            if (A.Priority != B.Priority)
                return A.Priority > B.Priority;

            return A.Promise.Title < B.Promise.Title;
        });

    mElectionStatus.ActivePromises = {};

    for (int Index = 0;
        Index < GElectionPromiseCount &&
            Index < static_cast<int>(Candidates.size());
        ++Index)
    {
        mElectionStatus.ActivePromises[static_cast<size_t>(Index)] =
            Candidates[static_cast<size_t>(Index)].Promise;
    }

    mElectionStatus.CampaignPromisesIssued =
        CountActiveElectionPromises(mElectionStatus) > 0;

    if (!mElectionStatus.CampaignPromisesIssued)
        return;

    mElectionStatus.PromiseIssueYear = Context.SimulationYear;
    mElectionStatus.PromiseIssueMonth = Context.SimulationMonth;
    mElectionStatus.PromiseIssueDay = Context.SimulationDay;
    mElectionStatus.HasPromiseEvaluation = false;
    mElectionStatus.LastPromiseMetCount = 0;
    mElectionStatus.LastPromiseFailedCount = 0;
    mElectionStatus.LastPromiseVoteModifierPercent = 0;
}

void CMainWorldElectionService::ResolveScheduledElection(
    const FPoliticalWorldSnapshot& PoliticalSnapshot,
    int SimulationYear,
    int SimulationMonth,
    int SimulationDay,
    int ElectionIntervalYears,
    int ElectionMonth,
    int ElectionDay)
{
    PoliticsSystem::ResolveScheduledElection(
        mElectionStatus,
        PoliticalSnapshot,
        SimulationYear,
        SimulationMonth,
        SimulationDay,
        ElectionIntervalYears,
        ElectionMonth,
        ElectionDay);
}

int CMainWorldElectionService::GetDaysUntilNextElection(
    int SimulationYear,
    int SimulationMonth,
    int SimulationDay) const
{
    return PoliticsSystem::GetDaysUntilNextElection(
        mElectionStatus,
        SimulationYear,
        SimulationMonth,
        SimulationDay);
}

double CMainWorldElectionService::GetElectionWarningScore(
    const FPoliticalWorldSnapshot& PoliticalSnapshot,
    const FTaxPolicyEventStatus& TaxEventStatus,
    int SimulationYear,
    int SimulationMonth,
    int SimulationDay) const
{
    return PoliticsSystem::GetElectionWarningScore(
        mElectionStatus,
        PoliticalSnapshot,
        TaxEventStatus,
        SimulationYear,
        SimulationMonth,
        SimulationDay);
}
