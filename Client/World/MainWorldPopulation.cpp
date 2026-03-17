#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "../Citizen/CitizenSystem.h"

void CMainWorld::TickCitizenPopulation(float DeltaTime)
{
    mPopulation.CitizenReassignAccum += DeltaTime;

    while (mPopulation.CitizenReassignAccum >= MainWorldConfig::GCitizenReassignInterval)
    {
        mPopulation.CitizenReassignAccum -= MainWorldConfig::GCitizenReassignInterval;
        ReassignCitizenNeeds();
    }

    if (mPopulation.SpawnedNpcCount >= MainWorldConfig::GMaxNpcCount)
        return;

    mPopulation.NpcSpawnAccum += DeltaTime;

    while (mPopulation.NpcSpawnAccum >= MainWorldConfig::GNpcSpawnInterval &&
        mPopulation.SpawnedNpcCount < MainWorldConfig::GMaxNpcCount)
    {
        mPopulation.NpcSpawnAccum -= MainWorldConfig::GNpcSpawnInterval;
        SpawnCitizenOrb();
    }
}

void CMainWorld::SpawnCitizenOrb()
{
    CitizenSystem::SpawnCitizenOrb(this, mPopulation.SpawnedNpcCount);
}

void CMainWorld::ReassignCitizenNeeds()
{
    CitizenSystem::ReassignCitizenNeeds(this);
}

