#pragma once

#include <cstddef>

enum class EScenarioEvent
{
    None = 0,
    ForeignDemand_USA,
    ForeignDemand_USSR,
    Crisis_LaborStrike,
    FactionDemand_Religious,
    ElectionPromptPopup
};

struct FScenarioEvent
{
    int Year = 0;
    int Month = 0;
    int Day = 0;
    EScenarioEvent Type = EScenarioEvent::None;

    bool IsValid() const
    {
        return Type != EScenarioEvent::None;
    }
};

class CScenarioRunner
{
public:
    void Init();
    FScenarioEvent Tick(int Year, int Month, int Day);

private:
    size_t mNextEventIndex = 0;
};
