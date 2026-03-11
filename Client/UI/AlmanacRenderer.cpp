#include "AlmanacRenderer.h"
#include "AlmanacRendererInternal.h"
#include "../Citizen/CitizenTypes.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace
{
    struct FSatisfactionDetailEntry
    {
        std::wstring Label;
        std::wstring Value;
        bool Highlight = false;
        FVector4 Tint = FVector4(0.31f, 0.27f, 0.21f, 1.f);
    };

    double ClampSatisfactionValue(double Value)
    {
        return (std::max)(18.0, (std::min)(96.0, Value));
    }

    std::array<float, GSatisfactionGraphPointCount> BuildSatisfactionTrend(
        double CurrentValue,
        double BaselineValue,
        double LiftBias = 0.0)
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

    std::array<float, GPopulationDistributionBarCount> BuildPopulationDistributionSeries(
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

    std::array<float, GPopulationDistributionBarCount> BuildPopulationHistoricalLayer(
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
}

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
    const double MonthlyImportCost =
        (std::max)(0.0, static_cast<double>(Snapshot.DailyImportExpense) * 30.0);
    const double MonthlyTotalCost =
        MonthlyBuildingCost + MonthlyPolicyCost + MonthlyImportCost;
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
    (void)CapitalistReaction;
    (void)CommunistReaction;
    (void)IntellectualReaction;
    (void)ConservativeReaction;
    (void)TaxFactionReactions;
    (void)StrongestPositiveReactionIter;
    (void)StrongestNegativeReactionIter;
    (void)TaxStanceSummary;
    (void)EconomicBlocReaction;
    (void)FactionDemandLabel;
    (void)FactionDemandSummary;
    (void)FactionDemandTint;
    (void)NormalizePoliticalScore;
    (void)BuildAxisBreakdown;
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
    (void)LastElectionLabel;
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

    struct FPoliticsModifierEntry
    {
        const wchar_t* Label = L"";
        int Value = 0;
    };

    struct FPoliticsFactionUiEntry
    {
        const wchar_t* Label = L"";
        int Count = 0;
        int Favor = 0;
        const wchar_t* Leader = L"";
        int ModerateCount = 0;
        int StrongCount = 0;
        int FerventCount = 0;
        int BuildingModifierTotal = 0;
        FVector4 IconTint = FVector4(0.90f, 0.72f, 0.18f, 0.96f);
        std::array<FPoliticsModifierEntry, 6> Modifiers = {};
    };

    const std::array<FPoliticsFactionUiEntry, GPoliticsFactionTileCount> PoliticsFactions =
    {
        FPoliticsFactionUiEntry
        {
            L"자본주의자",
            422,
            57,
            L"Adri\u00E1n L\u00F3pez",
            192,
            230,
            0,
            12,
            FVector4(0.34f, 0.78f, 0.40f, 0.96f),
            {{
                { L"\uC5D0\uD3A0 \uD0D1", 1 },
                { L"\uB300\uC2DD\uB2F9", 1 },
                { L"\uB178\uC774\uC288\uBC18\uC288\uD0C0\uC778 \uC131", 2 },
                { L"\uD328\uC2A4\uD2B8\uD478\uB4DC \uCCB4\uC778\uC810", 2 },
                { L"\uAC70\uC8FC\uC6A9 \uB3D9", -7 },
                { L"\uACF5\uD56D", 7 }
            }}
        },
        FPoliticsFactionUiEntry
        {
            L"\uACF5\uC0B0\uC8FC\uC758\uC790",
            436,
            71,
            L"Camila Ortega",
            204,
            232,
            0,
            9,
            FVector4(0.84f, 0.24f, 0.18f, 0.96f),
            {{
                { L"\uACF5\uB3D9 \uC8FC\uD0DD", 3 },
                { L"\uB300\uD559\uAD50", 2 },
                { L"\uBCF5\uC9C0 \uC0AC\uBB34\uC18C", 2 },
                { L"\uC740\uD589", -2 },
                { L"\uACF5\uD56D", -1 },
                { L"\uBCF4\uAC74\uC18C", 5 }
            }}
        },
        FPoliticsFactionUiEntry
        {
            L"\uC885\uAD50\uC778",
            420,
            41,
            L"Jacob Hughes",
            173,
            247,
            0,
            0,
            FVector4(0.90f, 0.86f, 0.78f, 0.96f),
            {{
                { L"\uC5D0\uD3A0 \uD0D1", 1 },
                { L"\uB300\uC131\uB2F9", 1 },
                { L"\uB178\uC774\uC288\uBC18\uC288\uD0C0\uC778 \uC131", 2 },
                { L"\uCE74\uBC14\uB808", -1 },
                { L"\uACF5\uD56D", 1 },
                { L"\uACE0\uAE09 \uD638\uD154", -1 }
            }}
        },
        FPoliticsFactionUiEntry
        {
            L"\uAD70\uAD6D\uC8FC\uC758\uC790",
            414,
            65,
            L"General Batista",
            186,
            228,
            0,
            8,
            FVector4(0.56f, 0.44f, 0.16f, 0.96f),
            {{
                { L"\uB9C9\uC0AC", 3 },
                { L"\uACBD\uBE44 \uCD08\uC18C", 2 },
                { L"\uC694\uC0C8", 4 },
                { L"\uAD50\uD68C", -1 },
                { L"\uBC29\uC1A1\uAD6D", -2 },
                { L"\uAD70\uD56D", 2 }
            }}
        },
        FPoliticsFactionUiEntry
        {
            L"\uD658\uACBD\uC8FC\uC758\uC790",
            447,
            83,
            L"Elena Verde",
            219,
            228,
            0,
            11,
            FVector4(0.22f, 0.70f, 0.48f, 0.96f),
            {{
                { L"\uAD6D\uB9BD\uACF5\uC6D0", 4 },
                { L"\uD48D\uB825 \uBC1C\uC804\uC18C", 3 },
                { L"\uC815\uC6D0", 2 },
                { L"\uAD11\uC0B0", -3 },
                { L"\uACF5\uC7A5", -2 },
                { L"\uD3D0\uAE30\uBB3C \uCC98\uB9AC\uC7A5", 7 }
            }}
        },
        FPoliticsFactionUiEntry
        {
            L"\uC2E4\uC5C5\uAC00",
            401,
            62,
            L"Rafael Cruz",
            183,
            218,
            0,
            7,
            FVector4(0.56f, 0.46f, 0.18f, 0.96f),
            {{
                { L"\uACF5\uC7A5", 3 },
                { L"\uBB34\uC5ED\uD56D", 2 },
                { L"\uC740\uD589", 2 },
                { L"\uD658\uACBD\uCCAD", -3 },
                { L"\uD30C\uC5C5", -1 },
                { L"\uACF5\uD56D", 4 }
            }}
        },
        FPoliticsFactionUiEntry
        {
            L"\uC9C0\uC2DD\uC778",
            427,
            75,
            L"Prof. Luc\u00EDa Vega",
            211,
            216,
            0,
            10,
            FVector4(0.92f, 0.72f, 0.22f, 0.96f),
            {{
                { L"\uACE0\uB4F1\uD559\uAD50", 2 },
                { L"\uB300\uD559\uAD50", 4 },
                { L"\uB3C4\uC11C\uAD00", 3 },
                { L"\uB9C9\uC0AC", -2 },
                { L"\uAC80\uBB38\uC18C", -1 },
                { L"\uBC29\uC1A1\uAD6D", 4 }
            }}
        },
        FPoliticsFactionUiEntry
        {
            L"\uBCF4\uC218\uC8FC\uC758\uC790",
            392,
            10,
            L"Mar\u00EDa del Sol",
            180,
            212,
            0,
            3,
            FVector4(0.46f, 0.52f, 0.60f, 0.96f),
            {{
                { L"\uC131\uB2F9", 1 },
                { L"\uACBD\uCC30\uC11C", 2 },
                { L"\uD328\uC2A4\uD2B8\uD478\uB4DC \uCCB4\uC778\uC810", -1 },
                { L"\uACF5\uD56D", -2 },
                { L"\uAD81\uC804", 2 },
                { L"\uD604\uB300 \uC608\uC220\uAD00", -3 }
            }}
        }
    };

    const int PoliticsNeutralCounts[GPoliticsNeutralCount] = { 124, 148, 134, 163 };
    const int SelectedPoliticsFactionIndex =
        (std::max)(0,
            (std::min)(
                static_cast<int>(PoliticsFactions.size()) - 1,
                Widget.mSelectedPoliticsFactionIndex));
    const FPoliticsFactionUiEntry& SelectedFaction =
        PoliticsFactions[static_cast<size_t>(SelectedPoliticsFactionIndex)];

    for (int Index = 0; Index < static_cast<int>(PoliticsFactions.size()); ++Index)
    {
        SetPoliticsFactionTileData(
            Widget.mPoliticsFactionTiles[Index],
            GPoliticsFactionIcons[Index],
            PoliticsFactions[Index].IconTint,
            PoliticsFactions[Index].Label,
            PoliticsFactions[Index].Count,
            PoliticsFactions[Index].Favor,
            Index == SelectedPoliticsFactionIndex);
    }

    for (int Index = 0; Index < GPoliticsNeutralCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPoliticsNeutralTexts.size()))
            break;

        if (auto Text = Widget.mPoliticsNeutralTexts[Index].lock())
        {
            Text->SetText(
                (std::wstring(L"\uBB34\uAD00\uC2EC\n") +
                    std::to_wstring(PoliticsNeutralCounts[Index])).c_str());
        }
    }

    if (auto Title = Widget.mPoliticsFactionTitle.lock())
        Title->SetText(SelectedFaction.Label);
    if (auto Label = Widget.mPoliticsFactionApprovalLabel.lock())
    {
        Label->SetText(
            (std::wstring(SelectedFaction.Label) + L" \uC6B0\uD638\uB3C4").c_str());
    }
    if (auto Value = Widget.mPoliticsFactionApprovalValue.lock())
        Value->SetText(std::to_wstring(SelectedFaction.Favor).c_str());

    if (auto Title = Widget.mPoliticsSupportTitle.lock())
        Title->SetText(L"\uC9C0\uC9C0\uC728");

    SetDetailRowData(
        Widget.mPoliticsSupportRows[0],
        L"\uC720\uAD8C\uC790",
        L"982");
    SetDetailRowData(
        Widget.mPoliticsSupportRows[1],
        L"\uC9C0\uC9C0\uC728",
        L"42%");
    SetDetailRowData(
        Widget.mPoliticsSupportRows[2],
        L"\uBBF8\uC815",
        L"20%");
    SetDetailRowData(
        Widget.mPoliticsSupportRows[3],
        L"\uBC18\uB300",
        L"38%");

    for (const auto& Row : Widget.mPoliticsSupportRows)
    {
        if (auto Background = Row.Background.lock())
            Background->SetTint(1.f, 1.f, 1.f, 0.88f);
    }

    if (auto Text = Widget.mPoliticsElectionText.lock())
        Text->SetText(L"\uB2E4\uC74C \uC120\uAC70\n1\uC6D4, 2029");

    SetDetailRowData(
        Widget.mPoliticsDetails[0],
        L"\u25BD \uC138\uB825 \uC9C0\uB3C4\uC790",
        L"");
    SetDetailRowData(
        Widget.mPoliticsDetails[1],
        SelectedFaction.Leader,
        L"");
    SetDetailRowData(
        Widget.mPoliticsDetails[2],
        (std::wstring(L"\u25BD ") + SelectedFaction.Label + L"\uC758 \uC218"),
        std::to_wstring(SelectedFaction.Count));
    SetDetailRowData(
        Widget.mPoliticsDetails[3],
        std::wstring(L"\uBCF4\uD1B5 ") + SelectedFaction.Label,
        std::to_wstring(SelectedFaction.ModerateCount));
    SetDetailRowData(
        Widget.mPoliticsDetails[4],
        std::wstring(L"\uAC15\uD55C ") + SelectedFaction.Label,
        std::to_wstring(SelectedFaction.StrongCount));
    SetDetailRowData(
        Widget.mPoliticsDetails[5],
        std::wstring(L"\uC5F4\uB82C\uD55C ") + SelectedFaction.Label,
        std::to_wstring(SelectedFaction.FerventCount));
    SetDetailRowData(
        Widget.mPoliticsDetails[6],
        L"\u25BD \uC6B0\uD638\uB3C4 \uC218\uC815\uCE58",
        L"");
    SetDetailRowData(
        Widget.mPoliticsDetails[7],
        L"\uAC74\uBB3C \uC218\uC815\uCE58",
        std::to_wstring(SelectedFaction.BuildingModifierTotal),
        false,
        SelectedFaction.BuildingModifierTotal > 0 ?
            FVector4(0.18f, 0.62f, 0.34f, 1.f) :
        SelectedFaction.BuildingModifierTotal < 0 ?
            FVector4(0.80f, 0.22f, 0.18f, 1.f) :
            FVector4(0.31f, 0.27f, 0.21f, 1.f));

    for (int Index = 0; Index < 6; ++Index)
    {
        const int DetailIndex = 8 + Index;
        const int ModifierValue = SelectedFaction.Modifiers[static_cast<size_t>(Index)].Value;
        const std::wstring ModifierText =
            ModifierValue > 0 ?
                L"+" + std::to_wstring(ModifierValue) :
                std::to_wstring(ModifierValue);

        SetDetailRowData(
            Widget.mPoliticsDetails[DetailIndex],
            SelectedFaction.Modifiers[static_cast<size_t>(Index)].Label,
            ModifierText,
            false,
            FVector4(0.31f, 0.27f, 0.21f, 1.f));
    }

    const int PoliticsHeaderRows[3] = { 0, 2, 6 };
    for (int HeaderIndex : PoliticsHeaderRows)
    {
        if (HeaderIndex >= static_cast<int>(Widget.mPoliticsDetails.size()))
            continue;

        if (auto Background =
                Widget.mPoliticsDetails[static_cast<size_t>(HeaderIndex)].Background.lock())
        {
            Background->SetTint(0.98f, 0.95f, 0.84f, 0.94f);
        }
        if (auto Label =
                Widget.mPoliticsDetails[static_cast<size_t>(HeaderIndex)].Label.lock())
        {
            Label->SetTextColor(112, 86, 38, 255);
        }
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mPoliticsDetails.size()); ++Index)
    {
        if (auto Background = Widget.mPoliticsDetails[Index].Background.lock())
        {
            if (Index != 0 && Index != 2 && Index != 6)
                Background->SetTint(1.f, 1.f, 1.f, 0.88f);
        }
    }

    if (auto Label = Widget.mPoliticsDetails[1].Label.lock())
    {
        Label->SetFontSize(18.f);
        Label->SetTextColor(86, 70, 44, 255);
    }

    struct FForeignModifierEntry
    {
        const wchar_t* Label = L"";
        int Value = 0;
    };

    struct FForeignPowerEntry
    {
        const wchar_t* Name = L"";
        int Relation = 0;
        const wchar_t* DisplayValue = L"0";
        const wchar_t* Status = L"미확정";
        int EconomicAid = 0;
        int BuildingModifier = 0;
        int EdictModifier = 0;
        int ConstitutionModifier = 0;
        int MiscModifier = 0;
        int TradeModifier = 0;
        std::array<FForeignModifierEntry, 3> BuildingLines = {};
        std::array<FForeignModifierEntry, 2> EdictLines = {};
        std::array<FForeignModifierEntry, 2> MiscLines = {};
    };

    const std::array<FForeignPowerEntry, GForeignPowerCount> ForeignPowers =
    {
        FForeignPowerEntry
        {
            L"중국",
            97,
            L"97",
            L"미확정",
            0,
            5,
            39,
            0,
            3,
            0,
            {{
                { L"대사관", 5 },
                { L"", 0 },
                { L"", 0 }
            }},
            {{
                { L"외교적 슈퍼 파티", 39 },
                { L"", 0 }
            }},
            {{
                { L"역사적 관계", 3 },
                { L"", 0 }
            }}
        },
        FForeignPowerEntry
        {
            L"러시아",
            97,
            L"97",
            L"미확정",
            0,
            5,
            39,
            0,
            3,
            0,
            {{
                { L"대사관", 5 },
                { L"", 0 },
                { L"", 0 }
            }},
            {{
                { L"외교적 슈퍼 파티", 39 },
                { L"", 0 }
            }},
            {{
                { L"역사적 관계", 3 },
                { L"", 0 }
            }}
        },
        FForeignPowerEntry
        {
            L"미국",
            104,
            L">100",
            L"동맹적",
            10000,
            6,
            14,
            3,
            1,
            8,
            {{
                { L"대사관", 5 },
                { L"관광 항구", 1 },
                { L"", 0 }
            }},
            {{
                { L"외교적 슈퍼 파티", 10 },
                { L"열강 원조", 4 }
            }},
            {{
                { L"역사적 관계", 1 },
                { L"", 0 }
            }}
        },
        FForeignPowerEntry
        {
            L"중동",
            76,
            L"76",
            L"호의적",
            0,
            2,
            5,
            0,
            1,
            3,
            {{
                { L"대사관", 2 },
                { L"", 0 },
                { L"", 0 }
            }},
            {{
                { L"석유 거래", 5 },
                { L"", 0 }
            }},
            {{
                { L"역사적 관계", 1 },
                { L"", 0 }
            }}
        },
        FForeignPowerEntry
        {
            L"유럽연합",
            49,
            L"49",
            L"경계",
            0,
            3,
            0,
            -2,
            0,
            1,
            {{
                { L"대사관", 3 },
                { L"", 0 },
                { L"", 0 }
            }},
            {{
                { L"", 0 },
                { L"", 0 }
            }},
            {{
                { L"역사적 관계", 1 },
                { L"", 0 }
            }}
        }
    };

    const int SelectedForeignPowerIndex =
        (std::max)(0,
            (std::min)(
                static_cast<int>(ForeignPowers.size()) - 1,
                Widget.mSelectedForeignPowerIndex));
    const FForeignPowerEntry& SelectedForeignPower =
        ForeignPowers[static_cast<size_t>(SelectedForeignPowerIndex)];

    for (int Index = 0; Index < static_cast<int>(ForeignPowers.size()); ++Index)
    {
        SetForeignPowerRowData(
            Widget.mForeignRows[Index],
            GForeignPowerIcons[Index],
            ForeignPowers[Index].Name,
            ForeignPowers[Index].DisplayValue,
            static_cast<float>(Clamp01(ForeignPowers[Index].Relation / 100.0)),
            Index == SelectedForeignPowerIndex);
    }

    if (auto Title = Widget.mForeignTitle.lock())
        Title->SetText(SelectedForeignPower.Name);
    if (auto Text = Widget.mForeignStatusLabel.lock())
        Text->SetText(L"현황");
    if (auto Text = Widget.mForeignStatusValue.lock())
        Text->SetText(SelectedForeignPower.Status);

    const std::wstring RelationLabel =
        std::wstring(SelectedForeignPower.Name) + L"(와)과의 관계";
    SetDetailRowData(
        Widget.mForeignDetails[0],
        RelationLabel,
        std::to_wstring(SelectedForeignPower.Relation));
    SetDetailRowData(
        Widget.mForeignDetails[1],
        L"경제 원조",
        std::to_wstring(SelectedForeignPower.EconomicAid));
    SetDetailRowData(
        Widget.mForeignDetails[2],
        L"▽ 관계 수정치",
        L"");
    SetDetailRowData(
        Widget.mForeignDetails[3],
        L"건물 수정치",
        std::to_wstring(SelectedForeignPower.BuildingModifier));
    SetDetailRowData(
        Widget.mForeignDetails[4],
        SelectedForeignPower.BuildingLines[0].Label,
        SelectedForeignPower.BuildingLines[0].Value > 0 ?
            L"+" + std::to_wstring(SelectedForeignPower.BuildingLines[0].Value) :
            std::to_wstring(SelectedForeignPower.BuildingLines[0].Value));
    SetDetailRowData(
        Widget.mForeignDetails[5],
        L"칙령 수정치",
        std::to_wstring(SelectedForeignPower.EdictModifier));
    SetDetailRowData(
        Widget.mForeignDetails[6],
        SelectedForeignPower.EdictLines[0].Label,
        SelectedForeignPower.EdictLines[0].Value > 0 ?
            L"+" + std::to_wstring(SelectedForeignPower.EdictLines[0].Value) :
            std::to_wstring(SelectedForeignPower.EdictLines[0].Value));
    SetDetailRowData(
        Widget.mForeignDetails[7],
        L"헌법 수정치",
        std::to_wstring(SelectedForeignPower.ConstitutionModifier));
    SetDetailRowData(
        Widget.mForeignDetails[8],
        L"기타 수정치",
        std::to_wstring(SelectedForeignPower.MiscModifier));
    SetDetailRowData(
        Widget.mForeignDetails[9],
        SelectedForeignPower.MiscLines[0].Label,
        SelectedForeignPower.MiscLines[0].Value > 0 ?
            L"+" + std::to_wstring(SelectedForeignPower.MiscLines[0].Value) :
            std::to_wstring(SelectedForeignPower.MiscLines[0].Value));
    SetDetailRowData(
        Widget.mForeignDetails[10],
        L"무역 수정치",
        std::to_wstring(SelectedForeignPower.TradeModifier));

    if (auto Background = Widget.mForeignDetails[2].Background.lock())
        Background->SetTint(0.98f, 0.95f, 0.84f, 0.94f);
    if (auto Label = Widget.mForeignDetails[2].Label.lock())
        Label->SetTextColor(112, 86, 38, 255);

    for (int Index = 0; Index < static_cast<int>(Widget.mForeignDetails.size()); ++Index)
    {
        if (Index == 2)
            continue;

        if (auto Background = Widget.mForeignDetails[Index].Background.lock())
            Background->SetTint(1.f, 1.f, 1.f, 0.88f);
    }

    if (auto Notice = Widget.mForeignNotice.lock())
    {
        Notice->SetEnable(false);
        Notice->SetText(L"");
    }

    struct FBuildingDetailEntry
    {
        const wchar_t* Label = L"";
        int Count = 0;
    };

    struct FBuildingCategoryUiEntry
    {
        const wchar_t* Label = L"";
        int Count = 0;
        std::array<FBuildingDetailEntry, GBuildingDetailCount> Details = {};
    };

    const std::array<FBuildingCategoryUiEntry, GBuildingRowCount> BuildingCategories =
    {
        FBuildingCategoryUiEntry
        {
            L"교통 & 기반시설",
            96,
            {{
                { L"▷ 지하철역", 54 },
                { L"▷ 운송업자 사무소", 19 },
                { L"▷ 태양광 발전소", 10 },
                { L"▷ 해상 풍력 터빈", 7 },
                { L"▷ 변전소", 2 },
                { L"▷ 건설 사무소", 2 },
                { L"▷ 항구", 1 },
                { L"▷ 화물 공항", 1 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"음식 & 자원",
            65,
            {{
                { L"▷ 대규모 수경 농장", 39 },
                { L"▷ 양식장", 13 },
                { L"▷ 공장식 목장", 12 },
                { L"▷ 목장", 1 },
                { L"", 0 },
                { L"", 0 },
                { L"", 0 },
                { L"", 0 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"산업",
            0,
            {{
                { L"▷ 가구 공장", 0 },
                { L"▷ 제약회사", 0 },
                { L"▷ 통조림 공장", 0 },
                { L"▷ 양조장", 0 },
                { L"▷ 자동차 공장", 0 },
                { L"▷ 전자 공장", 0 },
                { L"▷ 무기 공장", 0 },
                { L"▷ 섬유 공장", 0 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"주거지",
            50,
            {{
                { L"▷ 합숙소", 14 },
                { L"▷ 아파트", 10 },
                { L"▷ 현대식 아파트", 7 },
                { L"▷ 공동주택", 6 },
                { L"▷ 저택", 5 },
                { L"▷ 오두막", 4 },
                { L"▷ 컨트리 하우스", 2 },
                { L"▷ 펜트하우스", 2 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"오락",
            39,
            {{
                { L"▷ 선술집", 10 },
                { L"▷ 레스토랑", 8 },
                { L"▷ 극장", 6 },
                { L"▷ 놀이공원", 5 },
                { L"▷ 나이트클럽", 4 },
                { L"▷ 영화관", 3 },
                { L"▷ 클럽", 2 },
                { L"▷ 해변 부두", 1 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"호화 오락",
            17,
            {{
                { L"▷ 카지노", 4 },
                { L"▷ 골프장", 3 },
                { L"▷ 스파", 3 },
                { L"▷ 요트 클럽", 2 },
                { L"▷ 카바레", 2 },
                { L"▷ 고급 레스토랑", 1 },
                { L"▷ 오페라 하우스", 1 },
                { L"▷ 고급 클럽", 1 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"언론 & 교육",
            12,
            {{
                { L"▷ 초등학교", 3 },
                { L"▷ 고등학교", 2 },
                { L"▷ 대학교", 1 },
                { L"▷ 도서관", 2 },
                { L"▷ 라디오 방송국", 1 },
                { L"▷ TV 방송국", 1 },
                { L"▷ 신문사", 1 },
                { L"▷ 사이버 운영 센터", 1 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"관광업",
            124,
            {{
                { L"▷ 호텔", 45 },
                { L"▷ 고급 호텔", 19 },
                { L"▷ 관광 항구", 14 },
                { L"▷ 해변 별장", 12 },
                { L"▷ 관광 식당", 11 },
                { L"▷ 스노클링 베이", 9 },
                { L"▷ 휴양 리조트", 8 },
                { L"▷ 관광 공항", 6 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"공익 서비스",
            9,
            {{
                { L"▷ 병원", 2 },
                { L"▷ 경찰서", 2 },
                { L"▷ 소방서", 1 },
                { L"▷ 교도소", 1 },
                { L"▷ 교회", 1 },
                { L"▷ 묘지", 1 },
                { L"▷ 진료소", 1 },
                { L"▷ 법원", 0 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"습격 & 군사",
            16,
            {{
                { L"▷ 막사", 5 },
                { L"▷ 요새", 3 },
                { L"▷ 경비 초소", 2 },
                { L"▷ 검문소", 2 },
                { L"▷ 군항", 1 },
                { L"▷ 군수 공장", 1 },
                { L"▷ 무기고", 1 },
                { L"▷ 공군 기지", 1 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"정부",
            12,
            {{
                { L"▷ 궁전", 1 },
                { L"▷ 이민국", 2 },
                { L"▷ 세관", 2 },
                { L"▷ 고등 법원", 2 },
                { L"▷ 은행", 2 },
                { L"▷ 부동산청", 1 },
                { L"▷ 노동부", 1 },
                { L"▷ 관공서", 1 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"장식물",
            40,
            {{
                { L"▷ 공원", 10 },
                { L"▷ 분수", 8 },
                { L"▷ 동상", 7 },
                { L"▷ 광장", 5 },
                { L"▷ 꽃밭", 4 },
                { L"▷ 나무길", 3 },
                { L"▷ 기념비", 2 },
                { L"▷ 조각상", 1 }
            }}
        },
        FBuildingCategoryUiEntry
        {
            L"세계 불가사의",
            4,
            {{
                { L"▷ 에펠 탑", 1 },
                { L"▷ 노이슈반슈타인 성", 1 },
                { L"▷ 타지마할", 1 },
                { L"▷ 브란덴부르크 문", 1 },
                { L"", 0 },
                { L"", 0 },
                { L"", 0 },
                { L"", 0 }
            }}
        }
    };

    const int SelectedBuildingCategoryIndex =
        (std::max)(0,
            (std::min)(
                static_cast<int>(BuildingCategories.size()) - 1,
                Widget.mSelectedBuildingCategoryIndex));
    const FBuildingCategoryUiEntry& SelectedBuildingCategory =
        BuildingCategories[static_cast<size_t>(SelectedBuildingCategoryIndex)];

    for (int Index = 0; Index < static_cast<int>(BuildingCategories.size()); ++Index)
    {
        SetDetailRowData(
            Widget.mBuildingRows[Index],
            BuildingCategories[Index].Label,
            std::to_wstring(BuildingCategories[Index].Count),
            Index == SelectedBuildingCategoryIndex);
    }

    if (auto Title = Widget.mBuildingCategoryTitle.lock())
        Title->SetText(SelectedBuildingCategory.Label);

    for (int Index = 0; Index < GBuildingDetailCount; ++Index)
    {
        const FBuildingDetailEntry& Detail =
            SelectedBuildingCategory.Details[static_cast<size_t>(Index)];
        const bool HasLabel = Detail.Label != nullptr && Detail.Label[0] != L'\0';

        SetDetailRowData(
            Widget.mBuildingDetails[Index],
            HasLabel ? Detail.Label : L"",
            HasLabel ? std::to_wstring(Detail.Count) : L"",
            false);

        if (auto Background = Widget.mBuildingDetails[Index].Background.lock())
            Background->SetTint(1.f, 1.f, 1.f, HasLabel ? 0.88f : 0.f);
        if (auto Label = Widget.mBuildingDetails[Index].Label.lock())
            Label->SetEnable(HasLabel);
        if (auto Value = Widget.mBuildingDetails[Index].Value.lock())
            Value->SetEnable(HasLabel);
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
