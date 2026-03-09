#pragma once

namespace MainWorldConfig
{
    constexpr int GInitialNpcCount = 10;
    constexpr int GMaxNpcCount = 2000;
    constexpr float GNpcSpawnInterval = 5.f;
    constexpr float GCitizenReassignInterval = 0.5f;
    constexpr long long GInitialNationalBudget = 500000;
    constexpr int GInitialElectionLeadYears = 2;
    constexpr int GElectionIntervalYears = 4;
    constexpr int GElectionMonth = 1;
    constexpr int GElectionDay = 1;
    constexpr int GSimulationStartYear = 2000;
    constexpr int GSimulationStartMonth = 1;
    constexpr int GSimulationStartDay = 1;
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
    constexpr const char* GStarterDormitoryObjectName =
        "StarterDormitory";
    constexpr const char* GStarterTeamsterObjectName =
        "StarterTeamsterOffice";
    constexpr const char* GStarterRanchObjectName = "StarterRanch";
    constexpr const char* GStarterFarmObjectName = "StarterLargeFarm";
    constexpr const char* GStarterHarborObjectName = "StarterHarbor";
}
