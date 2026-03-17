#include "ScenarioRunner.h"

namespace
{
    int CompareDate(
        int LeftYear,
        int LeftMonth,
        int LeftDay,
        int RightYear,
        int RightMonth,
        int RightDay)
    {
        if (LeftYear != RightYear)
            return LeftYear < RightYear ? -1 : 1;

        if (LeftMonth != RightMonth)
            return LeftMonth < RightMonth ? -1 : 1;

        if (LeftDay != RightDay)
            return LeftDay < RightDay ? -1 : 1;

        return 0;
    }

    static const FScenarioEvent GScenarioEvents[] =
    {
        { 1962, 2, 1, EScenarioEvent::ForeignDemand_USA },
        { 1962, 6, 1, EScenarioEvent::ForeignDemand_USSR },
        { 1963, 3, 1, EScenarioEvent::Crisis_LaborStrike },
        { 1964, 1, 1, EScenarioEvent::FactionDemand_Religious },
        { 1965, 3, 1, EScenarioEvent::ElectionPromptPopup }
    };

    constexpr size_t GScenarioEventCount =
        sizeof(GScenarioEvents) / sizeof(GScenarioEvents[0]);
}

void CScenarioRunner::Init()
{
    mNextEventIndex = 0;
}

FScenarioEvent CScenarioRunner::Tick(int Year, int Month, int Day)
{
    while (mNextEventIndex < GScenarioEventCount)
    {
        const FScenarioEvent& Event = GScenarioEvents[mNextEventIndex];
        const int CompareResult =
            CompareDate(
                Year,
                Month,
                Day,
                Event.Year,
                Event.Month,
                Event.Day);

        if (CompareResult < 0)
            break;

        ++mNextEventIndex;

        if (CompareResult == 0)
            return Event;
    }

    return FScenarioEvent();
}
