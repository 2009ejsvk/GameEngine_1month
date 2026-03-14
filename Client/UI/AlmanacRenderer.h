#pragma once

#include "AlmanacDataProvider.h"

class CAlmanacWidget;
struct FAlmanacLayoutContext;
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
    static void CreateChromeWidgets(CAlmanacWidget& Widget);
    static void CreateTabWidgets(CAlmanacWidget& Widget);
    static void CreatePageContainers(CAlmanacWidget& Widget);
    static void CreateOverviewWidgets(CAlmanacWidget& Widget);
    static void CreateSatisfactionWidgets(CAlmanacWidget& Widget);
    static void CreatePopulationWidgets(CAlmanacWidget& Widget);
    static void CreateEconomyWidgets(CAlmanacWidget& Widget);
    static void CreateResourceWidgets(CAlmanacWidget& Widget);
    static void CreatePoliticsWidgets(CAlmanacWidget& Widget);
    static void CreateForeignWidgets(CAlmanacWidget& Widget);
    static void CreateBuildingWidgets(CAlmanacWidget& Widget);
    static void CreateConflictWidgets(CAlmanacWidget& Widget);
    static void RefreshChromeLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void RefreshNavigationLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void RefreshPageContainerLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void RefreshOverviewLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void RefreshSatisfactionLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void RefreshPopulationLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void RefreshEconomyLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void RefreshResourceLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void RefreshPoliticsLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void RefreshForeignLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void RefreshBuildingLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void RefreshConflictLayout(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
    static void ApplyOverviewPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void ApplySatisfactionPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void ApplyPopulationPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void ApplyEconomyPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void ApplyResourcePage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void ApplyPoliticsPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void ApplyForeignPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void ApplyBuildingPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot);
    static void ApplyConflictPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot,
        const AlmanacRendererCalc::FConflictPageComputedData& ComputedData);
    static void ValidateLayoutForDebug(
        CAlmanacWidget& Widget,
        const FAlmanacLayoutContext& Layout);
};
