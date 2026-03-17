#pragma once

#include <algorithm>
#include <string>
#include <unordered_set>

struct FKnowledgeState
{
    int Points = 0;
    int DailyGeneration = 0;
    std::unordered_set<std::wstring> UnlockedResearchKeys;

    void Reset()
    {
        Points = 0;
        DailyGeneration = 0;
        UnlockedResearchKeys.clear();
    }
};

inline bool IsResearchUnlocked(
    const FKnowledgeState& State,
    const std::wstring& Key)
{
    if (Key.empty())
        return true;

    return State.UnlockedResearchKeys.find(Key) !=
        State.UnlockedResearchKeys.end();
}

inline bool CanUnlockResearch(
    const FKnowledgeState& State,
    const std::wstring& Key,
    int Cost)
{
    if (Key.empty() || IsResearchUnlocked(State, Key))
        return true;

    return State.Points >= (std::max)(0, Cost);
}

inline bool TryUnlockResearch(
    FKnowledgeState& State,
    const std::wstring& Key,
    int Cost)
{
    if (Key.empty())
        return true;

    if (IsResearchUnlocked(State, Key))
        return true;

    const int SafeCost = (std::max)(0, Cost);

    if (State.Points < SafeCost)
        return false;

    State.Points -= SafeCost;
    State.UnlockedResearchKeys.insert(Key);
    return true;
}
