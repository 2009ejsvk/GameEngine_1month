#include "TopHudWidget.h"
#include "AlmanacWidget.h"
#include "BuildMenuWidget.h"
#include "EdictWidget.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../World/MainWorldAccess.h"
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

    std::wstring FormatDate(int Year, int Month, int Day)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%04d.%02d.%02d", Year, Month, Day);
        return Buffer;
    }

    std::wstring FormatHudDate(int Year, int Month, int Day)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%04d년 %d월 %d일", Year, Month, Day);
        return Buffer;
    }

    std::wstring FormatTaxPolicyCompact(const FTaxPolicy& TaxPolicy)
    {
        return
            L"세금 " +
            std::to_wstring(TaxPolicy.ConsumptionRatePercent) +
            L"/" +
            std::to_wstring(TaxPolicy.IncomeRatePercent) +
            L"/" +
            std::to_wstring(TaxPolicy.PropertyRatePercent) +
            L"%";
    }

    const wchar_t* GetElectionWarningTierLabel(double Score)
    {
        if (Score >= 0.78)
            return L"재선 위험 높음";
        if (Score >= 0.52)
            return L"재선 주의";
        if (Score >= 0.32)
            return L"선거 점검";
        return L"안정";
    }

    bool HasElectionWarning(int DaysUntilElection, double Score)
    {
        return DaysUntilElection >= 0 &&
            DaysUntilElection <= 180 &&
            Score >= 0.32;
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
        DateText->SetText(TEXT("1959년 2월 1일"));
        DateText->SetFontSize(20.f);
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
        BudgetText->SetFontSize(20.f);
        BudgetText->SetAlignH(ETextAlignH::Left);
        BudgetText->SetAlignV(ETextAlignV::Middle);
        BudgetText->SetTextColor(245, 235, 210, 255);
        BudgetText->EnableShadow(true);
        BudgetText->SetShadowOffset(1.f, 1.f);
        BudgetText->SetShadowTextColor(16, 16, 16, 220);
        mBudgetText = BudgetText;
    }

    auto BudgetLabelText =
        CreateWidget<CTextBlock>("TopHud_BudgetLabelText", 20).lock();

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
        mBudgetLabelText = BudgetLabelText;
    }

    auto ElectionText =
        CreateWidget<CTextBlock>("TopHud_ElectionText", 17).lock();

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
        mElectionText = ElectionText;
    }

    auto TaxPolicyText =
        CreateWidget<CTextBlock>("TopHud_TaxPolicyText", 17).lock();

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
        mTaxPolicyText = TaxPolicyText;
    }

    auto EventText =
        CreateWidget<CTextBlock>("TopHud_EventText", 17).lock();

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
        mEventText = EventText;
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
        NpcText->SetFontSize(20.f);
        NpcText->SetAlignH(ETextAlignH::Left);
        NpcText->SetAlignV(ETextAlignV::Middle);
        NpcText->SetTextColor(245, 235, 210, 255);
        NpcText->EnableShadow(true);
        NpcText->SetShadowOffset(1.f, 1.f);
        NpcText->SetShadowTextColor(16, 16, 16, 220);
        mNpcText = NpcText;
    }

    auto NpcLabelText =
        CreateWidget<CTextBlock>("TopHud_StatusNpcLabelText", 20).lock();

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
        mNpcLabelText = NpcLabelText;
    }

    auto SupportText =
        CreateWidget<CTextBlock>("TopHud_StatusSupportText", 20).lock();

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
        mSupportText = SupportText;
    }

    auto SupportLabelText =
        CreateWidget<CTextBlock>("TopHud_StatusSupportLabelText", 20).lock();

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
        mSupportLabelText = SupportLabelText;
    }

    auto GameOverDim =
        CreateWidget<CImage>("TopHud_GameOverDim", 90).lock();

    if (GameOverDim)
    {
        GameOverDim->SetTexture("TopHudGameOverDim", GStatusBarTexture);
        GameOverDim->SetTint(0.03f, 0.03f, 0.03f, 0.72f);
        GameOverDim->SetEnable(false);
        mGameOverDim = GameOverDim;
    }

    auto GameOverPanel =
        CreateWidget<CImage>("TopHud_GameOverPanel", 91).lock();

    if (GameOverPanel)
    {
        GameOverPanel->SetTexture("TopHudGameOverPanel", GCenterPopupTexture);
        GameOverPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        GameOverPanel->SetEnable(false);
        mGameOverPanel = GameOverPanel;
    }

    auto GameOverTitleText =
        CreateWidget<CTextBlock>("TopHud_GameOverTitleText", 92).lock();

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
        mGameOverTitleText = GameOverTitleText;
    }

    auto GameOverBodyText =
        CreateWidget<CTextBlock>("TopHud_GameOverBodyText", 92).lock();

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
        mGameOverBodyText = GameOverBodyText;
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
    mMenuButtonTexts.resize(GMenuButtonCount);

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
        else if (i == GMenuEdictsIndex)
            MenuCallback = &CTopHudWidget::OnEdictsButtonClick;
        else if (i == GMenuAlmanacIndex)
            MenuCallback = &CTopHudWidget::OnAlmanacButtonClick;

        Button->SetEventCallback<CTopHudWidget>(
            EButtonEventState::Click, this, MenuCallback);

        ApplyTextureToAllButtonStates(
            Button,
            "TopHudMenuIcon_" + std::to_string(i),
            GMenuIcons[i]);

        mMenuButtons[i] = Button;

        auto MenuText = CreateWidget<CTextBlock>(
            "TopHud_MenuText_" + std::to_string(i + 1), 18).lock();

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
        mMenuButtonTexts[i] = MenuText;
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

    auto MainWorld = std::dynamic_pointer_cast<IMainWorldAccess>(World);

    if (!MainWorld)
        return;

    const FElectionStatus& ElectionStatus =
        MainWorld->GetElectionStatus();
    const FTaxPolicyEventStatus& TaxEventStatus =
        MainWorld->GetTaxPolicyEventStatus();
    const int DaysUntilElection = MainWorld->GetDaysUntilNextElection();
    const double ElectionWarningScore = MainWorld->GetElectionWarningScore();
    const bool ElectionWarningActive =
        HasElectionWarning(DaysUntilElection, ElectionWarningScore);
    auto BudgetText = mBudgetText.lock();
    auto DateText = mDateText.lock();
    auto ElectionText = mElectionText.lock();
    auto TaxPolicyText = mTaxPolicyText.lock();
    auto EventText = mEventText.lock();
    auto NpcText = mNpcText.lock();
    auto SupportText = mSupportText.lock();
    auto GameOverDim = mGameOverDim.lock();
    auto GameOverPanel = mGameOverPanel.lock();
    auto GameOverTitleText = mGameOverTitleText.lock();
    auto GameOverBodyText = mGameOverBodyText.lock();

    if (BudgetText)
    {
        BudgetText->SetText(
            FormatCurrency(MainWorld->GetNationalBudget()).c_str());
    }

    const FPoliticalWorldSnapshot& PoliticalSnapshot =
        MainWorld->GetPoliticalSnapshot();
    const int ActiveNpcCount = PoliticalSnapshot.ActiveCitizenCount;

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
                static_cast<double>(PoliticalSnapshot.IncumbentCount) /
                static_cast<double>(ActiveNpcCount) * 100.0));
        }

        SupportPercent = (std::max)(0, (std::min)(100, SupportPercent));

        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%d%%", SupportPercent);
        SupportText->SetText(Buffer);
    }

    const int Day = MainWorld->GetSimulationDay();
    const int Month = MainWorld->GetSimulationMonth();
    const int Year = MainWorld->GetSimulationYear();

    if (DateText)
    {
        DateText->SetText(FormatHudDate(Year, Month, Day).c_str());
    }

    if (ElectionText)
    {
        std::wstring ElectionLabel;

        if (ElectionStatus.GameLost)
        {
            ElectionLabel = L"선거 패배";
            ElectionText->SetTextColor(232, 86, 72, 255);
        }
        else
        {
            ElectionLabel =
                L"차기 선거 " +
                FormatDate(
                    ElectionStatus.NextElectionYear,
                    ElectionStatus.NextElectionMonth,
                    ElectionStatus.NextElectionDay);

            if (DaysUntilElection >= 0)
            {
                ElectionLabel += L" | ";
                ElectionLabel += std::to_wstring(DaysUntilElection);
                ElectionLabel += L"일";
            }

            if (ElectionWarningActive)
            {
                ElectionLabel += L" | ";
                ElectionLabel +=
                    std::wstring(
                        GetElectionWarningTierLabel(ElectionWarningScore));
            }
            else if (ElectionStatus.HasRecordedElection)
            {
                ElectionLabel += ElectionStatus.IncumbentWonLastElection ?
                    L" | 직전 승리" :
                    L" | 직전 패배";
            }

            if (ElectionWarningScore >= 0.78)
                ElectionText->SetTextColor(232, 86, 72, 255);
            else if (ElectionWarningScore >= 0.52)
                ElectionText->SetTextColor(238, 178, 88, 255);
            else if (ElectionWarningActive)
                ElectionText->SetTextColor(240, 214, 124, 255);
            else
                ElectionText->SetTextColor(245, 235, 210, 255);
        }

        ElectionText->SetText(ElectionLabel.c_str());
    }

    if (TaxPolicyText)
    {
        TaxPolicyText->SetText(
            FormatTaxPolicyCompact(MainWorld->GetTaxPolicy()).c_str());
    }

    if (EventText)
    {
        std::wstring EventLabel = L"현재 상태 안정";

        if (TaxEventStatus.Active)
        {
            EventLabel = std::wstring(L"경고 ") + TaxEventStatus.Title;

            if (ElectionWarningActive)
            {
                EventLabel += L" | ";
                EventLabel += GetElectionWarningTierLabel(ElectionWarningScore);
            }
            else
            {
                EventLabel += L" | ";
                EventLabel +=
                    std::to_wstring((std::max)(0, TaxEventStatus.RemainingDays));
                EventLabel += L"일";
            }

            if (ElectionWarningScore >= 0.78 ||
                TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis ||
                TaxEventStatus.DaysActive >= 4)
            {
                EventText->SetTextColor(238, 108, 90, 255);
            }
            else if (ElectionWarningActive)
            {
                EventText->SetTextColor(238, 178, 88, 255);
            }
            else
            {
                EventText->SetTextColor(236, 182, 94, 255);
            }
        }
        else if (ElectionWarningActive)
        {
            EventLabel = L"선거 경고 | 지지 기반 흔들림";

            if (DaysUntilElection >= 0)
            {
                EventLabel += L" | ";
                EventLabel += std::to_wstring(DaysUntilElection);
                EventLabel += L"일";
            }

            if (ElectionWarningScore >= 0.78)
                EventText->SetTextColor(238, 108, 90, 255);
            else if (ElectionWarningScore >= 0.52)
                EventText->SetTextColor(238, 178, 88, 255);
            else
                EventText->SetTextColor(240, 214, 124, 255);
        }
        else if (TaxEventStatus.NotificationDays > 0 &&
            !TaxEventStatus.Summary.empty())
        {
            EventLabel = std::wstring(L"최근 경고 | ") +
                TaxEventStatus.Summary;
            EventText->SetTextColor(228, 214, 188, 255);
        }
        else
        {
            EventText->SetTextColor(208, 226, 198, 255);
        }

        EventText->SetText(EventLabel.c_str());
    }

    if (GameOverTitleText)
        GameOverTitleText->SetText(TEXT("정권 상실"));

    if (ElectionStatus.GameLost && !mGameOverMenusClosed)
    {
        CloseMenus(true, true, true);
        mGameOverMenusClosed = true;
    }
    else if (!ElectionStatus.GameLost)
    {
        mGameOverMenusClosed = false;
    }

    for (size_t i = 0; i < mSpeedButtons.size(); ++i)
    {
        auto Button = mSpeedButtons[i].lock();

        if (Button)
            Button->ButtonEnable(!ElectionStatus.GameLost);
    }

    for (size_t i = 0; i < mMenuButtons.size(); ++i)
    {
        auto Button = mMenuButtons[i].lock();

        if (Button)
            Button->ButtonEnable(!ElectionStatus.GameLost);
    }

    if (GameOverDim)
        GameOverDim->SetEnable(ElectionStatus.GameLost);

    if (GameOverPanel)
        GameOverPanel->SetEnable(ElectionStatus.GameLost);

    if (GameOverTitleText)
        GameOverTitleText->SetEnable(ElectionStatus.GameLost);

    if (GameOverBodyText)
    {
        GameOverBodyText->SetEnable(ElectionStatus.GameLost);

        if (ElectionStatus.GameLost)
        {
            wchar_t Buffer[512] = {};
            swprintf_s(
                Buffer,
                L"%04d.%02d.%02d 선거에서 재집권에 실패했습니다.\n"
                L"지지 %d / 야당 %d / 기권 %d\n"
                L"득표율 %.1f%% / 투표율 %.1f%%\n"
                L"시뮬레이션이 정지되었습니다.",
                ElectionStatus.LastElectionYear,
                ElectionStatus.LastElectionMonth,
                ElectionStatus.LastElectionDay,
                ElectionStatus.LastIncumbentVotes,
                ElectionStatus.LastOppositionVotes,
                ElectionStatus.LastAbstainVotes,
                ElectionStatus.LastVoteShare,
                ElectionStatus.LastTurnoutPercent);
            GameOverBodyText->SetText(Buffer);
        }
    }

    const float MonthProgress = MainWorld->GetSimulationMonthProgress();
    mMonthProgress = (std::max)(0.f, (std::min)(1.f, MonthProgress));
}

void CTopHudWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);

    auto SpeedPanel = mSpeedPanel.lock();
    auto TimeBarBack = mTimeBarBack.lock();
    auto TimeBarFill = mTimeBarFill.lock();
    auto DateText = mDateText.lock();
    auto BudgetText = mBudgetText.lock();
    auto BudgetLabelText = mBudgetLabelText.lock();
    auto StatusBar = mStatusBar.lock();
    auto StatusMoneyIcon = mStatusMoneyIcon.lock();
    auto StatusNpcIcon = mStatusNpcIcon.lock();
    auto StatusSupportIcon = mStatusSupportIcon.lock();
    auto ElectionText = mElectionText.lock();
    auto TaxPolicyText = mTaxPolicyText.lock();
    auto EventText = mEventText.lock();
    auto NpcLabelText = mNpcLabelText.lock();
    auto NpcText = mNpcText.lock();
    auto SupportLabelText = mSupportLabelText.lock();
    auto SupportText = mSupportText.lock();
    auto GameOverDim = mGameOverDim.lock();
    auto GameOverPanel = mGameOverPanel.lock();
    auto GameOverTitleText = mGameOverTitleText.lock();
    auto GameOverBodyText = mGameOverBodyText.lock();
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
        TimeBarFill->SetSize(
            TimeBarW * mMonthProgress,
            TimeBarH);
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

    for (int i = 0; i < static_cast<int>(mSpeedButtons.size()); ++i)
    {
        auto SpeedButton = mSpeedButtons[i].lock();

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

    for (int i = 0; i < static_cast<int>(mMenuButtons.size()); ++i)
    {
        auto MenuButton = mMenuButtons[i].lock();
        auto MenuText = mMenuButtonTexts[i].lock();
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

void CTopHudWidget::CloseMenus(
    bool CloseBuildMenu,
    bool CloseAlmanac,
    bool CloseEdicts)
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto BuildMenu =
        UIManager->FindWidget<CBuildMenuWidget>(GBuildMenuWidgetName).lock();
    auto AlmanacWidget =
        UIManager->FindWidget<CAlmanacWidget>(GAlmanacWidgetName).lock();
    auto EdictWidget =
        UIManager->FindWidget<CEdictWidget>(GEdictWidgetName).lock();

    if (BuildMenu)
    {
        if (CloseBuildMenu)
            BuildMenu->SetBuildMenuOpen(false);

        if (CloseAlmanac)
            BuildMenu->SetAlmanacOpen(false);
    }

    if (AlmanacWidget && CloseAlmanac)
        AlmanacWidget->SetOpen(false);

    if (EdictWidget && CloseEdicts)
        EdictWidget->SetOpen(false);
}

void CTopHudWidget::OnConstructionButtonClick()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto MainWorld = std::dynamic_pointer_cast<IMainWorldAccess>(World);

    if (MainWorld && MainWorld->GetElectionStatus().GameLost)
        return;

    CloseMenus(false, true, true);

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

    auto MainWorld = std::dynamic_pointer_cast<IMainWorldAccess>(World);

    if (MainWorld && MainWorld->GetElectionStatus().GameLost)
        return;

    CloseMenus(true, false, true);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto AlmanacWidget =
        UIManager->FindWidget<CAlmanacWidget>(GAlmanacWidgetName).lock();

    if (AlmanacWidget)
        AlmanacWidget->ToggleOpen();
}

void CTopHudWidget::OnEdictsButtonClick()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto MainWorld = std::dynamic_pointer_cast<IMainWorldAccess>(World);

    if (MainWorld && MainWorld->GetElectionStatus().GameLost)
        return;

    CloseMenus(true, true, false);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto EdictWidget =
        UIManager->FindWidget<CEdictWidget>(GEdictWidgetName).lock();

    if (EdictWidget)
        EdictWidget->ToggleOpen();
}

void CTopHudWidget::OnAnyButtonClick()
{
}

