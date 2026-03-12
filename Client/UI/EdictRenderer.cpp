#include "EdictRenderer.h"
#include "EdictRendererInternal.h"
#include "Device.h"
#include <algorithm>
#include <string>

void FEdictRenderer::ApplySnapshot(
    CEdictWidget& Widget,
    const EdictDataProvider::FEdictSnapshot& Snapshot)
{
    Widget.mSelectedCategory = Snapshot.Catalog.SelectedCategory;
    Widget.mCurrentPage = Snapshot.Catalog.CurrentPage;
    Widget.mPageCount = Snapshot.Catalog.PageCount;
    Widget.mPreviewEntryIndex = Snapshot.Catalog.PreviewEntryIndex;
    Widget.mSelectedEntryIndex = Snapshot.Catalog.SelectedEntryIndex;
    Widget.mVisibleEntryIndices = Snapshot.Catalog.VisibleEntryIndices;
    Widget.mShowTaxPolicyPanel = Snapshot.TaxPolicy.ShowPanel;

    for (int i = 0; i < static_cast<int>(Widget.mCategoryButtons.size()); ++i)
    {
        auto CategoryButton = Widget.mCategoryButtons[i].lock();
        auto CategoryIcon = i < static_cast<int>(Widget.mCategoryButtonIcons.size()) ?
            Widget.mCategoryButtonIcons[i].lock() : nullptr;
        const bool Selected = i == static_cast<int>(Snapshot.Catalog.SelectedCategory);

        if (!CategoryButton)
            continue;

        ApplyButtonTextureSet(
            CategoryButton,
            "EdictCategoryTabRefresh_" + std::to_string(i),
            Selected ? GCategoryTabTextureSelected : GCategoryTabTextureHidden,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(CategoryButton, Selected);
        CategoryButton->ButtonEnable(
            Widget.mOpen &&
            i <= Snapshot.Catalog.MaxUnlockedCategoryIndex);

        if (CategoryIcon)
        {
            CategoryIcon->SetTint(
                i > Snapshot.Catalog.MaxUnlockedCategoryIndex ?
                    0.45f :
                    (Selected ? 1.f : 0.88f),
                i > Snapshot.Catalog.MaxUnlockedCategoryIndex ?
                    0.45f :
                    (Selected ? 1.f : 0.90f),
                i > Snapshot.Catalog.MaxUnlockedCategoryIndex ?
                    0.45f :
                    (Selected ? 1.f : 0.94f),
                1.f);
        }
    }

    auto TitleText = Widget.mTitleText.lock();
    auto PageText = Widget.mPageText.lock();
    auto PrevPageButton = Widget.mPrevPageButton.lock();
    auto NextPageButton = Widget.mNextPageButton.lock();

    if (TitleText)
        TitleText->SetText(Snapshot.Catalog.TitleText.c_str());

    if (PageText)
        PageText->SetText(Snapshot.Catalog.PageText.c_str());

    if (PrevPageButton)
        PrevPageButton->ButtonEnable(Snapshot.Catalog.CanMovePrev);

    if (NextPageButton)
        NextPageButton->ButtonEnable(Snapshot.Catalog.CanMoveNext);

    for (int i = 0; i < static_cast<int>(Widget.mEdictButtons.size()); ++i)
    {
        auto Button = Widget.mEdictButtons[i].lock();
        auto ButtonText = i < static_cast<int>(Widget.mEdictButtonTexts.size()) ?
            Widget.mEdictButtonTexts[i].lock() : nullptr;
        auto ButtonIcon = i < static_cast<int>(Widget.mEdictButtonIcons.size()) ?
            Widget.mEdictButtonIcons[i].lock() : nullptr;
        auto StarLeft = i < static_cast<int>(Widget.mEdictButtonStarLefts.size()) ?
            Widget.mEdictButtonStarLefts[i].lock() : nullptr;
        auto StarRight = i < static_cast<int>(Widget.mEdictButtonStarRights.size()) ?
            Widget.mEdictButtonStarRights[i].lock() : nullptr;
        auto ButtonCheck = i < static_cast<int>(Widget.mEdictButtonChecks.size()) ?
            Widget.mEdictButtonChecks[i].lock() : nullptr;

        if (!Button || !ButtonText || !ButtonIcon ||
            !StarLeft || !StarRight || !ButtonCheck)
        {
            continue;
        }

        const bool HasEntry =
            i < static_cast<int>(Snapshot.Catalog.Slots.size()) &&
            Snapshot.Catalog.Slots[i].HasEntry;

        if (HasEntry)
        {
            const auto& Slot = Snapshot.Catalog.Slots[i];
            const bool Available = Slot.Available || Slot.Active;
            const int EntryIndex = Slot.EntryIndex >= 0 ? Slot.EntryIndex : i;

            Button->ButtonEnable(true);
            Button->SetEnable(Widget.mOpen);
            ConfigureEdictSlotButtonVisual(
                Button,
                "EdictSlot_" + std::to_string(EntryIndex),
                Slot.Focused,
                Slot.Active,
                Slot.CoolingDown,
                Available);

            ButtonText->SetText(Slot.DisplayName.c_str());
            ButtonText->SetTextColor(
                !Available && !Slot.Active ? 116 : 58,
                !Available && !Slot.Active ? 116 : 47,
                !Available && !Slot.Active ? 116 : 31,
                255);

            if (Slot.IconPath)
            {
                ButtonIcon->SetTexture(
                    "EdictSlotIcon_" + std::to_string(EntryIndex),
                    Slot.IconPath);
            }
            else
            {
                ButtonIcon->SetTexture(
                    "EdictSlotFallback_" + std::to_string(EntryIndex),
                    GCostIconTexture);
            }

            ButtonIcon->SetTint(
                !Available && !Slot.Active ? 0.68f : 1.f,
                !Available && !Slot.Active ? 0.68f : 1.f,
                !Available && !Slot.Active ? 0.68f : 1.f,
                1.f);

            StarLeft->SetTexture(
                "EdictSlotStarLeftState_" + std::to_string(EntryIndex),
                Slot.Active ? GStarFullTexture : GStarEmptyTexture);
            StarRight->SetTexture(
                "EdictSlotStarRightState_" + std::to_string(EntryIndex),
                Slot.Active ? GStarFullTexture : GStarEmptyTexture);
            StarLeft->SetEnable(Widget.mOpen);
            StarRight->SetEnable(Widget.mOpen);
            ButtonCheck->SetEnable(Widget.mOpen && Slot.Active);
            ButtonIcon->SetEnable(Widget.mOpen);
            ButtonText->SetEnable(Widget.mOpen);
        }
        else
        {
            Button->ButtonEnable(false);
            Button->SetEnable(false);
            ConfigureEdictSlotButtonVisual(
                Button,
                "EdictSlotEmpty_" + std::to_string(i),
                false,
                false,
                false,
                false);
            ButtonText->SetText(TEXT(""));
            ButtonIcon->SetEnable(false);
            StarLeft->SetEnable(false);
            StarRight->SetEnable(false);
            ButtonCheck->SetEnable(false);
        }
    }

    auto DetailTitleText = Widget.mDetailTitleText.lock();
    auto DetailCostText = Widget.mDetailCostText.lock();
    auto DetailInfoText = Widget.mDetailInfoText.lock();
    auto DetailBodyText = Widget.mDetailBodyText.lock();
    auto FeedbackText = Widget.mFeedbackText.lock();
    auto RequirementText = Widget.mRequirementText.lock();
    auto ApplyButton = Widget.mApplyButton.lock();
    auto ApplyButtonText = Widget.mApplyButtonText.lock();

    if (DetailTitleText)
        DetailTitleText->SetText(Snapshot.Detail.Title.c_str());

    if (DetailCostText)
        DetailCostText->SetText(Snapshot.Detail.CostText.c_str());

    if (DetailInfoText)
        DetailInfoText->SetText(Snapshot.Detail.InfoText.c_str());

    if (DetailBodyText)
        DetailBodyText->SetText(Snapshot.Detail.BodyText.c_str());

    if (FeedbackText)
    {
        FeedbackText->SetText(Snapshot.Detail.FeedbackText.c_str());
        FeedbackText->SetTextColor(170, 118, 27, 255);
        FeedbackText->SetShadowTextColor(245, 235, 205, 160);
    }

    if (RequirementText)
    {
        RequirementText->SetText(Snapshot.Detail.RequirementText.c_str());

        switch (Snapshot.Detail.RequirementTone)
        {
        case EdictDataProvider::EEdictRequirementTone::BudgetShortage:
            RequirementText->SetTextColor(178, 48, 30, 255);
            RequirementText->SetShadowTextColor(255, 226, 214, 170);
            break;
        case EdictDataProvider::EEdictRequirementTone::Warning:
            RequirementText->SetTextColor(211, 36, 20, 255);
            RequirementText->SetShadowTextColor(255, 228, 216, 170);
            break;
        case EdictDataProvider::EEdictRequirementTone::None:
        default:
            RequirementText->SetTextColor(210, 34, 18, 255);
            RequirementText->SetShadowTextColor(255, 230, 220, 170);
            break;
        }
    }

    ConfigureEdictActionButtonVisual(
        ApplyButton,
        ApplyButtonText,
        Snapshot.Detail.ActionMode,
        Snapshot.Detail.ActionLabel);

    if (ApplyButton)
        ApplyButton->ButtonEnable(Snapshot.Detail.ActionEnabled);

    auto TaxPolicyTitleText = Widget.mTaxPolicyTitleText.lock();
    auto TaxPolicySummaryText = Widget.mTaxPolicySummaryText.lock();

    if (TaxPolicyTitleText)
        TaxPolicyTitleText->SetText(Snapshot.TaxPolicy.TitleText.c_str());

    if (TaxPolicySummaryText)
        TaxPolicySummaryText->SetText(Snapshot.TaxPolicy.SummaryText.c_str());

    for (int i = 0; i < static_cast<int>(Widget.mTaxPolicyRowTexts.size()); ++i)
    {
        auto RowText = Widget.mTaxPolicyRowTexts[i].lock();
        auto DecreaseButton = i < static_cast<int>(Widget.mTaxDecreaseButtons.size()) ?
            Widget.mTaxDecreaseButtons[i].lock() : nullptr;
        auto IncreaseButton = i < static_cast<int>(Widget.mTaxIncreaseButtons.size()) ?
            Widget.mTaxIncreaseButtons[i].lock() : nullptr;
        const bool HasRow =
            i < static_cast<int>(Snapshot.TaxPolicy.Rows.size());

        if (RowText)
        {
            RowText->SetText(
                HasRow ? Snapshot.TaxPolicy.Rows[i].Text.c_str() : TEXT("-"));
        }

        if (DecreaseButton)
            DecreaseButton->ButtonEnable(
                HasRow && Snapshot.TaxPolicy.Rows[i].CanDecrease);

        if (IncreaseButton)
            IncreaseButton->ButtonEnable(
                HasRow && Snapshot.TaxPolicy.Rows[i].CanIncrease);
    }

    ApplyOpenState(Widget);
}

