#include "AlmanacRenderer.h"
#include "AlmanacRendererCalc.h"

void FAlmanacRenderer::ApplySnapshot(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    ApplyOverviewPage(Widget);
    ApplySatisfactionPage(Widget, Snapshot);
    ApplyPopulationPage(Widget, Snapshot);
    ApplyEconomyPage(Widget, Snapshot);
    ApplyResourcePage(Widget);
    ApplyPoliticsPage(Widget);
    ApplyForeignPage(Widget);
    ApplyBuildingPage(Widget);
    ApplyConflictPage(
        Widget,
        Snapshot,
        AlmanacRendererCalc::BuildConflictPageComputedData(Snapshot));
}
