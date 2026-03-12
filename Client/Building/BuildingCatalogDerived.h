#pragma once

#include "BuildingCatalog.h"
#include <vector>

EPlacementTemplateType ResolveLegacyTemplateTypeByBuildingId(
    const std::string& BuildingId);

void InitializeDerivedCatalogEntry(
    FBuildingCatalogEntry& Entry);

void FinalizeCatalogEntryAfterRecipeLoad(
    FBuildingCatalogEntry& Entry);

std::vector<std::wstring> ExtractUpgradeHintsFromDetail(
    const std::wstring& DetailText);

std::vector<FBuildingOperationModeDef> ExtractOperationModeDefsFromEntry(
    const FBuildingCatalogEntry& Entry);

std::vector<FBuildingRuntimeUpgradeDef> ExtractRuntimeUpgradeDefsFromEntry(
    const FBuildingCatalogEntry& Entry);

std::vector<std::wstring> SplitOperationModeSummaryClauses(
    const std::wstring& Text);

void ApplyOperationModeEffectClause(
    const FBuildingCatalogEntry& Entry,
    const std::wstring& RawClause,
    FBuildingOperationModeEffect& OutEffect);

void ApplyCatalogUiBehaviorFlags(
    std::vector<FBuildingCatalogEntry>& Entries);

void PopulateProductionChainMetadata(
    std::vector<FBuildingCatalogEntry>& Entries);
