#include "AlmanacRenderer.h"
#include "AlmanacRendererInternal.h"
#include "UILayoutConfig.h"
#include "Device.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

struct FAlmanacChromeMetrics
{
    float PanelTopOffset = 0.f;
    float RibbonTopOffset = 0.f;
    float FrameInsetX = 0.f;
    float FrameHeaderOverlap = 0.f;
    float FrameBottomInset = 0.f;
    float RailLeftInset = 0.f;
    float RailTopInset = 0.f;
    float RailBottomInset = 0.f;
    float RailThumbTopOffset = 0.f;
    float RailThumbMinHeight = 0.f;
    float RailThumbExpand = 0.f;
    float RailToContentGap = 0.f;
    float ContentTopInset = 0.f;
    float ContentBottomInset = 0.f;
    float TitlePaddingX = 0.f;
    float TitlePaddingY = 0.f;
    float CloseButtonSize = 0.f;
    float CloseButtonOffsetX = 0.f;
    float CloseButtonOffsetY = 0.f;
};

struct FAlmanacPageMetrics
{
    float ColumnGap = 0.f;
    float WideColumnGap = 0.f;
    float TitleHeight = 0.f;
    float FrameTop = 0.f;
};

struct FAlmanacLayoutContext
{
    float Scale = 1.f;
    float PanelLeft = 0.f;
    float PanelTop = 0.f;
    float PanelWidth = 0.f;
    float PanelHeight = 0.f;
    float RibbonLeft = 0.f;
    float RibbonTop = 0.f;
    float RibbonWidth = 0.f;
    float RibbonHeight = 0.f;
    float FrameLeft = 0.f;
    float FrameTop = 0.f;
    float FrameWidth = 0.f;
    float FrameHeight = 0.f;
    float RailTrackLeft = 0.f;
    float RailTrackTop = 0.f;
    float RailTrackWidth = 0.f;
    float RailTrackHeight = 0.f;
    float RailThumbHeight = 0.f;
    float ContentLeft = 0.f;
    float ContentTop = 0.f;
    float ContentWidth = 0.f;
    float ContentHeight = 0.f;
    float HeaderPadding = 0.f;
    float TabSize = 0.f;
    float TabGap = 0.f;
    float TabBaseY = 0.f;
    float TabSelectedOffsetY = 0.f;
    float MetricRowHeight = 0.f;
    float MetricRowGap = 0.f;
    float DetailRowHeight = 0.f;
    float DetailRowGap = 0.f;
    int CardColumns = 1;
    float CardGapX = 0.f;
    float CardGapY = 0.f;
    float LeftPanelRatio = 0.5f;
    FAlmanacChromeMetrics Chrome;
    FAlmanacPageMetrics PageMetrics;
};

namespace
{
    constexpr float GAlmanacMinimumPanelBaseWidth = 720.f;
    constexpr float GAlmanacMinimumPanelBaseHeight = 520.f;
    constexpr float GAlmanacMinimumAvailableExtent = 360.f;
    constexpr float GAlmanacHorizontalViewportInset = 80.f;
    constexpr float GAlmanacVerticalViewportInset = 120.f;
    constexpr float GAlmanacMinimumRibbonWidth = 420.f;
    constexpr float GAlmanacRibbonWidthInset = 120.f;
    constexpr float GAlmanacRailTrackBaseWidth = 10.f;
    constexpr float GAlmanacTabMarkerWidth = 28.f;
    constexpr float GAlmanacTabMarkerHeight = 16.f;
    constexpr float GAlmanacTabMarkerTopOffset = 3.f;

    float ResolveOffscreenHiddenCoord(float ContentExtent, float Scale)
    {
        return ContentExtent + UIConfig::AlmanacOffscreenHideOffset * Scale;
    }

    FAlmanacLayoutContext BuildLayoutContext(const FResolution& Resolution)
    {
        FAlmanacLayoutContext Layout;
        const float ScreenWidth = static_cast<float>(Resolution.Width);
        const float ScreenHeight = static_cast<float>(Resolution.Height);
        const float PanelBaseWidth =
            (std::max)(GAlmanacMinimumPanelBaseWidth, UIConfig::AlmanacPanelWidth);
        const float PanelBaseHeight =
            (std::max)(GAlmanacMinimumPanelBaseHeight, UIConfig::AlmanacPanelHeight);
        const float AvailableWidth =
            (std::max)(
                GAlmanacMinimumAvailableExtent,
                ScreenWidth - GAlmanacHorizontalViewportInset);
        const float AvailableHeight =
            (std::max)(
                GAlmanacMinimumAvailableExtent,
                ScreenHeight - GAlmanacVerticalViewportInset);

        Layout.Scale =
            (std::min)(1.f,
                (std::min)(
                    AvailableWidth / PanelBaseWidth,
                    AvailableHeight / PanelBaseHeight));
        Layout.PanelWidth = PanelBaseWidth * Layout.Scale;
        Layout.PanelHeight = PanelBaseHeight * Layout.Scale;
        Layout.PanelLeft = (ScreenWidth - Layout.PanelWidth) * 0.5f;
        Layout.Chrome =
        {
            UIConfig::AlmanacPanelTopOffset * Layout.Scale,
            UIConfig::AlmanacRibbonTopOffset * Layout.Scale,
            UIConfig::AlmanacFrameInsetX * Layout.Scale,
            UIConfig::AlmanacFrameHeaderOverlap * Layout.Scale,
            UIConfig::AlmanacFrameBottomInset * Layout.Scale,
            UIConfig::AlmanacRailLeftInset * Layout.Scale,
            UIConfig::AlmanacRailTopInset * Layout.Scale,
            UIConfig::AlmanacRailBottomInset * Layout.Scale,
            UIConfig::AlmanacRailThumbTopOffset * Layout.Scale,
            UIConfig::AlmanacRailThumbMinHeight * Layout.Scale,
            UIConfig::AlmanacRailThumbExpand * Layout.Scale,
            UIConfig::AlmanacRailToContentGap * Layout.Scale,
            UIConfig::AlmanacContentTopInset * Layout.Scale,
            UIConfig::AlmanacContentBottomInset * Layout.Scale,
            UIConfig::AlmanacTitlePaddingX * Layout.Scale,
            UIConfig::AlmanacTitlePaddingY * Layout.Scale,
            UIConfig::AlmanacCloseButtonSize * Layout.Scale,
            UIConfig::AlmanacCloseButtonOffsetX * Layout.Scale,
            UIConfig::AlmanacCloseButtonOffsetY * Layout.Scale
        };
        Layout.PageMetrics =
        {
            UIConfig::AlmanacPageColumnGap * Layout.Scale,
            UIConfig::AlmanacWidePageColumnGap * Layout.Scale,
            UIConfig::AlmanacPageTitleHeight * Layout.Scale,
            UIConfig::AlmanacPageFrameTop * Layout.Scale
        };
        Layout.PanelTop =
            (ScreenHeight - Layout.PanelHeight) * 0.5f +
            Layout.Chrome.PanelTopOffset;

        const float HeaderHeight = UIConfig::AlmanacHeaderHeight * Layout.Scale;
        Layout.HeaderPadding = UIConfig::AlmanacHeaderPadding * Layout.Scale;
        const float ContentMarginX = UIConfig::AlmanacContentMarginX * Layout.Scale;
        const float ContentMarginTop =
            UIConfig::AlmanacContentMarginTop * Layout.Scale;
        const float ContentMarginBottom =
            UIConfig::AlmanacContentMarginBottom * Layout.Scale;
        Layout.TabSize = UIConfig::AlmanacTabSize * Layout.Scale;
        Layout.TabGap = UIConfig::AlmanacTabGap * Layout.Scale;
        const float TabBaseOffsetY =
            UIConfig::AlmanacTabBaseOffsetY * Layout.Scale;
        Layout.TabSelectedOffsetY =
            UIConfig::AlmanacTabSelectedOffsetY * Layout.Scale;
        Layout.MetricRowHeight = UIConfig::AlmanacMetricRowHeight * Layout.Scale;
        Layout.MetricRowGap = UIConfig::AlmanacMetricRowGap * Layout.Scale;
        Layout.DetailRowHeight = UIConfig::AlmanacDetailRowHeight * Layout.Scale;
        Layout.DetailRowGap = UIConfig::AlmanacDetailRowGap * Layout.Scale;
        Layout.CardColumns =
            (std::max)(1, static_cast<int>(UIConfig::AlmanacCardColumns));
        Layout.CardGapX = UIConfig::AlmanacCardGapX * Layout.Scale;
        Layout.CardGapY = UIConfig::AlmanacCardGapY * Layout.Scale;
        Layout.LeftPanelRatio = UIConfig::AlmanacLeftPanelRatio;

        Layout.RibbonWidth =
            (std::max)(
                GAlmanacMinimumRibbonWidth * Layout.Scale,
                Layout.PanelWidth - GAlmanacRibbonWidthInset * Layout.Scale);
        Layout.RibbonHeight = HeaderHeight * 0.64f;
        Layout.RibbonLeft =
            Layout.PanelLeft + (Layout.PanelWidth - Layout.RibbonWidth) * 0.5f;
        Layout.RibbonTop = Layout.PanelTop + Layout.Chrome.RibbonTopOffset;
        Layout.FrameLeft = Layout.PanelLeft + Layout.Chrome.FrameInsetX;
        Layout.FrameTop =
            Layout.PanelTop + HeaderHeight - Layout.Chrome.FrameHeaderOverlap;
        Layout.FrameWidth = Layout.PanelWidth - Layout.Chrome.FrameInsetX * 2.f;
        Layout.FrameHeight =
            Layout.PanelHeight -
            (Layout.FrameTop - Layout.PanelTop) -
            Layout.Chrome.FrameBottomInset;
        Layout.RailTrackLeft = Layout.FrameLeft + Layout.Chrome.RailLeftInset;
        Layout.RailTrackTop = Layout.FrameTop + Layout.Chrome.RailTopInset;
        Layout.RailTrackWidth = GAlmanacRailTrackBaseWidth * Layout.Scale;
        Layout.RailTrackHeight = Layout.FrameHeight - Layout.Chrome.RailBottomInset;
        Layout.RailThumbHeight =
            (std::max)(
                Layout.Chrome.RailThumbMinHeight,
                Layout.RailTrackHeight * 0.18f);
        Layout.ContentLeft =
            Layout.RailTrackLeft + Layout.RailTrackWidth +
            Layout.Chrome.RailToContentGap +
            ContentMarginX * 0.45f;
        Layout.ContentTop =
            Layout.FrameTop + Layout.Chrome.ContentTopInset + ContentMarginTop;
        Layout.ContentWidth =
            Layout.FrameLeft + Layout.FrameWidth -
            ContentMarginX * 0.65f - Layout.ContentLeft;
        Layout.ContentHeight =
            Layout.FrameTop + Layout.FrameHeight -
            ContentMarginBottom - Layout.Chrome.ContentBottomInset -
            Layout.ContentTop;
        Layout.TabBaseY = Layout.PanelTop - TabBaseOffsetY;
        return Layout;
    }

    void LayoutMetricRows(
        float Scale,
        const std::vector<CAlmanacWidget::FMetricRowWidgets>& Rows,
        float X,
        float Y,
        float Width,
        float RowHeight,
        float Gap)
    {
        const float LabelWidth = Width * 0.42f;
        const float ValueWidth = 100.f * Scale;
        const float BarGap = 12.f * Scale;
        const float LabelInsetX = 14.f * Scale;
        const float LabelWidthTrim = 16.f * Scale;
        const float ValueInsetX = 12.f * Scale;
        const float MinBarWidth = 40.f;
        const float LabelBaselineOffsetY = 1.f * Scale;
        const float MinBarHeight = 4.f * Scale;
        const float BarLeft = X + LabelWidth + BarGap;
        const float BarWidth =
            (std::max)(
                MinBarWidth,
                Width - LabelWidth - ValueWidth - 34.f * Scale);

        for (size_t i = 0; i < Rows.size(); ++i)
        {
            const float RowY = Y + (RowHeight + Gap) * static_cast<float>(i);

            if (auto Bg = Rows[i].Background.lock())
            {
                Bg->SetPos(X, RowY);
                Bg->SetSize(Width, RowHeight);
            }
            if (auto Lbl = Rows[i].Label.lock())
            {
                Lbl->SetPos(X + LabelInsetX, RowY - LabelBaselineOffsetY);
                Lbl->SetSize(LabelWidth - LabelWidthTrim, RowHeight);
            }
            if (auto Bar = Rows[i].Bar.lock())
            {
                Bar->SetPos(BarLeft, RowY + RowHeight * 0.58f);
                Bar->SetSize(
                    BarWidth,
                    (std::max)(MinBarHeight, RowHeight * 0.12f));
            }
            if (auto Val = Rows[i].Value.lock())
            {
                Val->SetPos(
                    X + Width - ValueWidth - ValueInsetX,
                    RowY - LabelBaselineOffsetY);
                Val->SetSize(ValueWidth, RowHeight);
            }
        }
    }

    void LayoutSatisfactionRows(
        float Scale,
        const std::vector<CAlmanacWidget::FSatisfactionRowWidgets>& Rows,
        float X,
        float Y,
        float Width,
        float RowHeight,
        float Gap)
    {
        const float InnerPadding = 12.f * Scale;
        const float IconSize = 30.f * Scale;
        const float ValueWidth = 64.f * Scale;
        const float LabelGap = 12.f * Scale;
        const float LabelRightTrim = 18.f * Scale;
        const float BarBottomInset = 6.f * Scale;
        const float LabelLeft = InnerPadding + IconSize + LabelGap;
        const float BarWidth =
            (std::max)(
                40.f,
                Width - LabelLeft - ValueWidth - InnerPadding * 2.f);

        for (size_t i = 0; i < Rows.size(); ++i)
        {
            const float RowY = Y + (RowHeight + Gap) * static_cast<float>(i);

            if (auto Button = Rows[i].Button.lock())
            {
                Button->SetPos(X, RowY);
                Button->SetSize(Width, RowHeight);
            }
            if (auto Icon = Rows[i].Icon.lock())
            {
                Icon->SetPos(
                    InnerPadding,
                    (RowHeight - IconSize) * 0.5f);
                Icon->SetSize(IconSize, IconSize);
            }
            if (auto Label = Rows[i].Label.lock())
            {
                Label->SetPos(LabelLeft, 0.f);
                Label->SetSize(
                    Width - LabelLeft - ValueWidth - LabelRightTrim,
                    RowHeight);
            }
            if (auto Bar = Rows[i].Bar.lock())
            {
                Bar->SetPos(LabelLeft, RowHeight - BarBottomInset);
                Bar->SetSize(BarWidth, 1.f * Scale);
            }
            if (auto Value = Rows[i].Value.lock())
            {
                Value->SetPos(Width - ValueWidth - InnerPadding, 0.f);
                Value->SetSize(ValueWidth, RowHeight);
            }
        }
    }

    void LayoutPoliticsFactionTiles(
        float Scale,
        const std::vector<CAlmanacWidget::FPoliticsFactionTileWidgets>& Tiles,
        float X,
        float Y,
        float TileWidth,
        float TileHeight,
        float GapX,
        float GapY)
    {
        const float IconSize = 28.f * Scale;
        const float SmallIconSize = 13.f * Scale;
        const float IconInsetX = 12.f * Scale;
        const float IconInsetY = 10.f * Scale;
        const float LabelLeft = 46.f * Scale;
        const float LabelTop = 8.f * Scale;
        const float LabelRightTrim = 58.f * Scale;
        const float CountRowBottomInset = 22.f * Scale;
        const float CountValueLeft = 28.f * Scale;
        const float CountValueBottomInset = 25.f * Scale;
        const float CountValueWidth = 44.f * Scale;
        const float CountValueHeight = 20.f * Scale;
        const float FavorIconRightInset = 58.f * Scale;
        const float FavorValueRightInset = 40.f * Scale;
        const float FavorValueBottomInset = 25.f * Scale;
        const float FavorValueWidth = 32.f * Scale;
        const float FavorValueHeight = 20.f * Scale;

        for (size_t i = 0; i < Tiles.size(); ++i)
        {
            const int Row = static_cast<int>(i) / 2;
            const int Col = static_cast<int>(i) % 2;
            const float TileX =
                X + (TileWidth + GapX) * static_cast<float>(Col);
            const float TileY =
                Y + (TileHeight + GapY) * static_cast<float>(Row);

            if (auto Button = Tiles[i].Button.lock())
            {
                Button->SetPos(TileX, TileY);
                Button->SetSize(TileWidth, TileHeight);
            }
            if (auto Icon = Tiles[i].Icon.lock())
            {
                Icon->SetPos(IconInsetX, IconInsetY);
                Icon->SetSize(IconSize, IconSize);
            }
            if (auto Label = Tiles[i].Label.lock())
            {
                Label->SetPos(LabelLeft, LabelTop);
                Label->SetSize(TileWidth - LabelRightTrim, 28.f * Scale);
            }
            if (auto CountIcon = Tiles[i].CountIcon.lock())
            {
                CountIcon->SetPos(IconInsetX, TileHeight - CountRowBottomInset);
                CountIcon->SetSize(SmallIconSize, SmallIconSize);
            }
            if (auto CountValue = Tiles[i].CountValue.lock())
            {
                CountValue->SetPos(
                    CountValueLeft,
                    TileHeight - CountValueBottomInset);
                CountValue->SetSize(CountValueWidth, CountValueHeight);
            }
            if (auto FavorIcon = Tiles[i].FavorIcon.lock())
            {
                FavorIcon->SetPos(
                    TileWidth - FavorIconRightInset,
                    TileHeight - CountRowBottomInset);
                FavorIcon->SetSize(SmallIconSize, SmallIconSize);
            }
            if (auto FavorValue = Tiles[i].FavorValue.lock())
            {
                FavorValue->SetPos(
                    TileWidth - FavorValueRightInset,
                    TileHeight - FavorValueBottomInset);
                FavorValue->SetSize(FavorValueWidth, FavorValueHeight);
            }
        }
    }

    void LayoutDetailRows(
        float Scale,
        const std::vector<CAlmanacWidget::FDetailRowWidgets>& Rows,
        float X,
        float Y,
        float Width,
        float RowHeight,
        float Gap)
    {
        const float ValueWidth = 176.f * Scale;
        const float LabelInsetX = 14.f * Scale;
        const float ValueInsetX = 12.f * Scale;
        const float LabelWidthTrim = 26.f * Scale;
        const float LabelBaselineOffsetY = 1.f * Scale;

        for (size_t i = 0; i < Rows.size(); ++i)
        {
            const float RowY = Y + (RowHeight + Gap) * static_cast<float>(i);

            if (auto Button = Rows[i].Button.lock())
            {
                Button->SetPos(X, RowY);
                Button->SetSize(Width, RowHeight);
            }
            else if (auto Bg = Rows[i].Background.lock())
            {
                Bg->SetPos(X, RowY);
                Bg->SetSize(Width, RowHeight);
            }
            if (auto Lbl = Rows[i].Label.lock())
            {
                const bool UseLocalSpace = !Rows[i].Button.expired();
                Lbl->SetPos(
                    UseLocalSpace ? LabelInsetX : X + LabelInsetX,
                    UseLocalSpace ? 0.f : RowY - LabelBaselineOffsetY);
                Lbl->SetSize(Width - ValueWidth - LabelWidthTrim, RowHeight);
            }
            if (auto Val = Rows[i].Value.lock())
            {
                const bool UseLocalSpace = !Rows[i].Button.expired();
                Val->SetPos(
                    UseLocalSpace ?
                        Width - ValueWidth - ValueInsetX :
                        X + Width - ValueWidth - ValueInsetX,
                    UseLocalSpace ? 0.f : RowY - LabelBaselineOffsetY);
                Val->SetSize(ValueWidth, RowHeight);
            }
        }
    }

    void LayoutOverviewCard(
        float Scale,
        const CAlmanacWidget::FCardWidgets& Card,
        float X,
        float Y,
        float Width,
        float Height)
    {
        const float IconSize = 58.f * Scale;
        const float TextInsetX = 10.f * Scale;
        const float TitleTop = 82.f * Scale;
        const float TitleHeight = 38.f * Scale;
        const float ValueTop = 112.f * Scale;
        const float ValueHeight = 28.f * Scale;
        const float DetailTop = 140.f * Scale;
        const float DetailBottomInset = 148.f * Scale;

        if (auto Background = Card.Background.lock())
        {
            Background->SetPos(X, Y);
            Background->SetSize(Width, Height);
        }
        if (auto Icon = Card.Icon.lock())
        {
            Icon->SetPos(X + (Width - IconSize) * 0.5f, Y + 16.f * Scale);
            Icon->SetSize(IconSize, IconSize);
        }
        if (auto Title = Card.Title.lock())
        {
            Title->SetPos(X + TextInsetX, Y + TitleTop);
            Title->SetSize(Width - TextInsetX * 2.f, TitleHeight);
        }
        if (auto Value = Card.Value.lock())
        {
            Value->SetPos(X + TextInsetX, Y + ValueTop);
            Value->SetSize(Width - TextInsetX * 2.f, ValueHeight);
        }
        if (auto Detail = Card.Detail.lock())
        {
            Detail->SetPos(X + TextInsetX, Y + DetailTop);
            Detail->SetSize(
                Width - TextInsetX * 2.f,
                Height - DetailBottomInset);
        }
    }
}

void FAlmanacRenderer::RefreshLayout(CAlmanacWidget& Widget)
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    Widget.mLastResolutionWidth = Resolution.Width;
    Widget.mLastResolutionHeight = Resolution.Height;

    const FAlmanacLayoutContext Layout = BuildLayoutContext(Resolution);
    ValidateLayoutForDebug(Widget, Layout);
    RefreshChromeLayout(Widget, Layout);
    RefreshNavigationLayout(Widget, Layout);
    RefreshPageContainerLayout(Widget, Layout);
    RefreshOverviewLayout(Widget, Layout);
    RefreshSatisfactionLayout(Widget, Layout);
    RefreshPopulationLayout(Widget, Layout);
    RefreshEconomyLayout(Widget, Layout);
    RefreshResourceLayout(Widget, Layout);
    RefreshPoliticsLayout(Widget, Layout);
    RefreshForeignLayout(Widget, Layout);
    RefreshBuildingLayout(Widget, Layout);
    RefreshConflictLayout(Widget, Layout);
    Widget.mLayoutDirty = false;
}

void FAlmanacRenderer::ValidateLayoutForDebug(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
#ifndef NDEBUG
    auto IsPositiveFinite = [](float Value)
    {
        return std::isfinite(Value) && Value > 0.f;
    };

    const auto ExpectedPageCount =
        static_cast<size_t>(EAlmanacPage::Count);

    assert(IsPositiveFinite(Layout.Scale));
    assert(IsPositiveFinite(Layout.PanelWidth));
    assert(IsPositiveFinite(Layout.PanelHeight));
    assert(IsPositiveFinite(Layout.FrameWidth));
    assert(IsPositiveFinite(Layout.FrameHeight));
    assert(IsPositiveFinite(Layout.ContentWidth));
    assert(IsPositiveFinite(Layout.ContentHeight));
    assert(IsPositiveFinite(Layout.RibbonWidth));
    assert(IsPositiveFinite(Layout.RibbonHeight));
    assert(IsPositiveFinite(Layout.RailTrackWidth));
    assert(IsPositiveFinite(Layout.RailTrackHeight));
    assert(Layout.CardColumns >= 1);
    assert(UIConfig::AlmanacOffscreenHideOffset >= 0.f);
    assert(Widget.mTabButtons.size() == ExpectedPageCount);
    assert(Widget.mPages.size() == ExpectedPageCount);
    assert(
        static_cast<size_t>(Widget.mSelectedPage) < ExpectedPageCount);
    assert(
        Widget.mOverviewCards.size() ==
        static_cast<size_t>(GOverviewCardCount));
    assert(
        Widget.mOverviewSectionTitles.size() ==
        static_cast<size_t>(GOverviewSectionTitleCount));
    assert(
        Widget.mSatisfactionRows.size() ==
        static_cast<size_t>(GSatisfactionRowCount));
    assert(
        Widget.mSatisfactionChartGridLines.size() ==
        static_cast<size_t>(GSatisfactionGraphGridLineCount));
    assert(
        Widget.mSatisfactionChartPrimaryLines.size() ==
        static_cast<size_t>(GSatisfactionGraphSegmentCount));
    assert(
        Widget.mSatisfactionChartSecondaryLines.size() ==
        static_cast<size_t>(GSatisfactionGraphSegmentCount));
    assert(
        Widget.mSatisfactionChartXAxisLabels.size() ==
        static_cast<size_t>(GSatisfactionGraphPointCount));
    assert(
        Widget.mSatisfactionChartYAxisLabels.size() ==
        static_cast<size_t>(GSatisfactionGraphGridLineCount));
    assert(
        Widget.mSatisfactionDetails.size() ==
        static_cast<size_t>(GSatisfactionDetailCount));
    assert(
        Widget.mPopulationDetails.size() ==
        static_cast<size_t>(GPopulationDetailCount));
    assert(
        Widget.mPopulationMetrics.size() ==
        static_cast<size_t>(GPopulationMetricCount));
    assert(
        Widget.mPopulationTrendGridLines.size() ==
        static_cast<size_t>(GPopulationTrendGridLineCount));
    assert(
        Widget.mPopulationTrendLines.size() ==
        static_cast<size_t>(GPopulationTrendSegmentCount));
    assert(
        Widget.mPopulationTrendChildBars.size() ==
        static_cast<size_t>(GPopulationDistributionBarCount));
    assert(
        Widget.mPopulationTrendAdultBars.size() ==
        static_cast<size_t>(GPopulationDistributionBarCount));
    assert(
        Widget.mPopulationTrendRetiredBars.size() ==
        static_cast<size_t>(GPopulationDistributionBarCount));
    assert(
        Widget.mPopulationTrendRichBars.size() ==
        static_cast<size_t>(GPopulationDistributionBarCount));
    assert(
        Widget.mPopulationTrendFilthyRichBars.size() ==
        static_cast<size_t>(GPopulationDistributionBarCount));
    assert(
        Widget.mPopulationTrendXAxisLabels.size() ==
        static_cast<size_t>(GPopulationTrendXAxisLabelCount));
    assert(
        Widget.mPopulationTrendYAxisLabels.size() ==
        static_cast<size_t>(GPopulationTrendYAxisLabelCount));
    assert(
        Widget.mPopulationChangeGridLines.size() ==
        static_cast<size_t>(GPopulationChangeGridLineCount));
    assert(
        Widget.mPopulationChangePositiveBars.size() ==
        static_cast<size_t>(GPopulationChangeBarCount));
    assert(
        Widget.mPopulationChangeNegativeBars.size() ==
        static_cast<size_t>(GPopulationChangeBarCount));
    assert(
        Widget.mPopulationChangeXAxisLabels.size() ==
        static_cast<size_t>(GPopulationChangeXAxisLabelCount));
    assert(
        Widget.mPopulationChangeYAxisLabels.size() ==
        static_cast<size_t>(GPopulationChangeYAxisLabelCount));
    assert(
        Widget.mEconomyDetails.size() ==
        static_cast<size_t>(GEconomyDetailCount));
    assert(
        Widget.mEconomyMetrics.size() ==
        static_cast<size_t>(GEconomyMetricCount));
    assert(
        Widget.mEconomyTrendGridLines.size() ==
        static_cast<size_t>(GEconomyTrendGridLineCount));
    assert(
        Widget.mEconomyTrendLines.size() ==
        static_cast<size_t>(GEconomyTrendSegmentCount * 2));
    assert(
        Widget.mEconomyTrendBars.size() ==
        static_cast<size_t>(GEconomyTrendBarCount));
    assert(
        Widget.mEconomyTrendSecondaryBars.size() ==
        static_cast<size_t>(GEconomyTrendBarCount));
    assert(
        Widget.mEconomyTrendTertiaryBars.size() ==
        static_cast<size_t>(GEconomyTrendBarCount));
    assert(
        Widget.mEconomyTrendXAxisLabels.size() ==
        static_cast<size_t>(GEconomyTrendXAxisLabelCount));
    assert(
        Widget.mEconomyTrendYAxisLabels.size() ==
        static_cast<size_t>(GEconomyTrendYAxisLabelCount));
    assert(
        Widget.mEconomyChangeGridLines.size() ==
        static_cast<size_t>(GEconomyChangeGridLineCount));
    assert(
        Widget.mEconomyChangePositiveBars.size() ==
        static_cast<size_t>(GEconomyChangeBarCount));
    assert(
        Widget.mEconomyChangeNegativeBars.size() ==
        static_cast<size_t>(GEconomyChangeBarCount));
    assert(
        Widget.mEconomyChangeYAxisLabels.size() ==
        static_cast<size_t>(GEconomyChangeYAxisLabelCount));
    assert(
        Widget.mEconomyBreakdownRows.size() ==
        static_cast<size_t>(GEconomyBreakdownRowCount));
    assert(
        Widget.mResourceRows.size() ==
        static_cast<size_t>(GResourceRowCount));
    assert(
        Widget.mResourceProductionGridLines.size() ==
        static_cast<size_t>(GResourceProductionGridLineCount));
    assert(
        Widget.mResourceProductionBars.size() ==
        static_cast<size_t>(GResourceProductionBarCount));
    assert(
        Widget.mResourceProductionXAxisLabels.size() ==
        static_cast<size_t>(GResourceProductionXAxisLabelCount));
    assert(
        Widget.mResourceProductionYAxisLabels.size() ==
        static_cast<size_t>(GResourceProductionYAxisLabelCount));
    assert(
        Widget.mResourceDistributionRows.size() ==
        static_cast<size_t>(GResourceDistributionRowCount));
    assert(
        Widget.mResourceDetails.size() ==
        static_cast<size_t>(GResourceDetailCount));
    assert(
        Widget.mPoliticsFactionTiles.size() ==
        static_cast<size_t>(GPoliticsFactionTileCount));
    assert(
        Widget.mPoliticsNeutralTexts.size() ==
        static_cast<size_t>(GPoliticsNeutralCount));
    assert(
        Widget.mPoliticsSupportRows.size() ==
        static_cast<size_t>(GPoliticsSupportRowCount));
    assert(
        Widget.mPoliticsDetails.size() ==
        static_cast<size_t>(GPoliticsDetailCount));
    assert(
        Widget.mForeignRows.size() ==
        static_cast<size_t>(GForeignPowerCount));
    assert(
        Widget.mForeignDetails.size() ==
        static_cast<size_t>(GForeignDetailCount));
    assert(
        Widget.mForeignMetrics.size() ==
        static_cast<size_t>(GForeignMetricCount));
    assert(
        Widget.mBuildingRows.size() ==
        static_cast<size_t>(GBuildingRowCount));
    assert(
        Widget.mBuildingDetails.size() ==
        static_cast<size_t>(GBuildingDetailCount));
    assert(
        Widget.mConflictDetails.size() ==
        static_cast<size_t>(GConflictDetailCount));
    assert(
        Widget.mConflictMetrics.size() ==
        static_cast<size_t>(GConflictMetricCount));
#else
    (void)Widget;
    (void)Layout;
#endif
}

void FAlmanacRenderer::RefreshChromeLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    if (auto Background = Widget.mPanelBackground.lock())
    {
        Background->SetPos(Layout.PanelLeft, Layout.PanelTop);
        Background->SetSize(Layout.PanelWidth, Layout.PanelHeight);
    }

    if (auto ContentFrame = Widget.mContentFrame.lock())
    {
        ContentFrame->SetPos(Layout.FrameLeft, Layout.FrameTop);
        ContentFrame->SetSize(Layout.FrameWidth, Layout.FrameHeight);
    }

    if (auto TitleRibbon = Widget.mTitleRibbon.lock())
    {
        TitleRibbon->SetPos(Layout.RibbonLeft, Layout.RibbonTop);
        TitleRibbon->SetSize(Layout.RibbonWidth, Layout.RibbonHeight);
    }

    if (auto RailTrack = Widget.mLeftRailTrack.lock())
    {
        RailTrack->SetPos(Layout.RailTrackLeft, Layout.RailTrackTop);
        RailTrack->SetSize(Layout.RailTrackWidth, Layout.RailTrackHeight);
    }

    if (auto RailThumb = Widget.mLeftRailThumb.lock())
    {
        RailThumb->SetPos(
            Layout.RailTrackLeft - Layout.Chrome.RailThumbExpand,
            Layout.RailTrackTop + Layout.Chrome.RailThumbTopOffset);
        RailThumb->SetSize(
            Layout.RailTrackWidth + Layout.Chrome.RailThumbExpand * 2.f,
            Layout.RailThumbHeight);
    }

    if (auto TitleText = Widget.mTitleText.lock())
    {
        TitleText->SetFontSize(UIConfig::AlmanacTitleFontSize * Layout.Scale);
        TitleText->SetPos(
            Layout.RibbonLeft + Layout.Chrome.TitlePaddingX,
            Layout.RibbonTop + Layout.Chrome.TitlePaddingY);
        TitleText->SetSize(
            Layout.RibbonWidth - Layout.Chrome.TitlePaddingX * 2.f,
            Layout.RibbonHeight - Layout.Chrome.TitlePaddingY * 2.f);
    }

    if (auto CloseButton = Widget.mCloseButton.lock())
    {
        CloseButton->SetPos(
            Layout.PanelLeft + Layout.PanelWidth - Layout.HeaderPadding -
                Layout.Chrome.CloseButtonOffsetX,
            Layout.PanelTop + Layout.Chrome.CloseButtonOffsetY);
        CloseButton->SetSize(
            Layout.Chrome.CloseButtonSize,
            Layout.Chrome.CloseButtonSize);
    }
}

void FAlmanacRenderer::RefreshNavigationLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    const float TabsWidth =
        Layout.TabSize * static_cast<float>(Widget.mTabButtons.size()) +
        Layout.TabGap * static_cast<float>((std::max)(
            0,
            static_cast<int>(Widget.mTabButtons.size()) - 1));
    const float TabsStartX =
        Layout.PanelLeft + (Layout.PanelWidth - TabsWidth) * 0.5f;

    for (size_t Index = 0; Index < Widget.mTabButtons.size(); ++Index)
    {
        auto Button = Widget.mTabButtons[Index].lock();
        if (!Button)
            continue;

        const bool Selected = Index == static_cast<size_t>(Widget.mSelectedPage);
        const float OffsetY = Selected ? Layout.TabSelectedOffsetY : 0.f;

        Button->SetPos(
            TabsStartX + (Layout.TabSize + Layout.TabGap) * static_cast<float>(Index),
            Layout.TabBaseY + OffsetY);
        Button->SetSize(Layout.TabSize, Layout.TabSize);
    }

    if (auto TabMarker = Widget.mTabMarker.lock())
    {
        const size_t SelectedIndex = static_cast<size_t>(Widget.mSelectedPage);
        const float SelectedX =
            TabsStartX +
            (Layout.TabSize + Layout.TabGap) * static_cast<float>(SelectedIndex);
        TabMarker->SetPos(
            SelectedX + Layout.TabSize * 0.5f -
                GAlmanacTabMarkerWidth * 0.5f * Layout.Scale,
            Layout.PanelTop - GAlmanacTabMarkerTopOffset * Layout.Scale);
        TabMarker->SetSize(
            GAlmanacTabMarkerWidth * Layout.Scale,
            GAlmanacTabMarkerHeight * Layout.Scale);
    }
}

void FAlmanacRenderer::RefreshPageContainerLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    for (size_t Index = 0; Index < Widget.mPages.size(); ++Index)
    {
        auto Page = Widget.mPages[Index].lock();
        if (!Page)
            continue;

        Page->SetPos(Layout.ContentLeft, Layout.ContentTop);
        Page->SetSize(Layout.ContentWidth, Layout.ContentHeight);
    }
}

void FAlmanacRenderer::RefreshOverviewLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    const float Scale = Layout.Scale;
    const float ContentWidth = Layout.ContentWidth;
    const float ContentHeight = Layout.ContentHeight;
    const float OverviewGroupGap = 24.f * Scale;
    const float OverviewInnerGap = 10.f * Scale;
    const float OverviewSectionTitleTop = 6.f * Scale;
    const float OverviewSectionTitleHeight = 26.f * Scale;
    const float OverviewTopGroupWidth =
        (ContentWidth - OverviewGroupGap * 2.f) / 3.f;
    const float OverviewTopCardWidth =
        (OverviewTopGroupWidth - OverviewInnerGap) * 0.5f;
    const float OverviewTopCardHeight = 170.f * Scale;
    const float OverviewTopCardY = 34.f * Scale;
    const float OverviewBottomTitleY = OverviewTopCardY + OverviewTopCardHeight + 18.f * Scale;
    const float OverviewBottomCardY = OverviewBottomTitleY + 30.f * Scale;
    const float OverviewBottomCardHeight = 170.f * Scale;
    const float OverviewCenterCardWidth = 142.f * Scale;
    const float OverviewElectionTextWidth = 144.f * Scale;
    const float OverviewElectionY = ContentHeight - 50.f * Scale;
    const float OverviewArrowSize = 16.f * Scale;
    const float HiddenX = ResolveOffscreenHiddenCoord(ContentWidth, Scale);
    const float HiddenY = ResolveOffscreenHiddenCoord(ContentHeight, Scale);
    const float GroupX0 = 0.f;
    const float GroupX1 = OverviewTopGroupWidth + OverviewGroupGap;
    const float GroupX2 = (OverviewTopGroupWidth + OverviewGroupGap) * 2.f;

    if (Widget.mOverviewCards.size() >= GOverviewCardCount)
    {
        LayoutOverviewCard(
            Scale,
            Widget.mOverviewCards[0],
            GroupX0,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);
        LayoutOverviewCard(
            Scale,
            Widget.mOverviewCards[1],
            GroupX0 + OverviewTopCardWidth + OverviewInnerGap,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);
        LayoutOverviewCard(
            Scale,
            Widget.mOverviewCards[2],
            GroupX1,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);
        LayoutOverviewCard(
            Scale,
            Widget.mOverviewCards[3],
            GroupX1 + OverviewTopCardWidth + OverviewInnerGap,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);
        LayoutOverviewCard(
            Scale,
            Widget.mOverviewCards[4],
            GroupX2,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);
        LayoutOverviewCard(
            Scale,
            Widget.mOverviewCards[5],
            GroupX2 + OverviewTopCardWidth + OverviewInnerGap,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);

        LayoutOverviewCard(
            Scale,
            Widget.mOverviewCards[6],
            GroupX0,
            OverviewBottomCardY,
            OverviewTopCardWidth,
            OverviewBottomCardHeight);
        LayoutOverviewCard(
            Scale,
            Widget.mOverviewCards[7],
            GroupX0 + OverviewTopCardWidth + OverviewInnerGap,
            OverviewBottomCardY,
            OverviewTopCardWidth,
            OverviewBottomCardHeight);
        LayoutOverviewCard(
            Scale,
            Widget.mOverviewCards[8],
            ContentWidth * 0.5f - OverviewCenterCardWidth * 0.5f,
            OverviewBottomCardY,
            OverviewCenterCardWidth,
            OverviewBottomCardHeight);
        LayoutOverviewCard(
            Scale,
            Widget.mOverviewCards[9],
            GroupX2,
            OverviewBottomCardY,
            OverviewTopCardWidth,
            OverviewBottomCardHeight);
        LayoutOverviewCard(
            Scale,
            Widget.mOverviewCards[10],
            GroupX2 + OverviewTopCardWidth + OverviewInnerGap,
            OverviewBottomCardY,
            OverviewTopCardWidth,
            OverviewBottomCardHeight);
    }

    if (Widget.mOverviewSectionTitles.size() >= GOverviewSectionTitleCount)
    {
        if (auto Text = Widget.mOverviewSectionTitles[0].lock())
        {
            Text->SetPos(GroupX0, OverviewSectionTitleTop);
            Text->SetSize(OverviewTopGroupWidth, OverviewSectionTitleHeight);
        }
        if (auto Text = Widget.mOverviewSectionTitles[1].lock())
        {
            Text->SetPos(GroupX1, OverviewSectionTitleTop);
            Text->SetSize(OverviewTopGroupWidth, OverviewSectionTitleHeight);
        }
        if (auto Text = Widget.mOverviewSectionTitles[2].lock())
        {
            Text->SetPos(GroupX2, OverviewSectionTitleTop);
            Text->SetSize(OverviewTopGroupWidth, OverviewSectionTitleHeight);
        }
        if (auto Text = Widget.mOverviewSectionTitles[3].lock())
        {
            Text->SetPos(GroupX0, OverviewBottomTitleY);
            Text->SetSize(OverviewTopGroupWidth, OverviewSectionTitleHeight);
        }
        if (auto Text = Widget.mOverviewSectionTitles[4].lock())
        {
            Text->SetPos(
                ContentWidth * 0.5f - OverviewCenterCardWidth * 0.5f,
                OverviewBottomTitleY);
            Text->SetSize(OverviewCenterCardWidth, OverviewSectionTitleHeight);
        }
        if (auto Text = Widget.mOverviewSectionTitles[5].lock())
        {
            Text->SetPos(GroupX2, OverviewBottomTitleY);
            Text->SetSize(OverviewTopGroupWidth, OverviewSectionTitleHeight);
        }
    }

    if (auto Arrow = Widget.mOverviewElectionLeftArrow.lock())
    {
        Arrow->SetPos(
            ContentWidth * 0.5f - OverviewElectionTextWidth * 0.5f -
                OverviewArrowSize - 8.f * Scale,
            OverviewElectionY + 8.f * Scale);
        Arrow->SetSize(OverviewArrowSize, OverviewArrowSize);
    }
    if (auto Arrow = Widget.mOverviewElectionRightArrow.lock())
    {
        Arrow->SetPos(
            ContentWidth * 0.5f + OverviewElectionTextWidth * 0.5f +
                8.f * Scale,
            OverviewElectionY + 8.f * Scale);
        Arrow->SetSize(OverviewArrowSize, OverviewArrowSize);
    }
    if (auto Text = Widget.mOverviewElectionText.lock())
    {
        Text->SetPos(
            ContentWidth * 0.5f - OverviewElectionTextWidth * 0.5f,
            OverviewElectionY);
        Text->SetSize(OverviewElectionTextWidth, 42.f * Scale);
    }

    if (auto SumL = Widget.mOverviewSummaryLeft.lock())
    {
        SumL->SetPos(HiddenX, HiddenY);
        SumL->SetSize(1.f, 1.f);
    }
    if (auto SumR = Widget.mOverviewSummaryRight.lock())
    {
        SumR->SetPos(HiddenX, HiddenY);
        SumR->SetSize(1.f, 1.f);
    }

}

void FAlmanacRenderer::RefreshSatisfactionLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    const float Scale = Layout.Scale;
    const float ContentWidth = Layout.ContentWidth;
    const float ContentHeight = Layout.ContentHeight;
    const float HiddenY = ResolveOffscreenHiddenCoord(ContentHeight, Scale);
    const auto& PageMetrics = Layout.PageMetrics;
    // 만족도
    const float SatisfactionLeftWide = ContentWidth * 0.46f;
    const float SatisfactionRightX = SatisfactionLeftWide + PageMetrics.WideColumnGap;
    const float SatisfactionRightW = ContentWidth - SatisfactionRightX;
    const float SatisfactionRowHeight = 44.f * Scale;
    const float SatisfactionRowGap = 5.f * Scale;
    const float SatisfactionDetailRowHeight = 36.f * Scale;
    const float SatisfactionDetailGap = 4.f * Scale;
    const float SatisfactionChartTitleTop = 0.f;
    const float SatisfactionChartTitleHeight = PageMetrics.TitleHeight;
    const float SatisfactionChartFrameTop = PageMetrics.FrameTop;
    const float SatisfactionChartFrameHeight = 206.f * Scale;
    const float SatisfactionDetailTop =
        SatisfactionChartFrameTop + SatisfactionChartFrameHeight + 12.f * Scale;

    if (auto ListTitleBackground = Widget.mSatisfactionListTitleBackground.lock())
    {
        ListTitleBackground->SetPos(0.f, SatisfactionChartTitleTop + 2.f * Scale);
        ListTitleBackground->SetSize(SatisfactionLeftWide, SatisfactionChartTitleHeight - 2.f * Scale);
    }

    if (auto ListTitle = Widget.mSatisfactionListTitle.lock())
    {
        ListTitle->SetPos(0.f, SatisfactionChartTitleTop);
        ListTitle->SetSize(SatisfactionLeftWide, SatisfactionChartTitleHeight);
    }

    LayoutSatisfactionRows(Scale, 
        Widget.mSatisfactionRows,
        0.f,
        SatisfactionChartFrameTop,
        SatisfactionLeftWide,
        SatisfactionRowHeight,
        SatisfactionRowGap);

    if (auto ChartTitleBackground = Widget.mSatisfactionChartTitleBackground.lock())
    {
        ChartTitleBackground->SetPos(
            SatisfactionRightX,
            SatisfactionChartTitleTop + 2.f * Scale);
        ChartTitleBackground->SetSize(
            SatisfactionRightW,
            SatisfactionChartTitleHeight - 2.f * Scale);
    }

    if (auto ChartTitle = Widget.mSatisfactionChartTitle.lock())
    {
        ChartTitle->SetPos(SatisfactionRightX, SatisfactionChartTitleTop);
        ChartTitle->SetSize(SatisfactionRightW, SatisfactionChartTitleHeight);
    }

    if (auto ChartFrame = Widget.mSatisfactionChartFrame.lock())
    {
        ChartFrame->SetPos(SatisfactionRightX, SatisfactionChartFrameTop);
        ChartFrame->SetSize(SatisfactionRightW, SatisfactionChartFrameHeight);
    }

    const float GraphLeft = SatisfactionRightX + 42.f * Scale;
    const float GraphTop = SatisfactionChartFrameTop + 24.f * Scale;
    const float GraphWidth = SatisfactionRightW - 58.f * Scale;
    const float GraphHeight = SatisfactionChartFrameHeight - 56.f * Scale;
    const float GraphBottom = GraphTop + GraphHeight;
    const float GraphGridGapY =
        GSatisfactionGraphGridLineCount > 1 ?
        GraphHeight /
            static_cast<float>(GSatisfactionGraphGridLineCount - 1) :
        GraphHeight;
    const float GraphPointGapX =
        GSatisfactionGraphPointCount > 1 ?
        GraphWidth /
            static_cast<float>(GSatisfactionGraphPointCount - 1) :
        GraphWidth;

    for (int Index = 0; Index < GSatisfactionGraphGridLineCount; ++Index)
    {
        const float LineY =
            GraphTop + GraphHeight -
            GraphGridGapY * static_cast<float>(Index);

        if (Index < static_cast<int>(Widget.mSatisfactionChartGridLines.size()))
        {
            if (auto GridLine = Widget.mSatisfactionChartGridLines[Index].lock())
            {
                GridLine->SetPos(GraphLeft, LineY);
                GridLine->SetSize(GraphWidth, 1.f * Scale);
            }
        }

        if (Index < static_cast<int>(Widget.mSatisfactionChartYAxisLabels.size()))
        {
            if (auto YLabel = Widget.mSatisfactionChartYAxisLabels[Index].lock())
            {
                YLabel->SetFontSize(11.f * Scale);
                YLabel->SetPos(
                    GraphLeft - 28.f * Scale,
                    LineY - 8.f * Scale);
                YLabel->SetSize(24.f * Scale, 16.f * Scale);
            }
        }
    }

    if (auto YAxisLine = Widget.mSatisfactionChartYAxisLine.lock())
    {
        YAxisLine->SetPos(GraphLeft - 1.f * Scale, GraphTop - 4.f * Scale);
        YAxisLine->SetSize(1.f * Scale, GraphHeight + 6.f * Scale);
    }

    if (auto XAxisLine = Widget.mSatisfactionChartXAxisLine.lock())
    {
        XAxisLine->SetPos(GraphLeft, GraphBottom - 1.f * Scale);
        XAxisLine->SetSize(GraphWidth + 4.f * Scale, 1.f * Scale);
    }

    if (auto YAxisArrow = Widget.mSatisfactionChartYAxisArrow.lock())
    {
        YAxisArrow->SetPos(GraphLeft - 1.f * Scale, GraphTop - 8.f * Scale);
        YAxisArrow->SetSize(12.f * Scale, 12.f * Scale);
    }

    if (auto XAxisArrow = Widget.mSatisfactionChartXAxisArrow.lock())
    {
        XAxisArrow->SetPos(
            GraphLeft + GraphWidth + 4.f * Scale,
            GraphBottom - 1.f * Scale);
        XAxisArrow->SetSize(12.f * Scale, 12.f * Scale);
    }

    for (int Index = 0; Index < GSatisfactionGraphPointCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mSatisfactionChartXAxisLabels.size()))
            continue;

        auto XLabel = Widget.mSatisfactionChartXAxisLabels[Index].lock();

        if (!XLabel)
            continue;

        XLabel->SetFontSize(10.5f * Scale);
        XLabel->SetPos(
            GraphLeft + GraphPointGapX * static_cast<float>(Index) -
                24.f * Scale,
            GraphBottom + 7.f * Scale);
        XLabel->SetSize(48.f * Scale, 16.f * Scale);
    }

    if (!Widget.mSatisfactionRows.empty())
    {
        const int SelectedIndex = (std::max)(
            0,
            (std::min)(
                static_cast<int>(Widget.mSatisfactionRows.size()) - 1,
                Widget.mSelectedSatisfactionIndex));
        const float TooltipWidth =
            (std::min)(268.f * Scale, ContentWidth * 0.32f);
        const float TooltipHeight = 160.f * Scale;
        const float TooltipX =
            SatisfactionLeftWide * 0.46f;
        float TooltipY = SatisfactionChartFrameTop + 46.f * Scale;

        if (auto SelectedButton =
            Widget.mSatisfactionRows[static_cast<size_t>(SelectedIndex)].Button.lock())
        {
            TooltipY =
                SelectedButton->GetPos().y +
                SelectedButton->GetSize().y +
                2.f * Scale;
        }

        TooltipY = (std::max)(8.f * Scale,
            (std::min)(
                TooltipY,
                ContentHeight - TooltipHeight - 8.f * Scale));

        if (auto TooltipPanel = Widget.mSatisfactionTooltipPanel.lock())
        {
            TooltipPanel->SetPos(TooltipX, TooltipY);
            TooltipPanel->SetSize(TooltipWidth, TooltipHeight);
        }

        if (auto TooltipText = Widget.mSatisfactionTooltipText.lock())
        {
            TooltipText->SetFontSize(12.6f * Scale);
            TooltipText->SetPos(
                TooltipX + 12.f * Scale,
                TooltipY + 10.f * Scale);
            TooltipText->SetSize(
                TooltipWidth - 24.f * Scale,
                TooltipHeight - 18.f * Scale);
        }
    }

    LayoutDetailRows(Scale, 
        Widget.mSatisfactionDetails,
        SatisfactionRightX,
        SatisfactionDetailTop,
        SatisfactionRightW,
        SatisfactionDetailRowHeight,
        SatisfactionDetailGap);

    if (Widget.mSelectedSatisfactionIndex == 1)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mSatisfactionDetails.size()); ++Index)
        {
            auto Label = Widget.mSatisfactionDetails[static_cast<size_t>(Index)].Label.lock();
            auto Value = Widget.mSatisfactionDetails[static_cast<size_t>(Index)].Value.lock();

            if (Label)
            {
                const FVector3 LabelPos = Label->GetPos();
                const float ExtraIndent =
                    Index >= 3 ? 28.f * Scale :
                    (Index >= 1 ? 4.f * Scale : 0.f);
                Label->SetPos(LabelPos.x + ExtraIndent, LabelPos.y);
            }

            if (Value)
            {
                const FVector3 ValuePos = Value->GetPos();
                Value->SetPos(ValuePos.x - 4.f * Scale, ValuePos.y);
            }
        }
    }

}

void FAlmanacRenderer::RefreshPopulationLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    const float Scale = Layout.Scale;
    const float ContentWidth = Layout.ContentWidth;
    const float ContentHeight = Layout.ContentHeight;
    const float HiddenX = ResolveOffscreenHiddenCoord(ContentWidth, Scale);
    const float HiddenY = ResolveOffscreenHiddenCoord(ContentHeight, Scale);
    const auto& PageMetrics = Layout.PageMetrics;
    // 국민
    const int SelectedPopulationIndex =
        (std::max)(0,
            (std::min)(
                static_cast<int>(Widget.mPopulationDetails.size()) - 1,
                Widget.mSelectedPopulationIndex));
    const bool ShowPopulationOverviewCharts =
        SelectedPopulationIndex != 1 &&
        SelectedPopulationIndex != 2 &&
        SelectedPopulationIndex != 3 &&
        SelectedPopulationIndex != 4 &&
        SelectedPopulationIndex != 5 &&
        SelectedPopulationIndex != 6 &&
        SelectedPopulationIndex != 7 &&
        SelectedPopulationIndex != 8 &&
        SelectedPopulationIndex != 9 &&
        SelectedPopulationIndex != 10 &&
        SelectedPopulationIndex != 11 &&
        SelectedPopulationIndex != 12 &&
        SelectedPopulationIndex != 13;
    const bool ShowPopulationAgeBreakdown =
        SelectedPopulationIndex == 3;
    const bool ShowPopulationStackedMetrics =
        SelectedPopulationIndex == 3 ||
        SelectedPopulationIndex == 4 ||
        SelectedPopulationIndex == 7 ||
        SelectedPopulationIndex == 10 ||
        SelectedPopulationIndex == 11 ||
        SelectedPopulationIndex == 12 ||
        SelectedPopulationIndex == 13;
    const bool ShowPopulationLowerStackedMetrics =
        SelectedPopulationIndex == 5 ||
        SelectedPopulationIndex == 6 ||
        SelectedPopulationIndex == 8 ||
        SelectedPopulationIndex == 9;
    const float PopulationLeftWide = ContentWidth * 0.47f;
    const float PopulationRightX = PopulationLeftWide + PageMetrics.ColumnGap;
    const float PopulationRightW = ContentWidth - PopulationRightX;
    const float PopulationDetailRowHeight = 31.f * Scale;
    const float PopulationDetailGap = 3.f * Scale;
    LayoutDetailRows(Scale, 
        Widget.mPopulationDetails,
        0.f,
        0.f,
        PopulationLeftWide,
        PopulationDetailRowHeight,
        PopulationDetailGap);

    for (size_t Index = 0; Index < Widget.mPopulationDetails.size(); ++Index)
    {
        auto Label = Widget.mPopulationDetails[Index].Label.lock();
        auto Value = Widget.mPopulationDetails[Index].Value.lock();

        if (Label)
        {
            Label->SetFontSize(15.f * Scale);
            if (Index == 3 || Index == 10 || Index == 11 || Index == 12)
                Label->SetTextColor(92, 84, 66, 255);
        }

        if (Value)
            Value->SetFontSize(15.f * Scale);
    }

    const float PopulationTitleHeight = PageMetrics.TitleHeight;
    const float PopulationTrendFrameTop = PageMetrics.FrameTop;
    const float PopulationTrendFrameHeight = 206.f * Scale;
    const float PopulationSummaryTop =
        PopulationTrendFrameTop + PopulationTrendFrameHeight + 8.f * Scale;
    const float PopulationSummaryHeight = 32.f * Scale;
    const float PopulationChangeSummaryHeight = 32.f * Scale;
    const float PopulationChangeTitleTop =
        SelectedPopulationIndex == 10 ?
            PopulationSummaryTop +
                3.f * (PopulationChangeSummaryHeight + 4.f * Scale) +
                8.f * Scale :
        SelectedPopulationIndex == 13 ?
            PopulationSummaryTop +
                2.f * (PopulationChangeSummaryHeight + 4.f * Scale) +
                8.f * Scale :
            PopulationSummaryTop + PopulationSummaryHeight + 14.f * Scale;
    const float PopulationChangeFrameTop =
        PopulationChangeTitleTop + PageMetrics.FrameTop;
    const float PopulationChangeFrameHeight =
        ShowPopulationOverviewCharts ? 150.f * Scale : 0.f;
    const float PopulationChangeSummaryTop =
        ShowPopulationOverviewCharts ?
            PopulationChangeFrameTop + PopulationChangeFrameHeight + 8.f * Scale :
            PopulationChangeTitleTop + PopulationTitleHeight + 12.f * Scale;

    if (auto TrendTitleBackground = Widget.mPopulationTrendTitleBackground.lock())
    {
        TrendTitleBackground->SetPos(
            PopulationRightX,
            2.f * Scale);
        TrendTitleBackground->SetSize(
            PopulationRightW,
            PopulationTitleHeight - 2.f * Scale);
    }

    if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
    {
        TrendTitle->SetPos(PopulationRightX, 0.f);
        TrendTitle->SetSize(PopulationRightW, PopulationTitleHeight);
    }

    if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
    {
        TrendFrame->SetPos(PopulationRightX, PopulationTrendFrameTop);
        TrendFrame->SetSize(PopulationRightW, PopulationTrendFrameHeight);
    }

    const float PopulationTrendGraphLeft =
        PopulationRightX + 36.f * Scale;
    const float PopulationTrendGraphTop =
        PopulationTrendFrameTop + 18.f * Scale;
    const float PopulationTrendGraphWidth =
        PopulationRightW - 54.f * Scale;
    const float PopulationTrendGraphHeight =
        PopulationTrendFrameHeight - 46.f * Scale;

    for (int Index = 0; Index < GPopulationTrendGridLineCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPopulationTrendGridLines.size()))
            break;

        if (auto GridLine = Widget.mPopulationTrendGridLines[Index].lock())
        {
            const float Fraction =
                static_cast<float>(Index) /
                static_cast<float>((std::max)(1, GPopulationTrendGridLineCount - 1));
            const float LineY =
                PopulationTrendGraphTop +
                PopulationTrendGraphHeight * (1.f - Fraction);
            GridLine->SetPos(PopulationTrendGraphLeft, LineY);
            GridLine->SetSize(PopulationTrendGraphWidth, 1.2f * Scale);
        }
    }

    for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
            break;

        if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
        {
            const float Fraction =
                static_cast<float>(Index) /
                static_cast<float>((std::max)(1, GPopulationTrendYAxisLabelCount - 1));
            const float LabelY =
                PopulationTrendGraphTop +
                PopulationTrendGraphHeight -
                PopulationTrendGraphHeight * Fraction -
                8.f * Scale;
            YLabel->SetPos(PopulationRightX - 6.f * Scale, LabelY);
            YLabel->SetSize(48.f * Scale, 16.f * Scale);
            YLabel->SetFontSize(11.f * Scale);
        }
    }

    if (auto YAxisLine = Widget.mPopulationTrendYAxisLine.lock())
    {
        YAxisLine->SetPos(PopulationTrendGraphLeft, PopulationTrendGraphTop - 6.f * Scale);
        YAxisLine->SetSize(1.6f * Scale, PopulationTrendGraphHeight + 8.f * Scale);
    }

    if (auto XAxisLine = Widget.mPopulationTrendXAxisLine.lock())
    {
        XAxisLine->SetPos(PopulationTrendGraphLeft, PopulationTrendGraphTop + PopulationTrendGraphHeight);
        XAxisLine->SetSize(PopulationTrendGraphWidth, 1.6f * Scale);
    }

    if (auto YAxisArrow = Widget.mPopulationTrendYAxisArrow.lock())
    {
        YAxisArrow->SetPos(
            PopulationTrendGraphLeft + 0.8f * Scale,
            PopulationTrendGraphTop - 8.f * Scale);
        YAxisArrow->SetSize(12.f * Scale, 12.f * Scale);
    }

    if (auto XAxisArrow = Widget.mPopulationTrendXAxisArrow.lock())
    {
        XAxisArrow->SetPos(
            PopulationTrendGraphLeft + PopulationTrendGraphWidth + 2.f * Scale,
            PopulationTrendGraphTop + PopulationTrendGraphHeight + 1.f * Scale);
        XAxisArrow->SetSize(12.f * Scale, 12.f * Scale);
    }

    for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
            break;

        auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock();

        if (!XLabel)
            continue;

        const float Fraction =
            GPopulationTrendXAxisLabelCount <= 1 ?
                0.f :
                static_cast<float>(Index) /
                static_cast<float>(GPopulationTrendXAxisLabelCount - 1);
        XLabel->SetPos(
            PopulationTrendGraphLeft +
                PopulationTrendGraphWidth * Fraction -
                40.f * Scale,
            PopulationTrendGraphTop + PopulationTrendGraphHeight + 8.f * Scale);
        XLabel->SetSize(80.f * Scale, 18.f * Scale);
        XLabel->SetFontSize(11.f * Scale);
    }

    const float PopulationSummaryLabelWidth = PopulationRightW * 0.42f;
    const float PopulationSummaryValueWidth = 90.f * Scale;
    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
    {
        const float RowY =
            ShowPopulationStackedMetrics ?
                PopulationSummaryTop +
                    static_cast<float>(Index) *
                        (PopulationChangeSummaryHeight + 4.f * Scale) :
            ShowPopulationLowerStackedMetrics ?
                PopulationChangeSummaryTop +
                    static_cast<float>(Index) *
                        (PopulationChangeSummaryHeight + 4.f * Scale) :
            Index == 0 ?
                PopulationSummaryTop :
                PopulationChangeSummaryTop +
                    static_cast<float>(Index - 1) *
                        (PopulationChangeSummaryHeight + 4.f * Scale);
        const float RowHeight =
            ShowPopulationStackedMetrics ||
            ShowPopulationLowerStackedMetrics ?
                PopulationChangeSummaryHeight :
            Index == 0 ? PopulationSummaryHeight : PopulationChangeSummaryHeight;
        auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];

        if (auto Bg = Row.Background.lock())
        {
            Bg->SetPos(PopulationRightX, RowY);
            Bg->SetSize(PopulationRightW, RowHeight);
        }
        if (auto Lbl = Row.Label.lock())
        {
            Lbl->SetPos(PopulationRightX + 12.f * Scale, RowY);
            Lbl->SetSize(PopulationSummaryLabelWidth, RowHeight);
            Lbl->SetFontSize(15.f * Scale);
        }
        if (auto Bar = Row.Bar.lock())
        {
            Bar->SetEnable(false);
        }
        if (auto Val = Row.Value.lock())
        {
            Val->SetPos(
                PopulationRightX + PopulationRightW - PopulationSummaryValueWidth - 10.f * Scale,
                RowY);
            Val->SetSize(PopulationSummaryValueWidth, RowHeight);
            Val->SetFontSize(15.f * Scale);
        }
    }

    if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
    {
        ChangeTitleBackground->SetPos(
            PopulationRightX,
            PopulationChangeTitleTop + 2.f * Scale);
        ChangeTitleBackground->SetSize(
            PopulationRightW,
            PopulationTitleHeight - 2.f * Scale);
    }

    if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
    {
        ChangeTitle->SetPos(PopulationRightX, PopulationChangeTitleTop);
        ChangeTitle->SetSize(PopulationRightW, PopulationTitleHeight);
    }

    if (auto ChangeFrame = Widget.mPopulationChangeFrame.lock())
    {
        ChangeFrame->SetPos(PopulationRightX, PopulationChangeFrameTop);
        ChangeFrame->SetSize(PopulationRightW, PopulationChangeFrameHeight);
    }

    const float PopulationChangeGraphLeft =
        PopulationRightX + 36.f * Scale;
    const float PopulationChangeGraphTop =
        PopulationChangeFrameTop + 14.f * Scale;
    const float PopulationChangeGraphWidth =
        PopulationRightW - 54.f * Scale;
    const float PopulationChangeGraphHeight =
        PopulationChangeFrameHeight - 32.f * Scale;
    const float PopulationChangeZeroY =
        PopulationChangeGraphTop + PopulationChangeGraphHeight * 0.54f;

    for (int Index = 0; Index < GPopulationChangeGridLineCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPopulationChangeGridLines.size()))
            break;

        if (auto GridLine = Widget.mPopulationChangeGridLines[Index].lock())
        {
            const float Fraction =
                static_cast<float>(Index) /
                static_cast<float>((std::max)(1, GPopulationChangeGridLineCount - 1));
            const float LineY =
                PopulationChangeGraphTop +
                PopulationChangeGraphHeight * (1.f - Fraction);
            GridLine->SetPos(PopulationChangeGraphLeft, LineY);
            GridLine->SetSize(PopulationChangeGraphWidth, 1.2f * Scale);
        }
    }

    for (int Index = 0; Index < GPopulationChangeYAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPopulationChangeYAxisLabels.size()))
            break;

        if (auto YLabel = Widget.mPopulationChangeYAxisLabels[Index].lock())
        {
            const float Fraction =
                static_cast<float>(Index) /
                static_cast<float>((std::max)(1, GPopulationChangeYAxisLabelCount - 1));
            const float LabelY =
                PopulationChangeGraphTop +
                PopulationChangeGraphHeight -
                PopulationChangeGraphHeight * Fraction -
                8.f * Scale;
            YLabel->SetPos(PopulationRightX - 6.f * Scale, LabelY);
            YLabel->SetSize(48.f * Scale, 16.f * Scale);
            YLabel->SetFontSize(11.f * Scale);
        }
    }

    if (auto YAxisLine = Widget.mPopulationChangeYAxisLine.lock())
    {
        YAxisLine->SetPos(PopulationChangeGraphLeft, PopulationChangeGraphTop - 4.f * Scale);
        YAxisLine->SetSize(1.6f * Scale, PopulationChangeGraphHeight + 6.f * Scale);
    }

    if (auto XAxisLine = Widget.mPopulationChangeXAxisLine.lock())
    {
        XAxisLine->SetPos(PopulationChangeGraphLeft, PopulationChangeZeroY);
        XAxisLine->SetSize(PopulationChangeGraphWidth, 1.6f * Scale);
    }

    if (auto YAxisArrow = Widget.mPopulationChangeYAxisArrow.lock())
    {
        YAxisArrow->SetPos(
            PopulationChangeGraphLeft + 0.8f * Scale,
            PopulationChangeGraphTop - 6.f * Scale);
        YAxisArrow->SetSize(12.f * Scale, 12.f * Scale);
    }

    if (auto XAxisArrow = Widget.mPopulationChangeXAxisArrow.lock())
    {
        XAxisArrow->SetPos(
            PopulationChangeGraphLeft + PopulationChangeGraphWidth + 2.f * Scale,
            PopulationChangeZeroY + 1.f * Scale);
        XAxisArrow->SetSize(12.f * Scale, 12.f * Scale);
    }

    for (int Index = 0; Index < GPopulationChangeXAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPopulationChangeXAxisLabels.size()))
            break;

        auto XLabel = Widget.mPopulationChangeXAxisLabels[Index].lock();

        if (!XLabel)
            continue;

        const float Fraction =
            GPopulationChangeXAxisLabelCount <= 1 ?
                0.f :
                static_cast<float>(Index) /
                static_cast<float>(GPopulationChangeXAxisLabelCount - 1);
        XLabel->SetPos(
            PopulationChangeGraphLeft +
                PopulationChangeGraphWidth * Fraction -
                40.f * Scale,
            PopulationChangeFrameTop + PopulationChangeFrameHeight - 8.f * Scale);
        XLabel->SetSize(80.f * Scale, 18.f * Scale);
        XLabel->SetFontSize(11.f * Scale);
    }

}

void FAlmanacRenderer::RefreshEconomyLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    const float Scale = Layout.Scale;
    const float ContentWidth = Layout.ContentWidth;
    const float ContentHeight = Layout.ContentHeight;
    const float HiddenY = ResolveOffscreenHiddenCoord(ContentHeight, Scale);
    const auto& PageMetrics = Layout.PageMetrics;
    // 경제
    const float EconomyLeftWide = ContentWidth * 0.47f;
    const float EconomyRightX = EconomyLeftWide + PageMetrics.ColumnGap;
    const float EconomyRightW = ContentWidth - EconomyRightX;
    const float EconomyDetailRowHeight = 31.f * Scale;
    const float EconomyDetailGap = 3.f * Scale;
    const bool ShowEconomyCorruptionScreen = Widget.mSelectedEconomyIndex == 4;
    const bool ShowEconomyProductionScreen = Widget.mSelectedEconomyIndex == 5;
    const bool ShowEconomyTouristRatingScreen = Widget.mSelectedEconomyIndex == 7;
    const bool ShowEconomyTouristCapacityScreen = Widget.mSelectedEconomyIndex == 8;
    const bool ShowEconomyLaborScreen = Widget.mSelectedEconomyIndex == 9;
    const bool ShowEconomyUnemployedScreen = Widget.mSelectedEconomyIndex == 10;
    const bool ShowEconomyVacancyScreen = Widget.mSelectedEconomyIndex == 11;
    const bool ShowEconomyElectricityScreen = Widget.mSelectedEconomyIndex == 12;
    LayoutDetailRows(Scale, 
        Widget.mEconomyDetails,
        0.f,
        0.f,
        EconomyLeftWide,
        EconomyDetailRowHeight,
        EconomyDetailGap);

    const float EconomyTitleHeight = PageMetrics.TitleHeight;
    const float EconomyTrendFrameTop = PageMetrics.FrameTop;
    const float EconomyTrendFrameHeight = 146.f * Scale;
    const float EconomyMidSummaryTop =
        EconomyTrendFrameTop + EconomyTrendFrameHeight + 8.f * Scale;
    const float EconomySummaryHeight = 32.f * Scale;
    const float EconomyCorruptionTop = 2.f * Scale;
    const float EconomyCorruptionTitleTop =
        EconomyCorruptionTop + EconomySummaryHeight + 10.f * Scale;
    const float EconomyCorruptionSecondMetricTop =
        EconomyCorruptionTitleTop + EconomyTitleHeight + 6.f * Scale;
    const float EconomyCorruptionRowTop =
        EconomyCorruptionSecondMetricTop + EconomySummaryHeight + 4.f * Scale;
    const float EconomyBreakdownTitleTop =
        ShowEconomyCorruptionScreen ?
            EconomyCorruptionTitleTop :
        ShowEconomyUnemployedScreen || ShowEconomyVacancyScreen ?
            EconomyMidSummaryTop :
            EconomyMidSummaryTop + EconomySummaryHeight + 10.f * Scale;
    const float EconomyBreakdownRowsTop =
        ShowEconomyProductionScreen || ShowEconomyElectricityScreen ?
            EconomyTitleHeight + 8.f * Scale :
        ShowEconomyCorruptionScreen ?
            EconomyCorruptionRowTop :
        ShowEconomyTouristRatingScreen ?
            EconomyMidSummaryTop + EconomySummaryHeight + 4.f * Scale :
        ShowEconomyTouristCapacityScreen ?
            EconomyMidSummaryTop +
                2.f * (EconomySummaryHeight + 4.f * Scale) +
                6.f * Scale :
        ShowEconomyLaborScreen ?
            EconomyMidSummaryTop +
                2.f * (EconomySummaryHeight + 4.f * Scale) +
                4.f * Scale :
            EconomyBreakdownTitleTop + EconomyTitleHeight + 6.f * Scale;
    const float EconomyChangeFrameTop =
        EconomyMidSummaryTop +
            2.f * (EconomySummaryHeight + 4.f * Scale) +
            8.f * Scale;
    const float EconomyChangeFrameHeight = 138.f * Scale;
    const float EconomyBottomSummaryTop =
        EconomyChangeFrameTop + EconomyChangeFrameHeight + 8.f * Scale;

    if (auto TrendTitleBackground = Widget.mEconomyTrendTitleBackground.lock())
    {
        TrendTitleBackground->SetPos(EconomyRightX, 2.f * Scale);
        TrendTitleBackground->SetSize(
            EconomyRightW,
            EconomyTitleHeight - 2.f * Scale);
    }

    if (auto TrendTitle = Widget.mEconomyTrendTitle.lock())
    {
        TrendTitle->SetPos(EconomyRightX, 0.f);
        TrendTitle->SetSize(EconomyRightW, EconomyTitleHeight);
    }

    if (auto TrendFrame = Widget.mEconomyTrendFrame.lock())
    {
        TrendFrame->SetPos(EconomyRightX, EconomyTrendFrameTop);
        TrendFrame->SetSize(EconomyRightW, EconomyTrendFrameHeight);
    }

    const float EconomyTrendGraphLeft = EconomyRightX + 22.f * Scale;
    const float EconomyTrendGraphTop = EconomyTrendFrameTop + 14.f * Scale;
    const float EconomyTrendGraphWidth = EconomyRightW - 40.f * Scale;
    const float EconomyTrendGraphHeight = EconomyTrendFrameHeight - 32.f * Scale;

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyTrendGridLines.size()); ++Index)
    {
        const float Fraction =
            GEconomyTrendGridLineCount <= 1 ?
                0.f :
                static_cast<float>(Index) /
                    static_cast<float>(GEconomyTrendGridLineCount - 1);
        const float LineY =
            EconomyTrendGraphTop + EconomyTrendGraphHeight * Fraction;
        if (auto GridLine = Widget.mEconomyTrendGridLines[static_cast<size_t>(Index)].lock())
        {
            GridLine->SetPos(EconomyTrendGraphLeft, LineY);
            GridLine->SetSize(EconomyTrendGraphWidth, 1.2f * Scale);
        }
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyTrendYAxisLabels.size()); ++Index)
    {
        const float Fraction =
            GEconomyTrendYAxisLabelCount <= 1 ?
                0.f :
                static_cast<float>(Index) /
                    static_cast<float>(GEconomyTrendYAxisLabelCount - 1);
        const float LabelY =
            EconomyTrendGraphTop + EconomyTrendGraphHeight * Fraction - 8.f * Scale;
        if (auto YLabel = Widget.mEconomyTrendYAxisLabels[static_cast<size_t>(Index)].lock())
        {
            YLabel->SetPos(EconomyRightX - 8.f * Scale, LabelY);
            YLabel->SetSize(58.f * Scale, 16.f * Scale);
            YLabel->SetFontSize(11.f * Scale);
        }
    }

    if (auto YAxisLine = Widget.mEconomyTrendYAxisLine.lock())
    {
        YAxisLine->SetPos(EconomyTrendGraphLeft, EconomyTrendGraphTop - 4.f * Scale);
        YAxisLine->SetSize(1.6f * Scale, EconomyTrendGraphHeight + 6.f * Scale);
    }
    if (auto XAxisLine = Widget.mEconomyTrendXAxisLine.lock())
    {
        XAxisLine->SetPos(
            EconomyTrendGraphLeft,
            EconomyTrendGraphTop + EconomyTrendGraphHeight);
        XAxisLine->SetSize(EconomyTrendGraphWidth, 1.6f * Scale);
    }
    if (auto YAxisArrow = Widget.mEconomyTrendYAxisArrow.lock())
    {
        YAxisArrow->SetPos(
            EconomyTrendGraphLeft,
            EconomyTrendGraphTop - 8.f * Scale);
        YAxisArrow->SetSize(12.f * Scale, 12.f * Scale);
    }
    if (auto XAxisArrow = Widget.mEconomyTrendXAxisArrow.lock())
    {
        XAxisArrow->SetPos(
            EconomyTrendGraphLeft + EconomyTrendGraphWidth,
            EconomyTrendGraphTop + EconomyTrendGraphHeight);
        XAxisArrow->SetSize(12.f * Scale, 12.f * Scale);
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyTrendXAxisLabels.size()); ++Index)
    {
        const float Fraction =
            GEconomyTrendXAxisLabelCount <= 1 ?
                0.f :
                static_cast<float>(Index) /
                    static_cast<float>(GEconomyTrendXAxisLabelCount - 1);
        if (auto XLabel = Widget.mEconomyTrendXAxisLabels[static_cast<size_t>(Index)].lock())
        {
            XLabel->SetPos(
                EconomyTrendGraphLeft +
                    EconomyTrendGraphWidth * Fraction -
                    40.f * Scale,
                EconomyTrendGraphTop + EconomyTrendGraphHeight + 8.f * Scale);
            XLabel->SetSize(80.f * Scale, 18.f * Scale);
            XLabel->SetFontSize(11.f * Scale);
        }
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyMetrics.size()); ++Index)
    {
        const bool IsTopSummary = Index < 2;
        const bool IsBottomSummary = Index >= 2 && Index < 4;
        const bool IsTouristCapacityMetric =
            ShowEconomyTouristCapacityScreen && Index < 4;
        const float EconomySummaryColumnGap = 10.f * Scale;
        const float EconomySummaryColumnWidth =
            (EconomyRightW - EconomySummaryColumnGap) * 0.5f;
        const float RowX =
            IsTouristCapacityMetric ?
                EconomyRightX +
                    static_cast<float>(Index % 2) *
                        (EconomySummaryColumnWidth + EconomySummaryColumnGap) :
                EconomyRightX;
        const float RowY =
            ShowEconomyCorruptionScreen && Index == 0 ?
                EconomyCorruptionTop :
            ShowEconomyCorruptionScreen && Index == 1 ?
                EconomyCorruptionSecondMetricTop :
            IsTouristCapacityMetric ?
                EconomyMidSummaryTop +
                    static_cast<float>(Index / 2) *
                        (EconomySummaryHeight + 4.f * Scale) :
            IsTopSummary ?
                EconomyMidSummaryTop +
                    static_cast<float>(Index) * (EconomySummaryHeight + 4.f * Scale) :
            IsBottomSummary ?
                EconomyBottomSummaryTop +
                    static_cast<float>(Index - 2) * (EconomySummaryHeight + 4.f * Scale) :
                HiddenY;
        auto& Row = Widget.mEconomyMetrics[static_cast<size_t>(Index)];

        if (auto Bg = Row.Background.lock())
        {
            Bg->SetPos(RowX, RowY);
            Bg->SetSize(
                IsTouristCapacityMetric ?
                    EconomySummaryColumnWidth :
                    EconomyRightW,
                EconomySummaryHeight);
        }
        if (auto Lbl = Row.Label.lock())
        {
            Lbl->SetPos(RowX + 12.f * Scale, RowY);
            Lbl->SetSize(
                IsTouristCapacityMetric ?
                    EconomySummaryColumnWidth - 82.f * Scale :
                    EconomyRightW * 0.55f,
                EconomySummaryHeight);
            Lbl->SetFontSize(15.f * Scale);
        }
        if (auto Bar = Row.Bar.lock())
        {
            Bar->SetEnable(false);
        }
        if (auto Val = Row.Value.lock())
        {
            Val->SetPos(
                RowX +
                    (IsTouristCapacityMetric ?
                        EconomySummaryColumnWidth - 72.f * Scale :
                        EconomyRightW) -
                    12.f * Scale,
                RowY);
            Val->SetSize(
                IsTouristCapacityMetric ?
                    60.f * Scale :
                    96.f * Scale,
                EconomySummaryHeight);
            Val->SetFontSize(15.f * Scale);
        }
    }

    if (auto BreakdownTitleBackground = Widget.mEconomyBreakdownTitleBackground.lock())
    {
        BreakdownTitleBackground->SetPos(EconomyRightX, EconomyBreakdownTitleTop);
        BreakdownTitleBackground->SetSize(
            EconomyRightW,
            EconomyTitleHeight - 2.f * Scale);
    }

    if (auto BreakdownTitle = Widget.mEconomyBreakdownTitle.lock())
    {
        BreakdownTitle->SetPos(EconomyRightX, EconomyBreakdownTitleTop - 2.f * Scale);
        BreakdownTitle->SetSize(EconomyRightW, EconomyTitleHeight);
    }

    LayoutDetailRows(Scale, 
        Widget.mEconomyBreakdownRows,
        EconomyRightX,
        EconomyBreakdownRowsTop,
        EconomyRightW,
        EconomyDetailRowHeight,
        EconomyDetailGap);

    if (auto ChangeFrame = Widget.mEconomyChangeFrame.lock())
    {
        ChangeFrame->SetPos(EconomyRightX, EconomyChangeFrameTop);
        ChangeFrame->SetSize(EconomyRightW, EconomyChangeFrameHeight);
    }

    const float EconomyChangeGraphLeft = EconomyRightX + 22.f * Scale;
    const float EconomyChangeGraphTop = EconomyChangeFrameTop + 12.f * Scale;
    const float EconomyChangeGraphWidth = EconomyRightW - 40.f * Scale;
    const float EconomyChangeGraphHeight = EconomyChangeFrameHeight - 26.f * Scale;

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyChangeGridLines.size()); ++Index)
    {
        const float Fraction =
            GEconomyChangeGridLineCount <= 1 ?
                0.f :
                static_cast<float>(Index) /
                    static_cast<float>(GEconomyChangeGridLineCount - 1);
        const float LineY =
            EconomyChangeGraphTop + EconomyChangeGraphHeight * Fraction;
        if (auto GridLine = Widget.mEconomyChangeGridLines[static_cast<size_t>(Index)].lock())
        {
            GridLine->SetPos(EconomyChangeGraphLeft, LineY);
            GridLine->SetSize(EconomyChangeGraphWidth, 1.2f * Scale);
        }
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mEconomyChangeYAxisLabels.size()); ++Index)
    {
        const float Fraction =
            GEconomyChangeYAxisLabelCount <= 1 ?
                0.f :
                static_cast<float>(Index) /
                    static_cast<float>(GEconomyChangeYAxisLabelCount - 1);
        const float LabelY =
            EconomyChangeGraphTop + EconomyChangeGraphHeight * Fraction - 8.f * Scale;
        if (auto YLabel = Widget.mEconomyChangeYAxisLabels[static_cast<size_t>(Index)].lock())
        {
            YLabel->SetPos(EconomyRightX - 8.f * Scale, LabelY);
            YLabel->SetSize(58.f * Scale, 16.f * Scale);
            YLabel->SetFontSize(11.f * Scale);
        }
    }

    if (auto YAxisLine = Widget.mEconomyChangeYAxisLine.lock())
    {
        YAxisLine->SetPos(EconomyChangeGraphLeft, EconomyChangeGraphTop - 4.f * Scale);
        YAxisLine->SetSize(1.6f * Scale, EconomyChangeGraphHeight + 6.f * Scale);
    }
    if (auto XAxisLine = Widget.mEconomyChangeXAxisLine.lock())
    {
        XAxisLine->SetPos(
            EconomyChangeGraphLeft,
            EconomyChangeGraphTop + EconomyChangeGraphHeight * 0.50f);
        XAxisLine->SetSize(EconomyChangeGraphWidth, 1.6f * Scale);
    }
    if (auto YAxisArrow = Widget.mEconomyChangeYAxisArrow.lock())
    {
        YAxisArrow->SetPos(
            EconomyChangeGraphLeft,
            EconomyChangeGraphTop - 8.f * Scale);
        YAxisArrow->SetSize(12.f * Scale, 12.f * Scale);
    }
    if (auto XAxisArrow = Widget.mEconomyChangeXAxisArrow.lock())
    {
        XAxisArrow->SetPos(
            EconomyChangeGraphLeft + EconomyChangeGraphWidth,
            EconomyChangeGraphTop + EconomyChangeGraphHeight * 0.50f);
        XAxisArrow->SetSize(12.f * Scale, 12.f * Scale);
    }

}

void FAlmanacRenderer::RefreshResourceLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    const float Scale = Layout.Scale;
    const float ContentWidth = Layout.ContentWidth;
    const float ContentHeight = Layout.ContentHeight;
    const float HiddenX = ResolveOffscreenHiddenCoord(ContentWidth, Scale);
    const float HiddenY = ResolveOffscreenHiddenCoord(ContentHeight, Scale);
    const float DetailRowHeight = Layout.DetailRowHeight;
    const float DetailRowGap = Layout.DetailRowGap;
    const auto& PageMetrics = Layout.PageMetrics;
    // 자원
    const float ResourceLeftWide = ContentWidth * 0.46f;
    const float ResourceRightX = ResourceLeftWide + PageMetrics.ColumnGap;
    const float ResourceRightW = ContentWidth - ResourceRightX;
    const float ResourceTitleHeight = PageMetrics.TitleHeight;
    const float ResourceFilterTop = PageMetrics.FrameTop;
    const float ResourceFilterHeight = 30.f * Scale;
    const float ResourceRowHeight = 32.f * Scale;
    const float ResourceRowGap = 3.f * Scale;
    const float ResourceRowsTop =
        ResourceFilterTop + ResourceFilterHeight + 6.f * Scale;
    const float ResourceProductionFrameTop = PageMetrics.FrameTop;
    const float ResourceProductionFrameHeight = 128.f * Scale;
    const float ResourceDistributionTitleTop =
        ResourceProductionFrameTop + ResourceProductionFrameHeight + 8.f * Scale;
    const float ResourceDistributionFilterTop =
        ResourceDistributionTitleTop + ResourceTitleHeight + 4.f * Scale;
    const float ResourceDistributionFilterHeight = 24.f * Scale;
    const float ResourceDistributionRowsTop =
        ResourceDistributionFilterTop + ResourceDistributionFilterHeight + 8.f * Scale;
    const float ResourceDistributionRowHeight = 22.f * Scale;
    const float ResourceDistributionRowGap = 4.f * Scale;
    const float ResourceTrackingTitleTop =
        ResourceDistributionRowsTop +
        static_cast<float>(GResourceDistributionRowCount) *
            (ResourceDistributionRowHeight + ResourceDistributionRowGap) +
        10.f * Scale;
    const float ResourceTrackingHeaderTop =
        ResourceTrackingTitleTop + ResourceTitleHeight + 6.f * Scale;
    const float ResourceTrackingRowsTop =
        ResourceTrackingHeaderTop + 28.f * Scale;

    if (auto Background = Widget.mResourceListTitleBackground.lock())
    {
        Background->SetPos(0.f, 2.f * Scale);
        Background->SetSize(ResourceLeftWide, ResourceTitleHeight - 2.f * Scale);
    }
    if (auto Title = Widget.mResourceListTitle.lock())
    {
        Title->SetPos(0.f, 0.f);
        Title->SetSize(ResourceLeftWide, ResourceTitleHeight);
    }
    if (auto Background = Widget.mResourceFilterBackground.lock())
    {
        Background->SetPos(82.f * Scale, ResourceFilterTop);
        Background->SetSize(150.f * Scale, ResourceFilterHeight);
    }
    if (auto Text = Widget.mResourceFilterText.lock())
    {
        Text->SetPos(98.f * Scale, ResourceFilterTop - 1.f * Scale);
        Text->SetSize(114.f * Scale, ResourceFilterHeight);
        Text->SetFontSize(15.f * Scale);
    }
    if (auto Icon = Widget.mResourceFilterLeftIcon.lock())
    {
        Icon->SetPos(6.f * Scale, ResourceFilterTop - 1.f * Scale);
        Icon->SetSize(28.f * Scale, 28.f * Scale);
    }
    if (auto Icon = Widget.mResourceFilterSortIcon.lock())
    {
        Icon->SetPos(44.f * Scale, ResourceFilterTop + 9.f * Scale);
        Icon->SetSize(12.f * Scale, 12.f * Scale);
    }
    if (auto Arrow = Widget.mResourceFilterSortArrow.lock())
    {
        Arrow->SetPos(232.f * Scale, ResourceFilterTop + 9.f * Scale);
        Arrow->SetSize(12.f * Scale, 12.f * Scale);
    }

    LayoutDetailRows(Scale, 
        Widget.mResourceRows,
        0.f,
        ResourceRowsTop,
        ResourceLeftWide,
        ResourceRowHeight,
        ResourceRowGap);

    if (auto Background = Widget.mResourceProductionTitleBackground.lock())
    {
        Background->SetPos(ResourceRightX, 2.f * Scale);
        Background->SetSize(ResourceRightW, ResourceTitleHeight - 2.f * Scale);
    }
    if (auto Title = Widget.mResourceProductionTitle.lock())
    {
        Title->SetPos(ResourceRightX, 0.f);
        Title->SetSize(ResourceRightW, ResourceTitleHeight);
    }
    if (auto Frame = Widget.mResourceProductionFrame.lock())
    {
        Frame->SetPos(ResourceRightX, ResourceProductionFrameTop);
        Frame->SetSize(ResourceRightW, ResourceProductionFrameHeight);
    }

    const float ResourceGraphLeft = ResourceRightX + 22.f * Scale;
    const float ResourceGraphTop = ResourceProductionFrameTop + 14.f * Scale;
    const float ResourceGraphWidth = ResourceRightW - 40.f * Scale;
    const float ResourceGraphHeight = ResourceProductionFrameHeight - 32.f * Scale;

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceProductionGridLines.size()); ++Index)
    {
        const float Fraction =
            GResourceProductionGridLineCount <= 1 ?
                0.f :
                static_cast<float>(Index) /
                    static_cast<float>(GResourceProductionGridLineCount - 1);
        const float LineY = ResourceGraphTop + ResourceGraphHeight * Fraction;
        if (auto Grid = Widget.mResourceProductionGridLines[static_cast<size_t>(Index)].lock())
        {
            Grid->SetPos(ResourceGraphLeft, LineY);
            Grid->SetSize(ResourceGraphWidth, 1.2f * Scale);
        }
    }

    if (auto Line = Widget.mResourceProductionYAxisLine.lock())
    {
        Line->SetPos(ResourceGraphLeft, ResourceGraphTop - 4.f * Scale);
        Line->SetSize(1.6f * Scale, ResourceGraphHeight + 6.f * Scale);
    }
    if (auto Line = Widget.mResourceProductionXAxisLine.lock())
    {
        Line->SetPos(ResourceGraphLeft, ResourceGraphTop + ResourceGraphHeight);
        Line->SetSize(ResourceGraphWidth, 1.6f * Scale);
    }
    if (auto Arrow = Widget.mResourceProductionYAxisArrow.lock())
    {
        Arrow->SetPos(ResourceGraphLeft, ResourceGraphTop - 8.f * Scale);
        Arrow->SetSize(12.f * Scale, 12.f * Scale);
    }
    if (auto Arrow = Widget.mResourceProductionXAxisArrow.lock())
    {
        Arrow->SetPos(
            ResourceGraphLeft + ResourceGraphWidth,
            ResourceGraphTop + ResourceGraphHeight);
        Arrow->SetSize(12.f * Scale, 12.f * Scale);
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceProductionXAxisLabels.size()); ++Index)
    {
        const float Fraction =
            GResourceProductionXAxisLabelCount <= 1 ?
                0.f :
                static_cast<float>(Index + 1) /
                    static_cast<float>(GResourceProductionXAxisLabelCount);
        if (auto Label = Widget.mResourceProductionXAxisLabels[static_cast<size_t>(Index)].lock())
        {
            Label->SetPos(
                ResourceGraphLeft + ResourceGraphWidth * Fraction - 32.f * Scale,
                ResourceGraphTop + ResourceGraphHeight + 8.f * Scale);
            Label->SetSize(64.f * Scale, 16.f * Scale);
            Label->SetFontSize(11.f * Scale);
        }
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceProductionYAxisLabels.size()); ++Index)
    {
        const float Fraction =
            GResourceProductionYAxisLabelCount <= 1 ?
                0.f :
                static_cast<float>(Index) /
                    static_cast<float>(GResourceProductionYAxisLabelCount - 1);
        if (auto Label = Widget.mResourceProductionYAxisLabels[static_cast<size_t>(Index)].lock())
        {
            Label->SetPos(
                ResourceRightX - 10.f * Scale,
                ResourceGraphTop + ResourceGraphHeight * Fraction - 8.f * Scale);
            Label->SetSize(54.f * Scale, 16.f * Scale);
            Label->SetFontSize(11.f * Scale);
        }
    }

    if (auto Swatch = Widget.mResourceProductionLegendPrimarySwatch.lock())
    {
        Swatch->SetPos(ResourceRightX + ResourceRightW - 74.f * Scale, ResourceProductionFrameTop + 92.f * Scale);
        Swatch->SetSize(8.f * Scale, 8.f * Scale);
    }
    if (auto Text = Widget.mResourceProductionLegendPrimaryText.lock())
    {
        Text->SetPos(ResourceRightX + ResourceRightW - 62.f * Scale, ResourceProductionFrameTop + 86.f * Scale);
        Text->SetSize(58.f * Scale, 18.f * Scale);
        Text->SetFontSize(10.f * Scale);
    }
    if (auto Swatch = Widget.mResourceProductionLegendSecondarySwatch.lock())
    {
        Swatch->SetPos(ResourceRightX + ResourceRightW - 74.f * Scale, ResourceProductionFrameTop + 102.f * Scale);
        Swatch->SetSize(8.f * Scale, 8.f * Scale);
    }
    if (auto Text = Widget.mResourceProductionLegendSecondaryText.lock())
    {
        Text->SetPos(ResourceRightX + ResourceRightW - 62.f * Scale, ResourceProductionFrameTop + 96.f * Scale);
        Text->SetSize(58.f * Scale, 18.f * Scale);
        Text->SetFontSize(10.f * Scale);
    }

    if (auto Background = Widget.mResourceDistributionTitleBackground.lock())
    {
        Background->SetPos(ResourceRightX, ResourceDistributionTitleTop);
        Background->SetSize(ResourceRightW, ResourceTitleHeight - 2.f * Scale);
    }
    if (auto Title = Widget.mResourceDistributionTitle.lock())
    {
        Title->SetPos(ResourceRightX, ResourceDistributionTitleTop - 2.f * Scale);
        Title->SetSize(ResourceRightW, ResourceTitleHeight);
    }
    if (auto Background = Widget.mResourceDistributionFilterBackground.lock())
    {
        Background->SetPos(ResourceRightX, ResourceDistributionFilterTop);
        Background->SetSize(116.f * Scale, ResourceDistributionFilterHeight);
    }
    if (auto Text = Widget.mResourceDistributionFilterText.lock())
    {
        Text->SetPos(ResourceRightX + 10.f * Scale, ResourceDistributionFilterTop - 1.f * Scale);
        Text->SetSize(100.f * Scale, ResourceDistributionFilterHeight);
        Text->SetFontSize(13.f * Scale);
    }

    LayoutMetricRows(Scale, 
        Widget.mResourceDistributionRows,
        ResourceRightX,
        ResourceDistributionRowsTop,
        ResourceRightW,
        ResourceDistributionRowHeight,
        ResourceDistributionRowGap);

    if (auto Background = Widget.mResourceTrackingTitleBackground.lock())
    {
        Background->SetPos(ResourceRightX, ResourceTrackingTitleTop);
        Background->SetSize(ResourceRightW, ResourceTitleHeight - 2.f * Scale);
    }
    if (auto Title = Widget.mResourceTrackingTitle.lock())
    {
        Title->SetPos(ResourceRightX, ResourceTrackingTitleTop - 2.f * Scale);
        Title->SetSize(ResourceRightW, ResourceTitleHeight);
    }
    if (auto Text = Widget.mResourceTrackingName.lock())
    {
        Text->SetPos(ResourceRightX + 12.f * Scale, ResourceTrackingHeaderTop);
        Text->SetSize(ResourceRightW * 0.55f, 24.f * Scale);
        Text->SetFontSize(16.f * Scale);
    }
    if (auto Text = Widget.mResourceTrackingValue.lock())
    {
        Text->SetPos(ResourceRightX + ResourceRightW - 84.f * Scale, ResourceTrackingHeaderTop);
        Text->SetSize(72.f * Scale, 24.f * Scale);
        Text->SetFontSize(16.f * Scale);
    }

    LayoutDetailRows(Scale, 
        Widget.mResourceDetails,
        ResourceRightX,
        ResourceTrackingRowsTop,
        ResourceRightW,
        DetailRowHeight,
        DetailRowGap);

    if (auto Notice = Widget.mResourceNotice.lock())
    {
        Notice->SetPos(HiddenX, HiddenY);
        Notice->SetSize(1.f, 1.f);
    }

}

void FAlmanacRenderer::RefreshPoliticsLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    const float Scale = Layout.Scale;
    const float ContentWidth = Layout.ContentWidth;
    const auto& PageMetrics = Layout.PageMetrics;
    // 정치
    const float PoliticsLeftWide = ContentWidth * 0.48f;
    const float PoliticsRightX = PoliticsLeftWide + PageMetrics.ColumnGap;
    const float PoliticsRightW = ContentWidth - PoliticsRightX;
    const float PoliticsTitleTop = 0.f;
    const float PoliticsTitleHeight = PageMetrics.TitleHeight;
    const float PoliticsTileTop = PoliticsTitleHeight + 10.f * Scale;
    const float PoliticsTileGapX = 78.f * Scale;
    const float PoliticsTileGapY = 8.f * Scale;
    const float PoliticsTileWidth =
        (PoliticsLeftWide - PoliticsTileGapX) * 0.5f;
    const float PoliticsTileHeight = 70.f * Scale;
    const float PoliticsNeutralWidth = 56.f * Scale;

    if (auto Background = Widget.mPoliticsListTitleBackground.lock())
    {
        Background->SetPos(0.f, PoliticsTitleTop + 2.f * Scale);
        Background->SetSize(
            PoliticsLeftWide,
            PoliticsTitleHeight - 2.f * Scale);
    }
    if (auto Title = Widget.mPoliticsListTitle.lock())
    {
        Title->SetPos(0.f, PoliticsTitleTop);
        Title->SetSize(PoliticsLeftWide, PoliticsTitleHeight);
    }

    LayoutPoliticsFactionTiles(Scale, 
        Widget.mPoliticsFactionTiles,
        0.f,
        PoliticsTileTop,
        PoliticsTileWidth,
        PoliticsTileHeight,
        PoliticsTileGapX,
        PoliticsTileGapY);

    for (int Index = 0; Index < static_cast<int>(Widget.mPoliticsNeutralTexts.size()); ++Index)
    {
        if (auto Text = Widget.mPoliticsNeutralTexts[Index].lock())
        {
            const float RowY =
                PoliticsTileTop +
                (PoliticsTileHeight + PoliticsTileGapY) *
                    static_cast<float>(Index);
            Text->SetPos(
                PoliticsTileWidth +
                    (PoliticsTileGapX - PoliticsNeutralWidth) * 0.5f,
                RowY + 12.f * Scale);
            Text->SetSize(PoliticsNeutralWidth, 40.f * Scale);
        }
    }

    const float PoliticsSupportTitleTop =
        PoliticsTileTop +
        (PoliticsTileHeight + PoliticsTileGapY) * 4.f +
        16.f * Scale;
    const float PoliticsSupportRowTop =
        PoliticsSupportTitleTop + PoliticsTitleHeight + 8.f * Scale;
    const float PoliticsSupportRowHeight = 25.f * Scale;
    const float PoliticsSupportRowGap = 3.f * Scale;

    if (auto Background = Widget.mPoliticsSupportTitleBackground.lock())
    {
        Background->SetPos(0.f, PoliticsSupportTitleTop + 2.f * Scale);
        Background->SetSize(
            PoliticsLeftWide,
            PoliticsTitleHeight - 2.f * Scale);
    }
    if (auto Title = Widget.mPoliticsSupportTitle.lock())
    {
        Title->SetPos(0.f, PoliticsSupportTitleTop);
        Title->SetSize(PoliticsLeftWide, PoliticsTitleHeight);
    }

    LayoutDetailRows(Scale, 
        Widget.mPoliticsSupportRows,
        0.f,
        PoliticsSupportRowTop,
        PoliticsLeftWide,
        PoliticsSupportRowHeight,
        PoliticsSupportRowGap);

    const float PoliticsElectionTop =
        PoliticsSupportRowTop +
        (PoliticsSupportRowHeight + PoliticsSupportRowGap) *
            static_cast<float>(GPoliticsSupportRowCount) +
        10.f * Scale;
    const float PoliticsElectionTextWidth = 132.f * Scale;
    const float PoliticsElectionArrowSize = 16.f * Scale;
    const float PoliticsElectionCenterX = PoliticsLeftWide * 0.5f;

    if (auto Arrow = Widget.mPoliticsElectionLeftArrow.lock())
    {
        Arrow->SetPos(
            PoliticsElectionCenterX - PoliticsElectionTextWidth * 0.5f -
                PoliticsElectionArrowSize - 8.f * Scale,
            PoliticsElectionTop + 8.f * Scale);
        Arrow->SetSize(PoliticsElectionArrowSize, PoliticsElectionArrowSize);
    }
    if (auto Arrow = Widget.mPoliticsElectionRightArrow.lock())
    {
        Arrow->SetPos(
            PoliticsElectionCenterX + PoliticsElectionTextWidth * 0.5f +
                8.f * Scale,
            PoliticsElectionTop + 8.f * Scale);
        Arrow->SetSize(PoliticsElectionArrowSize, PoliticsElectionArrowSize);
    }
    if (auto Text = Widget.mPoliticsElectionText.lock())
    {
        Text->SetPos(
            PoliticsElectionCenterX - PoliticsElectionTextWidth * 0.5f,
            PoliticsElectionTop);
        Text->SetSize(PoliticsElectionTextWidth, 44.f * Scale);
    }

    if (auto Title = Widget.mPoliticsFactionTitle.lock())
    {
        Title->SetPos(PoliticsRightX, PoliticsTitleTop);
        Title->SetSize(PoliticsRightW, PoliticsTitleHeight);
    }
    if (auto Label = Widget.mPoliticsFactionApprovalLabel.lock())
    {
        Label->SetPos(PoliticsRightX, PoliticsTitleHeight + 6.f * Scale);
        Label->SetSize(PoliticsRightW - 64.f * Scale, 24.f * Scale);
    }
    if (auto Value = Widget.mPoliticsFactionApprovalValue.lock())
    {
        Value->SetPos(
            PoliticsRightX + PoliticsRightW - 60.f * Scale,
            PoliticsTitleHeight + 4.f * Scale);
        Value->SetSize(60.f * Scale, 26.f * Scale);
    }

    LayoutDetailRows(Scale, 
        Widget.mPoliticsDetails,
        PoliticsRightX,
        PoliticsTitleHeight + 36.f * Scale,
        PoliticsRightW,
        29.f * Scale,
        3.f * Scale);

}

void FAlmanacRenderer::RefreshForeignLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    const float Scale = Layout.Scale;
    const float ContentWidth = Layout.ContentWidth;
    const float ContentHeight = Layout.ContentHeight;
    const float HiddenX = ResolveOffscreenHiddenCoord(ContentWidth, Scale);
    const float HiddenY = ResolveOffscreenHiddenCoord(ContentHeight, Scale);
    const auto& PageMetrics = Layout.PageMetrics;
    // 대외관계
    const float ForeignLeftWide = ContentWidth * 0.49f;
    const float ForeignRightX = ForeignLeftWide + PageMetrics.ColumnGap;
    const float ForeignRightW = ContentWidth - ForeignRightX;
    const float ForeignRowTop = 10.f * Scale;
    const float ForeignRowHeight = 46.f * Scale;
    const float ForeignRowGap = 8.f * Scale;
    const float ForeignTitleHeight = PageMetrics.TitleHeight;
    const float ForeignStatusTop = 36.f * Scale;
    const float ForeignDetailsTop = 62.f * Scale;
    const float ForeignDetailRowHeight = 29.f * Scale;
    const float ForeignDetailGap = 4.f * Scale;

    LayoutSatisfactionRows(Scale, 
        Widget.mForeignRows,
        0.f,
        ForeignRowTop,
        ForeignLeftWide,
        ForeignRowHeight,
        ForeignRowGap);

    if (auto Background = Widget.mForeignTitleBackground.lock())
    {
        Background->SetPos(ForeignRightX, 2.f * Scale);
        Background->SetSize(ForeignRightW, ForeignTitleHeight - 2.f * Scale);
    }
    if (auto Title = Widget.mForeignTitle.lock())
    {
        Title->SetPos(ForeignRightX, 0.f);
        Title->SetSize(ForeignRightW, ForeignTitleHeight);
    }
    if (auto Text = Widget.mForeignStatusLabel.lock())
    {
        Text->SetPos(ForeignRightX + 8.f * Scale, ForeignStatusTop);
        Text->SetSize(ForeignRightW * 0.48f, 22.f * Scale);
    }
    if (auto Text = Widget.mForeignStatusValue.lock())
    {
        Text->SetPos(
            ForeignRightX + ForeignRightW - 104.f * Scale,
            ForeignStatusTop);
        Text->SetSize(96.f * Scale, 22.f * Scale);
    }

    LayoutDetailRows(Scale, 
        Widget.mForeignDetails,
        ForeignRightX,
        ForeignDetailsTop,
        ForeignRightW,
        ForeignDetailRowHeight,
        ForeignDetailGap);

    for (const auto& Metric : Widget.mForeignMetrics)
    {
        if (auto Bg = Metric.Background.lock())
        {
            Bg->SetPos(HiddenX, HiddenY);
            Bg->SetSize(1.f, 1.f);
        }
        if (auto Lbl = Metric.Label.lock())
        {
            Lbl->SetPos(HiddenX, HiddenY);
            Lbl->SetSize(1.f, 1.f);
        }
        if (auto Bar = Metric.Bar.lock())
        {
            Bar->SetPos(HiddenX, HiddenY);
            Bar->SetSize(1.f, 1.f);
        }
        if (auto Val = Metric.Value.lock())
        {
            Val->SetPos(HiddenX, HiddenY);
            Val->SetSize(1.f, 1.f);
        }
    }

    if (auto Notice = Widget.mForeignNotice.lock())
    {
        Notice->SetPos(HiddenX, HiddenY);
        Notice->SetSize(1.f, 1.f);
    }

}

void FAlmanacRenderer::RefreshBuildingLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    const float Scale = Layout.Scale;
    const float ContentWidth = Layout.ContentWidth;
    const auto& PageMetrics = Layout.PageMetrics;
    // 건물 목록
    const float BuildingLeftWide = ContentWidth * 0.49f;
    const float BuildingRightX = BuildingLeftWide + PageMetrics.ColumnGap;
    const float BuildingRightW = ContentWidth - BuildingRightX;
    const float BuildingRowHeight = 33.f * Scale;
    const float BuildingRowGap = 4.f * Scale;
    const float BuildingTitleHeight = PageMetrics.TitleHeight;
    const float BuildingDetailTop = BuildingTitleHeight + 12.f * Scale;
    const float BuildingDetailRowHeight = 34.f * Scale;
    const float BuildingDetailGap = 5.f * Scale;

    LayoutDetailRows(Scale, 
        Widget.mBuildingRows,
        0.f,
        0.f,
        BuildingLeftWide,
        BuildingRowHeight,
        BuildingRowGap);

    if (auto Title = Widget.mBuildingCategoryTitle.lock())
    {
        Title->SetPos(BuildingRightX, 0.f);
        Title->SetSize(BuildingRightW, BuildingTitleHeight);
    }

    LayoutDetailRows(Scale, 
        Widget.mBuildingDetails,
        BuildingRightX,
        BuildingDetailTop,
        BuildingRightW,
        BuildingDetailRowHeight,
        BuildingDetailGap);

}

void FAlmanacRenderer::RefreshConflictLayout(
    CAlmanacWidget& Widget,
    const FAlmanacLayoutContext& Layout)
{
    const float Scale = Layout.Scale;
    const float ContentWidth = Layout.ContentWidth;
    const float ContentHeight = Layout.ContentHeight;
    const float MetricRowHeight = Layout.MetricRowHeight;
    const float MetricRowGap = Layout.MetricRowGap;
    const auto& PageMetrics = Layout.PageMetrics;
    // 분쟁
    const float ConflictLeft    = ContentWidth * 0.54f;
    const float ConflictRight   = ContentWidth - ConflictLeft - PageMetrics.ColumnGap;
    const float ConflictDetailGap = 8.f * Scale;
    const float ConflictDetailRowH =
        (std::max)(30.f * Scale,
            (ContentHeight -
                140.f * Scale -
                ConflictDetailGap * static_cast<float>((std::max)(0, GConflictDetailCount - 1))) /
            static_cast<float>((std::max)(1, GConflictDetailCount)));

    if (auto HeadlineBg = Widget.mConflictHeadlineBackground.lock())
    {
        HeadlineBg->SetPos(0.f, 0.f);
        HeadlineBg->SetSize(ConflictLeft, 122.f * Scale);
    }
    if (auto HeadlineTxt = Widget.mConflictHeadlineText.lock())
    {
        HeadlineTxt->SetPos(18.f * Scale, 14.f * Scale);
        HeadlineTxt->SetSize(ConflictLeft - 36.f * Scale, 96.f * Scale);
    }

    LayoutDetailRows(Scale, 
        Widget.mConflictDetails,
        0.f, 140.f * Scale, ConflictLeft,
        ConflictDetailRowH, ConflictDetailGap);
    LayoutMetricRows(Scale, 
        Widget.mConflictMetrics,
        ConflictLeft + PageMetrics.ColumnGap, 0.f, ConflictRight,
        MetricRowHeight + 8.f * Scale, MetricRowGap + 2.f * Scale);

}


void FAlmanacRenderer::ApplyOpenState(CAlmanacWidget& Widget)
{
    if (auto Background = Widget.mPanelBackground.lock())
        Background->SetEnable(Widget.mOpen);

    if (auto ContentFrame = Widget.mContentFrame.lock())
        ContentFrame->SetEnable(Widget.mOpen);

    if (auto TitleRibbon = Widget.mTitleRibbon.lock())
        TitleRibbon->SetEnable(Widget.mOpen);

    if (auto TabMarker = Widget.mTabMarker.lock())
        TabMarker->SetEnable(Widget.mOpen);

    if (auto RailTrack = Widget.mLeftRailTrack.lock())
        RailTrack->SetEnable(Widget.mOpen);

    if (auto RailThumb = Widget.mLeftRailThumb.lock())
        RailThumb->SetEnable(Widget.mOpen);

    if (auto TitleText = Widget.mTitleText.lock())
        TitleText->SetEnable(Widget.mOpen);

    if (auto CloseButton = Widget.mCloseButton.lock())
        CloseButton->SetEnable(Widget.mOpen);

    for (size_t Index = 0; Index < Widget.mTabButtons.size(); ++Index)
    {
        auto Button = Widget.mTabButtons[Index].lock();

        if (Button)
            Button->SetEnable(Widget.mOpen);
    }

    ApplySelectedPage(Widget);
}


void FAlmanacRenderer::ApplySelectedPage(CAlmanacWidget& Widget)
{
    if (auto TitleText = Widget.mTitleText.lock())
        TitleText->SetText(GetPageTitle(Widget.mSelectedPage).c_str());

    for (size_t Index = 0; Index < Widget.mTabButtons.size(); ++Index)
    {
        auto Button = Widget.mTabButtons[Index].lock();

        if (Button)
        {
            ConfigureTabButtonStyle(
                Button,
                Index == static_cast<size_t>(Widget.mSelectedPage));
        }
    }

    for (size_t Index = 0; Index < Widget.mPages.size(); ++Index)
    {
        auto Page = Widget.mPages[Index].lock();

        if (Page)
            Page->SetEnable(
                Widget.mOpen && Index == static_cast<size_t>(Widget.mSelectedPage));
    }
}

