#include "BuildMenuWidget.h"
#include "BuildMenuDataProvider.h"
#include "BuildMenuRenderer.h"
#include "AlmanacWidget.h"
#include "EdictWidget.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/PlacementController.h"
#include "../ObjectNames.h"
#include "../World/IWorldUIAccess.h"
#include "World/World.h"

CBuildMenuWidget::CBuildMenuWidget()
{
}

CBuildMenuWidget::~CBuildMenuWidget()
{
}

bool CBuildMenuWidget::Init()
{
    CWidgetContainer::Init();

    mMenuOpen = false;
    mYearbookOpen = false;
    mSelectedCategory = EBuildingCategory::Infrastructure;
    mPreviewEntryIndex = -1;
    mCurrentPage = 0;
    mPageCount = 1;
    mVisibleEntryIndices.clear();

    FBuildMenuRenderer::CreateWidgets(*this);
    SelectCategory(EBuildingCategory::Infrastructure);
    RefreshFromState();
    return true;
}

void CBuildMenuWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    if (mMenuOpen)
    {
        auto World = mWorld.lock();

        if (World)
        {
            auto Input = World->GetInput().lock();

            if (Input)
            {
                const int WheelDelta = Input->GetMouseWheelDelta();

                if (WheelDelta != 0 &&
                    IsMouseOverOpenPanel(Input->GetMousePos()))
                {
                    MovePage(WheelDelta < 0 ? 1 : -1);
                }
            }
        }
    }

    RefreshFromState();
}

void CBuildMenuWidget::Render()
{
    CWidgetContainer::Render();
}

bool CBuildMenuWidget::IsMouseOverOpenPanel(const FVector2& MousePos) const
{
    return FBuildMenuRenderer::IsMouseOverPanel(*this, MousePos);
}

void CBuildMenuWidget::RefreshFromState()
{
    const auto Snapshot = BuildMenuDataProvider::BuildSnapshot(
        mWorld.lock(),
        mSelectedCategory,
        mCurrentPage,
        mPreviewEntryIndex);

    FBuildMenuRenderer::ApplySnapshot(*this, Snapshot);
    FBuildMenuRenderer::RefreshLayout(*this);
}

void CBuildMenuWidget::SelectCategory(EBuildingCategory Category)
{
    mSelectedCategory = Category;
    mCurrentPage = 0;
    mPageCount = 1;
    mPreviewEntryIndex = -1;
}

void CBuildMenuWidget::MovePage(int DeltaPage)
{
    if (DeltaPage == 0)
        return;

    mCurrentPage += DeltaPage;
    mPreviewEntryIndex = -1;
}

void CBuildMenuWidget::PreviewSlot(int SlotIndex)
{
    if (SlotIndex < 0 ||
        SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
    {
        return;
    }

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    const auto& Catalog = GetBuildingCatalog();

    if (EntryIndex >= static_cast<int>(Catalog.size()))
        return;

    if (mPreviewEntryIndex == EntryIndex)
        return;

    mPreviewEntryIndex = EntryIndex;
}

void CBuildMenuWidget::StartPlacementBySlot(int SlotIndex)
{
    if (SlotIndex < 0 ||
        SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
    {
        return;
    }

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    const auto& Catalog = GetBuildingCatalog();

    if (EntryIndex >= static_cast<int>(Catalog.size()))
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto PlacementCtrl =
        World->FindObject<CPlacementController>(GPlacementControllerName).lock();

    if (!PlacementCtrl)
        return;

    const FBuildingCatalogEntry& Entry = Catalog[EntryIndex];

    if (Entry.IsDemolish)
    {
        PlacementCtrl->SetDemolitionMode(true);
        SetBuildMenuOpen(false);
        return;
    }

    PlacementCtrl->SetDemolitionMode(false);

    const std::string SpriteTexturePath =
        GetCatalogEntrySpriteTexturePathUtf8(Entry);

    const bool Started = PlacementCtrl->BeginBuildPlacement(
        Entry,
        SpriteTexturePath);

    if (!Started)
        return;

    SetBuildMenuOpen(false);
}

void CBuildMenuWidget::OnBuildButtonClick()
{
    const bool NextOpen = !mMenuOpen;

    if (NextOpen)
    {
        auto World = mWorld.lock();

        if (World)
        {
            auto UIManager = World->GetUIManager().lock();

            if (UIManager)
            {
                auto EdictWidget =
                    UIManager->FindWidget<CEdictWidget>(GEdictWidgetName).lock();
                auto AlmanacWidget =
                    UIManager->FindWidget<CAlmanacWidget>(
                        GAlmanacWidgetName).lock();

                if (EdictWidget)
                    EdictWidget->SetOpen(false);

                if (AlmanacWidget)
                    AlmanacWidget->SetOpen(false);
            }
        }
    }

    SetBuildMenuOpen(NextOpen);
}

void CBuildMenuWidget::OnYearbookButtonClick()
{
    auto World = mWorld.lock();

    if (World)
    {
        auto UIManager = World->GetUIManager().lock();

        if (UIManager)
        {
            auto AlmanacWidget =
                UIManager->FindWidget<CAlmanacWidget>(
                    GAlmanacWidgetName).lock();
            auto EdictWidget =
                UIManager->FindWidget<CEdictWidget>(
                    GEdictWidgetName).lock();

            if (AlmanacWidget)
            {
                mYearbookOpen = false;
                SetBuildMenuOpen(false);

                if (EdictWidget)
                    EdictWidget->SetOpen(false);

                AlmanacWidget->ToggleOpen();
                return;
            }
        }
    }

    const bool NextOpen = !mYearbookOpen;
    mYearbookOpen = NextOpen;

    if (NextOpen)
    {
        mMenuOpen = false;

        if (World)
        {
            auto UIManager = World->GetUIManager().lock();

            if (UIManager)
            {
                auto EdictWidget =
                    UIManager->FindWidget<CEdictWidget>(
                        GEdictWidgetName).lock();

                if (EdictWidget)
                    EdictWidget->SetOpen(false);
            }
        }
    }

    RefreshFromState();
}

void CBuildMenuWidget::OnMenuCloseButtonClick()
{
    SetBuildMenuOpen(false);
}

void CBuildMenuWidget::OnYearbookCloseButtonClick()
{
    SetAlmanacOpen(false);
}

void CBuildMenuWidget::OnPrevPageClick()
{
    MovePage(-1);
    RefreshFromState();
}

void CBuildMenuWidget::OnNextPageClick()
{
    MovePage(1);
    RefreshFromState();
}

void CBuildMenuWidget::ToggleBuildMenu()
{
    OnBuildButtonClick();
}

void CBuildMenuWidget::ToggleAlmanac()
{
    OnYearbookButtonClick();
}

void CBuildMenuWidget::SetBuildMenuOpen(bool Open)
{
    if (mMenuOpen != Open)
    {
        auto* Access = ResolveWorldUIAccess(mWorld.lock().get());

        if (Open)
        {
            if (Access && !Access->Read().IsSimulationPaused())
            {
                Access->Commands().ToggleSimulationPaused();
                mAutoPaused = true;
            }
        }
        else if (mAutoPaused)
        {
            if (Access && Access->Read().IsSimulationPaused())
                Access->Commands().ToggleSimulationPaused();
            mAutoPaused = false;
        }
    }

    mMenuOpen = Open;

    if (Open)
    {
        mYearbookOpen = false;
        mPreviewEntryIndex = -1;
    }

    RefreshFromState();
}

void CBuildMenuWidget::SetAlmanacOpen(bool Open)
{
    auto World = mWorld.lock();

    if (World)
    {
        auto UIManager = World->GetUIManager().lock();

        if (UIManager)
        {
            auto AlmanacWidget =
                UIManager->FindWidget<CAlmanacWidget>(
                    GAlmanacWidgetName).lock();

            if (AlmanacWidget)
            {
                mYearbookOpen = false;

                if (Open)
                    SetBuildMenuOpen(false);

                AlmanacWidget->SetOpen(Open);
                return;
            }
        }
    }

    mYearbookOpen = Open;

    if (Open)
        mMenuOpen = false;

    RefreshFromState();
}
