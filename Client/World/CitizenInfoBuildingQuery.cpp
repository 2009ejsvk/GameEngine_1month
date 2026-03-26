#include "CitizenInfoWorldQuerySourceInternal.h"
#include "../StringUtils.h"
#include "../World/MainWorldConfig.h"

using namespace CitizenInfoWorldQuerySourceInternal;

bool CWorldCitizenInfoQuerySource::TryGetBuildingRecord(
    const std::string& BuildingName,
    CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord) const
{
    return mBuildingQuery.TryGetBuildingRecord(
        BuildingName,
        OutRecord);
}

bool CWorldCitizenInfoBuildingQuery::TryGetBuildingRecord(
    const std::string& BuildingName,
    CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord) const
{
    auto Building = FindValidBuilding(BuildingName);

    if (!Building)
        return false;

    OutRecord =
        CitizenInfoDataProvider::FCitizenInfoBuildingRecord();
    OutRecord.Valid = true;
    OutRecord.ObjectName = StringUtils::Utf8ToWide(Building->GetName());
    OutRecord.DisplayName =
        StringUtils::Utf8ToWide(Building->GetBuildingDisplayName());
    OutRecord.CategoryName =
        StringUtils::Utf8ToWide(Building->GetBuildingCategoryName());
    OutRecord.BuildingId = Building->GetBuildingId();
    const FBuildingCatalogEntry* const CatalogEntry =
        FindBuildingCatalogEntry(OutRecord.BuildingId);
    OutRecord.Residential = Building->IsResidential();
    OutRecord.WorkProvider =
        !OutRecord.Residential &&
        Building->GetCapacity() > 0;
    OutRecord.FoodProvider = Building->IsFoodProvider();
    OutRecord.EntertainmentProvider =
        Building->IsEntertainmentProvider();
    OutRecord.HealthProvider = Building->IsHealthProvider();
    OutRecord.FaithProvider = Building->IsFaithProvider();
    OutRecord.Harbor = Building->IsHarbor();
    OutRecord.Warehouse = Building->IsWarehouse();
    OutRecord.IsRoad = Building->IsRoad();
    OutRecord.CanGenerateWorkOutput =
        Building->CanGenerateWorkOutput();
    OutRecord.Capacity = (std::max)(0, Building->GetCapacity());
    OutRecord.BudgetLevel = Building->GetBudgetLevel();
    OutRecord.BudgetScale = Building->GetBudgetSatisfactionScale();
    OutRecord.AccessibilityScore =
        Building->GetAccessibilityScore();
    OutRecord.HousingCap = Building->GetHousingSatisfactionCap();
    OutRecord.JobCap = Building->GetJobSatisfactionCap();
    OutRecord.FoodCap = Building->GetFoodSatisfactionCap();
    OutRecord.FunCap = Building->GetFunSatisfactionCap();
    OutRecord.HealthCap = Building->GetHealthSatisfactionCap();
    OutRecord.FaithCap = Building->GetFaithSatisfactionCap();
    OutRecord.ServiceCapacity =
        Building->GetMaxServiceVisitCapacity();
    OutRecord.PollutionOutput = Building->GetPollutionOutput();
    OutRecord.PollutionMitigation =
        Building->GetPollutionMitigation();
    OutRecord.LocalPollutionExposure =
        Building->GetLocalPollutionExposure();
    OutRecord.ResourceStock = Building->GetResourceStock();
    OutRecord.ExportableStock =
        Building->GetExportableResourceStock();
    OutRecord.MaxResourceStock = Building->GetMaxResourceStock();
    PopulateProductionResourceView(*Building, CatalogEntry, OutRecord);
    const CitizenInfoDataProvider::EProductionChainStage RuntimeChainStage =
        CatalogEntry ?
            BuildRuntimeProductionChainStage(
                *Building,
                *CatalogEntry) :
            CitizenInfoDataProvider::EProductionChainStage::None;
    OutRecord.ChainStage = RuntimeChainStage;
    OutRecord.ProducedPowerMW = Building->GetProducedPowerMW();
    OutRecord.RequiredPowerMW = Building->GetRequiredPowerMW();
    OutRecord.PowerSupplyRatio = Building->GetPowerSupplyRatio();
    OutRecord.LastProductionEfficiency =
        Building->GetLastProductionEfficiency();
    OutRecord.DamageEfficiencyMultiplier =
        Building->GetDamageEfficiencyMultiplier();
    OutRecord.HarborShipProgressPercent =
        Building->GetHarborShipProgressPercent();
    OutRecord.ActiveOperationModeIndex =
        Building->GetActiveOperationModeIndex();
    OutRecord.ActiveRuntimeUpgradeIndex =
        Building->GetActiveRuntimeUpgradeIndex();
    OutRecord.DamageLevel = Building->GetDamageLevel();
    OutRecord.RepairCost = Building->GetRepairCost();
    OutRecord.RepairAffordable =
        OutRecord.RepairCost <= 0 ||
        (mOwner.mMainWorldAccess &&
            static_cast<long long>(OutRecord.RepairCost) <=
                mOwner.mMainWorldAccess->GetNationalBudget());
    OutRecord.ActiveOperationModeText =
        Building->GetActiveOperationModeDisplayName();
    OutRecord.ActiveOperationModeEffectSummary =
        Building->GetActiveOperationModeEffectSummary();
    OutRecord.ActiveRuntimeUpgradeText =
        Building->GetActiveRuntimeUpgradeDisplayName();
    OutRecord.ActiveRuntimeUpgradeEffectSummary =
        Building->GetActiveRuntimeUpgradeEffectSummary();
    OutRecord.KnowledgePoints =
        mOwner.mMainWorldKnowledgeAccess ?
            mOwner.mMainWorldKnowledgeAccess->GetKnowledgePoints() :
            0;
    OutRecord.DailyKnowledgeGeneration =
        mOwner.mMainWorldKnowledgeAccess ?
            mOwner.mMainWorldKnowledgeAccess->GetDailyKnowledgeGeneration() :
            0;

    const int OperationModeCount = Building->GetOperationModeCount();

    for (int ModeIndex = 0; ModeIndex < OperationModeCount; ++ModeIndex)
    {
        OutRecord.OperationModeResearchLocked.push_back(
            Building->IsOperationModeResearchLocked(ModeIndex));
        OutRecord.OperationModeResearchCosts.push_back(
            Building->GetOperationModeResearchCost(ModeIndex));
        OutRecord.OperationModeResearchLabels.push_back(
            Building->GetOperationModeResearchLabel(ModeIndex));
    }

    OutRecord.ProductionChainStageText =
        RuntimeChainStage !=
            CitizenInfoDataProvider::EProductionChainStage::None ?
            std::wstring(
                GetProductionChainStageDisplayName(
                    RuntimeChainStage)) :
            std::wstring();
    OutRecord.SupplyChainSummaryText =
        CatalogEntry ?
            BuildRuntimeProductionChainSummary(
                *Building,
                *CatalogEntry) :
            std::wstring();
    OutRecord.RequiredEducationLevel =
        Building->GetRequiredEducationLevel();
    OutRecord.UsesResourceStock =
        OutRecord.ResourceStock > 0 ||
        OutRecord.CanGenerateWorkOutput ||
        OutRecord.ProducedResourceType != EResourceType::None ||
        HasProductionInputSlots(OutRecord.ProductionInputs) ||
        HasCatalogProductionIdentity(CatalogEntry) ||
        OutRecord.FoodProvider ||
        OutRecord.Harbor ||
        OutRecord.Warehouse;
    OutRecord.DaysInMonth = mOwner.mMainWorldAccess ?
        (std::max)(
            1,
            mOwner.mMainWorldAccess->GetSimulationMonthDayCount()) :
        30;
    OutRecord.MonthlyWageCost = Building->GetMonthlyWageCost();
    OutRecord.MonthlyUpkeepCost = Building->GetMonthlyUpkeepCost();
    OutRecord.DailyWageCost =
        Building->GetDailyWageCost(OutRecord.DaysInMonth);
    OutRecord.DailyUpkeepCost =
        Building->GetDailyUpkeepCost(OutRecord.DaysInMonth);
    PopulateProductionFlowMetrics(Building, CatalogEntry, OutRecord);

    if (OutRecord.Warehouse)
    {
        OutRecord.WarehousePolicySelectionText =
            Building->GetWarehouseStoragePolicyDisplayName();
        OutRecord.WarehousePrioritySelectionText =
            Building->GetWarehousePriorityDisplayName();

        for (int SlotIndex = 0;
            SlotIndex < Building->GetWarehouseSlotCount();
            ++SlotIndex)
        {
            CitizenInfoDataProvider::FWarehouseSlotRecord SlotRecord;
            SlotRecord.Type =
                Building->GetWarehouseSlotType(SlotIndex);
            SlotRecord.Capacity =
                Building->GetWarehouseSlotCapacityUnits();
            SlotRecord.Stock =
                SlotRecord.Type == EResourceType::None ?
                    0 :
                    Building->GetResourceStock(SlotRecord.Type);
            if (SlotRecord.Type != EResourceType::None)
            {
                SlotRecord.Capacity =
                    Building->GetResourceTypeCapacity(SlotRecord.Type);
            }
            OutRecord.WarehouseSlots.push_back(SlotRecord);
        }
    }

    PopulatePowerTotals(OutRecord);
    mOwner.mCitizenQuery.PopulateCitizenAssignments(
        BuildingName,
        *Building,
        OutRecord);
    mOwner.mTradeQuery.PopulateLogisticsLines(Building, OutRecord);

    if (OutRecord.Harbor)
    {
        mOwner.mTradeQuery.PopulateHarborTradePolicy(
            *Building,
            OutRecord);

        for (int TypeIndex = 1;
            TypeIndex < static_cast<int>(EResourceType::Count);
            ++TypeIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(TypeIndex);
            const int Stock =
                Building->GetResourceStock(ResourceType);

            if (Stock <= 0)
                continue;

            CitizenInfoDataProvider::FWarehouseSlotRecord Slot;
            Slot.Type = ResourceType;
            Slot.Stock = Stock;
            Slot.Capacity = 0;
            OutRecord.HarborResourceSlots.push_back(Slot);
        }
    }

    if (IsCustomsOfficeBuildingId(OutRecord.BuildingId))
    {
        mOwner.mTradeQuery.PopulateCustomsTradeSummary(OutRecord);
    }

    return true;
}

std::shared_ptr<CPlacementAreaObject>
    CWorldCitizenInfoBuildingQuery::FindValidBuilding(
        const std::string& BuildingName) const
{
    if (!mOwner.mWorld || BuildingName.empty())
        return nullptr;

    auto Building =
        mOwner.mWorld->FindObject<CPlacementAreaObject>(BuildingName).lock();

    if (!Building || !Building->GetAlive() || !Building->GetEnable())
        return nullptr;

    return Building;
}

int CWorldCitizenInfoBuildingQuery::CountActiveCitizenOrbs() const
{
    if (!mOwner.mWorld)
        return 0;

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

    if (!mOwner.mWorld->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        return 0;

    int Count = 0;

    for (size_t Index = 0; Index < OrbList.size(); ++Index)
    {
        const auto Orb = OrbList[Index].lock();

        if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
            continue;

        ++Count;
    }

    return Count;
}

void CWorldCitizenInfoBuildingQuery::PopulateProductionFlowMetrics(
    const std::shared_ptr<CPlacementAreaObject>& Building,
    const FBuildingCatalogEntry* CatalogEntry,
    CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord) const
{
    if (!Building)
        return;

    OutRecord.CurrentWorkerOccupancy = (std::max)(
        0,
        Building->GetCurrentWorkerOccupancy());
    OutRecord.WorkingNowOccupancy = (std::max)(
        0,
        Building->GetWorkingNowOccupancy());

    if (!Building->CanGenerateWorkOutput() ||
        Building->GetProducedResourceType() == EResourceType::None)
    {
        return;
    }

    const float BaseUnitsPerSecond =
        ResolveBaseProductionUnitsPerSecond(*Building);

    if (BaseUnitsPerSecond <= 0.f)
        return;

    std::array<EResourceType, GProductionInputSlotCount> InputTypes = {};
    const FBuildingOperationModeEffect RuntimeEffect =
        ResolveCatalogRuntimeEffect(
            CatalogEntry,
            Building->GetActiveOperationModeIndex(),
            Building->GetActiveRuntimeUpgradeIndex());

    for (int SlotIndex = 0;
        SlotIndex < GProductionInputSlotCount;
        ++SlotIndex)
    {
        InputTypes[static_cast<size_t>(SlotIndex)] =
            SlotIndex < Building->GetProductionInputCount() ?
                Building->GetProductionInputType(SlotIndex) :
                EResourceType::None;
    }

    FGovernmentEdictModifiers EdictModifiers;
    const FGovernmentProfile* const GovernmentProfile =
        mOwner.mMainWorldPolicyAccess ?
            &mOwner.mMainWorldPolicyAccess->GetGovernmentProfile() :
            nullptr;
    const FTaxPolicyEventStatus* const TaxEventStatus =
        mOwner.mMainWorldPolicyAccess ?
            &mOwner.mMainWorldPolicyAccess->GetTaxPolicyEventStatus() :
            nullptr;
    const FWorldCrisisStatus* const WorldCrisisStatus =
        mOwner.mMainWorldPolicyAccess ?
            &mOwner.mMainWorldPolicyAccess->GetWorldCrisisStatus() :
            nullptr;

    if (mOwner.mMainWorldPolicyAccess)
    {
        EdictModifiers =
            EdictSystem::CalculateEdictModifiers(
                mOwner.mMainWorldPolicyAccess->GetGovernmentEdictStates(),
                CountActiveCitizenOrbs());
    }

    const float TradePolicyProductionMultiplier =
        GovernmentProfile ?
            TradePolicyRuntime::ComputeBuildingProductionMultiplier(
                Building->GetProducedResourceType(),
                InputTypes,
                GovernmentProfile->ExportTradePolicy,
                GovernmentProfile->ImportTradePolicy) :
            1.f;
    const float NominalUnitsPerSecond =
        BaseUnitsPerSecond *
        (std::max)(0.f, EdictModifiers.ProductionMultiplier) *
        ResolveTaxEventProductionMultiplier(TaxEventStatus) *
        ResolveWorldCrisisProductionMultiplier(WorldCrisisStatus) *
        TradePolicyProductionMultiplier *
        (std::max)(0.f, RuntimeEffect.ProductionMultiplier) *
        (std::max)(0.f, Building->GetBudgetSatisfactionScale()) *
        Building->GetDamageEfficiencyMultiplier() *
        ResolveUiPowerOperationalMultiplier(*Building);
    const float EffectiveProductionEfficiency =
        (OutRecord.CurrentWorkerOccupancy > 0 &&
            OutRecord.Capacity > 0) ?
            (std::max)(
                0.f,
                (std::min)(
                    1.f,
                    Building->GetLastProductionEfficiency())) :
            0.f;

    OutRecord.CurrentProductionUnitsPerSecond =
        NominalUnitsPerSecond * EffectiveProductionEfficiency;
    const float DailyProductionUnits =
        OutRecord.CurrentProductionUnitsPerSecond *
        MainWorldConfig::GSecondsPerSimulationDay;
    OutRecord.EstimatedDailyProductionUnits = (std::max)(
        0,
        static_cast<int>(roundf(DailyProductionUnits)));
    OutRecord.EstimatedMonthlyProductionUnits = (std::max)(
        0,
        static_cast<int>(roundf(
            DailyProductionUnits *
            static_cast<float>((std::max)(1, OutRecord.DaysInMonth)))));

    const float EffectiveInputConsumptionMultiplier = (std::max)(
        0.f,
        RuntimeEffect.InputConsumptionMultiplier);

    for (size_t Index = 0; Index < OutRecord.ProductionInputs.size();
        ++Index)
    {
        CitizenInfoDataProvider::FProductionInputSlotView& InputRecord =
            OutRecord.ProductionInputs[Index];
        if (!IsValidProductionInputSlot(InputRecord))
            continue;
        const float ConsumptionUnitsPerSecond =
            OutRecord.CurrentProductionUnitsPerSecond *
            static_cast<float>((std::max)(1, InputRecord.RequiredAmount)) *
            EffectiveInputConsumptionMultiplier;

        InputRecord.ConsumptionUnitsPerSecond =
            ConsumptionUnitsPerSecond;
        InputRecord.EstimatedDailyConsumptionUnits = (std::max)(
            0,
            static_cast<int>(roundf(
                ConsumptionUnitsPerSecond *
                MainWorldConfig::GSecondsPerSimulationDay)));
        InputRecord.EstimatedMonthlyConsumptionUnits = (std::max)(
            0,
            static_cast<int>(roundf(
                static_cast<float>(
                    InputRecord.EstimatedDailyConsumptionUnits) *
                static_cast<float>((std::max)(
                    1,
                    OutRecord.DaysInMonth)))));
    }
}

void CWorldCitizenInfoBuildingQuery::PopulatePowerTotals(
    CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord) const
{
    if (!mOwner.mWorld)
        return;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!mOwner.mWorld->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return;

    for (size_t Index = 0; Index < BuildingList.size(); ++Index)
    {
        auto OtherBuilding = BuildingList[Index].lock();

        if (!OtherBuilding ||
            !OtherBuilding->GetAlive() ||
            !OtherBuilding->GetEnable() ||
            !OtherBuilding->HasPlacedArea())
        {
            continue;
        }

        const FBuildingCatalogEntry* Entry =
            FindBuildingCatalogEntry(OtherBuilding->GetBuildingId());

        if (!Entry)
            continue;

        OutRecord.TotalProducedPowerMW +=
            (std::max)(0, OtherBuilding->GetProducedPowerMW());
        OutRecord.TotalRequiredPowerMW +=
            (std::max)(0, OtherBuilding->GetRequiredPowerMW());
    }
}
