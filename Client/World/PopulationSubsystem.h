#pragma once

#include "World/WorldSubsystem.h"

class CPopulationSubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    void Reset();
    void Tick(float DeltaTime);
    void SpawnCitizenOrb();
    void ReassignCitizenNeeds();
    void LoadCitizenAnimation2D();

    int SpawnedNpcCount = 0;
    float NpcSpawnAccum = 0.f;
    float CitizenReassignAccum = 0.f;
};
