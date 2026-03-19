#include "AlmanacRenderer.h"
#include "AlmanacCalc.h"
#include "AlmanacRendererInternal.h"
#include "../Building/BuildingTypes.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace
{
    template <typename TValueContainer>
    float ResolveTrendSeriesMax(
        const TValueContainer& Values,
        float Minimum)
    {
        float Result = Minimum;

        for (float Value : Values)
            Result = (std::max)(Result, Value);

        return Result;
    }

    int ResolveTrendAxisStep(int RawMax)
    {
        const int TargetStep = (std::max)(1, (RawMax + 4) / 5);
        int Magnitude = 1;

        while (Magnitude * 10 < TargetStep)
            Magnitude *= 10;

        if (TargetStep <= Magnitude * 2)
            return Magnitude * 2;
        if (TargetStep <= Magnitude * 5)
            return Magnitude * 5;
        return Magnitude * 10;
    }

    std::array<int, GEconomyTrendYAxisLabelCount> BuildTrendAxisLabels(
        int AxisMax)
    {
        std::array<int, GEconomyTrendYAxisLabelCount> Labels = {};
        const int Step = (std::max)(1, AxisMax / 5);

        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
            Labels[static_cast<size_t>(Index)] =
                (std::max)(0, AxisMax - Step * Index);

        return Labels;
    }
}

using namespace AlmanacCalc;
void FAlmanacRenderer::ApplyEconomyPage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    const int EconomyPageSelectedIndex =
        (std::max)(
            0,
            (std::min)(
                Widget.mSelectedEconomyIndex,
                (std::max)(
                    0,
                    static_cast<int>(Widget.mEconomyDetails.size()) - 1)));
    const auto& EconomyLedger = Snapshot.EconomyLedgerHistory;

    auto BuildFallbackDailyRecord = [&Snapshot]()
    {
        FEconomyLedgerPeriodRecord Record;
        Record.NationalBudget = Snapshot.NationalBudget;
        Record.ExportIncome = Snapshot.DailyExportIncome;
        Record.ImportExpense = Snapshot.DailyImportExpense;
        Record.ConsumptionTaxIncome = Snapshot.DailyConsumptionTaxIncome;
        Record.IncomeTaxIncome = Snapshot.DailyIncomeTaxIncome;
        Record.PropertyTaxIncome = Snapshot.DailyPropertyTaxIncome;
        Record.WageCost = RoundToInt(
            static_cast<double>(Snapshot.MonthlyWageCost) / 30.0);
        Record.UpkeepCost = RoundToInt(
            static_cast<double>(Snapshot.MonthlyUpkeepCost) / 30.0);
        Record.EdictUpkeepCost = Snapshot.DailyEdictCost;
        Record.NetBudgetChange = Snapshot.DailyNetChange;
        Record.TaxCollectionEfficiency = Snapshot.TaxCollectionEfficiency;
        Record.ActiveCitizenCount = Snapshot.ActiveCitizenCount;
        Record.AssignedJobCount = Snapshot.AssignedJobCount;
        Record.JobCapacity = Snapshot.JobCapacity;
        Record.UnemployedCount = Snapshot.UnemployedCount;
        Record.WorkVacancyCount =
            (std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount);
        Record.ActiveTouristCount = Snapshot.ActiveTouristCount;
        Record.TourismVisitCapacity = Snapshot.TourismVisitCapacity;
        Record.TourismVisitOccupancy = Snapshot.TourismVisitOccupancy;
        return Record;
    };

    const FEconomyLedgerPeriodRecord LatestDailyEconomyRecord =
        EconomyLedger.DailyHistoryCount > 0 ?
            EconomyLedger.DailyHistory[
                static_cast<size_t>(EconomyLedger.DailyHistoryCount - 1)] :
            BuildFallbackDailyRecord();
    const FEconomyLedgerPeriodRecord LatestMonthlyEconomyRecord =
        EconomyLedger.CurrentMonthDayCount > 0 ?
            EconomyLedger.CurrentMonthToDate :
            (EconomyLedger.MonthlyHistoryCount > 0 ?
                EconomyLedger.MonthlyHistory[
                    static_cast<size_t>(EconomyLedger.MonthlyHistoryCount - 1)] :
                FEconomyLedgerPeriodRecord());
    const FEconomyLedgerPeriodRecord PreviousMonthlyEconomyRecord =
        EconomyLedger.MonthlyHistoryCount > 0 ?
            EconomyLedger.MonthlyHistory[
                static_cast<size_t>(EconomyLedger.MonthlyHistoryCount - 1)] :
            FEconomyLedgerPeriodRecord();

    auto BuildDailySeries = [&EconomyLedger](auto ResolveValue)
    {
        std::array<float, GEconomyTrendBarCount> Values = {};
        const int Count =
            (std::min)(GEconomyTrendBarCount, EconomyLedger.DailyHistoryCount);
        const int SourceStart =
            (std::max)(0, EconomyLedger.DailyHistoryCount - Count);
        const int DestStart = GEconomyTrendBarCount - Count;

        for (int Index = 0; Index < Count; ++Index)
        {
            Values[static_cast<size_t>(DestStart + Index)] =
                static_cast<float>(
                    ResolveValue(
                        EconomyLedger.DailyHistory[
                            static_cast<size_t>(SourceStart + Index)]));
        }

        return Values;
    };

    auto BuildMonthlySeries = [&EconomyLedger](auto ResolveValue)
    {
        std::array<float, GEconomyTrendBarCount> Values = {};
        const int CurrentMonthVisible =
            EconomyLedger.CurrentMonthDayCount > 0 ? 1 : 0;
        const int TotalCount =
            EconomyLedger.MonthlyHistoryCount + CurrentMonthVisible;
        const int Count = (std::min)(GEconomyTrendBarCount, TotalCount);
        const int SourceStart = (std::max)(0, TotalCount - Count);
        const int DestStart = GEconomyTrendBarCount - Count;

        for (int Index = 0; Index < Count; ++Index)
        {
            const int SourceIndex = SourceStart + Index;
            const FEconomyLedgerPeriodRecord* Source = nullptr;

            if (SourceIndex < EconomyLedger.MonthlyHistoryCount)
            {
                Source =
                    &EconomyLedger.MonthlyHistory[
                        static_cast<size_t>(SourceIndex)];
            }
            else if (EconomyLedger.CurrentMonthDayCount > 0)
            {
                Source = &EconomyLedger.CurrentMonthToDate;
            }

            if (!Source)
                continue;

            Values[static_cast<size_t>(DestStart + Index)] =
                static_cast<float>(ResolveValue(*Source));
        }

        return Values;
    };

    auto SumRecentMonths = [&EconomyLedger](auto ResolveValue, int MonthCount)
    {
        long long Total = 0;
        int Included = 0;

        if (MonthCount > 0 && EconomyLedger.CurrentMonthDayCount > 0)
        {
            Total += static_cast<long long>(
                ResolveValue(EconomyLedger.CurrentMonthToDate));
            ++Included;
        }

        for (int Index = EconomyLedger.MonthlyHistoryCount - 1;
            Index >= 0 && Included < MonthCount;
            --Index)
        {
            Total += static_cast<long long>(
                ResolveValue(
                    EconomyLedger.MonthlyHistory[static_cast<size_t>(Index)]));
            ++Included;
        }

        return Total;
    };

    const long long RealDailyIncome = LatestDailyEconomyRecord.GetTotalIncome();
    const long long RealDailyExpense =
        LatestDailyEconomyRecord.GetTotalExpense();
    const long long RealAnnualIncome =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.GetTotalIncome();
            },
            12);
    const long long RealAnnualExpense =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.GetTotalExpense();
            },
            12);
    const long long RealAnnualExportIncome =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.ExportIncome;
            },
            12);
    const long long RealAnnualConsumptionTaxIncome =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.ConsumptionTaxIncome;
            },
            12);
    const long long RealAnnualIncomeTaxIncome =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.IncomeTaxIncome;
            },
            12);
    const long long RealAnnualPropertyTaxIncome =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.PropertyTaxIncome;
            },
            12);
    const long long RealAnnualTaxBudgetDelta =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.TaxBudgetDelta;
            },
            12);
    const long long RealAnnualTradePolicyBudgetDelta =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.TradePolicyBudgetDelta;
            },
            12);
    const long long RealAnnualSpecialEventIncome =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return (std::max)(0LL, Record.SpecialEventBudgetDelta);
            },
            12);
    const long long RealAnnualOtherDirectIncome =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return (std::max)(0LL, Record.OtherBudgetDelta);
            },
            12);
    const long long RealAnnualSpecialIncome =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return (std::max)(0LL, Record.SpecialEventBudgetDelta) +
                    (std::max)(0LL, Record.OtherBudgetDelta);
            },
            12);
    const long long RealAnnualImportExpense =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.ImportExpense;
            },
            12);
    const long long RealAnnualWageExpense =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.WageCost;
            },
            12);
    const long long RealAnnualUpkeepExpense =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.UpkeepCost;
            },
            12);
    const long long RealAnnualEdictUpkeepExpense =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.EdictUpkeepCost;
            },
            12);
    const long long RealAnnualConstructionExpense =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.ConstructionExpense;
            },
            12);
    const long long RealAnnualRepairExpense =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.RepairExpense;
            },
            12);
    const long long RealAnnualEdictActivationExpense =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.EdictActivationExpense;
            },
            12);
    const long long RealAnnualSpecialEventExpense =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return (std::max)(0LL, -Record.SpecialEventBudgetDelta);
            },
            12);
    const long long RealAnnualOtherDirectExpense =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return (std::max)(0LL, -Record.OtherBudgetDelta);
            },
            12);
    const long long RealAnnualSpecialExpense =
        SumRecentMonths(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return (std::max)(0LL, -Record.SpecialEventBudgetDelta) +
                    (std::max)(0LL, -Record.OtherBudgetDelta);
            },
            12);
    const long long RealAnnualTradeBalance =
        RealAnnualExportIncome - RealAnnualImportExpense;
    const double RealCurrentTaxEfficiencyPercent =
        (EconomyLedger.CurrentMonthDayCount > 0 ?
            EconomyLedger.CurrentMonthToDate.TaxCollectionEfficiency :
            LatestDailyEconomyRecord.TaxCollectionEfficiency) * 100.0;
    const double RealPreviousMonthTaxEfficiencyPercent =
        PreviousMonthlyEconomyRecord.TaxCollectionEfficiency * 100.0;
    const long long RealDailyTaxIncome =
        LatestDailyEconomyRecord.GetTotalTaxIncome();
    const long long RealDailyDirectExpense =
        LatestDailyEconomyRecord.ConstructionExpense +
        LatestDailyEconomyRecord.RepairExpense +
        LatestDailyEconomyRecord.EdictActivationExpense;
    const long long RealDailySpecialIncome =
        (std::max)(0LL, LatestDailyEconomyRecord.SpecialEventBudgetDelta) +
        (std::max)(0LL, LatestDailyEconomyRecord.OtherBudgetDelta);
    const long long RealDailySpecialExpense =
        (std::max)(0LL, -LatestDailyEconomyRecord.SpecialEventBudgetDelta) +
        (std::max)(0LL, -LatestDailyEconomyRecord.OtherBudgetDelta);

    const std::array<float, GEconomyTrendBarCount> RealTreasuryBars =
        BuildMonthlySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.NationalBudget;
            });
    const std::array<float, GEconomyTrendBarCount> RealDailyIncomeBars =
        BuildDailySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.GetTotalIncome();
            });
    const std::array<float, GEconomyTrendBarCount> RealDailyExpenseBars =
        BuildDailySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.GetTotalExpense();
            });
    const std::array<float, GEconomyTrendBarCount> RealMonthlyIncomeBars =
        BuildMonthlySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.GetTotalIncome();
            });
    const std::array<float, GEconomyTrendBarCount> RealMonthlyExpenseBars =
        BuildMonthlySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.GetTotalExpense();
            });
    const std::array<float, GEconomyTrendBarCount> RealMonthlyExportBars =
        BuildMonthlySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.ExportIncome;
            });
    const std::array<float, GEconomyTrendBarCount> RealMonthlyImportBars =
        BuildMonthlySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.ImportExpense;
            });
    const std::array<float, GEconomyTrendBarCount> RealTaxEfficiencyBars =
        BuildDailySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return std::lround(Record.TaxCollectionEfficiency * 100.0);
            });
    const std::array<float, GEconomyTrendBarCount> RealTouristCountBars =
        BuildDailySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.ActiveTouristCount;
            });
    const std::array<float, GEconomyTrendBarCount> RealTourismRatingBars =
        BuildDailySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.TourismRating;
            });
    const std::array<float, GEconomyTrendBarCount> RealTourismCapacityBars =
        BuildDailySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.TourismVisitCapacity;
            });
    const std::array<float, GEconomyTrendBarCount> RealTourismOccupancyBars =
        BuildDailySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.TourismVisitOccupancy;
            });
    const std::array<float, GEconomyTrendBarCount> RealJobOccupancyBars =
        BuildDailySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.GetJobOccupancyPercent();
            });
    const std::array<float, GEconomyTrendBarCount> RealUnemploymentPercentBars =
        BuildDailySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.GetUnemploymentPercent();
            });
    const std::array<float, GEconomyTrendBarCount> RealUnemployedCountBars =
        BuildDailySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.UnemployedCount;
            });
    const std::array<float, GEconomyTrendBarCount> RealVacancyBars =
        BuildDailySeries(
            [](const FEconomyLedgerPeriodRecord& Record)
            {
                return Record.WorkVacancyCount;
            });

    const int RealTreasuryAxisMax =
        ResolveTrendAxisStep(
            (std::max)(
                1000,
                RoundToInt(
                    ResolveTrendSeriesMax(
                        RealTreasuryBars,
                        static_cast<float>(Snapshot.NationalBudget))))) * 5;
    const int RealIncomeAxisMax =
        ResolveTrendAxisStep(
            (std::max)(
                1000,
                RoundToInt(ResolveTrendSeriesMax(RealMonthlyIncomeBars, 0.f)))) * 5;
    const int RealExpenseAxisMax =
        ResolveTrendAxisStep(
            (std::max)(
                1000,
                RoundToInt(ResolveTrendSeriesMax(RealMonthlyExpenseBars, 0.f)))) * 5;
    const int RealTradeAxisMax =
        ResolveTrendAxisStep(
            (std::max)(
                500,
                RoundToInt(
                    (std::max)(
                        ResolveTrendSeriesMax(RealMonthlyExportBars, 0.f),
                        ResolveTrendSeriesMax(RealMonthlyImportBars, 0.f))))) * 5;
    const int RealUnemploymentCountAxisMax =
        ResolveTrendAxisStep(
            (std::max)(
                10,
                RoundToInt(
                    ResolveTrendSeriesMax(
                        RealUnemployedCountBars,
                        static_cast<float>(Snapshot.UnemployedCount))))) * 5;
    const int RealVacancyAxisMax =
        ResolveTrendAxisStep(
            (std::max)(
                10,
                RoundToInt(
                    ResolveTrendSeriesMax(
                        RealVacancyBars,
                        static_cast<float>(
                            (std::max)(
                                0,
                                Snapshot.JobCapacity -
                                    Snapshot.AssignedJobCount)))))) * 5;
    const int RealChangeAxisMax =
        ResolveTrendAxisStep(
            (std::max)(
                500,
                RoundToInt(
                    (std::max)(
                        ResolveTrendSeriesMax(RealDailyIncomeBars, 0.f),
                        ResolveTrendSeriesMax(RealDailyExpenseBars, 0.f))))) * 3;
    const std::array<int, GEconomyTrendYAxisLabelCount>
        RealTreasuryAxisLabels =
            BuildTrendAxisLabels(RealTreasuryAxisMax);
    const std::array<int, GEconomyTrendYAxisLabelCount>
        RealIncomeAxisLabels =
            BuildTrendAxisLabels(RealIncomeAxisMax);
    const std::array<int, GEconomyTrendYAxisLabelCount>
        RealExpenseAxisLabels =
            BuildTrendAxisLabels(RealExpenseAxisMax);
    const std::array<int, GEconomyTrendYAxisLabelCount>
        RealTradeAxisLabels =
            BuildTrendAxisLabels(RealTradeAxisMax);
    const std::array<int, GEconomyTrendYAxisLabelCount>
        RealUnemploymentCountAxisLabels =
            BuildTrendAxisLabels(RealUnemploymentCountAxisMax);
    const std::array<int, GEconomyTrendYAxisLabelCount>
        RealVacancyAxisLabels =
            BuildTrendAxisLabels(RealVacancyAxisMax);
    const std::array<int, GEconomyTrendYAxisLabelCount>
        RealPercentAxisLabels =
    {
        100, 80, 60, 40, 20, 0
    };

    std::vector<std::pair<std::wstring, int>> RealResourceRows;
    for (int ResourceIndex = 1;
        ResourceIndex < static_cast<int>(EResourceType::Count);
        ++ResourceIndex)
    {
        const auto& Resource =
            Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)];

        if (Resource.TotalStock <= 0)
            continue;

        RealResourceRows.emplace_back(
            GetResourceTypeDisplayName(Resource.Type),
            Resource.TotalStock);
    }
    std::sort(
        RealResourceRows.begin(),
        RealResourceRows.end(),
        [](const auto& A, const auto& B)
        {
            if (A.second != B.second)
                return A.second > B.second;

            return A.first < B.first;
        });

    if (EconomyPageSelectedIndex <= 5)
    {
        auto FormatInteger = [](long long Value)
        {
            const bool Negative = Value < 0;
            unsigned long long AbsValue = Negative ?
                static_cast<unsigned long long>(-Value) :
                static_cast<unsigned long long>(Value);
            std::wstring Digits = std::to_wstring(AbsValue);

            for (int Index = static_cast<int>(Digits.size()) - 3;
                Index > 0;
                Index -= 3)
            {
                Digits.insert(static_cast<size_t>(Index), 1, L',');
            }

            if (Negative)
                Digits.insert(Digits.begin(), L'-');

            return Digits;
        };

        auto SetMetricVisibility =
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

        auto SetDetailVisibility =
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

        auto SetMetricSummaryStyle =
            [](const CAlmanacWidget::FMetricRowWidgets& Row,
                const FVector4& Tint,
                bool BrightText)
        {
            if (auto Background = Row.Background.lock())
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(Tint);
            }
            if (auto Label = Row.Label.lock())
            {
                Label->SetTextColor(
                    BrightText ? 248 : 58,
                    BrightText ? 242 : 56,
                    BrightText ? 226 : 42,
                    255);
            }
            if (auto Value = Row.Value.lock())
            {
                Value->SetTextColor(
                    BrightText ? 248 : 58,
                    BrightText ? 242 : 56,
                    BrightText ? 226 : 42,
                    255);
            }
        };

        auto SetBreakdownTitle =
            [&Widget](const wchar_t* Title)
        {
            if (auto BreakdownTitleBackground =
                Widget.mEconomyBreakdownTitleBackground.lock())
            {
                BreakdownTitleBackground->SetEnable(true);
            }
            if (auto BreakdownTitle = Widget.mEconomyBreakdownTitle.lock())
            {
                BreakdownTitle->SetEnable(true);
                BreakdownTitle->SetText(Title);
            }
        };

        auto SetBreakdownRow =
            [&Widget, &SetDetailVisibility](int Index,
                const wchar_t* LabelText,
                const std::wstring& ValueText,
                bool Header)
        {
            if (Index < 0 ||
                Index >= static_cast<int>(Widget.mEconomyBreakdownRows.size()))
            {
                return;
            }

            auto& Row =
                Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)];
            SetDetailRowData(
                Row,
                LabelText,
                ValueText,
                false);
            SetDetailVisibility(Row, true);

            if (auto Background = Row.Background.lock())
            {
                Background->SetTexture(
                    Background->GetName() + "_base",
                    GRowTexture);
                Background->SetTint(
                    Header ? 0.96f : 1.f,
                    Header ? 0.95f : 1.f,
                    Header ? 0.89f : 1.f,
                    0.96f);
            }
            if (auto Label = Row.Label.lock())
            {
                Label->SetTextColor(
                    Header ? 92 : 58,
                    Header ? 84 : 56,
                    Header ? 66 : 42,
                    255);
            }
            if (auto Value = Row.Value.lock())
            {
                Value->SetTextColor(
                    Header ? 92 : 58,
                    Header ? 84 : 56,
                    Header ? 66 : 42,
                    255);
            }
        };

        const int CurrentTourismRating = LatestDailyEconomyRecord.TourismRating;
        const int PowerSurplusMW =
            Snapshot.TotalProducedPowerMW - Snapshot.TotalRequiredPowerMW;
        const int CurrentJobOccupancyPercent =
            Snapshot.JobCapacity > 0 ?
                RoundToInt(
                    static_cast<double>(Snapshot.AssignedJobCount) * 100.0 /
                    static_cast<double>(Snapshot.JobCapacity)) :
                0;

        SetDetailRowData(
            Widget.mEconomyDetails[0],
            L"국고",
            FormatCompactCurrency(Snapshot.NationalBudget),
            EconomyPageSelectedIndex == 0);
        SetDetailRowData(
            Widget.mEconomyDetails[1],
            L"수익 (최근 12개월)",
            FormatCompactCurrency(RealAnnualIncome),
            EconomyPageSelectedIndex == 1);
        SetDetailRowData(
            Widget.mEconomyDetails[2],
            L"경비 (최근 12개월)",
            FormatCompactCurrency(RealAnnualExpense),
            EconomyPageSelectedIndex == 2);
        SetDetailRowData(
            Widget.mEconomyDetails[3],
            L"무역 수지",
            FormatCompactCurrency(RealAnnualTradeBalance),
            EconomyPageSelectedIndex == 3);
        SetDetailRowData(
            Widget.mEconomyDetails[4],
            L"징세 효율",
            FormatPercent(RealCurrentTaxEfficiencyPercent),
            EconomyPageSelectedIndex == 4);
        SetDetailRowData(
            Widget.mEconomyDetails[5],
            L"자원 재고",
            FormatInteger(Snapshot.TotalResourceStock),
            EconomyPageSelectedIndex == 5);
        SetDetailRowData(
            Widget.mEconomyDetails[6],
            L"현재 관광객",
            FormatInteger(Snapshot.ActiveTouristCount),
            false);
        SetDetailRowData(
            Widget.mEconomyDetails[7],
            L"관광객 평가",
            FormatInteger(CurrentTourismRating),
            false);
        SetDetailRowData(
            Widget.mEconomyDetails[8],
            L"관광객 수용력",
            FormatInteger(Snapshot.TourismVisitOccupancy) + L"/" +
                FormatInteger(Snapshot.TourismVisitCapacity),
            false);
        SetDetailRowData(
            Widget.mEconomyDetails[9],
            L"노동력",
            FormatInteger(CurrentJobOccupancyPercent) + L"%",
            false);
        SetDetailRowData(
            Widget.mEconomyDetails[10],
            L"실업자",
            FormatInteger(Snapshot.UnemployedCount),
            false);
        SetDetailRowData(
            Widget.mEconomyDetails[11],
            L"빈 일자리",
            FormatInteger((std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount)),
            false);
        SetDetailRowData(
            Widget.mEconomyDetails[12],
            L"전기",
            FormatInteger(PowerSurplusMW) + L"MW",
            false);

        for (size_t Index = 0; Index < Widget.mEconomyMetrics.size(); ++Index)
            SetMetricVisibility(Widget.mEconomyMetrics[Index], false);
        for (size_t Index = 0; Index < Widget.mEconomyBreakdownRows.size(); ++Index)
            SetDetailVisibility(Widget.mEconomyBreakdownRows[Index], false);
        if (auto BreakdownTitleBackground =
            Widget.mEconomyBreakdownTitleBackground.lock())
        {
            BreakdownTitleBackground->SetEnable(false);
        }
        if (auto BreakdownTitle = Widget.mEconomyBreakdownTitle.lock())
            BreakdownTitle->SetEnable(false);
        if (auto TrendTitleBackground =
            Widget.mEconomyTrendTitleBackground.lock())
        {
            TrendTitleBackground->SetEnable(true);
        }
        for (size_t Index = 0; Index < Widget.mEconomyTrendLines.size(); ++Index)
        {
            if (auto Line = Widget.mEconomyTrendLines[Index].lock())
                Line->SetEnable(false);
        }
        for (size_t Index = 0; Index < Widget.mEconomyTrendBars.size(); ++Index)
        {
            if (auto Bar = Widget.mEconomyTrendBars[Index].lock())
                Bar->SetEnable(false);
        }
        for (size_t Index = 0; Index < Widget.mEconomyTrendSecondaryBars.size(); ++Index)
        {
            if (auto Bar = Widget.mEconomyTrendSecondaryBars[Index].lock())
                Bar->SetEnable(false);
        }
        for (size_t Index = 0; Index < Widget.mEconomyTrendTertiaryBars.size(); ++Index)
        {
            if (auto Bar = Widget.mEconomyTrendTertiaryBars[Index].lock())
                Bar->SetEnable(false);
        }
        for (size_t Index = 0; Index < Widget.mEconomyTrendYAxisLabels.size(); ++Index)
        {
            if (auto Label = Widget.mEconomyTrendYAxisLabels[Index].lock())
                Label->SetEnable(false);
        }
        for (size_t Index = 0; Index < Widget.mEconomyChangePositiveBars.size(); ++Index)
        {
            if (auto Bar = Widget.mEconomyChangePositiveBars[Index].lock())
                Bar->SetEnable(false);
        }
        for (size_t Index = 0; Index < Widget.mEconomyChangeNegativeBars.size(); ++Index)
        {
            if (auto Bar = Widget.mEconomyChangeNegativeBars[Index].lock())
                Bar->SetEnable(false);
        }
        for (size_t Index = 0; Index < Widget.mEconomyChangeYAxisLabels.size(); ++Index)
        {
            if (auto Label = Widget.mEconomyChangeYAxisLabels[Index].lock())
                Label->SetEnable(false);
        }

        auto SetTrendLabels =
            [&Widget](const std::array<int, GEconomyTrendYAxisLabelCount>& Labels,
                bool Percent)
        {
            for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
            {
                if (Index >=
                    static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                {
                    break;
                }

                if (auto Label =
                    Widget.mEconomyTrendYAxisLabels[
                        static_cast<size_t>(Index)].lock())
                {
                    Label->SetEnable(true);
                    Label->SetText(
                        Percent ?
                            (std::to_wstring(
                                Labels[static_cast<size_t>(Index)]) + L"%").c_str() :
                            std::to_wstring(
                                Labels[static_cast<size_t>(Index)]).c_str());
                }
            }
        };

        auto DrawTrendBars =
            [&Widget](const auto& Values,
                float MaxValue,
                const FVector4& Tint)
        {
            auto Frame = Widget.mEconomyTrendFrame.lock();

            if (!Frame || MaxValue <= 0.f)
                return;

            const float GraphLeft = Frame->GetPos().x + 22.f;
            const float GraphTop = Frame->GetPos().y + 14.f;
            const float GraphWidth = Frame->GetSize().x - 40.f;
            const float GraphHeight = Frame->GetSize().y - 32.f;
            const float GroupWidth =
                GraphWidth / static_cast<float>((std::max)(1, GEconomyTrendBarCount));
            const float SingleWidth = (std::max)(4.f, GroupWidth * 0.66f);

            for (int Index = 0; Index < GEconomyTrendBarCount; ++Index)
            {
                if (Index >= static_cast<int>(Widget.mEconomyTrendBars.size()))
                    break;

                if (auto Bar =
                    Widget.mEconomyTrendBars[static_cast<size_t>(Index)].lock())
                {
                    const float Height =
                        GraphHeight *
                        Clamp01(Values[static_cast<size_t>(Index)] / MaxValue);
                    const float BarX =
                        GraphLeft + GroupWidth * static_cast<float>(Index) +
                        (GroupWidth - SingleWidth) * 0.5f;
                    Bar->SetTint(Tint);
                    Bar->SetEnable(Height > 0.f);
                    Bar->SetPos(BarX, GraphTop + GraphHeight - Height);
                    Bar->SetSize(SingleWidth, (std::max)(2.f, Height));
                }
            }
        };

        if (auto TrendTitle = Widget.mEconomyTrendTitle.lock())
        {
            TrendTitle->SetText(
                EconomyPageSelectedIndex == 0 ? L"국고 추세" :
                EconomyPageSelectedIndex == 1 ? L"수익 추세" :
                EconomyPageSelectedIndex == 2 ? L"경비 추세" :
                EconomyPageSelectedIndex == 3 ? L"무역 흐름" :
                EconomyPageSelectedIndex == 4 ? L"징세 효율" :
                    L"자원 재고");
        }

        const wchar_t* MonthlyLabels[GEconomyTrendXAxisLabelCount] =
        {
            L"24개월전", L"18개월전", L"12개월전", L"6개월전", L"이번달"
        };
        const wchar_t* DailyLabels[GEconomyTrendXAxisLabelCount] =
        {
            L"24일전", L"18일전", L"12일전", L"6일전", L"오늘"
        };
        const bool UseDailyLabels = EconomyPageSelectedIndex == 4;
        for (int Index = 0; Index < GEconomyTrendXAxisLabelCount; ++Index)
        {
            if (Index >=
                static_cast<int>(Widget.mEconomyTrendXAxisLabels.size()))
            {
                break;
            }

            if (auto Label =
                Widget.mEconomyTrendXAxisLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetEnable(EconomyPageSelectedIndex != 5);
                if (EconomyPageSelectedIndex != 5)
                    Label->SetText(UseDailyLabels ? DailyLabels[Index] : MonthlyLabels[Index]);
            }
        }

        if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
            TrendFrame->SetEnable(EconomyPageSelectedIndex != 5);
        if (auto TrendYAxisLine = Widget.mEconomyTrendYAxisLine.lock())
            TrendYAxisLine->SetEnable(EconomyPageSelectedIndex != 5);
        if (auto TrendXAxisLine = Widget.mEconomyTrendXAxisLine.lock())
            TrendXAxisLine->SetEnable(EconomyPageSelectedIndex != 5);
        if (auto TrendYAxisArrow = Widget.mEconomyTrendYAxisArrow.lock())
            TrendYAxisArrow->SetEnable(EconomyPageSelectedIndex != 5);
        if (auto TrendXAxisArrow = Widget.mEconomyTrendXAxisArrow.lock())
            TrendXAxisArrow->SetEnable(EconomyPageSelectedIndex != 5);
        for (size_t Index = 0; Index < Widget.mEconomyTrendGridLines.size(); ++Index)
        {
            if (auto GridLine = Widget.mEconomyTrendGridLines[Index].lock())
                GridLine->SetEnable(EconomyPageSelectedIndex != 5);
        }

        if (auto ChangeFrame = Widget.mEconomyChangeFrame.lock())
            ChangeFrame->SetEnable(false);
        if (auto ChangeYAxisLine = Widget.mEconomyChangeYAxisLine.lock())
            ChangeYAxisLine->SetEnable(false);
        if (auto ChangeXAxisLine = Widget.mEconomyChangeXAxisLine.lock())
            ChangeXAxisLine->SetEnable(false);
        if (auto ChangeYAxisArrow = Widget.mEconomyChangeYAxisArrow.lock())
            ChangeYAxisArrow->SetEnable(false);
        if (auto ChangeXAxisArrow = Widget.mEconomyChangeXAxisArrow.lock())
            ChangeXAxisArrow->SetEnable(false);
        for (size_t Index = 0; Index < Widget.mEconomyChangeGridLines.size(); ++Index)
        {
            if (auto GridLine = Widget.mEconomyChangeGridLines[Index].lock())
                GridLine->SetEnable(false);
        }

        auto DrawChangeBars =
            [&Widget](const auto& PositiveValues,
                const auto& NegativeValues,
                int AxisMax)
        {
            if (AxisMax <= 0)
                return;

            if (auto ChangeFrame = Widget.mEconomyChangeFrame.lock())
                ChangeFrame->SetEnable(true);
            if (auto ChangeYAxisLine = Widget.mEconomyChangeYAxisLine.lock())
                ChangeYAxisLine->SetEnable(true);
            if (auto ChangeXAxisLine = Widget.mEconomyChangeXAxisLine.lock())
                ChangeXAxisLine->SetEnable(true);
            if (auto ChangeYAxisArrow = Widget.mEconomyChangeYAxisArrow.lock())
                ChangeYAxisArrow->SetEnable(true);
            if (auto ChangeXAxisArrow = Widget.mEconomyChangeXAxisArrow.lock())
                ChangeXAxisArrow->SetEnable(true);
            for (size_t Index = 0; Index < Widget.mEconomyChangeGridLines.size(); ++Index)
            {
                if (auto GridLine = Widget.mEconomyChangeGridLines[Index].lock())
                    GridLine->SetEnable(true);
            }

            const int Step = (std::max)(1, AxisMax / 3);
            const int Labels[GEconomyChangeYAxisLabelCount] =
            {
                Step * 3,
                Step * 2,
                Step,
                -Step,
                -Step * 2,
                -Step * 3
            };

            for (int Index = 0; Index < GEconomyChangeYAxisLabelCount; ++Index)
            {
                if (Index >=
                    static_cast<int>(Widget.mEconomyChangeYAxisLabels.size()))
                {
                    break;
                }

                if (auto Label =
                    Widget.mEconomyChangeYAxisLabels[
                        static_cast<size_t>(Index)].lock())
                {
                    Label->SetEnable(true);
                    Label->SetText(std::to_wstring(Labels[Index]).c_str());
                }
            }

            auto ChangeFrame = Widget.mEconomyChangeFrame.lock();
            if (!ChangeFrame)
                return;

            const float GraphLeft = ChangeFrame->GetPos().x + 22.f;
            const float GraphTop = ChangeFrame->GetPos().y + 12.f;
            const float GraphWidth = ChangeFrame->GetSize().x - 40.f;
            const float GraphHeight = ChangeFrame->GetSize().y - 26.f;
            const float ZeroY =
                ResolveGraphYInRange(
                    GraphTop,
                    GraphHeight,
                    0.f,
                    -static_cast<float>(AxisMax),
                    static_cast<float>(AxisMax));
            const float BarGroupWidth =
                GraphWidth / static_cast<float>((std::max)(1, GEconomyChangeBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.64f);
            const float MaxValue = static_cast<float>((std::max)(1, AxisMax));

            for (int Index = 0; Index < GEconomyChangeBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                    (BarGroupWidth - SingleBarWidth) * 0.5f;

                if (Index <
                    static_cast<int>(Widget.mEconomyChangePositiveBars.size()))
                {
                    if (auto PositiveBar =
                        Widget.mEconomyChangePositiveBars[
                            static_cast<size_t>(Index)].lock())
                    {
                        const float PositiveHeight =
                            GraphHeight *
                            Clamp01(
                                PositiveValues[static_cast<size_t>(Index)] /
                                MaxValue) * 0.48f;
                        PositiveBar->SetEnable(true);
                        PositiveBar->SetTint(0.28f, 0.46f, 0.78f, 0.94f);
                        PositiveBar->SetPos(BarX, ZeroY - PositiveHeight);
                        PositiveBar->SetSize(
                            SingleBarWidth,
                            (std::max)(2.f, PositiveHeight));
                    }
                }

                if (Index <
                    static_cast<int>(Widget.mEconomyChangeNegativeBars.size()))
                {
                    if (auto NegativeBar =
                        Widget.mEconomyChangeNegativeBars[
                            static_cast<size_t>(Index)].lock())
                    {
                        const float NegativeHeight =
                            GraphHeight *
                            Clamp01(
                                NegativeValues[static_cast<size_t>(Index)] /
                                MaxValue) * 0.48f;
                        NegativeBar->SetEnable(true);
                        NegativeBar->SetTint(0.84f, 0.34f, 0.30f, 0.92f);
                        NegativeBar->SetPos(BarX, ZeroY);
                        NegativeBar->SetSize(
                            SingleBarWidth,
                            (std::max)(2.f, NegativeHeight));
                    }
                }
            }
        };

        auto DrawDualTrendBars =
            [&Widget](const auto& PrimaryValues,
                const auto& SecondaryValues,
                float MaxValue,
                const FVector4& PrimaryTint,
                const FVector4& SecondaryTint)
        {
            auto Frame = Widget.mEconomyTrendFrame.lock();

            if (!Frame || MaxValue <= 0.f)
                return;

            const float GraphLeft = Frame->GetPos().x + 22.f;
            const float GraphTop = Frame->GetPos().y + 14.f;
            const float GraphWidth = Frame->GetSize().x - 40.f;
            const float GraphHeight = Frame->GetSize().y - 32.f;
            const float GroupWidth =
                GraphWidth / static_cast<float>((std::max)(1, GEconomyTrendBarCount));
            const float SingleWidth =
                (std::max)(3.f, GroupWidth * 0.30f);

            for (int Index = 0; Index < GEconomyTrendBarCount; ++Index)
            {
                const float BaseX =
                    GraphLeft + GroupWidth * static_cast<float>(Index) +
                    (GroupWidth - SingleWidth * 2.f - 2.f) * 0.5f;
                const float PrimaryHeight =
                    GraphHeight *
                    Clamp01(
                        PrimaryValues[static_cast<size_t>(Index)] /
                        MaxValue);
                const float SecondaryHeight =
                    GraphHeight *
                    Clamp01(
                        SecondaryValues[static_cast<size_t>(Index)] /
                        MaxValue);

                if (Index < static_cast<int>(Widget.mEconomyTrendBars.size()))
                {
                    if (auto Bar =
                        Widget.mEconomyTrendBars[
                            static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetTint(PrimaryTint);
                        Bar->SetEnable(PrimaryHeight > 0.f);
                        Bar->SetPos(
                            BaseX,
                            GraphTop + GraphHeight - PrimaryHeight);
                        Bar->SetSize(
                            SingleWidth,
                            (std::max)(2.f, PrimaryHeight));
                    }
                }

                if (Index <
                    static_cast<int>(Widget.mEconomyTrendSecondaryBars.size()))
                {
                    if (auto Bar =
                        Widget.mEconomyTrendSecondaryBars[
                            static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetTint(SecondaryTint);
                        Bar->SetEnable(SecondaryHeight > 0.f);
                        Bar->SetPos(
                            BaseX + SingleWidth + 2.f,
                            GraphTop + GraphHeight - SecondaryHeight);
                        Bar->SetSize(
                            SingleWidth,
                            (std::max)(2.f, SecondaryHeight));
                    }
                }
            }
        };

        if (EconomyPageSelectedIndex == 0)
        {
            SetTrendLabels(RealTreasuryAxisLabels, false);
            DrawTrendBars(
                RealTreasuryBars,
                static_cast<float>((std::max)(1, RealTreasuryAxisMax)),
                FVector4(0.12f, 0.82f, 0.38f, 0.95f));
            DrawChangeBars(
                RealDailyIncomeBars,
                RealDailyExpenseBars,
                (std::max)(1, RealChangeAxisMax));

            SetMetricRowData(
                Widget.mEconomyMetrics[0],
                L"현재 국고",
                FormatCompactCurrency(Snapshot.NationalBudget),
                0.f,
                FVector4(0.10f, 0.72f, 0.32f, 0.95f),
                false);
            SetMetricRowData(
                Widget.mEconomyMetrics[1],
                L"전일 순변동",
                FormatCurrency(LatestDailyEconomyRecord.NetBudgetChange),
                0.f,
                FVector4(0.90f, 0.88f, 0.80f, 0.96f),
                false);
            SetMetricRowData(
                Widget.mEconomyMetrics[2],
                L"수익 (전일)",
                FormatCurrency(RealDailyIncome),
                0.f,
                FVector4(0.28f, 0.46f, 0.78f, 0.94f),
                false);
            SetMetricRowData(
                Widget.mEconomyMetrics[3],
                L"경비 (전일)",
                FormatCurrency(RealDailyExpense),
                0.f,
                FVector4(0.76f, 0.31f, 0.28f, 0.94f),
                false);
            for (int Index = 0; Index < 4; ++Index)
            {
                SetMetricVisibility(
                    Widget.mEconomyMetrics[static_cast<size_t>(Index)],
                    true);
            }
            SetMetricSummaryStyle(
                Widget.mEconomyMetrics[0],
                FVector4(0.10f, 0.72f, 0.32f, 0.95f),
                false);
            SetMetricSummaryStyle(
                Widget.mEconomyMetrics[2],
                FVector4(0.28f, 0.46f, 0.78f, 0.94f),
                false);
            SetMetricSummaryStyle(
                Widget.mEconomyMetrics[3],
                FVector4(0.76f, 0.31f, 0.28f, 0.94f),
                false);

            SetBreakdownTitle(L"전일 정산 명세");
            SetBreakdownRow(0, L"수출", FormatCurrency(LatestDailyEconomyRecord.ExportIncome), false);
            SetBreakdownRow(1, L"세금 합계", FormatCurrency(RealDailyTaxIncome), false);
            SetBreakdownRow(
                2,
                L"무역정책 보정",
                FormatCurrency(LatestDailyEconomyRecord.TradePolicyBudgetDelta),
                false);
            SetBreakdownRow(3, L"수입", FormatCurrency(LatestDailyEconomyRecord.ImportExpense), false);
            SetBreakdownRow(
                4,
                L"임금 + 유지비",
                FormatCurrency(
                    LatestDailyEconomyRecord.WageCost +
                    LatestDailyEconomyRecord.UpkeepCost),
                false);
            SetBreakdownRow(5, L"칙령 유지비", FormatCurrency(LatestDailyEconomyRecord.EdictUpkeepCost), false);
            SetBreakdownRow(
                6,
                L"건설 + 수리 + 칙령 발동",
                FormatCurrency(RealDailyDirectExpense),
                false);
            SetBreakdownRow(7, L"특별/기타 유입", FormatCurrency(RealDailySpecialIncome), false);
            SetBreakdownRow(8, L"특별/기타 유출", FormatCurrency(RealDailySpecialExpense), false);
            SetBreakdownRow(9, L"오늘 순변동", FormatCurrency(LatestDailyEconomyRecord.NetBudgetChange), true);
        }
        else if (EconomyPageSelectedIndex == 1)
        {
            SetTrendLabels(RealIncomeAxisLabels, false);
            DrawTrendBars(
                RealMonthlyIncomeBars,
                static_cast<float>((std::max)(1, RealIncomeAxisMax)),
                FVector4(0.28f, 0.46f, 0.78f, 0.94f));
            SetMetricRowData(
                Widget.mEconomyMetrics[0],
                L"수익 (최근 12개월)",
                FormatCompactCurrency(RealAnnualIncome),
                0.f,
                FVector4(0.28f, 0.46f, 0.78f, 0.94f),
                false);
            SetMetricVisibility(Widget.mEconomyMetrics[0], true);
            SetMetricSummaryStyle(
                Widget.mEconomyMetrics[0],
                FVector4(0.28f, 0.46f, 0.78f, 0.94f),
                false);

            SetBreakdownTitle(L"수익 명세");
            SetBreakdownRow(0, L"수출", FormatCurrency(RealAnnualExportIncome), false);
            SetBreakdownRow(1, L"소비세", FormatCurrency(RealAnnualConsumptionTaxIncome), false);
            SetBreakdownRow(2, L"소득세", FormatCurrency(RealAnnualIncomeTaxIncome), false);
            SetBreakdownRow(3, L"재산세", FormatCurrency(RealAnnualPropertyTaxIncome), false);
            SetBreakdownRow(4, L"세금 정책 효과", FormatCurrency(RealAnnualTaxBudgetDelta), false);
            SetBreakdownRow(5, L"무역정책 보정", FormatCurrency(RealAnnualTradePolicyBudgetDelta), false);
            SetBreakdownRow(6, L"특별 이벤트", FormatCurrency(RealAnnualSpecialEventIncome), false);
            SetBreakdownRow(7, L"기타 직접 유입", FormatCurrency(RealAnnualOtherDirectIncome), false);
            SetBreakdownRow(8, L"세금 합계", FormatCurrency(
                RealAnnualConsumptionTaxIncome +
                RealAnnualIncomeTaxIncome +
                RealAnnualPropertyTaxIncome), false);
            SetBreakdownRow(9, L"총수익", FormatCurrency(RealAnnualIncome), true);
        }
        else if (EconomyPageSelectedIndex == 2)
        {
            SetTrendLabels(RealExpenseAxisLabels, false);
            DrawTrendBars(
                RealMonthlyExpenseBars,
                static_cast<float>((std::max)(1, RealExpenseAxisMax)),
                FVector4(0.84f, 0.34f, 0.30f, 0.92f));
            SetMetricRowData(
                Widget.mEconomyMetrics[0],
                L"경비 (최근 12개월)",
                FormatCompactCurrency(RealAnnualExpense),
                0.f,
                FVector4(0.84f, 0.34f, 0.30f, 0.92f),
                false);
            SetMetricVisibility(Widget.mEconomyMetrics[0], true);
            SetMetricSummaryStyle(
                Widget.mEconomyMetrics[0],
                FVector4(0.84f, 0.34f, 0.30f, 0.92f),
                false);

            SetBreakdownTitle(L"경비 명세");
            SetBreakdownRow(0, L"수입", FormatCurrency(RealAnnualImportExpense), false);
            SetBreakdownRow(1, L"임금", FormatCurrency(RealAnnualWageExpense), false);
            SetBreakdownRow(2, L"유지비", FormatCurrency(RealAnnualUpkeepExpense), false);
            SetBreakdownRow(3, L"칙령 유지비", FormatCurrency(RealAnnualEdictUpkeepExpense), false);
            SetBreakdownRow(4, L"건설", FormatCurrency(RealAnnualConstructionExpense), false);
            SetBreakdownRow(5, L"수리", FormatCurrency(RealAnnualRepairExpense), false);
            SetBreakdownRow(6, L"칙령 발동비", FormatCurrency(RealAnnualEdictActivationExpense), false);
            SetBreakdownRow(7, L"특별 이벤트", FormatCurrency(RealAnnualSpecialEventExpense), false);
            SetBreakdownRow(8, L"기타 직접 유출", FormatCurrency(RealAnnualOtherDirectExpense), false);
            SetBreakdownRow(9, L"총경비", FormatCurrency(RealAnnualExpense), true);
        }
        else if (EconomyPageSelectedIndex == 3)
        {
            SetTrendLabels(RealTradeAxisLabels, false);
            DrawDualTrendBars(
                RealMonthlyExportBars,
                RealMonthlyImportBars,
                static_cast<float>((std::max)(1, RealTradeAxisMax)),
                FVector4(0.28f, 0.46f, 0.78f, 0.94f),
                FVector4(0.84f, 0.34f, 0.30f, 0.92f));
            SetMetricRowData(
                Widget.mEconomyMetrics[0],
                L"무역 수지 (최근 12개월)",
                FormatCompactCurrency(RealAnnualTradeBalance),
                0.f,
                FVector4(0.28f, 0.46f, 0.78f, 0.94f),
                false);
            SetMetricVisibility(Widget.mEconomyMetrics[0], true);
            SetMetricSummaryStyle(
                Widget.mEconomyMetrics[0],
                FVector4(0.28f, 0.46f, 0.78f, 0.94f),
                false);

            SetBreakdownTitle(L"무역 명세");
            SetBreakdownRow(0, L"수출 합계", FormatCurrency(RealAnnualExportIncome), false);
            SetBreakdownRow(1, L"수입 합계", FormatCurrency(RealAnnualImportExpense), false);
            SetBreakdownRow(2, L"무역 수지", FormatCurrency(RealAnnualTradeBalance), false);
            SetBreakdownRow(3, L"무역정책 보정", FormatCurrency(RealAnnualTradePolicyBudgetDelta), false);
            SetBreakdownRow(4, L"전일 수출", FormatCurrency(LatestDailyEconomyRecord.ExportIncome), false);
            SetBreakdownRow(5, L"전일 수입", FormatCurrency(LatestDailyEconomyRecord.ImportExpense), false);
        }
        else if (EconomyPageSelectedIndex == 4)
        {
            SetTrendLabels(RealPercentAxisLabels, true);
            DrawTrendBars(
                RealTaxEfficiencyBars,
                100.f,
                FVector4(0.72f, 0.56f, 0.54f, 0.92f));
            SetMetricRowData(
                Widget.mEconomyMetrics[0],
                L"현재 징세 효율",
                FormatPercent(RealCurrentTaxEfficiencyPercent),
                0.f,
                FVector4(0.72f, 0.56f, 0.54f, 0.92f),
                false);
            SetMetricRowData(
                Widget.mEconomyMetrics[1],
                L"전월 평균",
                FormatPercent(RealPreviousMonthTaxEfficiencyPercent),
                0.f,
                FVector4(0.72f, 0.56f, 0.54f, 0.92f),
                false);
            SetMetricVisibility(Widget.mEconomyMetrics[0], true);
            SetMetricVisibility(Widget.mEconomyMetrics[1], true);
            SetMetricSummaryStyle(
                Widget.mEconomyMetrics[0],
                FVector4(0.72f, 0.56f, 0.54f, 0.92f),
                false);
            SetMetricSummaryStyle(
                Widget.mEconomyMetrics[1],
                FVector4(0.72f, 0.56f, 0.54f, 0.92f),
                false);

            SetBreakdownTitle(L"세금 명세");
            SetBreakdownRow(0, L"소비세 (최근 12개월)", FormatCurrency(RealAnnualConsumptionTaxIncome), false);
            SetBreakdownRow(1, L"소득세 (최근 12개월)", FormatCurrency(RealAnnualIncomeTaxIncome), false);
            SetBreakdownRow(2, L"재산세 (최근 12개월)", FormatCurrency(RealAnnualPropertyTaxIncome), false);
            SetBreakdownRow(3, L"세금 합계", FormatCurrency(
                RealAnnualConsumptionTaxIncome +
                RealAnnualIncomeTaxIncome +
                RealAnnualPropertyTaxIncome), false);
            SetBreakdownRow(4, L"세금 정책 효과", FormatCurrency(RealAnnualTaxBudgetDelta), false);
            SetBreakdownRow(5, L"현재 징세 효율", FormatPercent(RealCurrentTaxEfficiencyPercent), false);
            SetBreakdownRow(6, L"전월 평균", FormatPercent(RealPreviousMonthTaxEfficiencyPercent), false);
        }
        else
        {
            if (auto BreakdownTitleBackground =
                Widget.mEconomyBreakdownTitleBackground.lock())
            {
                BreakdownTitleBackground->SetEnable(true);
            }
            if (auto BreakdownTitle = Widget.mEconomyBreakdownTitle.lock())
            {
                BreakdownTitle->SetEnable(true);
                BreakdownTitle->SetText(L"총 자원 재고");
            }
            if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
                TrendFrame->SetEnable(false);
            if (auto TrendYAxisLine = Widget.mEconomyTrendYAxisLine.lock())
                TrendYAxisLine->SetEnable(false);
            if (auto TrendXAxisLine = Widget.mEconomyTrendXAxisLine.lock())
                TrendXAxisLine->SetEnable(false);
            if (auto TrendYAxisArrow = Widget.mEconomyTrendYAxisArrow.lock())
                TrendYAxisArrow->SetEnable(false);
            if (auto TrendXAxisArrow = Widget.mEconomyTrendXAxisArrow.lock())
                TrendXAxisArrow->SetEnable(false);
            for (size_t Index = 0; Index < Widget.mEconomyTrendGridLines.size(); ++Index)
            {
                if (auto GridLine = Widget.mEconomyTrendGridLines[Index].lock())
                    GridLine->SetEnable(false);
            }
            for (int Index = 0;
                Index < static_cast<int>(RealResourceRows.size()) &&
                Index < static_cast<int>(Widget.mEconomyBreakdownRows.size()) &&
                Index < 8;
                ++Index)
            {
                SetDetailRowData(
                    Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)],
                    RealResourceRows[static_cast<size_t>(Index)].first,
                    FormatInteger(RealResourceRows[static_cast<size_t>(Index)].second),
                    false);
                SetDetailVisibility(
                    Widget.mEconomyBreakdownRows[static_cast<size_t>(Index)],
                    true);
            }
        }

        return;
    }

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
        RealVacancyBars;
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
    auto GetTouristProfileCount = [&Snapshot](ETouristPreference Preference)
    {
        const int PreferenceIndex = static_cast<int>(Preference);
        if (PreferenceIndex < 0 || PreferenceIndex >= GTouristPreferenceCount)
            return 0;

        return (std::max)(0, Snapshot.TouristProfileCount[PreferenceIndex]);
    };

    const int CurrentTouristCount = (std::max)(0, Snapshot.ActiveTouristCount);
    const int CulturalTouristCount =
        GetTouristProfileCount(ETouristPreference::Cultural);
    const int FamilyTouristCount =
        GetTouristProfileCount(ETouristPreference::Family);
    const int BackpackerTouristCount =
        GetTouristProfileCount(ETouristPreference::Backpacker);
    const int RelaxationTouristCount =
        GetTouristProfileCount(ETouristPreference::Relaxation);
    const int ThrillTouristCount =
        GetTouristProfileCount(ETouristPreference::ThrillSeeker);
    const int VipTouristCount =
        GetTouristProfileCount(ETouristPreference::Celebrity);
    const int TourismVisitCapacity = (std::max)(0, Snapshot.TourismVisitCapacity);
    const int TourismVisitOccupancy = (std::max)(0, Snapshot.TourismVisitOccupancy);
    const int TourismVisitSlack =
        (std::max)(0, TourismVisitCapacity - TourismVisitOccupancy);
    const int TouristPreferenceMatchedCount =
        (std::max)(0, Snapshot.TouristPreferenceMatchedCount);
    const int TouristPreferenceMatchedPercent =
        CurrentTouristCount > 0 ?
            (std::min)(
                100,
                (std::max)(
                    0,
                    RoundToInt(
                        static_cast<double>(TouristPreferenceMatchedCount) * 100.0 /
                        static_cast<double>(CurrentTouristCount)))) :
            0;
    const int TourismVisitUtilizationPercent =
        TourismVisitCapacity > 0 ?
            (std::min)(
                100,
                (std::max)(
                    0,
                    RoundToInt(
                        static_cast<double>(TourismVisitOccupancy) * 100.0 /
                        static_cast<double>(TourismVisitCapacity)))) :
            0;
    int TourismProfileDiversityCount = 0;
    for (int PreferenceIndex = 1; PreferenceIndex < GTouristPreferenceCount; ++PreferenceIndex)
    {
        if (Snapshot.TouristProfileCount[PreferenceIndex] > 0)
            ++TourismProfileDiversityCount;
    }

    const int TourismRating =
        Snapshot.TourismBuildingCount > 0 || CurrentTouristCount > 0 ?
            (std::min)(
                99,
                (std::max)(
                    0,
                    18 +
                        (std::min)(28, Snapshot.TourismBuildingCount * 4) +
                        (std::min)(18, TourismProfileDiversityCount * 3) +
                        (std::min)(22, TouristPreferenceMatchedPercent / 3) +
                        (std::min)(14, TourismVisitUtilizationPercent / 6) +
                        (std::min)(8, Snapshot.HarborCount * 2))) :
            0;

    const int CurrentTouristAxisMax =
        ResolveTrendAxisStep(
            (std::max)(
                25,
                RoundToInt(
                    ResolveTrendSeriesMax(RealTouristCountBars, 0.f)))) * 5;
    const int TourismCapacityAxisMax =
        ResolveTrendAxisStep(
            (std::max)(
                50,
                RoundToInt(
                    (std::max)(
                        ResolveTrendSeriesMax(RealTourismCapacityBars, 0.f),
                        ResolveTrendSeriesMax(RealTourismOccupancyBars, 0.f))))) * 5;
    const std::array<int, GEconomyTrendYAxisLabelCount> CurrentTouristAxisLabels =
        BuildTrendAxisLabels(CurrentTouristAxisMax);
    const std::array<int, GEconomyTrendYAxisLabelCount> TourismCapacityAxisLabels =
        BuildTrendAxisLabels(TourismCapacityAxisMax);
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
        RealTouristCountBars;
    const std::array<float, GEconomyTrendBarCount> TourismRatingTrend =
        RealTourismRatingBars;
    const std::array<float, GEconomyTrendBarCount> TourismCapacityTrend =
        RealTourismCapacityBars;
    const std::array<float, GEconomyTrendBarCount> TourismArrivalTrend =
        RealTourismOccupancyBars;
    const std::array<float, GEconomyTrendBarCount> EconomyJobOccupancyTrend =
        RealJobOccupancyBars;
    const std::array<float, GEconomyTrendBarCount> EconomyUnemploymentTrend =
        RealUnemploymentPercentBars;
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
        L"24일전",
        L"18일전",
        L"12일전",
        L"6일전",
        L"오늘"
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
        L"수익 (최근 12개월)",
        FormatCompactCurrency(RealAnnualIncome),
        SelectedEconomyIndex == 1);
    SetDetailRowData(
        Widget.mEconomyDetails[2],
        L"경비 (최근 12개월)",
        FormatCompactCurrency(RealAnnualExpense),
        SelectedEconomyIndex == 2);
    SetDetailRowData(
        Widget.mEconomyDetails[3],
        L"무역 수지",
        FormatCompactCurrency(RealAnnualTradeBalance),
        SelectedEconomyIndex == 3);
    SetDetailRowData(
        Widget.mEconomyDetails[4],
        L"징세 효율",
        FormatPercent(RealCurrentTaxEfficiencyPercent),
        SelectedEconomyIndex == 4);
    SetDetailRowData(
        Widget.mEconomyDetails[5],
        L"자원 재고",
        FormatInteger(Snapshot.TotalResourceStock),
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
        FormatInteger(TourismVisitOccupancy) + L"/" + FormatInteger(TourismVisitCapacity),
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
        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                YLabel->SetEnable(Index < 5);
                if (Index < 5)
                {
                    YLabel->SetText(
                        std::to_wstring(
                            CurrentTouristAxisLabels[static_cast<size_t>(Index)]).c_str());
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
                (std::max)(4.f, BarGroupWidth * 0.54f);
            const float MaxValue = static_cast<float>((std::max)(1, CurrentTouristAxisMax));

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
            100,
            80,
            60,
            40,
            20,
            0
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
                        0.f,
                        100.f);
                const float Y1 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        TourismRatingTrend[static_cast<size_t>(SegmentIndex + 1)],
                        0.f,
                        100.f);
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
        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(
                    std::to_wstring(
                        TourismCapacityAxisLabels[static_cast<size_t>(Index)]).c_str());
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
                        static_cast<float>((std::max)(1, TourismCapacityAxisMax)));
                const float CapacityY1 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        TourismCapacityTrend[static_cast<size_t>(SegmentIndex + 1)],
                        0.f,
                        static_cast<float>((std::max)(1, TourismCapacityAxisMax)));
                const float ArrivalY0 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        TourismArrivalTrend[static_cast<size_t>(SegmentIndex)],
                        0.f,
                        static_cast<float>((std::max)(1, TourismCapacityAxisMax)));
                const float ArrivalY1 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        TourismArrivalTrend[static_cast<size_t>(SegmentIndex + 1)],
                        0.f,
                        static_cast<float>((std::max)(1, TourismCapacityAxisMax)));

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
                YLabel->SetEnable(true);
                YLabel->SetText(
                    std::to_wstring(
                        RealUnemploymentCountAxisLabels[
                            static_cast<size_t>(Index)]).c_str());
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
            const float MaxValue =
                static_cast<float>((std::max)(1, RealUnemploymentCountAxisMax));

            for (int Index = 0; Index < GEconomyTrendBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                    (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float TotalHeight =
                    GraphHeight *
                    Clamp01(RealUnemployedCountBars[static_cast<size_t>(Index)] / MaxValue);
                const float BarTop = GraphTop + GraphHeight - TotalHeight;

                if (Index < static_cast<int>(Widget.mEconomyTrendBars.size()))
                {
                    if (auto Bar = Widget.mEconomyTrendBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetTint(0.74f, 0.22f, 0.18f, 0.94f);
                        Bar->SetEnable(TotalHeight > 0.f);
                        Bar->SetPos(BarX, BarTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, TotalHeight));
                    }
                }
            }
        }
    }
    else if (ShowEconomyVacancyScreen)
    {
        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(
                    std::to_wstring(
                        RealVacancyAxisLabels[static_cast<size_t>(Index)]).c_str());
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
            const float MaxValue =
                static_cast<float>((std::max)(1, RealVacancyAxisMax));

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
            FormatInteger(CurrentTouristCount),
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
            L"가족",
            L"VIP"
        };
        const int BreakdownValues[GEconomyBreakdownRowCount] =
        {
            RelaxationTouristCount,
            CulturalTouristCount,
            ThrillTouristCount,
            BackpackerTouristCount,
            FamilyTouristCount,
            VipTouristCount
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
            L"▽ 관광객 프로필",
            L"▷ 휴양",
            L"▷ 문화",
            L"▷ 스릴 중독",
            L"▷ 배낭여행",
            L"▷ 가족",
            L"▷ VIP",
            L"▽ 운영 지표",
            L"▷ 선호 일치 / 슬롯 점유율"
        };
        const std::wstring BreakdownValues[GEconomyBreakdownRowCount] =
        {
            L"",
            FormatInteger(RelaxationTouristCount),
            FormatInteger(CulturalTouristCount),
            FormatInteger(ThrillTouristCount),
            FormatInteger(BackpackerTouristCount),
            FormatInteger(FamilyTouristCount),
            FormatInteger(VipTouristCount),
            L"",
            FormatInteger(TouristPreferenceMatchedPercent) +
                L"% / " +
                FormatInteger(TourismVisitUtilizationPercent) +
                L"%"
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
            FormatInteger(TourismVisitOccupancy),
            0.f,
            FVector4(0.94f, 0.90f, 0.78f, 0.96f),
            false);
        SetMetricRowData(
            Widget.mEconomyMetrics[2],
            L"✓ 수용력 점유율",
            FormatInteger(TourismVisitUtilizationPercent) + L"%",
            0.f,
            FVector4(0.94f, 0.90f, 0.78f, 0.96f),
            false);
        SetMetricRowData(
            Widget.mEconomyMetrics[3],
            L"✓ 총 숙박 슬롯",
            FormatInteger(TourismVisitCapacity),
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
            L"▽ 관광 인프라",
            L"▷ 관광 건물",
            L"▷ 항만 진입점",
            L"▷ 오락 시설",
            L"▽ 방문 현황",
            L"▷ 선호 일치 방문",
            L"▷ 여유 슬롯"
        };
        const std::wstring BreakdownValues[GEconomyBreakdownRowCount] =
        {
            L"",
            FormatInteger(Snapshot.TourismBuildingCount),
            FormatInteger(Snapshot.HarborCount),
            FormatInteger(Snapshot.EntertainmentBuildingCount),
            L"",
            FormatInteger(TouristPreferenceMatchedCount),
            FormatInteger(TourismVisitSlack)
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
            FormatInteger(
                Snapshot.ActiveCitizenCount > 0 ?
                    RoundToInt(
                        static_cast<double>(Snapshot.UnemployedCount) * 100.0 /
                        static_cast<double>(Snapshot.ActiveCitizenCount)) :
                    0) + L"%",
            0.f,
            FVector4(0.74f, 0.22f, 0.18f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mEconomyMetrics[1],
            L"현재 직장 점유율",
            FormatInteger(
                Snapshot.JobCapacity > 0 ?
                    RoundToInt(
                        static_cast<double>(Snapshot.AssignedJobCount) * 100.0 /
                        static_cast<double>(Snapshot.JobCapacity)) :
                    0) + L"%",
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
            L"▷ 배정 일자리",
            L"▷ 실업자 시민"
        };
        const std::wstring BreakdownValues[GEconomyBreakdownRowCount] =
        {
            FormatInteger((std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount)),
            FormatInteger(Snapshot.AssignedJobCount),
            FormatInteger(Snapshot.UnemployedCount)
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

