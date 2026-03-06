#include "BuildMenuWidget.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementController.h"
#include "../World/MainWorld.h"
#include "../ObjectNames.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/ProgressBar.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <string>

namespace
{
    constexpr int CategoryCount = 8;
    constexpr int SlotsPerPage = 12;
    constexpr int SlotColumnCount = 4;
    constexpr int SlotRowCount = 3;
    constexpr const TCHAR* GBuildMenuPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\5_MainMenu\\CenterPopUp\\T_center_popUp.png");
    constexpr const TCHAR* GYearbookPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\4_Modern\\CenterPopUp\\T_center_popUp.png");
    constexpr const TCHAR* GEmptySlotTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_standard.png");
    constexpr const TCHAR* GCategoryTabBackgroundTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\IconBackground\\T_icon_background.png");

    EPlacementTemplateType ResolveTemplateTypeByBuildingId(
        const std::string& BuildingId)
    {
        // 건물별 배치 크기/형태를 한 곳에서 관리하기 위한 룰 테이블.
        // 필요한 건물 ID를 여기에 추가하면 된다.
        struct FTemplateRule
        {
            const char* BuildingId;
            EPlacementTemplateType TemplateType;
        };

        static const std::vector<FTemplateRule> GRules =
        {
            // 예시:
            // { "build_1_3", EPlacementTemplateType::Diamond5x5TwoMarker },
            // { "build_4_3", EPlacementTemplateType::Diamond5x5FourMarker },
            // { "build_7_5", EPlacementTemplateType::Diamond7x7ThreeMarker },
        };

        for (const FTemplateRule& Rule : GRules)
        {
            if (BuildingId == Rule.BuildingId)
                return Rule.TemplateType;
        }

        // 룰이 없으면 기존과 동일하게 3x3 템플릿 사용.
        return EPlacementTemplateType::Diamond3x3SingleMarker;
    }

    const wchar_t* CategoryLabels[CategoryCount] =
    {
        L"교통 및 기반시설",
        L"음식 및 자원",
        L"산업",
        L"주거지",
        L"오락",
        L"미디어 및 교육",
        L"관광업",
        L"공익 서비스"
    };

    const TCHAR* const GCategoryTabIcons[CategoryCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_traffic.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_raw_resources.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_industry.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_housing.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_entertainment.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_mediaEducation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_tourism.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_publicServices.png")
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
}

CBuildMenuWidget::CBuildMenuWidget()
{
}

CBuildMenuWidget::~CBuildMenuWidget()
{
}

bool CBuildMenuWidget::Init()
{
    CWidgetContainer::Init();

    mVisibleEntryIndices.assign(SlotsPerPage, -1);

    auto BuildButton = CreateWidget<CButton>("BuildMenu_BuildButton", 10).lock();

    if (BuildButton)
    {
        ConfigureHighlightedButtonStyle(BuildButton);
        BuildButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnBuildButtonClick);

        auto BuildButtonText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_BuildButtonText", mWorld);

        if (BuildButtonText)
        {
            BuildButtonText->SetText(TEXT("건축"));
            BuildButtonText->SetFontSize(24.f);
            BuildButtonText->SetAlignH(ETextAlignH::Center);
            BuildButtonText->SetAlignV(ETextAlignV::Middle);
            BuildButtonText->SetTextColor(255, 255, 255, 255);
            BuildButtonText->EnableShadow(true);
            BuildButtonText->SetShadowOffset(1.f, 1.f);
            BuildButtonText->SetShadowTextColor(40, 40, 40, 255);
            BuildButton->SetChild(BuildButtonText);
        }

        mBuildButton = BuildButton;
    }

    auto YearbookButton =
        CreateWidget<CButton>("BuildMenu_YearbookButton", 10).lock();

    if (YearbookButton)
    {
        ConfigureDefaultButtonStyle(YearbookButton);
        YearbookButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnYearbookButtonClick);

        auto YearbookButtonText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_YearbookButtonText", mWorld);

        if (YearbookButtonText)
        {
            YearbookButtonText->SetText(TEXT("연감"));
            YearbookButtonText->SetFontSize(24.f);
            YearbookButtonText->SetAlignH(ETextAlignH::Center);
            YearbookButtonText->SetAlignV(ETextAlignV::Middle);
            YearbookButtonText->SetTextColor(255, 255, 255, 255);
            YearbookButtonText->EnableShadow(true);
            YearbookButtonText->SetShadowOffset(1.f, 1.f);
            YearbookButtonText->SetShadowTextColor(40, 40, 40, 255);
            YearbookButton->SetChild(YearbookButtonText);
            mYearbookButtonText = YearbookButtonText;
        }

        mYearbookButton = YearbookButton;
    }

    auto NpcCountText = CreateWidget<CTextBlock>("BuildMenu_NpcCount", 11).lock();

    if (NpcCountText)
    {
        NpcCountText->SetText(TEXT("NPC: 0"));
        NpcCountText->SetFontSize(20.f);
        NpcCountText->SetAlignH(ETextAlignH::Left);
        NpcCountText->SetAlignV(ETextAlignV::Middle);
        NpcCountText->SetTextColor(245, 245, 245, 255);
        NpcCountText->EnableShadow(true);
        NpcCountText->SetShadowOffset(1.f, 1.f);
        NpcCountText->SetShadowTextColor(20, 20, 20, 255);
        mNpcCountText = NpcCountText;
    }

    auto BudgetText = CreateWidget<CTextBlock>(
        "BuildMenu_NationalBudget", 11).lock();

    if (BudgetText)
    {
        BudgetText->SetText(TEXT("국가 예산: $0"));
        BudgetText->SetFontSize(20.f);
        BudgetText->SetAlignH(ETextAlignH::Left);
        BudgetText->SetAlignV(ETextAlignV::Middle);
        BudgetText->SetTextColor(245, 245, 210, 255);
        BudgetText->EnableShadow(true);
        BudgetText->SetShadowOffset(1.f, 1.f);
        BudgetText->SetShadowTextColor(20, 20, 20, 255);
        mBudgetText = BudgetText;
    }

    auto DateText = CreateWidget<CTextBlock>(
        "BuildMenu_SimulationDate", 11).lock();

    if (DateText)
    {
        DateText->SetText(TEXT("날짜: 2000-01-01"));
        DateText->SetFontSize(20.f);
        DateText->SetAlignH(ETextAlignH::Left);
        DateText->SetAlignV(ETextAlignV::Middle);
        DateText->SetTextColor(220, 235, 255, 255);
        DateText->EnableShadow(true);
        DateText->SetShadowOffset(1.f, 1.f);
        DateText->SetShadowTextColor(20, 20, 20, 255);
        mDateText = DateText;
    }

    auto DayProgressText = CreateWidget<CTextBlock>(
        "BuildMenu_DayProgressText", 11).lock();

    if (DayProgressText)
    {
        DayProgressText->SetText(TEXT("월 진행: 0%"));
        DayProgressText->SetFontSize(16.f);
        DayProgressText->SetAlignH(ETextAlignH::Left);
        DayProgressText->SetAlignV(ETextAlignV::Middle);
        DayProgressText->SetTextColor(220, 220, 220, 255);
        DayProgressText->EnableShadow(true);
        DayProgressText->SetShadowOffset(1.f, 1.f);
        DayProgressText->SetShadowTextColor(20, 20, 20, 255);
        mDayProgressText = DayProgressText;
    }

    auto DayProgressBar = CreateWidget<CProgressBar>(
        "BuildMenu_DayProgressBar", 10).lock();

    if (DayProgressBar)
    {
        DayProgressBar->SetTint(
            EProgressBarImageType::Back,
            FVector4(0.08f, 0.08f, 0.10f, 0.85f));
        DayProgressBar->SetTint(
            EProgressBarImageType::Fill,
            FVector4(0.18f, 0.62f, 0.34f, 0.95f));
        DayProgressBar->SetPercent(0.f);
        DayProgressBar->SetBarDir(EProgressBarDir::RightToLeft);
        mDayProgressBar = DayProgressBar;
    }

    auto YearbookPanel = CreateWidget<CImage>("BuildMenu_YearbookPanel", 12).lock();

    if (YearbookPanel)
    {
        YearbookPanel->SetTexture("BuildMenuYearbookBackground",
            GYearbookPanelTexture);
        YearbookPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        mYearbookPanel = YearbookPanel;
    }

    auto YearbookTitleText =
        CreateWidget<CTextBlock>("BuildMenu_YearbookTitle", 13).lock();

    if (YearbookTitleText)
    {
        YearbookTitleText->SetText(TEXT("연감"));
        YearbookTitleText->SetFontSize(24.f);
        YearbookTitleText->SetAlignH(ETextAlignH::Left);
        YearbookTitleText->SetAlignV(ETextAlignV::Middle);
        YearbookTitleText->SetTextColor(245, 245, 245, 255);
        YearbookTitleText->EnableShadow(true);
        YearbookTitleText->SetShadowOffset(1.f, 1.f);
        YearbookTitleText->SetShadowTextColor(20, 20, 20, 255);
        mYearbookTitleText = YearbookTitleText;
    }

    auto YearbookBodyText =
        CreateWidget<CTextBlock>("BuildMenu_YearbookBody", 13).lock();

    if (YearbookBodyText)
    {
        YearbookBodyText->SetText(
            TEXT("종합 만족도: -\n"
                "음식: -\n"
                "보건: -\n"
                "유흥: -\n"
                "신앙: -\n"
                "주거: -\n"
                "직업: -\n"
                "자유: -\n"
                "치안: -\n"
                "무주택자 수: 0명\n"
                "실업자 수: 0명"));
        YearbookBodyText->SetFontSize(14.f);
        YearbookBodyText->SetAlignH(ETextAlignH::Left);
        YearbookBodyText->SetAlignV(ETextAlignV::Top);
        YearbookBodyText->SetTextColor(225, 225, 225, 255);
        YearbookBodyText->EnableShadow(true);
        YearbookBodyText->SetShadowOffset(1.f, 1.f);
        YearbookBodyText->SetShadowTextColor(20, 20, 20, 255);
        mYearbookBodyText = YearbookBodyText;
    }

    auto MenuBackground = CreateWidget<CImage>("BuildMenu_Background", 6).lock();

    if (MenuBackground)
    {
        MenuBackground->SetTexture("BuildMenuBackground",
            GBuildMenuPanelTexture);
        MenuBackground->SetTint(1.f, 1.f, 1.f, 1.f);
        mMenuBackground = MenuBackground;
    }

    auto TitleText = CreateWidget<CTextBlock>("BuildMenu_Title", 7).lock();

    if (TitleText)
    {
        TitleText->SetText(CategoryLabels[static_cast<int>(EBuildingCategory::Infrastructure)]);
        TitleText->SetFontSize(28.f);
        TitleText->SetAlignH(ETextAlignH::Center);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(245, 245, 245, 255);
        TitleText->EnableShadow(true);
        TitleText->SetShadowOffset(1.f, 1.f);
        TitleText->SetShadowTextColor(20, 20, 20, 255);
        mTitleText = TitleText;
    }

    auto PageText = CreateWidget<CTextBlock>("BuildMenu_PageText", 7).lock();

    if (PageText)
    {
        PageText->SetText(TEXT("1 / 1"));
        PageText->SetFontSize(18.f);
        PageText->SetAlignH(ETextAlignH::Center);
        PageText->SetAlignV(ETextAlignV::Middle);
        PageText->SetTextColor(220, 220, 220, 255);
        mPageText = PageText;
    }

    auto PrevPageButton = CreateWidget<CButton>("BuildMenu_PrevPage", 7).lock();

    if (PrevPageButton)
    {
        ConfigureDefaultButtonStyle(PrevPageButton);
        PrevPageButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnPrevPageClick);

        auto PrevText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_PrevText", mWorld);

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

    auto NextPageButton = CreateWidget<CButton>("BuildMenu_NextPage", 7).lock();

    if (NextPageButton)
    {
        ConfigureDefaultButtonStyle(NextPageButton);
        NextPageButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnNextPageClick);

        auto NextText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_NextText", mWorld);

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

    mCategoryButtons.resize(CategoryCount);
    void (CBuildMenuWidget::* CategoryCallbacks[CategoryCount])() =
    {
        &CBuildMenuWidget::OnCategoryInfrastructureClick,
        &CBuildMenuWidget::OnCategoryFoodResourceClick,
        &CBuildMenuWidget::OnCategoryIndustryClick,
        &CBuildMenuWidget::OnCategoryHousingClick,
        &CBuildMenuWidget::OnCategoryEntertainmentClick,
        &CBuildMenuWidget::OnCategoryMediaEducationClick,
        &CBuildMenuWidget::OnCategoryTourismClick,
        &CBuildMenuWidget::OnCategoryPublicServiceClick
    };

    for (int i = 0; i < CategoryCount; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "BuildMenu_Category_" + std::to_string(i + 1), 7).lock();

        if (!Button)
            continue;

        ConfigureCategoryTabButtonStyle(Button, false);
        ApplyTextureToAllButtonStates(
            Button,
            "BuildMenuCategoryTabBackground",
            GCategoryTabBackgroundTexture);
        Button->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this, CategoryCallbacks[i]);

        auto CategoryIcon = CWidget::CreateStaticWidget<CImage>(
            "BuildMenu_CategoryIcon_" + std::to_string(i + 1), mWorld);

        if (CategoryIcon)
        {
            CategoryIcon->SetTexture(
                "BuildMenuCategoryIconTex_" + std::to_string(i + 1),
                GCategoryTabIcons[i]);
            CategoryIcon->SetTint(1.f, 1.f, 1.f, 1.f);
            Button->SetChild(CategoryIcon);
        }

        mCategoryButtons[i] = Button;
    }

    mBuildingButtons.resize(SlotsPerPage);
    mBuildingButtonTexts.resize(SlotsPerPage);

    void (CBuildMenuWidget::* SlotCallbacks[SlotsPerPage])() =
    {
        &CBuildMenuWidget::OnSlot0Click,
        &CBuildMenuWidget::OnSlot1Click,
        &CBuildMenuWidget::OnSlot2Click,
        &CBuildMenuWidget::OnSlot3Click,
        &CBuildMenuWidget::OnSlot4Click,
        &CBuildMenuWidget::OnSlot5Click,
        &CBuildMenuWidget::OnSlot6Click,
        &CBuildMenuWidget::OnSlot7Click,
        &CBuildMenuWidget::OnSlot8Click,
        &CBuildMenuWidget::OnSlot9Click,
        &CBuildMenuWidget::OnSlot10Click,
        &CBuildMenuWidget::OnSlot11Click
    };
    void (CBuildMenuWidget::* SlotHoverCallbacks[SlotsPerPage])() =
    {
        &CBuildMenuWidget::OnSlot0Hovered,
        &CBuildMenuWidget::OnSlot1Hovered,
        &CBuildMenuWidget::OnSlot2Hovered,
        &CBuildMenuWidget::OnSlot3Hovered,
        &CBuildMenuWidget::OnSlot4Hovered,
        &CBuildMenuWidget::OnSlot5Hovered,
        &CBuildMenuWidget::OnSlot6Hovered,
        &CBuildMenuWidget::OnSlot7Hovered,
        &CBuildMenuWidget::OnSlot8Hovered,
        &CBuildMenuWidget::OnSlot9Hovered,
        &CBuildMenuWidget::OnSlot10Hovered,
        &CBuildMenuWidget::OnSlot11Hovered
    };

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "BuildMenu_Slot_" + std::to_string(i + 1), 7).lock();

        if (!Button)
            continue;

        ConfigureIconSlotButtonStyle(Button);
        Button->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this, SlotCallbacks[i]);
        Button->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Hovered, this, SlotHoverCallbacks[i]);

        auto ButtonText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_SlotText_" + std::to_string(i + 1), mWorld);

        if (ButtonText)
        {
            ButtonText->SetText(TEXT("-"));
            ButtonText->SetFontSize(14.f);
            ButtonText->SetAlignH(ETextAlignH::Center);
            ButtonText->SetAlignV(ETextAlignV::Bottom);
            ButtonText->SetTextColor(240, 240, 240, 255);
            ButtonText->EnableShadow(true);
            ButtonText->SetShadowOffset(1.f, 1.f);
            ButtonText->SetShadowTextColor(16, 16, 16, 220);
            Button->SetChild(ButtonText);
        }

        mBuildingButtons[i] = Button;
        mBuildingButtonTexts[i] = ButtonText;
    }

    auto DetailTitleText =
        CreateWidget<CTextBlock>("BuildMenu_DetailTitle", 7).lock();

    if (DetailTitleText)
    {
        DetailTitleText->SetText(TEXT("건물 정보"));
        DetailTitleText->SetFontSize(20.f);
        DetailTitleText->SetAlignH(ETextAlignH::Left);
        DetailTitleText->SetAlignV(ETextAlignV::Middle);
        DetailTitleText->SetTextColor(235, 235, 235, 255);
        DetailTitleText->EnableShadow(true);
        DetailTitleText->SetShadowOffset(1.f, 1.f);
        DetailTitleText->SetShadowTextColor(18, 18, 18, 220);
        mDetailTitleText = DetailTitleText;
    }

    auto DetailBodyText =
        CreateWidget<CTextBlock>("BuildMenu_DetailBody", 7).lock();

    if (DetailBodyText)
    {
        DetailBodyText->SetText(TEXT("건물 아이콘에 마우스를 올리면 상세 정보가 표시됩니다."));
        DetailBodyText->SetFontSize(13.f);
        DetailBodyText->SetAlignH(ETextAlignH::Left);
        DetailBodyText->SetAlignV(ETextAlignV::Top);
        DetailBodyText->SetTextColor(230, 230, 230, 255);
        DetailBodyText->EnableShadow(true);
        DetailBodyText->SetShadowOffset(1.f, 1.f);
        DetailBodyText->SetShadowTextColor(16, 16, 16, 220);
        mDetailBodyText = DetailBodyText;
    }

    if (BuildButton)
        BuildButton->SetEnable(false);
    if (YearbookButton)
        YearbookButton->SetEnable(false);
    if (NpcCountText)
        NpcCountText->SetEnable(false);
    if (BudgetText)
        BudgetText->SetEnable(false);
    if (DateText)
        DateText->SetEnable(false);
    if (DayProgressText)
        DayProgressText->SetEnable(false);
    if (DayProgressBar)
        DayProgressBar->SetEnable(false);

    mMenuOpen = false;
    mYearbookOpen = false;
    SelectCategory(EBuildingCategory::Infrastructure);
    ApplyMenuOpenState();
    ApplyYearbookOpenState();
    RefreshNpcCountText();
    RefreshEconomyStatus();
    RefreshYearbookStatus();
    RefreshLayout();

    return true;
}

void CBuildMenuWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
    RefreshNpcCountText();
    RefreshEconomyStatus();
    RefreshYearbookStatus();
    RefreshLayout();
}

void CBuildMenuWidget::Render()
{
    CWidgetContainer::Render();
}

void CBuildMenuWidget::RefreshLayout()
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

    auto BuildButton = mBuildButton.lock();
    auto YearbookButton = mYearbookButton.lock();

    const float BuildButtonWidth = 220.f;
    const float YearbookButtonWidth = 140.f;
    const float BottomButtonGap = 14.f;
    const float BottomButtonHeight = 58.f;
    const float BottomButtonTop =
        ScreenWidth * 0.5f -
        (BuildButtonWidth + BottomButtonGap + YearbookButtonWidth) * 0.5f;

    if (BuildButton)
    {
        BuildButton->SetPivot(0.f, 1.f);
        BuildButton->SetPos(BottomButtonTop, ScreenHeight - 20.f);
        BuildButton->SetSize(BuildButtonWidth, BottomButtonHeight);
    }

    if (YearbookButton)
    {
        YearbookButton->SetPivot(0.f, 1.f);
        YearbookButton->SetPos(
            BottomButtonTop + BuildButtonWidth + BottomButtonGap,
            ScreenHeight - 20.f);
        YearbookButton->SetSize(YearbookButtonWidth, BottomButtonHeight);
    }

    auto NpcCountText = mNpcCountText.lock();
    auto BudgetText = mBudgetText.lock();
    auto DateText = mDateText.lock();
    auto DayProgressText = mDayProgressText.lock();
    auto DayProgressBar = mDayProgressBar.lock();

    if (NpcCountText)
    {
        NpcCountText->SetPivot(0.f, 1.f);
        NpcCountText->SetPos(20.f, ScreenHeight - 20.f);
        NpcCountText->SetSize(380.f, 28.f);
    }

    if (BudgetText)
    {
        BudgetText->SetPivot(0.f, 1.f);
        BudgetText->SetPos(20.f, ScreenHeight - 48.f);
        BudgetText->SetSize(420.f, 28.f);
    }

    if (DateText)
    {
        DateText->SetPivot(0.f, 1.f);
        DateText->SetPos(20.f, ScreenHeight - 76.f);
        DateText->SetSize(420.f, 28.f);
    }

    if (DayProgressText)
    {
        DayProgressText->SetPivot(0.f, 1.f);
        DayProgressText->SetPos(20.f, ScreenHeight - 104.f);
        DayProgressText->SetSize(420.f, 24.f);
    }

    if (DayProgressBar)
    {
        DayProgressBar->SetPivot(0.f, 1.f);
        DayProgressBar->SetPos(20.f, ScreenHeight - 124.f);
        DayProgressBar->SetSize(300.f, 14.f);
    }

    auto YearbookPanel = mYearbookPanel.lock();
    auto YearbookTitleText = mYearbookTitleText.lock();
    auto YearbookBodyText = mYearbookBodyText.lock();

    const float YearbookPanelWidth = PanelWidth;
    const float YearbookPanelHeight = PanelHeight;
    const float YearbookLeft = PanelLeft;
    const float YearbookTop = PanelTop;
    const float YearbookBodyTop =
        YearbookTop + HeaderTopPadding + HeaderHeight + 14.f * Scale;
    const float YearbookBodyHeight = (std::max)(
        40.f, YearbookPanelHeight - (YearbookBodyTop - YearbookTop) - 16.f * Scale);

    if (YearbookPanel)
    {
        YearbookPanel->SetPos(YearbookLeft, YearbookTop);
        YearbookPanel->SetSize(YearbookPanelWidth, YearbookPanelHeight);
    }

    if (YearbookTitleText)
    {
        YearbookTitleText->SetPos(
            YearbookLeft + HorizontalMargin,
            YearbookTop + HeaderTopPadding);
        YearbookTitleText->SetSize(ContentWidth, HeaderHeight);
    }

    if (YearbookBodyText)
    {
        YearbookBodyText->SetPos(
            YearbookLeft + HorizontalMargin, YearbookBodyTop);
        YearbookBodyText->SetSize(ContentWidth, YearbookBodyHeight);
    }

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

    if (PrevPageButton)
    {
        PrevPageButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 136.f * Scale,
            PanelTop + HeaderTopPadding);
        PrevPageButton->SetSize(36.f * Scale, HeaderHeight);
    }

    if (PageText)
    {
        PageText->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 96.f * Scale,
            PanelTop + HeaderTopPadding);
        PageText->SetSize(56.f * Scale, HeaderHeight);
    }

    if (NextPageButton)
    {
        NextPageButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 36.f * Scale,
            PanelTop + HeaderTopPadding);
        NextPageButton->SetSize(36.f * Scale, HeaderHeight);
    }

    const float CategoryTop = PanelTop - 34.f * Scale;
    const float CategoryGap = 8.f * Scale;
    const float CategoryWidth = 64.f * Scale;
    const float CategoryHeight = 64.f * Scale;
    const float CategoryStartX = PanelLeft + 12.f * Scale;

    for (int i = 0; i < CategoryCount; ++i)
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
    const float DetailHeight = 152.f * Scale;
    const float SlotBottom =
        PanelTop + PanelHeight - DetailHeight - DetailGap;
    const float SlotAreaHeight = (std::max)(
        96.f * Scale,
        SlotBottom - SlotTop);
    const float SlotWidth =
        (ContentWidth - SlotGapX * (SlotColumnCount - 1)) /
        static_cast<float>(SlotColumnCount);
    const float SlotHeight =
        (SlotAreaHeight - SlotGapY * (SlotRowCount - 1)) /
        static_cast<float>(SlotRowCount);

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        const int Row = i / SlotColumnCount;
        const int Col = i % SlotColumnCount;

        auto SlotButton = mBuildingButtons[i].lock();

        if (!SlotButton)
            continue;

        SlotButton->SetPos(
            PanelLeft + HorizontalMargin +
            (SlotWidth + SlotGapX) * static_cast<float>(Col),
            SlotTop + (SlotHeight + SlotGapY) * static_cast<float>(Row));
        SlotButton->SetSize(SlotWidth, SlotHeight);
    }

    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailBodyText = mDetailBodyText.lock();
    const float DetailTop = SlotBottom + DetailTopPadding;

    if (DetailTitleText)
    {
        DetailTitleText->SetPos(PanelLeft + HorizontalMargin, DetailTop);
        DetailTitleText->SetSize(ContentWidth, DetailTitleHeight);
    }

    if (DetailBodyText)
    {
        DetailBodyText->SetPos(
            PanelLeft + HorizontalMargin,
            DetailTop + DetailTitleHeight);
        DetailBodyText->SetSize(
            ContentWidth,
            PanelTop + PanelHeight - (DetailTop + DetailTitleHeight) -
            DetailBottomPadding);
    }
}

void CBuildMenuWidget::ApplyMenuOpenState()
{
    auto MenuBackground = mMenuBackground.lock();
    auto TitleText = mTitleText.lock();
    auto PageText = mPageText.lock();
    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailBodyText = mDetailBodyText.lock();
    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();

    if (MenuBackground)
        MenuBackground->SetEnable(mMenuOpen);
    if (TitleText)
        TitleText->SetEnable(mMenuOpen);
    if (PageText)
        PageText->SetEnable(mMenuOpen);
    if (DetailTitleText)
        DetailTitleText->SetEnable(mMenuOpen);
    if (DetailBodyText)
        DetailBodyText->SetEnable(mMenuOpen);
    if (PrevPageButton)
        PrevPageButton->SetEnable(mMenuOpen);
    if (NextPageButton)
        NextPageButton->SetEnable(mMenuOpen);

    for (size_t i = 0; i < mCategoryButtons.size(); ++i)
    {
        auto Button = mCategoryButtons[i].lock();

        if (Button)
            Button->SetEnable(mMenuOpen);
    }

    for (size_t i = 0; i < mBuildingButtons.size(); ++i)
    {
        auto Button = mBuildingButtons[i].lock();

        if (Button)
            Button->SetEnable(mMenuOpen);
    }

}

void CBuildMenuWidget::ApplyYearbookOpenState()
{
    auto YearbookPanel = mYearbookPanel.lock();
    auto YearbookTitleText = mYearbookTitleText.lock();
    auto YearbookBodyText = mYearbookBodyText.lock();
    auto YearbookButton = mYearbookButton.lock();
    auto YearbookButtonText = mYearbookButtonText.lock();

    if (YearbookPanel)
        YearbookPanel->SetEnable(mYearbookOpen);
    if (YearbookTitleText)
        YearbookTitleText->SetEnable(mYearbookOpen);
    if (YearbookBodyText)
        YearbookBodyText->SetEnable(mYearbookOpen);

    if (YearbookButton)
    {
        if (mYearbookOpen)
            ConfigureHighlightedButtonStyle(YearbookButton);
        else
            ConfigureDefaultButtonStyle(YearbookButton);
    }

    if (YearbookButtonText)
    {
        if (mYearbookOpen)
            YearbookButtonText->SetText(TEXT("연감 ON"));
        else
            YearbookButtonText->SetText(TEXT("연감"));
    }

}

void CBuildMenuWidget::RefreshNpcCountText()
{
    auto NpcCountText = mNpcCountText.lock();
    auto World = mWorld.lock();

    if (!NpcCountText || !World)
        return;

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;
    int NpcCount = 0;

    if (World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
    {
        for (size_t i = 0; i < OrbList.size(); ++i)
        {
            auto Orb = OrbList[i].lock();

            if (Orb && Orb->GetAlive())
                ++NpcCount;
        }
    }

    wchar_t CountBuffer[64] = {};
    swprintf_s(CountBuffer, L"NPC: %d", NpcCount);
    NpcCountText->SetText(CountBuffer);
}

void CBuildMenuWidget::RefreshEconomyStatus()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);

    if (!MainWorld)
        return;

    auto BudgetText = mBudgetText.lock();
    auto DateText = mDateText.lock();
    auto DayProgressText = mDayProgressText.lock();
    auto DayProgressBar = mDayProgressBar.lock();

    if (BudgetText)
    {
        const std::wstring BudgetLabel =
            L"국가 예산: " +
            FormatCurrency(MainWorld->GetNationalBudget());
        BudgetText->SetText(BudgetLabel.c_str());
    }

    const int Year = MainWorld->GetSimulationYear();
    const int Month = MainWorld->GetSimulationMonth();
    const int Day = MainWorld->GetSimulationDay();
    const int MonthDays = MainWorld->GetSimulationMonthDayCount();
    const float MonthProgress = MainWorld->GetSimulationMonthProgress();

    if (DateText)
    {
        wchar_t DateBuffer[64] = {};
        swprintf_s(DateBuffer, L"날짜: %04d-%02d-%02d", Year, Month, Day);
        DateText->SetText(DateBuffer);
    }

    if (DayProgressBar)
        DayProgressBar->SetPercent(MonthProgress);

    if (DayProgressText)
    {
        wchar_t ProgressBuffer[96] = {};
        swprintf_s(
            ProgressBuffer,
            L"월 진행: %d%%  |  %d / %d일",
            static_cast<int>(roundf(MonthProgress * 100.f)),
            Day,
            MonthDays);
        DayProgressText->SetText(ProgressBuffer);
    }
}

void CBuildMenuWidget::RefreshYearbookStatus()
{
    auto YearbookBodyText = mYearbookBodyText.lock();
    auto World = mWorld.lock();

    if (!YearbookBodyText || !World)
        return;

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;
    int ActiveNpcCount = 0;
    int HomelessCount = 0;
    int UnemployedCount = 0;
    int PoliticalCount[static_cast<int>(EPoliticalAxis::Count)][3] = {};
    double FoodSum = 0.0;
    double HealthSum = 0.0;
    double FunSum = 0.0;
    double FaithSum = 0.0;
    double HousingSum = 0.0;
    double JobSum = 0.0;
    double FreedomSum = 0.0;
    double SecuritySum = 0.0;
    double OverallSatisfactionSum = 0.0;

    if (World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
    {
        for (size_t i = 0; i < OrbList.size(); ++i)
        {
            auto Orb = OrbList[i].lock();

            if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                continue;

            const FNpcSatisfaction& Satisfaction = Orb->GetSatisfaction();
            const FNpcPoliticalProfile& Political = Orb->GetPoliticalProfile();
            ++ActiveNpcCount;
            FoodSum += static_cast<double>(Satisfaction.Food);
            HealthSum += static_cast<double>(Satisfaction.Health);
            FunSum += static_cast<double>(Satisfaction.Fun);
            FaithSum += static_cast<double>(Satisfaction.Faith);
            HousingSum += static_cast<double>(Satisfaction.Housing);
            JobSum += static_cast<double>(Satisfaction.Job);
            FreedomSum += static_cast<double>(Satisfaction.Freedom);
            SecuritySum += static_cast<double>(Satisfaction.Security);
            OverallSatisfactionSum += static_cast<double>(Satisfaction.Overall);

            if (Orb->GetHomeBuilding().empty())
                ++HomelessCount;

            if (Orb->GetWorkBuilding().empty())
                ++UnemployedCount;

            for (int AxisIndex = 0;
                AxisIndex < static_cast<int>(EPoliticalAxis::Count);
                ++AxisIndex)
            {
                const EPoliticalAxis Axis =
                    static_cast<EPoliticalAxis>(AxisIndex);
                const int StanceIndex =
                    static_cast<int>(Political.Get(Axis).Stance);

                if (StanceIndex >= 0 && StanceIndex < 3)
                    ++PoliticalCount[AxisIndex][StanceIndex];
            }
        }
    }

    const double AverageSatisfaction =
        ActiveNpcCount > 0 ?
        OverallSatisfactionSum / static_cast<double>(ActiveNpcCount) :
        0.0;
    const double AverageFood =
        ActiveNpcCount > 0 ? FoodSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageHealth =
        ActiveNpcCount > 0 ? HealthSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageFun =
        ActiveNpcCount > 0 ? FunSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageFaith =
        ActiveNpcCount > 0 ? FaithSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageHousing =
        ActiveNpcCount > 0 ? HousingSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageJob =
        ActiveNpcCount > 0 ? JobSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageFreedom =
        ActiveNpcCount > 0 ? FreedomSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageSecurity =
        ActiveNpcCount > 0 ? SecuritySum / static_cast<double>(ActiveNpcCount) : 0.0;

    std::wstring Body;

    if (ActiveNpcCount > 0)
    {
        wchar_t Buffer[256] = {};
        swprintf_s(Buffer, L"종합 만족도: %.1f / 100\n", AverageSatisfaction);
        Body += Buffer;
        swprintf_s(Buffer, L"음식: %.1f\n", AverageFood); Body += Buffer;
        swprintf_s(Buffer, L"보건: %.1f\n", AverageHealth); Body += Buffer;
        swprintf_s(Buffer, L"유흥: %.1f\n", AverageFun); Body += Buffer;
        swprintf_s(Buffer, L"신앙: %.1f\n", AverageFaith); Body += Buffer;
        swprintf_s(Buffer, L"주거: %.1f\n", AverageHousing); Body += Buffer;
        swprintf_s(Buffer, L"직업: %.1f\n", AverageJob); Body += Buffer;
        swprintf_s(Buffer, L"자유: %.1f\n", AverageFreedom); Body += Buffer;
        swprintf_s(Buffer, L"치안: %.1f\n", AverageSecurity); Body += Buffer;
        swprintf_s(Buffer, L"무주택자 수: %d명\n", HomelessCount); Body += Buffer;
        swprintf_s(Buffer, L"실업자 수: %d명\n", UnemployedCount); Body += Buffer;
    }
    else
    {
        Body +=
            L"종합 만족도: -\n"
            L"음식: -\n"
            L"보건: -\n"
            L"유흥: -\n"
            L"신앙: -\n"
            L"주거: -\n"
            L"직업: -\n"
            L"자유: -\n"
            L"치안: -\n"
            L"무주택자 수: 0명\n"
            L"실업자 수: 0명\n";
    }

    Body += L"\n정치 성향 인원\n";

    for (int AxisIndex = 0;
        AxisIndex < static_cast<int>(EPoliticalAxis::Count);
        ++AxisIndex)
    {
        const EPoliticalAxis Axis =
            static_cast<EPoliticalAxis>(AxisIndex);
        wchar_t LineBuffer[384] = {};
        swprintf_s(
            LineBuffer,
            L"%s: %s %d명 / %s %d명 / %s %d명\n",
            GetPoliticalAxisDisplayName(Axis),
            GetPoliticalFactionDisplayName(Axis, EPoliticalStance::Left),
            PoliticalCount[AxisIndex][static_cast<int>(EPoliticalStance::Left)],
            GetPoliticalFactionDisplayName(Axis, EPoliticalStance::Neutral),
            PoliticalCount[AxisIndex][static_cast<int>(EPoliticalStance::Neutral)],
            GetPoliticalFactionDisplayName(Axis, EPoliticalStance::Right),
            PoliticalCount[AxisIndex][static_cast<int>(EPoliticalStance::Right)]);
        Body += LineBuffer;
    }

    YearbookBodyText->SetText(Body.c_str());
}

void CBuildMenuWidget::RefreshCategoryButtons()
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

    if (TitleText &&
        static_cast<int>(mSelectedCategory) < CategoryCount)
    {
        TitleText->SetText(CategoryLabels[static_cast<int>(mSelectedCategory)]);
    }
}

std::vector<int> CBuildMenuWidget::CollectCategoryEntryIndices() const
{
    std::vector<int> Result;
    const auto& Catalog = GetBuildingCatalog();

    for (int i = 0; i < static_cast<int>(Catalog.size()); ++i)
    {
        if (Catalog[i].Category != mSelectedCategory)
            continue;

        if (Catalog[i].IsHiddenFromBuildMenu)
            continue;

        Result.push_back(i);
    }

    return Result;
}

void CBuildMenuWidget::RefreshBuildingButtons()
{
    const std::vector<int> CategoryEntries = CollectCategoryEntryIndices();
    const int EntryCount = static_cast<int>(CategoryEntries.size());
    const int PageCount = (std::max)(
        1, (EntryCount + SlotsPerPage - 1) / SlotsPerPage);

    if (mCurrentPage < 0)
        mCurrentPage = 0;
    else if (mCurrentPage >= PageCount)
        mCurrentPage = PageCount - 1;

    mVisibleEntryIndices.assign(SlotsPerPage, -1);
    const int BeginIndex = mCurrentPage * SlotsPerPage;

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        const int CategoryListIndex = BeginIndex + i;
        auto Button = mBuildingButtons[i].lock();
        auto ButtonText = mBuildingButtonTexts[i].lock();

        if (!Button || !ButtonText)
            continue;

        if (CategoryListIndex >= 0 && CategoryListIndex < EntryCount)
        {
            const int EntryIndex = CategoryEntries[CategoryListIndex];
            const auto& Entry = GetBuildingCatalog()[EntryIndex];
            const TCHAR* IconPath = GetCatalogEntryIconPath(
                Entry.Category, Entry.CategoryLocalIndex);
            const std::string TextureKey = "BuildMenuSlotIcon_" +
                std::to_string(static_cast<int>(Entry.Category)) + "_" +
                std::to_string(Entry.CategoryLocalIndex);

            mVisibleEntryIndices[i] = EntryIndex;
            Button->ButtonEnable(true);
            ButtonText->SetText(Entry.DisplayName.c_str());

            if (IconPath)
            {
                ApplyTextureToAllButtonStates(Button, TextureKey, IconPath);
            }
            else
            {
                ApplyTextureToAllButtonStates(
                    Button,
                    "BuildMenuSlotEmptyTexture",
                    GEmptySlotTexture);
            }
        }
        else
        {
            Button->ButtonEnable(false);
            ButtonText->SetText(TEXT("-"));
            ApplyTextureToAllButtonStates(
                Button,
                "BuildMenuSlotEmptyTexture",
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

    bool HasPreviewOnPage = false;

    if (mPreviewEntryIndex >= 0)
    {
        for (int i = 0; i < static_cast<int>(mVisibleEntryIndices.size()); ++i)
        {
            if (mVisibleEntryIndices[i] == mPreviewEntryIndex)
            {
                HasPreviewOnPage = true;
                break;
            }
        }
    }

    if (!HasPreviewOnPage)
    {
        mPreviewEntryIndex = -1;

        for (int i = 0; i < static_cast<int>(mVisibleEntryIndices.size()); ++i)
        {
            if (mVisibleEntryIndices[i] >= 0)
            {
                mPreviewEntryIndex = mVisibleEntryIndices[i];
                break;
            }
        }
    }

    RefreshDetailPanel();
}

void CBuildMenuWidget::SelectCategory(EBuildingCategory Category)
{
    mSelectedCategory = Category;
    mCurrentPage = 0;
    RefreshCategoryButtons();
    RefreshBuildingButtons();
}

void CBuildMenuWidget::MovePage(int DeltaPage)
{
    mCurrentPage += DeltaPage;
    RefreshBuildingButtons();
}

void CBuildMenuWidget::PreviewSlot(int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
        return;

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    const auto& Catalog = GetBuildingCatalog();

    if (EntryIndex >= static_cast<int>(Catalog.size()))
        return;

    if (mPreviewEntryIndex == EntryIndex)
        return;

    mPreviewEntryIndex = EntryIndex;
    RefreshDetailPanel();
}

void CBuildMenuWidget::RefreshDetailPanel()
{
    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailBodyText = mDetailBodyText.lock();

    if (!DetailTitleText && !DetailBodyText)
        return;

    const auto& Catalog = GetBuildingCatalog();

    if (mPreviewEntryIndex < 0 ||
        mPreviewEntryIndex >= static_cast<int>(Catalog.size()))
    {
        if (DetailTitleText)
            DetailTitleText->SetText(TEXT("건물 정보"));
        if (DetailBodyText)
            DetailBodyText->SetText(TEXT("건물 아이콘에 마우스를 올리면 상세 정보가 표시됩니다."));
        return;
    }

    const auto& Entry = Catalog[mPreviewEntryIndex];

    if (DetailTitleText)
        DetailTitleText->SetText(Entry.DisplayName.c_str());

    if (DetailBodyText)
    {
        if (!Entry.DetailText.empty())
            DetailBodyText->SetText(Entry.DetailText.c_str());
        else
            DetailBodyText->SetText(TEXT("세부 데이터 준비 중"));
    }
}

void CBuildMenuWidget::StartPlacementBySlot(int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
        return;

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    const auto& Catalog = GetBuildingCatalog();

    if (EntryIndex >= static_cast<int>(Catalog.size()))
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto PlacementCtrl =
        World->FindObject<CPlacementController>(GPlacementControllerName).lock();

    if (!PlacementCtrl)
        return;

    const FBuildingCatalogEntry& Entry = Catalog[EntryIndex];

    if (Entry.IsDemolish)
    {
        PlacementCtrl->SetDemolitionMode(true);
        mMenuOpen = false;
        ApplyMenuOpenState();
        return;
    }

    PlacementCtrl->SetDemolitionMode(false);

    const std::string SpriteTexturePath =
        GetCatalogEntryIconPathUtf8(
            Entry.Category, Entry.CategoryLocalIndex);

    const bool Started = PlacementCtrl->BeginBuildPlacement(
        Entry, SpriteTexturePath);

    if (!Started)
        return;

    mMenuOpen = false;
    ApplyMenuOpenState();
}


void CBuildMenuWidget::OnBuildButtonClick()
{
    const bool NextOpen = !mMenuOpen;
    mMenuOpen = NextOpen;

    if (NextOpen)
        mYearbookOpen = false;

    ApplyMenuOpenState();
    ApplyYearbookOpenState();
}

void CBuildMenuWidget::OnYearbookButtonClick()
{
    const bool NextOpen = !mYearbookOpen;
    mYearbookOpen = NextOpen;

    if (NextOpen)
        mMenuOpen = false;

    ApplyMenuOpenState();
    ApplyYearbookOpenState();
}

void CBuildMenuWidget::OnCategoryInfrastructureClick()
{
    SelectCategory(EBuildingCategory::Infrastructure);
}

void CBuildMenuWidget::OnCategoryFoodResourceClick()
{
    SelectCategory(EBuildingCategory::FoodResource);
}

void CBuildMenuWidget::OnCategoryIndustryClick()
{
    SelectCategory(EBuildingCategory::Industry);
}

void CBuildMenuWidget::OnCategoryHousingClick()
{
    SelectCategory(EBuildingCategory::Housing);
}

void CBuildMenuWidget::OnCategoryEntertainmentClick()
{
    SelectCategory(EBuildingCategory::Entertainment);
}

void CBuildMenuWidget::OnCategoryMediaEducationClick()
{
    SelectCategory(EBuildingCategory::MediaEducation);
}

void CBuildMenuWidget::OnCategoryTourismClick()
{
    SelectCategory(EBuildingCategory::Tourism);
}

void CBuildMenuWidget::OnCategoryPublicServiceClick()
{
    SelectCategory(EBuildingCategory::PublicService);
}

void CBuildMenuWidget::OnPrevPageClick()
{
    MovePage(-1);
}

void CBuildMenuWidget::OnNextPageClick()
{
    MovePage(1);
}

void CBuildMenuWidget::OnSlot0Click()
{
    StartPlacementBySlot(0);
}

void CBuildMenuWidget::OnSlot1Click()
{
    StartPlacementBySlot(1);
}

void CBuildMenuWidget::OnSlot2Click()
{
    StartPlacementBySlot(2);
}

void CBuildMenuWidget::OnSlot3Click()
{
    StartPlacementBySlot(3);
}

void CBuildMenuWidget::OnSlot4Click()
{
    StartPlacementBySlot(4);
}

void CBuildMenuWidget::OnSlot5Click()
{
    StartPlacementBySlot(5);
}

void CBuildMenuWidget::OnSlot6Click()
{
    StartPlacementBySlot(6);
}

void CBuildMenuWidget::OnSlot7Click()
{
    StartPlacementBySlot(7);
}

void CBuildMenuWidget::OnSlot8Click()
{
    StartPlacementBySlot(8);
}

void CBuildMenuWidget::OnSlot9Click()
{
    StartPlacementBySlot(9);
}

void CBuildMenuWidget::OnSlot10Click()
{
    StartPlacementBySlot(10);
}

void CBuildMenuWidget::OnSlot11Click()
{
    StartPlacementBySlot(11);
}

void CBuildMenuWidget::OnSlot0Hovered()
{
    PreviewSlot(0);
}

void CBuildMenuWidget::OnSlot1Hovered()
{
    PreviewSlot(1);
}

void CBuildMenuWidget::OnSlot2Hovered()
{
    PreviewSlot(2);
}

void CBuildMenuWidget::OnSlot3Hovered()
{
    PreviewSlot(3);
}

void CBuildMenuWidget::OnSlot4Hovered()
{
    PreviewSlot(4);
}

void CBuildMenuWidget::OnSlot5Hovered()
{
    PreviewSlot(5);
}

void CBuildMenuWidget::OnSlot6Hovered()
{
    PreviewSlot(6);
}

void CBuildMenuWidget::OnSlot7Hovered()
{
    PreviewSlot(7);
}

void CBuildMenuWidget::OnSlot8Hovered()
{
    PreviewSlot(8);
}

void CBuildMenuWidget::OnSlot9Hovered()
{
    PreviewSlot(9);
}

void CBuildMenuWidget::OnSlot10Hovered()
{
    PreviewSlot(10);
}

void CBuildMenuWidget::OnSlot11Hovered()
{
    PreviewSlot(11);
}

void CBuildMenuWidget::ToggleBuildMenu()
{
    OnBuildButtonClick();
}

void CBuildMenuWidget::ToggleAlmanac()
{
    OnYearbookButtonClick();
}
