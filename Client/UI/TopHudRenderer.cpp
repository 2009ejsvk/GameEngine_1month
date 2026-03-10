#include "TopHudRenderer.h"
#include "TopHudWidget.h"
#include "TropicoUiStyle.h"
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

    constexpr int GSpeedButtonCount = 4;
    constexpr int GMenuButtonCount = 8;
    constexpr int GMenuConstructionIndex = 1;
    constexpr int GMenuEdictsIndex = 2;
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
    constexpr const TCHAR* GStatusMoneyIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png");
    constexpr const TCHAR* GStatusNpcIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_population.png");
    constexpr const TCHAR* GStatusSupportIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_political_approval.png");

    const TCHAR* const GSpeedIcons[GSpeedButtonCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ButtonIcons\\T_ICO_gamespeed_pause.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ButtonIcons\\T_ICO_gamespeed_playTwo.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\ButtonIcons\\T_ICO_gamespeed_playThree.png"),
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

    const wchar_t* const GMenuLabels[GMenuButtonCount] =
    {
        L"임무",
        L"건설",
        L"칙령",
        L"헌법",
        L"무역",
        L"원정",
        L"연구",
        L"연감"
    };

    void ConfigureIconButtonStyle(const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(0.96f, 0.96f, 0.96f, 0.95f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(1.f, 1.f, 1.f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.78f, 0.78f, 0.78f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.40f, 0.40f, 0.40f, 0.75f));
    }

    void ApplyTopHudTextureToAllButtonStates(
        const std::shared_ptr<CButton>& Button,
        const std::string& TextureKey,
        const TCHAR* TextureFile)
    {
        if (!Button || !TextureFile)
            return;

        if (!Button->SetTexture(EButtonState::Normal, TextureKey, TextureFile))
            return;

        Button->SetTexture(EButtonState::Hovered, TextureKey);
        Button->SetTexture(EButtonState::Click, TextureKey);
        Button->SetTexture(EButtonState::Disable, TextureKey);
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
        Widget.CreateWidget<CTextBlock>("TopHud_BudgetText", 17).lock();

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
        BudgetLabelText->SetText(TEXT("예산"));
        BudgetLabelText->SetFontSize(12.f);
        BudgetLabelText->SetAlignH(ETextAlignH::Left);
        BudgetLabelText->SetAlignV(ETextAlignV::Middle);
        BudgetLabelText->SetTextColor(172, 146, 98, 255);
        BudgetLabelText->EnableShadow(true);
        BudgetLabelText->SetShadowOffset(1.f, 1.f);
        BudgetLabelText->SetShadowTextColor(22, 18, 12, 180);
        Widget.mBudgetLabelText = BudgetLabelText;
    }

    auto ElectionText =
        Widget.CreateWidget<CTextBlock>("TopHud_ElectionText", 17).lock();

    if (ElectionText)
    {
        ElectionText->SetText(TEXT("차기 선거 -"));
        ElectionText->SetFontSize(15.f);
        ElectionText->SetAlignH(ETextAlignH::Left);
        ElectionText->SetAlignV(ETextAlignV::Top);
        ElectionText->SetTextColor(245, 235, 210, 255);
        ElectionText->EnableShadow(true);
        ElectionText->SetShadowOffset(1.f, 1.f);
        ElectionText->SetShadowTextColor(16, 16, 16, 220);
        Widget.mElectionText = ElectionText;
    }

    auto TaxPolicyText =
        Widget.CreateWidget<CTextBlock>("TopHud_TaxPolicyText", 17).lock();

    if (TaxPolicyText)
    {
        TaxPolicyText->SetText(TEXT("세금 10/12/35%"));
        TaxPolicyText->SetFontSize(13.f);
        TaxPolicyText->SetAlignH(ETextAlignH::Left);
        TaxPolicyText->SetAlignV(ETextAlignV::Top);
        TaxPolicyText->SetTextColor(229, 220, 198, 255);
        TaxPolicyText->EnableShadow(true);
        TaxPolicyText->SetShadowOffset(1.f, 1.f);
        TaxPolicyText->SetShadowTextColor(16, 16, 16, 220);
        Widget.mTaxPolicyText = TaxPolicyText;
    }

    auto EventText =
        Widget.CreateWidget<CTextBlock>("TopHud_EventText", 17).lock();

    if (EventText)
    {
        EventText->SetText(TEXT("현재 상태 안정"));
        EventText->SetFontSize(13.f);
        EventText->SetAlignH(ETextAlignH::Left);
        EventText->SetAlignV(ETextAlignV::Top);
        EventText->SetTextColor(208, 226, 198, 255);
        EventText->EnableShadow(true);
        EventText->SetShadowOffset(1.f, 1.f);
        EventText->SetShadowTextColor(16, 16, 16, 220);
        Widget.mEventText = EventText;
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
        NpcLabelText->SetText(TEXT("인구"));
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
        SupportLabelText->SetText(TEXT("지지율"));
        SupportLabelText->SetFontSize(12.f);
        SupportLabelText->SetAlignH(ETextAlignH::Left);
        SupportLabelText->SetAlignV(ETextAlignV::Middle);
        SupportLabelText->SetTextColor(172, 146, 98, 255);
        SupportLabelText->EnableShadow(true);
        SupportLabelText->SetShadowOffset(1.f, 1.f);
        SupportLabelText->SetShadowTextColor(22, 18, 12, 180);
        Widget.mSupportLabelText = SupportLabelText;
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
        GameOverTitleText->SetText(TEXT("정권 상실"));
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

    for (int i = 0; i < GSpeedButtonCount; ++i)
    {
        auto Button = Widget.CreateWidget<CButton>(
            "TopHud_SpeedButton_" + std::to_string(i + 1),
            18).lock();

        if (!Button)
            continue;

        ConfigureIconButtonStyle(Button);
        Button->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click,
            &Widget,
            &CTopHudWidget::OnAnyButtonClick);

        ApplyTopHudTextureToAllButtonStates(
            Button,
            "TopHudSpeedIcon_" + std::to_string(i),
            GSpeedIcons[i]);

        Widget.mSpeedButtons[i] = Button;
    }

    Widget.mMenuButtons.resize(GMenuButtonCount);
    Widget.mMenuButtonTexts.resize(GMenuButtonCount);

    for (int i = 0; i < GMenuButtonCount; ++i)
    {
        auto Button = Widget.CreateWidget<CButton>(
            "TopHud_MenuButton_" + std::to_string(i + 1),
            18).lock();

        if (!Button)
            continue;

        ConfigureIconButtonStyle(Button);

        void (CTopHudWidget::*MenuCallback)() =
            &CTopHudWidget::OnAnyButtonClick;

        if (i == GMenuConstructionIndex)
            MenuCallback = &CTopHudWidget::OnConstructionButtonClick;
        else if (i == GMenuEdictsIndex)
            MenuCallback = &CTopHudWidget::OnEdictsButtonClick;
        else if (i == GMenuAlmanacIndex)
            MenuCallback = &CTopHudWidget::OnAlmanacButtonClick;

        Button->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click,
            &Widget,
            MenuCallback);

        ApplyTopHudTextureToAllButtonStates(
            Button,
            "TopHudMenuIcon_" + std::to_string(i),
            GMenuIcons[i]);

        Widget.mMenuButtons[i] = Button;

        auto MenuText = Widget.CreateWidget<CTextBlock>(
            "TopHud_MenuText_" + std::to_string(i + 1),
            18).lock();

        if (!MenuText)
            continue;

        MenuText->SetText(GMenuLabels[i]);
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
    auto ElectionText = Widget.mElectionText.lock();
    auto TaxPolicyText = Widget.mTaxPolicyText.lock();
    auto EventText = Widget.mEventText.lock();
    auto NpcText = Widget.mNpcText.lock();
    auto SupportText = Widget.mSupportText.lock();
    auto GameOverDim = Widget.mGameOverDim.lock();
    auto GameOverPanel = Widget.mGameOverPanel.lock();
    auto GameOverTitleText = Widget.mGameOverTitleText.lock();
    auto GameOverBodyText = Widget.mGameOverBodyText.lock();

    if (DateText)
        DateText->SetText(Snapshot.DateText.c_str());

    if (BudgetText)
        BudgetText->SetText(Snapshot.BudgetText.c_str());

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

    if (NpcText)
        NpcText->SetText(Snapshot.NpcText.c_str());

    if (SupportText)
        SupportText->SetText(Snapshot.SupportText.c_str());

    for (size_t i = 0; i < Widget.mSpeedButtons.size(); ++i)
    {
        auto Button = Widget.mSpeedButtons[i].lock();

        if (Button)
            Button->ButtonEnable(Snapshot.CanUseButtons);
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
    auto ElectionText = Widget.mElectionText.lock();
    auto TaxPolicyText = Widget.mTaxPolicyText.lock();
    auto EventText = Widget.mEventText.lock();
    auto NpcLabelText = Widget.mNpcLabelText.lock();
    auto NpcText = Widget.mNpcText.lock();
    auto SupportLabelText = Widget.mSupportLabelText.lock();
    auto SupportText = Widget.mSupportText.lock();
    auto GameOverDim = Widget.mGameOverDim.lock();
    auto GameOverPanel = Widget.mGameOverPanel.lock();
    auto GameOverTitleText = Widget.mGameOverTitleText.lock();
    auto GameOverBodyText = Widget.mGameOverBodyText.lock();
    const float TopHudScale =
        (std::max)(0.72f, (std::min)(1.05f, ScreenWidth / 1920.f));
    const float TopHudPanelX = 16.f;
    const float TopHudPanelW = 388.f * TopHudScale;
    const float TopHudPanelH = 182.f * TopHudScale;
    const float TopHudPanelY = (std::max)(
        12.f, ScreenHeight - TopHudPanelH - 18.f);

    const float StatusScale =
        (std::max)(0.72f, (std::min)(1.05f, ScreenWidth / 1920.f));
    const float StatusX = 14.f;
    const float StatusY = 10.f;
    const float StatusPaddingX = 20.f * StatusScale;
    const float StatusBlockGap = 18.f * StatusScale;
    const float StatusMoneyBlockW = 218.f * StatusScale;
    const float StatusNpcBlockW = 128.f * StatusScale;
    const float StatusSupportBlockW = 132.f * StatusScale;
    const float StatusW =
        StatusPaddingX * 2.f +
        StatusMoneyBlockW +
        StatusNpcBlockW +
        StatusSupportBlockW +
        StatusBlockGap * 2.f;
    const float StatusH = 94.f * StatusScale;
    const float StatusIconSize = 26.f * StatusScale;
    const float StatusLabelY = StatusY + 12.f * StatusScale;
    const float StatusValueY = StatusY + 30.f * StatusScale;
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
            float BlockW)
        {
            if (Icon)
            {
                Icon->SetPos(BlockX, StatusValueY + 1.f * StatusScale);
                Icon->SetSize(StatusIconSize, StatusIconSize);
            }

            const float TextX = BlockX + StatusIconSize + 10.f * StatusScale;
            const float TextW = BlockW - StatusIconSize - 10.f * StatusScale;

            if (LabelText)
            {
                LabelText->SetFontSize(12.f * StatusScale);
                LabelText->SetPos(TextX, StatusLabelY);
                LabelText->SetSize(TextW, 16.f * StatusScale);
            }

            if (ValueText)
            {
                ValueText->SetFontSize(22.f * StatusScale);
                ValueText->SetPos(TextX, StatusValueY);
                ValueText->SetSize(TextW, 30.f * StatusScale);
            }
        };

    LayoutStatusBlock(
        StatusMoneyIcon,
        BudgetLabelText,
        BudgetText,
        StatusBlockMoneyX,
        StatusMoneyBlockW);
    LayoutStatusBlock(
        StatusNpcIcon,
        NpcLabelText,
        NpcText,
        StatusBlockNpcX,
        StatusNpcBlockW);
    LayoutStatusBlock(
        StatusSupportIcon,
        SupportLabelText,
        SupportText,
        StatusBlockSupportX,
        StatusSupportBlockW);

    if (SpeedPanel)
    {
        SpeedPanel->SetPos(TopHudPanelX, TopHudPanelY);
        SpeedPanel->SetSize(TopHudPanelW, TopHudPanelH);
    }

    const float TimeBarX = TopHudPanelX + 74.f * TopHudScale;
    const float TimeBarY = TopHudPanelY + 16.f * TopHudScale;
    const float TimeBarW = 226.f * TopHudScale;
    const float TimeBarH = 14.f * TopHudScale;

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
        DateText->SetFontSize(22.f * TopHudScale);
        DateText->SetPos(
            TopHudPanelX + 74.f * TopHudScale,
            TopHudPanelY + 34.f * TopHudScale);
        DateText->SetSize(240.f * TopHudScale, 28.f * TopHudScale);
    }

    if (ElectionText)
    {
        ElectionText->SetFontSize(15.f * TopHudScale);
        ElectionText->SetPos(
            TopHudPanelX + 74.f * TopHudScale,
            TopHudPanelY + 68.f * TopHudScale);
        ElectionText->SetSize(248.f * TopHudScale, 22.f * TopHudScale);
    }

    if (TaxPolicyText)
    {
        TaxPolicyText->SetFontSize(13.f * TopHudScale);
        TaxPolicyText->SetPos(
            TopHudPanelX + 74.f * TopHudScale,
            TopHudPanelY + 92.f * TopHudScale);
        TaxPolicyText->SetSize(248.f * TopHudScale, 18.f * TopHudScale);
    }

    if (EventText)
    {
        EventText->SetFontSize(13.5f * TopHudScale);
        EventText->SetPos(
            TopHudPanelX + 74.f * TopHudScale,
            TopHudPanelY + 114.f * TopHudScale);
        EventText->SetSize(276.f * TopHudScale, 34.f * TopHudScale);
    }

    const float SpeedButtonSize = 44.f * TopHudScale;
    const float SpeedButtonStep = 52.f * TopHudScale;
    const float SpeedButtonY =
        TopHudPanelY + TopHudPanelH - SpeedButtonSize - 18.f * TopHudScale;
    const float SpeedButtonStartX = TopHudPanelX + 24.f * TopHudScale;

    for (int i = 0; i < static_cast<int>(Widget.mSpeedButtons.size()); ++i)
    {
        auto SpeedButton = Widget.mSpeedButtons[i].lock();

        if (!SpeedButton)
            continue;

        SpeedButton->SetPos(
            SpeedButtonStartX + SpeedButtonStep * static_cast<float>(i),
            SpeedButtonY);
        SpeedButton->SetSize(SpeedButtonSize, SpeedButtonSize);
    }

    float MenuButtonSize = 60.f * TopHudScale;
    float MenuButtonGap = 10.f * TopHudScale;
    float MenuLabelGap = 8.f * TopHudScale;
    const float MenuStartX =
        TopHudPanelX + TopHudPanelW + 26.f * TopHudScale;
    const float MenuY = TopHudPanelY + 8.f * TopHudScale;

    const float WantedMenuWidth =
        MenuButtonSize * GMenuButtonCount +
        MenuButtonGap * static_cast<float>(GMenuButtonCount - 1);
    const float MaxMenuWidth =
        (std::max)(120.f, ScreenWidth - MenuStartX - 14.f);

    if (WantedMenuWidth > MaxMenuWidth)
    {
        const float MenuScale =
            (std::max)(0.70f, MaxMenuWidth / WantedMenuWidth);
        MenuButtonSize *= MenuScale;
        MenuButtonGap *= MenuScale;
        MenuLabelGap *= MenuScale;
    }

    const float MenuTextFontSize = 12.5f * MenuButtonSize / 60.f;
    const float MenuTextHeight = 18.f * MenuButtonSize / 60.f;

    for (int i = 0; i < static_cast<int>(Widget.mMenuButtons.size()); ++i)
    {
        auto MenuButton = Widget.mMenuButtons[i].lock();
        auto MenuText = Widget.mMenuButtonTexts[i].lock();
        const float MenuButtonX =
            MenuStartX +
            (MenuButtonSize + MenuButtonGap) * static_cast<float>(i);

        if (MenuButton)
        {
            MenuButton->SetPos(MenuButtonX, MenuY);
            MenuButton->SetSize(MenuButtonSize, MenuButtonSize);
        }

        if (MenuText)
        {
            MenuText->SetFontSize(MenuTextFontSize);
            MenuText->SetPos(
                MenuButtonX - MenuButtonSize * 0.10f,
                MenuY + MenuButtonSize + MenuLabelGap);
            MenuText->SetSize(
                MenuButtonSize * 1.20f,
                MenuTextHeight);
        }
    }

    const float OverlayWidth = ScreenWidth;
    const float OverlayHeight = ScreenHeight;
    const float PanelScale =
        (std::max)(0.72f, (std::min)(1.05f, ScreenWidth / 1920.f));
    const float PanelWidth = 720.f * PanelScale;
    const float PanelHeight = 390.f * PanelScale;
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
            PanelX + 70.f * PanelScale,
            PanelY + 70.f * PanelScale);
        GameOverTitleText->SetSize(
            PanelWidth - 140.f * PanelScale,
            48.f * PanelScale);
    }

    if (GameOverBodyText)
    {
        GameOverBodyText->SetPos(
            PanelX + 84.f * PanelScale,
            PanelY + 138.f * PanelScale);
        GameOverBodyText->SetSize(
            PanelWidth - 168.f * PanelScale,
            PanelHeight - 210.f * PanelScale);
    }
}
