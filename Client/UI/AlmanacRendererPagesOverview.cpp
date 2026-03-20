#include "AlmanacRendererPagesCoreShared.h"

void FAlmanacRenderer::ApplyOverviewPage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    const int SafeCitizenCount = (std::max)(1, Snapshot.ActiveCitizenCount);
    const int HousingVacancy =
        (std::max)(0, Snapshot.ResidentialCapacity - Snapshot.AssignedHomeCount);
    const int JobVacancy =
        (std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount);
    const double HomelessRatio =
        Snapshot.ActiveCitizenCount > 0 ?
            static_cast<double>(Snapshot.HomelessCount) /
                static_cast<double>(SafeCitizenCount) :
            0.0;
    const double UnemploymentRatio =
        Snapshot.ActiveCitizenCount > 0 ?
            static_cast<double>(Snapshot.UnemployedCount) /
                static_cast<double>(SafeCitizenCount) :
            0.0;
    const long long DailyOperatingCost =
        static_cast<long long>(std::llround(
            static_cast<double>(Snapshot.MonthlyWageCost +
                Snapshot.MonthlyUpkeepCost) / 30.0)) +
        Snapshot.DailyEdictCost +
        Snapshot.DailyImportExpense;
    const double TaxCollectionEfficiencyPercent =
        Clamp01(Snapshot.TaxCollectionEfficiency) * 100.0;
    const bool HasAdministrativeData = Snapshot.HasMainWorld;
    const bool HasPoliticalData =
        Snapshot.PoliticalSnapshot.ActiveCitizenCount > 0;
    const bool ElectionWarningActive =
        GameBalanceTuning::Politics::HasElectionWarning(
            Snapshot.DaysUntilNextElection,
            Snapshot.ElectionWarningScore);
    const std::wstring ElectionWarningSummary =
        HasAdministrativeData ?
            AlmanacCalc::BuildElectionWarningSummary(
                Snapshot.ElectionStatus.GameLost,
                Snapshot.DaysUntilNextElection,
                Snapshot.ElectionWarningScore,
                Snapshot.TaxEventStatus) :
            L"행정 데이터 없음";
    const std::wstring TaxPolicySummary =
        HasAdministrativeData ?
            (L"세율 " +
                FormatTaxPolicyCompact(Snapshot.GovernmentProfile.TaxPolicy) +
                L" / 징세 효율 " +
                FormatPercent(TaxCollectionEfficiencyPercent)) :
            L"행정 데이터 없음";

    if (Widget.mOverviewCards.size() >= GOverviewCardCount)
    {
        SetCardData(
            Widget.mOverviewCards[0],
            L"무주택자 시민",
            FormatInteger(Snapshot.HomelessCount),
            L"비율 " +
                FormatPercent(HomelessRatio * 100.0) +
                L" / 여유 " +
                FormatInteger(HousingVacancy),
            false);
        SetCardData(
            Widget.mOverviewCards[1],
            L"실업자 시민",
            FormatInteger(Snapshot.UnemployedCount),
            L"비율 " +
                FormatPercent(UnemploymentRatio * 100.0) +
                L" / 여유 " +
                FormatInteger(JobVacancy),
            false);
        SetCardData(
            Widget.mOverviewCards[2],
            L"종합 만족도",
            FormatFixed1(Snapshot.AverageOverall),
            L"음식 " +
                FormatFixed1(Snapshot.AverageFood) +
                L" / 보건 " +
                FormatFixed1(Snapshot.AverageHealth),
            false);
        SetCardData(
            Widget.mOverviewCards[3],
            L"직업",
            FormatFixed1(Snapshot.AverageJob),
            L"주거 " +
                FormatFixed1(Snapshot.AverageHousing) +
                L" / 자유 " +
                FormatFixed1(Snapshot.AverageFreedom),
            false);
        SetCardData(
            Widget.mOverviewCards[4],
            L"국고",
            HasAdministrativeData ?
                FormatCompactCurrency(Snapshot.NationalBudget) :
                std::wstring(L"-"),
            HasAdministrativeData ?
                (L"일일 수입 " +
                    FormatCompactCurrency(
                        Snapshot.DailyExportIncome + Snapshot.DailyTaxIncome)) :
                std::wstring(L"행정 데이터 없음"),
            false);
        SetCardData(
            Widget.mOverviewCards[5],
            L"일일 순변동",
            HasAdministrativeData ?
                FormatSignedCompactCurrency(Snapshot.DailyNetChange) :
                std::wstring(L"-"),
            HasAdministrativeData ?
                (L"세수 " +
                    FormatCompactCurrency(Snapshot.DailyTaxIncome) +
                    L" / 운영 " +
                    FormatCompactCurrency(DailyOperatingCost)) :
                std::wstring(L"행정 데이터 없음"),
            false);
        SetCardData(
            Widget.mOverviewCards[6],
            L"현 정권 지지",
            HasPoliticalData ?
                FormatPercent(Snapshot.SupportPercent) :
                std::wstring(L"-"),
            HasPoliticalData ?
                (FormatInteger(Snapshot.PoliticalSnapshot.IncumbentCount) +
                    L"명 / 평균 " +
                    FormatFixed1(
                        Snapshot.PoliticalSnapshot.AverageSupportScore)) :
                std::wstring(L"정치 데이터 없음"),
            false);
        SetCardData(
            Widget.mOverviewCards[7],
            L"야권 지지",
            HasPoliticalData ?
                FormatPercent(Snapshot.OppositionPercent) :
                std::wstring(L"-"),
            HasPoliticalData ?
                (FormatInteger(Snapshot.PoliticalSnapshot.OppositionCount) +
                    L"명 / 부동층 " +
                    FormatInteger(Snapshot.PoliticalSnapshot.AbstainCount)) :
                std::wstring(L"정치 데이터 없음"),
            false);
        SetCardData(
            Widget.mOverviewCards[8],
            L"반란군 위험",
            Snapshot.RebelRiskLabel,
            Snapshot.TaxEventStatus.Active ?
                Snapshot.TaxEventStatus.Title :
                (L"지수 " + FormatFixed1(Snapshot.RebelRiskScore)),
            false);
        SetCardData(
            Widget.mOverviewCards[9],
            L"징세 효율",
            HasAdministrativeData ?
                FormatPercent(TaxCollectionEfficiencyPercent) :
                std::wstring(L"-"),
            HasAdministrativeData ?
                (L"세율 " +
                    FormatTaxPolicyCompact(Snapshot.GovernmentProfile.TaxPolicy)) :
                std::wstring(L"행정 데이터 없음"),
            false);
        SetCardData(
            Widget.mOverviewCards[10],
            L"활성 칙령",
            HasAdministrativeData ?
                FormatInteger(Snapshot.ActiveEdictCount) :
                std::wstring(L"-"),
            HasAdministrativeData ?
                BuildOverviewActiveEdictDetail(Snapshot) :
                std::wstring(L"행정 데이터 없음"),
            false);

        for (int Index = 0; Index < GOverviewCardCount; ++Index)
        {
            if (auto Background = Widget.mOverviewCards[Index].Background.lock())
                Background->SetTint(1.f, 1.f, 1.f, 0.96f);
            if (auto Title = Widget.mOverviewCards[Index].Title.lock())
                Title->SetTextColor(100, 82, 46, 255);
            if (auto Detail = Widget.mOverviewCards[Index].Detail.lock())
            {
                Detail->SetTextColor(116, 108, 96, 255);
                Detail->SetFontSize(Index >= 6 ? 14.f : 13.f);
            }
            if (auto Value = Widget.mOverviewCards[Index].Value.lock())
            {
                Value->SetFontSize(Index >= 4 && Index <= 5 ? 20.f : 24.f);
                Value->SetTextColor(63, 59, 51, 255);
            }
        }

        auto SetOverviewValueColor =
            [&Widget](int Index, unsigned char R, unsigned char G, unsigned char B)
        {
            if (Index < 0 ||
                Index >= static_cast<int>(Widget.mOverviewCards.size()))
            {
                return;
            }

            if (auto Value =
                Widget.mOverviewCards[static_cast<size_t>(Index)].Value.lock())
            {
                Value->SetTextColor(R, G, B, 255);
            }
        };

        SetOverviewValueColor(
            0,
            HomelessRatio >= 0.08 ? 210 : (HomelessRatio >= 0.03 ? 184 : 54),
            HomelessRatio >= 0.08 ? 48 : (HomelessRatio >= 0.03 ? 118 : 154),
            HomelessRatio >= 0.08 ? 34 : (HomelessRatio >= 0.03 ? 40 : 54));
        SetOverviewValueColor(
            1,
            UnemploymentRatio >= 0.12 ? 210 : (UnemploymentRatio >= 0.06 ? 184 : 54),
            UnemploymentRatio >= 0.12 ? 48 : (UnemploymentRatio >= 0.06 ? 118 : 154),
            UnemploymentRatio >= 0.12 ? 34 : (UnemploymentRatio >= 0.06 ? 40 : 54));
        SetOverviewValueColor(
            2,
            Snapshot.AverageOverall >= 75.0 ? 54 :
                (Snapshot.AverageOverall >= 50.0 ? 184 : 210),
            Snapshot.AverageOverall >= 75.0 ? 154 :
                (Snapshot.AverageOverall >= 50.0 ? 118 : 48),
            Snapshot.AverageOverall >= 75.0 ? 54 :
                (Snapshot.AverageOverall >= 50.0 ? 40 : 34));
        SetOverviewValueColor(
            3,
            Snapshot.AverageJob >= 75.0 ? 54 :
                (Snapshot.AverageJob >= 50.0 ? 184 : 210),
            Snapshot.AverageJob >= 75.0 ? 154 :
                (Snapshot.AverageJob >= 50.0 ? 118 : 48),
            Snapshot.AverageJob >= 75.0 ? 54 :
                (Snapshot.AverageJob >= 50.0 ? 40 : 34));
        SetOverviewValueColor(
            4,
            HasAdministrativeData ?
                (Snapshot.NationalBudget >= 0 ? 54 : 210) :
                63,
            HasAdministrativeData ?
                (Snapshot.NationalBudget >= 0 ? 154 : 48) :
                59,
            HasAdministrativeData ?
                (Snapshot.NationalBudget >= 0 ? 54 : 34) :
                51);
        SetOverviewValueColor(
            5,
            HasAdministrativeData ?
                (Snapshot.DailyNetChange > 0 ? 54 :
                    (Snapshot.DailyNetChange < 0 ? 210 : 63)) :
                63,
            HasAdministrativeData ?
                (Snapshot.DailyNetChange > 0 ? 154 :
                    (Snapshot.DailyNetChange < 0 ? 48 : 59)) :
                59,
            HasAdministrativeData ?
                (Snapshot.DailyNetChange > 0 ? 54 :
                    (Snapshot.DailyNetChange < 0 ? 34 : 51)) :
                51);
        SetOverviewValueColor(
            6,
            HasPoliticalData ?
                (Snapshot.SupportPercent >= 50.0 ? 54 :
                    (Snapshot.SupportPercent >= 35.0 ? 184 : 210)) :
                63,
            HasPoliticalData ?
                (Snapshot.SupportPercent >= 50.0 ? 154 :
                    (Snapshot.SupportPercent >= 35.0 ? 118 : 48)) :
                59,
            HasPoliticalData ?
                (Snapshot.SupportPercent >= 50.0 ? 54 :
                    (Snapshot.SupportPercent >= 35.0 ? 40 : 34)) :
                51);
        SetOverviewValueColor(
            7,
            HasPoliticalData ?
                (Snapshot.OppositionPercent >= 45.0 ? 210 :
                    (Snapshot.OppositionPercent >= 30.0 ? 184 : 92)) :
                63,
            HasPoliticalData ?
                (Snapshot.OppositionPercent >= 45.0 ? 48 :
                    (Snapshot.OppositionPercent >= 30.0 ? 118 : 78)) :
                59,
            HasPoliticalData ?
                (Snapshot.OppositionPercent >= 45.0 ? 34 :
                    (Snapshot.OppositionPercent >= 30.0 ? 40 : 54)) :
                51);
        SetOverviewValueColor(
            8,
            Snapshot.RebelRiskScore >= 66.0 ? 210 :
                (Snapshot.RebelRiskScore >= 33.0 ? 184 : 54),
            Snapshot.RebelRiskScore >= 66.0 ? 48 :
                (Snapshot.RebelRiskScore >= 33.0 ? 118 : 154),
            Snapshot.RebelRiskScore >= 66.0 ? 34 :
                (Snapshot.RebelRiskScore >= 33.0 ? 40 : 54));
        SetOverviewValueColor(
            9,
            HasAdministrativeData ?
                (TaxCollectionEfficiencyPercent >= 85.0 ? 54 :
                    (TaxCollectionEfficiencyPercent >= 65.0 ? 184 : 210)) :
                63,
            HasAdministrativeData ?
                (TaxCollectionEfficiencyPercent >= 85.0 ? 154 :
                    (TaxCollectionEfficiencyPercent >= 65.0 ? 118 : 48)) :
                59,
            HasAdministrativeData ?
                (TaxCollectionEfficiencyPercent >= 85.0 ? 54 :
                    (TaxCollectionEfficiencyPercent >= 65.0 ? 40 : 34)) :
                51);
        SetOverviewValueColor(
            10,
            HasAdministrativeData ?
                (Snapshot.ActiveEdictCount > 0 ? 184 : 54) :
                63,
            HasAdministrativeData ?
                (Snapshot.ActiveEdictCount > 0 ? 118 : 154) :
                59,
            HasAdministrativeData ?
                (Snapshot.ActiveEdictCount > 0 ? 40 : 54) :
                51);

        if (auto Icon = Widget.mOverviewCards[8].Icon.lock())
        {
            Icon->SetTint(
                Snapshot.RebelRiskScore >= 66.0 ? 0.92f :
                    (Snapshot.RebelRiskScore >= 33.0 ? 0.94f : 0.36f),
                Snapshot.RebelRiskScore >= 66.0 ? 0.34f :
                    (Snapshot.RebelRiskScore >= 33.0 ? 0.62f : 0.68f),
                Snapshot.RebelRiskScore >= 66.0 ? 0.24f :
                    (Snapshot.RebelRiskScore >= 33.0 ? 0.18f : 0.28f),
                1.f);
        }
    }

    if (auto Text = Widget.mOverviewElectionText.lock())
    {
        Text->SetText(BuildOverviewElectionText(Snapshot).c_str());
        if (Snapshot.ElectionStatus.GameLost)
            Text->SetTextColor(232, 86, 72, 255);
        else if (ElectionWarningActive)
        {
            if (GameBalanceTuning::Politics::IsElectionWarningCritical(
                    Snapshot.ElectionWarningScore))
                Text->SetTextColor(232, 86, 72, 255);
            else if (GameBalanceTuning::Politics::IsElectionWarningCaution(
                Snapshot.ElectionWarningScore))
                Text->SetTextColor(238, 178, 88, 255);
            else
                Text->SetTextColor(240, 214, 124, 255);
        }
        else
        {
            Text->SetTextColor(136, 123, 98, 255);
        }
    }

    if (auto SummaryLeft = Widget.mOverviewSummaryLeft.lock())
        SummaryLeft->SetText(ElectionWarningSummary.c_str());

    if (auto SummaryRight = Widget.mOverviewSummaryRight.lock())
        SummaryRight->SetText(TaxPolicySummary.c_str());

}

