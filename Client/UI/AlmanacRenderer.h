#pragma once

#include "AlmanacDataProvider.h"

class CAlmanacWidget;

class FAlmanacRenderer final
{
public:
    static void CreateWidgets(CAlmanacWidget& Widget);
    static void ApplySnapshot(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void RefreshLayout(CAlmanacWidget& Widget);
    static void ApplyOpenState(CAlmanacWidget& Widget);
    static void ApplySelectedPage(CAlmanacWidget& Widget);
};
