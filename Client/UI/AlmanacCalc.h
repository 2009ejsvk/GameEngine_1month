#pragma once

#include "AlmanacDataProvider.h"
#include "AlmanacRendererConstants.h"
#include "Vector4.h"
#include <array>
#include <string>

namespace AlmanacCalc
{
    struct FConflictPageComputedData
    {
        double UnemploymentRate = 0.0;
        double FiscalStress = 0.0;
        double Stability = 0.0;
        double ControlStrength = 0.0;
        bool ElectionWarningActive = false;
        FVector4 ElectionWarningTint = FVector4(0.f, 0.f, 0.f, 0.f);
        std::wstring ElectionWarningSummary;
        std::wstring TaxEventWorldEffectSummary;
    };

    double ClampSatisfactionValue(double Value);
    std::wstring BuildElectionWarningSummary(
        bool GameLost,
        int DaysUntilNextElection,
        double ElectionWarningScore,
        const FTaxPolicyEventStatus& TaxEventStatus);
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
        const int HomelessWealthCount[GCitizenWealthLevelCount]);
    std::array<int, 5> BuildCitizenWealthBuckets(
        int CitizenCount,
        const int CitizenWealthCount[GCitizenWealthLevelCount]);
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
