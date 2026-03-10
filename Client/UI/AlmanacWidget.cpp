#include "AlmanacWidget.h"
#include "AlmanacDataProvider.h"
#include "AlmanacRenderer.h"
#include "Device.h"
#include "World/World.h"
#include <cmath>

namespace
{
    constexpr float GDataRefreshIntervalSeconds = 0.5f;
}

CAlmanacWidget::CAlmanacWidget()
{
}

CAlmanacWidget::~CAlmanacWidget()
{
}

bool CAlmanacWidget::Init()
{
    CWidgetContainer::Init();

    mOpen = false;
    mSelectedPage = EAlmanacPage::Overview;
    mDataRefreshAccum = 0.f;
    mLayoutDirty = true;
    mLastResolutionWidth = 0;
    mLastResolutionHeight = 0;

    FAlmanacRenderer::CreateWidgets(*this);
    RefreshData();
    FAlmanacRenderer::RefreshLayout(*this);
    return true;
}

void CAlmanacWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    if (!mOpen)
        return;

    mDataRefreshAccum += DeltaTime;

    if (mDataRefreshAccum >= GDataRefreshIntervalSeconds)
    {
        RefreshData();
        mDataRefreshAccum =
            std::fmod(mDataRefreshAccum, GDataRefreshIntervalSeconds);
    }

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();

    if (mLastResolutionWidth != Resolution.Width ||
        mLastResolutionHeight != Resolution.Height)
    {
        mLayoutDirty = true;
    }

    if (mLayoutDirty)
        FAlmanacRenderer::RefreshLayout(*this);
}

void CAlmanacWidget::ToggleOpen()
{
    SetOpen(!mOpen);
}

void CAlmanacWidget::SetOpen(bool Open)
{
    if (mOpen == Open)
        return;

    mOpen = Open;

    if (mOpen)
    {
        mDataRefreshAccum = 0.f;
        mLayoutDirty = true;
        RefreshData();
        FAlmanacRenderer::RefreshLayout(*this);
    }

    FAlmanacRenderer::ApplyOpenState(*this);
}

void CAlmanacWidget::RefreshData()
{
    const auto Snapshot = AlmanacDataProvider::BuildSnapshot(mWorld.lock());
    FAlmanacRenderer::ApplySnapshot(*this, Snapshot);
}

void CAlmanacWidget::SelectPage(EAlmanacPage Page)
{
    if (mSelectedPage == Page)
        return;

    mSelectedPage = Page;
    FAlmanacRenderer::ApplySelectedPage(*this);
    mLayoutDirty = true;
    FAlmanacRenderer::RefreshLayout(*this);
}

void CAlmanacWidget::OnCloseButtonClick()
{
    SetOpen(false);
}

void CAlmanacWidget::OnOverviewTabClick()
{
    SelectPage(EAlmanacPage::Overview);
}

void CAlmanacWidget::OnSatisfactionTabClick()
{
    SelectPage(EAlmanacPage::Satisfaction);
}

void CAlmanacWidget::OnPopulationTabClick()
{
    SelectPage(EAlmanacPage::Population);
}

void CAlmanacWidget::OnEconomyTabClick()
{
    SelectPage(EAlmanacPage::Economy);
}

void CAlmanacWidget::OnResourcesTabClick()
{
    SelectPage(EAlmanacPage::Resources);
}

void CAlmanacWidget::OnPoliticsTabClick()
{
    SelectPage(EAlmanacPage::Politics);
}

void CAlmanacWidget::OnForeignTabClick()
{
    SelectPage(EAlmanacPage::Foreign);
}

void CAlmanacWidget::OnBuildingsTabClick()
{
    SelectPage(EAlmanacPage::Buildings);
}

void CAlmanacWidget::OnConflictTabClick()
{
    SelectPage(EAlmanacPage::Conflict);
}
