#include "CitizenInfoRenderer.h"
#include "CitizenInfoRendererInternal.h"
#include "CitizenInfoWidget.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include <algorithm>
#include <cassert>
#include <cmath>

using namespace CitizenInfoRendererInternal;

void FCitizenInfoRenderer::ValidateSnapshotForDebug(
    const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot)
{
#ifndef NDEBUG
    auto IsRatio = [](float Value)
    {
        return std::isfinite(Value) &&
            Value >= -0.01f &&
            Value <= 1.01f;
    };

    const int VisibleTabCount =
        Snapshot.Mode == CitizenInfoDataProvider::EPanelMode::Citizen ?
            GCitizenTabCount :
            GBuildingTabCount;

    assert(Snapshot.SelectedTabIndex >= 0);
    assert(Snapshot.SelectedTabIndex < VisibleTabCount);
    assert(Snapshot.BudgetLevel >= 1);
    assert(Snapshot.BudgetLevel <= GBudgetLevelCount);
    assert(Snapshot.OverviewResidentCount >= 0);
    assert(Snapshot.OverviewResidentCapacity >= 0);
    assert(Snapshot.OverviewVisitorCount >= 0);
    assert(Snapshot.OverviewVisitorCapacity >= 0);
    assert(Snapshot.CitizenPortraitSlotCount >= 0);
    assert(
        Snapshot.CitizenPortraitSlotCount <=
        CCitizenInfoWidget::GOverviewResidentSlotCount);
    assert(
        Snapshot.CitizenPortraitOccupiedSlot == -1 ||
        (Snapshot.CitizenPortraitOccupiedSlot >= 0 &&
            Snapshot.CitizenPortraitOccupiedSlot <
                Snapshot.CitizenPortraitSlotCount));
    assert(IsRatio(Snapshot.CitizenPoliticsSupportRatio));

    for (float Ratio : Snapshot.CitizenPoliticsSatisfactionRatios)
    {
        assert(IsRatio(Ratio));
    }
#else
    (void)Snapshot;
#endif
}

void FCitizenInfoRenderer::ApplySnapshot(
    CCitizenInfoWidget& Widget,
    const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot)
{
    ValidateSnapshotForDebug(Snapshot);
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
                Label->SetText(
                    CitizenInfoConstants::GetBuildingTabLabel(Index).c_str());

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
