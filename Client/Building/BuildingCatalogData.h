#pragma once

#include "BuildingCatalogQuery.h"
#include "BuildingCatalogFwd.h"
#include "BuildingTypes.h"

namespace BuildingCatalogData
{
    void ResetRuntimeDefaults();
    void ReloadCatalogStore();
    unsigned long long GetGeneration();
    const FBuildingCatalogEntry* FindBuildingCatalogEntryByCategoryLocalIndex(
        EBuildingCategory Category,
        int CategoryLocalIndex);
}
