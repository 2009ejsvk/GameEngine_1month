#include "BuildingCatalogData.h"
#include "BuildingCatalogLoader.h"
#include <algorithm>
#include <memory>
#include <vector>

namespace
{
    constexpr size_t GRetiredCatalogSnapshotLimit = 4;

    std::shared_ptr<const std::vector<FBuildingCatalogEntry>>
        GCurrentBuildingCatalog;
    std::vector<std::shared_ptr<const std::vector<FBuildingCatalogEntry>>>
        GRetiredBuildingCatalogs;
    unsigned long long GBuildingCatalogGeneration = 0;

    void ReplaceBuildingCatalogStore(
        std::vector<FBuildingCatalogEntry>&& Entries)
    {
        if (GCurrentBuildingCatalog)
        {
            GRetiredBuildingCatalogs.push_back(GCurrentBuildingCatalog);

            while (GRetiredBuildingCatalogs.size() >
                GRetiredCatalogSnapshotLimit)
            {
                GRetiredBuildingCatalogs.erase(
                    GRetiredBuildingCatalogs.begin());
            }
        }

        GCurrentBuildingCatalog =
            std::make_shared<const std::vector<FBuildingCatalogEntry>>(
                std::move(Entries));
        ++GBuildingCatalogGeneration;
    }
}

namespace BuildingCatalogData
{
    void ResetRuntimeDefaults()
    {
    }

    void ReloadCatalogStore()
    {
        ReplaceBuildingCatalogStore(
            BuildingCatalogLoader::BuildBuildingCatalogEntries());
    }

    const std::vector<FBuildingCatalogEntry>& ResolveCurrentCatalog()
    {
        if (!GCurrentBuildingCatalog)
            ReloadCatalogStore();

        return *GCurrentBuildingCatalog;
    }

    unsigned long long GetGeneration()
    {
        return GBuildingCatalogGeneration;
    }

    const FBuildingCatalogEntry* FindBuildingCatalogEntryByCategoryLocalIndex(
        EBuildingCategory Category,
        int CategoryLocalIndex)
    {
        const auto& Catalog = ResolveCurrentCatalog();
        const auto It = std::find_if(
            Catalog.begin(),
            Catalog.end(),
            [&](const FBuildingCatalogEntry& Entry)
            {
                return Entry.Category == Category &&
                    Entry.CategoryLocalIndex == CategoryLocalIndex;
            });

        return It != Catalog.end() ? &(*It) : nullptr;
    }
}

const std::vector<FBuildingCatalogEntry>& GetBuildingCatalog()
{
    return BuildingCatalogData::ResolveCurrentCatalog();
}

unsigned long long GetRuntimeConfigGeneration()
{
    return BuildingCatalogData::GetGeneration();
}

const FBuildingCatalogEntry* FindBuildingCatalogEntry(const std::string& EntryId)
{
    const auto& Catalog = GetBuildingCatalog();
    const auto It = std::find_if(
        Catalog.begin(),
        Catalog.end(),
        [&](const FBuildingCatalogEntry& Entry)
        {
            return Entry.Id == EntryId;
        });

    return It != Catalog.end() ? &(*It) : nullptr;
}

const FBuildingCatalogRuntimeData* FindBuildingCatalogRuntimeData(
    const std::string& EntryId)
{
    const FBuildingCatalogEntry* const Entry = FindBuildingCatalogEntry(EntryId);
    return Entry ? static_cast<const FBuildingCatalogRuntimeData*>(Entry) : nullptr;
}

const FBuildingCatalogCitizenData* FindBuildingCatalogCitizenData(
    const std::string& EntryId)
{
    const FBuildingCatalogEntry* const Entry = FindBuildingCatalogEntry(EntryId);
    return Entry ? static_cast<const FBuildingCatalogCitizenData*>(Entry) : nullptr;
}

const FBuildingCatalogPoliticalData* FindBuildingCatalogPoliticalData(
    const std::string& EntryId)
{
    const FBuildingCatalogEntry* const Entry = FindBuildingCatalogEntry(EntryId);
    return Entry ? static_cast<const FBuildingCatalogPoliticalData*>(Entry) : nullptr;
}
