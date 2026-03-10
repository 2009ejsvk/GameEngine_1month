#pragma once

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
    };

    FCitizenInfoSnapshot BuildCitizenSnapshot(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FCitizenIdentityProfile& IdentityProfile,
        const FNpcPoliticalProfile& PoliticalProfile);

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& CitizenName);

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& BuildingName,
        int SelectedTabIndex);
}
