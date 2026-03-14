#include "AlmanacRenderer.h"
#include "AlmanacRendererPopulationPage.h"

void FAlmanacRenderer::ApplyPopulationPage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    AlmanacRendererPopulationPage::Apply(Widget, Snapshot);
}
