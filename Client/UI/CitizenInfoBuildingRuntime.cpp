#include "CitizenInfoBuildingRuntime.h"
#include "UIStrings.h"
#include "../Economy/ResourceTradePricing.h"
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <vector>

namespace
{
    using namespace CitizenInfoBuildingRuntime;

    std::wstring Trim(const std::wstring& Text)
    {
        size_t Start = 0;

        while (Start < Text.size() && iswspace(Text[Start]))
            ++Start;

        size_t End = Text.size();

        while (End > Start && iswspace(Text[End - 1]))
            --End;

        return Text.substr(Start, End - Start);
    }

    bool StartsWith(const std::wstring& Text, const wchar_t* Prefix)
    {
        if (!Prefix)
            return false;

        const size_t PrefixLength = wcslen(Prefix);

        if (Text.size() < PrefixLength)
            return false;

        return Text.compare(0, PrefixLength, Prefix) == 0;
    }

    std::vector<std::wstring> SplitLines(const std::wstring& Text)
    {
        std::vector<std::wstring> Lines;
        std::wstring Current;

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const wchar_t Ch = Text[Index];

            if (Ch == L'\r')
                continue;

            if (Ch == L'\n')
            {
                Lines.push_back(Current);
                Current.clear();
                continue;
            }

            Current.push_back(Ch);
        }

        if (!Current.empty() || Text.empty())
            Lines.push_back(Current);

        return Lines;
    }

    std::wstring FormatInteger(long long Value)
    {
        const bool Negative = Value < 0;
        unsigned long long UnsignedValue = Negative ?
            static_cast<unsigned long long>(-Value) :
            static_cast<unsigned long long>(Value);
        std::wstring Digits = std::to_wstring(UnsignedValue);
        std::wstring Result;
        int GroupCount = 0;

        for (int Index = static_cast<int>(Digits.size()) - 1; Index >= 0; --Index)
        {
            if (GroupCount == 3)
            {
                Result.insert(Result.begin(), L',');
                GroupCount = 0;
            }

            Result.insert(Result.begin(), Digits[static_cast<size_t>(Index)]);
            ++GroupCount;
        }

        if (Negative)
            Result.insert(Result.begin(), L'-');

        return Result;
    }

    std::wstring FormatMoney(long long Value)
    {
        if (Value < 0)
            return L"-$" + FormatInteger(-Value);

        return L"$" + FormatInteger(Value);
    }

    std::wstring FormatMoneyDollarFirst(long long Value)
    {
        if (Value < 0)
            return L"$-" + FormatInteger(-Value);

        return L"$" + FormatInteger(Value);
    }

    std::wstring FormatMegawattValue(int Value)
    {
        return std::to_wstring(Value) +
            UIStrings::Get(L"citizen_info.unit.megawatt");
    }

    std::wstring ExtractDetailValue(
        const std::wstring& DetailText,
        const wchar_t* Prefix)
    {
        const std::vector<std::wstring> Lines = SplitLines(DetailText);

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::wstring Line = Trim(Lines[Index]);

            if (!StartsWith(Line, Prefix))
                continue;

            return Trim(Line.substr(wcslen(Prefix)));
        }

        return std::wstring();
    }
    std::wstring BuildAllowedWealthRequirementText(unsigned int AllowedWealthMask)
    {
        const unsigned int EffectiveMask =
            AllowedWealthMask == GBuildingWealthMaskNone ?
                GBuildingWealthMaskAll :
                AllowedWealthMask;
        switch (EffectiveMask)
        {
        case GBuildingWealthMaskPoor:
            return UIStrings::Get(L"citizen.wealth.poor");
        case GBuildingWealthMaskWellOff:
            return UIStrings::Get(L"citizen.wealth.well_off");
        case GBuildingWealthMaskRich:
            return UIStrings::Get(L"citizen.wealth.rich");
        case GBuildingWealthMaskWellOff | GBuildingWealthMaskRich:
            return UIStrings::Get(L"citizen.wealth.well_off") + L" 이상";
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
            FormatMoneyDollarFirst(
                ResourceTradePricing::GetExportPricePerStockUnit(Type)) +
            L" / " +
            UIStrings::Get(L"citizen_info.label.import_short") +
            L" " +
            FormatMoneyDollarFirst(
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
        std::vector<std::wstring> Result;
        const std::vector<std::wstring> Lines = SplitLines(DetailText);

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            std::wstring Line = Trim(Lines[Index]);

            if (Line.empty())
                continue;

            if (Line[0] == L'-' ||
                StartsWith(Line, L"건설 비용:") ||
                StartsWith(Line, L"설계도 비용:") ||
                StartsWith(Line, L"등장 시기:") ||
                StartsWith(Line, L"필요 인력:") ||
                StartsWith(Line, L"필요 전력:") ||
                StartsWith(Line, L"생산 전력:") ||
                StartsWith(Line, L"크기:") ||
                StartsWith(Line, L"직업 품질:") ||
                StartsWith(Line, L"서비스 품질:") ||
                StartsWith(Line, L"수용 인원:") ||
                StartsWith(Line, L"수용 가구:") ||
                StartsWith(Line, L"재산 요구치:") ||
                StartsWith(Line, L"필요 재산:") ||
                StartsWith(Line, L"관광객 재산:") ||
                StartsWith(Line, L"선호 관광객:") ||
                StartsWith(Line, L"운영 모드:") ||
                StartsWith(Line, L"업그레이드"))
            {
                continue;
            }

            if (StartsWith(Line, L"효과:"))
            {
                Result.push_back(
                    UIStrings::Get(L"citizen_info.section.main_effect") +
                    L": " +
                    Trim(Line.substr(wcslen(L"효과:"))));
                continue;
            }

            if (StartsWith(Line, L"비고:"))
            {
                Result.push_back(
                    UIStrings::Get(L"citizen_info.section.note") +
                    L": " +
                    Trim(Line.substr(wcslen(L"비고:"))));
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
        return Snapshot.BuildingId == "build_2_10" ||
            Snapshot.DisplayName == L"대규모 수경 농장";
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
            static_cast<int>(Snapshot.AssignedEmployees.size()));
        const long long WagePerWorker =
            Snapshot.MonthlyWageCost > 0 ?
                static_cast<long long>(std::llround(
                    static_cast<double>(Snapshot.MonthlyWageCost) /
                    static_cast<double>(WorkerCount))) :
                21ll;
        return FormatMoney(WagePerWorker);
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

        OutSnapshot = FBuildingUiSnapshot();
        OutSnapshot.CatalogEntry =
            FindBuildingCatalogEntry(BuildingRecord.BuildingId);
        OutSnapshot.BuildingId = BuildingRecord.BuildingId;
        OutSnapshot.ObjectName = BuildingRecord.ObjectName;
        OutSnapshot.DisplayName = BuildingRecord.DisplayName;
        OutSnapshot.CategoryName = BuildingRecord.CategoryName;
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
        OutSnapshot.RequiredEducationLevel =
            BuildingRecord.RequiredEducationLevel;
        OutSnapshot.Residential = BuildingRecord.Residential;
        OutSnapshot.WorkProvider = BuildingRecord.WorkProvider;
        OutSnapshot.FoodProvider = BuildingRecord.FoodProvider;
        OutSnapshot.EntertainmentProvider =
            BuildingRecord.EntertainmentProvider;
        OutSnapshot.HealthProvider = BuildingRecord.HealthProvider;
        OutSnapshot.FaithProvider = BuildingRecord.FaithProvider;
        OutSnapshot.UsesResourceStock = BuildingRecord.UsesResourceStock;
        OutSnapshot.Harbor = BuildingRecord.Harbor;
        OutSnapshot.Warehouse = BuildingRecord.Warehouse;
        OutSnapshot.IsRoad = BuildingRecord.IsRoad;
        OutSnapshot.CanGenerateWorkOutput =
            BuildingRecord.CanGenerateWorkOutput;
        OutSnapshot.Capacity = BuildingRecord.Capacity;
        OutSnapshot.HouseholdCapacity = OutSnapshot.CatalogEntry ?
            (std::max)(0, OutSnapshot.CatalogEntry->HouseholdCapacity) :
            0;
        OutSnapshot.ServiceCapacityUsesHouseholds =
            OutSnapshot.CatalogEntry &&
            OutSnapshot.CatalogEntry->ServiceCapacityUsesHouseholds;
        OutSnapshot.BudgetLevel = BuildingRecord.BudgetLevel;
        OutSnapshot.DaysInMonth = BuildingRecord.DaysInMonth;
        OutSnapshot.MonthlyWageCost = BuildingRecord.MonthlyWageCost;
        OutSnapshot.MonthlyUpkeepCost = BuildingRecord.MonthlyUpkeepCost;
        OutSnapshot.DailyWageCost = BuildingRecord.DailyWageCost;
        OutSnapshot.DailyUpkeepCost = BuildingRecord.DailyUpkeepCost;
        OutSnapshot.HousingCap = BuildingRecord.HousingCap;
        OutSnapshot.JobCap = BuildingRecord.JobCap;
        OutSnapshot.FoodCap = BuildingRecord.FoodCap;
        OutSnapshot.FunCap = BuildingRecord.FunCap;
        OutSnapshot.HealthCap = BuildingRecord.HealthCap;
        OutSnapshot.FaithCap = BuildingRecord.FaithCap;
        OutSnapshot.PollutionOutput = BuildingRecord.PollutionOutput;
        OutSnapshot.PollutionMitigation = BuildingRecord.PollutionMitigation;
        OutSnapshot.LocalPollutionExposure =
            BuildingRecord.LocalPollutionExposure;
        OutSnapshot.ResourceStock = BuildingRecord.ResourceStock;
        OutSnapshot.ExportableStock = BuildingRecord.ExportableStock;
        OutSnapshot.MaxResourceStock = BuildingRecord.MaxResourceStock;
        OutSnapshot.ProducedResourceType =
            BuildingRecord.ProducedResourceType != EResourceType::None ?
                BuildingRecord.ProducedResourceType :
                (OutSnapshot.CatalogEntry ?
                    OutSnapshot.CatalogEntry->ProducedResourceType :
                    EResourceType::None);
        OutSnapshot.ProducedResourceStock =
            BuildingRecord.ProducedResourceStock;
        OutSnapshot.ProducedPowerMW = BuildingRecord.ProducedPowerMW;
        OutSnapshot.RequiredPowerMW = BuildingRecord.RequiredPowerMW;
        OutSnapshot.TotalProducedPowerMW =
            BuildingRecord.TotalProducedPowerMW;
        OutSnapshot.TotalRequiredPowerMW =
            BuildingRecord.TotalRequiredPowerMW;
        OutSnapshot.LastDailyExportIncome =
            BuildingRecord.LastDailyExportIncome;
        OutSnapshot.LastDailyImportExpense =
            BuildingRecord.LastDailyImportExpense;
        OutSnapshot.TradeRouteExportFulfilledUnits =
            BuildingRecord.TradeRouteExportFulfilledUnits;
        OutSnapshot.TradeRouteImportFulfilledUnits =
            BuildingRecord.TradeRouteImportFulfilledUnits;
        OutSnapshot.TradeRouteExportContractUnits =
            BuildingRecord.TradeRouteExportContractUnits;
        OutSnapshot.TourismArrivalCount =
            BuildingRecord.TourismArrivalCount;
        OutSnapshot.BudgetScale = BuildingRecord.BudgetScale;
        OutSnapshot.AccessibilityScore =
            BuildingRecord.AccessibilityScore;
        OutSnapshot.PowerSupplyRatio = BuildingRecord.PowerSupplyRatio;
        OutSnapshot.HarborShipProgressPercent =
            BuildingRecord.HarborShipProgressPercent;
        OutSnapshot.ActiveOperationModeIndex =
            BuildingRecord.ActiveOperationModeIndex;
        OutSnapshot.ActiveRuntimeUpgradeIndex =
            BuildingRecord.ActiveRuntimeUpgradeIndex;
        OutSnapshot.ActiveOperationModeText =
            BuildingRecord.ActiveOperationModeText;
        OutSnapshot.ActiveOperationModeEffectSummary =
            BuildingRecord.ActiveOperationModeEffectSummary;
        OutSnapshot.ActiveRuntimeUpgradeText =
            BuildingRecord.ActiveRuntimeUpgradeText;
        OutSnapshot.ActiveRuntimeUpgradeEffectSummary =
            BuildingRecord.ActiveRuntimeUpgradeEffectSummary;
        OutSnapshot.LogisticsLines = BuildingRecord.LogisticsLines;
        OutSnapshot.Residents = BuildingRecord.Residents;
        OutSnapshot.AssignedEmployees = BuildingRecord.AssignedEmployees;
        OutSnapshot.WorkingEmployees = BuildingRecord.WorkingEmployees;
        OutSnapshot.AssignedVisitors = BuildingRecord.AssignedVisitors;
        OutSnapshot.ArrivedVisitors = BuildingRecord.ArrivedVisitors;
        OutSnapshot.IncomingVisitors = BuildingRecord.IncomingVisitors;
        OutSnapshot.HarborPolicyLines = BuildingRecord.HarborPolicyLines;
        OutSnapshot.HarborPriorityLines = BuildingRecord.HarborPriorityLines;
        OutSnapshot.WarehousePolicySelectionText =
            BuildingRecord.WarehousePolicySelectionText;
        OutSnapshot.WarehousePrioritySelectionText =
            BuildingRecord.WarehousePrioritySelectionText;
        OutSnapshot.HarborDomesticReserveSelectionText =
            BuildingRecord.HarborDomesticReserveSelectionText;
        OutSnapshot.HarborExportSelectionText =
            BuildingRecord.HarborExportSelectionText;
        OutSnapshot.HarborImportCapSelectionText =
            BuildingRecord.HarborImportCapSelectionText;
        OutSnapshot.HarborImportBudgetSelectionText =
            BuildingRecord.HarborImportBudgetSelectionText;
        OutSnapshot.HarborImportSelectionText =
            BuildingRecord.HarborImportSelectionText;

        if (OutSnapshot.Warehouse)
        {
            for (size_t SlotIndex = 0;
                SlotIndex < BuildingRecord.WarehouseSlots.size();
                ++SlotIndex)
            {
                const CitizenInfoDataProvider::FWarehouseSlotRecord& SlotRecord =
                    BuildingRecord.WarehouseSlots[SlotIndex];
                std::wstring SlotValue;

                if (SlotRecord.Type == EResourceType::None)
                {
                    SlotValue =
                        UIStrings::Get(L"citizen_info.building.warehouse.empty") +
                        L" / " +
                        FormatInteger(SlotRecord.Capacity);
                }
                else
                {
                    SlotValue =
                        std::wstring(GetResourceTypeDisplayName(
                            SlotRecord.Type)) +
                        L" " +
                        FormatInteger(SlotRecord.Stock) +
                        L" / " +
                        FormatInteger(SlotRecord.Capacity);

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

        const int EffectiveRequiredPowerMW =
            OutSnapshot.RequiredPowerMW > 0 ?
                OutSnapshot.RequiredPowerMW :
                (OutSnapshot.CatalogEntry ?
                    OutSnapshot.CatalogEntry->BaseRequiredPowerMW :
                    0);
        const int EffectiveProducedPowerMW =
            OutSnapshot.ProducedPowerMW > 0 ?
                OutSnapshot.ProducedPowerMW :
                (OutSnapshot.CatalogEntry ?
                    OutSnapshot.CatalogEntry->BaseProducedPowerMW :
                    0);
        OutSnapshot.RequiredPowerText =
            EffectiveRequiredPowerMW > 0 ?
                FormatMegawattValue(EffectiveRequiredPowerMW) :
                std::wstring();
        OutSnapshot.ProducedPowerText =
            EffectiveProducedPowerMW > 0 ?
                FormatMegawattValue(EffectiveProducedPowerMW) :
                std::wstring();
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
            ExtractDetailValue(OutSnapshot.DetailText, L"효과:");
        OutSnapshot.NoteText =
            ExtractDetailValue(OutSnapshot.DetailText, L"비고:");

        const int CatalogServiceCapacity = OutSnapshot.CatalogEntry ?
            (std::max)(0, OutSnapshot.CatalogEntry->ServiceCapacity) :
            0;
        OutSnapshot.ServiceCapacity = (std::max)(
            BuildingRecord.ServiceCapacity,
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
            ExtractBulletSection(OutSnapshot.DetailText, L"업그레이드");
        OutSnapshot.NarrativeLines =
            ExtractNarrativeLines(OutSnapshot.DetailText);
        return true;
    }
}
