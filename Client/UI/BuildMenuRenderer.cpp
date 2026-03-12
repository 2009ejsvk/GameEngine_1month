#include "BuildMenuRenderer.h"
#include "BuildMenuWidget.h"
#include "TropicoUiStyle.h"
#include "../Building/BuildingCategoryInfo.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include <algorithm>

namespace
{
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

    constexpr int GCategoryCount = BuildingCategoryInfo::GBuildingCategoryCount;
    constexpr int GSlotsPerPage = 15;
    constexpr int GSlotColumnCount = 5;
    constexpr int GSlotRowCount = 3;
    constexpr const TCHAR* GBuildMenuPanelTexture = GMainMenuPanelTexture;
    constexpr const TCHAR* GYearbookPanelTexture = GModernPanelTexture;
    constexpr const TCHAR* GBlueprintCostIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_blueprint_cost.png");
    constexpr const TCHAR* GConstructionCostIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png");
}

void FBuildMenuRenderer::CreateWidgets(CBuildMenuWidget& Widget)
{
    CreateHudWidgets(Widget);
    CreateYearbookWidgets(Widget);
    CreateMenuFrameWidgets(Widget);
    CreateCategoryButtons(Widget);
    CreateBuildingButtons(Widget);
    CreateDetailWidgets(Widget);
}

void FBuildMenuRenderer::ApplySnapshot(
    CBuildMenuWidget& Widget,
    const BuildMenuDataProvider::FBuildMenuSnapshot& Snapshot)
{
    Widget.mCurrentPage = Snapshot.Catalog.CurrentPage;
    Widget.mPageCount = Snapshot.Catalog.PageCount;
    Widget.mPreviewEntryIndex = Snapshot.Catalog.PreviewEntryIndex;
    Widget.mVisibleEntryIndices = Snapshot.Catalog.VisibleEntryIndices;

    auto YearbookBodyText = Widget.mYearbookBodyText.lock();
    auto TitleText = Widget.mTitleText.lock();
    auto PageText = Widget.mPageText.lock();

    if (YearbookBodyText)
        YearbookBodyText->SetText(Snapshot.Status.YearbookBodyText.c_str());
    if (TitleText)
        TitleText->SetText(Snapshot.Catalog.TitleText.c_str());
    if (PageText)
        PageText->SetText(Snapshot.Catalog.PageText.c_str());

    for (int i = 0; i < static_cast<int>(Widget.mCategoryButtons.size()); ++i)
    {
        auto CategoryButton = Widget.mCategoryButtons[i].lock();

        if (!CategoryButton)
            continue;

        const EBuildingCategory Category =
            BuildingCategoryInfo::GetBuildMenuCategory(i);
        const bool Selected = Category == Widget.mSelectedCategory;
        ApplyButtonTextureSet(
            CategoryButton,
            "BuildMenuCategoryRefresh_" + std::to_string(i),
            Selected ? GCategoryTabTextureSelected : GCategoryTabTextureHidden,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(CategoryButton, Selected);
    }

    for (int i = 0; i < static_cast<int>(Widget.mBuildingButtons.size()); ++i)
    {
        auto Button = Widget.mBuildingButtons[i].lock();
        auto ButtonIcon = Widget.mBuildingButtonIcons[i].lock();
        auto ButtonText = Widget.mBuildingButtonTexts[i].lock();

        if (!Button || !ButtonText)
            continue;

        const bool HasSlot =
            i < static_cast<int>(Snapshot.Catalog.Slots.size());

        const auto& Slot = HasSlot ?
            Snapshot.Catalog.Slots[i] :
            BuildMenuDataProvider::FBuildMenuSlotSnapshot();

        ApplyButtonTextureSet(
            Button,
            "BuildMenuSlotCardRefresh_" + std::to_string(i),
            Slot.Previewed ? GSlotCardSelectedTexture : GSlotCardTexture,
            Slot.Previewed ? GSlotCardSelectedTexture : GSlotCardHoverTexture,
            GSlotCardSelectedTexture,
            GSlotCardDisabledTexture);

        Button->ButtonEnable(Slot.Enabled && Widget.mMenuOpen);
        Button->SetEnable(Slot.Enabled && Widget.mMenuOpen);
        ButtonText->SetText(
            Slot.Enabled ? Slot.DisplayName.c_str() : TEXT(""));

        if (ButtonIcon && Slot.Enabled && Slot.IconPath)
        {
            ButtonIcon->SetTexture(
                Slot.IconTextureKey,
                Slot.IconPath);
            ButtonIcon->SetEnable(true);
            ButtonIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        }
        else if (ButtonIcon)
        {
            ButtonIcon->SetEnable(false);
        }
    }

    auto PrevPageButton = Widget.mPrevPageButton.lock();
    auto NextPageButton = Widget.mNextPageButton.lock();

    if (PrevPageButton)
        PrevPageButton->ButtonEnable(
            Widget.mCurrentPage > 0 && Widget.mMenuOpen);
    if (NextPageButton)
    {
        NextPageButton->ButtonEnable(
            Widget.mCurrentPage < Widget.mPageCount - 1 &&
            Widget.mMenuOpen);
    }

    auto DetailTitleText = Widget.mDetailTitleText.lock();
    auto DetailBlueprintCostText = Widget.mDetailBlueprintCostText.lock();
    auto DetailConstructionCostText =
        Widget.mDetailConstructionCostText.lock();
    auto DetailInfoText = Widget.mDetailInfoText.lock();
    auto DetailBodyText = Widget.mDetailBodyText.lock();

    if (DetailTitleText)
        DetailTitleText->SetText(Snapshot.Detail.Title.c_str());
    if (DetailBlueprintCostText)
        DetailBlueprintCostText->SetText(
            Snapshot.Detail.BlueprintCost.c_str());
    if (DetailConstructionCostText)
    {
        DetailConstructionCostText->SetText(
            Snapshot.Detail.ConstructionCost.c_str());
    }
    if (DetailInfoText)
        DetailInfoText->SetText(Snapshot.Detail.InfoText.c_str());
    if (DetailBodyText)
        DetailBodyText->SetText(Snapshot.Detail.BodyText.c_str());

    ApplyMenuOpenState(Widget);
    ApplyYearbookOpenState(Widget);
}

void FBuildMenuRenderer::RefreshLayout(CBuildMenuWidget& Widget)
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(480.f, ScreenWidth - 80.f);
    const float AvailableHeight = (std::max)(420.f, ScreenHeight - 100.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(AvailableWidth / Widget.mPanelWidth,
                AvailableHeight / Widget.mPanelHeight));
    const float PanelWidth = Widget.mPanelWidth * Scale;
    const float PanelHeight = Widget.mPanelHeight * Scale;
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
    const float GridFrameTop =
        PanelTop + HeaderTopPadding + HeaderHeight + 16.f * Scale;
    const float GridFrameWidth = PanelWidth - HorizontalMargin * 2.f;
    const float GridFrameHeight = PanelHeight - 324.f * Scale;
    const float DetailFrameLeft = GridFrameLeft;
    const float DetailFrameTop =
        GridFrameTop + GridFrameHeight + 18.f * Scale;
    const float DetailFrameWidth = GridFrameWidth;
    const float DetailFrameHeight =
        PanelTop + PanelHeight - DetailFrameTop - 22.f * Scale;

    auto YearbookPanel = Widget.mYearbookPanel.lock();
    auto YearbookTitleText = Widget.mYearbookTitleText.lock();
    auto YearbookBodyText = Widget.mYearbookBodyText.lock();
    auto YearbookCloseButton = Widget.mYearbookCloseButton.lock();
    const float YearbookPanelWidth = PanelWidth;
    const float YearbookPanelHeight = PanelHeight;
    const float YearbookLeft = PanelLeft;
    const float YearbookTop = PanelTop;
    const float YearbookBodyTop =
        YearbookTop + HeaderTopPadding + HeaderHeight + 14.f * Scale;
    const float YearbookBodyHeight = (std::max)(
        40.f,
        YearbookPanelHeight -
            (YearbookBodyTop - YearbookTop) -
            16.f * Scale);

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

    if (YearbookCloseButton)
    {
        YearbookCloseButton->SetPos(
            YearbookLeft + YearbookPanelWidth - HorizontalMargin -
                44.f * Scale,
            YearbookTop + HeaderTopPadding - 4.f * Scale);
        YearbookCloseButton->SetSize(40.f * Scale, 40.f * Scale);
    }

    if (YearbookBodyText)
    {
        YearbookBodyText->SetPos(
            YearbookLeft + HorizontalMargin,
            YearbookBodyTop);
        YearbookBodyText->SetSize(ContentWidth, YearbookBodyHeight);
    }

    auto MenuBackground = Widget.mMenuBackground.lock();
    auto MenuTitleRibbon = Widget.mMenuTitleRibbon.lock();
    auto MenuGridFrame = Widget.mMenuGridFrame.lock();
    auto MenuDetailFrame = Widget.mMenuDetailFrame.lock();
    auto DetailInfoPanel = Widget.mDetailInfoPanel.lock();
    auto ScrollTrack = Widget.mScrollTrack.lock();
    auto ScrollThumb = Widget.mScrollThumb.lock();

    if (MenuBackground)
    {
        MenuBackground->SetPos(PanelLeft, PanelTop);
        MenuBackground->SetSize(PanelWidth, PanelHeight);
    }

    if (MenuTitleRibbon)
    {
        MenuTitleRibbon->SetPos(
            TitleRibbonLeft,
            PanelTop + HeaderTopPadding);
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

    auto TitleText = Widget.mTitleText.lock();
    auto MenuCloseButton = Widget.mMenuCloseButton.lock();

    if (TitleText)
    {
        TitleText->SetFontSize(32.f * Scale);
        TitleText->SetPos(
            TitleRibbonLeft + 32.f * Scale,
            PanelTop + HeaderTopPadding);
        TitleText->SetSize(
            TitleRibbonWidth - 64.f * Scale,
            HeaderHeight);
    }

    if (MenuCloseButton)
    {
        MenuCloseButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 44.f * Scale,
            PanelTop + HeaderTopPadding - 4.f * Scale);
        MenuCloseButton->SetSize(40.f * Scale, 40.f * Scale);
    }

    auto PrevPageButton = Widget.mPrevPageButton.lock();
    auto NextPageButton = Widget.mNextPageButton.lock();
    auto PageText = Widget.mPageText.lock();
    const int PageCount = (std::max)(1, Widget.mPageCount);
    const bool ShowPageControls = PageCount > 1;
    const float ScrollTrackLeft =
        GridFrameLeft + GridFrameWidth - 20.f * Scale;
    const float ScrollTrackTop = GridFrameTop + 46.f * Scale;
    const float ScrollTrackHeight = GridFrameHeight - 92.f * Scale;
    const float ScrollTrackWidth = 12.f * Scale;

    if (PrevPageButton)
    {
        PrevPageButton->SetPos(
            ScrollTrackLeft - 7.f * Scale,
            GridFrameTop + 10.f * Scale);
        PrevPageButton->SetSize(26.f * Scale, 20.f * Scale);
        PrevPageButton->SetEnable(Widget.mMenuOpen && ShowPageControls);
    }

    if (PageText)
    {
        PageText->SetFontSize(15.f * Scale);
        PageText->SetPos(
            ScrollTrackLeft - 52.f * Scale,
            GridFrameTop + GridFrameHeight - 30.f * Scale);
        PageText->SetSize(82.f * Scale, 20.f * Scale);
        PageText->SetEnable(Widget.mMenuOpen && ShowPageControls);
    }

    if (NextPageButton)
    {
        NextPageButton->SetPos(
            ScrollTrackLeft - 7.f * Scale,
            GridFrameTop + GridFrameHeight - 28.f * Scale);
        NextPageButton->SetSize(26.f * Scale, 20.f * Scale);
        NextPageButton->SetEnable(Widget.mMenuOpen && ShowPageControls);
    }

    if (ScrollTrack)
    {
        ScrollTrack->SetPos(ScrollTrackLeft, ScrollTrackTop);
        ScrollTrack->SetSize(ScrollTrackWidth, ScrollTrackHeight);
        ScrollTrack->SetEnable(Widget.mMenuOpen && ShowPageControls);
    }

    if (ScrollThumb)
    {
        const float ThumbHeight = (std::max)(
            30.f * Scale,
            ShowPageControls ?
                ScrollTrackHeight / static_cast<float>(PageCount) :
                ScrollTrackHeight);
        const float ThumbTravel =
            (std::max)(0.f, ScrollTrackHeight - ThumbHeight);
        const float ThumbRatio =
            PageCount > 1 ?
            static_cast<float>(Widget.mCurrentPage) /
                static_cast<float>(PageCount - 1) :
            0.f;

        ScrollThumb->SetPos(
            ScrollTrackLeft,
            ScrollTrackTop + ThumbTravel * ThumbRatio);
        ScrollThumb->SetSize(ScrollTrackWidth, ThumbHeight);
        ScrollThumb->SetEnable(Widget.mMenuOpen && ShowPageControls);
    }

    const float CategoryTop = PanelTop - 64.f * Scale;
    const float CategoryGap = 10.f * Scale;
    const float CategoryWidth = 78.f * Scale;
    const float CategoryHeight = 90.f * Scale;
    const float CategoryTotalWidth =
        CategoryWidth * GCategoryCount +
        CategoryGap * static_cast<float>(GCategoryCount - 1);
    const float CategoryStartX =
        PanelLeft + (PanelWidth - CategoryTotalWidth) * 0.5f;

    for (int i = 0; i < GCategoryCount; ++i)
    {
        auto CategoryButton = Widget.mCategoryButtons[i].lock();
        auto CategoryIcon = Widget.mCategoryButtonIcons[i].lock();

        if (!CategoryButton)
            continue;

        CategoryButton->SetPos(
            CategoryStartX +
            (CategoryWidth + CategoryGap) * static_cast<float>(i),
            CategoryTop);
        CategoryButton->SetSize(CategoryWidth, CategoryHeight);

        if (CategoryIcon)
        {
            CategoryIcon->SetPos(18.f * Scale, 10.f * Scale);
            CategoryIcon->SetSize(42.f * Scale, 42.f * Scale);
        }
    }

    const float SlotGapX = 12.f * Scale;
    const float SlotGapY = 14.f * Scale;
    const float SlotLeft = GridFrameLeft + 20.f * Scale;
    const float SlotTop = GridFrameTop + 18.f * Scale;
    const float SlotAreaWidth =
        GridFrameWidth - 56.f * Scale - ScrollTrackWidth;
    const float SlotAreaHeight = GridFrameHeight - 36.f * Scale;
    const float SlotWidth =
        (SlotAreaWidth - SlotGapX * (GSlotColumnCount - 1)) /
        static_cast<float>(GSlotColumnCount);
    const float SlotHeight =
        (SlotAreaHeight - SlotGapY * (GSlotRowCount - 1)) /
        static_cast<float>(GSlotRowCount);
    const float SlotIconHorizontalPadding = 16.f * Scale;
    const float SlotIconTopPadding = 12.f * Scale;
    const float SlotTextHeight = 36.f * Scale;
    const float SlotIconHeight =
        (std::max)(28.f * Scale, SlotHeight - SlotTextHeight - 20.f * Scale);

    for (int i = 0; i < GSlotsPerPage; ++i)
    {
        const int Row = i / GSlotColumnCount;
        const int Col = i % GSlotColumnCount;

        auto SlotButton = Widget.mBuildingButtons[i].lock();
        auto SlotIcon = Widget.mBuildingButtonIcons[i].lock();
        auto SlotText = Widget.mBuildingButtonTexts[i].lock();

        if (!SlotButton)
            continue;

        SlotButton->SetPos(
            SlotLeft + (SlotWidth + SlotGapX) * static_cast<float>(Col),
            SlotTop + (SlotHeight + SlotGapY) * static_cast<float>(Row));
        SlotButton->SetSize(SlotWidth, SlotHeight);

        if (SlotIcon)
        {
            SlotIcon->SetPos(
                SlotIconHorizontalPadding,
                SlotIconTopPadding);
            SlotIcon->SetSize(
                SlotWidth - SlotIconHorizontalPadding * 2.f,
                SlotIconHeight);
        }

        if (SlotText)
        {
            SlotText->SetFontSize(15.f * Scale);
            SlotText->SetPos(10.f * Scale, SlotHeight - SlotTextHeight);
            SlotText->SetSize(
                SlotWidth - 20.f * Scale,
                SlotTextHeight - 4.f * Scale);
        }
    }

    auto DetailTitleText = Widget.mDetailTitleText.lock();
    auto DetailBlueprintIcon = Widget.mDetailBlueprintIcon.lock();
    auto DetailBlueprintCostText = Widget.mDetailBlueprintCostText.lock();
    auto DetailConstructionIcon = Widget.mDetailConstructionIcon.lock();
    auto DetailConstructionCostText =
        Widget.mDetailConstructionCostText.lock();
    auto DetailInfoText = Widget.mDetailInfoText.lock();
    auto DetailBodyText = Widget.mDetailBodyText.lock();
    const float DetailPadding = 18.f * Scale;
    const float DetailColumnGap = 16.f * Scale;
    const float DetailTitleTop = DetailFrameTop + 12.f * Scale;
    const float DetailTitleHeight = 30.f * Scale;
    const float CostTop = DetailTitleTop + DetailTitleHeight + 10.f * Scale;
    const float CostIconSize = 22.f * Scale;
    const float CostTextHeight = 26.f * Scale;
    const float CostLabelGap = 6.f * Scale;
    const float SecondaryCostOffset = 166.f * Scale;
    const float DetailContentTop = CostTop + CostTextHeight + 12.f * Scale;
    const float DetailContentHeight = (std::max)(
        48.f * Scale,
        DetailFrameHeight -
            (DetailContentTop - DetailFrameTop) -
            DetailPadding);
    float DetailInfoWidth = (std::max)(
        250.f * Scale,
        DetailFrameWidth * 0.34f);
    DetailInfoWidth = (std::min)(
        DetailInfoWidth,
        DetailFrameWidth - DetailPadding * 2.f - 220.f * Scale);
    const float DetailBodyLeft = DetailFrameLeft + DetailPadding;
    const float DetailInfoLeft =
        DetailFrameLeft + DetailFrameWidth -
        DetailPadding -
        DetailInfoWidth;
    const float DetailBodyTop = DetailContentTop;
    const float DetailInfoTop = DetailContentTop;
    const float DetailInfoHeight = DetailContentHeight;
    const float DetailBodyWidth = (std::max)(
        200.f * Scale,
        DetailInfoLeft - DetailBodyLeft - DetailColumnGap);
    const float DetailBodyHeight = DetailContentHeight;
    const float CostLeft = DetailFrameLeft + DetailPadding;

    if (DetailTitleText)
    {
        DetailTitleText->SetFontSize(24.f * Scale);
        DetailTitleText->SetPos(
            DetailFrameLeft + DetailPadding,
            DetailTitleTop);
        DetailTitleText->SetSize(
            DetailFrameWidth - DetailPadding * 2.f,
            DetailTitleHeight);
    }

    if (DetailBlueprintIcon)
    {
        DetailBlueprintIcon->SetPos(CostLeft, CostTop);
        DetailBlueprintIcon->SetSize(CostIconSize, CostIconSize);
    }

    if (DetailBlueprintCostText)
    {
        DetailBlueprintCostText->SetFontSize(17.f * Scale);
        DetailBlueprintCostText->SetPos(
            CostLeft + CostIconSize + CostLabelGap,
            CostTop - 1.f * Scale);
        DetailBlueprintCostText->SetSize(132.f * Scale, CostTextHeight);
    }

    if (DetailConstructionIcon)
    {
        DetailConstructionIcon->SetPos(
            CostLeft + SecondaryCostOffset,
            CostTop + 1.f * Scale);
        DetailConstructionIcon->SetSize(CostIconSize, CostIconSize);
    }

    if (DetailConstructionCostText)
    {
        DetailConstructionCostText->SetFontSize(17.f * Scale);
        DetailConstructionCostText->SetPos(
            CostLeft + SecondaryCostOffset + CostIconSize + CostLabelGap,
            CostTop - 1.f * Scale);
        DetailConstructionCostText->SetSize(180.f * Scale, CostTextHeight);
    }

    if (DetailInfoPanel)
    {
        DetailInfoPanel->SetPos(DetailInfoLeft, DetailInfoTop);
        DetailInfoPanel->SetSize(DetailInfoWidth, DetailInfoHeight);
    }

    if (DetailInfoText)
    {
        DetailInfoText->SetFontSize(16.f * Scale);
        DetailInfoText->SetPos(
            DetailInfoLeft + 16.f * Scale,
            DetailInfoTop + 14.f * Scale);
        DetailInfoText->SetSize(
            DetailInfoWidth - 32.f * Scale,
            DetailInfoHeight - 28.f * Scale);
    }

    if (DetailBodyText)
    {
        DetailBodyText->SetFontSize(15.f * Scale);
        DetailBodyText->SetPos(DetailBodyLeft, DetailBodyTop);
        DetailBodyText->SetSize(DetailBodyWidth, DetailBodyHeight);
    }
}

bool FBuildMenuRenderer::IsMouseOverPanel(
    const CBuildMenuWidget& Widget,
    const FVector2& MousePos)
{
    if (!Widget.mMenuOpen && !Widget.mYearbookOpen)
        return false;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(480.f, ScreenWidth - 80.f);
    const float AvailableHeight = (std::max)(420.f, ScreenHeight - 100.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(AvailableWidth / Widget.mPanelWidth,
                AvailableHeight / Widget.mPanelHeight));
    const float PanelWidth = Widget.mPanelWidth * Scale;
    const float PanelHeight = Widget.mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;

    return MousePos.x >= PanelLeft &&
        MousePos.x <= PanelLeft + PanelWidth &&
        MousePos.y >= PanelTop &&
        MousePos.y <= PanelTop + PanelHeight;
}

void FBuildMenuRenderer::CreateHudWidgets(CBuildMenuWidget& Widget)
{
}

void FBuildMenuRenderer::CreateYearbookWidgets(CBuildMenuWidget& Widget)
{
    auto YearbookPanel =
        Widget.CreateWidget<CImage>("BuildMenu_YearbookPanel", 12).lock();

    if (YearbookPanel)
    {
        YearbookPanel->SetTexture(
            "BuildMenuYearbookBackground",
            GYearbookPanelTexture);
        YearbookPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mYearbookPanel = YearbookPanel;
    }

    auto YearbookTitleText =
        Widget.CreateWidget<CTextBlock>("BuildMenu_YearbookTitle", 13).lock();

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
        Widget.mYearbookTitleText = YearbookTitleText;
    }

    auto YearbookCloseButton =
        Widget.CreateWidget<CButton>("BuildMenu_YearbookClose", 13).lock();

    if (YearbookCloseButton)
    {
        ApplyButtonTextureSet(
            YearbookCloseButton,
            "BuildMenuYearbookClose",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        ConfigureIconSlotButtonStyle(YearbookCloseButton);
        YearbookCloseButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click,
            &Widget,
            &CBuildMenuWidget::OnYearbookCloseButtonClick);

        auto CloseText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_YearbookCloseText",
                Widget.mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            CloseText->SetFontSize(22.f);
            CloseText->SetAlignH(ETextAlignH::Center);
            CloseText->SetAlignV(ETextAlignV::Middle);
            CloseText->SetTextColor(95, 68, 18, 255);
            CloseText->EnableShadow(true);
            CloseText->SetShadowOffset(1.f, 1.f);
            CloseText->SetShadowTextColor(246, 219, 117, 150);
            YearbookCloseButton->SetChild(CloseText);
        }

        Widget.mYearbookCloseButton = YearbookCloseButton;
    }

    auto YearbookBodyText =
        Widget.CreateWidget<CTextBlock>("BuildMenu_YearbookBody", 13).lock();

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
        Widget.mYearbookBodyText = YearbookBodyText;
    }
}

void FBuildMenuRenderer::CreateMenuFrameWidgets(CBuildMenuWidget& Widget)
{
    auto MenuBackground =
        Widget.CreateWidget<CImage>("BuildMenu_Background", 6).lock();

    if (MenuBackground)
    {
        MenuBackground->SetTexture(
            "BuildMenuBackground",
            GBuildMenuPanelTexture);
        MenuBackground->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mMenuBackground = MenuBackground;
    }

    auto MenuTitleRibbon =
        Widget.CreateWidget<CImage>("BuildMenu_TitleRibbon", 7).lock();

    if (MenuTitleRibbon)
    {
        MenuTitleRibbon->SetTexture(
            "BuildMenuTitleRibbonTexture",
            GMenuTitleRibbonTexture);
        MenuTitleRibbon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mMenuTitleRibbon = MenuTitleRibbon;
    }

    auto MenuGridFrame =
        Widget.CreateWidget<CImage>("BuildMenu_GridFrame", 7).lock();

    if (MenuGridFrame)
    {
        MenuGridFrame->SetTexture(
            "BuildMenuGridFrameTexture",
            GMenuGridFrameTexture);
        MenuGridFrame->SetTint(1.f, 1.f, 1.f, 0.98f);
        Widget.mMenuGridFrame = MenuGridFrame;
    }

    auto MenuDetailFrame =
        Widget.CreateWidget<CImage>("BuildMenu_DetailFrame", 7).lock();

    if (MenuDetailFrame)
    {
        MenuDetailFrame->SetTexture(
            "BuildMenuDetailFrameTexture",
            GMenuDetailFrameTexture);
        MenuDetailFrame->SetTint(1.f, 1.f, 1.f, 0.98f);
        Widget.mMenuDetailFrame = MenuDetailFrame;
    }

    auto DetailInfoPanel =
        Widget.CreateWidget<CImage>("BuildMenu_DetailInfoPanel", 7).lock();

    if (DetailInfoPanel)
    {
        DetailInfoPanel->SetTexture(
            "BuildMenuDetailInfoPanelTexture",
            GDetailInfoPanelTexture);
        DetailInfoPanel->SetTint(1.f, 1.f, 1.f, 0.96f);
        Widget.mDetailInfoPanel = DetailInfoPanel;
    }

    auto ScrollTrack =
        Widget.CreateWidget<CImage>("BuildMenu_ScrollTrack", 7).lock();

    if (ScrollTrack)
    {
        ScrollTrack->SetTexture(
            "BuildMenuScrollTrackTexture",
            GScrollTrackTexture);
        ScrollTrack->SetTint(1.f, 1.f, 1.f, 0.95f);
        Widget.mScrollTrack = ScrollTrack;
    }

    auto ScrollThumb =
        Widget.CreateWidget<CImage>("BuildMenu_ScrollThumb", 8).lock();

    if (ScrollThumb)
    {
        ScrollThumb->SetTexture(
            "BuildMenuScrollThumbTexture",
            GScrollThumbTexture);
        ScrollThumb->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mScrollThumb = ScrollThumb;
    }

    auto TitleText =
        Widget.CreateWidget<CTextBlock>("BuildMenu_Title", 7).lock();

    if (TitleText)
    {
        TitleText->SetText(
            BuildingCategoryInfo::GetDisplayName(
                EBuildingCategory::Infrastructure));
        TitleText->SetFontSize(32.f);
        TitleText->SetAlignH(ETextAlignH::Center);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(78, 60, 28, 255);
        TitleText->EnableShadow(true);
        TitleText->SetShadowOffset(1.f, 1.f);
        TitleText->SetShadowTextColor(235, 220, 180, 180);
        Widget.mTitleText = TitleText;
    }

    auto PageText =
        Widget.CreateWidget<CTextBlock>("BuildMenu_PageText", 7).lock();

    if (PageText)
    {
        PageText->SetText(TEXT("1 / 1"));
        PageText->SetFontSize(16.f);
        PageText->SetAlignH(ETextAlignH::Center);
        PageText->SetAlignV(ETextAlignV::Middle);
        PageText->SetTextColor(102, 82, 46, 255);
        PageText->EnableShadow(true);
        PageText->SetShadowOffset(1.f, 1.f);
        PageText->SetShadowTextColor(235, 220, 180, 180);
        Widget.mPageText = PageText;
    }

    auto MenuCloseButton =
        Widget.CreateWidget<CButton>("BuildMenu_Close", 7).lock();

    if (MenuCloseButton)
    {
        ApplyButtonTextureSet(
            MenuCloseButton,
            "BuildMenuClose",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        ConfigureIconSlotButtonStyle(MenuCloseButton);
        MenuCloseButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click,
            &Widget,
            &CBuildMenuWidget::OnMenuCloseButtonClick);

        auto CloseText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_CloseText",
                Widget.mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            CloseText->SetFontSize(22.f);
            CloseText->SetAlignH(ETextAlignH::Center);
            CloseText->SetAlignV(ETextAlignV::Middle);
            CloseText->SetTextColor(95, 68, 18, 255);
            CloseText->EnableShadow(true);
            CloseText->SetShadowOffset(1.f, 1.f);
            CloseText->SetShadowTextColor(246, 219, 117, 150);
            MenuCloseButton->SetChild(CloseText);
        }

        Widget.mMenuCloseButton = MenuCloseButton;
    }

    auto PrevPageButton =
        Widget.CreateWidget<CButton>("BuildMenu_PrevPage", 7).lock();

    if (PrevPageButton)
    {
        ApplyButtonTextureSet(
            PrevPageButton,
            "BuildMenuPrevPage",
            GDropdownArrowTexture,
            GDropdownArrowHoverTexture,
            GDropdownArrowTexture,
            GDropdownArrowTexture);
        ConfigureIconSlotButtonStyle(PrevPageButton);
        PrevPageButton->SetAngle(180.f);
        PrevPageButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click,
            &Widget,
            &CBuildMenuWidget::OnPrevPageClick);
        Widget.mPrevPageButton = PrevPageButton;
    }

    auto NextPageButton =
        Widget.CreateWidget<CButton>("BuildMenu_NextPage", 7).lock();

    if (NextPageButton)
    {
        ApplyButtonTextureSet(
            NextPageButton,
            "BuildMenuNextPage",
            GDropdownArrowTexture,
            GDropdownArrowHoverTexture,
            GDropdownArrowTexture,
            GDropdownArrowTexture);
        ConfigureIconSlotButtonStyle(NextPageButton);
        NextPageButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click,
            &Widget,
            &CBuildMenuWidget::OnNextPageClick);
        Widget.mNextPageButton = NextPageButton;
    }
}

void FBuildMenuRenderer::CreateCategoryButtons(CBuildMenuWidget& Widget)
{
    Widget.mCategoryButtons.resize(GCategoryCount);
    Widget.mCategoryButtonIcons.resize(GCategoryCount);

    for (int i = 0; i < GCategoryCount; ++i)
    {
        const EBuildingCategory Category =
            BuildingCategoryInfo::GetBuildMenuCategory(i);
        auto Button =
            Widget.CreateWidget<CButton>(
                "BuildMenu_Category_" + std::to_string(i + 1),
                7).lock();

        if (!Button)
            continue;

        ApplyButtonTextureSet(
            Button,
            "BuildMenuCategoryTab_" + std::to_string(i),
            GCategoryTabTextureHidden,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(Button, false);
        Button->SetEventCallback(
            EButtonEventState::Click,
            [&Widget, Category]()
            {
                Widget.SelectCategory(Category);
                Widget.RefreshFromState();
            });

        auto CategoryIcon =
            CWidget::CreateStaticWidget<CImage>(
                "BuildMenu_CategoryIcon_" + std::to_string(i + 1),
                Widget.mWorld);

        if (CategoryIcon)
        {
            CategoryIcon->SetTexture(
                "BuildMenuCategoryIconTex_" + std::to_string(i + 1),
                BuildingCategoryInfo::GetTabIconPath(Category));
            CategoryIcon->SetTint(1.f, 1.f, 1.f, 1.f);
            Button->SetChild(CategoryIcon);
            Widget.mCategoryButtonIcons[i] = CategoryIcon;
        }

        Widget.mCategoryButtons[i] = Button;
    }
}

void FBuildMenuRenderer::CreateBuildingButtons(CBuildMenuWidget& Widget)
{
    Widget.mBuildingButtons.resize(GSlotsPerPage);
    Widget.mBuildingButtonIcons.resize(GSlotsPerPage);
    Widget.mBuildingButtonTexts.resize(GSlotsPerPage);

    for (int i = 0; i < GSlotsPerPage; ++i)
    {
        auto Button =
            Widget.CreateWidget<CButton>(
                "BuildMenu_Slot_" + std::to_string(i + 1),
                7).lock();

        if (!Button)
            continue;

        ApplyButtonTextureSet(
            Button,
            "BuildMenuSlotCard_" + std::to_string(i),
            GSlotCardTexture,
            GSlotCardHoverTexture,
            GSlotCardSelectedTexture,
            GSlotCardDisabledTexture);
        ConfigureIconSlotButtonStyle(Button);
        Button->SetEventCallback(
            EButtonEventState::Click,
            [&Widget, i]()
            {
                Widget.StartPlacementBySlot(i);
            });
        Button->SetEventCallback(
            EButtonEventState::Hovered,
            [&Widget, i]()
            {
                Widget.PreviewSlot(i);
                Widget.RefreshFromState();
            });

        auto SlotContent =
            CWidget::CreateStaticWidget<CWidgetContainer>(
                "BuildMenu_SlotContent_" + std::to_string(i + 1),
                Widget.mWorld);
        auto SlotIcon =
            CWidget::CreateStaticWidget<CImage>(
                "BuildMenu_SlotIcon_" + std::to_string(i + 1),
                Widget.mWorld);
        auto ButtonText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_SlotText_" + std::to_string(i + 1),
                Widget.mWorld);

        if (SlotContent && SlotIcon && ButtonText)
        {
            SlotIcon->SetTint(1.f, 1.f, 1.f, 1.f);
            SlotContent->AddWidget(SlotIcon);

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
            Widget.mBuildingButtonIcons[i] = SlotIcon;
        }

        Widget.mBuildingButtons[i] = Button;
        Widget.mBuildingButtonTexts[i] = ButtonText;
    }
}

void FBuildMenuRenderer::CreateDetailWidgets(CBuildMenuWidget& Widget)
{
    auto DetailTitleText =
        Widget.CreateWidget<CTextBlock>("BuildMenu_DetailTitle", 7).lock();

    if (DetailTitleText)
    {
        DetailTitleText->SetText(TEXT("건물 정보"));
        DetailTitleText->SetFontSize(26.f);
        DetailTitleText->SetAlignH(ETextAlignH::Left);
        DetailTitleText->SetAlignV(ETextAlignV::Middle);
        DetailTitleText->SetTextColor(100, 72, 28, 255);
        DetailTitleText->EnableShadow(true);
        DetailTitleText->SetShadowOffset(1.f, 1.f);
        DetailTitleText->SetShadowTextColor(245, 235, 205, 180);
        Widget.mDetailTitleText = DetailTitleText;
    }

    auto DetailBlueprintIcon =
        Widget.CreateWidget<CImage>(
            "BuildMenu_DetailBlueprintIcon",
            8).lock();

    if (DetailBlueprintIcon)
    {
        DetailBlueprintIcon->SetTexture(
            "BuildMenu_DetailBlueprintIconTexture",
            GBlueprintCostIconTexture);
        DetailBlueprintIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mDetailBlueprintIcon = DetailBlueprintIcon;
    }

    auto DetailBlueprintCostText =
        Widget.CreateWidget<CTextBlock>(
            "BuildMenu_DetailBlueprintCost",
            8).lock();

    if (DetailBlueprintCostText)
    {
        DetailBlueprintCostText->SetText(TEXT("-"));
        DetailBlueprintCostText->SetFontSize(18.f);
        DetailBlueprintCostText->SetAlignH(ETextAlignH::Left);
        DetailBlueprintCostText->SetAlignV(ETextAlignV::Middle);
        DetailBlueprintCostText->SetTextColor(62, 116, 204, 255);
        DetailBlueprintCostText->EnableShadow(true);
        DetailBlueprintCostText->SetShadowOffset(1.f, 1.f);
        DetailBlueprintCostText->SetShadowTextColor(240, 240, 240, 170);
        Widget.mDetailBlueprintCostText = DetailBlueprintCostText;
    }

    auto DetailConstructionIcon =
        Widget.CreateWidget<CImage>(
            "BuildMenu_DetailConstructionIcon",
            8).lock();

    if (DetailConstructionIcon)
    {
        DetailConstructionIcon->SetTexture(
            "BuildMenu_DetailConstructionIconTexture",
            GConstructionCostIconTexture);
        DetailConstructionIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mDetailConstructionIcon = DetailConstructionIcon;
    }

    auto DetailConstructionCostText =
        Widget.CreateWidget<CTextBlock>(
            "BuildMenu_DetailConstructionCost",
            8).lock();

    if (DetailConstructionCostText)
    {
        DetailConstructionCostText->SetText(TEXT("-"));
        DetailConstructionCostText->SetFontSize(18.f);
        DetailConstructionCostText->SetAlignH(ETextAlignH::Left);
        DetailConstructionCostText->SetAlignV(ETextAlignV::Middle);
        DetailConstructionCostText->SetTextColor(168, 120, 28, 255);
        DetailConstructionCostText->EnableShadow(true);
        DetailConstructionCostText->SetShadowOffset(1.f, 1.f);
        DetailConstructionCostText->SetShadowTextColor(240, 240, 240, 170);
        Widget.mDetailConstructionCostText = DetailConstructionCostText;
    }

    auto DetailInfoText =
        Widget.CreateWidget<CTextBlock>(
            "BuildMenu_DetailInfoText",
            8).lock();

    if (DetailInfoText)
    {
        DetailInfoText->SetText(TEXT("핵심 정보\n- 준비 중"));
        DetailInfoText->SetFontSize(16.f);
        DetailInfoText->SetAlignH(ETextAlignH::Left);
        DetailInfoText->SetAlignV(ETextAlignV::Top);
        DetailInfoText->SetTextColor(56, 56, 56, 255);
        DetailInfoText->EnableShadow(true);
        DetailInfoText->SetShadowOffset(1.f, 1.f);
        DetailInfoText->SetShadowTextColor(255, 255, 255, 150);
        Widget.mDetailInfoText = DetailInfoText;
    }

    auto DetailBodyText =
        Widget.CreateWidget<CTextBlock>("BuildMenu_DetailBody", 7).lock();

    if (DetailBodyText)
    {
        DetailBodyText->SetText(
            TEXT("건물을 선택하면 설명과 핵심 정보가 이 영역에 함께 표시됩니다."));
        DetailBodyText->SetFontSize(15.f);
        DetailBodyText->SetAlignH(ETextAlignH::Left);
        DetailBodyText->SetAlignV(ETextAlignV::Top);
        DetailBodyText->SetTextColor(72, 62, 40, 255);
        DetailBodyText->EnableShadow(true);
        DetailBodyText->SetShadowOffset(1.f, 1.f);
        DetailBodyText->SetShadowTextColor(242, 235, 219, 170);
        Widget.mDetailBodyText = DetailBodyText;
    }
}

void FBuildMenuRenderer::ApplyMenuOpenState(CBuildMenuWidget& Widget)
{
    auto MenuBackground = Widget.mMenuBackground.lock();
    auto MenuTitleRibbon = Widget.mMenuTitleRibbon.lock();
    auto MenuGridFrame = Widget.mMenuGridFrame.lock();
    auto MenuDetailFrame = Widget.mMenuDetailFrame.lock();
    auto DetailInfoPanel = Widget.mDetailInfoPanel.lock();
    auto DetailBlueprintIcon = Widget.mDetailBlueprintIcon.lock();
    auto DetailConstructionIcon = Widget.mDetailConstructionIcon.lock();
    auto DetailBlueprintCostText = Widget.mDetailBlueprintCostText.lock();
    auto DetailConstructionCostText =
        Widget.mDetailConstructionCostText.lock();
    auto DetailInfoText = Widget.mDetailInfoText.lock();
    auto ScrollTrack = Widget.mScrollTrack.lock();
    auto ScrollThumb = Widget.mScrollThumb.lock();
    auto TitleText = Widget.mTitleText.lock();
    auto PageText = Widget.mPageText.lock();
    auto DetailTitleText = Widget.mDetailTitleText.lock();
    auto DetailBodyText = Widget.mDetailBodyText.lock();
    auto MenuCloseButton = Widget.mMenuCloseButton.lock();
    auto PrevPageButton = Widget.mPrevPageButton.lock();
    auto NextPageButton = Widget.mNextPageButton.lock();

    if (MenuBackground)
        MenuBackground->SetEnable(Widget.mMenuOpen);
    if (MenuTitleRibbon)
        MenuTitleRibbon->SetEnable(Widget.mMenuOpen);
    if (MenuGridFrame)
        MenuGridFrame->SetEnable(Widget.mMenuOpen);
    if (MenuDetailFrame)
        MenuDetailFrame->SetEnable(Widget.mMenuOpen);
    if (DetailInfoPanel)
        DetailInfoPanel->SetEnable(Widget.mMenuOpen);
    if (DetailBlueprintIcon)
        DetailBlueprintIcon->SetEnable(Widget.mMenuOpen);
    if (DetailConstructionIcon)
        DetailConstructionIcon->SetEnable(Widget.mMenuOpen);
    if (DetailBlueprintCostText)
        DetailBlueprintCostText->SetEnable(Widget.mMenuOpen);
    if (DetailConstructionCostText)
        DetailConstructionCostText->SetEnable(Widget.mMenuOpen);
    if (DetailInfoText)
        DetailInfoText->SetEnable(Widget.mMenuOpen);
    if (ScrollTrack)
        ScrollTrack->SetEnable(Widget.mMenuOpen);
    if (ScrollThumb)
        ScrollThumb->SetEnable(Widget.mMenuOpen);
    if (TitleText)
        TitleText->SetEnable(Widget.mMenuOpen);
    if (PageText)
        PageText->SetEnable(Widget.mMenuOpen);
    if (DetailTitleText)
        DetailTitleText->SetEnable(Widget.mMenuOpen);
    if (DetailBodyText)
        DetailBodyText->SetEnable(Widget.mMenuOpen);
    if (MenuCloseButton)
        MenuCloseButton->SetEnable(Widget.mMenuOpen);
    if (PrevPageButton)
        PrevPageButton->SetEnable(Widget.mMenuOpen);
    if (NextPageButton)
        NextPageButton->SetEnable(Widget.mMenuOpen);

    for (size_t i = 0; i < Widget.mCategoryButtons.size(); ++i)
    {
        auto Button = Widget.mCategoryButtons[i].lock();

        if (Button)
            Button->SetEnable(Widget.mMenuOpen);
    }

    for (size_t i = 0; i < Widget.mBuildingButtons.size(); ++i)
    {
        auto Button = Widget.mBuildingButtons[i].lock();
        const bool HasEntry =
            i < Widget.mVisibleEntryIndices.size() &&
            Widget.mVisibleEntryIndices[i] >= 0;

        if (Button)
            Button->SetEnable(Widget.mMenuOpen && HasEntry);
    }

}

void FBuildMenuRenderer::ApplyYearbookOpenState(CBuildMenuWidget& Widget)
{
    auto YearbookPanel = Widget.mYearbookPanel.lock();
    auto YearbookTitleText = Widget.mYearbookTitleText.lock();
    auto YearbookBodyText = Widget.mYearbookBodyText.lock();
    auto YearbookCloseButton = Widget.mYearbookCloseButton.lock();

    if (YearbookPanel)
        YearbookPanel->SetEnable(Widget.mYearbookOpen);
    if (YearbookTitleText)
        YearbookTitleText->SetEnable(Widget.mYearbookOpen);
    if (YearbookBodyText)
        YearbookBodyText->SetEnable(Widget.mYearbookOpen);
    if (YearbookCloseButton)
        YearbookCloseButton->SetEnable(Widget.mYearbookOpen);
}
