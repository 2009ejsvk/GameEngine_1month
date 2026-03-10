#include "AlmanacRenderer.h"
#include "AlmanacRendererInternal.h"
#include "World/World.h"
#include <string>

void FAlmanacRenderer::CreateWidgets(CAlmanacWidget& Widget)
{
    auto PanelBackground = Widget.CreateWidget<CImage>("Almanac_Background", 6).lock();

    if (PanelBackground)
    {
        PanelBackground->SetTexture("AlmanacPanelBackground", GPanelTexture);
        PanelBackground->SetTint(1.f, 1.f, 1.f, 1.f);
        Widget.mPanelBackground = PanelBackground;
    }

    auto TitleText = Widget.CreateWidget<CTextBlock>("Almanac_Title", 8).lock();

    if (TitleText)
    {
        TitleText->SetText(GPageTitles[static_cast<size_t>(Widget.mSelectedPage)]);
        ConfigureTitleText(TitleText);
        Widget.mTitleText = TitleText;
    }

    auto CloseButton = Widget.CreateWidget<CButton>("Almanac_Close", 9).lock();

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
            CloseText->SetFontSize(18.f);
            CloseText->SetAlignH(ETextAlignH::Center);
            CloseText->SetAlignV(ETextAlignV::Middle);
            CloseText->SetTextColor(72, 48, 12, 255);
            CloseButton->SetChild(CloseText);
        }

        Widget.mCloseButton = CloseButton;
    }

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
            "Almanac_Tab_" + std::to_string(Index + 1), 9).lock();

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

    for (size_t Index = 0; Index < Widget.mPages.size(); ++Index)
    {
        auto Page = Widget.CreateWidget<CWidgetContainer>(
            "Almanac_Page_" + std::to_string(Index), 7).lock();

        if (Page)
            Widget.mPages[Index] = Page;
    }

    auto CreateCard = [](
        const std::shared_ptr<CWidgetContainer>& Page,
        const std::string& Prefix,
        const TCHAR* IconPath) -> CAlmanacWidget::FCardWidgets
    {
        CAlmanacWidget::FCardWidgets Card;

        if (!Page)
            return Card;

        Card.Background = Page->CreateWidget<CImage>(
            Prefix + "_Background", 1);

        if (auto Background = Card.Background.lock())
            ConfigureRowBackground(Background, Prefix + "_Texture");

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
    };

    auto CreateMetricRow = [](
        const std::shared_ptr<CWidgetContainer>& Page,
        const std::string& Prefix) -> CAlmanacWidget::FMetricRowWidgets
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
    };

    auto CreateDetailRow = [](
        const std::shared_ptr<CWidgetContainer>& Page,
        const std::string& Prefix) -> CAlmanacWidget::FDetailRowWidgets
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
    };

    {
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Overview)].lock();
        const TCHAR* CardIcons[GOverviewCardCount] =
        {
            GPopulationIcon,
            TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_housing.png"),
            TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_industry.png"),
            GApprovalIcon,
            TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Constitution.png"),
            GMoneyIcon
        };

        Widget.mOverviewCards.reserve(GOverviewCardCount);

        for (int Index = 0; Index < GOverviewCardCount; ++Index)
        {
            Widget.mOverviewCards.push_back(CreateCard(
                Page,
                "Almanac_OverviewCard_" + std::to_string(Index + 1),
                CardIcons[Index]));
        }

        Widget.mOverviewSummaryLeft = Page->CreateWidget<CTextBlock>(
            "Almanac_OverviewSummaryLeft", 2);
        Widget.mOverviewSummaryRight = Page->CreateWidget<CTextBlock>(
            "Almanac_OverviewSummaryRight", 2);

        ConfigureNoticeText(Widget.mOverviewSummaryLeft.lock());
        ConfigureNoticeText(Widget.mOverviewSummaryRight.lock());
    }

    {
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Satisfaction)].lock();

        for (int Index = 0; Index < GSatisfactionRowCount; ++Index)
        {
            Widget.mSatisfactionRows.push_back(CreateMetricRow(
                Page,
                "Almanac_SatisfactionRow_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GSatisfactionDetailCount; ++Index)
        {
            Widget.mSatisfactionDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_SatisfactionDetail_" + std::to_string(Index + 1)));
        }
    }

    {
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Population)].lock();

        for (int Index = 0; Index < GPopulationDetailCount; ++Index)
        {
            Widget.mPopulationDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_PopulationDetail_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GPopulationMetricCount; ++Index)
        {
            Widget.mPopulationMetrics.push_back(CreateMetricRow(
                Page,
                "Almanac_PopulationMetric_" + std::to_string(Index + 1)));
        }
    }

    {
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Economy)].lock();

        for (int Index = 0; Index < GEconomyDetailCount; ++Index)
        {
            Widget.mEconomyDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_EconomyDetail_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GEconomyMetricCount; ++Index)
        {
            Widget.mEconomyMetrics.push_back(CreateMetricRow(
                Page,
                "Almanac_EconomyMetric_" + std::to_string(Index + 1)));
        }
    }

    {
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Resources)].lock();

        for (int Index = 0; Index < GResourceRowCount; ++Index)
        {
            Widget.mResourceRows.push_back(CreateMetricRow(
                Page,
                "Almanac_ResourceRow_" + std::to_string(Index + 1)));
        }

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

    {
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Politics)].lock();

        for (int Index = 0; Index < GPoliticsRowCount; ++Index)
        {
            Widget.mPoliticsRows.push_back(CreateMetricRow(
                Page,
                "Almanac_PoliticsRow_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GPoliticsDetailCount; ++Index)
        {
            Widget.mPoliticsDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_PoliticsDetail_" + std::to_string(Index + 1)));
        }
    }

    {
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Foreign)].lock();

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

    {
        auto Page = Widget.mPages[static_cast<size_t>(EAlmanacPage::Buildings)].lock();

        for (int Index = 0; Index < GBuildingRowCount; ++Index)
        {
            Widget.mBuildingRows.push_back(CreateMetricRow(
                Page,
                "Almanac_BuildingRow_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GBuildingDetailCount; ++Index)
        {
            Widget.mBuildingDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_BuildingDetail_" + std::to_string(Index + 1)));
        }
    }

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

    ApplySelectedPage(Widget);
    ApplyOpenState(Widget);
}


