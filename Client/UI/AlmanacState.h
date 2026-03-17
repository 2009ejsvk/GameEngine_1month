#pragma once

enum class EAlmanacPage
{
    Overview = 0,
    Satisfaction,
    Population,
    Economy,
    Resources,
    Politics,
    Foreign,
    Buildings,
    Conflict,
    Count
};

struct FAlmanacState
{
    bool Open = false;
    EAlmanacPage SelectedPage = EAlmanacPage::Overview;
    int SelectedSatisfactionIndex = 0;
    int SelectedPopulationIndex = 0;
    int SelectedEconomyIndex = 0;
    int SelectedResourceIndex = 0;
    int VisibleResourceRowOffset = 0;
    int SelectedPoliticsFactionIndex = 0;
    int SelectedForeignPowerIndex = 0;
    int SelectedBuildingCategoryIndex = 0;
    float PanelWidth = 1060.f;
    float PanelHeight = 744.f;
    float DataRefreshAccum = 0.f;
    bool LayoutDirty = true;
    int LastResolutionWidth = 0;
    int LastResolutionHeight = 0;
};
