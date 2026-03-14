#pragma once

#include "BuildingCatalog.h"

namespace BuildingCatalogData
{
    void ResetRuntimeDefaults();
    void ReloadCatalogStore();
    unsigned long long GetGeneration();
    const FBuildingCatalogEntry* FindBuildingCatalogEntryByCategoryLocalIndex(
        EBuildingCategory Category,
        int CategoryLocalIndex);
}
