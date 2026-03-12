#pragma once

#include "BuildingTypes.h"
#include <tchar.h>

namespace BuildingCategoryInfo
{
    constexpr int GBuildingCategoryCount =
        static_cast<int>(EBuildingCategory::Count);

    constexpr const wchar_t* const GDisplayNames[GBuildingCategoryCount] =
    {
        L"교통 및 기반시설",
        L"음식 및 자원",
        L"산업",
        L"주거지",
        L"오락",
        L"미디어 및 교육",
        L"관광업",
        L"공익 서비스",
        L"호화 오락",
        L"습격 및 군사",
        L"정부 및 재정"
    };

    constexpr const TCHAR* const GTabIconPaths[GBuildingCategoryCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_infrastructure.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_raw_resources.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_industry.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_housing.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_entertainment.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_mediaEducation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_tourism.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_publicServices.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_luxuryEntertainment.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_military.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_government.png")
    };

    constexpr EBuildingCategory
        GBuildMenuDisplayOrder[GBuildingCategoryCount] =
    {
        EBuildingCategory::Infrastructure,
        EBuildingCategory::FoodResource,
        EBuildingCategory::Industry,
        EBuildingCategory::Housing,
        EBuildingCategory::Entertainment,
        EBuildingCategory::LuxuryEntertainment,
        EBuildingCategory::MediaEducation,
        EBuildingCategory::Tourism,
        EBuildingCategory::PublicService,
        EBuildingCategory::Military,
        EBuildingCategory::GovernmentFinance
    };

    inline bool IsValidCategoryIndex(int Index)
    {
        return Index >= 0 && Index < GBuildingCategoryCount;
    }

    inline int ToIndex(EBuildingCategory Category)
    {
        return static_cast<int>(Category);
    }

    inline const wchar_t* GetDisplayName(int Index)
    {
        return IsValidCategoryIndex(Index) ? GDisplayNames[Index] : L"";
    }

    inline const wchar_t* GetDisplayName(EBuildingCategory Category)
    {
        return GetDisplayName(ToIndex(Category));
    }

    inline const TCHAR* GetTabIconPath(int Index)
    {
        return IsValidCategoryIndex(Index) ? GTabIconPaths[Index] : nullptr;
    }

    inline const TCHAR* GetTabIconPath(EBuildingCategory Category)
    {
        return GetTabIconPath(ToIndex(Category));
    }

    inline EBuildingCategory GetBuildMenuCategory(int DisplayIndex)
    {
        return IsValidCategoryIndex(DisplayIndex) ?
            GBuildMenuDisplayOrder[DisplayIndex] :
            EBuildingCategory::Infrastructure;
    }

    inline int GetBuildMenuDisplayIndex(EBuildingCategory Category)
    {
        for (int Index = 0; Index < GBuildingCategoryCount; ++Index)
        {
            if (GBuildMenuDisplayOrder[Index] == Category)
                return Index;
        }

        return 0;
    }
}
