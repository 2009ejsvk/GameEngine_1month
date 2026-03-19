#include "TopHudDataProvider.h"
#include "UIStrings.h"
#include "../World/IWorldUIAccess.h"
#include "World/World.h"
#include <algorithm>
#include <cmath>
#include <cwchar>

namespace
{
    FVector4 MakeColor(
        unsigned char R,
        unsigned char G,
        unsigned char B,
        unsigned char A = 255)
    {
        return FVector4(
            static_cast<float>(R) / 255.f,
            static_cast<float>(G) / 255.f,
            static_cast<float>(B) / 255.f,
            static_cast<float>(A) / 255.f);
    }

    std::wstring FormatCurrency(long long Value)
    {
        bool Negative = false;
        unsigned long long AbsValue = 0;

        if (Value < 0)
        {
            Negative = true;
            AbsValue = static_cast<unsigned long long>(-Value);
        }
        else
        {
            AbsValue = static_cast<unsigned long long>(Value);
        }

        std::wstring Digits = std::to_wstring(AbsValue);

        for (int i = static_cast<int>(Digits.size()) - 3; i > 0; i -= 3)
        {
            Digits.insert(static_cast<size_t>(i), 1, L',');
        }

        if (Negative)
            Digits.insert(Digits.begin(), L'-');

        return L"$" + Digits;
    }

    std::wstring FormatDate(int Year, int Month, int Day)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%04d.%02d.%02d", Year, Month, Day);
        return Buffer;
    }

    std::wstring FormatInteger(int Value)
    {
        const bool Negative = Value < 0;
        unsigned int AbsValue = Negative ?
            static_cast<unsigned int>(-Value) :
            static_cast<unsigned int>(Value);
        std::wstring Digits = std::to_wstring(AbsValue);

        for (int i = static_cast<int>(Digits.size()) - 3; i > 0; i -= 3)
        {
            Digits.insert(static_cast<size_t>(i), 1, L',');
        }

        if (Negative)
            Digits.insert(Digits.begin(), L'-');

        return Digits;
    }

    std::wstring FormatHudDate(int Year, int Month, int Day)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%04d년 %d월 %d일", Year, Month, Day);
        return Buffer;
    }

    const wchar_t* GetCompactEraLabel(EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::WorldWars:
            return L"세계대전";
        case EBuildingEra::ColdWar:
            return L"냉전";
        case EBuildingEra::Modern:
            return L"현대";
        case EBuildingEra::Colonial:
        default:
            return L"식민지";
        }
    }

    std::wstring FormatTaxPolicyCompact(const FTaxPolicy& TaxPolicy)
    {
        return
            UIStrings::Get(L"top_hud.fragment.tax_policy_prefix") +
            std::to_wstring(TaxPolicy.ConsumptionRatePercent) +
            L"/" +
            std::to_wstring(TaxPolicy.IncomeRatePercent) +
            L"/" +
            std::to_wstring(TaxPolicy.PropertyRatePercent) +
            L"%";
    }

    const wchar_t* GetElectionWarningTierLabel(double Score)
    {
        if (Score >= 0.78)
            return UIStrings::Get(L"top_hud.warning.high").c_str();
        if (Score >= 0.52)
            return UIStrings::Get(L"top_hud.warning.caution").c_str();
        if (Score >= 0.32)
            return UIStrings::Get(L"top_hud.warning.check").c_str();
        return UIStrings::Get(L"top_hud.warning.stable").c_str();
    }

    std::wstring FormatOneDecimal(double Value)
    {
        wchar_t Buffer[32] = {};
        swprintf_s(Buffer, L"%.1f", Value);
        return Buffer;
    }

    bool HasElectionWarning(int DaysUntilElection, double Score)
    {
        return DaysUntilElection >= 0 &&
            DaysUntilElection <= 180 &&
            Score >= 0.32;
    }

    const wchar_t* GetFactionHudLabel(EPoliticalFaction Faction)
    {
        switch (Faction)
        {
        case EPoliticalFaction::Communists:
            return L"공산";
        case EPoliticalFaction::Capitalists:
            return L"자본";
        case EPoliticalFaction::Religious:
            return L"종교";
        case EPoliticalFaction::Militarists:
            return L"군부";
        case EPoliticalFaction::Environmentalists:
            return L"환경";
        case EPoliticalFaction::Industrialists:
            return L"산업";
        case EPoliticalFaction::Intellectuals:
            return L"지식";
        case EPoliticalFaction::Conservatives:
            return L"보수";
        default:
            return L"세력";
        }
    }

    std::wstring JoinFactionHudLabels(
        const std::vector<EPoliticalFaction>& Factions)
    {
        if (Factions.empty())
            return std::wstring();

        std::wstring Result;
        const int DisplayCount =
            (std::min)(2, static_cast<int>(Factions.size()));

        for (int Index = 0; Index < DisplayCount; ++Index)
        {
            if (!Result.empty())
                Result += L", ";

            Result += GetFactionHudLabel(Factions[static_cast<size_t>(Index)]);
        }

        const int ExtraCount =
            static_cast<int>(Factions.size()) - DisplayCount;

        if (ExtraCount > 0)
        {
            Result += L" 외 ";
            Result += std::to_wstring(ExtraCount);
        }

        return Result;
    }

    struct FPoliticalEscalationHudInfo
    {
        bool Active = false;
        bool Critical = false;
        std::wstring PrefixText;
        std::wstring SummaryText;
        FVector4 Color = FVector4(1.f, 1.f, 1.f, 1.f);
    };

    FPoliticalEscalationHudInfo BuildPoliticalEscalationHudInfo(
        const std::array<int, GPoliticalFactionCount>& PressureDays,
        const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
            DemandStates)
    {
        constexpr int GWarningPressureThreshold = 10;
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

        FPoliticalEscalationHudInfo Result;

        if (!RevoltFactions.empty())
        {
            Result.Active = true;
            Result.Critical = true;
            Result.PrefixText = L"[!!]";
            Result.SummaryText =
                UIStrings::Get(L"escalation.stage.revolt") +
                std::wstring(L" ") +
                JoinFactionHudLabels(RevoltFactions);
            Result.Color = MakeColor(238, 108, 90, 255);
            return Result;
        }

        if (!UltimatumFactions.empty())
        {
            Result.Active = true;
            Result.Critical = true;
            Result.PrefixText = L"[!!]";
            Result.SummaryText =
                UIStrings::Get(L"escalation.stage.ultimatum") +
                std::wstring(L" ") +
                JoinFactionHudLabels(UltimatumFactions);
            Result.Color = MakeColor(238, 108, 90, 255);
            return Result;
        }

        if (!WarningFactions.empty())
        {
            Result.Active = true;
            Result.Critical = false;
            Result.PrefixText = L"[!]";
            Result.SummaryText =
                UIStrings::Get(L"escalation.stage.warning") +
                std::wstring(L" ") +
                JoinFactionHudLabels(WarningFactions);
            Result.Color = MakeColor(240, 214, 124, 255);
        }

        return Result;
    }

    bool HasElectionSchedule(const FElectionStatus& ElectionStatus)
    {
        return !ElectionStatus.GameLost &&
            ElectionStatus.NextElectionYear > 0 &&
            ElectionStatus.NextElectionMonth > 0 &&
            ElectionStatus.NextElectionDay > 0;
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

    int CountMetElectionPromises(const FElectionStatus& ElectionStatus)
    {
        int Count = 0;

        for (int Index = 0; Index < GElectionPromiseCount; ++Index)
        {
            if (IsElectionPromiseMet(
                    ElectionStatus.ActivePromises[static_cast<size_t>(Index)]))
            {
                ++Count;
            }
        }

        return Count;
    }

    std::wstring BuildElectionPromiseHudSummary(
        const FElectionStatus& ElectionStatus)
    {
        const int ActiveCount = CountActiveElectionPromises(ElectionStatus);

        if (ActiveCount <= 0)
            return std::wstring();

        return
            L"공약 " +
            std::to_wstring(CountMetElectionPromises(ElectionStatus)) +
            L"/" +
            std::to_wstring(ActiveCount) +
            L" 이행";
    }

    std::wstring BuildEraTransitionHudSummary(
        const FEraTransitionState& TransitionState)
    {
        std::wstring Summary = TransitionState.Summary;

        for (size_t Index = 0; Index < Summary.size(); ++Index)
        {
            if (Summary[Index] == L'\n' || Summary[Index] == L'\r')
                Summary[Index] = L' ';
        }

        return Summary;
    }

    std::wstring BuildEraTransitionPopupTitle(
        const FEraProgressState& EraProgress,
        const FEraTransitionState& TransitionState)
    {
        if (!TransitionState.Title.empty())
            return TransitionState.Title;

        if (!EraProgress.HasNextEra)
            return L"시대 전환";

        switch (EraProgress.NextEra)
        {
        case EBuildingEra::WorldWars:
            return L"독립 선언";
        case EBuildingEra::ColdWar:
            return L"전후 질서 편입";
        case EBuildingEra::Modern:
            return L"현대화 선언";
        case EBuildingEra::Colonial:
        default:
            return L"시대 전환";
        }
    }

    void AppendEraRequirementLine(
        std::wstring& OutText,
        const wchar_t* Label,
        int Current,
        int Required,
        const wchar_t* Suffix = L"")
    {
        if (!OutText.empty())
            OutText += L"\n";

        OutText += Label;
        OutText += L" ";
        OutText += std::to_wstring(Current);
        OutText += L" / ";
        OutText += std::to_wstring(Required);
        OutText += Suffix;
    }

    void AppendEraRequirementPairLine(
        std::wstring& OutText,
        const wchar_t* LeftLabel,
        int LeftCurrent,
        int LeftRequired,
        const wchar_t* RightLabel,
        int RightCurrent,
        int RightRequired,
        const wchar_t* LeftSuffix = L"",
        const wchar_t* RightSuffix = L"")
    {
        AppendEraRequirementLine(
            OutText,
            LeftLabel,
            LeftCurrent,
            LeftRequired,
            LeftSuffix);
        OutText += L"  |  ";
        OutText += RightLabel;
        OutText += L" ";
        OutText += std::to_wstring(RightCurrent);
        OutText += L" / ";
        OutText += std::to_wstring(RightRequired);
        OutText += RightSuffix;
    }

    std::wstring BuildEraTransitionProgressBody(
        const FEraProgressState& EraProgress)
    {
        if (!EraProgress.HasNextEra)
        {
            return
                L"최종 시대에 도달했습니다.\n"
                L"추가 시대 전환은 없습니다.";
        }

        std::wstring Result;

        switch (EraProgress.NextEra)
        {
        case EBuildingEra::WorldWars:
            Result = L"독립 선언 조건";
            AppendEraRequirementPairLine(
                Result,
                L"인구",
                EraProgress.Population,
                EraProgress.NextRequirement.MinPopulation,
                L"총 건물",
                EraProgress.TotalBuildings,
                EraProgress.NextRequirement.MinTotalBuildings);
            AppendEraRequirementLine(
                Result,
                L"식량 시설",
                EraProgress.FoodProviders,
                EraProgress.NextRequirement.MinFoodProviders);
            break;
        case EBuildingEra::ColdWar:
            Result = L"냉전 진입 조건";
            AppendEraRequirementPairLine(
                Result,
                L"인구",
                EraProgress.Population,
                EraProgress.NextRequirement.MinPopulation,
                L"총 건물",
                EraProgress.TotalBuildings,
                EraProgress.NextRequirement.MinTotalBuildings);
            AppendEraRequirementPairLine(
                Result,
                L"산업 건물",
                EraProgress.IndustryBuildings,
                EraProgress.NextRequirement.MinIndustryBuildings,
                L"전력",
                EraProgress.PowerMW,
                EraProgress.NextRequirement.MinPowerMW,
                L"",
                L"MW");
            break;
        case EBuildingEra::Modern:
            Result = L"현대화 조건";
            AppendEraRequirementPairLine(
                Result,
                L"인구",
                EraProgress.Population,
                EraProgress.NextRequirement.MinPopulation,
                L"총 건물",
                EraProgress.TotalBuildings,
                EraProgress.NextRequirement.MinTotalBuildings);
            AppendEraRequirementPairLine(
                Result,
                L"공공 서비스",
                EraProgress.PublicServiceBuildings,
                EraProgress.NextRequirement.MinPublicServiceBuildings,
                L"오락 건물",
                EraProgress.EntertainmentBuildings,
                EraProgress.NextRequirement.MinEntertainmentBuildings);
            AppendEraRequirementLine(
                Result,
                L"전력",
                EraProgress.PowerMW,
                EraProgress.NextRequirement.MinPowerMW,
                L"MW");
            break;
        case EBuildingEra::Colonial:
        default:
            Result = UIStrings::Get(L"top_hud.era_transition.unavailable");
            break;
        }

        if (EraProgress.NextEraReady)
        {
            Result += L"\n\n모든 조건 충족. 승인하면 즉시 전환됩니다.";
        }
        else
        {
            Result += L"\n\n";
            Result += UIStrings::Get(L"top_hud.era_transition.unavailable");
        }

        return Result;
    }
}

namespace TopHudDataProvider
{
    FTopHudSnapshot BuildSnapshot(const std::shared_ptr<CWorld>& World)
    {
        FTopHudSnapshot Result;
        Result.ElectionTextColor = MakeColor(245, 235, 210, 255);
        Result.EventTextColor = MakeColor(208, 226, 198, 255);

        if (!World)
            return Result;

        auto* Access = ResolveWorldUIAccess(World.get());

        if (!Access)
            return Result;

        const FElectionStatus& ElectionStatus =
            Access->Read().GetElectionStatus();
        const FTaxPolicyEventStatus& TaxEventStatus =
            Access->Read().GetTaxPolicyEventStatus();
        const FWorldCrisisStatus& WorldCrisisStatus =
            Access->Read().GetWorldCrisisStatus();
        const int DaysUntilElection = Access->Read().GetDaysUntilNextElection();
        const double ElectionWarningScore =
            Access->Read().GetElectionWarningScore();
        const bool ElectionScheduled = HasElectionSchedule(ElectionStatus);
        const bool ElectionWarningActive =
            HasElectionWarning(DaysUntilElection, ElectionWarningScore);
        const FPoliticalWorldSnapshot& PoliticalSnapshot =
            Access->Read().GetPoliticalSnapshot();
        const auto& FactionDemandPressureDays =
            Access->Read().GetFactionDemandPressureDays();
        const auto& FactionDemandStates =
            Access->Read().GetFactionDemandStates();
        const FPoliticalDemandNotice& PoliticalDemandNotice =
            Access->Read().GetPoliticalDemandNotice();
        const FEraProgressState& EraProgress =
            Access->Read().GetEraProgress();
        const FEraTransitionState& EraTransitionState =
            Access->Read().GetEraTransitionState();
        const FPoliticalEscalationHudInfo PoliticalEscalationHudInfo =
            BuildPoliticalEscalationHudInfo(
                FactionDemandPressureDays,
                FactionDemandStates);
        const int ActiveNpcCount = PoliticalSnapshot.ActiveCitizenCount;
        const std::wstring PromiseSummary =
            BuildElectionPromiseHudSummary(ElectionStatus);

        Result.GamePaused = Access->Read().IsSimulationPaused();
        Result.GameSpeedMultiplier = (std::max)(
            1, Access->Read().GetSimulationSpeedMultiplier());
        Result.EraTransitionButtonEnabled = true;
        Result.EraTransitionAvailable =
            EraTransitionState.Stage == EEraTransitionStage::Available;
        Result.EraTransitionTitle =
            BuildEraTransitionPopupTitle(EraProgress, EraTransitionState);
        if (!EraTransitionState.Summary.empty())
            Result.EraTransitionBody = EraTransitionState.Summary;
        else
            Result.EraTransitionBody =
                BuildEraTransitionProgressBody(EraProgress);
        if (!EraTransitionState.ConfirmText.empty())
            Result.EraTransitionConfirmText = EraTransitionState.ConfirmText;
        else if (EraProgress.NextEraReady)
            Result.EraTransitionConfirmText = L"전환 승인";
        else
            Result.EraTransitionConfirmText = L"조건 미충족";
        Result.BudgetText = FormatCurrency(Access->Read().GetNationalBudget());
        Result.NpcText = std::to_wstring(ActiveNpcCount);

        int SupportPercent = 0;

        if (ActiveNpcCount > 0)
        {
            SupportPercent = static_cast<int>(round(
                static_cast<double>(PoliticalSnapshot.IncumbentCount) /
                static_cast<double>(ActiveNpcCount) * 100.0));
        }

        SupportPercent = (std::max)(0, (std::min)(100, SupportPercent));
        Result.SupportText = std::to_wstring(SupportPercent) + L"%";
        Result.ResearchText =
            FormatInteger(Access->Read().GetKnowledgePoints());

        if (Access->Read().GetDailyKnowledgeGeneration() > 0)
        {
            Result.ResearchText += L" (+";
            Result.ResearchText += FormatInteger(
                Access->Read().GetDailyKnowledgeGeneration());
            Result.ResearchText += L")";
        }
        Result.DateText = FormatHudDate(
            Access->Read().GetSimulationYear(),
            Access->Read().GetSimulationMonth(),
            Access->Read().GetSimulationDay());
        Result.DateText += L" | ";
        Result.DateText += GetCompactEraLabel(
            Access->Read().GetCurrentEra());

        if (ElectionStatus.GameLost)
        {
            Result.ElectionText = UIStrings::Get(L"top_hud.election.defeat");
            Result.ElectionTextColor = MakeColor(232, 86, 72, 255);
        }
        else if (!ElectionScheduled)
        {
            Result.ElectionText = UIStrings::Get(L"top_hud.placeholder.election");
            Result.ElectionTextColor = MakeColor(186, 173, 144, 255);
        }
        else
        {
            Result.ElectionText =
                UIStrings::Get(L"top_hud.fragment.next_election_prefix") +
                FormatDate(
                    ElectionStatus.NextElectionYear,
                    ElectionStatus.NextElectionMonth,
                    ElectionStatus.NextElectionDay);

            if (DaysUntilElection >= 0)
            {
                Result.ElectionText += UIStrings::Get(L"top_hud.fragment.separator");
                Result.ElectionText += std::to_wstring(DaysUntilElection);
                Result.ElectionText += UIStrings::Get(L"top_hud.fragment.day_suffix");
            }

            if (ElectionWarningActive)
            {
                Result.ElectionText += UIStrings::Get(L"top_hud.fragment.separator");
                Result.ElectionText +=
                    std::wstring(GetElectionWarningTierLabel(ElectionWarningScore));
            }
            else if (ElectionStatus.HasRecordedElection)
            {
                Result.ElectionText += ElectionStatus.IncumbentWonLastElection ?
                    UIStrings::Get(L"top_hud.fragment.separator") +
                        UIStrings::Get(L"top_hud.fragment.last_win") :
                    UIStrings::Get(L"top_hud.fragment.separator") +
                        UIStrings::Get(L"top_hud.fragment.last_loss");
            }

            if (!PromiseSummary.empty())
            {
                Result.ElectionText += UIStrings::Get(L"top_hud.fragment.separator");
                Result.ElectionText += PromiseSummary;
            }

            if (ElectionWarningScore >= 0.78)
                Result.ElectionTextColor = MakeColor(232, 86, 72, 255);
            else if (ElectionWarningScore >= 0.52)
                Result.ElectionTextColor = MakeColor(238, 178, 88, 255);
            else if (ElectionWarningActive)
                Result.ElectionTextColor = MakeColor(240, 214, 124, 255);
            else
                Result.ElectionTextColor = MakeColor(245, 235, 210, 255);
        }

        Result.TaxPolicyText =
            FormatTaxPolicyCompact(Access->Read().GetTaxPolicy());

        Result.EventText = UIStrings::Get(L"top_hud.placeholder.event_stable");
        Result.EventTextColor = MakeColor(208, 226, 198, 255);

        if (WorldCrisisStatus.Active)
        {
            Result.EventText =
                UIStrings::Get(L"top_hud.fragment.warning_prefix") +
                WorldCrisisStatus.Title;

            if (ElectionWarningActive)
            {
                Result.EventText += UIStrings::Get(L"top_hud.fragment.separator");
                Result.EventText += GetElectionWarningTierLabel(ElectionWarningScore);
            }
            else
            {
                Result.EventText += UIStrings::Get(L"top_hud.fragment.separator");
                Result.EventText +=
                    std::to_wstring((std::max)(0, WorldCrisisStatus.RemainingDays));
                Result.EventText += UIStrings::Get(L"top_hud.fragment.day_suffix");
            }

            if (ElectionWarningScore >= 0.78 ||
                WorldCrisisStatus.DaysActive >= 4)
            {
                Result.EventTextColor = MakeColor(238, 108, 90, 255);
            }
            else
            {
                Result.EventTextColor = MakeColor(238, 178, 88, 255);
            }
        }
        else if (TaxEventStatus.Active)
        {
            Result.EventText =
                UIStrings::Get(L"top_hud.fragment.warning_prefix") +
                TaxEventStatus.Title;

            if (ElectionWarningActive)
            {
                Result.EventText += UIStrings::Get(L"top_hud.fragment.separator");
                Result.EventText += GetElectionWarningTierLabel(ElectionWarningScore);
            }
            else
            {
                Result.EventText += UIStrings::Get(L"top_hud.fragment.separator");
                Result.EventText +=
                    std::to_wstring((std::max)(0, TaxEventStatus.RemainingDays));
                Result.EventText += UIStrings::Get(L"top_hud.fragment.day_suffix");
            }

            if (ElectionWarningScore >= 0.78 ||
                TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis ||
                TaxEventStatus.DaysActive >= 4)
            {
                Result.EventTextColor = MakeColor(238, 108, 90, 255);
            }
            else if (ElectionWarningActive)
            {
                Result.EventTextColor = MakeColor(238, 178, 88, 255);
            }
            else
            {
                Result.EventTextColor = MakeColor(236, 182, 94, 255);
            }
        }
        else if (PoliticalDemandNotice.RemainingDays > 0 &&
            !PoliticalDemandNotice.Title.empty())
        {
            if (PoliticalEscalationHudInfo.Active)
            {
                Result.EventText = PoliticalEscalationHudInfo.PrefixText;
                Result.EventText += L" ";
            }

            Result.EventText += PoliticalDemandNotice.Title;

            if (!PoliticalDemandNotice.Summary.empty())
            {
                Result.EventText +=
                    UIStrings::Get(L"top_hud.fragment.separator");
                Result.EventText += PoliticalDemandNotice.Summary;
            }

            if (PoliticalEscalationHudInfo.Active)
            {
                Result.EventTextColor = PoliticalEscalationHudInfo.Color;
            }
            else
            {
                Result.EventTextColor =
                    PoliticalDemandNotice.Positive ?
                        MakeColor(208, 226, 198, 255) :
                        (PoliticalDemandNotice.ActiveDemand ?
                            MakeColor(240, 214, 124, 255) :
                            MakeColor(236, 182, 94, 255));
            }
        }
        else if (PoliticalEscalationHudInfo.Active)
        {
            Result.EventText = PoliticalEscalationHudInfo.PrefixText;
            Result.EventText += L" ";
            Result.EventText += PoliticalEscalationHudInfo.SummaryText;
            Result.EventTextColor = PoliticalEscalationHudInfo.Color;
        }
        else if (EraTransitionState.Stage == EEraTransitionStage::Available &&
            !EraTransitionState.Summary.empty())
        {
            Result.EventText =
                L"페놀티코 과제 | " +
                BuildEraTransitionHudSummary(EraTransitionState);
            Result.EventTextColor = MakeColor(236, 206, 118, 255);
        }
        else if (EraTransitionState.Stage == EEraTransitionStage::Cooldown &&
            EraTransitionState.NotificationDays > 0 &&
            !EraTransitionState.Summary.empty())
        {
            Result.EventText =
                UIStrings::Get(L"top_hud.fragment.era_complete_prefix") +
                BuildEraTransitionHudSummary(EraTransitionState);
            Result.EventTextColor = MakeColor(208, 226, 198, 255);
        }
        else if (ElectionWarningActive)
        {
            Result.EventText = UIStrings::Get(L"top_hud.event.election_warning");

            if (DaysUntilElection >= 0)
            {
                Result.EventText += UIStrings::Get(L"top_hud.fragment.separator");
                Result.EventText += std::to_wstring(DaysUntilElection);
                Result.EventText += UIStrings::Get(L"top_hud.fragment.day_suffix");
            }

            if (ElectionWarningScore >= 0.78)
                Result.EventTextColor = MakeColor(238, 108, 90, 255);
            else if (ElectionWarningScore >= 0.52)
                Result.EventTextColor = MakeColor(238, 178, 88, 255);
            else
                Result.EventTextColor = MakeColor(240, 214, 124, 255);
        }
        else if (!PromiseSummary.empty() && DaysUntilElection >= 0)
        {
            Result.EventText = PromiseSummary;
            Result.EventText += UIStrings::Get(L"top_hud.fragment.separator");
            Result.EventText += std::to_wstring(DaysUntilElection);
            Result.EventText += UIStrings::Get(L"top_hud.fragment.day_suffix");

            if (CountActiveElectionPromises(ElectionStatus) > 0 &&
                CountMetElectionPromises(ElectionStatus) <
                    CountActiveElectionPromises(ElectionStatus))
            {
                Result.EventTextColor = MakeColor(236, 182, 94, 255);
            }
            else
            {
                Result.EventTextColor = MakeColor(208, 226, 198, 255);
            }
        }
        else if (TaxEventStatus.NotificationDays > 0 &&
            !TaxEventStatus.Summary.empty())
        {
            Result.EventText =
                UIStrings::Get(L"top_hud.fragment.recent_warning_prefix") +
                TaxEventStatus.Summary;
            Result.EventTextColor = MakeColor(228, 214, 188, 255);
        }
        else if (WorldCrisisStatus.NotificationDays > 0 &&
            !WorldCrisisStatus.Summary.empty())
        {
            Result.EventText =
                UIStrings::Get(L"top_hud.fragment.recent_warning_prefix") +
                WorldCrisisStatus.Summary;
            Result.EventTextColor = MakeColor(228, 214, 188, 255);
        }
        else if (Access->Read().GetLastDailyExportIncome() > 0)
        {
            Result.EventText =
                L"수출 수입: +" +
                FormatCurrency(Access->Read().GetLastDailyExportIncome());
            Result.EventTextColor = MakeColor(160, 218, 160, 255);
        }

        Result.GameLost = ElectionStatus.GameLost;
        Result.CanUseButtons = !ElectionStatus.GameLost;
        Result.GameOverTitleText = UIStrings::Get(L"top_hud.game_over.title");

        if (ElectionStatus.GameLost)
        {
            Result.GameOverBodyText = UIStrings::Format(
                L"top_hud.game_over.body_template",
                {
                    FormatDate(
                        ElectionStatus.LastElectionYear,
                        ElectionStatus.LastElectionMonth,
                        ElectionStatus.LastElectionDay),
                    std::to_wstring(ElectionStatus.LastIncumbentVotes),
                    std::to_wstring(ElectionStatus.LastOppositionVotes),
                    std::to_wstring(ElectionStatus.LastAbstainVotes),
                    FormatOneDecimal(ElectionStatus.LastVoteShare),
                    FormatOneDecimal(ElectionStatus.LastTurnoutPercent)
                });
        }

        const float MonthProgress = Access->Read().GetSimulationMonthProgress();
        Result.MonthProgress = (std::max)(0.f, (std::min)(1.f, MonthProgress));
        return Result;
    }
}
