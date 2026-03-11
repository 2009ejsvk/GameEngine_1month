#include "CitizenInfoRenderer.h"
#include "CitizenInfoWidget.h"
#include "TropicoUiStyle.h"
#include "UILayoutConfig.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include <array>
#include <algorithm>
#include <cwchar>


namespace
{
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

    constexpr int GBuildingTabCount = 5;
    constexpr int GCitizenTabCount = 3;
    constexpr int GBudgetLevelCount = 5;
    constexpr int GCitizenActionButtonCount = 6;
    constexpr const wchar_t* GBuildingTabLabels[GBuildingTabCount] =
    {
        L"기본",
        L"통계",
        L"업글",
        L"효율",
        L"정보"
    };
    constexpr const wchar_t* GCitizenTabLabels[GBuildingTabCount] =
    {
        L"기본",
        L"정치",
        L"성향",
        L"",
        L""
    };
    constexpr const TCHAR* GCitizenTabIcons[GCitizenTabCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_formalWorker1_m_1.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_theatre.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingDescription.png")
    };
    constexpr const TCHAR* GBuildingTabIcons[GBuildingTabCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingSettings.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingStatus.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingUpgrades.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingEfficiency.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingDescription.png")
    };
    constexpr const TCHAR* GBudgetIcons[GBudgetLevelCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BudgetIcons\\T_ICO_budget_01.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BudgetIcons\\T_ICO_budget_02.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BudgetIcons\\T_ICO_budget_03.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BudgetIcons\\T_ICO_budget_04.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BudgetIcons\\T_ICO_budget_05.png")
    };
    constexpr const TCHAR* GActionIconButtonTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\IconBackground\\T_icon_background.png");
    constexpr const TCHAR* GMoveActionIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingInfoIcons\\T_ICO_buildingAddRoute.png");
    constexpr const TCHAR* GCloneActionIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\ConstructionIcons\\T_ICO_blueprint.png");
    constexpr const TCHAR* GOverviewEmptyResidentIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_population.png");
    constexpr const TCHAR* GSectionDividerTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\4_Modern\\InfoPopUp\\T_agent_deco.png");
    constexpr const TCHAR* GCitizenPoliticsSectionTexture = GMenuDetailFrameTexture;
    constexpr const TCHAR* GCitizenPoliticsBarRailTexture = GScrollTrackTexture;
    constexpr const TCHAR* GCitizenPoliticsBarFillTexture = GScrollThumbTexture;
    constexpr const TCHAR* GCitizenPoliticsSupportIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_political_approval.png");
    constexpr const TCHAR* GCitizenThoughtTitleTexture = GMenuDetailFrameTexture;
    constexpr std::array<const TCHAR*, 4> GOverviewResidentPortraits =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_formalWorker1_m_1.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_formalWorker1_f_2.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_LightDutyWorker2_m_3.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TropicanIcons\\T_CW_LightDutyWorker2_f_4.png")
    };
    constexpr const TCHAR* GCitizenActionIcons[GCitizenActionButtonCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_weaponsFactory.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_nuclearTesting.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_guardTower.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_asylum.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_spaceProgram.png")
    };

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

    constexpr const wchar_t* GCitizenPoliticsSectionTitles[
        CCitizenInfoWidget::GCitizenPoliticsSectionCount] =
    {
        L"만족도",
        L"견해",
        L"지지도"
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
            Title->SetText(GCitizenPoliticsSectionTitles[Index]);
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
        CitizenThoughtTitleText->SetText(L"생각");
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
                Widget.SelectBuildingTab(
                    static_cast<CCitizenInfoWidget::EBuildingInfoTab>(Index));
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
            Label->SetText(GBuildingTabLabels[Index]);
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

void FCitizenInfoRenderer::ApplySnapshot(
    CCitizenInfoWidget& Widget,
    const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot)
{
    const bool IsCitizenMode =
        (Snapshot.Mode == CitizenInfoDataProvider::EPanelMode::Citizen);
    const bool ShowCitizenProfile = Snapshot.ShowCitizenProfileOverview;
    const bool ShowCitizenPolitics = Snapshot.ShowCitizenPoliticsOverview;
    const bool ShowCitizenThoughts = Snapshot.ShowCitizenThoughtsOverview;
    const bool ShowMetricRows =
        ShowCitizenProfile ||
        Snapshot.ShowBuildingOverview ||
        Snapshot.ShowBuildingWorkOverview ||
        Snapshot.ShowBuildingMetricRows;
    const bool ShowUpgradeCard = Snapshot.ShowBuildingUpgradeCard;
    const bool ShowInformationParagraphs =
        Snapshot.ShowBuildingInformationParagraphs;
    const bool ShowOverviewLayout =
        Snapshot.ShowBuildingOverview ||
        Snapshot.ShowBuildingWorkOverview;

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
        if (Snapshot.ShowHeaderNote)
        {
            SubtitleText->SetText(Snapshot.HeaderNoteText.c_str());
            SubtitleText->SetAlignH(ETextAlignH::Left);
            SubtitleText->SetAlignV(ETextAlignV::Middle);
            SubtitleText->SetTextColor(FVector4(0.24f, 0.24f, 0.24f, 1.f));
            SubtitleText->EnableShadow(false);
            SubtitleText->SetEnable(!Snapshot.HeaderNoteText.empty());
        }
        else if (Snapshot.ShowBuildingSubtitle)
        {
            SubtitleText->SetText(Snapshot.BuildingSubtitleText.c_str());
            SubtitleText->SetAlignH(ETextAlignH::Center);
            SubtitleText->SetAlignV(ETextAlignV::Middle);
            SubtitleText->SetTextColor(FVector4(0.26f, 0.62f, 0.82f, 1.f));
            SubtitleText->EnableShadow(false);
            SubtitleText->SetEnable(!Snapshot.BuildingSubtitleText.empty());
        }
        else
        {
            SubtitleText->SetText(Snapshot.Subtitle.c_str());
            SubtitleText->SetAlignH(ETextAlignH::Center);
            SubtitleText->SetAlignV(ETextAlignV::Middle);
            SubtitleText->SetTextColor(FVector4(0.35f, 0.30f, 0.22f, 1.f));
            SubtitleText->EnableShadow(false);
            SubtitleText->SetEnable(
                IsCitizenMode &&
                !Snapshot.Subtitle.empty());
        }
    }

    if (PageTitleText)
    {
        PageTitleText->SetText(Snapshot.PageTitle.c_str());
        PageTitleText->SetEnable(Snapshot.ShowSectionRibbon);
    }

    if (BodyText)
    {
        BodyText->SetText(Snapshot.BodyText.c_str());
        BodyText->SetEnable(
            !ShowCitizenProfile &&
            !ShowCitizenPolitics &&
            !ShowCitizenThoughts &&
            !ShowOverviewLayout &&
            !Snapshot.ShowBuildingMetricRows &&
            !Snapshot.ShowBuildingUpgradeCard &&
            !ShowInformationParagraphs);
    }

    if (BudgetText)
    {
        BudgetText->SetText(Snapshot.BudgetText.c_str());
        BudgetText->SetEnable(
            Snapshot.ShowBudgetControls &&
            !ShowOverviewLayout);
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
        auto Icon = Widget.mTabButtonIcons[static_cast<size_t>(Index)].lock();
        const bool Selected = Snapshot.SelectedTabIndex == Index;

        // 시민 모드: 탭 0~2만 표시, 건물 모드: 전체 5탭 표시
        const bool TabVisible = IsCitizenMode ?
            (Index < GCitizenTabCount) : true;

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
            Button->SetEnable(Snapshot.ShowTabButtons && TabVisible);
        }

        if (Label)
        {
            if (!IsCitizenMode)
                Label->SetText(GBuildingTabLabels[Index]);

            Label->SetTextColor(
                Selected ?
                    FVector4(0.27f, 0.17f, 0.06f, 1.f) :
                    FVector4(0.22f, 0.20f, 0.17f, 1.f));
            Label->SetEnable(!IsCitizenMode);
        }

        if (Icon)
        {
            if (IsCitizenMode && Index < GCitizenTabCount)
            {
                Icon->SetTexture(
                    "CitizenInfoCitizenTabIconTexture_" +
                        std::to_string(Index),
                    GCitizenTabIcons[Index]);
            }
            else if (!IsCitizenMode)
            {
                Icon->SetTexture(
                    "CitizenInfoTabIconRefreshTexture_" +
                        std::to_string(Index),
                    GBuildingTabIcons[Index]);
            }

            Icon->SetEnable(
                (IsCitizenMode && Index < GCitizenTabCount) ||
                !IsCitizenMode);
            Icon->SetTint(
                Selected ?
                    FVector4(1.f, 1.f, 1.f, 1.f) :
                    FVector4(0.92f, 0.92f, 0.92f, 0.95f));
        }

        if (Button)
        {
            if (IsCitizenMode && Icon)
                Button->SetChild(Icon);
            else if (!IsCitizenMode && Icon)
                Button->SetChild(Icon);
        }
    }

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        auto Button = Widget.mBudgetButtons[static_cast<size_t>(Index)].lock();
        auto Label = Widget.mBudgetButtonTexts[static_cast<size_t>(Index)].lock();
        const bool Selected = Snapshot.BudgetLevel == Index + 1;

        if (Button)
        {
            if (ShowOverviewLayout)
            {
                ApplyButtonTextureSet(
                    Button,
                    "CitizenInfoBudgetIconRefresh_" + std::to_string(Index),
                    GBudgetIcons[Index],
                    GBudgetIcons[Index],
                    GBudgetIcons[Index],
                    GBudgetIcons[Index]);
                ConfigureIconSlotButtonStyle(Button);
            }
            else
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
                ConfigureDefaultButtonStyle(Button);
            }
            Button->SetEnable(Snapshot.ShowBudgetControls);
            Button->ButtonEnable(Snapshot.ShowBudgetControls);
            Button->SetOpacityAll(
                ShowOverviewLayout && !Selected ?
                    0.45f :
                    1.f);
        }

        if (Label)
        {
            Label->SetTextColor(
                Selected ?
                    FVector4(0.36f, 0.22f, 0.08f, 1.f) :
                    FVector4(0.30f, 0.22f, 0.12f, 1.f));
            Label->SetEnable(!ShowOverviewLayout);
        }
    }

    if (auto Text = Widget.mOverviewWorkModeLabel.lock())
    {
        Text->SetText(Snapshot.OverviewWorkModeLabel.c_str());
        Text->SetEnable(Snapshot.ShowBuildingWorkOverview);
    }

    if (auto Background = Widget.mOverviewWorkModeBackground.lock())
        Background->SetEnable(Snapshot.ShowBuildingWorkOverview);

    if (auto Text = Widget.mOverviewWorkModeText.lock())
    {
        Text->SetText(Snapshot.OverviewWorkModeValue.c_str());
        Text->SetEnable(
            Snapshot.ShowBuildingWorkOverview &&
            !Snapshot.OverviewWorkModeValue.empty());
    }

    if (auto Text = Widget.mOverviewBudgetLabel.lock())
    {
        Text->SetText(Snapshot.OverviewBudgetLabel.c_str());
        Text->SetEnable(ShowOverviewLayout);
    }

    if (auto Text = Widget.mOverviewBudgetValue.lock())
    {
        Text->SetText(Snapshot.OverviewBudgetValue.c_str());
        Text->SetEnable(ShowOverviewLayout);
    }

    if (auto Text = Widget.mOverviewOccupancyLabel.lock())
    {
        Text->SetText(Snapshot.OverviewOccupancyLabel.c_str());
        Text->SetEnable(
            ShowOverviewLayout &&
            Snapshot.OverviewResidentCapacity > 0);
    }

    if (auto Text = Widget.mOverviewOccupancyValue.lock())
    {
        Text->SetText(Snapshot.OverviewOccupancyValue.c_str());
        Text->SetEnable(
            ShowOverviewLayout &&
            Snapshot.OverviewResidentCapacity > 0);
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GOverviewMetricRowCount;
        ++Index)
    {
        auto Label =
            Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock();
        auto Value =
            Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock();
        const bool LabelEnabled =
            ShowMetricRows &&
            !Snapshot.OverviewMetricLabels[static_cast<size_t>(Index)].empty();
        const bool ValueEnabled =
            LabelEnabled &&
            !Snapshot.OverviewMetricValues[static_cast<size_t>(Index)].empty();
        const bool SectionHeader =
            LabelEnabled &&
            !ValueEnabled;

        if (Label)
        {
            Label->SetText(
                Snapshot.OverviewMetricLabels[static_cast<size_t>(Index)].c_str());
            Label->SetFontSize(
                ShowCitizenProfile ? 18.f :
                    (SectionHeader ? 18.f : 17.f));
            Label->SetTextColor(
                ShowCitizenProfile ?
                    FVector4(0.33f, 0.30f, 0.26f, 1.f) :
                    (SectionHeader ?
                        FVector4(0.24f, 0.24f, 0.24f, 1.f) :
                        FVector4(0.28f, 0.28f, 0.28f, 1.f)));
            Label->SetEnable(LabelEnabled);
        }

        if (Value)
        {
            Value->SetText(
                Snapshot.OverviewMetricValues[static_cast<size_t>(Index)].c_str());
            Value->SetTextColor(
                Snapshot.OverviewMetricAccentValues[static_cast<size_t>(Index)] ?
                    FVector4(0.33f, 0.62f, 0.88f, 1.f) :
                    (ShowCitizenProfile ?
                        FVector4(0.27f, 0.27f, 0.27f, 1.f) :
                        FVector4(0.28f, 0.24f, 0.20f, 1.f)));
            Value->SetEnable(ValueEnabled);
        }
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
        ++Index)
    {
        auto Icon =
            Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock();

        if (!Icon)
            continue;

        const bool Enabled =
            ShowCitizenProfile ?
                (Index < Snapshot.CitizenPortraitSlotCount) :
                (ShowOverviewLayout &&
                    Index < Snapshot.OverviewResidentCapacity &&
                    Index < CCitizenInfoWidget::GOverviewResidentSlotCount);

        if (!Enabled)
        {
            Icon->SetEnable(false);
            continue;
        }

        if (ShowCitizenProfile)
        {
            if (Index == Snapshot.CitizenPortraitOccupiedSlot)
            {
                const size_t PortraitIndex =
                    static_cast<size_t>(
                        (std::max)(0, Snapshot.CitizenPortraitVariant)) %
                    GOverviewResidentPortraits.size();
                Icon->SetTexture(
                    "CitizenInfoCitizenPortrait_" + std::to_string(Index),
                    GOverviewResidentPortraits[PortraitIndex]);
                Icon->SetTint(1.f, 1.f, 1.f, 1.f);
            }
            else
            {
                Icon->SetTexture(
                    "CitizenInfoCitizenPortraitEmpty_" + std::to_string(Index),
                    GMenuDetailFrameTexture);
                Icon->SetTint(0.94f, 0.92f, 0.86f, 0.92f);
            }
        }
        else if (Index < Snapshot.OverviewResidentCount)
        {
            const size_t PortraitIndex =
                static_cast<size_t>(Index) % GOverviewResidentPortraits.size();
            Icon->SetTexture(
                "CitizenInfoOverviewResidentOccupied_" + std::to_string(Index),
                GOverviewResidentPortraits[PortraitIndex]);
            Icon->SetTint(1.f, 1.f, 1.f, 1.f);
        }
        else
        {
            Icon->SetTexture(
                "CitizenInfoOverviewResidentEmpty_" + std::to_string(Index),
                GOverviewEmptyResidentIcon);
            Icon->SetTint(1.00f, 0.90f, 0.35f, 0.85f);
        }

        Icon->SetEnable(true);
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
        ++Index)
    {
        auto Icon =
            Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock();

        if (!Icon)
            continue;

        const bool Enabled =
            Snapshot.ShowBuildingVisitorIcons &&
            Index < Snapshot.OverviewVisitorCapacity &&
            Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;

        if (!Enabled)
        {
            Icon->SetEnable(false);
            continue;
        }

        const size_t PortraitIndex =
            static_cast<size_t>(Index) % GOverviewResidentPortraits.size();
        Icon->SetTexture(
            "CitizenInfoOverviewVisitor_" + std::to_string(Index),
            GOverviewResidentPortraits[PortraitIndex]);
        Icon->SetTint(
            Index < Snapshot.OverviewVisitorCount ?
                FVector4(1.f, 1.f, 1.f, 1.f) :
                FVector4(0.72f, 0.72f, 0.72f, 0.65f));
        Icon->SetEnable(true);
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenPoliticsSectionCount;
        ++Index)
    {
        if (auto Background =
            Widget.mCitizenPoliticsSectionBackgrounds[
                static_cast<size_t>(Index)].lock())
        {
            Background->SetEnable(ShowCitizenPolitics);
        }

        if (auto Title =
            Widget.mCitizenPoliticsSectionTitles[
                static_cast<size_t>(Index)].lock())
        {
            Title->SetEnable(ShowCitizenPolitics);
        }
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount;
        ++Index)
    {
        Widget.mCitizenPoliticsSatisfactionFillRatios[
            static_cast<size_t>(Index)] =
                Snapshot.CitizenPoliticsSatisfactionRatios[
                    static_cast<size_t>(Index)];

        if (auto Label =
            Widget.mCitizenPoliticsSatisfactionLabels[
                static_cast<size_t>(Index)].lock())
        {
            Label->SetText(
                Snapshot.CitizenPoliticsSatisfactionLabels[
                    static_cast<size_t>(Index)].c_str());
            Label->SetEnable(
                ShowCitizenPolitics &&
                !Snapshot.CitizenPoliticsSatisfactionLabels[
                    static_cast<size_t>(Index)].empty());
        }

        if (auto Rail =
            Widget.mCitizenPoliticsSatisfactionRails[
                static_cast<size_t>(Index)].lock())
        {
            Rail->SetEnable(
                ShowCitizenPolitics &&
                !Snapshot.CitizenPoliticsSatisfactionLabels[
                    static_cast<size_t>(Index)].empty());
        }

        if (auto Fill =
            Widget.mCitizenPoliticsSatisfactionFills[
                static_cast<size_t>(Index)].lock())
        {
            Fill->SetEnable(
                ShowCitizenPolitics &&
                !Snapshot.CitizenPoliticsSatisfactionLabels[
                    static_cast<size_t>(Index)].empty());
        }
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenPoliticsOpinionCount;
        ++Index)
    {
        if (auto Text =
            Widget.mCitizenPoliticsOpinionTexts[
                static_cast<size_t>(Index)].lock())
        {
            Text->SetText(
                Snapshot.CitizenPoliticsOpinionLines[
                    static_cast<size_t>(Index)].c_str());
            Text->SetEnable(
                ShowCitizenPolitics &&
                !Snapshot.CitizenPoliticsOpinionLines[
                    static_cast<size_t>(Index)].empty());
        }
    }

    Widget.mCitizenPoliticsSupportRatio =
        Snapshot.CitizenPoliticsSupportRatio;

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenPoliticsSupportIconCount;
        ++Index)
    {
        if (auto Icon =
            Widget.mCitizenPoliticsSupportIcons[
                static_cast<size_t>(Index)].lock())
        {
            Icon->SetEnable(ShowCitizenPolitics);
        }
    }

    if (auto Rail = Widget.mCitizenPoliticsSupportRail.lock())
        Rail->SetEnable(ShowCitizenPolitics);

    if (auto Thumb = Widget.mCitizenPoliticsSupportThumb.lock())
        Thumb->SetEnable(ShowCitizenPolitics);

    if (auto Background = Widget.mCitizenThoughtTitleBackground.lock())
        Background->SetEnable(ShowCitizenThoughts);

    if (auto Text = Widget.mCitizenThoughtTitleText.lock())
        Text->SetEnable(ShowCitizenThoughts);

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenThoughtCount;
        ++Index)
    {
        if (auto Text =
            Widget.mCitizenThoughtTexts[static_cast<size_t>(Index)].lock())
        {
            Text->SetText(
                Snapshot.CitizenThoughtLines[static_cast<size_t>(Index)].c_str());
            Text->SetEnable(
                ShowCitizenThoughts &&
                !Snapshot.CitizenThoughtLines[
                    static_cast<size_t>(Index)].empty());
        }
    }

    for (int Index = 0;
        Index < CCitizenInfoWidget::GCitizenThoughtDividerCount;
        ++Index)
    {
        if (auto Divider =
            Widget.mCitizenThoughtDividers[static_cast<size_t>(Index)].lock())
        {
            Divider->SetEnable(ShowCitizenThoughts);
        }
    }

    if (auto Card = Widget.mUpgradeCardBackground.lock())
        Card->SetEnable(ShowUpgradeCard);

    if (auto Icon = Widget.mUpgradeCardIcon.lock())
    {
        const bool Enabled =
            ShowUpgradeCard &&
            Snapshot.UpgradeCardIconPath &&
            Icon->SetTexture(
                Snapshot.UpgradeCardIconTextureKey,
                Snapshot.UpgradeCardIconPath);
        Icon->SetEnable(Enabled);
    }

    if (auto Text = Widget.mUpgradeCardTitle.lock())
    {
        Text->SetText(Snapshot.UpgradeCardTitle.c_str());
        Text->SetEnable(ShowUpgradeCard);
    }

    if (auto Text = Widget.mUpgradeDescriptionText.lock())
    {
        Text->SetText(Snapshot.UpgradeCardDescription.c_str());
        Text->SetEnable(ShowUpgradeCard);
    }

    if (auto Text = Widget.mInformationAccentText.lock())
    {
        Text->SetText(Snapshot.InformationAccentText.c_str());
        Text->SetEnable(
            ShowInformationParagraphs &&
            !Snapshot.InformationAccentText.empty());
    }

    if (auto Text = Widget.mInformationTopText.lock())
    {
        Text->SetText(Snapshot.InformationTopText.c_str());
        Text->SetEnable(
            ShowInformationParagraphs &&
            !Snapshot.InformationTopText.empty());
    }

    if (auto Text = Widget.mInformationBottomText.lock())
    {
        Text->SetText(Snapshot.InformationBottomText.c_str());
        Text->SetEnable(
            ShowInformationParagraphs &&
            !Snapshot.InformationBottomText.empty());
    }

    if (auto Divider = Widget.mSectionDivider.lock())
        Divider->SetEnable(Snapshot.ShowSectionDivider);

    if (auto DemolishButton = Widget.mDemolishButton.lock())
        DemolishButton->SetEnable(Snapshot.ShowActionButtons);

    if (auto MoveButton = Widget.mMoveButton.lock())
        MoveButton->SetEnable(Snapshot.ShowActionButtons);

    if (auto CloneButton = Widget.mCloneButton.lock())
        CloneButton->SetEnable(Snapshot.ShowActionButtons);

    if (auto Button = Widget.mOverviewCommandButton.lock())
        Button->SetEnable(Snapshot.ShowOverviewCommandButton);

    if (auto Text = Widget.mOverviewCommandButtonText.lock())
    {
        Text->SetText(Snapshot.OverviewCommandButtonText.c_str());
        Text->SetEnable(Snapshot.ShowOverviewCommandButton);
    }

    for (int Index = 0; Index < CCitizenInfoWidget::GCitizenActionButtonCount;
        ++Index)
    {
        if (auto Button =
            Widget.mCitizenActionButtons[static_cast<size_t>(Index)].lock())
        {
            Button->SetEnable(Snapshot.ShowCitizenActionButtons);
        }

        if (auto Text =
            Widget.mCitizenActionButtonTexts[static_cast<size_t>(Index)].lock())
        {
            const std::wstring ButtonLabel =
                Snapshot.CitizenActionLabels[static_cast<size_t>(Index)].empty() ?
                    std::wstring() :
                    (L"      " +
                        Snapshot.CitizenActionLabels[static_cast<size_t>(Index)]);
            Text->SetText(ButtonLabel.c_str());
            Text->SetEnable(
                Snapshot.ShowCitizenActionButtons &&
                !Snapshot.CitizenActionLabels[static_cast<size_t>(Index)].empty());
        }

        if (auto Icon =
            Widget.mCitizenActionButtonIcons[static_cast<size_t>(Index)].lock())
        {
            Icon->SetEnable(Snapshot.ShowCitizenActionButtons);
        }
    }

    if (auto Text = Widget.mCitizenFooterText.lock())
    {
        Text->SetText(Snapshot.CitizenFooterText.c_str());
        Text->SetEnable(
            Snapshot.ShowCitizenProfileOverview &&
            !Snapshot.CitizenFooterText.empty());
    }
}

void FCitizenInfoRenderer::RefreshLayout(CCitizenInfoWidget& Widget)
{
    (void)Widget.mRequestedScreenPos;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth  = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);

    // INI 변수로 패널 크기 결정
    Widget.mPanelWidth = (std::min)(
        UIConfig::CitizenPanelMaxWidth,
        (std::max)(UIConfig::CitizenPanelMinWidth,
            ScreenWidth * UIConfig::CitizenPanelWidthRatio));
    Widget.mPanelTop    = UIConfig::CitizenPanelTopOffset;
    Widget.mPanelHeight = (std::max)(
        420.f,
        ScreenHeight - Widget.mPanelTop - 10.f);

    const float PanelLeft   = ScreenWidth - Widget.mPanelWidth - 10.f;
    Widget.SetPos(PanelLeft, 0.f);
    Widget.SetSize(Widget.mPanelWidth, Widget.mPanelTop + Widget.mPanelHeight);

    const float OuterTop    = Widget.mPanelTop;
    const float PanelWidth  = Widget.mPanelWidth;
    const float PanelHeight = Widget.mPanelHeight;

    const float InnerMarginX   = UIConfig::CitizenPanelInnerMarginX;
    const float InnerLeft      = InnerMarginX;
    const float InnerTop       = OuterTop + 16.f;
    const float InnerWidth     = PanelWidth - InnerMarginX * 2.f;
    const float InnerHeight    = PanelHeight - 28.f;

    const float ScrollTrackW   = UIConfig::CitizenScrollTrackWidth;
    const float ScrollThumbH   = UIConfig::CitizenScrollThumbHeight;
    const float TitleRibbonH   = UIConfig::CitizenTitleRibbonHeight;
    const float SectionRibbonH = UIConfig::CitizenSectionRibbonHeight;
    const float CloseButtonSz  = UIConfig::CitizenCloseButtonSize;
    const float IconSz         = UIConfig::BuildingIconSize;

    // 폰트 크기
    if (auto TitleTxt = Widget.mTitleText.lock())
        TitleTxt->SetFontSize(UIConfig::CitizenTitleFontSize);
    if (auto SubTxt = Widget.mSubtitleText.lock())
        SubTxt->SetFontSize(UIConfig::CitizenSubtitleFontSize);
    if (auto BodyTxt = Widget.mBodyText.lock())
        BodyTxt->SetFontSize(UIConfig::CitizenBodyFontSize);

    // 패널 배경
    if (auto PanelImage = Widget.mPanelImage.lock())
    {
        PanelImage->SetPos(0.f, OuterTop);
        PanelImage->SetSize(PanelWidth, PanelHeight);
    }

    // 내부 프레임
    if (auto InnerFrame = Widget.mInnerFrame.lock())
    {
        InnerFrame->SetPos(InnerLeft, InnerTop);
        InnerFrame->SetSize(InnerWidth, InnerHeight);
    }

    // 스크롤바
    if (auto ScrollTrack = Widget.mScrollTrack.lock())
    {
        ScrollTrack->SetPos(InnerMarginX * 0.5f - ScrollTrackW * 0.5f,
            OuterTop + TitleRibbonH + SectionRibbonH);
        ScrollTrack->SetSize(ScrollTrackW, PanelHeight - TitleRibbonH - SectionRibbonH - 52.f);
    }
    if (auto ScrollThumb = Widget.mScrollThumb.lock())
    {
        ScrollThumb->SetPos(InnerMarginX * 0.5f - ScrollTrackW * 0.5f + 0.5f,
            OuterTop + TitleRibbonH + SectionRibbonH + 14.f);
        ScrollThumb->SetSize(ScrollTrackW - 1.f, ScrollThumbH);
    }

    // 상단 탭 (건물/NPC 공용)
    const bool IsCitizenMode =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen;
    const int VisibleTabCount = IsCitizenMode ? GCitizenTabCount : GBuildingTabCount;
    const float TabWidth = UIConfig::BuildingTabWidth;
    const float TabHeight = UIConfig::BuildingTabHeight;
    const float TabGap   = UIConfig::BuildingTabGap;
    const float TotalTabsWidth =
        static_cast<float>(VisibleTabCount) * TabWidth +
        static_cast<float>((std::max)(0, VisibleTabCount - 1)) * TabGap;
    const float TabStartX = (std::max)(
        4.f, (PanelWidth - TotalTabsWidth) * 0.5f);

    for (int Index = 0; Index < GBuildingTabCount; ++Index)
    {
        auto Button = Widget.mTabButtons[static_cast<size_t>(Index)].lock();
        if (!Button)
            continue;

        Button->SetPos(
            TabStartX + static_cast<float>(Index) * (TabWidth + TabGap), 0.f);
        Button->SetSize(TabWidth, TabHeight);
    }

    // 제목 리본
    const float RibbonOffsetX = UIConfig::BuildingTitleRibbonOffsetX;
    const float RibbonOffsetY = UIConfig::BuildingTitleRibbonOffsetY;

    if (auto TitleRibbon = Widget.mTitleRibbon.lock())
    {
        TitleRibbon->SetPos(RibbonOffsetX, OuterTop + RibbonOffsetY);
        TitleRibbon->SetSize(PanelWidth - RibbonOffsetX * 2.f - CloseButtonSz, TitleRibbonH);
    }

    // 닫기 버튼
    const float CloseOffsetX = UIConfig::BuildingCloseButtonOffsetX;
    if (auto CloseButton = Widget.mCloseButton.lock())
    {
        CloseButton->SetPos(PanelWidth - CloseOffsetX, OuterTop + 4.f);
        CloseButton->SetSize(CloseButtonSz, CloseButtonSz);
    }

    // 아이콘 & 제목 텍스트
    const bool TitleIconVisible =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        !Widget.mTitleIcon.expired() &&
        Widget.mTitleIcon.lock()->GetEnable();
    const float TitleLeft = TitleIconVisible ? (RibbonOffsetX + IconSz + 6.f) : (RibbonOffsetX + 14.f);

    if (auto TitleIcon = Widget.mTitleIcon.lock())
    {
        TitleIcon->SetPos(RibbonOffsetX + 6.f, OuterTop + RibbonOffsetY + (TitleRibbonH - IconSz) * 0.5f);
        TitleIcon->SetSize(IconSz, IconSz);
    }
    if (auto TitleText = Widget.mTitleText.lock())
    {
        TitleText->SetPos(TitleLeft, OuterTop + RibbonOffsetY);
        TitleText->SetSize(PanelWidth - TitleLeft - CloseButtonSz - CloseOffsetX, TitleRibbonH);
    }
    if (auto SubtitleText = Widget.mSubtitleText.lock())
    {
        SubtitleText->SetPos(RibbonOffsetX, OuterTop + RibbonOffsetY + TitleRibbonH + 4.f);
        SubtitleText->SetSize(PanelWidth - RibbonOffsetX * 2.f, 22.f);
    }

    // 섹션 리본 (페이지 전환 시)
    const bool ShowSectionRibbon =
        !Widget.mSectionRibbon.expired() &&
        Widget.mSectionRibbon.lock()->GetEnable();
    const bool ShowCitizenProfile =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview &&
        !Widget.mCitizenFooterText.expired() &&
        Widget.mCitizenFooterText.lock()->GetEnable();
    const bool ShowCitizenPolitics =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Statistics &&
        !Widget.mCitizenPoliticsSectionTitles[0].expired() &&
        Widget.mCitizenPoliticsSectionTitles[0].lock()->GetEnable();
    const bool ShowCitizenThoughts =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Upgrades &&
        !Widget.mCitizenThoughtTitleText.expired() &&
        Widget.mCitizenThoughtTitleText.lock()->GetEnable();
    const float SectionRibbonY = OuterTop + RibbonOffsetY + TitleRibbonH + 28.f;
    const bool ShowWorkOverview =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview &&
        !Widget.mOverviewWorkModeLabel.expired() &&
        Widget.mOverviewWorkModeLabel.lock()->GetEnable();
    const bool ShowCustomOverview =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview &&
        !ShowWorkOverview &&
        !Widget.mOverviewBudgetLabel.expired() &&
        Widget.mOverviewBudgetLabel.lock()->GetEnable();
    const bool ShowAnyOverview =
        ShowCustomOverview ||
        ShowWorkOverview;
    const bool ShowCompactRows =
        !ShowCitizenProfile &&
        !ShowCitizenPolitics &&
        !ShowCitizenThoughts &&
        !ShowAnyOverview &&
        !Widget.mOverviewMetricLabels[0].expired() &&
        Widget.mOverviewMetricLabels[0].lock()->GetEnable();
    const bool ShowUpgradeCard =
        !Widget.mUpgradeCardBackground.expired() &&
        Widget.mUpgradeCardBackground.lock()->GetEnable();
    const bool ShowInformationParagraphs =
        !Widget.mInformationTopText.expired() &&
        Widget.mInformationTopText.lock()->GetEnable();
    const bool ShowOverviewCommandButton =
        !Widget.mOverviewCommandButton.expired() &&
        Widget.mOverviewCommandButton.lock()->GetEnable();

    if (auto SectionRibbon = Widget.mSectionRibbon.lock())
    {
        SectionRibbon->SetPos(RibbonOffsetX, SectionRibbonY);
        SectionRibbon->SetSize(PanelWidth - RibbonOffsetX * 2.f, SectionRibbonH);
        SectionRibbon->SetEnable(ShowSectionRibbon);
    }
    if (auto PageTitleText = Widget.mPageTitleText.lock())
    {
        PageTitleText->SetPos(RibbonOffsetX, SectionRibbonY);
        PageTitleText->SetSize(PanelWidth - RibbonOffsetX * 2.f, SectionRibbonH);
        PageTitleText->SetEnable(ShowSectionRibbon);
    }

    if (auto ScrollTrack = Widget.mScrollTrack.lock())
    {
        const float ScrollTop =
            OuterTop + TitleRibbonH + (ShowSectionRibbon ? SectionRibbonH : 6.f);
        ScrollTrack->SetPos(
            InnerMarginX * 0.5f - ScrollTrackW * 0.5f,
            ScrollTop);
        ScrollTrack->SetSize(
            ScrollTrackW,
            PanelHeight - (ScrollTop - OuterTop) - 52.f);
    }
    if (auto ScrollThumb = Widget.mScrollThumb.lock())
    {
        const float ScrollTop =
            OuterTop + TitleRibbonH + (ShowSectionRibbon ? SectionRibbonH : 6.f);
        ScrollThumb->SetPos(
            InnerMarginX * 0.5f - ScrollTrackW * 0.5f + 0.5f,
            ScrollTop + 14.f);
        ScrollThumb->SetSize(ScrollTrackW - 1.f, ScrollThumbH);
    }

    // 예산 컨트롤
    const float BudgetBaseY = OuterTop + RibbonOffsetY + TitleRibbonH + 36.f;
    if (auto BudgetText = Widget.mBudgetText.lock())
    {
        if (ShowAnyOverview)
        {
            BudgetText->SetPos(0.f, 0.f);
            BudgetText->SetSize(0.f, 0.f);
        }
        else
        {
            BudgetText->SetPos(RibbonOffsetX, BudgetBaseY);
            BudgetText->SetSize(PanelWidth - RibbonOffsetX * 2.f, 22.f);
        }
    }

    const float BudgetButtonH   = ShowAnyOverview ? 22.f :
        UIConfig::BuildingBudgetButtonHeight;
    const float BudgetButtonTop = ShowCustomOverview ?
        (BudgetBaseY + 20.f) :
        (ShowWorkOverview ? (BudgetBaseY + 78.f) :
        (BudgetBaseY + 26.f));
    const float WorkModeTop = BudgetBaseY - 2.f;
    const float WorkModeBoxTop = WorkModeTop + 22.f;
    const float BudgetMargin    = RibbonOffsetX;
    const float BudgetGap       = ShowAnyOverview ? 6.f : 8.f;
    const float BudgetButtonW   = ShowAnyOverview ?
        36.f :
        (PanelWidth - BudgetMargin * 2.f - BudgetGap * 4.f) / 5.f;

    if (auto Text = Widget.mOverviewWorkModeLabel.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(BudgetMargin, WorkModeTop);
            Text->SetSize(PanelWidth * 0.5f, 22.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Background = Widget.mOverviewWorkModeBackground.lock())
    {
        if (ShowWorkOverview)
        {
            Background->SetPos(BudgetMargin, WorkModeBoxTop);
            Background->SetSize(PanelWidth - BudgetMargin * 2.f, 34.f);
        }
        else
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewWorkModeText.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(BudgetMargin + 12.f, WorkModeBoxTop + 2.f);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f - 24.f, 30.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewBudgetLabel.lock())
    {
        if (ShowAnyOverview)
        {
            Text->SetPos(BudgetMargin, BudgetBaseY - 2.f);
            Text->SetSize(PanelWidth * 0.5f, 22.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewBudgetValue.lock())
    {
        if (ShowAnyOverview)
        {
            Text->SetPos(PanelWidth - BudgetMargin - 120.f, BudgetBaseY - 2.f);
            Text->SetSize(120.f, 22.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        auto Button = Widget.mBudgetButtons[static_cast<size_t>(Index)].lock();
        if (!Button)
            continue;

        Button->SetPos(
            BudgetMargin + static_cast<float>(Index) * (BudgetButtonW + BudgetGap),
            BudgetButtonTop);
        Button->SetSize(BudgetButtonW, BudgetButtonH);
    }

    const float OccupancyTop = BudgetButtonTop + BudgetButtonH + 18.f;
    if (auto Text = Widget.mOverviewOccupancyLabel.lock())
    {
        if (ShowAnyOverview)
        {
            Text->SetPos(BudgetMargin, OccupancyTop);
            Text->SetSize(PanelWidth * 0.5f, 22.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewOccupancyValue.lock())
    {
        if (ShowAnyOverview)
        {
            Text->SetPos(PanelWidth - BudgetMargin - 120.f, OccupancyTop);
            Text->SetSize(120.f, 22.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (ShowCitizenThoughts)
    {
        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mOverviewBudgetLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewBudgetValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Background = Widget.mOverviewWorkModeBackground.lock())
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock())
            {
                Value->SetPos(0.f, 0.f);
                Value->SetSize(0.f, 0.f);
            }
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(0.f, 0.f);
            Divider->SetSize(0.f, 0.f);
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSectionCount;
            ++Index)
        {
            if (auto Background =
                Widget.mCitizenPoliticsSectionBackgrounds[
                    static_cast<size_t>(Index)].lock())
            {
                Background->SetPos(0.f, 0.f);
                Background->SetSize(0.f, 0.f);
            }

            if (auto Title =
                Widget.mCitizenPoliticsSectionTitles[
                    static_cast<size_t>(Index)].lock())
            {
                Title->SetPos(0.f, 0.f);
                Title->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount;
            ++Index)
        {
            if (auto Label =
                Widget.mCitizenPoliticsSatisfactionLabels[
                    static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Rail =
                Widget.mCitizenPoliticsSatisfactionRails[
                    static_cast<size_t>(Index)].lock())
            {
                Rail->SetPos(0.f, 0.f);
                Rail->SetSize(0.f, 0.f);
            }

            if (auto Fill =
                Widget.mCitizenPoliticsSatisfactionFills[
                    static_cast<size_t>(Index)].lock())
            {
                Fill->SetPos(0.f, 0.f);
                Fill->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsOpinionCount;
            ++Index)
        {
            if (auto Text =
                Widget.mCitizenPoliticsOpinionTexts[
                    static_cast<size_t>(Index)].lock())
            {
                Text->SetPos(0.f, 0.f);
                Text->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSupportIconCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mCitizenPoliticsSupportIcons[
                    static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        if (auto Rail = Widget.mCitizenPoliticsSupportRail.lock())
        {
            Rail->SetPos(0.f, 0.f);
            Rail->SetSize(0.f, 0.f);
        }

        if (auto Thumb = Widget.mCitizenPoliticsSupportThumb.lock())
        {
            Thumb->SetPos(0.f, 0.f);
            Thumb->SetSize(0.f, 0.f);
        }

        const float SectionTop =
            OuterTop + RibbonOffsetY + TitleRibbonH + 42.f;
        const float SectionWidth = PanelWidth - BudgetMargin * 2.f;
        const float SectionHeight = 30.f;

        if (auto Background = Widget.mCitizenThoughtTitleBackground.lock())
        {
            Background->SetPos(BudgetMargin, SectionTop);
            Background->SetSize(SectionWidth, SectionHeight);
        }

        if (auto Text = Widget.mCitizenThoughtTitleText.lock())
        {
            Text->SetPos(BudgetMargin, SectionTop);
            Text->SetSize(SectionWidth, SectionHeight);
        }

        const float ThoughtTop = SectionTop + SectionHeight + 14.f;
        const float ThoughtWidth = SectionWidth - 6.f;
        const float ThoughtHeight = 50.f;
        const float DividerWidth = 172.f;
        const float DividerHeight = 14.f;
        const float ThoughtStep = 82.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenThoughtCount;
            ++Index)
        {
            if (auto Text =
                Widget.mCitizenThoughtTexts[static_cast<size_t>(Index)].lock())
            {
                Text->SetPos(
                    BudgetMargin + 2.f,
                    ThoughtTop + ThoughtStep * static_cast<float>(Index));
                Text->SetSize(ThoughtWidth, ThoughtHeight);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenThoughtDividerCount;
            ++Index)
        {
            if (auto Divider =
                Widget.mCitizenThoughtDividers[static_cast<size_t>(Index)].lock())
            {
                Divider->SetPos(
                    PanelWidth * 0.5f - DividerWidth * 0.5f,
                    ThoughtTop + 38.f + ThoughtStep * static_cast<float>(Index));
                Divider->SetSize(DividerWidth, DividerHeight);
            }
        }
    }
    else if (ShowCitizenPolitics)
    {
        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mOverviewBudgetLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewBudgetValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Background = Widget.mOverviewWorkModeBackground.lock())
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock())
            {
                Value->SetPos(0.f, 0.f);
                Value->SetSize(0.f, 0.f);
            }
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(0.f, 0.f);
            Divider->SetSize(0.f, 0.f);
        }

        const float SectionWidth = PanelWidth - BudgetMargin * 2.f;
        const float SectionHeight = 30.f;
        const float SatisfactionTitleTop =
            OuterTop + RibbonOffsetY + TitleRibbonH + 42.f;
        const float SatisfactionRowsTop = SatisfactionTitleTop + SectionHeight + 10.f;
        const float SatisfactionRowH = 28.f;
        const float SatisfactionLabelW = 82.f;
        const float SatisfactionRailLeft = BudgetMargin + SatisfactionLabelW;
        const float SatisfactionRailWidth =
            PanelWidth - BudgetMargin - SatisfactionRailLeft - 4.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSectionCount;
            ++Index)
        {
            float SectionTop = 0.f;

            if (Index == 0)
                SectionTop = SatisfactionTitleTop;
            else if (Index == 1)
                SectionTop = SatisfactionRowsTop +
                    SatisfactionRowH *
                    static_cast<float>(
                        CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount) +
                    8.f;
            else
                SectionTop = SatisfactionRowsTop +
                    SatisfactionRowH *
                    static_cast<float>(
                        CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount) +
                    8.f +
                    SectionHeight +
                    8.f +
                    26.f *
                    static_cast<float>(
                        CCitizenInfoWidget::GCitizenPoliticsOpinionCount) +
                    14.f;

            if (auto Background =
                Widget.mCitizenPoliticsSectionBackgrounds[
                    static_cast<size_t>(Index)].lock())
            {
                Background->SetPos(BudgetMargin, SectionTop);
                Background->SetSize(SectionWidth, SectionHeight);
            }

            if (auto Title =
                Widget.mCitizenPoliticsSectionTitles[
                    static_cast<size_t>(Index)].lock())
            {
                Title->SetPos(BudgetMargin, SectionTop);
                Title->SetSize(SectionWidth, SectionHeight);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount;
            ++Index)
        {
            const float RowTop =
                SatisfactionRowsTop +
                SatisfactionRowH * static_cast<float>(Index);
            const float FillRatio =
                (std::max)(
                    0.f,
                    (std::min)(
                        1.f,
                        Widget.mCitizenPoliticsSatisfactionFillRatios[
                            static_cast<size_t>(Index)]));
            const float FillWidth =
                (std::max)(8.f, SatisfactionRailWidth * FillRatio);

            if (auto Label =
                Widget.mCitizenPoliticsSatisfactionLabels[
                    static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(BudgetMargin, RowTop);
                Label->SetSize(SatisfactionLabelW - 8.f, 22.f);
            }

            if (auto Rail =
                Widget.mCitizenPoliticsSatisfactionRails[
                    static_cast<size_t>(Index)].lock())
            {
                Rail->SetPos(SatisfactionRailLeft, RowTop + 5.f);
                Rail->SetSize(SatisfactionRailWidth, 13.f);
            }

            if (auto Fill =
                Widget.mCitizenPoliticsSatisfactionFills[
                    static_cast<size_t>(Index)].lock())
            {
                Fill->SetPos(SatisfactionRailLeft + 1.f, RowTop + 6.f);
                Fill->SetSize(
                    (std::min)(SatisfactionRailWidth - 2.f, FillWidth),
                    11.f);
            }
        }

        const float OpinionTitleTop =
            SatisfactionRowsTop +
            SatisfactionRowH *
                static_cast<float>(
                    CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount) +
            8.f;
        const float OpinionRowsTop = OpinionTitleTop + SectionHeight + 8.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsOpinionCount;
            ++Index)
        {
            if (auto Text =
                Widget.mCitizenPoliticsOpinionTexts[
                    static_cast<size_t>(Index)].lock())
            {
                Text->SetPos(
                    BudgetMargin + 4.f,
                    OpinionRowsTop + 26.f * static_cast<float>(Index));
                Text->SetSize(SectionWidth - 8.f, 22.f);
            }
        }

        const float SupportTitleTop =
            OpinionRowsTop +
            26.f *
                static_cast<float>(
                    CCitizenInfoWidget::GCitizenPoliticsOpinionCount) +
            14.f;
        const float SupportIconsTop = SupportTitleTop + SectionHeight + 8.f;
        const float SupportIconSpacing = SectionWidth / 3.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenPoliticsSupportIconCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mCitizenPoliticsSupportIcons[
                    static_cast<size_t>(Index)].lock())
            {
                const float IconLeft =
                    BudgetMargin +
                    SupportIconSpacing * static_cast<float>(Index) +
                    SupportIconSpacing * 0.5f -
                    10.f;
                Icon->SetPos(IconLeft, SupportIconsTop);
                Icon->SetSize(20.f, 20.f);
            }
        }

        const float SupportRailTop = SupportIconsTop + 26.f;
        const float SupportRailLeft = BudgetMargin + 10.f;
        const float SupportRailWidth = SectionWidth - 20.f;

        if (auto Rail = Widget.mCitizenPoliticsSupportRail.lock())
        {
            Rail->SetPos(SupportRailLeft, SupportRailTop);
            Rail->SetSize(SupportRailWidth, 10.f);
        }

        if (auto Thumb = Widget.mCitizenPoliticsSupportThumb.lock())
        {
            const float SupportRatio =
                (std::max)(0.f, (std::min)(1.f, Widget.mCitizenPoliticsSupportRatio));
            Thumb->SetPos(
                SupportRailLeft + SupportRatio * (SupportRailWidth - 10.f),
                SupportRailTop - 4.f);
            Thumb->SetSize(10.f, 18.f);
        }
    }
    else if (ShowCitizenProfile)
    {
        const std::array<FVector2, 11> PortraitPositions =
        {
            FVector2(BudgetMargin + 24.f, OccupancyTop + 10.f),
            FVector2(BudgetMargin + 94.f, OccupancyTop + 10.f),
            FVector2(BudgetMargin + 164.f, OccupancyTop + 10.f),
            FVector2(BudgetMargin + 234.f, OccupancyTop + 10.f),
            FVector2(BudgetMargin + 57.f, OccupancyTop + 62.f),
            FVector2(BudgetMargin + 235.f, OccupancyTop + 62.f),
            FVector2(BudgetMargin + 6.f, OccupancyTop + 112.f),
            FVector2(BudgetMargin + 76.f, OccupancyTop + 112.f),
            FVector2(BudgetMargin + 146.f, OccupancyTop + 112.f),
            FVector2(BudgetMargin + 216.f, OccupancyTop + 112.f),
            FVector2(BudgetMargin + 286.f, OccupancyTop + 112.f)
        };
        const float PortraitSize = 34.f;
        const float DetailTop = OccupancyTop + 170.f;
        const float DetailRowH = 28.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock();

            if (!Icon)
                continue;

            if (Index < static_cast<int>(PortraitPositions.size()))
            {
                Icon->SetPos(
                    PortraitPositions[static_cast<size_t>(Index)].x,
                    PortraitPositions[static_cast<size_t>(Index)].y);
                Icon->SetSize(PortraitSize, PortraitSize);
            }
            else
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock();
            auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock();
            const float RowTop =
                DetailTop + static_cast<float>(Index) * DetailRowH;

            if (Label)
            {
                Label->SetPos(BudgetMargin, RowTop);
                Label->SetSize(110.f, 24.f);
            }

            if (Value)
            {
                Value->SetPos(PanelWidth - BudgetMargin - 164.f, RowTop);
                Value->SetSize(164.f, 24.f);
            }
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(
                PanelWidth * 0.5f - 86.f,
                DetailTop +
                    DetailRowH * 8.f +
                    6.f);
            Divider->SetSize(172.f, 14.f);
        }
    }
    else if (ShowCustomOverview)
    {
        const float ResidentStartX = BudgetMargin + 6.f;
        const float ResidentStartY = OccupancyTop + 26.f;
        const float ResidentIconSize = 22.f;
        const float ResidentGapX = 34.f;
        const float ResidentGapY = 24.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock();

            if (!Icon)
                continue;

            const int Column = Index % 4;
            const int Row = Index / 4;
            Icon->SetPos(
                ResidentStartX + static_cast<float>(Column) * ResidentGapX,
                ResidentStartY + static_cast<float>(Row) * ResidentGapY);
            Icon->SetSize(ResidentIconSize, ResidentIconSize);
        }

        const float MetricsTop = ResidentStartY + ResidentGapY * 4.f + 14.f;
        const float MetricRowH = 24.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock();
            auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock();
            const float RowTop =
                MetricsTop + static_cast<float>(Index) * MetricRowH;

            if (Label)
            {
                Label->SetPos(BudgetMargin, RowTop);
                Label->SetSize(140.f, 22.f);
            }

            if (Value)
            {
                Value->SetPos(PanelWidth - BudgetMargin - 160.f, RowTop);
                Value->SetSize(160.f, 22.f);
            }
        }
    }
    else if (ShowWorkOverview)
    {
        const float ResidentStartX = BudgetMargin + 6.f;
        const float ResidentStartY = OccupancyTop + 26.f;
        const float ResidentIconSize = 24.f;
        const float ResidentGapX = 26.f;
        const float MetricsTop = ResidentStartY + ResidentIconSize + 18.f;
        const float MetricRowH = 23.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock();

            if (!Icon)
                continue;

            if (Index < 6)
            {
                Icon->SetPos(
                    ResidentStartX + static_cast<float>(Index) * ResidentGapX,
                    ResidentStartY);
                Icon->SetSize(ResidentIconSize, ResidentIconSize);
            }
            else
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock();
            auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock();
            const bool SectionHeader =
                Value &&
                !Value->GetEnable();
            const float RowTop =
                MetricsTop + static_cast<float>(Index) * MetricRowH;

            if (Label)
            {
                Label->SetPos(BudgetMargin, RowTop);
                Label->SetSize(
                    SectionHeader ?
                        (PanelWidth - BudgetMargin * 2.f) :
                        160.f,
                    22.f);
            }

            if (Value)
            {
                if (SectionHeader)
                {
                    Value->SetPos(0.f, 0.f);
                    Value->SetSize(0.f, 0.f);
                }
                else
                {
                    Value->SetPos(PanelWidth - BudgetMargin - 140.f, RowTop);
                    Value->SetSize(140.f, 22.f);
                }
            }
        }

        const bool ShowVisitorIcons =
            !Widget.mOverviewVisitorIcons[0].expired() &&
            Widget.mOverviewVisitorIcons[0].lock()->GetEnable();

        if (ShowVisitorIcons)
        {
            const float VisitorStartX = BudgetMargin + 2.f;
            const float VisitorStartY =
                MetricsTop +
                static_cast<float>(CCitizenInfoWidget::GOverviewMetricRowCount) *
                    MetricRowH +
                10.f;
            const float VisitorIconSize = 24.f;
            const float VisitorGapX = 22.f;

            for (int Index = 0;
                Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
                ++Index)
            {
                auto Icon =
                    Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock();

                if (!Icon)
                    continue;

                Icon->SetPos(
                    VisitorStartX +
                        static_cast<float>(Index) * VisitorGapX,
                    VisitorStartY);
                Icon->SetSize(VisitorIconSize, VisitorIconSize);
            }
        }
        else
        {
            for (int Index = 0;
                Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
                ++Index)
            {
                if (auto Icon =
                    Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock())
                {
                    Icon->SetPos(0.f, 0.f);
                    Icon->SetSize(0.f, 0.f);
                }
            }
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(0.f, 0.f);
            Divider->SetSize(0.f, 0.f);
        }
    }
    else if (ShowCompactRows)
    {
        const float MetricsTop =
            SectionRibbonY + SectionRibbonH + 12.f;
        const float MetricRowH = 30.f;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewVisitorSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewVisitorIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mOverviewBudgetLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewBudgetValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock();
            auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock();
            const float RowTop =
                MetricsTop + static_cast<float>(Index) * MetricRowH;

            if (Label)
            {
                Label->SetPos(BudgetMargin, RowTop);
                Label->SetSize(180.f, 24.f);
            }

            if (Value)
            {
                Value->SetPos(PanelWidth - BudgetMargin - 120.f, RowTop);
                Value->SetSize(120.f, 24.f);
            }
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(
                PanelWidth * 0.5f - 86.f,
                MetricsTop + MetricRowH + 8.f);
            Divider->SetSize(172.f, 14.f);
        }
    }
    else if (ShowInformationParagraphs)
    {
        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock())
            {
                Value->SetPos(0.f, 0.f);
                Value->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mOverviewBudgetLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewBudgetValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        const float InfoTop = OuterTop + RibbonOffsetY + TitleRibbonH + 18.f;
        const float DividerTop = InfoTop + 98.f;

        if (auto Text = Widget.mInformationAccentText.lock())
        {
            Text->SetPos(BudgetMargin, InfoTop + 2.f);
            Text->SetSize(32.f, 30.f);
        }

        if (auto Text = Widget.mInformationTopText.lock())
        {
            Text->SetPos(BudgetMargin + 30.f, InfoTop);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f - 30.f, 100.f);
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(PanelWidth * 0.5f - 86.f, DividerTop);
            Divider->SetSize(172.f, 14.f);
        }

        if (auto Text = Widget.mInformationBottomText.lock())
        {
            Text->SetPos(BudgetMargin, DividerTop + 28.f);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f, 110.f);
        }
    }
    else
    {
        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
            ++Index)
        {
            if (auto Icon =
                Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Value =
                Widget.mOverviewMetricValues[static_cast<size_t>(Index)].lock())
            {
                Value->SetPos(0.f, 0.f);
                Value->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mOverviewBudgetLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewBudgetValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewOccupancyValue.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeLabel.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Background = Widget.mOverviewWorkModeBackground.lock())
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mOverviewWorkModeText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(0.f, 0.f);
            Divider->SetSize(0.f, 0.f);
        }
    }

    if (!ShowInformationParagraphs)
    {
        if (auto Text = Widget.mInformationAccentText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mInformationTopText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mInformationBottomText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (ShowUpgradeCard)
    {
        const float CardLeft = BudgetMargin;
        const float CardTop = SectionRibbonY + SectionRibbonH + 12.f;
        const float CardWidth = PanelWidth - BudgetMargin * 2.f;
        const float CardHeight = 46.f;

        if (auto Card = Widget.mUpgradeCardBackground.lock())
        {
            Card->SetPos(CardLeft, CardTop);
            Card->SetSize(CardWidth, CardHeight);
        }

        if (auto Icon = Widget.mUpgradeCardIcon.lock())
        {
            Icon->SetPos(CardLeft + 8.f, CardTop + 7.f);
            Icon->SetSize(30.f, 30.f);
        }

        if (auto Text = Widget.mUpgradeCardTitle.lock())
        {
            Text->SetPos(CardLeft + 44.f, CardTop + 2.f);
            Text->SetSize(CardWidth - 52.f, CardHeight - 4.f);
        }

        if (auto Text = Widget.mUpgradeDescriptionText.lock())
        {
            Text->SetPos(BudgetMargin, CardTop + CardHeight + 10.f);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f, 54.f);
        }
    }
    else
    {
        if (auto Card = Widget.mUpgradeCardBackground.lock())
        {
            Card->SetPos(0.f, 0.f);
            Card->SetSize(0.f, 0.f);
        }

        if (auto Icon = Widget.mUpgradeCardIcon.lock())
        {
            Icon->SetPos(0.f, 0.f);
            Icon->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mUpgradeCardTitle.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }

        if (auto Text = Widget.mUpgradeDescriptionText.lock())
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    // 액션 버튼 (철거/이동/복제)
    const bool ShowActions =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        Widget.mSelectedBuildingTab ==
            CCitizenInfoWidget::EBuildingInfoTab::Overview;

    const float ActionBtnH      = UIConfig::BuildingActionButtonHeight;
    const float ActionBtnW      = UIConfig::BuildingActionButtonWidth;
    const float ActionBtmMargin = UIConfig::BuildingActionButtonBottomMargin;
    const float ActionTop       = OuterTop + PanelHeight - ActionBtmMargin;

    if (auto DemolishButton = Widget.mDemolishButton.lock())
    {
        DemolishButton->SetPos(BudgetMargin, ActionTop);
        DemolishButton->SetSize(
            ShowAnyOverview ? (PanelWidth - BudgetMargin * 2.f - 88.f) :
                ActionBtnW,
            ActionBtnH);
    }
    if (auto MoveButton = Widget.mMoveButton.lock())
    {
        if (ShowAnyOverview || ShowInformationParagraphs)
        {
            MoveButton->SetPos(
                PanelWidth - BudgetMargin - 82.f,
                ActionTop + 2.f);
            MoveButton->SetSize(34.f, 34.f);
        }
        else
        {
            MoveButton->SetPos(PanelWidth - BudgetMargin - ActionBtnW * 1.1f - 6.f, ActionTop);
            MoveButton->SetSize(ActionBtnW * 0.5f, ActionBtnH);
        }
    }
    if (auto CloneButton = Widget.mCloneButton.lock())
    {
        if (ShowAnyOverview || ShowInformationParagraphs)
        {
            CloneButton->SetPos(
                PanelWidth - BudgetMargin - 40.f,
                ActionTop + 2.f);
            CloneButton->SetSize(34.f, 34.f);
        }
        else
        {
            CloneButton->SetPos(PanelWidth - BudgetMargin - ActionBtnW * 0.55f, ActionTop);
            CloneButton->SetSize(ActionBtnW * 0.5f, ActionBtnH);
        }
    }

    if (auto Button = Widget.mOverviewCommandButton.lock())
    {
        if (ShowOverviewCommandButton)
        {
            Button->SetPos(BudgetMargin, ActionTop - ActionBtnH - 10.f);
            Button->SetSize(PanelWidth - BudgetMargin * 2.f, ActionBtnH);
        }
        else
        {
            Button->SetPos(0.f, 0.f);
            Button->SetSize(0.f, 0.f);
        }
    }

    const float CitizenActionBtnH = 38.f;
    const float CitizenActionGap = 4.f;
    const float CitizenActionTop = ActionTop - 238.f;

    for (int Index = 0; Index < CCitizenInfoWidget::GCitizenActionButtonCount;
        ++Index)
    {
        const float ButtonTop =
            CitizenActionTop +
            static_cast<float>(Index) * (CitizenActionBtnH + CitizenActionGap);

        if (auto Button =
            Widget.mCitizenActionButtons[static_cast<size_t>(Index)].lock())
        {
            if (ShowCitizenProfile)
            {
                Button->SetPos(BudgetMargin, ButtonTop);
                Button->SetSize(PanelWidth - BudgetMargin * 2.f, CitizenActionBtnH);
            }
            else
            {
                Button->SetPos(0.f, 0.f);
                Button->SetSize(0.f, 0.f);
            }
        }

        if (auto Icon =
            Widget.mCitizenActionButtonIcons[static_cast<size_t>(Index)].lock())
        {
            if (ShowCitizenProfile)
            {
                Icon->SetPos(BudgetMargin + 8.f, ButtonTop + 8.f);
                Icon->SetSize(22.f, 22.f);
            }
            else
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }
        }
    }

    if (auto Text = Widget.mCitizenFooterText.lock())
    {
        if (ShowCitizenProfile)
        {
            Text->SetPos(BudgetMargin, OuterTop + PanelHeight - 28.f);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f, 18.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    // 본문 텍스트 영역
    const float BodyTop =
        ShowSectionRibbon ? (SectionRibbonY + SectionRibbonH + 8.f) :
        (ShowCitizenProfile || ShowCitizenPolitics || ShowCitizenThoughts ?
            (OuterTop + PanelHeight) :
            (ShowAnyOverview ? (OuterTop + PanelHeight) :
            (ShowActions ? (BudgetButtonTop + BudgetButtonH + 14.f) :
                           (BudgetBaseY - 4.f))));
    const float BodyBottom =
        ShowActions ? (ActionTop - 12.f) :
        (OuterTop + PanelHeight - 22.f);

    if (auto BodyText = Widget.mBodyText.lock())
    {
        if (ShowCitizenProfile || ShowCitizenPolitics ||
            ShowCitizenThoughts || ShowAnyOverview || ShowInformationParagraphs)
        {
            BodyText->SetPos(0.f, 0.f);
            BodyText->SetSize(0.f, 0.f);
        }
        else
        {
            BodyText->SetPos(BudgetMargin, BodyTop);
            BodyText->SetSize(
                PanelWidth - BudgetMargin * 2.f,
                (std::max)(80.f, BodyBottom - BodyTop));
        }
    }
}
