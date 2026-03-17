#pragma once

#include "CitizenInfoConstants.h"
#include "Vector2.h"
#include <array>
#include <string>

enum class ECitizenInfoPanelMode : int
{
    Citizen = 0,
    Building
};

struct FCitizenInfoCitizenState
{
    CitizenInfoConstants::ECitizenInfoTab SelectedTab =
        CitizenInfoConstants::ECitizenInfoTab::Overview;
    std::string TrackedCitizenName;
    std::array<float, 9>
        PoliticsSatisfactionFillRatios = {};
    float PoliticsSupportRatio = 0.f;
};

struct FCitizenInfoBuildingState
{
    CitizenInfoConstants::EBuildingInfoTab SelectedTab =
        CitizenInfoConstants::EBuildingInfoTab::Overview;
    std::string TrackedBuildingName;
    bool CustomsModeSelectionOpen = false;
    bool OverviewMetricScrollActive = false;
    int OverviewMetricScrollOffset = 0;
    int OverviewMetricScrollVisibleLineCount = 0;
    int OverviewMetricScrollTotalLineCount = 0;
    int OverviewMetricScrollFirstRowIndex = 0;
};

struct FCitizenInfoState
{
    ECitizenInfoPanelMode PanelMode = ECitizenInfoPanelMode::Citizen;
    FCitizenInfoCitizenState Citizen;
    FCitizenInfoBuildingState Building;
    float PanelWidth = 360.f;
    float PanelHeight = 720.f;
    float PanelTop = 56.f;
    FVector2 RequestedScreenPos = FVector2(0.f, 0.f);
};
