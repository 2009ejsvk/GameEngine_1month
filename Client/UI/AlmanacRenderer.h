#pragma once

#include "AlmanacDataProvider.h"

class CAlmanacWidget;
namespace AlmanacRendererCalc
{
    struct FConflictPageComputedData;
}

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

private:
    static void ApplyOverviewPage(CAlmanacWidget& Widget);
    static void ApplySatisfactionPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void ApplyPopulationPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void ApplyEconomyPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void ApplyResourcePage(CAlmanacWidget& Widget);
    static void ApplyPoliticsPage(CAlmanacWidget& Widget);
    static void ApplyForeignPage(CAlmanacWidget& Widget);
    static void ApplyBuildingPage(CAlmanacWidget& Widget);
    static void ApplyConflictPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot,
        const AlmanacRendererCalc::FConflictPageComputedData& ComputedData);
};
