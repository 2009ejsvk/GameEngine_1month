#pragma once

#include "BuildingCatalogAspectData.h"
#include "BuildingCatalogFwd.h"
#include <string>

const FBuildingCatalogEntry* FindBuildingCatalogEntry(const std::string& EntryId);

const FBuildingCatalogRuntimeData* FindBuildingCatalogRuntimeData(
    const std::string& EntryId);
const FBuildingCatalogCitizenData* FindBuildingCatalogCitizenData(
    const std::string& EntryId);
const FBuildingCatalogPoliticalData* FindBuildingCatalogPoliticalData(
    const std::string& EntryId);

std::wstring GetOperationModeDisplayName(
    const FBuildingCatalogRuntimeData& Entry,
    int ModeIndex);
std::wstring GetOperationModeEffectSummary(
    const FBuildingCatalogRuntimeData& Entry,
    int ModeIndex);
std::wstring GetOperationModeTransitionNotice(
    const FBuildingCatalogRuntimeData& Entry);
std::wstring GetRuntimeUpgradeDisplayName(
    const FBuildingCatalogRuntimeData& Entry,
    int UpgradeIndex);
std::wstring GetRuntimeUpgradeEffectSummary(
    const FBuildingCatalogRuntimeData& Entry,
    int UpgradeIndex);
