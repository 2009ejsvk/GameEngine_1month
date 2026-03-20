#include "TopHudRenderer.h"
#include "TopHudWidget.h"
#include "UIStrings.h"
#include "TropicoUiStyle.h"
#include "UILayoutValues.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include <algorithm>
#include <array>
#include <string>

namespace
{
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

    constexpr int GSpeedButtonCount = 2;
    constexpr int GSpeedStateButtonIndex = 0;
    constexpr int GSpeedMultiplierButtonIndex = 1;
    constexpr int GMenuButtonCount = 8;
    constexpr int GMenuTaskIndex = 0;
    constexpr int GMenuConstructionIndex = 1;
    constexpr int GMenuEdictsIndex = 2;
    constexpr int GMenuEraTransitionIndex = 3;
    constexpr int GMenuTradeIndex = 4;
    constexpr int GMenuAlmanacIndex = 7;
    constexpr int GConstitutionTopicTabCount =
        static_cast<int>(GConstitutionTopicCount);
    constexpr int GConstitutionTabSlotCount =
        GConstitutionTopicTabCount + 1;
    constexpr int GConstitutionOverviewTabNumber = 1;
    constexpr int GConstitutionTopicTabStartNumber =
        GConstitutionOverviewTabNumber + 1;
    constexpr int GConstitutionOptionCardCount = 3;
    const std::array<EConstitutionTopic, GConstitutionTopicCount>
        GConstitutionTabTopics =
    {
        EConstitutionTopic::VotingRights,
        EConstitutionTopic::ArmedForces,
        EConstitutionTopic::ReligionAndState,
        EConstitutionTopic::LaborPolicy,
        EConstitutionTopic::MediaIndependence
    };

    constexpr const TCHAR* GSpeedPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Gamespeed\\T_gamespeed_deco_bg.png");
    constexpr const TCHAR* GTimeBarBackTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Gamespeed\\T_gamespeed_timeBar_bg.png");
    constexpr const TCHAR* GTimeBarFillTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Gamespeed\\T_gamespeed_timeBar.png");
    constexpr const TCHAR* GStatusBarTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Notifications\\T_staticData_bg.png");
    constexpr const TCHAR* GCenterPopupTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\5_MainMenu\\CenterPopUp\\T_center_popUp.png");
    constexpr const TCHAR* GMenuButtonBackgroundTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\IconBackground\\T_icon_background.png");
    constexpr const TCHAR* GStatusMoneyIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png");
    constexpr const TCHAR* GStatusNpcIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_population.png");
    constexpr const TCHAR* GStatusSupportIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_political_approval.png");
    constexpr const TCHAR* GStatusResearchIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Research.png");

    const TCHAR* const GSpeedStateIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ButtonIcons\\T_ICO_gamespeed_pause.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ButtonIcons\\T_ICO_gamespeed_play.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ButtonIcons\\T_ICO_gamespeed_playTwo.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ButtonIcons\\T_ICO_gamespeed_playThree.png")
    };

    const TCHAR* const GSpeedMultiplierIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ButtonIcons\\T_ICO_gamespeed_single.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ButtonIcons\\T_ICO_gamespeed_double.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ButtonIcons\\T_ICO_gamespeed_tripple.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ButtonIcons\\T_ICO_gamespeed_quadruple.png")
    };

    const TCHAR* const GMenuIcons[GMenuButtonCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Tasks.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Construction.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Edicts.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Constitution.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Trade.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Raids.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Research.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Almanac.png")
    };

    const wchar_t* const GMenuLabelKeys[GMenuButtonCount] =
    {
        L"top_hud.menu.mission",
        L"top_hud.menu.construction",
        L"top_hud.menu.edict",
        L"top_hud.menu.constitution",
        L"top_hud.menu.trade",
        L"top_hud.menu.raid",
        L"top_hud.menu.research",
        L"top_hud.menu.almanac"
    };

    void ConfigureIconButtonStyle(const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(0.96f, 0.96f, 0.96f, 0.98f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(1.06f, 1.04f, 0.98f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.86f, 0.84f, 0.74f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.40f, 0.40f, 0.40f, 0.75f));
    }

    void ConfigureSpeedButtonStyle(const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(1.f, 1.f, 1.f, 1.f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(1.08f, 1.04f, 0.96f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.90f, 0.86f, 0.76f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.50f, 0.45f, 0.35f, 0.70f));
    }

    void ConfigureInfoText(
        const std::shared_ptr<CTextBlock>& Text,
        float FontSize,
        unsigned char R,
        unsigned char G,
        unsigned char B)
    {
        if (!Text)
            return;

        Text->SetFontSize(FontSize);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(R, G, B, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(16, 16, 16, 220);
    }

    void ConfigureConstitutionText(
        const std::shared_ptr<CTextBlock>& Text,
        float FontSize,
        ETextAlignH AlignH,
        ETextAlignV AlignV,
        unsigned char R,
        unsigned char G,
        unsigned char B)
    {
        if (!Text)
            return;

        Text->SetFontSize(FontSize);
        Text->SetAlignH(AlignH);
        Text->SetAlignV(AlignV);
        Text->SetTextColor(R, G, B, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(245, 239, 220, 96);
    }

    const TCHAR* GetSpeedStateIconPath(
        const TopHudDataProvider::FTopHudSnapshot& Snapshot)
    {
        return Snapshot.GamePaused ? GSpeedStateIcons[1] : GSpeedStateIcons[0];
    }

    const TCHAR* GetSpeedMultiplierIconPath(
        const TopHudDataProvider::FTopHudSnapshot& Snapshot)
    {
        if (Snapshot.GameSpeedMultiplier >= 4)
            return GSpeedMultiplierIcons[3];

        if (Snapshot.GameSpeedMultiplier >= 3)
            return GSpeedMultiplierIcons[2];

        if (Snapshot.GameSpeedMultiplier >= 2)
            return GSpeedMultiplierIcons[1];

        return GSpeedMultiplierIcons[0];
    }

    FVector4 GetSpeedStateTint(
        const TopHudDataProvider::FTopHudSnapshot& Snapshot)
    {
        if (Snapshot.GamePaused)
            return FVector4(1.10f, 0.78f, 0.56f, 1.f);

        return FVector4(1.15f, 1.00f, 0.52f, 1.f);
    }

    FVector4 GetSpeedMultiplierTint(
        const TopHudDataProvider::FTopHudSnapshot& Snapshot)
    {
        if (Snapshot.GameSpeedMultiplier >= 4)
            return FVector4(1.22f, 1.06f, 0.58f, 1.f);

        return FVector4(1.12f, 1.00f, 0.60f, 1.f);
    }

}

void FTopHudRenderer::CreateWidgets(CTopHudWidget& Widget)
{
    auto SpeedPanel =
        Widget.CreateWidget<CImage>("TopHud_SpeedPanel", 14).lock();

    if (SpeedPanel)
    {
        SpeedPanel->SetTexture("TopHudSpeedPanel", GSpeedPanelTexture);
        SpeedPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mSpeedPanel = SpeedPanel;
    }

    auto TimeBarBack =
        Widget.CreateWidget<CImage>("TopHud_TimeBarBack", 15).lock();

    if (TimeBarBack)
    {
        TimeBarBack->SetTexture("TopHudTimeBarBack", GTimeBarBackTexture);
        TimeBarBack->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mTimeBarBack = TimeBarBack;
    }

    auto TimeBarFill =
        Widget.CreateWidget<CImage>("TopHud_TimeBarFill", 16).lock();

    if (TimeBarFill)
    {
        TimeBarFill->SetTexture("TopHudTimeBarFill", GTimeBarFillTexture);
        TimeBarFill->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mTimeBarFill = TimeBarFill;
    }

    auto DateText =
        Widget.CreateWidget<CTextBlock>("TopHud_DateText", 17).lock();

    if (DateText)
    {
        DateText->SetText(TEXT("1959년 2월 1일"));
        DateText->SetFontSize(20.f);
        DateText->SetAlignH(ETextAlignH::Left);
        DateText->SetAlignV(ETextAlignV::Middle);
        DateText->SetTextColor(245, 235, 210, 255);
        DateText->EnableShadow(true);
        DateText->SetShadowOffset(1.f, 1.f);
        DateText->SetShadowTextColor(18, 18, 18, 220);
        Widget.mDateText = DateText;
    }

    auto BudgetText =
        Widget.CreateWidget<CTextBlock>("TopHud_BudgetText", 20).lock();

    if (BudgetText)
    {
        BudgetText->SetText(TEXT("$0"));
        BudgetText->SetFontSize(20.f);
        BudgetText->SetAlignH(ETextAlignH::Left);
        BudgetText->SetAlignV(ETextAlignV::Middle);
        BudgetText->SetTextColor(245, 235, 210, 255);
        BudgetText->EnableShadow(true);
        BudgetText->SetShadowOffset(1.f, 1.f);
        BudgetText->SetShadowTextColor(16, 16, 16, 220);
        Widget.mBudgetText = BudgetText;
    }

    auto BudgetLabelText =
        Widget.CreateWidget<CTextBlock>("TopHud_BudgetLabelText", 20).lock();

    if (BudgetLabelText)
    {
        BudgetLabelText->SetText(UIStrings::Get(L"top_hud.label.budget").c_str());
        BudgetLabelText->SetFontSize(12.f);
        BudgetLabelText->SetAlignH(ETextAlignH::Left);
        BudgetLabelText->SetAlignV(ETextAlignV::Middle);
        BudgetLabelText->SetTextColor(172, 146, 98, 255);
        BudgetLabelText->EnableShadow(true);
        BudgetLabelText->SetShadowOffset(1.f, 1.f);
        BudgetLabelText->SetShadowTextColor(22, 18, 12, 180);
        Widget.mBudgetLabelText = BudgetLabelText;
    }

    auto StatusBar =
        Widget.CreateWidget<CImage>("TopHud_StatusBar", 19).lock();

    if (StatusBar)
    {
        StatusBar->SetTexture("TopHudStatusBar", GStatusBarTexture);
        StatusBar->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mStatusBar = StatusBar;
    }

    auto StatusMoneyIcon =
        Widget.CreateWidget<CImage>("TopHud_StatusMoneyIcon", 20).lock();

    if (StatusMoneyIcon)
    {
        StatusMoneyIcon->SetTexture(
            "TopHudStatusMoneyIcon",
            GStatusMoneyIconTexture);
        StatusMoneyIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mStatusMoneyIcon = StatusMoneyIcon;
    }

    auto StatusNpcIcon =
        Widget.CreateWidget<CImage>("TopHud_StatusNpcIcon", 20).lock();

    if (StatusNpcIcon)
    {
        StatusNpcIcon->SetTexture(
            "TopHudStatusNpcIcon",
            GStatusNpcIconTexture);
        StatusNpcIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mStatusNpcIcon = StatusNpcIcon;
    }

    auto StatusSupportIcon =
        Widget.CreateWidget<CImage>("TopHud_StatusSupportIcon", 20).lock();

    if (StatusSupportIcon)
    {
        StatusSupportIcon->SetTexture(
            "TopHudStatusSupportIcon",
            GStatusSupportIconTexture);
        StatusSupportIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mStatusSupportIcon = StatusSupportIcon;
    }

    auto StatusResearchIcon =
        Widget.CreateWidget<CImage>("TopHud_StatusResearchIcon", 20).lock();

    if (StatusResearchIcon)
    {
        StatusResearchIcon->SetTexture(
            "TopHudStatusResearchIcon",
            GStatusResearchIconTexture);
        StatusResearchIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mStatusResearchIcon = StatusResearchIcon;
    }

    auto NpcText =
        Widget.CreateWidget<CTextBlock>("TopHud_StatusNpcText", 20).lock();

    if (NpcText)
    {
        NpcText->SetText(TEXT("0"));
        NpcText->SetFontSize(20.f);
        NpcText->SetAlignH(ETextAlignH::Left);
        NpcText->SetAlignV(ETextAlignV::Middle);
        NpcText->SetTextColor(245, 235, 210, 255);
        NpcText->EnableShadow(true);
        NpcText->SetShadowOffset(1.f, 1.f);
        NpcText->SetShadowTextColor(16, 16, 16, 220);
        Widget.mNpcText = NpcText;
    }

    auto NpcLabelText =
        Widget.CreateWidget<CTextBlock>("TopHud_StatusNpcLabelText", 20).lock();

    if (NpcLabelText)
    {
        NpcLabelText->SetText(UIStrings::Get(L"top_hud.label.population").c_str());
        NpcLabelText->SetFontSize(12.f);
        NpcLabelText->SetAlignH(ETextAlignH::Left);
        NpcLabelText->SetAlignV(ETextAlignV::Middle);
        NpcLabelText->SetTextColor(172, 146, 98, 255);
        NpcLabelText->EnableShadow(true);
        NpcLabelText->SetShadowOffset(1.f, 1.f);
        NpcLabelText->SetShadowTextColor(22, 18, 12, 180);
        Widget.mNpcLabelText = NpcLabelText;
    }

    auto SupportText =
        Widget.CreateWidget<CTextBlock>("TopHud_StatusSupportText", 20).lock();

    if (SupportText)
    {
        SupportText->SetText(TEXT("0%"));
        SupportText->SetFontSize(20.f);
        SupportText->SetAlignH(ETextAlignH::Left);
        SupportText->SetAlignV(ETextAlignV::Middle);
        SupportText->SetTextColor(245, 235, 210, 255);
        SupportText->EnableShadow(true);
        SupportText->SetShadowOffset(1.f, 1.f);
        SupportText->SetShadowTextColor(16, 16, 16, 220);
        Widget.mSupportText = SupportText;
    }

    auto SupportLabelText =
        Widget.CreateWidget<CTextBlock>("TopHud_StatusSupportLabelText", 20).lock();

    if (SupportLabelText)
    {
        SupportLabelText->SetText(UIStrings::Get(L"top_hud.label.support").c_str());
        SupportLabelText->SetFontSize(12.f);
        SupportLabelText->SetAlignH(ETextAlignH::Left);
        SupportLabelText->SetAlignV(ETextAlignV::Middle);
        SupportLabelText->SetTextColor(172, 146, 98, 255);
        SupportLabelText->EnableShadow(true);
        SupportLabelText->SetShadowOffset(1.f, 1.f);
        SupportLabelText->SetShadowTextColor(22, 18, 12, 180);
        Widget.mSupportLabelText = SupportLabelText;
    }

    auto ResearchText =
        Widget.CreateWidget<CTextBlock>("TopHud_StatusResearchText", 20).lock();

    if (ResearchText)
    {
        ResearchText->SetText(TEXT("0"));
        ResearchText->SetFontSize(20.f);
        ResearchText->SetAlignH(ETextAlignH::Left);
        ResearchText->SetAlignV(ETextAlignV::Middle);
        ResearchText->SetTextColor(245, 235, 210, 255);
        ResearchText->EnableShadow(true);
        ResearchText->SetShadowOffset(1.f, 1.f);
        ResearchText->SetShadowTextColor(16, 16, 16, 220);
        Widget.mResearchText = ResearchText;
    }

    auto ResearchLabelText =
        Widget.CreateWidget<CTextBlock>("TopHud_StatusResearchLabelText", 20).lock();

    if (ResearchLabelText)
    {
        ResearchLabelText->SetText(UIStrings::Get(L"top_hud.label.knowledge").c_str());
        ResearchLabelText->SetFontSize(12.f);
        ResearchLabelText->SetAlignH(ETextAlignH::Left);
        ResearchLabelText->SetAlignV(ETextAlignV::Middle);
        ResearchLabelText->SetTextColor(172, 146, 98, 255);
        ResearchLabelText->EnableShadow(true);
        ResearchLabelText->SetShadowOffset(1.f, 1.f);
        ResearchLabelText->SetShadowTextColor(22, 18, 12, 180);
        Widget.mResearchLabelText = ResearchLabelText;
    }

    auto ElectionText =
        Widget.CreateWidget<CTextBlock>("TopHud_ElectionText", 20).lock();

    if (ElectionText)
    {
        ElectionText->SetText(
            UIStrings::Get(L"top_hud.placeholder.election").c_str());
        ConfigureInfoText(ElectionText, 15.f, 238, 229, 198);
        Widget.mElectionText = ElectionText;
    }

    auto TaxPolicyText =
        Widget.CreateWidget<CTextBlock>("TopHud_TaxPolicyText", 20).lock();

    if (TaxPolicyText)
    {
        TaxPolicyText->SetText(
            UIStrings::Get(L"top_hud.placeholder.tax_policy").c_str());
        ConfigureInfoText(TaxPolicyText, 13.f, 186, 173, 144);
        Widget.mTaxPolicyText = TaxPolicyText;
    }

    auto EventText =
        Widget.CreateWidget<CTextBlock>("TopHud_EventText", 20).lock();

    if (EventText)
    {
        EventText->SetText(
            UIStrings::Get(L"top_hud.placeholder.event_stable").c_str());
        ConfigureInfoText(EventText, 13.f, 208, 226, 198);
        Widget.mEventText = EventText;
    }

    auto GameOverDim =
        Widget.CreateWidget<CImage>("TopHud_GameOverDim", 90).lock();

    if (GameOverDim)
    {
        GameOverDim->SetTexture("TopHudGameOverDim", GStatusBarTexture);
        GameOverDim->SetTint(0.03f, 0.03f, 0.03f, 0.72f);
        GameOverDim->SetEnable(false);
        Widget.mGameOverDim = GameOverDim;
    }

    auto GameOverPanel =
        Widget.CreateWidget<CImage>("TopHud_GameOverPanel", 91).lock();

    if (GameOverPanel)
    {
        GameOverPanel->SetTexture("TopHudGameOverPanel", GCenterPopupTexture);
        GameOverPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        GameOverPanel->SetEnable(false);
        Widget.mGameOverPanel = GameOverPanel;
    }

    auto GameOverTitleText =
        Widget.CreateWidget<CTextBlock>("TopHud_GameOverTitleText", 92).lock();

    if (GameOverTitleText)
    {
        GameOverTitleText->SetText(
            UIStrings::Get(L"top_hud.game_over.title").c_str());
        GameOverTitleText->SetFontSize(26.f);
        GameOverTitleText->SetAlignH(ETextAlignH::Center);
        GameOverTitleText->SetAlignV(ETextAlignV::Middle);
        GameOverTitleText->SetTextColor(244, 229, 201, 255);
        GameOverTitleText->EnableShadow(true);
        GameOverTitleText->SetShadowOffset(1.f, 1.f);
        GameOverTitleText->SetShadowTextColor(20, 18, 16, 220);
        GameOverTitleText->SetEnable(false);
        Widget.mGameOverTitleText = GameOverTitleText;
    }

    auto GameOverBodyText =
        Widget.CreateWidget<CTextBlock>("TopHud_GameOverBodyText", 92).lock();

    if (GameOverBodyText)
    {
        GameOverBodyText->SetText(TEXT(""));
        GameOverBodyText->SetFontSize(18.f);
        GameOverBodyText->SetAlignH(ETextAlignH::Center);
        GameOverBodyText->SetAlignV(ETextAlignV::Middle);
        GameOverBodyText->SetTextColor(236, 225, 198, 255);
        GameOverBodyText->EnableShadow(true);
        GameOverBodyText->SetShadowOffset(1.f, 1.f);
        GameOverBodyText->SetShadowTextColor(16, 16, 16, 200);
        GameOverBodyText->SetEnable(false);
        Widget.mGameOverBodyText = GameOverBodyText;
    }

    auto EraTransitionPanel =
        Widget.CreateWidget<CImage>("TopHud_EraTransitionPanel", 95).lock();

    if (EraTransitionPanel)
    {
        EraTransitionPanel->SetTexture(
            "TopHudEraTransitionPanel",
            GCenterPopupTexture);
        EraTransitionPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        EraTransitionPanel->SetEnable(false);
        Widget.mEraTransitionPanel = EraTransitionPanel;
    }

    auto EraTransitionTitleText =
        Widget.CreateWidget<CTextBlock>(
            "TopHud_EraTransitionTitleText",
            96).lock();

    if (EraTransitionTitleText)
    {
        EraTransitionTitleText->SetText(TEXT("시대 전환"));
        EraTransitionTitleText->SetFontSize(24.f);
        EraTransitionTitleText->SetAlignH(ETextAlignH::Center);
        EraTransitionTitleText->SetAlignV(ETextAlignV::Middle);
        EraTransitionTitleText->SetTextColor(244, 229, 201, 255);
        EraTransitionTitleText->EnableShadow(true);
        EraTransitionTitleText->SetShadowOffset(1.f, 1.f);
        EraTransitionTitleText->SetShadowTextColor(20, 18, 16, 220);
        EraTransitionTitleText->SetEnable(false);
        Widget.mEraTransitionTitleText = EraTransitionTitleText;
    }

    auto EraTransitionBodyText =
        Widget.CreateWidget<CTextBlock>(
            "TopHud_EraTransitionBodyText",
            96).lock();

    if (EraTransitionBodyText)
    {
        EraTransitionBodyText->SetText(TEXT(""));
        EraTransitionBodyText->SetFontSize(15.f);
        EraTransitionBodyText->SetAlignH(ETextAlignH::Left);
        EraTransitionBodyText->SetAlignV(ETextAlignV::Top);
        EraTransitionBodyText->SetTextColor(236, 225, 198, 255);
        EraTransitionBodyText->EnableShadow(true);
        EraTransitionBodyText->SetShadowOffset(1.f, 1.f);
        EraTransitionBodyText->SetShadowTextColor(16, 16, 16, 200);
        EraTransitionBodyText->SetEnable(false);
        Widget.mEraTransitionBodyText = EraTransitionBodyText;
    }

    auto PopupRightButton =
        Widget.CreateWidget<CButton>(
            "TopHud_PopupRightButton",
            96).lock();

    if (PopupRightButton)
    {
        ApplyButtonTextureSet(
            PopupRightButton,
            "TopHudPopupRightButton",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureHighlightedButtonStyle(PopupRightButton);
        PopupRightButton->SetEnable(false);
        PopupRightButton->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click,
            &Widget,
            &CTopHudWidget::OnPopupRightButtonClick);
        Widget.mPopupRightButton = PopupRightButton;

        auto RightButtonText = PopupRightButton->CreateChildWidget<CTextBlock>(
            "TopHud_PopupRightButtonText",
            97).lock();

        if (RightButtonText)
        {
            RightButtonText->SetText(TEXT("승인"));
            RightButtonText->SetFontSize(18.f);
            RightButtonText->SetAlignH(ETextAlignH::Center);
            RightButtonText->SetAlignV(ETextAlignV::Middle);
            RightButtonText->SetTextColor(76, 56, 28, 255);
            RightButtonText->EnableShadow(true);
            RightButtonText->SetShadowOffset(1.f, 1.f);
            RightButtonText->SetShadowTextColor(245, 234, 208, 140);
            Widget.mPopupRightButtonText = RightButtonText;
        }
    }

    auto PopupLeftButton =
        Widget.CreateWidget<CButton>(
            "TopHud_PopupLeftButton",
            96).lock();

    if (PopupLeftButton)
    {
        ApplyButtonTextureSet(
            PopupLeftButton,
            "TopHudPopupLeftButton",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureDefaultButtonStyle(PopupLeftButton);
        PopupLeftButton->SetEnable(false);
        PopupLeftButton->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click,
            &Widget,
            &CTopHudWidget::OnPopupLeftButtonClick);
        Widget.mPopupLeftButton = PopupLeftButton;

        auto LeftButtonText = PopupLeftButton->CreateChildWidget<CTextBlock>(
            "TopHud_PopupLeftButtonText",
            97).lock();

        if (LeftButtonText)
        {
            LeftButtonText->SetText(UIStrings::Get(
                L"top_hud.era_transition.cancel").c_str());
            LeftButtonText->SetFontSize(18.f);
            LeftButtonText->SetAlignH(ETextAlignH::Center);
            LeftButtonText->SetAlignV(ETextAlignV::Middle);
            LeftButtonText->SetTextColor(76, 56, 28, 255);
            LeftButtonText->EnableShadow(true);
            LeftButtonText->SetShadowOffset(1.f, 1.f);
            LeftButtonText->SetShadowTextColor(245, 234, 208, 140);
            Widget.mPopupLeftButtonText = LeftButtonText;
        }
    }

    auto ConstitutionRightButton =
        Widget.CreateWidget<CButton>(
            "TopHud_ConstitutionRightButton",
            96).lock();

    if (ConstitutionRightButton)
    {
        ApplyButtonTextureSet(
            ConstitutionRightButton,
            "TopHudConstitutionRightButton",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureHighlightedButtonStyle(ConstitutionRightButton);
        ConstitutionRightButton->SetEnable(false);
        ConstitutionRightButton->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click,
            &Widget,
            &CTopHudWidget::OnPopupRightButtonClick);
        Widget.mConstitutionRightButton = ConstitutionRightButton;

        auto RightButtonText =
            ConstitutionRightButton->CreateChildWidget<CTextBlock>(
                "TopHud_ConstitutionRightButtonText",
                97).lock();

        if (RightButtonText)
        {
            RightButtonText->SetText(TEXT(""));
            RightButtonText->SetFontSize(18.f);
            RightButtonText->SetAlignH(ETextAlignH::Center);
            RightButtonText->SetAlignV(ETextAlignV::Middle);
            RightButtonText->SetTextColor(76, 56, 28, 255);
            RightButtonText->EnableShadow(true);
            RightButtonText->SetShadowOffset(1.f, 1.f);
            RightButtonText->SetShadowTextColor(245, 234, 208, 140);
            Widget.mConstitutionRightButtonText = RightButtonText;
        }
    }

    auto ConstitutionLeftButton =
        Widget.CreateWidget<CButton>(
            "TopHud_ConstitutionLeftButton",
            96).lock();

    if (ConstitutionLeftButton)
    {
        ApplyButtonTextureSet(
            ConstitutionLeftButton,
            "TopHudConstitutionLeftButton",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureDefaultButtonStyle(ConstitutionLeftButton);
        ConstitutionLeftButton->SetEnable(false);
        ConstitutionLeftButton->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click,
            &Widget,
            &CTopHudWidget::OnPopupLeftButtonClick);
        Widget.mConstitutionLeftButton = ConstitutionLeftButton;

        auto LeftButtonText =
            ConstitutionLeftButton->CreateChildWidget<CTextBlock>(
                "TopHud_ConstitutionLeftButtonText",
                97).lock();

        if (LeftButtonText)
        {
            LeftButtonText->SetText(TEXT(""));
            LeftButtonText->SetFontSize(18.f);
            LeftButtonText->SetAlignH(ETextAlignH::Center);
            LeftButtonText->SetAlignV(ETextAlignV::Middle);
            LeftButtonText->SetTextColor(76, 56, 28, 255);
            LeftButtonText->EnableShadow(true);
            LeftButtonText->SetShadowOffset(1.f, 1.f);
            LeftButtonText->SetShadowTextColor(245, 234, 208, 140);
            Widget.mConstitutionLeftButtonText = LeftButtonText;
        }
    }

    auto ConstitutionPanel =
        Widget.CreateWidget<CImage>("TopHud_ConstitutionPanel", 94).lock();

    if (ConstitutionPanel)
    {
        ConstitutionPanel->SetTexture(
            "TopHudConstitutionPanel",
            GModernPanelTexture);
        ConstitutionPanel->SetTint(1.f, 1.f, 1.f, 0.99f);
        ConstitutionPanel->SetEnable(false);
        Widget.mConstitutionPanel = ConstitutionPanel;
    }

    auto ConstitutionTitleRibbon =
        Widget.CreateWidget<CImage>(
            "TopHud_ConstitutionTitleRibbon",
            95).lock();

    if (ConstitutionTitleRibbon)
    {
        ConstitutionTitleRibbon->SetTexture(
            "TopHudConstitutionTitleRibbon",
            GMenuTitleRibbonTexture);
        ConstitutionTitleRibbon->SetTint(1.f, 1.f, 1.f, 1.f);
        ConstitutionTitleRibbon->SetEnable(false);
        Widget.mConstitutionTitleRibbon = ConstitutionTitleRibbon;
    }

    auto ConstitutionTitleText =
        Widget.CreateWidget<CTextBlock>(
            "TopHud_ConstitutionTitleText",
            96).lock();

    if (ConstitutionTitleText)
    {
        ConstitutionTitleText->SetText(
            UIStrings::Get(L"constitution.panel.title").c_str());
        ConfigureConstitutionText(
            ConstitutionTitleText,
            28.f,
            ETextAlignH::Center,
            ETextAlignV::Middle,
            78, 56, 28);
        ConstitutionTitleText->SetEnable(false);
        Widget.mConstitutionTitleText = ConstitutionTitleText;
    }

    auto ConstitutionSubtitleText =
        Widget.CreateWidget<CTextBlock>(
            "TopHud_ConstitutionSubtitleText",
            96).lock();

    if (ConstitutionSubtitleText)
    {
        ConstitutionSubtitleText->SetText(
            UIStrings::Get(L"constitution.panel.subtitle").c_str());
        ConfigureConstitutionText(
            ConstitutionSubtitleText,
            18.f,
            ETextAlignH::Left,
            ETextAlignV::Top,
            98, 82, 52);
        ConstitutionSubtitleText->SetEnable(false);
        Widget.mConstitutionSubtitleText = ConstitutionSubtitleText;
    }

    auto ConstitutionPendingText =
        Widget.CreateWidget<CTextBlock>(
            "TopHud_ConstitutionPendingText",
            96).lock();

    if (ConstitutionPendingText)
    {
        ConstitutionPendingText->SetText(TEXT(""));
        ConfigureConstitutionText(
            ConstitutionPendingText,
            17.f,
            ETextAlignH::Right,
            ETextAlignV::Top,
            144, 72, 24);
        ConstitutionPendingText->SetEnable(false);
        Widget.mConstitutionPendingText = ConstitutionPendingText;
    }

    auto ConstitutionCloseButton =
        Widget.CreateWidget<CButton>(
            "TopHud_ConstitutionCloseButton",
            97).lock();

    if (ConstitutionCloseButton)
    {
        ApplyButtonTextureSet(
            ConstitutionCloseButton,
            "TopHudConstitutionCloseButton",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        ConfigureIconSlotButtonStyle(ConstitutionCloseButton);
        ConstitutionCloseButton->SetEnable(false);
        ConstitutionCloseButton->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click,
            &Widget,
            &CTopHudWidget::OnConstitutionCloseButtonClick);

        auto CloseText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "TopHud_ConstitutionCloseText",
                Widget.mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            ConfigureConstitutionText(
                CloseText,
                22.f,
                ETextAlignH::Center,
                ETextAlignV::Middle,
                95, 68, 18);
            ConstitutionCloseButton->SetChild(CloseText);
        }

        Widget.mConstitutionCloseButton = ConstitutionCloseButton;
    }

    auto ConstitutionOverviewButton =
        Widget.CreateWidget<CButton>(
            "TopHud_ConstitutionTab_" +
                std::to_string(GConstitutionOverviewTabNumber),
            96).lock();

    if (ConstitutionOverviewButton)
    {
        ApplyButtonTextureSet(
            ConstitutionOverviewButton,
            "TopHudConstitutionOverviewButton",
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(ConstitutionOverviewButton, false);
        ConstitutionOverviewButton->SetEnable(false);
        ConstitutionOverviewButton->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click,
            &Widget,
            &CTopHudWidget::OnConstitutionOverviewButtonClick);
        Widget.mConstitutionOverviewButton = ConstitutionOverviewButton;

        auto OverviewText =
            ConstitutionOverviewButton->CreateChildWidget<CTextBlock>(
                "TopHud_ConstitutionTabText_" +
                    std::to_string(GConstitutionOverviewTabNumber),
                97).lock();

        if (OverviewText)
        {
            OverviewText->SetText(
                UIStrings::Get(
                    L"constitution.panel.overview_button").c_str());
            ConfigureConstitutionText(
                OverviewText,
                14.f,
                ETextAlignH::Center,
                ETextAlignV::Middle,
                64, 54, 36);
            Widget.mConstitutionOverviewButtonText = OverviewText;
        }
    }

    Widget.mConstitutionTopicTabButtons.resize(GConstitutionTopicTabCount);
    Widget.mConstitutionTopicTabTexts.resize(GConstitutionTopicTabCount);

    for (int Index = 0; Index < GConstitutionTopicTabCount; ++Index)
    {
        auto TabButton = Widget.CreateWidget<CButton>(
            "TopHud_ConstitutionTab_" +
                std::to_string(Index + GConstitutionTopicTabStartNumber),
            96).lock();

        if (TabButton)
        {
            ApplyButtonTextureSet(
                TabButton,
                "TopHudConstitutionTab_" + std::to_string(Index),
                GCategoryTabTextureSelected,
                GCategoryTabTextureSelected,
                GCategoryTabTextureSelected,
                GCategoryTabTextureHidden);
            ConfigureCategoryTabButtonStyle(TabButton, false);
            TabButton->SetEnable(false);

            const EConstitutionTopic Topic =
                GConstitutionTabTopics[static_cast<size_t>(Index)];

            TabButton->SetEventCallback(
                EButtonEventState::Click,
                [&Widget, Topic]()
                {
                    Widget.ShowConstitutionTopic(Topic);
                    Widget.RefreshFromState();
                });

            Widget.mConstitutionTopicTabButtons[Index] = TabButton;
        }

        if (TabButton)
        {
            auto TabText = TabButton->CreateChildWidget<CTextBlock>(
                "TopHud_ConstitutionTabText_" +
                    std::to_string(Index + GConstitutionTopicTabStartNumber),
                97).lock();

            if (TabText)
            {
                ConfigureConstitutionText(
                    TabText,
                    14.f,
                    ETextAlignH::Center,
                    ETextAlignV::Middle,
                    64, 54, 36);
                TabText->SetEnable(false);
                Widget.mConstitutionTopicTabTexts[Index] = TabText;
            }
        }
    }

    Widget.mConstitutionSummaryButtons.resize(GConstitutionTopicTabCount);
    Widget.mConstitutionSummaryTopicTexts.resize(GConstitutionTopicTabCount);
    Widget.mConstitutionSummaryValueTexts.resize(GConstitutionTopicTabCount);
    Widget.mConstitutionSummaryStatusTexts.resize(GConstitutionTopicTabCount);

    for (int Index = 0; Index < GConstitutionTopicTabCount; ++Index)
    {
        auto CardButton = Widget.CreateWidget<CButton>(
            "TopHud_ConstitutionSummaryButton_" + std::to_string(Index + 1),
            96).lock();

        if (CardButton)
        {
            ApplyButtonTextureSet(
                CardButton,
                "TopHudConstitutionSummaryButton_" + std::to_string(Index),
                GDetailInfoPanelTexture,
                GDetailInfoPanelTexture,
                GDetailInfoPanelTexture,
                GDetailInfoPanelTexture);
            ConfigureDefaultButtonStyle(CardButton);
            CardButton->SetEnable(false);

            const EConstitutionTopic Topic =
                GConstitutionTabTopics[static_cast<size_t>(Index)];

            CardButton->SetEventCallback(
                EButtonEventState::Click,
                [&Widget, Topic]()
                {
                    Widget.ShowConstitutionTopic(Topic);
                    Widget.RefreshFromState();
                });

            Widget.mConstitutionSummaryButtons[Index] = CardButton;
        }

        auto TopicText = Widget.CreateWidget<CTextBlock>(
            "TopHud_ConstitutionSummaryTopic_" + std::to_string(Index + 1),
            97).lock();

        if (TopicText)
        {
            ConfigureConstitutionText(
                TopicText,
                18.f,
                ETextAlignH::Left,
                ETextAlignV::Top,
                72, 56, 34);
            TopicText->SetEnable(false);
            Widget.mConstitutionSummaryTopicTexts[Index] = TopicText;
        }

        auto ValueText = Widget.CreateWidget<CTextBlock>(
            "TopHud_ConstitutionSummaryValue_" + std::to_string(Index + 1),
            97).lock();

        if (ValueText)
        {
            ConfigureConstitutionText(
                ValueText,
                22.f,
                ETextAlignH::Left,
                ETextAlignV::Top,
                48, 48, 48);
            ValueText->SetEnable(false);
            Widget.mConstitutionSummaryValueTexts[Index] = ValueText;
        }

        auto StatusText = Widget.CreateWidget<CTextBlock>(
            "TopHud_ConstitutionSummaryStatus_" + std::to_string(Index + 1),
            97).lock();

        if (StatusText)
        {
            ConfigureConstitutionText(
                StatusText,
                16.f,
                ETextAlignH::Left,
                ETextAlignV::Top,
                132, 96, 44);
            StatusText->SetEnable(false);
            Widget.mConstitutionSummaryStatusTexts[Index] = StatusText;
        }
    }

    Widget.mConstitutionOptionButtons.resize(GConstitutionOptionCardCount);
    Widget.mConstitutionOptionTitleTexts.resize(GConstitutionOptionCardCount);
    Widget.mConstitutionOptionSummaryTexts.resize(GConstitutionOptionCardCount);
    Widget.mConstitutionOptionBodyTexts.resize(GConstitutionOptionCardCount);

    for (int Index = 0; Index < GConstitutionOptionCardCount; ++Index)
    {
        auto OptionButton = Widget.CreateWidget<CButton>(
            "TopHud_ConstitutionOptionButton_" + std::to_string(Index + 1),
            96).lock();

        if (OptionButton)
        {
            ApplyButtonTextureSet(
                OptionButton,
                "TopHudConstitutionOptionButton_" + std::to_string(Index),
                GDetailInfoPanelTexture,
                GDetailInfoPanelTexture,
                GDetailInfoPanelTexture,
                GDetailInfoPanelTexture);
            ConfigureDefaultButtonStyle(OptionButton);
            OptionButton->SetEnable(false);
            OptionButton->SetEventCallback(
                EButtonEventState::Click,
                [&Widget, Index]()
                {
                    Widget.TrySelectViewedConstitutionOption(
                        static_cast<size_t>(Index));
                });
            Widget.mConstitutionOptionButtons[Index] = OptionButton;
        }

        auto TitleText = Widget.CreateWidget<CTextBlock>(
            "TopHud_ConstitutionOptionTitle_" + std::to_string(Index + 1),
            97).lock();

        if (TitleText)
        {
            ConfigureConstitutionText(
                TitleText,
                22.f,
                ETextAlignH::Left,
                ETextAlignV::Top,
                74, 54, 24);
            TitleText->SetEnable(false);
            Widget.mConstitutionOptionTitleTexts[Index] = TitleText;
        }

        auto SummaryText = Widget.CreateWidget<CTextBlock>(
            "TopHud_ConstitutionOptionSummary_" + std::to_string(Index + 1),
            97).lock();

        if (SummaryText)
        {
            ConfigureConstitutionText(
                SummaryText,
                16.f,
                ETextAlignH::Left,
                ETextAlignV::Top,
                116, 84, 42);
            SummaryText->SetEnable(false);
            Widget.mConstitutionOptionSummaryTexts[Index] = SummaryText;
        }

        auto BodyText = Widget.CreateWidget<CTextBlock>(
            "TopHud_ConstitutionOptionBody_" + std::to_string(Index + 1),
            97).lock();

        if (BodyText)
        {
            ConfigureConstitutionText(
                BodyText,
                17.f,
                ETextAlignH::Left,
                ETextAlignV::Top,
                62, 60, 54);
            BodyText->SetEnable(false);
            Widget.mConstitutionOptionBodyTexts[Index] = BodyText;
        }
    }

    Widget.mSpeedButtons.resize(GSpeedButtonCount);
    Widget.mSpeedButtonIcons.resize(GSpeedButtonCount);

    for (int i = 0; i < GSpeedButtonCount; ++i)
    {
        auto Button = Widget.CreateWidget<CButton>(
            "TopHud_SpeedButton_" + std::to_string(i + 1),
            18).lock();

        if (!Button)
            continue;

        ConfigureSpeedButtonStyle(Button);
        ApplyButtonTextureSet(
            Button,
            "TopHudSpeedButtonFrame_" + std::to_string(i),
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        if (i == GSpeedStateButtonIndex)
        {
            Button->SetEventCallback<CTopHudWidget>(
                EButtonEventState::Click,
                &Widget,
                &CTopHudWidget::OnSpeedStateButtonClick);
        }
        else
        {
            Button->SetEventCallback<CTopHudWidget>(
                EButtonEventState::Click,
                &Widget,
                &CTopHudWidget::OnSpeedMultiplierButtonClick);
        }

        auto Icon = CWidget::CreateStaticWidget<CImage>(
            "TopHud_SpeedButtonIcon_" + std::to_string(i + 1),
            Widget.mWorld);

        if (Icon)
        {
            Icon->SetTexture(
                "TopHudSpeedIcon_" + std::to_string(i),
                i == GSpeedStateButtonIndex ?
                GSpeedStateIcons[1] :
                GSpeedMultiplierIcons[0]);
            Icon->SetTint(
                i == GSpeedStateButtonIndex ?
                FVector4(1.15f, 1.00f, 0.52f, 1.f) :
                FVector4(1.12f, 1.00f, 0.60f, 1.f));
            Button->SetChild(Icon);
            Widget.mSpeedButtonIcons[i] = Icon;
        }

        Widget.mSpeedButtons[i] = Button;
    }

    Widget.mMenuButtons.resize(GMenuButtonCount);
    Widget.mMenuButtonIcons.resize(GMenuButtonCount);
    Widget.mMenuButtonTexts.resize(GMenuButtonCount);

    for (int i = 0; i < GMenuButtonCount; ++i)
    {
        auto Button = Widget.CreateWidget<CButton>(
            "TopHud_MenuButton_" + std::to_string(i + 1),
            18).lock();

        if (!Button)
            continue;

        ConfigureIconButtonStyle(Button);
        ApplyButtonTextureSet(
            Button,
            "TopHudMenuButtonFrame_" + std::to_string(i),
            GMenuButtonBackgroundTexture,
            GMenuButtonBackgroundTexture,
            GMenuButtonBackgroundTexture,
            GMenuButtonBackgroundTexture);

        void (CTopHudWidget::*MenuCallback)() =
            &CTopHudWidget::OnAnyButtonClick;

        if (i == GMenuTaskIndex)
            MenuCallback = &CTopHudWidget::OnTaskButtonClick;
        else if (i == GMenuConstructionIndex)
            MenuCallback = &CTopHudWidget::OnConstructionButtonClick;
        else if (i == GMenuEdictsIndex)
            MenuCallback = &CTopHudWidget::OnEdictsButtonClick;
        else if (i == GMenuEraTransitionIndex)
            MenuCallback = &CTopHudWidget::OnEraTransitionButtonClick;
        else if (i == GMenuTradeIndex)
            MenuCallback = &CTopHudWidget::OnTradeButtonClick;
        else if (i == GMenuAlmanacIndex)
            MenuCallback = &CTopHudWidget::OnAlmanacButtonClick;

        Button->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click,
            &Widget,
            MenuCallback);

        Widget.mMenuButtons[i] = Button;

        auto MenuIcon = CWidget::CreateStaticWidget<CImage>(
            "TopHud_MenuIcon_" + std::to_string(i + 1),
            Widget.mWorld);

        if (MenuIcon)
        {
            MenuIcon->SetTexture(
                "TopHudMenuIcon_" + std::to_string(i),
                GMenuIcons[i]);
            MenuIcon->SetTint(1.f, 1.f, 1.f, 1.f);
            Button->SetChild(MenuIcon);
            Widget.mMenuButtonIcons[i] = MenuIcon;
        }

        auto MenuText = Widget.CreateWidget<CTextBlock>(
            "TopHud_MenuText_" + std::to_string(i + 1),
            18).lock();

        if (!MenuText)
            continue;

        MenuText->SetText(UIStrings::Get(GMenuLabelKeys[i]).c_str());
        MenuText->SetFontSize(12.f);
        MenuText->SetAlignH(ETextAlignH::Center);
        MenuText->SetAlignV(ETextAlignV::Middle);
        MenuText->SetTextColor(240, 228, 204, 255);
        MenuText->EnableShadow(true);
        MenuText->SetShadowOffset(1.f, 1.f);
        MenuText->SetShadowTextColor(18, 16, 14, 210);
        Widget.mMenuButtonTexts[i] = MenuText;
    }
}

void FTopHudRenderer::ApplySnapshot(
    CTopHudWidget& Widget,
    const TopHudDataProvider::FTopHudSnapshot& Snapshot)
{
    auto& State = Widget.GetMutableState();

    State.MonthProgress = Snapshot.MonthProgress;
    State.GameLost = Snapshot.GameLost;

    auto DateText = Widget.mDateText.lock();
    auto BudgetText = Widget.mBudgetText.lock();
    auto NpcText = Widget.mNpcText.lock();
    auto SupportText = Widget.mSupportText.lock();
    auto ResearchText = Widget.mResearchText.lock();
    auto ElectionText = Widget.mElectionText.lock();
    auto TaxPolicyText = Widget.mTaxPolicyText.lock();
    auto EventText = Widget.mEventText.lock();
    auto GameOverDim = Widget.mGameOverDim.lock();
    auto GameOverPanel = Widget.mGameOverPanel.lock();
    auto GameOverTitleText = Widget.mGameOverTitleText.lock();
    auto GameOverBodyText = Widget.mGameOverBodyText.lock();
    auto EraTransitionPanel = Widget.mEraTransitionPanel.lock();
    auto EraTransitionTitleText = Widget.mEraTransitionTitleText.lock();
    auto EraTransitionBodyText = Widget.mEraTransitionBodyText.lock();
    auto ConstitutionPanel = Widget.mConstitutionPanel.lock();
    auto ConstitutionTitleRibbon = Widget.mConstitutionTitleRibbon.lock();
    auto ConstitutionCloseButton =
        Widget.mConstitutionCloseButton.lock();
    auto PopupRightButton = Widget.mPopupRightButton.lock();
    auto PopupLeftButton = Widget.mPopupLeftButton.lock();
    auto ConstitutionRightButton =
        Widget.mConstitutionRightButton.lock();
    auto ConstitutionLeftButton =
        Widget.mConstitutionLeftButton.lock();
    auto PopupRightButtonText =
        Widget.mPopupRightButtonText.lock();
    auto PopupLeftButtonText =
        Widget.mPopupLeftButtonText.lock();
    auto ConstitutionRightButtonText =
        Widget.mConstitutionRightButtonText.lock();
    auto ConstitutionLeftButtonText =
        Widget.mConstitutionLeftButtonText.lock();
    const bool ShowPopupOverlay =
        Widget.GetState().EraTransitionPopupOpen &&
        !Snapshot.GameLost;
    const bool ShowConstitutionPanel =
        Widget.GetState().ConstitutionPanelOpen &&
        !Snapshot.GameLost;
    const bool ShowConstitutionPopup = false;
    const bool ShowEraTransitionPopup =
        ShowPopupOverlay &&
        !ShowConstitutionPopup;
    const bool ConstitutionPopupButtonsEnabled =
        ShowConstitutionPopup &&
        Snapshot.CanUseButtons;
    const bool EraTransitionLeftButtonEnabled =
        ShowEraTransitionPopup &&
        Snapshot.CanUseButtons;
    const bool EraTransitionRightButtonEnabled =
        ShowEraTransitionPopup &&
        Snapshot.CanUseButtons &&
        Snapshot.EraTransitionAvailable;

    if (DateText)
        DateText->SetText(Snapshot.DateText.c_str());

    if (BudgetText)
        BudgetText->SetText(Snapshot.BudgetText.c_str());

    if (NpcText)
        NpcText->SetText(Snapshot.NpcText.c_str());

    if (SupportText)
        SupportText->SetText(Snapshot.SupportText.c_str());

    if (ResearchText)
        ResearchText->SetText(Snapshot.ResearchText.c_str());

    if (ElectionText)
    {
        ElectionText->SetText(Snapshot.ElectionText.c_str());
        ElectionText->SetTextColor(Snapshot.ElectionTextColor);
    }

    if (TaxPolicyText)
        TaxPolicyText->SetText(Snapshot.TaxPolicyText.c_str());

    if (EventText)
    {
        EventText->SetText(Snapshot.EventText.c_str());
        EventText->SetTextColor(Snapshot.EventTextColor);
    }

    for (size_t i = 0; i < Widget.mSpeedButtons.size(); ++i)
    {
        auto Button = Widget.mSpeedButtons[i].lock();

        if (Button)
        {
            Button->ButtonEnable(
                Snapshot.CanUseButtons &&
                !ShowEraTransitionPopup &&
                !ShowConstitutionPanel);
        }
    }

    if (Widget.mSpeedButtonIcons.size() > GSpeedStateButtonIndex)
    {
        auto StateIcon = Widget.mSpeedButtonIcons[GSpeedStateButtonIndex].lock();

        if (StateIcon)
        {
            StateIcon->SetTexture(
                "TopHudSpeedStateIcon_" +
                    std::to_string(Snapshot.GamePaused ? 1 : 0),
                GetSpeedStateIconPath(Snapshot));
            StateIcon->SetTint(GetSpeedStateTint(Snapshot));
        }
    }

    if (Widget.mSpeedButtonIcons.size() > GSpeedMultiplierButtonIndex)
    {
        auto MultiplierIcon =
            Widget.mSpeedButtonIcons[GSpeedMultiplierButtonIndex].lock();

        if (MultiplierIcon)
        {
            const int IconIndex =
                Snapshot.GameSpeedMultiplier >= 4 ? 3 :
                Snapshot.GameSpeedMultiplier >= 3 ? 2 :
                Snapshot.GameSpeedMultiplier >= 2 ? 1 : 0;

            MultiplierIcon->SetTexture(
                "TopHudSpeedMultiplierIcon_" + std::to_string(IconIndex),
                GetSpeedMultiplierIconPath(Snapshot));
            MultiplierIcon->SetTint(GetSpeedMultiplierTint(Snapshot));
        }
    }

    for (size_t i = 0; i < Widget.mMenuButtons.size(); ++i)
    {
        auto Button = Widget.mMenuButtons[i].lock();

        if (Button)
        {
            const bool ConstitutionMenuButton =
                static_cast<int>(i) == GMenuEraTransitionIndex;
            const bool Enabled =
                Snapshot.CanUseButtons &&
                !ShowEraTransitionPopup &&
                (!ShowConstitutionPanel || ConstitutionMenuButton);
            Button->ButtonEnable(Enabled);
        }
    }

    if (EraTransitionPanel)
        EraTransitionPanel->SetEnable(ShowPopupOverlay);

    if (EraTransitionTitleText)
    {
        EraTransitionTitleText->SetText(Snapshot.EraTransitionTitle.c_str());
        EraTransitionTitleText->SetEnable(ShowPopupOverlay);
    }

    if (EraTransitionBodyText)
    {
        EraTransitionBodyText->SetText(Snapshot.EraTransitionBody.c_str());
        EraTransitionBodyText->SetEnable(ShowPopupOverlay);
    }

    if (ConstitutionPanel)
        ConstitutionPanel->SetEnable(ShowConstitutionPanel);

    if (ConstitutionTitleRibbon)
        ConstitutionTitleRibbon->SetEnable(ShowConstitutionPanel);

    if (ConstitutionCloseButton)
    {
        ConstitutionCloseButton->SetEnable(ShowConstitutionPanel);
        ConstitutionCloseButton->ButtonEnable(
            ShowConstitutionPanel && Snapshot.CanUseButtons);
    }

    if (PopupRightButton)
    {
        PopupRightButton->SetEnable(ShowEraTransitionPopup);
        PopupRightButton->ButtonEnable(EraTransitionRightButtonEnabled);
    }

    if (PopupLeftButton)
    {
        PopupLeftButton->SetEnable(ShowEraTransitionPopup);
        PopupLeftButton->ButtonEnable(EraTransitionLeftButtonEnabled);
    }

    if (ConstitutionRightButton)
    {
        ConstitutionRightButton->SetEnable(ShowConstitutionPopup);
        ConstitutionRightButton->ButtonEnable(
            ConstitutionPopupButtonsEnabled);
    }

    if (ConstitutionLeftButton)
    {
        ConstitutionLeftButton->SetEnable(ShowConstitutionPopup);
        ConstitutionLeftButton->ButtonEnable(
            ConstitutionPopupButtonsEnabled);
    }

    if (PopupRightButtonText)
    {
        PopupRightButtonText->SetText(
            Snapshot.EraTransitionConfirmText.c_str());
        PopupRightButtonText->SetEnable(ShowEraTransitionPopup);
    }

    if (PopupLeftButtonText)
    {
        PopupLeftButtonText->SetText(
            Snapshot.EraTransitionCancelText.c_str());
        PopupLeftButtonText->SetEnable(ShowEraTransitionPopup);
    }

    if (ConstitutionRightButtonText)
        ConstitutionRightButtonText->SetEnable(ShowConstitutionPopup);

    if (ConstitutionLeftButtonText)
        ConstitutionLeftButtonText->SetEnable(ShowConstitutionPopup);

    if (GameOverDim)
        GameOverDim->SetEnable(Snapshot.GameLost);

    if (GameOverPanel)
        GameOverPanel->SetEnable(Snapshot.GameLost);

    if (GameOverTitleText)
    {
        GameOverTitleText->SetText(Snapshot.GameOverTitleText.c_str());
        GameOverTitleText->SetEnable(Snapshot.GameLost);
    }

    if (GameOverBodyText)
    {
        GameOverBodyText->SetText(Snapshot.GameOverBodyText.c_str());
        GameOverBodyText->SetEnable(Snapshot.GameLost);
    }
}

void FTopHudRenderer::RefreshLayout(CTopHudWidget& Widget)
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);

    auto SpeedPanel = Widget.mSpeedPanel.lock();
    auto TimeBarBack = Widget.mTimeBarBack.lock();
    auto TimeBarFill = Widget.mTimeBarFill.lock();
    auto DateText = Widget.mDateText.lock();
    auto BudgetText = Widget.mBudgetText.lock();
    auto BudgetLabelText = Widget.mBudgetLabelText.lock();
    auto StatusBar = Widget.mStatusBar.lock();
    auto StatusMoneyIcon = Widget.mStatusMoneyIcon.lock();
    auto StatusNpcIcon = Widget.mStatusNpcIcon.lock();
    auto StatusSupportIcon = Widget.mStatusSupportIcon.lock();
    auto StatusResearchIcon = Widget.mStatusResearchIcon.lock();
    auto NpcLabelText = Widget.mNpcLabelText.lock();
    auto NpcText = Widget.mNpcText.lock();
    auto SupportLabelText = Widget.mSupportLabelText.lock();
    auto SupportText = Widget.mSupportText.lock();
    auto ResearchLabelText = Widget.mResearchLabelText.lock();
    auto ResearchText = Widget.mResearchText.lock();
    auto ElectionText = Widget.mElectionText.lock();
    auto TaxPolicyText = Widget.mTaxPolicyText.lock();
    auto EventText = Widget.mEventText.lock();
    auto GameOverDim = Widget.mGameOverDim.lock();
    auto GameOverPanel = Widget.mGameOverPanel.lock();
    auto GameOverTitleText = Widget.mGameOverTitleText.lock();
    auto GameOverBodyText = Widget.mGameOverBodyText.lock();
    auto EraTransitionPanel = Widget.mEraTransitionPanel.lock();
    auto EraTransitionTitleText = Widget.mEraTransitionTitleText.lock();
    auto EraTransitionBodyText = Widget.mEraTransitionBodyText.lock();
    auto ConstitutionPanel = Widget.mConstitutionPanel.lock();
    auto ConstitutionTitleRibbon = Widget.mConstitutionTitleRibbon.lock();
    auto ConstitutionTitleText = Widget.mConstitutionTitleText.lock();
    auto ConstitutionSubtitleText = Widget.mConstitutionSubtitleText.lock();
    auto ConstitutionPendingText = Widget.mConstitutionPendingText.lock();
    auto ConstitutionCloseButton =
        Widget.mConstitutionCloseButton.lock();
    auto ConstitutionOverviewButton =
        Widget.mConstitutionOverviewButton.lock();
    auto ConstitutionOverviewButtonText =
        Widget.mConstitutionOverviewButtonText.lock();
    auto PopupRightButton = Widget.mPopupRightButton.lock();
    auto PopupLeftButton = Widget.mPopupLeftButton.lock();
    auto ConstitutionRightButton =
        Widget.mConstitutionRightButton.lock();
    auto ConstitutionLeftButton =
        Widget.mConstitutionLeftButton.lock();
    auto PopupRightButtonText =
        Widget.mPopupRightButtonText.lock();
    auto PopupLeftButtonText =
        Widget.mPopupLeftButtonText.lock();
    auto ConstitutionRightButtonText =
        Widget.mConstitutionRightButtonText.lock();
    auto ConstitutionLeftButtonText =
        Widget.mConstitutionLeftButtonText.lock();
    const float TopHudScale =
        (std::max)(0.72f, (std::min)(1.05f, ScreenWidth / 1920.f));
    const float TopHudPanelX = UIConfig::SpeedPanelX;
    const float TopHudPanelW = UIConfig::SpeedPanelWidth * TopHudScale;
    const float TopHudPanelH = UIConfig::SpeedPanelHeight * TopHudScale;
    const float TopHudPanelY = (std::max)(
        UIConfig::SpeedPanelMinY,
        ScreenHeight - TopHudPanelH - UIConfig::SpeedPanelBottomMargin);

    const float StatusScale =
        (std::max)(0.72f, (std::min)(1.05f, ScreenWidth / 1920.f));
    const float StatusX = UIConfig::StatusBarX;
    const float StatusY = UIConfig::StatusBarY;
    const float StatusPaddingX = UIConfig::StatusBarPaddingX * StatusScale;
    const float StatusBlockGap = UIConfig::StatusBlockGap * StatusScale;
    const float StatusMoneyBlockW = UIConfig::StatusBudgetBlockWidth * StatusScale;
    const float StatusNpcBlockW = UIConfig::StatusNpcBlockWidth * StatusScale;
    const float StatusSupportBlockW = UIConfig::StatusSupportBlockWidth * StatusScale;
    const float StatusResearchBlockW =
        UIConfig::StatusSupportBlockWidth * StatusScale;
    const float StatusW =
        StatusPaddingX * 2.f +
        StatusMoneyBlockW +
        StatusNpcBlockW +
        StatusSupportBlockW +
        StatusResearchBlockW +
        StatusBlockGap * 3.f;
    const float StatusH = UIConfig::StatusBarHeight * StatusScale;
    const float StatusIconSize = UIConfig::StatusIconSize * StatusScale;
    const float StatusLabelY = StatusY + UIConfig::StatusLabelOffsetY * StatusScale;
    const float StatusValueY = StatusY + UIConfig::StatusValueOffsetY * StatusScale;
    const float StatusBlockMoneyX = StatusX + StatusPaddingX;
    const float StatusBlockNpcX =
        StatusBlockMoneyX + StatusMoneyBlockW + StatusBlockGap;
    const float StatusBlockSupportX =
        StatusBlockNpcX + StatusNpcBlockW + StatusBlockGap;
    const float StatusBlockResearchX =
        StatusBlockSupportX + StatusSupportBlockW + StatusBlockGap;

    if (StatusBar)
    {
        StatusBar->SetPos(StatusX, StatusY);
        StatusBar->SetSize(StatusW, StatusH);
    }

    auto LayoutStatusBlock =
        [StatusIconSize, StatusLabelY, StatusValueY, StatusScale](
            const std::shared_ptr<CImage>& Icon,
            const std::shared_ptr<CTextBlock>& LabelText,
            const std::shared_ptr<CTextBlock>& ValueText,
            float BlockX,
            float BlockW,
            float OffsetX,
            float OffsetY)
        {
            const float X = BlockX + OffsetX;
            const float LabelY = StatusLabelY + OffsetY;
            const float ValueY = StatusValueY + OffsetY;

            if (Icon)
            {
                Icon->SetPos(X, ValueY + 1.f * StatusScale);
                Icon->SetSize(StatusIconSize, StatusIconSize);
            }

            const float TextX = X + StatusIconSize +
                UIConfig::StatusIconTextGap * StatusScale;
            const float TextW = BlockW - StatusIconSize -
                UIConfig::StatusIconTextGap * StatusScale;

            if (LabelText)
            {
                LabelText->SetFontSize(UIConfig::StatusLabelFontSize * StatusScale);
                LabelText->SetPos(TextX, LabelY);
                LabelText->SetSize(TextW, UIConfig::StatusLabelHeight * StatusScale);
            }

            if (ValueText)
            {
                ValueText->SetFontSize(UIConfig::StatusValueFontSize * StatusScale);
                ValueText->SetPos(TextX, ValueY);
                ValueText->SetSize(TextW, UIConfig::StatusValueHeight * StatusScale);
            }
        };

    LayoutStatusBlock(
        StatusMoneyIcon, BudgetLabelText, BudgetText,
        StatusBlockMoneyX, StatusMoneyBlockW,
        UIConfig::BudgetBlockOffsetX * StatusScale,
        UIConfig::BudgetBlockOffsetY * StatusScale);
    LayoutStatusBlock(
        StatusNpcIcon, NpcLabelText, NpcText,
        StatusBlockNpcX, StatusNpcBlockW,
        UIConfig::NpcBlockOffsetX * StatusScale,
        UIConfig::NpcBlockOffsetY * StatusScale);
    LayoutStatusBlock(
        StatusSupportIcon, SupportLabelText, SupportText,
        StatusBlockSupportX, StatusSupportBlockW,
        UIConfig::SupportBlockOffsetX * StatusScale,
        UIConfig::SupportBlockOffsetY * StatusScale);
    LayoutStatusBlock(
        StatusResearchIcon, ResearchLabelText, ResearchText,
        StatusBlockResearchX, StatusResearchBlockW,
        UIConfig::SupportBlockOffsetX * StatusScale,
        UIConfig::SupportBlockOffsetY * StatusScale);

    const float InfoTextX = StatusX + 16.f * StatusScale;
    const float InfoTextTop = StatusY + StatusH + 8.f * StatusScale;
    const float InfoTextWidth = 540.f * StatusScale;
    const float InfoLineHeight = 18.f * StatusScale;

    if (ElectionText)
    {
        ElectionText->SetFontSize(15.f * StatusScale);
        ElectionText->SetPos(InfoTextX, InfoTextTop);
        ElectionText->SetSize(InfoTextWidth, InfoLineHeight);
    }

    if (TaxPolicyText)
    {
        TaxPolicyText->SetFontSize(13.f * StatusScale);
        TaxPolicyText->SetPos(InfoTextX, InfoTextTop + 18.f * StatusScale);
        TaxPolicyText->SetSize(InfoTextWidth, InfoLineHeight);
    }

    if (EventText)
    {
        EventText->SetFontSize(13.f * StatusScale);
        EventText->SetPos(InfoTextX, InfoTextTop + 36.f * StatusScale);
        EventText->SetSize(InfoTextWidth, InfoLineHeight);
    }

    if (SpeedPanel)
    {
        SpeedPanel->SetPos(TopHudPanelX, TopHudPanelY);
        SpeedPanel->SetSize(TopHudPanelW, TopHudPanelH);
    }

    const float PanelTextX = TopHudPanelX + UIConfig::PanelTextOffsetX * TopHudScale;
    const float TimeBarX = PanelTextX;
    const float TimeBarY = TopHudPanelY + UIConfig::TimeBarOffsetY * TopHudScale;
    const float TimeBarW = UIConfig::TimeBarWidth * TopHudScale;
    const float TimeBarH = UIConfig::TimeBarHeight * TopHudScale;

    if (TimeBarBack)
    {
        TimeBarBack->SetPos(TimeBarX, TimeBarY);
        TimeBarBack->SetSize(TimeBarW, TimeBarH);
    }

    if (TimeBarFill)
    {
        TimeBarFill->SetPos(TimeBarX, TimeBarY);
        TimeBarFill->SetSize(
            TimeBarW * Widget.GetState().MonthProgress,
            TimeBarH);
    }

    if (DateText)
    {
        DateText->SetFontSize(UIConfig::DateFontSize * TopHudScale);
        DateText->SetPos(
            PanelTextX,
            TopHudPanelY + UIConfig::DateTextOffsetY * TopHudScale);
        DateText->SetSize(
            UIConfig::DateTextWidth * TopHudScale,
            UIConfig::DateTextHeight * TopHudScale);
    }

    const float PlayPauseButtonSize =
        UIConfig::PlayPauseButtonSize * TopHudScale;
    const float PlayPauseButtonX =
        TopHudPanelX + UIConfig::PlayPauseButtonOffsetX * TopHudScale;
    const float PlayPauseButtonY =
        TopHudPanelY + TopHudPanelH - PlayPauseButtonSize -
        UIConfig::PlayPauseButtonBottomMargin * TopHudScale +
        UIConfig::PlayPauseButtonOffsetY * TopHudScale;
    const float SpeedMultiplierButtonSize =
        UIConfig::SpeedMultiplierButtonSize * TopHudScale;
    const float SpeedMultiplierButtonX =
        TopHudPanelX + UIConfig::SpeedMultiplierButtonOffsetX * TopHudScale;
    const float SpeedMultiplierButtonY =
        TopHudPanelY + TopHudPanelH - SpeedMultiplierButtonSize -
        UIConfig::SpeedMultiplierButtonBottomMargin * TopHudScale +
        UIConfig::SpeedMultiplierButtonOffsetY * TopHudScale;

    for (int i = 0; i < static_cast<int>(Widget.mSpeedButtons.size()); ++i)
    {
        auto SpeedButton = Widget.mSpeedButtons[i].lock();
        auto SpeedIcon =
            i < static_cast<int>(Widget.mSpeedButtonIcons.size()) ?
            Widget.mSpeedButtonIcons[i].lock() :
            nullptr;

        if (!SpeedButton)
            continue;

        const bool IsPlayPauseButton = i == GSpeedStateButtonIndex;
        const float ButtonSize =
            IsPlayPauseButton ? PlayPauseButtonSize : SpeedMultiplierButtonSize;
        const float ButtonX =
            IsPlayPauseButton ? PlayPauseButtonX : SpeedMultiplierButtonX;
        const float ButtonY =
            IsPlayPauseButton ? PlayPauseButtonY : SpeedMultiplierButtonY;

        SpeedButton->SetPos(
            ButtonX,
            ButtonY);
        SpeedButton->SetSize(ButtonSize, ButtonSize);

        if (SpeedIcon)
        {
            SpeedIcon->SetPos(
                ButtonSize * 0.18f,
                ButtonSize * 0.18f);
            SpeedIcon->SetSize(
                ButtonSize * 0.64f,
                ButtonSize * 0.64f);
        }
    }

    float MenuButtonSize = UIConfig::MenuButtonSize * TopHudScale * 0.70f;
    float MenuButtonGap = UIConfig::MenuButtonGap * TopHudScale * 0.34f;

    const float WantedMenuWidth =
        MenuButtonSize * GMenuButtonCount +
        MenuButtonGap * static_cast<float>(GMenuButtonCount - 1);
    const float MaxMenuWidth =
        (std::max)(520.f * TopHudScale,
            ScreenWidth - 220.f * TopHudScale);

    if (WantedMenuWidth > MaxMenuWidth)
    {
        const float MenuScale =
            (std::max)(UIConfig::MenuMinScaleFactor,
                MaxMenuWidth / WantedMenuWidth);
        MenuButtonSize *= MenuScale;
        MenuButtonGap *= MenuScale;
    }

    const float MenuStripWidth =
        MenuButtonSize * GMenuButtonCount +
        MenuButtonGap * static_cast<float>(GMenuButtonCount - 1);
    const float MenuStartX = (ScreenWidth - MenuStripWidth) * 0.5f;
    const float MenuBottomMargin = 18.f * TopHudScale;
    const float MenuY = ScreenHeight - MenuButtonSize - MenuBottomMargin;

    for (int i = 0; i < static_cast<int>(Widget.mMenuButtons.size()); ++i)
    {
        auto MenuButton = Widget.mMenuButtons[i].lock();
        auto MenuIcon =
            i < static_cast<int>(Widget.mMenuButtonIcons.size()) ?
            Widget.mMenuButtonIcons[i].lock() :
            nullptr;
        auto MenuText = Widget.mMenuButtonTexts[i].lock();
        const float MenuButtonX =
            MenuStartX +
            (MenuButtonSize + MenuButtonGap) * static_cast<float>(i);

        if (MenuButton)
        {
            MenuButton->SetPos(MenuButtonX, MenuY);
            MenuButton->SetSize(MenuButtonSize, MenuButtonSize);
        }

        if (MenuIcon)
        {
            MenuIcon->SetPos(
                MenuButtonSize * 0.18f,
                MenuButtonSize * 0.16f);
            MenuIcon->SetSize(
                MenuButtonSize * 0.64f,
                MenuButtonSize * 0.64f);
        }

        if (MenuText)
        {
            MenuText->SetEnable(false);
        }
    }

    const float OverlayWidth = ScreenWidth;
    const float OverlayHeight = ScreenHeight;
    const float PanelScale =
        (std::max)(0.72f, (std::min)(1.05f, ScreenWidth / 1920.f));
    const float PanelWidth = UIConfig::GameOverPanelWidth * PanelScale;
    const float PanelHeight = UIConfig::GameOverPanelHeight * PanelScale;
    const float PanelX = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelY = (ScreenHeight - PanelHeight) * 0.5f;

    if (GameOverDim)
    {
        GameOverDim->SetPos(0.f, 0.f);
        GameOverDim->SetSize(OverlayWidth, OverlayHeight);
    }

    if (GameOverPanel)
    {
        GameOverPanel->SetPos(PanelX, PanelY);
        GameOverPanel->SetSize(PanelWidth, PanelHeight);
    }

    if (GameOverTitleText)
    {
        GameOverTitleText->SetPos(
            PanelX + UIConfig::GameOverTitlePaddingX * PanelScale,
            PanelY + UIConfig::GameOverTitleOffsetY * PanelScale);
        GameOverTitleText->SetSize(
            PanelWidth - UIConfig::GameOverTitlePaddingX * 2.f * PanelScale,
            UIConfig::GameOverTitleHeight * PanelScale);
    }

    if (GameOverBodyText)
    {
        GameOverBodyText->SetPos(
            PanelX + UIConfig::GameOverBodyPaddingX * PanelScale,
            PanelY + UIConfig::GameOverBodyOffsetY * PanelScale);
        GameOverBodyText->SetSize(
            PanelWidth - UIConfig::GameOverBodyPaddingX * 2.f * PanelScale,
            PanelHeight - UIConfig::GameOverBodyBottomPadding * PanelScale);
    }

    const float EraPanelWidth =
        (std::min)(ScreenWidth * 0.72f, (std::max)(560.f, PanelWidth * 1.12f));
    const float EraPanelHeight =
        (std::min)(ScreenHeight * 0.62f, (std::max)(340.f, PanelHeight * 1.14f));
    const float EraPanelX = (ScreenWidth - EraPanelWidth) * 0.5f;
    const float EraPanelY = (ScreenHeight - EraPanelHeight) * 0.5f;
    const float EraHorizontalPadding = 38.f * PanelScale;
    const float EraButtonWidth = 200.f * PanelScale;
    const float EraButtonHeight = 54.f * PanelScale;
    const float EraButtonGap = 20.f * PanelScale;
    const float EraButtonY = EraPanelY + EraPanelHeight - 78.f * PanelScale;
    const float EraCancelButtonX =
        EraPanelX + EraPanelWidth * 0.5f - EraButtonGap * 0.5f - EraButtonWidth;
    const float EraConfirmButtonX =
        EraPanelX + EraPanelWidth * 0.5f + EraButtonGap * 0.5f;

    if (EraTransitionPanel)
    {
        EraTransitionPanel->SetPos(EraPanelX, EraPanelY);
        EraTransitionPanel->SetSize(EraPanelWidth, EraPanelHeight);
    }

    if (EraTransitionTitleText)
    {
        EraTransitionTitleText->SetPos(
            EraPanelX + EraHorizontalPadding,
            EraPanelY + 28.f * PanelScale);
        EraTransitionTitleText->SetSize(
            EraPanelWidth - EraHorizontalPadding * 2.f,
            40.f * PanelScale);
    }

    if (EraTransitionBodyText)
    {
        EraTransitionBodyText->SetPos(
            EraPanelX + EraHorizontalPadding,
            EraPanelY + 82.f * PanelScale);
        EraTransitionBodyText->SetSize(
            EraPanelWidth - EraHorizontalPadding * 2.f,
            EraPanelHeight - 164.f * PanelScale);
    }

    if (PopupLeftButton)
    {
        PopupLeftButton->SetPos(EraCancelButtonX, EraButtonY);
        PopupLeftButton->SetSize(EraButtonWidth, EraButtonHeight);
    }

    if (PopupRightButton)
    {
        PopupRightButton->SetPos(EraConfirmButtonX, EraButtonY);
        PopupRightButton->SetSize(EraButtonWidth, EraButtonHeight);
    }

    if (PopupLeftButtonText)
    {
        PopupLeftButtonText->SetPos(0.f, 0.f);
        PopupLeftButtonText->SetSize(
            EraButtonWidth,
            EraButtonHeight);
    }

    if (PopupRightButtonText)
    {
        PopupRightButtonText->SetPos(0.f, 0.f);
        PopupRightButtonText->SetSize(
            EraButtonWidth,
            EraButtonHeight);
    }

    if (ConstitutionLeftButton)
    {
        ConstitutionLeftButton->SetPos(EraCancelButtonX, EraButtonY);
        ConstitutionLeftButton->SetSize(EraButtonWidth, EraButtonHeight);
    }

    if (ConstitutionRightButton)
    {
        ConstitutionRightButton->SetPos(EraConfirmButtonX, EraButtonY);
        ConstitutionRightButton->SetSize(EraButtonWidth, EraButtonHeight);
    }

    if (ConstitutionLeftButtonText)
    {
        ConstitutionLeftButtonText->SetPos(0.f, 0.f);
        ConstitutionLeftButtonText->SetSize(
            EraButtonWidth,
            EraButtonHeight);
    }

    if (ConstitutionRightButtonText)
    {
        ConstitutionRightButtonText->SetPos(0.f, 0.f);
        ConstitutionRightButtonText->SetSize(
            EraButtonWidth,
            EraButtonHeight);
    }

    const float ConstitutionScale =
        (std::max)(0.72f, (std::min)(
            1.0f,
            (std::min)(ScreenWidth / 1920.f, ScreenHeight / 1080.f)));
    const float ConstitutionPanelWidth =
        (std::min)(ScreenWidth - 220.f * ConstitutionScale,
            1060.f * ConstitutionScale);
    const float ConstitutionPanelHeight =
        (std::min)(ScreenHeight - 110.f * ConstitutionScale,
            860.f * ConstitutionScale);
    const float ConstitutionPanelLeft =
        (ScreenWidth - ConstitutionPanelWidth) * 0.5f;
    const float ConstitutionPanelTop =
        (ScreenHeight - ConstitutionPanelHeight) * 0.5f;
    const float ConstitutionInnerPadding = 34.f * ConstitutionScale;
    const float ConstitutionTabWidth =
        (ConstitutionPanelWidth - ConstitutionInnerPadding * 2.f -
            12.f * ConstitutionScale * static_cast<float>(
                (std::max)(0, GConstitutionTabSlotCount - 1))) /
        static_cast<float>((std::max)(1, GConstitutionTabSlotCount));
    const float ConstitutionTabHeight = 62.f * ConstitutionScale;
    const float ConstitutionTabBaseY =
        ConstitutionPanelTop - 44.f * ConstitutionScale;
    const float ConstitutionTitleRibbonWidth =
        (std::min)(420.f * ConstitutionScale,
            ConstitutionPanelWidth - 180.f * ConstitutionScale);
    const float ConstitutionTitleRibbonHeight =
        46.f * ConstitutionScale;
    const float ConstitutionRibbonLeft =
        ConstitutionPanelLeft +
        (ConstitutionPanelWidth - ConstitutionTitleRibbonWidth) * 0.5f;
    const float ConstitutionHeaderTop =
        ConstitutionPanelTop + 18.f * ConstitutionScale;
    const float ConstitutionCloseSize = 38.f * ConstitutionScale;
    const float ConstitutionInfoTop =
        ConstitutionPanelTop + 74.f * ConstitutionScale;
    const float ConstitutionContentTop =
        ConstitutionPanelTop + 130.f * ConstitutionScale;
    const float ConstitutionContentWidth =
        ConstitutionPanelWidth - ConstitutionInnerPadding * 2.f;
    const float ConstitutionSummaryGap = 18.f * ConstitutionScale;
    const float ConstitutionSummaryWidth =
        (ConstitutionContentWidth - ConstitutionSummaryGap) * 0.5f;
    const float ConstitutionSummaryHeight = 116.f * ConstitutionScale;
    const float ConstitutionOptionGap = 18.f * ConstitutionScale;
    const float ConstitutionOptionWidth = ConstitutionContentWidth;
    const float ConstitutionOptionHeight =
        (ConstitutionPanelHeight -
            (ConstitutionContentTop - ConstitutionPanelTop) -
            ConstitutionInnerPadding -
            ConstitutionOptionGap * 2.f) / 3.f;

    if (ConstitutionPanel)
    {
        ConstitutionPanel->SetPos(
            ConstitutionPanelLeft,
            ConstitutionPanelTop);
        ConstitutionPanel->SetSize(
            ConstitutionPanelWidth,
            ConstitutionPanelHeight);
    }

    if (ConstitutionTitleRibbon)
    {
        ConstitutionTitleRibbon->SetPos(
            ConstitutionRibbonLeft,
            ConstitutionHeaderTop);
        ConstitutionTitleRibbon->SetSize(
            ConstitutionTitleRibbonWidth,
            ConstitutionTitleRibbonHeight);
    }

    if (ConstitutionTitleText)
    {
        ConstitutionTitleText->SetPos(
            ConstitutionRibbonLeft + 22.f * ConstitutionScale,
            ConstitutionHeaderTop);
        ConstitutionTitleText->SetSize(
            ConstitutionTitleRibbonWidth - 44.f * ConstitutionScale,
            ConstitutionTitleRibbonHeight);
    }

    if (ConstitutionCloseButton)
    {
        ConstitutionCloseButton->SetPos(
            ConstitutionPanelLeft + ConstitutionPanelWidth -
                ConstitutionCloseSize - 12.f * ConstitutionScale,
            ConstitutionPanelTop + 10.f * ConstitutionScale);
        ConstitutionCloseButton->SetSize(
            ConstitutionCloseSize,
            ConstitutionCloseSize);
    }

    if (ConstitutionOverviewButton)
    {
        const float SelectedOffset =
            Widget.GetState().ConstitutionOverviewMode ?
            8.f * ConstitutionScale :
            0.f;
        ConstitutionOverviewButton->SetPos(
            ConstitutionPanelLeft + ConstitutionInnerPadding,
            ConstitutionTabBaseY + SelectedOffset);
        ConstitutionOverviewButton->SetSize(
            ConstitutionTabWidth,
            ConstitutionTabHeight);
    }

    if (ConstitutionOverviewButtonText)
    {
        ConstitutionOverviewButtonText->SetPos(
            10.f * ConstitutionScale,
            6.f * ConstitutionScale);
        ConstitutionOverviewButtonText->SetSize(
            ConstitutionTabWidth - 20.f * ConstitutionScale,
            ConstitutionTabHeight - 12.f * ConstitutionScale);
    }

    if (ConstitutionSubtitleText)
    {
        ConstitutionSubtitleText->SetPos(
            ConstitutionPanelLeft + ConstitutionInnerPadding,
            ConstitutionInfoTop);
        ConstitutionSubtitleText->SetSize(
            ConstitutionContentWidth * 0.58f,
            40.f * ConstitutionScale);
    }

    if (ConstitutionPendingText)
    {
        ConstitutionPendingText->SetPos(
            ConstitutionPanelLeft + ConstitutionInnerPadding +
                ConstitutionContentWidth * 0.40f,
            ConstitutionInfoTop);
        ConstitutionPendingText->SetSize(
            ConstitutionContentWidth * 0.60f,
            40.f * ConstitutionScale);
    }

    for (size_t Index = 0;
        Index < Widget.mConstitutionTopicTabButtons.size();
        ++Index)
    {
        auto Button = Widget.mConstitutionTopicTabButtons[Index].lock();
        auto Text = Index < Widget.mConstitutionTopicTabTexts.size() ?
            Widget.mConstitutionTopicTabTexts[Index].lock() :
            nullptr;

        const EConstitutionTopic Topic = GConstitutionTabTopics[Index];
        const float SelectedOffset =
            (!Widget.GetState().ConstitutionOverviewMode &&
                Widget.GetState().ConstitutionViewedTopic == Topic) ?
            8.f * ConstitutionScale : 0.f;
        const float TabX =
            ConstitutionPanelLeft + ConstitutionInnerPadding +
            (ConstitutionTabWidth + 12.f * ConstitutionScale) *
                static_cast<float>(Index + 1);

        if (Button)
        {
            Button->SetPos(TabX, ConstitutionTabBaseY + SelectedOffset);
            Button->SetSize(
                ConstitutionTabWidth,
                ConstitutionTabHeight);
        }

        if (Text)
        {
            Text->SetPos(
                10.f * ConstitutionScale,
                6.f * ConstitutionScale);
            Text->SetSize(
                ConstitutionTabWidth - 20.f * ConstitutionScale,
                ConstitutionTabHeight - 12.f * ConstitutionScale);
        }
    }

    for (size_t Index = 0;
        Index < Widget.mConstitutionSummaryButtons.size();
        ++Index)
    {
        const int Column = static_cast<int>(Index % 2);
        const int Row = static_cast<int>(Index / 2);
        const float CardX =
            ConstitutionPanelLeft + ConstitutionInnerPadding +
            (ConstitutionSummaryWidth + ConstitutionSummaryGap) *
                static_cast<float>(Column);
        const float CardY =
            ConstitutionContentTop +
            (ConstitutionSummaryHeight + ConstitutionSummaryGap) *
                static_cast<float>(Row);

        auto Button = Widget.mConstitutionSummaryButtons[Index].lock();
        auto TopicText = Index < Widget.mConstitutionSummaryTopicTexts.size() ?
            Widget.mConstitutionSummaryTopicTexts[Index].lock() :
            nullptr;
        auto ValueText = Index < Widget.mConstitutionSummaryValueTexts.size() ?
            Widget.mConstitutionSummaryValueTexts[Index].lock() :
            nullptr;
        auto StatusText = Index < Widget.mConstitutionSummaryStatusTexts.size() ?
            Widget.mConstitutionSummaryStatusTexts[Index].lock() :
            nullptr;

        if (Button)
        {
            Button->SetPos(CardX, CardY);
            Button->SetSize(
                ConstitutionSummaryWidth,
                ConstitutionSummaryHeight);
        }

        if (TopicText)
        {
            TopicText->SetPos(
                CardX + 22.f * ConstitutionScale,
                CardY + 18.f * ConstitutionScale);
            TopicText->SetSize(
                ConstitutionSummaryWidth - 44.f * ConstitutionScale,
                28.f * ConstitutionScale);
        }

        if (ValueText)
        {
            ValueText->SetPos(
                CardX + 22.f * ConstitutionScale,
                CardY + 48.f * ConstitutionScale);
            ValueText->SetSize(
                ConstitutionSummaryWidth - 44.f * ConstitutionScale,
                32.f * ConstitutionScale);
        }

        if (StatusText)
        {
            StatusText->SetPos(
                CardX + 22.f * ConstitutionScale,
                CardY + 82.f * ConstitutionScale);
            StatusText->SetSize(
                ConstitutionSummaryWidth - 44.f * ConstitutionScale,
                24.f * ConstitutionScale);
        }
    }

    for (size_t Index = 0;
        Index < Widget.mConstitutionOptionButtons.size();
        ++Index)
    {
        const float CardX =
            ConstitutionPanelLeft + ConstitutionInnerPadding;
        const float CardY =
            ConstitutionContentTop +
            (ConstitutionOptionHeight + ConstitutionOptionGap) *
                static_cast<float>(Index);

        auto Button = Widget.mConstitutionOptionButtons[Index].lock();
        auto TitleText = Index < Widget.mConstitutionOptionTitleTexts.size() ?
            Widget.mConstitutionOptionTitleTexts[Index].lock() :
            nullptr;
        auto SummaryText = Index < Widget.mConstitutionOptionSummaryTexts.size() ?
            Widget.mConstitutionOptionSummaryTexts[Index].lock() :
            nullptr;
        auto BodyText = Index < Widget.mConstitutionOptionBodyTexts.size() ?
            Widget.mConstitutionOptionBodyTexts[Index].lock() :
            nullptr;

        if (Button)
        {
            Button->SetPos(CardX, CardY);
            Button->SetSize(
                ConstitutionOptionWidth,
                ConstitutionOptionHeight);
        }

        if (TitleText)
        {
            TitleText->SetPos(
                CardX + 24.f * ConstitutionScale,
                CardY + 18.f * ConstitutionScale);
            TitleText->SetSize(
                ConstitutionOptionWidth - 48.f * ConstitutionScale,
                32.f * ConstitutionScale);
        }

        if (SummaryText)
        {
            SummaryText->SetPos(
                CardX + 24.f * ConstitutionScale,
                CardY + 56.f * ConstitutionScale);
            SummaryText->SetSize(
                ConstitutionOptionWidth - 48.f * ConstitutionScale,
                24.f * ConstitutionScale);
        }

        if (BodyText)
        {
            BodyText->SetPos(
                CardX + 24.f * ConstitutionScale,
                CardY + 84.f * ConstitutionScale);
            BodyText->SetSize(
                ConstitutionOptionWidth - 48.f * ConstitutionScale,
                ConstitutionOptionHeight - 104.f * ConstitutionScale);
        }
    }
}
