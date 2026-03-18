#pragma once

#include "../Building/BuildingCatalogFwd.h"
#include "../Building/BuildingTypes.h"
#include "../Citizen/CitizenTypes.h"
#include "../Map/PlacementAreaRuntimeState.h"
#include "CitizenInfoSnapshot.h"
#include "Vector4.h"
#include <array>
#include <memory>
#include <string>
#include <tchar.h>
#include <vector>

class CWorld;

namespace CitizenInfoDataProvider
{
    using EProductionChainStage = EBuildingProductionChainStage;

    struct FWarehouseSlotRecord
    {
        EResourceType Type = EResourceType::None;
        int Stock = 0;
        int Capacity = 0;
    };

    struct FProductionInputSlotView
    {
        EResourceType Type = EResourceType::None;
        int RequiredAmount = 0;
        int CurrentStock = 0;
        int MaxStock = 0;
        float ConsumptionUnitsPerSecond = 0.f;
        int EstimatedDailyConsumptionUnits = 0;
        int EstimatedMonthlyConsumptionUnits = 0;
    };

    struct FCitizenInfoBuildingRecord
    {
        bool Valid = false;
        std::wstring ObjectName;
        std::wstring DisplayName;
        std::wstring CategoryName;
        std::string BuildingId;
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
        int Capacity = 0;
        int CurrentWorkerOccupancy = 0;
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
        int ServiceCapacity = 0;
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
        int TotalProducedPowerMW = 0;
        int TotalRequiredPowerMW = 0;
        long long LastDailyExportIncome = 0;
        long long LastDailyImportExpense = 0;
        int TradeRouteExportFulfilledUnits = 0;
        int TradeRouteImportFulfilledUnits = 0;
        int TradeRouteExportContractUnits = 0;
        int TourismArrivalCount = 0;
        EProductionChainStage ChainStage =
            EProductionChainStage::None;
        float BudgetScale = 1.f;
        float AccessibilityScore = 0.f;
        float PowerSupplyRatio = 1.f;
        float LastProductionEfficiency = 1.f;
        float DamageEfficiencyMultiplier = 1.f;
        float HarborShipProgressPercent = 0.f;
        int ActiveOperationModeIndex = 0;
        int ActiveRuntimeUpgradeIndex = -1;
        int KnowledgePoints = 0;
        int DailyKnowledgeGeneration = 0;
        int RepairCost = 0;
        bool RepairAffordable = false;
        EBuildingDamageLevel DamageLevel = EBuildingDamageLevel::None;
        ECitizenEducationLevel RequiredEducationLevel =
            ECitizenEducationLevel::Uneducated;
        std::wstring ActiveOperationModeText;
        std::wstring ActiveOperationModeEffectSummary;
        std::wstring ActiveRuntimeUpgradeText;
        std::wstring ActiveRuntimeUpgradeEffectSummary;
        std::wstring ProductionChainStageText;
        std::wstring SupplyChainSummaryText;
        std::vector<std::wstring> LogisticsLines;
        std::vector<bool> OperationModeResearchLocked;
        std::vector<int> OperationModeResearchCosts;
        std::vector<std::wstring> OperationModeResearchLabels;
        std::vector<FWarehouseSlotRecord> WarehouseSlots;
        std::vector<FWarehouseSlotRecord> HarborResourceSlots;
        std::array<FProductionInputSlotView, GProductionInputSlotCount>
            ProductionInputs = {};
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
    };

    struct FCitizenInfoCitizenRecord
    {
        bool Valid = false;
        std::string Name;
        FNpcSatisfaction Satisfaction;
        FCitizenIdentityProfile IdentityProfile;
        FNpcPoliticalProfile PoliticalProfile;
        ECitizenState State = ECitizenState::Wander;
        std::string HomeBuildingName;
        std::string WorkBuildingName;
        std::string FoodBuildingName;
        std::string FoodVisitBuildingName;
        std::string FunBuildingName;
        std::string FunVisitBuildingName;
        std::string HealthBuildingName;
        std::string HealthVisitBuildingName;
        std::string FaithBuildingName;
        std::string FaithVisitBuildingName;
    };

    class ICitizenInfoQuerySource
    {
    public:
        virtual ~ICitizenInfoQuerySource() = default;

        virtual bool TryGetBuildingRecord(
            const std::string& BuildingName,
            FCitizenInfoBuildingRecord& OutRecord) const = 0;
        virtual bool TryGetCitizenRecord(
            const std::string& CitizenName,
            FCitizenInfoCitizenRecord& OutRecord) const = 0;
        virtual std::wstring ResolveBuildingDisplayName(
            const std::string& BuildingName) const = 0;
    };

    FCitizenInfoSnapshot BuildCitizenSnapshot(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FCitizenIdentityProfile& IdentityProfile,
        const FNpcPoliticalProfile& PoliticalProfile,
        int TabIndex = 0);

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex = 0);

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex = 0);

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex,
        bool ShowCustomsModeSelection = false,
        int OperationModeSelectionPageIndex = -1,
        int OverviewMetricScrollOffset = 0);

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex,
        bool ShowCustomsModeSelection = false,
        int OperationModeSelectionPageIndex = -1,
        int OverviewMetricScrollOffset = 0);
}
