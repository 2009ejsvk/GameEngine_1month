#pragma once

#include "ScenarioConfig.h"

namespace MainWorldConfig
{
    constexpr int GInitialNpcCount = ScenarioConfig::GInitialNpcCount;
    constexpr int GMaxNpcCount = 2000;
    constexpr float GNpcSpawnInterval = 5.f;
    constexpr float GCitizenReassignInterval = 0.5f;
    constexpr long long GInitialNationalBudget = ScenarioConfig::GInitialBudget;
    constexpr int GInitialElectionLeadYears = ScenarioConfig::GElectionLeadYears;
    constexpr int GElectionIntervalYears = ScenarioConfig::GElectionIntervalYears;
    constexpr int GElectionMonth = ScenarioConfig::GElectionMonth;
    constexpr int GElectionDay = ScenarioConfig::GElectionDay;
    constexpr int GSimulationStartYear = ScenarioConfig::GStartYear;
    constexpr int GSimulationStartMonth = ScenarioConfig::GStartMonth;
    constexpr int GSimulationStartDay = ScenarioConfig::GStartDay;
    constexpr float GSecondsPerSimulationDay = 2.f;
    constexpr float GPoliticalSnapshotInterval = 1.f;
    constexpr int GCitizenDirectionCount = 8;
    constexpr float GCitizenFrameWidth = 16.f;
    constexpr float GCitizenFrameHeight = 19.f;
    constexpr float GCitizenRightBottomStartX = 15.f;
    constexpr float GCitizenDirectionStepX = 16.f;
    constexpr float GCitizenBlueWalkTopY = 125.f;
    constexpr float GCitizenBlueIdleTopY = 148.f;
    constexpr float GCitizenBlueWalkReverseTopY = 173.f;
    constexpr float GCitizenRedWalkTopY = 221.f;
    constexpr float GCitizenRedIdleTopY = 244.f;
    constexpr float GCitizenRedWalkReverseTopY = 269.f;
    constexpr const char* GCitizenBlueAnimationPrefix = "CitizenBlue";
    constexpr const char* GCitizenRedAnimationPrefix = "CitizenRed";
    constexpr const char* GCitizenSheetTextureName =
        "CitizenSmall8DirectionSheet";
    constexpr const TCHAR* GCitizenSheetTextureFile =
        TEXT("Small-8-Direction-Characters_by_AxulArt.png");
    constexpr const char* GStarterTeamsterObjectName =
        "StarterTeamsterOffice";
    constexpr const char* GStarterAlfalfaFarmObjectName =
        "StarterAlfalfaFarm";
    constexpr const char* GStarterFarmObjectName = "StarterFarm";
    constexpr const char* GStarterTenementAObjectName =
        "StarterTenementA";
    constexpr const char* GStarterTenementBObjectName =
        "StarterTenementB";
    constexpr const char* GStarterHarborObjectName = "StarterHarbor";
}
