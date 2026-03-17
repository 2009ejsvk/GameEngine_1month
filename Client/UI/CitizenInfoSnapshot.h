#pragma once

#include "Vector4.h"
#include <array>
#include <string>
#include <tchar.h>

namespace CitizenInfoDataProvider
{
    enum class EPanelMode : int
    {
        Citizen = 0,
        Building
    };

    struct FCitizenInfoSnapshotCommon
    {
        bool Valid = false;
        EPanelMode Mode = EPanelMode::Citizen;
        int SelectedTabIndex = 0;
        int BudgetLevel = 3;
        std::wstring Title = L"-";
        std::wstring Subtitle;
        std::wstring PageTitle;
        std::wstring BodyText;
        std::wstring BudgetText;
        bool ShowBudgetText = true;
        std::string TitleIconTextureKey;
        const TCHAR* TitleIconPath = nullptr;
        bool ShowTitleIcon = false;
        bool ShowSectionRibbon = false;
        bool ShowBudgetControls = false;
        bool ShowActionButtons = false;
        bool ShowTabButtons = false;
        bool ShowHeaderNote = false;
        bool ShowBuildingSubtitle = false;
        bool ShowSectionDivider = false;
        std::wstring HeaderNoteText;
        std::wstring BuildingSubtitleText;
        FVector4 BuildingSubtitleColor = FVector4(0.26f, 0.62f, 0.82f, 1.f);
    };

    struct FCitizenInfoSnapshotBuilding
    {
        bool ShowBuildingOverview = false;
        bool ShowBuildingWorkOverview = false;
        bool ShowBuildingVisitorIcons = false;
        bool ShowBuildingMetricRows = false;
        bool ShowBuildingUpgradeCard = false;
        bool ShowBuildingInformationParagraphs = false;
        std::wstring InformationAccentText;
        std::wstring InformationTopText;
        std::wstring InformationBottomText;
        bool ShowOverviewCommandButton = false;
        bool ShowOverviewWorkModeButton = false;
        bool ShowDemolishButton = true;
        bool ShowMoveButton = true;
        bool ShowFocusButton = true;
        std::wstring OverviewCommandButtonText;
        std::wstring OverviewWorkModeLabel = L"근무 형태";
        std::wstring OverviewWorkModeValue;
        std::wstring OverviewBudgetLabel = L"예산";
        std::wstring OverviewBudgetValue;
        std::wstring OverviewOccupancyLabel = L"거주지";
        std::wstring OverviewOccupancyValue;
        int OverviewResidentCount = 0;
        int OverviewResidentCapacity = 0;
        int OverviewVisitorCount = 0;
        int OverviewVisitorCapacity = 0;
        std::array<std::wstring, 5> BudgetButtonLabels;
        std::array<bool, 5> BudgetButtonEnabled =
        {
            true, true, true, true, true
        };
        std::array<std::wstring, 14> OverviewMetricLabels;
        std::array<std::wstring, 14> OverviewMetricValues;
        std::array<bool, 14> OverviewMetricAccentValues = {};
        bool ShowOverviewMetricScroll = false;
        int OverviewMetricScrollOffset = 0;
        int OverviewMetricScrollVisibleLineCount = 0;
        int OverviewMetricScrollTotalLineCount = 0;
        int OverviewMetricScrollFirstRowIndex = 0;
        std::wstring UpgradeCardTitle;
        std::wstring UpgradeCardDescription;
        const TCHAR* UpgradeCardIconPath = nullptr;
        std::string UpgradeCardIconTextureKey;
    };

    struct FCitizenInfoSnapshotCitizenProfile
    {
        bool ShowCitizenProfileOverview = false;
        bool ShowCitizenActionButtons = false;
        int CitizenPortraitSlotCount = 0;
        int CitizenPortraitOccupiedSlot = -1;
        int CitizenPortraitVariant = 0;
        std::array<std::wstring, 6> CitizenActionLabels;
        std::wstring CitizenFooterText;
    };

    struct FCitizenInfoSnapshotCitizenPolitics
    {
        bool ShowCitizenPoliticsOverview = false;
        std::array<std::wstring, 9> CitizenPoliticsSatisfactionLabels;
        std::array<float, 9> CitizenPoliticsSatisfactionRatios = {};
        std::array<std::wstring, 3> CitizenPoliticsOpinionLines;
        float CitizenPoliticsSupportRatio = 0.f;
    };

    struct FCitizenInfoSnapshotCitizenThoughts
    {
        bool ShowCitizenThoughtsOverview = false;
        std::array<std::wstring, 5> CitizenThoughtLines;
    };

    struct FCitizenInfoSnapshot final :
        public FCitizenInfoSnapshotCommon,
        public FCitizenInfoSnapshotBuilding,
        public FCitizenInfoSnapshotCitizenProfile,
        public FCitizenInfoSnapshotCitizenPolitics,
        public FCitizenInfoSnapshotCitizenThoughts
    {
    };
}
