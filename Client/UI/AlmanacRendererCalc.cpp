#include "AlmanacRendererCalc.h"
#include "UIStrings.h"
#include "../Citizen/CitizenTypes.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace
{
    std::wstring ResolveTaxEventWorldEffectSummary(
        const FTaxPolicyEventStatus& TaxEventStatus)
    {
        if (TaxEventStatus.Active)
        {
            switch (TaxEventStatus.Type)
            {
            case ETaxPolicyEventType::WorkerTaxStrike:
                return UIStrings::Get(
                    L"almanac.tax_event_effect.worker_tax_strike");
            case ETaxPolicyEventType::PropertyTaxBacklash:
                return UIStrings::Get(
                    L"almanac.tax_event_effect.property_tax_backlash");
            case ETaxPolicyEventType::BudgetCrisis:
                return UIStrings::Get(
                    L"almanac.tax_event_effect.budget_crisis");
            default:
                break;
            }
        }
        else if (TaxEventStatus.NotificationDays > 0 &&
            !TaxEventStatus.Summary.empty())
        {
            return UIStrings::Get(
                L"almanac.tax_event_effect.recovering");
        }

        return UIStrings::Get(L"almanac.tax_event_effect.none");
    }
}

namespace AlmanacRendererCalc
{
    double ClampSatisfactionValue(double Value)
    {
        return (std::max)(18.0, (std::min)(96.0, Value));
    }

    std::array<float, GSatisfactionGraphPointCount> BuildSatisfactionTrend(
        double CurrentValue,
        double BaselineValue,
        double LiftBias)
    {
        std::array<float, GSatisfactionGraphPointCount> Points = {};
        const double ClampedCurrent =
            ClampSatisfactionValue(CurrentValue);
        const double Lift =
            (std::max)(1.0,
                (std::min)(
                    6.0,
                    (60.0 - ClampedCurrent) * 0.08 + 2.0 + LiftBias));
        const double BaselinePull =
            (std::max)(-3.0,
                (std::min)(
                    3.0,
                    (BaselineValue - ClampedCurrent) * 0.08));

        Points[0] = static_cast<float>(ClampSatisfactionValue(
            ClampedCurrent - Lift * 1.35 + BaselinePull * 0.35));
        Points[1] = static_cast<float>(ClampSatisfactionValue(
            ClampedCurrent - Lift * 0.82 + BaselinePull * 0.20));
        Points[2] = static_cast<float>(ClampSatisfactionValue(
            ClampedCurrent - Lift * 0.34 + BaselinePull * 0.10));
        Points[3] = static_cast<float>(ClampedCurrent);
        return Points;
    }

    float ResolveGraphY(
        float GraphTop,
        float GraphHeight,
        float Value)
    {
        return GraphTop +
            GraphHeight * (1.f - Clamp01(Value / 100.f));
    }

    float ResolveGraphYInRange(
        float GraphTop,
        float GraphHeight,
        float Value,
        float MinValue,
        float MaxValue)
    {
        const float Range =
            (std::max)(1.f, MaxValue - MinValue);
        const float Normalized =
            Clamp01((Value - MinValue) / Range);
        return GraphTop + GraphHeight * (1.f - Normalized);
    }

    FVector4 GetSatisfactionTint(int Index)
    {
        switch (Index)
        {
        case 0:
            return FVector4(0.96f, 0.80f, 0.12f, 0.98f);
        case 1:
            return FVector4(0.94f, 0.66f, 0.16f, 0.98f);
        case 2:
            return FVector4(0.48f, 0.74f, 0.40f, 0.98f);
        case 3:
            return FVector4(0.86f, 0.56f, 0.18f, 0.98f);
        case 4:
            return FVector4(0.72f, 0.56f, 0.78f, 0.98f);
        case 5:
            return FVector4(0.74f, 0.64f, 0.34f, 0.98f);
        case 6:
            return FVector4(0.64f, 0.46f, 0.22f, 0.98f);
        case 7:
            return FVector4(0.62f, 0.72f, 0.92f, 0.98f);
        case 8:
            return FVector4(0.86f, 0.72f, 0.24f, 0.98f);
        default:
            return FVector4(0.90f, 0.72f, 0.18f, 0.95f);
        }
    }

    int RoundToInt(double Value)
    {
        return static_cast<int>(std::lround(Value));
    }

    int ResolvePopulationSatisfactionTier(double Value)
    {
        if (Value < 20.0)
            return 0;
        if (Value < 40.0)
            return 1;
        if (Value < 55.0)
            return 2;
        if (Value < 75.0)
            return 3;
        return 4;
    }

    std::array<float, GPopulationTrendPointCount> BuildPopulationTrend(
        int CurrentPopulation,
        int Growth12M,
        int Decline12M)
    {
        std::array<float, GPopulationTrendPointCount> Points = {};
        const float StartPopulation = static_cast<float>(
            (std::max)(0, CurrentPopulation - Growth12M - Decline12M / 2));
        const float EndPopulation =
            static_cast<float>((std::max)(0, CurrentPopulation));
        const float PopulationDelta = EndPopulation - StartPopulation;

        for (int Index = 0; Index < GPopulationTrendPointCount; ++Index)
        {
            const float Progress =
                GPopulationTrendPointCount <= 1 ?
                    0.f :
                    static_cast<float>(Index) /
                    static_cast<float>(GPopulationTrendPointCount - 1);
            float Value =
                StartPopulation +
                PopulationDelta * Progress +
                std::sin(Progress * 3.6f) * 7.f +
                std::cos(Progress * 8.4f) * 3.f;

            if (Index == 0)
                Value = StartPopulation;
            else
                Value = (std::max)(Value, Points[static_cast<size_t>(Index - 1)] - 2.5f);

            Points[static_cast<size_t>(Index)] = Value;
        }

        Points.back() = EndPopulation;
        return Points;
    }

    std::array<float, GPopulationChangeBarCount> BuildPopulationChangeSeries(
        float BaseValue,
        bool Positive)
    {
        constexpr float GPositivePattern[GPopulationChangeBarCount] =
        {
            0.86f, 0.68f, 0.94f, 0.74f, 0.80f, 0.86f,
            0.72f, 0.72f, 0.92f, 0.70f, 1.18f, 0.98f
        };
        constexpr float GNegativePattern[GPopulationChangeBarCount] =
        {
            0.54f, 0.86f, 0.64f, 0.92f, 0.72f, 0.62f,
            0.88f, 0.74f, 0.66f, 0.80f, 0.56f, 0.70f
        };

        std::array<float, GPopulationChangeBarCount> Values = {};

        for (int Index = 0; Index < GPopulationChangeBarCount; ++Index)
        {
            const float Pattern =
                Positive ?
                    GPositivePattern[Index] :
                    GNegativePattern[Index];
            const float RawValue =
                BaseValue * Pattern +
                std::sin(static_cast<float>(Index) * 0.7f) *
                    (Positive ? 1.8f : 1.2f);

            Values[static_cast<size_t>(Index)] =
                Positive ?
                    (std::max)(4.f, (std::min)(58.f, RawValue)) :
                    (std::max)(3.f, (std::min)(36.f, RawValue));
        }

        return Values;
    }

    std::array<float, GPopulationDistributionBarCount>
        BuildPopulationDistributionSeries(
            float CurrentValue,
            float StartRatio,
            float EndRatio)
    {
        std::array<float, GPopulationDistributionBarCount> Values = {};
        const float StartValue = CurrentValue * StartRatio;
        const float EndValue = CurrentValue * EndRatio;

        for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
        {
            const float Progress =
                GPopulationDistributionBarCount <= 1 ?
                    0.f :
                    static_cast<float>(Index) /
                    static_cast<float>(GPopulationDistributionBarCount - 1);
            Values[static_cast<size_t>(Index)] =
                StartValue +
                (EndValue - StartValue) * Progress +
                std::sin(static_cast<float>(Index) * 0.55f) * 3.f;
        }

        return Values;
    }

    std::array<float, GPopulationDistributionBarCount> BuildPopulationDetailTrend(
        float StartValue,
        float EndValue,
        float WaveA,
        float WaveB)
    {
        std::array<float, GPopulationDistributionBarCount> Values = {};

        for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
        {
            const float Progress =
                GPopulationDistributionBarCount <= 1 ?
                    0.f :
                    static_cast<float>(Index) /
                    static_cast<float>(GPopulationDistributionBarCount - 1);
            const float Value =
                StartValue +
                (EndValue - StartValue) * Progress +
                std::sin(static_cast<float>(Index) * 0.62f) * WaveA +
                std::cos(static_cast<float>(Index) * 1.15f) * WaveB;
            Values[static_cast<size_t>(Index)] = (std::max)(0.f, Value);
        }

        return Values;
    }

    std::array<float, GPopulationTrendPointCount> BuildPopulationRateTrend(
        float StartValue,
        float EndValue,
        float WaveA,
        float WaveB)
    {
        std::array<float, GPopulationTrendPointCount> Values = {};

        for (int Index = 0; Index < GPopulationTrendPointCount; ++Index)
        {
            const float Progress =
                GPopulationTrendPointCount <= 1 ?
                    0.f :
                    static_cast<float>(Index) /
                    static_cast<float>(GPopulationTrendPointCount - 1);
            const float BaseValue =
                StartValue + (EndValue - StartValue) * Progress;
            const float Value =
                BaseValue +
                std::sin(static_cast<float>(Index) * 0.78f) * WaveA +
                std::cos(static_cast<float>(Index) * 0.33f) * WaveB;
            Values[static_cast<size_t>(Index)] =
                (std::max)(0.f, (std::min)(100.f, Value));
        }

        Values.front() = (std::max)(0.f, (std::min)(100.f, StartValue));
        Values.back() = (std::max)(0.f, (std::min)(100.f, EndValue));
        return Values;
    }

    std::array<int, 5> AllocatePopulationBuckets(
        int TotalCount,
        const std::array<float, 5>& Weights)
    {
        std::array<int, 5> Result = {};

        if (TotalCount <= 0)
            return Result;

        float WeightSum = 0.f;

        for (int Index = 0; Index < 5; ++Index)
            WeightSum += (std::max)(0.f, Weights[static_cast<size_t>(Index)]);

        if (WeightSum <= 0.0001f)
        {
            Result[3] = TotalCount;
            return Result;
        }

        std::array<float, 5> Remainders = {};
        int Assigned = 0;

        for (int Index = 0; Index < 5; ++Index)
        {
            const float NormalizedWeight =
                (std::max)(0.f, Weights[static_cast<size_t>(Index)]) / WeightSum;
            const float ExactValue =
                static_cast<float>(TotalCount) * NormalizedWeight;
            const int BaseCount =
                static_cast<int>(std::floor(ExactValue));

            Result[static_cast<size_t>(Index)] = BaseCount;
            Remainders[static_cast<size_t>(Index)] =
                ExactValue - static_cast<float>(BaseCount);
            Assigned += BaseCount;
        }

        while (Assigned < TotalCount)
        {
            int BestIndex = 0;
            float BestRemainder = -1.f;

            for (int Index = 0; Index < 5; ++Index)
            {
                const float Remainder =
                    Remainders[static_cast<size_t>(Index)];

                if (Remainder > BestRemainder)
                {
                    BestRemainder = Remainder;
                    BestIndex = Index;
                }
            }

            ++Result[static_cast<size_t>(BestIndex)];
            Remainders[static_cast<size_t>(BestIndex)] = -1.f;
            ++Assigned;
        }

        return Result;
    }

    std::array<int, 5> BuildHomelessFamilyWealthBuckets(
        int HomelessFamilyCount,
        const int HomelessWealthCount[3])
    {
        std::array<int, 5> Buckets = {};

        if (HomelessFamilyCount <= 0)
            return Buckets;

        const int PoorCount =
            (std::max)(0,
                HomelessWealthCount[static_cast<int>(ECitizenWealthLevel::Poor)]);
        const int WellOffCount =
            (std::max)(0,
                HomelessWealthCount[static_cast<int>(ECitizenWealthLevel::WellOff)]);
        const int RichCount =
            (std::max)(0,
                HomelessWealthCount[static_cast<int>(ECitizenWealthLevel::Rich)]);
        const int TotalHomelessCitizens =
            PoorCount + WellOffCount + RichCount;

        std::array<float, 5> DerivedWeights =
        {
            0.18f, 0.04f, 0.03f, 0.70f, 0.05f
        };

        if (TotalHomelessCitizens > 0)
        {
            const float PoorShare =
                static_cast<float>(PoorCount) /
                static_cast<float>(TotalHomelessCitizens);
            const float WellOffShare =
                static_cast<float>(WellOffCount) /
                static_cast<float>(TotalHomelessCitizens);
            const float RichShare =
                static_cast<float>(RichCount) /
                static_cast<float>(TotalHomelessCitizens);
            const std::array<float, 5> SnapshotWeights =
            {
                PoorShare * 0.35f,
                PoorShare * 0.65f,
                WellOffShare,
                RichShare * 0.85f,
                RichShare * 0.15f
            };

            for (int Index = 0; Index < 5; ++Index)
            {
                DerivedWeights[static_cast<size_t>(Index)] =
                    DerivedWeights[static_cast<size_t>(Index)] * 0.85f +
                    SnapshotWeights[static_cast<size_t>(Index)] * 0.15f;
            }
        }

        Buckets = AllocatePopulationBuckets(
            HomelessFamilyCount,
            DerivedWeights);

        if (HomelessFamilyCount >= 4 &&
            Buckets[3] <= 0)
        {
            for (int Index = 0; Index < 5; ++Index)
            {
                if (Index == 3 ||
                    Buckets[static_cast<size_t>(Index)] <= 1)
                {
                    continue;
                }

                --Buckets[static_cast<size_t>(Index)];
                ++Buckets[3];
                break;
            }
        }

        return Buckets;
    }

    std::array<int, 5> BuildCitizenWealthBuckets(
        int CitizenCount,
        const int CitizenWealthCount[3])
    {
        std::array<int, 5> Buckets = {};

        if (CitizenCount <= 0)
            return Buckets;

        const int PoorCount =
            (std::max)(0,
                CitizenWealthCount[static_cast<int>(ECitizenWealthLevel::Poor)]);
        const int WellOffCount =
            (std::max)(0,
                CitizenWealthCount[static_cast<int>(ECitizenWealthLevel::WellOff)]);
        const int RichCount =
            (std::max)(0,
                CitizenWealthCount[static_cast<int>(ECitizenWealthLevel::Rich)]);
        const int TotalCitizens =
            PoorCount + WellOffCount + RichCount;

        std::array<float, 5> DerivedWeights =
        {
            0.015f, 0.040f, 0.740f, 0.190f, 0.015f
        };

        if (TotalCitizens > 0)
        {
            const float PoorShare =
                static_cast<float>(PoorCount) /
                static_cast<float>(TotalCitizens);
            const float WellOffShare =
                static_cast<float>(WellOffCount) /
                static_cast<float>(TotalCitizens);
            const float RichShare =
                static_cast<float>(RichCount) /
                static_cast<float>(TotalCitizens);
            const std::array<float, 5> SnapshotWeights =
            {
                PoorShare * 0.24f,
                PoorShare * 0.76f,
                WellOffShare,
                RichShare * 0.94f,
                RichShare * 0.06f
            };

            for (int Index = 0; Index < 5; ++Index)
            {
                DerivedWeights[static_cast<size_t>(Index)] =
                    DerivedWeights[static_cast<size_t>(Index)] * 0.15f +
                    SnapshotWeights[static_cast<size_t>(Index)] * 0.85f;
            }
        }

        Buckets = AllocatePopulationBuckets(
            CitizenCount,
            DerivedWeights);

        if (PoorCount > 0 &&
            Buckets[0] <= 0 &&
            Buckets[1] > 1)
        {
            --Buckets[1];
            ++Buckets[0];
        }

        if (RichCount > 0 &&
            Buckets[4] <= 0 &&
            Buckets[3] > 1)
        {
            --Buckets[3];
            ++Buckets[4];
        }

        return Buckets;
    }

    std::array<float, GPopulationDistributionBarCount>
        BuildPopulationHistoricalLayer(
            float StartValue,
            float PeakValue,
            float EndValue,
            float WaveA,
            float WaveB)
    {
        std::array<float, GPopulationDistributionBarCount> Values = {};

        for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
        {
            const float Progress =
                GPopulationDistributionBarCount <= 1 ?
                    0.f :
                    static_cast<float>(Index) /
                    static_cast<float>(GPopulationDistributionBarCount - 1);
            const float BaseValue =
                Progress < 0.58f ?
                    StartValue +
                        (PeakValue - StartValue) * (Progress / 0.58f) :
                    PeakValue +
                        (EndValue - PeakValue) * ((Progress - 0.58f) / 0.42f);
            const float Value =
                BaseValue +
                std::sin(static_cast<float>(Index) * 0.57f) * WaveA +
                std::cos(static_cast<float>(Index) * 0.23f) * WaveB;
            Values[static_cast<size_t>(Index)] = (std::max)(0.f, Value);
        }

        Values.back() = (std::max)(0.f, EndValue);
        return Values;
    }

    FConflictPageComputedData BuildConflictPageComputedData(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
        FConflictPageComputedData Result;
        const int ActiveCitizenCount =
            (std::max)(1, Snapshot.ActiveCitizenCount);
        const double MonthlyBuildingCost =
            static_cast<double>(Snapshot.MonthlyWageCost + Snapshot.MonthlyUpkeepCost);
        const double MonthlyPolicyCost =
            (std::max)(0.0, static_cast<double>(Snapshot.DailyEdictCost) * 30.0);
        const double MonthlyImportCost =
            (std::max)(0.0, static_cast<double>(Snapshot.DailyImportExpense) * 30.0);
        const double MonthlyTotalCost =
            MonthlyBuildingCost + MonthlyPolicyCost + MonthlyImportCost;
        const double DailyOperatingCost =
            MonthlyTotalCost > 0.0 ? MonthlyTotalCost / 30.0 : 0.0;

        Result.UnemploymentRate =
            static_cast<double>(Snapshot.UnemployedCount) /
            static_cast<double>(ActiveCitizenCount);
        Result.FiscalStress =
            DailyOperatingCost > 0.0 && Snapshot.DailyNetChange < 0 ?
                Clamp01(
                    static_cast<double>(-Snapshot.DailyNetChange) /
                    DailyOperatingCost) :
                0.0;
        Result.ControlStrength =
            Clamp01(
                Clamp01(Snapshot.AverageSecurity / 100.0) * 0.55 +
                Clamp01(Snapshot.SupportPercent / 100.0) * 0.25 +
                (1.0 - Result.FiscalStress) * 0.20);
        Result.Stability =
            Clamp01(1.0 - Snapshot.RebelRiskScore / 100.0);
        Result.TaxEventWorldEffectSummary =
            ResolveTaxEventWorldEffectSummary(Snapshot.TaxEventStatus);
        Result.ElectionWarningSummary =
            BuildElectionWarningSummary(
                Snapshot.ElectionStatus.GameLost,
                Snapshot.DaysUntilNextElection,
                Snapshot.ElectionWarningScore,
                Snapshot.TaxEventStatus);
        Result.ElectionWarningActive =
            Snapshot.DaysUntilNextElection >= 0 &&
            Snapshot.DaysUntilNextElection <= 180 &&
            Snapshot.ElectionWarningScore >= 0.32;
        Result.ElectionWarningTint =
            ResolveElectionWarningTint(Snapshot.ElectionWarningScore);
        return Result;
    }
}
