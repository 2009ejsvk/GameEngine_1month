#include "EdictWidget.h"
#include "TropicoUiStyle.h"
#include "../Politics/EdictSystem.h"
#include "../World/GovernmentCommandService.h"
#include "../World/MainWorldAccess.h"
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
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

    constexpr int GEdictCategoryCount = 4;
    constexpr int GEdictSlotsPerPage = 14;
    constexpr int GEdictSlotColumnCount = 7;
    constexpr int GEdictSlotRowCount = 2;
    constexpr int GTaxPolicyRowCount = 3;
    constexpr bool GEnableTaxPolicyPanel = false;

    constexpr const TCHAR* GEdictMenuPanelTexture = GMainMenuPanelTexture;
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
        L"식민지 시대",
        L"세계대전 시대",
        L"냉전 시대",
        L"현대 시대"
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

    enum class EEdictActionVisualMode
    {
        Neutral = 0,
        Primary,
        Active,
        CoolingDown,
        Requirement,
        BudgetShortage,
        Waiting
    };

    ETaxPolicyEventType ResolveRequiredTaxPolicyEvent(
        EGovernmentEdictType Type);
    const wchar_t* GetTaxPolicyEventDisplayName(
        ETaxPolicyEventType Type);

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

    void ConfigureEdictActionButtonVisual(
        const std::shared_ptr<CButton>& Button,
        const std::shared_ptr<CTextBlock>& ButtonText,
        EEdictActionVisualMode Mode,
        const std::wstring& Label)
    {
        if (ButtonText)
            ButtonText->SetText(Label.c_str());

        if (!Button)
            return;

        const TCHAR* NormalTexture = GBigTextButtonTexture;
        const TCHAR* HoverTexture = GBigTextButtonHoverTexture;
        const TCHAR* ClickTexture = GBigTextButtonSelectedTexture;
        const TCHAR* DisableTexture = GBigTextButtonDisabledTexture;
        FVector4 NormalTint = FVector4(1.f, 1.f, 1.f, 0.96f);
        FVector4 HoverTint = FVector4(1.f, 1.f, 1.f, 1.f);
        FVector4 ClickTint = FVector4(0.86f, 0.86f, 0.86f, 1.f);
        FVector4 DisableTint = FVector4(0.55f, 0.55f, 0.55f, 0.80f);
        unsigned char TextR = 89;
        unsigned char TextG = 60;
        unsigned char TextB = 16;
        float FontSize = 18.f;

        switch (Mode)
        {
        case EEdictActionVisualMode::Primary:
            NormalTexture = GBigTextButtonSelectedTexture;
            HoverTexture = GBigTextButtonSelectedTexture;
            ClickTexture = GBigTextButtonSelectedTexture;
            DisableTexture = GBigTextButtonSelectedTexture;
            NormalTint = FVector4(1.16f, 1.04f, 0.72f, 1.f);
            HoverTint = FVector4(1.22f, 1.10f, 0.80f, 1.f);
            ClickTint = FVector4(1.02f, 0.92f, 0.64f, 1.f);
            DisableTint = FVector4(0.92f, 0.86f, 0.70f, 0.86f);
            TextR = 84;
            TextG = 54;
            TextB = 10;
            FontSize = 21.f;
            break;
        case EEdictActionVisualMode::Active:
            NormalTexture = GBigTextButtonSelectedTexture;
            HoverTexture = GBigTextButtonSelectedTexture;
            ClickTexture = GBigTextButtonSelectedTexture;
            DisableTexture = GBigTextButtonSelectedTexture;
            NormalTint = FVector4(0.96f, 1.02f, 1.08f, 1.f);
            HoverTint = NormalTint;
            ClickTint = FVector4(0.88f, 0.94f, 1.00f, 1.f);
            DisableTint = FVector4(0.92f, 0.98f, 1.04f, 0.96f);
            TextR = 70;
            TextG = 78;
            TextB = 88;
            FontSize = 19.f;
            break;
        case EEdictActionVisualMode::CoolingDown:
            NormalTexture = GBigTextButtonDisabledTexture;
            HoverTexture = GBigTextButtonDisabledTexture;
            ClickTexture = GBigTextButtonDisabledTexture;
            DisableTexture = GBigTextButtonDisabledTexture;
            NormalTint = FVector4(0.98f, 0.94f, 0.84f, 1.f);
            HoverTint = NormalTint;
            ClickTint = FVector4(0.92f, 0.88f, 0.78f, 1.f);
            DisableTint = FVector4(0.94f, 0.90f, 0.82f, 0.96f);
            TextR = 110;
            TextG = 83;
            TextB = 34;
            FontSize = 19.f;
            break;
        case EEdictActionVisualMode::Requirement:
            NormalTexture = GBigTextButtonSelectedTexture;
            HoverTexture = GBigTextButtonSelectedTexture;
            ClickTexture = GBigTextButtonSelectedTexture;
            DisableTexture = GBigTextButtonSelectedTexture;
            NormalTint = FVector4(1.10f, 0.94f, 0.66f, 1.f);
            HoverTint = NormalTint;
            ClickTint = FVector4(1.00f, 0.86f, 0.58f, 1.f);
            DisableTint = FVector4(1.04f, 0.90f, 0.62f, 0.96f);
            TextR = 128;
            TextG = 82;
            TextB = 12;
            FontSize = 20.f;
            break;
        case EEdictActionVisualMode::BudgetShortage:
            NormalTexture = GBigTextButtonSelectedTexture;
            HoverTexture = GBigTextButtonSelectedTexture;
            ClickTexture = GBigTextButtonSelectedTexture;
            DisableTexture = GBigTextButtonSelectedTexture;
            NormalTint = FVector4(1.06f, 0.82f, 0.66f, 1.f);
            HoverTint = NormalTint;
            ClickTint = FVector4(0.98f, 0.74f, 0.58f, 1.f);
            DisableTint = FVector4(1.00f, 0.78f, 0.62f, 0.96f);
            TextR = 145;
            TextG = 52;
            TextB = 28;
            FontSize = 20.f;
            break;
        case EEdictActionVisualMode::Waiting:
            NormalTexture = GBigTextButtonDisabledTexture;
            HoverTexture = GBigTextButtonDisabledTexture;
            ClickTexture = GBigTextButtonDisabledTexture;
            DisableTexture = GBigTextButtonDisabledTexture;
            NormalTint = FVector4(0.95f, 0.95f, 0.92f, 1.f);
            HoverTint = NormalTint;
            ClickTint = FVector4(0.88f, 0.88f, 0.85f, 1.f);
            DisableTint = FVector4(0.92f, 0.92f, 0.89f, 0.95f);
            TextR = 92;
            TextG = 78;
            TextB = 52;
            FontSize = 18.f;
            break;
        default:
            break;
        }

        ApplyButtonTextureSet(
            Button,
            "EdictMenu_ApplyButtonVisual_" +
            std::to_string(static_cast<int>(Mode)),
            NormalTexture,
            HoverTexture,
            ClickTexture,
            DisableTexture);
        Button->SetTint(EButtonState::Normal, NormalTint);
        Button->SetTint(EButtonState::Hovered, HoverTint);
        Button->SetTint(EButtonState::Click, ClickTint);
        Button->SetTint(EButtonState::Disable, DisableTint);

        if (!ButtonText)
            return;

        ButtonText->SetFontSize(FontSize);
        ButtonText->SetTextColor(TextR, TextG, TextB, 255);
        ButtonText->SetShadowTextColor(255, 244, 214, 170);
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

    EEdictUiCategory ResolveEdictUiCategory(EEdictEra Era)
    {
        switch (Era)
        {
        case EEdictEra::Colonial:
            return EEdictUiCategory::Colonial;
        case EEdictEra::WorldWars:
            return EEdictUiCategory::WorldWars;
        case EEdictEra::ColdWar:
            return EEdictUiCategory::ColdWar;
        case EEdictEra::Modern:
            return EEdictUiCategory::Modern;
        default:
            return EEdictUiCategory::Colonial;
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
        const IMainWorldEdictReadAccess* MainWorld)
    {
        FEdictAvailabilityInfo Info;
        Info.StatusText = L"상태 확인 중";
        Info.ActivationCost = Definition.BaseCost;

        if (!Definition.Implemented)
        {
            Info.StatusText = L"준비 중";
            Info.RequirementText = L"아직 구현되지 않은 칙령입니다.";
            Info.ActivationCost = 0;
            return Info;
        }

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
                Info.RequirementText.clear();
                return Info;
            }

            Info.CanApply = true;
            Info.StatusText = L"사용 가능";
            Info.RequirementText.clear();
        }
        else
        {
            if (State->Active)
            {
                Info.StatusText =
                    L"시행 중 (" +
                    std::to_wstring((std::max)(0, State->RemainingDays)) +
                    L"일 남음)";
                Info.RequirementText.clear();
                return Info;
            }

            if (State->CooldownDays > 0)
            {
                Info.StatusText =
                    L"재사용 대기 (" +
                    std::to_wstring(State->CooldownDays) +
                    L"일)";
                Info.RequirementText.clear();
                return Info;
            }

            Info.CanApply = true;
            Info.StatusText = L"시행 가능";
            Info.RequirementText.clear();
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

        Info.RequirementText.clear();
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
    mPanelWidth = 1120.f;
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
        Button->SetEventCallback(
            EButtonEventState::Click,
            [this, i]()
            {
                SelectCategory(static_cast<EEdictUiCategory>(i));
            });

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
        Button->SetEventCallback(
            EButtonEventState::Click,
            [this, i]()
            {
                SelectOrApplySlot(i);
            });
        Button->SetEventCallback(
            EButtonEventState::Hovered,
            [this, i]()
            {
                PreviewSlot(i);
            });

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
        DetailInfoText->SetFontSize(13.f);
        DetailInfoText->SetAlignH(ETextAlignH::Center);
        DetailInfoText->SetAlignV(ETextAlignV::Middle);
        DetailInfoText->SetTextColor(102, 91, 68, 255);
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
        FeedbackText->SetFontSize(13.f);
        FeedbackText->SetAlignH(ETextAlignH::Center);
        FeedbackText->SetAlignV(ETextAlignV::Middle);
        FeedbackText->SetTextColor(170, 118, 27, 255);
        FeedbackText->EnableShadow(true);
        FeedbackText->SetShadowOffset(1.f, 1.f);
        FeedbackText->SetShadowTextColor(245, 235, 205, 160);
        mFeedbackText = FeedbackText;
    }

    auto DetailBodyText =
        CreateWidget<CTextBlock>("EdictMenu_DetailBody", 8).lock();

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
            ApplyButtonText->SetFontSize(20.f);
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

    if (mDoubleClickTimer > 0.f)
        mDoubleClickTimer -= DeltaTime;

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

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    if (Resolution.Width != mLastLayoutWidth || Resolution.Height != mLastLayoutHeight)
    {
        mLastLayoutWidth = Resolution.Width;
        mLastLayoutHeight = Resolution.Height;
        RefreshLayout();
    }
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
    const float AvailableWidth = (std::max)(520.f, ScreenWidth - 120.f);
    const float AvailableHeight = (std::max)(520.f, ScreenHeight - 140.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(AvailableWidth / mPanelWidth,
                AvailableHeight / mPanelHeight));
    const float PanelWidth = mPanelWidth * Scale;
    const float PanelHeight = mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;
    const float HorizontalMargin = 24.f * Scale;
    const float VerticalMargin = 18.f * Scale;
    const float HeaderTopPadding = 40.f * Scale;
    const float HeaderHeight = 48.f * Scale;
    const float TitleRibbonWidth = PanelWidth - 118.f * Scale;
    const float TitleRibbonLeft =
        PanelLeft + (PanelWidth - TitleRibbonWidth) * 0.5f;
    const float GridFrameLeft = PanelLeft + HorizontalMargin;
    const float GridFrameTop =
        PanelTop + HeaderTopPadding + HeaderHeight + 10.f * Scale;
    const float GridFrameWidth = PanelWidth - HorizontalMargin * 2.f;
    const float GridFrameHeight = 404.f * Scale;
    const float DetailFrameLeft = GridFrameLeft;
    const float DetailFrameTop =
        GridFrameTop + GridFrameHeight + 12.f * Scale;
    const float DetailFrameWidth = GridFrameWidth;
    const float DetailFrameHeight =
        PanelTop + PanelHeight - DetailFrameTop - VerticalMargin;

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
        TitleText->SetFontSize(30.f * Scale);
        TitleText->SetPos(
            TitleRibbonLeft + 32.f * Scale,
            PanelTop + HeaderTopPadding);
        TitleText->SetSize(TitleRibbonWidth - 64.f * Scale, HeaderHeight);
    }

    if (CloseButton)
    {
        CloseButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 44.f * Scale,
            PanelTop + HeaderTopPadding - 2.f * Scale);
        CloseButton->SetSize(40.f * Scale, 40.f * Scale);
    }

    const std::vector<int> CategoryEntries = CollectCategoryEntryIndices();
    const int EntryCount = static_cast<int>(CategoryEntries.size());
    const int PageCount = (std::max)(
        1, (EntryCount + GEdictSlotsPerPage - 1) / GEdictSlotsPerPage);
    const bool ShowScrollBar = PageCount > 1;
    const float ScrollTrackWidth = 10.f * Scale;
    const float ScrollTrackLeft =
        GridFrameLeft + GridFrameWidth - 18.f * Scale;
    const float ScrollTrackTop = GridFrameTop + 24.f * Scale;
    const float ScrollTrackHeight = GridFrameHeight - 48.f * Scale;

    if (PrevPageButton)
    {
        PrevPageButton->SetPos(0.f, 0.f);
        PrevPageButton->SetSize(0.f, 0.f);
        PrevPageButton->SetEnable(false);
    }

    if (PageText)
    {
        PageText->SetPos(0.f, 0.f);
        PageText->SetSize(0.f, 0.f);
        PageText->SetEnable(false);
    }

    if (NextPageButton)
    {
        NextPageButton->SetPos(0.f, 0.f);
        NextPageButton->SetSize(0.f, 0.f);
        NextPageButton->SetEnable(false);
    }

    if (ScrollTrack)
    {
        ScrollTrack->SetPos(ScrollTrackLeft, ScrollTrackTop);
        ScrollTrack->SetSize(ScrollTrackWidth, ScrollTrackHeight);
        ScrollTrack->SetEnable(mOpen && ShowScrollBar);
    }

    if (ScrollThumb)
    {
        const float ThumbHeight =
            ShowScrollBar ?
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
        ScrollThumb->SetEnable(mOpen && ShowScrollBar);
    }

    const float CategoryGap = 12.f * Scale;
    const float CategoryWidth = 74.f * Scale;
    const float CategoryHeight = 90.f * Scale;
    const float CategoryStartX =
        PanelLeft +
        (PanelWidth -
            (CategoryWidth * static_cast<float>(GEdictCategoryCount) +
                CategoryGap * static_cast<float>(GEdictCategoryCount - 1))) *
        0.5f;
    const float CategoryTop = PanelTop - 10.f * Scale;

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
            CategoryIcon->SetPos(17.f * Scale, 10.f * Scale);
            CategoryIcon->SetSize(40.f * Scale, 40.f * Scale);
        }
    }

    const float SlotPaddingLeft = 18.f * Scale;
    const float SlotPaddingTop = 26.f * Scale;
    const float SlotPaddingRight = 22.f * Scale;
    const float SlotPaddingBottom = 22.f * Scale;
    const float SlotAreaLeft = GridFrameLeft + SlotPaddingLeft;
    const float SlotAreaTop = GridFrameTop + SlotPaddingTop;
    const float SlotGapX = 12.f * Scale;
    const float SlotGapY = 14.f * Scale;
    const float SlotAreaWidth =
        GridFrameWidth - SlotPaddingLeft - SlotPaddingRight -
        ScrollTrackWidth - 22.f * Scale;
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

        const float StarSize = 24.f * Scale;
        const float StarGap = 2.f * Scale;
        const float StarTop = 3.f * Scale;
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
                SlotWidth - 22.f * Scale,
                SlotHeight - 66.f * Scale);
            SlotIcon->SetPos(
                (SlotWidth - IconSize) * 0.5f,
                26.f * Scale);
            SlotIcon->SetSize(IconSize, IconSize);
        }

        if (SlotCheck)
        {
            const float CheckSize = 38.f * Scale;
            SlotCheck->SetPos(
                SlotWidth - CheckSize - 7.f * Scale,
                SlotHeight - CheckSize - 7.f * Scale);
            SlotCheck->SetSize(CheckSize, CheckSize);
        }

        if (SlotText)
        {
            SlotText->SetPos(10.f * Scale, SlotHeight - 44.f * Scale);
            SlotText->SetSize(SlotWidth - 20.f * Scale, 38.f * Scale);
            SlotText->SetFontSize(14.5f * Scale);
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
    auto ApplyButtonText = mApplyButtonText.lock();
    auto TaxPolicyTitleText = mTaxPolicyTitleText.lock();
    auto TaxPolicySummaryText = mTaxPolicySummaryText.lock();
    const float DetailInnerLeft = DetailFrameLeft + 16.f * Scale;
    const float DetailInnerWidth = DetailFrameWidth - 32.f * Scale;
    const float DetailHeaderCenterX = DetailFrameLeft + DetailFrameWidth * 0.5f;
    const float DetailTitleTop = DetailFrameTop + 10.f * Scale;
    const float DetailTitleWidth = DetailFrameWidth * 0.52f;
    const float DetailTitleHeight = 30.f * Scale;
    const float CostIconSize = 24.f * Scale;
    const float CostTop = DetailTitleTop + DetailTitleHeight - 3.f * Scale;
    const float CostRowWidth = 140.f * Scale;
    const float CostLeft = DetailHeaderCenterX - CostRowWidth * 0.5f;
    const float DetailInfoTop = CostTop + 28.f * Scale;
    const float DetailInfoHeight = 20.f * Scale;
    const float FeedbackTop = DetailInfoTop + DetailInfoHeight + 2.f * Scale;
    const float FeedbackHeight = 22.f * Scale;
    const float ApplyButtonWidth = 140.f * Scale;
    const float ApplyButtonHeight = 38.f * Scale;
    const float ApplyButtonTop = DetailFrameTop + 14.f * Scale;
    const float ApplyButtonLeft =
        DetailFrameLeft + DetailFrameWidth - ApplyButtonWidth - 18.f * Scale;
    const float InfoPanelTop = DetailFrameTop + 74.f * Scale;
    const float InfoPanelHeight =
        DetailFrameTop + DetailFrameHeight - InfoPanelTop - 14.f * Scale;
    const float FeedbackGap = 8.f * Scale;
    const float DetailBodyLeft = DetailInnerLeft + 14.f * Scale;
    const float DetailBodyTop = FeedbackTop + FeedbackHeight + FeedbackGap;
    const float DetailBodyWidth = DetailInnerWidth - 28.f * Scale;
    const float RequirementHeight = 28.f * Scale;
    const float RequirementTop =
        InfoPanelTop + InfoPanelHeight - RequirementHeight - 8.f * Scale;
    const float DetailBodyBottom =
        RequirementTop - 10.f * Scale;
    const float DetailBodyHeight =
        (std::max)(0.f, DetailBodyBottom - DetailBodyTop);
    const float TaxPanelHeight = 0.f;
    const float TaxSummaryHeight = 0.f;
    const bool ShowTaxPanel = GEnableTaxPolicyPanel && TaxPanelHeight > 0.f;
    const float TaxButtonWidth = 0.f;
    const float TaxButtonHeight = 0.f;

    if (DetailTitleText)
    {
        DetailTitleText->SetFontSize(23.f * Scale);
        DetailTitleText->SetPos(
            DetailHeaderCenterX - DetailTitleWidth * 0.5f,
            DetailTitleTop);
        DetailTitleText->SetSize(DetailTitleWidth, DetailTitleHeight);
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
            CostRowWidth - CostIconSize - 8.f * Scale,
            30.f * Scale);
        DetailCostText->SetFontSize(18.f * Scale);
    }

    if (DetailInfoPanel)
    {
        DetailInfoPanel->SetPos(DetailInnerLeft, InfoPanelTop);
        DetailInfoPanel->SetSize(DetailInnerWidth, InfoPanelHeight);
    }

    if (DetailInfoText)
    {
        DetailInfoText->SetPos(
            DetailInnerLeft,
            DetailInfoTop);
        DetailInfoText->SetSize(
            DetailInnerWidth,
            DetailInfoHeight);
        DetailInfoText->SetFontSize(12.5f * Scale);
    }

    if (ApplyButton)
    {
        ApplyButton->SetPos(ApplyButtonLeft, ApplyButtonTop);
        ApplyButton->SetSize(ApplyButtonWidth, ApplyButtonHeight);
    }

    if (ApplyButtonText)
        ApplyButtonText->SetFontSize(16.f * Scale);

    if (FeedbackText)
    {
        FeedbackText->SetPos(DetailInnerLeft, FeedbackTop);
        FeedbackText->SetSize(DetailInnerWidth, FeedbackHeight);
        FeedbackText->SetFontSize(12.5f * Scale);
    }

    if (DetailBodyText)
    {
        DetailBodyText->SetPos(DetailBodyLeft, DetailBodyTop);
        DetailBodyText->SetSize(DetailBodyWidth, DetailBodyHeight);
        DetailBodyText->SetFontSize(15.f * Scale);
    }

    if (RequirementText)
    {
        RequirementText->SetPos(DetailInnerLeft, RequirementTop);
        RequirementText->SetSize(DetailInnerWidth, RequirementHeight);
        RequirementText->SetFontSize(13.5f * Scale);
    }

    if (TaxInfoPanel)
    {
        TaxInfoPanel->SetPos(0.f, 0.f);
        TaxInfoPanel->SetSize(0.f, 0.f);
        TaxInfoPanel->SetEnable(mOpen && ShowTaxPanel);
    }

    if (TaxPolicyTitleText)
    {
        TaxPolicyTitleText->SetPos(0.f, 0.f);
        TaxPolicyTitleText->SetSize(0.f, 0.f);
        TaxPolicyTitleText->SetEnable(mOpen && ShowTaxPanel);
    }

    for (int i = 0; i < GTaxPolicyRowCount; ++i)
    {
        auto RowText = mTaxPolicyRowTexts[i].lock();
        auto DecreaseButton = mTaxDecreaseButtons[i].lock();
        auto IncreaseButton = mTaxIncreaseButtons[i].lock();

        if (RowText)
        {
            RowText->SetPos(0.f, 0.f);
            RowText->SetSize(0.f, 0.f);
            RowText->SetEnable(mOpen && ShowTaxPanel);
        }

        if (DecreaseButton)
        {
            DecreaseButton->SetPos(0.f, 0.f);
            DecreaseButton->SetSize(TaxButtonWidth, TaxButtonHeight);
            DecreaseButton->SetEnable(mOpen && ShowTaxPanel);
        }

        if (IncreaseButton)
        {
            IncreaseButton->SetPos(0.f, 0.f);
            IncreaseButton->SetSize(TaxButtonWidth, TaxButtonHeight);
            IncreaseButton->SetEnable(mOpen && ShowTaxPanel);
        }
    }

    if (TaxPolicySummaryText)
    {
        TaxPolicySummaryText->SetPos(0.f, 0.f);
        TaxPolicySummaryText->SetSize(0.f, 0.f);
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
        ApplyButton->SetEnable(false);
    if (ApplyButtonText)
        ApplyButtonText->SetEnable(false);

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
        const bool HasEntry =
            i < mVisibleEntryIndices.size() &&
            mVisibleEntryIndices[i] >= 0;
        const bool SlotVisible = mOpen && HasEntry;
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
            Button->SetEnable(SlotVisible);
        if (Icon)
            Icon->SetEnable(SlotVisible && Icon->GetEnable());
        if (StarLeft)
            StarLeft->SetEnable(SlotVisible && StarLeft->GetEnable());
        if (StarRight)
            StarRight->SetEnable(SlotVisible && StarRight->GetEnable());
        if (Check)
            Check->SetEnable(SlotVisible && Check->GetEnable());
        if (ButtonText)
            ButtonText->SetEnable(SlotVisible);
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
        auto CategoryIcon = i < static_cast<int>(mCategoryButtonIcons.size()) ?
            mCategoryButtonIcons[i].lock() : nullptr;
        const bool Selected = i == static_cast<int>(mSelectedCategory);

        if (!CategoryButton)
            continue;

        ApplyButtonTextureSet(
            CategoryButton,
            "EdictCategoryTabRefresh_" + std::to_string(i),
            Selected ?
            GCategoryTabTextureSelected :
            GCategoryTabTextureHidden,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(
            CategoryButton,
            Selected);

        if (CategoryIcon)
        {
            CategoryIcon->SetTint(
                Selected ? 1.f : 0.88f,
                Selected ? 1.f : 0.90f,
                Selected ? 1.f : 0.94f,
                1.f);
        }
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
    auto MainWorld = std::dynamic_pointer_cast<IMainWorldEdictReadAccess>(World);
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
            Button->SetEnable(mOpen);
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
            StarLeft->SetEnable(mOpen);
            StarRight->SetEnable(mOpen);
            ButtonCheck->SetEnable(mOpen && Availability.Active);
            ButtonIcon->SetEnable(mOpen);
            ButtonText->SetEnable(mOpen);
        }
        else
        {
            Button->ButtonEnable(false);
            Button->SetEnable(false);
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
    auto MainWorld = std::dynamic_pointer_cast<IMainWorldEdictReadAccess>(World);

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
            Summary = L"세금 보고 준비 중";
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

    if (mPreviewEntryIndex == EntryIndex)
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

void CEdictWidget::SelectOrApplySlot(int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
        return;

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    const bool IsDoubleClick =
        EntryIndex == mLastClickedEntryIndex && mDoubleClickTimer > 0.f;

    mLastClickedEntryIndex = EntryIndex;
    mDoubleClickTimer = 0.45f;
    mSelectedEntryIndex = EntryIndex;
    mPreviewEntryIndex = EntryIndex;
    mFeedbackMessage.clear();
    RefreshEdictButtons();

    if (IsDoubleClick)
        OnApplyButtonClick();
}

void CEdictWidget::AdjustTaxPolicy(ETaxPolicyType Type, int DeltaPercent)
{
    auto World = mWorld.lock();
    auto MainWorld = std::dynamic_pointer_cast<IGovernmentCommandService>(World);

    if (!MainWorld)
    {
        mFeedbackMessage = L"지금은 행정 정보를 확인할 수 없습니다.";
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

    if (FeedbackText)
    {
        FeedbackText->SetText(mFeedbackMessage.c_str());
        FeedbackText->SetTextColor(170, 118, 27, 255);
        FeedbackText->SetShadowTextColor(245, 235, 205, 160);
    }

    const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();
    const int DetailEntryIndex =
        mPreviewEntryIndex >= 0 ? mPreviewEntryIndex : mSelectedEntryIndex;

    if (DetailEntryIndex < 0 ||
        DetailEntryIndex >= static_cast<int>(Definitions.size()))
    {
        if (DetailTitleText)
            DetailTitleText->SetText(TEXT("칙령 선택"));
        if (DetailCostText)
            DetailCostText->SetText(TEXT("$0"));
        if (DetailInfoText)
            DetailInfoText->SetText(TEXT("왼쪽 카드에서 칙령을 선택하세요."));
        if (DetailBodyText)
            DetailBodyText->SetText(
                TEXT("카드를 고르면 효과와 제약 조건이 아래 패널에 표시됩니다.\n현재 시행 중인 칙령은 금색 강조와 체크 표시로 구분됩니다."));
        if (RequirementText)
            RequirementText->SetText(TEXT(""));
        return;
    }

    const FGovernmentEdictDefinition& Definition = Definitions[DetailEntryIndex];

    if (DetailTitleText)
        DetailTitleText->SetText(Definition.DisplayName.c_str());

    auto World = mWorld.lock();
    auto MainWorld = std::dynamic_pointer_cast<IMainWorldEdictReadAccess>(World);
    const FGovernmentEdictState* State = nullptr;

    if (MainWorld)
        State = MainWorld->GetGovernmentEdictState(Definition.Type);
    const FEdictAvailabilityInfo Availability =
        EvaluateEdictAvailability(
            Definition,
            State,
            MainWorld.get());

    if (!Definition.Implemented)
    {
        if (DetailCostText)
            DetailCostText->SetText(TEXT("$0"));
        if (DetailInfoText)
            DetailInfoText->SetText(TEXT("준비 중  |  참고용 칙령"));
        if (DetailBodyText)
            DetailBodyText->SetText(
                TEXT("아이콘과 시대 배치만 연결된 칙령입니다.\n실제 효과와 적용 로직은 아직 연결되지 않았습니다."));
        if (RequirementText)
        {
            RequirementText->SetText(TEXT("미구현: 아직 게임 로직이 연결되지 않았습니다."));
            RequirementText->SetTextColor(211, 36, 20, 255);
            RequirementText->SetShadowTextColor(255, 228, 216, 170);
        }
        return;
    }

    const ETaxPolicyEventType RequiredTaxEvent =
        ResolveRequiredTaxPolicyEvent(Definition.Type);
    std::wstring Body = Definition.Summary;
    std::wstring Info =
        Availability.StatusText +
        L"  |  " +
        (Definition.Mode == EGovernmentEdictMode::Passive ?
            L"상시 칙령" :
            L"기간 칙령");
    std::wstring RequirementMessage;

    if (DetailCostText)
        DetailCostText->SetText(FormatCurrency(Availability.ActivationCost).c_str());

    if (!Definition.EffectText.empty())
    {
        if (!Body.empty())
            Body += L"\n";
        Body += Definition.EffectText;
    }

    if (Definition.MonthlyUpkeep > 0)
    {
        Body +=
            L"\n\n매달 유지비 " +
            FormatCurrency(Definition.MonthlyUpkeep) +
            L"이 소요됩니다.";
    }

    if (Definition.Mode == EGovernmentEdictMode::Active)
    {
        const int DurationMonths = FormatDaysToMonths(Definition.DurationDays);
        Info +=
            L"  |  지속 " +
            std::to_wstring(DurationMonths) +
            L"개월";
    }

    if (Definition.CooldownDays > 0)
    {
        const int CooldownMonths = FormatDaysToMonths(Definition.CooldownDays);
        Info +=
            L"  |  재사용 " +
            std::to_wstring(CooldownMonths) +
            L"개월";
    }

    if (!Availability.CanApply &&
        !Availability.Active &&
        !Availability.CoolingDown &&
        !Availability.RequirementText.empty())
    {
        RequirementMessage = L"미충족: " + Availability.RequirementText;
    }

    if (MainWorld && RequiredTaxEvent != ETaxPolicyEventType::None)
    {
        Body +=
            L"\n\n필요 사건: " +
            std::wstring(GetTaxPolicyEventDisplayName(RequiredTaxEvent));

        const FTaxPolicyEventStatus& TaxEventStatus =
            MainWorld->GetTaxPolicyEventStatus();

        if (TaxEventStatus.Active &&
            TaxEventStatus.Type == RequiredTaxEvent)
        {
            Info += L"  |  대응 가능";
        }
        else if (!Availability.CanApply)
        {
            if (TaxEventStatus.Active)
            {
                RequirementMessage =
                    L"미충족: 현재 사건은 " + TaxEventStatus.Title;
            }
            else if (RequirementMessage.empty())
            {
                RequirementMessage =
                    L"미충족: " +
                    std::wstring(GetTaxPolicyEventDisplayName(RequiredTaxEvent)) +
                    L" 발생 필요";
            }
        }
    }

    if (DetailInfoText)
        DetailInfoText->SetText(Info.c_str());

    if (DetailBodyText)
        DetailBodyText->SetText(Body.c_str());
    if (RequirementText)
    {
        if (!RequirementMessage.empty() &&
            !Availability.Active &&
            !Availability.CoolingDown)
        {
            RequirementText->SetText(RequirementMessage.c_str());
            if (Availability.RequirementText == L"예산 부족")
            {
                RequirementText->SetTextColor(178, 48, 30, 255);
                RequirementText->SetShadowTextColor(255, 226, 214, 170);
            }
            else
            {
                RequirementText->SetTextColor(211, 36, 20, 255);
                RequirementText->SetShadowTextColor(255, 228, 216, 170);
            }
        }
        else
            RequirementText->SetText(TEXT(""));
    }

}

std::vector<int> CEdictWidget::CollectCategoryEntryIndices() const
{
    std::vector<int> Result;
    const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();

    for (int i = 0; i < static_cast<int>(Definitions.size()); ++i)
    {
        if (ResolveEdictUiCategory(Definitions[i].Era) != mSelectedCategory)
            continue;

        Result.push_back(i);
    }

    return Result;
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
    auto MainWorld = std::dynamic_pointer_cast<IGovernmentCommandService>(World);

    if (!MainWorld)
    {
        mFeedbackMessage = L"지금은 행정 정보를 확인할 수 없습니다.";
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
