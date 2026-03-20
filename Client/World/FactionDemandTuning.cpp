#include "FactionDemandTuning.h"
#include "MainWorldTradeRuntime.h"
#include "WorldStatsSnapshot.h"
#include "../GameConstants.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
    namespace MWDemand = GameConstants::MainWorld::PoliticalDemand;
    constexpr int GWarningPressureThreshold = 10;

    using FFactionDemandBuilder = bool (*)(
        const FactionDemandTuning::FFactionDemandContext& Context,
        FPoliticalDemandState& Demand,
        double& OutPriority);

    const std::array<int, 4> GFaithDemandTargetMaxByEra =
    {{
        56,
        68,
        82,
        82
    }};

    const FPoliticalFactionSnapshot& GetFactionSnapshot(
        const FactionDemandTuning::FFactionDemandContext& Context)
    {
        return Context.PoliticalSnapshot.Factions[
            static_cast<size_t>(Context.Faction)];
    }

    double GetApprovalPressure(
        const FactionDemandTuning::FFactionDemandContext& Context)
    {
        return static_cast<double>(MWDemand::FactionApprovalThreshold) -
            GetFactionSnapshot(Context).AverageApproval;
    }

    float GetDemandMultiplier(
        const FactionDemandTuning::FFactionDemandContext& Context)
    {
        return GameConstants::MainWorld::EraDemandThresholdMultipliers[
            static_cast<size_t>(GetBuildingEraRank(Context.CurrentEra))];
    }

    int ScaleDemandThreshold(int Value, float Multiplier)
    {
        return static_cast<int>(static_cast<float>(Value) * Multiplier + 0.5f);
    }

    bool TryBuildScoreDemand(
        const FactionDemandTuning::FFactionDemandContext& Context,
        EPoliticalDemandObjectiveType ObjectiveType,
        int CurrentValue,
        const wchar_t* Summary,
        const wchar_t* ObjectivePrefix,
        const MWDemand::FScoreDemandTuning& Tuning,
        int TargetMax,
        FPoliticalDemandState& Demand,
        double& OutPriority)
    {
        if (CurrentValue >=
            ScaleDemandThreshold(
                Tuning.IgnoreAtOrAboveValue,
                GetDemandMultiplier(Context)))
        {
            return false;
        }

        if (TargetMax < Tuning.TargetMin)
            return false;

        Demand.ObjectiveType = ObjectiveType;
        Demand.TargetValue = (std::max)(
            Tuning.TargetMin,
            (std::min)(TargetMax, CurrentValue + Tuning.TargetDelta));

        if (Demand.TargetValue <= CurrentValue)
            return false;

        Demand.CurrentValue = CurrentValue;
        Demand.Summary = Summary;
        Demand.ObjectiveText =
            std::wstring(ObjectivePrefix) +
            std::to_wstring(Demand.TargetValue) +
            L" 이상";
        Demand.RewardBudgetDelta = Tuning.RewardBudgetDelta;
        Demand.RewardFactionApprovalDelta = Tuning.RewardApprovalDelta;
        Demand.PenaltyFactionApprovalDelta = Tuning.PenaltyApprovalDelta;
        OutPriority =
            GetApprovalPressure(Context) * Tuning.ApprovalPriorityWeight +
            static_cast<double>((std::max)(
                0,
                Tuning.IgnoreAtOrAboveValue - CurrentValue)) *
                Tuning.DeficitPriorityWeight;
        return true;
    }

    bool TryBuildTaxCeilingDemand(
        const FactionDemandTuning::FFactionDemandContext& Context,
        EPoliticalDemandObjectiveType ObjectiveType,
        int CurrentValue,
        const wchar_t* Summary,
        const wchar_t* ObjectivePrefix,
        const MWDemand::FTaxCeilingDemandTuning& Tuning,
        FPoliticalDemandState& Demand,
        double& OutPriority)
    {
        if (CurrentValue <= Tuning.IgnoreAtOrBelowValue)
            return false;

        Demand.ObjectiveType = ObjectiveType;
        Demand.TargetValue = Tuning.TargetValue;
        Demand.CurrentValue = CurrentValue;
        Demand.Summary = Summary;
        Demand.ObjectiveText =
            std::wstring(ObjectivePrefix) +
            std::to_wstring(Demand.TargetValue) +
            L"% 이하";
        Demand.RewardBudgetDelta = Tuning.RewardBudgetDelta;
        Demand.RewardFactionApprovalDelta = Tuning.RewardApprovalDelta;
        Demand.PenaltyFactionApprovalDelta = Tuning.PenaltyApprovalDelta;
        OutPriority =
            GetApprovalPressure(Context) * Tuning.ApprovalPriorityWeight +
            static_cast<double>(CurrentValue - Demand.TargetValue) *
                Tuning.ExcessPriorityWeight;
        return true;
    }

    bool BuildCommunistsDemand(
        const FactionDemandTuning::FFactionDemandContext& Context,
        FPoliticalDemandState& Demand,
        double& OutPriority)
    {
        const auto& Tuning = MWDemand::Communists;
        const auto& FactionSnapshot = GetFactionSnapshot(Context);
        const int CurrentHousing =
            static_cast<int>(std::lround(Context.Snapshot.AverageHousing));
        const int ScaledIgnoreHousing =
            ScaleDemandThreshold(
                Tuning.IgnoreAtOrAboveHousing,
                GetDemandMultiplier(Context));

        if (CurrentHousing >= ScaledIgnoreHousing &&
            Context.Snapshot.HomelessHouseholdCount <=
                Tuning.IgnoreAtOrBelowHomeless)
        {
            return false;
        }

        Demand.ObjectiveType = EPoliticalDemandObjectiveType::Housing;
        Demand.TargetValue = (std::max)(
            Tuning.TargetMin,
            (std::min)(Tuning.TargetMax, CurrentHousing + Tuning.TargetDelta));
        Demand.CurrentValue = CurrentHousing;
        Demand.Summary = L"무주택과 저질 주거를 줄이라고 압박합니다.";
        Demand.ObjectiveText =
            L"평균 주거 " + std::to_wstring(Demand.TargetValue) + L" 이상";
        Demand.RewardBudgetDelta =
            static_cast<long long>(Tuning.RewardBudgetBase) +
            static_cast<long long>(FactionSnapshot.MemberCount) *
                static_cast<long long>(Tuning.RewardBudgetPerMember);
        Demand.RewardFactionApprovalDelta = Tuning.RewardApprovalDelta;
        Demand.PenaltyFactionApprovalDelta = Tuning.PenaltyApprovalDelta;
        OutPriority =
            GetApprovalPressure(Context) * Tuning.ApprovalPriorityWeight +
            static_cast<double>((std::max)(
                0,
                Tuning.IgnoreAtOrAboveHousing - CurrentHousing)) *
                Tuning.HousingDeficitPriorityWeight +
            static_cast<double>(Context.Snapshot.HomelessHouseholdCount) *
                Tuning.HomelessPriorityWeight;
        return true;
    }

    bool BuildCapitalistsDemand(
        const FactionDemandTuning::FFactionDemandContext& Context,
        FPoliticalDemandState& Demand,
        double& OutPriority)
    {
        return TryBuildTaxCeilingDemand(
            Context,
            EPoliticalDemandObjectiveType::IncomeTaxCeiling,
            Context.GovernmentProfile.TaxPolicy.IncomeRatePercent,
            L"소득세 인하와 투자 여건 개선을 요구합니다.",
            L"소득세 ",
            MWDemand::Capitalists,
            Demand,
            OutPriority);
    }

    bool BuildReligiousDemand(
        const FactionDemandTuning::FFactionDemandContext& Context,
        FPoliticalDemandState& Demand,
        double& OutPriority)
    {
        return TryBuildScoreDemand(
            Context,
            EPoliticalDemandObjectiveType::Faith,
            static_cast<int>(std::lround(Context.Snapshot.AverageFaith)),
            L"신앙 만족도 회복과 종교 서비스 강화를 요구합니다.",
            L"평균 신앙 ",
            MWDemand::Religious,
            (std::min)(
                MWDemand::Religious.TargetMax,
                FactionDemandTuning::ResolveFaithDemandTargetMax(
                    Context.CurrentEra)),
            Demand,
            OutPriority);
    }

    bool BuildMilitaristsDemand(
        const FactionDemandTuning::FFactionDemandContext& Context,
        FPoliticalDemandState& Demand,
        double& OutPriority)
    {
        return TryBuildScoreDemand(
            Context,
            EPoliticalDemandObjectiveType::Security,
            static_cast<int>(std::lround(Context.Snapshot.AverageSecurity)),
            L"치안 안정과 군사 통제를 강화하라고 압박합니다.",
            L"평균 치안 ",
            MWDemand::Militarists,
            MWDemand::Militarists.TargetMax,
            Demand,
            OutPriority);
    }

    bool BuildEnvironmentalistsDemand(
        const FactionDemandTuning::FFactionDemandContext& Context,
        FPoliticalDemandState& Demand,
        double& OutPriority)
    {
        return TryBuildScoreDemand(
            Context,
            EPoliticalDemandObjectiveType::Health,
            static_cast<int>(std::lround(Context.Snapshot.AverageHealth)),
            L"보건과 환경 악화를 바로잡으라고 요구합니다.",
            L"평균 보건 ",
            MWDemand::Environmentalists,
            MWDemand::Environmentalists.TargetMax,
            Demand,
            OutPriority);
    }

    bool BuildIndustrialistsDemand(
        const FactionDemandTuning::FFactionDemandContext& Context,
        FPoliticalDemandState& Demand,
        double& OutPriority)
    {
        const auto& Tuning = MWDemand::Industrialists;
        const int CurrentExportIncome =
            (std::max)(0, static_cast<int>(Context.LastDailyExportIncome));

        if (CurrentExportIncome >= Tuning.IgnoreAtOrAboveValue)
            return false;

        Demand.ObjectiveType = EPoliticalDemandObjectiveType::ExportIncome;
        Demand.TargetValue = (std::max)(
            Tuning.TargetMin,
            CurrentExportIncome + Tuning.TargetDelta);
        Demand.CurrentValue = CurrentExportIncome;
        Demand.Summary = L"산업 생산을 끌어올려 수출 실적을 내라고 요구합니다.";
        Demand.ObjectiveText =
            L"일일 수출 " +
            MainWorldTradeRuntime::FormatCurrency(Demand.TargetValue) +
            L" 이상";
        Demand.RewardBudgetDelta = Tuning.RewardBudgetDelta;
        Demand.RewardFactionApprovalDelta = Tuning.RewardApprovalDelta;
        Demand.PenaltyFactionApprovalDelta = Tuning.PenaltyApprovalDelta;
        OutPriority =
            GetApprovalPressure(Context) * Tuning.ApprovalPriorityWeight +
            static_cast<double>((std::max)(
                0,
                Demand.TargetValue - CurrentExportIncome)) /
                static_cast<double>((std::max)(1.0f, Tuning.DeficitDivisor));
        return true;
    }

    bool BuildIntellectualsDemand(
        const FactionDemandTuning::FFactionDemandContext& Context,
        FPoliticalDemandState& Demand,
        double& OutPriority)
    {
        return TryBuildScoreDemand(
            Context,
            EPoliticalDemandObjectiveType::Freedom,
            static_cast<int>(std::lround(Context.Snapshot.AverageFreedom)),
            L"자유와 개방성을 회복하라고 요구합니다.",
            L"평균 자유 ",
            MWDemand::Intellectuals,
            MWDemand::Intellectuals.TargetMax,
            Demand,
            OutPriority);
    }

    bool BuildConservativesDemand(
        const FactionDemandTuning::FFactionDemandContext& Context,
        FPoliticalDemandState& Demand,
        double& OutPriority)
    {
        return TryBuildTaxCeilingDemand(
            Context,
            EPoliticalDemandObjectiveType::PropertyTaxCeiling,
            Context.GovernmentProfile.TaxPolicy.PropertyRatePercent,
            L"재산세 완화와 질서 회복을 동시에 요구합니다.",
            L"재산세 ",
            MWDemand::Conservatives,
            Demand,
            OutPriority);
    }

    const std::array<FFactionDemandBuilder, GPoliticalFactionCount>
        GFactionDemandBuilders =
    {{
        &BuildCommunistsDemand,
        &BuildCapitalistsDemand,
        &BuildReligiousDemand,
        &BuildMilitaristsDemand,
        &BuildEnvironmentalistsDemand,
        &BuildIndustrialistsDemand,
        &BuildIntellectualsDemand,
        &BuildConservativesDemand
    }};
}

namespace FactionDemandTuning
{
    const std::array<FFactionDefinition, GPoliticalFactionCount>
        GFactionDefinitions =
    {{
        {
            EPoliticalFaction::Communists,
            L"공산주의자",
            L"공산",
            L"escalation.warning.communists",
            EBuildingEra::WorldWars
        },
        {
            EPoliticalFaction::Capitalists,
            L"자본가",
            L"자본",
            L"escalation.warning.capitalists",
            EBuildingEra::WorldWars
        },
        {
            EPoliticalFaction::Religious,
            L"종교인",
            L"종교",
            L"escalation.warning.religious",
            EBuildingEra::WorldWars
        },
        {
            EPoliticalFaction::Militarists,
            L"군부",
            L"군부",
            L"escalation.warning.militarists",
            EBuildingEra::WorldWars
        },
        {
            EPoliticalFaction::Environmentalists,
            L"환경주의자",
            L"환경",
            L"escalation.warning.environmentalists",
            EBuildingEra::WorldWars
        },
        {
            EPoliticalFaction::Industrialists,
            L"산업주의자",
            L"산업",
            L"escalation.warning.industrialists",
            EBuildingEra::WorldWars
        },
        {
            EPoliticalFaction::Intellectuals,
            L"지식인",
            L"지식",
            L"escalation.warning.intellectuals",
            EBuildingEra::Modern
        },
        {
            EPoliticalFaction::Conservatives,
            L"보수주의자",
            L"보수",
            L"escalation.warning.conservatives",
            EBuildingEra::Modern
        }
    }};

    const FFactionDefinition& GetFactionDefinition(EPoliticalFaction Faction)
    {
        const size_t Index = static_cast<size_t>(Faction);
        return Index < GFactionDefinitions.size() ?
            GFactionDefinitions[Index] :
            GFactionDefinitions.front();
    }

    const wchar_t* GetPoliticalFactionName(EPoliticalFaction Faction)
    {
        return GetFactionDefinition(Faction).DisplayName;
    }

    const wchar_t* GetFactionCompactLabel(EPoliticalFaction Faction)
    {
        return GetFactionDefinition(Faction).CompactLabel;
    }

    const wchar_t* GetFactionWarningSummaryKey(EPoliticalFaction Faction)
    {
        return GetFactionDefinition(Faction).WarningSummaryKey;
    }

    bool IsFactionAvailableInEra(
        EPoliticalFaction Faction,
        EBuildingEra Era)
    {
        return GetBuildingEraRank(Era) >=
            GetBuildingEraRank(GetFactionDefinition(Faction).FirstAvailableEra);
    }

    int ResolveFaithDemandTargetMax(EBuildingEra Era)
    {
        const size_t EraIndex = static_cast<size_t>(GetBuildingEraRank(Era));
        return EraIndex < GFaithDemandTargetMaxByEra.size() ?
            GFaithDemandTargetMaxByEra[EraIndex] :
            GFaithDemandTargetMaxByEra.back();
    }

    bool TryBuildFactionDemand(
        const FFactionDemandContext& Context,
        FPoliticalDemandState& OutDemand,
        double& OutPriority)
    {
        if (!IsFactionAvailableInEra(Context.Faction, Context.CurrentEra))
            return false;

        const auto& FactionSnapshot = GetFactionSnapshot(Context);
        const float MemberMultiplier =
            GameConstants::MainWorld::EraFactionMemberMinMultipliers[
                static_cast<size_t>(GetBuildingEraRank(Context.CurrentEra))];
        const int ScaledMemberMin = static_cast<int>(
            static_cast<float>(MWDemand::FactionMemberMinCount) *
            MemberMultiplier + 0.5f);

        if (FactionSnapshot.MemberCount < ScaledMemberMin ||
            FactionSnapshot.AverageApproval >=
                static_cast<double>(MWDemand::FactionApprovalThreshold))
        {
            return false;
        }

        FPoliticalDemandState Demand;
        Demand.Active = true;
        Demand.IssuerType = EPoliticalDemandIssuerType::Faction;
        Demand.IssuerIndex = static_cast<int>(Context.Faction);
        Demand.Status = EPoliticalDemandStatus::PendingResponse;
        Demand.Stage = EPoliticalDemandStage::Demand;
        Demand.DurationDays = MWDemand::FactionDurationDays;
        Demand.RemainingDays = Demand.DurationDays;
        Demand.ModifierDurationDays = MWDemand::FactionModifierDurationDays;
        Demand.PenaltyBudgetDelta = 0;
        OutPriority = 0.0;

        const size_t FactionIndex = static_cast<size_t>(Context.Faction);
        if (FactionIndex >= GFactionDemandBuilders.size())
            return false;

        if (!GFactionDemandBuilders[FactionIndex](Context, Demand, OutPriority))
            return false;

        if (Demand.ObjectiveType == EPoliticalDemandObjectiveType::None ||
            OutPriority <= 0.0)
        {
            return false;
        }

        OutDemand = std::move(Demand);
        return true;
    }

    FPoliticalEscalationSummary BuildPoliticalEscalationSummary(
        const std::array<int, GPoliticalFactionCount>& PressureDays,
        const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
            DemandStates)
    {
        std::vector<EPoliticalFaction> RevoltFactions;
        std::vector<EPoliticalFaction> UltimatumFactions;
        std::vector<EPoliticalFaction> WarningFactions;

        for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
        {
            const EPoliticalFaction Faction =
                static_cast<EPoliticalFaction>(Index);
            const FPoliticalDemandState& Demand =
                DemandStates[static_cast<size_t>(Index)];

            if (Demand.Active && Demand.Stage == EPoliticalDemandStage::Revolt)
            {
                RevoltFactions.push_back(Faction);
                continue;
            }

            if (Demand.Active &&
                Demand.Stage == EPoliticalDemandStage::Ultimatum)
            {
                UltimatumFactions.push_back(Faction);
                continue;
            }

            if (PressureDays[static_cast<size_t>(Index)] >=
                GWarningPressureThreshold)
            {
                WarningFactions.push_back(Faction);
            }
        }

        FPoliticalEscalationSummary Summary;

        if (!RevoltFactions.empty())
        {
            Summary.Active = true;
            Summary.Critical = true;
            Summary.HighestStage = EPoliticalDemandStage::Revolt;
            Summary.Factions = std::move(RevoltFactions);
            return Summary;
        }

        if (!UltimatumFactions.empty())
        {
            Summary.Active = true;
            Summary.Critical = true;
            Summary.HighestStage = EPoliticalDemandStage::Ultimatum;
            Summary.Factions = std::move(UltimatumFactions);
            return Summary;
        }

        if (!WarningFactions.empty())
        {
            Summary.Active = true;
            Summary.Critical = false;
            Summary.HighestStage = EPoliticalDemandStage::Warning;
            Summary.Factions = std::move(WarningFactions);
        }

        return Summary;
    }
}
