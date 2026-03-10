#pragma once

#include "CitizenInfoDataProvider.h"

class CCitizenInfoWidget;

class FCitizenInfoRenderer final
{
public:
    static void CreateWidgets(CCitizenInfoWidget& Widget);
    static void ApplySnapshot(
        CCitizenInfoWidget& Widget,
        const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot);
    static void RefreshLayout(CCitizenInfoWidget& Widget);
};
