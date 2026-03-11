#pragma once
#include "CitizenInfoConstants.h"
#include "TropicoUiStyle.h"
#include "UI/TextBlock.h"
#include <array>
#include <memory>

namespace CitizenInfoRendererInternal
{
    using CitizenInfoConstants::GBuildingTabCount;
    using CitizenInfoConstants::GCitizenTabCount;
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

    constexpr int GBudgetLevelCount = 5;
    constexpr int GCitizenActionButtonCount = 6;
    constexpr const TCHAR* GCitizenTabIcons[GCitizenTabCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_formalWorker1_m_1.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_theatre.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingDescription.png")
    };
    constexpr const TCHAR* GBuildingTabIcons[GBuildingTabCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingSettings.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingStatus.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingUpgrades.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingEfficiency.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingDescription.png")
    };
    constexpr const TCHAR* GBudgetIcons[GBudgetLevelCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BudgetIcons\\T_ICO_budget_01.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BudgetIcons\\T_ICO_budget_02.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BudgetIcons\\T_ICO_budget_03.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BudgetIcons\\T_ICO_budget_04.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BudgetIcons\\T_ICO_budget_05.png")
    };
    constexpr const TCHAR* GActionIconButtonTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\IconBackground\\T_icon_background.png");
    constexpr const TCHAR* GMoveActionIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingAddRoute.png");
    constexpr const TCHAR* GCloneActionIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\ConstructionIcons\\T_ICO_blueprint.png");
    constexpr const TCHAR* GOverviewEmptyResidentIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_population.png");
    constexpr const TCHAR* GSectionDividerTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\4_Modern\\InfoPopUp\\T_agent_deco.png");
    constexpr const TCHAR* GCitizenPoliticsSectionTexture = GMenuDetailFrameTexture;
    constexpr const TCHAR* GCitizenPoliticsBarRailTexture = GScrollTrackTexture;
    constexpr const TCHAR* GCitizenPoliticsBarFillTexture = GScrollThumbTexture;
    constexpr const TCHAR* GCitizenPoliticsSupportIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_political_approval.png");
    constexpr const TCHAR* GCitizenThoughtTitleTexture = GMenuDetailFrameTexture;
    constexpr std::array<const TCHAR*, 4> GOverviewResidentPortraits =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_formalWorker1_m_1.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_formalWorker1_f_2.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_LightDutyWorker2_m_3.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_LightDutyWorker2_f_4.png")
    };
    constexpr const TCHAR* GCitizenActionIcons[GCitizenActionButtonCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_weaponsFactory.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_nuclearTesting.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_guardTower.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_asylum.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_spaceProgram.png")
    };

    inline void SetPanelTextStyle(
        const std::shared_ptr<CTextBlock>& Text,
        float FontSize,
        const FVector4& Color,
        ETextAlignH AlignH = ETextAlignH::Left,
        ETextAlignV AlignV = ETextAlignV::Middle,
        bool Shadow = false)
    {
        if (!Text)
            return;

        Text->SetFontSize(FontSize);
        Text->SetAlignH(AlignH);
        Text->SetAlignV(AlignV);
        Text->SetTextColor(Color);
        Text->EnableShadow(Shadow);

        if (Shadow)
        {
            Text->SetShadowOffset(1.f, 1.f);
            Text->SetShadowTextColor(245, 235, 205, 180);
        }
    }
}
