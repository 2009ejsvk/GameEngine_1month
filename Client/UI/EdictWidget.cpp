#include "EdictWidget.h"
#include "../Politics/EdictSystem.h"
#include "../World/MainWorld.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    constexpr int GEdictCategoryCount = 4;
    constexpr int GEdictSlotsPerPage = 12;
    constexpr int GEdictSlotColumnCount = 4;
    constexpr int GEdictSlotRowCount = 3;
    constexpr int GTaxPolicyRowCount = 3;

    constexpr const TCHAR* GEdictMenuPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\5_MainMenu\\CenterPopUp\\T_center_popUp.png");
    constexpr const TCHAR* GEmptySlotTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_standard.png");
    constexpr const TCHAR* GCategoryTabBackgroundTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\IconBackground\\T_icon_background.png");

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

    void ConfigureEdictSlotButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected,
        bool Active,
        bool CoolingDown)
    {
        if (!Button)
            return;

        if (Selected)
        {
            Button->SetTint(EButtonState::Normal,
                FVector4(1.00f, 0.94f, 0.72f, 1.f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(1.00f, 0.98f, 0.84f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.88f, 0.82f, 0.60f, 1.f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.50f, 0.50f, 0.50f, 0.75f));
            return;
        }

        if (Active)
        {
            Button->SetTint(EButtonState::Normal,
                FVector4(0.82f, 1.00f, 0.84f, 0.98f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(0.92f, 1.00f, 0.92f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.68f, 0.90f, 0.72f, 1.f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.50f, 0.50f, 0.50f, 0.75f));
            return;
        }

        if (CoolingDown)
        {
            Button->SetTint(EButtonState::Normal,
                FVector4(0.78f, 0.84f, 0.92f, 0.96f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(0.88f, 0.92f, 0.98f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.66f, 0.72f, 0.84f, 1.f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.50f, 0.50f, 0.50f, 0.75f));
            return;
        }

        ConfigureIconSlotButtonStyle(Button);
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
                FVector4(0.5f, 0.5f, 0.5f, 0.7f));
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

    std::wstring FormatPercentValue(int Value)
    {
        return std::to_wstring(Value) + L"%";
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

    std::wstring BuildEdictSlotStatusText(
        const FGovernmentEdictDefinition& Definition,
        const FGovernmentEdictState* State)
    {
        if (!State)
            return L"정보 없음";

        if (Definition.Mode == EGovernmentEdictMode::Passive)
            return State->Active ? L"활성" : L"사용 가능";

        if (State->Active)
            return L"시행중";

        if (State->CooldownDays > 0)
            return L"대기 " + std::to_wstring(State->CooldownDays) + L"일";

        return L"사용 가능";
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

    auto MenuBackground = CreateWidget<CImage>("EdictMenu_Background", 6).lock();

    if (MenuBackground)
    {
        MenuBackground->SetTexture("EdictMenuBackground", GEdictMenuPanelTexture);
        MenuBackground->SetTint(1.f, 1.f, 1.f, 1.f);
        mMenuBackground = MenuBackground;
    }

    auto TitleText = CreateWidget<CTextBlock>("EdictMenu_Title", 7).lock();

    if (TitleText)
    {
        TitleText->SetText(GetCategoryLabel(mSelectedCategory));
        TitleText->SetFontSize(28.f);
        TitleText->SetAlignH(ETextAlignH::Center);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(245, 245, 245, 255);
        TitleText->EnableShadow(true);
        TitleText->SetShadowOffset(1.f, 1.f);
        TitleText->SetShadowTextColor(20, 20, 20, 255);
        mTitleText = TitleText;
    }

    auto PageText = CreateWidget<CTextBlock>("EdictMenu_PageText", 7).lock();

    if (PageText)
    {
        PageText->SetText(TEXT("1 / 1"));
        PageText->SetFontSize(18.f);
        PageText->SetAlignH(ETextAlignH::Center);
        PageText->SetAlignV(ETextAlignV::Middle);
        PageText->SetTextColor(220, 220, 220, 255);
        mPageText = PageText;
    }

    auto CloseButton = CreateWidget<CButton>("EdictMenu_CloseButton", 7).lock();

    if (CloseButton)
    {
        ConfigureDefaultButtonStyle(CloseButton);
        CloseButton->SetEventCallback<CEdictWidget>(
            EButtonEventState::Click, this,
            &CEdictWidget::OnCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "EdictMenu_CloseText", mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            CloseText->SetFontSize(18.f);
            CloseText->SetAlignH(ETextAlignH::Center);
            CloseText->SetAlignV(ETextAlignV::Middle);
            CloseText->SetTextColor(255, 255, 255, 255);
            CloseButton->SetChild(CloseText);
        }

        mCloseButton = CloseButton;
    }

    auto PrevPageButton = CreateWidget<CButton>("EdictMenu_PrevPage", 7).lock();

    if (PrevPageButton)
    {
        ConfigureDefaultButtonStyle(PrevPageButton);
        PrevPageButton->SetEventCallback<CEdictWidget>(
            EButtonEventState::Click, this,
            &CEdictWidget::OnPrevPageClick);

        auto PrevText = CWidget::CreateStaticWidget<CTextBlock>(
            "EdictMenu_PrevText", mWorld);

        if (PrevText)
        {
            PrevText->SetText(TEXT("<"));
            PrevText->SetFontSize(20.f);
            PrevText->SetAlignH(ETextAlignH::Center);
            PrevText->SetAlignV(ETextAlignV::Middle);
            PrevText->SetTextColor(255, 255, 255, 255);
            PrevPageButton->SetChild(PrevText);
        }

        mPrevPageButton = PrevPageButton;
    }

    auto NextPageButton = CreateWidget<CButton>("EdictMenu_NextPage", 7).lock();

    if (NextPageButton)
    {
        ConfigureDefaultButtonStyle(NextPageButton);
        NextPageButton->SetEventCallback<CEdictWidget>(
            EButtonEventState::Click, this,
            &CEdictWidget::OnNextPageClick);

        auto NextText = CWidget::CreateStaticWidget<CTextBlock>(
            "EdictMenu_NextText", mWorld);

        if (NextText)
        {
            NextText->SetText(TEXT(">"));
            NextText->SetFontSize(20.f);
            NextText->SetAlignH(ETextAlignH::Center);
            NextText->SetAlignV(ETextAlignV::Middle);
            NextText->SetTextColor(255, 255, 255, 255);
            NextPageButton->SetChild(NextText);
        }

        mNextPageButton = NextPageButton;
    }

    mCategoryButtons.resize(GEdictCategoryCount);
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

        ConfigureCategoryTabButtonStyle(Button, false);
        ApplyTextureToAllButtonStates(
            Button,
            "EdictCategoryTabBackground",
            GCategoryTabBackgroundTexture);
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
        }

        mCategoryButtons[i] = Button;
    }

    mEdictButtons.resize(GEdictSlotsPerPage);
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
    for (int i = 0; i < GEdictSlotsPerPage; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "EdictMenu_Slot_" + std::to_string(i + 1), 7).lock();

        if (!Button)
            continue;

        ConfigureIconSlotButtonStyle(Button);
        Button->SetEventCallback<CEdictWidget>(
            EButtonEventState::Click, this, SlotCallbacks[i]);

        auto ButtonText = CWidget::CreateStaticWidget<CTextBlock>(
            "EdictMenu_SlotText_" + std::to_string(i + 1), mWorld);

        if (ButtonText)
        {
            ButtonText->SetText(TEXT("-"));
            ButtonText->SetFontSize(12.f);
            ButtonText->SetAlignH(ETextAlignH::Center);
            ButtonText->SetAlignV(ETextAlignV::Bottom);
            ButtonText->SetTextColor(240, 240, 240, 255);
            ButtonText->EnableShadow(true);
            ButtonText->SetShadowOffset(1.f, 1.f);
            ButtonText->SetShadowTextColor(16, 16, 16, 220);
            Button->SetChild(ButtonText);
        }

        mEdictButtons[i] = Button;
        mEdictButtonTexts[i] = ButtonText;
    }

    auto DetailTitleText =
        CreateWidget<CTextBlock>("EdictMenu_DetailTitle", 7).lock();

    if (DetailTitleText)
    {
        DetailTitleText->SetText(TEXT("칙령 정보"));
        DetailTitleText->SetFontSize(20.f);
        DetailTitleText->SetAlignH(ETextAlignH::Left);
        DetailTitleText->SetAlignV(ETextAlignV::Middle);
        DetailTitleText->SetTextColor(235, 235, 235, 255);
        DetailTitleText->EnableShadow(true);
        DetailTitleText->SetShadowOffset(1.f, 1.f);
        DetailTitleText->SetShadowTextColor(18, 18, 18, 220);
        mDetailTitleText = DetailTitleText;
    }

    auto FeedbackText =
        CreateWidget<CTextBlock>("EdictMenu_FeedbackText", 7).lock();

    if (FeedbackText)
    {
        FeedbackText->SetText(TEXT(""));
        FeedbackText->SetFontSize(12.f);
        FeedbackText->SetAlignH(ETextAlignH::Left);
        FeedbackText->SetAlignV(ETextAlignV::Middle);
        FeedbackText->SetTextColor(255, 230, 150, 255);
        FeedbackText->EnableShadow(true);
        FeedbackText->SetShadowOffset(1.f, 1.f);
        FeedbackText->SetShadowTextColor(16, 16, 16, 220);
        mFeedbackText = FeedbackText;
    }

    auto DetailBodyText =
        CreateWidget<CTextBlock>("EdictMenu_DetailBody", 7).lock();

    if (DetailBodyText)
    {
        DetailBodyText->SetText(TEXT("칙령을 클릭하면 상세 정보가 표시됩니다."));
        DetailBodyText->SetFontSize(13.f);
        DetailBodyText->SetAlignH(ETextAlignH::Left);
        DetailBodyText->SetAlignV(ETextAlignV::Top);
        DetailBodyText->SetTextColor(230, 230, 230, 255);
        DetailBodyText->EnableShadow(true);
        DetailBodyText->SetShadowOffset(1.f, 1.f);
        DetailBodyText->SetShadowTextColor(16, 16, 16, 220);
        mDetailBodyText = DetailBodyText;
    }

    auto ApplyButton = CreateWidget<CButton>("EdictMenu_ApplyButton", 7).lock();

    if (ApplyButton)
    {
        ConfigureHighlightedButtonStyle(ApplyButton);
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
            ApplyButtonText->SetTextColor(255, 255, 255, 255);
            ApplyButtonText->EnableShadow(true);
            ApplyButtonText->SetShadowOffset(1.f, 1.f);
            ApplyButtonText->SetShadowTextColor(40, 40, 40, 255);
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
        TaxPolicyTitleText->SetFontSize(15.f);
        TaxPolicyTitleText->SetAlignH(ETextAlignH::Left);
        TaxPolicyTitleText->SetAlignV(ETextAlignV::Middle);
        TaxPolicyTitleText->SetTextColor(235, 235, 235, 255);
        TaxPolicyTitleText->EnableShadow(true);
        TaxPolicyTitleText->SetShadowOffset(1.f, 1.f);
        TaxPolicyTitleText->SetShadowTextColor(16, 16, 16, 220);
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
        TaxPolicySummaryText->SetTextColor(214, 214, 214, 255);
        TaxPolicySummaryText->EnableShadow(true);
        TaxPolicySummaryText->SetShadowOffset(1.f, 1.f);
        TaxPolicySummaryText->SetShadowTextColor(16, 16, 16, 220);
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
            RowText->SetTextColor(232, 232, 232, 255);
            RowText->EnableShadow(true);
            RowText->SetShadowOffset(1.f, 1.f);
            RowText->SetShadowTextColor(16, 16, 16, 220);
            mTaxPolicyRowTexts[i] = RowText;
        }

        auto DecreaseButton = CreateWidget<CButton>(
            "EdictMenu_TaxDown_" + std::to_string(i + 1), 7).lock();

        if (DecreaseButton)
        {
            ConfigureDefaultButtonStyle(DecreaseButton);
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
                DecreaseText->SetTextColor(255, 255, 255, 255);
                DecreaseButton->SetChild(DecreaseText);
            }

            mTaxDecreaseButtons[i] = DecreaseButton;
        }

        auto IncreaseButton = CreateWidget<CButton>(
            "EdictMenu_TaxUp_" + std::to_string(i + 1), 7).lock();

        if (IncreaseButton)
        {
            ConfigureDefaultButtonStyle(IncreaseButton);
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
                IncreaseText->SetTextColor(255, 255, 255, 255);
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
    RefreshData();
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
}

void CEdictWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(320.f, ScreenWidth - 80.f);
    const float AvailableHeight = (std::max)(320.f, ScreenHeight - 180.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(AvailableWidth / mPanelWidth,
                AvailableHeight / mPanelHeight));
    const float PanelWidth = mPanelWidth * Scale;
    const float PanelHeight = mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;
    const float HorizontalMargin = 24.f * Scale;
    const float ContentWidth = PanelWidth - HorizontalMargin * 2.f;
    const float HeaderTopPadding = 14.f * Scale;
    const float HeaderHeight = 36.f * Scale;

    auto MenuBackground = mMenuBackground.lock();

    if (MenuBackground)
    {
        MenuBackground->SetPos(PanelLeft, PanelTop);
        MenuBackground->SetSize(PanelWidth, PanelHeight);
    }

    auto TitleText = mTitleText.lock();

    if (TitleText)
    {
        TitleText->SetPos(
            PanelLeft + HorizontalMargin,
            PanelTop + HeaderTopPadding);
        TitleText->SetSize(280.f * Scale, HeaderHeight);
    }

    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();
    auto PageText = mPageText.lock();
    auto CloseButton = mCloseButton.lock();

    if (CloseButton)
    {
        CloseButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 36.f * Scale,
            PanelTop + HeaderTopPadding);
        CloseButton->SetSize(36.f * Scale, HeaderHeight);
    }

    if (PrevPageButton)
    {
        PrevPageButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 176.f * Scale,
            PanelTop + HeaderTopPadding);
        PrevPageButton->SetSize(36.f * Scale, HeaderHeight);
    }

    if (PageText)
    {
        PageText->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 136.f * Scale,
            PanelTop + HeaderTopPadding);
        PageText->SetSize(56.f * Scale, HeaderHeight);
    }

    if (NextPageButton)
    {
        NextPageButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 76.f * Scale,
            PanelTop + HeaderTopPadding);
        NextPageButton->SetSize(36.f * Scale, HeaderHeight);
    }

    const float CategoryTop = PanelTop - 34.f * Scale;
    const float CategoryGap = 8.f * Scale;
    const float CategoryWidth = 64.f * Scale;
    const float CategoryHeight = 64.f * Scale;
    const float CategoryStartX = PanelLeft + 12.f * Scale;

    for (int i = 0; i < static_cast<int>(mCategoryButtons.size()); ++i)
    {
        auto CategoryButton = mCategoryButtons[i].lock();

        if (!CategoryButton)
            continue;

        CategoryButton->SetPos(
            CategoryStartX +
            (CategoryWidth + CategoryGap) * static_cast<float>(i),
            CategoryTop);
        CategoryButton->SetSize(CategoryWidth, CategoryHeight);
    }

    const float SlotGapX = 12.f * Scale;
    const float SlotGapY = 12.f * Scale;
    const float SlotTop =
        PanelTop + HeaderTopPadding + HeaderHeight + 52.f * Scale;
    const float DetailGap = 10.f * Scale;
    const float DetailTopPadding = 4.f * Scale;
    const float DetailBottomPadding = 12.f * Scale;
    const float DetailTitleHeight = 28.f * Scale;
    const float FeedbackHeight = 20.f * Scale;
    const float DetailHeight = 152.f * Scale;
    const float SlotBottom =
        PanelTop + PanelHeight - DetailHeight - DetailGap;
    const float SlotAreaHeight = (std::max)(
        96.f * Scale,
        SlotBottom - SlotTop);
    const float SlotWidth =
        (ContentWidth - SlotGapX * (GEdictSlotColumnCount - 1)) /
        static_cast<float>(GEdictSlotColumnCount);
    const float SlotHeight =
        (SlotAreaHeight - SlotGapY * (GEdictSlotRowCount - 1)) /
        static_cast<float>(GEdictSlotRowCount);

    for (int i = 0; i < static_cast<int>(mEdictButtons.size()); ++i)
    {
        const int Row = i / GEdictSlotColumnCount;
        const int Col = i % GEdictSlotColumnCount;
        auto SlotButton = mEdictButtons[i].lock();

        if (!SlotButton)
            continue;

        SlotButton->SetPos(
            PanelLeft + HorizontalMargin +
            (SlotWidth + SlotGapX) * static_cast<float>(Col),
            SlotTop + (SlotHeight + SlotGapY) * static_cast<float>(Row));
        SlotButton->SetSize(SlotWidth, SlotHeight);
    }

    const float DetailTop = SlotBottom + DetailTopPadding;
    const float TaxColumnWidth = 178.f * Scale;
    const float DetailColumnGap = 14.f * Scale;
    const float ApplyButtonWidth = TaxColumnWidth;
    const float ApplyButtonHeight = 32.f * Scale;
    const float DetailTitleWidth =
        (std::max)(120.f, ContentWidth - TaxColumnWidth - DetailColumnGap);
    const float TaxColumnLeft =
        PanelLeft + HorizontalMargin + DetailTitleWidth + DetailColumnGap;
    const float TaxButtonWidth = 26.f * Scale;
    const float TaxButtonHeight = 22.f * Scale;
    const float TaxRowHeight = 24.f * Scale;

    auto DetailTitleText = mDetailTitleText.lock();
    auto FeedbackText = mFeedbackText.lock();
    auto DetailBodyText = mDetailBodyText.lock();
    auto ApplyButton = mApplyButton.lock();
    auto TaxPolicyTitleText = mTaxPolicyTitleText.lock();
    auto TaxPolicySummaryText = mTaxPolicySummaryText.lock();

    if (DetailTitleText)
    {
        DetailTitleText->SetPos(PanelLeft + HorizontalMargin, DetailTop);
        DetailTitleText->SetSize(DetailTitleWidth, DetailTitleHeight);
    }

    if (ApplyButton)
    {
        ApplyButton->SetPos(TaxColumnLeft, DetailTop - 1.f * Scale);
        ApplyButton->SetSize(ApplyButtonWidth, ApplyButtonHeight);
    }

    if (FeedbackText)
    {
        FeedbackText->SetPos(
            PanelLeft + HorizontalMargin,
            DetailTop + DetailTitleHeight);
        FeedbackText->SetSize(DetailTitleWidth, FeedbackHeight);
    }

    if (DetailBodyText)
    {
        DetailBodyText->SetPos(
            PanelLeft + HorizontalMargin,
            DetailTop + DetailTitleHeight + FeedbackHeight);
        DetailBodyText->SetSize(
            DetailTitleWidth,
            PanelTop + PanelHeight -
            (DetailTop + DetailTitleHeight + FeedbackHeight) -
            DetailBottomPadding);
    }

    if (TaxPolicyTitleText)
    {
        TaxPolicyTitleText->SetPos(
            TaxColumnLeft,
            DetailTop + ApplyButtonHeight + 6.f * Scale);
        TaxPolicyTitleText->SetSize(TaxColumnWidth, 18.f * Scale);
    }

    for (int i = 0; i < GTaxPolicyRowCount; ++i)
    {
        const float RowTop =
            DetailTop + ApplyButtonHeight + 26.f * Scale +
            TaxRowHeight * static_cast<float>(i);
        auto RowText = mTaxPolicyRowTexts[i].lock();
        auto DecreaseButton = mTaxDecreaseButtons[i].lock();
        auto IncreaseButton = mTaxIncreaseButtons[i].lock();

        if (RowText)
        {
            RowText->SetPos(TaxColumnLeft, RowTop);
            RowText->SetSize(
                TaxColumnWidth - TaxButtonWidth * 2.f - 10.f * Scale,
                TaxRowHeight);
        }

        if (DecreaseButton)
        {
            DecreaseButton->SetPos(
                TaxColumnLeft + TaxColumnWidth - TaxButtonWidth * 2.f - 6.f * Scale,
                RowTop + (TaxRowHeight - TaxButtonHeight) * 0.5f);
            DecreaseButton->SetSize(TaxButtonWidth, TaxButtonHeight);
        }

        if (IncreaseButton)
        {
            IncreaseButton->SetPos(
                TaxColumnLeft + TaxColumnWidth - TaxButtonWidth,
                RowTop + (TaxRowHeight - TaxButtonHeight) * 0.5f);
            IncreaseButton->SetSize(TaxButtonWidth, TaxButtonHeight);
        }
    }

    if (TaxPolicySummaryText)
    {
        TaxPolicySummaryText->SetPos(
            TaxColumnLeft,
            DetailTop + ApplyButtonHeight + 102.f * Scale);
        TaxPolicySummaryText->SetSize(
            TaxColumnWidth,
            PanelTop + PanelHeight -
            (DetailTop + ApplyButtonHeight + 102.f * Scale) -
            DetailBottomPadding);
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
    auto TitleText = mTitleText.lock();
    auto PageText = mPageText.lock();
    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailBodyText = mDetailBodyText.lock();
    auto FeedbackText = mFeedbackText.lock();
    auto TaxPolicyTitleText = mTaxPolicyTitleText.lock();
    auto TaxPolicySummaryText = mTaxPolicySummaryText.lock();
    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();
    auto CloseButton = mCloseButton.lock();
    auto ApplyButton = mApplyButton.lock();

    if (MenuBackground)
        MenuBackground->SetEnable(mOpen);
    if (TitleText)
        TitleText->SetEnable(mOpen);
    if (PageText)
        PageText->SetEnable(mOpen);
    if (DetailTitleText)
        DetailTitleText->SetEnable(mOpen);
    if (DetailBodyText)
        DetailBodyText->SetEnable(mOpen);
    if (FeedbackText)
        FeedbackText->SetEnable(mOpen);
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

    for (size_t i = 0; i < mCategoryButtons.size(); ++i)
    {
        auto Button = mCategoryButtons[i].lock();

        if (Button)
            Button->SetEnable(mOpen);
    }

    for (size_t i = 0; i < mEdictButtons.size(); ++i)
    {
        auto Button = mEdictButtons[i].lock();

        if (Button)
            Button->SetEnable(mOpen);
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

    for (int i = 0; i < GEdictSlotsPerPage; ++i)
    {
        const int CategoryListIndex = BeginIndex + i;
        auto Button = mEdictButtons[i].lock();
        auto ButtonText = mEdictButtonTexts[i].lock();

        if (!Button || !ButtonText)
            continue;

        if (CategoryListIndex >= 0 && CategoryListIndex < EntryCount)
        {
            const int EntryIndex = CategoryEntries[CategoryListIndex];
            const auto& Definition = Definitions[EntryIndex];
            const FGovernmentEdictState* State = nullptr;

            if (MainWorld)
                State = MainWorld->GetGovernmentEdictState(Definition.Type);

            const bool Active =
                State && State->Active;
            const bool CoolingDown =
                State &&
                !State->Active &&
                State->CooldownDays > 0;
            const bool Selected =
                mSelectedEntryIndex == EntryIndex;

            mVisibleEntryIndices[i] = EntryIndex;
            Button->ButtonEnable(true);
            ConfigureEdictSlotButtonStyle(
                Button, Selected, Active, CoolingDown);

            std::wstring SlotLabel = Definition.DisplayName;
            SlotLabel += L"\n";
            SlotLabel += BuildEdictSlotStatusText(Definition, State);
            ButtonText->SetText(SlotLabel.c_str());

            if (Definition.IconPath)
            {
                ApplyTextureToAllButtonStates(
                    Button,
                    "EdictSlotIcon_" + std::to_string(EntryIndex),
                    Definition.IconPath);
            }
            else
            {
                ApplyTextureToAllButtonStates(
                    Button,
                    "EdictSlotEmptyTexture",
                    GEmptySlotTexture);
            }
        }
        else
        {
            Button->ButtonEnable(false);
            ButtonText->SetText(TEXT("-"));
            ConfigureIconSlotButtonStyle(Button);
            ApplyTextureToAllButtonStates(
                Button,
                "EdictSlotEmptyTexture",
                GEmptySlotTexture);
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

    bool HasSelectionOnPage = false;

    if (mSelectedEntryIndex >= 0)
    {
        for (int i = 0; i < static_cast<int>(mVisibleEntryIndices.size()); ++i)
        {
            if (mVisibleEntryIndices[i] == mSelectedEntryIndex)
            {
                HasSelectionOnPage = true;
                break;
            }
        }
    }

    if (!HasSelectionOnPage)
        mSelectedEntryIndex = -1;

    mPreviewEntryIndex = mSelectedEntryIndex;

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
    RefreshEdictButtons();
}

void CEdictWidget::PreviewSlot(int SlotIndex)
{
    (void)SlotIndex;
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
    RefreshDetailPanel();
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
    auto DetailBodyText = mDetailBodyText.lock();
    auto FeedbackText = mFeedbackText.lock();
    auto ApplyButton = mApplyButton.lock();
    auto ApplyButtonText = mApplyButtonText.lock();

    if (FeedbackText)
        FeedbackText->SetText(mFeedbackMessage.c_str());

    const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();

    const int DetailEntryIndex = mSelectedEntryIndex;

    if (DetailEntryIndex < 0 ||
        DetailEntryIndex >= static_cast<int>(Definitions.size()))
    {
        if (DetailTitleText)
            DetailTitleText->SetText(TEXT("칙령 정보"));
        if (DetailBodyText)
            DetailBodyText->SetText(TEXT("칸을 클릭해 칙령을 선택하면 상세 정보가 표시됩니다.\n슬롯 하단의 상태 텍스트로 활성/대기 여부를 바로 확인할 수 있습니다.\n우측 하단의 세금 정책은 칙령과 별도로 즉시 조정할 수 있습니다."));
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
    ETaxPolicyEventType RequiredTaxEvent = ETaxPolicyEventType::None;
    long long ActivationCost = Definition.BaseCost;
    long long Budget = 0;
    int ActiveCitizenCount = 0;

    if (MainWorld)
    {
        State = MainWorld->GetGovernmentEdictState(Definition.Type);
        Budget = MainWorld->GetNationalBudget();
        ActiveCitizenCount = (std::max)(
            0, MainWorld->GetPoliticalSnapshot().ActiveCitizenCount);
        ActivationCost = EdictSystem::ResolveEdictActivationCost(
            Definition, ActiveCitizenCount);
    }

    RequiredTaxEvent = ResolveRequiredTaxPolicyEvent(Definition.Type);

    bool CanApply = true;
    std::wstring ApplyLabel = L"시행";
    std::wstring Body = Definition.Summary;

    Body += L"\n\n";
    Body += Definition.EffectText;
    Body += L"\n\n시행 비용: ";
    Body += FormatCurrency(ActivationCost);

    if (Definition.MonthlyUpkeep > 0)
    {
        Body += L"\n월 유지비: ";
        Body += FormatCurrency(Definition.MonthlyUpkeep);
    }

    if (Definition.Mode == EGovernmentEdictMode::Active)
    {
        Body += L"\n지속 기간: ";
        Body += std::to_wstring((std::max)(1, Definition.DurationDays) / 30);
        Body += L"개월";
    }

    if (Definition.CooldownDays > 0)
    {
        Body += L"\n재사용 대기: ";
        Body += std::to_wstring((std::max)(1, Definition.CooldownDays) / 30);
        Body += L"개월";
    }

    if (State)
    {
        if (Definition.Mode == EGovernmentEdictMode::Passive)
        {
            if (State->Active)
            {
                Body += L"\n현재 상태: 활성";
                ApplyLabel = L"해제";
            }
            else
            {
                Body += L"\n현재 상태: 비활성";
            }
        }
        else if (State->Active)
        {
            Body += L"\n현재 상태: 시행 중 (";
            Body += std::to_wstring(State->RemainingDays);
            Body += L"일 남음)";
            ApplyLabel = L"시행 중";
            CanApply = false;
        }
        else if (State->CooldownDays > 0)
        {
            Body += L"\n현재 상태: 재사용 대기 (";
            Body += std::to_wstring(State->CooldownDays);
            Body += L"일 남음)";
            ApplyLabel = L"대기 중";
            CanApply = false;
        }
        else
        {
            Body += L"\n현재 상태: 사용 가능";
        }
    }

    if (MainWorld && RequiredTaxEvent != ETaxPolicyEventType::None)
    {
        const FTaxPolicyEventStatus& TaxEventStatus =
            MainWorld->GetTaxPolicyEventStatus();

        Body += L"\n대응 사건: ";
        Body += GetTaxPolicyEventDisplayName(RequiredTaxEvent);

        if (TaxEventStatus.Active &&
            TaxEventStatus.Type == RequiredTaxEvent)
        {
            Body += L"\n현재 사건: 대응 가능 (";
            Body += std::to_wstring((std::max)(0, TaxEventStatus.RemainingDays));
            Body += L"일 남음)";
        }
        else if (TaxEventStatus.Active)
        {
            Body += L"\n현재 사건: ";
            Body += TaxEventStatus.Title;
            Body += L" (대상 아님)";
        }
        else
        {
            Body += L"\n현재 사건: 없음";
        }

        if (CanApply &&
            !(TaxEventStatus.Active &&
                TaxEventStatus.Type == RequiredTaxEvent))
        {
            ApplyLabel = L"사건 필요";
            CanApply = false;
        }
    }

    if (!State && !MainWorld)
        CanApply = false;

    if (ApplyLabel == L"시행" && Budget < ActivationCost)
    {
        ApplyLabel = L"예산 부족";
        CanApply = false;
    }

    if (DetailBodyText)
        DetailBodyText->SetText(Body.c_str());

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
    const int EntryIndex = mSelectedEntryIndex;

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
