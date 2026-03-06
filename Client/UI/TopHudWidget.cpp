#include "TopHudWidget.h"
#include "BuildMenuWidget.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../World/MainWorld.h"
#include "../ObjectNames.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/World.h"
#include "World/WorldUIManager.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    constexpr int GSpeedButtonCount = 4;
    constexpr int GMenuButtonCount = 8;
    constexpr int GMenuConstructionIndex = 1;
    constexpr int GMenuAlmanacIndex = 7;

    constexpr const TCHAR* GSpeedPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Gamespeed\\T_gamespeed_deco_bg.png");
    constexpr const TCHAR* GTimeBarBackTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Gamespeed\\T_gamespeed_timeBar_bg.png");
    constexpr const TCHAR* GTimeBarFillTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Gamespeed\\T_gamespeed_timeBar.png");
    constexpr const TCHAR* GStatusBarTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Notifications\\T_staticData_bg.png");
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

    void ApplyTextureToAllButtonStates(
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

    std::wstring FormatCurrency(long long Value)
    {
        bool Negative = false;
        unsigned long long AbsValue = 0;

        if (Value < 0)
        {
            Negative = true;
            AbsValue = static_cast<unsigned long long>(-Value);
        }
        else
        {
            AbsValue = static_cast<unsigned long long>(Value);
        }

        std::wstring Digits = std::to_wstring(AbsValue);

        for (int i = static_cast<int>(Digits.size()) - 3; i > 0; i -= 3)
        {
            Digits.insert(static_cast<size_t>(i), 1, L',');
        }

        if (Negative)
            Digits.insert(Digits.begin(), L'-');

        return L"$" + Digits;
    }
}

CTopHudWidget::CTopHudWidget()
{
}

CTopHudWidget::~CTopHudWidget()
{
}

bool CTopHudWidget::Init()
{
    CWidgetContainer::Init();

    auto SpeedPanel =
        CreateWidget<CImage>("TopHud_SpeedPanel", 14).lock();

    if (SpeedPanel)
    {
        SpeedPanel->SetTexture("TopHudSpeedPanel", GSpeedPanelTexture);
        SpeedPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        mSpeedPanel = SpeedPanel;
    }

    auto TimeBarBack =
        CreateWidget<CImage>("TopHud_TimeBarBack", 15).lock();

    if (TimeBarBack)
    {
        TimeBarBack->SetTexture("TopHudTimeBarBack", GTimeBarBackTexture);
        TimeBarBack->SetTint(1.f, 1.f, 1.f, 1.f);
        mTimeBarBack = TimeBarBack;
    }

    auto TimeBarFill =
        CreateWidget<CImage>("TopHud_TimeBarFill", 16).lock();

    if (TimeBarFill)
    {
        TimeBarFill->SetTexture("TopHudTimeBarFill", GTimeBarFillTexture);
        TimeBarFill->SetTint(1.f, 1.f, 1.f, 1.f);
        mTimeBarFill = TimeBarFill;
    }

    auto DateText =
        CreateWidget<CTextBlock>("TopHud_DateText", 17).lock();

    if (DateText)
    {
        DateText->SetText(TEXT("2월, 1959"));
        DateText->SetFontSize(16.f);
        DateText->SetAlignH(ETextAlignH::Left);
        DateText->SetAlignV(ETextAlignV::Middle);
        DateText->SetTextColor(245, 235, 210, 255);
        DateText->EnableShadow(true);
        DateText->SetShadowOffset(1.f, 1.f);
        DateText->SetShadowTextColor(18, 18, 18, 220);
        mDateText = DateText;
    }

    auto BudgetText =
        CreateWidget<CTextBlock>("TopHud_BudgetText", 17).lock();

    if (BudgetText)
    {
        BudgetText->SetText(TEXT("$0"));
        BudgetText->SetFontSize(15.f);
        BudgetText->SetAlignH(ETextAlignH::Left);
        BudgetText->SetAlignV(ETextAlignV::Middle);
        BudgetText->SetTextColor(245, 235, 210, 255);
        BudgetText->EnableShadow(true);
        BudgetText->SetShadowOffset(1.f, 1.f);
        BudgetText->SetShadowTextColor(16, 16, 16, 220);
        mBudgetText = BudgetText;
    }

    auto StatusBar =
        CreateWidget<CImage>("TopHud_StatusBar", 19).lock();

    if (StatusBar)
    {
        StatusBar->SetTexture("TopHudStatusBar", GStatusBarTexture);
        StatusBar->SetTint(1.f, 1.f, 1.f, 1.f);
        mStatusBar = StatusBar;
    }

    auto StatusMoneyIcon =
        CreateWidget<CImage>("TopHud_StatusMoneyIcon", 20).lock();

    if (StatusMoneyIcon)
    {
        StatusMoneyIcon->SetTexture(
            "TopHudStatusMoneyIcon", GStatusMoneyIconTexture);
        StatusMoneyIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        mStatusMoneyIcon = StatusMoneyIcon;
    }

    auto StatusNpcIcon =
        CreateWidget<CImage>("TopHud_StatusNpcIcon", 20).lock();

    if (StatusNpcIcon)
    {
        StatusNpcIcon->SetTexture(
            "TopHudStatusNpcIcon", GStatusNpcIconTexture);
        StatusNpcIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        mStatusNpcIcon = StatusNpcIcon;
    }

    auto StatusSupportIcon =
        CreateWidget<CImage>("TopHud_StatusSupportIcon", 20).lock();

    if (StatusSupportIcon)
    {
        StatusSupportIcon->SetTexture(
            "TopHudStatusSupportIcon", GStatusSupportIconTexture);
        StatusSupportIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        mStatusSupportIcon = StatusSupportIcon;
    }

    auto NpcText =
        CreateWidget<CTextBlock>("TopHud_StatusNpcText", 20).lock();

    if (NpcText)
    {
        NpcText->SetText(TEXT("0"));
        NpcText->SetFontSize(15.f);
        NpcText->SetAlignH(ETextAlignH::Left);
        NpcText->SetAlignV(ETextAlignV::Middle);
        NpcText->SetTextColor(245, 235, 210, 255);
        NpcText->EnableShadow(true);
        NpcText->SetShadowOffset(1.f, 1.f);
        NpcText->SetShadowTextColor(16, 16, 16, 220);
        mNpcText = NpcText;
    }

    auto SupportText =
        CreateWidget<CTextBlock>("TopHud_StatusSupportText", 20).lock();

    if (SupportText)
    {
        SupportText->SetText(TEXT("0%"));
        SupportText->SetFontSize(15.f);
        SupportText->SetAlignH(ETextAlignH::Left);
        SupportText->SetAlignV(ETextAlignV::Middle);
        SupportText->SetTextColor(245, 235, 210, 255);
        SupportText->EnableShadow(true);
        SupportText->SetShadowOffset(1.f, 1.f);
        SupportText->SetShadowTextColor(16, 16, 16, 220);
        mSupportText = SupportText;
    }

    mSpeedButtons.resize(GSpeedButtonCount);

    for (int i = 0; i < GSpeedButtonCount; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "TopHud_SpeedButton_" + std::to_string(i + 1), 18).lock();

        if (!Button)
            continue;

        ConfigureIconButtonStyle(Button);
        Button->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click, this,
            &CTopHudWidget::OnAnyButtonClick);

        ApplyTextureToAllButtonStates(
            Button,
            "TopHudSpeedIcon_" + std::to_string(i),
            GSpeedIcons[i]);

        mSpeedButtons[i] = Button;
    }

    mMenuButtons.resize(GMenuButtonCount);

    for (int i = 0; i < GMenuButtonCount; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "TopHud_MenuButton_" + std::to_string(i + 1), 18).lock();

        if (!Button)
            continue;

        ConfigureIconButtonStyle(Button);

        void (CTopHudWidget::* MenuCallback)() = &CTopHudWidget::OnAnyButtonClick;

        if (i == GMenuConstructionIndex)
            MenuCallback = &CTopHudWidget::OnConstructionButtonClick;
        else if (i == GMenuAlmanacIndex)
            MenuCallback = &CTopHudWidget::OnAlmanacButtonClick;

        Button->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click, this, MenuCallback);

        ApplyTextureToAllButtonStates(
            Button,
            "TopHudMenuIcon_" + std::to_string(i),
            GMenuIcons[i]);

        mMenuButtons[i] = Button;
    }

    return true;
}

void CTopHudWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
    RefreshData();
    RefreshLayout();
}

void CTopHudWidget::RefreshData()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);

    if (!MainWorld)
        return;

    auto BudgetText = mBudgetText.lock();
    auto DateText = mDateText.lock();
    auto NpcText = mNpcText.lock();
    auto SupportText = mSupportText.lock();

    if (BudgetText)
    {
        BudgetText->SetText(
            FormatCurrency(MainWorld->GetNationalBudget()).c_str());
    }

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;
    int ActiveNpcCount = 0;
    double TotalSupportValue = 0.0;

    if (World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
    {
        for (size_t i = 0; i < OrbList.size(); ++i)
        {
            auto Orb = OrbList[i].lock();

            if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                continue;

            ++ActiveNpcCount;
            TotalSupportValue +=
                static_cast<double>(Orb->GetSatisfaction().Overall);
        }
    }

    if (NpcText)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%d", ActiveNpcCount);
        NpcText->SetText(Buffer);
    }

    if (SupportText)
    {
        int SupportPercent = 0;

        if (ActiveNpcCount > 0)
        {
            SupportPercent = static_cast<int>(round(
                TotalSupportValue / static_cast<double>(ActiveNpcCount)));
        }

        SupportPercent = (std::max)(0, (std::min)(100, SupportPercent));

        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%d%%", SupportPercent);
        SupportText->SetText(Buffer);
    }

    const int Month = MainWorld->GetSimulationMonth();
    const int Year = MainWorld->GetSimulationYear();

    if (DateText)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%d월, %04d", Month, Year);
        DateText->SetText(Buffer);
    }

    const float MonthProgress = MainWorld->GetSimulationMonthProgress();
    mMonthProgress = (std::max)(0.f, (std::min)(1.f, MonthProgress));
}

void CTopHudWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);

    const float TopHudScale =
        (std::max)(0.62f, (std::min)(1.f, ScreenWidth / 1920.f));
    const float TopHudPanelX = 16.f;
    const float TopHudPanelW = 303.f * TopHudScale;
    const float TopHudPanelH = 130.f * TopHudScale;
    const float TopHudPanelY = (std::max)(
        12.f, ScreenHeight - TopHudPanelH - 18.f);

    auto SpeedPanel = mSpeedPanel.lock();
    auto TimeBarBack = mTimeBarBack.lock();
    auto TimeBarFill = mTimeBarFill.lock();
    auto DateText = mDateText.lock();
    auto BudgetText = mBudgetText.lock();
    auto StatusBar = mStatusBar.lock();
    auto StatusMoneyIcon = mStatusMoneyIcon.lock();
    auto StatusNpcIcon = mStatusNpcIcon.lock();
    auto StatusSupportIcon = mStatusSupportIcon.lock();
    auto NpcText = mNpcText.lock();
    auto SupportText = mSupportText.lock();

    const float StatusScale =
        (std::max)(0.58f, (std::min)(0.78f, ScreenWidth / 2400.f));
    const float StatusX = 14.f;
    const float StatusY = 10.f;
    const float StatusW = 504.f * StatusScale;
    const float StatusH = 76.f * StatusScale;
    const float StatusIconSize = 50.f * StatusScale * 0.58f;
    const float StatusIconY =
        StatusY + (StatusH - StatusIconSize) * 0.5f - 1.f;
    const float MoneyIconX = StatusX + 132.f * StatusScale;
    const float NpcIconX = StatusX + 252.f * StatusScale;
    const float SupportIconX = StatusX + 362.f * StatusScale;
    const float StatusTextY = StatusY + 2.f * StatusScale;
    const float StatusTextH = StatusH - 4.f * StatusScale;

    if (StatusBar)
    {
        StatusBar->SetPos(StatusX, StatusY);
        StatusBar->SetSize(StatusW, StatusH);
    }

    if (StatusMoneyIcon)
    {
        StatusMoneyIcon->SetPos(MoneyIconX, StatusIconY);
        StatusMoneyIcon->SetSize(StatusIconSize, StatusIconSize);
    }

    if (StatusNpcIcon)
    {
        StatusNpcIcon->SetPos(NpcIconX, StatusIconY);
        StatusNpcIcon->SetSize(StatusIconSize, StatusIconSize);
    }

    if (StatusSupportIcon)
    {
        StatusSupportIcon->SetPos(SupportIconX, StatusIconY);
        StatusSupportIcon->SetSize(StatusIconSize, StatusIconSize);
    }

    if (BudgetText)
    {
        BudgetText->SetPos(
            MoneyIconX + StatusIconSize + 5.f * StatusScale,
            StatusTextY);
        BudgetText->SetSize(108.f * StatusScale, StatusTextH);
    }

    if (NpcText)
    {
        NpcText->SetPos(
            NpcIconX + StatusIconSize + 5.f * StatusScale,
            StatusTextY);
        NpcText->SetSize(68.f * StatusScale, StatusTextH);
    }

    if (SupportText)
    {
        SupportText->SetPos(
            SupportIconX + StatusIconSize + 5.f * StatusScale,
            StatusTextY);
        SupportText->SetSize(72.f * StatusScale, StatusTextH);
    }

    if (SpeedPanel)
    {
        SpeedPanel->SetPos(TopHudPanelX, TopHudPanelY);
        SpeedPanel->SetSize(TopHudPanelW, TopHudPanelH);
    }

    const float TimeBarX = TopHudPanelX + 62.f * TopHudScale;
    const float TimeBarY = TopHudPanelY + 10.f * TopHudScale;
    const float TimeBarW = 177.f * TopHudScale;
    const float TimeBarH = 13.f * TopHudScale;

    if (TimeBarBack)
    {
        TimeBarBack->SetPos(TimeBarX, TimeBarY);
        TimeBarBack->SetSize(TimeBarW, TimeBarH);
    }

    if (TimeBarFill)
    {
        TimeBarFill->SetPos(TimeBarX, TimeBarY);
        TimeBarFill->SetSize(
            TimeBarW * mMonthProgress,
            TimeBarH);
    }

    if (DateText)
    {
        DateText->SetPos(
            TopHudPanelX + 62.f * TopHudScale,
            TopHudPanelY + 24.f * TopHudScale);
        DateText->SetSize(170.f * TopHudScale, 20.f * TopHudScale);
    }

    const float SpeedButtonSize = 40.f * TopHudScale;
    const float SpeedButtonGap = 44.f * TopHudScale;
    const float SpeedButtonY =
        TopHudPanelY + TopHudPanelH - SpeedButtonSize - 14.f * TopHudScale;
    const float SpeedButtonStartX = TopHudPanelX + 18.f * TopHudScale;

    for (int i = 0; i < static_cast<int>(mSpeedButtons.size()); ++i)
    {
        auto SpeedButton = mSpeedButtons[i].lock();

        if (!SpeedButton)
            continue;

        SpeedButton->SetPos(
            SpeedButtonStartX + SpeedButtonGap * static_cast<float>(i),
            SpeedButtonY);
        SpeedButton->SetSize(SpeedButtonSize, SpeedButtonSize);
    }

    float MenuButtonSize = 48.f * TopHudScale;
    float MenuButtonGap = 6.f * TopHudScale;
    const float MenuStartX =
        TopHudPanelX + TopHudPanelW + 24.f * TopHudScale;
    const float MenuY = TopHudPanelY + 4.f * TopHudScale;

    const float WantedMenuWidth =
        MenuButtonSize * GMenuButtonCount +
        MenuButtonGap * static_cast<float>(GMenuButtonCount - 1);
    const float MaxMenuWidth =
        (std::max)(80.f, ScreenWidth - MenuStartX - 14.f);

    if (WantedMenuWidth > MaxMenuWidth)
    {
        const float MenuScale =
            (std::max)(0.62f, MaxMenuWidth / WantedMenuWidth);
        MenuButtonSize *= MenuScale;
        MenuButtonGap *= MenuScale;
    }

    for (int i = 0; i < static_cast<int>(mMenuButtons.size()); ++i)
    {
        auto MenuButton = mMenuButtons[i].lock();

        if (!MenuButton)
            continue;

        MenuButton->SetPos(
            MenuStartX + (MenuButtonSize + MenuButtonGap) * static_cast<float>(i),
            MenuY);
        MenuButton->SetSize(MenuButtonSize, MenuButtonSize);
    }
}

void CTopHudWidget::OnConstructionButtonClick()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto BuildMenu =
        UIManager->FindWidget<CBuildMenuWidget>(GBuildMenuWidgetName).lock();

    if (BuildMenu)
        BuildMenu->ToggleBuildMenu();
}

void CTopHudWidget::OnAlmanacButtonClick()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto BuildMenu =
        UIManager->FindWidget<CBuildMenuWidget>(GBuildMenuWidgetName).lock();

    if (BuildMenu)
        BuildMenu->ToggleAlmanac();
}

void CTopHudWidget::OnAnyButtonClick()
{
}
