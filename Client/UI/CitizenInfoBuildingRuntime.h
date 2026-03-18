#pragma once

#include "CitizenInfoDataProvider.h"
#include "../Building/BuildingCatalogFwd.h"
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace CitizenInfoBuildingRuntime
{
    struct FBuildingUiSnapshot
    {
        const FBuildingCatalogEntry* CatalogEntry = nullptr;
        std::string BuildingId;
        std::wstring ObjectName;
        std::wstring DisplayName;
        std::wstring CategoryName;
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
        std::wstring ActiveOperationModeText;
        std::wstring ActiveOperationModeEffectSummary;
        std::wstring ActiveRuntimeUpgradeText;
        std::wstring ActiveRuntimeUpgradeEffectSummary;
        std::wstring ProductionChainStageText;
        std::wstring SupplyChainSummaryText;
        std::vector<std::wstring> NarrativeLines;
        std::vector<std::wstring> LogisticsLines;
        std::vector<std::wstring> UpgradeHints;
        std::vector<std::wstring> OperationModes;
        std::vector<std::wstring> WarehouseSlotLines;
        std::vector<std::wstring> HarborResourceLines;
        std::array<CitizenInfoDataProvider::FProductionInputSlotView,
            GProductionInputSlotCount> ProductionInputs = {};
        std::vector<std::wstring> HarborPolicyLines;
        std::vector<std::wstring> HarborPriorityLines;
        std::wstring WarehousePolicySelectionText;
        std::wstring WarehousePrioritySelectionText;
        std::wstring HarborExportSelectionText;
        std::vector<std::string> Residents;
        std::vector<std::string> AssignedEmployees;
        std::vector<std::string> WorkingEmployees;
        std::vector<std::string> AssignedVisitors;
        std::vector<std::string> ArrivedVisitors;
        std::vector<std::string> IncomingVisitors;
        EBuildingCostState BlueprintCostState = EBuildingCostState::None;
        int BlueprintCost = 0;
        EBuildingCostState ConstructionCostState = EBuildingCostState::None;
        int ConstructionCost = 0;
        int Capacity = 0;
        int CurrentWorkerOccupancy = 0;
        int HouseholdCapacity = 0;
        int BudgetLevel = 3;
        int DaysInMonth = 30;
        int MonthlyWageCost = 0;
        int MonthlyUpkeepCost = 0;
        int DailyWageCost = 0;
        int DailyUpkeepCost = 0;
        int HousingCap = 100;
        int JobCap = 100;
        int FoodCap = 100;
        int FunCap = 100;
        int HealthCap = 100;
        int FaithCap = 100;
        int PollutionOutput = 0;
        int PollutionMitigation = 0;
        int LocalPollutionExposure = 0;
        int ResourceStock = 0;
        int ExportableStock = 0;
        int MaxResourceStock = 0;
        EResourceType ProducedResourceType = EResourceType::None;
        int ProducedResourceStock = 0;
        float CurrentProductionUnitsPerSecond = 0.f;
        int EstimatedDailyProductionUnits = 0;
        int EstimatedMonthlyProductionUnits = 0;
        int ProducedPowerMW = 0;
        int RequiredPowerMW = 0;
        int ServiceCapacity = 0;
        bool ServiceCapacityUsesHouseholds = false;
        int TotalProducedPowerMW = 0;
        int TotalRequiredPowerMW = 0;
        long long LastDailyExportIncome = 0;
        long long LastDailyImportExpense = 0;
        int TradeRouteExportFulfilledUnits = 0;
        int TradeRouteImportFulfilledUnits = 0;
        int TradeRouteExportContractUnits = 0;
        int TourismArrivalCount = 0;
        CitizenInfoDataProvider::EProductionChainStage ChainStage =
            CitizenInfoDataProvider::EProductionChainStage::None;
        int ActiveOperationModeIndex = 0;
        int ActiveRuntimeUpgradeIndex = -1;
        int KnowledgePoints = 0;
        int DailyKnowledgeGeneration = 0;
        int RepairCost = 0;
        bool RepairAffordable = false;
        EBuildingDamageLevel DamageLevel = EBuildingDamageLevel::None;
        float BudgetScale = 1.f;
        float AccessibilityScore = 0.f;
        float PowerSupplyRatio = 1.f;
        float LastProductionEfficiency = 1.f;
        float DamageEfficiencyMultiplier = 1.f;
        float HarborShipProgressPercent = 0.f;
        ECitizenEducationLevel RequiredEducationLevel =
            ECitizenEducationLevel::Uneducated;
        bool Residential = false;
        bool WorkProvider = false;
        bool FoodProvider = false;
        bool EntertainmentProvider = false;
        bool HealthProvider = false;
        bool FaithProvider = false;
        bool UsesResourceStock = false;
        bool Harbor = false;
        bool Warehouse = false;
        bool IsRoad = false;
        bool CanGenerateWorkOutput = false;
        std::vector<bool> OperationModeResearchLocked;
        std::vector<int> OperationModeResearchCosts;
        std::vector<std::wstring> OperationModeResearchLabels;
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
