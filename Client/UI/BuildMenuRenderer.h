#pragma once

#include "BuildMenuDataProvider.h"

struct FVector2;
class CBuildMenuWidget;

class FBuildMenuRenderer final
{
public:
    static void CreateWidgets(CBuildMenuWidget& Widget);
    static void ApplySnapshot(
        CBuildMenuWidget& Widget,
        const BuildMenuDataProvider::FBuildMenuSnapshot& Snapshot);
    static void RefreshLayout(CBuildMenuWidget& Widget);
    static bool IsMouseOverPanel(
        const CBuildMenuWidget& Widget,
        const FVector2& MousePos);

private:
    static void CreateHudWidgets(CBuildMenuWidget& Widget);
    static void CreateYearbookWidgets(CBuildMenuWidget& Widget);
    static void CreateMenuFrameWidgets(CBuildMenuWidget& Widget);
    static void CreateCategoryButtons(CBuildMenuWidget& Widget);
    static void CreateBuildingButtons(CBuildMenuWidget& Widget);
    static void CreateDetailWidgets(CBuildMenuWidget& Widget);
    static void ApplyMenuOpenState(CBuildMenuWidget& Widget);
    static void ApplyYearbookOpenState(CBuildMenuWidget& Widget);
};
