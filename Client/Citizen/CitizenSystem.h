#pragma once

class CWorld;

namespace CitizenSystem
{
    // 새 시민 오브를 World에 생성하고 SpawnedNpcCount를 증가시킨다.
    void SpawnCitizenOrb(CWorld* World, int& SpawnedNpcCount);

    // 모든 시민의 집/직장/음식/오락 배정을 재계산한다.
    void ReassignCitizenNeeds(CWorld* World);
}
