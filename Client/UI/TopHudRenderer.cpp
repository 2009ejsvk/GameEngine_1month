#include "TopHudRenderer.h"
#include "TopHudWidget.h"
#include "UIStrings.h"
#include "TropicoUiStyle.h"
#include "UILayoutConfig.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include <algorithm>
#include <string>

namespace
{
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

    constexpr int GSpeedButtonCount = 2;
    constexpr int GSpeedStateButtonIndex = 0;
    constexpr int GSpeedMultiplierButtonIndex = 1;
    constexpr int GMenuButtonCount = 8;
    constexpr int GMenuConstructionIndex = 1;
    constexpr int GMenuEdictsIndex = 2;
    constexpr int GMenuTradeIndex = 4;
    constexpr int GMenuAlmanacIndex = 7;

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

        if (i == GMenuConstructionIndex)
            MenuCallback = &CTopHudWidget::OnConstructionButtonClick;
        else if (i == GMenuEdictsIndex)
            MenuCallback = &CTopHudWidget::OnEdictsButtonClick;
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
    Widget.mMonthProgress = Snapshot.MonthProgress;
    Widget.mGameLost = Snapshot.GameLost;

    auto DateText = Widget.mDateText.lock();
    auto BudgetText = Widget.mBudgetText.lock();
    auto NpcText = Widget.mNpcText.lock();
    auto SupportText = Widget.mSupportText.lock();
    auto ElectionText = Widget.mElectionText.lock();
    auto TaxPolicyText = Widget.mTaxPolicyText.lock();
    auto EventText = Widget.mEventText.lock();
    auto GameOverDim = Widget.mGameOverDim.lock();
    auto GameOverPanel = Widget.mGameOverPanel.lock();
    auto GameOverTitleText = Widget.mGameOverTitleText.lock();
    auto GameOverBodyText = Widget.mGameOverBodyText.lock();

    if (DateText)
        DateText->SetText(Snapshot.DateText.c_str());

    if (BudgetText)
        BudgetText->SetText(Snapshot.BudgetText.c_str());

    if (NpcText)
        NpcText->SetText(Snapshot.NpcText.c_str());

    if (SupportText)
        SupportText->SetText(Snapshot.SupportText.c_str());

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
            Button->ButtonEnable(Snapshot.CanUseButtons);
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
            Button->ButtonEnable(Snapshot.CanUseButtons);
    }

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
    auto NpcLabelText = Widget.mNpcLabelText.lock();
    auto NpcText = Widget.mNpcText.lock();
    auto SupportLabelText = Widget.mSupportLabelText.lock();
    auto SupportText = Widget.mSupportText.lock();
    auto ElectionText = Widget.mElectionText.lock();
    auto TaxPolicyText = Widget.mTaxPolicyText.lock();
    auto EventText = Widget.mEventText.lock();
    auto GameOverDim = Widget.mGameOverDim.lock();
    auto GameOverPanel = Widget.mGameOverPanel.lock();
    auto GameOverTitleText = Widget.mGameOverTitleText.lock();
    auto GameOverBodyText = Widget.mGameOverBodyText.lock();
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
    const float StatusW =
        StatusPaddingX * 2.f +
        StatusMoneyBlockW +
        StatusNpcBlockW +
        StatusSupportBlockW +
        StatusBlockGap * 2.f;
    const float StatusH = UIConfig::StatusBarHeight * StatusScale;
    const float StatusIconSize = UIConfig::StatusIconSize * StatusScale;
    const float StatusLabelY = StatusY + UIConfig::StatusLabelOffsetY * StatusScale;
    const float StatusValueY = StatusY + UIConfig::StatusValueOffsetY * StatusScale;
    const float StatusBlockMoneyX = StatusX + StatusPaddingX;
    const float StatusBlockNpcX =
        StatusBlockMoneyX + StatusMoneyBlockW + StatusBlockGap;
    const float StatusBlockSupportX =
        StatusBlockNpcX + StatusNpcBlockW + StatusBlockGap;

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
        TimeBarFill->SetSize(TimeBarW * Widget.mMonthProgress, TimeBarH);
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
}
