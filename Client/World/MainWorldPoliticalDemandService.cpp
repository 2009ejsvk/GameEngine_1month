#include "MainWorldPoliticalDemandService.h"
#include "MainWorldTradeRuntime.h"
#include "WorldStatsSnapshot.h"
#include "../UI/UIStrings.h"
#include "../GameConstants.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
    namespace MWDemand = GameConstants::MainWorld::PoliticalDemand;
    constexpr double GWarningApprovalThreshold = 35.0;
    constexpr int GWarningPressureThreshold = 10;
    constexpr int GUltimatumPressureThreshold = 30;
    constexpr int GRevoltPressureThreshold = 60;
    constexpr int GPressureDecayPerSafeDay = 2;

    bool IsFactionAvailableInEra(
        EPoliticalFaction Faction,
        EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::Colonial:
            return false;
        case EBuildingEra::WorldWars:
        case EBuildingEra::ColdWar:
            return Faction != EPoliticalFaction::Intellectuals &&
                Faction != EPoliticalFaction::Conservatives;
        case EBuildingEra::Modern:
        default:
            return true;
        }
    }

    int ResolveFaithDemandTargetMax(EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::Colonial:
            return 56;
        case EBuildingEra::WorldWars:
            return 68;
        case EBuildingEra::ColdWar:
        case EBuildingEra::Modern:
        default:
            return 82;
        }
    }

    const wchar_t* GetPoliticalFactionName(EPoliticalFaction Faction)
    {
        switch (Faction)
        {
        case EPoliticalFaction::Communists:
            return L"공산주의자";
        case EPoliticalFaction::Capitalists:
            return L"자본가";
        case EPoliticalFaction::Religious:
            return L"종교인";
        case EPoliticalFaction::Militarists:
            return L"군부";
        case EPoliticalFaction::Environmentalists:
            return L"환경주의자";
        case EPoliticalFaction::Industrialists:
            return L"산업주의자";
        case EPoliticalFaction::Intellectuals:
            return L"지식인";
        case EPoliticalFaction::Conservatives:
            return L"보수주의자";
        default:
            return L"세력";
        }
    }

    std::wstring FormatSignedInt(int Value)
    {
        if (Value > 0)
            return L"+" + std::to_wstring(Value);

        return std::to_wstring(Value);
    }

    std::wstring FormatBudgetDelta(long long Value)
    {
        if (Value > 0)
            return L"+" + MainWorldTradeRuntime::FormatCurrency(Value);
        if (Value < 0)
            return L"-" + MainWorldTradeRuntime::FormatCurrency(-Value);

        return L"$0";
    }

    std::wstring BuildPoliticalDemandEffectText(
        long long BudgetDelta,
        int FactionApprovalDelta,
        int RelationDelta,
        int StandingDelta,
        int DurationDays)
    {
        std::wstring Result;
        const auto AppendPart =
            [&](const std::wstring& Part)
            {
                if (Part.empty())
                    return;

                if (!Result.empty())
                    Result += L" / ";

                Result += Part;
            };

        if (BudgetDelta != 0)
            AppendPart(L"예산 " + FormatBudgetDelta(BudgetDelta));

        if (FactionApprovalDelta != 0)
        {
            std::wstring Part =
                L"승인도 " + FormatSignedInt(FactionApprovalDelta);

            if (DurationDays > 0)
                Part += L" (" + std::to_wstring(DurationDays) + L"일)";

            AppendPart(Part);
        }

        if (RelationDelta != 0)
            AppendPart(L"관계 " + FormatSignedInt(RelationDelta));

        if (StandingDelta != 0)
            AppendPart(L"standing " + FormatSignedInt(StandingDelta));

        if (Result.empty())
            Result = L"직접 변화 없음";

        return Result;
    }

    const std::wstring& GetPoliticalDemandStageLabel(
        EPoliticalDemandStage Stage)
    {
        switch (Stage)
        {
        case EPoliticalDemandStage::Warning:
            return UIStrings::Get(L"escalation.stage.warning");
        case EPoliticalDemandStage::Ultimatum:
            return UIStrings::Get(L"escalation.stage.ultimatum");
        case EPoliticalDemandStage::Revolt:
            return UIStrings::Get(L"escalation.stage.revolt");
        case EPoliticalDemandStage::Demand:
        default:
            return UIStrings::Get(L"escalation.stage.demand");
        }
    }

    const wchar_t* GetFactionWarningSummaryKey(EPoliticalFaction Faction)
    {
        switch (Faction)
        {
        case EPoliticalFaction::Communists:
            return L"escalation.warning.communists";
        case EPoliticalFaction::Capitalists:
            return L"escalation.warning.capitalists";
        case EPoliticalFaction::Religious:
            return L"escalation.warning.religious";
        case EPoliticalFaction::Militarists:
            return L"escalation.warning.militarists";
        case EPoliticalFaction::Environmentalists:
            return L"escalation.warning.environmentalists";
        case EPoliticalFaction::Industrialists:
            return L"escalation.warning.industrialists";
        case EPoliticalFaction::Intellectuals:
            return L"escalation.warning.intellectuals";
        case EPoliticalFaction::Conservatives:
            return L"escalation.warning.conservatives";
        default:
            return L"escalation.warning.generic";
        }
    }

    void RefreshFactionDemandStagePresentation(FPoliticalDemandState& Demand)
    {
        if (Demand.IssuerType != EPoliticalDemandIssuerType::Faction ||
            Demand.IssuerIndex < 0 ||
            Demand.IssuerIndex >= GPoliticalFactionCount)
        {
            return;
        }

        Demand.Title =
            std::wstring(GetPoliticalFactionName(
                static_cast<EPoliticalFaction>(Demand.IssuerIndex))) +
            L" " +
            GetPoliticalDemandStageLabel(Demand.Stage);
        Demand.RewardText = BuildPoliticalDemandEffectText(
            Demand.RewardBudgetDelta,
            Demand.RewardFactionApprovalDelta,
            0,
            0,
            Demand.ModifierDurationDays);
        Demand.PenaltyText = BuildPoliticalDemandEffectText(
            Demand.PenaltyBudgetDelta,
            Demand.PenaltyFactionApprovalDelta,
            0,
            0,
            Demand.ModifierDurationDays);
    }

    bool PromoteFactionDemandStage(
        FPoliticalDemandState& Demand,
        EPoliticalDemandStage NewStage)
    {
        if (!Demand.Active ||
            Demand.IssuerType != EPoliticalDemandIssuerType::Faction ||
            static_cast<int>(Demand.Stage) >= static_cast<int>(NewStage))
        {
            return false;
        }

        const bool NeedsUltimatumPenaltyUpgrade =
            static_cast<int>(Demand.Stage) <
                static_cast<int>(EPoliticalDemandStage::Ultimatum) &&
            static_cast<int>(NewStage) >=
                static_cast<int>(EPoliticalDemandStage::Ultimatum);

        Demand.Stage = NewStage;

        if (NeedsUltimatumPenaltyUpgrade)
        {
            const int NewDurationDays = (std::max)(1, Demand.DurationDays / 2);
            Demand.DurationDays = NewDurationDays;

            if (Demand.RemainingDays > 0)
            {
                Demand.RemainingDays =
                    (std::max)(
                        1,
                        (std::min)(
                            NewDurationDays,
                            (Demand.RemainingDays + 1) / 2));
            }
            else
            {
                Demand.RemainingDays = NewDurationDays;
            }

            Demand.PenaltyBudgetDelta *= 2;
            Demand.PenaltyFactionApprovalDelta *= 2;
        }

        RefreshFactionDemandStagePresentation(Demand);
        return true;
    }

    FPoliticalDemandNotice BuildFactionWarningNotice(
        EPoliticalFaction Faction,
        int PressureDays)
    {
        FPoliticalDemandNotice Notice;
        Notice.ActiveDemand = false;
        Notice.Positive = false;
        Notice.RemainingDays = MWDemand::NoticeDurationDays;
        Notice.Title = UIStrings::Format(
            L"escalation.warning.title_template",
            { std::wstring(GetPoliticalFactionName(Faction)) });
        Notice.Summary = UIStrings::Format(
            GetFactionWarningSummaryKey(Faction),
            { std::to_wstring((std::max)(0, PressureDays)) });
        return Notice;
    }

    int CountActiveFactionDemands(
        const std::array<FPoliticalDemandState, GPoliticalFactionCount>& Demands)
    {
        int Count = 0;

        for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
        {
            if (Demands[static_cast<size_t>(Index)].Active)
                ++Count;
        }

        return Count;
    }

    int CountActiveForeignDemands(
        const std::array<
            FPoliticalDemandState,
            TradeDiplomacyRuntime::GForeignPowerCount>& Demands)
    {
        int Count = 0;

        for (int Index = 0;
            Index < TradeDiplomacyRuntime::GForeignPowerCount;
            ++Index)
        {
            if (Demands[static_cast<size_t>(Index)].Active)
                ++Count;
        }

        return Count;
    }

    int EvaluatePoliticalDemandCurrentValue(
        const FPoliticalDemandState& Demand,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        long long LastDailyExportIncome,
        const std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount>& ForeignPowerStates,
        const std::vector<FTradeRouteRuntimeState>& ActiveTradeRoutes)
    {
        switch (Demand.ObjectiveType)
        {
        case EPoliticalDemandObjectiveType::Housing:
            return static_cast<int>(std::lround(Snapshot.AverageHousing));
        case EPoliticalDemandObjectiveType::Food:
            return static_cast<int>(std::lround(Snapshot.AverageFood));
        case EPoliticalDemandObjectiveType::Faith:
            return static_cast<int>(std::lround(Snapshot.AverageFaith));
        case EPoliticalDemandObjectiveType::Security:
            return static_cast<int>(std::lround(Snapshot.AverageSecurity));
        case EPoliticalDemandObjectiveType::Freedom:
            return static_cast<int>(std::lround(Snapshot.AverageFreedom));
        case EPoliticalDemandObjectiveType::Health:
            return static_cast<int>(std::lround(Snapshot.AverageHealth));
        case EPoliticalDemandObjectiveType::ExportIncome:
            return (std::max)(0, static_cast<int>(LastDailyExportIncome));
        case EPoliticalDemandObjectiveType::IncomeTaxCeiling:
            return GovernmentProfile.TaxPolicy.IncomeRatePercent;
        case EPoliticalDemandObjectiveType::PropertyTaxCeiling:
            return GovernmentProfile.TaxPolicy.PropertyRatePercent;
        case EPoliticalDemandObjectiveType::ActiveTradeRoutes:
            if (Demand.IssuerType == EPoliticalDemandIssuerType::ForeignPower &&
                Demand.IssuerIndex >= 0 &&
                Demand.IssuerIndex < TradeDiplomacyRuntime::GForeignPowerCount)
            {
                return MainWorldTradeRuntime::CountActiveTradeRoutesForPower(
                    ActiveTradeRoutes,
                    Demand.IssuerIndex);
            }

            return static_cast<int>(ActiveTradeRoutes.size());
        case EPoliticalDemandObjectiveType::None:
        default:
            break;
        }

        (void)ForeignPowerStates;
        return 0;
    }

    bool IsPoliticalDemandSatisfied(const FPoliticalDemandState& Demand)
    {
        switch (Demand.ObjectiveType)
        {
        case EPoliticalDemandObjectiveType::IncomeTaxCeiling:
        case EPoliticalDemandObjectiveType::PropertyTaxCeiling:
            return Demand.CurrentValue <= Demand.TargetValue;
        case EPoliticalDemandObjectiveType::Housing:
        case EPoliticalDemandObjectiveType::Food:
        case EPoliticalDemandObjectiveType::Faith:
        case EPoliticalDemandObjectiveType::Security:
        case EPoliticalDemandObjectiveType::Freedom:
        case EPoliticalDemandObjectiveType::Health:
        case EPoliticalDemandObjectiveType::ExportIncome:
        case EPoliticalDemandObjectiveType::ActiveTradeRoutes:
            return Demand.CurrentValue >= Demand.TargetValue;
        case EPoliticalDemandObjectiveType::None:
        default:
            return false;
        }
    }

    int GetEraIndex(EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::WorldWars: return 1;
        case EBuildingEra::ColdWar:   return 2;
        case EBuildingEra::Modern:    return 3;
        case EBuildingEra::Colonial:
        default:                      return 0;
        }
    }

    bool TryBuildFactionDemand(
        EPoliticalFaction Faction,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FPoliticalWorldSnapshot& PoliticalSnapshot,
        const FGovernmentProfile& GovernmentProfile,
        long long LastDailyExportIncome,
        EBuildingEra CurrentEra,
        FPoliticalDemandState& OutDemand,
        double& OutPriority)
    {
        if (!IsFactionAvailableInEra(Faction, CurrentEra))
            return false;

        const auto& FactionSnapshot =
            PoliticalSnapshot.Factions[static_cast<size_t>(Faction)];

        const int EraIndex = GetEraIndex(CurrentEra);
        const float DemandMultiplier =
            GameConstants::MainWorld::EraDemandThresholdMultipliers[EraIndex];
        const float MemberMultiplier =
            GameConstants::MainWorld::EraFactionMemberMinMultipliers[EraIndex];
        const int ScaledMemberMin = static_cast<int>(
            static_cast<float>(MWDemand::FactionMemberMinCount) *
            MemberMultiplier + 0.5f);

        if (FactionSnapshot.MemberCount < ScaledMemberMin ||
            FactionSnapshot.AverageApproval >=
                static_cast<double>(MWDemand::FactionApprovalThreshold))
        {
            return false;
        }

        const double ApprovalPressure =
            static_cast<double>(MWDemand::FactionApprovalThreshold) -
            FactionSnapshot.AverageApproval;
        FPoliticalDemandState Demand;
        Demand.Active = true;
        Demand.IssuerType = EPoliticalDemandIssuerType::Faction;
        Demand.IssuerIndex = static_cast<int>(Faction);
        Demand.Status = EPoliticalDemandStatus::PendingResponse;
        Demand.DurationDays = MWDemand::FactionDurationDays;
        Demand.RemainingDays = Demand.DurationDays;
        Demand.ModifierDurationDays = MWDemand::FactionModifierDurationDays;
        Demand.PenaltyBudgetDelta = 0;
        Demand.Title =
            std::wstring(GetPoliticalFactionName(Faction)) +
            L" " +
            GetPoliticalDemandStageLabel(EPoliticalDemandStage::Demand);
        OutPriority = 0.0;

        switch (Faction)
        {
        case EPoliticalFaction::Communists:
        {
            const auto& Tuning = MWDemand::Communists;
            const int CurrentHousing =
                static_cast<int>(std::lround(Snapshot.AverageHousing));
            const int ScaledIgnoreHousing = static_cast<int>(
                static_cast<float>(Tuning.IgnoreAtOrAboveHousing) *
                DemandMultiplier + 0.5f);
            if (CurrentHousing >= ScaledIgnoreHousing &&
                Snapshot.HomelessHouseholdCount <= Tuning.IgnoreAtOrBelowHomeless)
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
                ApprovalPressure * Tuning.ApprovalPriorityWeight +
                static_cast<double>((std::max)(
                    0,
                    Tuning.IgnoreAtOrAboveHousing - CurrentHousing)) *
                    Tuning.HousingDeficitPriorityWeight +
                static_cast<double>(Snapshot.HomelessHouseholdCount) *
                    Tuning.HomelessPriorityWeight;
            break;
        }
        case EPoliticalFaction::Capitalists:
        {
            const auto& Tuning = MWDemand::Capitalists;
            const int CurrentIncomeTax =
                GovernmentProfile.TaxPolicy.IncomeRatePercent;
            if (CurrentIncomeTax <= Tuning.IgnoreAtOrBelowValue)
                return false;

            Demand.ObjectiveType =
                EPoliticalDemandObjectiveType::IncomeTaxCeiling;
            Demand.TargetValue = Tuning.TargetValue;
            Demand.CurrentValue = CurrentIncomeTax;
            Demand.Summary = L"소득세 인하와 투자 여건 개선을 요구합니다.";
            Demand.ObjectiveText =
                L"소득세 " + std::to_wstring(Demand.TargetValue) + L"% 이하";
            Demand.RewardBudgetDelta = Tuning.RewardBudgetDelta;
            Demand.RewardFactionApprovalDelta = Tuning.RewardApprovalDelta;
            Demand.PenaltyFactionApprovalDelta = Tuning.PenaltyApprovalDelta;
            OutPriority =
                ApprovalPressure * Tuning.ApprovalPriorityWeight +
                static_cast<double>(CurrentIncomeTax - Demand.TargetValue) *
                    Tuning.ExcessPriorityWeight;
            break;
        }
        case EPoliticalFaction::Religious:
        {
            const auto& Tuning = MWDemand::Religious;
            const int CurrentFaith =
                static_cast<int>(std::lround(Snapshot.AverageFaith));
            const int FaithTargetMax = (std::min)(
                Tuning.TargetMax,
                ResolveFaithDemandTargetMax(CurrentEra));
            if (CurrentFaith >= static_cast<int>(
                    static_cast<float>(Tuning.IgnoreAtOrAboveValue) *
                    DemandMultiplier + 0.5f))
                return false;
            if (FaithTargetMax < Tuning.TargetMin)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Faith;
            Demand.TargetValue = (std::max)(
                Tuning.TargetMin,
                (std::min)(FaithTargetMax, CurrentFaith + Tuning.TargetDelta));
            if (Demand.TargetValue <= CurrentFaith)
                return false;
            Demand.CurrentValue = CurrentFaith;
            Demand.Summary = L"신앙 만족도 회복과 종교 서비스 강화를 요구합니다.";
            Demand.ObjectiveText =
                L"평균 신앙 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            Demand.RewardBudgetDelta = Tuning.RewardBudgetDelta;
            Demand.RewardFactionApprovalDelta = Tuning.RewardApprovalDelta;
            Demand.PenaltyFactionApprovalDelta = Tuning.PenaltyApprovalDelta;
            OutPriority =
                ApprovalPressure * Tuning.ApprovalPriorityWeight +
                static_cast<double>((std::max)(
                    0,
                    Tuning.IgnoreAtOrAboveValue - CurrentFaith)) *
                    Tuning.DeficitPriorityWeight;
            break;
        }
        case EPoliticalFaction::Militarists:
        {
            const auto& Tuning = MWDemand::Militarists;
            const int CurrentSecurity =
                static_cast<int>(std::lround(Snapshot.AverageSecurity));
            if (CurrentSecurity >= static_cast<int>(
                    static_cast<float>(Tuning.IgnoreAtOrAboveValue) *
                    DemandMultiplier + 0.5f))
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Security;
            Demand.TargetValue =
                (std::max)(
                    Tuning.TargetMin,
                    (std::min)(
                        Tuning.TargetMax,
                        CurrentSecurity + Tuning.TargetDelta));
            Demand.CurrentValue = CurrentSecurity;
            Demand.Summary = L"치안 안정과 군사 통제를 강화하라고 압박합니다.";
            Demand.ObjectiveText =
                L"평균 치안 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            Demand.RewardBudgetDelta = Tuning.RewardBudgetDelta;
            Demand.RewardFactionApprovalDelta = Tuning.RewardApprovalDelta;
            Demand.PenaltyFactionApprovalDelta = Tuning.PenaltyApprovalDelta;
            OutPriority =
                ApprovalPressure * Tuning.ApprovalPriorityWeight +
                static_cast<double>((std::max)(
                    0,
                    Tuning.IgnoreAtOrAboveValue - CurrentSecurity)) *
                    Tuning.DeficitPriorityWeight;
            break;
        }
        case EPoliticalFaction::Environmentalists:
        {
            const auto& Tuning = MWDemand::Environmentalists;
            const int CurrentHealth =
                static_cast<int>(std::lround(Snapshot.AverageHealth));
            if (CurrentHealth >= static_cast<int>(
                    static_cast<float>(Tuning.IgnoreAtOrAboveValue) *
                    DemandMultiplier + 0.5f))
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Health;
            Demand.TargetValue =
                (std::max)(
                    Tuning.TargetMin,
                    (std::min)(
                        Tuning.TargetMax,
                        CurrentHealth + Tuning.TargetDelta));
            Demand.CurrentValue = CurrentHealth;
            Demand.Summary = L"보건과 환경 악화를 바로잡으라고 요구합니다.";
            Demand.ObjectiveText =
                L"평균 보건 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            Demand.RewardBudgetDelta = Tuning.RewardBudgetDelta;
            Demand.RewardFactionApprovalDelta = Tuning.RewardApprovalDelta;
            Demand.PenaltyFactionApprovalDelta = Tuning.PenaltyApprovalDelta;
            OutPriority =
                ApprovalPressure * Tuning.ApprovalPriorityWeight +
                static_cast<double>((std::max)(
                    0,
                    Tuning.IgnoreAtOrAboveValue - CurrentHealth)) *
                    Tuning.DeficitPriorityWeight;
            break;
        }
        case EPoliticalFaction::Industrialists:
        {
            const auto& Tuning = MWDemand::Industrialists;
            const int CurrentExportIncome =
                (std::max)(0, static_cast<int>(LastDailyExportIncome));
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
                ApprovalPressure * Tuning.ApprovalPriorityWeight +
                static_cast<double>((std::max)(
                    0,
                    Demand.TargetValue - CurrentExportIncome)) /
                    static_cast<double>((std::max)(1.0f, Tuning.DeficitDivisor));
            break;
        }
        case EPoliticalFaction::Intellectuals:
        {
            const auto& Tuning = MWDemand::Intellectuals;
            const int CurrentFreedom =
                static_cast<int>(std::lround(Snapshot.AverageFreedom));
            if (CurrentFreedom >= static_cast<int>(
                    static_cast<float>(Tuning.IgnoreAtOrAboveValue) *
                    DemandMultiplier + 0.5f))
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Freedom;
            Demand.TargetValue =
                (std::max)(
                    Tuning.TargetMin,
                    (std::min)(
                        Tuning.TargetMax,
                        CurrentFreedom + Tuning.TargetDelta));
            Demand.CurrentValue = CurrentFreedom;
            Demand.Summary = L"자유와 개방성을 회복하라고 요구합니다.";
            Demand.ObjectiveText =
                L"평균 자유 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            Demand.RewardBudgetDelta = Tuning.RewardBudgetDelta;
            Demand.RewardFactionApprovalDelta = Tuning.RewardApprovalDelta;
            Demand.PenaltyFactionApprovalDelta = Tuning.PenaltyApprovalDelta;
            OutPriority =
                ApprovalPressure * Tuning.ApprovalPriorityWeight +
                static_cast<double>((std::max)(
                    0,
                    Tuning.IgnoreAtOrAboveValue - CurrentFreedom)) *
                    Tuning.DeficitPriorityWeight;
            break;
        }
        case EPoliticalFaction::Conservatives:
        {
            const auto& Tuning = MWDemand::Conservatives;
            const int CurrentPropertyTax =
                GovernmentProfile.TaxPolicy.PropertyRatePercent;
            if (CurrentPropertyTax <= Tuning.IgnoreAtOrBelowValue)
                return false;

            Demand.ObjectiveType =
                EPoliticalDemandObjectiveType::PropertyTaxCeiling;
            Demand.TargetValue = Tuning.TargetValue;
            Demand.CurrentValue = CurrentPropertyTax;
            Demand.Summary = L"재산세 완화와 질서 회복을 동시에 요구합니다.";
            Demand.ObjectiveText =
                L"재산세 " + std::to_wstring(Demand.TargetValue) + L"% 이하";
            Demand.RewardBudgetDelta = Tuning.RewardBudgetDelta;
            Demand.RewardFactionApprovalDelta = Tuning.RewardApprovalDelta;
            Demand.PenaltyFactionApprovalDelta = Tuning.PenaltyApprovalDelta;
            OutPriority =
                ApprovalPressure * Tuning.ApprovalPriorityWeight +
                static_cast<double>(CurrentPropertyTax - Demand.TargetValue) *
                    Tuning.ExcessPriorityWeight;
            break;
        }
        default:
            return false;
        }

        if (Demand.ObjectiveType == EPoliticalDemandObjectiveType::None)
            return false;

        Demand.RewardText = BuildPoliticalDemandEffectText(
            Demand.RewardBudgetDelta,
            Demand.RewardFactionApprovalDelta,
            0,
            0,
            Demand.ModifierDurationDays);
        Demand.PenaltyText = BuildPoliticalDemandEffectText(
            Demand.PenaltyBudgetDelta,
            Demand.PenaltyFactionApprovalDelta,
            0,
            0,
            Demand.ModifierDurationDays);
        RefreshFactionDemandStagePresentation(Demand);
        OutDemand = std::move(Demand);
        return OutPriority > 0.0;
    }

    bool TryBuildForeignDemand(
        int ForeignPowerIndex,
        EBuildingEra CurrentEra,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount>& ForeignPowerStates,
        const std::vector<FTradeRouteRuntimeState>& ActiveTradeRoutes,
        FPoliticalDemandState& OutDemand,
        double& OutPriority)
    {
        if (ForeignPowerIndex < 0 ||
            ForeignPowerIndex >= TradeDiplomacyRuntime::GForeignPowerCount)
        {
            return false;
        }

        if (!TradeDiplomacyRuntime::IsForeignPowerActiveForEra(
                ForeignPowerIndex,
                CurrentEra))
        {
            return false;
        }

        const auto& ForeignState =
            ForeignPowerStates[static_cast<size_t>(ForeignPowerIndex)];
        const auto& GlobalTuning = MWDemand::ForeignGlobal;
        FPoliticalDemandState Demand;
        Demand.Active = true;
        Demand.IssuerType = EPoliticalDemandIssuerType::ForeignPower;
        Demand.IssuerIndex = ForeignPowerIndex;
        Demand.Status = EPoliticalDemandStatus::PendingResponse;
        Demand.DurationDays = MWDemand::ForeignDurationDays;
        Demand.RemainingDays = Demand.DurationDays;
        Demand.Title =
            std::wstring(MainWorldTradeRuntime::GetForeignPowerName(
                ForeignPowerIndex,
                CurrentEra)) +
            L" 요구";
        Demand.RewardBudgetDelta = GlobalTuning.RewardBudgetDelta;
        Demand.PenaltyBudgetDelta = 0;
        Demand.RewardForeignRelationDelta = GlobalTuning.RewardRelationDelta;
        Demand.RewardForeignStandingDelta = GlobalTuning.RewardStandingDelta;
        Demand.PenaltyForeignRelationDelta = GlobalTuning.PenaltyRelationDelta;
        Demand.PenaltyForeignStandingDelta = GlobalTuning.PenaltyStandingDelta;
        OutPriority =
            static_cast<double>((std::max)(
                0,
                GlobalTuning.RelationPressureStart - ForeignState.Relation)) *
                GlobalTuning.RelationPriorityWeight +
            static_cast<double>((std::max)(
                0,
                GlobalTuning.StandingPressureStart - ForeignState.Standing)) *
                GlobalTuning.StandingPriorityWeight;

        switch (ForeignPowerIndex)
        {
        case 0:
        {
            const auto& Tuning = MWDemand::China;
            const int ActiveRouteCount =
                MainWorldTradeRuntime::CountActiveTradeRoutesForPower(
                    ActiveTradeRoutes,
                    ForeignPowerIndex);
            if (ActiveRouteCount >= Tuning.IgnoreAtOrAboveActiveRoutes)
                return false;

            Demand.ObjectiveType =
                EPoliticalDemandObjectiveType::ActiveTradeRoutes;
            Demand.TargetValue = Tuning.TargetValue;
            Demand.CurrentValue = ActiveRouteCount;
            Demand.Summary = L"무역 계약을 늘려 교역 규모를 키우라고 요구합니다.";
            Demand.ObjectiveText =
                std::wstring(MainWorldTradeRuntime::GetForeignPowerName(
                    ForeignPowerIndex,
                    CurrentEra)) +
                L"과 활성 무역로 " +
                std::to_wstring(Demand.TargetValue) +
                L"개 이상";
            OutPriority +=
                static_cast<double>(Demand.TargetValue - ActiveRouteCount) *
                Tuning.PriorityWeight;
            break;
        }
        case 1:
        {
            const auto& Tuning = MWDemand::Russia;
            const int CurrentSecurity =
                static_cast<int>(std::lround(Snapshot.AverageSecurity));
            if (CurrentSecurity >= Tuning.IgnoreAtOrAboveValue)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Security;
            Demand.TargetValue = Tuning.TargetValue;
            Demand.CurrentValue = CurrentSecurity;
            Demand.Summary = L"군사 협력을 위해 치안 확보를 요구합니다.";
            Demand.ObjectiveText =
                L"평균 치안 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            OutPriority +=
                static_cast<double>((std::max)(
                    0,
                    Tuning.IgnoreAtOrAboveValue - CurrentSecurity)) *
                Tuning.PriorityWeight;
            break;
        }
        case 2:
        {
            const auto& Tuning = MWDemand::UnitedStates;
            const int CurrentFreedom =
                static_cast<int>(std::lround(Snapshot.AverageFreedom));
            if (CurrentFreedom >= Tuning.IgnoreAtOrAboveValue)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Freedom;
            Demand.TargetValue = Tuning.TargetValue;
            Demand.CurrentValue = CurrentFreedom;
            Demand.Summary = L"대외 협력 조건으로 자유 확대를 요구합니다.";
            Demand.ObjectiveText =
                L"평균 자유 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            OutPriority +=
                static_cast<double>((std::max)(
                    0,
                    Tuning.IgnoreAtOrAboveValue - CurrentFreedom)) *
                Tuning.PriorityWeight;
            break;
        }
        case 3:
        {
            const auto& Tuning = MWDemand::MiddleEast;
            const int CurrentFaith =
                static_cast<int>(std::lround(Snapshot.AverageFaith));
            if (CurrentFaith >= Tuning.IgnoreAtOrAboveValue)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Faith;
            Demand.TargetValue = Tuning.TargetValue;
            Demand.CurrentValue = CurrentFaith;
            Demand.Summary = L"종교 기반 안정을 위해 신앙 서비스를 요구합니다.";
            Demand.ObjectiveText =
                L"평균 신앙 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            OutPriority +=
                static_cast<double>((std::max)(
                    0,
                    Tuning.IgnoreAtOrAboveValue - CurrentFaith)) *
                Tuning.PriorityWeight;
            break;
        }
        case 4:
        default:
        {
            const auto& Tuning = MWDemand::EuropeanUnion;
            const int CurrentHealth =
                static_cast<int>(std::lround(Snapshot.AverageHealth));
            if (CurrentHealth >= Tuning.IgnoreAtOrAboveValue)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Health;
            Demand.TargetValue = Tuning.TargetValue;
            Demand.CurrentValue = CurrentHealth;
            Demand.Summary = L"협력 유지를 위해 보건 수준 개선을 요구합니다.";
            Demand.ObjectiveText =
                L"평균 보건 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            OutPriority +=
                static_cast<double>((std::max)(
                    0,
                    Tuning.IgnoreAtOrAboveValue - CurrentHealth)) *
                Tuning.PriorityWeight;
            break;
        }
        }

        Demand.RewardText = BuildPoliticalDemandEffectText(
            Demand.RewardBudgetDelta,
            0,
            Demand.RewardForeignRelationDelta,
            Demand.RewardForeignStandingDelta,
            0);
        Demand.PenaltyText = BuildPoliticalDemandEffectText(
            Demand.PenaltyBudgetDelta,
            0,
            Demand.PenaltyForeignRelationDelta,
            Demand.PenaltyForeignStandingDelta,
            0);
        OutDemand = std::move(Demand);
        return OutPriority > 0.0;
    }

    void ApplyPoliticalDemandBudgetDelta(
        long long Delta,
        long long& InOutBudget,
        long long& InOutLastDailyNetChange)
    {
        if (Delta == 0)
            return;

        InOutBudget += Delta;
        InOutLastDailyNetChange += Delta;
    }

    void SetPoliticalDemandResolutionNotice(
        FPoliticalDemandNotice& OutNotice,
        const FPoliticalDemandState& Demand,
        bool Positive,
        const wchar_t* StatusLabel)
    {
        OutNotice = FPoliticalDemandNotice();
        OutNotice.ActiveDemand = false;
        OutNotice.Positive = Positive;
        OutNotice.RemainingDays = MWDemand::NoticeDurationDays;
        OutNotice.Title = Demand.Title;
        OutNotice.Summary =
            std::wstring(StatusLabel ? StatusLabel : L"처리") +
            L" / " +
            (Positive ? Demand.RewardText : Demand.PenaltyText);
    }

    FPoliticalDemandNotice BuildPriorityDemandNotice(
        const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
            FactionDemands,
        const std::array<
            FPoliticalDemandState,
            TradeDiplomacyRuntime::GForeignPowerCount>& ForeignDemands)
    {
        const FPoliticalDemandState* BestDemand = nullptr;

        const auto ConsiderDemand =
            [&](const FPoliticalDemandState& Demand)
            {
                if (!Demand.Active)
                    return;

                if (!BestDemand)
                {
                    BestDemand = &Demand;
                    return;
                }

                const bool DemandPending =
                    Demand.Status == EPoliticalDemandStatus::PendingResponse;
                const bool BestPending =
                    BestDemand->Status == EPoliticalDemandStatus::PendingResponse;

                if (DemandPending != BestPending)
                {
                    if (DemandPending)
                        BestDemand = &Demand;
                    return;
                }

                if (Demand.RemainingDays < BestDemand->RemainingDays)
                {
                    BestDemand = &Demand;
                    return;
                }

                if (Demand.RemainingDays == BestDemand->RemainingDays &&
                    Demand.Title < BestDemand->Title)
                {
                    BestDemand = &Demand;
                }
            };

        for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
        {
            ConsiderDemand(FactionDemands[static_cast<size_t>(Index)]);
        }

        for (int Index = 0;
            Index < TradeDiplomacyRuntime::GForeignPowerCount;
            ++Index)
        {
            ConsiderDemand(ForeignDemands[static_cast<size_t>(Index)]);
        }

        if (!BestDemand)
            return FPoliticalDemandNotice();

        FPoliticalDemandNotice Notice;
        Notice.ActiveDemand = true;
        Notice.Positive = false;
        Notice.RemainingDays = BestDemand->RemainingDays;
        Notice.Title = BestDemand->Title;
        Notice.Summary =
            (BestDemand->Status == EPoliticalDemandStatus::Accepted ?
                L"수행 중 / " :
                L"응답 대기 / ") +
            BestDemand->ObjectiveText +
            L" / " +
            std::to_wstring((std::max)(0, BestDemand->RemainingDays)) +
            L"일";
        return Notice;
    }
} // namespace

void CMainWorldPoliticalDemandService::Reset()
{
    mPoliticalDemandNotice = FPoliticalDemandNotice();
    mFactionDemands = {};
    mFactionPressureDays = {};
    mFactionDemandCooldownDays = {};
    mFactionDemandModifierDays = {};
    mForeignDemands = {};
    mForeignDemandCooldownDays = {};
}

bool CMainWorldPoliticalDemandService::InjectScenarioDemand(
    FPoliticalDemandState Demand)
{
    if (Demand.IssuerType == EPoliticalDemandIssuerType::None ||
        Demand.IssuerIndex < 0)
    {
        return false;
    }

    Demand.Active = true;

    if (Demand.Status == EPoliticalDemandStatus::None)
        Demand.Status = EPoliticalDemandStatus::PendingResponse;

    if (Demand.DurationDays <= 0 && Demand.RemainingDays > 0)
        Demand.DurationDays = Demand.RemainingDays;

    if (Demand.RemainingDays <= 0 && Demand.DurationDays > 0)
        Demand.RemainingDays = Demand.DurationDays;

    switch (Demand.IssuerType)
    {
    case EPoliticalDemandIssuerType::Faction:
        if (Demand.IssuerIndex >= GPoliticalFactionCount)
            return false;

        mFactionDemandCooldownDays[static_cast<size_t>(Demand.IssuerIndex)] = 0;
        mFactionDemands[static_cast<size_t>(Demand.IssuerIndex)] =
            std::move(Demand);
        break;
    case EPoliticalDemandIssuerType::ForeignPower:
        if (Demand.IssuerIndex >= TradeDiplomacyRuntime::GForeignPowerCount)
            return false;

        mForeignDemandCooldownDays[static_cast<size_t>(Demand.IssuerIndex)] = 0;
        mForeignDemands[static_cast<size_t>(Demand.IssuerIndex)] =
            std::move(Demand);
        break;
    case EPoliticalDemandIssuerType::None:
    default:
        return false;
    }

    mPoliticalDemandNotice =
        BuildPriorityDemandNotice(mFactionDemands, mForeignDemands);
    return true;
}

bool CMainWorldPoliticalDemandService::RespondPoliticalDemand(
    EPoliticalDemandIssuerType IssuerType,
    int IssuerIndex,
    bool Accept,
    std::wstring& OutMessage,
    const FContext& Context,
    FRefreshRequests& OutRefreshRequests)
{
    if (!Context.World)
    {
        OutMessage = L"월드 상태를 확인할 수 없습니다.";
        return false;
    }

    FPoliticalDemandState* Demand = nullptr;
    int* CooldownDays = nullptr;
    const int SafeFactionIndex =
        (std::max)(0, (std::min)(GPoliticalFactionCount - 1, IssuerIndex));
    const int SafeForeignIndex =
        (std::max)(
            0,
            (std::min)(TradeDiplomacyRuntime::GForeignPowerCount - 1, IssuerIndex));

    switch (IssuerType)
    {
    case EPoliticalDemandIssuerType::Faction:
        Demand = &mFactionDemands[static_cast<size_t>(SafeFactionIndex)];
        CooldownDays =
            &mFactionDemandCooldownDays[static_cast<size_t>(SafeFactionIndex)];
        break;
    case EPoliticalDemandIssuerType::ForeignPower:
        Demand = &mForeignDemands[static_cast<size_t>(SafeForeignIndex)];
        CooldownDays =
            &mForeignDemandCooldownDays[static_cast<size_t>(SafeForeignIndex)];
        break;
    case EPoliticalDemandIssuerType::None:
    default:
        OutMessage = L"요구 발신자를 확인할 수 없습니다.";
        return false;
    }

    if (!Demand || !Demand->Active)
    {
        OutMessage = L"현재 응답할 수 있는 요구가 없습니다.";
        return false;
    }

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(Context.World);

    const auto ApplyFactionModifier =
        [&](int FactionIndex, int Delta, int DurationDays)
        {
            if (Delta == 0 ||
                FactionIndex < 0 ||
                FactionIndex >= GPoliticalFactionCount)
            {
                return;
            }

            int& Modifier =
                Context.GovernmentProfile.FactionApprovalModifiers[
                    static_cast<size_t>(FactionIndex)];
            Modifier = (std::max)(-25, (std::min)(25, Modifier + Delta));
            mFactionDemandModifierDays[static_cast<size_t>(FactionIndex)] =
                (std::max)(
                    mFactionDemandModifierDays[static_cast<size_t>(FactionIndex)],
                    (std::max)(1, DurationDays));
        };

    const auto ApplyDemandOutcome =
        [&](bool Success, const wchar_t* StatusLabel)
        {
            const long long BudgetDelta =
                Success ? Demand->RewardBudgetDelta : Demand->PenaltyBudgetDelta;
            const int FactionApprovalDelta =
                Success ?
                    Demand->RewardFactionApprovalDelta :
                    Demand->PenaltyFactionApprovalDelta;
            const int RelationDelta =
                Success ?
                    Demand->RewardForeignRelationDelta :
                    Demand->PenaltyForeignRelationDelta;
            const int StandingDelta =
                Success ?
                    Demand->RewardForeignStandingDelta :
                    Demand->PenaltyForeignStandingDelta;

            ApplyPoliticalDemandBudgetDelta(
                BudgetDelta,
                Context.NationalBudget,
                Context.LastDailyNetChange);

            if (Demand->IssuerType == EPoliticalDemandIssuerType::Faction)
            {
                ApplyFactionModifier(
                    Demand->IssuerIndex,
                    FactionApprovalDelta,
                    Demand->ModifierDurationDays > 0 ?
                        Demand->ModifierDurationDays :
                        MWDemand::FactionModifierDurationDays);
                OutRefreshRequests.RefreshPoliticalSnapshot = true;
            }
            else if (
                Demand->IssuerType == EPoliticalDemandIssuerType::ForeignPower &&
                Demand->IssuerIndex >= 0 &&
                Demand->IssuerIndex < TradeDiplomacyRuntime::GForeignPowerCount)
            {
                MainWorldTradeRuntime::ApplyForeignDemandStandingDelta(
                    Context.ForeignPowerStandingStates[
                        static_cast<size_t>(Demand->IssuerIndex)],
                    RelationDelta,
                    StandingDelta);
                OutRefreshRequests.RefreshForeignTradeDiplomacy = true;
                OutRefreshRequests.RefreshWorldMarketPrices = true;
            }

            if (CooldownDays)
            {
                *CooldownDays =
                    Demand->IssuerType == EPoliticalDemandIssuerType::Faction ?
                        MWDemand::FactionCooldownDays :
                        MWDemand::ForeignCooldownDays;
            }

            SetPoliticalDemandResolutionNotice(
                mPoliticalDemandNotice,
                *Demand,
                Success,
                StatusLabel);
            *Demand = FPoliticalDemandState();
        };

    if (!Accept)
    {
        const std::wstring DemandTitle = Demand->Title;
        const bool WasAccepted =
            Demand->Status == EPoliticalDemandStatus::Accepted;
        const bool TriggerFactionRevolt =
            Demand->IssuerType == EPoliticalDemandIssuerType::Faction &&
            Demand->Stage == EPoliticalDemandStage::Revolt &&
            Demand->IssuerIndex >= 0 &&
            Demand->IssuerIndex < GPoliticalFactionCount;
        const int RevoltFactionIndex = Demand->IssuerIndex;
        Demand->Status = EPoliticalDemandStatus::Rejected;
        ApplyDemandOutcome(
            false,
            WasAccepted ?
                L"요구 포기" :
                L"요구 거절");

        if (TriggerFactionRevolt)
        {
            OutRefreshRequests.TriggerFactionRevolts[
                static_cast<size_t>(RevoltFactionIndex)] = true;
        }

        OutMessage =
            DemandTitle.empty() ?
                L"요구를 거절했습니다." :
                DemandTitle + L" 거절";
        return true;
    }

    if (Demand->Status == EPoliticalDemandStatus::Accepted)
    {
        OutMessage = L"이미 수락한 요구입니다.";
        return false;
    }

    Demand->Status = EPoliticalDemandStatus::Accepted;
    Demand->CurrentValue = EvaluatePoliticalDemandCurrentValue(
        *Demand,
        Snapshot,
        Context.GovernmentProfile,
        Context.LastDailyExportIncome,
        Context.ForeignPowerStates,
        Context.ActiveTradeRoutes);
    OutMessage = Demand->Title + L" 수락";

    if (IsPoliticalDemandSatisfied(*Demand))
    {
        ApplyDemandOutcome(true, L"요구 완료");
        OutMessage += L" / 즉시 완료";
        return true;
    }

    mPoliticalDemandNotice =
        BuildPriorityDemandNotice(mFactionDemands, mForeignDemands);
    return true;
}

CMainWorldPoliticalDemandService::FRefreshRequests
    CMainWorldPoliticalDemandService::Tick(const FContext& Context)
{
    FRefreshRequests RefreshRequests;

    if (!Context.World)
        return RefreshRequests;

    if (!mPoliticalDemandNotice.ActiveDemand &&
        mPoliticalDemandNotice.RemainingDays > 0)
    {
        --mPoliticalDemandNotice.RemainingDays;

        if (mPoliticalDemandNotice.RemainingDays <= 0)
            mPoliticalDemandNotice = FPoliticalDemandNotice();
    }

    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        if (mFactionDemandCooldownDays[static_cast<size_t>(Index)] > 0)
            --mFactionDemandCooldownDays[static_cast<size_t>(Index)];
    }

    for (int Index = 0;
        Index < TradeDiplomacyRuntime::GForeignPowerCount;
        ++Index)
    {
        if (mForeignDemandCooldownDays[static_cast<size_t>(Index)] > 0)
            --mForeignDemandCooldownDays[static_cast<size_t>(Index)];
    }

    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        int& RemainingDays =
            mFactionDemandModifierDays[static_cast<size_t>(Index)];

        if (RemainingDays <= 0)
            continue;

        --RemainingDays;

        if (RemainingDays <= 0 &&
            Context.GovernmentProfile.FactionApprovalModifiers[
                static_cast<size_t>(Index)] != 0)
        {
            Context.GovernmentProfile.FactionApprovalModifiers[
                static_cast<size_t>(Index)] = 0;
            RefreshRequests.RefreshPoliticalSnapshot = true;
        }
    }

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(Context.World);

    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        const EPoliticalFaction Faction =
            static_cast<EPoliticalFaction>(Index);

        if (IsFactionAvailableInEra(Faction, Context.CurrentEra))
            continue;

        mFactionDemands[static_cast<size_t>(Index)] = FPoliticalDemandState();
        mFactionPressureDays[static_cast<size_t>(Index)] = 0;
        mFactionDemandCooldownDays[static_cast<size_t>(Index)] = 0;
        mFactionDemandModifierDays[static_cast<size_t>(Index)] = 0;
    }

    int ActiveFactionDemandCount = CountActiveFactionDemands(mFactionDemands);

    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        const EPoliticalFaction Faction =
            static_cast<EPoliticalFaction>(Index);

        if (!IsFactionAvailableInEra(Faction, Context.CurrentEra))
            continue;

        const auto& FactionSnapshot =
            Context.PoliticalSnapshot.Factions[static_cast<size_t>(Index)];
        int& PressureDays = mFactionPressureDays[static_cast<size_t>(Index)];
        const int PreviousPressureDays = PressureDays;

        if (FactionSnapshot.AverageApproval < GWarningApprovalThreshold)
        {
            ++PressureDays;
        }
        else
        {
            PressureDays =
                (std::max)(0, PressureDays - GPressureDecayPerSafeDay);
        }

        if (mFactionDemandCooldownDays[static_cast<size_t>(Index)] > 0)
            continue;

        FPoliticalDemandState& ActiveDemand =
            mFactionDemands[static_cast<size_t>(Index)];

        if (PressureDays >= GRevoltPressureThreshold)
        {
            if (ActiveDemand.Active)
            {
                PromoteFactionDemandStage(
                    ActiveDemand,
                    EPoliticalDemandStage::Revolt);
            }
            else if (
                ActiveFactionDemandCount < MWDemand::MaxActiveFactionDemandCount)
            {
                FPoliticalDemandState CandidateDemand;
                double CandidatePriority = 0.0;

                if (TryBuildFactionDemand(
                        static_cast<EPoliticalFaction>(Index),
                        Snapshot,
                        Context.PoliticalSnapshot,
                        Context.GovernmentProfile,
                        Context.LastDailyExportIncome,
                        Context.CurrentEra,
                        CandidateDemand,
                        CandidatePriority))
                {
                    PromoteFactionDemandStage(
                        CandidateDemand,
                        EPoliticalDemandStage::Revolt);
                    ActiveDemand = std::move(CandidateDemand);
                    ++ActiveFactionDemandCount;
                }
            }

            continue;
        }

        if (PressureDays >= GUltimatumPressureThreshold)
        {
            if (ActiveDemand.Active)
            {
                PromoteFactionDemandStage(
                    ActiveDemand,
                    EPoliticalDemandStage::Ultimatum);
            }
            else if (
                ActiveFactionDemandCount < MWDemand::MaxActiveFactionDemandCount)
            {
                FPoliticalDemandState CandidateDemand;
                double CandidatePriority = 0.0;

                if (TryBuildFactionDemand(
                        static_cast<EPoliticalFaction>(Index),
                        Snapshot,
                        Context.PoliticalSnapshot,
                        Context.GovernmentProfile,
                        Context.LastDailyExportIncome,
                        Context.CurrentEra,
                        CandidateDemand,
                        CandidatePriority))
                {
                    PromoteFactionDemandStage(
                        CandidateDemand,
                        EPoliticalDemandStage::Ultimatum);
                    ActiveDemand = std::move(CandidateDemand);
                    ++ActiveFactionDemandCount;
                }
            }

            continue;
        }

        if (PreviousPressureDays < GWarningPressureThreshold &&
            PressureDays >= GWarningPressureThreshold &&
            !ActiveDemand.Active)
        {
            mPoliticalDemandNotice = BuildFactionWarningNotice(
                Faction,
                PressureDays);
        }
    }

    const auto ApplyFactionModifier =
        [&](int FactionIndex, int Delta, int DurationDays)
        {
            if (Delta == 0 ||
                FactionIndex < 0 ||
                FactionIndex >= GPoliticalFactionCount)
            {
                return;
            }

            int& Modifier =
                Context.GovernmentProfile.FactionApprovalModifiers[
                    static_cast<size_t>(FactionIndex)];
            Modifier = (std::max)(-25, (std::min)(25, Modifier + Delta));
            mFactionDemandModifierDays[static_cast<size_t>(FactionIndex)] =
                (std::max)(
                    mFactionDemandModifierDays[static_cast<size_t>(FactionIndex)],
                    (std::max)(1, DurationDays));
        };

    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        FPoliticalDemandState& Demand =
            mFactionDemands[static_cast<size_t>(Index)];

        if (!Demand.Active)
            continue;

        Demand.CurrentValue = EvaluatePoliticalDemandCurrentValue(
            Demand,
            Snapshot,
            Context.GovernmentProfile,
            Context.LastDailyExportIncome,
            Context.ForeignPowerStates,
            Context.ActiveTradeRoutes);

        if (Demand.Status == EPoliticalDemandStatus::Accepted &&
            IsPoliticalDemandSatisfied(Demand))
        {
            ApplyPoliticalDemandBudgetDelta(
                Demand.RewardBudgetDelta,
                Context.NationalBudget,
                Context.LastDailyNetChange);
            ApplyFactionModifier(
                Demand.IssuerIndex,
                Demand.RewardFactionApprovalDelta,
                Demand.ModifierDurationDays > 0 ?
                    Demand.ModifierDurationDays :
                    MWDemand::FactionModifierDurationDays);
            mFactionDemandCooldownDays[static_cast<size_t>(Index)] =
                MWDemand::FactionCooldownDays;
            SetPoliticalDemandResolutionNotice(
                mPoliticalDemandNotice,
                Demand,
                true,
                L"요구 완료");
            Demand = FPoliticalDemandState();
            RefreshRequests.RefreshPoliticalSnapshot = true;
            continue;
        }

        if (Demand.RemainingDays > 0)
            --Demand.RemainingDays;

        if (Demand.RemainingDays > 0)
            continue;

        const bool WasAccepted =
            Demand.Status == EPoliticalDemandStatus::Accepted;
        const bool TriggerFactionRevolt =
            Demand.Stage == EPoliticalDemandStage::Revolt &&
            Demand.IssuerIndex >= 0 &&
            Demand.IssuerIndex < GPoliticalFactionCount;
        const int RevoltFactionIndex = Demand.IssuerIndex;
        Demand.Status = EPoliticalDemandStatus::Failed;
        ApplyPoliticalDemandBudgetDelta(
            Demand.PenaltyBudgetDelta,
            Context.NationalBudget,
            Context.LastDailyNetChange);
        ApplyFactionModifier(
            Demand.IssuerIndex,
            Demand.PenaltyFactionApprovalDelta,
            Demand.ModifierDurationDays > 0 ?
                Demand.ModifierDurationDays :
                MWDemand::FactionModifierDurationDays);
        mFactionDemandCooldownDays[static_cast<size_t>(Index)] =
            MWDemand::FactionCooldownDays;
        SetPoliticalDemandResolutionNotice(
            mPoliticalDemandNotice,
            Demand,
            false,
            WasAccepted ?
                L"요구 실패" :
                L"요구 만료");

        if (TriggerFactionRevolt)
        {
            RefreshRequests.TriggerFactionRevolts[
                static_cast<size_t>(RevoltFactionIndex)] = true;
        }

        Demand = FPoliticalDemandState();
        RefreshRequests.RefreshPoliticalSnapshot = true;
    }

    for (int Index = 0;
        Index < TradeDiplomacyRuntime::GForeignPowerCount;
        ++Index)
    {
        FPoliticalDemandState& Demand =
            mForeignDemands[static_cast<size_t>(Index)];

        if (!Demand.Active)
            continue;

        Demand.CurrentValue = EvaluatePoliticalDemandCurrentValue(
            Demand,
            Snapshot,
            Context.GovernmentProfile,
            Context.LastDailyExportIncome,
            Context.ForeignPowerStates,
            Context.ActiveTradeRoutes);

        if (Demand.Status == EPoliticalDemandStatus::Accepted &&
            IsPoliticalDemandSatisfied(Demand))
        {
            ApplyPoliticalDemandBudgetDelta(
                Demand.RewardBudgetDelta,
                Context.NationalBudget,
                Context.LastDailyNetChange);
            MainWorldTradeRuntime::ApplyForeignDemandStandingDelta(
                Context.ForeignPowerStandingStates[static_cast<size_t>(Index)],
                Demand.RewardForeignRelationDelta,
                Demand.RewardForeignStandingDelta);
            mForeignDemandCooldownDays[static_cast<size_t>(Index)] =
                MWDemand::ForeignCooldownDays;
            SetPoliticalDemandResolutionNotice(
                mPoliticalDemandNotice,
                Demand,
                true,
                L"요구 완료");
            Demand = FPoliticalDemandState();
            RefreshRequests.RefreshForeignTradeDiplomacy = true;
            RefreshRequests.RefreshWorldMarketPrices = true;
            continue;
        }

        if (Demand.RemainingDays > 0)
            --Demand.RemainingDays;

        if (Demand.RemainingDays > 0)
            continue;

        ApplyPoliticalDemandBudgetDelta(
            Demand.PenaltyBudgetDelta,
            Context.NationalBudget,
            Context.LastDailyNetChange);
        MainWorldTradeRuntime::ApplyForeignDemandStandingDelta(
            Context.ForeignPowerStandingStates[static_cast<size_t>(Index)],
            Demand.PenaltyForeignRelationDelta,
            Demand.PenaltyForeignStandingDelta);
        mForeignDemandCooldownDays[static_cast<size_t>(Index)] =
            MWDemand::ForeignCooldownDays;
        SetPoliticalDemandResolutionNotice(
            mPoliticalDemandNotice,
            Demand,
            false,
            Demand.Status == EPoliticalDemandStatus::Accepted ?
                L"요구 실패" :
                L"요구 만료");
        Demand = FPoliticalDemandState();
        RefreshRequests.RefreshForeignTradeDiplomacy = true;
        RefreshRequests.RefreshWorldMarketPrices = true;
    }

    ActiveFactionDemandCount = CountActiveFactionDemands(mFactionDemands);

    if (ActiveFactionDemandCount <
        MWDemand::MaxActiveFactionDemandCount)
    {
        bool FoundDemand = false;
        FPoliticalDemandState BestDemand;
        int BestIndex = -1;
        double BestPriority = 0.0;

        for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
        {
            if (mFactionDemands[static_cast<size_t>(Index)].Active ||
                mFactionDemandCooldownDays[static_cast<size_t>(Index)] > 0)
            {
                continue;
            }

            FPoliticalDemandState CandidateDemand;
            double CandidatePriority = 0.0;

            if (!TryBuildFactionDemand(
                    static_cast<EPoliticalFaction>(Index),
                    Snapshot,
                    Context.PoliticalSnapshot,
                    Context.GovernmentProfile,
                    Context.LastDailyExportIncome,
                    Context.CurrentEra,
                    CandidateDemand,
                    CandidatePriority))
            {
                continue;
            }

            if (!FoundDemand || CandidatePriority > BestPriority)
            {
                FoundDemand = true;
                BestDemand = std::move(CandidateDemand);
                BestIndex = Index;
                BestPriority = CandidatePriority;
            }
        }

        if (FoundDemand && BestIndex >= 0)
        {
            mFactionDemands[static_cast<size_t>(BestIndex)] = std::move(BestDemand);
            ++ActiveFactionDemandCount;
        }
    }

    if (CountActiveForeignDemands(mForeignDemands) <
        MWDemand::MaxActiveForeignDemandCount)
    {
        bool FoundDemand = false;
        FPoliticalDemandState BestDemand;
        int BestIndex = -1;
        double BestPriority = 0.0;

        for (int Index = 0;
            Index < TradeDiplomacyRuntime::GForeignPowerCount;
            ++Index)
        {
            if (mForeignDemands[static_cast<size_t>(Index)].Active ||
                mForeignDemandCooldownDays[static_cast<size_t>(Index)] > 0)
            {
                continue;
            }

            FPoliticalDemandState CandidateDemand;
            double CandidatePriority = 0.0;

            if (!TryBuildForeignDemand(
                    Index,
                    Context.CurrentEra,
                    Snapshot,
                    Context.ForeignPowerStates,
                    Context.ActiveTradeRoutes,
                    CandidateDemand,
                    CandidatePriority))
            {
                continue;
            }

            if (!FoundDemand || CandidatePriority > BestPriority)
            {
                FoundDemand = true;
                BestDemand = std::move(CandidateDemand);
                BestIndex = Index;
                BestPriority = CandidatePriority;
            }
        }

        if (FoundDemand && BestIndex >= 0)
            mForeignDemands[static_cast<size_t>(BestIndex)] = std::move(BestDemand);
    }

    const FPoliticalDemandNotice ActiveNotice =
        BuildPriorityDemandNotice(mFactionDemands, mForeignDemands);

    if (ActiveNotice.ActiveDemand)
    {
        mPoliticalDemandNotice = ActiveNotice;
    }
    else if (mPoliticalDemandNotice.ActiveDemand)
    {
        mPoliticalDemandNotice = FPoliticalDemandNotice();
    }

    return RefreshRequests;
}
