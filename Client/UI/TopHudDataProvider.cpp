#include "TopHudDataProvider.h"
#include "UIStrings.h"
#include "../World/MainWorldAccess.h"
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

        auto MainWorld = std::dynamic_pointer_cast<IMainWorldHudAccess>(World);

        if (!MainWorld)
            return Result;

        const FElectionStatus& ElectionStatus =
            MainWorld->GetElectionStatus();
        const FTaxPolicyEventStatus& TaxEventStatus =
            MainWorld->GetTaxPolicyEventStatus();
        const FWorldCrisisStatus& WorldCrisisStatus =
            MainWorld->GetWorldCrisisStatus();
        const int DaysUntilElection = MainWorld->GetDaysUntilNextElection();
        const double ElectionWarningScore = MainWorld->GetElectionWarningScore();
        const bool ElectionWarningActive =
            HasElectionWarning(DaysUntilElection, ElectionWarningScore);
        const FPoliticalWorldSnapshot& PoliticalSnapshot =
            MainWorld->GetPoliticalSnapshot();
        const int ActiveNpcCount = PoliticalSnapshot.ActiveCitizenCount;

        Result.GamePaused = MainWorld->IsSimulationPaused();
        Result.GameSpeedMultiplier = (std::max)(
            1, MainWorld->GetSimulationSpeedMultiplier());
        Result.BudgetText = FormatCurrency(MainWorld->GetNationalBudget());
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
        Result.DateText = FormatHudDate(
            MainWorld->GetSimulationYear(),
            MainWorld->GetSimulationMonth(),
            MainWorld->GetSimulationDay());
        Result.DateText += L" | ";
        Result.DateText += GetCompactEraLabel(
            MainWorld->GetCurrentEra());

        if (ElectionStatus.GameLost)
        {
            Result.ElectionText = UIStrings::Get(L"top_hud.election.defeat");
            Result.ElectionTextColor = MakeColor(232, 86, 72, 255);
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
            FormatTaxPolicyCompact(MainWorld->GetTaxPolicy());

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

        const float MonthProgress = MainWorld->GetSimulationMonthProgress();
        Result.MonthProgress = (std::max)(0.f, (std::min)(1.f, MonthProgress));
        return Result;
    }
}
