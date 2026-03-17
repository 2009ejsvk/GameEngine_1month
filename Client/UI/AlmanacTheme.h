#pragma once

#include "AlmanacDataProvider.h"
#include "Vector4.h"

namespace AlmanacCalc
{
    struct FConflictPageComputedData;
}

namespace AlmanacTheme
{
    FVector4 GetSatisfactionTint(int Index);
    FVector4 GetElectionWarningTint(double Score);
    FVector4 GetConflictHeadlineTint(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot,
        const AlmanacCalc::FConflictPageComputedData& ComputedData);
}
