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

    constexpr int GBudgetLevelCount = 12;
    constexpr int GBudgetDisplayCount = 5;
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
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\ActionIcons\\T_ICO_action_relocate.png");
    constexpr const TCHAR* GFocusActionIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\ActionIcons\\T_ICO_action_focus.png");
    constexpr const TCHAR* GOverviewEmptyResidentIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_population.png");
    constexpr const TCHAR* GSectionDividerTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\4_Modern\\InfoPopUp\\T_agent_deco.png");
    static const auto& GCitizenPoliticsSectionTexture = GMenuDetailFrameTexture;
    static const auto& GCitizenPoliticsBarRailTexture = GScrollTrackTexture;
    static const auto& GCitizenPoliticsBarFillTexture = GScrollThumbTexture;
    constexpr const TCHAR* GCitizenPoliticsSupportIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_political_approval.png");
    static const auto& GCitizenThoughtTitleTexture = GMenuDetailFrameTexture;
    constexpr std::array<const TCHAR*, 4> GOverviewResidentPortraits =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_formalWorker1_m_1.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_formalWorker1_f_2.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_LightDutyWorker2_m_3.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_LightDutyWorker2_f_4.png")
    };
    constexpr const TCHAR* GCitizenActionIcons[GCitizenActionButtonCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ActionIcons\\T_ICO_action_bribe.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ActionIcons\\T_ICO_action_kill.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ActionIcons\\T_ICO_action_doubleBlindTrial.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ActionIcons\\T_ICO_action_arrest.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ActionIcons\\T_ICO_action_hospitalize.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ActionIcons\\T_ICO_action_banish.png")
    };
    constexpr const TCHAR* GMetricValueBgTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_standard.png");
    constexpr int GCitizenOverviewMetricIconCount = 8;
    constexpr const TCHAR* GCitizenOverviewMetricIcons[GCitizenOverviewMetricIconCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\AgentInfoIcons\\T_ICO_agentInfo.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingStatus.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_date.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_population.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_money.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_education_random.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\AlmanacIcons\\T_ICO_lowestHappiness_work.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\AlmanacIcons\\T_ICO_lowestHappiness_housing.png")
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
