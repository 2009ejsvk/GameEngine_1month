#include "TopHudWidget.h"
#include "TopHudDataProvider.h"
#include "TopHudRenderer.h"
#include "UILayoutConfig.h"
#include "AlmanacWidget.h"
#include "BuildMenuWidget.h"
#include "EdictWidget.h"
#include "../ObjectNames.h"
#include "World/World.h"
#include "World/WorldUIManager.h"

CTopHudWidget::CTopHudWidget()
{
}

CTopHudWidget::~CTopHudWidget()
{
}

bool CTopHudWidget::Init()
{
    CWidgetContainer::Init();

    mMonthProgress = 0.f;
    mGameLost = false;
    mGameOverMenusClosed = false;

    FTopHudRenderer::CreateWidgets(*this);
    RefreshFromState();
    return true;
}

void CTopHudWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
    UIConfig::ReloadIfChanged(DeltaTime);
    RefreshFromState();
}

void CTopHudWidget::RefreshFromState()
{
    const auto Snapshot = TopHudDataProvider::BuildSnapshot(mWorld.lock());

    if (Snapshot.GameLost && !mGameOverMenusClosed)
    {
        CloseMenus(true, true, true);
        mGameOverMenusClosed = true;
    }
    else if (!Snapshot.GameLost)
    {
        mGameOverMenusClosed = false;
    }

    FTopHudRenderer::ApplySnapshot(*this, Snapshot);
    FTopHudRenderer::RefreshLayout(*this);
}

void CTopHudWidget::CloseMenus(
    bool CloseBuildMenu,
    bool CloseAlmanac,
    bool CloseEdicts)
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto BuildMenu =
        UIManager->FindWidget<CBuildMenuWidget>(GBuildMenuWidgetName).lock();
    auto AlmanacWidget =
        UIManager->FindWidget<CAlmanacWidget>(GAlmanacWidgetName).lock();
    auto EdictWidget =
        UIManager->FindWidget<CEdictWidget>(GEdictWidgetName).lock();

    if (BuildMenu)
    {
        if (CloseBuildMenu)
            BuildMenu->SetBuildMenuOpen(false);

        if (CloseAlmanac)
            BuildMenu->SetAlmanacOpen(false);
    }

    if (AlmanacWidget && CloseAlmanac)
        AlmanacWidget->SetOpen(false);

    if (EdictWidget && CloseEdicts)
        EdictWidget->SetOpen(false);
}

void CTopHudWidget::OnConstructionButtonClick()
{
    if (mGameLost)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    CloseMenus(false, true, true);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto BuildMenu =
        UIManager->FindWidget<CBuildMenuWidget>(GBuildMenuWidgetName).lock();

    if (BuildMenu)
        BuildMenu->ToggleBuildMenu();
}

void CTopHudWidget::OnAlmanacButtonClick()
{
    if (mGameLost)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    CloseMenus(true, false, true);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto AlmanacWidget =
        UIManager->FindWidget<CAlmanacWidget>(GAlmanacWidgetName).lock();

    if (AlmanacWidget)
        AlmanacWidget->ToggleOpen();
}

void CTopHudWidget::OnEdictsButtonClick()
{
    if (mGameLost)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    CloseMenus(true, true, false);

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto EdictWidget =
        UIManager->FindWidget<CEdictWidget>(GEdictWidgetName).lock();

    if (EdictWidget)
        EdictWidget->ToggleOpen();
}

void CTopHudWidget::OnAnyButtonClick()
{
}
