#include "AlmanacRenderer.h"
#include "AlmanacRendererInternal.h"
#include "World/World.h"
#include <string>

namespace
{
CAlmanacWidget::FCardWidgets CreateCard(
    const std::shared_ptr<CWidgetContainer>& Page,
    const std::string& Prefix,
    const TCHAR* IconPath)
{
        CAlmanacWidget::FCardWidgets Card;

        if (!Page)
            return Card;

        Card.Background = Page->CreateWidget<CImage>(
            Prefix + "_Background", 1);

        if (auto Background = Card.Background.lock())
        {
            ConfigureFrameImage(
                Background,
                Prefix + "_Texture",
                GCardTexture);
            ApplySelectableBackground(Background, false, true);
        }

        Card.Icon = Page->CreateWidget<CImage>(Prefix + "_Icon", 2);

        if (auto Icon = Card.Icon.lock())
        {
            Icon->SetTexture(Prefix + "_IconTexture", IconPath);
            Icon->SetTint(1.f, 1.f, 1.f, 1.f);
        }

        Card.Title = Page->CreateWidget<CTextBlock>(Prefix + "_Title", 2);
        Card.Value = Page->CreateWidget<CTextBlock>(Prefix + "_Value", 2);
        Card.Detail = Page->CreateWidget<CTextBlock>(Prefix + "_Detail", 2);

        ConfigureCardTitleText(Card.Title.lock());
        ConfigureCardValueText(Card.Value.lock());
        ConfigureCardDetailText(Card.Detail.lock());

        return Card;
}

CAlmanacWidget::FMetricRowWidgets CreateMetricRow(
    const std::shared_ptr<CWidgetContainer>& Page,
    const std::string& Prefix)
{
        CAlmanacWidget::FMetricRowWidgets Row;

        if (!Page)
            return Row;

        Row.Background = Page->CreateWidget<CImage>(
            Prefix + "_Background", 1);
        Row.Label = Page->CreateWidget<CTextBlock>(Prefix + "_Label", 2);
        Row.Bar = Page->CreateWidget<CProgressBar>(Prefix + "_Bar", 2);
        Row.Value = Page->CreateWidget<CTextBlock>(Prefix + "_Value", 2);

        ConfigureRowBackground(Row.Background.lock(), Prefix + "_Texture");
        ConfigureBodyLabelText(Row.Label.lock());
        ConfigureMetricBar(Row.Bar.lock());
        ConfigureBodyValueText(Row.Value.lock());

        return Row;
}

CAlmanacWidget::FSatisfactionRowWidgets CreateSatisfactionRow(
    CAlmanacWidget& Widget,
    const std::weak_ptr<CWorld>& World,
    const std::shared_ptr<CWidgetContainer>& Page,
    const std::string& Prefix,
    int Index)
{
        CAlmanacWidget::FSatisfactionRowWidgets Row;

        if (!Page)
            return Row;

        Row.Button = Page->CreateWidget<CButton>(Prefix + "_Button", 1);
        auto Button = Row.Button.lock();

        if (!Button)
            return Row;

        ConfigureSatisfactionRowButtonStyle(Button, false);
        Button->SetEventCallback(
            EButtonEventState::Hovered,
            [&Widget, Index]()
            {
                Widget.SelectSatisfactionRow(Index);
            });
        Button->SetEventCallback(
            EButtonEventState::Click,
            [&Widget, Index]()
            {
                Widget.SelectSatisfactionRow(Index);
            });

        auto Content = CWidget::CreateStaticWidget<CWidgetContainer>(
            Prefix + "_Content", World);
        auto Icon = CWidget::CreateStaticWidget<CImage>(
            Prefix + "_Icon", World);
        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Label", World);
        auto Bar = CWidget::CreateStaticWidget<CProgressBar>(
            Prefix + "_Bar", World);
        auto Value = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Value", World);

        if (Content && Icon && Label && Bar && Value)
        {
            Icon->SetTexture(Prefix + "_IconTexture", GSatisfactionIcons[Index]);
            Icon->SetTint(1.f, 1.f, 1.f, 1.f);

            ConfigureBodyLabelText(Label);
            ConfigureSatisfactionRowBar(Bar);
            ConfigureBodyValueText(Value);

            Content->AddWidget(Icon);
            Content->AddWidget(Label);
            Content->AddWidget(Bar);
            Content->AddWidget(Value);
            Button->SetChild(Content);

            Row.Icon = Icon;
            Row.Label = Label;
            Row.Bar = Bar;
            Row.Value = Value;
        }

        return Row;
}

CAlmanacWidget::FDetailRowWidgets CreateDetailRow(
    const std::shared_ptr<CWidgetContainer>& Page,
    const std::string& Prefix)
{
        CAlmanacWidget::FDetailRowWidgets Row;

        if (!Page)
            return Row;

        Row.Background = Page->CreateWidget<CImage>(
            Prefix + "_Background", 1);
        Row.Label = Page->CreateWidget<CTextBlock>(Prefix + "_Label", 2);
        Row.Value = Page->CreateWidget<CTextBlock>(Prefix + "_Value", 2);

        ConfigureRowBackground(Row.Background.lock(), Prefix + "_Texture");
        ConfigureBodyLabelText(Row.Label.lock());
        ConfigureBodyValueText(Row.Value.lock());

        return Row;
}

CAlmanacWidget::FDetailRowWidgets CreateSelectableDetailRow(
    CAlmanacWidget& Widget,
    const std::weak_ptr<CWorld>& World,
    const std::shared_ptr<CWidgetContainer>& Page,
    const std::string& Prefix,
    int Index)
{
        CAlmanacWidget::FDetailRowWidgets Row;

        if (!Page)
            return Row;

        Row.Button = Page->CreateWidget<CButton>(Prefix + "_Button", 1);
        auto Button = Row.Button.lock();

        if (!Button)
            return Row;

        ConfigureSatisfactionRowButtonStyle(Button, false);
        Button->SetEventCallback(
            EButtonEventState::Hovered,
            [&Widget, Index]()
            {
                Widget.SelectPopulationRow(Index);
            });
        Button->SetEventCallback(
            EButtonEventState::Click,
            [&Widget, Index]()
            {
                Widget.SelectPopulationRow(Index);
            });

        auto Content = CWidget::CreateStaticWidget<CWidgetContainer>(
            Prefix + "_Content", World);
        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Label", World);
        auto Value = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Value", World);

        if (Content && Label && Value)
        {
            ConfigureBodyLabelText(Label);
            ConfigureBodyValueText(Value);

            Content->AddWidget(Label);
            Content->AddWidget(Value);
            Button->SetChild(Content);

            Row.Label = Label;
            Row.Value = Value;
        }

        return Row;
}

CAlmanacWidget::FDetailRowWidgets CreateSelectableEconomyDetailRow(
    CAlmanacWidget& Widget,
    const std::weak_ptr<CWorld>& World,
    const std::shared_ptr<CWidgetContainer>& Page,
    const std::string& Prefix,
    int Index)
{
        CAlmanacWidget::FDetailRowWidgets Row;

        if (!Page)
            return Row;

        Row.Button = Page->CreateWidget<CButton>(Prefix + "_Button", 1);
        auto Button = Row.Button.lock();

        if (!Button)
            return Row;

        ConfigureSatisfactionRowButtonStyle(Button, false);
        Button->SetEventCallback(
            EButtonEventState::Hovered,
            [&Widget, Index]()
            {
                Widget.SelectEconomyRow(Index);
            });
        Button->SetEventCallback(
            EButtonEventState::Click,
            [&Widget, Index]()
            {
                Widget.SelectEconomyRow(Index);
            });

        auto Content = CWidget::CreateStaticWidget<CWidgetContainer>(
            Prefix + "_Content", World);
        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Label", World);
        auto Value = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Value", World);

        if (Content && Label && Value)
        {
            ConfigureBodyLabelText(Label);
            ConfigureBodyValueText(Value);

            Content->AddWidget(Label);
            Content->AddWidget(Value);
            Button->SetChild(Content);

            Row.Label = Label;
            Row.Value = Value;
        }

        return Row;
}

CAlmanacWidget::FDetailRowWidgets CreateSelectableResourceDetailRow(
    CAlmanacWidget& Widget,
    const std::weak_ptr<CWorld>& World,
    const std::shared_ptr<CWidgetContainer>& Page,
    const std::string& Prefix,
    int Index)
{
        CAlmanacWidget::FDetailRowWidgets Row;

        if (!Page)
            return Row;

        Row.Button = Page->CreateWidget<CButton>(Prefix + "_Button", 1);
        auto Button = Row.Button.lock();

        if (!Button)
            return Row;

        ConfigureSatisfactionRowButtonStyle(Button, false);
        Button->SetEventCallback(
            EButtonEventState::Hovered,
            [&Widget, Index]()
            {
                Widget.SelectResourceRow(Index);
            });
        Button->SetEventCallback(
            EButtonEventState::Click,
            [&Widget, Index]()
            {
                Widget.SelectResourceRow(Index);
            });

        auto Content = CWidget::CreateStaticWidget<CWidgetContainer>(
            Prefix + "_Content", World);
        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Label", World);
        auto Value = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Value", World);

        if (Content && Label && Value)
        {
            ConfigureBodyLabelText(Label);
            ConfigureBodyValueText(Value);

            Content->AddWidget(Label);
            Content->AddWidget(Value);
            Button->SetChild(Content);

            Row.Label = Label;
            Row.Value = Value;
        }

        return Row;
}

CAlmanacWidget::FPoliticsFactionTileWidgets CreatePoliticsFactionTile(
    CAlmanacWidget& Widget,
    const std::weak_ptr<CWorld>& World,
    const std::shared_ptr<CWidgetContainer>& Page,
    const std::string& Prefix,
    int Index)
{
        CAlmanacWidget::FPoliticsFactionTileWidgets Tile;

        if (!Page)
            return Tile;

        Tile.Button = Page->CreateWidget<CButton>(Prefix + "_Button", 1);
        auto Button = Tile.Button.lock();

        if (!Button)
            return Tile;

        ConfigurePoliticsFactionButtonStyle(Button, false);
        Button->SetEventCallback(
            EButtonEventState::Hovered,
            [&Widget, Index]()
            {
                Widget.SelectPoliticsFaction(Index);
            });
        Button->SetEventCallback(
            EButtonEventState::Click,
            [&Widget, Index]()
            {
                Widget.SelectPoliticsFaction(Index);
            });

        auto Content = CWidget::CreateStaticWidget<CWidgetContainer>(
            Prefix + "_Content", World);
        auto Icon = CWidget::CreateStaticWidget<CImage>(
            Prefix + "_Icon", World);
        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Label", World);
        auto CountIcon = CWidget::CreateStaticWidget<CImage>(
            Prefix + "_CountIcon", World);
        auto CountValue = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_CountValue", World);
        auto FavorIcon = CWidget::CreateStaticWidget<CImage>(
            Prefix + "_FavorIcon", World);
        auto FavorValue = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_FavorValue", World);

        if (Content && Icon && Label && CountIcon && CountValue && FavorIcon && FavorValue)
        {
            Icon->SetTexture(Prefix + "_IconTexture", GPoliticsFactionIcons[0]);
            Icon->SetTint(0.90f, 0.74f, 0.18f, 0.96f);

            CountIcon->SetTexture(Prefix + "_CountIconTexture", GPopulationIcon);
            FavorIcon->SetTexture(Prefix + "_FavorIconTexture", GApprovalIcon);

            ConfigureBodyLabelText(Label);
            Label->SetFontSize(15.f);
            Label->SetAlignV(ETextAlignV::Top);

            ConfigureBodyLabelText(CountValue);
            CountValue->SetFontSize(14.f);
            CountValue->SetAlignH(ETextAlignH::Left);

            ConfigureBodyLabelText(FavorValue);
            FavorValue->SetFontSize(14.f);
            FavorValue->SetAlignH(ETextAlignH::Left);

            Content->AddWidget(Icon);
            Content->AddWidget(Label);
            Content->AddWidget(CountIcon);
            Content->AddWidget(CountValue);
            Content->AddWidget(FavorIcon);
            Content->AddWidget(FavorValue);
            Button->SetChild(Content);

            Tile.Icon = Icon;
            Tile.Label = Label;
            Tile.CountIcon = CountIcon;
            Tile.CountValue = CountValue;
            Tile.FavorIcon = FavorIcon;
            Tile.FavorValue = FavorValue;
        }

        return Tile;
}

CAlmanacWidget::FSatisfactionRowWidgets CreateForeignPowerRow(
    CAlmanacWidget& Widget,
    const std::weak_ptr<CWorld>& World,
    const std::shared_ptr<CWidgetContainer>& Page,
    const std::string& Prefix,
    int Index)
{
        CAlmanacWidget::FSatisfactionRowWidgets Row;

        if (!Page)
            return Row;

        Row.Button = Page->CreateWidget<CButton>(Prefix + "_Button", 1);
        auto Button = Row.Button.lock();

        if (!Button)
            return Row;

        ConfigureSatisfactionRowButtonStyle(Button, false);
        Button->SetEventCallback(
            EButtonEventState::Hovered,
            [&Widget, Index]()
            {
                Widget.SelectForeignPower(Index);
            });
        Button->SetEventCallback(
            EButtonEventState::Click,
            [&Widget, Index]()
            {
                Widget.SelectForeignPower(Index);
            });

        auto Content = CWidget::CreateStaticWidget<CWidgetContainer>(
            Prefix + "_Content", World);
        auto Icon = CWidget::CreateStaticWidget<CImage>(
            Prefix + "_Icon", World);
        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Label", World);
        auto Bar = CWidget::CreateStaticWidget<CProgressBar>(
            Prefix + "_Bar", World);
        auto Value = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Value", World);

        if (Content && Icon && Label && Bar && Value)
        {
            ConfigureBodyLabelText(Label);
            ConfigureSatisfactionRowBar(Bar);
            ConfigureBodyValueText(Value);

            Content->AddWidget(Icon);
            Content->AddWidget(Label);
            Content->AddWidget(Bar);
            Content->AddWidget(Value);
            Button->SetChild(Content);

            Row.Icon = Icon;
            Row.Label = Label;
            Row.Bar = Bar;
            Row.Value = Value;
        }

        return Row;
}

CAlmanacWidget::FDetailRowWidgets CreateSelectableBuildingDetailRow(
    CAlmanacWidget& Widget,
    const std::weak_ptr<CWorld>& World,
    const std::shared_ptr<CWidgetContainer>& Page,
    const std::string& Prefix,
    int Index)
{
        CAlmanacWidget::FDetailRowWidgets Row;

        if (!Page)
            return Row;

        Row.Button = Page->CreateWidget<CButton>(Prefix + "_Button", 1);
        auto Button = Row.Button.lock();

        if (!Button)
            return Row;

        ConfigureSatisfactionRowButtonStyle(Button, false);
        Button->SetEventCallback(
            EButtonEventState::Hovered,
            [&Widget, Index]()
            {
                Widget.SelectBuildingCategory(Index);
            });
        Button->SetEventCallback(
            EButtonEventState::Click,
            [&Widget, Index]()
            {
                Widget.SelectBuildingCategory(Index);
            });

        auto Content = CWidget::CreateStaticWidget<CWidgetContainer>(
            Prefix + "_Content", World);
        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Label", World);
        auto Value = CWidget::CreateStaticWidget<CTextBlock>(
            Prefix + "_Value", World);

        if (Content && Label && Value)
        {
            ConfigureBodyLabelText(Label);
            ConfigureBodyValueText(Value);

            Content->AddWidget(Label);
            Content->AddWidget(Value);
            Button->SetChild(Content);

            Row.Label = Label;
            Row.Value = Value;
        }

        return Row;
}
} // namespace

void FAlmanacRenderer::CreateWidgets(CAlmanacWidget& Widget)
{
    CreateChromeWidgets(Widget);
    CreateTabWidgets(Widget);
    CreatePageContainers(Widget);
    CreateOverviewWidgets(Widget);
    CreateSatisfactionWidgets(Widget);
    CreatePopulationWidgets(Widget);
    CreateEconomyWidgets(Widget);
    CreateResourceWidgets(Widget);
    CreatePoliticsWidgets(Widget);
    CreateForeignWidgets(Widget);
    CreateBuildingWidgets(Widget);
    CreateConflictWidgets(Widget);
    ApplySelectedPage(Widget);
    ApplyOpenState(Widget);
}

void FAlmanacRenderer::CreateChromeWidgets(CAlmanacWidget& Widget)
{
    auto PanelBackground = Widget.CreateWidget<CImage>("Almanac_Background", 6).lock();

    if (PanelBackground)
    {
        ConfigureFrameImage(
            PanelBackground,
            "AlmanacPanelBackground",
            GPanelTexture);
        Widget.mPanelBackground = PanelBackground;
    }

    auto ContentFrame = Widget.CreateWidget<CImage>("Almanac_ContentFrame", 7).lock();

    if (ContentFrame)
    {
        ConfigureFrameImage(
            ContentFrame,
            "AlmanacContentFrameTexture",
            GContentFrameTexture);
        Widget.mContentFrame = ContentFrame;
    }

    auto TitleRibbon = Widget.CreateWidget<CImage>("Almanac_TitleRibbon", 8).lock();

    if (TitleRibbon)
    {
        ConfigureFrameImage(
            TitleRibbon,
            "AlmanacTitleRibbonTexture",
            GTitleRibbonTexture);
        Widget.mTitleRibbon = TitleRibbon;
    }

    auto TabMarker = Widget.CreateWidget<CImage>("Almanac_TabMarker", 8).lock();

    if (TabMarker)
    {
        ConfigureFrameImage(
            TabMarker,
            "AlmanacTabMarkerTexture",
            GTabMarkerTexture);
        TabMarker->SetTint(0.32f, 0.46f, 0.72f, 0.98f);
        Widget.mTabMarker = TabMarker;
    }

    auto LeftRailTrack = Widget.CreateWidget<CImage>("Almanac_LeftRailTrack", 8).lock();

    if (LeftRailTrack)
    {
        ConfigureFrameImage(
            LeftRailTrack,
            "AlmanacLeftRailTrackTexture",
            GRailTrackTexture);
        LeftRailTrack->SetTint(0.94f, 0.90f, 0.78f, 0.95f);
        Widget.mLeftRailTrack = LeftRailTrack;
    }

    auto LeftRailThumb = Widget.CreateWidget<CImage>("Almanac_LeftRailThumb", 9).lock();

    if (LeftRailThumb)
    {
        ConfigureFrameImage(
            LeftRailThumb,
            "AlmanacLeftRailThumbTexture",
            GRailThumbTexture);
        LeftRailThumb->SetTint(0.96f, 0.82f, 0.26f, 0.95f);
        Widget.mLeftRailThumb = LeftRailThumb;
    }

    auto TitleText = Widget.CreateWidget<CTextBlock>("Almanac_Title", 9).lock();

    if (TitleText)
    {
        TitleText->SetText(GetPageTitle(Widget.mSelectedPage).c_str());
        ConfigureTitleText(TitleText);
        Widget.mTitleText = TitleText;
    }

    auto CloseButton = Widget.CreateWidget<CButton>("Almanac_Close", 10).lock();

    if (CloseButton)
    {
        ConfigureCloseButtonStyle(CloseButton);
        CloseButton->SetEventCallback<CAlmanacWidget>(
            EButtonEventState::Click, &Widget,
            &CAlmanacWidget::OnCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "Almanac_CloseText", Widget.mWorld);

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
            CloseButton->SetChild(CloseText);
        }

        Widget.mCloseButton = CloseButton;
    }
}

void FAlmanacRenderer::CreateTabWidgets(CAlmanacWidget& Widget)
{
    Widget.mTabButtons.resize(static_cast<size_t>(EAlmanacPage::Count));

    void (CAlmanacWidget::*TabCallbacks[static_cast<size_t>(EAlmanacPage::Count)])() =
    {
        &CAlmanacWidget::OnOverviewTabClick,
        &CAlmanacWidget::OnSatisfactionTabClick,
        &CAlmanacWidget::OnPopulationTabClick,
        &CAlmanacWidget::OnEconomyTabClick,
        &CAlmanacWidget::OnResourcesTabClick,
        &CAlmanacWidget::OnPoliticsTabClick,
        &CAlmanacWidget::OnForeignTabClick,
        &CAlmanacWidget::OnBuildingsTabClick,
        &CAlmanacWidget::OnConflictTabClick
    };

    for (size_t Index = 0; Index < Widget.mTabButtons.size(); ++Index)
    {
        auto Button = Widget.CreateWidget<CButton>(
            "Almanac_Tab_" + std::to_string(Index + 1), 10).lock();

        if (!Button)
            continue;

        ConfigureTabButtonStyle(
            Button, Index == static_cast<size_t>(Widget.mSelectedPage));
        ApplyTextureToAllButtonStates(
            Button,
            "AlmanacTabTexture_" + std::to_string(Index),
            GTabTexture);
        Button->SetEventCallback<CAlmanacWidget>(
            EButtonEventState::Click, &Widget, TabCallbacks[Index]);

        auto Icon = CWidget::CreateStaticWidget<CImage>(
            "Almanac_TabIcon_" + std::to_string(Index + 1), Widget.mWorld);

        if (Icon)
        {
            Icon->SetTexture(
                "AlmanacTabIconTex_" + std::to_string(Index),
                GPageTabIcons[Index]);
            Icon->SetTint(1.f, 1.f, 1.f, 1.f);
            Button->SetChild(Icon);
        }

        Widget.mTabButtons[Index] = Button;
    }
}

void FAlmanacRenderer::CreatePageContainers(CAlmanacWidget& Widget)
{
    for (size_t Index = 0; Index < Widget.mPages.size(); ++Index)
    {
        auto Page = Widget.CreateWidget<CWidgetContainer>(
            "Almanac_Page_" + std::to_string(Index), 7).lock();

        if (Page)
            Widget.mPages[Index] = Page;
    }
}

void FAlmanacRenderer::CreateOverviewWidgets(CAlmanacWidget& Widget)
{
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Overview)].lock();
        const TCHAR* CardIcons[GOverviewCardCount] =
        {
            TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_housing.png"),
            TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_industry.png"),
            GApprovalIcon,
            TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_industry.png"),
            GMoneyIcon,
            GMoneyIcon,
            GPoliticsFactionIcons[0],
            GPoliticsFactionIcons[5],
            TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Raids.png"),
            GForeignPowerIcons[0],
            GForeignPowerIcons[2]
        };
        const wchar_t* SectionTitles[GOverviewSectionTitleCount] =
        {
            UIStrings::Get(L"almanac.overview.section.population").c_str(),
            UIStrings::Get(L"almanac.overview.section.satisfaction").c_str(),
            UIStrings::Get(L"almanac.overview.section.economy").c_str(),
            UIStrings::Get(L"almanac.overview.section.politics").c_str(),
            UIStrings::Get(L"almanac.overview.section.conflict").c_str(),
            UIStrings::Get(L"almanac.overview.section.foreign").c_str()
        };

        Widget.mOverviewCards.reserve(GOverviewCardCount);

        for (int Index = 0; Index < GOverviewCardCount; ++Index)
        {
            Widget.mOverviewCards.push_back(CreateCard(
                Page,
                "Almanac_OverviewCard_" + std::to_string(Index + 1),
                CardIcons[Index]));
        }

        for (int Index = 0; Index < GOverviewSectionTitleCount; ++Index)
        {
            auto Title = Page->CreateWidget<CTextBlock>(
                "Almanac_OverviewSectionTitle_" + std::to_string(Index + 1), 2);
            if (auto Text = Title.lock())
            {
                ConfigureSectionText(Text);
                Text->SetText(SectionTitles[Index]);
            }
            Widget.mOverviewSectionTitles.push_back(Title);
        }

        Widget.mOverviewElectionLeftArrow = Page->CreateWidget<CImage>(
            "Almanac_OverviewElectionLeftArrow", 2);
        if (auto Arrow = Widget.mOverviewElectionLeftArrow.lock())
        {
            Arrow->SetTexture(
                "AlmanacOverviewElectionLeftArrowTexture",
                GDropdownArrowTexture);
            Arrow->SetTint(0.30f, 0.70f, 0.88f, 0.96f);
            Arrow->SetPivot(0.5f, 0.5f);
            Arrow->SetAngle(90.f);
        }

        Widget.mOverviewElectionRightArrow = Page->CreateWidget<CImage>(
            "Almanac_OverviewElectionRightArrow", 2);
        if (auto Arrow = Widget.mOverviewElectionRightArrow.lock())
        {
            Arrow->SetTexture(
                "AlmanacOverviewElectionRightArrowTexture",
                GDropdownArrowTexture);
            Arrow->SetTint(0.30f, 0.70f, 0.88f, 0.96f);
            Arrow->SetPivot(0.5f, 0.5f);
            Arrow->SetAngle(-90.f);
        }

        Widget.mOverviewElectionText = Page->CreateWidget<CTextBlock>(
            "Almanac_OverviewElectionText", 2);
        if (auto Text = Widget.mOverviewElectionText.lock())
        {
            ConfigureAuxiliaryText(Text);
            Text->SetFontSize(16.f);
        }

        Widget.mOverviewSummaryLeft = Page->CreateWidget<CTextBlock>(
            "Almanac_OverviewSummaryLeft", 2);
        Widget.mOverviewSummaryRight = Page->CreateWidget<CTextBlock>(
            "Almanac_OverviewSummaryRight", 2);

        ConfigureNoticeText(Widget.mOverviewSummaryLeft.lock());
        ConfigureNoticeText(Widget.mOverviewSummaryRight.lock());
}

void FAlmanacRenderer::CreateSatisfactionWidgets(CAlmanacWidget& Widget)
{
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Satisfaction)].lock();

        Widget.mSatisfactionListTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_SatisfactionListTitleBackground", 1);
        if (auto ListTitleBackground = Widget.mSatisfactionListTitleBackground.lock())
        {
            ListTitleBackground->SetTexture(
                "AlmanacSatisfactionListTitleBackgroundTexture",
                GBarBackTexture);
            ListTitleBackground->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mSatisfactionListTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_SatisfactionListTitle", 2);
        if (auto ListTitle = Widget.mSatisfactionListTitle.lock())
        {
            ConfigureSectionText(ListTitle);
            ListTitle->SetText(
                UIStrings::Get(L"almanac.satisfaction.title").c_str());
        }

        for (int Index = 0; Index < GSatisfactionRowCount; ++Index)
        {
            Widget.mSatisfactionRows.push_back(CreateSatisfactionRow(
                Widget,
                Widget.mWorld,
                Page,
                "Almanac_SatisfactionRow_" + std::to_string(Index + 1),
                Index));
        }

        Widget.mSatisfactionChartTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_SatisfactionChartTitleBackground", 1);
        if (auto ChartTitleBackground = Widget.mSatisfactionChartTitleBackground.lock())
        {
            ChartTitleBackground->SetTexture(
                "AlmanacSatisfactionChartTitleBackgroundTexture",
                GBarBackTexture);
            ChartTitleBackground->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mSatisfactionChartFrame = Page->CreateWidget<CImage>(
            "Almanac_SatisfactionChartFrame", 1);
        if (auto ChartFrame = Widget.mSatisfactionChartFrame.lock())
        {
            ChartFrame->SetTexture(
                "AlmanacSatisfactionChartFrameTexture",
                GBarBackTexture);
            ChartFrame->SetTint(0.98f, 0.98f, 0.96f, 0.14f);
        }

        Widget.mSatisfactionChartYAxisLine = Page->CreateWidget<CImage>(
            "Almanac_SatisfactionChartYAxisLine", 3);
        if (auto YAxisLine = Widget.mSatisfactionChartYAxisLine.lock())
        {
            YAxisLine->SetTexture(
                "AlmanacSatisfactionChartYAxisLineTexture",
                GBarFillTexture);
            YAxisLine->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mSatisfactionChartXAxisLine = Page->CreateWidget<CImage>(
            "Almanac_SatisfactionChartXAxisLine", 3);
        if (auto XAxisLine = Widget.mSatisfactionChartXAxisLine.lock())
        {
            XAxisLine->SetTexture(
                "AlmanacSatisfactionChartXAxisLineTexture",
                GBarFillTexture);
            XAxisLine->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mSatisfactionChartYAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_SatisfactionChartYAxisArrow", 4);
        if (auto YAxisArrow = Widget.mSatisfactionChartYAxisArrow.lock())
        {
            YAxisArrow->SetTexture(
                "AlmanacSatisfactionChartYAxisArrowTexture",
                GTabMarkerTexture);
            YAxisArrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            YAxisArrow->SetPivot(0.5f, 0.5f);
            YAxisArrow->SetAngle(180.f);
        }

        Widget.mSatisfactionChartXAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_SatisfactionChartXAxisArrow", 4);
        if (auto XAxisArrow = Widget.mSatisfactionChartXAxisArrow.lock())
        {
            XAxisArrow->SetTexture(
                "AlmanacSatisfactionChartXAxisArrowTexture",
                GTabMarkerTexture);
            XAxisArrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            XAxisArrow->SetPivot(0.5f, 0.5f);
            XAxisArrow->SetAngle(-90.f);
        }

        Widget.mSatisfactionChartTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_SatisfactionChartTitle", 2);
        ConfigureSectionText(Widget.mSatisfactionChartTitle.lock());

        Widget.mSatisfactionTooltipPanel = Page->CreateWidget<CImage>(
            "Almanac_SatisfactionTooltipPanel", 3);
        if (auto TooltipPanel = Widget.mSatisfactionTooltipPanel.lock())
        {
            ConfigureFrameImage(
                TooltipPanel,
                "AlmanacSatisfactionTooltipTexture",
                GMenuDetailFrameTexture);
            TooltipPanel->SetTint(0.95f, 0.95f, 0.93f, 0.98f);
        }

        Widget.mSatisfactionTooltipText = Page->CreateWidget<CTextBlock>(
            "Almanac_SatisfactionTooltipText", 4);
        if (auto TooltipText = Widget.mSatisfactionTooltipText.lock())
        {
            ConfigureNoticeText(TooltipText);
            TooltipText->SetFontSize(14.f);
            TooltipText->SetTextColor(96, 96, 96, 255);
        }

        for (int Index = 0; Index < GSatisfactionGraphGridLineCount; ++Index)
        {
            auto Line = Page->CreateWidget<CImage>(
                "Almanac_SatisfactionGridLine_" + std::to_string(Index + 1), 2).lock();
            if (Line)
            {
                Line->SetTexture(
                    "AlmanacSatisfactionGridLineTexture_" + std::to_string(Index),
                    GBarBackTexture);
                Line->SetTint(0.90f, 0.76f, 0.32f, 0.38f);
            }
            Widget.mSatisfactionChartGridLines.push_back(Line);
        }

        for (int Index = 0; Index < GSatisfactionGraphSegmentCount; ++Index)
        {
            auto PrimaryLine = Page->CreateWidget<CImage>(
                "Almanac_SatisfactionPrimaryLine_" + std::to_string(Index + 1), 3).lock();
            if (PrimaryLine)
            {
                PrimaryLine->SetTexture(
                    "AlmanacSatisfactionPrimaryLineTexture_" +
                        std::to_string(Index),
                    GBarBackTexture);
                PrimaryLine->SetTint(0.28f, 0.50f, 0.86f, 0.96f);
            }
            Widget.mSatisfactionChartPrimaryLines.push_back(PrimaryLine);

            auto SecondaryLine = Page->CreateWidget<CImage>(
                "Almanac_SatisfactionSecondaryLine_" + std::to_string(Index + 1), 3).lock();
            if (SecondaryLine)
            {
                SecondaryLine->SetTexture(
                    "AlmanacSatisfactionSecondaryLineTexture_" +
                        std::to_string(Index),
                    GBarBackTexture);
                SecondaryLine->SetTint(0.84f, 0.34f, 0.24f, 0.84f);
            }
            Widget.mSatisfactionChartSecondaryLines.push_back(SecondaryLine);
        }

        for (int Index = 0; Index < GSatisfactionGraphPointCount; ++Index)
        {
            auto XAxisLabel = Page->CreateWidget<CTextBlock>(
                "Almanac_SatisfactionXAxisLabel_" + std::to_string(Index + 1), 2);
            ConfigureAuxiliaryText(XAxisLabel.lock());
            Widget.mSatisfactionChartXAxisLabels.push_back(XAxisLabel);
        }

        for (int Index = 0; Index < GSatisfactionGraphGridLineCount; ++Index)
        {
            auto YAxisLabel = Page->CreateWidget<CTextBlock>(
                "Almanac_SatisfactionYAxisLabel_" + std::to_string(Index + 1), 2);
            ConfigureAuxiliaryText(YAxisLabel.lock());
            Widget.mSatisfactionChartYAxisLabels.push_back(YAxisLabel);
        }

        for (int Index = 0; Index < GSatisfactionDetailCount; ++Index)
        {
            Widget.mSatisfactionDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_SatisfactionDetail_" + std::to_string(Index + 1)));
        }
}

void FAlmanacRenderer::CreatePopulationWidgets(CAlmanacWidget& Widget)
{
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Population)].lock();

        for (int Index = 0; Index < GPopulationDetailCount; ++Index)
        {
            Widget.mPopulationDetails.push_back(CreateSelectableDetailRow(
                Widget,
                Widget.mWorld,
                Page,
                "Almanac_PopulationDetail_" + std::to_string(Index + 1),
                Index));
        }

        for (int Index = 0; Index < GPopulationMetricCount; ++Index)
        {
            Widget.mPopulationMetrics.push_back(CreateMetricRow(
                Page,
                "Almanac_PopulationMetric_" + std::to_string(Index + 1)));
        }

        Widget.mPopulationTrendTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_PopulationTrendTitleBackground", 1);
        if (auto ChartTitleBackground = Widget.mPopulationTrendTitleBackground.lock())
        {
            ChartTitleBackground->SetTexture(
                "AlmanacPopulationTrendTitleBackgroundTexture",
                GBarBackTexture);
            ChartTitleBackground->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mPopulationTrendFrame = Page->CreateWidget<CImage>(
            "Almanac_PopulationTrendFrame", 1);
        if (auto ChartFrame = Widget.mPopulationTrendFrame.lock())
        {
            ChartFrame->SetTexture(
                "AlmanacPopulationTrendFrameTexture",
                GBarBackTexture);
            ChartFrame->SetTint(0.98f, 0.98f, 0.96f, 0.14f);
        }

        Widget.mPopulationTrendYAxisLine = Page->CreateWidget<CImage>(
            "Almanac_PopulationTrendYAxisLine", 3);
        if (auto YAxisLine = Widget.mPopulationTrendYAxisLine.lock())
        {
            YAxisLine->SetTexture(
                "AlmanacPopulationTrendYAxisLineTexture",
                GBarFillTexture);
            YAxisLine->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mPopulationTrendXAxisLine = Page->CreateWidget<CImage>(
            "Almanac_PopulationTrendXAxisLine", 3);
        if (auto XAxisLine = Widget.mPopulationTrendXAxisLine.lock())
        {
            XAxisLine->SetTexture(
                "AlmanacPopulationTrendXAxisLineTexture",
                GBarFillTexture);
            XAxisLine->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mPopulationTrendYAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_PopulationTrendYAxisArrow", 4);
        if (auto YAxisArrow = Widget.mPopulationTrendYAxisArrow.lock())
        {
            YAxisArrow->SetTexture(
                "AlmanacPopulationTrendYAxisArrowTexture",
                GTabMarkerTexture);
            YAxisArrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            YAxisArrow->SetPivot(0.5f, 0.5f);
            YAxisArrow->SetAngle(180.f);
        }

        Widget.mPopulationTrendXAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_PopulationTrendXAxisArrow", 4);
        if (auto XAxisArrow = Widget.mPopulationTrendXAxisArrow.lock())
        {
            XAxisArrow->SetTexture(
                "AlmanacPopulationTrendXAxisArrowTexture",
                GTabMarkerTexture);
            XAxisArrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            XAxisArrow->SetPivot(0.5f, 0.5f);
            XAxisArrow->SetAngle(-90.f);
        }

        Widget.mPopulationTrendTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_PopulationTrendTitle", 2);
        ConfigureSectionText(Widget.mPopulationTrendTitle.lock());

        for (int Index = 0; Index < GPopulationTrendGridLineCount; ++Index)
        {
            auto Line = Page->CreateWidget<CImage>(
                "Almanac_PopulationTrendGridLine_" + std::to_string(Index + 1), 2).lock();
            if (Line)
            {
                Line->SetTexture(
                    "AlmanacPopulationTrendGridLineTexture_" +
                        std::to_string(Index),
                    GBarBackTexture);
                Line->SetTint(0.90f, 0.76f, 0.32f, 0.38f);
            }
            Widget.mPopulationTrendGridLines.push_back(Line);
        }

        for (int Index = 0; Index < GPopulationTrendSegmentCount; ++Index)
        {
            auto TrendLine = Page->CreateWidget<CImage>(
                "Almanac_PopulationTrendLine_" + std::to_string(Index + 1), 3).lock();
            if (TrendLine)
            {
                TrendLine->SetTexture(
                    "AlmanacPopulationTrendLineTexture_" +
                        std::to_string(Index),
                    GBarBackTexture);
                TrendLine->SetTint(0.26f, 0.46f, 0.82f, 0.96f);
            }
            Widget.mPopulationTrendLines.push_back(TrendLine);
        }

        for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
        {
            auto ChildBar = Page->CreateWidget<CImage>(
                "Almanac_PopulationTrendChildBar_" + std::to_string(Index + 1), 3).lock();
            if (ChildBar)
            {
                ChildBar->SetTexture(
                    "AlmanacPopulationTrendChildBarTexture_" +
                        std::to_string(Index),
                    GBarFillTexture);
                ChildBar->SetTint(0.31f, 0.48f, 0.80f, 0.94f);
            }
            Widget.mPopulationTrendChildBars.push_back(ChildBar);

            auto AdultBar = Page->CreateWidget<CImage>(
                "Almanac_PopulationTrendAdultBar_" + std::to_string(Index + 1), 3).lock();
            if (AdultBar)
            {
                AdultBar->SetTexture(
                    "AlmanacPopulationTrendAdultBarTexture_" +
                        std::to_string(Index),
                    GBarFillTexture);
                AdultBar->SetTint(0.80f, 0.34f, 0.28f, 0.92f);
            }
            Widget.mPopulationTrendAdultBars.push_back(AdultBar);

            auto RetiredBar = Page->CreateWidget<CImage>(
                "Almanac_PopulationTrendRetiredBar_" + std::to_string(Index + 1), 3).lock();
            if (RetiredBar)
            {
                RetiredBar->SetTexture(
                    "AlmanacPopulationTrendRetiredBarTexture_" +
                        std::to_string(Index),
                    GBarFillTexture);
                RetiredBar->SetTint(0.50f, 0.64f, 0.24f, 0.92f);
            }
            Widget.mPopulationTrendRetiredBars.push_back(RetiredBar);

            auto RichBar = Page->CreateWidget<CImage>(
                "Almanac_PopulationTrendRichBar_" + std::to_string(Index + 1), 3).lock();
            if (RichBar)
            {
                RichBar->SetTexture(
                    "AlmanacPopulationTrendRichBarTexture_" +
                        std::to_string(Index),
                    GBarFillTexture);
                RichBar->SetTint(0.90f, 0.76f, 0.18f, 0.94f);
            }
            Widget.mPopulationTrendRichBars.push_back(RichBar);

            auto FilthyRichBar = Page->CreateWidget<CImage>(
                "Almanac_PopulationTrendFilthyRichBar_" + std::to_string(Index + 1), 3).lock();
            if (FilthyRichBar)
            {
                FilthyRichBar->SetTexture(
                    "AlmanacPopulationTrendFilthyRichBarTexture_" +
                        std::to_string(Index),
                    GBarFillTexture);
                FilthyRichBar->SetTint(0.58f, 0.30f, 0.66f, 0.94f);
            }
            Widget.mPopulationTrendFilthyRichBars.push_back(FilthyRichBar);
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            auto XAxisLabel = Page->CreateWidget<CTextBlock>(
                "Almanac_PopulationTrendXAxisLabel_" + std::to_string(Index + 1), 2);
            ConfigureAuxiliaryText(XAxisLabel.lock());
            Widget.mPopulationTrendXAxisLabels.push_back(XAxisLabel);
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            auto YAxisLabel = Page->CreateWidget<CTextBlock>(
                "Almanac_PopulationTrendYAxisLabel_" + std::to_string(Index + 1), 2);
            ConfigureAuxiliaryText(YAxisLabel.lock());
            Widget.mPopulationTrendYAxisLabels.push_back(YAxisLabel);
        }

        Widget.mPopulationChangeTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_PopulationChangeTitleBackground", 1);
        if (auto ChartTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
        {
            ChartTitleBackground->SetTexture(
                "AlmanacPopulationChangeTitleBackgroundTexture",
                GBarBackTexture);
            ChartTitleBackground->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mPopulationChangeFrame = Page->CreateWidget<CImage>(
            "Almanac_PopulationChangeFrame", 1);
        if (auto ChartFrame = Widget.mPopulationChangeFrame.lock())
        {
            ChartFrame->SetTexture(
                "AlmanacPopulationChangeFrameTexture",
                GBarBackTexture);
            ChartFrame->SetTint(0.98f, 0.98f, 0.96f, 0.14f);
        }

        Widget.mPopulationChangeYAxisLine = Page->CreateWidget<CImage>(
            "Almanac_PopulationChangeYAxisLine", 3);
        if (auto YAxisLine = Widget.mPopulationChangeYAxisLine.lock())
        {
            YAxisLine->SetTexture(
                "AlmanacPopulationChangeYAxisLineTexture",
                GBarFillTexture);
            YAxisLine->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mPopulationChangeXAxisLine = Page->CreateWidget<CImage>(
            "Almanac_PopulationChangeXAxisLine", 3);
        if (auto XAxisLine = Widget.mPopulationChangeXAxisLine.lock())
        {
            XAxisLine->SetTexture(
                "AlmanacPopulationChangeXAxisLineTexture",
                GBarFillTexture);
            XAxisLine->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mPopulationChangeYAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_PopulationChangeYAxisArrow", 4);
        if (auto YAxisArrow = Widget.mPopulationChangeYAxisArrow.lock())
        {
            YAxisArrow->SetTexture(
                "AlmanacPopulationChangeYAxisArrowTexture",
                GTabMarkerTexture);
            YAxisArrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            YAxisArrow->SetPivot(0.5f, 0.5f);
            YAxisArrow->SetAngle(180.f);
        }

        Widget.mPopulationChangeXAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_PopulationChangeXAxisArrow", 4);
        if (auto XAxisArrow = Widget.mPopulationChangeXAxisArrow.lock())
        {
            XAxisArrow->SetTexture(
                "AlmanacPopulationChangeXAxisArrowTexture",
                GTabMarkerTexture);
            XAxisArrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            XAxisArrow->SetPivot(0.5f, 0.5f);
            XAxisArrow->SetAngle(-90.f);
        }

        Widget.mPopulationChangeTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_PopulationChangeTitle", 2);
        ConfigureSectionText(Widget.mPopulationChangeTitle.lock());

        for (int Index = 0; Index < GPopulationChangeGridLineCount; ++Index)
        {
            auto Line = Page->CreateWidget<CImage>(
                "Almanac_PopulationChangeGridLine_" + std::to_string(Index + 1), 2).lock();
            if (Line)
            {
                Line->SetTexture(
                    "AlmanacPopulationChangeGridLineTexture_" +
                        std::to_string(Index),
                    GBarBackTexture);
                Line->SetTint(0.90f, 0.76f, 0.32f, 0.38f);
            }
            Widget.mPopulationChangeGridLines.push_back(Line);
        }

        for (int Index = 0; Index < GPopulationChangeBarCount; ++Index)
        {
            auto PositiveBar = Page->CreateWidget<CImage>(
                "Almanac_PopulationChangePositiveBar_" + std::to_string(Index + 1), 3).lock();
            if (PositiveBar)
            {
                PositiveBar->SetTexture(
                    "AlmanacPopulationChangePositiveBarTexture_" +
                        std::to_string(Index),
                    GBarFillTexture);
                PositiveBar->SetTint(0.31f, 0.49f, 0.82f, 0.92f);
            }
            Widget.mPopulationChangePositiveBars.push_back(PositiveBar);

            auto NegativeBar = Page->CreateWidget<CImage>(
                "Almanac_PopulationChangeNegativeBar_" + std::to_string(Index + 1), 3).lock();
            if (NegativeBar)
            {
                NegativeBar->SetTexture(
                    "AlmanacPopulationChangeNegativeBarTexture_" +
                        std::to_string(Index),
                    GBarFillTexture);
                NegativeBar->SetTint(0.80f, 0.28f, 0.24f, 0.88f);
            }
            Widget.mPopulationChangeNegativeBars.push_back(NegativeBar);
        }

        for (int Index = 0; Index < GPopulationChangeXAxisLabelCount; ++Index)
        {
            auto XAxisLabel = Page->CreateWidget<CTextBlock>(
                "Almanac_PopulationChangeXAxisLabel_" + std::to_string(Index + 1), 2);
            ConfigureAuxiliaryText(XAxisLabel.lock());
            Widget.mPopulationChangeXAxisLabels.push_back(XAxisLabel);
        }

        for (int Index = 0; Index < GPopulationChangeYAxisLabelCount; ++Index)
        {
            auto YAxisLabel = Page->CreateWidget<CTextBlock>(
                "Almanac_PopulationChangeYAxisLabel_" + std::to_string(Index + 1), 2);
            ConfigureAuxiliaryText(YAxisLabel.lock());
            Widget.mPopulationChangeYAxisLabels.push_back(YAxisLabel);
        }
}

void FAlmanacRenderer::CreateEconomyWidgets(CAlmanacWidget& Widget)
{
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Economy)].lock();

        Widget.mEconomyTrendTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_EconomyTrendTitleBackground", 1);
        if (auto ChartTitleBackground = Widget.mEconomyTrendTitleBackground.lock())
        {
            ChartTitleBackground->SetTexture(
                "AlmanacEconomyTrendTitleBackgroundTexture",
                GRowTexture);
            ChartTitleBackground->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mEconomyTrendFrame = Page->CreateWidget<CImage>(
            "Almanac_EconomyTrendFrame", 1);
        if (auto ChartFrame = Widget.mEconomyTrendFrame.lock())
        {
            ChartFrame->SetTexture(
                "AlmanacEconomyTrendFrameTexture",
                GCardTexture);
            ChartFrame->SetTint(0.98f, 0.98f, 0.96f, 0.14f);
        }

        Widget.mEconomyTrendYAxisLine = Page->CreateWidget<CImage>(
            "Almanac_EconomyTrendYAxisLine", 2);
        if (auto YAxisLine = Widget.mEconomyTrendYAxisLine.lock())
        {
            YAxisLine->SetTexture(
                "AlmanacEconomyTrendYAxisLineTexture",
                GBarBackTexture);
            YAxisLine->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mEconomyTrendXAxisLine = Page->CreateWidget<CImage>(
            "Almanac_EconomyTrendXAxisLine", 2);
        if (auto XAxisLine = Widget.mEconomyTrendXAxisLine.lock())
        {
            XAxisLine->SetTexture(
                "AlmanacEconomyTrendXAxisLineTexture",
                GBarBackTexture);
            XAxisLine->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mEconomyTrendYAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_EconomyTrendYAxisArrow", 2);
        if (auto YAxisArrow = Widget.mEconomyTrendYAxisArrow.lock())
        {
            YAxisArrow->SetTexture(
                "AlmanacEconomyTrendYAxisArrowTexture",
                GDropdownArrowTexture);
            YAxisArrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            YAxisArrow->SetPivot(0.5f, 0.5f);
            YAxisArrow->SetAngle(180.f);
        }

        Widget.mEconomyTrendXAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_EconomyTrendXAxisArrow", 2);
        if (auto XAxisArrow = Widget.mEconomyTrendXAxisArrow.lock())
        {
            XAxisArrow->SetTexture(
                "AlmanacEconomyTrendXAxisArrowTexture",
                GDropdownArrowTexture);
            XAxisArrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            XAxisArrow->SetPivot(0.5f, 0.5f);
            XAxisArrow->SetAngle(-90.f);
        }

        Widget.mEconomyTrendTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_EconomyTrendTitle", 2);
        ConfigureSectionText(Widget.mEconomyTrendTitle.lock());

        Widget.mEconomyBreakdownTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_EconomyBreakdownTitleBackground", 1);
        if (auto TitleBackground = Widget.mEconomyBreakdownTitleBackground.lock())
        {
            TitleBackground->SetTexture(
                "AlmanacEconomyBreakdownTitleBackgroundTexture",
                GBarBackTexture);
            TitleBackground->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mEconomyBreakdownTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_EconomyBreakdownTitle", 2);
        ConfigureSectionText(Widget.mEconomyBreakdownTitle.lock());

        for (int Index = 0; Index < GEconomyTrendGridLineCount; ++Index)
        {
            auto Line = Page->CreateWidget<CImage>(
                "Almanac_EconomyTrendGridLine_" + std::to_string(Index + 1), 1).lock();
            if (Line)
            {
                Line->SetTexture(
                    "AlmanacEconomyTrendGridLineTexture_" + std::to_string(Index),
                    GBarBackTexture);
                Line->SetTint(0.90f, 0.76f, 0.32f, 0.38f);
            }
            Widget.mEconomyTrendGridLines.push_back(Line);
        }

        for (int Index = 0; Index < GEconomyTrendSegmentCount * 2; ++Index)
        {
            auto TrendLine = Page->CreateWidget<CImage>(
                "Almanac_EconomyTrendLine_" + std::to_string(Index + 1), 3).lock();
            if (TrendLine)
            {
                TrendLine->SetTexture(
                    "AlmanacEconomyTrendLineTexture_" + std::to_string(Index),
                    GBarBackTexture);
                TrendLine->SetTint(0.84f, 0.66f, 0.06f, 0.96f);
            }
            Widget.mEconomyTrendLines.push_back(TrendLine);
        }

        for (int Index = 0; Index < GEconomyTrendBarCount; ++Index)
        {
            auto Bar = Page->CreateWidget<CImage>(
                "Almanac_EconomyTrendBar_" + std::to_string(Index + 1), 3).lock();
            if (Bar)
            {
                Bar->SetTexture(
                    "AlmanacEconomyTrendBarTexture_" + std::to_string(Index),
                    GBarFillTexture);
                Bar->SetTint(0.12f, 0.82f, 0.38f, 0.95f);
            }
            Widget.mEconomyTrendBars.push_back(Bar);

            auto SecondaryBar = Page->CreateWidget<CImage>(
                "Almanac_EconomyTrendSecondaryBar_" + std::to_string(Index + 1), 3).lock();
            if (SecondaryBar)
            {
                SecondaryBar->SetTexture(
                    "AlmanacEconomyTrendSecondaryBarTexture_" + std::to_string(Index),
                    GBarFillTexture);
                SecondaryBar->SetTint(0.30f, 0.48f, 0.78f, 0.94f);
            }
            Widget.mEconomyTrendSecondaryBars.push_back(SecondaryBar);

            auto TertiaryBar = Page->CreateWidget<CImage>(
                "Almanac_EconomyTrendTertiaryBar_" + std::to_string(Index + 1), 3).lock();
            if (TertiaryBar)
            {
                TertiaryBar->SetTexture(
                    "AlmanacEconomyTrendTertiaryBarTexture_" + std::to_string(Index),
                    GBarFillTexture);
                TertiaryBar->SetTint(0.56f, 0.68f, 0.24f, 0.92f);
            }
            Widget.mEconomyTrendTertiaryBars.push_back(TertiaryBar);
        }

        for (int Index = 0; Index < GEconomyTrendXAxisLabelCount; ++Index)
        {
            auto XAxisLabel = Page->CreateWidget<CTextBlock>(
                "Almanac_EconomyTrendXAxisLabel_" + std::to_string(Index + 1), 2);
            ConfigureAuxiliaryText(XAxisLabel.lock());
            Widget.mEconomyTrendXAxisLabels.push_back(XAxisLabel);
        }

        for (int Index = 0; Index < GEconomyTrendYAxisLabelCount; ++Index)
        {
            auto YAxisLabel = Page->CreateWidget<CTextBlock>(
                "Almanac_EconomyTrendYAxisLabel_" + std::to_string(Index + 1), 2);
            ConfigureAuxiliaryText(YAxisLabel.lock());
            Widget.mEconomyTrendYAxisLabels.push_back(YAxisLabel);
        }

        Widget.mEconomyChangeFrame = Page->CreateWidget<CImage>(
            "Almanac_EconomyChangeFrame", 1);
        if (auto ChartFrame = Widget.mEconomyChangeFrame.lock())
        {
            ChartFrame->SetTexture(
                "AlmanacEconomyChangeFrameTexture",
                GCardTexture);
            ChartFrame->SetTint(0.98f, 0.98f, 0.96f, 0.14f);
        }

        Widget.mEconomyChangeYAxisLine = Page->CreateWidget<CImage>(
            "Almanac_EconomyChangeYAxisLine", 2);
        if (auto YAxisLine = Widget.mEconomyChangeYAxisLine.lock())
        {
            YAxisLine->SetTexture(
                "AlmanacEconomyChangeYAxisLineTexture",
                GBarBackTexture);
            YAxisLine->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mEconomyChangeXAxisLine = Page->CreateWidget<CImage>(
            "Almanac_EconomyChangeXAxisLine", 2);
        if (auto XAxisLine = Widget.mEconomyChangeXAxisLine.lock())
        {
            XAxisLine->SetTexture(
                "AlmanacEconomyChangeXAxisLineTexture",
                GBarBackTexture);
            XAxisLine->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mEconomyChangeYAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_EconomyChangeYAxisArrow", 2);
        if (auto YAxisArrow = Widget.mEconomyChangeYAxisArrow.lock())
        {
            YAxisArrow->SetTexture(
                "AlmanacEconomyChangeYAxisArrowTexture",
                GDropdownArrowTexture);
            YAxisArrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            YAxisArrow->SetPivot(0.5f, 0.5f);
            YAxisArrow->SetAngle(180.f);
        }

        Widget.mEconomyChangeXAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_EconomyChangeXAxisArrow", 2);
        if (auto XAxisArrow = Widget.mEconomyChangeXAxisArrow.lock())
        {
            XAxisArrow->SetTexture(
                "AlmanacEconomyChangeXAxisArrowTexture",
                GDropdownArrowTexture);
            XAxisArrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            XAxisArrow->SetPivot(0.5f, 0.5f);
            XAxisArrow->SetAngle(-90.f);
        }

        for (int Index = 0; Index < GEconomyChangeGridLineCount; ++Index)
        {
            auto Line = Page->CreateWidget<CImage>(
                "Almanac_EconomyChangeGridLine_" + std::to_string(Index + 1), 1).lock();
            if (Line)
            {
                Line->SetTexture(
                    "AlmanacEconomyChangeGridLineTexture_" + std::to_string(Index),
                    GBarBackTexture);
                Line->SetTint(0.90f, 0.76f, 0.32f, 0.38f);
            }
            Widget.mEconomyChangeGridLines.push_back(Line);
        }

        for (int Index = 0; Index < GEconomyChangeBarCount; ++Index)
        {
            auto PositiveBar = Page->CreateWidget<CImage>(
                "Almanac_EconomyChangePositiveBar_" + std::to_string(Index + 1), 3).lock();
            if (PositiveBar)
            {
                PositiveBar->SetTexture(
                    "AlmanacEconomyChangePositiveBarTexture_" + std::to_string(Index),
                    GBarFillTexture);
                PositiveBar->SetTint(0.34f, 0.56f, 0.88f, 0.92f);
            }
            Widget.mEconomyChangePositiveBars.push_back(PositiveBar);

            auto NegativeBar = Page->CreateWidget<CImage>(
                "Almanac_EconomyChangeNegativeBar_" + std::to_string(Index + 1), 3).lock();
            if (NegativeBar)
            {
                NegativeBar->SetTexture(
                    "AlmanacEconomyChangeNegativeBarTexture_" + std::to_string(Index),
                    GBarFillTexture);
                NegativeBar->SetTint(0.82f, 0.30f, 0.28f, 0.90f);
            }
            Widget.mEconomyChangeNegativeBars.push_back(NegativeBar);
        }

        for (int Index = 0; Index < GEconomyChangeYAxisLabelCount; ++Index)
        {
            auto YAxisLabel = Page->CreateWidget<CTextBlock>(
                "Almanac_EconomyChangeYAxisLabel_" + std::to_string(Index + 1), 2);
            ConfigureAuxiliaryText(YAxisLabel.lock());
            Widget.mEconomyChangeYAxisLabels.push_back(YAxisLabel);
        }

        for (int Index = 0; Index < GEconomyDetailCount; ++Index)
        {
            Widget.mEconomyDetails.push_back(CreateSelectableEconomyDetailRow(
                Widget,
                Widget.mWorld,
                Page,
                "Almanac_EconomyDetail_" + std::to_string(Index + 1),
                Index));
        }

        for (int Index = 0; Index < GEconomyBreakdownRowCount; ++Index)
        {
            Widget.mEconomyBreakdownRows.push_back(CreateDetailRow(
                Page,
                "Almanac_EconomyBreakdownRow_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GEconomyMetricCount; ++Index)
        {
            Widget.mEconomyMetrics.push_back(CreateMetricRow(
                Page,
                "Almanac_EconomyMetric_" + std::to_string(Index + 1)));
        }
}

void FAlmanacRenderer::CreateResourceWidgets(CAlmanacWidget& Widget)
{
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Resources)].lock();

        Widget.mResourceListTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_ResourceListTitleBackground", 1);
        if (auto Background = Widget.mResourceListTitleBackground.lock())
        {
            Background->SetTexture(
                "AlmanacResourceListTitleBackgroundTexture",
                GBarBackTexture);
            Background->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mResourceListTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceListTitle", 2);
        if (auto Title = Widget.mResourceListTitle.lock())
        {
            ConfigureSectionText(Title);
            Title->SetText(UIStrings::Get(L"almanac.resource.title").c_str());
        }

        Widget.mResourceFilterBackground = Page->CreateWidget<CImage>(
            "Almanac_ResourceFilterBackground", 1);
        if (auto Background = Widget.mResourceFilterBackground.lock())
        {
            Background->SetTexture(
                "AlmanacResourceFilterBackgroundTexture",
                GRowTexture);
            Background->SetTint(1.f, 1.f, 1.f, 0.94f);
        }

        Widget.mResourceFilterText = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceFilterText", 2);
        if (auto Text = Widget.mResourceFilterText.lock())
        {
            ConfigureBodyLabelText(Text);
            Text->SetText(
                UIStrings::Get(L"almanac.resource.filter.all").c_str());
        }

        Widget.mResourceFilterLeftIcon = Page->CreateWidget<CImage>(
            "Almanac_ResourceFilterLeftIcon", 2);
        if (auto Icon = Widget.mResourceFilterLeftIcon.lock())
        {
            Icon->SetTexture(
                "AlmanacResourceFilterLeftIconTexture",
                GPageTabIcons[4]);
            Icon->SetTint(0.94f, 0.78f, 0.20f, 0.96f);
        }

        Widget.mResourceFilterSortIcon = Page->CreateWidget<CImage>(
            "Almanac_ResourceFilterSortIcon", 2);
        if (auto Icon = Widget.mResourceFilterSortIcon.lock())
        {
            Icon->SetTexture(
                "AlmanacResourceFilterSortIconTexture",
                GBarFillTexture);
            Icon->SetTint(0.92f, 0.74f, 0.18f, 0.92f);
        }

        Widget.mResourceFilterSortArrow = Page->CreateWidget<CImage>(
            "Almanac_ResourceFilterSortArrow", 2);
        if (auto Arrow = Widget.mResourceFilterSortArrow.lock())
        {
            Arrow->SetTexture(
                "AlmanacResourceFilterSortArrowTexture",
                GDropdownArrowTexture);
            Arrow->SetTint(0.92f, 0.74f, 0.18f, 0.96f);
            Arrow->SetPivot(0.5f, 0.5f);
        }

        for (int Index = 0; Index < GResourceRowCount; ++Index)
        {
            Widget.mResourceRows.push_back(CreateSelectableResourceDetailRow(
                Widget,
                Widget.mWorld,
                Page,
                "Almanac_ResourceRow_" + std::to_string(Index + 1),
                Index));
        }

        Widget.mResourceProductionTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_ResourceProductionTitleBackground", 1);
        if (auto Background = Widget.mResourceProductionTitleBackground.lock())
        {
            Background->SetTexture(
                "AlmanacResourceProductionTitleBackgroundTexture",
                GBarBackTexture);
            Background->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mResourceProductionTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceProductionTitle", 2);
        ConfigureSectionText(Widget.mResourceProductionTitle.lock());

        Widget.mResourceProductionFrame = Page->CreateWidget<CImage>(
            "Almanac_ResourceProductionFrame", 1);
        if (auto Frame = Widget.mResourceProductionFrame.lock())
        {
            Frame->SetTexture(
                "AlmanacResourceProductionFrameTexture",
                GBarBackTexture);
            Frame->SetTint(0.98f, 0.98f, 0.96f, 0.12f);
        }

        Widget.mResourceProductionYAxisLine = Page->CreateWidget<CImage>(
            "Almanac_ResourceProductionYAxisLine", 3);
        if (auto Line = Widget.mResourceProductionYAxisLine.lock())
        {
            Line->SetTexture(
                "AlmanacResourceProductionYAxisLineTexture",
                GBarFillTexture);
            Line->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mResourceProductionXAxisLine = Page->CreateWidget<CImage>(
            "Almanac_ResourceProductionXAxisLine", 3);
        if (auto Line = Widget.mResourceProductionXAxisLine.lock())
        {
            Line->SetTexture(
                "AlmanacResourceProductionXAxisLineTexture",
                GBarFillTexture);
            Line->SetTint(0.88f, 0.70f, 0.18f, 0.92f);
        }

        Widget.mResourceProductionYAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_ResourceProductionYAxisArrow", 3);
        if (auto Arrow = Widget.mResourceProductionYAxisArrow.lock())
        {
            Arrow->SetTexture(
                "AlmanacResourceProductionYAxisArrowTexture",
                GDropdownArrowTexture);
            Arrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            Arrow->SetPivot(0.5f, 0.5f);
            Arrow->SetAngle(180.f);
        }

        Widget.mResourceProductionXAxisArrow = Page->CreateWidget<CImage>(
            "Almanac_ResourceProductionXAxisArrow", 3);
        if (auto Arrow = Widget.mResourceProductionXAxisArrow.lock())
        {
            Arrow->SetTexture(
                "AlmanacResourceProductionXAxisArrowTexture",
                GDropdownArrowTexture);
            Arrow->SetTint(0.88f, 0.70f, 0.18f, 0.96f);
            Arrow->SetPivot(0.5f, 0.5f);
            Arrow->SetAngle(-90.f);
        }

        for (int Index = 0; Index < GResourceProductionGridLineCount; ++Index)
        {
            auto Line = Page->CreateWidget<CImage>(
                "Almanac_ResourceProductionGridLine_" + std::to_string(Index + 1), 1).lock();
            if (Line)
            {
                Line->SetTexture(
                    "AlmanacResourceProductionGridLineTexture_" + std::to_string(Index),
                    GBarBackTexture);
                Line->SetTint(0.90f, 0.76f, 0.32f, 0.34f);
            }
            Widget.mResourceProductionGridLines.push_back(Line);
        }

        for (int Index = 0; Index < GResourceProductionBarCount; ++Index)
        {
            auto Bar = Page->CreateWidget<CImage>(
                "Almanac_ResourceProductionBar_" + std::to_string(Index + 1), 3).lock();
            if (Bar)
            {
                Bar->SetTexture(
                    "AlmanacResourceProductionBarTexture_" + std::to_string(Index),
                    GBarFillTexture);
                Bar->SetTint(0.22f, 0.58f, 0.82f, 0.92f);
            }
            Widget.mResourceProductionBars.push_back(Bar);
        }

        for (int Index = 0; Index < GResourceProductionXAxisLabelCount; ++Index)
        {
            auto Label = Page->CreateWidget<CTextBlock>(
                "Almanac_ResourceProductionXAxisLabel_" + std::to_string(Index + 1), 2);
            ConfigureAuxiliaryText(Label.lock());
            Widget.mResourceProductionXAxisLabels.push_back(Label);
        }

        for (int Index = 0; Index < GResourceProductionYAxisLabelCount; ++Index)
        {
            auto Label = Page->CreateWidget<CTextBlock>(
                "Almanac_ResourceProductionYAxisLabel_" + std::to_string(Index + 1), 2);
            ConfigureAuxiliaryText(Label.lock());
            Widget.mResourceProductionYAxisLabels.push_back(Label);
        }

        Widget.mResourceProductionLegendPrimarySwatch = Page->CreateWidget<CImage>(
            "Almanac_ResourceProductionLegendPrimarySwatch", 2);
        if (auto Swatch = Widget.mResourceProductionLegendPrimarySwatch.lock())
        {
            Swatch->SetTexture(
                "AlmanacResourceProductionLegendPrimarySwatchTexture",
                GBarFillTexture);
            Swatch->SetTint(0.22f, 0.58f, 0.82f, 0.92f);
        }

        Widget.mResourceProductionLegendPrimaryText = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceProductionLegendPrimaryText", 2);
        ConfigureAuxiliaryText(Widget.mResourceProductionLegendPrimaryText.lock());

        Widget.mResourceProductionLegendSecondarySwatch = Page->CreateWidget<CImage>(
            "Almanac_ResourceProductionLegendSecondarySwatch", 2);
        if (auto Swatch = Widget.mResourceProductionLegendSecondarySwatch.lock())
        {
            Swatch->SetTexture(
                "AlmanacResourceProductionLegendSecondarySwatchTexture",
                GBarFillTexture);
            Swatch->SetTint(0.38f, 0.70f, 0.28f, 0.92f);
        }

        Widget.mResourceProductionLegendSecondaryText = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceProductionLegendSecondaryText", 2);
        ConfigureAuxiliaryText(Widget.mResourceProductionLegendSecondaryText.lock());

        Widget.mResourceDistributionTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_ResourceDistributionTitleBackground", 1);
        if (auto Background = Widget.mResourceDistributionTitleBackground.lock())
        {
            Background->SetTexture(
                "AlmanacResourceDistributionTitleBackgroundTexture",
                GBarBackTexture);
            Background->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mResourceDistributionTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceDistributionTitle", 2);
        ConfigureSectionText(Widget.mResourceDistributionTitle.lock());

        Widget.mResourceDistributionFilterBackground = Page->CreateWidget<CImage>(
            "Almanac_ResourceDistributionFilterBackground", 1);
        if (auto Background = Widget.mResourceDistributionFilterBackground.lock())
        {
            Background->SetTexture(
                "AlmanacResourceDistributionFilterBackgroundTexture",
                GRowTexture);
            Background->SetTint(1.f, 1.f, 1.f, 0.94f);
        }

        Widget.mResourceDistributionFilterText = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceDistributionFilterText", 2);
        if (auto Text = Widget.mResourceDistributionFilterText.lock())
        {
            ConfigureBodyLabelText(Text);
            Text->SetText(
                UIStrings::Get(L"almanac.resource.filter.period_24m").c_str());
        }

        for (int Index = 0; Index < GResourceDistributionRowCount; ++Index)
        {
            Widget.mResourceDistributionRows.push_back(CreateMetricRow(
                Page,
                "Almanac_ResourceDistributionRow_" + std::to_string(Index + 1)));
        }

        Widget.mResourceTrackingTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_ResourceTrackingTitleBackground", 1);
        if (auto Background = Widget.mResourceTrackingTitleBackground.lock())
        {
            Background->SetTexture(
                "AlmanacResourceTrackingTitleBackgroundTexture",
                GBarBackTexture);
            Background->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mResourceTrackingTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceTrackingTitle", 2);
        ConfigureSectionText(Widget.mResourceTrackingTitle.lock());

        Widget.mResourceTrackingName = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceTrackingName", 2);
        ConfigureBodyLabelText(Widget.mResourceTrackingName.lock());

        Widget.mResourceTrackingValue = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceTrackingValue", 2);
        ConfigureBodyValueText(Widget.mResourceTrackingValue.lock());

        for (int Index = 0; Index < GResourceDetailCount; ++Index)
        {
            Widget.mResourceDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_ResourceDetail_" + std::to_string(Index + 1)));
        }

        Widget.mResourceNotice = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceNotice", 2);
        ConfigureNoticeText(Widget.mResourceNotice.lock());
}

void FAlmanacRenderer::CreatePoliticsWidgets(CAlmanacWidget& Widget)
{
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Politics)].lock();

        Widget.mPoliticsListTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_PoliticsListTitleBackground", 1);
        if (auto Background = Widget.mPoliticsListTitleBackground.lock())
        {
            Background->SetTexture(
                "AlmanacPoliticsListTitleBackgroundTexture",
                GBarBackTexture);
            Background->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mPoliticsListTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_PoliticsListTitle", 2);
        if (auto Title = Widget.mPoliticsListTitle.lock())
        {
            ConfigureSectionText(Title);
            Title->SetText(
                UIStrings::Get(L"almanac.politics.title.factions").c_str());
        }

        for (int Index = 0; Index < GPoliticsFactionTileCount; ++Index)
        {
            Widget.mPoliticsFactionTiles.push_back(CreatePoliticsFactionTile(
                Widget,
                Widget.mWorld,
                Page,
                "Almanac_PoliticsFactionTile_" + std::to_string(Index + 1),
                Index));
        }

        for (int Index = 0; Index < GPoliticsNeutralCount; ++Index)
        {
            auto Text = Page->CreateWidget<CTextBlock>(
                "Almanac_PoliticsNeutralText_" + std::to_string(Index + 1), 2);
            if (auto Neutral = Text.lock())
            {
                ConfigureAuxiliaryText(Neutral);
                Neutral->SetFontSize(14.f);
                Neutral->SetAlignH(ETextAlignH::Center);
            }
            Widget.mPoliticsNeutralTexts.push_back(Text);
        }

        Widget.mPoliticsSupportTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_PoliticsSupportTitleBackground", 1);
        if (auto Background = Widget.mPoliticsSupportTitleBackground.lock())
        {
            Background->SetTexture(
                "AlmanacPoliticsSupportTitleBackgroundTexture",
                GBarBackTexture);
            Background->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mPoliticsSupportTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_PoliticsSupportTitle", 2);
        if (auto Title = Widget.mPoliticsSupportTitle.lock())
        {
            ConfigureSectionText(Title);
            Title->SetText(
                UIStrings::Get(L"almanac.politics.title.support").c_str());
        }

        for (int Index = 0; Index < GPoliticsSupportRowCount; ++Index)
        {
            Widget.mPoliticsSupportRows.push_back(CreateDetailRow(
                Page,
                "Almanac_PoliticsSupportRow_" + std::to_string(Index + 1)));
        }

        Widget.mPoliticsElectionLeftArrow = Page->CreateWidget<CImage>(
            "Almanac_PoliticsElectionLeftArrow", 2);
        if (auto Arrow = Widget.mPoliticsElectionLeftArrow.lock())
        {
            Arrow->SetTexture(
                "AlmanacPoliticsElectionLeftArrowTexture",
                GDropdownArrowTexture);
            Arrow->SetTint(0.30f, 0.70f, 0.88f, 0.96f);
            Arrow->SetPivot(0.5f, 0.5f);
            Arrow->SetAngle(90.f);
        }

        Widget.mPoliticsElectionRightArrow = Page->CreateWidget<CImage>(
            "Almanac_PoliticsElectionRightArrow", 2);
        if (auto Arrow = Widget.mPoliticsElectionRightArrow.lock())
        {
            Arrow->SetTexture(
                "AlmanacPoliticsElectionRightArrowTexture",
                GDropdownArrowTexture);
            Arrow->SetTint(0.30f, 0.70f, 0.88f, 0.96f);
            Arrow->SetPivot(0.5f, 0.5f);
            Arrow->SetAngle(-90.f);
        }

        Widget.mPoliticsElectionText = Page->CreateWidget<CTextBlock>(
            "Almanac_PoliticsElectionText", 2);
        if (auto Text = Widget.mPoliticsElectionText.lock())
        {
            ConfigureAuxiliaryText(Text);
            Text->SetFontSize(16.f);
        }

        Widget.mPoliticsFactionTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_PoliticsFactionTitle", 2);
        if (auto Text = Widget.mPoliticsFactionTitle.lock())
        {
            ConfigureSectionText(Text);
            Text->SetFontSize(24.f);
        }

        Widget.mPoliticsFactionApprovalLabel = Page->CreateWidget<CTextBlock>(
            "Almanac_PoliticsFactionApprovalLabel", 2);
        if (auto Text = Widget.mPoliticsFactionApprovalLabel.lock())
        {
            ConfigureBodyLabelText(Text);
            Text->SetFontSize(16.f);
        }

        Widget.mPoliticsFactionApprovalValue = Page->CreateWidget<CTextBlock>(
            "Almanac_PoliticsFactionApprovalValue", 2);
        if (auto Text = Widget.mPoliticsFactionApprovalValue.lock())
        {
            ConfigureBodyValueText(Text);
            Text->SetFontSize(20.f);
        }

        for (int Index = 0; Index < GPoliticsDetailCount; ++Index)
        {
            Widget.mPoliticsDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_PoliticsDetail_" + std::to_string(Index + 1)));
        }
}

void FAlmanacRenderer::CreateForeignWidgets(CAlmanacWidget& Widget)
{
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Foreign)].lock();

        for (int Index = 0; Index < GForeignPowerCount; ++Index)
        {
            Widget.mForeignRows.push_back(CreateForeignPowerRow(
                Widget,
                Widget.mWorld,
                Page,
                "Almanac_ForeignRow_" + std::to_string(Index + 1),
                Index));
        }

        Widget.mForeignTitleBackground = Page->CreateWidget<CImage>(
            "Almanac_ForeignTitleBackground", 1);
        if (auto Background = Widget.mForeignTitleBackground.lock())
        {
            Background->SetTexture(
                "AlmanacForeignTitleBackgroundTexture",
                GBarBackTexture);
            Background->SetTint(0.96f, 0.90f, 0.74f, 0.42f);
        }

        Widget.mForeignTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_ForeignTitle", 2);
        ConfigureSectionText(Widget.mForeignTitle.lock());

        Widget.mForeignStatusLabel = Page->CreateWidget<CTextBlock>(
            "Almanac_ForeignStatusLabel", 2);
        if (auto Text = Widget.mForeignStatusLabel.lock())
        {
            ConfigureBodyLabelText(Text);
        }

        Widget.mForeignStatusValue = Page->CreateWidget<CTextBlock>(
            "Almanac_ForeignStatusValue", 2);
        if (auto Text = Widget.mForeignStatusValue.lock())
        {
            ConfigureBodyValueText(Text);
        }

        for (int Index = 0; Index < GForeignDetailCount; ++Index)
        {
            Widget.mForeignDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_ForeignDetail_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GForeignMetricCount; ++Index)
        {
            Widget.mForeignMetrics.push_back(CreateMetricRow(
                Page,
                "Almanac_ForeignMetric_" + std::to_string(Index + 1)));
        }

        Widget.mForeignNotice = Page->CreateWidget<CTextBlock>(
            "Almanac_ForeignNotice", 2);
        ConfigureNoticeText(Widget.mForeignNotice.lock());
}

void FAlmanacRenderer::CreateBuildingWidgets(CAlmanacWidget& Widget)
{
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Buildings)].lock();

        for (int Index = 0; Index < GBuildingRowCount; ++Index)
        {
            Widget.mBuildingRows.push_back(CreateSelectableBuildingDetailRow(
                Widget,
                Widget.mWorld,
                Page,
                "Almanac_BuildingRow_" + std::to_string(Index + 1),
                Index));
        }

        Widget.mBuildingCategoryTitle = Page->CreateWidget<CTextBlock>(
            "Almanac_BuildingCategoryTitle", 2);
        ConfigureSectionText(Widget.mBuildingCategoryTitle.lock());

        for (int Index = 0; Index < GBuildingDetailCount; ++Index)
        {
            Widget.mBuildingDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_BuildingDetail_" + std::to_string(Index + 1)));
        }
}

void FAlmanacRenderer::CreateConflictWidgets(CAlmanacWidget& Widget)
{
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Conflict)].lock();

        Widget.mConflictHeadlineBackground = Page->CreateWidget<CImage>(
            "Almanac_ConflictHeadlineBackground", 1);
        Widget.mConflictHeadlineText = Page->CreateWidget<CTextBlock>(
            "Almanac_ConflictHeadlineText", 2);

        ConfigureRowBackground(
            Widget.mConflictHeadlineBackground.lock(),
            "Almanac_ConflictHeadlineTexture");
        ConfigureSectionText(Widget.mConflictHeadlineText.lock());
        if (auto HeadlineText = Widget.mConflictHeadlineText.lock())
            HeadlineText->SetAlignV(ETextAlignV::Top);

        for (int Index = 0; Index < GConflictDetailCount; ++Index)
        {
            Widget.mConflictDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_ConflictDetail_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GConflictMetricCount; ++Index)
        {
            Widget.mConflictMetrics.push_back(CreateMetricRow(
                Page,
                "Almanac_ConflictMetric_" + std::to_string(Index + 1)));
        }
}

