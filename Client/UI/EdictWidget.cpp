#include "EdictWidget.h"
#include "EdictDataProvider.h"
#include "EdictRenderer.h"
#include "../Politics/EdictSystem.h"
#include "../World/GovernmentCommandService.h"
#include "World/Input.h"
#include "World/World.h"

CEdictWidget::CEdictWidget()
{
}

CEdictWidget::~CEdictWidget()
{
}

bool CEdictWidget::Init()
{
    CWidgetContainer::Init();

    mOpen = false;
    mSelectedCategory = EEdictUiCategory::Colonial;
    mPreviewEntryIndex = -1;
    mSelectedEntryIndex = -1;
    mCurrentPage = 0;
    mPageCount = 1;
    mVisibleEntryIndices.assign(14, -1);
    mPanelWidth = 1120.f;
    mPanelHeight = 760.f;
    mFeedbackMessage.clear();
    mLastLayoutWidth = 0;
    mLastLayoutHeight = 0;
    mLastClickedEntryIndex = -1;
    mDoubleClickTimer = 0.f;

    FEdictRenderer::CreateWidgets(*this);
    RefreshFromState();
    return true;
}

void CEdictWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    if (!mOpen)
        return;

    if (mDoubleClickTimer > 0.f)
        mDoubleClickTimer -= DeltaTime;

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

    RefreshFromState();
}

void CEdictWidget::ToggleOpen()
{
    SetOpen(!mOpen);
}

void CEdictWidget::SetOpen(bool Open)
{
    if (mOpen == Open)
        return;

    mOpen = Open;

    if (mOpen)
    {
        mFeedbackMessage.clear();
        mCurrentPage = 0;
        mPageCount = 1;
        mPreviewEntryIndex = -1;
        mSelectedEntryIndex = -1;
        mLastClickedEntryIndex = -1;
        mDoubleClickTimer = 0.f;
    }

    RefreshFromState();
}

bool CEdictWidget::IsMouseOverOpenPanel(const FVector2& MousePos) const
{
    return FEdictRenderer::IsMouseOverPanel(*this, MousePos);
}

void CEdictWidget::RefreshFromState()
{
    const auto Snapshot = EdictDataProvider::BuildSnapshot(
        mWorld.lock(),
        mSelectedCategory,
        mCurrentPage,
        mPreviewEntryIndex,
        mSelectedEntryIndex,
        mFeedbackMessage);

    FEdictRenderer::ApplySnapshot(*this, Snapshot);
    FEdictRenderer::RefreshLayout(*this);
}

void CEdictWidget::SelectCategory(EEdictUiCategory Category)
{
    mSelectedCategory = Category;
    mCurrentPage = 0;
    mPageCount = 1;
    mSelectedEntryIndex = -1;
    mPreviewEntryIndex = -1;
    mFeedbackMessage.clear();
}

void CEdictWidget::MovePage(int DeltaPage)
{
    if (DeltaPage == 0)
        return;

    mCurrentPage += DeltaPage;
    mPreviewEntryIndex = -1;
    mSelectedEntryIndex = -1;
}

void CEdictWidget::PreviewSlot(int SlotIndex)
{
    if (SlotIndex < 0 ||
        SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
    {
        return;
    }

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    if (mPreviewEntryIndex == EntryIndex)
        return;

    mPreviewEntryIndex = EntryIndex;
}

void CEdictWidget::ActivateSlot(int SlotIndex)
{
    if (SlotIndex < 0 ||
        SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
    {
        return;
    }

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    mSelectedEntryIndex = EntryIndex;
    mPreviewEntryIndex = EntryIndex;
    mFeedbackMessage.clear();
}

void CEdictWidget::SelectOrApplySlot(int SlotIndex)
{
    if (SlotIndex < 0 ||
        SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
    {
        return;
    }

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    const bool IsDoubleClick =
        EntryIndex == mLastClickedEntryIndex &&
        mDoubleClickTimer > 0.f;

    mLastClickedEntryIndex = EntryIndex;
    mDoubleClickTimer = 0.45f;
    mSelectedEntryIndex = EntryIndex;
    mPreviewEntryIndex = EntryIndex;
    mFeedbackMessage.clear();

    if (IsDoubleClick)
        OnApplyButtonClick();
}

void CEdictWidget::AdjustTaxPolicy(ETaxPolicyType Type, int DeltaPercent)
{
    auto World = mWorld.lock();
    auto MainWorld = std::dynamic_pointer_cast<IGovernmentCommandService>(World);

    if (!MainWorld)
    {
        mFeedbackMessage = L"지금은 행정 정보를 확인할 수 없습니다.";
        RefreshFromState();
        return;
    }

    MainWorld->AdjustTaxPolicy(Type, DeltaPercent, mFeedbackMessage);
    RefreshFromState();
}

void CEdictWidget::OnPrevPageClick()
{
    MovePage(-1);
    RefreshFromState();
}

void CEdictWidget::OnNextPageClick()
{
    MovePage(1);
    RefreshFromState();
}

void CEdictWidget::OnCloseButtonClick()
{
    SetOpen(false);
}

void CEdictWidget::OnApplyButtonClick()
{
    const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();
    const int EntryIndex =
        mPreviewEntryIndex >= 0 ? mPreviewEntryIndex : mSelectedEntryIndex;

    if (EntryIndex < 0 ||
        EntryIndex >= static_cast<int>(Definitions.size()))
    {
        mFeedbackMessage = L"먼저 칙령을 선택하세요.";
        RefreshFromState();
        return;
    }

    auto World = mWorld.lock();
    auto MainWorld = std::dynamic_pointer_cast<IGovernmentCommandService>(World);

    if (!MainWorld)
    {
        mFeedbackMessage = L"지금은 행정 정보를 확인할 수 없습니다.";
        RefreshFromState();
        return;
    }

    MainWorld->TryApplyEdict(
        Definitions[EntryIndex].Type,
        mFeedbackMessage);
    RefreshFromState();
}

void CEdictWidget::OnConsumptionTaxDownClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Consumption,
        -GetTaxPolicyStepPercent(ETaxPolicyType::Consumption));
}

void CEdictWidget::OnConsumptionTaxUpClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Consumption,
        GetTaxPolicyStepPercent(ETaxPolicyType::Consumption));
}

void CEdictWidget::OnIncomeTaxDownClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Income,
        -GetTaxPolicyStepPercent(ETaxPolicyType::Income));
}

void CEdictWidget::OnIncomeTaxUpClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Income,
        GetTaxPolicyStepPercent(ETaxPolicyType::Income));
}

void CEdictWidget::OnPropertyTaxDownClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Property,
        -GetTaxPolicyStepPercent(ETaxPolicyType::Property));
}

void CEdictWidget::OnPropertyTaxUpClick()
{
    AdjustTaxPolicy(
        ETaxPolicyType::Property,
        GetTaxPolicyStepPercent(ETaxPolicyType::Property));
}
