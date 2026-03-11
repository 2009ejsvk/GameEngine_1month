#include "CitizenInfoRenderer.h"
#include "CitizenInfoRendererInternal.h"
#include "CitizenInfoWidget.h"
#include "UILayoutConfig.h"
#include "Device.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include <algorithm>
#include <array>

using namespace CitizenInfoRendererInternal;

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
        ScreenHeight - Widget.mPanelTop - UIConfig::CitizenPanelBottomMargin);

    const float PanelLeft   = ScreenWidth - Widget.mPanelWidth - 10.f;
    Widget.SetPos(PanelLeft, 0.f);
    Widget.SetSize(Widget.mPanelWidth, Widget.mPanelTop + Widget.mPanelHeight);

    const float OuterTop    = Widget.mPanelTop;
    const float PanelWidth  = Widget.mPanelWidth;
    const float PanelHeight = Widget.mPanelHeight;
    struct FCitizenLayoutMetrics
    {
        float PanelInnerTopOffset;
        float PanelInnerBottomInset;
        float ScrollBottomInset;
        float ScrollThumbTopOffset;
        float CloseButtonOffsetY;
        float TitleIconInsetX;
        float TitleTextInsetX;
        float TitleIconGap;
        float SubtitleOffsetY;
        float SectionRibbonOffsetY;
        float CollapsedSectionGap;
        float BudgetBaseOffsetY;
        float BudgetLabelOffsetY;
        float BudgetCustomButtonsOffsetY;
        float BudgetWorkButtonsOffsetY;
        float BudgetDefaultButtonsOffsetY;
        float BudgetCompactGap;
        float BudgetDefaultGap;
        float OccupancyGapY;
        float ActionCompactIconSize;
        float ActionCompactIconOffsetY;
        float MoveCompactRightOffset;
        float CloneCompactRightOffset;
        float OverviewCommandGap;
        float CitizenActionButtonHeight;
        float CitizenActionGap;
        float ActionStackTopOffset;
        float ActionIconInset;
        float ActionIconSize;
        float FooterBottomInset;
        float BodyGapAfterSection;
        float BodyGapAfterActions;
        float BodyGapBeforeActions;
        float BodyFallbackOffset;
        float BodyBottomInset;
    } Layout =
    {
        UIConfig::CitizenPanelInnerTopOffset,
        UIConfig::CitizenPanelInnerBottomInset,
        UIConfig::CitizenScrollBottomInset,
        UIConfig::CitizenScrollThumbTopOffset,
        UIConfig::CitizenCloseButtonOffsetY,
        6.f,
        14.f,
        6.f,
        4.f,
        UIConfig::CitizenSectionRibbonOffsetY,
        6.f,
        UIConfig::CitizenBudgetBaseOffsetY,
        2.f,
        20.f,
        78.f,
        26.f,
        6.f,
        8.f,
        18.f,
        34.f,
        2.f,
        82.f,
        40.f,
        10.f,
        38.f,
        4.f,
        UIConfig::CitizenActionStackTopOffset,
        8.f,
        22.f,
        UIConfig::CitizenFooterBottomInset,
        8.f,
        14.f,
        12.f,
        4.f,
        UIConfig::CitizenBodyBottomInset
    };

    const float InnerMarginX   = UIConfig::CitizenPanelInnerMarginX;
    const float InnerLeft      = InnerMarginX;
    const float InnerTop       = OuterTop + Layout.PanelInnerTopOffset;
    const float InnerWidth     = PanelWidth - InnerMarginX * 2.f;
    const float InnerHeight    = PanelHeight - Layout.PanelInnerBottomInset;

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
        ScrollTrack->SetSize(
            ScrollTrackW,
            PanelHeight - TitleRibbonH - SectionRibbonH - Layout.ScrollBottomInset);
    }
    if (auto ScrollThumb = Widget.mScrollThumb.lock())
    {
        ScrollThumb->SetPos(InnerMarginX * 0.5f - ScrollTrackW * 0.5f + 0.5f,
            OuterTop + TitleRibbonH + SectionRibbonH + Layout.ScrollThumbTopOffset);
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
        CloseButton->SetPos(
            PanelWidth - CloseOffsetX,
            OuterTop + Layout.CloseButtonOffsetY);
        CloseButton->SetSize(CloseButtonSz, CloseButtonSz);
    }

    // 아이콘 & 제목 텍스트
    const bool TitleIconVisible =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Building &&
        !Widget.mTitleIcon.expired() &&
        Widget.mTitleIcon.lock()->GetEnable();
    const float TitleLeft = TitleIconVisible ?
        (RibbonOffsetX + IconSz + Layout.TitleIconGap) :
        (RibbonOffsetX + Layout.TitleTextInsetX);

    if (auto TitleIcon = Widget.mTitleIcon.lock())
    {
        TitleIcon->SetPos(
            RibbonOffsetX + Layout.TitleIconInsetX,
            OuterTop + RibbonOffsetY + (TitleRibbonH - IconSz) * 0.5f);
        TitleIcon->SetSize(IconSz, IconSz);
    }
    if (auto TitleText = Widget.mTitleText.lock())
    {
        TitleText->SetPos(TitleLeft, OuterTop + RibbonOffsetY);
        TitleText->SetSize(PanelWidth - TitleLeft - CloseButtonSz - CloseOffsetX, TitleRibbonH);
    }
    if (auto SubtitleText = Widget.mSubtitleText.lock())
    {
        SubtitleText->SetPos(
            RibbonOffsetX,
            OuterTop + RibbonOffsetY + TitleRibbonH + Layout.SubtitleOffsetY);
        SubtitleText->SetSize(PanelWidth - RibbonOffsetX * 2.f, 22.f);
    }

    // 섹션 리본 (페이지 전환 시)
    const bool ShowSectionRibbon =
        !Widget.mSectionRibbon.expired() &&
        Widget.mSectionRibbon.lock()->GetEnable();
    const bool ShowCitizenProfile =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedCitizenTab ==
            CCitizenInfoWidget::ECitizenInfoTab::Overview &&
        !Widget.mCitizenFooterText.expired() &&
        Widget.mCitizenFooterText.lock()->GetEnable();
    const bool ShowCitizenPolitics =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedCitizenTab ==
            CCitizenInfoWidget::ECitizenInfoTab::Politics &&
        !Widget.mCitizenPoliticsSectionTitles[0].expired() &&
        Widget.mCitizenPoliticsSectionTitles[0].lock()->GetEnable();
    const bool ShowCitizenThoughts =
        Widget.mPanelMode == CCitizenInfoWidget::EPanelMode::Citizen &&
        Widget.mSelectedCitizenTab ==
            CCitizenInfoWidget::ECitizenInfoTab::Thoughts &&
        !Widget.mCitizenThoughtTitleText.expired() &&
        Widget.mCitizenThoughtTitleText.lock()->GetEnable();
    const float SectionRibbonY =
        OuterTop + RibbonOffsetY + TitleRibbonH + Layout.SectionRibbonOffsetY;
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
            OuterTop + TitleRibbonH +
            (ShowSectionRibbon ? SectionRibbonH : Layout.CollapsedSectionGap);
        ScrollTrack->SetPos(
            InnerMarginX * 0.5f - ScrollTrackW * 0.5f,
            ScrollTop);
        ScrollTrack->SetSize(
            ScrollTrackW,
            PanelHeight - (ScrollTop - OuterTop) - Layout.ScrollBottomInset);
    }
    if (auto ScrollThumb = Widget.mScrollThumb.lock())
    {
        const float ScrollTop =
            OuterTop + TitleRibbonH +
            (ShowSectionRibbon ? SectionRibbonH : Layout.CollapsedSectionGap);
        ScrollThumb->SetPos(
            InnerMarginX * 0.5f - ScrollTrackW * 0.5f + 0.5f,
            ScrollTop + Layout.ScrollThumbTopOffset);
        ScrollThumb->SetSize(ScrollTrackW - 1.f, ScrollThumbH);
    }

    // 예산 컨트롤
    const float BudgetBaseY =
        OuterTop + RibbonOffsetY + TitleRibbonH + Layout.BudgetBaseOffsetY;
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
        (BudgetBaseY + Layout.BudgetCustomButtonsOffsetY) :
        (ShowWorkOverview ? (BudgetBaseY + Layout.BudgetWorkButtonsOffsetY) :
        (BudgetBaseY + Layout.BudgetDefaultButtonsOffsetY));
    const float WorkModeTop = BudgetBaseY - Layout.BudgetLabelOffsetY;
    const float WorkModeBoxTop = WorkModeTop + 22.f;
    const float BudgetMargin    = RibbonOffsetX;
    const float BudgetGap       = ShowAnyOverview ?
        Layout.BudgetCompactGap :
        Layout.BudgetDefaultGap;
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
            Text->SetPos(BudgetMargin, BudgetBaseY - Layout.BudgetLabelOffsetY);
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
            Text->SetPos(
                PanelWidth - BudgetMargin - 120.f,
                BudgetBaseY - Layout.BudgetLabelOffsetY);
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

    const float OccupancyTop =
        BudgetButtonTop + BudgetButtonH + Layout.OccupancyGapY;
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
                PanelWidth - BudgetMargin - Layout.MoveCompactRightOffset,
                ActionTop + Layout.ActionCompactIconOffsetY);
            MoveButton->SetSize(
                Layout.ActionCompactIconSize,
                Layout.ActionCompactIconSize);
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
                PanelWidth - BudgetMargin - Layout.CloneCompactRightOffset,
                ActionTop + Layout.ActionCompactIconOffsetY);
            CloneButton->SetSize(
                Layout.ActionCompactIconSize,
                Layout.ActionCompactIconSize);
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
            Button->SetPos(
                BudgetMargin,
                ActionTop - ActionBtnH - Layout.OverviewCommandGap);
            Button->SetSize(PanelWidth - BudgetMargin * 2.f, ActionBtnH);
        }
        else
        {
            Button->SetPos(0.f, 0.f);
            Button->SetSize(0.f, 0.f);
        }
    }

    const float CitizenActionBtnH = Layout.CitizenActionButtonHeight;
    const float CitizenActionGap = Layout.CitizenActionGap;
    const float CitizenActionTop = ActionTop - Layout.ActionStackTopOffset;

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
                Icon->SetPos(
                    BudgetMargin + Layout.ActionIconInset,
                    ButtonTop + Layout.ActionIconInset);
                Icon->SetSize(
                    Layout.ActionIconSize,
                    Layout.ActionIconSize);
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
            Text->SetPos(
                BudgetMargin,
                OuterTop + PanelHeight - Layout.FooterBottomInset);
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
        ShowSectionRibbon ? (SectionRibbonY + SectionRibbonH + Layout.BodyGapAfterSection) :
        (ShowCitizenProfile || ShowCitizenPolitics || ShowCitizenThoughts ?
            (OuterTop + PanelHeight) :
            (ShowAnyOverview ? (OuterTop + PanelHeight) :
            (ShowActions ? (BudgetButtonTop + BudgetButtonH + Layout.BodyGapAfterActions) :
                           (BudgetBaseY - Layout.BodyFallbackOffset))));
    const float BodyBottom =
        ShowActions ? (ActionTop - Layout.BodyGapBeforeActions) :
        (OuterTop + PanelHeight - Layout.BodyBottomInset);

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
