#pragma once

#include "TopHudDataProvider.h"

class CTopHudWidget;

class FTopHudRenderer final
{
public:
    static void CreateWidgets(CTopHudWidget& Widget);
    static void ApplySnapshot(
        CTopHudWidget& Widget,
        const TopHudDataProvider::FTopHudSnapshot& Snapshot);
    static void RefreshLayout(CTopHudWidget& Widget);
};
