#pragma once

#include "Citizen/CitizenTypes.h"

namespace GameBalanceTuning
{
    void RegisterRuntimeConfig();
    bool ReloadIfChanged(float DeltaTime);
    unsigned long long GetRuntimeConfigGeneration();

    namespace Almanac
    {
        extern double SatisfactionClampMin;
        extern double SatisfactionClampMax;
        extern double SatisfactionLiftReferenceValue;
        extern double SatisfactionLiftScale;
        extern double SatisfactionLiftBase;
        extern double SatisfactionLiftMin;
        extern double SatisfactionLiftMax;
        extern double SatisfactionBaselinePullScale;
        extern double SatisfactionBaselinePullMin;
        extern double SatisfactionBaselinePullMax;
        extern double SatisfactionPoint0LiftMultiplier;
        extern double SatisfactionPoint0BaselineMultiplier;
        extern double SatisfactionPoint1LiftMultiplier;
        extern double SatisfactionPoint1BaselineMultiplier;
        extern double SatisfactionPoint2LiftMultiplier;
        extern double SatisfactionPoint2BaselineMultiplier;
        extern double SatisfactionTierThreshold0;
        extern double SatisfactionTierThreshold1;
        extern double SatisfactionTierThreshold2;
        extern double SatisfactionTierThreshold3;
    }

    namespace Politics
    {
        struct FWealthTierTuning
        {
            float TaxSensitivity = 1.f;
            float ExpectedLivingStandard = 67.f;
        };

        extern double ElectionWarningCriticalThreshold;
        extern double ElectionWarningCautionThreshold;
        extern double ElectionWarningCheckThreshold;
        constexpr int ElectionWarningActiveWindowDays = 180;

        inline bool IsElectionWarningCritical(double Score)
        {
            return Score >= ElectionWarningCriticalThreshold;
        }

        inline bool IsElectionWarningCaution(double Score)
        {
            return Score >= ElectionWarningCautionThreshold;
        }

        inline bool IsElectionWarningCheck(double Score)
        {
            return Score >= ElectionWarningCheckThreshold;
        }

        inline bool HasElectionWarning(int DaysUntilNextElection, double Score)
        {
            return DaysUntilNextElection >= 0 &&
                DaysUntilNextElection <= ElectionWarningActiveWindowDays &&
                IsElectionWarningCheck(Score);
        }

        const FWealthTierTuning& GetWealthTier(
            ECitizenWealthLevel WealthLevel);
    }
}
