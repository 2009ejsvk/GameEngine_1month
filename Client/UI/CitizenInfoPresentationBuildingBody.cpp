#include "CitizenInfoPresentationInternal.h"
#include "UIStrings.h"
#include "../Building/BuildingCatalog.h"
#include <algorithm>
#include <cmath>

using namespace CitizenInfoPresentationInternal;

namespace CitizenInfoPresentation
{
    std::wstring BuildOverviewBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body;

        if (!Snapshot.OperationModes.empty())
        {
            AppendLine(Body, Ui(L"citizen_info.section.operation_modes"));
            AppendLine(
                Body,
                L"  " +
                (Snapshot.ActiveOperationModeText.empty() ?
                    Snapshot.OperationModes.front() :
                    Snapshot.ActiveOperationModeText));

            if (!Snapshot.ActiveOperationModeEffectSummary.empty())
            {
                AppendLine(
                    Body,
                    L"  (" + Snapshot.ActiveOperationModeEffectSummary + L")");
            }

            if (HasLockedOperationModeResearch(Snapshot))
                AppendLine(Body, L"  " + BuildKnowledgeSummaryText(Snapshot));
        }

        if (Snapshot.DamageLevel != EBuildingDamageLevel::None)
        {
            AppendLine(Body, L"");
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.damage_status",
                GetDamageLevelDisplayName(Snapshot.DamageLevel));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.damage_efficiency",
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.DamageEfficiencyMultiplier * 100.f))) +
                    L"%");

            if (Snapshot.RepairCost > 0)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.repair_cost",
                    FormatMoney(Snapshot.RepairCost));
            }
        }

        if (Snapshot.Residential)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.residents_status") + L": " +
                std::to_wstring(Snapshot.Residents.size()) +
                L" / " +
                std::to_wstring(Snapshot.Capacity));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.household_capacity",
                Snapshot.HouseholdCapacityText);
        }
        else if (Snapshot.WorkProvider)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.workers") + L": " +
                std::to_wstring(Snapshot.AssignedEmployees.size()) +
                L" / " +
                std::to_wstring(Snapshot.Capacity));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.required_education",
                GetCitizenEducationDisplayName(
                    Snapshot.RequiredEducationLevel));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.job_quality",
                Snapshot.JobQualityText);
        }

        if (Snapshot.CatalogEntry)
        {
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.production",
                ResolveSupplyChainSummaryText(Snapshot));

            if (HasProductionInputRecords(Snapshot))
            {
                AppendLine(Body, L"");
                AppendProductionInputLines(Body, Snapshot);
            }

            if (Snapshot.CatalogEntry->HousingClass !=
                EBuildingHousingClass::None)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.housing_grade",
                    GetHousingClassDisplayName(
                        Snapshot.CatalogEntry->HousingClass));
            }
        }

        if (Snapshot.FoodProvider ||
            Snapshot.EntertainmentProvider ||
            Snapshot.HealthProvider ||
            Snapshot.FaithProvider)
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.service"));

            if (Snapshot.ServiceCapacity > 0)
            {
                AppendLine(
                    Body,
                    UIStrings::Get(GetServiceCapacityLabelKey(Snapshot)) +
                        L": " +
                        Snapshot.ServiceCapacityText);
            }

            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.service_quality",
                Snapshot.ServiceQualityText);
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.required_wealth",
                NormalizeWealthRequirementText(
                    Snapshot.WealthRequirementText));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.primary_tourist",
                Snapshot.TouristPreferenceText);
        }

        const std::wstring RequiredPowerText =
            ResolveRequiredPowerDisplayText(Snapshot);
        const std::wstring ProducedPowerText =
            ResolveProducedPowerDisplayText(Snapshot);

        if (!RequiredPowerText.empty() ||
            !ProducedPowerText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.power"));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.required_power",
                RequiredPowerText);
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.produced_power",
                ProducedPowerText);

            if (Snapshot.RequiredPowerMW > 0)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.power_network",
                    FormatPowerCoverageValue(
                        Snapshot.PowerSupplyRatio));
            }
        }

        if (Snapshot.PollutionOutput > 0 ||
            Snapshot.PollutionMitigation > 0 ||
            Snapshot.LocalPollutionExposure > 0)
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.environment"));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.pollution_output",
                std::to_wstring(Snapshot.PollutionOutput));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.pollution_mitigation",
                std::to_wstring(Snapshot.PollutionMitigation));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.local_pollution",
                std::to_wstring(Snapshot.LocalPollutionExposure));
        }

        if (Snapshot.UsesResourceStock ||
            Snapshot.Warehouse ||
            !Snapshot.LogisticsLines.empty() ||
            Snapshot.Harbor)
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.storage_logistics"));

            if (Snapshot.UsesResourceStock)
            {
                if (Snapshot.ProducedResourceType != EResourceType::None)
                {
                    AppendLine(
                        Body,
                        ResolveProducedResourceStockLabel(
                            Snapshot,
                            L"citizen_info.label.output_stock") +
                        L": " +
                        FormatInteger(Snapshot.ProducedResourceStock) +
                        L" / " +
                        FormatInteger(Snapshot.MaxResourceStock));
                }
                else
                {
                    AppendLine(
                        Body,
                        Ui(L"citizen_info.label.stock") + L": " +
                        FormatInteger(Snapshot.ResourceStock) +
                        L" / " +
                        FormatInteger(Snapshot.MaxResourceStock));
                }
                AppendProducedResourceTradeLines(Body, Snapshot);
            }

            if (Snapshot.Warehouse)
            {
                for (size_t SlotIndex = 0;
                    SlotIndex < Snapshot.WarehouseSlotLines.size();
                    ++SlotIndex)
                {
                    AppendLine(Body, Snapshot.WarehouseSlotLines[SlotIndex]);
                }
            }

            for (size_t LineIndex = 0;
                LineIndex < Snapshot.LogisticsLines.size();
                ++LineIndex)
            {
                AppendLine(Body, Snapshot.LogisticsLines[LineIndex]);
            }

            if (Snapshot.Harbor)
            {
                AppendLine(
                    Body,
                    Ui(L"citizen_info.label.exportable_stock") + L": " +
                    FormatInteger(Snapshot.ExportableStock));
                AppendLine(
                    Body,
                    Ui(L"citizen_info.label.ship_progress") + L": " +
                    std::to_wstring(static_cast<int>(roundf(
                        Snapshot.HarborShipProgressPercent * 100.f))) +
                    L"%");
                AppendHarborPolicyReference(Body, Snapshot);
                AppendHarborPriorityReference(Body, Snapshot);
                AppendHarborTradePriceReference(Body);
            }
        }

        if (!Snapshot.EffectText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.main_effect"));
            AppendLine(Body, Snapshot.EffectText);
        }

        if (!Snapshot.NoteText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.note"));
            AppendLine(Body, Snapshot.NoteText);
        }

        return Body.empty() ?
            UIStrings::Get(L"citizen_info.building.data_pending") :
            Body;
    }

    std::wstring BuildStatisticsBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body;

        if (Snapshot.DamageLevel != EBuildingDamageLevel::None)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.damage_status") + L": " +
                GetDamageLevelDisplayName(Snapshot.DamageLevel));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.damage_efficiency") + L": " +
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.DamageEfficiencyMultiplier * 100.f))) +
                L"%");

            if (Snapshot.RepairCost > 0)
            {
                AppendLine(
                    Body,
                    Ui(L"citizen_info.label.repair_cost") + L": " +
                    FormatMoney(Snapshot.RepairCost));
            }

            AppendLine(Body, L"");
        }

        AppendLine(
            Body,
            Ui(L"citizen_info.label.monthly_wage_cost") +
                L": " + FormatMoney(Snapshot.MonthlyWageCost));
        AppendLine(
            Body,
            Ui(L"citizen_info.label.monthly_upkeep_cost") +
                L": " + FormatMoney(Snapshot.MonthlyUpkeepCost));
        AppendLine(
            Body,
            Ui(L"citizen_info.label.monthly_total_cost") + L": " +
            FormatMoney(
                static_cast<long long>(Snapshot.MonthlyWageCost) +
                static_cast<long long>(Snapshot.MonthlyUpkeepCost)));
        AppendLine(
            Body,
            Ui(L"citizen_info.label.daily_wage_cost") +
                L": " + FormatMoney(Snapshot.DailyWageCost));
        AppendLine(
            Body,
            Ui(L"citizen_info.label.daily_upkeep_cost") +
                L": " + FormatMoney(Snapshot.DailyUpkeepCost));

        if (Snapshot.UsesResourceStock)
        {
            AppendLine(Body, L"");
            if (Snapshot.CatalogEntry)
            {
                const std::wstring ProductionStageText =
                    ResolveProductionChainStageText(Snapshot);

                if (!ProductionStageText.empty())
                {
                    AppendLine(
                        Body,
                        Ui(L"citizen_info.label.production_stage") +
                            L": " +
                            ProductionStageText);
                    AppendLine(
                        Body,
                        Ui(L"citizen_info.label.production") +
                            L": " +
                            ResolveSupplyChainSummaryText(Snapshot));
                }
            }

            if (Snapshot.ProducedResourceType != EResourceType::None)
            {
                AppendLine(
                    Body,
                    ResolveProducedResourceStockLabel(
                        Snapshot,
                        L"citizen_info.label.output_stock") +
                    L": " +
                    FormatInteger(Snapshot.ProducedResourceStock) +
                    L" / " +
                    FormatInteger(Snapshot.MaxResourceStock));
            }
            else
            {
                AppendLine(
                    Body,
                    Ui(L"citizen_info.label.current_stock") + L": " +
                    FormatInteger(Snapshot.ResourceStock) +
                    L" / " +
                    FormatInteger(Snapshot.MaxResourceStock));
            }
            if (HasProductionFlowEstimate(Snapshot))
            {
                AppendLine(Body, L"");
                AppendProductionFlowLines(Body, Snapshot);
            }
            if (HasProductionInputRecords(Snapshot))
            {
                AppendLine(Body, L"");
                AppendProductionInputLines(Body, Snapshot);
            }
            AppendProducedResourceTradeLines(Body, Snapshot);
        }

        if (Snapshot.Warehouse && !Snapshot.WarehouseSlotLines.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.warehouse_slots"));

            for (size_t SlotIndex = 0;
                SlotIndex < Snapshot.WarehouseSlotLines.size();
                ++SlotIndex)
            {
                AppendLine(Body, Snapshot.WarehouseSlotLines[SlotIndex]);
            }
        }

        if (!Snapshot.LogisticsLines.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.storage_logistics"));

            for (size_t LineIndex = 0;
                LineIndex < Snapshot.LogisticsLines.size();
                ++LineIndex)
            {
                AppendLine(Body, Snapshot.LogisticsLines[LineIndex]);
            }
        }

        if (Snapshot.Harbor)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.ship_arrival_progress") + L": " +
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.HarborShipProgressPercent * 100.f))) +
                L"%");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.exportable_total") + L": " +
                FormatInteger(Snapshot.ExportableStock));
            AppendLine(Body, L"");
            AppendHarborPolicyReference(Body, Snapshot);
            AppendHarborPriorityReference(Body, Snapshot);
            AppendHarborTradePriceReference(Body);
        }

        if (Snapshot.Residential)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.resident_assignment") + L": " +
                std::to_wstring(Snapshot.Residents.size()) +
                L" / " +
                std::to_wstring(Snapshot.Capacity));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.representative_resident") +
                    L": " + SummarizeNames(Snapshot.Residents));
        }

        if (Snapshot.WorkProvider)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.assigned_workers") + L": " +
                std::to_wstring(Snapshot.AssignedEmployees.size()));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.working_now") + L": " +
                std::to_wstring((std::max)(
                    0,
                    Snapshot.CurrentWorkerOccupancy)));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.representative_worker") + L": " +
                SummarizeNames(Snapshot.AssignedEmployees));
        }

        if (!Snapshot.AssignedVisitors.empty() ||
            !Snapshot.ArrivedVisitors.empty() ||
            !Snapshot.IncomingVisitors.empty())
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.assigned_visitors") + L": " +
                std::to_wstring(Snapshot.AssignedVisitors.size()));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.on_site_visitors") + L": " +
                std::to_wstring(Snapshot.ArrivedVisitors.size()));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.incoming_visitors") + L": " +
                std::to_wstring(Snapshot.IncomingVisitors.size()));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.representative_visitor") + L": " +
                SummarizeNames(Snapshot.ArrivedVisitors));
        }

        return Body;
    }

    std::wstring BuildUpgradesBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body;

        if (Snapshot.CatalogEntry &&
            !Snapshot.CatalogEntry->RuntimeUpgradeDefs.empty())
        {
            const FBuildingRuntimeUpgradeDef* const ActiveUpgradeDef =
                ResolveActiveRuntimeUpgradeDef(Snapshot);
            AppendLine(Body, Ui(L"citizen_info.section.active_upgrades"));

            if (ActiveUpgradeDef)
            {
                AppendLine(
                    Body,
                    BuildRuntimeUpgradeLine(
                        *ActiveUpgradeDef,
                        true,
                        Snapshot.ActiveRuntimeUpgradeEffectSummary.empty() ?
                            nullptr :
                            &Snapshot.ActiveRuntimeUpgradeEffectSummary));
            }
            else if (Snapshot.ActiveRuntimeUpgradeText.empty())
            {
                AppendLine(Body, L"-");
            }
            else
            {
                std::wstring ActiveLine =
                    L"* " + Snapshot.ActiveRuntimeUpgradeText;

                if (!Snapshot.ActiveRuntimeUpgradeEffectSummary.empty())
                {
                    ActiveLine += L" (";
                    ActiveLine += Snapshot.ActiveRuntimeUpgradeEffectSummary;
                    ActiveLine += L")";
                }

                AppendLine(Body, ActiveLine);
            }

            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.runtime_upgrades"));

            for (size_t Index = 0;
                Index < Snapshot.CatalogEntry->RuntimeUpgradeDefs.size();
                ++Index)
            {
                const bool IsActiveUpgrade =
                    static_cast<int>(Index) ==
                    Snapshot.ActiveRuntimeUpgradeIndex;
                const FBuildingRuntimeUpgradeDef& UpgradeDef =
                    Snapshot.CatalogEntry->RuntimeUpgradeDefs[Index];
                const std::wstring* const ActiveEffectSummary =
                    IsActiveUpgrade &&
                        !Snapshot.ActiveRuntimeUpgradeEffectSummary.empty() ?
                        &Snapshot.ActiveRuntimeUpgradeEffectSummary :
                        nullptr;
                AppendLine(
                    Body,
                    BuildRuntimeUpgradeLine(
                        UpgradeDef,
                        IsActiveUpgrade,
                        ActiveEffectSummary));
            }

            AppendLine(Body, L"");
        }

        if (!Snapshot.UpgradeHints.empty())
        {
            AppendLine(Body, Ui(L"citizen_info.section.registered_upgrades"));

            for (size_t Index = 0; Index < Snapshot.UpgradeHints.size(); ++Index)
                AppendLine(Body, L"- " + Snapshot.UpgradeHints[Index]);
        }
        else
        {
            AppendLine(Body, Ui(L"citizen_info.upgrades.none"));
        }

        if (!Snapshot.OperationModes.empty())
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.section.operation_mode_candidates"));

            for (size_t Index = 0; Index < Snapshot.OperationModes.size(); ++Index)
            {
                const bool IsActiveMode =
                    static_cast<int>(Index) == Snapshot.ActiveOperationModeIndex;
                std::wstring Line =
                    (IsActiveMode ? L"* " : L"- ") +
                    Snapshot.OperationModes[Index];

                Line += BuildOperationModeResearchSuffix(Snapshot, Index);

                if (IsActiveMode &&
                    !Snapshot.ActiveOperationModeEffectSummary.empty())
                {
                    Line += L" (";
                    Line += Snapshot.ActiveOperationModeEffectSummary;
                    Line += L")";
                }

                AppendLine(Body, Line);
            }

            if (HasLockedOperationModeResearch(Snapshot))
            {
                AppendLine(Body, L"");
                AppendLine(Body, BuildKnowledgeSummaryText(Snapshot));
            }

            if (Snapshot.CatalogEntry)
            {
                const std::wstring TransitionNotice =
                    GetOperationModeTransitionNotice(*Snapshot.CatalogEntry);

                if (!TransitionNotice.empty())
                {
                    AppendLine(Body, L"");
                    AppendLine(Body, L"참고: " + TransitionNotice);
                }
            }
        }

        if (Snapshot.Harbor)
        {
            AppendLine(Body, L"");
            AppendHarborPolicyReference(Body, Snapshot);
        }

        return Body;
    }

    std::wstring BuildEfficiencyBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body;
        const int CurrentEfficiencyPercent =
            static_cast<int>(roundf(
                ResolveDisplayedEfficiencyRatio(Snapshot) * 100.f));
        const int CapacityFillPercent =
            Snapshot.Capacity > 0 ?
            static_cast<int>(roundf(
                (static_cast<float>(
                    Snapshot.Residential ?
                    Snapshot.Residents.size() :
                    Snapshot.AssignedEmployees.size()) /
                    static_cast<float>(Snapshot.Capacity)) * 100.f)) :
            100;
        const int VisitorFillPercent =
            Snapshot.ServiceCapacity > 0 ?
            static_cast<int>(roundf(
                (static_cast<float>(Snapshot.AssignedVisitors.size()) /
                    static_cast<float>(Snapshot.ServiceCapacity)) * 100.f)) :
            0;

        AppendLine(
            Body,
            Ui(L"citizen_info.label.current_efficiency") + L": " +
            std::to_wstring((std::max)(0, CurrentEfficiencyPercent)) +
            L"%");

        if (Snapshot.DamageLevel != EBuildingDamageLevel::None)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.damage_status") + L": " +
                GetDamageLevelDisplayName(Snapshot.DamageLevel));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.damage_efficiency") + L": " +
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.DamageEfficiencyMultiplier * 100.f))) +
                L"%");
        }

        AppendLine(
            Body,
            Ui(L"citizen_info.label.budget_scale") +
                L": " + FormatMultiplier(Snapshot.BudgetScale));

        if (!Snapshot.IsRoad)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.road_accessibility") + L": " +
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.AccessibilityScore * 100.f))) +
                L"%");
        }

        if (Snapshot.Residential)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.housing_fill_rate") + L": " +
                std::to_wstring((std::max)(0, CapacityFillPercent)) +
                L"%");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.housing_satisfaction_cap") + L": " +
                std::to_wstring(Snapshot.HousingCap));
        }
        else if (Snapshot.WorkProvider)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.worker_fill_rate") + L": " +
                std::to_wstring((std::max)(0, CapacityFillPercent)) +
                L"%");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.job_satisfaction_cap") + L": " +
                std::to_wstring(Snapshot.JobCap));
        }

        if (Snapshot.FoodProvider)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.food_satisfaction_cap") + L": " +
                std::to_wstring(Snapshot.FoodCap));
        }

        if (Snapshot.EntertainmentProvider)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.fun_satisfaction_cap") + L": " +
                std::to_wstring(Snapshot.FunCap));
        }

        if (Snapshot.ServiceCapacity > 0)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.visitor_utilization") + L": " +
                std::to_wstring((std::max)(0, VisitorFillPercent)) +
                L"%");
        }

        const std::wstring RequiredPowerText =
            ResolveRequiredPowerDisplayText(Snapshot);

        if (!RequiredPowerText.empty())
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.power_demand",
                RequiredPowerText);

        if (HasProductionInputRecords(Snapshot))
        {
            AppendLine(Body, L"");
            AppendProductionInputLines(Body, Snapshot);
        }

        if (Snapshot.PollutionOutput > 0 ||
            Snapshot.PollutionMitigation > 0 ||
            Snapshot.LocalPollutionExposure > 0)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.pollution_output") + L": " +
                std::to_wstring(Snapshot.PollutionOutput));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.pollution_mitigation") + L": " +
                std::to_wstring(Snapshot.PollutionMitigation));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.local_pollution") + L": " +
                std::to_wstring(Snapshot.LocalPollutionExposure));
        }

        if (Snapshot.Harbor)
        {
            AppendLine(Body, L"");
            AppendHarborPolicyReference(Body, Snapshot);
        }

        return Body;
    }

    std::wstring BuildInformationBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body = ResolveRoleSummary(Snapshot);

        AppendLine(Body, L"");
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.category",
            Snapshot.CategoryName);
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.blueprint_cost",
            FormatCatalogCostValue(
                Snapshot.BlueprintCostState,
                Snapshot.BlueprintCost));
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.construction_cost",
            FormatCatalogCostValue(
                Snapshot.ConstructionCostState,
                Snapshot.ConstructionCost));
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.required_power",
            ResolveRequiredPowerDisplayText(Snapshot));
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.produced_power",
            ResolveProducedPowerDisplayText(Snapshot));
        AppendKeyValueByKey(
            Body,
            Snapshot.Residential ?
                L"citizen_info.label.household_capacity" :
                GetServiceCapacityLabelKey(Snapshot),
            Snapshot.Residential ?
                Snapshot.HouseholdCapacityText :
                Snapshot.ServiceCapacityText);
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.housing_quality",
            Snapshot.HousingQualityText);
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.job_quality",
            Snapshot.JobQualityText);
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.service_quality",
            Snapshot.ServiceQualityText);
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.required_wealth",
            NormalizeWealthRequirementText(
                Snapshot.WealthRequirementText));

        if (Snapshot.CatalogEntry)
        {
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.unlock_era",
                GetBuildingEraDisplayName(Snapshot.CatalogEntry->UnlockEra));

            const std::wstring ProductionStageText =
                ResolveProductionChainStageText(Snapshot);

            if (!ProductionStageText.empty())
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.production_stage",
                    ProductionStageText);
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.production",
                    ResolveSupplyChainSummaryText(Snapshot));
            }

            if (HasProductionInputRecords(Snapshot))
            {
                AppendLine(Body, L"");
                AppendProductionInputLines(Body, Snapshot);
            }

            if (Snapshot.CatalogEntry->HousingClass !=
                EBuildingHousingClass::None)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.housing_grade",
                    GetHousingClassDisplayName(
                        Snapshot.CatalogEntry->HousingClass));
            }

            if (Snapshot.CatalogEntry->LeisureClass !=
                EBuildingLeisureClass::None)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.leisure_grade",
                    GetLeisureClassDisplayName(
                        Snapshot.CatalogEntry->LeisureClass));
            }

            if (Snapshot.CatalogEntry->PrimaryTouristPreference !=
                ETouristPreference::None)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.primary_tourist",
                    GetTouristPreferenceDisplayName(
                        Snapshot.CatalogEntry->PrimaryTouristPreference));
            }
        }

        if (Snapshot.PollutionOutput > 0 ||
            Snapshot.PollutionMitigation > 0 ||
            Snapshot.LocalPollutionExposure > 0)
        {
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.pollution_output",
                std::to_wstring(Snapshot.PollutionOutput));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.pollution_mitigation",
                std::to_wstring(Snapshot.PollutionMitigation));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.local_pollution",
                std::to_wstring(Snapshot.LocalPollutionExposure));
        }

        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.required_education",
            GetCitizenEducationDisplayName(
                Snapshot.RequiredEducationLevel));

        if (!Snapshot.NarrativeLines.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, JoinLines(Snapshot.NarrativeLines));
        }
        else if (!Snapshot.DetailText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Snapshot.DetailText);
        }

        return Body;
    }

    std::wstring BuildCustomsModeSelectionBody(
        const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body =
            L"해당 건물의 운영 모드를 선택하십시오.";

        if (!Snapshot.ActiveOperationModeText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, L"현재 선택: " + Snapshot.ActiveOperationModeText);
        }

        const std::wstring Description =
            CitizenInfoBuildingRuntime::ResolveCustomsModeDescription(
                Snapshot,
                Snapshot.ActiveOperationModeIndex);

        if (!Description.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, L"효과: " + Description);
        }

        return Body;
    }

    std::wstring BuildCustomsUpgradesBody(
        const FBuildingUiSnapshot& Snapshot)
    {
        return BuildUpgradesBody(Snapshot);
    }

}
