#include "AlmanacTheme.h"
#include "AlmanacCalc.h"
#include "TropicoUiTheme.h"

namespace
{
    const FVector4 GConflictHeadlineNeutralTint(0.82f, 0.92f, 0.76f, 0.98f);
    const FVector4 GConflictHeadlineHighRiskTint(0.96f, 0.48f, 0.38f, 0.98f);
    const FVector4 GConflictHeadlineMediumRiskTint(0.96f, 0.78f, 0.28f, 0.98f);
    const FVector4 GFiscalEmergencyTint(0.94f, 0.54f, 0.40f, 0.98f);
    const FVector4 GWorldCrisisTint(0.94f, 0.70f, 0.30f, 0.98f);
    const FVector4 GBudgetCrisisTint(0.94f, 0.54f, 0.40f, 0.98f);
    const FVector4 GTaxEventTint(0.94f, 0.76f, 0.32f, 0.98f);
}

namespace AlmanacTheme
{
    FVector4 GetSatisfactionTint(int Index)
    {
        return TropicoUiTheme::GetAlmanacSatisfactionTint(Index);
    }

    FVector4 GetElectionWarningTint(double Score)
    {
        if (Score >= 0.78)
            return TropicoUiTheme::GStatusDangerTint;
        if (Score >= 0.52)
            return TropicoUiTheme::GStatusWarningTint;
        if (Score >= 0.32)
            return TropicoUiTheme::GStatusCautionTint;
        return TropicoUiTheme::GStatusSuccessTint;
    }

    FVector4 GetConflictHeadlineTint(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot,
        const AlmanacCalc::FConflictPageComputedData& ComputedData)
    {
        const bool HasActiveWorldCrisis = Snapshot.WorldCrisisStatus.Active;

        if (Snapshot.RebelRiskScore >= 66.0)
            return GConflictHeadlineHighRiskTint;
        if (Snapshot.RebelRiskScore >= 33.0)
            return GConflictHeadlineMediumRiskTint;
        if (HasActiveWorldCrisis)
        {
            return Snapshot.WorldCrisisStatus.Type ==
                    EWorldCrisisType::FiscalEmergency ?
                GFiscalEmergencyTint :
                GWorldCrisisTint;
        }
        if (Snapshot.TaxEventStatus.Active)
        {
            return Snapshot.TaxEventStatus.Type ==
                    ETaxPolicyEventType::BudgetCrisis ?
                GBudgetCrisisTint :
                GTaxEventTint;
        }
        if (ComputedData.ElectionWarningActive)
            return ComputedData.ElectionWarningTint;

        return GConflictHeadlineNeutralTint;
    }
}
