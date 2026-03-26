#include "TradeWidget.h"
#include "TradeWidgetRuntime.h"
#include "../World/GovernmentCommandService.h"
#include "../World/IWorldUIAccess.h"
#include "../World/MainWorldTradeAccess.h"
#include "Device.h"
#include "World/World.h"

using namespace TradeWidgetRuntime;

void CTradeWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    auto TradeAccess =
        ResolveMainWorldTradeAccess(mWorld.lock());

    if (TradeAccess)
    {
        const int NotificationVersion =
            TradeAccess->GetTradeRouteCompletionNotificationVersion();

        if (NotificationVersion > mLastSeenCompletionNotificationVersion)
        {
            if (mAutoOpenCompletionPage)
            {
                SetOpen(true);
                mSelectedPageIndex =
                    static_cast<int>(ETradePageType::CompletedRoutes);
                mSelectedCompletedRouteIndex = 0;
                mFeedbackMessage.clear();
            }

            mLastSeenCompletionNotificationVersion = NotificationVersion;
        }
    }

    if (!mOpen)
        return;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();

    if (mLastResolutionWidth != Resolution.Width ||
        mLastResolutionHeight != Resolution.Height)
    {
        RefreshLayout();
    }

    RefreshFromState();
}

void CTradeWidget::ToggleOpen()
{
    SetOpen(!mOpen);
}

void CTradeWidget::SetOpen(bool Open)
{
    if (mOpen == Open)
        return;

    mOpen = Open;

    {
        auto* Access = ResolveWorldUIAccess(mWorld.lock().get());

        if (mOpen)
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

    if (mOpen)
    {
        mSelectedProposalIndex = 0;
        mSelectedPriceIndex = 0;
        mSelectedActiveRouteIndex = 0;
        mSelectedCompletedRouteIndex = 0;
        mSelectedAmountIndex = 0;
        mFeedbackMessage.clear();
        RefreshLayout();
    }

    RefreshFromState();
}

void CTradeWidget::OnCloseButtonClick()
{
    SetOpen(false);
}

void CTradeWidget::OnPageButtonClick(int PageIndex)
{
    mSelectedPageIndex = ClampInt(PageIndex, 0, GTradePageCount - 1);
    mSelectedProposalIndex = 0;
    mSelectedPriceIndex = 0;
    mSelectedActiveRouteIndex = 0;
    mSelectedCompletedRouteIndex = 0;
    mSelectedAmountIndex = 0;
    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTradeWidget::OnFilterButtonClick()
{
    mSelectedFilterIndex =
        (mSelectedFilterIndex + 1) %
        static_cast<int>(ETradeFilterType::Count);
    mSelectedProposalIndex = 0;
    mSelectedAmountIndex = 0;
    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTradeWidget::OnSortButtonClick(int SortIndex)
{
    const int SafeSortIndex = ClampInt(SortIndex, 0, GTradeSortCount - 1);

    if (mSelectedSortIndex == SafeSortIndex)
        mSortDescending = !mSortDescending;
    else
        mSortDescending = true;

    mSelectedSortIndex = SafeSortIndex;
    mSelectedProposalIndex = 0;
    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTradeWidget::OnProposalButtonClick(int RowIndex)
{
    if (mSelectedPageIndex == 4)
    {
        mSelectedCompletedRouteIndex = ClampInt(
            RowIndex,
            0,
            GTradeVisibleProposalCount - 1);
    }
    else if (mSelectedPageIndex == 3)
    {
        mSelectedActiveRouteIndex = ClampInt(
            RowIndex,
            0,
            GTradeVisibleProposalCount - 1);
    }
    else if (mSelectedPageIndex == 1)
    {
        mSelectedPriceIndex = ClampInt(
            RowIndex,
            0,
            GTradeVisibleProposalCount - 1);
    }
    else if (mSelectedPageIndex != 2)
    {
        mSelectedProposalIndex = ClampInt(
            RowIndex,
            0,
            GTradeVisibleProposalCount - 1);
        mSelectedAmountIndex = 0;
    }

    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTradeWidget::OnAmountButtonClick(int AmountIndex)
{
    mSelectedAmountIndex = ClampInt(
        AmountIndex,
        0,
        GTradeAmountPresetCount - 1);
    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTradeWidget::OnActionButtonClick()
{
    const FTradeWidgetSnapshot Snapshot = BuildSnapshot(
        mWorld.lock(),
        mSelectedPageIndex,
        mSelectedFilterIndex,
        mSelectedSortIndex,
        mSortDescending,
        mSelectedProposalIndex,
        mSelectedPriceIndex,
        mSelectedActiveRouteIndex,
        mSelectedCompletedRouteIndex);

    const bool ShowingProductPrices = mSelectedPageIndex == 1;
    const bool ShowingPriceModifiers = mSelectedPageIndex == 2;
    const bool ShowingActiveRoutes = mSelectedPageIndex == 3;
    const bool ShowingCompletedRoutes = mSelectedPageIndex == 4;

    if (ShowingProductPrices || ShowingPriceModifiers || ShowingCompletedRoutes)
    {
        mFeedbackMessage.clear();
        RefreshFromState();
        return;
    }

    if (ShowingActiveRoutes)
    {
        if (!Snapshot.HasSelectedRoute)
        {
            mFeedbackMessage = L"취소할 수 있는 무역 계약이 없습니다.";
            RefreshFromState();
            return;
        }

        auto World = mWorld.lock();
        auto CommandService =
            ResolveGovernmentCommandService(World);

        if (!CommandService)
        {
            mFeedbackMessage = L"행정 명령을 확인할 수 없습니다.";
            RefreshFromState();
            return;
        }

        std::wstring ResponseMessage;
        CommandService->CancelTradeRoute(
            Snapshot.SelectedRoute.RouteId,
            ResponseMessage);
        mFeedbackMessage = ResponseMessage;
        RefreshFromState();
        return;
    }

    if (!Snapshot.HasSelectedProposal)
    {
        mFeedbackMessage = L"체결 가능한 제안이 없습니다.";
        RefreshFromState();
        return;
    }

    const FTradeProposal& Proposal = Snapshot.SelectedProposal;
    const int Amount = GTradeAmountPresets[static_cast<size_t>(ClampInt(
        mSelectedAmountIndex,
        0,
        GTradeAmountPresetCount - 1))];

    if (Amount > Proposal.MaxAmount)
    {
        mFeedbackMessage = L"선택한 물량이 현재 제안 한도를 초과합니다.";
        RefreshFromState();
        return;
    }

    auto World = mWorld.lock();
    auto CommandService =
        ResolveGovernmentCommandService(World);

    if (!CommandService)
    {
        mFeedbackMessage = L"행정 명령을 확인할 수 없습니다.";
        RefreshFromState();
        return;
    }

    std::wstring ResponseMessage;
    CommandService->ExecuteTradeProposal(
        Proposal.ImportRoute,
        Proposal.ResourceType,
        Proposal.ForeignPowerIndex,
        Proposal.OfferPricePerThousand,
        Amount,
        ResponseMessage);
    mFeedbackMessage = ResponseMessage;
    mSelectedPageIndex = 3;
    mSelectedActiveRouteIndex = 0;
    RefreshFromState();
}

void CTradeWidget::OnCompletionAutoOpenButtonClick()
{
    mAutoOpenCompletionPage = !mAutoOpenCompletionPage;
    mFeedbackMessage =
        mAutoOpenCompletionPage ?
            L"무역 완료 시 무역 화면이 자동으로 열립니다." :
            L"무역 완료 시 무역 화면이 자동으로 열리지 않습니다.";
    RefreshFromState();
}
