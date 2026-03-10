#pragma once

#include "CitizenTypes.h"

namespace CitizenPolitics
{
    // 시민 생성 시 정치 성향을 무작위로 초기화한다.
    // TickAccum도 분산을 위해 초기화한다.
    void Init(FNpcPoliticalProfile& Profile, float& TickAccum);

    // 매 Update마다 호출. 정치 이동 주기마다 축 하나를 재추출한다.
    // OverallSatisfaction은 성향 이동 확률 보정에 사용된다.
    void Update(
        FNpcPoliticalProfile& Profile,
        float& TickAccum,
        float DeltaTime,
        float OverallSatisfaction);

    // 단일 정치 선택 항목을 만족도 기반으로 갱신한다.
    void UpdateChoice(FNpcPoliticalChoice& Choice, float OverallSatisfaction);
}
