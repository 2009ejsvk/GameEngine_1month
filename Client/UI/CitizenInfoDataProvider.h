#pragma once

#include <array>
#include <memory>
#include <string>
#include <tchar.h>

class CWorld;
struct FNpcSatisfaction;
struct FNpcPoliticalProfile;
struct FCitizenIdentityProfile;

namespace CitizenInfoDataProvider
{
    enum class EPanelMode : int
    {
        Citizen = 0,
        Building
    };

    struct FCitizenInfoSnapshot
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
        bool ShowBuildingOverview = false;
        bool ShowBuildingWorkOverview = false;
        bool ShowBuildingVisitorIcons = false;
        bool ShowBuildingMetricRows = false;
        bool ShowBuildingUpgradeCard = false;
        bool ShowBuildingInformationParagraphs = false;
        bool ShowCitizenProfileOverview = false;
        bool ShowCitizenPoliticsOverview = false;
        bool ShowCitizenThoughtsOverview = false;
        bool ShowCitizenActionButtons = false;
        std::wstring HeaderNoteText;
        std::wstring BuildingSubtitleText;
        std::wstring InformationAccentText;
        std::wstring InformationTopText;
        std::wstring InformationBottomText;
        bool ShowOverviewCommandButton = false;
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
        int CitizenPortraitSlotCount = 0;
        int CitizenPortraitOccupiedSlot = -1;
        int CitizenPortraitVariant = 0;
        std::array<std::wstring, 14> OverviewMetricLabels;
        std::array<std::wstring, 14> OverviewMetricValues;
        std::array<bool, 14> OverviewMetricAccentValues = {};
        std::array<std::wstring, 9> CitizenPoliticsSatisfactionLabels;
        std::array<float, 9> CitizenPoliticsSatisfactionRatios = {};
        std::array<std::wstring, 3> CitizenPoliticsOpinionLines;
        float CitizenPoliticsSupportRatio = 0.f;
        std::array<std::wstring, 5> CitizenThoughtLines;
        std::array<std::wstring, 6> CitizenActionLabels;
        std::wstring CitizenFooterText;
        std::wstring UpgradeCardTitle;
        std::wstring UpgradeCardDescription;
        const TCHAR* UpgradeCardIconPath = nullptr;
        std::string UpgradeCardIconTextureKey;
    };

    FCitizenInfoSnapshot BuildCitizenSnapshot(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FCitizenIdentityProfile& IdentityProfile,
        const FNpcPoliticalProfile& PoliticalProfile,
        int TabIndex = 0);

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& CitizenName,
        int TabIndex = 0);

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& BuildingName,
        int SelectedTabIndex);
}
