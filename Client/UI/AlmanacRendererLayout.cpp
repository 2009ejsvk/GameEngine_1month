#include "AlmanacRenderer.h"
#include "AlmanacRendererInternal.h"
#include "UILayoutConfig.h"
#include "Device.h"
#include <algorithm>

void FAlmanacRenderer::RefreshLayout(CAlmanacWidget& Widget)
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    Widget.mLastResolutionWidth = Resolution.Width;
    Widget.mLastResolutionHeight = Resolution.Height;
    const float ScreenWidth  = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float PanelBaseWidth =
        (std::max)(720.f, UIConfig::AlmanacPanelWidth);
    const float PanelBaseHeight =
        (std::max)(520.f, UIConfig::AlmanacPanelHeight);
    const float AvailableWidth  = (std::max)(360.f, ScreenWidth  - 80.f);
    const float AvailableHeight = (std::max)(360.f, ScreenHeight - 120.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(
                AvailableWidth  / PanelBaseWidth,
                AvailableHeight / PanelBaseHeight));

    const float PanelWidth  = PanelBaseWidth  * Scale;
    const float PanelHeight = PanelBaseHeight * Scale;
    const float PanelLeft   = (ScreenWidth  - PanelWidth)  * 0.5f;
    struct FAlmanacChromeMetrics
    {
        float PanelTopOffset;
        float RibbonTopOffset;
        float FrameInsetX;
        float FrameHeaderOverlap;
        float FrameBottomInset;
        float RailLeftInset;
        float RailTopInset;
        float RailBottomInset;
        float RailThumbTopOffset;
        float RailThumbMinHeight;
        float RailThumbExpand;
        float RailToContentGap;
        float ContentTopInset;
        float ContentBottomInset;
        float TitlePaddingX;
        float TitlePaddingY;
        float CloseButtonSize;
        float CloseButtonOffsetX;
        float CloseButtonOffsetY;
    } Chrome =
    {
        UIConfig::AlmanacPanelTopOffset * Scale,
        UIConfig::AlmanacRibbonTopOffset * Scale,
        UIConfig::AlmanacFrameInsetX * Scale,
        UIConfig::AlmanacFrameHeaderOverlap * Scale,
        UIConfig::AlmanacFrameBottomInset * Scale,
        UIConfig::AlmanacRailLeftInset * Scale,
        UIConfig::AlmanacRailTopInset * Scale,
        UIConfig::AlmanacRailBottomInset * Scale,
        UIConfig::AlmanacRailThumbTopOffset * Scale,
        UIConfig::AlmanacRailThumbMinHeight * Scale,
        UIConfig::AlmanacRailThumbExpand * Scale,
        UIConfig::AlmanacRailToContentGap * Scale,
        UIConfig::AlmanacContentTopInset * Scale,
        UIConfig::AlmanacContentBottomInset * Scale,
        UIConfig::AlmanacTitlePaddingX * Scale,
        UIConfig::AlmanacTitlePaddingY * Scale,
        UIConfig::AlmanacCloseButtonSize * Scale,
        UIConfig::AlmanacCloseButtonOffsetX * Scale,
        UIConfig::AlmanacCloseButtonOffsetY * Scale
    };
    struct FAlmanacPageMetrics
    {
        float ColumnGap;
        float WideColumnGap;
        float TitleHeight;
        float FrameTop;
    } PageMetrics =
    {
        UIConfig::AlmanacPageColumnGap * Scale,
        UIConfig::AlmanacWidePageColumnGap * Scale,
        UIConfig::AlmanacPageTitleHeight * Scale,
        UIConfig::AlmanacPageFrameTop * Scale
    };
    const float PanelTop =
        (ScreenHeight - PanelHeight) * 0.5f + Chrome.PanelTopOffset;

    // INI 제어 가능한 변수
    const float HeaderHeight  = UIConfig::AlmanacHeaderHeight  * Scale;
    const float HeaderPadding = UIConfig::AlmanacHeaderPadding * Scale;
    const float ContentMarginX      = UIConfig::AlmanacContentMarginX      * Scale;
    const float ContentMarginTop    = UIConfig::AlmanacContentMarginTop    * Scale;
    const float ContentMarginBottom = UIConfig::AlmanacContentMarginBottom * Scale;
    const float TabSize     = UIConfig::AlmanacTabSize    * Scale;
    const float TabGap      = UIConfig::AlmanacTabGap     * Scale;
    const float TabBaseOffsetY      = UIConfig::AlmanacTabBaseOffsetY      * Scale;
    const float TabSelectedOffsetY  = UIConfig::AlmanacTabSelectedOffsetY  * Scale;
    const float MetricRowHeight = UIConfig::AlmanacMetricRowHeight * Scale;
    const float MetricRowGap    = UIConfig::AlmanacMetricRowGap    * Scale;
    const float DetailRowHeight = UIConfig::AlmanacDetailRowHeight * Scale;
    const float DetailRowGap    = UIConfig::AlmanacDetailRowGap    * Scale;
    const int   CardColumns     = (std::max)(1, static_cast<int>(UIConfig::AlmanacCardColumns));
    const float CardGapX        = UIConfig::AlmanacCardGapX * Scale;
    const float CardGapY        = UIConfig::AlmanacCardGapY * Scale;
    const float LeftPanelRatio  = UIConfig::AlmanacLeftPanelRatio;

    const float RibbonWidth =
        (std::max)(420.f * Scale, PanelWidth - 120.f * Scale);
    const float RibbonHeight = HeaderHeight * 0.64f;
    const float RibbonLeft =
        PanelLeft + (PanelWidth - RibbonWidth) * 0.5f;
    const float RibbonTop = PanelTop + Chrome.RibbonTopOffset;
    const float FrameLeft = PanelLeft + Chrome.FrameInsetX;
    const float FrameTop = PanelTop + HeaderHeight - Chrome.FrameHeaderOverlap;
    const float FrameWidth = PanelWidth - Chrome.FrameInsetX * 2.f;
    const float FrameHeight =
        PanelHeight - (FrameTop - PanelTop) - Chrome.FrameBottomInset;
    const float RailTrackLeft = FrameLeft + Chrome.RailLeftInset;
    const float RailTrackTop = FrameTop + Chrome.RailTopInset;
    const float RailTrackWidth = 10.f * Scale;
    const float RailTrackHeight = FrameHeight - Chrome.RailBottomInset;
    const float RailThumbHeight =
        (std::max)(Chrome.RailThumbMinHeight, RailTrackHeight * 0.18f);
    const float ContentLeft =
        RailTrackLeft + RailTrackWidth + Chrome.RailToContentGap + ContentMarginX * 0.45f;
    const float ContentTop = FrameTop + Chrome.ContentTopInset + ContentMarginTop;
    const float ContentWidth =
        FrameLeft + FrameWidth - ContentMarginX * 0.65f - ContentLeft;
    const float ContentHeight =
        FrameTop + FrameHeight - ContentMarginBottom - Chrome.ContentBottomInset - ContentTop;

    if (auto Background = Widget.mPanelBackground.lock())
    {
        Background->SetPos(PanelLeft, PanelTop);
        Background->SetSize(PanelWidth, PanelHeight);
    }

    if (auto ContentFrame = Widget.mContentFrame.lock())
    {
        ContentFrame->SetPos(FrameLeft, FrameTop);
        ContentFrame->SetSize(FrameWidth, FrameHeight);
    }

    if (auto TitleRibbon = Widget.mTitleRibbon.lock())
    {
        TitleRibbon->SetPos(RibbonLeft, RibbonTop);
        TitleRibbon->SetSize(RibbonWidth, RibbonHeight);
    }

    if (auto RailTrack = Widget.mLeftRailTrack.lock())
    {
        RailTrack->SetPos(RailTrackLeft, RailTrackTop);
        RailTrack->SetSize(RailTrackWidth, RailTrackHeight);
    }

    if (auto RailThumb = Widget.mLeftRailThumb.lock())
    {
        RailThumb->SetPos(
            RailTrackLeft - Chrome.RailThumbExpand,
            RailTrackTop + Chrome.RailThumbTopOffset);
        RailThumb->SetSize(
            RailTrackWidth + Chrome.RailThumbExpand * 2.f,
            RailThumbHeight);
    }

    if (auto TitleText = Widget.mTitleText.lock())
    {
        TitleText->SetFontSize(UIConfig::AlmanacTitleFontSize * Scale);
        TitleText->SetPos(
            RibbonLeft + Chrome.TitlePaddingX,
            RibbonTop + Chrome.TitlePaddingY);
        TitleText->SetSize(
            RibbonWidth - Chrome.TitlePaddingX * 2.f,
            RibbonHeight - Chrome.TitlePaddingY * 2.f);
    }

    if (auto CloseButton = Widget.mCloseButton.lock())
    {
        CloseButton->SetPos(
            PanelLeft + PanelWidth - HeaderPadding - Chrome.CloseButtonOffsetX,
            PanelTop + Chrome.CloseButtonOffsetY);
        CloseButton->SetSize(
            Chrome.CloseButtonSize,
            Chrome.CloseButtonSize);
    }

    // 상단 탭 배치
    const float TabsWidth =
        TabSize * static_cast<float>(Widget.mTabButtons.size()) +
        TabGap  * static_cast<float>((std::max)(0, static_cast<int>(Widget.mTabButtons.size()) - 1));
    const float TabsStartX = PanelLeft + (PanelWidth - TabsWidth) * 0.5f;
    const float TabBaseY   = PanelTop - TabBaseOffsetY;

    for (size_t Index = 0; Index < Widget.mTabButtons.size(); ++Index)
    {
        auto Button = Widget.mTabButtons[Index].lock();
        if (!Button)
            continue;

        const bool  Selected = Index == static_cast<size_t>(Widget.mSelectedPage);
        const float OffsetY  = Selected ? TabSelectedOffsetY : 0.f;

        Button->SetPos(
            TabsStartX + (TabSize + TabGap) * static_cast<float>(Index),
            TabBaseY + OffsetY);
        Button->SetSize(TabSize, TabSize);
    }

    if (auto TabMarker = Widget.mTabMarker.lock())
    {
        const size_t SelectedIndex = static_cast<size_t>(Widget.mSelectedPage);
        const float SelectedX =
            TabsStartX + (TabSize + TabGap) * static_cast<float>(SelectedIndex);
        TabMarker->SetPos(
            SelectedX + TabSize * 0.5f - 14.f * Scale,
            PanelTop - 3.f * Scale);
        TabMarker->SetSize(28.f * Scale, 16.f * Scale);
    }

    // 페이지 영역 공통 위치
    for (size_t Index = 0; Index < Widget.mPages.size(); ++Index)
    {
        auto Page = Widget.mPages[Index].lock();
        if (!Page)
            continue;

        Page->SetPos(ContentLeft, ContentTop);
        Page->SetSize(ContentWidth, ContentHeight);
    }

    // ── 공통 레이아웃 헬퍼 람다 ───────────────────────────────

    auto LayoutMetricRows =
        [Scale](const std::vector<CAlmanacWidget::FMetricRowWidgets>& Rows,
            float X, float Y, float Width, float RowHeight, float Gap)
    {
        const float LabelWidth = Width * 0.42f;
        const float ValueWidth = 100.f * Scale;
        const float BarLeft    = X + LabelWidth + 12.f * Scale;
        const float BarWidth   =
            (std::max)(40.f, Width - LabelWidth - ValueWidth - 34.f * Scale);

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
                Lbl->SetPos(X + 14.f * Scale, RowY - 1.f * Scale);
                Lbl->SetSize(LabelWidth - 16.f * Scale, RowHeight);
            }
            if (auto Bar = Rows[i].Bar.lock())
            {
                Bar->SetPos(BarLeft, RowY + RowHeight * 0.58f);
                Bar->SetSize(BarWidth, (std::max)(4.f * Scale, RowHeight * 0.12f));
            }
            if (auto Val = Rows[i].Value.lock())
            {
                Val->SetPos(X + Width - ValueWidth - 12.f * Scale, RowY - 1.f * Scale);
                Val->SetSize(ValueWidth, RowHeight);
            }
        }
    };

    auto LayoutSatisfactionRows =
        [Scale](const std::vector<CAlmanacWidget::FSatisfactionRowWidgets>& Rows,
            float X, float Y, float Width, float RowHeight, float Gap)
    {
        const float InnerPadding = 12.f * Scale;
        const float IconSize = 30.f * Scale;
        const float ValueWidth = 64.f * Scale;
        const float LabelLeft = InnerPadding + IconSize + 12.f * Scale;
        const float BarWidth = (std::max)(
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
                    Width - LabelLeft - ValueWidth - 18.f * Scale,
                    RowHeight);
            }

            if (auto Bar = Rows[i].Bar.lock())
            {
                Bar->SetPos(LabelLeft, RowHeight - 6.f * Scale);
                Bar->SetSize(BarWidth, 1.f * Scale);
            }

            if (auto Value = Rows[i].Value.lock())
            {
                Value->SetPos(Width - ValueWidth - InnerPadding, 0.f);
                Value->SetSize(ValueWidth, RowHeight);
            }
        }
    };

    auto LayoutPoliticsFactionTiles =
        [Scale](const std::vector<CAlmanacWidget::FPoliticsFactionTileWidgets>& Tiles,
            float X, float Y, float TileWidth, float TileHeight,
            float GapX, float GapY)
    {
        const float IconSize = 28.f * Scale;
        const float SmallIconSize = 13.f * Scale;

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
                Icon->SetPos(12.f * Scale, 10.f * Scale);
                Icon->SetSize(IconSize, IconSize);
            }
            if (auto Label = Tiles[i].Label.lock())
            {
                Label->SetPos(46.f * Scale, 8.f * Scale);
                Label->SetSize(TileWidth - 58.f * Scale, 28.f * Scale);
            }
            if (auto CountIcon = Tiles[i].CountIcon.lock())
            {
                CountIcon->SetPos(12.f * Scale, TileHeight - 22.f * Scale);
                CountIcon->SetSize(SmallIconSize, SmallIconSize);
            }
            if (auto CountValue = Tiles[i].CountValue.lock())
            {
                CountValue->SetPos(28.f * Scale, TileHeight - 25.f * Scale);
                CountValue->SetSize(44.f * Scale, 20.f * Scale);
            }
            if (auto FavorIcon = Tiles[i].FavorIcon.lock())
            {
                FavorIcon->SetPos(
                    TileWidth - 58.f * Scale,
                    TileHeight - 22.f * Scale);
                FavorIcon->SetSize(SmallIconSize, SmallIconSize);
            }
            if (auto FavorValue = Tiles[i].FavorValue.lock())
            {
                FavorValue->SetPos(
                    TileWidth - 40.f * Scale,
                    TileHeight - 25.f * Scale);
                FavorValue->SetSize(32.f * Scale, 20.f * Scale);
            }
        }
    };

    auto LayoutDetailRows =
        [Scale](const std::vector<CAlmanacWidget::FDetailRowWidgets>& Rows,
            float X, float Y, float Width, float RowHeight, float Gap)
    {
        const float ValueWidth = 176.f * Scale;

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
                    UseLocalSpace ? 14.f * Scale : X + 14.f * Scale,
                    UseLocalSpace ? 0.f : RowY - 1.f * Scale);
                Lbl->SetSize(Width - ValueWidth - 26.f * Scale, RowHeight);
            }
            if (auto Val = Rows[i].Value.lock())
            {
                const bool UseLocalSpace = !Rows[i].Button.expired();
                Val->SetPos(
                    UseLocalSpace ?
                        Width - ValueWidth - 12.f * Scale :
                        X + Width - ValueWidth - 12.f * Scale,
                    UseLocalSpace ? 0.f : RowY - 1.f * Scale);
                Val->SetSize(ValueWidth, RowHeight);
            }
        }
    };

    auto LayoutCards =
        [Scale](const std::vector<CAlmanacWidget::FCardWidgets>& Cards,
            float X, float Y, float Width, float Height,
            int Columns, float GapX, float GapY)
    {
        if (Columns <= 0)
            return;

        const float CardWidth  =
            (Width - GapX * static_cast<float>(Columns - 1)) /
            static_cast<float>(Columns);
        const float CardHeight =
            (Height - GapY) * 0.5f;

        for (size_t i = 0; i < Cards.size(); ++i)
        {
            const int   Row    = static_cast<int>(i) / Columns;
            const int   Col    = static_cast<int>(i) % Columns;
            const float CardX  = X + (CardWidth  + GapX) * static_cast<float>(Col);
            const float CardY  = Y + (CardHeight + GapY) * static_cast<float>(Row);

            if (auto Bg = Cards[i].Background.lock())
            {
                Bg->SetPos(CardX, CardY);
                Bg->SetSize(CardWidth, CardHeight);
            }
            if (auto Icon = Cards[i].Icon.lock())
            {
                Icon->SetPos(CardX + 16.f * Scale, CardY + 16.f * Scale);
                Icon->SetSize(34.f * Scale, 34.f * Scale);
            }
            if (auto Title = Cards[i].Title.lock())
            {
                Title->SetPos(CardX + 58.f * Scale, CardY + 12.f * Scale);
                Title->SetSize(CardWidth - 70.f * Scale, 24.f * Scale);
            }
            if (auto Val = Cards[i].Value.lock())
            {
                Val->SetPos(CardX + 16.f * Scale, CardY + 42.f * Scale);
                Val->SetSize(CardWidth - 32.f * Scale, 34.f * Scale);
            }
            if (auto Detail = Cards[i].Detail.lock())
            {
                Detail->SetPos(CardX + 16.f * Scale, CardY + 80.f * Scale);
                Detail->SetSize(CardWidth - 32.f * Scale, CardHeight - 90.f * Scale);
            }
        }
    };

    // ── 페이지별 레이아웃 ──────────────────────────────────────

    const float LeftWide  = ContentWidth * LeftPanelRatio;
    const float RightWide = ContentWidth - LeftWide - PageMetrics.ColumnGap;
    const float SepX      = LeftWide + PageMetrics.ColumnGap;

    // 개요 카드
    auto LayoutOverviewCard =
        [Scale](const CAlmanacWidget::FCardWidgets& Card,
            float X, float Y, float Width, float Height)
    {
        if (auto Background = Card.Background.lock())
        {
            Background->SetPos(X, Y);
            Background->SetSize(Width, Height);
        }
        if (auto Icon = Card.Icon.lock())
        {
            const float IconSize = 58.f * Scale;
            Icon->SetPos(X + (Width - IconSize) * 0.5f, Y + 16.f * Scale);
            Icon->SetSize(IconSize, IconSize);
        }
        if (auto Title = Card.Title.lock())
        {
            Title->SetPos(X + 10.f * Scale, Y + 82.f * Scale);
            Title->SetSize(Width - 20.f * Scale, 38.f * Scale);
        }
        if (auto Value = Card.Value.lock())
        {
            Value->SetPos(X + 10.f * Scale, Y + 112.f * Scale);
            Value->SetSize(Width - 20.f * Scale, 28.f * Scale);
        }
        if (auto Detail = Card.Detail.lock())
        {
            Detail->SetPos(X + 10.f * Scale, Y + 140.f * Scale);
            Detail->SetSize(Width - 20.f * Scale, Height - 148.f * Scale);
        }
    };

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
    const float GroupX0 = 0.f;
    const float GroupX1 = OverviewTopGroupWidth + OverviewGroupGap;
    const float GroupX2 = (OverviewTopGroupWidth + OverviewGroupGap) * 2.f;

    if (Widget.mOverviewCards.size() >= GOverviewCardCount)
    {
        LayoutOverviewCard(
            Widget.mOverviewCards[0],
            GroupX0,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);
        LayoutOverviewCard(
            Widget.mOverviewCards[1],
            GroupX0 + OverviewTopCardWidth + OverviewInnerGap,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);
        LayoutOverviewCard(
            Widget.mOverviewCards[2],
            GroupX1,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);
        LayoutOverviewCard(
            Widget.mOverviewCards[3],
            GroupX1 + OverviewTopCardWidth + OverviewInnerGap,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);
        LayoutOverviewCard(
            Widget.mOverviewCards[4],
            GroupX2,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);
        LayoutOverviewCard(
            Widget.mOverviewCards[5],
            GroupX2 + OverviewTopCardWidth + OverviewInnerGap,
            OverviewTopCardY,
            OverviewTopCardWidth,
            OverviewTopCardHeight);

        LayoutOverviewCard(
            Widget.mOverviewCards[6],
            GroupX0,
            OverviewBottomCardY,
            OverviewTopCardWidth,
            OverviewBottomCardHeight);
        LayoutOverviewCard(
            Widget.mOverviewCards[7],
            GroupX0 + OverviewTopCardWidth + OverviewInnerGap,
            OverviewBottomCardY,
            OverviewTopCardWidth,
            OverviewBottomCardHeight);
        LayoutOverviewCard(
            Widget.mOverviewCards[8],
            ContentWidth * 0.5f - OverviewCenterCardWidth * 0.5f,
            OverviewBottomCardY,
            OverviewCenterCardWidth,
            OverviewBottomCardHeight);
        LayoutOverviewCard(
            Widget.mOverviewCards[9],
            GroupX2,
            OverviewBottomCardY,
            OverviewTopCardWidth,
            OverviewBottomCardHeight);
        LayoutOverviewCard(
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
        SumL->SetPos(ContentWidth + 200.f * Scale, ContentHeight + 200.f * Scale);
        SumL->SetSize(1.f, 1.f);
    }
    if (auto SumR = Widget.mOverviewSummaryRight.lock())
    {
        SumR->SetPos(ContentWidth + 200.f * Scale, ContentHeight + 200.f * Scale);
        SumR->SetSize(1.f, 1.f);
    }

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

    LayoutSatisfactionRows(
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

    LayoutDetailRows(
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
    LayoutDetailRows(
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
    LayoutDetailRows(
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
                ContentHeight + 200.f * Scale;
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

    LayoutDetailRows(
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

    LayoutDetailRows(
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

    LayoutMetricRows(
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

    LayoutDetailRows(
        Widget.mResourceDetails,
        ResourceRightX,
        ResourceTrackingRowsTop,
        ResourceRightW,
        DetailRowHeight,
        DetailRowGap);

    if (auto Notice = Widget.mResourceNotice.lock())
    {
        Notice->SetPos(ContentWidth + 200.f * Scale, ContentHeight + 200.f * Scale);
        Notice->SetSize(1.f, 1.f);
    }

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

    LayoutPoliticsFactionTiles(
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

    LayoutDetailRows(
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

    LayoutDetailRows(
        Widget.mPoliticsDetails,
        PoliticsRightX,
        PoliticsTitleHeight + 36.f * Scale,
        PoliticsRightW,
        29.f * Scale,
        3.f * Scale);

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

    LayoutSatisfactionRows(
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

    LayoutDetailRows(
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
            Bg->SetPos(ContentWidth + 200.f * Scale, ContentHeight + 200.f * Scale);
            Bg->SetSize(1.f, 1.f);
        }
        if (auto Lbl = Metric.Label.lock())
        {
            Lbl->SetPos(ContentWidth + 200.f * Scale, ContentHeight + 200.f * Scale);
            Lbl->SetSize(1.f, 1.f);
        }
        if (auto Bar = Metric.Bar.lock())
        {
            Bar->SetPos(ContentWidth + 200.f * Scale, ContentHeight + 200.f * Scale);
            Bar->SetSize(1.f, 1.f);
        }
        if (auto Val = Metric.Value.lock())
        {
            Val->SetPos(ContentWidth + 200.f * Scale, ContentHeight + 200.f * Scale);
            Val->SetSize(1.f, 1.f);
        }
    }

    if (auto Notice = Widget.mForeignNotice.lock())
    {
        Notice->SetPos(ContentWidth + 200.f * Scale, ContentHeight + 200.f * Scale);
        Notice->SetSize(1.f, 1.f);
    }

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

    LayoutDetailRows(
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

    LayoutDetailRows(
        Widget.mBuildingDetails,
        BuildingRightX,
        BuildingDetailTop,
        BuildingRightW,
        BuildingDetailRowHeight,
        BuildingDetailGap);

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

    LayoutDetailRows(
        Widget.mConflictDetails,
        0.f, 140.f * Scale, ConflictLeft,
        ConflictDetailRowH, ConflictDetailGap);
    LayoutMetricRows(
        Widget.mConflictMetrics,
        ConflictLeft + PageMetrics.ColumnGap, 0.f, ConflictRight,
        MetricRowHeight + 8.f * Scale, MetricRowGap + 2.f * Scale);

    Widget.mLayoutDirty = false;
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
        TitleText->SetText(GPageTitles[static_cast<size_t>(Widget.mSelectedPage)]);

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
