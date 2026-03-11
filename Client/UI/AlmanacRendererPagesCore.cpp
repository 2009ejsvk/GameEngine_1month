#include "AlmanacRenderer.h"
#include "AlmanacRendererCalc.h"
#include "AlmanacRendererInternal.h"
#include <algorithm>
#include <array>
#include <cmath>

using namespace AlmanacRendererCalc;

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
}

void FAlmanacRenderer::ApplyOverviewPage(CAlmanacWidget& Widget)
{
    if (Widget.mOverviewCards.size() >= GOverviewCardCount)
    {
        SetCardData(
            Widget.mOverviewCards[0],
            L"무주택자 시민",
            L"5",
            L"",
            false);
        SetCardData(
            Widget.mOverviewCards[1],
            L"실업자 시민",
            L"19",
            L"",
            false);
        SetCardData(
            Widget.mOverviewCards[2],
            L"종합 만족도",
            L"84",
            L"",
            false);
        SetCardData(
            Widget.mOverviewCards[3],
            L"직업",
            L"73",
            L"",
            false);
        SetCardData(
            Widget.mOverviewCards[4],
            L"잔고 (지난 12\n개월)",
            L"$407,270",
            L"",
            false);
        SetCardData(
            Widget.mOverviewCards[5],
            L"스위스 은행\n계좌",
            L"S$23,900",
            L"",
            false);
        SetCardData(
            Widget.mOverviewCards[6],
            L"자본주의자",
            L"100",
            L"(1,041)",
            false);
        SetCardData(
            Widget.mOverviewCards[7],
            L"실업가",
            L"55",
            L"(1,041)",
            false);
        SetCardData(
            Widget.mOverviewCards[8],
            L"반란군 위험",
            L"없음",
            L"",
            false);
        SetCardData(
            Widget.mOverviewCards[9],
            L"중국",
            L">100",
            L"",
            false);
        SetCardData(
            Widget.mOverviewCards[10],
            L"미국",
            L"64",
            L"",
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

        if (auto Value = Widget.mOverviewCards[6].Value.lock())
            Value->SetTextColor(210, 48, 34, 255);
        if (auto Value = Widget.mOverviewCards[7].Value.lock())
            Value->SetTextColor(210, 48, 34, 255);
        if (auto Value = Widget.mOverviewCards[8].Value.lock())
            Value->SetTextColor(54, 154, 54, 255);
        if (auto Value = Widget.mOverviewCards[9].Value.lock())
            Value->SetTextColor(54, 154, 54, 255);

        if (auto Icon = Widget.mOverviewCards[8].Icon.lock())
            Icon->SetTint(0.92f, 0.34f, 0.24f, 1.f);
    }

    if (auto Text = Widget.mOverviewElectionText.lock())
        Text->SetText(L"다음 선거\n1월, 2039");

    if (auto SummaryLeft = Widget.mOverviewSummaryLeft.lock())
        SummaryLeft->SetText(L"");

    if (auto SummaryRight = Widget.mOverviewSummaryRight.lock())
        SummaryRight->SetText(L"");

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
            GSatisfactionLabels[Index],
            std::to_wstring(static_cast<int>(std::round(SatisfactionValues[Index]))),
            static_cast<float>(Clamp01(SatisfactionValues[Index] / 100.0)),
            GetSatisfactionTint(Index),
            Index == SelectedSatisfactionIndex);
    }

    const FVector4 SatisfactionAccentTint =
        GetSatisfactionTint(SelectedSatisfactionIndex);
    const std::wstring SelectedSatisfactionLabel =
        GSatisfactionLabels[SelectedSatisfactionIndex];
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
            L"▷ 방",
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
            XLabel->SetText(GSatisfactionTrendLabels[Index]);
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
void FAlmanacRenderer::ApplyPopulationPage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    const int HousingVacancy =
        (std::max)(0, Snapshot.ResidentialCapacity - Snapshot.AssignedHomeCount);
    const int JobVacancy =
        (std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount);
    const double HousingOccupancyRate =
        Snapshot.ResidentialCapacity > 0 ?
        static_cast<double>(Snapshot.AssignedHomeCount) /
        static_cast<double>(Snapshot.ResidentialCapacity) : 0.0;
    const double HomelessRate =
        static_cast<double>(Snapshot.HomelessCount) /
        static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount));
    const int PopulationGrowth12M =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.084));
    const int PopulationDecline12M =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.058));
    const int HomelessFamilyCount =
        (std::max)(0, RoundToInt(
            static_cast<double>(
                (std::max)(0, Snapshot.ActiveCitizenCount - Snapshot.AssignedHomeCount)) /
            128.0));
    const int JoblessCitizenCount =
        (std::max)(0, RoundToInt(
            static_cast<double>(
                (std::max)(0, Snapshot.ActiveCitizenCount - Snapshot.AssignedJobCount)) /
            64.0));
    const int SpecialCitizenCount =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.008));
    const int RivalCitizenCount =
        SpecialCitizenCount <= 1 ?
            0 :
            (std::min)(
                SpecialCitizenCount - 1,
                (std::max)(1, RoundToInt(
                    static_cast<double>(SpecialCitizenCount) * 0.11)));
    const int FactionLeaderCount =
        (std::max)(0, SpecialCitizenCount - RivalCitizenCount);
    const int SelectedPopulationIndex =
        (std::max)(0,
            (std::min)(
                static_cast<int>(Widget.mPopulationDetails.size()) - 1,
                Widget.mSelectedPopulationIndex));
    const int PopulationGrowthSummary =
        (std::max)(1, RoundToInt(
            static_cast<double>(PopulationGrowth12M) / 5.0));
    const int PopulationDeclineSummary =
        (std::max)(1, RoundToInt(
            static_cast<double>(PopulationDecline12M) / 6.5));
    const int PopulationImmigrantGrowth =
        (std::max)(0, RoundToInt(
            static_cast<double>(PopulationGrowth12M) * 0.79));
    const int PopulationBirthGrowth =
        (std::max)(0, PopulationGrowth12M - PopulationImmigrantGrowth);
    const int PopulationChildCount =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.0955));
    const int PopulationAdultCount =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.8405));
    const int PopulationRetiredCount =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.0193));
    const int PopulationHousingVacancyPercent =
        RoundToInt(HousingOccupancyRate < 1.0 ?
            (1.0 - HousingOccupancyRate) * 100.0 : 0.0);
    const int PopulationHomelessPercent =
        RoundToInt(HomelessRate * 100.0);
    const int PopulationCurrentUnemploymentPercent =
        (std::max)(1, RoundToInt(
            static_cast<double>(JoblessCitizenCount) /
            static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount)) *
            120.0));
    const int PopulationCurrentJobOccupancyPercent =
        (std::max)(0, (std::min)(99, RoundToInt(
            static_cast<double>(Snapshot.AssignedJobCount) /
            static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount)) *
            100.0 + 4.0)));
    const std::array<int, 5> HomelessFamilyWealthBuckets =
        BuildHomelessFamilyWealthBuckets(
            HomelessFamilyCount,
            Snapshot.HomelessWealthCount);
    const std::array<int, 5> CitizenWealthBuckets =
        BuildCitizenWealthBuckets(
            Snapshot.ActiveCitizenCount,
            Snapshot.CitizenWealthCount);
    const int UnemployedUneducatedCount =
        (std::max)(0,
            Snapshot.UnemployedEducationCount[
                static_cast<int>(ECitizenEducationLevel::Uneducated)]);
    const int UnemployedHighSchoolCount =
        (std::max)(0,
            Snapshot.UnemployedEducationCount[
                static_cast<int>(ECitizenEducationLevel::HighSchool)]);
    const int UnemployedCollegeCount =
        (std::max)(0,
            Snapshot.UnemployedEducationCount[
                static_cast<int>(ECitizenEducationLevel::College)]);
    const int WorkVacancyUneducatedCount =
        (std::max)(0,
            Snapshot.WorkVacancyEducationCount[
                static_cast<int>(ECitizenEducationLevel::Uneducated)]);
    const int WorkVacancyHighSchoolCount =
        (std::max)(0,
            Snapshot.WorkVacancyEducationCount[
                static_cast<int>(ECitizenEducationLevel::HighSchool)]);
    const int WorkVacancyCollegeCount =
        (std::max)(0,
            Snapshot.WorkVacancyEducationCount[
                static_cast<int>(ECitizenEducationLevel::College)]);
    const int EducationUneducatedCount =
        (std::max)(0,
            Snapshot.EducationCount[
                static_cast<int>(ECitizenEducationLevel::Uneducated)]);
    const int EducationHighSchoolCount =
        (std::max)(0,
            Snapshot.EducationCount[
                static_cast<int>(ECitizenEducationLevel::HighSchool)]);
    const int EducationCollegeCount =
        (std::max)(0,
            Snapshot.EducationCount[
                static_cast<int>(ECitizenEducationLevel::College)]);
    const int OverallSatisfactionVeryLowCitizenCount =
        (std::max)(0, Snapshot.OverallSatisfactionCitizenCount[0]);
    const int OverallSatisfactionLowCitizenCount =
        (std::max)(0, Snapshot.OverallSatisfactionCitizenCount[1]);
    const int OverallSatisfactionMediumCitizenCount =
        (std::max)(0, Snapshot.OverallSatisfactionCitizenCount[2]);
    const int OverallSatisfactionHighCitizenCount =
        (std::max)(0, Snapshot.OverallSatisfactionCitizenCount[3]);
    const int OverallSatisfactionVeryHighCitizenCount =
        (std::max)(0, Snapshot.OverallSatisfactionCitizenCount[4]);
    std::array<int, 5> OverallSatisfactionMetricBuckets = {};
    const std::array<double, 8> OverallSatisfactionValues =
    {
        Snapshot.AverageFood,
        Snapshot.AverageHealth,
        Snapshot.AverageFun,
        Snapshot.AverageFaith,
        Snapshot.AverageHousing,
        Snapshot.AverageJob,
        Snapshot.AverageFreedom,
        Snapshot.AverageSecurity
    };
    for (double Value : OverallSatisfactionValues)
    {
        const int TierIndex = ResolvePopulationSatisfactionTier(Value);
        ++OverallSatisfactionMetricBuckets[static_cast<size_t>(TierIndex)];
    }
    const float CitizenBankruptChartEnd =
        static_cast<float>(CitizenWealthBuckets[0]);
    const float CitizenPoorChartEnd =
        static_cast<float>(CitizenWealthBuckets[1]);
    const float CitizenWellOffChartEnd =
        static_cast<float>(CitizenWealthBuckets[2]);
    const float CitizenRichChartEnd =
        static_cast<float>(CitizenWealthBuckets[3]);
    const float CitizenFilthyRichChartEnd =
        static_cast<float>(CitizenWealthBuckets[4]);
    const std::array<int, 5> ResidentialVacancyWealthBuckets =
    {
        Snapshot.ResidentialVacancyWealthCount[0],
        Snapshot.ResidentialVacancyWealthCount[1],
        Snapshot.ResidentialVacancyWealthCount[2],
        Snapshot.ResidentialVacancyWealthCount[3],
        Snapshot.ResidentialVacancyWealthCount[4]
    };
    const float HomelessBankruptChartEnd =
        static_cast<float>(HomelessFamilyWealthBuckets[0]) * 2.05f;
    const float HomelessPoorChartEnd =
        static_cast<float>(HomelessFamilyWealthBuckets[1]) * 1.70f;
    const float HomelessWellOffChartEnd =
        static_cast<float>(HomelessFamilyWealthBuckets[2]) * 1.55f;
    const float HomelessRichChartEnd =
        static_cast<float>(HomelessFamilyWealthBuckets[3]) * 2.12f;
    const float HomelessFilthyRichChartEnd =
        static_cast<float>(HomelessFamilyWealthBuckets[4]) * 1.42f;
    const std::array<float, GPopulationTrendPointCount> PopulationTrend =
        BuildPopulationTrend(
            Snapshot.ActiveCitizenCount,
            PopulationGrowth12M,
            PopulationDecline12M);
    const std::array<float, GPopulationChangeBarCount> PopulationGrowthBars =
        BuildPopulationChangeSeries(
            static_cast<float>(PopulationGrowthSummary) * 2.05f,
            true);
    const std::array<float, GPopulationChangeBarCount> PopulationDeclineBars =
        BuildPopulationChangeSeries(
            static_cast<float>(PopulationDeclineSummary) * 1.42f,
            false);
    const std::array<float, GPopulationChangeBarCount> PopulationImmigrantBars =
        BuildPopulationChangeSeries(
            (std::max)(12.f,
                static_cast<float>(PopulationImmigrantGrowth) / 4.0f),
            false);
    const std::array<float, GPopulationChangeBarCount> PopulationBirthBars =
        BuildPopulationChangeSeries(
            (std::max)(3.5f,
                static_cast<float>(PopulationBirthGrowth) / 5.0f),
            true);
    const std::array<float, GPopulationDistributionBarCount> PopulationChildBars =
        BuildPopulationDistributionSeries(
            static_cast<float>(PopulationChildCount),
            0.92f,
            1.0f);
    const std::array<float, GPopulationDistributionBarCount> PopulationAdultBars =
        BuildPopulationDistributionSeries(
            static_cast<float>(PopulationAdultCount),
            0.96f,
            1.0f);
    const std::array<float, GPopulationDistributionBarCount> PopulationRetiredBars =
        BuildPopulationDistributionSeries(
            static_cast<float>(PopulationRetiredCount),
            0.84f,
            1.0f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHousingVacancyTrend =
        BuildPopulationDetailTrend(15.f, 22.f, 2.1f, 1.4f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessTrend =
        BuildPopulationDetailTrend(0.0f, 1.2f, 0.30f, 0.15f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessBankruptBars =
        BuildPopulationHistoricalLayer(
            HomelessFamilyCount > 0 ?
                (std::max)(0.85f, HomelessBankruptChartEnd * 0.74f) :
                0.f,
            HomelessFamilyCount > 0 ?
                (std::max)(1.10f, HomelessBankruptChartEnd * 1.02f) :
                0.f,
            HomelessBankruptChartEnd,
            0.18f,
            0.10f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessPoorBars =
        BuildPopulationHistoricalLayer(
            HomelessFamilyCount > 0 ?
                (HomelessPoorChartEnd > 0.f ?
                    (std::max)(0.40f, HomelessPoorChartEnd * 0.60f) :
                    0.30f) :
                0.f,
            HomelessFamilyCount > 0 ?
                (HomelessPoorChartEnd > 0.f ?
                    (std::max)(0.58f, HomelessPoorChartEnd * 0.92f + 0.18f) :
                    0.46f) :
                0.f,
            HomelessPoorChartEnd,
            0.14f,
            0.08f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessWellOffBars =
        BuildPopulationHistoricalLayer(
            HomelessFamilyCount > 0 ?
                (HomelessWellOffChartEnd > 0.f ?
                    (std::max)(0.32f, HomelessWellOffChartEnd * 0.54f) :
                    0.18f) :
                0.f,
            HomelessFamilyCount > 0 ?
                (HomelessWellOffChartEnd > 0.f ?
                    (std::max)(0.52f, HomelessWellOffChartEnd * 1.22f + 0.18f) :
                    0.42f) :
                0.f,
            HomelessWellOffChartEnd,
            0.12f,
            0.08f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessRichBars =
        BuildPopulationHistoricalLayer(
            HomelessFamilyCount > 0 ?
                (std::max)(1.25f, HomelessRichChartEnd * 0.72f) :
                0.f,
            HomelessFamilyCount > 0 ?
                (std::max)(1.85f, HomelessRichChartEnd * 1.28f + 0.72f) :
                0.f,
            HomelessRichChartEnd,
            0.22f,
            0.14f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessFilthyRichBars =
        BuildPopulationHistoricalLayer(
            HomelessFamilyCount > 0 ?
                (HomelessFilthyRichChartEnd > 0.f ?
                    (std::max)(0.22f, HomelessFilthyRichChartEnd * 0.46f) :
                    0.0f) :
                0.f,
            HomelessFamilyCount > 0 ?
                (HomelessFilthyRichChartEnd > 0.f ?
                    (std::max)(0.36f, HomelessFilthyRichChartEnd * 0.90f + 0.10f) :
                    0.24f) :
                0.f,
            HomelessFilthyRichChartEnd,
            0.10f,
            0.06f);
    const std::array<float, GPopulationDistributionBarCount> PopulationVacantBankruptBars =
        BuildPopulationHistoricalLayer(
            ResidentialVacancyWealthBuckets[0] > 0 ?
                (std::max)(0.25f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[0]) * 0.78f) :
                0.f,
            ResidentialVacancyWealthBuckets[0] > 0 ?
                (std::max)(0.40f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[0]) * 1.05f) :
                0.f,
            static_cast<float>(ResidentialVacancyWealthBuckets[0]),
            0.10f,
            0.05f);
    const std::array<float, GPopulationDistributionBarCount> PopulationVacantPoorBars =
        BuildPopulationHistoricalLayer(
            ResidentialVacancyWealthBuckets[1] > 0 ?
                (std::max)(3.5f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[1]) * 1.12f) :
                0.f,
            ResidentialVacancyWealthBuckets[1] > 0 ?
                (std::max)(7.5f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[1]) * 1.45f + 1.0f) :
                0.f,
            static_cast<float>(ResidentialVacancyWealthBuckets[1]),
            0.75f,
            0.35f);
    const std::array<float, GPopulationDistributionBarCount> PopulationVacantWellOffBars =
        BuildPopulationHistoricalLayer(
            ResidentialVacancyWealthBuckets[2] > 0 ?
                (std::max)(6.5f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[2]) * 0.88f) :
                0.f,
            ResidentialVacancyWealthBuckets[2] > 0 ?
                (std::max)(12.0f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[2]) * 1.58f) :
                0.f,
            static_cast<float>(ResidentialVacancyWealthBuckets[2]),
            1.20f,
            0.55f);
    const std::array<float, GPopulationDistributionBarCount> PopulationVacantRichBars =
        BuildPopulationHistoricalLayer(
            ResidentialVacancyWealthBuckets[3] > 0 ?
                (std::max)(0.35f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[3]) * 0.64f) :
                0.f,
            ResidentialVacancyWealthBuckets[3] > 0 ?
                (std::max)(0.52f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[3]) * 1.20f) :
                0.f,
            static_cast<float>(ResidentialVacancyWealthBuckets[3]),
            0.12f,
            0.06f);
    const std::array<float, GPopulationDistributionBarCount> PopulationVacantFilthyRichBars =
        BuildPopulationHistoricalLayer(
            ResidentialVacancyWealthBuckets[4] > 0 ?
                (std::max)(0.15f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[4]) * 0.58f) :
                0.f,
            ResidentialVacancyWealthBuckets[4] > 0 ?
                (std::max)(0.25f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[4]) * 0.92f) :
                0.f,
            static_cast<float>(ResidentialVacancyWealthBuckets[4]),
            0.08f,
            0.04f);
    const std::array<float, GPopulationTrendPointCount> PopulationJobOccupancyTrend =
        BuildPopulationRateTrend(
            static_cast<float>((std::max)(62, PopulationCurrentJobOccupancyPercent - 5)),
            static_cast<float>(PopulationCurrentJobOccupancyPercent),
            3.0f,
            1.6f);
    const std::array<float, GPopulationTrendPointCount> PopulationJobUnemploymentTrend =
        BuildPopulationRateTrend(
            static_cast<float>(PopulationCurrentUnemploymentPercent),
            static_cast<float>(PopulationCurrentUnemploymentPercent),
            0.32f,
            0.12f);
    const std::array<float, GPopulationDistributionBarCount> PopulationUnemployedUneducatedBars =
        BuildPopulationHistoricalLayer(
            UnemployedUneducatedCount > 0 ?
                (std::max)(0.85f,
                    static_cast<float>(UnemployedUneducatedCount) * 0.86f) :
                0.f,
            UnemployedUneducatedCount > 0 ?
                (std::max)(1.15f,
                    static_cast<float>(UnemployedUneducatedCount) * 1.20f) :
                0.f,
            static_cast<float>(UnemployedUneducatedCount),
            0.20f,
            0.10f);
    const std::array<float, GPopulationDistributionBarCount> PopulationUnemployedHighSchoolBars =
        BuildPopulationHistoricalLayer(
            UnemployedHighSchoolCount > 0 ?
                (std::max)(0.60f,
                    static_cast<float>(UnemployedHighSchoolCount) * 0.80f) :
                0.f,
            UnemployedHighSchoolCount > 0 ?
                (std::max)(0.92f,
                    static_cast<float>(UnemployedHighSchoolCount) * 1.18f) :
                0.f,
            static_cast<float>(UnemployedHighSchoolCount),
            0.18f,
            0.08f);
    const std::array<float, GPopulationDistributionBarCount> PopulationUnemployedCollegeBars =
        BuildPopulationHistoricalLayer(
            UnemployedCollegeCount > 0 ?
                (std::max)(2.2f,
                    static_cast<float>(UnemployedCollegeCount) * 0.92f) :
                0.f,
            UnemployedCollegeCount > 0 ?
                (std::max)(4.2f,
                    static_cast<float>(UnemployedCollegeCount) * 1.70f) :
                0.f,
            static_cast<float>(UnemployedCollegeCount),
            0.90f,
            0.40f);
    const std::array<float, GPopulationDistributionBarCount> PopulationWorkVacancyBars =
        BuildPopulationHistoricalLayer(
            JobVacancy > 0 ?
                (std::max)(560.f,
                    static_cast<float>(JobVacancy) * 0.88f) :
                0.f,
            JobVacancy > 0 ?
                (std::max)(620.f,
                    static_cast<float>(JobVacancy) * 1.02f) :
                0.f,
            static_cast<float>(JobVacancy),
            18.f,
            10.f);
    const std::array<float, GPopulationDistributionBarCount> PopulationEducationUneducatedBars =
        BuildPopulationHistoricalLayer(
            EducationUneducatedCount > 0 ?
                (std::max)(72.f,
                    static_cast<float>(EducationUneducatedCount) * 0.90f) :
                0.f,
            EducationUneducatedCount > 0 ?
                (std::max)(96.f,
                    static_cast<float>(EducationUneducatedCount) * 0.99f) :
                0.f,
            static_cast<float>(EducationUneducatedCount),
            7.5f,
            3.6f);
    const std::array<float, GPopulationDistributionBarCount> PopulationEducationHighSchoolBars =
        BuildPopulationHistoricalLayer(
            EducationHighSchoolCount > 0 ?
                (std::max)(42.f,
                    static_cast<float>(EducationHighSchoolCount) * 0.86f) :
                0.f,
            EducationHighSchoolCount > 0 ?
                (std::max)(58.f,
                    static_cast<float>(EducationHighSchoolCount) * 1.02f) :
                0.f,
            static_cast<float>(EducationHighSchoolCount),
            5.4f,
            2.2f);
    const std::array<float, GPopulationDistributionBarCount> PopulationEducationCollegeBars =
        BuildPopulationHistoricalLayer(
            EducationCollegeCount > 0 ?
                (std::max)(18.f,
                    static_cast<float>(EducationCollegeCount) * 0.74f) :
                0.f,
            EducationCollegeCount > 0 ?
                (std::max)(32.f,
                    static_cast<float>(EducationCollegeCount) * 0.98f) :
                0.f,
            static_cast<float>(EducationCollegeCount),
            4.2f,
            1.8f);
    const std::array<float, GPopulationDistributionBarCount> PopulationCitizenBankruptBars =
        BuildPopulationHistoricalLayer(
            CitizenBankruptChartEnd > 0.f ?
                (std::max)(6.f, CitizenBankruptChartEnd * 0.84f) :
                0.f,
            CitizenBankruptChartEnd > 0.f ?
                (std::max)(10.f, CitizenBankruptChartEnd * 1.02f) :
                0.f,
            CitizenBankruptChartEnd,
            1.8f,
            0.7f);
    const std::array<float, GPopulationDistributionBarCount> PopulationCitizenPoorBars =
        BuildPopulationHistoricalLayer(
            CitizenPoorChartEnd > 0.f ?
                (std::max)(22.f, CitizenPoorChartEnd * 0.86f) :
                0.f,
            CitizenPoorChartEnd > 0.f ?
                (std::max)(35.f, CitizenPoorChartEnd * 1.04f) :
                0.f,
            CitizenPoorChartEnd,
            2.8f,
            1.1f);
    const std::array<float, GPopulationDistributionBarCount> PopulationCitizenWellOffBars =
        BuildPopulationHistoricalLayer(
            CitizenWellOffChartEnd > 0.f ?
                (std::max)(640.f, CitizenWellOffChartEnd * 0.96f) :
                0.f,
            CitizenWellOffChartEnd > 0.f ?
                (std::max)(710.f, CitizenWellOffChartEnd * 1.03f) :
                0.f,
            CitizenWellOffChartEnd,
            10.5f,
            4.4f);
    const std::array<float, GPopulationDistributionBarCount> PopulationCitizenRichBars =
        BuildPopulationHistoricalLayer(
            CitizenRichChartEnd > 0.f ?
                (std::max)(150.f, CitizenRichChartEnd * 0.88f) :
                0.f,
            CitizenRichChartEnd > 0.f ?
                (std::max)(184.f, CitizenRichChartEnd * 1.04f) :
                0.f,
            CitizenRichChartEnd,
            4.6f,
            1.8f);
    const std::array<float, GPopulationDistributionBarCount> PopulationCitizenFilthyRichBars =
        BuildPopulationHistoricalLayer(
            CitizenFilthyRichChartEnd > 0.f ?
                (std::max)(6.f, CitizenFilthyRichChartEnd * 0.82f) :
                0.f,
            CitizenFilthyRichChartEnd > 0.f ?
                (std::max)(10.f, CitizenFilthyRichChartEnd * 1.06f) :
                0.f,
            CitizenFilthyRichChartEnd,
            1.2f,
            0.5f);
    const std::array<float, GPopulationDistributionBarCount> PopulationOverallVeryLowBars =
        BuildPopulationHistoricalLayer(
            OverallSatisfactionVeryLowCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionVeryLowCitizenCount) * 1.20f) :
                0.f,
            OverallSatisfactionVeryLowCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionVeryLowCitizenCount) * 1.08f) :
                0.f,
            static_cast<float>(OverallSatisfactionVeryLowCitizenCount),
            2.4f,
            1.2f);
    const std::array<float, GPopulationDistributionBarCount> PopulationOverallLowBars =
        BuildPopulationHistoricalLayer(
            OverallSatisfactionLowCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionLowCitizenCount) * 1.14f) :
                0.f,
            OverallSatisfactionLowCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionLowCitizenCount) * 1.04f) :
                0.f,
            static_cast<float>(OverallSatisfactionLowCitizenCount),
            3.2f,
            1.6f);
    const std::array<float, GPopulationDistributionBarCount> PopulationOverallMediumBars =
        BuildPopulationHistoricalLayer(
            OverallSatisfactionMediumCitizenCount > 0 ?
                (std::max)(320.f,
                    static_cast<float>(OverallSatisfactionMediumCitizenCount) * 1.24f) :
                0.f,
            OverallSatisfactionMediumCitizenCount > 0 ?
                (std::max)(380.f,
                    static_cast<float>(OverallSatisfactionMediumCitizenCount) * 1.10f) :
                0.f,
            static_cast<float>(OverallSatisfactionMediumCitizenCount),
            8.8f,
            4.2f);
    const std::array<float, GPopulationDistributionBarCount> PopulationOverallHighBars =
        BuildPopulationHistoricalLayer(
            OverallSatisfactionHighCitizenCount > 0 ?
                (std::max)(260.f,
                    static_cast<float>(OverallSatisfactionHighCitizenCount) * 0.76f) :
                0.f,
            OverallSatisfactionHighCitizenCount > 0 ?
                (std::max)(340.f,
                    static_cast<float>(OverallSatisfactionHighCitizenCount) * 0.92f) :
                0.f,
            static_cast<float>(OverallSatisfactionHighCitizenCount),
            9.6f,
            4.6f);
    const std::array<float, GPopulationDistributionBarCount> PopulationOverallVeryHighBars =
        BuildPopulationHistoricalLayer(
            OverallSatisfactionVeryHighCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionVeryHighCitizenCount) * 0.62f) :
                0.f,
            OverallSatisfactionVeryHighCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionVeryHighCitizenCount) * 0.84f) :
                0.f,
            static_cast<float>(OverallSatisfactionVeryHighCitizenCount),
            2.0f,
            0.9f);

    SetDetailRowData(
        Widget.mPopulationDetails[0],
        L"인구",
        std::to_wstring(Snapshot.ActiveCitizenCount),
        SelectedPopulationIndex == 0);
    SetDetailRowData(
        Widget.mPopulationDetails[1],
        L"성장 (지난 12개월)",
        std::to_wstring(PopulationGrowth12M),
        SelectedPopulationIndex == 1);
    SetDetailRowData(
        Widget.mPopulationDetails[2],
        L"하락률 (지난 12개월)",
        std::to_wstring(PopulationDecline12M),
        SelectedPopulationIndex == 2);
    SetDetailRowData(
        Widget.mPopulationDetails[3],
        L"연령",
        L"",
        SelectedPopulationIndex == 3);
    SetDetailRowData(
        Widget.mPopulationDetails[4],
        L"주택 (점유 / 전체)",
        std::to_wstring(Snapshot.AssignedHomeCount) +
            L"/" + std::to_wstring(Snapshot.ResidentialCapacity),
        SelectedPopulationIndex == 4);
    SetDetailRowData(
        Widget.mPopulationDetails[5],
        L"무주택자 가족",
        std::to_wstring(HomelessFamilyCount),
        SelectedPopulationIndex == 5);
    SetDetailRowData(
        Widget.mPopulationDetails[6],
        L"빈방",
        std::to_wstring(HousingVacancy),
        SelectedPopulationIndex == 6);
    SetDetailRowData(
        Widget.mPopulationDetails[7],
        L"직업 (취업 / 전체)",
        std::to_wstring(Snapshot.AssignedJobCount) +
            L"/" + std::to_wstring(Snapshot.JobCapacity),
        SelectedPopulationIndex == 7);
    SetDetailRowData(
        Widget.mPopulationDetails[8],
        L"실업자",
        std::to_wstring(JoblessCitizenCount),
        SelectedPopulationIndex == 8);
    SetDetailRowData(
        Widget.mPopulationDetails[9],
        L"빈 일자리",
        std::to_wstring(JobVacancy),
        SelectedPopulationIndex == 9);
    SetDetailRowData(
        Widget.mPopulationDetails[10],
        L"교육",
        L"",
        SelectedPopulationIndex == 10);
    SetDetailRowData(
        Widget.mPopulationDetails[11],
        L"재산",
        L"",
        SelectedPopulationIndex == 11);
    SetDetailRowData(
        Widget.mPopulationDetails[12],
        L"종합 만족도",
        L"",
        SelectedPopulationIndex == 12);
    SetDetailRowData(
        Widget.mPopulationDetails[13],
        L"특별 시민",
        std::to_wstring(SpecialCitizenCount),
        SelectedPopulationIndex == 13);

    const bool ShowPopulationOverviewCharts =
        SelectedPopulationIndex != 1 &&
        SelectedPopulationIndex != 2 &&
        SelectedPopulationIndex != 3 &&
        SelectedPopulationIndex != 4 &&
        SelectedPopulationIndex != 5 &&
        SelectedPopulationIndex != 6 &&
        SelectedPopulationIndex != 7 &&
        SelectedPopulationIndex != 8 &&
        SelectedPopulationIndex != 9 &&
        SelectedPopulationIndex != 10 &&
        SelectedPopulationIndex != 11 &&
        SelectedPopulationIndex != 12 &&
        SelectedPopulationIndex != 13;
    const bool ShowPopulationChangeTitleSection =
        SelectedPopulationIndex != 3 &&
        SelectedPopulationIndex != 4 &&
        SelectedPopulationIndex != 7 &&
        SelectedPopulationIndex != 11 &&
        SelectedPopulationIndex != 12;

    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationChangeGridLines.size()); ++Index)
    {
        if (auto GridLine = Widget.mPopulationChangeGridLines[static_cast<size_t>(Index)].lock())
            GridLine->SetEnable(ShowPopulationOverviewCharts);
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationChangeXAxisLabels.size()); ++Index)
    {
        if (auto XLabel = Widget.mPopulationChangeXAxisLabels[static_cast<size_t>(Index)].lock())
            XLabel->SetEnable(ShowPopulationOverviewCharts);
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationChangeYAxisLabels.size()); ++Index)
    {
    if (auto YLabel = Widget.mPopulationChangeYAxisLabels[static_cast<size_t>(Index)].lock())
        YLabel->SetEnable(ShowPopulationOverviewCharts);
    }

    if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
        ChangeTitleBackground->SetEnable(ShowPopulationChangeTitleSection);
    if (auto ChangeFrame = Widget.mPopulationChangeFrame.lock())
        ChangeFrame->SetEnable(ShowPopulationOverviewCharts);
    if (auto ChangeYAxisLine = Widget.mPopulationChangeYAxisLine.lock())
        ChangeYAxisLine->SetEnable(ShowPopulationOverviewCharts);
    if (auto ChangeXAxisLine = Widget.mPopulationChangeXAxisLine.lock())
        ChangeXAxisLine->SetEnable(ShowPopulationOverviewCharts);
    if (auto ChangeYAxisArrow = Widget.mPopulationChangeYAxisArrow.lock())
        ChangeYAxisArrow->SetEnable(ShowPopulationOverviewCharts);
    if (auto ChangeXAxisArrow = Widget.mPopulationChangeXAxisArrow.lock())
        ChangeXAxisArrow->SetEnable(ShowPopulationOverviewCharts);
    if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        ChangeTitle->SetEnable(ShowPopulationChangeTitleSection);

    const auto SetPopulationMetricEnable =
        [](const CAlmanacWidget::FMetricRowWidgets& Row, bool Enable)
    {
        if (auto Background = Row.Background.lock())
            Background->SetEnable(Enable);
        if (auto Label = Row.Label.lock())
            Label->SetEnable(Enable);
        if (auto Value = Row.Value.lock())
            Value->SetEnable(Enable);
        if (auto Bar = Row.Bar.lock())
            Bar->SetEnable(false);
    };
    const auto SetPopulationTrendDistributionEnable =
        [&Widget](bool Enable)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()); ++Index)
        {
            if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                Bar->SetEnable(Enable);
        }
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()); ++Index)
        {
            if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                Bar->SetEnable(Enable);
        }
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()); ++Index)
        {
            if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                Bar->SetEnable(Enable);
        }
    };
    const auto SetPopulationTrendWealthLayerEnable =
        [&Widget](bool Enable)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendRichBars.size()); ++Index)
        {
            if (auto Bar = Widget.mPopulationTrendRichBars[static_cast<size_t>(Index)].lock())
                Bar->SetEnable(Enable);
        }
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendFilthyRichBars.size()); ++Index)
        {
            if (auto Bar = Widget.mPopulationTrendFilthyRichBars[static_cast<size_t>(Index)].lock())
                Bar->SetEnable(Enable);
        }
    };

    SetPopulationTrendDistributionEnable(false);
    SetPopulationTrendWealthLayerEnable(false);

    if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        TrendFrame->SetEnable(true);
    if (auto TrendYAxisLine = Widget.mPopulationTrendYAxisLine.lock())
        TrendYAxisLine->SetEnable(true);
    if (auto TrendXAxisLine = Widget.mPopulationTrendXAxisLine.lock())
        TrendXAxisLine->SetEnable(true);
    if (auto TrendYAxisArrow = Widget.mPopulationTrendYAxisArrow.lock())
        TrendYAxisArrow->SetEnable(true);
    if (auto TrendXAxisArrow = Widget.mPopulationTrendXAxisArrow.lock())
        TrendXAxisArrow->SetEnable(true);
    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendGridLines.size()); ++Index)
    {
        if (auto GridLine = Widget.mPopulationTrendGridLines[static_cast<size_t>(Index)].lock())
            GridLine->SetEnable(true);
    }
    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()); ++Index)
    {
        if (auto XLabel = Widget.mPopulationTrendXAxisLabels[static_cast<size_t>(Index)].lock())
            XLabel->SetEnable(true);
    }
    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()); ++Index)
    {
        if (auto YLabel = Widget.mPopulationTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            YLabel->SetEnable(true);
    }

    if (SelectedPopulationIndex == 1)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"성장",
            std::to_wstring(PopulationGrowth12M),
            0.f,
            FVector4(0.24f, 0.42f, 0.68f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"▷ 이민자",
            std::to_wstring(PopulationImmigrantGrowth),
            0.f,
            FVector4(0.72f, 0.24f, 0.20f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"▷ 출생",
            std::to_wstring(PopulationBirthGrowth),
            0.f,
            FVector4(0.24f, 0.42f, 0.68f, 0.94f),
            false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();
            auto Bar = Row.Bar.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 1 ?
                        FVector4(0.78f, 0.34f, 0.28f, 0.92f) :
                        FVector4(0.30f, 0.48f, 0.74f, 0.90f));
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index == 1 ? 104 : 52,
                    Index == 1 ? 42 : 60,
                    Index == 1 ? 34 : 74,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index == 1 ? 104 : 52,
                    Index == 1 ? 42 : 60,
                    Index == 1 ? 34 : 74,
                    255);
            }

            if (Bar)
                Bar->SetEnable(false);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"성장");
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"성장 요인");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 4)
                {
                    const int GrowthChartValues[4] = { 0, 10, 20, 30 };
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(GrowthChartValues[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationChangeBarCount));
            const float SingleBarWidth =
                (std::max)(3.f, BarGroupWidth * 0.30f);
            const float MaxValue = 34.f;

            for (int Index = 0; Index < GPopulationChangeBarCount; ++Index)
            {
                const float BaseX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index);

                if (Index < static_cast<int>(Widget.mPopulationChangeNegativeBars.size()))
                {
                    if (auto ImmigrantBar =
                        Widget.mPopulationChangeNegativeBars[static_cast<size_t>(Index)].lock())
                    {
                        const float Height =
                            GraphHeight *
                            Clamp01(PopulationImmigrantBars[static_cast<size_t>(Index)] / MaxValue);
                        ImmigrantBar->SetEnable(true);
                        ImmigrantBar->SetTint(0.78f, 0.34f, 0.28f, 0.92f);
                        ImmigrantBar->SetPos(
                            BaseX + BarGroupWidth * 0.36f,
                            GraphTop + GraphHeight - Height);
                        ImmigrantBar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationChangePositiveBars.size()))
                {
                    if (auto BirthBar =
                        Widget.mPopulationChangePositiveBars[static_cast<size_t>(Index)].lock())
                    {
                        const float Height =
                            GraphHeight *
                            Clamp01(PopulationBirthBars[static_cast<size_t>(Index)] / MaxValue);
                        BirthBar->SetEnable(true);
                        BirthBar->SetTint(0.28f, 0.48f, 0.82f, 0.94f);
                        BirthBar->SetPos(
                            BaseX + BarGroupWidth * 0.08f,
                            GraphTop + GraphHeight - Height);
                        BirthBar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                    }
                }
            }
        }

        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);
    }
    else if (SelectedPopulationIndex == 2)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"하락률",
            std::to_wstring(PopulationDecline12M),
            0.f,
            FVector4(0.24f, 0.42f, 0.68f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"▷ 사망",
            std::to_wstring(PopulationDecline12M),
            0.f,
            FVector4(0.24f, 0.42f, 0.68f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"해외이주자",
            L"0",
            0.f,
            FVector4(0.72f, 0.24f, 0.20f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"게릴라",
            L"0",
            0.f,
            FVector4(0.40f, 0.56f, 0.22f, 0.92f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 || Index == 1 ?
                        FVector4(0.30f, 0.48f, 0.74f, 0.90f) :
                    Index == 2 ?
                        FVector4(0.78f, 0.34f, 0.28f, 0.92f) :
                        FVector4(0.42f, 0.55f, 0.22f, 0.92f));
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index == 2 ? 104 : (Index == 3 ? 74 : 52),
                    Index == 2 ? 42 : (Index == 3 ? 74 : 60),
                    Index == 2 ? 34 : (Index == 3 ? 32 : 74),
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index == 2 ? 104 : (Index == 3 ? 74 : 52),
                    Index == 2 ? 42 : (Index == 3 ? 74 : 60),
                    Index == 2 ? 34 : (Index == 3 ? 32 : 74),
                    255);
            }
        }

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"하락률");
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"하락 요인");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 2)
                {
                    const int DeclineChartValues[2] = { 0, 10 };
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(DeclineChartValues[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationChangeBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.56f);
            const float MaxValue = 10.f;

            for (int Index = 0; Index < GPopulationChangeBarCount; ++Index)
            {
                if (Index < static_cast<int>(Widget.mPopulationChangeNegativeBars.size()))
                {
                    if (auto HiddenBar =
                        Widget.mPopulationChangeNegativeBars[static_cast<size_t>(Index)].lock())
                    {
                        HiddenBar->SetEnable(false);
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationChangePositiveBars.size()))
                {
                    if (auto DeclineBar =
                        Widget.mPopulationChangePositiveBars[static_cast<size_t>(Index)].lock())
                    {
                        const float ChartValue =
                            (std::min)(
                                10.f,
                                (std::max)(1.0f,
                                    PopulationDeclineBars[static_cast<size_t>(Index)] * 0.58f));
                        const float Height =
                            GraphHeight * Clamp01(ChartValue / MaxValue);
                        DeclineBar->SetEnable(true);
                        DeclineBar->SetTint(0.34f, 0.50f, 0.78f, 0.94f);
                        DeclineBar->SetPos(
                            GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                                (BarGroupWidth - SingleBarWidth) * 0.5f,
                            GraphTop + GraphHeight - Height);
                        DeclineBar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 3)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"▷ 아동",
            std::to_wstring(PopulationChildCount),
            0.f,
            FVector4(0.31f, 0.48f, 0.80f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"▷ 성인",
            std::to_wstring(PopulationAdultCount),
            0.f,
            FVector4(0.80f, 0.34f, 0.28f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"▷ 은퇴",
            std::to_wstring(PopulationRetiredCount),
            0.f,
            FVector4(0.50f, 0.64f, 0.24f, 0.92f),
            false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.31f, 0.48f, 0.80f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.80f, 0.34f, 0.28f, 0.92f) :
                        FVector4(0.50f, 0.64f, 0.24f, 0.92f));
            }

            if (Label)
                Label->SetTextColor(58, 56, 42, 255);

            if (Value)
                Value->SetTextColor(58, 56, 42, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"연령 분포");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(false);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
            ChangeTitle->SetEnable(false);

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        const int PopulationDistributionLabels[GPopulationTrendYAxisLabelCount] =
        {
            0, 210, 420, 630, 840, 1050
        };
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(std::to_wstring(PopulationDistributionLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.72f);
            const float MaxValue = 1260.f;

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float ChildHeight =
                    GraphHeight *
                    Clamp01(PopulationChildBars[static_cast<size_t>(Index)] / MaxValue);
                const float AdultHeight =
                    GraphHeight *
                    Clamp01(PopulationAdultBars[static_cast<size_t>(Index)] / MaxValue);
                const float RetiredHeight =
                    GraphHeight *
                    Clamp01(PopulationRetiredBars[static_cast<size_t>(Index)] / MaxValue);
                const float ChildTop =
                    GraphTop + GraphHeight - ChildHeight;
                const float AdultTop = ChildTop - AdultHeight;
                const float RetiredTop = AdultTop - RetiredHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(true);
                        Bar->SetPos(BarX, ChildTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, ChildHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(true);
                        Bar->SetPos(BarX, AdultTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, AdultHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(true);
                        Bar->SetPos(BarX, RetiredTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, RetiredHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 4)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"현재 주택 공실률",
            std::to_wstring(PopulationHousingVacancyPercent) + L"%",
            0.f,
            FVector4(0.78f, 0.34f, 0.28f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"현재 노숙자 비율",
            std::to_wstring(PopulationHomelessPercent) + L"%",
            0.f,
            FVector4(0.30f, 0.48f, 0.74f, 0.90f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"▷ 빈 방이 있는 주거 건물",
            std::to_wstring(Snapshot.ResidentialVacancyBuildingCount),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"무주택자 가족",
            L"0",
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                if (Index <= 1)
                {
                    Background->SetTexture(
                        Background->GetName() + "_summary",
                        GBarFillTexture);
                    Background->SetTint(
                        Index == 0 ?
                            FVector4(0.78f, 0.34f, 0.28f, 0.92f) :
                            FVector4(0.30f, 0.48f, 0.74f, 0.90f));
                }
                else
                {
                    ApplySelectableBackground(Background, false);
                }
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index <= 1 ? 58 : 76,
                    Index <= 1 ? 56 : 70,
                    Index <= 1 ? 42 : 60,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index <= 1 ? 58 : 76,
                    Index <= 1 ? 56 : 70,
                    Index <= 1 ? 42 : 60,
                    255);
            }
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"가구");
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
            ChangeTitle->SetEnable(false);

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        const int HousingChartLabels[4] = { 0, 10, 20, 30 };
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 4)
                {
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(HousingChartLabels[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;

            for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()); ++Index)
            {
                if (Index < GPopulationDistributionBarCount - 1)
                {
                    if (auto Segment = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        const float X0 =
                            GraphLeft +
                            GraphWidth *
                                static_cast<float>(Index) /
                                static_cast<float>(GPopulationDistributionBarCount - 1);
                        const float X1 =
                            GraphLeft +
                            GraphWidth *
                                static_cast<float>(Index + 1) /
                                static_cast<float>(GPopulationDistributionBarCount - 1);
                        const float Y0 =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                PopulationHousingVacancyTrend[static_cast<size_t>(Index)],
                                0.f,
                                30.f);
                        const float Y1 =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                PopulationHousingVacancyTrend[static_cast<size_t>(Index + 1)],
                                0.f,
                                30.f);
                        SetLineSegment(
                            Segment,
                            X0,
                            Y0,
                            X1,
                            Y1,
                            2.4f,
                            FVector4(0.82f, 0.34f, 0.28f, 0.94f));
                    }
                }
                else if (auto Segment = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                {
                    Segment->SetEnable(false);
                }
            }

            for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()); ++Index)
            {
                if (Index < GPopulationDistributionBarCount - 1)
                {
                    if (auto Segment = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        const float X0 =
                            GraphLeft +
                            GraphWidth *
                                static_cast<float>(Index) /
                                static_cast<float>(GPopulationDistributionBarCount - 1);
                        const float X1 =
                            GraphLeft +
                            GraphWidth *
                                static_cast<float>(Index + 1) /
                                static_cast<float>(GPopulationDistributionBarCount - 1);
                        const float Y0 =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                PopulationHomelessTrend[static_cast<size_t>(Index)],
                                0.f,
                                30.f);
                        const float Y1 =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                PopulationHomelessTrend[static_cast<size_t>(Index + 1)],
                                0.f,
                                30.f);
                        SetLineSegment(
                            Segment,
                            X0,
                            Y0,
                            X1,
                            Y1,
                            2.0f,
                            FVector4(0.30f, 0.48f, 0.74f, 0.94f));
                    }
                }
                else if (auto Segment = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                {
                    Segment->SetEnable(false);
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 5)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);
        SetPopulationTrendWealthLayerEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"파산",
            std::to_wstring(HomelessFamilyWealthBuckets[0]),
            0.f,
            FVector4(0.28f, 0.46f, 0.78f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"가난함",
            std::to_wstring(HomelessFamilyWealthBuckets[1]),
            0.f,
            FVector4(0.76f, 0.25f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"유복함",
            std::to_wstring(HomelessFamilyWealthBuckets[2]),
            0.f,
            FVector4(0.56f, 0.66f, 0.24f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"부유함",
            std::to_wstring(HomelessFamilyWealthBuckets[3]),
            0.f,
            FVector4(0.92f, 0.76f, 0.22f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[4],
            L"더럽게 부유함",
            std::to_wstring(HomelessFamilyWealthBuckets[4]),
            0.f,
            FVector4(0.58f, 0.30f, 0.66f, 0.94f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.28f, 0.46f, 0.78f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.76f, 0.25f, 0.22f, 0.92f) :
                    Index == 2 ?
                        FVector4(0.56f, 0.66f, 0.24f, 0.92f) :
                    Index == 3 ?
                        FVector4(0.92f, 0.76f, 0.22f, 0.94f) :
                        FVector4(0.58f, 0.30f, 0.66f, 0.94f));
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"무주택자 가족");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"무주택자 (재산 순)");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 3)
                {
                    const int HomelessChartLabels[3] = { 0, 10, 20 };
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(HomelessChartLabels[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.76f);
            const float MaxValue = 20.f;

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float BankruptHeight =
                    GraphHeight *
                    Clamp01(PopulationHomelessBankruptBars[static_cast<size_t>(Index)] / MaxValue);
                const float PoorHeight =
                    GraphHeight *
                    Clamp01(PopulationHomelessPoorBars[static_cast<size_t>(Index)] / MaxValue);
                const float WellOffHeight =
                    GraphHeight *
                    Clamp01(PopulationHomelessWellOffBars[static_cast<size_t>(Index)] / MaxValue);
                const float RichHeight =
                    GraphHeight *
                    Clamp01(PopulationHomelessRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float FilthyRichHeight =
                    GraphHeight *
                    Clamp01(PopulationHomelessFilthyRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float BankruptTop =
                    GraphTop + GraphHeight - BankruptHeight;
                const float PoorTop = BankruptTop - PoorHeight;
                const float WellOffTop = PoorTop - WellOffHeight;
                const float RichTop = WellOffTop - RichHeight;
                const float FilthyRichTop = RichTop - FilthyRichHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(BankruptHeight > 0.f);
                        Bar->SetPos(BarX, BankruptTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, BankruptHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(PoorHeight > 0.f);
                        Bar->SetPos(BarX, PoorTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, PoorHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(WellOffHeight > 0.f);
                        Bar->SetPos(BarX, WellOffTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, WellOffHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(RichHeight > 0.f);
                        Bar->SetPos(BarX, RichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, RichHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendFilthyRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendFilthyRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(FilthyRichHeight > 0.f);
                        Bar->SetPos(BarX, FilthyRichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, FilthyRichHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 6)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);
        SetPopulationTrendWealthLayerEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"파산",
            std::to_wstring(ResidentialVacancyWealthBuckets[0]),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"가난함",
            std::to_wstring(ResidentialVacancyWealthBuckets[1]),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"유복함",
            std::to_wstring(ResidentialVacancyWealthBuckets[2]),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"부유함",
            std::to_wstring(ResidentialVacancyWealthBuckets[3]),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[4],
            L"더럽게 부유함",
            std::to_wstring(ResidentialVacancyWealthBuckets[4]),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
                ApplySelectableBackground(Background, false);

            if (Label)
                Label->SetTextColor(76, 70, 60, 255);

            if (Value)
                Value->SetTextColor(76, 70, 60, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"빈 방");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"빈 주택 (재산 순)");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 4)
                {
                    const int VacancyChartLabels[4] = { 0, 10, 20, 30 };
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(VacancyChartLabels[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.76f);
            const float MaxValue = 30.f;

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float BankruptHeight =
                    GraphHeight *
                    Clamp01(PopulationVacantBankruptBars[static_cast<size_t>(Index)] / MaxValue);
                const float PoorHeight =
                    GraphHeight *
                    Clamp01(PopulationVacantPoorBars[static_cast<size_t>(Index)] / MaxValue);
                const float WellOffHeight =
                    GraphHeight *
                    Clamp01(PopulationVacantWellOffBars[static_cast<size_t>(Index)] / MaxValue);
                const float RichHeight =
                    GraphHeight *
                    Clamp01(PopulationVacantRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float FilthyRichHeight =
                    GraphHeight *
                    Clamp01(PopulationVacantFilthyRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float BankruptTop =
                    GraphTop + GraphHeight - BankruptHeight;
                const float PoorTop = BankruptTop - PoorHeight;
                const float WellOffTop = PoorTop - WellOffHeight;
                const float RichTop = WellOffTop - RichHeight;
                const float FilthyRichTop = RichTop - FilthyRichHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(BankruptHeight > 0.f);
                        Bar->SetPos(BarX, BankruptTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, BankruptHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(PoorHeight > 0.f);
                        Bar->SetPos(BarX, PoorTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, PoorHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(WellOffHeight > 0.f);
                        Bar->SetPos(BarX, WellOffTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, WellOffHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(RichHeight > 0.f);
                        Bar->SetPos(BarX, RichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, RichHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendFilthyRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendFilthyRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(FilthyRichHeight > 0.f);
                        Bar->SetPos(BarX, FilthyRichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, FilthyRichHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 7)
    {
        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"현재 실업률",
            std::to_wstring(PopulationCurrentUnemploymentPercent) + L"%",
            0.f,
            FVector4(0.76f, 0.25f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"현재 직장 점유율",
            std::to_wstring(PopulationCurrentJobOccupancyPercent) + L"%",
            0.f,
            FVector4(0.24f, 0.42f, 0.68f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"▷ 빈 일자리",
            std::to_wstring(JobVacancy),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"▷ 폐쇄된 직업",
            L"0",
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[4],
            L"▷ 실업자 시민",
            std::to_wstring(JoblessCitizenCount),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                if (Index <= 1)
                {
                    Background->SetTexture(
                        Background->GetName() + "_summary",
                        GBarFillTexture);
                    Background->SetTint(
                        Index == 0 ?
                            FVector4(0.76f, 0.25f, 0.22f, 0.92f) :
                            FVector4(0.24f, 0.42f, 0.68f, 0.94f));
                }
                else
                {
                    ApplySelectableBackground(Background, false);
                }
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index <= 1 ? 58 : 76,
                    Index <= 1 ? 56 : 70,
                    Index <= 1 ? 42 : 60,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index <= 1 ? 58 : 76,
                    Index <= 1 ? 56 : 70,
                    Index <= 1 ? 42 : 60,
                    255);
            }
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"일자리");
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
            ChangeTitle->SetEnable(false);

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                const int JobChartLabels[GPopulationTrendYAxisLabelCount] =
                {
                    0, 20, 40, 60, 80, 100
                };
                YLabel->SetEnable(true);
                YLabel->SetText(
                    (std::to_wstring(JobChartLabels[Index]) + L"%").c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;

            for (int SegmentIndex = 0;
                SegmentIndex < GPopulationTrendSegmentCount;
                ++SegmentIndex)
            {
                if (SegmentIndex < static_cast<int>(Widget.mPopulationTrendLines.size()))
                {
                    const float X0 =
                        GraphLeft +
                        GraphWidth *
                            static_cast<float>(SegmentIndex) /
                            static_cast<float>(GPopulationTrendSegmentCount);
                    const float X1 =
                        GraphLeft +
                        GraphWidth *
                            static_cast<float>(SegmentIndex + 1) /
                            static_cast<float>(GPopulationTrendSegmentCount);
                    const float Y0 =
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            PopulationJobOccupancyTrend[static_cast<size_t>(SegmentIndex)],
                            0.f,
                            100.f);
                    const float Y1 =
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            PopulationJobOccupancyTrend[static_cast<size_t>(SegmentIndex + 1)],
                            0.f,
                            100.f);
                    SetLineSegment(
                        Widget.mPopulationTrendLines[static_cast<size_t>(SegmentIndex)].lock(),
                        X0,
                        Y0,
                        X1,
                        Y1,
                        2.6f,
                        FVector4(0.24f, 0.42f, 0.68f, 0.94f));
                }
            }

            for (int SegmentIndex = 0;
                SegmentIndex < static_cast<int>(Widget.mPopulationTrendChildBars.size());
                ++SegmentIndex)
            {
                if (SegmentIndex < GPopulationTrendSegmentCount)
                {
                    const float X0 =
                        GraphLeft +
                        GraphWidth *
                            static_cast<float>(SegmentIndex) /
                            static_cast<float>(GPopulationTrendSegmentCount);
                    const float X1 =
                        GraphLeft +
                        GraphWidth *
                            static_cast<float>(SegmentIndex + 1) /
                            static_cast<float>(GPopulationTrendSegmentCount);
                    const float Y0 =
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            PopulationJobUnemploymentTrend[static_cast<size_t>(SegmentIndex)],
                            0.f,
                            100.f);
                    const float Y1 =
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            PopulationJobUnemploymentTrend[static_cast<size_t>(SegmentIndex + 1)],
                            0.f,
                            100.f);
                    SetLineSegment(
                        Widget.mPopulationTrendChildBars[static_cast<size_t>(SegmentIndex)].lock(),
                        X0,
                        Y0,
                        X1,
                        Y1,
                        2.1f,
                        FVector4(0.78f, 0.26f, 0.22f, 0.92f));
                }
                else if (auto Segment = Widget.mPopulationTrendChildBars[static_cast<size_t>(SegmentIndex)].lock())
                {
                    Segment->SetEnable(false);
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 8)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"무학력",
            std::to_wstring(UnemployedUneducatedCount),
            0.f,
            FVector4(0.30f, 0.48f, 0.78f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"고등학교",
            std::to_wstring(UnemployedHighSchoolCount),
            0.f,
            FVector4(0.78f, 0.26f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"대학교",
            std::to_wstring(UnemployedCollegeCount),
            0.f,
            FVector4(0.56f, 0.68f, 0.24f, 0.92f),
            false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);
        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.30f, 0.48f, 0.78f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.78f, 0.26f, 0.22f, 0.92f) :
                        FVector4(0.56f, 0.68f, 0.24f, 0.92f));
            }

            if (Label)
                Label->SetTextColor(58, 56, 42, 255);

            if (Value)
                Value->SetTextColor(58, 56, 42, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"실업자");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"학력별 실업자");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 3)
                {
                    const int UnemploymentChartLabels[3] = { 0, 10, 20 };
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(UnemploymentChartLabels[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.76f);
            const float MaxValue = 20.f;

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float UneducatedHeight =
                    GraphHeight *
                    Clamp01(PopulationUnemployedUneducatedBars[static_cast<size_t>(Index)] / MaxValue);
                const float HighSchoolHeight =
                    GraphHeight *
                    Clamp01(PopulationUnemployedHighSchoolBars[static_cast<size_t>(Index)] / MaxValue);
                const float CollegeHeight =
                    GraphHeight *
                    Clamp01(PopulationUnemployedCollegeBars[static_cast<size_t>(Index)] / MaxValue);
                const float UneducatedTop =
                    GraphTop + GraphHeight - UneducatedHeight;
                const float HighSchoolTop = UneducatedTop - HighSchoolHeight;
                const float CollegeTop = HighSchoolTop - CollegeHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(UneducatedHeight > 0.f);
                        Bar->SetPos(BarX, UneducatedTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, UneducatedHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(HighSchoolHeight > 0.f);
                        Bar->SetPos(BarX, HighSchoolTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, HighSchoolHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(CollegeHeight > 0.f);
                        Bar->SetPos(BarX, CollegeTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, CollegeHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 9)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"▷ 무학력",
            std::to_wstring(WorkVacancyUneducatedCount),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"▷ 고등학교",
            std::to_wstring(WorkVacancyHighSchoolCount),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"▷ 대학교",
            std::to_wstring(WorkVacancyCollegeCount),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);
        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
                ApplySelectableBackground(Background, false);

            if (Label)
                Label->SetTextColor(76, 70, 60, 255);

            if (Value)
                Value->SetTextColor(76, 70, 60, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"빈 일자리");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"학력별 빈 일자리");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        const int WorkVacancyChartLabels[GPopulationTrendYAxisLabelCount] =
        {
            0, 150, 300, 450, 600, 750
        };
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(
                    std::to_wstring(WorkVacancyChartLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.72f);
            const float MaxValue = 750.f;

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float Height =
                    GraphHeight *
                    Clamp01(PopulationWorkVacancyBars[static_cast<size_t>(Index)] / MaxValue);

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(true);
                        Bar->SetTint(0.30f, 0.48f, 0.78f, 0.94f);
                        Bar->SetPos(BarX, GraphTop + GraphHeight - Height);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 10)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"무학력",
            std::to_wstring(EducationUneducatedCount),
            0.f,
            FVector4(0.30f, 0.48f, 0.78f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"고등학교",
            std::to_wstring(EducationHighSchoolCount),
            0.f,
            FVector4(0.78f, 0.26f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"대학교",
            std::to_wstring(EducationCollegeCount),
            0.f,
            FVector4(0.56f, 0.68f, 0.24f, 0.92f),
            false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);
        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.30f, 0.48f, 0.78f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.78f, 0.26f, 0.22f, 0.92f) :
                        FVector4(0.56f, 0.68f, 0.24f, 0.92f));
            }

            if (Label)
                Label->SetTextColor(58, 56, 42, 255);

            if (Value)
                Value->SetTextColor(58, 56, 42, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"교육");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"학교 건물");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        const int EducationChartMax =
            (std::max)(250,
                ((Snapshot.ActiveCitizenCount + 49) / 50) * 50);
        const int EducationChartStep =
            EducationChartMax / (GPopulationTrendYAxisLabelCount - 1);
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(
                    std::to_wstring(EducationChartStep * Index).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.72f);
            const float MaxValue = static_cast<float>(EducationChartMax);

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float UneducatedHeight =
                    GraphHeight *
                    Clamp01(PopulationEducationUneducatedBars[static_cast<size_t>(Index)] / MaxValue);
                const float HighSchoolHeight =
                    GraphHeight *
                    Clamp01(PopulationEducationHighSchoolBars[static_cast<size_t>(Index)] / MaxValue);
                const float CollegeHeight =
                    GraphHeight *
                    Clamp01(PopulationEducationCollegeBars[static_cast<size_t>(Index)] / MaxValue);
                const float UneducatedTop =
                    GraphTop + GraphHeight - UneducatedHeight;
                const float HighSchoolTop = UneducatedTop - HighSchoolHeight;
                const float CollegeTop = HighSchoolTop - CollegeHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(UneducatedHeight > 0.f);
                        Bar->SetTint(0.30f, 0.48f, 0.78f, 0.94f);
                        Bar->SetPos(BarX, UneducatedTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, UneducatedHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(HighSchoolHeight > 0.f);
                        Bar->SetTint(0.78f, 0.26f, 0.22f, 0.92f);
                        Bar->SetPos(BarX, HighSchoolTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, HighSchoolHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(CollegeHeight > 0.f);
                        Bar->SetTint(0.56f, 0.68f, 0.24f, 0.92f);
                        Bar->SetPos(BarX, CollegeTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, CollegeHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 11)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);
        SetPopulationTrendWealthLayerEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"파산",
            std::to_wstring(CitizenWealthBuckets[0]),
            0.f,
            FVector4(0.28f, 0.46f, 0.78f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"가난함",
            std::to_wstring(CitizenWealthBuckets[1]),
            0.f,
            FVector4(0.76f, 0.25f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"유복함",
            std::to_wstring(CitizenWealthBuckets[2]),
            0.f,
            FVector4(0.56f, 0.66f, 0.24f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"부유함",
            std::to_wstring(CitizenWealthBuckets[3]),
            0.f,
            FVector4(0.92f, 0.76f, 0.22f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[4],
            L"더럽게 부유함",
            std::to_wstring(CitizenWealthBuckets[4]),
            0.f,
            FVector4(0.58f, 0.30f, 0.66f, 0.94f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.28f, 0.46f, 0.78f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.76f, 0.25f, 0.22f, 0.92f) :
                    Index == 2 ?
                        FVector4(0.56f, 0.66f, 0.24f, 0.92f) :
                    Index == 3 ?
                        FVector4(0.92f, 0.76f, 0.22f, 0.94f) :
                        FVector4(0.58f, 0.30f, 0.66f, 0.94f));
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"재산");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(false);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
            ChangeTitle->SetEnable(false);

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        const int WealthChartMax =
            (std::max)(250,
                ((Snapshot.ActiveCitizenCount + 49) / 50) * 50);
        const int WealthChartStep =
            WealthChartMax / (GPopulationTrendYAxisLabelCount - 1);
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(
                    std::to_wstring(WealthChartStep * Index).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.76f);
            const float MaxValue = static_cast<float>(WealthChartMax);

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float BankruptHeight =
                    GraphHeight *
                    Clamp01(PopulationCitizenBankruptBars[static_cast<size_t>(Index)] / MaxValue);
                const float PoorHeight =
                    GraphHeight *
                    Clamp01(PopulationCitizenPoorBars[static_cast<size_t>(Index)] / MaxValue);
                const float WellOffHeight =
                    GraphHeight *
                    Clamp01(PopulationCitizenWellOffBars[static_cast<size_t>(Index)] / MaxValue);
                const float RichHeight =
                    GraphHeight *
                    Clamp01(PopulationCitizenRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float FilthyRichHeight =
                    GraphHeight *
                    Clamp01(PopulationCitizenFilthyRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float BankruptTop =
                    GraphTop + GraphHeight - BankruptHeight;
                const float PoorTop = BankruptTop - PoorHeight;
                const float WellOffTop = PoorTop - WellOffHeight;
                const float RichTop = WellOffTop - RichHeight;
                const float FilthyRichTop = RichTop - FilthyRichHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(BankruptHeight > 0.f);
                        Bar->SetTint(0.28f, 0.46f, 0.78f, 0.94f);
                        Bar->SetPos(BarX, BankruptTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, BankruptHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(PoorHeight > 0.f);
                        Bar->SetTint(0.76f, 0.25f, 0.22f, 0.92f);
                        Bar->SetPos(BarX, PoorTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, PoorHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(WellOffHeight > 0.f);
                        Bar->SetTint(0.56f, 0.66f, 0.24f, 0.92f);
                        Bar->SetPos(BarX, WellOffTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, WellOffHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(RichHeight > 0.f);
                        Bar->SetPos(BarX, RichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, RichHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendFilthyRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendFilthyRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(FilthyRichHeight > 0.f);
                        Bar->SetPos(BarX, FilthyRichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, FilthyRichHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 12)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);
        SetPopulationTrendWealthLayerEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"제일 낮음",
            std::to_wstring(OverallSatisfactionMetricBuckets[0]),
            0.f,
            FVector4(0.28f, 0.46f, 0.78f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"낮음",
            std::to_wstring(OverallSatisfactionMetricBuckets[1]),
            0.f,
            FVector4(0.76f, 0.25f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"중간",
            std::to_wstring(OverallSatisfactionMetricBuckets[2]),
            0.f,
            FVector4(0.56f, 0.66f, 0.24f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"높음",
            std::to_wstring(OverallSatisfactionMetricBuckets[3]),
            0.f,
            FVector4(0.92f, 0.76f, 0.22f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[4],
            L"제일 높음",
            std::to_wstring(OverallSatisfactionMetricBuckets[4]),
            0.f,
            FVector4(0.58f, 0.30f, 0.66f, 0.94f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.28f, 0.46f, 0.78f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.76f, 0.25f, 0.22f, 0.92f) :
                    Index == 2 ?
                        FVector4(0.56f, 0.66f, 0.24f, 0.92f) :
                    Index == 3 ?
                        FVector4(0.92f, 0.76f, 0.22f, 0.94f) :
                        FVector4(0.58f, 0.30f, 0.66f, 0.94f));
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"종합 만족도");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(false);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
            ChangeTitle->SetEnable(false);

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GSatisfactionTrendLabels[Index]);
            }
        }

        const int OverallSatisfactionChartMax =
            (std::max)(1260,
                ((Snapshot.ActiveCitizenCount + 209) / 210) * 210);
        const int OverallSatisfactionChartLabels[GPopulationTrendYAxisLabelCount] =
        {
            0, 210, 420, 630, 840, 1050
        };
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(
                    std::to_wstring(OverallSatisfactionChartLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.76f);
            const float MaxValue = static_cast<float>(OverallSatisfactionChartMax);

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float VeryLowHeight =
                    GraphHeight *
                    Clamp01(PopulationOverallVeryLowBars[static_cast<size_t>(Index)] / MaxValue);
                const float LowHeight =
                    GraphHeight *
                    Clamp01(PopulationOverallLowBars[static_cast<size_t>(Index)] / MaxValue);
                const float MediumHeight =
                    GraphHeight *
                    Clamp01(PopulationOverallMediumBars[static_cast<size_t>(Index)] / MaxValue);
                const float HighHeight =
                    GraphHeight *
                    Clamp01(PopulationOverallHighBars[static_cast<size_t>(Index)] / MaxValue);
                const float VeryHighHeight =
                    GraphHeight *
                    Clamp01(PopulationOverallVeryHighBars[static_cast<size_t>(Index)] / MaxValue);
                const float VeryLowTop =
                    GraphTop + GraphHeight - VeryLowHeight;
                const float LowTop = VeryLowTop - LowHeight;
                const float MediumTop = LowTop - MediumHeight;
                const float HighTop = MediumTop - HighHeight;
                const float VeryHighTop = HighTop - VeryHighHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(VeryLowHeight > 0.f);
                        Bar->SetTint(0.28f, 0.46f, 0.78f, 0.94f);
                        Bar->SetPos(BarX, VeryLowTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, VeryLowHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(LowHeight > 0.f);
                        Bar->SetTint(0.76f, 0.25f, 0.22f, 0.92f);
                        Bar->SetPos(BarX, LowTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, LowHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(MediumHeight > 0.f);
                        Bar->SetTint(0.56f, 0.66f, 0.24f, 0.92f);
                        Bar->SetPos(BarX, MediumTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, MediumHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(HighHeight > 0.f);
                        Bar->SetPos(BarX, HighTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, HighHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendFilthyRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendFilthyRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(VeryHighHeight > 0.f);
                        Bar->SetPos(BarX, VeryHighTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, VeryHighHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 13)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendGridLines.size()); ++Index)
        {
            if (auto GridLine = Widget.mPopulationTrendGridLines[static_cast<size_t>(Index)].lock())
                GridLine->SetEnable(false);
        }
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()); ++Index)
        {
            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[static_cast<size_t>(Index)].lock())
                XLabel->SetEnable(false);
        }
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()); ++Index)
        {
            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[static_cast<size_t>(Index)].lock())
                YLabel->SetEnable(false);
        }
        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
            TrendFrame->SetEnable(false);
        if (auto TrendYAxisLine = Widget.mPopulationTrendYAxisLine.lock())
            TrendYAxisLine->SetEnable(false);
        if (auto TrendXAxisLine = Widget.mPopulationTrendXAxisLine.lock())
            TrendXAxisLine->SetEnable(false);
        if (auto TrendYAxisArrow = Widget.mPopulationTrendYAxisArrow.lock())
            TrendYAxisArrow->SetEnable(false);
        if (auto TrendXAxisArrow = Widget.mPopulationTrendXAxisArrow.lock())
            TrendXAxisArrow->SetEnable(false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(false);
        SetPopulationTrendWealthLayerEnable(false);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"세력 지도자",
            std::to_wstring(FactionLeaderCount),
            0.f,
            FVector4(0.76f, 0.31f, 0.28f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"경쟁자",
            std::to_wstring(RivalCitizenCount),
            0.f,
            FVector4(0.26f, 0.78f, 0.80f, 0.94f),
            false);

        if (Widget.mPopulationMetrics.size() > 0)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[0], true);
        if (Widget.mPopulationMetrics.size() > 1)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[1], true);
        if (Widget.mPopulationMetrics.size() > 2)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[2], false);
        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < 2 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.76f, 0.31f, 0.28f, 0.94f) :
                        FVector4(0.26f, 0.78f, 0.80f, 0.94f));
            }

            if (Label)
                Label->SetTextColor(58, 56, 42, 255);

            if (Value)
                Value->SetTextColor(58, 56, 42, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"특별 시민");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"이름이 바뀐 사람");
        }

        std::vector<std::weak_ptr<CImage>> PieSegments;
        PieSegments.reserve(
            Widget.mPopulationTrendLines.size() +
            Widget.mPopulationTrendChildBars.size() +
            Widget.mPopulationTrendAdultBars.size() +
            Widget.mPopulationTrendRetiredBars.size() +
            Widget.mPopulationTrendRichBars.size() +
            Widget.mPopulationTrendFilthyRichBars.size());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendLines.begin(),
            Widget.mPopulationTrendLines.end());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendChildBars.begin(),
            Widget.mPopulationTrendChildBars.end());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendAdultBars.begin(),
            Widget.mPopulationTrendAdultBars.end());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendRetiredBars.begin(),
            Widget.mPopulationTrendRetiredBars.end());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendRichBars.begin(),
            Widget.mPopulationTrendRichBars.end());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendFilthyRichBars.begin(),
            Widget.mPopulationTrendFilthyRichBars.end());

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float FrameLeft = TrendFrame->GetPos().x;
            const float FrameTop = TrendFrame->GetPos().y;
            const float FrameWidth = TrendFrame->GetSize().x;
            const float FrameHeight = TrendFrame->GetSize().y;
            const float CenterX = FrameLeft + FrameWidth * 0.50f;
            const float CenterY = FrameTop + FrameHeight * 0.54f;
            const float Radius = (std::min)(FrameWidth, FrameHeight) * 0.34f;
            const int SegmentCount =
                (std::max)(24, static_cast<int>(PieSegments.size()));
            const int RivalSegmentCount =
                SpecialCitizenCount > 0 ?
                    (std::max)(0, (std::min)(
                        SegmentCount,
                        RoundToInt(
                            static_cast<double>(SegmentCount) *
                            static_cast<double>(RivalCitizenCount) /
                            static_cast<double>(SpecialCitizenCount)))) :
                    0;
            const float SegmentThickness =
                (std::max)(4.4f,
                    6.28318530718f * Radius /
                    static_cast<float>((std::max)(1, SegmentCount)));
            const float StartAngleRadians =
                -130.f * 3.14159265358979323846f / 180.f;
            const float AngleStepRadians =
                6.28318530718f /
                static_cast<float>((std::max)(1, SegmentCount));

            if (SpecialCitizenCount <= 0)
            {
                for (int Index = 0; Index < static_cast<int>(PieSegments.size()); ++Index)
                {
                    if (auto Segment = PieSegments[static_cast<size_t>(Index)].lock())
                        Segment->SetEnable(false);
                }
            }
            else
            {
                for (int Index = 0; Index < static_cast<int>(PieSegments.size()); ++Index)
                {
                    auto Segment = PieSegments[static_cast<size_t>(Index)].lock();
                    if (!Segment)
                        continue;

                    if (Index >= SegmentCount)
                    {
                        Segment->SetEnable(false);
                        continue;
                    }

                    const float Angle =
                        StartAngleRadians +
                        AngleStepRadians * static_cast<float>(Index);
                    const float EndX =
                        CenterX + std::cos(Angle) * Radius;
                    const float EndY =
                        CenterY + std::sin(Angle) * Radius;
                    const FVector4 Tint =
                        Index < RivalSegmentCount ?
                            FVector4(0.26f, 0.78f, 0.80f, 0.94f) :
                            FVector4(0.76f, 0.31f, 0.28f, 0.94f);
                    SetLineSegment(
                        Segment,
                        CenterX,
                        CenterY,
                        EndX,
                        EndY,
                        SegmentThickness,
                        Tint);
                }
            }
        }
    }
    else
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()); ++Index)
        {
            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[static_cast<size_t>(Index)].lock())
                YLabel->SetEnable(true);
        }

    SetMetricRowData(
        Widget.mPopulationMetrics[0],
        L"인구",
        std::to_wstring(Snapshot.ActiveCitizenCount),
        0.f,
        FVector4(0.24f, 0.42f, 0.68f, 0.94f),
        false);
    SetMetricRowData(
        Widget.mPopulationMetrics[1],
        L"성장",
        std::to_wstring(PopulationGrowthSummary),
        0.f,
        FVector4(0.24f, 0.42f, 0.68f, 0.94f),
        false);
    SetMetricRowData(
        Widget.mPopulationMetrics[2],
        L"하락률",
        std::to_wstring(PopulationDeclineSummary),
        0.f,
        FVector4(0.72f, 0.24f, 0.20f, 0.92f),
        false);

    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
    {
        auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
        auto Background = Row.Background.lock();
        auto Label = Row.Label.lock();
        auto Value = Row.Value.lock();
        auto Bar = Row.Bar.lock();

        if (Background)
        {
            Background->SetTexture(
                Background->GetName() + "_summary",
                GBarFillTexture);
            Background->SetTint(
                Index == 2 ?
                    FVector4(0.80f, 0.34f, 0.30f, 0.90f) :
                    FVector4(0.30f, 0.48f, 0.74f, 0.90f));
        }

        if (Label)
        {
            Label->SetTextColor(
                Index == 2 ? 96 : 54,
                Index == 2 ? 42 : 60,
                Index == 2 ? 38 : 74,
                255);
        }

        if (Value)
        {
            Value->SetTextColor(
                Index == 2 ? 96 : 54,
                Index == 2 ? 42 : 60,
                Index == 2 ? 38 : 74,
                255);
        }

        if (Bar)
            Bar->SetEnable(false);
    }

    if (Widget.mPopulationMetrics.size() > 0)
        SetPopulationMetricEnable(Widget.mPopulationMetrics[0], true);
    if (Widget.mPopulationMetrics.size() > 1)
        SetPopulationMetricEnable(Widget.mPopulationMetrics[1], true);
    if (Widget.mPopulationMetrics.size() > 2)
        SetPopulationMetricEnable(Widget.mPopulationMetrics[2], true);
    if (Widget.mPopulationMetrics.size() > 3)
        SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
    if (Widget.mPopulationMetrics.size() > 4)
        SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

    if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
        TrendTitle->SetText(L"인구");

    if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        ChangeTitle->SetEnable(true);

    for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
            break;

        if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            XLabel->SetText(GSatisfactionTrendLabels[Index]);
    }

    {
        const float MaxTrendValue =
            *std::max_element(PopulationTrend.begin(), PopulationTrend.end());
        const int PopulationTrendMax =
            (std::max)(1250,
                ((RoundToInt(MaxTrendValue) + 249) / 250) * 250);

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                const int Value =
                    (PopulationTrendMax / (GPopulationTrendYAxisLabelCount - 1)) * Index;
                YLabel->SetText(std::to_wstring(Value).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;

            for (int SegmentIndex = 0;
                SegmentIndex < GPopulationTrendSegmentCount;
                ++SegmentIndex)
            {
                if (SegmentIndex >= static_cast<int>(Widget.mPopulationTrendLines.size()))
                    break;

                const float X0 =
                    GraphLeft +
                    GraphWidth *
                        static_cast<float>(SegmentIndex) /
                        static_cast<float>(GPopulationTrendSegmentCount);
                const float X1 =
                    GraphLeft +
                    GraphWidth *
                        static_cast<float>(SegmentIndex + 1) /
                        static_cast<float>(GPopulationTrendSegmentCount);
                const float Y0 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        PopulationTrend[static_cast<size_t>(SegmentIndex)],
                        0.f,
                        static_cast<float>(PopulationTrendMax));
                const float Y1 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        PopulationTrend[static_cast<size_t>(SegmentIndex + 1)],
                        0.f,
                        static_cast<float>(PopulationTrendMax));
                SetLineSegment(
                    Widget.mPopulationTrendLines[static_cast<size_t>(SegmentIndex)].lock(),
                    X0,
                    Y0,
                    X1,
                    Y1,
                    2.6f,
                    FVector4(0.24f, 0.44f, 0.80f, 0.96f));
            }
        }
    }

    if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        ChangeTitle->SetText(L"인구 변화");

    for (int Index = 0; Index < GPopulationChangeXAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPopulationChangeXAxisLabels.size()))
            break;

        if (auto XLabel = Widget.mPopulationChangeXAxisLabels[Index].lock())
            XLabel->SetText(GSatisfactionTrendLabels[Index]);
    }

    {
        constexpr int PopulationChangeMin = -40;
        constexpr int PopulationChangeMax = 60;
        constexpr int PopulationChangeStep = 20;

        for (int Index = 0; Index < GPopulationChangeYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationChangeYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationChangeYAxisLabels[Index].lock())
            {
                const int Value =
                    PopulationChangeMin +
                    PopulationChangeStep * Index;
                YLabel->SetText(std::to_wstring(Value).c_str());
            }
        }

        if (auto ChangeFrame = Widget.mPopulationChangeFrame.lock())
        {
            const float GraphLeft = ChangeFrame->GetPos().x + 36.f;
            const float GraphTop = ChangeFrame->GetPos().y + 14.f;
            const float GraphWidth = ChangeFrame->GetSize().x - 54.f;
            const float GraphHeight = ChangeFrame->GetSize().y - 32.f;
            const float ZeroY =
                ResolveGraphYInRange(
                    GraphTop,
                    GraphHeight,
                    0.f,
                    static_cast<float>(PopulationChangeMin),
                    static_cast<float>(PopulationChangeMax));
            const float BarWidth =
                GraphWidth /
                    static_cast<float>((std::max)(1, GPopulationChangeBarCount)) * 0.64f;
            const float StepWidth =
                GraphWidth /
                    static_cast<float>((std::max)(1, GPopulationChangeBarCount));

            for (int Index = 0; Index < GPopulationChangeBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft +
                    StepWidth * static_cast<float>(Index) +
                    (StepWidth - BarWidth) * 0.5f;

                if (Index < static_cast<int>(Widget.mPopulationChangePositiveBars.size()))
                {
                    if (auto PositiveBar =
                        Widget.mPopulationChangePositiveBars[static_cast<size_t>(Index)].lock())
                    {
                        const float PositiveTop =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                PopulationGrowthBars[static_cast<size_t>(Index)],
                                static_cast<float>(PopulationChangeMin),
                                static_cast<float>(PopulationChangeMax));
                        PositiveBar->SetEnable(true);
                        PositiveBar->SetPos(BarX, PositiveTop);
                        PositiveBar->SetSize(BarWidth, (std::max)(2.f, ZeroY - PositiveTop));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationChangeNegativeBars.size()))
                {
                    if (auto NegativeBar =
                        Widget.mPopulationChangeNegativeBars[static_cast<size_t>(Index)].lock())
                    {
                        const float NegativeBottom =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                -PopulationDeclineBars[static_cast<size_t>(Index)],
                                static_cast<float>(PopulationChangeMin),
                                static_cast<float>(PopulationChangeMax));
                        NegativeBar->SetEnable(true);
                        NegativeBar->SetPos(BarX, ZeroY);
                        NegativeBar->SetSize(
                            BarWidth,
                            (std::max)(2.f, NegativeBottom - ZeroY));
                    }
                }
            }
        }
    }
    }

}

void FAlmanacRenderer::ApplyEconomyPage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    const int JobVacancy =
        (std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount);
    const int UnemployedUneducatedCount =
        (std::max)(0,
            Snapshot.UnemployedEducationCount[
                static_cast<int>(ECitizenEducationLevel::Uneducated)]);
    const int UnemployedHighSchoolCount =
        (std::max)(0,
            Snapshot.UnemployedEducationCount[
                static_cast<int>(ECitizenEducationLevel::HighSchool)]);
    const int UnemployedCollegeCount =
        (std::max)(0,
            Snapshot.UnemployedEducationCount[
                static_cast<int>(ECitizenEducationLevel::College)]);
    const int WorkVacancyUneducatedCount =
        (std::max)(0,
            Snapshot.WorkVacancyEducationCount[
                static_cast<int>(ECitizenEducationLevel::Uneducated)]);
    const int WorkVacancyHighSchoolCount =
        (std::max)(0,
            Snapshot.WorkVacancyEducationCount[
                static_cast<int>(ECitizenEducationLevel::HighSchool)]);
    const int WorkVacancyCollegeCount =
        (std::max)(0,
            Snapshot.WorkVacancyEducationCount[
                static_cast<int>(ECitizenEducationLevel::College)]);
    const std::array<float, GPopulationDistributionBarCount> PopulationUnemployedUneducatedBars =
        BuildPopulationHistoricalLayer(
            UnemployedUneducatedCount > 0 ?
                (std::max)(0.85f,
                    static_cast<float>(UnemployedUneducatedCount) * 0.86f) :
                0.f,
            UnemployedUneducatedCount > 0 ?
                (std::max)(1.15f,
                    static_cast<float>(UnemployedUneducatedCount) * 1.20f) :
                0.f,
            static_cast<float>(UnemployedUneducatedCount),
            0.20f,
            0.10f);
    const std::array<float, GPopulationDistributionBarCount> PopulationUnemployedHighSchoolBars =
        BuildPopulationHistoricalLayer(
            UnemployedHighSchoolCount > 0 ?
                (std::max)(0.60f,
                    static_cast<float>(UnemployedHighSchoolCount) * 0.80f) :
                0.f,
            UnemployedHighSchoolCount > 0 ?
                (std::max)(0.92f,
                    static_cast<float>(UnemployedHighSchoolCount) * 1.18f) :
                0.f,
            static_cast<float>(UnemployedHighSchoolCount),
            0.18f,
            0.08f);
    const std::array<float, GPopulationDistributionBarCount> PopulationUnemployedCollegeBars =
        BuildPopulationHistoricalLayer(
            UnemployedCollegeCount > 0 ?
                (std::max)(2.2f,
                    static_cast<float>(UnemployedCollegeCount) * 0.92f) :
                0.f,
            UnemployedCollegeCount > 0 ?
                (std::max)(4.2f,
                    static_cast<float>(UnemployedCollegeCount) * 1.70f) :
                0.f,
            static_cast<float>(UnemployedCollegeCount),
            0.90f,
            0.40f);
    const std::array<float, GPopulationDistributionBarCount> PopulationWorkVacancyBars =
        BuildPopulationHistoricalLayer(
            JobVacancy > 0 ?
                (std::max)(560.f,
                    static_cast<float>(JobVacancy) * 0.88f) :
                0.f,
            JobVacancy > 0 ?
                (std::max)(620.f,
                    static_cast<float>(JobVacancy) * 1.02f) :
                0.f,
            static_cast<float>(JobVacancy),
            18.f,
            10.f);
    const long long EconomyDailyIncome =
        Snapshot.DailyExportIncome + Snapshot.DailyTaxIncome;
    const long long EconomyDailyExpense =
        Snapshot.DailyEdictCost +
        Snapshot.DailyImportExpense +
        RoundToInt(
            (static_cast<double>(Snapshot.MonthlyWageCost) +
                static_cast<double>(Snapshot.MonthlyUpkeepCost)) / 30.0);
    const long long EconomyAnnualIncome =
        EconomyDailyIncome * 14LL;
    const long long EconomyAnnualExpense =
        EconomyDailyExpense * 11LL;
    const long long EconomyAnnualUpkeepExpense =
        Snapshot.MonthlyUpkeepCost * 12LL;
    const long long EconomyAnnualWageExpense =
        Snapshot.MonthlyWageCost * 12LL;
    const long long EconomyAnnualImportExpense =
        Snapshot.DailyImportExpense > 0 ?
            Snapshot.DailyImportExpense * 11LL :
            0LL;
    const long long EconomyAnnualCelebrityExpense = 0LL;
    const long long EconomyAnnualMiscExpense =
        Snapshot.DailyEdictCost > 0 ?
            Snapshot.DailyEdictCost * 365LL :
            (std::max)(0LL,
                static_cast<long long>(
                    RoundToInt(static_cast<double>(EconomyAnnualExpense) * 0.05)));
    const long long EconomyAnnualConstructionExpense =
        (std::max)(
            0LL,
            EconomyAnnualExpense -
                EconomyAnnualUpkeepExpense -
                EconomyAnnualWageExpense -
                EconomyAnnualImportExpense -
                EconomyAnnualMiscExpense -
                EconomyAnnualCelebrityExpense);
    const long long EconomyAnnualExportIncome =
        Snapshot.DailyExportIncome * 14LL;
    const long long EconomyAnnualLocalServiceIncome =
        Snapshot.DailyConsumptionTaxIncome * 14LL;
    const long long EconomyAnnualOtherIncome =
        Snapshot.DailyPropertyTaxIncome * 14LL;
    const long long EconomyAnnualAidIncome = 0LL;
    const long long EconomyAnnualTourismIncome =
        (std::max)(
            0LL,
            EconomyAnnualIncome -
                EconomyAnnualExportIncome -
                EconomyAnnualLocalServiceIncome -
                EconomyAnnualOtherIncome -
                EconomyAnnualAidIncome);
    const int CurrentTouristCount =
        Snapshot.TourismBuildingCount > 0 ?
            Snapshot.TourismBuildingCount * 360 +
                Snapshot.HarborCount * 140 + 267 :
            0;
    const int TourismRating =
        Snapshot.TourismBuildingCount > 0 ?
            (std::min)(99,
                90 + Snapshot.TourismBuildingCount + Snapshot.HarborCount) :
            0;
    const std::array<float, GPopulationDistributionBarCount> EconomyTreasuryBars =
        BuildPopulationHistoricalLayer(
            Snapshot.NationalBudget > 0 ?
                (std::max)(420000.f,
                    static_cast<float>(Snapshot.NationalBudget) * 0.42f) :
                0.f,
            Snapshot.NationalBudget > 0 ?
                (std::max)(680000.f,
                    static_cast<float>(Snapshot.NationalBudget) * 0.90f) :
                0.f,
            static_cast<float>((std::max)(0LL, Snapshot.NationalBudget)),
            26000.f,
            14000.f);
    const std::array<float, GPopulationDistributionBarCount> EconomyIncomeBars =
        BuildPopulationHistoricalLayer(
            EconomyDailyIncome > 0 ?
                (std::max)(48000.f,
                    static_cast<float>(EconomyDailyIncome) * 0.72f) :
                0.f,
            EconomyDailyIncome > 0 ?
                (std::max)(86000.f,
                    static_cast<float>(EconomyDailyIncome) * 1.08f) :
                0.f,
            static_cast<float>((std::max)(0LL, EconomyDailyIncome)),
            9000.f,
            4200.f);
    const std::array<float, GPopulationDistributionBarCount> EconomyExpenseBars =
        BuildPopulationHistoricalLayer(
            EconomyDailyExpense > 0 ?
                (std::max)(52000.f,
                    static_cast<float>(EconomyDailyExpense) * 0.78f) :
                0.f,
            EconomyDailyExpense > 0 ?
                (std::max)(98000.f,
                    static_cast<float>(EconomyDailyExpense) * 1.14f) :
                0.f,
            static_cast<float>((std::max)(0LL, EconomyDailyExpense)),
            11000.f,
            5200.f);
    const std::array<float, GEconomyTrendBarCount> CurrentTouristBars =
        BuildPopulationHistoricalLayer(
            CurrentTouristCount > 0 ?
                static_cast<float>(CurrentTouristCount) * 0.86f :
                0.f,
            CurrentTouristCount > 0 ?
                static_cast<float>(CurrentTouristCount) * 0.93f :
                0.f,
            static_cast<float>(CurrentTouristCount),
            42.f,
            18.f);
    const std::array<float, GEconomyTrendBarCount> TourismRatingTrend =
        BuildPopulationDetailTrend(
            (std::max)(90.f, static_cast<float>(TourismRating) - 1.4f),
            static_cast<float>(TourismRating),
            0.35f,
            0.16f);
    const std::array<float, GEconomyTrendBarCount> TourismCapacityTrend =
        BuildPopulationDetailTrend(
            22040.f,
            27550.f,
            220.f,
            120.f);
    const std::array<float, GEconomyTrendBarCount> TourismArrivalTrend =
        BuildPopulationDetailTrend(
            2550.f,
            static_cast<float>((std::max)(3000, CurrentTouristCount)),
            65.f,
            28.f);
    const std::array<float, GEconomyTrendBarCount> EconomyJobOccupancyTrend =
        BuildPopulationHistoricalLayer(
            76.f,
            86.f,
            80.f,
            1.8f,
            0.9f);
    const std::array<float, GEconomyTrendBarCount> EconomyUnemploymentTrend =
        BuildPopulationHistoricalLayer(
            0.6f,
            1.3f,
            1.0f,
            0.08f,
            0.05f);
    const long long EconomyTrendMaxRaw =
        (std::max)(
            Snapshot.NationalBudget,
            (std::max)(EconomyAnnualIncome, EconomyAnnualExpense));
    const int EconomyTrendStep =
        (std::max)(207000,
            ((RoundToInt(static_cast<double>(EconomyTrendMaxRaw) / 6.0) + 999) / 1000) *
                1000);
    const int EconomyTrendMax =
        EconomyTrendStep * GEconomyTrendYAxisLabelCount;
    const long long EconomyChangeMaxRaw =
        (std::max)(EconomyDailyIncome, EconomyDailyExpense);
    const int EconomyChangeStep =
        (std::max)(66250,
            ((RoundToInt(static_cast<double>(EconomyChangeMaxRaw) / 3.0) + 249) / 250) *
                250);
    const int EconomyChangeMax =
        EconomyChangeStep * 3;
    const int EconomyExpenseTrendStep =
        (std::max)(32500,
            ((RoundToInt(
                static_cast<double>(EconomyDailyExpense) * 1.6 / 5.0) + 249) / 250) *
                250);
    const int EconomyExpenseTrendMax =
        EconomyExpenseTrendStep * 5;
    const long long SwissBankAccountBalance = 2000LL;
    const int EconomyDetailMaxIndex =
        (std::max)(0, static_cast<int>(Widget.mEconomyDetails.size()) - 1);
    const int SelectedEconomyIndex =
        (std::max)(0, (std::min)(Widget.mSelectedEconomyIndex, EconomyDetailMaxIndex));
    const bool ShowEconomyIncomeScreen = SelectedEconomyIndex == 1;
    const bool ShowEconomyExpenseScreen = SelectedEconomyIndex == 2;
    const bool ShowEconomySwissAccountScreen = SelectedEconomyIndex == 3;
    const bool ShowEconomyCorruptionScreen = SelectedEconomyIndex == 4;
    const bool ShowEconomyProductionScreen = SelectedEconomyIndex == 5;
    const bool ShowEconomyCurrentTouristScreen = SelectedEconomyIndex == 6;
    const bool ShowEconomyTouristRatingScreen = SelectedEconomyIndex == 7;
    const bool ShowEconomyTouristCapacityScreen = SelectedEconomyIndex == 8;
    const bool ShowEconomyLaborScreen = SelectedEconomyIndex == 9;
    const bool ShowEconomyUnemployedScreen = SelectedEconomyIndex == 10;
    const bool ShowEconomyVacancyScreen = SelectedEconomyIndex == 11;
    const bool ShowEconomyElectricityScreen = SelectedEconomyIndex == 12;
    const bool ShowEconomyOverviewScreen =
        !ShowEconomyIncomeScreen &&
        !ShowEconomyExpenseScreen &&
        !ShowEconomySwissAccountScreen &&
        !ShowEconomyCorruptionScreen &&
        !ShowEconomyProductionScreen &&
        !ShowEconomyCurrentTouristScreen &&
        !ShowEconomyTouristRatingScreen &&
        !ShowEconomyTouristCapacityScreen &&
        !ShowEconomyLaborScreen &&
        !ShowEconomyUnemployedScreen &&
        !ShowEconomyVacancyScreen &&
        !ShowEconomyElectricityScreen;
    const int PowerSurplusMW =
        Snapshot.TotalProducedPowerMW - Snapshot.TotalRequiredPowerMW;
    const wchar_t* GEconomyTrendMonthLabels[GEconomyTrendXAxisLabelCount] =
    {
        L"12",
        L"24",
        L"36",
        L"48",
        L"60"
    };
    const wchar_t* GEconomyTrendYearLabels[GEconomyTrendXAxisLabelCount] =
    {
        L"",
        L"3년전",
        L"2년전",
        L"1년전",
        L"현재 연도"
    };
    std::array<float, GEconomyTrendBarCount> SwissAccountBars = {};
    for (int Index = GEconomyTrendBarCount - 5; Index < GEconomyTrendBarCount; ++Index)
    {
        if (Index >= 0)
            SwissAccountBars[static_cast<size_t>(Index)] = static_cast<float>(SwissBankAccountBalance);
    }

    auto FormatSwissCurrency = [](long long Value)
    {
        bool Negative = Value < 0;
        unsigned long long AbsValue = Negative ?
            static_cast<unsigned long long>(-Value) :
            static_cast<unsigned long long>(Value);
        std::wstring Digits = std::to_wstring(AbsValue);

        for (int i = static_cast<int>(Digits.size()) - 3; i > 0; i -= 3)
            Digits.insert(static_cast<size_t>(i), 1, L',');

        return std::wstring(Negative ? L"-S$" : L"S$") + Digits;
    };

    auto FormatInteger = [](long long Value)
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
    };

    auto FormatFixed2 = [](double Value)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%.2f", Value);
        return std::wstring(Buffer);
    };

    auto FormatSignedFixed2 = [&FormatFixed2](double Value)
    {
        if (Value > 0.0)
            return L"+" + FormatFixed2(Value);

        return FormatFixed2(Value);
    };

    auto SetEconomyMetricVisibility =
        [](const CAlmanacWidget::FMetricRowWidgets& Row, bool Enable)
    {
        if (auto Background = Row.Background.lock())
            Background->SetEnable(Enable);
        if (auto Label = Row.Label.lock())
            Label->SetEnable(Enable);
        if (auto Value = Row.Value.lock())
            Value->SetEnable(Enable);
        if (auto Bar = Row.Bar.lock())
            Bar->SetEnable(false);
    };

    auto SetEconomyDetailVisibility =
        [](const CAlmanacWidget::FDetailRowWidgets& Row, bool Enable)
    {
        if (auto Button = Row.Button.lock())
        {
            Button->SetEnable(Enable);
            Button->ButtonEnable(Enable);
        }
        if (auto Background = Row.Background.lock())
            Background->SetEnable(Enable);
        if (auto Label = Row.Label.lock())
            Label->SetEnable(Enable);
        if (auto Value = Row.Value.lock())
            Value->SetEnable(Enable);
    };

    auto ResetEconomyDetailStyle =
        [](const CAlmanacWidget::FDetailRowWidgets& Row)
    {
        if (auto Background = Row.Background.lock())
        {
            Background->SetTexture(
                Background->GetName() + "_base",
                GRowTexture);
            Background->SetTint(1.f, 1.f, 1.f, 0.94f);
        }
    };

    auto SetEconomyDetailHeaderStyle =
        [](const CAlmanacWidget::FDetailRowWidgets& Row)
    {
        if (auto Label = Row.Label.lock())
            Label->SetTextColor(92, 84, 66, 255);
        if (auto Value = Row.Value.lock())
            Value->SetTextColor(92, 84, 66, 255);
    };

    SetDetailRowData(
        Widget.mEconomyDetails[0],
        L"국고",
        FormatCompactCurrency(Snapshot.NationalBudget),
        SelectedEconomyIndex == 0);
    SetDetailRowData(
        Widget.mEconomyDetails[1],
        L"수익 (지난 12개월)",
        FormatCompactCurrency(EconomyAnnualIncome),
        SelectedEconomyIndex == 1);
    SetDetailRowData(
        Widget.mEconomyDetails[2],
        L"경비 (지난 12개월)",
        FormatCompactCurrency(EconomyAnnualExpense),
        SelectedEconomyIndex == 2);
    SetDetailRowData(
        Widget.mEconomyDetails[3],
        L"스위스 은행 계좌",
        L"S$2,000",
        SelectedEconomyIndex == 3);
    SetDetailRowData(
        Widget.mEconomyDetails[4],
        L"부패",
        L"0",
        SelectedEconomyIndex == 4);
    SetDetailRowData(
        Widget.mEconomyDetails[5],
        L"생산 건물",
        L"",
        SelectedEconomyIndex == 5);
    SetDetailRowData(
        Widget.mEconomyDetails[6],
        L"현재 관광객",
        std::to_wstring(CurrentTouristCount),
        SelectedEconomyIndex == 6);
    SetDetailRowData(
        Widget.mEconomyDetails[7],
        L"관광객 평가",
        std::to_wstring(TourismRating),
        SelectedEconomyIndex == 7);
    SetDetailRowData(
        Widget.mEconomyDetails[8],
        L"관광객 수용력",
        L"",
        SelectedEconomyIndex == 8);
    SetDetailRowData(
        Widget.mEconomyDetails[9],
        L"노동력",
        std::to_wstring(Snapshot.AssignedJobCount) +
            L"/" + std::to_wstring(Snapshot.JobCapacity),
        SelectedEconomyIndex == 9);
    SetDetailRowData(
        Widget.mEconomyDetails[10],
        L"실업자",
        std::to_wstring((std::max)(0, Snapshot.ActiveCitizenCount - Snapshot.AssignedJobCount)),
        SelectedEconomyIndex == 10);
    SetDetailRowData(
        Widget.mEconomyDetails[11],
        L"빈 일자리",
        std::to_wstring((std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount)),
        SelectedEconomyIndex == 11);
    SetDetailRowData(
        Widget.mEconomyDetails[12],
        L"전기",
        L"",
        SelectedEconomyIndex == 12);

    for (int Index = 5; Index < static_cast<int>(Widget.mEconomyDetails.size()); ++Index)
    {
        if (Index == 6 || Index == 7 || Index == 9 || Index == 10 || Index == 11)
            continue;

        if (auto Label = Widget.mEconomyDetails[static_cast<size_t>(Index)].Label.lock())
            Label->SetTextColor(92, 84, 66, 255);
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyMetrics.size()); ++Index)
        SetEconomyMetricVisibility(
            Widget.mEconomyMetrics[static_cast<size_t>(Index)],
            false);

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyBreakdownRows.size()); ++Index)
    {
        SetEconomyDetailVisibility(
            Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)],
            false);
        ResetEconomyDetailStyle(
            Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)]);
    }

    if (auto BreakdownTitleBackground = Widget.mEconomyBreakdownTitleBackground.lock())
        BreakdownTitleBackground->SetEnable(false);
    if (auto BreakdownTitle = Widget.mEconomyBreakdownTitle.lock())
        BreakdownTitle->SetEnable(false);

    if (auto TrendTitleBackground = Widget.mEconomyTrendTitleBackground.lock())
        TrendTitleBackground->SetEnable(!ShowEconomyCorruptionScreen);
    if (auto TrendTitle = Widget.mEconomyTrendTitle.lock())
    {
        TrendTitle->SetEnable(!ShowEconomyCorruptionScreen);
        if (!ShowEconomyCorruptionScreen)
        {
            TrendTitle->SetText(
                ShowEconomyIncomeScreen ?
                    L"수익" :
                ShowEconomyExpenseScreen ?
                    L"경비" :
                ShowEconomySwissAccountScreen ?
                    L"스위스 은행 계좌" :
                ShowEconomyCurrentTouristScreen ?
                    L"현재 관광객" :
                ShowEconomyTouristRatingScreen ?
                    L"관광객 평가" :
                ShowEconomyTouristCapacityScreen ?
                    L"관광객 수용력" :
                ShowEconomyLaborScreen ?
                    L"일자리" :
                ShowEconomyUnemployedScreen ?
                    L"실업자" :
                ShowEconomyVacancyScreen ?
                    L"빈 일자리" :
                ShowEconomyElectricityScreen ?
                    L"전기 개요" :
                ShowEconomyProductionScreen ?
                    L"생산 건물" :
                    L"국고");
        }
    }
    if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
        TrendFrame->SetEnable(
            !ShowEconomyCorruptionScreen &&
            !ShowEconomyProductionScreen &&
            !ShowEconomyElectricityScreen);
    if (auto TrendYAxisLine = Widget.mEconomyTrendYAxisLine.lock())
        TrendYAxisLine->SetEnable(
            !ShowEconomyCorruptionScreen &&
            !ShowEconomyProductionScreen &&
            !ShowEconomyElectricityScreen);
    if (auto TrendXAxisLine = Widget.mEconomyTrendXAxisLine.lock())
        TrendXAxisLine->SetEnable(
            !ShowEconomyCorruptionScreen &&
            !ShowEconomyProductionScreen &&
            !ShowEconomyElectricityScreen);
    if (auto TrendYAxisArrow = Widget.mEconomyTrendYAxisArrow.lock())
        TrendYAxisArrow->SetEnable(
            !ShowEconomyCorruptionScreen &&
            !ShowEconomyProductionScreen &&
            !ShowEconomyElectricityScreen);
    if (auto TrendXAxisArrow = Widget.mEconomyTrendXAxisArrow.lock())
        TrendXAxisArrow->SetEnable(
            !ShowEconomyCorruptionScreen &&
            !ShowEconomyProductionScreen &&
            !ShowEconomyElectricityScreen);

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyTrendGridLines.size()); ++Index)
    {
        if (auto GridLine = Widget.mEconomyTrendGridLines[static_cast<size_t>(Index)].lock())
            GridLine->SetEnable(
                !ShowEconomyCorruptionScreen &&
                !ShowEconomyProductionScreen &&
                !ShowEconomyElectricityScreen);
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyTrendLines.size()); ++Index)
    {
        if (auto TrendLine = Widget.mEconomyTrendLines[static_cast<size_t>(Index)].lock())
            TrendLine->SetEnable(false);
    }

    for (int Index = 0; Index < GEconomyTrendXAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mEconomyTrendXAxisLabels.size()))
            break;

        if (auto XLabel = Widget.mEconomyTrendXAxisLabels[static_cast<size_t>(Index)].lock())
        {
            XLabel->SetEnable(
                !ShowEconomyCorruptionScreen &&
                !ShowEconomyProductionScreen &&
                !ShowEconomyElectricityScreen);
            if (!ShowEconomyCorruptionScreen &&
                !ShowEconomyProductionScreen &&
                !ShowEconomyElectricityScreen)
            {
                const bool UseYearLabels =
                    ShowEconomyExpenseScreen ||
                    ShowEconomySwissAccountScreen ||
                    ShowEconomyCurrentTouristScreen ||
                    ShowEconomyTouristRatingScreen ||
                    ShowEconomyTouristCapacityScreen ||
                    ShowEconomyLaborScreen ||
                    ShowEconomyUnemployedScreen ||
                    ShowEconomyVacancyScreen;
                XLabel->SetText(
                    UseYearLabels ?
                        GEconomyTrendYearLabels[Index] :
                        GEconomyTrendMonthLabels[Index]);
            }
        }
    }

    for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
            break;

        if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            YLabel->SetEnable(false);
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyTrendBars.size()); ++Index)
    {
        if (auto Bar = Widget.mEconomyTrendBars[static_cast<size_t>(Index)].lock())
            Bar->SetEnable(false);
    }
    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyTrendSecondaryBars.size()); ++Index)
    {
        if (auto Bar = Widget.mEconomyTrendSecondaryBars[static_cast<size_t>(Index)].lock())
            Bar->SetEnable(false);
    }
    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyTrendTertiaryBars.size()); ++Index)
    {
        if (auto Bar = Widget.mEconomyTrendTertiaryBars[static_cast<size_t>(Index)].lock())
            Bar->SetEnable(false);
    }

    if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
    {
        const float GraphLeft = TrendFrame->GetPos().x + 22.f;
        const float GraphTop = TrendFrame->GetPos().y + 14.f;
        const float GraphWidth = TrendFrame->GetSize().x - 40.f;
        const float GraphHeight = TrendFrame->GetSize().y - 32.f;
        const float BarGroupWidth =
            GraphWidth / static_cast<float>((std::max)(1, GEconomyTrendBarCount));
        const float SingleBarWidth =
            (std::max)(4.f, BarGroupWidth * 0.72f);
        const float MaxValue = static_cast<float>((std::max)(1, EconomyTrendMax));

        for (int Index = 0; Index < GEconomyTrendBarCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendBars.size()))
                break;

            if (auto Bar = Widget.mEconomyTrendBars[static_cast<size_t>(Index)].lock())
            {
                const float Height =
                    GraphHeight *
                    Clamp01(EconomyTreasuryBars[static_cast<size_t>(Index)] / MaxValue);
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                    (BarGroupWidth - SingleBarWidth) * 0.5f;
                if (ShowEconomyOverviewScreen)
                {
                    Bar->SetTint(0.12f, 0.82f, 0.38f, 0.95f);
                    Bar->SetEnable(true);
                    Bar->SetPos(BarX, GraphTop + GraphHeight - Height);
                    Bar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                }
            }
        }
    }

    if (ShowEconomyIncomeScreen)
    {
        if (!Widget.mEconomyTrendYAxisLabels.empty())
        {
            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[0].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(L"100");
            }
        }

        if (Widget.mEconomyTrendYAxisLabels.size() > 2)
        {
            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[2].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(L"50");
            }
        }
    }
    else if (ShowEconomyCurrentTouristScreen)
    {
        const int TouristLabels[GEconomyTrendYAxisLabelCount] =
        {
            3900,
            3120,
            2340,
            1560,
            780,
            0
        };

        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                YLabel->SetEnable(Index < 5);
                if (Index < 5)
                    YLabel->SetText(std::to_wstring(TouristLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 22.f;
            const float GraphTop = TrendFrame->GetPos().y + 14.f;
            const float GraphWidth = TrendFrame->GetSize().x - 40.f;
            const float GraphHeight = TrendFrame->GetSize().y - 32.f;
            const float BarGroupWidth =
                GraphWidth / static_cast<float>((std::max)(1, GEconomyTrendBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.54f);
            const float MaxValue = 3900.f;

            for (int Index = 0; Index < GEconomyTrendBarCount; ++Index)
            {
                if (Index >= static_cast<int>(Widget.mEconomyTrendBars.size()))
                    break;

                if (auto Bar = Widget.mEconomyTrendBars[static_cast<size_t>(Index)].lock())
                {
                    const float Height =
                        GraphHeight *
                        Clamp01(CurrentTouristBars[static_cast<size_t>(Index)] / MaxValue);
                    const float BarX =
                        GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                    Bar->SetTint(0.82f, 0.63f, 0.04f, 0.94f);
                    Bar->SetEnable(true);
                    Bar->SetPos(BarX, GraphTop + GraphHeight - Height);
                    Bar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                }
            }
        }
    }
    else if (ShowEconomyExpenseScreen)
    {
        const int ExpenseLabels[GEconomyTrendYAxisLabelCount] =
        {
            EconomyExpenseTrendStep * 5,
            EconomyExpenseTrendStep * 4,
            EconomyExpenseTrendStep * 3,
            EconomyExpenseTrendStep * 2,
            EconomyExpenseTrendStep,
            0
        };

        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                YLabel->SetEnable(Index < 5);
                if (Index < 5)
                    YLabel->SetText(std::to_wstring(ExpenseLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 22.f;
            const float GraphTop = TrendFrame->GetPos().y + 14.f;
            const float GraphWidth = TrendFrame->GetSize().x - 40.f;
            const float GraphHeight = TrendFrame->GetSize().y - 32.f;
            const float BarGroupWidth =
                GraphWidth / static_cast<float>((std::max)(1, GEconomyTrendBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.58f);
            const float MaxValue = static_cast<float>((std::max)(1, EconomyExpenseTrendMax));

            for (int Index = 0; Index < GEconomyTrendBarCount; ++Index)
            {
                if (Index >= static_cast<int>(Widget.mEconomyTrendBars.size()))
                    break;

                if (auto Bar = Widget.mEconomyTrendBars[static_cast<size_t>(Index)].lock())
                {
                    const float Height =
                        GraphHeight *
                        Clamp01(EconomyExpenseBars[static_cast<size_t>(Index)] / MaxValue);
                    const float BarX =
                        GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                    Bar->SetTint(0.84f, 0.34f, 0.30f, 0.92f);
                    Bar->SetEnable(true);
                    Bar->SetPos(BarX, GraphTop + GraphHeight - Height);
                    Bar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                }
            }
        }
    }
    else if (ShowEconomySwissAccountScreen)
    {
        const int SwissLabels[GEconomyTrendYAxisLabelCount] =
        {
            2000,
            1600,
            1200,
            800,
            400,
            0
        };

        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                YLabel->SetEnable(Index < 5);
                if (Index < 5)
                    YLabel->SetText(std::to_wstring(SwissLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 22.f;
            const float GraphTop = TrendFrame->GetPos().y + 14.f;
            const float GraphWidth = TrendFrame->GetSize().x - 40.f;
            const float GraphHeight = TrendFrame->GetSize().y - 32.f;
            const float BarGroupWidth =
                GraphWidth / static_cast<float>((std::max)(1, GEconomyTrendBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.58f);
            const float MaxValue = static_cast<float>(SwissBankAccountBalance);

            for (int Index = 0; Index < GEconomyTrendBarCount; ++Index)
            {
                if (Index >= static_cast<int>(Widget.mEconomyTrendBars.size()))
                    break;

                if (auto Bar = Widget.mEconomyTrendBars[static_cast<size_t>(Index)].lock())
                {
                    const float Height =
                        GraphHeight *
                        Clamp01(SwissAccountBars[static_cast<size_t>(Index)] / MaxValue);
                    const float BarX =
                        GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                    if (Height > 0.f)
                    {
                        Bar->SetTint(0.30f, 0.52f, 0.84f, 0.94f);
                        Bar->SetEnable(true);
                        Bar->SetPos(BarX, GraphTop + GraphHeight - Height);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                    }
                }
            }
        }
    }
    else if (ShowEconomyTouristRatingScreen)
    {
        const int RatingLabels[GEconomyTrendYAxisLabelCount] =
        {
            120,
            100,
            80,
            60,
            40,
            20
        };

        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(std::to_wstring(RatingLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 22.f;
            const float GraphTop = TrendFrame->GetPos().y + 14.f;
            const float GraphWidth = TrendFrame->GetSize().x - 40.f;
            const float GraphHeight = TrendFrame->GetSize().y - 32.f;

            for (int SegmentIndex = 0; SegmentIndex < GEconomyTrendSegmentCount; ++SegmentIndex)
            {
                if (SegmentIndex >= static_cast<int>(Widget.mEconomyTrendLines.size()))
                    break;

                const float X0 =
                    GraphLeft +
                    GraphWidth *
                        static_cast<float>(SegmentIndex) /
                        static_cast<float>(GEconomyTrendSegmentCount);
                const float X1 =
                    GraphLeft +
                    GraphWidth *
                        static_cast<float>(SegmentIndex + 1) /
                        static_cast<float>(GEconomyTrendSegmentCount);
                const float Y0 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        TourismRatingTrend[static_cast<size_t>(SegmentIndex)],
                        20.f,
                        120.f);
                const float Y1 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        TourismRatingTrend[static_cast<size_t>(SegmentIndex + 1)],
                        20.f,
                        120.f);
                SetLineSegment(
                    Widget.mEconomyTrendLines[static_cast<size_t>(SegmentIndex)].lock(),
                    X0,
                    Y0,
                    X1,
                    Y1,
                    2.6f,
                    FVector4(0.82f, 0.63f, 0.04f, 0.96f));
            }
        }
    }
    else if (ShowEconomyTouristCapacityScreen)
    {
        const int CapacityLabels[GEconomyTrendYAxisLabelCount] =
        {
            33060,
            27550,
            22040,
            16530,
            11020,
            5510
        };

        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(std::to_wstring(CapacityLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 22.f;
            const float GraphTop = TrendFrame->GetPos().y + 14.f;
            const float GraphWidth = TrendFrame->GetSize().x - 40.f;
            const float GraphHeight = TrendFrame->GetSize().y - 32.f;

            for (int SegmentIndex = 0; SegmentIndex < GEconomyTrendSegmentCount; ++SegmentIndex)
            {
                const float X0 =
                    GraphLeft +
                    GraphWidth *
                        static_cast<float>(SegmentIndex) /
                        static_cast<float>(GEconomyTrendSegmentCount);
                const float X1 =
                    GraphLeft +
                    GraphWidth *
                        static_cast<float>(SegmentIndex + 1) /
                        static_cast<float>(GEconomyTrendSegmentCount);
                const float CapacityY0 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        TourismCapacityTrend[static_cast<size_t>(SegmentIndex)],
                        0.f,
                        33060.f);
                const float CapacityY1 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        TourismCapacityTrend[static_cast<size_t>(SegmentIndex + 1)],
                        0.f,
                        33060.f);
                const float ArrivalY0 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        TourismArrivalTrend[static_cast<size_t>(SegmentIndex)],
                        0.f,
                        33060.f);
                const float ArrivalY1 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        TourismArrivalTrend[static_cast<size_t>(SegmentIndex + 1)],
                        0.f,
                        33060.f);

                if (SegmentIndex < static_cast<int>(Widget.mEconomyTrendLines.size()))
                {
                    SetLineSegment(
                        Widget.mEconomyTrendLines[static_cast<size_t>(SegmentIndex)].lock(),
                        X0,
                        CapacityY0,
                        X1,
                        CapacityY1,
                        2.4f,
                        FVector4(0.82f, 0.63f, 0.04f, 0.96f));
                }

                const int SecondaryIndex = SegmentIndex + GEconomyTrendSegmentCount;
                if (SecondaryIndex < static_cast<int>(Widget.mEconomyTrendLines.size()))
                {
                    SetLineSegment(
                        Widget.mEconomyTrendLines[static_cast<size_t>(SecondaryIndex)].lock(),
                        X0,
                        ArrivalY0,
                        X1,
                        ArrivalY1,
                        2.4f,
                        FVector4(0.24f, 0.54f, 0.94f, 0.96f));
                }
            }
        }
    }
    else if (ShowEconomyLaborScreen)
    {
        const int LaborLabels[GEconomyTrendYAxisLabelCount] =
        {
            120,
            100,
            80,
            60,
            40,
            20
        };

        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText((std::to_wstring(LaborLabels[Index]) + L"%").c_str());
            }
        }

        if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 22.f;
            const float GraphTop = TrendFrame->GetPos().y + 14.f;
            const float GraphWidth = TrendFrame->GetSize().x - 40.f;
            const float GraphHeight = TrendFrame->GetSize().y - 32.f;

            for (int SegmentIndex = 0; SegmentIndex < GEconomyTrendSegmentCount; ++SegmentIndex)
            {
                const float X0 =
                    GraphLeft +
                    GraphWidth *
                        static_cast<float>(SegmentIndex) /
                        static_cast<float>(GEconomyTrendSegmentCount);
                const float X1 =
                    GraphLeft +
                    GraphWidth *
                        static_cast<float>(SegmentIndex + 1) /
                        static_cast<float>(GEconomyTrendSegmentCount);
                const float OccupancyY0 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        EconomyJobOccupancyTrend[static_cast<size_t>(SegmentIndex)],
                        0.f,
                        120.f);
                const float OccupancyY1 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        EconomyJobOccupancyTrend[static_cast<size_t>(SegmentIndex + 1)],
                        0.f,
                        120.f);
                const float UnemploymentY0 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        EconomyUnemploymentTrend[static_cast<size_t>(SegmentIndex)],
                        0.f,
                        120.f);
                const float UnemploymentY1 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        EconomyUnemploymentTrend[static_cast<size_t>(SegmentIndex + 1)],
                        0.f,
                        120.f);

                if (SegmentIndex < static_cast<int>(Widget.mEconomyTrendLines.size()))
                {
                    SetLineSegment(
                        Widget.mEconomyTrendLines[static_cast<size_t>(SegmentIndex)].lock(),
                        X0,
                        OccupancyY0,
                        X1,
                        OccupancyY1,
                        2.5f,
                        FVector4(0.30f, 0.48f, 0.82f, 0.96f));
                }

                const int SecondaryIndex = SegmentIndex + GEconomyTrendSegmentCount;
                if (SecondaryIndex < static_cast<int>(Widget.mEconomyTrendLines.size()))
                {
                    SetLineSegment(
                        Widget.mEconomyTrendLines[static_cast<size_t>(SecondaryIndex)].lock(),
                        X0,
                        UnemploymentY0,
                        X1,
                        UnemploymentY1,
                        2.3f,
                        FVector4(0.80f, 0.18f, 0.14f, 0.96f));
                }
            }
        }
    }
    else if (ShowEconomyUnemployedScreen)
    {
        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                if (Index == 0)
                {
                    YLabel->SetEnable(true);
                    YLabel->SetText(L"20");
                }
                else if (Index == 2)
                {
                    YLabel->SetEnable(true);
                    YLabel->SetText(L"10");
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 22.f;
            const float GraphTop = TrendFrame->GetPos().y + 14.f;
            const float GraphWidth = TrendFrame->GetSize().x - 40.f;
            const float GraphHeight = TrendFrame->GetSize().y - 32.f;
            const float BarGroupWidth =
                GraphWidth / static_cast<float>((std::max)(1, GEconomyTrendBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.74f);
            const float MaxValue = 20.f;

            for (int Index = 0; Index < GEconomyTrendBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                    (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float UneducatedHeight =
                    GraphHeight *
                    Clamp01(PopulationUnemployedUneducatedBars[static_cast<size_t>(Index)] / MaxValue);
                const float HighSchoolHeight =
                    GraphHeight *
                    Clamp01(PopulationUnemployedHighSchoolBars[static_cast<size_t>(Index)] / MaxValue);
                const float CollegeHeight =
                    GraphHeight *
                    Clamp01(PopulationUnemployedCollegeBars[static_cast<size_t>(Index)] / MaxValue);
                const float UneducatedTop = GraphTop + GraphHeight - UneducatedHeight;
                const float HighSchoolTop = UneducatedTop - HighSchoolHeight;
                const float CollegeTop = HighSchoolTop - CollegeHeight;

                if (Index < static_cast<int>(Widget.mEconomyTrendBars.size()))
                {
                    if (auto Bar = Widget.mEconomyTrendBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetTint(0.30f, 0.48f, 0.78f, 0.94f);
                        Bar->SetEnable(UneducatedHeight > 0.f);
                        Bar->SetPos(BarX, UneducatedTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, UneducatedHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mEconomyTrendSecondaryBars.size()))
                {
                    if (auto Bar = Widget.mEconomyTrendSecondaryBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetTint(0.78f, 0.26f, 0.22f, 0.92f);
                        Bar->SetEnable(HighSchoolHeight > 0.f);
                        Bar->SetPos(BarX, HighSchoolTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, HighSchoolHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mEconomyTrendTertiaryBars.size()))
                {
                    if (auto Bar = Widget.mEconomyTrendTertiaryBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetTint(0.56f, 0.68f, 0.24f, 0.92f);
                        Bar->SetEnable(CollegeHeight > 0.f);
                        Bar->SetPos(BarX, CollegeTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, CollegeHeight));
                    }
                }
            }
        }
    }
    else if (ShowEconomyVacancyScreen)
    {
        const int VacancyLabels[GEconomyTrendYAxisLabelCount] =
        {
            750,
            600,
            450,
            300,
            150,
            0
        };

        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(std::to_wstring(VacancyLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 22.f;
            const float GraphTop = TrendFrame->GetPos().y + 14.f;
            const float GraphWidth = TrendFrame->GetSize().x - 40.f;
            const float GraphHeight = TrendFrame->GetSize().y - 32.f;
            const float BarGroupWidth =
                GraphWidth / static_cast<float>((std::max)(1, GEconomyTrendBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.72f);
            const float MaxValue = 750.f;

            for (int Index = 0; Index < GEconomyTrendBarCount; ++Index)
            {
                if (Index >= static_cast<int>(Widget.mEconomyTrendBars.size()))
                    break;

                if (auto Bar = Widget.mEconomyTrendBars[static_cast<size_t>(Index)].lock())
                {
                    const float Height =
                        GraphHeight *
                        Clamp01(PopulationWorkVacancyBars[static_cast<size_t>(Index)] / MaxValue);
                    const float BarX =
                        GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                    Bar->SetTint(0.30f, 0.48f, 0.78f, 0.94f);
                    Bar->SetEnable(true);
                    Bar->SetPos(BarX, GraphTop + GraphHeight - Height);
                    Bar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                }
            }
        }
    }
    else if (ShowEconomyElectricityScreen)
    {
        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
                YLabel->SetEnable(false);
        }
    }
    else if (ShowEconomyCorruptionScreen)
    {
        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
                YLabel->SetEnable(false);
        }
    }
    else if (ShowEconomyProductionScreen)
    {
        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
                YLabel->SetEnable(false);
        }
    }
    else
    {
        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                YLabel->SetEnable(true);
                const int Value = EconomyTrendStep * (GEconomyTrendYAxisLabelCount - Index);
                YLabel->SetText(std::to_wstring(Value).c_str());
            }
        }
    }

    if (auto ChangeFrame = Widget.mEconomyChangeFrame.lock())
        ChangeFrame->SetEnable(ShowEconomyOverviewScreen);
    if (auto ChangeYAxisLine = Widget.mEconomyChangeYAxisLine.lock())
        ChangeYAxisLine->SetEnable(ShowEconomyOverviewScreen);
    if (auto ChangeXAxisLine = Widget.mEconomyChangeXAxisLine.lock())
        ChangeXAxisLine->SetEnable(ShowEconomyOverviewScreen);
    if (auto ChangeYAxisArrow = Widget.mEconomyChangeYAxisArrow.lock())
        ChangeYAxisArrow->SetEnable(ShowEconomyOverviewScreen);
    if (auto ChangeXAxisArrow = Widget.mEconomyChangeXAxisArrow.lock())
        ChangeXAxisArrow->SetEnable(ShowEconomyOverviewScreen);

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyChangeGridLines.size()); ++Index)
    {
        if (auto GridLine = Widget.mEconomyChangeGridLines[static_cast<size_t>(Index)].lock())
            GridLine->SetEnable(ShowEconomyOverviewScreen);
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyChangePositiveBars.size()); ++Index)
    {
        if (auto PositiveBar = Widget.mEconomyChangePositiveBars[static_cast<size_t>(Index)].lock())
            PositiveBar->SetEnable(false);
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyChangeNegativeBars.size()); ++Index)
    {
        if (auto NegativeBar = Widget.mEconomyChangeNegativeBars[static_cast<size_t>(Index)].lock())
            NegativeBar->SetEnable(false);
    }

    const int EconomyChangeLabels[GEconomyChangeYAxisLabelCount] =
    {
        EconomyChangeStep * 3,
        EconomyChangeStep * 2,
        EconomyChangeStep,
        -EconomyChangeStep,
        -EconomyChangeStep * 2,
        -EconomyChangeStep * 3
    };
    for (int Index = 0; Index < GEconomyChangeYAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mEconomyChangeYAxisLabels.size()))
            break;

        if (auto YLabel = Widget.mEconomyChangeYAxisLabels[static_cast<size_t>(Index)].lock())
        {
            YLabel->SetEnable(ShowEconomyOverviewScreen);
            if (ShowEconomyOverviewScreen)
                YLabel->SetText(std::to_wstring(EconomyChangeLabels[Index]).c_str());
        }
    }

    if (auto ChangeFrame = Widget.mEconomyChangeFrame.lock())
    {
        if (ShowEconomyOverviewScreen)
        {
            const float GraphLeft = ChangeFrame->GetPos().x + 22.f;
            const float GraphTop = ChangeFrame->GetPos().y + 12.f;
            const float GraphWidth = ChangeFrame->GetSize().x - 40.f;
            const float GraphHeight = ChangeFrame->GetSize().y - 26.f;
            const float ZeroY =
                ResolveGraphYInRange(
                    GraphTop,
                    GraphHeight,
                    0.f,
                    -static_cast<float>(EconomyChangeMax),
                    static_cast<float>(EconomyChangeMax));
            const float BarGroupWidth =
                GraphWidth / static_cast<float>((std::max)(1, GEconomyChangeBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.64f);
            const float MaxValue = static_cast<float>((std::max)(1, EconomyChangeMax));

            for (int Index = 0; Index < GEconomyChangeBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                    (BarGroupWidth - SingleBarWidth) * 0.5f;

                if (Index < static_cast<int>(Widget.mEconomyChangePositiveBars.size()))
                {
                    if (auto PositiveBar = Widget.mEconomyChangePositiveBars[static_cast<size_t>(Index)].lock())
                    {
                        const float PositiveHeight =
                            GraphHeight *
                            Clamp01(EconomyIncomeBars[static_cast<size_t>(Index)] / MaxValue) * 0.48f;
                        PositiveBar->SetEnable(true);
                        PositiveBar->SetPos(BarX, ZeroY - PositiveHeight);
                        PositiveBar->SetSize(SingleBarWidth, (std::max)(2.f, PositiveHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mEconomyChangeNegativeBars.size()))
                {
                    if (auto NegativeBar = Widget.mEconomyChangeNegativeBars[static_cast<size_t>(Index)].lock())
                    {
                        const float NegativeHeight =
                            GraphHeight *
                            Clamp01(EconomyExpenseBars[static_cast<size_t>(Index)] / MaxValue) * 0.48f;
                        NegativeBar->SetEnable(true);
                        NegativeBar->SetPos(BarX, ZeroY);
                        NegativeBar->SetSize(SingleBarWidth, (std::max)(2.f, NegativeHeight));
                    }
                }
            }
        }
    }

    if (ShowEconomyIncomeScreen)
    {
        SetMetricRowData(
            Widget.mEconomyMetrics[0],
            L"수익 (지난 12개월)",
            FormatCompactCurrency(EconomyAnnualIncome),
            0.f,
            FVector4(0.28f, 0.46f, 0.78f, 0.94f),
            false);
        SetEconomyMetricVisibility(Widget.mEconomyMetrics[0], true);

        if (auto Background = Widget.mEconomyMetrics[0].Background.lock())
        {
            Background->SetTexture(
                Background->GetName() + "_summary",
                GBarFillTexture);
            Background->SetTint(0.28f, 0.46f, 0.78f, 0.94f);
        }
        if (auto Label = Widget.mEconomyMetrics[0].Label.lock())
            Label->SetTextColor(58, 56, 42, 255);
        if (auto Value = Widget.mEconomyMetrics[0].Value.lock())
            Value->SetTextColor(58, 56, 42, 255);

        if (auto BreakdownTitleBackground = Widget.mEconomyBreakdownTitleBackground.lock())
            BreakdownTitleBackground->SetEnable(true);
        if (auto BreakdownTitle = Widget.mEconomyBreakdownTitle.lock())
        {
            BreakdownTitle->SetEnable(true);
            BreakdownTitle->SetText(L"수익 명세");
        }

        const wchar_t* BreakdownLabels[GEconomyBreakdownRowCount] =
        {
            L"수출",
            L"지역 서비스",
            L"관광객 서비스",
            L"열강 원조",
            L"기타",
            L""
        };
        const long long BreakdownValues[GEconomyBreakdownRowCount] =
        {
            EconomyAnnualExportIncome,
            EconomyAnnualLocalServiceIncome,
            EconomyAnnualTourismIncome,
            EconomyAnnualAidIncome,
            EconomyAnnualOtherIncome,
            0LL
        };

        for (int Index = 0; Index < GEconomyBreakdownRowCount; ++Index)
        {
            if (Index >= 5)
                break;

            if (Index >= static_cast<int>(Widget.mEconomyBreakdownRows.size()))
                break;

            auto& Row = Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)];
            SetDetailRowData(
                Row,
                BreakdownLabels[Index],
                FormatCurrency(BreakdownValues[Index]),
                false);
            SetEconomyDetailVisibility(Row, true);
        }
    }
    else if (ShowEconomyExpenseScreen)
    {
        SetMetricRowData(
            Widget.mEconomyMetrics[0],
            L"경비 (지난 12개월)",
            FormatCompactCurrency(EconomyAnnualExpense),
            0.f,
            FVector4(0.74f, 0.24f, 0.22f, 0.94f),
            false);
        SetEconomyMetricVisibility(Widget.mEconomyMetrics[0], true);

        if (auto Background = Widget.mEconomyMetrics[0].Background.lock())
        {
            Background->SetTexture(
                Background->GetName() + "_summary",
                GBarFillTexture);
            Background->SetTint(0.74f, 0.24f, 0.22f, 0.94f);
        }
        if (auto Label = Widget.mEconomyMetrics[0].Label.lock())
            Label->SetTextColor(58, 56, 42, 255);
        if (auto Value = Widget.mEconomyMetrics[0].Value.lock())
            Value->SetTextColor(58, 56, 42, 255);

        if (auto BreakdownTitleBackground = Widget.mEconomyBreakdownTitleBackground.lock())
            BreakdownTitleBackground->SetEnable(true);
        if (auto BreakdownTitle = Widget.mEconomyBreakdownTitle.lock())
        {
            BreakdownTitle->SetEnable(true);
            BreakdownTitle->SetText(L"경비 명세");
        }

        const wchar_t* BreakdownLabels[GEconomyBreakdownRowCount] =
        {
            L"건설",
            L"유지 비용",
            L"임금",
            L"수입",
            L"기타",
            L"유명인"
        };
        const long long BreakdownValues[GEconomyBreakdownRowCount] =
        {
            EconomyAnnualConstructionExpense,
            EconomyAnnualUpkeepExpense,
            EconomyAnnualWageExpense,
            EconomyAnnualImportExpense,
            EconomyAnnualMiscExpense,
            EconomyAnnualCelebrityExpense
        };

        for (int Index = 0; Index < GEconomyBreakdownRowCount; ++Index)
        {
            if (Index >= 6)
                break;

            if (Index >= static_cast<int>(Widget.mEconomyBreakdownRows.size()))
                break;

            auto& Row = Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)];
            SetDetailRowData(
                Row,
                BreakdownLabels[Index],
                FormatCurrency(BreakdownValues[Index]),
                false);
            SetEconomyDetailVisibility(Row, true);
        }
    }
    else if (ShowEconomySwissAccountScreen)
    {
        SetMetricRowData(
            Widget.mEconomyMetrics[0],
            L"스위스 은행 계좌",
            FormatSwissCurrency(SwissBankAccountBalance),
            0.f,
            FVector4(0.28f, 0.46f, 0.78f, 0.94f),
            false);
        SetEconomyMetricVisibility(Widget.mEconomyMetrics[0], true);

        if (auto Background = Widget.mEconomyMetrics[0].Background.lock())
        {
            Background->SetTexture(
                Background->GetName() + "_summary",
                GBarFillTexture);
            Background->SetTint(0.28f, 0.46f, 0.78f, 0.94f);
        }
        if (auto Label = Widget.mEconomyMetrics[0].Label.lock())
            Label->SetTextColor(58, 56, 42, 255);
        if (auto Value = Widget.mEconomyMetrics[0].Value.lock())
            Value->SetTextColor(58, 56, 42, 255);

        const wchar_t* BreakdownLabels[GEconomyBreakdownRowCount] =
        {
            L"거래",
            L"제안",
            L"기타",
            L"",
            L"",
            L""
        };
        const long long BreakdownValues[GEconomyBreakdownRowCount] =
        {
            SwissBankAccountBalance,
            0LL,
            0LL,
            0LL,
            0LL,
            0LL
        };

        for (int Index = 0; Index < 3; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyBreakdownRows.size()))
                break;

            auto& Row = Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)];
            SetDetailRowData(
                Row,
                BreakdownLabels[Index],
                FormatSwissCurrency(BreakdownValues[Index]),
                false);
            SetEconomyDetailVisibility(Row, true);
        }
    }
    else if (ShowEconomyCorruptionScreen)
    {
        SetMetricRowData(
            Widget.mEconomyMetrics[0],
            L"부패",
            FormatFixed2(0.0),
            0.f,
            FVector4(0.72f, 0.56f, 0.54f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mEconomyMetrics[1],
            L"부패 (전월)",
            FormatFixed2(0.0),
            0.f,
            FVector4(0.72f, 0.56f, 0.54f, 0.92f),
            false);
        SetEconomyMetricVisibility(Widget.mEconomyMetrics[0], true);
        SetEconomyMetricVisibility(Widget.mEconomyMetrics[1], true);

        for (int Index = 0; Index < 2; ++Index)
        {
            auto& Row = Widget.mEconomyMetrics[static_cast<size_t>(Index)];
            if (auto Background = Row.Background.lock())
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(0.72f, 0.56f, 0.54f, 0.92f);
            }
            if (auto Label = Row.Label.lock())
                Label->SetTextColor(74, 62, 54, 255);
            if (auto Value = Row.Value.lock())
                Value->SetTextColor(74, 62, 54, 255);
        }

        if (auto BreakdownTitleBackground = Widget.mEconomyBreakdownTitleBackground.lock())
            BreakdownTitleBackground->SetEnable(true);
        if (auto BreakdownTitle = Widget.mEconomyBreakdownTitle.lock())
        {
            BreakdownTitle->SetEnable(true);
            BreakdownTitle->SetText(L"부패 요인");
        }

        if (!Widget.mEconomyBreakdownRows.empty())
        {
            auto& Row = Widget.mEconomyBreakdownRows[0];
            SetDetailRowData(
                Row,
                L"균형",
                FormatSignedFixed2(0.0),
                false);
            SetEconomyDetailVisibility(Row, true);
        }
    }
    else if (ShowEconomyProductionScreen)
    {
        const wchar_t* BreakdownLabels[GEconomyBreakdownRowCount] =
        {
            L"고기 1000개당",
            L"생가죽 1000개당",
            L"가죽 1000개당",
            L"담배 1000개당",
            L"코코아 1000개당",
            L"옥수수 1000개당",
            L"설탕 1000개당",
            L"물고기 1000개당"
        };
        const long long BreakdownValues[GEconomyBreakdownRowCount] =
        {
            1987LL,
            75LL,
            5119LL,
            0LL,
            2663LL,
            2122LL,
            1832LL,
            2462LL
        };

        for (int Index = 0; Index < GEconomyBreakdownRowCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyBreakdownRows.size()))
                break;

            auto& Row = Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)];
            SetDetailRowData(
                Row,
                BreakdownLabels[Index],
                FormatCurrency(BreakdownValues[Index]),
                false);
            SetEconomyDetailVisibility(Row, true);
        }
    }
    else if (ShowEconomyCurrentTouristScreen)
    {
        SetMetricRowData(
            Widget.mEconomyMetrics[0],
            L"현재 관광객",
            std::to_wstring(CurrentTouristCount),
            0.f,
            FVector4(0.84f, 0.66f, 0.08f, 0.94f),
            false);
        SetEconomyMetricVisibility(Widget.mEconomyMetrics[0], true);

        if (auto Background = Widget.mEconomyMetrics[0].Background.lock())
        {
            Background->SetTexture(
                Background->GetName() + "_summary",
                GBarFillTexture);
            Background->SetTint(0.84f, 0.66f, 0.08f, 0.94f);
        }
        if (auto Label = Widget.mEconomyMetrics[0].Label.lock())
            Label->SetTextColor(78, 64, 22, 255);
        if (auto Value = Widget.mEconomyMetrics[0].Value.lock())
            Value->SetTextColor(78, 64, 22, 255);

        if (auto BreakdownTitleBackground = Widget.mEconomyBreakdownTitleBackground.lock())
            BreakdownTitleBackground->SetEnable(true);
        if (auto BreakdownTitle = Widget.mEconomyBreakdownTitle.lock())
        {
            BreakdownTitle->SetEnable(true);
            BreakdownTitle->SetText(L"관광객 분포");
        }

        const wchar_t* BreakdownLabels[GEconomyBreakdownRowCount] =
        {
            L"휴양",
            L"문화",
            L"스릴 중독",
            L"배낭여행",
            L"아동",
            L"유명인"
        };
        const int BreakdownValues[GEconomyBreakdownRowCount] =
        {
            750,
            456,
            415,
            470,
            1195,
            1
        };

        for (int Index = 0; Index < 6; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyBreakdownRows.size()))
                break;

            auto& Row = Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)];
            SetDetailRowData(
                Row,
                BreakdownLabels[Index],
                std::to_wstring(BreakdownValues[Index]),
                false);
            SetEconomyDetailVisibility(Row, true);
        }
    }
    else if (ShowEconomyTouristRatingScreen)
    {
        SetMetricRowData(
            Widget.mEconomyMetrics[0],
            L"관광객 평가",
            std::to_wstring(TourismRating),
            0.f,
            FVector4(0.84f, 0.66f, 0.08f, 0.94f),
            false);
        SetEconomyMetricVisibility(Widget.mEconomyMetrics[0], true);

        if (auto Background = Widget.mEconomyMetrics[0].Background.lock())
        {
            Background->SetTexture(
                Background->GetName() + "_summary",
                GBarFillTexture);
            Background->SetTint(0.84f, 0.66f, 0.08f, 0.94f);
        }
        if (auto Label = Widget.mEconomyMetrics[0].Label.lock())
            Label->SetTextColor(78, 64, 22, 255);
        if (auto Value = Widget.mEconomyMetrics[0].Value.lock())
            Value->SetTextColor(78, 64, 22, 255);

        const wchar_t* BreakdownLabels[GEconomyBreakdownRowCount] =
        {
            L"▽ 관광객 평가 수정치",
            L"▷ 휴양",
            L"▷ 문화",
            L"▷ 스릴 중독",
            L"▷ 배낭여행",
            L"▷ 아동",
            L"▷ 유명인",
            L"▽ 관광객 숙박 시설",
            L"▷ 고급 호텔"
        };
        const wchar_t* BreakdownValues[GEconomyBreakdownRowCount] =
        {
            L"",
            L"3",
            L"3",
            L"3",
            L"3",
            L"2",
            L"2",
            L"114",
            L"133"
        };

        for (int Index = 0; Index < 9; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyBreakdownRows.size()))
                break;

            auto& Row = Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)];
            SetDetailRowData(
                Row,
                BreakdownLabels[Index],
                BreakdownValues[Index],
                false);
            SetEconomyDetailVisibility(Row, true);

            if (Index == 0 || Index == 7)
                SetEconomyDetailHeaderStyle(Row);
        }
    }
    else if (ShowEconomyTouristCapacityScreen)
    {
        SetMetricRowData(
            Widget.mEconomyMetrics[0],
            L"✓ 관광객 도착",
            FormatInteger(CurrentTouristCount),
            0.f,
            FVector4(0.94f, 0.90f, 0.78f, 0.96f),
            false);
        SetMetricRowData(
            Widget.mEconomyMetrics[1],
            L"✓ 사용 중인 슬롯",
            FormatInteger(2396),
            0.f,
            FVector4(0.94f, 0.90f, 0.78f, 0.96f),
            false);
        SetMetricRowData(
            Widget.mEconomyMetrics[2],
            L"✓ 수용력 점유율",
            L"87%",
            0.f,
            FVector4(0.94f, 0.90f, 0.78f, 0.96f),
            false);
        SetMetricRowData(
            Widget.mEconomyMetrics[3],
            L"✓ 총 숙박 슬롯",
            FormatInteger(2756),
            0.f,
            FVector4(0.94f, 0.90f, 0.78f, 0.96f),
            false);

        for (int Index = 0; Index < 4; ++Index)
        {
            auto& Row = Widget.mEconomyMetrics[static_cast<size_t>(Index)];
            SetEconomyMetricVisibility(Row, true);

            if (auto Background = Row.Background.lock())
            {
                Background->SetTexture(
                    Background->GetName() + "_tourism",
                    GRowTexture);
                Background->SetTint(0.98f, 0.96f, 0.88f, 0.98f);
            }
            if (auto Label = Row.Label.lock())
                Label->SetTextColor(112, 86, 28, 255);
            if (auto Value = Row.Value.lock())
                Value->SetTextColor(94, 78, 48, 255);
        }

        const wchar_t* BreakdownLabels[GEconomyBreakdownRowCount] =
        {
            L"▽ 관광객 숙박 시설",
            L"▷ 호텔",
            L"▷ 초고층 호텔",
            L"▷ 고급 호텔",
            L"▽ 관광객 도착 건물",
            L"▷ 여객선 터미널",
            L"▷ 공항"
        };
        const std::wstring BreakdownValues[GEconomyBreakdownRowCount] =
        {
            L"114",
            FormatInteger(2296) + L"/" + FormatInteger(2616),
            FormatInteger(98) + L"/" + FormatInteger(128),
            FormatInteger(2) + L"/" + FormatInteger(12),
            L"8",
            FormatInteger(337) + L"/" + FormatInteger(1000),
            FormatInteger(110) + L"/" + FormatInteger(150)
        };

        for (int Index = 0; Index < 7; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyBreakdownRows.size()))
                break;

            auto& Row = Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)];
            SetDetailRowData(
                Row,
                BreakdownLabels[Index],
                BreakdownValues[Index],
                false);
            SetEconomyDetailVisibility(Row, true);

            if (Index == 0 || Index == 4)
                SetEconomyDetailHeaderStyle(Row);
        }
    }
    else if (ShowEconomyLaborScreen)
    {
        SetMetricRowData(
            Widget.mEconomyMetrics[0],
            L"현재 실업률",
            L"1%",
            0.f,
            FVector4(0.74f, 0.22f, 0.18f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mEconomyMetrics[1],
            L"현재 직장 점유율",
            L"76%",
            0.f,
            FVector4(0.24f, 0.42f, 0.74f, 0.94f),
            false);
        SetEconomyMetricVisibility(Widget.mEconomyMetrics[0], true);
        SetEconomyMetricVisibility(Widget.mEconomyMetrics[1], true);

        for (int Index = 0; Index < 2; ++Index)
        {
            auto& Row = Widget.mEconomyMetrics[static_cast<size_t>(Index)];
            if (auto Background = Row.Background.lock())
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.74f, 0.22f, 0.18f, 0.94f) :
                        FVector4(0.24f, 0.42f, 0.74f, 0.94f));
            }
            if (auto Label = Row.Label.lock())
                Label->SetTextColor(248, 242, 226, 255);
            if (auto Value = Row.Value.lock())
                Value->SetTextColor(248, 242, 226, 255);
        }

        const wchar_t* BreakdownLabels[GEconomyBreakdownRowCount] =
        {
            L"▷ 빈 일자리",
            L"▷ 폐쇄된 직업",
            L"▷ 실업자 시민"
        };
        const wchar_t* BreakdownValues[GEconomyBreakdownRowCount] =
        {
            L"730",
            L"0",
            L"5"
        };

        for (int Index = 0; Index < 3; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyBreakdownRows.size()))
                break;

            auto& Row = Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)];
            SetDetailRowData(
                Row,
                BreakdownLabels[Index],
                BreakdownValues[Index],
                false);
            SetEconomyDetailVisibility(Row, true);
        }
    }
    else if (ShowEconomyUnemployedScreen)
    {
        if (auto BreakdownTitleBackground = Widget.mEconomyBreakdownTitleBackground.lock())
            BreakdownTitleBackground->SetEnable(true);
        if (auto BreakdownTitle = Widget.mEconomyBreakdownTitle.lock())
        {
            BreakdownTitle->SetEnable(true);
            BreakdownTitle->SetText(L"학력별 실업자");
        }

        const wchar_t* BreakdownLabels[GEconomyBreakdownRowCount] =
        {
            L"무학력",
            L"고등학교",
            L"대학교"
        };
        const int BreakdownValues[GEconomyBreakdownRowCount] =
        {
            UnemployedUneducatedCount,
            UnemployedHighSchoolCount,
            UnemployedCollegeCount
        };
        const FVector4 BreakdownTints[3] =
        {
            FVector4(0.30f, 0.48f, 0.78f, 0.94f),
            FVector4(0.78f, 0.26f, 0.22f, 0.92f),
            FVector4(0.56f, 0.68f, 0.24f, 0.92f)
        };

        for (int Index = 0; Index < 3; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyBreakdownRows.size()))
                break;

            auto& Row = Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)];
            SetDetailRowData(
                Row,
                BreakdownLabels[Index],
                std::to_wstring(BreakdownValues[Index]),
                false);
            SetEconomyDetailVisibility(Row, true);

            if (auto Background = Row.Background.lock())
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(BreakdownTints[Index]);
            }
            if (auto Label = Row.Label.lock())
                Label->SetTextColor(58, 56, 42, 255);
            if (auto Value = Row.Value.lock())
                Value->SetTextColor(58, 56, 42, 255);
        }
    }
    else if (ShowEconomyVacancyScreen)
    {
        if (auto BreakdownTitleBackground = Widget.mEconomyBreakdownTitleBackground.lock())
            BreakdownTitleBackground->SetEnable(true);
        if (auto BreakdownTitle = Widget.mEconomyBreakdownTitle.lock())
        {
            BreakdownTitle->SetEnable(true);
            BreakdownTitle->SetText(L"학력별 빈 일자리");
        }

        const wchar_t* BreakdownLabels[GEconomyBreakdownRowCount] =
        {
            L"▷ 무학력",
            L"▷ 고등학교",
            L"▷ 대학교"
        };
        const int BreakdownValues[GEconomyBreakdownRowCount] =
        {
            WorkVacancyUneducatedCount,
            WorkVacancyHighSchoolCount,
            WorkVacancyCollegeCount
        };

        for (int Index = 0; Index < 3; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyBreakdownRows.size()))
                break;

            auto& Row = Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)];
            SetDetailRowData(
                Row,
                BreakdownLabels[Index],
                std::to_wstring(BreakdownValues[Index]),
                false);
            SetEconomyDetailVisibility(Row, true);
        }
    }
    else if (ShowEconomyElectricityScreen)
    {
        const std::wstring PowerSurplusText =
            (PowerSurplusMW >= 0 ? L"+" : L"") +
            std::to_wstring(PowerSurplusMW) +
            L"메가와트";

        if (Widget.mEconomyBreakdownRows.size() > 0)
        {
            auto& Row = Widget.mEconomyBreakdownRows[0];
            SetDetailRowData(
                Row,
                L"▷ #1 전력 상태",
                PowerSurplusText,
                false);
            SetEconomyDetailVisibility(Row, true);
        }

        if (Widget.mEconomyBreakdownRows.size() > 1)
        {
            auto& Row = Widget.mEconomyBreakdownRows[1];
            SetDetailRowData(
                Row,
                L"단절된 소비자",
                std::to_wstring(Snapshot.DisconnectedConsumerCount),
                false);
            SetEconomyDetailVisibility(Row, true);
        }
    }
    else
    {
        SetMetricRowData(
            Widget.mEconomyMetrics[0],
            L"현재 국고",
            FormatCompactCurrency(Snapshot.NationalBudget),
            0.f,
            FVector4(0.10f, 0.72f, 0.32f, 0.95f),
            false);
        SetMetricRowData(
            Widget.mEconomyMetrics[1],
            L"현재 잔고",
            FormatCurrency(Snapshot.DailyNetChange),
            0.f,
            FVector4(0.56f, 0.56f, 0.56f, 0.95f),
            false);
        SetMetricRowData(
            Widget.mEconomyMetrics[2],
            L"수익 (전날)",
            FormatCurrency(EconomyDailyIncome),
            0.f,
            FVector4(0.28f, 0.46f, 0.78f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mEconomyMetrics[3],
            L"경비 (전날)",
            FormatCurrency(EconomyDailyExpense),
            0.f,
            FVector4(0.76f, 0.31f, 0.28f, 0.94f),
            false);

        for (int Index = 0; Index < 4 && Index < static_cast<int>(Widget.mEconomyMetrics.size()); ++Index)
        {
            auto& Row = Widget.mEconomyMetrics[static_cast<size_t>(Index)];
            SetEconomyMetricVisibility(Row, true);

            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                if (Index == 1)
                {
                    Background->SetTexture(
                        Background->GetName() + "_base",
                        GRowTexture);
                    Background->SetTint(0.86f, 0.86f, 0.84f, 0.96f);
                }
                else
                {
                    Background->SetTexture(
                        Background->GetName() + "_summary",
                        GBarFillTexture);
                    Background->SetTint(
                        Index == 0 ?
                            FVector4(0.10f, 0.72f, 0.32f, 0.95f) :
                        Index == 2 ?
                            FVector4(0.28f, 0.46f, 0.78f, 0.94f) :
                            FVector4(0.76f, 0.31f, 0.28f, 0.94f));
                }
            }

            if (Label)
                Label->SetTextColor(Index == 1 ? 82 : 58, Index == 1 ? 76 : 56, Index == 1 ? 66 : 42, 255);
            if (Value)
                Value->SetTextColor(Index == 1 ? 82 : 58, Index == 1 ? 76 : 56, Index == 1 ? 66 : 42, 255);
        }
    }

}
void FAlmanacRenderer::ApplyResourcePage(CAlmanacWidget& Widget)
{
    struct FResourceUiEntry
    {
        const wchar_t* Name = L"";
        int Amount = 0;
        int InTransit = 0;
        int Producing = 0;
        int ProductionCapacity = 0;
        int Stored = 0;
        int StorageCapacity = 0;
        int Processing = 0;
        int ProcessingCapacity = 0;
    };

    const std::array<FResourceUiEntry, GResourceRowCount> ResourceEntries =
    {
        FResourceUiEntry{ L"가구", 200, 0, 0, 0, 0, 20000, 200, 200 },
        FResourceUiEntry{ L"가죽", 36, 0, 0, 0, 36, 20000, 0, 0 },
        FResourceUiEntry{ L"감시 드론", 0, 0, 0, 0, 0, 20000, 0, 0 },
        FResourceUiEntry{ L"강철", 0, 0, 0, 0, 0, 20000, 0, 0 },
        FResourceUiEntry{ L"고기", 5942, 0, 0, 0, 5942, 20000, 0, 0 },
        FResourceUiEntry{ L"고무", 0, 0, 0, 0, 0, 20000, 0, 0 },
        FResourceUiEntry{ L"널빤지", 0, 0, 0, 0, 0, 20000, 0, 0 },
        FResourceUiEntry{ L"니켈", 0, 0, 0, 0, 0, 20000, 0, 0 },
        FResourceUiEntry{ L"담배", 0, 0, 0, 0, 0, 20000, 0, 0 },
        FResourceUiEntry{ L"럼주", 0, 0, 0, 0, 0, 20000, 0, 0 },
        FResourceUiEntry{ L"목화", 0, 0, 0, 0, 0, 20000, 0, 0 },
        FResourceUiEntry{ L"무기", 0, 0, 0, 0, 0, 20000, 0, 0 }
    };
    const int ResourceMaxIndex =
        (std::max)(0, static_cast<int>(ResourceEntries.size()) - 1);
    const int SelectedResourceIndex =
        (std::max)(0, (std::min)(Widget.mSelectedResourceIndex, ResourceMaxIndex));
    const FResourceUiEntry& SelectedResource =
        ResourceEntries[static_cast<size_t>(SelectedResourceIndex)];

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceRows.size()); ++Index)
    {
        if (Index >= static_cast<int>(ResourceEntries.size()))
            break;

        SetDetailRowData(
            Widget.mResourceRows[static_cast<size_t>(Index)],
            ResourceEntries[static_cast<size_t>(Index)].Name,
            FormatInteger(ResourceEntries[static_cast<size_t>(Index)].Amount),
            Index == SelectedResourceIndex);
    }

    if (auto Title = Widget.mResourceProductionTitle.lock())
        Title->SetText(L"생산 내역");
    if (auto Title = Widget.mResourceDistributionTitle.lock())
        Title->SetText(L"자원 분포");
    if (auto Title = Widget.mResourceTrackingTitle.lock())
        Title->SetText(L"추적 중");
    if (auto Name = Widget.mResourceTrackingName.lock())
        Name->SetText(SelectedResource.Name);
    if (auto Value = Widget.mResourceTrackingValue.lock())
        Value->SetText(FormatInteger(SelectedResource.Amount).c_str());
    if (auto Text = Widget.mResourceProductionLegendPrimaryText.lock())
        Text->SetText(L"가공됨");
    if (auto Text = Widget.mResourceProductionLegendSecondaryText.lock())
        Text->SetText(L"생산됨");

    const wchar_t* ResourceProductionLabels[GResourceProductionXAxisLabelCount] =
    {
        L"3년전",
        L"2년전",
        L"1년전",
        L"현재 연도"
    };
    for (int Index = 0; Index < GResourceProductionXAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mResourceProductionXAxisLabels.size()))
            break;

        if (auto Label = Widget.mResourceProductionXAxisLabels[static_cast<size_t>(Index)].lock())
        {
            Label->SetEnable(true);
            Label->SetText(ResourceProductionLabels[Index]);
        }
    }

    for (int Index = 0; Index < GResourceProductionYAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mResourceProductionYAxisLabels.size()))
            break;

        if (auto Label = Widget.mResourceProductionYAxisLabels[static_cast<size_t>(Index)].lock())
        {
            Label->SetEnable(Index == 0);
            if (Index == 0)
                Label->SetText(L"200");
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
        const float BarGroupWidth =
            GraphWidth / static_cast<float>((std::max)(1, GResourceProductionBarCount));
        const float SingleBarWidth =
            (std::max)(3.f, BarGroupWidth * 0.42f);

        const float ProductionValues[GResourceProductionBarCount] =
        {
            0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 6.f, 2.f
        };

        for (int Index = 0; Index < GResourceProductionBarCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mResourceProductionBars.size()))
                break;

            if (auto Bar = Widget.mResourceProductionBars[static_cast<size_t>(Index)].lock())
            {
                const float Height =
                    GraphHeight *
                    Clamp01(ProductionValues[Index] / 200.f);
                if (Height <= 0.f)
                    continue;

                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                    (BarGroupWidth - SingleBarWidth) * 0.5f;
                Bar->SetTint(
                    Index >= GResourceProductionBarCount - 2 ?
                        FVector4(0.38f, 0.70f, 0.28f, 0.92f) :
                        FVector4(0.22f, 0.58f, 0.82f, 0.92f));
                Bar->SetEnable(true);
                Bar->SetPos(BarX, GraphTop + GraphHeight - Height);
                Bar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
            }
        }
    }

    const struct FResourceDistributionRow
    {
        const wchar_t* Label;
        int Value;
        FVector4 Tint;
    } DistributionRows[GResourceDistributionRowCount] =
    {
        { L"유입량", 2500, FVector4(0.36f, 0.70f, 0.20f, 0.95f) },
        { L"운송", 2500, FVector4(0.36f, 0.70f, 0.20f, 0.95f) },
        { L"유출량", 2300, FVector4(0.10f, 0.56f, 0.74f, 0.95f) },
        { L"가공", 2300, FVector4(0.10f, 0.56f, 0.74f, 0.95f) }
    };

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceDistributionRows.size()); ++Index)
    {
        if (Index >= GResourceDistributionRowCount)
            break;

        auto& Row = Widget.mResourceDistributionRows[static_cast<size_t>(Index)];
        SetMetricRowData(
            Row,
            DistributionRows[Index].Label,
            FormatInteger(DistributionRows[Index].Value),
            static_cast<float>(DistributionRows[Index].Value) / 2500.f,
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
        std::to_wstring(SelectedResource.InTransit),
        FormatInteger(SelectedResource.Producing) +
            L" / " + FormatInteger(SelectedResource.ProductionCapacity),
        FormatInteger(SelectedResource.Stored) +
            L" / " + FormatInteger(SelectedResource.StorageCapacity),
        FormatInteger(SelectedResource.Processing) +
            L" / " + FormatInteger(SelectedResource.ProcessingCapacity)
    };
    const wchar_t* ResourceTrackingLabels[GResourceDetailCount] =
    {
        L"운송 중",
        L"생산 중",
        L"▷ 보관 중",
        L"▷ 가공 중"
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
                Index == 1 ?
                    FVector4(0.86f, 0.86f, 0.84f, 0.76f) :
                    FVector4(1.f, 1.f, 1.f, 0.94f));
        }
        if (auto Label = Widget.mResourceDetails[static_cast<size_t>(Index)].Label.lock())
        {
            Label->SetTextColor(
                Index == 1 ? 116 : 92,
                Index == 1 ? 112 : 84,
                Index == 1 ? 104 : 66,
                255);
        }
        if (auto Value = Widget.mResourceDetails[static_cast<size_t>(Index)].Value.lock())
        {
            Value->SetTextColor(
                Index == 1 ? 116 : 92,
                Index == 1 ? 112 : 84,
                Index == 1 ? 104 : 66,
                255);
        }
    }

    if (auto Notice = Widget.mResourceNotice.lock())
    {
        Notice->SetEnable(false);
        Notice->SetText(L"");
    }


}
