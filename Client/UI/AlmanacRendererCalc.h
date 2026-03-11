#pragma once

#include "AlmanacDataProvider.h"
#include "AlmanacRendererInternal.h"

namespace AlmanacRendererCalc
{
    struct FConflictPageComputedData
    {
        double UnemploymentRate = 0.0;
        double FiscalStress = 0.0;
        double Stability = 0.0;
        double ControlStrength = 0.0;
        bool ElectionWarningActive = false;
        FVector4 ElectionWarningTint = FVector4(0.31f, 0.27f, 0.21f, 1.f);
        std::wstring ElectionWarningSummary;
        std::wstring TaxEventWorldEffectSummary;
    };

    double ClampSatisfactionValue(double Value);
    std::array<float, GSatisfactionGraphPointCount> BuildSatisfactionTrend(
        double CurrentValue,
        double BaselineValue,
        double LiftBias = 0.0);
    float ResolveGraphY(
        float GraphTop,
        float GraphHeight,
        float Value);
    float ResolveGraphYInRange(
        float GraphTop,
        float GraphHeight,
        float Value,
        float MinValue,
        float MaxValue);
    FVector4 GetSatisfactionTint(int Index);
    int RoundToInt(double Value);
    int ResolvePopulationSatisfactionTier(double Value);
    std::array<float, GPopulationTrendPointCount> BuildPopulationTrend(
        int CurrentPopulation,
        int Growth12M,
        int Decline12M);
    std::array<float, GPopulationChangeBarCount> BuildPopulationChangeSeries(
        float BaseValue,
        bool Positive);
    std::array<float, GPopulationDistributionBarCount>
        BuildPopulationDistributionSeries(
            float CurrentValue,
            float StartRatio,
            float EndRatio);
    std::array<float, GPopulationDistributionBarCount> BuildPopulationDetailTrend(
        float StartValue,
        float EndValue,
        float WaveA,
        float WaveB);
    std::array<float, GPopulationTrendPointCount> BuildPopulationRateTrend(
        float StartValue,
        float EndValue,
        float WaveA,
        float WaveB);
    std::array<int, 5> AllocatePopulationBuckets(
        int TotalCount,
        const std::array<float, 5>& Weights);
    std::array<int, 5> BuildHomelessFamilyWealthBuckets(
        int HomelessFamilyCount,
        const int HomelessWealthCount[3]);
    std::array<int, 5> BuildCitizenWealthBuckets(
        int CitizenCount,
        const int CitizenWealthCount[3]);
    std::array<float, GPopulationDistributionBarCount>
        BuildPopulationHistoricalLayer(
            float StartValue,
            float PeakValue,
            float EndValue,
            float WaveA,
            float WaveB);

    FConflictPageComputedData BuildConflictPageComputedData(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
}
