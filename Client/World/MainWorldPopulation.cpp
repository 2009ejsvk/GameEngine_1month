#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "../Citizen/CitizenSystem.h"

void CMainWorld::TickCitizenPopulation(float DeltaTime)
{
    mCitizenReassignAccum += DeltaTime;

    while (mCitizenReassignAccum >= MainWorldConfig::GCitizenReassignInterval)
    {
        mCitizenReassignAccum -= MainWorldConfig::GCitizenReassignInterval;
        ReassignCitizenNeeds();
    }

    if (mSpawnedNpcCount >= MainWorldConfig::GMaxNpcCount)
        return;

    mNpcSpawnAccum += DeltaTime;

    while (mNpcSpawnAccum >= MainWorldConfig::GNpcSpawnInterval &&
        mSpawnedNpcCount < MainWorldConfig::GMaxNpcCount)
    {
        mNpcSpawnAccum -= MainWorldConfig::GNpcSpawnInterval;
        SpawnCitizenOrb();
    }
}

void CMainWorld::SpawnCitizenOrb()
{
    CitizenSystem::SpawnCitizenOrb(this, mSpawnedNpcCount);
}

void CMainWorld::ReassignCitizenNeeds()
{
    CitizenSystem::ReassignCitizenNeeds(this);
}
