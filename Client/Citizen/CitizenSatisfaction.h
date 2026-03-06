#pragma once

#include "CitizenTypes.h"

namespace CitizenSatisfaction
{
    // 각 욕구 수치로부터 Overall 만족도를 재계산한다.
    // 가중 평균과 최저 욕구 패널티를 조합한다.
    void RecalculateOverall(FNpcSatisfaction& Satisfaction);
}
