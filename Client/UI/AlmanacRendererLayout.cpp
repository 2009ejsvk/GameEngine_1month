#include "AlmanacRenderer.h"
#include "AlmanacRendererInternal.h"
#include "Device.h"
#include <algorithm>

void FAlmanacRenderer::RefreshLayout(CAlmanacWidget& Widget)
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    Widget.mLastResolutionWidth = Resolution.Width;
    Widget.mLastResolutionHeight = Resolution.Height;
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(360.f, ScreenWidth - 80.f);
    const float AvailableHeight = (std::max)(360.f, ScreenHeight - 120.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(
                AvailableWidth / Widget.mPanelWidth,
                AvailableHeight / Widget.mPanelHeight));
    const float PanelWidth = Widget.mPanelWidth * Scale;
    const float PanelHeight = Widget.mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f + 18.f * Scale;
    const float HeaderHeight = 68.f * Scale;
    const float HeaderPadding = 24.f * Scale;
    const float ContentLeft = PanelLeft + 38.f * Scale;
    const float ContentTop = PanelTop + HeaderHeight + 18.f * Scale;
    const float ContentWidth = PanelWidth - 76.f * Scale;
    const float ContentHeight = PanelHeight - HeaderHeight - 42.f * Scale;

    if (auto Background = Widget.mPanelBackground.lock())
    {
        Background->SetPos(PanelLeft, PanelTop);
        Background->SetSize(PanelWidth, PanelHeight);
    }

    if (auto TitleText = Widget.mTitleText.lock())
    {
        TitleText->SetPos(PanelLeft + HeaderPadding, PanelTop + 18.f * Scale);
        TitleText->SetSize(PanelWidth - 2.f * HeaderPadding, 38.f * Scale);
    }

    if (auto CloseButton = Widget.mCloseButton.lock())
    {
        CloseButton->SetPos(
            PanelLeft + PanelWidth - HeaderPadding - 42.f * Scale,
            PanelTop + 12.f * Scale);
        CloseButton->SetSize(42.f * Scale, 42.f * Scale);
    }

    const float TabSize = 66.f * Scale;
    const float TabGap = 8.f * Scale;
    const float TabsWidth =
        TabSize * static_cast<float>(Widget.mTabButtons.size()) +
        TabGap * static_cast<float>((std::max)(0, static_cast<int>(Widget.mTabButtons.size()) - 1));
    const float TabsStartX = PanelLeft + (PanelWidth - TabsWidth) * 0.5f;
    const float TabBaseY = PanelTop - 18.f * Scale;

    for (size_t Index = 0; Index < Widget.mTabButtons.size(); ++Index)
    {
        auto Button = Widget.mTabButtons[Index].lock();

        if (!Button)
            continue;

        const bool Selected = Index == static_cast<size_t>(Widget.mSelectedPage);
        const float OffsetY = Selected ? 12.f * Scale : 0.f;

        Button->SetPos(
            TabsStartX + (TabSize + TabGap) * static_cast<float>(Index),
            TabBaseY + OffsetY);
        Button->SetSize(TabSize, TabSize);
    }

    for (size_t Index = 0; Index < Widget.mPages.size(); ++Index)
    {
        auto Page = Widget.mPages[Index].lock();

        if (!Page)
            continue;

        Page->SetPos(ContentLeft, ContentTop);
        Page->SetSize(ContentWidth, ContentHeight);
    }

    auto LayoutMetricRows =
        [Scale](const std::vector<CAlmanacWidget::FMetricRowWidgets>& Rows,
            float X,
            float Y,
            float Width,
            float RowHeight,
            float Gap)
    {
        const float LabelWidth = Width * 0.34f;
        const float ValueWidth = 92.f * Scale;
        const float BarLeft = X + LabelWidth + 12.f * Scale;
        const float BarWidth =
            (std::max)(40.f, Width - LabelWidth - ValueWidth - 30.f * Scale);

        for (size_t Index = 0; Index < Rows.size(); ++Index)
        {
            const float RowY =
                Y + (RowHeight + Gap) * static_cast<float>(Index);

            if (auto Background = Rows[Index].Background.lock())
            {
                Background->SetPos(X, RowY);
                Background->SetSize(Width, RowHeight);
            }

            if (auto Label = Rows[Index].Label.lock())
            {
                Label->SetPos(X + 14.f * Scale, RowY);
                Label->SetSize(LabelWidth - 18.f * Scale, RowHeight);
            }

            if (auto Bar = Rows[Index].Bar.lock())
            {
                Bar->SetPos(BarLeft, RowY + RowHeight * 0.30f);
                Bar->SetSize(BarWidth, RowHeight * 0.34f);
            }

            if (auto Value = Rows[Index].Value.lock())
            {
                Value->SetPos(X + Width - ValueWidth - 12.f * Scale, RowY);
                Value->SetSize(ValueWidth, RowHeight);
            }
        }
    };

    auto LayoutDetailRows =
        [Scale](const std::vector<CAlmanacWidget::FDetailRowWidgets>& Rows,
            float X,
            float Y,
            float Width,
            float RowHeight,
            float Gap)
    {
        const float ValueWidth = 160.f * Scale;

        for (size_t Index = 0; Index < Rows.size(); ++Index)
        {
            const float RowY =
                Y + (RowHeight + Gap) * static_cast<float>(Index);

            if (auto Background = Rows[Index].Background.lock())
            {
                Background->SetPos(X, RowY);
                Background->SetSize(Width, RowHeight);
            }

            if (auto Label = Rows[Index].Label.lock())
            {
                Label->SetPos(X + 14.f * Scale, RowY);
                Label->SetSize(Width - ValueWidth - 24.f * Scale, RowHeight);
            }

            if (auto Value = Rows[Index].Value.lock())
            {
                Value->SetPos(X + Width - ValueWidth - 12.f * Scale, RowY);
                Value->SetSize(ValueWidth, RowHeight);
            }
        }
    };

    auto LayoutCards =
        [Scale](const std::vector<CAlmanacWidget::FCardWidgets>& Cards,
            float X,
            float Y,
            float Width,
            float Height,
            int Columns,
            float GapX,
            float GapY)
    {
        if (Columns <= 0)
            return;

        const float CardWidth =
            (Width - GapX * static_cast<float>(Columns - 1)) /
            static_cast<float>(Columns);
        const float CardHeight =
            (Height - GapY * static_cast<float>(1)) * 0.5f;

        for (size_t Index = 0; Index < Cards.size(); ++Index)
        {
            const int Row = static_cast<int>(Index) / Columns;
            const int Col = static_cast<int>(Index) % Columns;
            const float CardX =
                X + (CardWidth + GapX) * static_cast<float>(Col);
            const float CardY =
                Y + (CardHeight + GapY) * static_cast<float>(Row);

            if (auto Background = Cards[Index].Background.lock())
            {
                Background->SetPos(CardX, CardY);
                Background->SetSize(CardWidth, CardHeight);
            }

            if (auto Icon = Cards[Index].Icon.lock())
            {
                Icon->SetPos(CardX + 16.f * Scale, CardY + 18.f * Scale);
                Icon->SetSize(36.f * Scale, 36.f * Scale);
            }

            if (auto Title = Cards[Index].Title.lock())
            {
                Title->SetPos(CardX + 60.f * Scale, CardY + 14.f * Scale);
                Title->SetSize(CardWidth - 72.f * Scale, 28.f * Scale);
            }

            if (auto Value = Cards[Index].Value.lock())
            {
                Value->SetPos(CardX + 14.f * Scale, CardY + 42.f * Scale);
                Value->SetSize(CardWidth - 28.f * Scale, 40.f * Scale);
            }

            if (auto Detail = Cards[Index].Detail.lock())
            {
                Detail->SetPos(CardX + 14.f * Scale, CardY + 86.f * Scale);
                Detail->SetSize(CardWidth - 28.f * Scale, CardHeight - 96.f * Scale);
            }
        }
    };

    const float LeftWide = ContentWidth * 0.56f;
    const float RightWide = ContentWidth - LeftWide - 22.f * Scale;

    LayoutCards(
        Widget.mOverviewCards, 0.f, 0.f, ContentWidth, ContentHeight - 70.f * Scale,
        3, 14.f * Scale, 14.f * Scale);

    if (auto SummaryLeft = Widget.mOverviewSummaryLeft.lock())
    {
        SummaryLeft->SetPos(0.f, ContentHeight - 52.f * Scale);
        SummaryLeft->SetSize(ContentWidth * 0.48f, 48.f * Scale);
    }

    if (auto SummaryRight = Widget.mOverviewSummaryRight.lock())
    {
        SummaryRight->SetPos(ContentWidth * 0.52f, ContentHeight - 52.f * Scale);
        SummaryRight->SetSize(ContentWidth * 0.48f, 48.f * Scale);
    }

    LayoutMetricRows(Widget.mSatisfactionRows, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutDetailRows(Widget.mSatisfactionDetails, LeftWide + 22.f * Scale, 0.f, RightWide, 46.f * Scale, 10.f * Scale);
    LayoutDetailRows(Widget.mPopulationDetails, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutMetricRows(Widget.mPopulationMetrics, LeftWide + 22.f * Scale, 0.f, RightWide, 52.f * Scale, 14.f * Scale);
    LayoutDetailRows(Widget.mEconomyDetails, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutMetricRows(Widget.mEconomyMetrics, LeftWide + 22.f * Scale, 0.f, RightWide, 46.f * Scale, 10.f * Scale);
    LayoutMetricRows(Widget.mResourceRows, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutDetailRows(Widget.mResourceDetails, LeftWide + 22.f * Scale, 0.f, RightWide, 46.f * Scale, 8.f * Scale);
    const float PoliticsMetricGap = 6.f * Scale;
    const float PoliticsMetricRowHeight =
        (std::max)(
            28.f * Scale,
            (ContentHeight -
                PoliticsMetricGap *
                static_cast<float>((std::max)(0, GPoliticsRowCount - 1))) /
            static_cast<float>((std::max)(1, GPoliticsRowCount)));
    LayoutMetricRows(
        Widget.mPoliticsRows,
        0.f,
        0.f,
        ContentWidth * 0.52f,
        PoliticsMetricRowHeight,
        PoliticsMetricGap);
    const float PoliticsDetailGap = 6.f * Scale;
    const float PoliticsDetailRowHeight =
        (std::max)(
            30.f * Scale,
            (ContentHeight -
                PoliticsDetailGap *
                static_cast<float>((std::max)(0, GPoliticsDetailCount - 1))) /
            static_cast<float>((std::max)(1, GPoliticsDetailCount)));
    LayoutDetailRows(
        Widget.mPoliticsDetails,
        ContentWidth * 0.52f + 22.f * Scale,
        0.f,
        ContentWidth - ContentWidth * 0.52f - 22.f * Scale,
        PoliticsDetailRowHeight,
        PoliticsDetailGap);
    LayoutDetailRows(Widget.mForeignDetails, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutMetricRows(Widget.mForeignMetrics, LeftWide + 22.f * Scale, 0.f, RightWide, 54.f * Scale, 12.f * Scale);
    LayoutMetricRows(Widget.mBuildingRows, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutDetailRows(Widget.mBuildingDetails, LeftWide + 22.f * Scale, 0.f, RightWide, 46.f * Scale, 8.f * Scale);

    if (auto Notice = Widget.mResourceNotice.lock())
    {
        Notice->SetPos(LeftWide + 22.f * Scale, 6.f * 54.f * Scale);
        Notice->SetSize(RightWide, 120.f * Scale);
    }

    if (auto Notice = Widget.mForeignNotice.lock())
    {
        Notice->SetPos(LeftWide + 22.f * Scale, 4.f * 66.f * Scale);
        Notice->SetSize(RightWide, 120.f * Scale);
    }

    const float ConflictLeft = ContentWidth * 0.54f;
    const float ConflictRight = ContentWidth - ConflictLeft - 22.f * Scale;
    const float ConflictDetailGap = 8.f * Scale;
    const float ConflictDetailRowHeight =
        (std::max)(
            30.f * Scale,
            (ContentHeight -
                140.f * Scale -
                ConflictDetailGap *
                static_cast<float>((std::max)(0, GConflictDetailCount - 1))) /
            static_cast<float>((std::max)(1, GConflictDetailCount)));

    if (auto HeadlineBackground = Widget.mConflictHeadlineBackground.lock())
    {
        HeadlineBackground->SetPos(0.f, 0.f);
        HeadlineBackground->SetSize(ConflictLeft, 122.f * Scale);
    }

    if (auto HeadlineText = Widget.mConflictHeadlineText.lock())
    {
        HeadlineText->SetPos(18.f * Scale, 14.f * Scale);
        HeadlineText->SetSize(ConflictLeft - 36.f * Scale, 96.f * Scale);
    }

    LayoutDetailRows(
        Widget.mConflictDetails,
        0.f,
        140.f * Scale,
        ConflictLeft,
        ConflictDetailRowHeight,
        ConflictDetailGap);
    LayoutMetricRows(Widget.mConflictMetrics, ConflictLeft + 22.f * Scale, 0.f, ConflictRight, 54.f * Scale, 14.f * Scale);
    Widget.mLayoutDirty = false;
}


void FAlmanacRenderer::ApplyOpenState(CAlmanacWidget& Widget)
{
    if (auto Background = Widget.mPanelBackground.lock())
        Background->SetEnable(Widget.mOpen);

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


