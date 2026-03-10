#include "TopHudDataProvider.h"
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

    std::wstring FormatTaxPolicyCompact(const FTaxPolicy& TaxPolicy)
    {
        return
            L"세금 " +
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
            return L"재선 위험 높음";
        if (Score >= 0.52)
            return L"재선 주의";
        if (Score >= 0.32)
            return L"선거 점검";
        return L"안정";
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
        const int DaysUntilElection = MainWorld->GetDaysUntilNextElection();
        const double ElectionWarningScore = MainWorld->GetElectionWarningScore();
        const bool ElectionWarningActive =
            HasElectionWarning(DaysUntilElection, ElectionWarningScore);
        const FPoliticalWorldSnapshot& PoliticalSnapshot =
            MainWorld->GetPoliticalSnapshot();
        const int ActiveNpcCount = PoliticalSnapshot.ActiveCitizenCount;

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

        if (ElectionStatus.GameLost)
        {
            Result.ElectionText = L"선거 패배";
            Result.ElectionTextColor = MakeColor(232, 86, 72, 255);
        }
        else
        {
            Result.ElectionText =
                L"차기 선거 " +
                FormatDate(
                    ElectionStatus.NextElectionYear,
                    ElectionStatus.NextElectionMonth,
                    ElectionStatus.NextElectionDay);

            if (DaysUntilElection >= 0)
            {
                Result.ElectionText += L" | ";
                Result.ElectionText += std::to_wstring(DaysUntilElection);
                Result.ElectionText += L"일";
            }

            if (ElectionWarningActive)
            {
                Result.ElectionText += L" | ";
                Result.ElectionText +=
                    std::wstring(GetElectionWarningTierLabel(ElectionWarningScore));
            }
            else if (ElectionStatus.HasRecordedElection)
            {
                Result.ElectionText += ElectionStatus.IncumbentWonLastElection ?
                    L" | 직전 승리" :
                    L" | 직전 패배";
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

        Result.EventText = L"현재 상태 안정";
        Result.EventTextColor = MakeColor(208, 226, 198, 255);

        if (TaxEventStatus.Active)
        {
            Result.EventText = std::wstring(L"경고 ") + TaxEventStatus.Title;

            if (ElectionWarningActive)
            {
                Result.EventText += L" | ";
                Result.EventText += GetElectionWarningTierLabel(ElectionWarningScore);
            }
            else
            {
                Result.EventText += L" | ";
                Result.EventText +=
                    std::to_wstring((std::max)(0, TaxEventStatus.RemainingDays));
                Result.EventText += L"일";
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
            Result.EventText = L"선거 경고 | 지지 기반 흔들림";

            if (DaysUntilElection >= 0)
            {
                Result.EventText += L" | ";
                Result.EventText += std::to_wstring(DaysUntilElection);
                Result.EventText += L"일";
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
            Result.EventText = std::wstring(L"최근 경고 | ") +
                TaxEventStatus.Summary;
            Result.EventTextColor = MakeColor(228, 214, 188, 255);
        }

        Result.GameLost = ElectionStatus.GameLost;
        Result.CanUseButtons = !ElectionStatus.GameLost;
        Result.GameOverTitleText = L"정권 상실";

        if (ElectionStatus.GameLost)
        {
            wchar_t Buffer[512] = {};
            swprintf_s(
                Buffer,
                L"%04d.%02d.%02d 선거에서 재집권에 실패했습니다.\n"
                L"지지 %d / 야당 %d / 기권 %d\n"
                L"득표율 %.1f%% / 투표율 %.1f%%\n"
                L"시뮬레이션이 정지되었습니다.",
                ElectionStatus.LastElectionYear,
                ElectionStatus.LastElectionMonth,
                ElectionStatus.LastElectionDay,
                ElectionStatus.LastIncumbentVotes,
                ElectionStatus.LastOppositionVotes,
                ElectionStatus.LastAbstainVotes,
                ElectionStatus.LastVoteShare,
                ElectionStatus.LastTurnoutPercent);
            Result.GameOverBodyText = Buffer;
        }

        const float MonthProgress = MainWorld->GetSimulationMonthProgress();
        Result.MonthProgress = (std::max)(0.f, (std::min)(1.f, MonthProgress));
        return Result;
    }
}
