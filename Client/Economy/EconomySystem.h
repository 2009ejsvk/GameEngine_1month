#pragma once

class CWorld;

namespace EconomySystem
{
    struct FDailyResult
    {
        long long WageCost     = 0;
        long long UpkeepCost   = 0;
        long long ExportIncome = 0;
        long long NetChange    = 0;
    };

    // 하루 경제 정산을 수행하고 결과를 반환한다.
    // Budget은 직접 갱신하지 않으며 호출자가 NetChange를 반영한다.
    FDailyResult ApplyDailySettlement(CWorld* World, int DaysInMonth);
}
