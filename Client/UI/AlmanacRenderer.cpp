#include "AlmanacRenderer.h"
#include "AlmanacRendererInternal.h"
#include <algorithm>
#include <array>
#include <cmath>

void FAlmanacRenderer::ApplySnapshot(CAlmanacWidget& Widget, const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    const int ActiveCitizenCount = (std::max)(1, Snapshot.ActiveCitizenCount);
    const int HousingVacancy =
        (std::max)(0, Snapshot.ResidentialCapacity - Snapshot.AssignedHomeCount);
    const int JobVacancy =
        (std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount);
    const double HousingOccupancyRate =
        Snapshot.ResidentialCapacity > 0 ?
        static_cast<double>(Snapshot.AssignedHomeCount) /
        static_cast<double>(Snapshot.ResidentialCapacity) : 0.0;
    const double EmploymentRate =
        Snapshot.JobCapacity > 0 ?
        static_cast<double>(Snapshot.AssignedJobCount) /
        static_cast<double>(Snapshot.JobCapacity) : 0.0;
    const double HomelessRate =
        static_cast<double>(Snapshot.HomelessCount) /
        static_cast<double>(ActiveCitizenCount);
    const double UnemploymentRate =
        static_cast<double>(Snapshot.UnemployedCount) /
        static_cast<double>(ActiveCitizenCount);
    const double MonthlyBuildingCost =
        static_cast<double>(Snapshot.MonthlyWageCost + Snapshot.MonthlyUpkeepCost);
    const double MonthlyPolicyCost =
        (std::max)(0.0, static_cast<double>(Snapshot.DailyEdictCost) * 30.0);
    const double MonthlyTotalCost = MonthlyBuildingCost + MonthlyPolicyCost;
    const double WagePressure =
        MonthlyBuildingCost > 0.0 ?
        static_cast<double>(Snapshot.MonthlyWageCost) / MonthlyBuildingCost : 0.0;
    const double UpkeepPressure =
        MonthlyBuildingCost > 0.0 ?
        static_cast<double>(Snapshot.MonthlyUpkeepCost) / MonthlyBuildingCost : 0.0;
    const double TradeCoverage =
        MonthlyBuildingCost > 0.0 ?
        static_cast<double>(Snapshot.DailyExportIncome) /
        (MonthlyBuildingCost / 30.0) : 0.0;
    const double ConsumptionTaxShare =
        Snapshot.DailyTaxIncome > 0 ?
        static_cast<double>(Snapshot.DailyConsumptionTaxIncome) /
        static_cast<double>(Snapshot.DailyTaxIncome) : 0.0;
    const double IncomeTaxShare =
        Snapshot.DailyTaxIncome > 0 ?
        static_cast<double>(Snapshot.DailyIncomeTaxIncome) /
        static_cast<double>(Snapshot.DailyTaxIncome) : 0.0;
    const double PropertyTaxShare =
        Snapshot.DailyTaxIncome > 0 ?
        static_cast<double>(Snapshot.DailyPropertyTaxIncome) /
        static_cast<double>(Snapshot.DailyTaxIncome) : 0.0;
    const double EdictPressure =
        MonthlyTotalCost > 0.0 ?
        MonthlyPolicyCost / MonthlyTotalCost : 0.0;
    const double BudgetRunwayMonths =
        MonthlyTotalCost > 0.0 ?
        (std::max)(
            0.0,
            static_cast<double>(Snapshot.NationalBudget) / MonthlyTotalCost) :
        0.0;
    const double BudgetReserve =
        MonthlyTotalCost > 0.0 ?
        Clamp01(BudgetRunwayMonths / 6.0) :
        (Snapshot.NationalBudget >= 0 ? 1.0 : 0.0);
    const std::wstring BudgetRunwayText =
        MonthlyTotalCost > 0.0 ?
        FormatFixed1(BudgetRunwayMonths) + L"개월" :
        (Snapshot.NationalBudget >= 0 ? std::wstring(L"운영비 0") :
            std::wstring(L"적자"));
    const std::wstring TaxPolicySummary =
        FormatTaxPolicySummary(Snapshot.GovernmentProfile.TaxPolicy);
    const double ConsumptionTaxDeviation =
        static_cast<double>(GetTaxPolicyDeviationNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            ETaxPolicyType::Consumption));
    const double IncomeTaxDeviation =
        static_cast<double>(GetTaxPolicyDeviationNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            ETaxPolicyType::Income));
    const double PropertyTaxDeviation =
        static_cast<double>(GetTaxPolicyDeviationNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            ETaxPolicyType::Property));
    const double TaxBurden =
        static_cast<double>(GetCitizenTaxBurdenNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            true,
            true));
    const double WorkerTaxBurden =
        static_cast<double>(GetCitizenTaxBurdenNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            true,
            false));
    const double ResidentTaxBurden =
        static_cast<double>(GetCitizenTaxBurdenNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            false,
            true));
    const auto ResolveTaxPressureTint =
        [](double Value)
    {
        if (Value > 0.08)
            return FVector4(0.82f, 0.22f, 0.18f, 0.95f);

        if (Value < -0.08)
            return FVector4(0.18f, 0.66f, 0.34f, 0.95f);

        return FVector4(0.28f, 0.56f, 0.82f, 0.95f);
    };
    const auto ResolveFactionReactionTint =
        [](double Value)
    {
        if (Value > 0.08)
            return FVector4(0.18f, 0.66f, 0.34f, 0.95f);

        if (Value < -0.08)
            return FVector4(0.82f, 0.22f, 0.18f, 0.95f);

        return FVector4(0.28f, 0.56f, 0.82f, 0.95f);
    };
    const auto ResolveTaxStanceText =
        [](double Value)
    {
        if (Value >= 0.55)
            return std::wstring(L"강경 증세");
        if (Value >= 0.22)
            return std::wstring(L"증세");
        if (Value <= -0.55)
            return std::wstring(L"강한 감세");
        if (Value <= -0.22)
            return std::wstring(L"감세");
        return std::wstring(L"중립");
    };
    const auto ResolveTaxPressureFocusText =
        [](double WorkerValue, double ResidentValue)
    {
        const double WorkerMagnitude = std::fabs(WorkerValue);
        const double ResidentMagnitude = std::fabs(ResidentValue);
        const double DominantMagnitude =
            (std::max)(WorkerMagnitude, ResidentMagnitude);

        if (DominantMagnitude < 0.12)
            return std::wstring(L"부담 낮음");

        if (WorkerMagnitude > ResidentMagnitude + 0.08)
        {
            return WorkerValue >= 0.0 ?
                std::wstring(L"근로층 압박") :
                std::wstring(L"근로층 완화");
        }

        if (ResidentMagnitude > WorkerMagnitude + 0.08)
        {
            return ResidentValue >= 0.0 ?
                std::wstring(L"거주층 압박") :
                std::wstring(L"거주층 완화");
        }

        return (WorkerValue + ResidentValue) >= 0.0 ?
            std::wstring(L"전반 압박") :
            std::wstring(L"전반 완화");
    };
    const auto ClampSignedUnit =
        [](double Value)
    {
        return (std::max)(-1.0, (std::min)(1.0, Value));
    };
    const double CapitalistReaction = ClampSignedUnit(
        -(ConsumptionTaxDeviation * 0.28 +
            IncomeTaxDeviation * 0.42 +
            PropertyTaxDeviation * 0.30));
    const double CommunistReaction = ClampSignedUnit(
        -(ConsumptionTaxDeviation * 0.25) +
        IncomeTaxDeviation * 0.35 +
        PropertyTaxDeviation * 0.40);
    const double IntellectualReaction = ClampSignedUnit(
        -(ConsumptionTaxDeviation * 0.48) +
        IncomeTaxDeviation * 0.34 +
        PropertyTaxDeviation * 0.18);
    const double ConservativeReaction = ClampSignedUnit(
        -(ConsumptionTaxDeviation * 0.18 +
            IncomeTaxDeviation * 0.27 +
            PropertyTaxDeviation * 0.55));
    const std::array<std::pair<const wchar_t*, double>, 4>
        TaxFactionReactions =
    {
        std::pair<const wchar_t*, double>(L"자본주의자", CapitalistReaction),
        std::pair<const wchar_t*, double>(L"공산주의자", CommunistReaction),
        std::pair<const wchar_t*, double>(L"지식인", IntellectualReaction),
        std::pair<const wchar_t*, double>(L"보수주의자", ConservativeReaction)
    };
    const auto StrongestPositiveReactionIter = std::max_element(
        TaxFactionReactions.begin(),
        TaxFactionReactions.end(),
        [](const std::pair<const wchar_t*, double>& A,
            const std::pair<const wchar_t*, double>& B)
        {
            return A.second < B.second;
        });
    const auto StrongestNegativeReactionIter = std::min_element(
        TaxFactionReactions.begin(),
        TaxFactionReactions.end(),
        [](const std::pair<const wchar_t*, double>& A,
            const std::pair<const wchar_t*, double>& B)
        {
            return A.second < B.second;
        });
    const std::wstring TaxStanceSummary =
        ResolveTaxStanceText(TaxBurden) +
        L" / " +
        ResolveTaxPressureFocusText(
            WorkerTaxBurden,
            ResidentTaxBurden);
    std::wstring EconomicBlocReaction = L"뚜렷한 파벌 반응 없음";

    if (StrongestPositiveReactionIter->second > 0.08 ||
        StrongestNegativeReactionIter->second < -0.08)
    {
        EconomicBlocReaction = L"호의: ";

        if (StrongestPositiveReactionIter->second > 0.08)
        {
            EconomicBlocReaction += StrongestPositiveReactionIter->first;
            EconomicBlocReaction += L" ";
            EconomicBlocReaction +=
                FormatSignedPercentUnit(
                    StrongestPositiveReactionIter->second);
        }
        else
        {
            EconomicBlocReaction += L"뚜렷한 지지 없음";
        }

        EconomicBlocReaction += L" / 반발: ";

        if (StrongestNegativeReactionIter->second < -0.08)
        {
            EconomicBlocReaction += StrongestNegativeReactionIter->first;
            EconomicBlocReaction += L" ";
            EconomicBlocReaction +=
                FormatSignedPercentUnit(
                    StrongestNegativeReactionIter->second);
        }
        else
        {
            EconomicBlocReaction += L"뚜렷한 반발 없음";
        }
    }
    std::wstring FactionDemandLabel = L"파벌 요구";
    std::wstring FactionDemandSummary = L"현재 세금 사건에 묶인 요구 없음";
    FVector4 FactionDemandTint(0.31f, 0.27f, 0.21f, 1.f);

    if (Snapshot.TaxEventStatus.Active)
    {
        FactionDemandLabel = L"활성 파벌 요구";
        const std::wstring DaySuffix =
            L" (" +
            std::to_wstring((std::max)(1, Snapshot.TaxEventStatus.DaysActive + 1)) +
            L"일차)";

        switch (Snapshot.TaxEventStatus.Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            FactionDemandSummary =
                std::wstring(GetPoliticalFactionVerboseName(
                    EPoliticalAxis::Economy,
                    EPoliticalStance::Left)) +
                L"·" +
                GetPoliticalFactionVerboseName(
                    EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Left) +
                L" / 근로세 경감 요구" +
                DaySuffix;
            FactionDemandTint = FVector4(0.82f, 0.48f, 0.12f, 1.f);
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            FactionDemandSummary =
                std::wstring(GetPoliticalFactionVerboseName(
                    EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right)) +
                L"·" +
                GetPoliticalFactionVerboseName(
                    EPoliticalAxis::Economy,
                    EPoliticalStance::Left) +
                L" / 재산세 유예 요구" +
                DaySuffix;
            FactionDemandTint = FVector4(0.84f, 0.42f, 0.16f, 1.f);
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            FactionDemandSummary =
                std::wstring(GetPoliticalFactionVerboseName(
                    EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right)) +
                L"·" +
                GetPoliticalFactionVerboseName(
                    EPoliticalAxis::Economy,
                    EPoliticalStance::Right) +
                L" / 재정 안정 대책 요구" +
                DaySuffix;
            FactionDemandTint = FVector4(0.82f, 0.24f, 0.18f, 1.f);
            break;
        default:
            break;
        }
    }
    else if (Snapshot.TaxEventStatus.NotificationDays > 0 &&
        !Snapshot.TaxEventStatus.Summary.empty())
    {
        FactionDemandLabel = L"직전 파벌 요구";
        FactionDemandSummary = L"최근 요구 해소 / 경계 유지";
        FactionDemandTint = FVector4(0.42f, 0.52f, 0.72f, 1.f);
    }

    std::wstring TaxEventWorldEffectSummary = L"직접적인 월드 영향 없음";

    if (Snapshot.TaxEventStatus.Active)
    {
        switch (Snapshot.TaxEventStatus.Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            TaxEventWorldEffectSummary =
                L"생산 저하 · 선적 차질 · 근로세 누락";
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            TaxEventWorldEffectSummary =
                L"재산세 누락 · 주거 유지비 상승";
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            TaxEventWorldEffectSummary =
                L"수출 둔화 · 유지비 상승 · 징수 효율 저하";
            break;
        default:
            break;
        }
    }
    else if (Snapshot.TaxEventStatus.NotificationDays > 0 &&
        !Snapshot.TaxEventStatus.Summary.empty())
    {
        TaxEventWorldEffectSummary = L"최근 혼란 진정 · 정상화 진행 중";
    }
    const double DailyOperatingCost =
        MonthlyTotalCost > 0.0 ? MonthlyTotalCost / 30.0 : 0.0;
    const double FiscalStress =
        DailyOperatingCost > 0.0 && Snapshot.DailyNetChange < 0 ?
        Clamp01(
            static_cast<double>(-Snapshot.DailyNetChange) /
            DailyOperatingCost) :
        0.0;
    const double TourismShare =
        Snapshot.TotalBuildingCount > 0 ?
        static_cast<double>(Snapshot.TourismBuildingCount) /
        static_cast<double>(Snapshot.TotalBuildingCount) : 0.0;
    const double HarborShare =
        Snapshot.TotalBuildingCount > 0 ?
        static_cast<double>(Snapshot.HarborCount) /
        static_cast<double>(Snapshot.TotalBuildingCount) : 0.0;
    const double EmergencyPressure =
        Clamp01(
            Clamp01(Snapshot.RebelRiskScore / 100.0) * 0.80 +
            (Snapshot.MartialLawActive ? 0.20 : 0.0));
    const double ControlStrength =
        Clamp01(
            Clamp01(Snapshot.AverageSecurity / 100.0) * 0.55 +
            Clamp01(Snapshot.SupportPercent / 100.0) * 0.25 +
            (1.0 - FiscalStress) * 0.20);
    const double Stability =
        Clamp01(1.0 - Snapshot.RebelRiskScore / 100.0);

    const std::array<std::pair<const wchar_t*, double>, GSatisfactionRowCount - 1>
        NeedScores =
    {
        std::pair<const wchar_t*, double>(GSatisfactionLabels[1], Snapshot.AverageFood),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[2], Snapshot.AverageHealth),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[3], Snapshot.AverageFun),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[4], Snapshot.AverageFaith),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[5], Snapshot.AverageHousing),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[6], Snapshot.AverageJob),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[7], Snapshot.AverageFreedom),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[8], Snapshot.AverageSecurity)
    };

    const auto WorstNeedIter = std::min_element(
        NeedScores.begin(),
        NeedScores.end(),
        [](const std::pair<const wchar_t*, double>& A,
            const std::pair<const wchar_t*, double>& B)
        {
            return A.second < B.second;
        });
    const auto BestNeedIter = std::max_element(
        NeedScores.begin(),
        NeedScores.end(),
        [](const std::pair<const wchar_t*, double>& A,
            const std::pair<const wchar_t*, double>& B)
        {
            return A.second < B.second;
        });

    const auto NormalizePoliticalScore =
        [](double Value)
    {
        return static_cast<float>(Clamp01((Value + 25.0) / 50.0));
    };
    const auto BuildAxisBreakdown =
        [&Snapshot](EPoliticalAxis Axis)
    {
        const int AxisIndex = static_cast<int>(Axis);
        return std::to_wstring(
            Snapshot.PoliticalCount[AxisIndex]
                                   [static_cast<int>(EPoliticalStance::Left)]) +
            L" / " +
            std::to_wstring(
                Snapshot.PoliticalCount[AxisIndex]
                                       [static_cast<int>(EPoliticalStance::Neutral)]) +
            L" / " +
            std::to_wstring(
                Snapshot.PoliticalCount[AxisIndex]
                                       [static_cast<int>(EPoliticalStance::Right)]);
    };
    const std::wstring NextElectionLabel =
        Snapshot.ElectionStatus.GameLost ?
        std::wstring(L"정권 상실") :
        (Snapshot.ElectionStatus.NextElectionYear > 0 ?
            FormatDate(
                Snapshot.ElectionStatus.NextElectionYear,
                Snapshot.ElectionStatus.NextElectionMonth,
                Snapshot.ElectionStatus.NextElectionDay) :
            std::wstring(L"-"));
    const std::wstring LastElectionLabel =
        Snapshot.ElectionStatus.HasRecordedElection ?
        (FormatDate(
            Snapshot.ElectionStatus.LastElectionYear,
            Snapshot.ElectionStatus.LastElectionMonth,
            Snapshot.ElectionStatus.LastElectionDay) +
            L" " +
            (Snapshot.ElectionStatus.IncumbentWonLastElection ?
                L"재집권" :
                L"정권교체") +
            L" (" +
            FormatFixed1(Snapshot.ElectionStatus.LastVoteShare) +
            L"% / 투표율 " +
            FormatFixed1(Snapshot.ElectionStatus.LastTurnoutPercent) +
            L"%)") :
        std::wstring(L"선거 기록 없음");
    const std::wstring LastElectionCompactLabel =
        Snapshot.ElectionStatus.HasRecordedElection ?
        ((Snapshot.ElectionStatus.IncumbentWonLastElection ?
            std::wstring(L"재집권 ") :
            std::wstring(L"정권교체 ")) +
            FormatFixed1(Snapshot.ElectionStatus.LastVoteShare) +
            L"%") :
        std::wstring(L"선거 기록 없음");
    const std::wstring ElectionWarningSummary =
        BuildElectionWarningSummary(
            Snapshot.ElectionStatus.GameLost,
            Snapshot.DaysUntilNextElection,
            Snapshot.ElectionWarningScore,
            Snapshot.TaxEventStatus);
    const bool ElectionWarningActive =
        Snapshot.DaysUntilNextElection >= 0 &&
        Snapshot.DaysUntilNextElection <= 180 &&
        Snapshot.ElectionWarningScore >= 0.32;
    const FVector4 ElectionWarningTint =
        ResolveElectionWarningTint(Snapshot.ElectionWarningScore);

    if (Widget.mOverviewCards.size() >= GOverviewCardCount)
    {
        SetCardData(
            Widget.mOverviewCards[0],
            L"인구",
            std::to_wstring(Snapshot.ActiveCitizenCount),
            L"총 활성 시민 수",
            true);
        SetCardData(
            Widget.mOverviewCards[1],
            L"무주택 시민",
            std::to_wstring(Snapshot.HomelessCount),
            L"거주 배정 " + FormatPercent(HousingOccupancyRate * 100.0));
        SetCardData(
            Widget.mOverviewCards[2],
            L"실업 시민",
            std::to_wstring(Snapshot.UnemployedCount),
            L"고용률 " + FormatPercent(EmploymentRate * 100.0));
        SetCardData(
            Widget.mOverviewCards[3],
            L"종합 만족도",
            FormatFixed1(Snapshot.AverageOverall),
            L"자유 " + FormatFixed1(Snapshot.AverageFreedom) +
                L" / 치안 " + FormatFixed1(Snapshot.AverageSecurity));
        SetCardData(
            Widget.mOverviewCards[4],
            L"지지율",
            FormatPercent(Snapshot.SupportPercent),
            L"야권 " + FormatPercent(Snapshot.OppositionPercent));
        SetCardData(
            Widget.mOverviewCards[5],
            L"국고",
            FormatCurrency(Snapshot.NationalBudget),
            L"일일 순증감 " + FormatCurrency(Snapshot.DailyNetChange) +
                L" / 세수 " + FormatCurrency(Snapshot.DailyTaxIncome) +
                L"\n" + TaxPolicySummary);
    }

    if (auto SummaryLeft = Widget.mOverviewSummaryLeft.lock())
    {
        std::wstring Summary =
            L"활성 칙령: " + std::to_wstring(Snapshot.ActiveEdictCount) +
            L"개  |  차기 선거: " + NextElectionLabel;

        if (ElectionWarningActive)
            Summary += L"  |  선거 경고: " + ElectionWarningSummary;

        SummaryLeft->SetText(Summary.c_str());
    }

    if (auto SummaryRight = Widget.mOverviewSummaryRight.lock())
    {
        const std::wstring Summary =
            L"반란 위험: " + Snapshot.RebelRiskLabel +
            L"  |  직전 선거: " + LastElectionCompactLabel;
        SummaryRight->SetText(Summary.c_str());
    }

    const double SatisfactionValues[GSatisfactionRowCount] =
    {
        Snapshot.AverageOverall,
        Snapshot.AverageFood,
        Snapshot.AverageHealth,
        Snapshot.AverageFun,
        Snapshot.AverageFaith,
        Snapshot.AverageHousing,
        Snapshot.AverageJob,
        Snapshot.AverageFreedom,
        Snapshot.AverageSecurity
    };

    for (int Index = 0; Index < GSatisfactionRowCount; ++Index)
    {
        SetMetricRowData(
            Widget.mSatisfactionRows[Index],
            GSatisfactionLabels[Index],
            FormatFixed1(SatisfactionValues[Index]),
            static_cast<float>(Clamp01(SatisfactionValues[Index] / 100.0)),
            Index == 0 ?
                FVector4(0.96f, 0.82f, 0.12f, 0.98f) :
                FVector4(0.90f, 0.72f, 0.18f, 0.95f),
            Index == 0);
    }

    SetDetailRowData(
        Widget.mSatisfactionDetails[0],
        L"최저 만족 항목",
        std::wstring(WorstNeedIter->first) +
            L" " + FormatFixed1(WorstNeedIter->second),
        true,
        WorstNeedIter->second < 50.0 ?
            FVector4(0.78f, 0.18f, 0.18f, 1.f) :
            FVector4(0.84f, 0.48f, 0.12f, 1.f));
    SetDetailRowData(
        Widget.mSatisfactionDetails[1],
        L"최고 만족 항목",
        std::wstring(BestNeedIter->first) +
            L" " + FormatFixed1(BestNeedIter->second),
        false,
        FVector4(0.20f, 0.56f, 0.20f, 1.f));
    SetDetailRowData(
        Widget.mSatisfactionDetails[2],
        L"무주택 시민",
        FormatCountWithPercent(Snapshot.HomelessCount, HomelessRate));
    SetDetailRowData(
        Widget.mSatisfactionDetails[3],
        L"실업 시민",
        FormatCountWithPercent(Snapshot.UnemployedCount, UnemploymentRate));
    SetDetailRowData(
        Widget.mSatisfactionDetails[4],
        L"주거 점유율",
        FormatPercent(HousingOccupancyRate * 100.0));
    SetDetailRowData(
        Widget.mSatisfactionDetails[5],
        L"고용률",
        FormatPercent(EmploymentRate * 100.0));

    SetDetailRowData(
        Widget.mPopulationDetails[0],
        L"총 시민",
        std::to_wstring(Snapshot.ActiveCitizenCount),
        true);
    SetDetailRowData(
        Widget.mPopulationDetails[1],
        L"주거 배정",
        std::to_wstring(Snapshot.AssignedHomeCount));
    SetDetailRowData(
        Widget.mPopulationDetails[2],
        L"주거 수용력",
        std::to_wstring(Snapshot.ResidentialCapacity));
    SetDetailRowData(
        Widget.mPopulationDetails[3],
        L"빈 집",
        std::to_wstring(HousingVacancy));
    SetDetailRowData(
        Widget.mPopulationDetails[4],
        L"직업 배정",
        std::to_wstring(Snapshot.AssignedJobCount));
    SetDetailRowData(
        Widget.mPopulationDetails[5],
        L"일자리 수용력",
        std::to_wstring(Snapshot.JobCapacity));
    SetDetailRowData(
        Widget.mPopulationDetails[6],
        L"빈 일자리",
        std::to_wstring(JobVacancy));
    SetDetailRowData(
        Widget.mPopulationDetails[7],
        L"총 건물 수",
        std::to_wstring(Snapshot.TotalBuildingCount));

    SetMetricRowData(
        Widget.mPopulationMetrics[0],
        L"주거 점유율",
        FormatPercent(HousingOccupancyRate * 100.0),
        static_cast<float>(HousingOccupancyRate),
        FVector4(0.22f, 0.58f, 0.90f, 0.95f),
        true);
    SetMetricRowData(
        Widget.mPopulationMetrics[1],
        L"고용률",
        FormatPercent(EmploymentRate * 100.0),
        static_cast<float>(EmploymentRate),
        FVector4(0.28f, 0.74f, 0.36f, 0.95f));
    SetMetricRowData(
        Widget.mPopulationMetrics[2],
        L"무주택률",
        FormatPercent(HomelessRate * 100.0),
        static_cast<float>(HomelessRate),
        FVector4(0.88f, 0.58f, 0.18f, 0.95f));
    SetMetricRowData(
        Widget.mPopulationMetrics[3],
        L"실업률",
        FormatPercent(UnemploymentRate * 100.0),
        static_cast<float>(UnemploymentRate),
        FVector4(0.80f, 0.22f, 0.18f, 0.95f));

    SetDetailRowData(
        Widget.mEconomyDetails[0],
        L"국고",
        FormatCurrency(Snapshot.NationalBudget),
        true);
    SetDetailRowData(
        Widget.mEconomyDetails[1],
        L"월 임금 총액",
        FormatCurrency(Snapshot.MonthlyWageCost));
    SetDetailRowData(
        Widget.mEconomyDetails[2],
        L"월 유지비 총액",
        FormatCurrency(Snapshot.MonthlyUpkeepCost));
    SetDetailRowData(
        Widget.mEconomyDetails[3],
        L"일일 수출 수익",
        FormatCurrency(Snapshot.DailyExportIncome));
    SetDetailRowData(
        Widget.mEconomyDetails[4],
        L"일일 칙령 비용",
        FormatCurrency(Snapshot.DailyEdictCost));
    SetDetailRowData(
        Widget.mEconomyDetails[5],
        L"일일 순증감",
        FormatCurrency(Snapshot.DailyNetChange));
    SetDetailRowData(
        Widget.mEconomyDetails[6],
        L"일일 세수",
        FormatCurrency(Snapshot.DailyTaxIncome));
    SetDetailRowData(
        Widget.mEconomyDetails[7],
        L"세율 정책",
        TaxPolicySummary);
    SetDetailRowData(
        Widget.mEconomyDetails[8],
        L"예산 런웨이",
        BudgetRunwayText);

    SetMetricRowData(
        Widget.mEconomyMetrics[0],
        L"임금 부담",
        FormatPercent(WagePressure * 100.0),
        static_cast<float>(WagePressure),
        FVector4(0.84f, 0.54f, 0.14f, 0.95f),
        true);
    SetMetricRowData(
        Widget.mEconomyMetrics[1],
        L"유지비 부담",
        FormatPercent(UpkeepPressure * 100.0),
        static_cast<float>(UpkeepPressure),
        FVector4(0.72f, 0.34f, 0.18f, 0.95f));
    SetMetricRowData(
        Widget.mEconomyMetrics[2],
        L"수출 커버리지",
        FormatPercent(Clamp01(TradeCoverage) * 100.0),
        static_cast<float>(Clamp01(TradeCoverage)),
        FVector4(0.22f, 0.58f, 0.90f, 0.95f));
    SetMetricRowData(
        Widget.mEconomyMetrics[3],
        L"칙령 부담",
        FormatPercent(Clamp01(EdictPressure) * 100.0),
        static_cast<float>(Clamp01(EdictPressure)),
        FVector4(0.84f, 0.22f, 0.18f, 0.95f));
    SetMetricRowData(
        Widget.mEconomyMetrics[4],
        L"예산 여유",
        FormatPercent(BudgetReserve * 100.0),
        static_cast<float>(BudgetReserve),
        FVector4(0.18f, 0.70f, 0.30f, 0.95f));
    SetMetricRowData(
        Widget.mEconomyMetrics[5],
        L"징수 효율",
        FormatPercent(Snapshot.TaxCollectionEfficiency * 100.0),
        static_cast<float>(Clamp01(Snapshot.TaxCollectionEfficiency)),
        FVector4(0.28f, 0.62f, 0.82f, 0.95f));
    SetMetricRowData(
        Widget.mEconomyMetrics[6],
        L"소비세 비중",
        FormatPercent(ConsumptionTaxShare * 100.0),
        static_cast<float>(Clamp01(ConsumptionTaxShare)),
        FVector4(0.78f, 0.60f, 0.16f, 0.95f));
    SetMetricRowData(
        Widget.mEconomyMetrics[7],
        L"소득세 비중",
        FormatPercent(IncomeTaxShare * 100.0),
        static_cast<float>(Clamp01(IncomeTaxShare)),
        FVector4(0.32f, 0.72f, 0.34f, 0.95f));
    SetMetricRowData(
        Widget.mEconomyMetrics[8],
        L"재산세 비중",
        FormatPercent(PropertyTaxShare * 100.0),
        static_cast<float>(Clamp01(PropertyTaxShare)),
        FVector4(0.74f, 0.38f, 0.20f, 0.95f));

    const int ResourceRowCount =
        (std::min)(GResourceRowCount,
            static_cast<int>(Snapshot.TopResourceBuildings.size()));
    int MaxResourceStock = 1;

    for (int Index = 0; Index < ResourceRowCount; ++Index)
    {
        MaxResourceStock = (std::max)(
            MaxResourceStock,
            Snapshot.TopResourceBuildings[Index].second);
    }

    for (int Index = 0; Index < GResourceRowCount; ++Index)
    {
        if (Index < ResourceRowCount)
        {
            const auto& Entry = Snapshot.TopResourceBuildings[Index];
            const float Percent =
                static_cast<float>(Entry.second) /
                static_cast<float>(MaxResourceStock);

            SetMetricRowData(
                Widget.mResourceRows[Index],
                Entry.first,
                std::to_wstring(Entry.second),
                Percent,
                FVector4(0.26f, 0.64f, 0.32f, 0.95f),
                Index == 0);
        }
        else
        {
            SetMetricRowData(
                Widget.mResourceRows[Index],
                L"-",
                L"-",
                0.f,
                FVector4(0.78f, 0.78f, 0.78f, 0.95f));
        }
    }

    const int ProductionBuildingCount =
        Snapshot.BuildingCategoryCount[static_cast<int>(EBuildingCategory::FoodResource)] +
        Snapshot.BuildingCategoryCount[static_cast<int>(EBuildingCategory::Industry)];
    const std::wstring TopResourceName =
        !Snapshot.TopResourceBuildings.empty() ?
        Snapshot.TopResourceBuildings.front().first :
        std::wstring(L"-");
    const std::wstring TopResourceValue =
        !Snapshot.TopResourceBuildings.empty() ?
        std::to_wstring(Snapshot.TopResourceBuildings.front().second) :
        std::wstring(L"-");

    SetDetailRowData(
        Widget.mResourceDetails[0],
        L"총 저장 재고",
        std::to_wstring(Snapshot.TotalResourceStock),
        true);
    SetDetailRowData(
        Widget.mResourceDetails[1],
        L"생산 건물 수",
        std::to_wstring(ProductionBuildingCount));
    SetDetailRowData(
        Widget.mResourceDetails[2],
        L"식량 공급 건물",
        std::to_wstring(Snapshot.FoodProviderCount));
    SetDetailRowData(
        Widget.mResourceDetails[3],
        L"유흥 건물",
        std::to_wstring(Snapshot.EntertainmentBuildingCount));
    SetDetailRowData(
        Widget.mResourceDetails[4],
        L"항구/물류 거점",
        std::to_wstring(Snapshot.HarborCount));
    SetDetailRowData(
        Widget.mResourceDetails[5],
        TopResourceName,
        TopResourceValue);

    if (auto Notice = Widget.mResourceNotice.lock())
    {
        Notice->SetText(
            L"현재 자원 시스템은 건물별 단일 재고값만 집계합니다.\n"
            L"품목 구분과 생산/소비 추이는 아직 미연동입니다.");
    }

    struct FPoliticsMetric
    {
        std::wstring Label;
        std::wstring Value;
        float Percent = 0.f;
        FVector4 Tint = FVector4(0.90f, 0.72f, 0.18f, 0.95f);
        bool Highlight = false;
    };

    const std::array<FPoliticsMetric, GPoliticsRowCount> PoliticsMetrics =
    {
        FPoliticsMetric
        {
            L"지지율",
            FormatPercent(Snapshot.SupportPercent),
            static_cast<float>(Clamp01(Snapshot.SupportPercent / 100.0)),
            FVector4(0.18f, 0.62f, 0.40f, 0.95f),
            true
        },
        FPoliticsMetric
        {
            L"반대율",
            FormatPercent(Snapshot.OppositionPercent),
            static_cast<float>(Clamp01(Snapshot.OppositionPercent / 100.0)),
            FVector4(0.82f, 0.22f, 0.18f, 0.95f)
        },
        FPoliticsMetric
        {
            L"부동층",
            FormatPercent(Snapshot.AbstainPercent),
            static_cast<float>(Clamp01(Snapshot.AbstainPercent / 100.0)),
            FVector4(0.72f, 0.58f, 0.18f, 0.95f)
        },
        FPoliticsMetric
        {
            L"평균 생활 점수",
            FormatSignedFixed1(Snapshot.PoliticalSnapshot.AverageLifeScore),
            NormalizePoliticalScore(Snapshot.PoliticalSnapshot.AverageLifeScore),
            FVector4(0.18f, 0.62f, 0.44f, 0.95f)
        },
        FPoliticsMetric
        {
            L"정부 이념 일치",
            FormatSignedFixed1(
                Snapshot.PoliticalSnapshot.AverageGovernmentIdeologyScore),
            NormalizePoliticalScore(
                Snapshot.PoliticalSnapshot.AverageGovernmentIdeologyScore),
            FVector4(0.76f, 0.48f, 0.18f, 0.95f)
        },
        FPoliticsMetric
        {
            L"정책 행동 효과",
            FormatSignedFixed1(Snapshot.PoliticalSnapshot.AverageActionScore),
            NormalizePoliticalScore(Snapshot.PoliticalSnapshot.AverageActionScore),
            FVector4(0.46f, 0.36f, 0.18f, 0.95f)
        },
        FPoliticsMetric
        {
            L"세금 부담",
            FormatSignedPercentUnit(TaxBurden),
            static_cast<float>(Clamp01(std::fabs(TaxBurden))),
            ResolveTaxPressureTint(TaxBurden)
        },
        FPoliticsMetric
        {
            L"근로층 압박",
            FormatSignedPercentUnit(WorkerTaxBurden),
            static_cast<float>(Clamp01(std::fabs(WorkerTaxBurden))),
            ResolveTaxPressureTint(WorkerTaxBurden)
        },
        FPoliticsMetric
        {
            L"거주층 압박",
            FormatSignedPercentUnit(ResidentTaxBurden),
            static_cast<float>(Clamp01(std::fabs(ResidentTaxBurden))),
            ResolveTaxPressureTint(ResidentTaxBurden)
        },
        FPoliticsMetric
        {
            L"자본주의자 반응",
            FormatSignedPercentUnit(CapitalistReaction),
            static_cast<float>(Clamp01(std::fabs(CapitalistReaction))),
            ResolveFactionReactionTint(CapitalistReaction)
        },
        FPoliticsMetric
        {
            L"공산주의자 반응",
            FormatSignedPercentUnit(CommunistReaction),
            static_cast<float>(Clamp01(std::fabs(CommunistReaction))),
            ResolveFactionReactionTint(CommunistReaction)
        },
        FPoliticsMetric
        {
            L"지식인 반응",
            FormatSignedPercentUnit(IntellectualReaction),
            static_cast<float>(Clamp01(std::fabs(IntellectualReaction))),
            ResolveFactionReactionTint(IntellectualReaction)
        },
        FPoliticsMetric
        {
            L"보수주의자 반응",
            FormatSignedPercentUnit(ConservativeReaction),
            static_cast<float>(Clamp01(std::fabs(ConservativeReaction))),
            ResolveFactionReactionTint(ConservativeReaction)
        }
    };

    for (int Index = 0; Index < GPoliticsRowCount; ++Index)
    {
        SetMetricRowData(
            Widget.mPoliticsRows[Index],
            PoliticsMetrics[Index].Label,
            PoliticsMetrics[Index].Value,
            PoliticsMetrics[Index].Percent,
            PoliticsMetrics[Index].Tint,
            PoliticsMetrics[Index].Highlight);
    }

    SetDetailRowData(
        Widget.mPoliticsDetails[0],
        L"유권자",
        std::to_wstring(Snapshot.PoliticalSnapshot.ActiveCitizenCount),
        true);
    SetDetailRowData(
        Widget.mPoliticsDetails[1],
        std::wstring(GetPoliticalAxisDisplayName(EPoliticalAxis::Economy)) +
            L"축 " +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::Economy, EPoliticalStance::Left) +
            L"/중립/" +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::Economy, EPoliticalStance::Right),
        BuildAxisBreakdown(EPoliticalAxis::Economy));
    SetDetailRowData(
        Widget.mPoliticsDetails[2],
        std::wstring(
            GetPoliticalAxisDisplayName(EPoliticalAxis::ReligionMilitarism)) +
            L"축 " +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::ReligionMilitarism, EPoliticalStance::Left) +
            L"/중립/" +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::ReligionMilitarism, EPoliticalStance::Right),
        BuildAxisBreakdown(EPoliticalAxis::ReligionMilitarism));
    SetDetailRowData(
        Widget.mPoliticsDetails[3],
        std::wstring(
            GetPoliticalAxisDisplayName(EPoliticalAxis::EnvironmentIndustry)) +
            L"축 " +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::EnvironmentIndustry, EPoliticalStance::Left) +
            L"/중립/" +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::EnvironmentIndustry, EPoliticalStance::Right),
        BuildAxisBreakdown(EPoliticalAxis::EnvironmentIndustry));
    SetDetailRowData(
        Widget.mPoliticsDetails[4],
        std::wstring(
            GetPoliticalAxisDisplayName(
                EPoliticalAxis::IntellectualConservative)) +
            L"축 " +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Left) +
            L"/중립/" +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Right),
        BuildAxisBreakdown(EPoliticalAxis::IntellectualConservative));
    SetDetailRowData(
        Widget.mPoliticsDetails[5],
        L"중립 축 보유 시민",
        FormatCountWithPercent(
            Snapshot.AnyNeutralAxisCitizenCount,
            static_cast<double>(Snapshot.AnyNeutralAxisCitizenCount) /
                static_cast<double>(ActiveCitizenCount)));
    SetDetailRowData(
        Widget.mPoliticsDetails[6],
        L"완전 중립 시민",
        FormatCountWithPercent(
            Snapshot.FullyNeutralCitizenCount,
            static_cast<double>(Snapshot.FullyNeutralCitizenCount) /
                static_cast<double>(ActiveCitizenCount)));
    SetDetailRowData(
        Widget.mPoliticsDetails[7],
        L"현재 세율",
        FormatTaxPolicyCompact(Snapshot.GovernmentProfile.TaxPolicy));
    SetDetailRowData(
        Widget.mPoliticsDetails[8],
        L"세금 기조",
        TaxStanceSummary,
        false,
        ResolveTaxPressureTint(TaxBurden));
    SetDetailRowData(
        Widget.mPoliticsDetails[9],
        L"파벌별 반응 요약",
        EconomicBlocReaction,
        false,
        ResolveFactionReactionTint(
            std::fabs(StrongestPositiveReactionIter->second) >=
            std::fabs(StrongestNegativeReactionIter->second) ?
            StrongestPositiveReactionIter->second :
            StrongestNegativeReactionIter->second));
    SetDetailRowData(
        Widget.mPoliticsDetails[10],
        FactionDemandLabel,
        FactionDemandSummary,
        Snapshot.TaxEventStatus.Active,
        FactionDemandTint);
    SetDetailRowData(
        Widget.mPoliticsDetails[11],
        L"선거 경고",
        ElectionWarningSummary,
        ElectionWarningActive,
        ElectionWarningTint);
    SetDetailRowData(
        Widget.mPoliticsDetails[12],
        Snapshot.ElectionStatus.HasRecordedElection ?
            L"직전 선거" :
            L"차기 선거",
        Snapshot.ElectionStatus.HasRecordedElection ?
            LastElectionLabel :
            NextElectionLabel);

    SetDetailRowData(
        Widget.mForeignDetails[0],
        L"대외 시스템",
        L"미연동",
        true);
    SetDetailRowData(
        Widget.mForeignDetails[1],
        L"일일 수출 수익",
        FormatCurrency(Snapshot.DailyExportIncome));
    SetDetailRowData(
        Widget.mForeignDetails[2],
        L"관광 건물",
        std::to_wstring(Snapshot.TourismBuildingCount));
    SetDetailRowData(
        Widget.mForeignDetails[3],
        L"항구 수",
        std::to_wstring(Snapshot.HarborCount));
    SetDetailRowData(
        Widget.mForeignDetails[4],
        L"지지율",
        FormatPercent(Snapshot.SupportPercent));
    SetDetailRowData(
        Widget.mForeignDetails[5],
        L"계엄령",
        Snapshot.MartialLawActive ? L"활성" : L"비활성");

    SetMetricRowData(
        Widget.mForeignMetrics[0],
        L"수출 커버리지",
        FormatPercent(Clamp01(TradeCoverage) * 100.0),
        static_cast<float>(Clamp01(TradeCoverage)),
        FVector4(0.22f, 0.54f, 0.88f, 0.95f),
        true);
    SetMetricRowData(
        Widget.mForeignMetrics[1],
        L"관광 기반 비중",
        FormatPercent(TourismShare * 100.0),
        static_cast<float>(TourismShare),
        FVector4(0.18f, 0.66f, 0.40f, 0.95f));
    SetMetricRowData(
        Widget.mForeignMetrics[2],
        L"항만 네트워크 비중",
        FormatPercent(HarborShare * 100.0),
        static_cast<float>(HarborShare),
        FVector4(0.78f, 0.68f, 0.18f, 0.95f));
    SetMetricRowData(
        Widget.mForeignMetrics[3],
        L"비상 통치 압박",
        FormatPercent(EmergencyPressure * 100.0),
        static_cast<float>(EmergencyPressure),
        FVector4(0.82f, 0.24f, 0.18f, 0.95f));

    if (auto Notice = Widget.mForeignNotice.lock())
    {
        Notice->SetText(
            L"국가별 관계, 원조, 무역 계약은 아직 연결되지 않았습니다.\n"
            L"현재는 대외 활동에 영향을 주는 교역·관광·항만 기반만 표시합니다.");
    }

    int HighestCategoryCount = 0;

    for (int Index = 0; Index < GBuildingRowCount; ++Index)
    {
        HighestCategoryCount = (std::max)(
            HighestCategoryCount,
            Snapshot.BuildingCategoryCount[Index]);
    }

    HighestCategoryCount = (std::max)(1, HighestCategoryCount);
    const int SafeBuildingCount = (std::max)(1, Snapshot.TotalBuildingCount);

    for (int Index = 0; Index < GBuildingRowCount; ++Index)
    {
        const int Count = Snapshot.BuildingCategoryCount[Index];
        const float Percent =
            static_cast<float>(Count) /
            static_cast<float>(SafeBuildingCount);

        SetMetricRowData(
            Widget.mBuildingRows[Index],
            BuildingCategoryInfo::GetDisplayName(Index),
            std::to_wstring(Count),
            Percent,
            FVector4(0.84f, 0.66f, 0.18f, 0.95f),
            Count == HighestCategoryCount && HighestCategoryCount > 0);
    }

    for (int Index = 0; Index < GBuildingDetailCount; ++Index)
    {
        if (Index < static_cast<int>(Snapshot.TopBuildings.size()))
        {
            const auto& Entry = Snapshot.TopBuildings[Index];

            SetDetailRowData(
                Widget.mBuildingDetails[Index],
                Entry.first,
                std::to_wstring(Entry.second),
                Index == 0);
        }
        else
        {
            SetDetailRowData(
                Widget.mBuildingDetails[Index],
                L"-",
                L"-");
        }
    }

    auto ConflictHeadlineBackground = Widget.mConflictHeadlineBackground.lock();
    auto ConflictHeadlineText = Widget.mConflictHeadlineText.lock();
    FVector4 ConflictTint(0.82f, 0.92f, 0.76f, 0.98f);
    const bool HasRecentTaxEvent =
        Snapshot.TaxEventStatus.Active ||
        Snapshot.TaxEventStatus.NotificationDays > 0;

    if (Snapshot.RebelRiskScore >= 66.0)
        ConflictTint = FVector4(0.96f, 0.48f, 0.38f, 0.98f);
    else if (Snapshot.RebelRiskScore >= 33.0)
        ConflictTint = FVector4(0.96f, 0.78f, 0.28f, 0.98f);
    else if (Snapshot.TaxEventStatus.Active)
        ConflictTint =
            Snapshot.TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis ?
            FVector4(0.94f, 0.54f, 0.40f, 0.98f) :
            FVector4(0.94f, 0.76f, 0.32f, 0.98f);
    else if (ElectionWarningActive)
        ConflictTint = ElectionWarningTint;

    if (ConflictHeadlineBackground)
        ConflictHeadlineBackground->SetTint(ConflictTint);

    if (ConflictHeadlineText)
    {
        std::wstring Headline =
            L"반란 위험: " + Snapshot.RebelRiskLabel;

        if (Snapshot.TaxEventStatus.Active)
        {
            Headline +=
                L" / 파벌 경고: " +
                Snapshot.TaxEventStatus.Summary;
            Headline +=
                L"\n월드 효과: " +
                TaxEventWorldEffectSummary;
        }
        else if (Snapshot.TaxEventStatus.NotificationDays > 0 &&
            !Snapshot.TaxEventStatus.Summary.empty())
        {
            Headline +=
                L" / 최근 경고: " +
                Snapshot.TaxEventStatus.Summary;
            Headline +=
                L"\n정상화 상태: " +
                TaxEventWorldEffectSummary;
        }

        if (ElectionWarningActive)
        {
            Headline +=
                L"\n선거 압박: " +
                ElectionWarningSummary;
        }

        Headline +=
            L"\n평균 자유 만족도 " + FormatFixed1(Snapshot.AverageFreedom) +
            L" / 평균 치안 만족도 " + FormatFixed1(Snapshot.AverageSecurity) +
            L" / 평균 음식 만족도 " + FormatFixed1(Snapshot.AverageFood) +
            L"\n계엄령: " +
            std::wstring(
                Snapshot.MartialLawActive ? L"활성" : L"비활성") +
            L" / 평균 보건 만족도 " + FormatFixed1(Snapshot.AverageHealth);
        ConflictHeadlineText->SetText(Headline.c_str());
    }

    SetDetailRowData(
        Widget.mConflictDetails[0],
        L"정치 사건",
        Snapshot.TaxEventStatus.Active ?
            Snapshot.TaxEventStatus.Title :
            (HasRecentTaxEvent ?
                Snapshot.TaxEventStatus.Title :
                std::wstring(L"없음")),
        Snapshot.TaxEventStatus.Active,
        Snapshot.TaxEventStatus.Active ?
            (Snapshot.TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis ?
                FVector4(0.82f, 0.24f, 0.18f, 1.f) :
                FVector4(0.82f, 0.48f, 0.12f, 1.f)) :
            FVector4(0.20f, 0.56f, 0.20f, 1.f));
    SetDetailRowData(
        Widget.mConflictDetails[1],
        Snapshot.TaxEventStatus.Active ? L"파벌 경고 / 효과" :
            (ElectionWarningActive ? L"선거 경고" : L"사건 메모"),
        Snapshot.TaxEventStatus.Active ?
            (Snapshot.TaxEventStatus.Summary +
                L" / " +
                TaxEventWorldEffectSummary) :
            (ElectionWarningActive ?
                ElectionWarningSummary :
                (HasRecentTaxEvent ?
                (Snapshot.TaxEventStatus.Summary +
                    L" / " +
                    TaxEventWorldEffectSummary) :
                std::wstring(L"안정"))),
        Snapshot.TaxEventStatus.Active || ElectionWarningActive,
        Snapshot.TaxEventStatus.Active ?
            FVector4(0.82f, 0.48f, 0.12f, 1.f) :
            (ElectionWarningActive ?
                ElectionWarningTint :
                FVector4(0.31f, 0.27f, 0.21f, 1.f)));
    SetDetailRowData(
        Widget.mConflictDetails[2],
        L"반란 위험 점수",
        FormatFixed1(Snapshot.RebelRiskScore),
        true,
        Snapshot.RebelRiskScore >= 66.0 ?
            FVector4(0.78f, 0.18f, 0.18f, 1.f) :
            (Snapshot.RebelRiskScore >= 33.0 ?
                FVector4(0.82f, 0.48f, 0.12f, 1.f) :
                FVector4(0.20f, 0.56f, 0.20f, 1.f)));
    SetDetailRowData(
        Widget.mConflictDetails[3],
        L"평균 음식 만족도",
        FormatFixed1(Snapshot.AverageFood));
    SetDetailRowData(
        Widget.mConflictDetails[4],
        L"평균 보건 만족도",
        FormatFixed1(Snapshot.AverageHealth));
    SetDetailRowData(
        Widget.mConflictDetails[5],
        L"실업률",
        FormatPercent(UnemploymentRate * 100.0));
    SetDetailRowData(
        Widget.mConflictDetails[6],
        L"야권 지지율",
        FormatPercent(Snapshot.OppositionPercent));
    SetDetailRowData(
        Widget.mConflictDetails[7],
        L"재정 압박",
        FormatPercent(FiscalStress * 100.0));

    SetMetricRowData(
        Widget.mConflictMetrics[0],
        L"반란 위험",
        FormatPercent(Snapshot.RebelRiskScore),
        static_cast<float>(Clamp01(Snapshot.RebelRiskScore / 100.0)),
        FVector4(0.82f, 0.24f, 0.18f, 0.95f),
        true);
    SetMetricRowData(
        Widget.mConflictMetrics[1],
        L"체제 안정도",
        FormatPercent(Stability * 100.0),
        static_cast<float>(Stability),
        FVector4(0.18f, 0.66f, 0.32f, 0.95f));
    SetMetricRowData(
        Widget.mConflictMetrics[2],
        L"통제 강도",
        FormatPercent(ControlStrength * 100.0),
        static_cast<float>(ControlStrength),
        FVector4(0.24f, 0.52f, 0.88f, 0.95f));
}
