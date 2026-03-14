#pragma once

#include "AlmanacDataProvider.h"

class CAlmanacWidget;

namespace AlmanacRendererPopulationPage
{
    void Apply(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
}
