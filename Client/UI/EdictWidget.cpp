#include "EdictWidget.h"
#include "../Politics/EdictSystem.h"
#include "../World/MainWorld.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/Input.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    constexpr int GEdictCategoryCount = 4;
    constexpr int GEdictSlotsPerPage = 12;
    constexpr int GEdictSlotColumnCount = 6;
    constexpr int GEdictSlotRowCount = 2;
    constexpr int GTaxPolicyRowCount = 3;

    constexpr const TCHAR* GEdictMenuPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\5_MainMenu\\CenterPopUp\\T_center_popUp.png");
    constexpr const TCHAR* GMenuTitleRibbonTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\CenterPopUp\\T_center_popUp_title.png");
    constexpr const TCHAR* GMenuGridFrameTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\CenterPopUp\\T_center_popUp_frameContent.png");
    constexpr const TCHAR* GMenuDetailFrameTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\CenterPopUp\\T_center_popUp_frameDescription.png");
    constexpr const TCHAR* GDetailInfoPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\4_Modern\\SmallPopup\\T_small_popUp.png");
    constexpr const TCHAR* GCategoryTabTextureSelected = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\Tabs\\T_bookmark_standard.png");
    constexpr const TCHAR* GCategoryTabTextureHidden = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\Tabs\\T_bookmark_hidden.png");
    constexpr const TCHAR* GSlotCardTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg.png");
    constexpr const TCHAR* GSlotCardHoverTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg_hover.png");
    constexpr const TCHAR* GSlotCardSelectedTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg_selected.png");
    constexpr const TCHAR* GSlotCardDisabledTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg_disabled.png");
    constexpr const TCHAR* GBigTextButtonTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_standard.png");
    constexpr const TCHAR* GBigTextButtonHoverTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_hover.png");
    constexpr const TCHAR* GBigTextButtonSelectedTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_selected.png");
    constexpr const TCHAR* GBigTextButtonDisabledTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_deactivated.png");
    constexpr const TCHAR* GRoundButtonTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\RoundButton\\T_round_button_standard.png");
    constexpr const TCHAR* GRoundButtonHoverTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\RoundButton\\T_round_button_hover.png");
    constexpr const TCHAR* GRoundButtonSelectedTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\RoundButton\\T_round_button_selected.png");
    constexpr const TCHAR* GScrollTrackTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\Sliderthumb\\T_scrollbar_bg.png");
    constexpr const TCHAR* GScrollThumbTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\Sliderthumb\\T_scrollbar.png");
    constexpr const TCHAR* GDropdownArrowTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_dropDownArrow.png");
    constexpr const TCHAR* GDropdownArrowHoverTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_dropDownArrow_hover.png");
    constexpr const TCHAR* GCostIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png");
    constexpr const TCHAR* GStarEmptyTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_edicts_star_empty.png");
    constexpr const TCHAR* GStarFullTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_edicts_star_full.png");
    constexpr const TCHAR* GEdictActiveCheckTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_research_popUp_check.png");

    const wchar_t* GCategoryLabels[GEdictCategoryCount] =
    {
        L"일반 칙령",
        L"내정 칙령",
        L"국방 칙령",
        L"교육 칙령"
    };

    const TCHAR* const GCategoryTabIcons[GEdictCategoryCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\EdictCategories\\T_ICO_general.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\EdictCategories\\T_ICO_interior.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\EdictCategories\\T_ICO_defense.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\EdictCategories\\T_ICO_education.png")
    };

    const ETaxPolicyType GTaxPolicyTypes[GTaxPolicyRowCount] =
    {
        ETaxPolicyType::Consumption,
        ETaxPolicyType::Income,
        ETaxPolicyType::Property
    };

    struct FEdictAvailabilityInfo
    {
        bool Active = false;
        bool CoolingDown = false;
        bool CanApply = false;
        long long ActivationCost = 0;
        std::wstring StatusText;
        std::wstring RequirementText;
    };

    ETaxPolicyEventType ResolveRequiredTaxPolicyEvent(
        EGovernmentEdictType Type);
    const wchar_t* GetTaxPolicyEventDisplayName(
        ETaxPolicyEventType Type);

    void ConfigureDefaultButtonStyle(
        const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(0.20f, 0.22f, 0.26f, 0.92f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.26f, 0.30f, 0.35f, 0.95f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.14f, 0.16f, 0.20f, 0.98f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.10f, 0.10f, 0.12f, 0.70f));
    }

    void ConfigureHighlightedButtonStyle(
        const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(0.10f, 0.32f, 0.52f, 0.95f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.16f, 0.40f, 0.62f, 0.98f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.08f, 0.24f, 0.40f, 0.98f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.08f, 0.24f, 0.40f, 0.70f));
    }

    void ConfigureIconSlotButtonStyle(
        const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(1.f, 1.f, 1.f, 0.96f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(1.f, 1.f, 1.f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.80f, 0.80f, 0.80f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.35f, 0.35f, 0.35f, 0.75f));
    }

    void ConfigureCategoryTabButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected)
    {
        if (!Button)
            return;

        if (Selected)
        {
            Button->SetTint(EButtonState::Normal,
                FVector4(1.f, 1.f, 1.f, 1.f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(1.f, 1.f, 1.f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.90f, 0.90f, 0.90f, 1.f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.50f, 0.50f, 0.50f, 0.75f));
            return;
        }

        Button->SetTint(EButtonState::Normal,
            FVector4(0.74f, 0.84f, 0.98f, 0.96f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.88f, 0.95f, 1.f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.62f, 0.74f, 0.92f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.5f, 0.5f, 0.5f, 0.7f));
    }

    void ApplyButtonTextureSet(
        const std::shared_ptr<CButton>& Button,
        const std::string& TextureKeyBase,
        const TCHAR* NormalTexture,
        const TCHAR* HoverTexture = nullptr,
        const TCHAR* ClickTexture = nullptr,
        const TCHAR* DisableTexture = nullptr)
    {
        if (!Button || !NormalTexture)
            return;

        const TCHAR* ResolvedHover = HoverTexture ? HoverTexture : NormalTexture;
        const TCHAR* ResolvedClick = ClickTexture ? ClickTexture : ResolvedHover;
        const TCHAR* ResolvedDisable = DisableTexture ? DisableTexture : NormalTexture;

        if (!Button->SetTexture(
            EButtonState::Normal,
            TextureKeyBase + "_normal",
            NormalTexture))
        {
            return;
        }

        Button->SetTexture(
            EButtonState::Hovered,
            TextureKeyBase + "_hover",
            ResolvedHover);
        Button->SetTexture(
            EButtonState::Click,
            TextureKeyBase + "_click",
            ResolvedClick);
        Button->SetTexture(
            EButtonState::Disable,
            TextureKeyBase + "_disable",
            ResolvedDisable);
    }

    void ConfigureEdictSlotButtonVisual(
        const std::shared_ptr<CButton>& Button,
        const std::string& TextureKeyBase,
        bool Focused,
        bool Active,
        bool CoolingDown,
        bool Available)
    {
        if (!Button)
            return;

        const TCHAR* NormalTexture = GSlotCardTexture;
        const TCHAR* HoverTexture = GSlotCardHoverTexture;
        const TCHAR* ClickTexture = GSlotCardSelectedTexture;
        const TCHAR* DisableTexture = GSlotCardDisabledTexture;
        FVector4 Tint = FVector4(1.f, 1.f, 1.f, 1.f);

        if (Active)
        {
            NormalTexture = GSlotCardSelectedTexture;
            HoverTexture = GSlotCardSelectedTexture;
            ClickTexture = GSlotCardSelectedTexture;
            Tint = FVector4(1.15f, 0.90f, 0.20f, 1.f);
        }
        else if (CoolingDown)
        {
            NormalTexture = GSlotCardDisabledTexture;
            HoverTexture = GSlotCardDisabledTexture;
            ClickTexture = GSlotCardDisabledTexture;
            Tint = FVector4(0.86f, 0.88f, 0.92f, 1.f);
        }
        else if (!Available)
        {
            NormalTexture = GSlotCardDisabledTexture;
            HoverTexture = GSlotCardDisabledTexture;
            ClickTexture = GSlotCardDisabledTexture;
            Tint = FVector4(0.90f, 0.90f, 0.90f, 1.f);
        }
        else if (Focused)
        {
            NormalTexture = GSlotCardSelectedTexture;
            HoverTexture = GSlotCardSelectedTexture;
            ClickTexture = GSlotCardSelectedTexture;
            Tint = FVector4(1.03f, 0.98f, 0.84f, 1.f);
        }

        ApplyButtonTextureSet(
            Button,
            TextureKeyBase,
            NormalTexture,
            HoverTexture,
            ClickTexture,
            DisableTexture);
        ConfigureIconSlotButtonStyle(Button);
        Button->SetTint(EButtonState::Normal, Tint);
        Button->SetTint(EButtonState::Hovered, Tint);
        Button->SetTint(EButtonState::Click, Tint);
        Button->SetTint(EButtonState::Disable,
            FVector4(0.60f, 0.60f, 0.60f, 0.75f));
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

    std::wstring FormatPercentValue(int Value)
    {
        return std::to_wstring(Value) + L"%";
    }

    int FormatDaysToMonths(int Days)
    {
        return (std::max)(1, (Days + 29) / 30);
    }

    EEdictUiCategory ResolveEdictUiCategory(EGovernmentEdictType Type)
    {
        switch (Type)
        {
        case EGovernmentEdictType::FoodForThePeople:
        case EGovernmentEdictType::TaxCut:
        case EGovernmentEdictType::LaborTaxRelief:
        case EGovernmentEdictType::EmergencyAusterity:
            return EEdictUiCategory::General;
        case EGovernmentEdictType::FreeHousing:
        case EGovernmentEdictType::PropertyTaxRelief:
            return EEdictUiCategory::Interior;
        case EGovernmentEdictType::MartialLaw:
            return EEdictUiCategory::Defense;
        case EGovernmentEdictType::EmployeeOfTheMonth:
            return EEdictUiCategory::Education;
        default:
            return EEdictUiCategory::General;
        }
    }

    const wchar_t* GetCategoryLabel(EEdictUiCategory Category)
    {
        const int Index = static_cast<int>(Category);

        if (Index < 0 || Index >= GEdictCategoryCount)
            return L"칙령";

        return GCategoryLabels[Index];
    }

    FEdictAvailabilityInfo EvaluateEdictAvailability(
        const FGovernmentEdictDefinition& Definition,
        const FGovernmentEdictState* State,
        const CMainWorld* MainWorld)
    {
        FEdictAvailabilityInfo Info;
        Info.StatusText = L"정보 없음";
        Info.RequirementText = L"월드 정보를 찾을 수 없습니다.";
        Info.ActivationCost = Definition.BaseCost;

        if (!State || !MainWorld)
            return Info;

        const int ActiveCitizenCount = (std::max)(
            0, MainWorld->GetPoliticalSnapshot().ActiveCitizenCount);
        Info.ActivationCost = EdictSystem::ResolveEdictActivationCost(
            Definition, ActiveCitizenCount);
        Info.Active = State->Active;
        Info.CoolingDown =
            !State->Active &&
            Definition.Mode == EGovernmentEdictMode::Active &&
            State->CooldownDays > 0;

        if (Definition.Mode == EGovernmentEdictMode::Passive)
        {
            if (State->Active)
            {
                Info.CanApply = true;
                Info.StatusText = L"활성";
                return Info;
            }

            Info.CanApply = true;
            Info.StatusText = L"사용 가능";
        }
        else
        {
            if (State->Active)
            {
                Info.StatusText =
                    L"시행 중 (" +
                    std::to_wstring((std::max)(0, State->RemainingDays)) +
                    L"일 남음)";
                return Info;
            }

            if (State->CooldownDays > 0)
            {
                Info.StatusText =
                    L"재사용 대기 (" +
                    std::to_wstring(State->CooldownDays) +
                    L"일)";
                Info.RequirementText = Info.StatusText;
                return Info;
            }

            Info.CanApply = true;
            Info.StatusText = L"시행 가능";
        }

        const ETaxPolicyEventType RequiredTaxEvent =
            ResolveRequiredTaxPolicyEvent(Definition.Type);

        if (RequiredTaxEvent != ETaxPolicyEventType::None)
        {
            const FTaxPolicyEventStatus& TaxEventStatus =
                MainWorld->GetTaxPolicyEventStatus();

            if (!TaxEventStatus.Active ||
                TaxEventStatus.Type != RequiredTaxEvent)
            {
                Info.CanApply = false;
                Info.StatusText = L"조건 미충족";
                Info.RequirementText =
                    std::wstring(L"대응 사건 필요: ") +
                    GetTaxPolicyEventDisplayName(RequiredTaxEvent);
                return Info;
            }
        }

        if (!State->Active &&
            Info.ActivationCost > MainWorld->GetNationalBudget())
        {
            Info.CanApply = false;
            Info.StatusText = L"예산 부족";
            Info.RequirementText = L"예산 부족";
            return Info;
        }

        return Info;
    }

    ETaxPolicyEventType ResolveRequiredTaxPolicyEvent(EGovernmentEdictType Type)
    {
        switch (Type)
        {
        case EGovernmentEdictType::LaborTaxRelief:
            return ETaxPolicyEventType::WorkerTaxStrike;
        case EGovernmentEdictType::PropertyTaxRelief:
            return ETaxPolicyEventType::PropertyTaxBacklash;
        case EGovernmentEdictType::EmergencyAusterity:
            return ETaxPolicyEventType::BudgetCrisis;
        default:
            return ETaxPolicyEventType::None;
        }
    }

    const wchar_t* GetTaxPolicyEventDisplayName(ETaxPolicyEventType Type)
    {
        switch (Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            return L"근로층 세금 파업";
        case ETaxPolicyEventType::PropertyTaxBacklash:
            return L"재산세 반발";
        case ETaxPolicyEventType::BudgetCrisis:
            return L"국고 위기";
        default:
            return L"세금 사건";
        }
    }
}

CEdictWidget::CEdictWidget()
{
}

CEdictWidget::~CEdictWidget()
{
}

bool CEdictWidget::Init()
{
    CWidgetContainer::Init();

    mVisibleEntryIndices.assign(GEdictSlotsPerPage, -1);
    mPanelWidth = 1100.f;
    mPanelHeight = 760.f;

    auto MenuBackground = CreateWidget<CImage>("EdictMenu_Background", 6).lock();

    if (MenuBackground)
    {
        MenuBackground->SetTexture("EdictMenuBackground", GEdictMenuPanelTexture);
        MenuBackground->SetTint(1.f, 1.f, 1.f, 1.f);
        mMenuBackground = MenuBackground;
    }

    auto MenuTitleRibbon =
        CreateWidget<CImage>("EdictMenu_TitleRibbon", 7).lock();

    if (MenuTitleRibbon)
    {
        MenuTitleRibbon->SetTexture(
            "EdictMenu_TitleRibbonTex",
            GMenuTitleRibbonTexture);
        MenuTitleRibbon->SetTint(1.f, 1.f, 1.f, 1.f);
        mMenuTitleRibbon = MenuTitleRibbon;
    }

    auto MenuGridFrame =
        CreateWidget<CImage>("EdictMenu_GridFrame", 7).lock();

    if (MenuGridFrame)
    {
        MenuGridFrame->SetTexture(
            "EdictMenu_GridFrameTex",
            GMenuGridFrameTexture);
        MenuGridFrame->SetTint(1.f, 1.f, 1.f, 1.f);
        mMenuGridFrame = MenuGridFrame;
    }

    auto MenuDetailFrame =
        CreateWidget<CImage>("EdictMenu_DetailFrame", 7).lock();

    if (MenuDetailFrame)
    {
        MenuDetailFrame->SetTexture(
            "EdictMenu_DetailFrameTex",
            GMenuDetailFrameTexture);
        MenuDetailFrame->SetTint(1.f, 1.f, 1.f, 1.f);
        mMenuDetailFrame = MenuDetailFrame;
    }

    auto DetailInfoPanel =
        CreateWidget<CImage>("EdictMenu_DetailInfoPanel", 7).lock();

    if (DetailInfoPanel)
    {
        DetailInfoPanel->SetTexture(
            "EdictMenu_DetailInfoPanelTex",
            GDetailInfoPanelTexture);
        DetailInfoPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        mDetailInfoPanel = DetailInfoPanel;
    }

    auto TaxInfoPanel =
        CreateWidget<CImage>("EdictMenu_TaxInfoPanel", 7).lock();

    if (TaxInfoPanel)
    {
        TaxInfoPanel->SetTexture(
            "EdictMenu_TaxInfoPanelTex",
            GDetailInfoPanelTexture);
        TaxInfoPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        mTaxInfoPanel = TaxInfoPanel;
    }

    auto ScrollTrack =
        CreateWidget<CImage>("EdictMenu_ScrollTrack", 7).lock();

    if (ScrollTrack)
    {
        ScrollTrack->SetTexture(
            "EdictMenu_ScrollTrackTex",
            GScrollTrackTexture);
        ScrollTrack->SetTint(1.f, 1.f, 1.f, 1.f);
        mScrollTrack = ScrollTrack;
    }

    auto ScrollThumb =
        CreateWidget<CImage>("EdictMenu_ScrollThumb", 8).lock();

    if (ScrollThumb)
    {
        ScrollThumb->SetTexture(
            "EdictMenu_ScrollThumbTex",
            GScrollThumbTexture);
        ScrollThumb->SetTint(1.f, 1.f, 1.f, 1.f);
        mScrollThumb = ScrollThumb;
    }

    auto TitleText = CreateWidget<CTextBlock>("EdictMenu_Title", 7).lock();

    if (TitleText)
    {
        TitleText->SetText(GetCategoryLabel(mSelectedCategory));
        TitleText->SetFontSize(32.f);
        TitleText->SetAlignH(ETextAlignH::Center);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(100, 72, 28, 255);
        TitleText->EnableShadow(true);
        TitleText->SetShadowOffset(1.f, 1.f);
        TitleText->SetShadowTextColor(245, 235, 205, 180);
        mTitleText = TitleText;
    }

    auto PageText = CreateWidget<CTextBlock>("EdictMenu_PageText", 7).lock();

    if (PageText)
    {
        PageText->SetText(TEXT("1 / 1"));
        PageText->SetFontSize(15.f);
        PageText->SetAlignH(ETextAlignH::Center);
        PageText->SetAlignV(ETextAlignV::Middle);
        PageText->SetTextColor(102, 87, 53, 255);
        PageText->EnableShadow(true);
        PageText->SetShadowOffset(1.f, 1.f);
        PageText->SetShadowTextColor(245, 235, 205, 170);
        mPageText = PageText;
    }

    auto CloseButton = CreateWidget<CButton>("EdictMenu_CloseButton", 7).lock();

    if (CloseButton)
    {
        ApplyButtonTextureSet(
            CloseButton,
            "EdictMenu_CloseButton",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        ConfigureIconSlotButtonStyle(CloseButton);
        CloseButton->SetEventCallback<CEdictWidget>(
            EButtonEventState::Click, this,
            &CEdictWidget::OnCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "EdictMenu_CloseText", mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            CloseText->SetFontSize(22.f);
            CloseText->SetAlignH(ETextAlignH::Center);
            CloseText->SetAlignV(ETextAlignV::Middle);
            CloseText->SetTextColor(92, 60, 12, 255);
            CloseText->EnableShadow(true);
            CloseText->SetShadowOffset(1.f, 1.f);
            CloseText->SetShadowTextColor(255, 239, 196, 170);
            CloseButton->SetChild(CloseText);
        }

        mCloseButton = CloseButton;
    }

    auto PrevPageButton = CreateWidget<CButton>("EdictMenu_PrevPage", 7).lock();

    if (PrevPageButton)
    {
        ApplyButtonTextureSet(
            PrevPageButton,
            "EdictMenu_PrevPage",
            GDropdownArrowTexture,
            GDropdownArrowHoverTexture,
            GDropdownArrowHoverTexture,
            GDropdownArrowTexture);
        ConfigureIconSlotButtonStyle(PrevPageButton);
        PrevPageButton->SetAngle(180.f);
        PrevPageButton->SetEventCallback<CEdictWidget>(
            EButtonEventState::Click, this,
            &CEdictWidget::OnPrevPageClick);

        mPrevPageButton = PrevPageButton;
    }

    auto NextPageButton = CreateWidget<CButton>("EdictMenu_NextPage", 7).lock();

    if (NextPageButton)
    {
        ApplyButtonTextureSet(
            NextPageButton,
            "EdictMenu_NextPage",
            GDropdownArrowTexture,
            GDropdownArrowHoverTexture,
            GDropdownArrowHoverTexture,
            GDropdownArrowTexture);
        ConfigureIconSlotButtonStyle(NextPageButton);
        NextPageButton->SetEventCallback<CEdictWidget>(
            EButtonEventState::Click, this,
            &CEdictWidget::OnNextPageClick);

        mNextPageButton = NextPageButton;
    }

    mCategoryButtons.resize(GEdictCategoryCount);
    mCategoryButtonIcons.resize(GEdictCategoryCount);
    void (CEdictWidget::* CategoryCallbacks[GEdictCategoryCount])() =
    {
        &CEdictWidget::OnCategoryGeneralClick,
        &CEdictWidget::OnCategoryInteriorClick,
        &CEdictWidget::OnCategoryDefenseClick,
        &CEdictWidget::OnCategoryEducationClick
    };

    for (int i = 0; i < GEdictCategoryCount; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "EdictMenu_Category_" + std::to_string(i + 1), 7).lock();

        if (!Button)
            continue;

        ApplyButtonTextureSet(
            Button,
            "EdictCategoryTab_" + std::to_string(i),
            GCategoryTabTextureHidden,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(Button, false);
        Button->SetEventCallback<CEdictWidget>(
            EButtonEventState::Click, this, CategoryCallbacks[i]);

        auto CategoryIcon = CWidget::CreateStaticWidget<CImage>(
            "EdictMenu_CategoryIcon_" + std::to_string(i + 1), mWorld);

        if (CategoryIcon)
        {
            CategoryIcon->SetTexture(
                "EdictMenuCategoryIconTex_" + std::to_string(i + 1),
                GCategoryTabIcons[i]);
            CategoryIcon->SetTint(1.f, 1.f, 1.f, 1.f);
            Button->SetChild(CategoryIcon);
            mCategoryButtonIcons[i] = CategoryIcon;
        }

        mCategoryButtons[i] = Button;
    }

    mEdictButtons.resize(GEdictSlotsPerPage);
    mEdictButtonIcons.resize(GEdictSlotsPerPage);
    mEdictButtonStarLefts.resize(GEdictSlotsPerPage);
    mEdictButtonStarRights.resize(GEdictSlotsPerPage);
    mEdictButtonChecks.resize(GEdictSlotsPerPage);
    mEdictButtonTexts.resize(GEdictSlotsPerPage);

    void (CEdictWidget::* SlotCallbacks[GEdictSlotsPerPage])() =
    {
        &CEdictWidget::OnSlot0Click,
        &CEdictWidget::OnSlot1Click,
        &CEdictWidget::OnSlot2Click,
        &CEdictWidget::OnSlot3Click,
        &CEdictWidget::OnSlot4Click,
        &CEdictWidget::OnSlot5Click,
        &CEdictWidget::OnSlot6Click,
        &CEdictWidget::OnSlot7Click,
        &CEdictWidget::OnSlot8Click,
        &CEdictWidget::OnSlot9Click,
        &CEdictWidget::OnSlot10Click,
        &CEdictWidget::OnSlot11Click
    };
    void (CEdictWidget::* SlotHoverCallbacks[GEdictSlotsPerPage])() =
    {
        &CEdictWidget::OnSlot0Hovered,
        &CEdictWidget::OnSlot1Hovered,
        &CEdictWidget::OnSlot2Hovered,
        &CEdictWidget::OnSlot3Hovered,
        &CEdictWidget::OnSlot4Hovered,
        &CEdictWidget::OnSlot5Hovered,
        &CEdictWidget::OnSlot6Hovered,
        &CEdictWidget::OnSlot7Hovered,
        &CEdictWidget::OnSlot8Hovered,
        &CEdictWidget::OnSlot9Hovered,
        &CEdictWidget::OnSlot10Hovered,
        &CEdictWidget::OnSlot11Hovered
    };

    for (int i = 0; i < GEdictSlotsPerPage; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "EdictMenu_Slot_" + std::to_string(i + 1), 7).lock();

        if (!Button)
            continue;

        ApplyButtonTextureSet(
            Button,
            "EdictMenu_Slot_" + std::to_string(i),
            GSlotCardTexture,
            GSlotCardHoverTexture,
            GSlotCardSelectedTexture,
            GSlotCardDisabledTexture);
        ConfigureIconSlotButtonStyle(Button);
        Button->SetEventCallback<CEdictWidget>(
            EButtonEventState::Click, this, SlotCallbacks[i]);
        Button->SetEventCallback<CEdictWidget>(
            EButtonEventState::Hovered, this, SlotHoverCallbacks[i]);

        auto SlotContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "EdictMenu_SlotContent_" + std::to_string(i + 1), mWorld);
        auto StarLeft = CWidget::CreateStaticWidget<CImage>(
            "EdictMenu_SlotStarLeft_" + std::to_string(i + 1), mWorld);
        auto StarRight = CWidget::CreateStaticWidget<CImage>(
            "EdictMenu_SlotStarRight_" + std::to_string(i + 1), mWorld);
        auto SlotIcon = CWidget::CreateStaticWidget<CImage>(
            "EdictMenu_SlotIcon_" + std::to_string(i + 1), mWorld);
        auto SlotCheck = CWidget::CreateStaticWidget<CImage>(
            "EdictMenu_SlotCheck_" + std::to_string(i + 1), mWorld);
        auto ButtonText = CWidget::CreateStaticWidget<CTextBlock>(
            "EdictMenu_SlotText_" + std::to_string(i + 1), mWorld);

        if (SlotContent && StarLeft && StarRight &&
            SlotIcon && SlotCheck && ButtonText)
        {
            StarLeft->SetTexture(
                "EdictMenu_SlotStarLeftTex_" + std::to_string(i + 1),
                GStarEmptyTexture);
            StarLeft->SetTint(1.f, 1.f, 1.f, 1.f);
            SlotContent->AddWidget(StarLeft);

            StarRight->SetTexture(
                "EdictMenu_SlotStarRightTex_" + std::to_string(i + 1),
                GStarEmptyTexture);
            StarRight->SetTint(1.f, 1.f, 1.f, 1.f);
            SlotContent->AddWidget(StarRight);

            SlotIcon->SetTint(1.f, 1.f, 1.f, 1.f);
            SlotContent->AddWidget(SlotIcon);

            SlotCheck->SetTexture(
                "EdictMenu_SlotCheckTex_" + std::to_string(i + 1),
                GEdictActiveCheckTexture);
            SlotCheck->SetTint(1.f, 1.f, 1.f, 1.f);
            SlotContent->AddWidget(SlotCheck);

            ButtonText->SetText(TEXT(""));
            ButtonText->SetFontSize(14.f);
            ButtonText->SetAlignH(ETextAlignH::Center);
            ButtonText->SetAlignV(ETextAlignV::Bottom);
            ButtonText->SetTextColor(58, 47, 31, 255);
            ButtonText->EnableShadow(true);
            ButtonText->SetShadowOffset(1.f, 1.f);
            ButtonText->SetShadowTextColor(240, 229, 205, 190);
            SlotContent->AddWidget(ButtonText);

            Button->SetChild(SlotContent);
            mEdictButtonIcons[i] = SlotIcon;
            mEdictButtonStarLefts[i] = StarLeft;
            mEdictButtonStarRights[i] = StarRight;
            mEdictButtonChecks[i] = SlotCheck;
            mEdictButtonTexts[i] = ButtonText;
        }

        mEdictButtons[i] = Button;
    }

    auto DetailTitleText =
        CreateWidget<CTextBlock>("EdictMenu_DetailTitle", 7).lock();

    if (DetailTitleText)
    {
        DetailTitleText->SetText(TEXT("칙령 정보"));
        DetailTitleText->SetFontSize(26.f);
        DetailTitleText->SetAlignH(ETextAlignH::Center);
        DetailTitleText->SetAlignV(ETextAlignV::Middle);
        DetailTitleText->SetTextColor(100, 72, 28, 255);
        DetailTitleText->EnableShadow(true);
        DetailTitleText->SetShadowOffset(1.f, 1.f);
        DetailTitleText->SetShadowTextColor(245, 235, 205, 180);
        mDetailTitleText = DetailTitleText;
    }

    auto DetailCostIcon =
        CreateWidget<CImage>("EdictMenu_DetailCostIcon", 8).lock();

    if (DetailCostIcon)
    {
        DetailCostIcon->SetTexture(
            "EdictMenu_DetailCostIconTex",
            GCostIconTexture);
        DetailCostIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        mDetailCostIcon = DetailCostIcon;
    }

    auto DetailCostText =
        CreateWidget<CTextBlock>("EdictMenu_DetailCostText", 8).lock();

    if (DetailCostText)
    {
        DetailCostText->SetText(TEXT("$0"));
        DetailCostText->SetFontSize(18.f);
        DetailCostText->SetAlignH(ETextAlignH::Left);
        DetailCostText->SetAlignV(ETextAlignV::Middle);
        DetailCostText->SetTextColor(168, 120, 28, 255);
        DetailCostText->EnableShadow(true);
        DetailCostText->SetShadowOffset(1.f, 1.f);
        DetailCostText->SetShadowTextColor(240, 240, 240, 170);
        mDetailCostText = DetailCostText;
    }

    auto DetailInfoText =
        CreateWidget<CTextBlock>("EdictMenu_DetailInfoText", 8).lock();

    if (DetailInfoText)
    {
        DetailInfoText->SetText(TEXT(""));
        DetailInfoText->SetFontSize(14.f);
        DetailInfoText->SetAlignH(ETextAlignH::Left);
        DetailInfoText->SetAlignV(ETextAlignV::Top);
        DetailInfoText->SetTextColor(63, 53, 33, 255);
        DetailInfoText->EnableShadow(true);
        DetailInfoText->SetShadowOffset(1.f, 1.f);
        DetailInfoText->SetShadowTextColor(245, 235, 205, 170);
        mDetailInfoText = DetailInfoText;
    }

    auto FeedbackText =
        CreateWidget<CTextBlock>("EdictMenu_FeedbackText", 8).lock();

    if (FeedbackText)
    {
        FeedbackText->SetText(TEXT(""));
        FeedbackText->SetFontSize(12.f);
        FeedbackText->SetAlignH(ETextAlignH::Left);
        FeedbackText->SetAlignV(ETextAlignV::Top);
        FeedbackText->SetTextColor(170, 118, 27, 255);
        FeedbackText->EnableShadow(true);
        FeedbackText->SetShadowOffset(1.f, 1.f);
        FeedbackText->SetShadowTextColor(245, 235, 205, 160);
        mFeedbackText = FeedbackText;
    }

    auto DetailBodyText =
        CreateWidget<CTextBlock>("EdictMenu_DetailBody", 7).lock();

    if (DetailBodyText)
    {
        DetailBodyText->SetText(TEXT("칙령을 클릭하면 상세 정보가 표시됩니다."));
        DetailBodyText->SetFontSize(15.f);
        DetailBodyText->SetAlignH(ETextAlignH::Left);
        DetailBodyText->SetAlignV(ETextAlignV::Top);
        DetailBodyText->SetTextColor(62, 54, 38, 255);
        DetailBodyText->EnableShadow(true);
        DetailBodyText->SetShadowOffset(1.f, 1.f);
        DetailBodyText->SetShadowTextColor(245, 235, 205, 150);
        mDetailBodyText = DetailBodyText;
    }

    auto RequirementText =
        CreateWidget<CTextBlock>("EdictMenu_RequirementText", 8).lock();

    if (RequirementText)
    {
        RequirementText->SetText(TEXT(""));
        RequirementText->SetFontSize(14.f);
        RequirementText->SetAlignH(ETextAlignH::Center);
        RequirementText->SetAlignV(ETextAlignV::Middle);
        RequirementText->SetTextColor(210, 34, 18, 255);
        RequirementText->EnableShadow(true);
        RequirementText->SetShadowOffset(1.f, 1.f);
        RequirementText->SetShadowTextColor(255, 230, 220, 170);
        mRequirementText = RequirementText;
    }

    auto ApplyButton = CreateWidget<CButton>("EdictMenu_ApplyButton", 7).lock();

    if (ApplyButton)
    {
        ApplyButtonTextureSet(
            ApplyButton,
            "EdictMenu_ApplyButton",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureIconSlotButtonStyle(ApplyButton);
        ApplyButton->SetEventCallback<CEdictWidget>(
            EButtonEventState::Click, this,
            &CEdictWidget::OnApplyButtonClick);

        auto ApplyButtonText = CWidget::CreateStaticWidget<CTextBlock>(
            "EdictMenu_ApplyButtonText", mWorld);

        if (ApplyButtonText)
        {
            ApplyButtonText->SetText(TEXT("시행"));
            ApplyButtonText->SetFontSize(18.f);
            ApplyButtonText->SetAlignH(ETextAlignH::Center);
            ApplyButtonText->SetAlignV(ETextAlignV::Middle);
            ApplyButtonText->SetTextColor(89, 60, 16, 255);
            ApplyButtonText->EnableShadow(true);
            ApplyButtonText->SetShadowOffset(1.f, 1.f);
            ApplyButtonText->SetShadowTextColor(255, 239, 196, 160);
            ApplyButton->SetChild(ApplyButtonText);
            mApplyButtonText = ApplyButtonText;
        }

        mApplyButton = ApplyButton;
    }

    auto TaxPolicyTitleText =
        CreateWidget<CTextBlock>("EdictMenu_TaxPolicyTitle", 7).lock();

    if (TaxPolicyTitleText)
    {
        TaxPolicyTitleText->SetText(TEXT("세금 정책"));
        TaxPolicyTitleText->SetFontSize(16.f);
        TaxPolicyTitleText->SetAlignH(ETextAlignH::Left);
        TaxPolicyTitleText->SetAlignV(ETextAlignV::Middle);
        TaxPolicyTitleText->SetTextColor(73, 60, 35, 255);
        TaxPolicyTitleText->EnableShadow(true);
        TaxPolicyTitleText->SetShadowOffset(1.f, 1.f);
        TaxPolicyTitleText->SetShadowTextColor(245, 235, 205, 170);
        mTaxPolicyTitleText = TaxPolicyTitleText;
    }

    auto TaxPolicySummaryText =
        CreateWidget<CTextBlock>("EdictMenu_TaxPolicySummary", 7).lock();

    if (TaxPolicySummaryText)
    {
        TaxPolicySummaryText->SetText(TEXT("다음 일일 정산부터 반영"));
        TaxPolicySummaryText->SetFontSize(11.f);
        TaxPolicySummaryText->SetAlignH(ETextAlignH::Left);
        TaxPolicySummaryText->SetAlignV(ETextAlignV::Top);
        TaxPolicySummaryText->SetTextColor(94, 81, 54, 255);
        TaxPolicySummaryText->EnableShadow(true);
        TaxPolicySummaryText->SetShadowOffset(1.f, 1.f);
        TaxPolicySummaryText->SetShadowTextColor(245, 235, 205, 150);
        mTaxPolicySummaryText = TaxPolicySummaryText;
    }

    mTaxPolicyRowTexts.resize(GTaxPolicyRowCount);
    mTaxDecreaseButtons.resize(GTaxPolicyRowCount);
    mTaxIncreaseButtons.resize(GTaxPolicyRowCount);

    void (CEdictWidget::* TaxDecreaseCallbacks[GTaxPolicyRowCount])() =
    {
        &CEdictWidget::OnConsumptionTaxDownClick,
        &CEdictWidget::OnIncomeTaxDownClick,
        &CEdictWidget::OnPropertyTaxDownClick
    };
    void (CEdictWidget::* TaxIncreaseCallbacks[GTaxPolicyRowCount])() =
    {
        &CEdictWidget::OnConsumptionTaxUpClick,
        &CEdictWidget::OnIncomeTaxUpClick,
        &CEdictWidget::OnPropertyTaxUpClick
    };

    for (int i = 0; i < GTaxPolicyRowCount; ++i)
    {
        auto RowText = CreateWidget<CTextBlock>(
            "EdictMenu_TaxPolicyRow_" + std::to_string(i + 1), 7).lock();

        if (RowText)
        {
            RowText->SetText(TEXT("-"));
            RowText->SetFontSize(12.f);
            RowText->SetAlignH(ETextAlignH::Left);
            RowText->SetAlignV(ETextAlignV::Middle);
            RowText->SetTextColor(63, 53, 33, 255);
            RowText->EnableShadow(true);
            RowText->SetShadowOffset(1.f, 1.f);
            RowText->SetShadowTextColor(245, 235, 205, 150);
            mTaxPolicyRowTexts[i] = RowText;
        }

        auto DecreaseButton = CreateWidget<CButton>(
            "EdictMenu_TaxDown_" + std::to_string(i + 1), 7).lock();

        if (DecreaseButton)
        {
            ApplyButtonTextureSet(
                DecreaseButton,
                "EdictMenu_TaxDown_" + std::to_string(i),
                GRoundButtonTexture,
                GRoundButtonHoverTexture,
                GRoundButtonSelectedTexture,
                GRoundButtonTexture);
            ConfigureIconSlotButtonStyle(DecreaseButton);
            DecreaseButton->SetEventCallback<CEdictWidget>(
                EButtonEventState::Click, this, TaxDecreaseCallbacks[i]);

            auto DecreaseText = CWidget::CreateStaticWidget<CTextBlock>(
                "EdictMenu_TaxDownText_" + std::to_string(i + 1), mWorld);

            if (DecreaseText)
            {
                DecreaseText->SetText(TEXT("-"));
                DecreaseText->SetFontSize(16.f);
                DecreaseText->SetAlignH(ETextAlignH::Center);
                DecreaseText->SetAlignV(ETextAlignV::Middle);
                DecreaseText->SetTextColor(92, 60, 12, 255);
                DecreaseButton->SetChild(DecreaseText);
            }

            mTaxDecreaseButtons[i] = DecreaseButton;
        }

        auto IncreaseButton = CreateWidget<CButton>(
            "EdictMenu_TaxUp_" + std::to_string(i + 1), 7).lock();

        if (IncreaseButton)
        {
            ApplyButtonTextureSet(
                IncreaseButton,
                "EdictMenu_TaxUp_" + std::to_string(i),
                GRoundButtonTexture,
                GRoundButtonHoverTexture,
                GRoundButtonSelectedTexture,
                GRoundButtonTexture);
            ConfigureIconSlotButtonStyle(IncreaseButton);
            IncreaseButton->SetEventCallback<CEdictWidget>(
                EButtonEventState::Click, this, TaxIncreaseCallbacks[i]);

            auto IncreaseText = CWidget::CreateStaticWidget<CTextBlock>(
                "EdictMenu_TaxUpText_" + std::to_string(i + 1), mWorld);

            if (IncreaseText)
            {
                IncreaseText->SetText(TEXT("+"));
                IncreaseText->SetFontSize(16.f);
                IncreaseText->SetAlignH(ETextAlignH::Center);
                IncreaseText->SetAlignV(ETextAlignV::Middle);
                IncreaseText->SetTextColor(92, 60, 12, 255);
                IncreaseButton->SetChild(IncreaseText);
            }

            mTaxIncreaseButtons[i] = IncreaseButton;
        }
    }

    ApplyOpenState();
    RefreshCategoryButtons();
    RefreshData();
    RefreshLayout();

    return true;
}

void CEdictWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    if (!mOpen)
        return;

    auto World = mWorld.lock();

    if (World)
    {
        auto Input = World->GetInput().lock();

        if (Input)
        {
            const int WheelDelta = Input->GetMouseWheelDelta();

            if (WheelDelta != 0 && IsMouseOverOpenPanel(Input->GetMousePos()))
                MovePage(WheelDelta < 0 ? 1 : -1);
        }
    }

    RefreshLayout();
}

void CEdictWidget::ToggleOpen()
{
    SetOpen(!mOpen);
}

void CEdictWidget::SetOpen(bool Open)
{
    if (mOpen == Open)
        return;

    mOpen = Open;

    if (mOpen)
    {
        mFeedbackMessage.clear();
        mCurrentPage = 0;
        mPreviewEntryIndex = -1;
        mSelectedEntryIndex = -1;
        RefreshData();
    }

    ApplyOpenState();
    RefreshLayout();
}

bool CEdictWidget::IsMouseOverOpenPanel(const FVector2& MousePos) const
{
    if (!mOpen)
        return false;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(480.f, ScreenWidth - 80.f);
    const float AvailableHeight = (std::max)(420.f, ScreenHeight - 100.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(AvailableWidth / mPanelWidth,
                AvailableHeight / mPanelHeight));
    const float PanelWidth = mPanelWidth * Scale;
    const float PanelHeight = mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;

    return MousePos.x >= PanelLeft &&
        MousePos.x <= PanelLeft + PanelWidth &&
        MousePos.y >= PanelTop &&
        MousePos.y <= PanelTop + PanelHeight;
}

void CEdictWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(480.f, ScreenWidth - 80.f);
    const float AvailableHeight = (std::max)(420.f, ScreenHeight - 100.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(AvailableWidth / mPanelWidth,
                AvailableHeight / mPanelHeight));
    const float PanelWidth = mPanelWidth * Scale;
    const float PanelHeight = mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;
    const float HorizontalMargin = 30.f * Scale;
    const float ContentWidth = PanelWidth - HorizontalMargin * 2.f;
    const float HeaderTopPadding = 18.f * Scale;
    const float HeaderHeight = 56.f * Scale;
    const float TitleRibbonWidth = PanelWidth - 140.f * Scale;
    const float TitleRibbonLeft =
        PanelLeft + (PanelWidth - TitleRibbonWidth) * 0.5f;
    const float GridFrameLeft = PanelLeft + HorizontalMargin;
    const float GridFrameTop = PanelTop + HeaderTopPadding + HeaderHeight + 16.f * Scale;
    const float GridFrameWidth = PanelWidth - HorizontalMargin * 2.f;
    const float GridFrameHeight = PanelHeight - 310.f * Scale;
    const float DetailFrameLeft = GridFrameLeft;
    const float DetailFrameTop = GridFrameTop + GridFrameHeight + 22.f * Scale;
    const float DetailFrameWidth = GridFrameWidth;
    const float DetailFrameHeight =
        PanelTop + PanelHeight - DetailFrameTop - 26.f * Scale;

    auto MenuBackground = mMenuBackground.lock();
    auto MenuTitleRibbon = mMenuTitleRibbon.lock();
    auto MenuGridFrame = mMenuGridFrame.lock();
    auto MenuDetailFrame = mMenuDetailFrame.lock();
    auto DetailInfoPanel = mDetailInfoPanel.lock();
    auto TaxInfoPanel = mTaxInfoPanel.lock();
    auto ScrollTrack = mScrollTrack.lock();
    auto ScrollThumb = mScrollThumb.lock();
    auto TitleText = mTitleText.lock();
    auto PageText = mPageText.lock();
    auto CloseButton = mCloseButton.lock();
    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();

    if (MenuBackground)
    {
        MenuBackground->SetPos(PanelLeft, PanelTop);
        MenuBackground->SetSize(PanelWidth, PanelHeight);
    }

    if (MenuTitleRibbon)
    {
        MenuTitleRibbon->SetPos(TitleRibbonLeft, PanelTop + HeaderTopPadding);
        MenuTitleRibbon->SetSize(TitleRibbonWidth, HeaderHeight);
    }

    if (MenuGridFrame)
    {
        MenuGridFrame->SetPos(GridFrameLeft, GridFrameTop);
        MenuGridFrame->SetSize(GridFrameWidth, GridFrameHeight);
    }

    if (MenuDetailFrame)
    {
        MenuDetailFrame->SetPos(DetailFrameLeft, DetailFrameTop);
        MenuDetailFrame->SetSize(DetailFrameWidth, DetailFrameHeight);
    }

    if (TitleText)
    {
        TitleText->SetFontSize(32.f * Scale);
        TitleText->SetPos(
            TitleRibbonLeft + 32.f * Scale,
            PanelTop + HeaderTopPadding);
        TitleText->SetSize(TitleRibbonWidth - 64.f * Scale, HeaderHeight);
    }

    if (CloseButton)
    {
        CloseButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 44.f * Scale,
            PanelTop + HeaderTopPadding - 4.f * Scale);
        CloseButton->SetSize(40.f * Scale, 40.f * Scale);
    }

    const std::vector<int> CategoryEntries = CollectCategoryEntryIndices();
    const int EntryCount = static_cast<int>(CategoryEntries.size());
    const int PageCount = (std::max)(
        1, (EntryCount + GEdictSlotsPerPage - 1) / GEdictSlotsPerPage);
    const bool ShowPageControls = PageCount > 1;
    const float ScrollTrackLeft = GridFrameLeft + GridFrameWidth - 20.f * Scale;
    const float ScrollTrackTop = GridFrameTop + 46.f * Scale;
    const float ScrollTrackHeight = GridFrameHeight - 92.f * Scale;
    const float ScrollTrackWidth = 12.f * Scale;

    if (PrevPageButton)
    {
        PrevPageButton->SetPos(
            ScrollTrackLeft - 7.f * Scale,
            GridFrameTop + 10.f * Scale);
        PrevPageButton->SetSize(26.f * Scale, 20.f * Scale);
        PrevPageButton->SetEnable(mOpen && ShowPageControls);
    }

    if (PageText)
    {
        PageText->SetFontSize(15.f * Scale);
        PageText->SetPos(
            ScrollTrackLeft - 34.f * Scale,
            GridFrameTop + 12.f * Scale);
        PageText->SetSize(80.f * Scale, 18.f * Scale);
        PageText->SetEnable(mOpen && ShowPageControls);
    }

    if (NextPageButton)
    {
        NextPageButton->SetPos(
            ScrollTrackLeft - 7.f * Scale,
            GridFrameTop + 30.f * Scale);
        NextPageButton->SetSize(26.f * Scale, 20.f * Scale);
        NextPageButton->SetEnable(mOpen && ShowPageControls);
    }

    if (ScrollTrack)
    {
        ScrollTrack->SetPos(ScrollTrackLeft, ScrollTrackTop);
        ScrollTrack->SetSize(ScrollTrackWidth, ScrollTrackHeight);
        ScrollTrack->SetEnable(mOpen && ShowPageControls);
    }

    if (ScrollThumb)
    {
        const float ThumbHeight =
            ShowPageControls ?
            (std::max)(56.f * Scale,
                ScrollTrackHeight / static_cast<float>(PageCount)) :
            ScrollTrackHeight;
        const float ThumbTravel = (std::max)(0.f, ScrollTrackHeight - ThumbHeight);
        const float ThumbOffset =
            PageCount > 1 ?
            ThumbTravel *
            (static_cast<float>(mCurrentPage) / static_cast<float>(PageCount - 1)) :
            0.f;

        ScrollThumb->SetPos(ScrollTrackLeft, ScrollTrackTop + ThumbOffset);
        ScrollThumb->SetSize(ScrollTrackWidth, ThumbHeight);
        ScrollThumb->SetEnable(mOpen && ShowPageControls);
    }

    const float CategoryGap = 10.f * Scale;
    const float CategoryWidth = 74.f * Scale;
    const float CategoryHeight = 86.f * Scale;
    const float CategoryStartX =
        PanelLeft +
        (PanelWidth -
            (CategoryWidth * static_cast<float>(GEdictCategoryCount) +
                CategoryGap * static_cast<float>(GEdictCategoryCount - 1))) *
        0.5f;
    const float CategoryTop = PanelTop - 16.f * Scale;

    for (int i = 0; i < static_cast<int>(mCategoryButtons.size()); ++i)
    {
        auto CategoryButton = mCategoryButtons[i].lock();
        auto CategoryIcon = i < static_cast<int>(mCategoryButtonIcons.size()) ?
            mCategoryButtonIcons[i].lock() : nullptr;

        if (!CategoryButton)
            continue;

        CategoryButton->SetPos(
            CategoryStartX +
            (CategoryWidth + CategoryGap) * static_cast<float>(i),
            CategoryTop);
        CategoryButton->SetSize(CategoryWidth, CategoryHeight);

        if (CategoryIcon)
        {
            CategoryIcon->SetPos(17.f * Scale, 8.f * Scale);
            CategoryIcon->SetSize(40.f * Scale, 40.f * Scale);
        }
    }

    const float SlotPaddingLeft = 22.f * Scale;
    const float SlotPaddingTop = 52.f * Scale;
    const float SlotPaddingBottom = 26.f * Scale;
    const float SlotAreaLeft = GridFrameLeft + SlotPaddingLeft;
    const float SlotAreaTop = GridFrameTop + SlotPaddingTop;
    const float SlotGapX = 14.f * Scale;
    const float SlotGapY = 16.f * Scale;
    const float SlotAreaWidth =
        GridFrameWidth - SlotPaddingLeft * 2.f - ScrollTrackWidth - 10.f * Scale;
    const float SlotAreaHeight =
        GridFrameHeight - SlotPaddingTop - SlotPaddingBottom;
    const float SlotWidth =
        (SlotAreaWidth - SlotGapX * (GEdictSlotColumnCount - 1)) /
        static_cast<float>(GEdictSlotColumnCount);
    const float SlotHeight =
        (SlotAreaHeight - SlotGapY * (GEdictSlotRowCount - 1)) /
        static_cast<float>(GEdictSlotRowCount);

    for (int i = 0; i < static_cast<int>(mEdictButtons.size()); ++i)
    {
        const int Row = i / GEdictSlotColumnCount;
        const int Col = i % GEdictSlotColumnCount;
        auto SlotButton = mEdictButtons[i].lock();
        auto SlotIcon = i < static_cast<int>(mEdictButtonIcons.size()) ?
            mEdictButtonIcons[i].lock() : nullptr;
        auto StarLeft = i < static_cast<int>(mEdictButtonStarLefts.size()) ?
            mEdictButtonStarLefts[i].lock() : nullptr;
        auto StarRight = i < static_cast<int>(mEdictButtonStarRights.size()) ?
            mEdictButtonStarRights[i].lock() : nullptr;
        auto SlotCheck = i < static_cast<int>(mEdictButtonChecks.size()) ?
            mEdictButtonChecks[i].lock() : nullptr;
        auto SlotText = i < static_cast<int>(mEdictButtonTexts.size()) ?
            mEdictButtonTexts[i].lock() : nullptr;

        if (!SlotButton)
            continue;

        SlotButton->SetPos(
            SlotAreaLeft +
            (SlotWidth + SlotGapX) * static_cast<float>(Col),
            SlotAreaTop + (SlotHeight + SlotGapY) * static_cast<float>(Row));
        SlotButton->SetSize(SlotWidth, SlotHeight);

        const float StarSize = 22.f * Scale;
        const float StarGap = 4.f * Scale;
        const float StarTop = 8.f * Scale;
        const float StarStartX = (SlotWidth - StarSize * 2.f - StarGap) * 0.5f;

        if (StarLeft)
        {
            StarLeft->SetPos(StarStartX, StarTop);
            StarLeft->SetSize(StarSize, StarSize);
        }

        if (StarRight)
        {
            StarRight->SetPos(StarStartX + StarSize + StarGap, StarTop);
            StarRight->SetSize(StarSize, StarSize);
        }

        if (SlotIcon)
        {
            const float IconSize = (std::min)(
                SlotWidth - 30.f * Scale,
                SlotHeight - 64.f * Scale);
            SlotIcon->SetPos(
                (SlotWidth - IconSize) * 0.5f,
                28.f * Scale);
            SlotIcon->SetSize(IconSize, IconSize);
        }

        if (SlotCheck)
        {
            const float CheckSize = 36.f * Scale;
            SlotCheck->SetPos(
                SlotWidth - CheckSize - 8.f * Scale,
                SlotHeight - CheckSize - 10.f * Scale);
            SlotCheck->SetSize(CheckSize, CheckSize);
        }

        if (SlotText)
        {
            SlotText->SetPos(10.f * Scale, SlotHeight - 42.f * Scale);
            SlotText->SetSize(SlotWidth - 20.f * Scale, 34.f * Scale);
            SlotText->SetFontSize(14.f * Scale);
        }
    }

    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailCostIcon = mDetailCostIcon.lock();
    auto DetailCostText = mDetailCostText.lock();
    auto DetailInfoText = mDetailInfoText.lock();
    auto FeedbackText = mFeedbackText.lock();
    auto RequirementText = mRequirementText.lock();
    auto DetailBodyText = mDetailBodyText.lock();
    auto ApplyButton = mApplyButton.lock();
    auto TaxPolicyTitleText = mTaxPolicyTitleText.lock();
    auto TaxPolicySummaryText = mTaxPolicySummaryText.lock();
    const float DetailContentLeft = PanelLeft + HorizontalMargin;
    const float RightColumnWidth = 250.f * Scale;
    const float RightColumnLeft =
        PanelLeft + PanelWidth - HorizontalMargin - RightColumnWidth;
    const float LeftColumnWidth =
        RightColumnLeft - DetailContentLeft - 20.f * Scale;
    const float DetailTitleTop = DetailFrameTop + 18.f * Scale;
    const float CostBlockWidth = 220.f * Scale;
    const float CostIconSize = 28.f * Scale;
    const float CostLeft =
        DetailContentLeft + (LeftColumnWidth - CostBlockWidth) * 0.5f;
    const float CostTop = DetailTitleTop + 36.f * Scale;
    const float DetailBodyTop = DetailTitleTop + 72.f * Scale;
    const float RequirementHeight = 28.f * Scale;
    const float DetailBodyHeight =
        DetailFrameTop + DetailFrameHeight -
        DetailBodyTop - 18.f * Scale - RequirementHeight;
    const float InfoPanelTop = DetailFrameTop + 18.f * Scale;
    const float InfoPanelHeight = 92.f * Scale;
    const float ApplyButtonTop = InfoPanelTop + InfoPanelHeight + 10.f * Scale;
    const float ApplyButtonHeight = 42.f * Scale;
    const float FeedbackTop = ApplyButtonTop + ApplyButtonHeight + 8.f * Scale;
    const float FeedbackHeight = 28.f * Scale;
    const float TaxPanelTop = FeedbackTop + FeedbackHeight + 8.f * Scale;
    const float TaxPanelHeightRaw =
        DetailFrameTop + DetailFrameHeight - TaxPanelTop - 16.f * Scale;
    const bool ShowTaxPanel = TaxPanelHeightRaw >= 110.f * Scale;
    const float TaxPanelHeight =
        ShowTaxPanel ? TaxPanelHeightRaw : 0.f;
    const float TaxSummaryHeight =
        ShowTaxPanel ?
        (std::max)(0.f, TaxPanelHeight - 130.f * Scale) :
        0.f;
    const float TaxButtonWidth = 24.f * Scale;
    const float TaxButtonHeight = 24.f * Scale;
    const float TaxRowHeight = 24.f * Scale;

    if (DetailTitleText)
    {
        DetailTitleText->SetPos(DetailContentLeft, DetailTitleTop);
        DetailTitleText->SetSize(LeftColumnWidth, 30.f * Scale);
    }

    if (DetailCostIcon)
    {
        DetailCostIcon->SetPos(CostLeft, CostTop);
        DetailCostIcon->SetSize(CostIconSize, CostIconSize);
    }

    if (DetailCostText)
    {
        DetailCostText->SetPos(
            CostLeft + CostIconSize + 8.f * Scale,
            CostTop - 2.f * Scale);
        DetailCostText->SetSize(
            CostBlockWidth - CostIconSize - 8.f * Scale,
            30.f * Scale);
    }

    if (DetailInfoPanel)
    {
        DetailInfoPanel->SetPos(RightColumnLeft, InfoPanelTop);
        DetailInfoPanel->SetSize(RightColumnWidth, InfoPanelHeight);
    }

    if (DetailInfoText)
    {
        DetailInfoText->SetPos(
            RightColumnLeft + 16.f * Scale,
            InfoPanelTop + 14.f * Scale);
        DetailInfoText->SetSize(
            RightColumnWidth - 32.f * Scale,
            InfoPanelHeight - 24.f * Scale);
    }

    if (ApplyButton)
    {
        ApplyButton->SetPos(RightColumnLeft, ApplyButtonTop);
        ApplyButton->SetSize(RightColumnWidth, ApplyButtonHeight);
    }

    if (FeedbackText)
    {
        FeedbackText->SetPos(RightColumnLeft, FeedbackTop);
        FeedbackText->SetSize(RightColumnWidth, FeedbackHeight);
    }

    if (DetailBodyText)
    {
        DetailBodyText->SetPos(DetailContentLeft, DetailBodyTop);
        DetailBodyText->SetSize(LeftColumnWidth, DetailBodyHeight);
    }

    if (RequirementText)
    {
        RequirementText->SetPos(
            DetailContentLeft,
            DetailBodyTop + DetailBodyHeight + 4.f * Scale);
        RequirementText->SetSize(LeftColumnWidth, RequirementHeight);
    }

    if (TaxInfoPanel)
    {
        TaxInfoPanel->SetPos(RightColumnLeft, TaxPanelTop);
        TaxInfoPanel->SetSize(RightColumnWidth, TaxPanelHeight);
        TaxInfoPanel->SetEnable(mOpen && ShowTaxPanel);
    }

    if (TaxPolicyTitleText)
    {
        TaxPolicyTitleText->SetPos(
            RightColumnLeft + 16.f * Scale,
            TaxPanelTop + 12.f * Scale);
        TaxPolicyTitleText->SetSize(RightColumnWidth - 32.f * Scale, 18.f * Scale);
        TaxPolicyTitleText->SetEnable(mOpen && ShowTaxPanel);
    }

    for (int i = 0; i < GTaxPolicyRowCount; ++i)
    {
        const float RowTop =
            TaxPanelTop + 38.f * Scale +
            TaxRowHeight * static_cast<float>(i);
        auto RowText = mTaxPolicyRowTexts[i].lock();
        auto DecreaseButton = mTaxDecreaseButtons[i].lock();
        auto IncreaseButton = mTaxIncreaseButtons[i].lock();

        if (RowText)
        {
            RowText->SetPos(RightColumnLeft + 16.f * Scale, RowTop);
            RowText->SetSize(
                RightColumnWidth - 32.f * Scale - TaxButtonWidth * 2.f - 12.f * Scale,
                TaxRowHeight);
            RowText->SetEnable(mOpen && ShowTaxPanel);
        }

        if (DecreaseButton)
        {
            DecreaseButton->SetPos(
                RightColumnLeft + RightColumnWidth -
                TaxButtonWidth * 2.f - 16.f * Scale - 6.f * Scale,
                RowTop + (TaxRowHeight - TaxButtonHeight) * 0.5f);
            DecreaseButton->SetSize(TaxButtonWidth, TaxButtonHeight);
            DecreaseButton->SetEnable(mOpen && ShowTaxPanel);
        }

        if (IncreaseButton)
        {
            IncreaseButton->SetPos(
                RightColumnLeft + RightColumnWidth -
                TaxButtonWidth - 16.f * Scale,
                RowTop + (TaxRowHeight - TaxButtonHeight) * 0.5f);
            IncreaseButton->SetSize(TaxButtonWidth, TaxButtonHeight);
            IncreaseButton->SetEnable(mOpen && ShowTaxPanel);
        }
    }

    if (TaxPolicySummaryText)
    {
        TaxPolicySummaryText->SetPos(
            RightColumnLeft + 16.f * Scale,
            TaxPanelTop + 118.f * Scale);
        TaxPolicySummaryText->SetSize(
            RightColumnWidth - 32.f * Scale,
            TaxSummaryHeight);
        TaxPolicySummaryText->SetEnable(mOpen && ShowTaxPanel);
    }
}

void CEdictWidget::RefreshData()
{
    RefreshCategoryButtons();
    RefreshEdictButtons();
    RefreshTaxPolicyControls();
}

void CEdictWidget::ApplyOpenState()
{
    auto MenuBackground = mMenuBackground.lock();
    auto MenuTitleRibbon = mMenuTitleRibbon.lock();
    auto MenuGridFrame = mMenuGridFrame.lock();
    auto MenuDetailFrame = mMenuDetailFrame.lock();
    auto DetailInfoPanel = mDetailInfoPanel.lock();
    auto TaxInfoPanel = mTaxInfoPanel.lock();
    auto DetailCostIcon = mDetailCostIcon.lock();
    auto ScrollTrack = mScrollTrack.lock();
    auto ScrollThumb = mScrollThumb.lock();
    auto TitleText = mTitleText.lock();
    auto PageText = mPageText.lock();
    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailCostText = mDetailCostText.lock();
    auto DetailInfoText = mDetailInfoText.lock();
    auto DetailBodyText = mDetailBodyText.lock();
    auto FeedbackText = mFeedbackText.lock();
    auto RequirementText = mRequirementText.lock();
    auto TaxPolicyTitleText = mTaxPolicyTitleText.lock();
    auto TaxPolicySummaryText = mTaxPolicySummaryText.lock();
    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();
    auto CloseButton = mCloseButton.lock();
    auto ApplyButton = mApplyButton.lock();
    auto ApplyButtonText = mApplyButtonText.lock();

    if (MenuBackground)
        MenuBackground->SetEnable(mOpen);
    if (MenuTitleRibbon)
        MenuTitleRibbon->SetEnable(mOpen);
    if (MenuGridFrame)
        MenuGridFrame->SetEnable(mOpen);
    if (MenuDetailFrame)
        MenuDetailFrame->SetEnable(mOpen);
    if (DetailInfoPanel)
        DetailInfoPanel->SetEnable(mOpen);
    if (TaxInfoPanel)
        TaxInfoPanel->SetEnable(mOpen);
    if (DetailCostIcon)
        DetailCostIcon->SetEnable(mOpen);
    if (ScrollTrack)
        ScrollTrack->SetEnable(mOpen);
    if (ScrollThumb)
        ScrollThumb->SetEnable(mOpen);
    if (TitleText)
        TitleText->SetEnable(mOpen);
    if (PageText)
        PageText->SetEnable(mOpen);
    if (DetailTitleText)
        DetailTitleText->SetEnable(mOpen);
    if (DetailCostText)
        DetailCostText->SetEnable(mOpen);
    if (DetailInfoText)
        DetailInfoText->SetEnable(mOpen);
    if (DetailBodyText)
        DetailBodyText->SetEnable(mOpen);
    if (FeedbackText)
        FeedbackText->SetEnable(mOpen);
    if (RequirementText)
        RequirementText->SetEnable(mOpen);
    if (TaxPolicyTitleText)
        TaxPolicyTitleText->SetEnable(mOpen);
    if (TaxPolicySummaryText)
        TaxPolicySummaryText->SetEnable(mOpen);
    if (PrevPageButton)
        PrevPageButton->SetEnable(mOpen);
    if (NextPageButton)
        NextPageButton->SetEnable(mOpen);
    if (CloseButton)
        CloseButton->SetEnable(mOpen);
    if (ApplyButton)
        ApplyButton->SetEnable(mOpen);
    if (ApplyButtonText)
        ApplyButtonText->SetEnable(mOpen);

    for (size_t i = 0; i < mCategoryButtons.size(); ++i)
    {
        auto Button = mCategoryButtons[i].lock();
        auto Icon = i < mCategoryButtonIcons.size() ?
            mCategoryButtonIcons[i].lock() : nullptr;

        if (Button)
            Button->SetEnable(mOpen);
        if (Icon)
            Icon->SetEnable(mOpen);
    }

    for (size_t i = 0; i < mEdictButtons.size(); ++i)
    {
        auto Button = mEdictButtons[i].lock();
        auto Icon = i < mEdictButtonIcons.size() ?
            mEdictButtonIcons[i].lock() : nullptr;
        auto StarLeft = i < mEdictButtonStarLefts.size() ?
            mEdictButtonStarLefts[i].lock() : nullptr;
        auto StarRight = i < mEdictButtonStarRights.size() ?
            mEdictButtonStarRights[i].lock() : nullptr;
        auto Check = i < mEdictButtonChecks.size() ?
            mEdictButtonChecks[i].lock() : nullptr;
        auto ButtonText = i < mEdictButtonTexts.size() ?
            mEdictButtonTexts[i].lock() : nullptr;

        if (Button)
            Button->SetEnable(mOpen);
        if (Icon)
            Icon->SetEnable(mOpen);
        if (StarLeft)
            StarLeft->SetEnable(mOpen);
        if (StarRight)
            StarRight->SetEnable(mOpen);
        if (Check)
            Check->SetEnable(mOpen);
        if (ButtonText)
            ButtonText->SetEnable(mOpen);
    }

    for (size_t i = 0; i < mTaxPolicyRowTexts.size(); ++i)
    {
        auto RowText = mTaxPolicyRowTexts[i].lock();
        auto DecreaseButton = mTaxDecreaseButtons[i].lock();
        auto IncreaseButton = mTaxIncreaseButtons[i].lock();

        if (RowText)
            RowText->SetEnable(mOpen);
        if (DecreaseButton)
            DecreaseButton->SetEnable(mOpen);
        if (IncreaseButton)
            IncreaseButton->SetEnable(mOpen);
    }
}

void CEdictWidget::RefreshCategoryButtons()
{
    for (int i = 0; i < static_cast<int>(mCategoryButtons.size()); ++i)
    {
        auto CategoryButton = mCategoryButtons[i].lock();

        if (!CategoryButton)
            continue;

        ApplyButtonTextureSet(
            CategoryButton,
            "EdictCategoryTabRefresh_" + std::to_string(i),
            i == static_cast<int>(mSelectedCategory) ?
            GCategoryTabTextureSelected :
            GCategoryTabTextureHidden,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(
            CategoryButton,
            i == static_cast<int>(mSelectedCategory));
    }

    auto TitleText = mTitleText.lock();

    if (TitleText)
        TitleText->SetText(GetCategoryLabel(mSelectedCategory));
}

void CEdictWidget::RefreshEdictButtons()
{
    const std::vector<int> CategoryEntries = CollectCategoryEntryIndices();
    const int EntryCount = static_cast<int>(CategoryEntries.size());
    const int PageCount = (std::max)(
        1, (EntryCount + GEdictSlotsPerPage - 1) / GEdictSlotsPerPage);

    if (mCurrentPage < 0)
        mCurrentPage = 0;
    else if (mCurrentPage >= PageCount)
        mCurrentPage = PageCount - 1;

    mVisibleEntryIndices.assign(GEdictSlotsPerPage, -1);
    const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();
    auto World = mWorld.lock();
    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);
    const int BeginIndex = mCurrentPage * GEdictSlotsPerPage;
    bool HasSelectedOnPage = false;
    bool HasPreviewOnPage = false;

    for (int i = 0; i < GEdictSlotsPerPage; ++i)
    {
        const int CategoryListIndex = BeginIndex + i;

        if (CategoryListIndex < 0 || CategoryListIndex >= EntryCount)
            continue;

        const int EntryIndex = CategoryEntries[CategoryListIndex];

        if (EntryIndex == mSelectedEntryIndex)
            HasSelectedOnPage = true;
        if (EntryIndex == mPreviewEntryIndex)
            HasPreviewOnPage = true;
    }

    if (!HasSelectedOnPage)
        mSelectedEntryIndex = -1;
    if (!HasPreviewOnPage)
        mPreviewEntryIndex = -1;

    const int FocusedEntryIndex =
        mPreviewEntryIndex >= 0 ? mPreviewEntryIndex : mSelectedEntryIndex;

    for (int i = 0; i < GEdictSlotsPerPage; ++i)
    {
        const int CategoryListIndex = BeginIndex + i;
        auto Button = mEdictButtons[i].lock();
        auto ButtonText = mEdictButtonTexts[i].lock();
        auto ButtonIcon = i < static_cast<int>(mEdictButtonIcons.size()) ?
            mEdictButtonIcons[i].lock() : nullptr;
        auto StarLeft = i < static_cast<int>(mEdictButtonStarLefts.size()) ?
            mEdictButtonStarLefts[i].lock() : nullptr;
        auto StarRight = i < static_cast<int>(mEdictButtonStarRights.size()) ?
            mEdictButtonStarRights[i].lock() : nullptr;
        auto ButtonCheck = i < static_cast<int>(mEdictButtonChecks.size()) ?
            mEdictButtonChecks[i].lock() : nullptr;

        if (!Button || !ButtonText || !ButtonIcon ||
            !StarLeft || !StarRight || !ButtonCheck)
            continue;

        if (CategoryListIndex >= 0 && CategoryListIndex < EntryCount)
        {
            const int EntryIndex = CategoryEntries[CategoryListIndex];
            const auto& Definition = Definitions[EntryIndex];
            const FGovernmentEdictState* State = nullptr;

            if (MainWorld)
                State = MainWorld->GetGovernmentEdictState(Definition.Type);

            const FEdictAvailabilityInfo Availability =
                EvaluateEdictAvailability(
                    Definition,
                    State,
                    MainWorld.get());
            const bool Focused = FocusedEntryIndex == EntryIndex;
            const bool Available =
                Availability.CanApply || Availability.Active;

            mVisibleEntryIndices[i] = EntryIndex;
            Button->ButtonEnable(true);
            ConfigureEdictSlotButtonVisual(
                Button,
                "EdictSlot_" + std::to_string(EntryIndex),
                Focused,
                Availability.Active,
                Availability.CoolingDown,
                Available);

            ButtonText->SetText(Definition.DisplayName.c_str());
            ButtonText->SetTextColor(
                !Available && !Availability.Active ? 116 : 58,
                !Available && !Availability.Active ? 116 : 47,
                !Available && !Availability.Active ? 116 : 31,
                255);

            if (Definition.IconPath)
            {
                ButtonIcon->SetTexture(
                    "EdictSlotIcon_" + std::to_string(EntryIndex),
                    Definition.IconPath);
            }
            else
            {
                ButtonIcon->SetTexture(
                    "EdictSlotFallback_" + std::to_string(EntryIndex),
                    GCostIconTexture);
            }

            ButtonIcon->SetTint(
                !Available && !Availability.Active ? 0.68f : 1.f,
                !Available && !Availability.Active ? 0.68f : 1.f,
                !Available && !Availability.Active ? 0.68f : 1.f,
                1.f);

            StarLeft->SetTexture(
                "EdictSlotStarLeftState_" + std::to_string(EntryIndex),
                Availability.Active ? GStarFullTexture : GStarEmptyTexture);
            StarRight->SetTexture(
                "EdictSlotStarRightState_" + std::to_string(EntryIndex),
                Availability.Active ? GStarFullTexture : GStarEmptyTexture);
            StarLeft->SetEnable(true);
            StarRight->SetEnable(true);
            ButtonCheck->SetEnable(Availability.Active);
            ButtonIcon->SetEnable(true);
        }
        else
        {
            Button->ButtonEnable(false);
            ConfigureEdictSlotButtonVisual(
                Button,
                "EdictSlotEmpty_" + std::to_string(i),
                false,
                false,
                false,
                false);
            ButtonText->SetText(TEXT(""));
            ButtonIcon->SetEnable(false);
            StarLeft->SetEnable(false);
            StarRight->SetEnable(false);
            ButtonCheck->SetEnable(false);
        }
    }

    auto PageText = mPageText.lock();

    if (PageText)
    {
        wchar_t PageBuffer[64] = {};
        swprintf_s(PageBuffer, L"%d / %d", mCurrentPage + 1, PageCount);
        PageText->SetText(PageBuffer);
    }

    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();

    if (PrevPageButton)
        PrevPageButton->ButtonEnable(mCurrentPage > 0);
    if (NextPageButton)
        NextPageButton->ButtonEnable(mCurrentPage < PageCount - 1);

    RefreshDetailPanel();
}

void CEdictWidget::RefreshTaxPolicyControls()
{
    auto TaxPolicyTitleText = mTaxPolicyTitleText.lock();
    auto TaxPolicySummaryText = mTaxPolicySummaryText.lock();
    auto World = mWorld.lock();
    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);

    if (TaxPolicyTitleText)
        TaxPolicyTitleText->SetText(TEXT("세금 정책"));

    const FTaxPolicy* TaxPolicy = nullptr;

    if (MainWorld)
        TaxPolicy = &MainWorld->GetTaxPolicy();

    for (int i = 0; i < GTaxPolicyRowCount; ++i)
    {
        const ETaxPolicyType TaxType = GTaxPolicyTypes[i];
        const int TaxRatePercent =
            TaxPolicy ?
            (TaxType == ETaxPolicyType::Consumption ?
                TaxPolicy->ConsumptionRatePercent :
                (TaxType == ETaxPolicyType::Income ?
                    TaxPolicy->IncomeRatePercent :
                    TaxPolicy->PropertyRatePercent)) :
            0;
        auto RowText = mTaxPolicyRowTexts[i].lock();
        auto DecreaseButton = mTaxDecreaseButtons[i].lock();
        auto IncreaseButton = mTaxIncreaseButtons[i].lock();

        if (RowText)
        {
            const std::wstring RowLabel =
                std::wstring(GetTaxPolicyDisplayName(TaxType)) +
                L" " +
                FormatPercentValue(TaxRatePercent);
            RowText->SetText(RowLabel.c_str());
        }

        if (DecreaseButton)
        {
            DecreaseButton->ButtonEnable(
                TaxPolicy &&
                TaxRatePercent > GetTaxPolicyMinPercent(TaxType));
        }

        if (IncreaseButton)
        {
            IncreaseButton->ButtonEnable(
                TaxPolicy &&
                TaxRatePercent < GetTaxPolicyMaxPercent(TaxType));
        }
    }

    if (TaxPolicySummaryText)
    {
        std::wstring Summary;

        if (MainWorld)
        {
            const FTaxPolicyEventStatus& TaxEventStatus =
                MainWorld->GetTaxPolicyEventStatus();

            Summary =
                L"오늘 세수 " +
                FormatCurrency(MainWorld->GetLastDailyTaxIncome()) +
                L"\n다음 일일 정산부터 반영";

            if (TaxEventStatus.Active)
            {
                Summary +=
                    L"\n활성 사건: " +
                    TaxEventStatus.Title +
                    L" (" +
                    std::to_wstring((std::max)(0, TaxEventStatus.RemainingDays)) +
                    L"일)";
            }
        }
        else
        {
            Summary = L"월드 정보 없음";
        }

        TaxPolicySummaryText->SetText(Summary.c_str());
    }
}

void CEdictWidget::SelectCategory(EEdictUiCategory Category)
{
    mSelectedCategory = Category;
    mCurrentPage = 0;
    mSelectedEntryIndex = -1;
    mPreviewEntryIndex = -1;
    mFeedbackMessage.clear();
    RefreshCategoryButtons();
    RefreshEdictButtons();
}

void CEdictWidget::MovePage(int DeltaPage)
{
    mCurrentPage += DeltaPage;
    mPreviewEntryIndex = -1;
    mSelectedEntryIndex = -1;
    RefreshEdictButtons();
}

void CEdictWidget::PreviewSlot(int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
        return;

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    mPreviewEntryIndex = EntryIndex;
    RefreshEdictButtons();
}

void CEdictWidget::ActivateSlot(int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
        return;

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    mSelectedEntryIndex = EntryIndex;
    mPreviewEntryIndex = EntryIndex;
    mFeedbackMessage.clear();
    RefreshEdictButtons();
}

void CEdictWidget::AdjustTaxPolicy(ETaxPolicyType Type, int DeltaPercent)
{
    auto World = mWorld.lock();
    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);

    if (!MainWorld)
    {
        mFeedbackMessage = L"월드 정보를 찾을 수 없습니다.";
        RefreshData();
        return;
    }

    MainWorld->AdjustTaxPolicy(Type, DeltaPercent, mFeedbackMessage);
    RefreshData();
}

void CEdictWidget::RefreshDetailPanel()
{
    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailCostText = mDetailCostText.lock();
    auto DetailInfoText = mDetailInfoText.lock();
    auto DetailBodyText = mDetailBodyText.lock();
    auto FeedbackText = mFeedbackText.lock();
    auto RequirementText = mRequirementText.lock();
    auto ApplyButton = mApplyButton.lock();
    auto ApplyButtonText = mApplyButtonText.lock();

    if (FeedbackText)
        FeedbackText->SetText(mFeedbackMessage.c_str());

    const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();
    const int DetailEntryIndex =
        mPreviewEntryIndex >= 0 ? mPreviewEntryIndex : mSelectedEntryIndex;

    if (DetailEntryIndex < 0 ||
        DetailEntryIndex >= static_cast<int>(Definitions.size()))
    {
        if (DetailTitleText)
            DetailTitleText->SetText(TEXT("칙령 정보"));
        if (DetailCostText)
            DetailCostText->SetText(TEXT("$0"));
        if (DetailInfoText)
            DetailInfoText->SetText(
                TEXT("상태: 미선택\n유형: 칙령\n적용: 칸을 선택하세요"));
        if (DetailBodyText)
            DetailBodyText->SetText(TEXT("카드에 마우스를 올리면 칙령 설명이 표시됩니다.\n활성 칙령은 금색 카드와 체크 표시로 구분되며, 적용 불가 칙령은 회색 카드로 표시됩니다.\n우측 패널에서는 적용 상태와 세금 정책을 함께 확인할 수 있습니다."));
        if (RequirementText)
            RequirementText->SetText(TEXT(""));
        if (ApplyButton)
            ApplyButton->ButtonEnable(false);
        if (ApplyButtonText)
            ApplyButtonText->SetText(TEXT("시행"));
        return;
    }

    const FGovernmentEdictDefinition& Definition = Definitions[DetailEntryIndex];

    if (DetailTitleText)
        DetailTitleText->SetText(Definition.DisplayName.c_str());

    auto World = mWorld.lock();
    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);
    const FGovernmentEdictState* State = nullptr;

    if (MainWorld)
        State = MainWorld->GetGovernmentEdictState(Definition.Type);
    const FEdictAvailabilityInfo Availability =
        EvaluateEdictAvailability(
            Definition,
            State,
            MainWorld.get());
    const ETaxPolicyEventType RequiredTaxEvent =
        ResolveRequiredTaxPolicyEvent(Definition.Type);
    bool CanApply = Availability.CanApply;
    std::wstring ApplyLabel = L"시행";
    std::wstring Body = Definition.Summary;
    std::wstring Info =
        std::wstring(L"상태: ") +
        Availability.StatusText +
        L"\n유형: " +
        (Definition.Mode == EGovernmentEdictMode::Passive ?
            L"유지형 칙령" :
            L"기간형 칙령");

    if (DetailCostText)
        DetailCostText->SetText(FormatCurrency(Availability.ActivationCost).c_str());

    Body += L"\n\n";
    Body += Definition.EffectText;

    if (Definition.MonthlyUpkeep > 0)
    {
        Body += L"\n월 유지비: ";
        Body += FormatCurrency(Definition.MonthlyUpkeep);
        Info +=
            L"\n유지비: " +
            FormatCurrency(Definition.MonthlyUpkeep);
    }

    if (Definition.Mode == EGovernmentEdictMode::Active)
    {
        const int DurationMonths = FormatDaysToMonths(Definition.DurationDays);
        Body += L"\n지속 기간: ";
        Body += std::to_wstring(DurationMonths);
        Body += L"개월";
        Info +=
            L"\n지속: " +
            std::to_wstring(DurationMonths) +
            L"개월";
    }

    if (Definition.CooldownDays > 0)
    {
        const int CooldownMonths = FormatDaysToMonths(Definition.CooldownDays);
        Body += L"\n재사용 대기: ";
        Body += std::to_wstring(CooldownMonths);
        Body += L"개월";
        Info +=
            L"\n재사용: " +
            std::to_wstring(CooldownMonths) +
            L"개월";
    }

    if (Availability.Active && Definition.Mode == EGovernmentEdictMode::Passive)
    {
        ApplyLabel = L"해제";
        CanApply = true;
    }
    else if (Availability.Active)
    {
        ApplyLabel = L"시행 중";
        CanApply = false;
    }
    else if (Availability.CoolingDown)
    {
        ApplyLabel = L"대기 중";
        CanApply = false;
    }
    else if (!Availability.RequirementText.empty())
    {
        ApplyLabel =
            Availability.RequirementText == L"예산 부족" ?
            L"예산 부족" :
            L"조건 필요";
        CanApply = false;
    }

    if (MainWorld && RequiredTaxEvent != ETaxPolicyEventType::None)
    {
        Info +=
            L"\n대응: " +
            std::wstring(GetTaxPolicyEventDisplayName(RequiredTaxEvent));

        const FTaxPolicyEventStatus& TaxEventStatus =
            MainWorld->GetTaxPolicyEventStatus();

        if (TaxEventStatus.Active &&
            TaxEventStatus.Type == RequiredTaxEvent)
        {
            Info +=
                L"\n사건: 대응 가능";
        }
        else if (TaxEventStatus.Active)
        {
            Info +=
                L"\n사건: " +
                TaxEventStatus.Title;
        }
        else
            Info += L"\n사건: 없음";
    }

    if (DetailInfoText)
        DetailInfoText->SetText(Info.c_str());

    if (DetailBodyText)
        DetailBodyText->SetText(Body.c_str());
    if (RequirementText)
    {
        if (!Availability.RequirementText.empty() &&
            !(Availability.Active && Definition.Mode == EGovernmentEdictMode::Passive))
        {
            const std::wstring Requirement =
                std::wstring(L"미충족: ") +
                Availability.RequirementText;
            RequirementText->SetText(Requirement.c_str());
        }
        else
            RequirementText->SetText(TEXT(""));
    }

    if (ApplyButton)
        ApplyButton->ButtonEnable(CanApply);
    if (ApplyButtonText)
        ApplyButtonText->SetText(ApplyLabel.c_str());
}

std::vector<int> CEdictWidget::CollectCategoryEntryIndices() const
{
    std::vector<int> Result;
    const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();

    for (int i = 0; i < static_cast<int>(Definitions.size()); ++i)
    {
        if (ResolveEdictUiCategory(Definitions[i].Type) != mSelectedCategory)
            continue;

        Result.push_back(i);
    }

    return Result;
}

void CEdictWidget::OnCategoryGeneralClick()
{
    SelectCategory(EEdictUiCategory::General);
}

void CEdictWidget::OnCategoryInteriorClick()
{
    SelectCategory(EEdictUiCategory::Interior);
}

void CEdictWidget::OnCategoryDefenseClick()
{
    SelectCategory(EEdictUiCategory::Defense);
}

void CEdictWidget::OnCategoryEducationClick()
{
    SelectCategory(EEdictUiCategory::Education);
}

void CEdictWidget::OnPrevPageClick()
{
    MovePage(-1);
}

void CEdictWidget::OnNextPageClick()
{
    MovePage(1);
}

void CEdictWidget::OnCloseButtonClick()
{
    SetOpen(false);
}

void CEdictWidget::OnApplyButtonClick()
{
    const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();
    const int EntryIndex =
        mPreviewEntryIndex >= 0 ? mPreviewEntryIndex : mSelectedEntryIndex;

    if (EntryIndex < 0 ||
        EntryIndex >= static_cast<int>(Definitions.size()))
    {
        mFeedbackMessage = L"먼저 칙령을 선택하세요.";
        RefreshDetailPanel();
        return;
    }

    auto World = mWorld.lock();
    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);

    if (!MainWorld)
    {
        mFeedbackMessage = L"월드 정보를 찾을 수 없습니다.";
        RefreshDetailPanel();
        return;
    }

    MainWorld->TryApplyEdict(
        Definitions[EntryIndex].Type,
        mFeedbackMessage);
    RefreshData();
}

void CEdictWidget::OnConsumptionTaxDownClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Consumption,
        -GetTaxPolicyStepPercent(ETaxPolicyType::Consumption));
}

void CEdictWidget::OnConsumptionTaxUpClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Consumption,
        GetTaxPolicyStepPercent(ETaxPolicyType::Consumption));
}

void CEdictWidget::OnIncomeTaxDownClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Income,
        -GetTaxPolicyStepPercent(ETaxPolicyType::Income));
}

void CEdictWidget::OnIncomeTaxUpClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Income,
        GetTaxPolicyStepPercent(ETaxPolicyType::Income));
}

void CEdictWidget::OnPropertyTaxDownClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Property,
        -GetTaxPolicyStepPercent(ETaxPolicyType::Property));
}

void CEdictWidget::OnPropertyTaxUpClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Property,
        GetTaxPolicyStepPercent(ETaxPolicyType::Property));
}

#define DEFINE_EDICT_SLOT_CALLBACKS(Index) \
    void CEdictWidget::OnSlot##Index##Click() \
    { \
        ActivateSlot(Index); \
    } \
    void CEdictWidget::OnSlot##Index##Hovered() \
    { \
        PreviewSlot(Index); \
    }

DEFINE_EDICT_SLOT_CALLBACKS(0)
DEFINE_EDICT_SLOT_CALLBACKS(1)
DEFINE_EDICT_SLOT_CALLBACKS(2)
DEFINE_EDICT_SLOT_CALLBACKS(3)
DEFINE_EDICT_SLOT_CALLBACKS(4)
DEFINE_EDICT_SLOT_CALLBACKS(5)
DEFINE_EDICT_SLOT_CALLBACKS(6)
DEFINE_EDICT_SLOT_CALLBACKS(7)
DEFINE_EDICT_SLOT_CALLBACKS(8)
DEFINE_EDICT_SLOT_CALLBACKS(9)
DEFINE_EDICT_SLOT_CALLBACKS(10)
DEFINE_EDICT_SLOT_CALLBACKS(11)

#undef DEFINE_EDICT_SLOT_CALLBACKS
