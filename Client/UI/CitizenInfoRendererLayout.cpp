#include "CitizenInfoRenderer.h"
#include "CitizenInfoRendererInternal.h"
#include "CitizenInfoWidget.h"
#include "CitizenInfoLayoutMetrics.h"
#include "UILayoutValues.h"
#include "Device.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>

using namespace CitizenInfoRendererInternal;

void FCitizenInfoRenderer::RefreshCommonLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const auto& Layout = Context.Metrics;
    const float OuterTop = Context.Panel.OuterTop;
    const float PanelWidth = Context.Panel.Width;
    const float PanelHeight = Context.Panel.Height;
    const float InnerMarginX = Context.Inner.MarginX;
    const float InnerLeft = Context.Inner.Left;
    const float InnerTop = Context.Inner.Top;
    const float InnerWidth = Context.Inner.Width;
    const float InnerHeight = Context.Inner.Height;
    const float ScrollTrackW = Context.Chrome.ScrollTrackWidth;
    const float ScrollThumbH = Context.Chrome.ScrollThumbHeight;
    const float TitleRibbonH = Context.Ribbon.TitleHeight;
    const float SectionRibbonH = Context.Ribbon.SectionHeight;
    const float CloseButtonSz = Context.Chrome.CloseButtonSize;
    const float IconSz = Context.Chrome.IconSize;
    const bool ShowSectionRibbon = Context.Flags.ShowSectionRibbon;
    const float SectionRibbonY = Context.Ribbon.SectionY;
    const bool ShowWorkOverview = Context.Flags.ShowWorkOverview;
    const bool ShowCustomOverview = Context.Flags.ShowCustomOverview;
    const bool ShowResidentialWorkMode = Context.Flags.ShowResidentialWorkMode;
    const bool ShowAnyOverview = Context.Flags.ShowAnyOverview;
    const bool ShowOperationModeSelectionPage =
        Context.Flags.ShowOperationModeSelectionPage;
    const float TitleIconOffsetY = Context.Flags.IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::TitleIconOffsetY :
        UIConfig::BuildingTitleIconOffsetY;
    const float TitleTextOffsetX = Context.Flags.IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::TitleTextOffsetX :
        UIConfig::BuildingTitleTextOffsetX;
    const float TitleTextOffsetY = Context.Flags.IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::TitleTextOffsetY :
        UIConfig::BuildingTitleTextOffsetY;
    const float TitleTextWidthAdjust = Context.Flags.IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::TitleTextWidthAdjust :
        UIConfig::BuildingTitleTextWidthAdjust;
    const float TitleTextHeightAdjust = Context.Flags.IsCitizenMode ?
        UIConfig::CitizenInfoLayoutDefaults::TitleTextHeightAdjust :
        UIConfig::BuildingTitleTextHeightAdjust;
    // 폰트 크기
    if (auto TitleTxt = Widget.mTitleText.lock())
        TitleTxt->SetFontSize(
            Context.Flags.IsCitizenMode ?
                UIConfig::CitizenTitleFontSize :
                UIConfig::BuildingTitleFontSize);
    if (Context.Flags.IsCitizenMode)
    {
        for (auto& SubtitleWeak : Widget.mCitizenSubtitleTexts)
            if (auto SubTxt = SubtitleWeak.lock())
                SubTxt->SetFontSize(UIConfig::CitizenSubtitleFontSize);
    }
    else
    {
        for (auto& SubtitleWeak : Widget.mSubtitleTexts)
            if (auto SubTxt = SubtitleWeak.lock())
                SubTxt->SetFontSize(UIConfig::BuildingSubtitleFontSize);
    }
    if (auto BodyTxt = Widget.mBodyText.lock())
        BodyTxt->SetFontSize(
            Context.Flags.IsCitizenMode ?
                UIConfig::CitizenBodyFontSize :
                UIConfig::BuildingBodyFontSize);
    if (auto PageTitleTxt = Widget.mPageTitleText.lock())
        PageTitleTxt->SetFontSize(
            Context.Flags.IsCitizenMode ?
                UIConfig::CitizenPageTitleFontSize :
                UIConfig::BuildingPageTitleFontSize);
    if (auto BudgetTxt = Widget.mBudgetText.lock())
        BudgetTxt->SetFontSize(
            Context.Flags.IsCitizenMode ?
                UIConfig::CitizenBodyFontSize :
                UIConfig::BuildingBudgetTextFontSize);
    if (auto Text = Widget.mOverviewWorkModeLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewWorkModeLabelFontSize);
    if (auto Text = Widget.mOverviewWorkModeText.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewWorkModeValueFontSize);
    if (auto Text = Widget.mResidentialOverviewWorkModeLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewWorkModeLabelFontSize);
    if (auto Text = Widget.mResidentialOverviewWorkModeText.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewWorkModeValueFontSize);
    if (auto Text = Widget.mOverviewBudgetLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewBudgetLabelFontSize);
    if (auto Text = Widget.mOverviewBudgetValue.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewBudgetValueFontSize);
    if (auto Text = Widget.mOverviewOccupancyLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewOccupancyLabelFontSize);
    if (auto Text = Widget.mOverviewOccupancyValue.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewOccupancyValueFontSize);
    if (auto Text = Widget.mResidentialOverviewBudgetLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewBudgetLabelFontSize);
    if (auto Text = Widget.mResidentialOverviewBudgetValue.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewBudgetValueFontSize);
    if (auto Text = Widget.mResidentialOverviewOccupancyLabel.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewOccupancyLabelFontSize);
    if (auto Text = Widget.mResidentialOverviewOccupancyValue.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewOccupancyValueFontSize);
    if (auto Text = Widget.mUpgradeCardTitle.lock())
        Text->SetFontSize(UIConfig::BuildingUpgradeTitleFontSize);
    if (auto Text = Widget.mUpgradeDescriptionText.lock())
        Text->SetFontSize(UIConfig::BuildingUpgradeDescriptionFontSize);
    if (auto Text = Widget.mInformationAccentText.lock())
        Text->SetFontSize(UIConfig::BuildingInformationAccentFontSize);
    if (auto Text = Widget.mInformationTopText.lock())
        Text->SetFontSize(UIConfig::BuildingInformationBodyFontSize);
    if (auto Text = Widget.mInformationBottomText.lock())
        Text->SetFontSize(UIConfig::BuildingInformationBodyFontSize);
    if (auto Text = Widget.mOverviewCommandButtonText.lock())
        Text->SetFontSize(UIConfig::BuildingOverviewCommandButtonFontSize);

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        if (auto Text =
            Widget.mBudgetButtonTexts[static_cast<size_t>(Index)].lock())
        {
            Text->SetFontSize(UIConfig::BuildingBudgetButtonFontSize);
        }
    }

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
        UIConfig::BuildingMinimumTabStartInset,
        (PanelWidth - TotalTabsWidth) * 0.5f);

    for (int Index = 0; Index < GBuildingTabCount; ++Index)
    {
        auto Button = Widget.mTabButtons[static_cast<size_t>(Index)].lock();
        if (!Button)
            continue;

        Button->SetPos(
            TabStartX + static_cast<float>(Index) * (TabWidth + TabGap), 0.f);
        Button->SetSize(TabWidth, TabHeight);

        if (auto Label = Widget.mTabButtonTexts[static_cast<size_t>(Index)].lock())
        {
            Label->SetFontSize(UIConfig::BuildingTabLabelFontSize);
            Label->SetPos(
                UIConfig::BuildingTabLabelOffsetX,
                UIConfig::BuildingTabLabelOffsetY);
            Label->SetSize(TabWidth, TabHeight);
        }
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
            OuterTop + RibbonOffsetY + (TitleRibbonH - IconSz) * 0.5f +
                TitleIconOffsetY);
        TitleIcon->SetSize(IconSz, IconSz);
    }
    if (auto TitleText = Widget.mTitleText.lock())
    {
        TitleText->SetPos(
            TitleLeft + TitleTextOffsetX,
            OuterTop + RibbonOffsetY + TitleTextOffsetY);
        TitleText->SetSize(
            PanelWidth - TitleLeft - CloseButtonSz - CloseOffsetX +
                TitleTextWidthAdjust,
            TitleRibbonH + TitleTextHeightAdjust);
    }
    {
        auto SetSubtitleLayout = [&](CCitizenInfoWidget::WText& SubtitleWeak)
        {
            if (auto SubtitleText = SubtitleWeak.lock())
            {
                SubtitleText->SetPos(
                    RibbonOffsetX,
                    OuterTop + RibbonOffsetY + TitleRibbonH + Layout.SubtitleOffsetY);
                SubtitleText->SetSize(
                    PanelWidth - RibbonOffsetX * 2.f,
                    Layout.CompactControlHeight);
            }
        };
        if (Context.Flags.IsCitizenMode)
            for (auto& W : Widget.mCitizenSubtitleTexts) SetSubtitleLayout(W);
        else
            for (auto& W : Widget.mSubtitleTexts) SetSubtitleLayout(W);
    }


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
            BudgetText->SetSize(
                PanelWidth - RibbonOffsetX * 2.f,
                Layout.CompactControlHeight);
        }
    }

    const float BudgetButtonH   = UIConfig::BuildingBudgetButtonHeight;
    const float BudgetButtonTop = BudgetBaseY + Layout.BudgetWorkButtonsOffsetY;
    const float WorkModeTop = BudgetBaseY - Layout.BudgetLabelOffsetY;
    const float WorkModeBoxTop = WorkModeTop + Layout.CompactControlHeight;
    const float BudgetMargin    = RibbonOffsetX;
    const float BudgetGap       = ShowAnyOverview ?
        Layout.BudgetCompactGap :
        Layout.BudgetDefaultGap;
    const float BudgetButtonW   = ShowAnyOverview ?
        Layout.CompactBudgetButtonWidth :
        (PanelWidth - BudgetMargin * 2.f - BudgetGap * 4.f) / 5.f;

    if (auto Text = Widget.mOverviewWorkModeLabel.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(
                BudgetMargin,
                WorkModeTop);
            Text->SetSize(
                PanelWidth * 0.5f,
                Layout.CompactControlHeight);
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
            Background->SetPos(
                BudgetMargin,
                WorkModeBoxTop);
            Background->SetSize(
                PanelWidth - BudgetMargin * 2.f,
                34.f);
        }
        else
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }
    }

    if (auto Button = Widget.mOverviewWorkModeButton.lock())
    {
        if (ShowWorkOverview)
        {
            Button->SetPos(
                BudgetMargin,
                WorkModeBoxTop);
            Button->SetSize(
                PanelWidth - BudgetMargin * 2.f,
                34.f);
        }
        else
        {
            Button->SetPos(0.f, 0.f);
            Button->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewWorkModeText.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(
                BudgetMargin + 12.f,
                WorkModeBoxTop + 2.f);
            Text->SetSize(
                PanelWidth - BudgetMargin * 2.f - 24.f,
                30.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewWorkModeLabel.lock())
    {
        if (ShowResidentialWorkMode)
        {
            Text->SetPos(
                BudgetMargin,
                WorkModeTop);
            Text->SetSize(
                PanelWidth * 0.5f,
                Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Background = Widget.mResidentialOverviewWorkModeBackground.lock())
    {
        if (ShowCustomOverview)
        {
            Background->SetPos(
                BudgetMargin,
                WorkModeBoxTop);
            Background->SetSize(
                PanelWidth - BudgetMargin * 2.f,
                34.f);
        }
        else
        {
            Background->SetPos(0.f, 0.f);
            Background->SetSize(0.f, 0.f);
        }
    }

    if (auto Button = Widget.mResidentialOverviewWorkModeButton.lock())
    {
        if (ShowResidentialWorkMode)
        {
            Button->SetPos(
                BudgetMargin,
                WorkModeBoxTop);
            Button->SetSize(
                PanelWidth - BudgetMargin * 2.f,
                34.f);
        }
        else
        {
            Button->SetPos(0.f, 0.f);
            Button->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewWorkModeText.lock())
    {
        if (ShowResidentialWorkMode)
        {
            Text->SetPos(
                BudgetMargin + 12.f,
                WorkModeBoxTop + 2.f);
            Text->SetSize(
                PanelWidth - BudgetMargin * 2.f - 24.f,
                30.f);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewBudgetLabel.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(
                BudgetMargin,
                BudgetBaseY - Layout.BudgetLabelOffsetY);
            Text->SetSize(
                PanelWidth * 0.5f,
                Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewBudgetValue.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(
                PanelWidth - BudgetMargin - 120.f,
                BudgetBaseY - Layout.BudgetLabelOffsetY);
            Text->SetSize(
                120.f,
                Layout.CompactControlHeight);
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

        if (ShowOperationModeSelectionPage)
        {
            const float ModeH = UIConfig::BuildingModeButtonHeight;
            const float ModeGap = UIConfig::BuildingModeButtonGap;
            Button->SetPos(
                BudgetMargin,
                WorkModeBoxTop + static_cast<float>(Index) * (ModeH + ModeGap));
            Button->SetSize(PanelWidth - BudgetMargin * 2.f, ModeH);
        }
        else if (Index < GBudgetDisplayCount)
        {
            Button->SetPos(
                BudgetMargin + static_cast<float>(Index) * (BudgetButtonW + BudgetGap),
                BudgetButtonTop);
            Button->SetSize(BudgetButtonW, BudgetButtonH);
        }
        else
        {
            Button->SetPos(0.f, 0.f);
            Button->SetSize(0.f, 0.f);
        }
    }

    const float OccupancyTop =
        BudgetButtonTop + BudgetButtonH + Layout.OccupancyGapY;
    if (auto Text = Widget.mOverviewOccupancyLabel.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(BudgetMargin, OccupancyTop);
            Text->SetSize(PanelWidth * 0.5f, Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mOverviewOccupancyValue.lock())
    {
        if (ShowWorkOverview)
        {
            Text->SetPos(PanelWidth - BudgetMargin - 120.f, OccupancyTop);
            Text->SetSize(120.f, Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewOccupancyLabel.lock())
    {
        if (ShowCustomOverview)
        {
            Text->SetPos(BudgetMargin, OccupancyTop);
            Text->SetSize(PanelWidth * 0.5f, Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewOccupancyValue.lock())
    {
        if (ShowCustomOverview)
        {
            Text->SetPos(PanelWidth - BudgetMargin - 120.f, OccupancyTop);
            Text->SetSize(120.f, Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewBudgetLabel.lock())
    {
        if (ShowCustomOverview)
        {
            Text->SetPos(
                BudgetMargin,
                BudgetBaseY - Layout.BudgetLabelOffsetY);
            Text->SetSize(
                PanelWidth * 0.5f,
                Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

    if (auto Text = Widget.mResidentialOverviewBudgetValue.lock())
    {
        if (ShowCustomOverview)
        {
            Text->SetPos(
                PanelWidth - BudgetMargin - 120.f,
                BudgetBaseY - Layout.BudgetLabelOffsetY);
            Text->SetSize(
                120.f,
                Layout.CompactControlHeight);
        }
        else
        {
            Text->SetPos(0.f, 0.f);
            Text->SetSize(0.f, 0.f);
        }
    }

}

void FCitizenInfoRenderer::RefreshCitizenModeLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    if (!Context.Flags.IsCitizenMode)
    {
        return;
    }

    if (Context.Flags.ShowCitizenThoughts)
    {
        RefreshCitizenThoughtsLayout(Owner, Context);
        return;
    }

    if (Context.Flags.ShowCitizenPolitics)
    {
        RefreshCitizenPoliticsLayout(Owner, Context);
        return;
    }

    if (Context.Flags.ShowCitizenProfile)
    {
        RefreshCitizenProfileLayout(Owner, Context);
    }
}

void FCitizenInfoRenderer::RefreshCitizenThoughtsLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float OuterTop = Context.Panel.OuterTop;
    const float PanelWidth = Context.Panel.Width;
    const float RibbonOffsetY = Context.Ribbon.OffsetY;
    const float TitleRibbonH = Context.Ribbon.TitleHeight;
    const float BudgetMargin = Context.Budget.Margin;

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
            if (auto Icon =
                Widget.mOverviewMetricIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }

            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Bg =
                Widget.mOverviewMetricValueBgs[static_cast<size_t>(Index)].lock())
            {
                Bg->SetPos(0.f, 0.f);
                Bg->SetSize(0.f, 0.f);
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
            OuterTop + RibbonOffsetY + TitleRibbonH +
            UIConfig::CitizenThoughtsSectionTopOffset;
        const float SectionWidth = PanelWidth - BudgetMargin * 2.f;
        const float SectionHeight = UIConfig::CitizenThoughtsSectionHeight;

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

        const float ThoughtTop = SectionTop + SectionHeight +
            UIConfig::CitizenThoughtsTopGap;
        const float ThoughtWidth = SectionWidth -
            UIConfig::CitizenThoughtsWidthInset;
        const float ThoughtHeight = UIConfig::CitizenThoughtsEntryH;
        const float DividerWidth = Context.Metrics.SectionDividerWidth;
        const float DividerHeight = Context.Metrics.SectionDividerHeight;
        const float ThoughtStep = UIConfig::CitizenThoughtsStep;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenThoughtCount;
            ++Index)
        {
            if (auto Text =
                Widget.mCitizenThoughtTexts[static_cast<size_t>(Index)].lock())
            {
                Text->SetPos(
                    BudgetMargin + UIConfig::CitizenThoughtsTextOffsetX,
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
                ThoughtTop + UIConfig::CitizenThoughtsDividerOffsetY +
                    ThoughtStep * static_cast<float>(Index));
            Divider->SetSize(DividerWidth, DividerHeight);
            }
        }
}

void FCitizenInfoRenderer::RefreshCitizenPoliticsLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float OuterTop = Context.Panel.OuterTop;
    const float PanelWidth = Context.Panel.Width;
    const float RibbonOffsetY = Context.Ribbon.OffsetY;
    const float TitleRibbonH = Context.Ribbon.TitleHeight;
    const float BudgetMargin = Context.Budget.Margin;

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
            if (auto Icon =
                Widget.mOverviewMetricIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }

            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Bg =
                Widget.mOverviewMetricValueBgs[static_cast<size_t>(Index)].lock())
            {
                Bg->SetPos(0.f, 0.f);
                Bg->SetSize(0.f, 0.f);
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
        const float SectionHeight = UIConfig::CitizenPoliticsSectionHeight;
        const float SatisfactionTitleTop =
            OuterTop + RibbonOffsetY + TitleRibbonH +
            UIConfig::CitizenPoliticsSectionTopOffset;
        const float SatisfactionRowsTop = SatisfactionTitleTop + SectionHeight +
            UIConfig::CitizenPoliticsSatisfactionGap;
        const float SatisfactionRowH = UIConfig::CitizenPoliticsSatisfactionRowH;
        const float SatisfactionLabelW = UIConfig::CitizenPoliticsSatisfactionLabelW;
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
                    UIConfig::CitizenPoliticsOpinionSectionGap;
            else
                SectionTop = SatisfactionRowsTop +
                    SatisfactionRowH *
                    static_cast<float>(
                        CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount) +
                    UIConfig::CitizenPoliticsOpinionSectionGap +
                    SectionHeight +
                    UIConfig::CitizenPoliticsOpinionSectionGap +
                    UIConfig::CitizenPoliticsOpinionRowH *
                    static_cast<float>(
                        CCitizenInfoWidget::GCitizenPoliticsOpinionCount) +
                    UIConfig::CitizenPoliticsSupportGap;

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
                Label->SetSize(
                    SatisfactionLabelW - 8.f,
                    UIConfig::CitizenPoliticsBarLabelH);
            }

            if (auto Rail =
                Widget.mCitizenPoliticsSatisfactionRails[
                    static_cast<size_t>(Index)].lock())
            {
                Rail->SetPos(
                    SatisfactionRailLeft,
                    RowTop + UIConfig::CitizenPoliticsBarRailOffsetY);
                Rail->SetSize(SatisfactionRailWidth, UIConfig::CitizenPoliticsBarRailH);
            }

            if (auto Fill =
                Widget.mCitizenPoliticsSatisfactionFills[
                    static_cast<size_t>(Index)].lock())
            {
                Fill->SetPos(
                    SatisfactionRailLeft + 1.f,
                    RowTop + UIConfig::CitizenPoliticsBarFillOffsetY);
                Fill->SetSize(
                    (std::min)(SatisfactionRailWidth - 2.f, FillWidth),
                    UIConfig::CitizenPoliticsBarFillH);
            }
        }

        const float OpinionTitleTop =
            SatisfactionRowsTop +
            SatisfactionRowH *
                static_cast<float>(
                    CCitizenInfoWidget::GCitizenPoliticsSatisfactionCount) +
            UIConfig::CitizenPoliticsOpinionSectionGap;
        const float OpinionRowsTop = OpinionTitleTop + SectionHeight +
            UIConfig::CitizenPoliticsOpinionSectionGap;

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
                    OpinionRowsTop +
                        UIConfig::CitizenPoliticsOpinionRowH *
                        static_cast<float>(Index));
                Text->SetSize(
                    SectionWidth - 8.f,
                    UIConfig::CitizenPoliticsOpinionTextH);
            }
        }

        const float SupportTitleTop =
            OpinionRowsTop +
            UIConfig::CitizenPoliticsOpinionRowH *
                static_cast<float>(
                    CCitizenInfoWidget::GCitizenPoliticsOpinionCount) +
            UIConfig::CitizenPoliticsSupportGap;
        const float SupportIconsTop = SupportTitleTop + SectionHeight +
            UIConfig::CitizenPoliticsSupportIconsGap;
        const float SupportIconSpacing = SectionWidth / 3.f;
        const float SupportIconHalfSize =
            UIConfig::CitizenPoliticsSupportIconSize * 0.5f;

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
                    SupportIconHalfSize;
                Icon->SetPos(IconLeft, SupportIconsTop);
                Icon->SetSize(
                    UIConfig::CitizenPoliticsSupportIconSize,
                    UIConfig::CitizenPoliticsSupportIconSize);
            }
        }

        const float SupportRailTop = SupportIconsTop +
            UIConfig::CitizenPoliticsSupportRailTopOffset;
        const float SupportRailLeft = BudgetMargin +
            UIConfig::CitizenPoliticsSupportRailMargin;
        const float SupportRailWidth = SectionWidth -
            UIConfig::CitizenPoliticsSupportRailMargin * 2.f;

        if (auto Rail = Widget.mCitizenPoliticsSupportRail.lock())
        {
            Rail->SetPos(SupportRailLeft, SupportRailTop);
            Rail->SetSize(SupportRailWidth, UIConfig::CitizenPoliticsSupportRailH);
        }

        if (auto Thumb = Widget.mCitizenPoliticsSupportThumb.lock())
        {
            const float SupportRatio =
                (std::max)(0.f, (std::min)(1.f, Widget.mCitizenPoliticsSupportRatio));
            const float ThumbW = UIConfig::CitizenPoliticsSupportThumbW;
            Thumb->SetPos(
                SupportRailLeft + SupportRatio * (SupportRailWidth - ThumbW),
                SupportRailTop + UIConfig::CitizenPoliticsSupportThumbOffsetY);
            Thumb->SetSize(ThumbW, UIConfig::CitizenPoliticsSupportThumbH);
        }
}

void FCitizenInfoRenderer::RefreshCitizenProfileLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float PanelWidth = Context.Panel.Width;
    const float BudgetMargin = Context.Budget.Margin;
    const float SubtitleBottom =
        Context.Panel.OuterTop + Context.Ribbon.OffsetY +
        Context.Ribbon.TitleHeight +
        Context.Metrics.SubtitleOffsetY + Context.Metrics.CompactControlHeight;

        // 4열 × 3행 중앙 정렬 그리드 (11슬롯: 행0=[0-3], 행1=[4-7], 행2=[8-10])
        const float SlotSize  = UIConfig::CitizenProfileSlotSize;
        const float ColGap    = UIConfig::CitizenProfileSlotColGap;
        const float RowGap    = UIConfig::CitizenProfileSlotRowGap;
        const float ColStep   = SlotSize + ColGap;
        const float RowStep   = SlotSize + RowGap;
        const int   ColCount  = 4;
        const float GridW     = static_cast<float>(ColCount - 1) * ColStep + SlotSize;
        const float InnerW    = PanelWidth - BudgetMargin * 2.f;
        const float GridLeft  = BudgetMargin + (InnerW - GridW) * 0.5f;
        const float GridTop   = SubtitleBottom + UIConfig::CitizenProfileGridTopOffset;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenProfileSlotCount;
            ++Index)
        {
            auto Icon =
                Widget.mCitizenProfileIcons[static_cast<size_t>(Index)].lock();

            if (!Icon)
                continue;

            const int Row = Index / ColCount;
            const int Col = Index % ColCount;
            Icon->SetPos(
                GridLeft + static_cast<float>(Col) * ColStep,
                GridTop  + static_cast<float>(Row) * RowStep);
            Icon->SetSize(SlotSize, SlotSize);
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

        // 그리드 3행 아래에 메트릭 시작
        const float DetailTop   = GridTop + 2.f * RowStep + SlotSize +
            UIConfig::CitizenProfileDetailTopMargin;
        const float DetailRowH  = UIConfig::CitizenProfileMetricRowH;
        const float LabelX = BudgetMargin;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GCitizenMetricRowCount;
            ++Index)
        {
            auto Label =
                Widget.mCitizenMetricLabels[static_cast<size_t>(Index)].lock();
            auto Value =
                Widget.mCitizenMetricValues[static_cast<size_t>(Index)].lock();
            const float RowTop =
                DetailTop + static_cast<float>(Index) * DetailRowH;

            const float ValW = UIConfig::CitizenProfileMetricValueWidth;

            if (Label)
            {
                Label->SetPos(LabelX, RowTop);
                Label->SetSize(
                    PanelWidth - LabelX - BudgetMargin - ValW,
                    DetailRowH - 4.f);
            }

            const float ValueX = PanelWidth - BudgetMargin - ValW;

            if (Value)
            {
                Value->SetPos(ValueX, RowTop);
                Value->SetSize(ValW, DetailRowH - 4.f);
            }
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(
                PanelWidth * 0.5f - Context.Metrics.SectionDividerWidth * 0.5f,
                DetailTop +
                    DetailRowH * 8.f +
                    6.f);
            Divider->SetSize(
                Context.Metrics.SectionDividerWidth,
                Context.Metrics.SectionDividerHeight);
        }
}

void FCitizenInfoRenderer::RefreshBuildingModeLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    if (Context.Flags.IsCitizenMode &&
        (Context.Flags.ShowCitizenThoughts ||
            Context.Flags.ShowCitizenPolitics ||
            Context.Flags.ShowCitizenProfile))
    {
        return;
    }

    if (Context.Flags.ShowCustomOverview)
    {
        RefreshBuildingCustomOverviewLayout(Owner, Context);
    }
    else if (Context.Flags.ShowWorkOverview)
    {
        RefreshBuildingWorkOverviewLayout(Owner, Context);
    }
    else if (Context.Flags.ShowCompactRows)
    {
        RefreshBuildingCompactLayout(Owner, Context);
    }
    else if (Context.Flags.ShowInformationParagraphs)
    {
        RefreshBuildingInformationLayout(Owner, Context);
    }
    else
    {
        RefreshBuildingDefaultLayout(Owner, Context);
    }
}

void FCitizenInfoRenderer::RefreshBuildingCustomOverviewLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float PanelWidth = Context.Panel.Width;
    const float BudgetMargin = Context.Budget.Margin;
    const float OccupancyTop = Context.Budget.OccupancyTop;

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
                Widget.mResidentialOverviewResidentIcons[
                    static_cast<size_t>(Index)].lock();

            if (!Icon)
                continue;

            const int Column = Index % 4;
            const int Row = Index / 4;
            Icon->SetPos(
                ResidentStartX + static_cast<float>(Column) * ResidentGapX,
                ResidentStartY + static_cast<float>(Row) * ResidentGapY);
            Icon->SetSize(ResidentIconSize, ResidentIconSize);
        }

        const float MetricsTop = ResidentStartY + ResidentGapY * 4.f + 10.f;
        const float MetricRowH = UIConfig::BuildingCustomOverviewMetricRowHeight;

        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            auto Label =
                Widget.mResidentialOverviewMetricLabels[
                    static_cast<size_t>(Index)].lock();
            auto Value =
                Widget.mResidentialOverviewMetricValues[
                    static_cast<size_t>(Index)].lock();
            const float RowTop =
                MetricsTop + static_cast<float>(Index) * MetricRowH;

            if (Label)
            {
                Label->SetPos(BudgetMargin, RowTop);
                Label->SetSize(140.f, 20.f);
            }

            if (Value)
            {
                Value->SetPos(PanelWidth - BudgetMargin - 160.f, RowTop);
                Value->SetSize(160.f, 20.f);
            }
        }
}

void FCitizenInfoRenderer::RefreshBuildingWorkOverviewLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float PanelWidth = Context.Panel.Width;
    const float BudgetMargin = Context.Budget.Margin;
    const float OccupancyTop = Context.Budget.OccupancyTop;

    constexpr int GWorkOverviewResidentColumns = 8;
    constexpr int GWorkOverviewMaxVisibleResidents = 12;
    const float ResidentStartX = BudgetMargin + 6.f;
    const float ResidentStartY = OccupancyTop + 26.f;
    const float ResidentIconSize = 24.f;
    const float ResidentGapX = 26.f;
    const float ResidentGapY = 22.f;
    int VisibleResidentCount = 0;

    for (int Index = 0;
        Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
        ++Index)
    {
        auto Icon =
            Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock();

        if (!Icon || !Icon->GetEnable())
            continue;

        ++VisibleResidentCount;
    }

    VisibleResidentCount =
        (std::min)(VisibleResidentCount, GWorkOverviewMaxVisibleResidents);
    const int ResidentRowCount =
        VisibleResidentCount > 0 ?
            ((VisibleResidentCount + GWorkOverviewResidentColumns - 1) /
                GWorkOverviewResidentColumns) :
            1;
    const float MetricsTop =
        ResidentStartY +
        ResidentIconSize +
        static_cast<float>((std::max)(0, ResidentRowCount - 1)) *
            ResidentGapY +
        12.f;
    const float MetricRowH = UIConfig::BuildingWorkOverviewMetricRowHeight;

    for (int Index = 0;
        Index < CCitizenInfoWidget::GOverviewResidentSlotCount;
        ++Index)
    {
        auto Icon =
            Widget.mOverviewResidentIcons[static_cast<size_t>(Index)].lock();

        if (!Icon)
            continue;

        if (Index < VisibleResidentCount)
        {
            const int Column = Index % GWorkOverviewResidentColumns;
            const int Row = Index / GWorkOverviewResidentColumns;
            Icon->SetPos(
                ResidentStartX + static_cast<float>(Column) * ResidentGapX,
                ResidentStartY + static_cast<float>(Row) * ResidentGapY);
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
                20.f);
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
                Value->SetSize(140.f, 20.f);
            }
        }

        if (auto Bg =
            Widget.mOverviewMetricValueBgs[static_cast<size_t>(Index)].lock())
        {
            Bg->SetPos(0.f, 0.f);
            Bg->SetSize(0.f, 0.f);
        }

        if (auto Icon =
            Widget.mOverviewMetricIcons[static_cast<size_t>(Index)].lock())
        {
            Icon->SetPos(0.f, 0.f);
            Icon->SetSize(0.f, 0.f);
        }
    }

    LayoutOverviewMetricScrollWidgets(Owner, Context, MetricsTop, MetricRowH);

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

void FCitizenInfoRenderer::RefreshBuildingCompactLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float PanelWidth = Context.Panel.Width;
    const float BudgetMargin = Context.Budget.Margin;
    const float SectionRibbonY = Context.Ribbon.SectionY;
    const float SectionRibbonH = Context.Ribbon.SectionHeight;
    const bool UseStats = Context.Flags.ShowStatsMetricRows;
    const bool UseEfficiency = Context.Flags.ShowEfficiencyMetricRows;

        const float MetricsTop =
            SectionRibbonY + SectionRibbonH + 12.f;
        const float MetricRowH = UIConfig::BuildingCompactMetricRowHeight;

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

        auto& ActiveLabels = UseStats ?
            Widget.mStatsMetricLabels : Widget.mEfficiencyMetricLabels;
        auto& ActiveValues = UseStats ?
            Widget.mStatsMetricValues : Widget.mEfficiencyMetricValues;
        auto& InactiveLabels = UseStats ?
            Widget.mEfficiencyMetricLabels : Widget.mStatsMetricLabels;
        auto& InactiveValues = UseStats ?
            Widget.mEfficiencyMetricValues : Widget.mStatsMetricValues;

        // 비활성 배열 숨김
        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            if (auto L = InactiveLabels[static_cast<size_t>(Index)].lock())
            { L->SetPos(0.f, 0.f); L->SetSize(0.f, 0.f); }
            if (auto V = InactiveValues[static_cast<size_t>(Index)].lock())
            { V->SetPos(0.f, 0.f); V->SetSize(0.f, 0.f); }
        }

        // 활성 배열 배치
        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            auto Label = ActiveLabels[static_cast<size_t>(Index)].lock();
            auto Value = ActiveValues[static_cast<size_t>(Index)].lock();
            const float RowTop =
                MetricsTop + static_cast<float>(Index) * MetricRowH;

            if (Label)
            {
                Label->SetPos(BudgetMargin, RowTop);
                Label->SetSize(180.f, 20.f);
            }

            if (Value)
            {
                Value->SetPos(PanelWidth - BudgetMargin - 120.f, RowTop);
                Value->SetSize(120.f, 20.f);
            }
        }

        LayoutOverviewMetricScrollWidgets(Owner, Context, MetricsTop, MetricRowH);

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(
                PanelWidth * 0.5f - Context.Metrics.SectionDividerWidth * 0.5f,
                MetricsTop + MetricRowH + 8.f);
            Divider->SetSize(
                Context.Metrics.SectionDividerWidth,
                Context.Metrics.SectionDividerHeight);
        }
}

void FCitizenInfoRenderer::RefreshBuildingInformationLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float OuterTop = Context.Panel.OuterTop;
    const float PanelWidth = Context.Panel.Width;
    const float RibbonOffsetY = Context.Ribbon.OffsetY;
    const float TitleRibbonH = Context.Ribbon.TitleHeight;
    const float SectionRibbonY = Context.Ribbon.SectionY;
    const float SectionRibbonH = Context.Ribbon.SectionHeight;
    const bool ShowSectionRibbon = Context.Flags.ShowSectionRibbon;
    const float BudgetMargin = Context.Budget.Margin;

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
            if (auto Icon =
                Widget.mOverviewMetricIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }

            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Bg =
                Widget.mOverviewMetricValueBgs[static_cast<size_t>(Index)].lock())
            {
                Bg->SetPos(0.f, 0.f);
                Bg->SetSize(0.f, 0.f);
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

        const float InfoBaseTop =
            ShowSectionRibbon ?
                (SectionRibbonY + SectionRibbonH) :
                (OuterTop + RibbonOffsetY + TitleRibbonH);
        const float InfoTop = InfoBaseTop + 18.f;
        const float DividerTop = InfoTop + 98.f;
        auto AccentText = Widget.mInformationAccentText.lock();
        const bool ShowAccent = AccentText && AccentText->GetEnable();

        if (AccentText)
        {
            if (ShowAccent)
            {
                AccentText->SetPos(BudgetMargin, InfoTop + 2.f);
                AccentText->SetSize(32.f, 30.f);
            }
            else
            {
                AccentText->SetPos(0.f, 0.f);
                AccentText->SetSize(0.f, 0.f);
            }
        }

        if (auto Text = Widget.mInformationTopText.lock())
        {
            const float AccentOffset = ShowAccent ? 30.f : 0.f;
            Text->SetPos(BudgetMargin + AccentOffset, InfoTop);
            Text->SetSize(
                PanelWidth - BudgetMargin * 2.f - AccentOffset,
                100.f);
        }

        if (auto Divider = Widget.mSectionDivider.lock())
        {
            Divider->SetPos(
                PanelWidth * 0.5f - Context.Metrics.SectionDividerWidth * 0.5f,
                DividerTop);
            Divider->SetSize(
                Context.Metrics.SectionDividerWidth,
                Context.Metrics.SectionDividerHeight);
        }

        if (auto Text = Widget.mInformationBottomText.lock())
        {
            Text->SetPos(BudgetMargin, DividerTop + 28.f);
            Text->SetSize(PanelWidth - BudgetMargin * 2.f, 110.f);
        }
}

void FCitizenInfoRenderer::RefreshBuildingDefaultLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    (void)Context;

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
            if (auto Icon =
                Widget.mOverviewMetricIcons[static_cast<size_t>(Index)].lock())
            {
                Icon->SetPos(0.f, 0.f);
                Icon->SetSize(0.f, 0.f);
            }

            if (auto Label =
                Widget.mOverviewMetricLabels[static_cast<size_t>(Index)].lock())
            {
                Label->SetPos(0.f, 0.f);
                Label->SetSize(0.f, 0.f);
            }

            if (auto Bg =
                Widget.mOverviewMetricValueBgs[static_cast<size_t>(Index)].lock())
            {
                Bg->SetPos(0.f, 0.f);
                Bg->SetSize(0.f, 0.f);
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

void FCitizenInfoRenderer::RefreshInformationVisibilityLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    if (Context.Flags.ShowInformationParagraphs)
    {
        return;
    }

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
}

void FCitizenInfoRenderer::RefreshUpgradeLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const float PanelWidth = Context.Panel.Width;
    const float BudgetMargin = Context.Budget.Margin;
    const float SectionRibbonY = Context.Ribbon.SectionY;
    const float SectionRibbonH = Context.Ribbon.SectionHeight;

    if (Context.Flags.ShowUpgradeCard)
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

}

void FCitizenInfoRenderer::RefreshActionLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const auto& Layout = Context.Metrics;
    const float OuterTop = Context.Panel.OuterTop;
    const float PanelWidth = Context.Panel.Width;
    const float PanelHeight = Context.Panel.Height;
    const float BudgetMargin = Context.Budget.Margin;
    const float ActionTop = Context.Actions.Top;
    const bool ShowActions = Context.Flags.ShowActions;
    const bool ShowAnyOverview = Context.Flags.ShowAnyOverview;
    const bool ShowInformationParagraphs = Context.Flags.ShowInformationParagraphs;
    const bool ShowOverviewCommandButton = Context.Flags.ShowOverviewCommandButton;
    const bool ShowCitizenProfile = Context.Flags.ShowCitizenProfile;
    const float ActionBtnH = UIConfig::BuildingActionButtonHeight;
    const float ActionBtnW = UIConfig::BuildingActionButtonWidth;
    auto ApplyButtonChildFontSize = [](const std::weak_ptr<CButton>& Button, float FontSize)
    {
        auto ButtonShared = Button.lock();

        if (!ButtonShared)
            return;

        const auto Child =
            std::dynamic_pointer_cast<CTextBlock>(ButtonShared->GetChild());

        if (Child)
            Child->SetFontSize(FontSize);
    };

    ApplyButtonChildFontSize(
        Widget.mDemolishButton,
        UIConfig::BuildingActionButtonFontSize);
    ApplyButtonChildFontSize(
        Widget.mMoveButton,
        UIConfig::BuildingActionButtonFontSize);
    ApplyButtonChildFontSize(
        Widget.mFocusButton,
        UIConfig::BuildingActionButtonFontSize);

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
    if (auto FocusButton = Widget.mFocusButton.lock())
    {
        if (ShowAnyOverview || ShowInformationParagraphs)
        {
            FocusButton->SetPos(
                PanelWidth - BudgetMargin - Layout.FocusCompactRightOffset,
                ActionTop + Layout.ActionCompactIconOffsetY);
            FocusButton->SetSize(
                Layout.ActionCompactIconSize,
                Layout.ActionCompactIconSize);
        }
        else
        {
            FocusButton->SetPos(PanelWidth - BudgetMargin - ActionBtnW * 0.55f, ActionTop);
            FocusButton->SetSize(ActionBtnW * 0.5f, ActionBtnH);
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
    const float CitizenActionGroupOffsetX = Layout.ActionGroupOffsetX;
    const float CitizenActionGroupOffsetY = Layout.ActionGroupOffsetY;
    const float CitizenActionGroupWidthAdd = Layout.ActionGroupWidthAdd;
    const float CitizenActionGroupHeightAdd = Layout.ActionGroupHeightAdd;

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
                Button->SetPos(
                    BudgetMargin + CitizenActionGroupOffsetX,
                    ButtonTop + CitizenActionGroupOffsetY);
                Button->SetSize(
                    PanelWidth - BudgetMargin * 2.f + CitizenActionGroupWidthAdd,
                    CitizenActionBtnH + CitizenActionGroupHeightAdd);
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
                    BudgetMargin + CitizenActionGroupOffsetX +
                        Layout.ActionIconInset,
                    ButtonTop + CitizenActionGroupOffsetY +
                        Layout.ActionIconInset);
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

        if (auto Text =
            Widget.mCitizenActionButtonTexts[static_cast<size_t>(Index)].lock())
        {
            if (ShowCitizenProfile)
            {
                const float TextOffsetX =
                    Layout.ActionIconInset * 2.f + Layout.ActionIconSize;
                Text->SetPos(TextOffsetX, 0.f);
                Text->SetSize(
                    PanelWidth - BudgetMargin * 2.f - TextOffsetX +
                        CitizenActionGroupWidthAdd,
                    CitizenActionBtnH + CitizenActionGroupHeightAdd);
            }
            else
            {
                Text->SetPos(0.f, 0.f);
                Text->SetSize(0.f, 0.f);
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
}

void FCitizenInfoRenderer::RefreshBodyLayout(
    CCitizenInfoWidget& Owner,
    const FCitizenInfoLayoutContext& Context)
{
    auto Widget = Owner.GetRendererView();
    const auto& Layout = Context.Metrics;
    const float OuterTop = Context.Panel.OuterTop;
    const float PanelWidth = Context.Panel.Width;
    const float PanelHeight = Context.Panel.Height;
    const float RibbonOffsetY = Context.Ribbon.OffsetY;
    const float TitleRibbonH = Context.Ribbon.TitleHeight;
    const float SectionRibbonY = Context.Ribbon.SectionY;
    const float SectionRibbonH = Context.Ribbon.SectionHeight;
    const bool ShowSectionRibbon = Context.Flags.ShowSectionRibbon;
    const bool ShowCitizenProfile = Context.Flags.ShowCitizenProfile;
    const bool ShowCitizenPolitics = Context.Flags.ShowCitizenPolitics;
    const bool ShowCitizenThoughts = Context.Flags.ShowCitizenThoughts;
    const bool ShowAnyOverview = Context.Flags.ShowAnyOverview;
    const bool ShowCompactRows = Context.Flags.ShowCompactRows;
    const bool ShowUpgradeCard = Context.Flags.ShowUpgradeCard;
    const bool ShowActions = Context.Flags.ShowActions;
    const bool ShowInformationParagraphs = Context.Flags.ShowInformationParagraphs;
    const float BudgetButtonTop = Context.Budget.ButtonTop;
    const float BudgetButtonH = Context.Budget.ButtonHeight;
    const float BudgetBaseY = Context.Budget.BaseY;
    const float BudgetMargin = Context.Budget.Margin;
    const float ActionTop = Context.Actions.Top;
    int VisibleMetricRowCount = 0;

    if (ShowCompactRows)
    {
        auto& ActiveLabels = Context.Flags.ShowStatsMetricRows ?
            Widget.mStatsMetricLabels : Widget.mEfficiencyMetricLabels;
        for (int Index = 0;
            Index < CCitizenInfoWidget::GOverviewMetricRowCount;
            ++Index)
        {
            auto Label = ActiveLabels[static_cast<size_t>(Index)].lock();
            if (Label && Label->GetEnable())
                ++VisibleMetricRowCount;
        }
    }

    const float CompactMetricsBottom =
        SectionRibbonY + SectionRibbonH +
        12.f +
        static_cast<float>(VisibleMetricRowCount) * UIConfig::BuildingCompactMetricRowHeight;
    const float InformationBaseTop =
        ShowSectionRibbon ?
            (SectionRibbonY + SectionRibbonH) :
            (OuterTop + RibbonOffsetY + TitleRibbonH);

    const float BodyTop =
        ShowUpgradeCard ? (SectionRibbonY + SectionRibbonH + 136.f) :
        ShowInformationParagraphs ?
            (InformationBaseTop + 266.f) :
        ShowCompactRows ? (CompactMetricsBottom + 24.f) :
        ShowSectionRibbon ? (SectionRibbonY + SectionRibbonH + Layout.BodyGapAfterSection) :
        (ShowCitizenProfile || ShowCitizenPolitics || ShowCitizenThoughts ?
            (OuterTop + PanelHeight) :
            (ShowAnyOverview ? (OuterTop + PanelHeight) :
            (ShowActions ? (BudgetButtonTop + BudgetButtonH + Layout.BodyGapAfterActions) :
                           (BudgetBaseY - Layout.BodyFallbackOffset))));
    const float BodyBottom =
        ShowActions ? (ActionTop - Layout.BodyGapBeforeActions) :
        (OuterTop + PanelHeight - Layout.BodyBottomInset);

    const bool ShowStatsMetricRows = Context.Flags.ShowStatsMetricRows;
    const bool ShowEfficiencyMetricRows = Context.Flags.ShowEfficiencyMetricRows;
    const bool HideSharedBody =
        ShowCitizenProfile || ShowCitizenPolitics ||
        ShowCitizenThoughts || ShowAnyOverview ||
        ShowStatsMetricRows || ShowEfficiencyMetricRows ||
        ShowUpgradeCard;

    if (auto BodyText = Widget.mBodyText.lock())
    {
        if (HideSharedBody)
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

    const float TabBodyW = PanelWidth - BudgetMargin * 2.f;
    const float TabBodyH = (std::max)(80.f, BodyBottom - BodyTop);

    // 탭별 body text 위치 설정
    if (auto Text = Widget.mStatsBodyText.lock())
    {
        if (ShowStatsMetricRows)
        { Text->SetPos(BudgetMargin, BodyTop); Text->SetSize(TabBodyW, TabBodyH); }
        else
        { Text->SetPos(0.f, 0.f); Text->SetSize(0.f, 0.f); }
    }
    if (auto Text = Widget.mUpgradeBodyText.lock())
    {
        if (ShowUpgradeCard)
        { Text->SetPos(BudgetMargin, BodyTop); Text->SetSize(TabBodyW, TabBodyH); }
        else
        { Text->SetPos(0.f, 0.f); Text->SetSize(0.f, 0.f); }
    }
    if (auto Text = Widget.mEfficiencyBodyText.lock())
    {
        if (ShowEfficiencyMetricRows)
        { Text->SetPos(BudgetMargin, BodyTop); Text->SetSize(TabBodyW, TabBodyH); }
        else
        { Text->SetPos(0.f, 0.f); Text->SetSize(0.f, 0.f); }
    }
}

