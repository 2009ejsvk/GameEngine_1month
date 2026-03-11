#include "CitizenInfoRenderer.h"
#include "CitizenInfoRendererInternal.h"
#include "CitizenInfoWidget.h"
#include "UIStrings.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include <cwchar>
using namespace CitizenInfoRendererInternal;
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

    auto SectionDivider =
        Widget.CreateWidget<CImage>("CitizenInfo_SectionDivider", 9).lock();

    if (SectionDivider)
    {
        SectionDivider->SetTexture(
            "CitizenInfoSectionDividerTexture",
            GSectionDividerTexture);
        SectionDivider->SetTint(0.86f, 0.98f, 1.08f, 0.96f);
        Widget.mSectionDivider = SectionDivider;
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

    auto OverviewWorkModeLabel =
        Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_OverviewWorkModeLabel",
            9).lock();

    if (OverviewWorkModeLabel)
    {
        SetPanelTextStyle(
            OverviewWorkModeLabel,
            18.f,
            FVector4(0.24f, 0.24f, 0.24f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Middle,
            false);
        Widget.mOverviewWorkModeLabel = OverviewWorkModeLabel;
    }

    auto OverviewWorkModeBackground =
        Widget.CreateWidget<CImage>(
            "CitizenInfo_OverviewWorkModeBackground",
            9).lock();

    if (OverviewWorkModeBackground)
    {
        OverviewWorkModeBackground->SetTexture(
            "CitizenInfoOverviewWorkModeBackgroundTexture",
            GMenuDetailFrameTexture);
        OverviewWorkModeBackground->SetTint(1.f, 1.f, 1.f, 0.96f);
        Widget.mOverviewWorkModeBackground = OverviewWorkModeBackground;
    }

    auto OverviewWorkModeText =
        Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_OverviewWorkModeText",
            10).lock();

    if (OverviewWorkModeText)
    {
        SetPanelTextStyle(
            OverviewWorkModeText,
            17.f,
            FVector4(0.32f, 0.20f, 0.10f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Middle,
            false);
        Widget.mOverviewWorkModeText = OverviewWorkModeText;
    }

    auto InformationAccentText =
        Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_InformationAccentText",
            9).lock();

    if (InformationAccentText)
    {
        SetPanelTextStyle(
            InformationAccentText,
            28.f,
            FVector4(0.26f, 0.64f, 0.82f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Top,
            false);
        Widget.mInformationAccentText = InformationAccentText;
    }

    auto InformationTopText =
        Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_InformationTopText",
            9).lock();

    if (InformationTopText)
    {
        SetPanelTextStyle(
            InformationTopText,
            17.f,
            FVector4(0.22f, 0.22f, 0.22f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Top,
            false);
        Widget.mInformationTopText = InformationTopText;
    }

    auto InformationBottomText =
        Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_InformationBottomText",
            9).lock();

    if (InformationBottomText)
    {
        SetPanelTextStyle(
            InformationBottomText,
            17.f,
            FVector4(0.22f, 0.22f, 0.22f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Top,
            false);
        Widget.mInformationBottomText = InformationBottomText;
    }

    constexpr const wchar_t* GCitizenPoliticsSectionTitleKeys[
        CCitizenInfoWidget::GCitizenPoliticsSectionCount] =
    {
        L"citizen_info.politics.section.satisfaction",
        L"citizen_info.politics.section.opinion",
        L"citizen_info.politics.section.support"
    };

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenPoliticsSectionCount;
        ++Index)
    {
        auto Background = Widget.CreateWidget<CImage>(
            "CitizenInfo_CitizenPoliticsSectionBackground_" +
                std::to_string(Index + 1),
            9).lock();
        auto Title = Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_CitizenPoliticsSectionTitle_" +
                std::to_string(Index + 1),
            10).lock();

        if (Background)
        {
            Background->SetTexture(
                "CitizenInfo_CitizenPoliticsSectionTexture_" +
                    std::to_string(Index),
                GCitizenPoliticsSectionTexture);
            Background->SetTint(1.f, 0.98f, 0.92f, 0.94f);
            Widget.mCitizenPoliticsSectionBackgrounds[
                static_cast<size_t>(Index)] = Background;
        }

        if (Title)
        {
            SetPanelTextStyle(
                Title,
                17.f,
                FVector4(0.36f, 0.24f, 0.10f, 1.f),
                ETextAlignH::Center,
                ETextAlignV::Middle,
                true);
            Title->SetText(
                UIStrings::Get(
                    GCitizenPoliticsSectionTitleKeys[Index]).c_str());
            Widget.mCitizenPoliticsSectionTitles[
                static_cast<size_t>(Index)] = Title;
        }
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount;
        ++Index)
    {
        auto Label = Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_CitizenPoliticsSatisfactionLabel_" +
                std::to_string(Index + 1),
            9).lock();
        auto Rail = Widget.CreateWidget<CImage>(
            "CitizenInfo_CitizenPoliticsSatisfactionRail_" +
                std::to_string(Index + 1),
            9).lock();
        auto Fill = Widget.CreateWidget<CImage>(
            "CitizenInfo_CitizenPoliticsSatisfactionFill_" +
                std::to_string(Index + 1),
            10).lock();

        if (Label)
        {
            SetPanelTextStyle(
                Label,
                17.f,
                FVector4(0.28f, 0.28f, 0.28f, 1.f),
                ETextAlignH::Left,
                ETextAlignV::Middle,
                false);
            Widget.mCitizenPoliticsSatisfactionLabels[
                static_cast<size_t>(Index)] = Label;
        }

        if (Rail)
        {
            Rail->SetTexture(
                "CitizenInfo_CitizenPoliticsSatisfactionRailTexture_" +
                    std::to_string(Index),
                GCitizenPoliticsBarRailTexture);
            Rail->SetTint(0.90f, 0.89f, 0.82f, 0.92f);
            Widget.mCitizenPoliticsSatisfactionRails[
                static_cast<size_t>(Index)] = Rail;
        }

        if (Fill)
        {
            Fill->SetTexture(
                "CitizenInfo_CitizenPoliticsSatisfactionFillTexture_" +
                    std::to_string(Index),
                GCitizenPoliticsBarFillTexture);
            Fill->SetTint(0.22f, 0.53f, 0.90f, 0.95f);
            Widget.mCitizenPoliticsSatisfactionFills[
                static_cast<size_t>(Index)] = Fill;
        }
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenPoliticsOpinionCount;
        ++Index)
    {
        auto Text = Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_CitizenPoliticsOpinion_" +
                std::to_string(Index + 1),
            9).lock();

        if (Text)
        {
            SetPanelTextStyle(
                Text,
                17.f,
                FVector4(0.28f, 0.28f, 0.28f, 1.f),
                ETextAlignH::Left,
                ETextAlignV::Middle,
                false);
            Widget.mCitizenPoliticsOpinionTexts[static_cast<size_t>(Index)] =
                Text;
        }
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenPoliticsSupportIconCount;
        ++Index)
    {
        auto Icon = Widget.CreateWidget<CImage>(
            "CitizenInfo_CitizenPoliticsSupportIcon_" +
                std::to_string(Index + 1),
            9).lock();

        if (!Icon)
            continue;

        Icon->SetTexture(
            "CitizenInfo_CitizenPoliticsSupportIconTexture_" +
                std::to_string(Index),
            GCitizenPoliticsSupportIconTexture);
        Icon->SetTint(
            Index == 0 ?
                FVector4(0.92f, 0.62f, 0.48f, 0.95f) :
                (Index == 1 ?
                    FVector4(0.82f, 0.82f, 0.82f, 0.95f) :
                    FVector4(0.42f, 0.72f, 0.36f, 0.98f)));
        Widget.mCitizenPoliticsSupportIcons[static_cast<size_t>(Index)] = Icon;
    }

    auto SupportRail = Widget.CreateWidget<CImage>(
        "CitizenInfo_CitizenPoliticsSupportRail",
        9).lock();

    if (SupportRail)
    {
        SupportRail->SetTexture(
            "CitizenInfo_CitizenPoliticsSupportRailTexture",
            GCitizenPoliticsBarRailTexture);
        SupportRail->SetTint(0.90f, 0.89f, 0.82f, 0.92f);
        Widget.mCitizenPoliticsSupportRail = SupportRail;
    }

    auto SupportThumb = Widget.CreateWidget<CImage>(
        "CitizenInfo_CitizenPoliticsSupportThumb",
        10).lock();

    if (SupportThumb)
    {
        SupportThumb->SetTexture(
            "CitizenInfo_CitizenPoliticsSupportThumbTexture",
            GCitizenPoliticsBarFillTexture);
        SupportThumb->SetTint(0.20f, 0.72f, 0.16f, 0.98f);
        Widget.mCitizenPoliticsSupportThumb = SupportThumb;
    }

    auto CitizenThoughtTitleBackground = Widget.CreateWidget<CImage>(
        "CitizenInfo_CitizenThoughtTitleBackground",
        9).lock();

    if (CitizenThoughtTitleBackground)
    {
        CitizenThoughtTitleBackground->SetTexture(
            "CitizenInfo_CitizenThoughtTitleBackgroundTexture",
            GCitizenThoughtTitleTexture);
        CitizenThoughtTitleBackground->SetTint(1.f, 0.98f, 0.92f, 0.94f);
        Widget.mCitizenThoughtTitleBackground = CitizenThoughtTitleBackground;
    }

    auto CitizenThoughtTitleText = Widget.CreateWidget<CTextBlock>(
        "CitizenInfo_CitizenThoughtTitleText",
        10).lock();

    if (CitizenThoughtTitleText)
    {
        SetPanelTextStyle(
            CitizenThoughtTitleText,
            17.f,
            FVector4(0.36f, 0.24f, 0.10f, 1.f),
            ETextAlignH::Center,
            ETextAlignV::Middle,
            true);
        CitizenThoughtTitleText->SetText(
            UIStrings::Get(L"citizen_info.thought.title").c_str());
        Widget.mCitizenThoughtTitleText = CitizenThoughtTitleText;
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenThoughtCount;
        ++Index)
    {
        auto Text = Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_CitizenThoughtText_" +
                std::to_string(Index + 1),
            9).lock();

        if (!Text)
            continue;

        SetPanelTextStyle(
            Text,
            16.f,
            FVector4(0.24f, 0.24f, 0.24f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Top,
            false);
        Widget.mCitizenThoughtTexts[static_cast<size_t>(Index)] = Text;
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenThoughtDividerCount;
        ++Index)
    {
        auto Divider = Widget.CreateWidget<CImage>(
            "CitizenInfo_CitizenThoughtDivider_" +
                std::to_string(Index + 1),
            9).lock();

        if (!Divider)
            continue;

        Divider->SetTexture(
            "CitizenInfo_CitizenThoughtDividerTexture_" +
                std::to_string(Index),
            GSectionDividerTexture);
        Divider->SetTint(0.86f, 0.98f, 1.08f, 0.96f);
        Widget.mCitizenThoughtDividers[static_cast<size_t>(Index)] = Divider;
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

    auto OverviewBudgetLabel =
        Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_OverviewBudgetLabel",
            9).lock();

    if (OverviewBudgetLabel)
    {
        SetPanelTextStyle(
            OverviewBudgetLabel,
            18.f,
            FVector4(0.24f, 0.24f, 0.24f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Middle,
            false);
        Widget.mOverviewBudgetLabel = OverviewBudgetLabel;
    }

    auto OverviewBudgetValue =
        Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_OverviewBudgetValue",
            9).lock();

    if (OverviewBudgetValue)
    {
        SetPanelTextStyle(
            OverviewBudgetValue,
            18.f,
            FVector4(0.28f, 0.24f, 0.20f, 1.f),
            ETextAlignH::Right,
            ETextAlignV::Middle,
            false);
        Widget.mOverviewBudgetValue = OverviewBudgetValue;
    }

    auto OverviewOccupancyLabel =
        Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_OverviewOccupancyLabel",
            9).lock();

    if (OverviewOccupancyLabel)
    {
        SetPanelTextStyle(
            OverviewOccupancyLabel,
            18.f,
            FVector4(0.24f, 0.24f, 0.24f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Middle,
            false);
        Widget.mOverviewOccupancyLabel = OverviewOccupancyLabel;
    }

    auto OverviewOccupancyValue =
        Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_OverviewOccupancyValue",
            9).lock();

    if (OverviewOccupancyValue)
    {
        SetPanelTextStyle(
            OverviewOccupancyValue,
            18.f,
            FVector4(0.28f, 0.24f, 0.20f, 1.f),
            ETextAlignH::Right,
            ETextAlignV::Middle,
            false);
        Widget.mOverviewOccupancyValue = OverviewOccupancyValue;
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
        ++Index)
    {
        auto Icon = Widget.CreateWidget<CImage>(
            "CitizenInfo_OverviewResidentIcon_" + std::to_string(Index + 1),
            9).lock();

        if (!Icon)
            continue;

        Icon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mOverviewResidentIcons[static_cast<size_t>(Index)] = Icon;
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
        ++Index)
    {
        auto Icon = Widget.CreateWidget<CImage>(
            "CitizenInfo_OverviewVisitorIcon_" + std::to_string(Index + 1),
            9).lock();

        if (!Icon)
            continue;

        Icon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)] = Icon;
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GOverviewMetricRowCount;
        ++Index)
    {
        auto Label = Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_OverviewMetricLabel_" + std::to_string(Index + 1),
            9).lock();
        auto Value = Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_OverviewMetricValue_" + std::to_string(Index + 1),
            9).lock();

        if (Label)
        {
            SetPanelTextStyle(
                Label,
                17.f,
                FVector4(0.24f, 0.24f, 0.24f, 1.f),
                ETextAlignH::Left,
                ETextAlignV::Middle,
                false);
            Widget.mOverviewMetricLabels[static_cast<size_t>(Index)] = Label;
        }

        if (Value)
        {
            SetPanelTextStyle(
                Value,
                17.f,
                FVector4(0.28f, 0.24f, 0.20f, 1.f),
                ETextAlignH::Right,
                ETextAlignV::Middle,
                false);
            Widget.mOverviewMetricValues[static_cast<size_t>(Index)] = Value;
        }
    }

    auto UpgradeCardBackground =
        Widget.CreateWidget<CImage>(
            "CitizenInfo_UpgradeCardBackground",
            9).lock();

    if (UpgradeCardBackground)
    {
        UpgradeCardBackground->SetTexture(
            "CitizenInfoUpgradeCardBackgroundTexture",
            GMenuDetailFrameTexture);
        UpgradeCardBackground->SetTint(1.f, 1.f, 1.f, 0.96f);
        Widget.mUpgradeCardBackground = UpgradeCardBackground;
    }

    auto UpgradeCardIcon =
        Widget.CreateWidget<CImage>(
            "CitizenInfo_UpgradeCardIcon",
            10).lock();

    if (UpgradeCardIcon)
    {
        UpgradeCardIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mUpgradeCardIcon = UpgradeCardIcon;
    }

    auto UpgradeCardTitle =
        Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_UpgradeCardTitle",
            10).lock();

    if (UpgradeCardTitle)
    {
        SetPanelTextStyle(
            UpgradeCardTitle,
            18.f,
            FVector4(0.26f, 0.24f, 0.20f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Middle,
            false);
        Widget.mUpgradeCardTitle = UpgradeCardTitle;
    }

    auto UpgradeDescriptionText =
        Widget.CreateWidget<CTextBlock>(
            "CitizenInfo_UpgradeDescriptionText",
            9).lock();

    if (UpgradeDescriptionText)
    {
        SetPanelTextStyle(
            UpgradeDescriptionText,
            16.f,
            FVector4(0.22f, 0.22f, 0.22f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Top,
            false);
        Widget.mUpgradeDescriptionText = UpgradeDescriptionText;
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
                if (Widget.SelectCurrentModeTab(Index))
                    Widget.RefreshFromState();
            });

        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            "CitizenInfo_TabLabel_" + std::to_string(Index + 1),
            Widget.mWorld);
        auto Icon = CWidget::CreateStaticWidget<CImage>(
            "CitizenInfo_TabIcon_" + std::to_string(Index + 1),
            Widget.mWorld);

        if (Label)
        {
            Label->SetText(
                CitizenInfoConstants::GetBuildingTabLabel(Index).c_str());
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

        if (Icon)
        {
            Icon->SetTexture(
                "CitizenInfoTabIconTexture_" + std::to_string(Index),
                GBuildingTabIcons[Index]);
            Icon->SetTint(1.f, 1.f, 1.f, 1.f);
            Widget.mTabButtonIcons[static_cast<size_t>(Index)] = Icon;
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
            UIStrings::Get(L"citizen_info.action.demolish").c_str(),
            &CCitizenInfoWidget::OnDemolishButtonClick);
    Widget.mMoveButton =
        CreateActionButton(
            "CitizenInfo_MoveButton",
            UIStrings::Get(L"citizen_info.action.move").c_str(),
            &CCitizenInfoWidget::OnMoveButtonClick);
    Widget.mCloneButton =
        CreateActionButton(
            "CitizenInfo_CloneButton",
            UIStrings::Get(L"citizen_info.action.clone").c_str(),
            &CCitizenInfoWidget::OnCloneButtonClick);

    auto OverviewCommandButton = Widget.CreateWidget<CButton>(
        "CitizenInfo_OverviewCommandButton",
        9).lock();

    if (OverviewCommandButton)
    {
        ApplyButtonTextureSet(
            OverviewCommandButton,
            "CitizenInfo_OverviewCommandButton_Texture",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureDefaultButtonStyle(OverviewCommandButton);
        OverviewCommandButton->SetEventCallback<CCitizenInfoWidget>(
            EButtonEventState::Click,
            &Widget,
            &CCitizenInfoWidget::OnOverviewCommandButtonClick);

        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            "CitizenInfo_OverviewCommandButton_Label",
            Widget.mWorld);

        if (Label)
        {
            Label->SetText(L"");
            SetPanelTextStyle(
                Label,
                18.f,
                FVector4(0.29f, 0.22f, 0.12f, 1.f),
                ETextAlignH::Center,
                ETextAlignV::Middle,
                true);
            OverviewCommandButton->SetChild(Label);
            Widget.mOverviewCommandButtonText = Label;
        }

        Widget.mOverviewCommandButton = OverviewCommandButton;
    }

    for (int Index = 0; Index < GCitizenActionButtonCount; ++Index)
    {
        auto Button = Widget.CreateWidget<CButton>(
            "CitizenInfo_CitizenActionButton_" + std::to_string(Index + 1),
            9).lock();

        if (!Button)
            continue;

        ApplyButtonTextureSet(
            Button,
            "CitizenInfo_CitizenActionButton_Texture_" + std::to_string(Index),
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureDefaultButtonStyle(Button);

        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            "CitizenInfo_CitizenActionButton_Label_" +
                std::to_string(Index + 1),
            Widget.mWorld);

        if (Label)
        {
            Label->SetText(L"");
            SetPanelTextStyle(
                Label,
                17.f,
                FVector4(0.29f, 0.22f, 0.12f, 1.f),
                ETextAlignH::Left,
                ETextAlignV::Middle,
                true);
            Button->SetChild(Label);
            Widget.mCitizenActionButtonTexts[static_cast<size_t>(Index)] =
                Label;
        }

        auto Icon = Widget.CreateWidget<CImage>(
            "CitizenInfo_CitizenActionIcon_" + std::to_string(Index + 1),
            10).lock();

        if (Icon)
        {
            Icon->SetTexture(
                "CitizenInfo_CitizenActionIcon_Texture_" +
                    std::to_string(Index),
                GCitizenActionIcons[Index]);
            Icon->SetTint(1.f, 1.f, 1.f, 1.f);
            Widget.mCitizenActionButtonIcons[static_cast<size_t>(Index)] =
                Icon;
        }

        Widget.mCitizenActionButtons[static_cast<size_t>(Index)] = Button;
    }

    auto CitizenFooterText =
        Widget.CreateWidget<CTextBlock>("CitizenInfo_CitizenFooterText", 9).lock();

    if (CitizenFooterText)
    {
        SetPanelTextStyle(
            CitizenFooterText,
            15.f,
            FVector4(0.58f, 0.84f, 0.88f, 0.92f),
            ETextAlignH::Center,
            ETextAlignV::Middle,
            false);
        Widget.mCitizenFooterText = CitizenFooterText;
    }

    auto ConfigureActionIcon =
        [&Widget](
            const std::shared_ptr<CButton>& Button,
            const std::string& TextureKey,
            const TCHAR* IconTexture)
    {
        if (!Button || !IconTexture)
            return;

        ApplyButtonTextureSet(
            Button,
            TextureKey,
            GActionIconButtonTexture,
            GActionIconButtonTexture,
            GActionIconButtonTexture,
            GActionIconButtonTexture);
        ConfigureIconSlotButtonStyle(Button);

        auto Icon = CWidget::CreateStaticWidget<CImage>(
            TextureKey + "_Icon",
            Widget.mWorld);

        if (!Icon)
            return;

        Icon->SetTexture(TextureKey + "_Image", IconTexture);
        Icon->SetTint(1.f, 1.f, 1.f, 1.f);
        Button->SetChild(Icon);
    };

    if (auto MoveButton = Widget.mMoveButton.lock())
        ConfigureActionIcon(
            MoveButton,
            "CitizenInfoMoveAction",
            GMoveActionIcon);

    if (auto CloneButton = Widget.mCloneButton.lock())
        ConfigureActionIcon(
            CloneButton,
            "CitizenInfoCloneAction",
            GCloneActionIcon);

    if (auto CloneButton = Widget.mCloneButton.lock())
    {
        CloneButton->ButtonEnable(false);
        CloneButton->SetOpacityAll(0.72f);
    }
}
