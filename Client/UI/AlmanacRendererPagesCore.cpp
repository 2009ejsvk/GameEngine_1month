#include "AlmanacRenderer.h"
#include "AlmanacCalc.h"
#include "AlmanacRendererInternal.h"
#include "AlmanacTheme.h"
#include "../Economy/ResourceTradePricing.h"
#include <algorithm>
#include <array>
#include <cmath>

using namespace AlmanacCalc;

namespace
{
    struct FSatisfactionDetailEntry
    {
        std::wstring Label;
        std::wstring Value;
        bool Highlight = false;
        FVector4 Tint = FVector4(0.31f, 0.27f, 0.21f, 1.f);
    };
    std::wstring FormatInteger(long long Value)
    {
        const bool Negative = Value < 0;
        unsigned long long AbsValue = Negative ?
            static_cast<unsigned long long>(-Value) :
            static_cast<unsigned long long>(Value);
        std::wstring Digits = std::to_wstring(AbsValue);
        std::wstring Result;
        int GroupCount = 0;
        for (int Index = static_cast<int>(Digits.size()) - 1; Index >= 0; --Index)
        {
            if (GroupCount == 3)
            {
                Result.insert(Result.begin(), L',');
                GroupCount = 0;
            }
            Result.insert(Result.begin(), Digits[static_cast<size_t>(Index)]);
            ++GroupCount;
        }
        if (Negative)
            Result.insert(Result.begin(), L'-');
        return Result;
    }

    std::wstring FormatSignedCompactCurrency(long long Value)
    {
        if (Value > 0)
            return L"+" + FormatCompactCurrency(Value);

        return FormatCompactCurrency(Value);
    }

    std::wstring FormatSignedPercentValue(int Value)
    {
        return std::wstring(Value > 0 ? L"+" : L"") +
            std::to_wstring(Value) +
            L"%";
    }

    FVector4 ResolveMarketPressureTint(int Value)
    {
        if (Value > 0)
            return FVector4(0.82f, 0.52f, 0.20f, 0.95f);
        if (Value < 0)
            return FVector4(0.22f, 0.56f, 0.78f, 0.95f);

        return FVector4(0.62f, 0.60f, 0.52f, 0.90f);
    }

    std::wstring BuildStoragePressureText(int BiasPercent)
    {
        if (BiasPercent > 0)
            return L"희소성 " + FormatSignedPercentValue(BiasPercent);
        if (BiasPercent < 0)
            return L"공급 과잉 " + FormatSignedPercentValue(BiasPercent);

        return L"안정";
    }

    std::wstring BuildBalancePressureText(int BiasPercent)
    {
        if (BiasPercent > 0)
            return L"소비 우세 " + FormatSignedPercentValue(BiasPercent);
        if (BiasPercent < 0)
            return L"생산 우세 " + FormatSignedPercentValue(BiasPercent);

        return L"균형";
    }

    std::wstring BuildTemporalPressureText(int BiasPercent)
    {
        if (BiasPercent > 0)
            return L"상승 파동 " + FormatSignedPercentValue(BiasPercent);
        if (BiasPercent < 0)
            return L"하락 파동 " + FormatSignedPercentValue(BiasPercent);

        return L"횡보";
    }

    std::wstring BuildEventPressureText(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot,
        int BiasPercent)
    {
        std::wstring SourceText;

        if (Snapshot.TaxEventStatus.Active &&
            !Snapshot.TaxEventStatus.Title.empty())
        {
            SourceText = Snapshot.TaxEventStatus.Title;
        }

        if (Snapshot.WorldCrisisStatus.Active &&
            !Snapshot.WorldCrisisStatus.Title.empty())
        {
            if (!SourceText.empty())
                SourceText += L" / ";

            SourceText += Snapshot.WorldCrisisStatus.Title;
        }

        if (SourceText.empty())
            return L"없음";

        return SourceText + L" " + FormatSignedPercentValue(BiasPercent);
    }

    std::wstring BuildTopBuildingMetricSummary(
        const std::vector<std::pair<std::wstring, int>>& Entries,
        size_t MaxCount,
        const wchar_t* Suffix = nullptr)
    {
        std::wstring Result;

        for (size_t Index = 0;
            Index < Entries.size() && Index < MaxCount;
            ++Index)
        {
            if (Entries[Index].second <= 0)
                continue;

            if (!Result.empty())
                Result += L", ";

            Result += Entries[Index].first;
            Result += L" ";
            Result += FormatInteger(Entries[Index].second);

            if (Suffix && *Suffix)
                Result += Suffix;
        }

        return Result;
    }

    std::wstring BuildResourceLogisticsWarning(
        const AlmanacDataProvider::FAlmanacResourceTypeSnapshot& Resource)
    {
        if (Resource.ShortagePressure > 0 &&
            !Resource.TopShortageBuildings.empty())
        {
            return L"병목: " +
                BuildTopBuildingMetricSummary(
                    Resource.TopShortageBuildings,
                    2);
        }

        if (Resource.ReservedPickup > 0 &&
            !Resource.TopReservedBuildings.empty())
        {
            return L"운송 대기: " +
                BuildTopBuildingMetricSummary(
                    Resource.TopReservedBuildings,
                    2);
        }

        if (!Resource.TopOverflowBuildings.empty())
        {
            return L"과잉 재고: " +
                BuildTopBuildingMetricSummary(
                    Resource.TopOverflowBuildings,
                    2,
                    L"%");
        }

        return L"흐름 안정";
    }

    std::wstring BuildFlowStageHeadline(
        const std::vector<std::pair<std::wstring, int>>& Entries,
        const wchar_t* EmptyText)
    {
        if (Entries.empty())
            return EmptyText ? std::wstring(EmptyText) : std::wstring(L"-");

        return Entries.front().first;
    }

    std::wstring BuildFlowStageNotice(
        const AlmanacDataProvider::FAlmanacResourceTypeSnapshot& Resource)
    {
        return L"대표 흐름: " +
            BuildFlowStageHeadline(Resource.TopProducerBuildings, L"-") +
            L" -> " +
            BuildFlowStageHeadline(Resource.TopWarehouseBuildings, L"-") +
            L" -> " +
            BuildFlowStageHeadline(
                Resource.ShortagePressure > 0 ?
                    Resource.TopShortageBuildings :
                    Resource.TopConsumerBuildings,
                L"-") +
            L" -> " +
            BuildFlowStageHeadline(Resource.TopHarborBuildings, L"-");
    }

    std::wstring BuildOverviewElectionText(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
        if (!Snapshot.HasMainWorld)
            return L"행정 데이터 없음";

        if (Snapshot.ElectionStatus.GameLost)
            return L"정권 상실";

        if (Snapshot.ElectionStatus.NextElectionYear <= 0 ||
            Snapshot.ElectionStatus.NextElectionMonth <= 0 ||
            Snapshot.ElectionStatus.NextElectionDay <= 0)
        {
            return L"선거 일정 없음";
        }

        std::wstring Result = FormatDate(
            Snapshot.ElectionStatus.NextElectionYear,
            Snapshot.ElectionStatus.NextElectionMonth,
            Snapshot.ElectionStatus.NextElectionDay);

        if (Snapshot.DaysUntilNextElection >= 0)
        {
            Result += L"\n";
            Result += std::to_wstring(Snapshot.DaysUntilNextElection);
            Result += L"일 남음";
        }

        return Result;
    }

    std::wstring BuildOverviewActiveEdictDetail(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
        if (Snapshot.ActiveEdictLines.empty())
            return L"시행 중 없음";

        if (Snapshot.ActiveEdictLines.size() == 1)
            return Snapshot.ActiveEdictLines[0];

        return Snapshot.ActiveEdictLines[0] +
            L"\n외 " +
            std::to_wstring(Snapshot.ActiveEdictLines.size() - 1) +
            L"개";
    }
}

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
        Snapshot.DaysUntilNextElection >= 0 &&
        Snapshot.DaysUntilNextElection <= 180 &&
        Snapshot.ElectionWarningScore >= 0.32;
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
            if (Snapshot.ElectionWarningScore >= 0.78)
                Text->SetTextColor(232, 86, 72, 255);
            else if (Snapshot.ElectionWarningScore >= 0.52)
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

void FAlmanacRenderer::ApplySatisfactionPage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    const int ActiveCitizenCount = (std::max)(1, Snapshot.ActiveCitizenCount);
    const int HousingVacancy =
        (std::max)(0, Snapshot.ResidentialCapacity - Snapshot.AssignedHomeCount);
    const int JobVacancy =
        (std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount);
    const double HomelessRate =
        static_cast<double>(Snapshot.HomelessCount) /
        static_cast<double>(ActiveCitizenCount);
    const double UnemploymentRate =
        static_cast<double>(Snapshot.UnemployedCount) /
        static_cast<double>(ActiveCitizenCount);
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

    int SelectedSatisfactionIndex =
        (std::max)(0,
            (std::min)(
                GSatisfactionRowCount - 1,
                Widget.mSelectedSatisfactionIndex));
    Widget.mSelectedSatisfactionIndex = SelectedSatisfactionIndex;

    for (int Index = 0; Index < GSatisfactionRowCount; ++Index)
    {
        SetSatisfactionRowData(
            Widget.mSatisfactionRows[Index],
            GetSatisfactionLabel(Index),
            std::to_wstring(static_cast<int>(std::round(SatisfactionValues[Index]))),
            static_cast<float>(Clamp01(SatisfactionValues[Index] / 100.0)),
            AlmanacTheme::GetSatisfactionTint(Index),
            Index == SelectedSatisfactionIndex);
    }

    const FVector4 SatisfactionAccentTint =
        AlmanacTheme::GetSatisfactionTint(SelectedSatisfactionIndex);
    const std::wstring SelectedSatisfactionLabel =
        GetSatisfactionLabel(SelectedSatisfactionIndex);
    bool ShowSatisfactionTooltip = false;
    if (SelectedSatisfactionIndex >= 0 &&
        SelectedSatisfactionIndex < static_cast<int>(Widget.mSatisfactionRows.size()))
    {
        if (auto SelectedButton =
            Widget.mSatisfactionRows[static_cast<size_t>(SelectedSatisfactionIndex)].Button.lock())
        {
            ShowSatisfactionTooltip = SelectedButton->GetMouseOn();
        }
    }
    const double CaribbeanBenchmarkValue =
        ClampSatisfactionValue((std::max)(78.0, Snapshot.AverageOverall + 17.0));
    const int FoodSupplyNetworkCount =
        (std::max)(1, (Snapshot.FoodProviderCount + 1) / 3);
    const int GourmetRestaurantCount =
        (std::max)(1, Snapshot.FoodProviderCount / 20);
    const int FastFoodRestaurantCount =
        (std::max)(1, FoodSupplyNetworkCount / 2 - 1);
    const int RestaurantCount =
        (std::max)(1,
            FoodSupplyNetworkCount -
            FastFoodRestaurantCount -
            GourmetRestaurantCount);
    const int HealthBuildingCount =
        (std::max)(1, Snapshot.TotalBuildingCount / 800);
    const int HealthDeathCount =
        (std::max)(0,
            static_cast<int>(std::round(
                (1.0 - Clamp01(Snapshot.AverageHealth / 100.0)) *
                static_cast<double>(Snapshot.ActiveCitizenCount) *
                0.081)));
    const int FreedomGuerrillaCount =
        (std::max)(0,
            static_cast<int>(std::round(
                (45.0 - Snapshot.AverageFreedom) * 0.12)));
    const int FreedomModifierEstimate =
        (std::max)(0,
            Snapshot.FreedomInfluenceBuildingCount +
            (Snapshot.MartialLawActive ? -5 : 5));
    int SecurityModifierEstimate = 0;

    if (Snapshot.MartialLawActive)
        SecurityModifierEstimate += 5;

    if (Snapshot.TaxEventStatus.Active)
    {
        switch (Snapshot.TaxEventStatus.Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
        case ETaxPolicyEventType::PropertyTaxBacklash:
            --SecurityModifierEstimate;
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            SecurityModifierEstimate -= 2;
            break;
        default:
            break;
        }
    }

    const int SecurityCrimeLossCount =
        (std::max)(0,
            static_cast<int>(std::round(
                (34.0 - Snapshot.AverageSecurity) * 0.12 +
                HomelessRate * static_cast<double>(Snapshot.ActiveCitizenCount) * 0.03 +
                UnemploymentRate * static_cast<double>(Snapshot.ActiveCitizenCount) * 0.02)));
    const int SecurityDetectedCriminalCount =
        (std::max)(0,
            static_cast<int>(std::round(
                (38.0 - Snapshot.AverageSecurity) * 0.10 +
                Snapshot.RebelRiskScore * 0.015)));
    const long long EntertainmentRevenueEstimate =
        static_cast<long long>(Snapshot.EntertainmentBuildingCount) * 600LL +
        static_cast<long long>(std::llround(
            Snapshot.AverageFun * 48.0 +
            Snapshot.SupportPercent * 2.0));
    std::wstring SatisfactionDescription;
    std::array<FSatisfactionDetailEntry, GSatisfactionDetailCount>
        SatisfactionDetailEntries = {};
    int SatisfactionVisibleDetailCount = GSatisfactionDetailCount;

    const auto AssignSatisfactionDetail =
        [&SatisfactionDetailEntries](
            int Index,
            const std::wstring& Label,
            const std::wstring& Value,
            bool Highlight = false,
            const FVector4& Tint = FVector4(0.31f, 0.27f, 0.21f, 1.f))
    {
        if (Index < 0 || Index >= GSatisfactionDetailCount)
            return;

        SatisfactionDetailEntries[static_cast<size_t>(Index)].Label = Label;
        SatisfactionDetailEntries[static_cast<size_t>(Index)].Value = Value;
        SatisfactionDetailEntries[static_cast<size_t>(Index)].Highlight = Highlight;
        SatisfactionDetailEntries[static_cast<size_t>(Index)].Tint = Tint;
    };

    switch (SelectedSatisfactionIndex)
    {
    case 0:
        SatisfactionDescription =
            L"시민들은 모든 만족도의 평균인\n"
            L"종합 만족도를 카리브해 만족도와\n"
            L"비교합니다. 종합 만족도가 현저히\n"
            L"낮을 경우, 국민들은 이민을\n"
            L"결심하기도 합니다. 반대로\n"
            L"트로피코의 종합 만족도가\n"
            L"카리브해보다 높을 경우, 외지의\n"
            L"사람들이 트로피코로 이민을\n"
            L"결심을 하게 됩니다.";
        SatisfactionVisibleDetailCount = 2;
        AssignSatisfactionDetail(
            0, L"종합 만족도", FormatFixed1(Snapshot.AverageOverall), true,
            FVector4(0.18f, 0.42f, 0.86f, 1.f));
        AssignSatisfactionDetail(
            1, L"카리브해 만족도",
            FormatFixed1(CaribbeanBenchmarkValue),
            true,
            FVector4(0.72f, 0.18f, 0.18f, 1.f));
        AssignSatisfactionDetail(2, L"", L"");
        AssignSatisfactionDetail(3, L"", L"");
        AssignSatisfactionDetail(4, L"", L"");
        AssignSatisfactionDetail(5, L"", L"");
        break;
    case 1:
        SatisfactionDescription =
            L"시민들은 정기적으로 음식을 섭취해야 합니다.\n"
            L"음식이 충분하면 집에서 식사하고, 부족하면 공급 시설을 찾아갑니다.\n"
            L"음식 만족도가 매우 낮은 시민은 기아로 사망할 수도 있습니다.";
        AssignSatisfactionDetail(
            0,
            L"기아로 인한 사망 (지난 12개월)",
            L"0");
        AssignSatisfactionDetail(
            1,
            L"▷ 음식 제공 기관",
            std::to_wstring(Snapshot.FoodProviderCount),
            false,
            FVector4(0.47f, 0.41f, 0.22f, 1.f));
        AssignSatisfactionDetail(
            2,
            L"▽ 음식 공급",
            std::to_wstring(FoodSupplyNetworkCount),
            false,
            FVector4(0.47f, 0.41f, 0.22f, 1.f));
        AssignSatisfactionDetail(
            3,
            L"    ▷ 레스토랑",
            std::to_wstring(RestaurantCount));
        AssignSatisfactionDetail(
            4,
            L"    ▷ 패스트푸드 체인점",
            std::to_wstring(FastFoodRestaurantCount));
        AssignSatisfactionDetail(
            5,
            L"    ▷ 고급 레스토랑",
            std::to_wstring(GourmetRestaurantCount));
        break;
    case 2:
        SatisfactionDescription =
            L"시민들은 이따금씩 의료 서비스를\n"
            L"제공하는 건물을 방문해야 합니다.\n"
            L"해당 건물의 서비스 품질은\n"
            L"시민들의 보건 만족도에 영향을\n"
            L"미칩니다. 보건 만족도가 매우 낮은\n"
            L"시민은 질병으로 사망할 수도\n"
            L"있습니다.";
        AssignSatisfactionDetail(
            0,
            L"보건 문제로 인한 사망 (지난 12개월)",
            std::to_wstring(HealthDeathCount));
        AssignSatisfactionDetail(
            1,
            L"▷ 보건 건물",
            std::to_wstring(HealthBuildingCount),
            false,
            FVector4(0.47f, 0.41f, 0.22f, 1.f));
        SatisfactionVisibleDetailCount = 2;
        break;
    case 3:
        SatisfactionDescription =
            L"시민들은 오락 건물을 방문해야\n"
            L"합니다. 해당하는 건물들의 서비스\n"
            L"품질이 유흥 만족도에 영향을\n"
            L"미칩니다. 유흥 만족도가 매우 낮은\n"
            L"시민은 삶의 동기를 잃고 속도가\n"
            L"크게 감소합니다.";
        SatisfactionVisibleDetailCount = 2;
        AssignSatisfactionDetail(
            0,
            L"▷ 오락 건물 수익",
            std::to_wstring(EntertainmentRevenueEstimate));
        AssignSatisfactionDetail(
            1,
            L"▷ 오락 건물",
            std::to_wstring(Snapshot.EntertainmentBuildingCount));
        break;
    case 4:
        SatisfactionDescription =
            L"시민들은 이따금씩 신념을 새로이\n"
            L"하기 위해 종교적 건물을 방문해야\n"
            L"합니다. 건물의 서비스 품질은\n"
            L"시민들의 신앙 만족도를\n"
            L"정의합니다. 신앙 만족도가 매우\n"
            L"낮은 시민들은 가족을 부양하지\n"
            L"않게 됩니다.";
        AssignSatisfactionDetail(
            0,
            L"▷ 성직자 건물",
            std::to_wstring(Snapshot.FaithBuildingCount));
        SatisfactionVisibleDetailCount = 1;
        break;
    case 5:
        SatisfactionDescription =
            L"시민들의 주거 만족도는 현재\n"
            L"거주지의 주거 품질에 좌우됩니다.\n"
            L"직장 부근에 빈 건물이 없거나\n"
            L"경제적인 여유가 없는 시민은\n"
            L"판잣집을 짓고 거기서 살게 될\n"
            L"겁니다.";
        AssignSatisfactionDetail(
            0,
            L"▷ 무주택자",
            std::to_wstring(Snapshot.HomelessCount),
            Snapshot.HomelessCount > 0,
            Snapshot.HomelessCount > 0 ?
                FVector4(0.82f, 0.24f, 0.18f, 1.f) :
                FVector4(0.31f, 0.27f, 0.21f, 1.f));
        AssignSatisfactionDetail(
            1,
            L"▷ 빈 공간이 있는 주거 건물",
            std::to_wstring(Snapshot.ResidentialVacancyBuildingCount));
        AssignSatisfactionDetail(
            2,
            L"▷ 빈 주거 슬롯",
            std::to_wstring(HousingVacancy));
        SatisfactionVisibleDetailCount = 3;
        break;
    case 6:
        SatisfactionDescription =
            L"시민들의 직업 만족도는 직장에서\n"
            L"제공하는 직업 품질에 좌우됩니다.\n"
            L"직업이 없는 시민들은 치안 및 주거\n"
            L"수준에 따라 지도 새도 모르게\n"
            L"범죄자가 될 수 있습니다.";
        AssignSatisfactionDetail(
            0,
            L"▷ 실업자",
            std::to_wstring(Snapshot.UnemployedCount),
            Snapshot.UnemployedCount > 0,
            Snapshot.UnemployedCount > 0 ?
                FVector4(0.82f, 0.24f, 0.18f, 1.f) :
                FVector4(0.31f, 0.27f, 0.21f, 1.f));
        AssignSatisfactionDetail(
            1,
            L"▷ 빈 일자리가 있는 건물",
            std::to_wstring(Snapshot.WorkVacancyBuildingCount));
        AssignSatisfactionDetail(
            2,
            L"▷ 직장",
            std::to_wstring(Snapshot.JobCapacity));
        AssignSatisfactionDetail(
            3,
            L"▷ 직업 품질 수정치",
            std::to_wstring(JobVacancy));
        SatisfactionVisibleDetailCount = 4;
        break;
    case 7:
        SatisfactionDescription =
            L"시민의 집과 직장을 둘러싼 구역의\n"
            L"자유 등급은 그들이 일하거나 쉴\n"
            L"때마다 자유 만족도에 영향을\n"
            L"미칩니다. 자유 만족도가 매우 낮은\n"
            L"시민은 지도 새도 모르게 반란군이\n"
            L"될지도 모릅니다.";
        AssignSatisfactionDetail(
            0,
            L"게릴라로 돌아선 시민 (지난 12개월)",
            std::to_wstring(FreedomGuerrillaCount));
        AssignSatisfactionDetail(
            1,
            L"▷ 자유에 영향을 주는 건물",
            std::to_wstring(Snapshot.FreedomInfluenceBuildingCount));
        AssignSatisfactionDetail(
            2,
            L"▷ 자유 만족도 수정치",
            std::to_wstring(FreedomModifierEstimate));
        SatisfactionVisibleDetailCount = 3;
        break;
    case 8:
    default:
        SatisfactionDescription =
            L"시민의 집과 직장을 둘러싼 구역의\n"
            L"치안 등급은 그들이 일하거나 쉴\n"
            L"때마다 치안 만족도에 영향을\n"
            L"미칩니다. 치안 만족도가 매우 낮은\n"
            L"시민은 지도 새도 모르게 범죄자가\n"
            L"될지도 모릅니다.";
        AssignSatisfactionDetail(
            0,
            L"범죄 손실 (지난 12개월)",
            std::to_wstring(SecurityCrimeLossCount));
        AssignSatisfactionDetail(
            1,
            L"발견된 범죄자",
            std::to_wstring(SecurityDetectedCriminalCount));
        AssignSatisfactionDetail(
            2,
            L"▷ 치안에 영향을 주는 건물",
            std::to_wstring(Snapshot.SecurityInfluenceBuildingCount));
        AssignSatisfactionDetail(
            3,
            L"▷ 치안 만족도 수정치",
            std::to_wstring(SecurityModifierEstimate));
        SatisfactionVisibleDetailCount = 4;
        break;
    }

    if (auto ChartTitle = Widget.mSatisfactionChartTitle.lock())
    {
        const std::wstring ChartTitleText =
            SelectedSatisfactionIndex == 0 ?
                std::wstring(L"종합 만족도") :
                (SelectedSatisfactionLabel + L" 만족도");
        ChartTitle->SetText(ChartTitleText.c_str());
    }

    if (auto TooltipText = Widget.mSatisfactionTooltipText.lock())
        TooltipText->SetText(SatisfactionDescription.c_str());

    if (auto TooltipPanel = Widget.mSatisfactionTooltipPanel.lock())
        TooltipPanel->SetEnable(ShowSatisfactionTooltip);

    if (auto TooltipText = Widget.mSatisfactionTooltipText.lock())
        TooltipText->SetEnable(ShowSatisfactionTooltip);

    for (int Index = 0; Index < GSatisfactionGraphPointCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mSatisfactionChartXAxisLabels.size()))
            continue;

        if (auto XLabel = Widget.mSatisfactionChartXAxisLabels[Index].lock())
            XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
    }

    const wchar_t* SatisfactionYAxisLabels[GSatisfactionGraphGridLineCount] =
    {
        L"20",
        L"40",
        L"60",
        L"80"
    };
    float SatisfactionGraphMin = 0.f;
    float SatisfactionGraphMax = 100.f;

    if (SelectedSatisfactionIndex == 0)
    {
        SatisfactionYAxisLabels[0] = L"20";
        SatisfactionYAxisLabels[1] = L"40";
        SatisfactionYAxisLabels[2] = L"60";
        SatisfactionYAxisLabels[3] = L"80";
        SatisfactionGraphMin = 20.f;
        SatisfactionGraphMax = 80.f;
    }
    else if (SelectedSatisfactionIndex == 1)
    {
        SatisfactionYAxisLabels[0] = L"20";
        SatisfactionYAxisLabels[1] = L"30";
        SatisfactionYAxisLabels[2] = L"40";
        SatisfactionYAxisLabels[3] = L"50";
        SatisfactionGraphMin = 15.f;
        SatisfactionGraphMax = 55.f;
    }
    else if (SelectedSatisfactionIndex == 2)
    {
        SatisfactionYAxisLabels[0] = L"10";
        SatisfactionYAxisLabels[1] = L"20";
        SatisfactionYAxisLabels[2] = L"30";
        SatisfactionYAxisLabels[3] = L"40";
        SatisfactionGraphMin = 0.f;
        SatisfactionGraphMax = 45.f;
    }
    else if (SelectedSatisfactionIndex == 3)
    {
        SatisfactionYAxisLabels[0] = L"10";
        SatisfactionYAxisLabels[1] = L"30";
        SatisfactionYAxisLabels[2] = L"50";
        SatisfactionYAxisLabels[3] = L"70";
        SatisfactionGraphMin = -10.f;
        SatisfactionGraphMax = 70.f;
    }
    else if (SelectedSatisfactionIndex == 4)
    {
        SatisfactionYAxisLabels[0] = L"20";
        SatisfactionYAxisLabels[1] = L"30";
        SatisfactionYAxisLabels[2] = L"40";
        SatisfactionYAxisLabels[3] = L"50";
        SatisfactionGraphMin = 18.f;
        SatisfactionGraphMax = 52.f;
    }
    else if (SelectedSatisfactionIndex == 6)
    {
        SatisfactionYAxisLabels[0] = L"20";
        SatisfactionYAxisLabels[1] = L"40";
        SatisfactionYAxisLabels[2] = L"60";
        SatisfactionYAxisLabels[3] = L"80";
        SatisfactionGraphMin = 0.f;
        SatisfactionGraphMax = 80.f;
    }
    else if (SelectedSatisfactionIndex == 7)
    {
        SatisfactionYAxisLabels[0] = L"20";
        SatisfactionYAxisLabels[1] = L"40";
        SatisfactionYAxisLabels[2] = L"60";
        SatisfactionYAxisLabels[3] = L"80";
        SatisfactionGraphMin = 0.f;
        SatisfactionGraphMax = 100.f;
    }
    else if (SelectedSatisfactionIndex == 8)
    {
        SatisfactionYAxisLabels[0] = L"10";
        SatisfactionYAxisLabels[1] = L"30";
        SatisfactionYAxisLabels[2] = L"50";
        SatisfactionYAxisLabels[3] = L"70";
        SatisfactionGraphMin = 10.f;
        SatisfactionGraphMax = 80.f;
    }

    for (int Index = 0; Index < GSatisfactionGraphGridLineCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mSatisfactionChartYAxisLabels.size()))
            continue;

        if (auto YLabel = Widget.mSatisfactionChartYAxisLabels[Index].lock())
            YLabel->SetText(SatisfactionYAxisLabels[Index]);
    }

    const std::array<float, GSatisfactionGraphPointCount> SatisfactionTrend =
        SelectedSatisfactionIndex == 2 ?
            std::array<float, GSatisfactionGraphPointCount>
            {
                static_cast<float>(ClampSatisfactionValue(Snapshot.AverageHealth + 2.2)),
                static_cast<float>(ClampSatisfactionValue(Snapshot.AverageHealth + 1.8)),
                static_cast<float>(ClampSatisfactionValue(Snapshot.AverageHealth + 1.6)),
                static_cast<float>(ClampSatisfactionValue(Snapshot.AverageHealth + 1.4))
            } :
        SelectedSatisfactionIndex == 3 ?
            std::array<float, GSatisfactionGraphPointCount>
            {
                static_cast<float>((std::max)(-10.0, Snapshot.AverageFun - 18.0)),
                static_cast<float>((std::max)(-10.0, Snapshot.AverageFun - 13.0)),
                static_cast<float>((std::min)(70.0, Snapshot.AverageFun - 4.0)),
                static_cast<float>((std::min)(70.0, Snapshot.AverageFun + 2.0))
            } :
        SelectedSatisfactionIndex == 4 ?
            std::array<float, GSatisfactionGraphPointCount>
            {
                static_cast<float>((std::max)(18.0, (std::min)(52.0, Snapshot.AverageFaith + 3.2))),
                static_cast<float>((std::max)(18.0, (std::min)(52.0, Snapshot.AverageFaith + 2.6))),
                static_cast<float>((std::max)(18.0, (std::min)(52.0, Snapshot.AverageFaith + 1.4))),
                static_cast<float>((std::max)(18.0, (std::min)(52.0, Snapshot.AverageFaith)))
            } :
        SelectedSatisfactionIndex == 5 ?
            std::array<float, GSatisfactionGraphPointCount>
            {
                static_cast<float>((std::max)(0.0, (std::min)(100.0, Snapshot.AverageHousing + 1.4))),
                static_cast<float>((std::max)(0.0, (std::min)(100.0, Snapshot.AverageHousing + 1.2))),
                static_cast<float>((std::max)(0.0, (std::min)(100.0, Snapshot.AverageHousing + 0.8))),
                static_cast<float>((std::max)(0.0, (std::min)(100.0, Snapshot.AverageHousing + 0.2)))
            } :
        SelectedSatisfactionIndex == 6 ?
            std::array<float, GSatisfactionGraphPointCount>
            {
                static_cast<float>((std::max)(0.0, (std::min)(80.0, Snapshot.AverageJob - 0.6))),
                static_cast<float>((std::max)(0.0, (std::min)(80.0, Snapshot.AverageJob + 0.1))),
                static_cast<float>((std::max)(0.0, (std::min)(80.0, Snapshot.AverageJob - 0.2))),
                static_cast<float>((std::max)(0.0, (std::min)(80.0, Snapshot.AverageJob)))
            } :
        SelectedSatisfactionIndex == 7 ?
            std::array<float, GSatisfactionGraphPointCount>
            {
                static_cast<float>((std::max)(0.0, (std::min)(100.0, Snapshot.AverageFreedom - 15.0))),
                static_cast<float>((std::max)(0.0, (std::min)(100.0, Snapshot.AverageFreedom - 15.5))),
                static_cast<float>((std::max)(0.0, (std::min)(100.0, Snapshot.AverageFreedom - 3.5))),
                static_cast<float>((std::max)(0.0, (std::min)(100.0, Snapshot.AverageFreedom - 1.0)))
            } :
        SelectedSatisfactionIndex == 8 ?
            std::array<float, GSatisfactionGraphPointCount>
            {
                static_cast<float>((std::max)(10.0, (std::min)(80.0, Snapshot.AverageSecurity - 0.8))),
                static_cast<float>((std::max)(10.0, (std::min)(80.0, Snapshot.AverageSecurity - 1.4))),
                static_cast<float>((std::max)(10.0, (std::min)(80.0, Snapshot.AverageSecurity + 0.2))),
                static_cast<float>((std::max)(10.0, (std::min)(80.0, Snapshot.AverageSecurity - 0.6)))
            } :
            BuildSatisfactionTrend(
                SatisfactionValues[SelectedSatisfactionIndex],
                SelectedSatisfactionIndex == 0 ?
                    Snapshot.AverageOverall :
                    SatisfactionValues[SelectedSatisfactionIndex],
                SelectedSatisfactionIndex == 0 ? 0.0 : 0.4);
    const std::array<float, GSatisfactionGraphPointCount> BenchmarkTrend =
        BuildSatisfactionTrend(
            CaribbeanBenchmarkValue,
            CaribbeanBenchmarkValue,
            -0.6);

    if (auto ChartFrame = Widget.mSatisfactionChartFrame.lock())
    {
        const float GraphLeft = ChartFrame->GetPos().x + 42.f;
        const float GraphTop = ChartFrame->GetPos().y + 24.f;
        const float GraphWidth = ChartFrame->GetSize().x - 58.f;
        const float GraphHeight = ChartFrame->GetSize().y - 56.f;
        const float PointGapX =
            GSatisfactionGraphPointCount > 1 ?
            GraphWidth /
                static_cast<float>(GSatisfactionGraphPointCount - 1) :
            GraphWidth;

        for (int SegmentIndex = 0;
            SegmentIndex < GSatisfactionGraphSegmentCount;
            ++SegmentIndex)
        {
            if (SegmentIndex <
                static_cast<int>(Widget.mSatisfactionChartPrimaryLines.size()))
            {
                SetLineSegment(
                    Widget.mSatisfactionChartPrimaryLines[SegmentIndex].lock(),
                    GraphLeft + PointGapX * static_cast<float>(SegmentIndex),
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        SatisfactionTrend[SegmentIndex],
                        SatisfactionGraphMin,
                        SatisfactionGraphMax),
                    GraphLeft + PointGapX * static_cast<float>(SegmentIndex + 1),
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        SatisfactionTrend[SegmentIndex + 1],
                        SatisfactionGraphMin,
                        SatisfactionGraphMax),
                    2.2f,
                    SelectedSatisfactionIndex == 2 ?
                        FVector4(0.82f, 0.30f, 0.22f, 0.96f) :
                    SelectedSatisfactionIndex == 3 ?
                        FVector4(0.94f, 0.58f, 0.10f, 0.96f) :
                    SelectedSatisfactionIndex == 4 ?
                        FVector4(0.76f, 0.42f, 0.86f, 0.96f) :
                    SelectedSatisfactionIndex == 5 ?
                        FVector4(0.92f, 0.72f, 0.18f, 0.96f) :
                    SelectedSatisfactionIndex == 6 ?
                        FVector4(0.64f, 0.72f, 0.34f, 0.96f) :
                    SelectedSatisfactionIndex == 7 ?
                        FVector4(0.44f, 0.92f, 0.82f, 0.96f) :
                    SelectedSatisfactionIndex == 8 ?
                        FVector4(0.64f, 0.50f, 0.24f, 0.96f) :
                        FVector4(0.34f, 0.54f, 0.86f, 0.96f));
            }

            if (SegmentIndex <
                static_cast<int>(Widget.mSatisfactionChartSecondaryLines.size()))
            {
                auto SecondaryLine =
                    Widget.mSatisfactionChartSecondaryLines[SegmentIndex].lock();

                if (!SecondaryLine)
                    continue;

                if (SelectedSatisfactionIndex == 0)
                {
                    SetLineSegment(
                        SecondaryLine,
                        GraphLeft + PointGapX * static_cast<float>(SegmentIndex),
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            BenchmarkTrend[SegmentIndex],
                            SatisfactionGraphMin,
                            SatisfactionGraphMax),
                        GraphLeft + PointGapX * static_cast<float>(SegmentIndex + 1),
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            BenchmarkTrend[SegmentIndex + 1],
                            SatisfactionGraphMin,
                            SatisfactionGraphMax),
                        2.2f,
                        FVector4(0.82f, 0.30f, 0.22f, 0.90f));
                }
                else if (SelectedSatisfactionIndex == 2)
                {
                    SetLineSegment(
                        SecondaryLine,
                        GraphLeft + PointGapX * static_cast<float>(SegmentIndex),
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            0.f,
                            SatisfactionGraphMin,
                            SatisfactionGraphMax),
                        GraphLeft + PointGapX * static_cast<float>(SegmentIndex + 1),
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            0.f,
                            SatisfactionGraphMin,
                            SatisfactionGraphMax),
                        2.2f,
                        FVector4(0.34f, 0.54f, 0.86f, 0.92f));
                }
                else if (SelectedSatisfactionIndex == 3)
                {
                    SetLineSegment(
                        SecondaryLine,
                        GraphLeft + PointGapX * static_cast<float>(SegmentIndex),
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            0.f,
                            SatisfactionGraphMin,
                            SatisfactionGraphMax),
                        GraphLeft + PointGapX * static_cast<float>(SegmentIndex + 1),
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            0.f,
                            SatisfactionGraphMin,
                            SatisfactionGraphMax),
                        2.2f,
                        FVector4(0.68f, 0.36f, 0.82f, 0.92f));
                }
                else if (SelectedSatisfactionIndex == 8)
                {
                    SetLineSegment(
                        SecondaryLine,
                        GraphLeft + PointGapX * static_cast<float>(SegmentIndex),
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            10.f,
                            SatisfactionGraphMin,
                            SatisfactionGraphMax),
                        GraphLeft + PointGapX * static_cast<float>(SegmentIndex + 1),
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            10.f,
                            SatisfactionGraphMin,
                            SatisfactionGraphMax),
                        2.2f,
                        FVector4(0.40f, 0.88f, 0.80f, 0.92f));
                }
                else
                {
                    SecondaryLine->SetEnable(false);
                }
            }
        }
    }

    for (int Index = 0; Index < GSatisfactionDetailCount; ++Index)
    {
        SetDetailRowData(
            Widget.mSatisfactionDetails[Index],
            SatisfactionDetailEntries[static_cast<size_t>(Index)].Label,
            SatisfactionDetailEntries[static_cast<size_t>(Index)].Value,
            SatisfactionDetailEntries[static_cast<size_t>(Index)].Highlight,
            SatisfactionDetailEntries[static_cast<size_t>(Index)].Tint);

        const bool EnableRow =
            Index < SatisfactionVisibleDetailCount;
        auto RowBackground = Widget.mSatisfactionDetails[Index].Background.lock();
        auto RowLabel = Widget.mSatisfactionDetails[Index].Label.lock();
        auto RowValue = Widget.mSatisfactionDetails[Index].Value.lock();

        if (RowBackground)
        {
            RowBackground->SetEnable(EnableRow);

            if (EnableRow && SelectedSatisfactionIndex == 0)
            {
                RowBackground->SetTint(
                    Index == 0 ?
                        FVector4(0.36f, 0.56f, 0.90f, 0.94f) :
                        FVector4(0.82f, 0.36f, 0.30f, 0.94f));
            }
            else if (EnableRow && SelectedSatisfactionIndex == 2)
            {
                RowBackground->SetTexture(
                    RowBackground->GetName() + "_health",
                    GBarBackTexture);
                RowBackground->SetTint(
                    Index == 0 ?
                        FVector4(0.76f, 0.76f, 0.76f, 0.78f) :
                        FVector4(0.96f, 0.94f, 0.88f, 0.58f));
            }
            else if (EnableRow && SelectedSatisfactionIndex == 1)
            {
                RowBackground->SetTint(
                    Index == 0 ?
                        FVector4(0.84f, 0.84f, 0.84f, 0.78f) :
                        (Index <= 2 ?
                            FVector4(0.99f, 0.96f, 0.88f, 0.88f) :
                            FVector4(1.f, 1.f, 1.f, 0.86f)));
            }
            else if (EnableRow && SelectedSatisfactionIndex == 4)
            {
                RowBackground->SetTint(FVector4(0.98f, 0.96f, 0.92f, 0.86f));
            }
        }

        if (RowLabel)
        {
            RowLabel->SetEnable(EnableRow);
            if (EnableRow && SelectedSatisfactionIndex == 0)
            {
                RowLabel->SetTextColor(246, 244, 238, 255);
            }
            else if (EnableRow && SelectedSatisfactionIndex == 2)
            {
                RowLabel->SetTextColor(
                    Index == 0 ? 92 : 104,
                    Index == 0 ? 86 : 90,
                    Index == 0 ? 76 : 58,
                    255);
            }
            else if (EnableRow && SelectedSatisfactionIndex == 1)
            {
                RowLabel->SetTextColor(
                    Index <= 2 ? 124 : 108,
                    Index <= 2 ? 102 : 92,
                    Index <= 2 ? 48 : 58,
                    255);
            }
            else if (EnableRow && SelectedSatisfactionIndex == 4)
            {
                RowLabel->SetTextColor(118, 98, 52, 255);
            }
            else
            {
                RowLabel->SetTextColor(76, 70, 60, 255);
            }
        }
        if (RowValue)
        {
            RowValue->SetEnable(EnableRow);
            if (EnableRow && SelectedSatisfactionIndex == 0)
            {
                RowValue->SetTextColor(248, 246, 240, 255);
            }
            else if (EnableRow && SelectedSatisfactionIndex == 2)
            {
                RowValue->SetTextColor(
                    Index == 0 ? 88 : 120,
                    Index == 0 ? 82 : 96,
                    Index == 0 ? 72 : 42,
                    255);
            }
            else if (EnableRow && SelectedSatisfactionIndex == 1)
            {
                RowValue->SetTextColor(
                    Index <= 2 ? 112 : 94,
                    Index <= 2 ? 98 : 84,
                    Index <= 2 ? 54 : 52,
                    255);
            }
            else if (EnableRow && SelectedSatisfactionIndex == 4)
            {
                RowValue->SetTextColor(112, 92, 48, 255);
            }
        }
    }

}



void FAlmanacRenderer::ApplyResourcePage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    struct FResourceUiEntry
    {
        const AlmanacDataProvider::FAlmanacResourceTypeSnapshot* Resource =
            nullptr;
        std::wstring Name;
    };
    std::vector<FResourceUiEntry> ResourceEntries;
    ResourceEntries.reserve(static_cast<size_t>(EResourceType::Count));

    for (int ResourceIndex = 1;
        ResourceIndex < static_cast<int>(EResourceType::Count);
        ++ResourceIndex)
    {
        const EResourceType ResourceType =
            static_cast<EResourceType>(ResourceIndex);

        if (!IsExportableResourceType(ResourceType))
            continue;

        FResourceUiEntry Entry;
        Entry.Resource =
            &Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)];
        Entry.Name = GetResourceTypeDisplayName(ResourceType);
        ResourceEntries.push_back(std::move(Entry));
    }

    const int ResourceMaxIndex =
        (std::max)(0, static_cast<int>(ResourceEntries.size()) - 1);
    const int SelectedResourceIndex =
        (std::max)(0, (std::min)(Widget.mSelectedResourceIndex, ResourceMaxIndex));
    Widget.mSelectedResourceIndex = SelectedResourceIndex;
    const int VisibleRowCount =
        static_cast<int>(Widget.mResourceRows.size());
    const int ResourceWindowMaxStart = (std::max)(
        0,
        static_cast<int>(ResourceEntries.size()) - VisibleRowCount);
    const int VisibleStartIndex = (std::max)(
        0,
        (std::min)(
            SelectedResourceIndex - VisibleRowCount / 2,
            ResourceWindowMaxStart));
    Widget.mVisibleResourceRowOffset = VisibleStartIndex;
    const FResourceUiEntry& SelectedResourceEntry =
        ResourceEntries[static_cast<size_t>(SelectedResourceIndex)];
    const AlmanacDataProvider::FAlmanacResourceTypeSnapshot& SelectedResource =
        *SelectedResourceEntry.Resource;
    const int ExportUnitPrice =
        ResourceTradePricing::GetExportPricePerStockUnit(
            SelectedResource.Type);
    const int ImportUnitPrice =
        ResourceTradePricing::GetImportPricePerStockUnit(
            SelectedResource.Type);
    const int StorageBiasPercent =
        ResourceTradePricing::GetStorageBiasPercent(
            SelectedResource.Type);
    const int BalanceBiasPercent =
        ResourceTradePricing::GetBalanceBiasPercent(
            SelectedResource.Type);
    const int TemporalBiasPercent =
        ResourceTradePricing::GetTemporalBiasPercent(
            SelectedResource.Type);
    const int EventBiasPercent =
        ResourceTradePricing::GetEventBiasPercent(
            SelectedResource.Type);
    const int DiplomacyExportBiasPercent =
        ResourceTradePricing::GetDiplomacyExportBiasPercent(
            SelectedResource.Type);
    const int DiplomacyImportBiasPercent =
        ResourceTradePricing::GetDiplomacyImportBiasPercent(
            SelectedResource.Type);
    const int EdictExportBiasPercent =
        ResourceTradePricing::GetEdictExportBiasPercent(
            SelectedResource.Type);
    const int EdictImportBiasPercent =
        ResourceTradePricing::GetEdictImportBiasPercent(
            SelectedResource.Type);

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceRows.size()); ++Index)
    {
        auto& Row = Widget.mResourceRows[static_cast<size_t>(Index)];
        auto Button = Row.Button.lock();
        auto Label = Row.Label.lock();
        auto Value = Row.Value.lock();
        const int EntryIndex = VisibleStartIndex + Index;

        if (EntryIndex < static_cast<int>(ResourceEntries.size()))
        {
            SetDetailRowData(
                Row,
                ResourceEntries[static_cast<size_t>(EntryIndex)].Name,
                FormatInteger(
                    ResourceEntries[static_cast<size_t>(EntryIndex)].
                        Resource->TotalStock),
                EntryIndex == SelectedResourceIndex);

            if (Button)
            {
                Button->SetEnable(true);
                Button->ButtonEnable(true);
            }
            if (Label)
                Label->SetEnable(true);
            if (Value)
                Value->SetEnable(true);
        }
        else
        {
            SetDetailRowData(Row, L"", L"", false);

            if (Button)
            {
                Button->SetEnable(false);
                Button->ButtonEnable(false);
            }
            if (Label)
            {
                Label->SetText(L"");
                Label->SetEnable(false);
            }
            if (Value)
            {
                Value->SetText(L"");
                Value->SetEnable(false);
            }
        }
    }

    if (auto Title = Widget.mResourceListTitle.lock())
        Title->SetText(L"자원 유형");
    if (auto Text = Widget.mResourceFilterText.lock())
        Text->SetText(L"실시간 집계");
    if (auto Title = Widget.mResourceProductionTitle.lock())
        Title->SetText(L"시장 가격 추세");
    if (auto Title = Widget.mResourceDistributionTitle.lock())
        Title->SetText(L"자원 흐름 단계");
    if (auto Text = Widget.mResourceDistributionFilterText.lock())
        Text->SetText(L"생산지 -> 창고 -> 소비지 -> 항구");
    if (auto Title = Widget.mResourceTrackingTitle.lock())
        Title->SetText(L"흐름 세부");
    if (auto Name = Widget.mResourceTrackingName.lock())
        Name->SetText(SelectedResourceEntry.Name.c_str());
    if (auto Value = Widget.mResourceTrackingValue.lock())
        Value->SetText(FormatInteger(SelectedResource.TotalStock).c_str());
    if (auto Text = Widget.mResourceProductionLegendPrimaryText.lock())
        Text->SetText(L"수출 단가");
    if (auto Text = Widget.mResourceProductionLegendSecondaryText.lock())
        Text->SetText(L"수입 단가");
    if (auto Swatch = Widget.mResourceProductionLegendPrimarySwatch.lock())
        Swatch->SetTint(0.22f, 0.58f, 0.82f, 0.92f);
    if (auto Swatch = Widget.mResourceProductionLegendSecondarySwatch.lock())
        Swatch->SetTint(0.84f, 0.62f, 0.18f, 0.92f);

    constexpr int PriceHistoryGroupCount = GResourceProductionBarCount / 2;
    std::array<int, PriceHistoryGroupCount> ExportHistory = {};
    std::array<int, PriceHistoryGroupCount> ImportHistory = {};
    int GraphMaxValue = 1;

    for (int Index = 0; Index < PriceHistoryGroupCount; ++Index)
    {
        ExportHistory[static_cast<size_t>(Index)] =
            ResourceTradePricing::GetExportPriceHistoryPoint(
                SelectedResource.Type,
                Index);
        ImportHistory[static_cast<size_t>(Index)] =
            ResourceTradePricing::GetImportPriceHistoryPoint(
                SelectedResource.Type,
                Index);
        GraphMaxValue = (std::max)(
            GraphMaxValue,
            (std::max)(
                ExportHistory[static_cast<size_t>(Index)],
                ImportHistory[static_cast<size_t>(Index)]));
    }

    const std::wstring ResourceXAxisLabels[GResourceProductionXAxisLabelCount] =
    {
        L"11일 전",
        L"7일 전",
        L"3일 전",
        L"오늘"
    };

    for (int Index = 0; Index < GResourceProductionXAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mResourceProductionXAxisLabels.size()))
            break;

        if (auto Label = Widget.mResourceProductionXAxisLabels[static_cast<size_t>(Index)].lock())
        {
            Label->SetEnable(true);
            Label->SetText(ResourceXAxisLabels[static_cast<size_t>(Index)].c_str());
        }
    }

    const std::wstring ResourceYAxisLabels[GResourceProductionYAxisLabelCount] =
    {
        FormatInteger(GraphMaxValue),
        FormatInteger(GraphMaxValue / 2),
        L"0"
    };
    for (int Index = 0; Index < GResourceProductionYAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mResourceProductionYAxisLabels.size()))
            break;

        if (auto Label = Widget.mResourceProductionYAxisLabels[static_cast<size_t>(Index)].lock())
        {
            Label->SetEnable(true);
            Label->SetText(ResourceYAxisLabels[Index].c_str());
        }
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceProductionBars.size()); ++Index)
    {
        if (auto Bar = Widget.mResourceProductionBars[static_cast<size_t>(Index)].lock())
            Bar->SetEnable(false);
    }

    if (auto Frame = Widget.mResourceProductionFrame.lock())
    {
        const float GraphLeft = Frame->GetPos().x + 22.f;
        const float GraphTop = Frame->GetPos().y + 14.f;
        const float GraphWidth = Frame->GetSize().x - 40.f;
        const float GraphHeight = Frame->GetSize().y - 32.f;
        const float GroupWidth =
            GraphWidth / static_cast<float>((std::max)(1, PriceHistoryGroupCount));
        const float BarWidth =
            (std::max)(4.f, GroupWidth * 0.26f);
        const float GroupInset =
            (GroupWidth - BarWidth * 2.f) * 0.5f;

        for (int Index = 0; Index < PriceHistoryGroupCount; ++Index)
        {
            const float ExportHeight =
                GraphHeight *
                Clamp01(
                    static_cast<float>(ExportHistory[static_cast<size_t>(Index)]) /
                    static_cast<float>(GraphMaxValue));
            const float ImportHeight =
                GraphHeight *
                Clamp01(
                    static_cast<float>(ImportHistory[static_cast<size_t>(Index)]) /
                    static_cast<float>(GraphMaxValue));
            const float GroupX =
                GraphLeft + GroupWidth * static_cast<float>(Index);
            const int ExportBarIndex = Index * 2;
            const int ImportBarIndex = ExportBarIndex + 1;

            if (ExportBarIndex < static_cast<int>(Widget.mResourceProductionBars.size()))
            {
                if (auto Bar = Widget.mResourceProductionBars[
                        static_cast<size_t>(ExportBarIndex)].lock())
                {
                    if (ExportHeight <= 0.f)
                    {
                        Bar->SetEnable(false);
                    }
                    else
                    {
                        Bar->SetTint(FVector4(0.22f, 0.58f, 0.82f, 0.92f));
                        Bar->SetEnable(true);
                        Bar->SetPos(
                            GroupX + GroupInset,
                            GraphTop + GraphHeight - ExportHeight);
                        Bar->SetSize(BarWidth, (std::max)(2.f, ExportHeight));
                    }
                }
            }

            if (ImportBarIndex < static_cast<int>(Widget.mResourceProductionBars.size()))
            {
                if (auto Bar = Widget.mResourceProductionBars[
                        static_cast<size_t>(ImportBarIndex)].lock())
                {
                    if (ImportHeight <= 0.f)
                    {
                        Bar->SetEnable(false);
                    }
                    else
                    {
                        Bar->SetTint(FVector4(0.84f, 0.62f, 0.18f, 0.92f));
                        Bar->SetEnable(true);
                        Bar->SetPos(
                            GroupX + GroupInset + BarWidth,
                            GraphTop + GraphHeight - ImportHeight);
                        Bar->SetSize(BarWidth, (std::max)(2.f, ImportHeight));
                    }
                }
            }
        }
    }

    const int ProducerFlowValue =
        (std::max)(0, SelectedResource.ProducerAvailableStock);
    const int WarehouseFlowValue =
        (std::max)(0, SelectedResource.WarehouseBufferedStock);
    const int ConsumerFlowValue =
        (std::max)(
            0,
            SelectedResource.ShortagePressure > 0 ?
                SelectedResource.ShortagePressure :
                SelectedResource.ConsumerCoveredStock);
    const int HarborFlowValue =
        (std::max)(0, SelectedResource.HarborExportableStock);
    const int FlowStageMaxValue =
        (std::max)(
            1,
            (std::max)(
                (std::max)(ProducerFlowValue, WarehouseFlowValue),
                (std::max)(ConsumerFlowValue, HarborFlowValue)));

    const struct FResourceDistributionRow
    {
        const wchar_t* Label;
        std::wstring Value;
        float Percent;
        FVector4 Tint;
    } DistributionRows[GResourceDistributionRowCount] =
    {
        {
            L"생산지",
            L"대기 " +
                FormatInteger(SelectedResource.ProducerAvailableStock) +
                L" / 건물 " +
                FormatInteger(SelectedResource.ProducerBuildingCount) +
                L" / 대표 " +
                BuildFlowStageHeadline(
                    SelectedResource.TopProducerBuildings,
                    L"-"),
            static_cast<float>(Clamp01(
                static_cast<double>(ProducerFlowValue) /
                static_cast<double>(FlowStageMaxValue))),
            FVector4(0.22f, 0.58f, 0.82f, 0.95f)
        },
        {
            L"창고",
            L"보관 " +
                FormatInteger(SelectedResource.WarehouseBufferedStock) +
                L" / 창고 " +
                FormatInteger(SelectedResource.WarehouseBuildingCount) +
                L" / 대표 " +
                BuildFlowStageHeadline(
                    SelectedResource.TopWarehouseBuildings,
                    L"-"),
            static_cast<float>(Clamp01(
                static_cast<double>(WarehouseFlowValue) /
                static_cast<double>(FlowStageMaxValue))),
            FVector4(0.42f, 0.66f, 0.32f, 0.95f)
        },
        {
            L"소비지",
            (SelectedResource.ShortagePressure > 0 ?
                (L"부족 " +
                    FormatInteger(SelectedResource.ShortagePressure)) :
                (L"보급 " +
                    FormatInteger(SelectedResource.ConsumerCoveredStock))) +
                L" / 소비처 " +
                FormatInteger(SelectedResource.ConsumerBuildingCount) +
                L" / 대표 " +
                BuildFlowStageHeadline(
                    SelectedResource.ShortagePressure > 0 ?
                        SelectedResource.TopShortageBuildings :
                        SelectedResource.TopConsumerBuildings,
                    L"-"),
            static_cast<float>(Clamp01(
                static_cast<double>(ConsumerFlowValue) /
                static_cast<double>(FlowStageMaxValue))),
            SelectedResource.ShortagePressure > 0 ?
                FVector4(0.82f, 0.38f, 0.28f, 0.95f) :
                FVector4(0.80f, 0.62f, 0.22f, 0.95f)
        },
        {
            L"항구",
            L"수출 가능 " +
                FormatInteger(SelectedResource.HarborExportableStock) +
                L" / 예약 " +
                FormatInteger(SelectedResource.HarborReservedPickup) +
                L" / 대표 " +
                BuildFlowStageHeadline(
                    SelectedResource.TopHarborBuildings,
                    L"-"),
            static_cast<float>(Clamp01(
                static_cast<double>(HarborFlowValue) /
                static_cast<double>(FlowStageMaxValue))),
            FVector4(0.66f, 0.48f, 0.84f, 0.95f)
        }
    };

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceDistributionRows.size()); ++Index)
    {
        if (Index >= GResourceDistributionRowCount)
            break;

        auto& Row = Widget.mResourceDistributionRows[static_cast<size_t>(Index)];
        SetMetricRowData(
            Row,
            DistributionRows[Index].Label,
            DistributionRows[Index].Value,
            DistributionRows[Index].Percent,
            DistributionRows[Index].Tint,
            false);

        if (auto Background = Row.Background.lock())
        {
            Background->SetTexture(
                Background->GetName() + "_flat",
                GRowTexture);
            Background->SetTint(1.f, 1.f, 1.f, 0.22f);
        }
        if (auto Label = Row.Label.lock())
            Label->SetTextColor(106, 98, 84, 255);
        if (auto Value = Row.Value.lock())
            Value->SetTextColor(126, 118, 102, 255);
        if (auto Bar = Row.Bar.lock())
        {
            Bar->SetEnable(true);
            Bar->SetTint(
                EProgressBarImageType::Back,
                FVector4(0.88f, 0.84f, 0.74f, 0.20f));
        }
    }

    const std::wstring ResourceTrackingValues[GResourceDetailCount] =
    {
        FormatInteger(SelectedResource.AvailableStock) +
            L" / " +
            FormatInteger(SelectedResource.ReservedPickup),
        FormatInteger(SelectedResource.ReservedIncoming) +
            L" / " +
            FormatInteger(SelectedResource.AvailableIncomingCapacity),
        FormatCurrency(ExportUnitPrice) +
            L" / " +
            FormatCurrency(ImportUnitPrice) +
            L" | 가치 " +
            FormatCompactCurrency(
                ResourceTradePricing::ComputeExportValue(
                    SelectedResource.Type,
                    SelectedResource.AvailableStock)),
        BuildStoragePressureText(StorageBiasPercent) +
            L" | " +
            BuildBalancePressureText(BalanceBiasPercent) +
            L" | 외교 수출 " +
            FormatSignedPercentValue(DiplomacyExportBiasPercent) +
            L" / 수입 " +
            FormatSignedPercentValue(DiplomacyImportBiasPercent) +
            L" | 칙령 수출 " +
            FormatSignedPercentValue(EdictExportBiasPercent) +
            L" / 수입 " +
            FormatSignedPercentValue(EdictImportBiasPercent) +
            L" | " +
            BuildEventPressureText(Snapshot, EventBiasPercent)
    };
    const wchar_t* ResourceTrackingLabels[GResourceDetailCount] =
    {
        L"사용 가능 / 픽업 예약",
        L"예약 입고 / 여유 용량",
        L"수출 / 수입 단가",
        L"시장 요인"
    };

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceDetails.size()); ++Index)
    {
        if (Index >= GResourceDetailCount)
            break;

        SetDetailRowData(
            Widget.mResourceDetails[static_cast<size_t>(Index)],
            ResourceTrackingLabels[Index],
            ResourceTrackingValues[Index],
            false,
            FVector4(0.31f, 0.27f, 0.21f, 1.f));

        if (auto Background = Widget.mResourceDetails[static_cast<size_t>(Index)].Background.lock())
        {
            Background->SetTexture(
                Background->GetName() + "_base",
                GRowTexture);
            Background->SetTint(
                Index == 2 ?
                    FVector4(0.86f, 0.86f, 0.84f, 0.76f) :
                    FVector4(1.f, 1.f, 1.f, 0.94f));
        }
        if (auto Label = Widget.mResourceDetails[static_cast<size_t>(Index)].Label.lock())
        {
            Label->SetTextColor(
                Index == 2 ? 116 : 92,
                Index == 2 ? 112 : 84,
                Index == 2 ? 104 : 66,
                255);
        }
        if (auto Value = Widget.mResourceDetails[static_cast<size_t>(Index)].Value.lock())
        {
            Value->SetTextColor(
                Index == 2 ? 116 : 92,
                Index == 2 ? 112 : 84,
                Index == 2 ? 104 : 66,
                255);
        }
    }

    if (auto Notice = Widget.mResourceNotice.lock())
    {
        const bool ShowNotice =
            SelectedResource.TotalStock <= 0 &&
            SelectedResource.ReservedIncoming <= 0 &&
            SelectedResource.ReservedPickup <= 0 &&
            SelectedResource.ProducerBuildingCount <= 0 &&
            SelectedResource.ConsumerBuildingCount <= 0;
        const std::wstring NoticeText =
            ShowNotice ?
                std::wstring(L"활성 자원 흐름이 없습니다.") :
                (BuildFlowStageNotice(SelectedResource) +
                    L"\n" +
                    L"경보: " +
                    BuildResourceLogisticsWarning(SelectedResource));
        Notice->SetEnable(true);
        Notice->SetText(NoticeText.c_str());
    }
}

