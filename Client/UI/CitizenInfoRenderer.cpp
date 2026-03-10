#include "CitizenInfoRenderer.h"
#include "CitizenInfoWidget.h"
#include "TropicoUiStyle.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include <algorithm>
#include <cwchar>

namespace
{
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

    constexpr int GBuildingTabCount = 5;
    constexpr int GBudgetLevelCount = 5;

    void SetPanelTextStyle(
        const std::shared_ptr<CTextBlock>& Text,
        float FontSize,
        const FVector4& Color,
        ETextAlignH AlignH = ETextAlignH::Left,
        ETextAlignV AlignV = ETextAlignV::Middle,
        bool Shadow = false)
    {
        if (!Text)
            return;

        Text->SetFontSize(FontSize);
        Text->SetAlignH(AlignH);
        Text->SetAlignV(AlignV);
        Text->SetTextColor(Color);
        Text->EnableShadow(Shadow);

        if (Shadow)
        {
            Text->SetShadowOffset(1.f, 1.f);
            Text->SetShadowTextColor(245, 235, 205, 180);
        }
    }
}

void FCitizenInfoRenderer::CreateWidgets(CCitizenInfoWidget& Widget)
{
    auto PanelImage = Widget.CreateWidget<CImage>("CitizenInfo_Panel", 6).lock();

    if (PanelImage)
    {
        PanelImage->SetTexture("CitizenInfoPanelTexture", GModernPanelTexture);
        PanelImage->SetTint(1.f, 1.f, 1.f, 0.98f);
        Widget.mPanelImage = PanelImage;
    }

    auto InnerFrame = Widget.CreateWidget<CImage>("CitizenInfo_InnerFrame", 7).lock();

    if (InnerFrame)
    {
        InnerFrame->SetTexture(
            "CitizenInfoInnerFrameTexture",
            GMenuDetailFrameTexture);
        InnerFrame->SetTint(1.f, 1.f, 1.f, 0.98f);
        Widget.mInnerFrame = InnerFrame;
    }

    auto TitleRibbon = Widget.CreateWidget<CImage>("CitizenInfo_TitleRibbon", 8).lock();

    if (TitleRibbon)
    {
        TitleRibbon->SetTexture(
            "CitizenInfoTitleRibbonTexture",
            GMenuTitleRibbonTexture);
        TitleRibbon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mTitleRibbon = TitleRibbon;
    }

    auto SectionRibbon =
        Widget.CreateWidget<CImage>("CitizenInfo_SectionRibbon", 8).lock();

    if (SectionRibbon)
    {
        SectionRibbon->SetTexture(
            "CitizenInfoSectionRibbonTexture",
            GMenuTitleRibbonTexture);
        SectionRibbon->SetTint(1.f, 1.f, 1.f, 0.96f);
        Widget.mSectionRibbon = SectionRibbon;
    }

    auto ScrollTrack =
        Widget.CreateWidget<CImage>("CitizenInfo_ScrollTrack", 8).lock();

    if (ScrollTrack)
    {
        ScrollTrack->SetTexture(
            "CitizenInfoScrollTrackTexture",
            GScrollTrackTexture);
        ScrollTrack->SetTint(1.f, 1.f, 1.f, 0.92f);
        Widget.mScrollTrack = ScrollTrack;
    }

    auto ScrollThumb =
        Widget.CreateWidget<CImage>("CitizenInfo_ScrollThumb", 9).lock();

    if (ScrollThumb)
    {
        ScrollThumb->SetTexture(
            "CitizenInfoScrollThumbTexture",
            GScrollThumbTexture);
        ScrollThumb->SetTint(1.f, 1.f, 1.f, 0.98f);
        Widget.mScrollThumb = ScrollThumb;
    }

    auto TitleIcon = Widget.CreateWidget<CImage>("CitizenInfo_TitleIcon", 9).lock();

    if (TitleIcon)
    {
        TitleIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mTitleIcon = TitleIcon;
    }

    auto TitleText = Widget.CreateWidget<CTextBlock>("CitizenInfo_TitleText", 9).lock();

    if (TitleText)
    {
        SetPanelTextStyle(
            TitleText,
            26.f,
            FVector4(0.37f, 0.26f, 0.10f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Middle,
            true);
        Widget.mTitleText = TitleText;
    }

    auto SubtitleText =
        Widget.CreateWidget<CTextBlock>("CitizenInfo_SubtitleText", 9).lock();

    if (SubtitleText)
    {
        SetPanelTextStyle(
            SubtitleText,
            15.f,
            FVector4(0.35f, 0.30f, 0.22f, 1.f),
            ETextAlignH::Center,
            ETextAlignV::Middle,
            false);
        Widget.mSubtitleText = SubtitleText;
    }

    auto PageTitleText =
        Widget.CreateWidget<CTextBlock>("CitizenInfo_PageTitleText", 9).lock();

    if (PageTitleText)
    {
        SetPanelTextStyle(
            PageTitleText,
            21.f,
            FVector4(0.37f, 0.26f, 0.10f, 1.f),
            ETextAlignH::Center,
            ETextAlignV::Middle,
            true);
        Widget.mPageTitleText = PageTitleText;
    }

    auto BodyText = Widget.CreateWidget<CTextBlock>("CitizenInfo_BodyText", 9).lock();

    if (BodyText)
    {
        SetPanelTextStyle(
            BodyText,
            18.f,
            FVector4(0.22f, 0.22f, 0.22f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Top,
            false);
        Widget.mBodyText = BodyText;
    }

    auto BudgetText =
        Widget.CreateWidget<CTextBlock>("CitizenInfo_BudgetText", 9).lock();

    if (BudgetText)
    {
        SetPanelTextStyle(
            BudgetText,
            16.f,
            FVector4(0.24f, 0.24f, 0.24f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Middle,
            false);
        Widget.mBudgetText = BudgetText;
    }

    auto CloseButton =
        Widget.CreateWidget<CButton>("CitizenInfo_CloseButton", 9).lock();

    if (CloseButton)
    {
        ApplyButtonTextureSet(
            CloseButton,
            "CitizenInfoClose",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        ConfigureIconSlotButtonStyle(CloseButton);
        CloseButton->SetEventCallback<CCitizenInfoWidget>(
            EButtonEventState::Click,
            &Widget,
            &CCitizenInfoWidget::OnCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "CitizenInfo_CloseText",
            Widget.mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            SetPanelTextStyle(
                CloseText,
                20.f,
                FVector4(0.38f, 0.25f, 0.08f, 1.f),
                ETextAlignH::Center,
                ETextAlignV::Middle,
                true);
            CloseButton->SetChild(CloseText);
        }

        Widget.mCloseButton = CloseButton;
    }

    static const wchar_t* GTabLabels[GBuildingTabCount] =
    {
        L"기본",
        L"통계",
        L"업글",
        L"효율",
        L"정보"
    };

    for (int Index = 0; Index < GBuildingTabCount; ++Index)
    {
        auto Button = Widget.CreateWidget<CButton>(
            "CitizenInfo_Tab_" + std::to_string(Index + 1),
            8).lock();

        if (!Button)
            continue;

        ApplyButtonTextureSet(
            Button,
            "CitizenInfoTabTexture_" + std::to_string(Index),
            GCategoryTabTextureHidden,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(Button, false);
        Button->SetEventCallback(
            EButtonEventState::Click,
            [&Widget, Index]()
            {
                Widget.SelectBuildingTab(
                    static_cast<CCitizenInfoWidget::EBuildingInfoTab>(Index));
                Widget.RefreshFromState();
            });

        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            "CitizenInfo_TabLabel_" + std::to_string(Index + 1),
            Widget.mWorld);

        if (Label)
        {
            Label->SetText(GTabLabels[Index]);
            SetPanelTextStyle(
                Label,
                14.f,
                FVector4(0.28f, 0.22f, 0.12f, 1.f),
                ETextAlignH::Center,
                ETextAlignV::Middle,
                true);
            Button->SetChild(Label);
            Widget.mTabButtonTexts[static_cast<size_t>(Index)] = Label;
        }

        Widget.mTabButtons[static_cast<size_t>(Index)] = Button;
    }

    void (CCitizenInfoWidget::*BudgetCallbacks[GBudgetLevelCount])() =
    {
        &CCitizenInfoWidget::OnBudgetLevel1Click,
        &CCitizenInfoWidget::OnBudgetLevel2Click,
        &CCitizenInfoWidget::OnBudgetLevel3Click,
        &CCitizenInfoWidget::OnBudgetLevel4Click,
        &CCitizenInfoWidget::OnBudgetLevel5Click
    };

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        auto Button = Widget.CreateWidget<CButton>(
            "CitizenInfo_BudgetButton_" + std::to_string(Index + 1),
            9).lock();

        if (!Button)
            continue;

        ApplyButtonTextureSet(
            Button,
            "CitizenInfoBudgetButtonTexture_" + std::to_string(Index),
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureDefaultButtonStyle(Button);
        Button->SetEventCallback<CCitizenInfoWidget>(
            EButtonEventState::Click,
            &Widget,
            BudgetCallbacks[Index]);

        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            "CitizenInfo_BudgetLabel_" + std::to_string(Index + 1),
            Widget.mWorld);

        if (Label)
        {
            wchar_t Buffer[8] = {};
            swprintf_s(Buffer, L"%d", Index + 1);
            Label->SetText(Buffer);
            SetPanelTextStyle(
                Label,
                16.f,
                FVector4(0.30f, 0.22f, 0.12f, 1.f),
                ETextAlignH::Center,
                ETextAlignV::Middle,
                true);
            Button->SetChild(Label);
            Widget.mBudgetButtonTexts[static_cast<size_t>(Index)] = Label;
        }

        Widget.mBudgetButtons[static_cast<size_t>(Index)] = Button;
    }

    auto CreateActionButton =
        [&Widget](
            const std::string& Name,
            const wchar_t* LabelText,
            void (CCitizenInfoWidget::*Callback)())
        -> std::shared_ptr<CButton>
    {
        auto Button = Widget.CreateWidget<CButton>(Name, 9).lock();

        if (!Button)
            return std::shared_ptr<CButton>();

        ApplyButtonTextureSet(
            Button,
            Name + "_Texture",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureDefaultButtonStyle(Button);

        if (Callback)
        {
            Button->SetEventCallback<CCitizenInfoWidget>(
                EButtonEventState::Click,
                &Widget,
                Callback);
        }

        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            Name + "_Label",
            Widget.mWorld);

        if (Label)
        {
            Label->SetText(LabelText);
            SetPanelTextStyle(
                Label,
                18.f,
                FVector4(0.29f, 0.22f, 0.12f, 1.f),
                ETextAlignH::Center,
                ETextAlignV::Middle,
                true);
            Button->SetChild(Label);
        }

        return Button;
    };

    Widget.mDemolishButton =
        CreateActionButton(
            "CitizenInfo_DemolishButton",
            L"철거",
            &CCitizenInfoWidget::OnDemolishButtonClick);
    Widget.mMoveButton =
        CreateActionButton(
            "CitizenInfo_MoveButton",
            L"이동",
            &CCitizenInfoWidget::OnMoveButtonClick);
    Widget.mCloneButton =
        CreateActionButton(
            "CitizenInfo_CloneButton",
            L"복제",
            &CCitizenInfoWidget::OnCloneButtonClick);

    if (auto CloneButton = Widget.mCloneButton.lock())
    {
        CloneButton->ButtonEnable(false);
        CloneButton->SetOpacityAll(0.72f);
    }
}

void FCitizenInfoRenderer::ApplySnapshot(
    CCitizenInfoWidget& Widget,
    const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot)
{
    auto TitleText = Widget.mTitleText.lock();
    auto SubtitleText = Widget.mSubtitleText.lock();
    auto PageTitleText = Widget.mPageTitleText.lock();
    auto BodyText = Widget.mBodyText.lock();
    auto BudgetText = Widget.mBudgetText.lock();
    auto SectionRibbon = Widget.mSectionRibbon.lock();
    auto TitleIcon = Widget.mTitleIcon.lock();

    if (TitleText)
        TitleText->SetText(Snapshot.Title.c_str());

    if (SubtitleText)
    {
        SubtitleText->SetText(Snapshot.Subtitle.c_str());
        SubtitleText->SetEnable(!Snapshot.Subtitle.empty());
    }

    if (PageTitleText)
    {
        PageTitleText->SetText(Snapshot.PageTitle.c_str());
        PageTitleText->SetEnable(Snapshot.ShowSectionRibbon);
    }

    if (BodyText)
        BodyText->SetText(Snapshot.BodyText.c_str());

    if (BudgetText)
    {
        BudgetText->SetText(Snapshot.BudgetText.c_str());
        BudgetText->SetEnable(Snapshot.ShowBudgetControls);
    }

    if (SectionRibbon)
        SectionRibbon->SetEnable(Snapshot.ShowSectionRibbon);

    if (TitleIcon)
    {
        const bool IconEnabled =
            Snapshot.Mode == CitizenInfoDataProvider::EPanelMode::Building &&
            Snapshot.ShowTitleIcon &&
            Snapshot.TitleIconPath &&
            TitleIcon->SetTexture(
                Snapshot.TitleIconTextureKey,
                Snapshot.TitleIconPath);

        TitleIcon->SetEnable(IconEnabled);
    }

    for (int Index = 0; Index < GBuildingTabCount; ++Index)
    {
        auto Button = Widget.mTabButtons[static_cast<size_t>(Index)].lock();
        auto Label = Widget.mTabButtonTexts[static_cast<size_t>(Index)].lock();
        const bool Selected = Snapshot.SelectedTabIndex == Index;

        if (Button)
        {
            ApplyButtonTextureSet(
                Button,
                "CitizenInfoTabRefresh_" + std::to_string(Index),
                Selected ?
                    GCategoryTabTextureSelected :
                    GCategoryTabTextureHidden,
                GCategoryTabTextureSelected,
                GCategoryTabTextureSelected,
                GCategoryTabTextureHidden);
            ConfigureCategoryTabButtonStyle(Button, Selected);
            Button->SetEnable(Snapshot.ShowTabButtons);
        }

        if (Label)
        {
            Label->SetTextColor(
                Selected ?
                    FVector4(0.27f, 0.17f, 0.06f, 1.f) :
                    FVector4(0.22f, 0.20f, 0.17f, 1.f));
        }
    }

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        auto Button = Widget.mBudgetButtons[static_cast<size_t>(Index)].lock();
        auto Label = Widget.mBudgetButtonTexts[static_cast<size_t>(Index)].lock();
        const bool Selected = Snapshot.BudgetLevel == Index + 1;

        if (Button)
        {
            ApplyButtonTextureSet(
                Button,
                "CitizenInfoBudgetRefresh_" + std::to_string(Index),
                Selected ?
                    GBigTextButtonSelectedTexture :
                    GBigTextButtonTexture,
                GBigTextButtonHoverTexture,
                GBigTextButtonSelectedTexture,
                GBigTextButtonDisabledTexture);
            Button->SetEnable(Snapshot.ShowBudgetControls);
            Button->ButtonEnable(Snapshot.ShowBudgetControls);
        }

        if (Label)
        {
            Label->SetTextColor(
                Selected ?
                    FVector4(0.36f, 0.22f, 0.08f, 1.f) :
                    FVector4(0.30f, 0.22f, 0.12f, 1.f));
        }
    }

    if (auto DemolishButton = Widget.mDemolishButton.lock())
        DemolishButton->SetEnable(Snapshot.ShowActionButtons);

    if (auto MoveButton = Widget.mMoveButton.lock())
        MoveButton->SetEnable(Snapshot.ShowActionButtons);

    if (auto CloneButton = Widget.mCloneButton.lock())
        CloneButton->SetEnable(Snapshot.ShowActionButtons);
}

void FCitizenInfoRenderer::RefreshLayout(CCitizenInfoWidget& Widget)
{
    (void)Widget.mRequestedScreenPos;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    Widget.mPanelWidth = (std::min)(
        410.f,
        (std::max)(320.f, static_cast<float>(Resolution.Width) * 0.24f));
    Widget.mPanelTop = 58.f;
    Widget.mPanelHeight = (std::max)(
        420.f,
        static_cast<float>(Resolution.Height) - Widget.mPanelTop - 10.f);

    const float PanelLeft =
        static_cast<float>(Resolution.Width) - Widget.mPanelWidth - 10.f;

    Widget.SetPos(PanelLeft, 0.f);
    Widget.SetSize(Widget.mPanelWidth, Widget.mPanelTop + Widget.mPanelHeight);

    const float OuterLeft = 0.f;
    const float OuterTop = Widget.mPanelTop;
    const float PanelWidth = Widget.mPanelWidth;
    const float PanelHeight = Widget.mPanelHeight;
    const float InnerLeft = 18.f;
    const float InnerTop = OuterTop + 16.f;
    const float InnerWidth = PanelWidth - 36.f;
    const float InnerHeight = PanelHeight - 28.f;

    if (auto PanelImage = Widget.mPanelImage.lock())
    {
        PanelImage->SetPos(OuterLeft, OuterTop);
        PanelImage->SetSize(PanelWidth, PanelHeight);
    }

    if (auto InnerFrame = Widget.mInnerFrame.lock())
    {
        InnerFrame->SetPos(InnerLeft, InnerTop);
        InnerFrame->SetSize(InnerWidth, InnerHeight);
    }

    if (auto ScrollTrack = Widget.mScrollTrack.lock())
    {
        ScrollTrack->SetPos(8.f, OuterTop + 74.f);
        ScrollTrack->SetSize(15.f, PanelHeight - 126.f);
    }

    if (auto ScrollThumb = Widget.mScrollThumb.lock())
    {
        ScrollThumb->SetPos(8.5f, OuterTop + 88.f);
        ScrollThumb->SetSize(14.f, 94.f);
    }

    const float TabWidth = 56.f;
    const float TabGap = 10.f;
    const float TotalTabsWidth =
        static_cast<float>(GBuildingTabCount) * TabWidth +
        static_cast<float>(GBuildingTabCount - 1) * TabGap;
    const float TabStartX = (std::max)(
        4.f,
        (PanelWidth - TotalTabsWidth) * 0.5f);

    for (int Index = 0; Index < GBuildingTabCount; ++Index)
    {
        auto Button = Widget.mTabButtons[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        Button->SetPos(
            TabStartX + static_cast<float>(Index) * (TabWidth + TabGap),
            0.f);
        Button->SetSize(TabWidth, 56.f);
    }

    if (auto TitleRibbon = Widget.mTitleRibbon.lock())
    {
        TitleRibbon->SetPos(40.f, OuterTop + 18.f);
        TitleRibbon->SetSize(PanelWidth - 96.f, 44.f);
    }

    if (auto CloseButton = Widget.mCloseButton.lock())
    {
        CloseButton->SetPos(PanelWidth - 38.f, OuterTop + 4.f);
        CloseButton->SetSize(34.f, 34.f);
    }

    const bool TitleIconVisible =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        !Widget.mTitleIcon.expired() &&
        Widget.mTitleIcon.lock()->GetEnable();
    const float TitleLeft = TitleIconVisible ? 78.f : 54.f;

    if (auto TitleIcon = Widget.mTitleIcon.lock())
    {
        TitleIcon->SetPos(53.f, OuterTop + 27.f);
        TitleIcon->SetSize(22.f, 22.f);
    }

    if (auto TitleText = Widget.mTitleText.lock())
    {
        TitleText->SetPos(TitleLeft, OuterTop + 18.f);
        TitleText->SetSize(PanelWidth - TitleLeft - 62.f, 44.f);
    }

    if (auto SubtitleText = Widget.mSubtitleText.lock())
    {
        SubtitleText->SetPos(46.f, OuterTop + 67.f);
        SubtitleText->SetSize(PanelWidth - 92.f, 22.f);
    }

    const bool ShowSectionRibbon =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab !=
            CCitizenInfoWidget::EBuildingInfoTab::Overview;

    if (auto SectionRibbon = Widget.mSectionRibbon.lock())
    {
        SectionRibbon->SetPos(42.f, OuterTop + 96.f);
        SectionRibbon->SetSize(PanelWidth - 84.f, 34.f);
        SectionRibbon->SetEnable(ShowSectionRibbon);
    }

    if (auto PageTitleText = Widget.mPageTitleText.lock())
    {
        PageTitleText->SetPos(42.f, OuterTop + 96.f);
        PageTitleText->SetSize(PanelWidth - 84.f, 34.f);
        PageTitleText->SetEnable(ShowSectionRibbon);
    }

    if (auto BudgetText = Widget.mBudgetText.lock())
    {
        BudgetText->SetPos(44.f, OuterTop + 102.f);
        BudgetText->SetSize(PanelWidth - 88.f, 24.f);
    }

    const float BudgetButtonTop = OuterTop + 132.f;
    const float BudgetGap = 8.f;
    const float BudgetButtonWidth =
        (PanelWidth - 88.f - BudgetGap * 4.f) / 5.f;

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        auto Button = Widget.mBudgetButtons[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        Button->SetPos(
            44.f + static_cast<float>(Index) *
                (BudgetButtonWidth + BudgetGap),
            BudgetButtonTop);
        Button->SetSize(BudgetButtonWidth, 30.f);
    }

    const bool ShowActions =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview;
    const float ActionTop = OuterTop + PanelHeight - 52.f;

    if (auto DemolishButton = Widget.mDemolishButton.lock())
    {
        DemolishButton->SetPos(42.f, ActionTop);
        DemolishButton->SetSize(124.f, 34.f);
    }

    if (auto MoveButton = Widget.mMoveButton.lock())
    {
        MoveButton->SetPos(PanelWidth - 154.f, ActionTop);
        MoveButton->SetSize(50.f, 34.f);
    }

    if (auto CloneButton = Widget.mCloneButton.lock())
    {
        CloneButton->SetPos(PanelWidth - 98.f, ActionTop);
        CloneButton->SetSize(50.f, 34.f);
    }

    const float BodyTop =
        ShowSectionRibbon ? (OuterTop + 142.f) :
        (ShowActions ? (OuterTop + 172.f) : (OuterTop + 102.f));
    const float BodyBottom =
        ShowActions ? (ActionTop - 18.f) :
        (OuterTop + PanelHeight - 28.f);

    if (auto BodyText = Widget.mBodyText.lock())
    {
        BodyText->SetPos(44.f, BodyTop);
        BodyText->SetSize(
            PanelWidth - 88.f,
            (std::max)(80.f, BodyBottom - BodyTop));
    }
}
