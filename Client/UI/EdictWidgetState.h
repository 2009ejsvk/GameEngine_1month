#pragma once

#include <string>
#include <vector>

enum class EEdictUiCategory : int
{
    Colonial = 0,
    WorldWars,
    ColdWar,
    Modern
};

struct FEdictWidgetState
{
    std::vector<int> VisibleEntryIndices;
    bool Open = false;
    EEdictUiCategory SelectedCategory = EEdictUiCategory::Colonial;
    int PreviewEntryIndex = -1;
    int SelectedEntryIndex = -1;
    int CurrentPage = 0;
    int PageCount = 1;
    float PanelWidth = 1120.f;
    float PanelHeight = 760.f;
    bool ShowTaxPolicyPanel = false;
    std::wstring FeedbackMessage;
    int LastLayoutWidth = 0;
    int LastLayoutHeight = 0;
    int LastClickedEntryIndex = -1;
    float DoubleClickTimer = 0.f;
};
