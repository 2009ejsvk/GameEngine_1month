#pragma once

#include "CitizenInfoDataProvider.h"
#include "../Building/BuildingCatalogFwd.h"
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace CitizenInfoBuildingRuntime
{
    struct FBuildingUiSnapshot :
        CitizenInfoDataProvider::FCitizenInfoBuildingRecord
    {
        const FBuildingCatalogEntry* CatalogEntry = nullptr;
        std::wstring DetailText;
        std::wstring JobQualityText;
        std::wstring ServiceQualityText;
        std::wstring HousingQualityText;
        std::wstring HouseholdCapacityText;
        std::wstring WealthRequirementText;
        std::wstring TouristPreferenceText;
        std::wstring EffectText;
        std::wstring NoteText;
        std::wstring ServiceCapacityText;
        std::vector<std::wstring> NarrativeLines;
        std::vector<std::wstring> UpgradeHints;
        std::vector<std::wstring> OperationModes;
        std::vector<std::wstring> WarehouseSlotLines;
        std::vector<std::wstring> HarborResourceLines;
        EBuildingCostState BlueprintCostState = EBuildingCostState::None;
        int BlueprintCost = 0;
        EBuildingCostState ConstructionCostState = EBuildingCostState::None;
        int ConstructionCost = 0;
        int HouseholdCapacity = 0;
        bool ServiceCapacityUsesHouseholds = false;
    };

    bool BuildBuildingUiSnapshot(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>&
            QuerySource,
        const std::string& BuildingName,
        FBuildingUiSnapshot& OutSnapshot);

    bool IsHydroponicFarmBuilding(const FBuildingUiSnapshot& Snapshot);
    bool IsCustomsOfficeBuilding(const FBuildingUiSnapshot& Snapshot);
    int ComputeAverageCustomsDiplomacyExportBiasPercent();
    int ResolveCustomsBudgetModifierPercent(const FBuildingUiSnapshot& Snapshot);
    int ResolveCustomsEfficiencyPercent(const FBuildingUiSnapshot& Snapshot);
    std::wstring ResolveCustomsPerWorkerWage(
        const FBuildingUiSnapshot& Snapshot);
    std::wstring ResolveCustomsModeDescription(
        const FBuildingUiSnapshot& Snapshot,
        int ModeIndex);
}
