#pragma once

#include "CitizenInfoSnapshot.h"

class CCitizenInfoWidget;
struct FCitizenInfoLayoutContext;

class FCitizenInfoRenderer final
{
public:
    static void CreateWidgets(CCitizenInfoWidget& Widget);
    static void ApplySnapshot(
        CCitizenInfoWidget& Widget,
        const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot);
    static void RefreshLayout(CCitizenInfoWidget& Widget);

private:
    static void RefreshCommonLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshCitizenModeLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshCitizenThoughtsLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshCitizenPoliticsLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshCitizenProfileLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshBuildingModeLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshBuildingCustomOverviewLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshBuildingWorkOverviewLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshBuildingCompactLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshBuildingInformationLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshBuildingDefaultLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshInformationVisibilityLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshUpgradeLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshActionLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void RefreshBodyLayout(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
    static void ValidateSnapshotForDebug(
        const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot);
    static void ValidateLayoutForDebug(
        CCitizenInfoWidget& Widget,
        const FCitizenInfoLayoutContext& Layout);
};
