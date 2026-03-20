#pragma once

#include "UIStrings.h"

namespace CitizenInfoConstants
{
    constexpr const char GHydroponicFarmBuildingId[] = "build_2_10";

    enum class ECitizenInfoTab
    {
        Overview = 0,
        Politics,
        Thoughts,
        Count
    };

    enum class EBuildingInfoTab
    {
        Overview = 0,
        Statistics,
        Upgrades,
        Efficiency,
        Information,
        Count
    };

    constexpr int GCitizenTabCount =
        static_cast<int>(ECitizenInfoTab::Count);
    constexpr int GBuildingTabCount =
        static_cast<int>(EBuildingInfoTab::Count);
    constexpr int GTabButtonCount = GBuildingTabCount;

    inline const std::wstring& GetCitizenTabLabel(int Index)
    {
        switch (Index)
        {
        case 0:
            return UIStrings::Get(L"citizen_info.tab.citizen.overview");
        case 1:
            return UIStrings::Get(L"citizen_info.tab.citizen.politics");
        case 2:
            return UIStrings::Get(L"citizen_info.tab.citizen.thoughts");
        default:
            return UIStrings::Get(L"citizen_info.tab.citizen.overview");
        }
    }

    inline const std::wstring& GetCitizenPageTitle(int Index)
    {
        switch (Index)
        {
        case 0:
            return UIStrings::Get(L"citizen_info.page.citizen.overview");
        case 1:
            return UIStrings::Get(L"citizen_info.page.citizen.politics");
        case 2:
            return UIStrings::Get(L"citizen_info.page.citizen.thoughts");
        default:
            return UIStrings::Get(L"citizen_info.page.citizen.overview");
        }
    }

    inline const std::wstring& GetBuildingTabLabel(int Index)
    {
        switch (Index)
        {
        case 0:
            return UIStrings::Get(L"citizen_info.tab.building.overview");
        case 1:
            return UIStrings::Get(L"citizen_info.tab.building.statistics");
        case 2:
            return UIStrings::Get(L"citizen_info.tab.building.upgrades");
        case 3:
            return UIStrings::Get(L"citizen_info.tab.building.efficiency");
        case 4:
            return UIStrings::Get(L"citizen_info.tab.building.information");
        default:
            return UIStrings::Get(L"citizen_info.tab.building.overview");
        }
    }

    inline const std::wstring& GetBuildingPageTitle(int Index)
    {
        switch (Index)
        {
        case 0:
            return UIStrings::Get(L"citizen_info.page.building.overview");
        case 1:
            return UIStrings::Get(L"citizen_info.page.building.statistics");
        case 2:
            return UIStrings::Get(L"citizen_info.page.building.upgrades");
        case 3:
            return UIStrings::Get(L"citizen_info.page.building.efficiency");
        case 4:
            return UIStrings::Get(L"citizen_info.page.building.information");
        default:
            return UIStrings::Get(L"citizen_info.page.building.overview");
        }
    }
}
