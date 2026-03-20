#include "CitizenInfoBuildingRuntime.h"
#include "CitizenInfoConstants.h"
#include "UIStringShorthand.h"
#include "UIStrings.h"
#include "../Building/BuildingCatalog.h"
#include "../Economy/ResourceTradePricing.h"
#include "../StringUtils.h"
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <vector>

namespace
{
    using namespace CitizenInfoBuildingRuntime;
    using UIStringShorthand::Ui;
    using UIStringShorthand::UiText;
    using StringUtils::ExtractDetailValue;
    using StringUtils::SplitLines;
    using StringUtils::StartsWith;
    using StringUtils::Trim;

    std::wstring BuildAllowedWealthRequirementText(unsigned int AllowedWealthMask)
    {
        const unsigned int EffectiveMask =
            AllowedWealthMask == GBuildingWealthMaskNone ?
                GBuildingWealthMaskAll :
                AllowedWealthMask;
        switch (EffectiveMask)
        {
        case GBuildingWealthMaskBroke:
            return UIStrings::Get(L"citizen.wealth.broke");
        case GBuildingWealthMaskPoor:
            return UIStrings::Get(L"citizen.wealth.poor");
        case GBuildingWealthMaskWellOff:
            return UIStrings::Get(L"citizen.wealth.well_off");
        case GBuildingWealthMaskRich:
            return UIStrings::Get(L"citizen.wealth.rich");
        case GBuildingWealthMaskFilthyRich:
            return UIStrings::Get(L"citizen.wealth.filthy_rich");
        case GBuildingWealthMaskWellOff |
            GBuildingWealthMaskRich |
            GBuildingWealthMaskFilthyRich:
            return UIStrings::Format(
                L"citizen_info.wealth.at_least_template",
                { Ui(L"citizen.wealth.well_off") });
        case GBuildingWealthMaskRich | GBuildingWealthMaskFilthyRich:
            return UIStrings::Format(
                L"citizen_info.wealth.at_least_template",
                { Ui(L"citizen.wealth.rich") });
        case GBuildingWealthMaskPoor |
            GBuildingWealthMaskWellOff |
            GBuildingWealthMaskRich |
            GBuildingWealthMaskFilthyRich:
            return UIStrings::Format(
                L"citizen_info.wealth.at_least_template",
                { Ui(L"citizen.wealth.poor") });
        default:
            return std::wstring();
        }
    }

    std::wstring BuildTouristPreferenceText(
        const FBuildingCatalogEntry* Entry)
    {
        if (!Entry || !HasTouristPreference(Entry->PrimaryTouristPreference))
            return std::wstring();

        const wchar_t* DisplayName =
            GetTouristPreferenceDisplayName(Entry->PrimaryTouristPreference);
        return DisplayName ? std::wstring(DisplayName) : std::wstring();
    }

    bool HasTradeUnitPrice(EResourceType Type)
    {
        return IsExportableResourceType(Type);
    }

    std::wstring FormatTradeUnitPriceInline(EResourceType Type)
    {
        if (!HasTradeUnitPrice(Type))
            return std::wstring();

        return UIStrings::Get(L"citizen_info.label.export_short") +
            L" " +
            StringUtils::FormatCurrencyDollarFirst(
                ResourceTradePricing::GetExportPricePerStockUnit(Type)) +
            L" / " +
            UIStrings::Get(L"citizen_info.label.import_short") +
            L" " +
            StringUtils::FormatCurrencyDollarFirst(
                ResourceTradePricing::GetImportPricePerStockUnit(Type));
    }

    std::vector<std::wstring> ExtractBulletSection(
        const std::wstring& DetailText,
        const wchar_t* HeaderPrefix)
    {
        std::vector<std::wstring> Result;
        const std::vector<std::wstring> Lines = SplitLines(DetailText);
        bool Capture = false;

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::wstring Line = Trim(Lines[Index]);

            if (!Capture)
            {
                if (StartsWith(Line, HeaderPrefix))
                    Capture = true;

                continue;
            }

            if (Line.empty())
                break;

            if (!Line.empty() && Line[0] == L'-')
            {
                Result.push_back(Trim(Line.substr(1)));
                continue;
            }

            if (Line.find(L':') != std::wstring::npos)
                break;

            break;
        }

        return Result;
    }

    std::vector<std::wstring> ExtractNarrativeLines(
        const std::wstring& DetailText)
    {
        const wchar_t* SkipPrefixes[] =
        {
            UiText(L"citizen_info.detail.prefix.construction_cost"),
            UiText(L"citizen_info.detail.prefix.blueprint_cost"),
            UiText(L"citizen_info.detail.prefix.unlock_era"),
            UiText(L"citizen_info.detail.prefix.required_workers"),
            UiText(L"citizen_info.detail.prefix.required_power"),
            UiText(L"citizen_info.detail.prefix.produced_power"),
            UiText(L"citizen_info.detail.prefix.size"),
            UiText(L"citizen_info.detail.prefix.job_quality"),
            UiText(L"citizen_info.detail.prefix.service_quality"),
            UiText(L"citizen_info.detail.prefix.service_capacity"),
            UiText(L"citizen_info.detail.prefix.household_capacity"),
            UiText(L"citizen_info.detail.prefix.wealth_requirement"),
            UiText(L"citizen_info.detail.prefix.required_wealth"),
            UiText(L"citizen_info.detail.prefix.tourist_wealth"),
            UiText(L"citizen_info.detail.prefix.tourist_preference"),
            UiText(L"citizen_info.detail.prefix.operation_mode"),
            UiText(L"citizen_info.detail.prefix.upgrade")
        };
        const std::wstring& EffectPrefix =
            Ui(L"citizen_info.detail.prefix.effect");
        const std::wstring& NotePrefix =
            Ui(L"citizen_info.detail.prefix.note");
        std::vector<std::wstring> Result;
        const std::vector<std::wstring> Lines = SplitLines(DetailText);

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            std::wstring Line = Trim(Lines[Index]);

            if (Line.empty())
                continue;

            bool SkipLine = Line[0] == L'-';

            for (const wchar_t* Prefix : SkipPrefixes)
            {
                if (StartsWith(Line, Prefix))
                {
                    SkipLine = true;
                    break;
                }
            }

            if (SkipLine)
                continue;

            if (StartsWith(Line, EffectPrefix.c_str()))
            {
                Result.push_back(
                    UIStrings::Get(L"citizen_info.section.main_effect") +
                    L": " +
                    Trim(Line.substr(EffectPrefix.size())));
                continue;
            }

            if (StartsWith(Line, NotePrefix.c_str()))
            {
                Result.push_back(
                    UIStrings::Get(L"citizen_info.section.note") +
                    L": " +
                    Trim(Line.substr(NotePrefix.size())));
                continue;
            }

            if (Line.find(L':') != std::wstring::npos)
                continue;

            Result.push_back(Line);
        }

        return Result;
    }
}

namespace CitizenInfoBuildingRuntime
{
    bool IsHydroponicFarmBuilding(const FBuildingUiSnapshot& Snapshot)
    {
        return Snapshot.BuildingId ==
                CitizenInfoConstants::GHydroponicFarmBuildingId ||
            (Snapshot.CatalogEntry &&
                Snapshot.CatalogEntry->Id ==
                    CitizenInfoConstants::GHydroponicFarmBuildingId);
    }

    bool IsCustomsOfficeBuilding(const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.CatalogEntry)
            return IsCustomsOfficeCatalogEntry(*Snapshot.CatalogEntry);

        return IsCustomsOfficeBuildingId(Snapshot.BuildingId);
    }

    int ComputeAverageCustomsDiplomacyExportBiasPercent()
    {
        int TotalBias = 0;
        int SampleCount = 0;

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsExportableResourceType(ResourceType))
                continue;

            TotalBias +=
                ResourceTradePricing::GetDiplomacyExportBiasPercent(
                    ResourceType);
            ++SampleCount;
        }

        if (SampleCount <= 0)
            return 7;

        return static_cast<int>(std::lround(
            static_cast<double>(TotalBias) /
            static_cast<double>(SampleCount)));
    }

    int ResolveCustomsBudgetModifierPercent(const FBuildingUiSnapshot& Snapshot)
    {
        return static_cast<int>(std::lround(
            (Snapshot.BudgetScale - 1.f) * 100.f));
    }

    int ResolveCustomsEfficiencyPercent(const FBuildingUiSnapshot& Snapshot)
    {
        const int BudgetModifier =
            ResolveCustomsBudgetModifierPercent(Snapshot);
        const int DiplomacyModifier =
            ComputeAverageCustomsDiplomacyExportBiasPercent();
        return (std::max)(0, 100 + BudgetModifier + DiplomacyModifier);
    }

    std::wstring ResolveCustomsPerWorkerWage(
        const FBuildingUiSnapshot& Snapshot)
    {
        const int WorkerCount = (std::max)(
            1,
            (std::max)(
                static_cast<int>(Snapshot.AssignedEmployees.size()),
                Snapshot.Capacity));
        const long long WagePerWorker =
            Snapshot.MonthlyWageCost > 0 ?
                static_cast<long long>(std::llround(
                    static_cast<double>(Snapshot.MonthlyWageCost) /
                    static_cast<double>(WorkerCount))) :
                0ll;
        return StringUtils::FormatCurrency(WagePerWorker);
    }

    std::wstring ResolveCustomsModeDescription(
        const FBuildingUiSnapshot& Snapshot,
        int ModeIndex)
    {
        if (!Snapshot.CatalogEntry ||
            ModeIndex < 0 ||
            ModeIndex >=
                static_cast<int>(
                    Snapshot.CatalogEntry->OperationModeDefs.size()))
        {
            return std::wstring();
        }

        return GetOperationModeEffectSummary(
            *Snapshot.CatalogEntry,
            ModeIndex);
    }

    bool BuildBuildingUiSnapshot(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>&
            QuerySource,
        const std::string& BuildingName,
        FBuildingUiSnapshot& OutSnapshot)
    {
        if (!QuerySource || BuildingName.empty())
            return false;

        CitizenInfoDataProvider::FCitizenInfoBuildingRecord BuildingRecord;

        if (!QuerySource->TryGetBuildingRecord(BuildingName, BuildingRecord) ||
            !BuildingRecord.Valid)
        {
            return false;
        }

        // Fail closed if the query source resolves a different building than
        // the one currently tracked by the widget.
        if (!BuildingRecord.ObjectName.empty() &&
            StringUtils::WideToUtf8(BuildingRecord.ObjectName) != BuildingName)
        {
            return false;
        }

        using FBuildingRecord =
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord;

        OutSnapshot = FBuildingUiSnapshot();
        static_cast<FBuildingRecord&>(OutSnapshot) = std::move(BuildingRecord);
        OutSnapshot.CatalogEntry =
            FindBuildingCatalogEntry(OutSnapshot.BuildingId);
        OutSnapshot.DetailText = OutSnapshot.CatalogEntry ?
            OutSnapshot.CatalogEntry->DetailText :
            std::wstring();
        OutSnapshot.BlueprintCostState = OutSnapshot.CatalogEntry ?
            OutSnapshot.CatalogEntry->BlueprintCostState :
            EBuildingCostState::None;
        OutSnapshot.BlueprintCost = OutSnapshot.CatalogEntry ?
            OutSnapshot.CatalogEntry->BlueprintCost :
            0;
        OutSnapshot.ConstructionCostState = OutSnapshot.CatalogEntry ?
            OutSnapshot.CatalogEntry->ConstructionCostState :
            EBuildingCostState::None;
        OutSnapshot.ConstructionCost = OutSnapshot.CatalogEntry ?
            OutSnapshot.CatalogEntry->ConstructionCost :
            0;
        OutSnapshot.HouseholdCapacity = OutSnapshot.CatalogEntry ?
            (std::max)(0, OutSnapshot.CatalogEntry->HouseholdCapacity) :
            0;
        OutSnapshot.ServiceCapacityUsesHouseholds =
            OutSnapshot.CatalogEntry &&
            OutSnapshot.CatalogEntry->ServiceCapacityUsesHouseholds;
        OutSnapshot.ProducedResourceType =
            OutSnapshot.ProducedResourceType != EResourceType::None ?
                OutSnapshot.ProducedResourceType :
                (OutSnapshot.CatalogEntry ?
                    OutSnapshot.CatalogEntry->ProducedResourceType :
                    EResourceType::None);
        OutSnapshot.ProductionChainStageText =
            !OutSnapshot.ProductionChainStageText.empty() ?
                OutSnapshot.ProductionChainStageText :
                (OutSnapshot.ChainStage !=
                        CitizenInfoDataProvider::EProductionChainStage::None ?
                    std::wstring(
                        GetProductionChainStageDisplayName(
                            OutSnapshot.ChainStage)) :
                    std::wstring());

        if (OutSnapshot.Warehouse)
        {
            for (size_t SlotIndex = 0;
                SlotIndex < OutSnapshot.WarehouseSlots.size();
                ++SlotIndex)
            {
                const CitizenInfoDataProvider::FWarehouseSlotRecord& SlotRecord =
                    OutSnapshot.WarehouseSlots[SlotIndex];
                std::wstring SlotValue;

                if (SlotRecord.Type == EResourceType::None)
                {
                    SlotValue =
                        UIStrings::Get(L"citizen_info.building.warehouse.empty") +
                        L" / " +
                        StringUtils::FormatIntegerWithCommas(
                            SlotRecord.Capacity);
                }
                else
                {
                    SlotValue =
                        std::wstring(GetResourceTypeDisplayName(
                            SlotRecord.Type)) +
                        L" " +
                        StringUtils::FormatIntegerWithCommas(
                            SlotRecord.Stock) +
                        L" / " +
                        StringUtils::FormatIntegerWithCommas(
                            SlotRecord.Capacity);

                    const std::wstring TradePriceText =
                        FormatTradeUnitPriceInline(SlotRecord.Type);

                    if (!TradePriceText.empty())
                    {
                        SlotValue += L" (";
                        SlotValue += TradePriceText;
                        SlotValue += L")";
                    }
                }

                std::wstring SlotLine = UIStrings::Format(
                    L"citizen_info.building.warehouse.slot_line",
                    {
                        std::to_wstring(static_cast<int>(SlotIndex) + 1),
                        SlotValue
                    });
                OutSnapshot.WarehouseSlotLines.push_back(std::move(SlotLine));
            }
        }

        if (OutSnapshot.Harbor)
        {
            long long TotalExportValue = 0;

            for (size_t SlotIndex = 0;
                SlotIndex < OutSnapshot.HarborResourceSlots.size();
                ++SlotIndex)
            {
                const CitizenInfoDataProvider::FWarehouseSlotRecord& SlotRecord =
                    OutSnapshot.HarborResourceSlots[SlotIndex];

                if (SlotRecord.Type == EResourceType::None || SlotRecord.Stock <= 0)
                    continue;

                std::wstring Line =
                    std::wstring(GetResourceTypeDisplayName(SlotRecord.Type)) +
                    L": " +
                    StringUtils::FormatIntegerWithCommas(SlotRecord.Stock);

                if (HasTradeUnitPrice(SlotRecord.Type))
                {
                    const int UnitPrice =
                        ResourceTradePricing::GetExportPricePerStockUnit(
                            SlotRecord.Type);
                    Line += L" (";
                    Line += UIStrings::Get(L"citizen_info.label.export_short");
                    Line += L" ";
                    Line += StringUtils::FormatCurrencyDollarFirst(UnitPrice);
                    Line += L")";
                    TotalExportValue +=
                        static_cast<long long>(SlotRecord.Stock) * UnitPrice;
                }

                OutSnapshot.HarborResourceLines.push_back(std::move(Line));
            }

            if (TotalExportValue > 0)
            {
                OutSnapshot.HarborResourceLines.push_back(
                    L"합계: " +
                    StringUtils::FormatCurrencyDollarFirst(
                        static_cast<int>(TotalExportValue)));
            }
        }

        OutSnapshot.HouseholdCapacityText =
            OutSnapshot.HouseholdCapacity > 0 ?
                std::to_wstring(OutSnapshot.HouseholdCapacity) :
                std::wstring();
        OutSnapshot.WealthRequirementText =
            OutSnapshot.CatalogEntry ?
                BuildAllowedWealthRequirementText(
                    OutSnapshot.CatalogEntry->AllowedWealthMask) :
                std::wstring();
        OutSnapshot.TouristPreferenceText =
            BuildTouristPreferenceText(OutSnapshot.CatalogEntry);
        OutSnapshot.EffectText =
            ExtractDetailValue(
                OutSnapshot.DetailText,
                UiText(L"citizen_info.detail.prefix.effect"));
        OutSnapshot.NoteText =
            ExtractDetailValue(
                OutSnapshot.DetailText,
                UiText(L"citizen_info.detail.prefix.note"));

        const int CatalogServiceCapacity = OutSnapshot.CatalogEntry ?
            (std::max)(0, OutSnapshot.CatalogEntry->ServiceCapacity) :
            0;
        OutSnapshot.ServiceCapacity = (std::max)(
            OutSnapshot.ServiceCapacity,
            CatalogServiceCapacity);

        OutSnapshot.ServiceCapacityText =
            OutSnapshot.ServiceCapacity > 0 ?
                std::to_wstring(OutSnapshot.ServiceCapacity) :
                std::wstring();

        OutSnapshot.OperationModes.clear();

        if (OutSnapshot.CatalogEntry)
        {
            for (size_t ModeIndex = 0;
                ModeIndex < OutSnapshot.CatalogEntry->OperationModeDefs.size();
                ++ModeIndex)
            {
                OutSnapshot.OperationModes.push_back(
                    OutSnapshot.CatalogEntry->OperationModeDefs[ModeIndex].
                        DisplayName);
            }
        }

        if (OutSnapshot.ActiveOperationModeText.empty() &&
            !OutSnapshot.OperationModes.empty())
        {
            const int SafeModeIndex = (std::max)(
                0,
                (std::min)(
                    static_cast<int>(OutSnapshot.OperationModes.size()) - 1,
                    OutSnapshot.ActiveOperationModeIndex));
            OutSnapshot.ActiveOperationModeText =
                OutSnapshot.OperationModes[static_cast<size_t>(SafeModeIndex)];
        }

        if (OutSnapshot.Residential)
            OutSnapshot.HousingQualityText =
                std::to_wstring(OutSnapshot.HousingCap);
        else if (OutSnapshot.CatalogEntry &&
            OutSnapshot.CatalogEntry->BaseHousingQuality > 0)
        {
            OutSnapshot.HousingQualityText =
                std::to_wstring(OutSnapshot.CatalogEntry->BaseHousingQuality);
        }

        if (OutSnapshot.WorkProvider)
            OutSnapshot.JobQualityText =
                std::to_wstring(OutSnapshot.JobCap);
        else if (OutSnapshot.CatalogEntry &&
            OutSnapshot.CatalogEntry->BaseJobQuality > 0)
        {
            OutSnapshot.JobQualityText =
                std::to_wstring(OutSnapshot.CatalogEntry->BaseJobQuality);
        }

        if (OutSnapshot.FoodProvider ||
            OutSnapshot.EntertainmentProvider ||
            OutSnapshot.HealthProvider ||
            OutSnapshot.FaithProvider)
        {
            int EffectiveServiceCap = 0;

            if (OutSnapshot.FoodProvider)
                EffectiveServiceCap = (std::max)(
                    EffectiveServiceCap,
                    OutSnapshot.FoodCap);

            if (OutSnapshot.EntertainmentProvider)
                EffectiveServiceCap = (std::max)(
                    EffectiveServiceCap,
                    OutSnapshot.FunCap);

            if (OutSnapshot.HealthProvider)
                EffectiveServiceCap = (std::max)(
                    EffectiveServiceCap,
                    OutSnapshot.HealthCap);

            if (OutSnapshot.FaithProvider)
                EffectiveServiceCap = (std::max)(
                    EffectiveServiceCap,
                    OutSnapshot.FaithCap);

            OutSnapshot.ServiceQualityText =
                std::to_wstring(EffectiveServiceCap);
        }
        else if (OutSnapshot.CatalogEntry &&
            OutSnapshot.CatalogEntry->BaseServiceQuality > 0)
        {
            OutSnapshot.ServiceQualityText =
                std::to_wstring(OutSnapshot.CatalogEntry->BaseServiceQuality);
        }

        OutSnapshot.UpgradeHints = OutSnapshot.CatalogEntry ?
            OutSnapshot.CatalogEntry->UpgradeHints :
            ExtractBulletSection(
                OutSnapshot.DetailText,
                UiText(L"citizen_info.detail.prefix.upgrade"));
        OutSnapshot.NarrativeLines =
            ExtractNarrativeLines(OutSnapshot.DetailText);
        return true;
    }
}
