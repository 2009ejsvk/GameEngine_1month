#include "AlmanacRendererPagesCoreShared.h"

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



