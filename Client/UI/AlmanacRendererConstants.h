#pragma once

#include "AlmanacWidget.h"
#include "TropicoUiStyle.h"
#include "TropicoUiTheme.h"
#include <Windows.h>
#include <cstddef>

namespace
{
    using namespace TropicoUiAssets;

    constexpr float GDataRefreshIntervalSeconds = 0.5f;
    constexpr int GOverviewCardCount = 11;
    constexpr int GOverviewSectionTitleCount = 6;
    constexpr int GSatisfactionRowCount = 9;
    constexpr int GSatisfactionDetailCount = 6;
    constexpr int GSatisfactionGraphGridLineCount = 4;
    constexpr int GSatisfactionGraphPointCount = 4;
    constexpr int GSatisfactionGraphSegmentCount =
        GSatisfactionGraphPointCount - 1;
    constexpr int GPopulationDetailCount = 14;
    constexpr int GPopulationMetricCount = 5;
    constexpr int GPopulationTrendPointCount = 12;
    constexpr int GPopulationTrendSegmentCount =
        GPopulationTrendPointCount - 1;
    constexpr int GPopulationTrendGridLineCount = 5;
    constexpr int GPopulationTrendXAxisLabelCount = 4;
    constexpr int GPopulationTrendYAxisLabelCount = 6;
    constexpr int GPopulationDistributionBarCount = 24;
    constexpr int GPopulationChangeBarCount = 12;
    constexpr int GPopulationChangeGridLineCount = 5;
    constexpr int GPopulationChangeXAxisLabelCount = 4;
    constexpr int GPopulationChangeYAxisLabelCount = 6;
    constexpr int GEconomyDetailCount = 13;
    constexpr int GEconomyMetricCount = 9;
    constexpr int GEconomyTrendBarCount = 24;
    constexpr int GEconomyTrendSegmentCount =
        GEconomyTrendBarCount - 1;
    constexpr int GEconomyTrendGridLineCount = 6;
    constexpr int GEconomyTrendXAxisLabelCount = 5;
    constexpr int GEconomyTrendYAxisLabelCount = 6;
    constexpr int GEconomyChangeBarCount = 24;
    constexpr int GEconomyChangeGridLineCount = 6;
    constexpr int GEconomyChangeYAxisLabelCount = 6;
    constexpr int GEconomyBreakdownRowCount = 10;
    constexpr int GResourceRowCount = 12;
    constexpr int GResourceProductionBarCount = 24;
    constexpr int GResourceProductionGridLineCount = 3;
    constexpr int GResourceProductionXAxisLabelCount = 4;
    constexpr int GResourceProductionYAxisLabelCount = 3;
    constexpr int GResourceDistributionRowCount = 4;
    constexpr int GResourceDetailCount = 4;
    constexpr int GPoliticsFactionTileCount = 8;
    constexpr int GPoliticsNeutralCount = 4;
    constexpr int GPoliticsSupportRowCount = 4;
    constexpr int GPoliticsDetailCount = 14;
    constexpr int GForeignPowerCount = 5;
    constexpr int GForeignDetailCount = 11;
    constexpr int GForeignMetricCount = 4;
    constexpr int GBuildingRowCount = 13;
    constexpr int GBuildingDetailCount = 8;
    constexpr int GConflictDetailCount = 8;
    constexpr int GConflictMetricCount = 3;

    constexpr const TCHAR* GPanelTexture = GMainMenuPanelTexture;
    constexpr const TCHAR* GContentFrameTexture = GMenuGridFrameTexture;
    constexpr const TCHAR* GTitleRibbonTexture = GMenuTitleRibbonTexture;
    constexpr const TCHAR* GTabTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\IconBackground\\T_icon_background.png");
    constexpr const TCHAR* GTabMarkerTexture = GDropdownArrowTexture;
    constexpr const TCHAR* GRowTexture = GBigTextButtonTexture;
    constexpr const TCHAR* GRowTextureSelected = GBigTextButtonSelectedTexture;
    constexpr const TCHAR* GCardTexture = GSlotCardTexture;
    constexpr const TCHAR* GCardTextureSelected = GSlotCardSelectedTexture;
    constexpr const TCHAR* GRailTrackTexture = GScrollTrackTexture;
    constexpr const TCHAR* GRailThumbTexture = GScrollThumbTexture;
    constexpr const TCHAR* GBarBackTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Gamespeed\\T_gamespeed_timeBar_bg.png");
    constexpr const TCHAR* GBarFillTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Gamespeed\\T_gamespeed_timeBar.png");
    constexpr const TCHAR* GMoneyIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png");
    constexpr const TCHAR* GPopulationIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_population.png");
    constexpr const TCHAR* GApprovalIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_political_approval.png");

    constexpr const TCHAR* GPageTabIcons[static_cast<size_t>(EAlmanacPage::Count)] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Tasks.png"),
        GApprovalIcon,
        GPopulationIcon,
        GMoneyIcon,
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_raw_resources.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Constitution.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Trade.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Construction.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Raids.png")
    };

    constexpr const wchar_t* GPageTitles[static_cast<size_t>(EAlmanacPage::Count)] =
    {
        L"개요",
        L"만족도",
        L"국민",
        L"경제",
        L"자원 개요",
        L"정치",
        L"대외관계",
        L"건물 목록",
        L"분쟁"
    };

    constexpr const wchar_t* GSatisfactionLabels[GSatisfactionRowCount] =
    {
        L"종합 만족도",
        L"음식",
        L"보건",
        L"유흥",
        L"신앙",
        L"주거",
        L"직업",
        L"자유",
        L"치안"
    };

    constexpr const TCHAR* GSatisfactionIcons[GSatisfactionRowCount] =
    {
        GApprovalIcon,
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_restaurant.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_hospital.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_funFairPier.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_church.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_housing.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_industry.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Constitution.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Raids.png")
    };

    constexpr const TCHAR* GPoliticsFactionIcons[GPoliticsFactionTileCount] =
    {
        GMoneyIcon,
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Constitution.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_church.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Raids.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_raw_resources.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_industry.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_highschool.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_political_approval.png")
    };

    constexpr const TCHAR* GForeignPowerIcons[GForeignPowerCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FlagIcons\\T_ICO_almanac_china.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FlagIcons\\T_ICO_almanac_russia.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FlagIcons\\T_ICO_flags_us.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FlagIcons\\T_ICO_almanac_middleEast.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FlagIcons\\T_ICO_flags_eu.png")
    };

    constexpr const wchar_t* GSatisfactionTrendLabels[GSatisfactionGraphPointCount] =
    {
        L"3년전",
        L"2년전",
        L"1년전",
        L"현재 연도"
    };
}
