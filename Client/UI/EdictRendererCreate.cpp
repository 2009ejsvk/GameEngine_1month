#include "EdictRenderer.h"
#include "EdictRendererInternal.h"
#include "World/World.h"
#include <algorithm>
#include <string>

void FEdictRenderer::CreateWidgets(CEdictWidget& Widget)
{
    Widget.mVisibleEntryIndices.assign(GEdictSlotsPerPage, -1);
    Widget.mPanelWidth = 1120.f;
    Widget.mPanelHeight = 760.f;

    auto MenuBackground =
        Widget.CreateWidget<CImage>("EdictMenu_Background", 6).lock();

    if (MenuBackground)
    {
        MenuBackground->SetTexture("EdictMenuBackground", GEdictMenuPanelTexture);
        MenuBackground->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mMenuBackground = MenuBackground;
    }

    auto MenuTitleRibbon =
        Widget.CreateWidget<CImage>("EdictMenu_TitleRibbon", 7).lock();

    if (MenuTitleRibbon)
    {
        MenuTitleRibbon->SetTexture(
            "EdictMenu_TitleRibbonTex",
            GMenuTitleRibbonTexture);
        MenuTitleRibbon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mMenuTitleRibbon = MenuTitleRibbon;
    }

    auto MenuGridFrame =
        Widget.CreateWidget<CImage>("EdictMenu_GridFrame", 7).lock();

    if (MenuGridFrame)
    {
        MenuGridFrame->SetTexture(
            "EdictMenu_GridFrameTex",
            GMenuGridFrameTexture);
        MenuGridFrame->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mMenuGridFrame = MenuGridFrame;
    }

    auto MenuDetailFrame =
        Widget.CreateWidget<CImage>("EdictMenu_DetailFrame", 7).lock();

    if (MenuDetailFrame)
    {
        MenuDetailFrame->SetTexture(
            "EdictMenu_DetailFrameTex",
            GMenuDetailFrameTexture);
        MenuDetailFrame->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mMenuDetailFrame = MenuDetailFrame;
    }

    auto DetailInfoPanel =
        Widget.CreateWidget<CImage>("EdictMenu_DetailInfoPanel", 7).lock();

    if (DetailInfoPanel)
    {
        DetailInfoPanel->SetTexture(
            "EdictMenu_DetailInfoPanelTex",
            GDetailInfoPanelTexture);
        DetailInfoPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mDetailInfoPanel = DetailInfoPanel;
    }

    auto TaxInfoPanel =
        Widget.CreateWidget<CImage>("EdictMenu_TaxInfoPanel", 7).lock();

    if (TaxInfoPanel)
    {
        TaxInfoPanel->SetTexture(
            "EdictMenu_TaxInfoPanelTex",
            GDetailInfoPanelTexture);
        TaxInfoPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mTaxInfoPanel = TaxInfoPanel;
    }

    auto ScrollTrack =
        Widget.CreateWidget<CImage>("EdictMenu_ScrollTrack", 7).lock();

    if (ScrollTrack)
    {
        ScrollTrack->SetTexture(
            "EdictMenu_ScrollTrackTex",
            GScrollTrackTexture);
        ScrollTrack->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mScrollTrack = ScrollTrack;
    }

    auto ScrollThumb =
        Widget.CreateWidget<CImage>("EdictMenu_ScrollThumb", 8).lock();

    if (ScrollThumb)
    {
        ScrollThumb->SetTexture(
            "EdictMenu_ScrollThumbTex",
            GScrollThumbTexture);
        ScrollThumb->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mScrollThumb = ScrollThumb;
    }

    auto TitleText = Widget.CreateWidget<CTextBlock>("EdictMenu_Title", 7).lock();

    if (TitleText)
    {
        const int CategoryIndex =
            (std::max)(0, (std::min)(GEdictCategoryCount - 1,
                static_cast<int>(Widget.mSelectedCategory)));
        TitleText->SetText(GCategoryLabels[CategoryIndex]);
        TitleText->SetFontSize(32.f);
        TitleText->SetAlignH(ETextAlignH::Center);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(100, 72, 28, 255);
        TitleText->EnableShadow(true);
        TitleText->SetShadowOffset(1.f, 1.f);
        TitleText->SetShadowTextColor(245, 235, 205, 180);
        Widget.mTitleText = TitleText;
    }

    auto PageText =
        Widget.CreateWidget<CTextBlock>("EdictMenu_PageText", 7).lock();

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
        Widget.mPageText = PageText;
    }

    auto CloseButton =
        Widget.CreateWidget<CButton>("EdictMenu_CloseButton", 7).lock();

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
            EButtonEventState::Click,
            &Widget,
            &CEdictWidget::OnCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "EdictMenu_CloseText",
            Widget.mWorld);

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

        Widget.mCloseButton = CloseButton;
    }

    auto PrevPageButton =
        Widget.CreateWidget<CButton>("EdictMenu_PrevPage", 7).lock();

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
            EButtonEventState::Click,
            &Widget,
            &CEdictWidget::OnPrevPageClick);
        Widget.mPrevPageButton = PrevPageButton;
    }

    auto NextPageButton =
        Widget.CreateWidget<CButton>("EdictMenu_NextPage", 7).lock();

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
            EButtonEventState::Click,
            &Widget,
            &CEdictWidget::OnNextPageClick);
        Widget.mNextPageButton = NextPageButton;
    }

    Widget.mCategoryButtons.resize(GEdictCategoryCount);
    Widget.mCategoryButtonIcons.resize(GEdictCategoryCount);

    for (int i = 0; i < GEdictCategoryCount; ++i)
    {
        auto Button = Widget.CreateWidget<CButton>(
            "EdictMenu_Category_" + std::to_string(i + 1),
            7).lock();

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
            [&Widget, i]()
            {
                Widget.SelectCategory(static_cast<EEdictUiCategory>(i));
                Widget.RefreshFromState();
            });

        auto CategoryIcon = CWidget::CreateStaticWidget<CImage>(
            "EdictMenu_CategoryIcon_" + std::to_string(i + 1),
            Widget.mWorld);

        if (CategoryIcon)
        {
            CategoryIcon->SetTexture(
                "EdictMenuCategoryIconTex_" + std::to_string(i + 1),
                GCategoryTabIcons[i]);
            CategoryIcon->SetTint(1.f, 1.f, 1.f, 1.f);
            Button->SetChild(CategoryIcon);
            Widget.mCategoryButtonIcons[i] = CategoryIcon;
        }

        Widget.mCategoryButtons[i] = Button;
    }

    Widget.mEdictButtons.resize(GEdictSlotsPerPage);
    Widget.mEdictButtonIcons.resize(GEdictSlotsPerPage);
    Widget.mEdictButtonStarLefts.resize(GEdictSlotsPerPage);
    Widget.mEdictButtonStarRights.resize(GEdictSlotsPerPage);
    Widget.mEdictButtonChecks.resize(GEdictSlotsPerPage);
    Widget.mEdictButtonTexts.resize(GEdictSlotsPerPage);

    for (int i = 0; i < GEdictSlotsPerPage; ++i)
    {
        auto Button = Widget.CreateWidget<CButton>(
            "EdictMenu_Slot_" + std::to_string(i + 1),
            7).lock();

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
            [&Widget, i]()
            {
                Widget.SelectOrApplySlot(i);
                Widget.RefreshFromState();
            });
        Button->SetEventCallback(
            EButtonEventState::Hovered,
            [&Widget, i]()
            {
                Widget.PreviewSlot(i);
                Widget.RefreshFromState();
            });

        auto SlotContent = CWidget::CreateStaticWidget<CWidgetContainer>(
            "EdictMenu_SlotContent_" + std::to_string(i + 1),
            Widget.mWorld);
        auto StarLeft = CWidget::CreateStaticWidget<CImage>(
            "EdictMenu_SlotStarLeft_" + std::to_string(i + 1),
            Widget.mWorld);
        auto StarRight = CWidget::CreateStaticWidget<CImage>(
            "EdictMenu_SlotStarRight_" + std::to_string(i + 1),
            Widget.mWorld);
        auto SlotIcon = CWidget::CreateStaticWidget<CImage>(
            "EdictMenu_SlotIcon_" + std::to_string(i + 1),
            Widget.mWorld);
        auto SlotCheck = CWidget::CreateStaticWidget<CImage>(
            "EdictMenu_SlotCheck_" + std::to_string(i + 1),
            Widget.mWorld);
        auto ButtonText = CWidget::CreateStaticWidget<CTextBlock>(
            "EdictMenu_SlotText_" + std::to_string(i + 1),
            Widget.mWorld);

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
            Widget.mEdictButtonIcons[i] = SlotIcon;
            Widget.mEdictButtonStarLefts[i] = StarLeft;
            Widget.mEdictButtonStarRights[i] = StarRight;
            Widget.mEdictButtonChecks[i] = SlotCheck;
            Widget.mEdictButtonTexts[i] = ButtonText;
        }

        Widget.mEdictButtons[i] = Button;
    }

    auto DetailTitleText =
        Widget.CreateWidget<CTextBlock>("EdictMenu_DetailTitle", 7).lock();

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
        Widget.mDetailTitleText = DetailTitleText;
    }

    auto DetailCostIcon =
        Widget.CreateWidget<CImage>("EdictMenu_DetailCostIcon", 8).lock();

    if (DetailCostIcon)
    {
        DetailCostIcon->SetTexture(
            "EdictMenu_DetailCostIconTex",
            GCostIconTexture);
        DetailCostIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mDetailCostIcon = DetailCostIcon;
    }

    auto DetailCostText =
        Widget.CreateWidget<CTextBlock>("EdictMenu_DetailCostText", 8).lock();

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
        Widget.mDetailCostText = DetailCostText;
    }

    auto DetailInfoText =
        Widget.CreateWidget<CTextBlock>("EdictMenu_DetailInfoText", 8).lock();

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
        Widget.mDetailInfoText = DetailInfoText;
    }

    auto FeedbackText =
        Widget.CreateWidget<CTextBlock>("EdictMenu_FeedbackText", 8).lock();

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
        Widget.mFeedbackText = FeedbackText;
    }

    auto DetailBodyText =
        Widget.CreateWidget<CTextBlock>("EdictMenu_DetailBody", 8).lock();

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
        Widget.mDetailBodyText = DetailBodyText;
    }

    auto RequirementText =
        Widget.CreateWidget<CTextBlock>("EdictMenu_RequirementText", 8).lock();

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
        Widget.mRequirementText = RequirementText;
    }

    auto ApplyButton =
        Widget.CreateWidget<CButton>("EdictMenu_ApplyButton", 7).lock();

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
            EButtonEventState::Click,
            &Widget,
            &CEdictWidget::OnApplyButtonClick);

        auto ApplyButtonText = CWidget::CreateStaticWidget<CTextBlock>(
            "EdictMenu_ApplyButtonText",
            Widget.mWorld);

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
            Widget.mApplyButtonText = ApplyButtonText;
        }

        Widget.mApplyButton = ApplyButton;
    }

    auto TaxPolicyTitleText =
        Widget.CreateWidget<CTextBlock>("EdictMenu_TaxPolicyTitle", 7).lock();

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
        Widget.mTaxPolicyTitleText = TaxPolicyTitleText;
    }

    auto TaxPolicySummaryText =
        Widget.CreateWidget<CTextBlock>("EdictMenu_TaxPolicySummary", 7).lock();

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
        Widget.mTaxPolicySummaryText = TaxPolicySummaryText;
    }

    Widget.mTaxPolicyRowTexts.resize(GTaxPolicyRowCount);
    Widget.mTaxDecreaseButtons.resize(GTaxPolicyRowCount);
    Widget.mTaxIncreaseButtons.resize(GTaxPolicyRowCount);

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
        auto RowText = Widget.CreateWidget<CTextBlock>(
            "EdictMenu_TaxPolicyRow_" + std::to_string(i + 1),
            7).lock();

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
            Widget.mTaxPolicyRowTexts[i] = RowText;
        }

        auto DecreaseButton = Widget.CreateWidget<CButton>(
            "EdictMenu_TaxDown_" + std::to_string(i + 1),
            7).lock();

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
                EButtonEventState::Click,
                &Widget,
                TaxDecreaseCallbacks[i]);

            auto DecreaseText = CWidget::CreateStaticWidget<CTextBlock>(
                "EdictMenu_TaxDownText_" + std::to_string(i + 1),
                Widget.mWorld);

            if (DecreaseText)
            {
                DecreaseText->SetText(TEXT("-"));
                DecreaseText->SetFontSize(16.f);
                DecreaseText->SetAlignH(ETextAlignH::Center);
                DecreaseText->SetAlignV(ETextAlignV::Middle);
                DecreaseText->SetTextColor(92, 60, 12, 255);
                DecreaseButton->SetChild(DecreaseText);
            }

            Widget.mTaxDecreaseButtons[i] = DecreaseButton;
        }

        auto IncreaseButton = Widget.CreateWidget<CButton>(
            "EdictMenu_TaxUp_" + std::to_string(i + 1),
            7).lock();

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
                EButtonEventState::Click,
                &Widget,
                TaxIncreaseCallbacks[i]);

            auto IncreaseText = CWidget::CreateStaticWidget<CTextBlock>(
                "EdictMenu_TaxUpText_" + std::to_string(i + 1),
                Widget.mWorld);

            if (IncreaseText)
            {
                IncreaseText->SetText(TEXT("+"));
                IncreaseText->SetFontSize(16.f);
                IncreaseText->SetAlignH(ETextAlignH::Center);
                IncreaseText->SetAlignV(ETextAlignV::Middle);
                IncreaseText->SetTextColor(92, 60, 12, 255);
                IncreaseButton->SetChild(IncreaseText);
            }

            Widget.mTaxIncreaseButtons[i] = IncreaseButton;
        }
    }
}

