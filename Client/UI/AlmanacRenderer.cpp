#include "AlmanacRenderer.h"
#include "AlmanacCalc.h"

void FAlmanacRenderer::ApplySnapshot(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    ApplyOverviewPage(Widget, Snapshot);
    ApplySatisfactionPage(Widget, Snapshot);
    ApplyPopulationPage(Widget, Snapshot);
    ApplyEconomyPage(Widget, Snapshot);
    ApplyResourcePage(Widget, Snapshot);
    ApplyPoliticsPage(Widget, Snapshot);
    ApplyForeignPage(Widget, Snapshot);
    ApplyBuildingPage(Widget, Snapshot);
    ApplyConflictPage(
        Widget,
        Snapshot,
        AlmanacCalc::BuildConflictPageComputedData(Snapshot));
}
