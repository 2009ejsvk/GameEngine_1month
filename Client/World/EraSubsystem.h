#pragma once

#include "World/WorldSubsystem.h"
#include "../Building/BuildingTypes.h"

class CEraSubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    void Reset();
    void TickEraTransitionState();
    void RefreshEraProgress();
    void RefreshEraTransitionState();
    bool TryExecuteEraTransition(EEraTransitionChoice Choice);

    FEraProgressState EraProgress;
    FEraTransitionState EraTransition;
};
