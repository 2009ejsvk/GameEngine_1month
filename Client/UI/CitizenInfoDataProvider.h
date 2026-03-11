#pragma once

#include "../Building/BuildingTypes.h"
#include "../Citizen/CitizenTypes.h"
#include <array>
#include <memory>
#include <string>
#include <tchar.h>
#include <vector>

class CWorld;

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

    struct FWarehouseSlotRecord
    {
        EResourceType Type = EResourceType::None;
        int Stock = 0;
        int Capacity = 0;
    };

    struct FCitizenInfoBuildingRecord
    {
        bool Valid = false;
        std::wstring ObjectName;
        std::wstring DisplayName;
        std::wstring CategoryName;
        std::string BuildingId;
        bool Residential = false;
        bool WorkProvider = false;
        bool FoodProvider = false;
        bool EntertainmentProvider = false;
        bool UsesResourceStock = false;
        bool Harbor = false;
        bool Warehouse = false;
        bool IsRoad = false;
        bool CanGenerateWorkOutput = false;
        int Capacity = 0;
        int BudgetLevel = 3;
        int DaysInMonth = 30;
        int MonthlyWageCost = 0;
        int MonthlyUpkeepCost = 0;
        int DailyWageCost = 0;
        int DailyUpkeepCost = 0;
        int HousingCap = 100;
        int JobCap = 100;
        int FoodCap = 100;
        int FunCap = 100;
        int ResourceStock = 0;
        int ExportableStock = 0;
        int MaxResourceStock = 0;
        int TotalProducedPowerMW = 0;
        int TotalRequiredPowerMW = 0;
        float BudgetScale = 1.f;
        float AccessibilityScore = 0.f;
        float HarborShipProgressPercent = 0.f;
        ECitizenEducationLevel RequiredEducationLevel =
            ECitizenEducationLevel::Uneducated;
        std::vector<FWarehouseSlotRecord> WarehouseSlots;
        std::vector<std::string> Residents;
        std::vector<std::string> AssignedEmployees;
        std::vector<std::string> WorkingEmployees;
        std::vector<std::string> AssignedVisitors;
        std::vector<std::string> ArrivedVisitors;
        std::vector<std::string> IncomingVisitors;
    };

    struct FCitizenInfoCitizenRecord
    {
        bool Valid = false;
        std::string Name;
        FNpcSatisfaction Satisfaction;
        FCitizenIdentityProfile IdentityProfile;
        FNpcPoliticalProfile PoliticalProfile;
        ECitizenState State = ECitizenState::Wander;
        std::string HomeBuildingName;
        std::string WorkBuildingName;
        std::string FoodBuildingName;
        std::string FoodVisitBuildingName;
        std::string FunBuildingName;
    };

    class ICitizenInfoQuerySource
    {
    public:
        virtual ~ICitizenInfoQuerySource() = default;

        virtual bool TryGetBuildingRecord(
            const std::string& BuildingName,
            FCitizenInfoBuildingRecord& OutRecord) const = 0;
        virtual bool TryGetCitizenRecord(
            const std::string& CitizenName,
            FCitizenInfoCitizenRecord& OutRecord) const = 0;
        virtual std::wstring ResolveBuildingDisplayName(
            const std::string& BuildingName) const = 0;
    };

    FCitizenInfoSnapshot BuildCitizenSnapshot(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FCitizenIdentityProfile& IdentityProfile,
        const FNpcPoliticalProfile& PoliticalProfile,
        int TabIndex = 0);

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex = 0);

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex = 0);

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex);

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex);
}
