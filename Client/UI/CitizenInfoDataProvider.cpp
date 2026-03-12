#include "CitizenInfoDataProvider.h"
#include "CitizenInfoBuildingRuntime.h"
#include "CitizenInfoConstants.h"
#include "CitizenInfoQueryService.h"
#include "UIStrings.h"
#include "../Building/BuildingCatalog.h"
#include "../Economy/ResourceTradePricing.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <vector>

namespace
{
    using CitizenInfoConstants::GBuildingTabCount;
    using CitizenInfoConstants::GCitizenTabCount;
    using FBuildingUiSnapshot = CitizenInfoBuildingRuntime::FBuildingUiSnapshot;

    std::string BuildCatalogIconTextureKey(
        const FBuildingCatalogEntry& Entry)
    {
        return
            "BuildingCatalogIcon_" +
            Entry.Id +
            "_Gen_" +
            std::to_string(::GetRuntimeConfigGeneration());
    }

    #if 0
    struct FBuildingUiSnapshot
    {
        const FBuildingCatalogEntry* CatalogEntry = nullptr;
        std::string BuildingId;
        std::wstring ObjectName;
        std::wstring DisplayName;
        std::wstring CategoryName;
        std::wstring DetailText;
        std::wstring RequiredPowerText;
        std::wstring ProducedPowerText;
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
        std::vector<std::wstring> NarrativeLines;
        std::vector<std::wstring> LogisticsLines;
        std::vector<std::wstring> UpgradeHints;
        std::vector<std::wstring> OperationModes;
        std::vector<std::wstring> WarehouseSlotLines;
        std::vector<std::wstring> HarborPolicyLines;
        std::vector<std::wstring> HarborPriorityLines;
        std::wstring WarehousePolicySelectionText;
        std::wstring WarehousePrioritySelectionText;
        std::wstring HarborDomesticReserveSelectionText;
        std::wstring HarborExportSelectionText;
        std::wstring HarborImportCapSelectionText;
        std::wstring HarborImportBudgetSelectionText;
        std::wstring HarborImportSelectionText;
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
        int ActiveOperationModeIndex = 0;
        int ActiveRuntimeUpgradeIndex = -1;
        float BudgetScale = 1.f;
        float AccessibilityScore = 0.f;
        float PowerSupplyRatio = 1.f;
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
    };
    #endif

    std::wstring Utf8ToWide(const std::string& Text)
    {
        if (Text.empty())
            return std::wstring();

        const int RequiredCount = MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), -1, nullptr, 0);

        if (RequiredCount <= 1)
            return std::wstring(Text.begin(), Text.end());

        std::wstring WideText;
        WideText.resize(RequiredCount - 1);
        MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(),
            static_cast<int>(Text.size()),
            &WideText[0], RequiredCount - 1);
        return WideText;
    }

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

    void AppendLine(std::wstring& Body, const std::wstring& Line)
    {
        if (Line.empty())
            return;

        if (!Body.empty())
            Body += L"\n";

        Body += Line;
    }

    std::wstring JoinLines(const std::vector<std::wstring>& Lines)
    {
        std::wstring Result;

        for (size_t Index = 0; Index < Lines.size(); ++Index)
            AppendLine(Result, Lines[Index]);

        return Result;
    }

    const std::wstring& Ui(const wchar_t* Key)
    {
        return UIStrings::Get(Key);
    }

    const wchar_t* UiText(const wchar_t* Key)
    {
        return UIStrings::Get(Key).c_str();
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

    std::wstring FormatCatalogCostValue(
        EBuildingCostState State,
        int Value)
    {
        switch (State)
        {
        case EBuildingCostState::Known:
            return Value <= 0 ?
                Ui(L"citizen_info.value.free") :
                FormatMoney(Value);
        case EBuildingCostState::Unknown:
            return Ui(L"citizen_info.value.unknown");
        case EBuildingCostState::None:
        default:
            return std::wstring();
        }
    }

    std::wstring JoinInlineSegments(
        const std::vector<std::wstring>& Segments)
    {
        std::wstring Result;

        for (size_t Index = 0; Index < Segments.size(); ++Index)
        {
            const std::wstring Segment = Trim(Segments[Index]);

            if (Segment.empty())
                continue;

            if (!Result.empty())
                Result += L", ";

            Result += Segment;
        }

        return Result;
    }

    const FBuildingRuntimeUpgradeDef* ResolveActiveRuntimeUpgradeDef(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!Snapshot.CatalogEntry)
            return nullptr;

        if (Snapshot.ActiveRuntimeUpgradeIndex < 0 ||
            Snapshot.ActiveRuntimeUpgradeIndex >=
                static_cast<int>(
                    Snapshot.CatalogEntry->RuntimeUpgradeDefs.size()))
        {
            return nullptr;
        }

        return &Snapshot.CatalogEntry->RuntimeUpgradeDefs[
            static_cast<size_t>(Snapshot.ActiveRuntimeUpgradeIndex)];
    }

    std::wstring BuildRuntimeUpgradeSummary(
        const FBuildingRuntimeUpgradeDef& UpgradeDef,
        const std::wstring* OverrideEffectSummary = nullptr)
    {
        std::vector<std::wstring> Segments;

        if (UpgradeDef.HasUnlockEra)
            Segments.push_back(GetBuildingEraDisplayName(UpgradeDef.UnlockEra));

        const std::wstring CostText =
            FormatCatalogCostValue(UpgradeDef.CostState, UpgradeDef.Cost);

        if (!CostText.empty())
            Segments.push_back(CostText);

        const std::wstring EffectSummary =
            OverrideEffectSummary && !OverrideEffectSummary->empty() ?
                *OverrideEffectSummary :
                UpgradeDef.EffectSummary;

        if (!EffectSummary.empty())
            Segments.push_back(EffectSummary);

        return JoinInlineSegments(Segments);
    }

    std::wstring BuildRuntimeUpgradeLine(
        const FBuildingRuntimeUpgradeDef& UpgradeDef,
        bool Active,
        const std::wstring* OverrideEffectSummary = nullptr)
    {
        std::wstring Line =
            (Active ? L"* " : L"- ") + UpgradeDef.DisplayName;
        const std::wstring Summary =
            BuildRuntimeUpgradeSummary(UpgradeDef, OverrideEffectSummary);

        if (!Summary.empty())
        {
            Line += L" (";
            Line += Summary;
            Line += L")";
        }

        return Line;
    }

    std::wstring FormatMultiplier(float Value)
    {
        wchar_t Buffer[32] = {};
        swprintf_s(Buffer, L"x%.2f", Value);
        return Buffer;
    }

    std::wstring FormatSignedPercentValue(int Value)
    {
        return std::wstring(Value > 0 ? L"+" : L"") +
            std::to_wstring(Value) +
            L"%";
    }

    std::wstring BuildMarketPriceSummary(EResourceType Type)
    {
        if (!IsExportableResourceType(Type))
            return std::wstring();

        const int BaseShiftPercent =
            ResourceTradePricing::GetExportPriceIndexPercent(Type) - 100;
        const int DailyShiftPercent =
            ResourceTradePricing::GetExportPriceDeltaPercent(Type);
        return L"기준 " +
            FormatSignedPercentValue(BaseShiftPercent) +
            L" / 전일 " +
            FormatSignedPercentValue(DailyShiftPercent);
    }

    std::wstring FormatMegawattValue(int Value)
    {
        return std::to_wstring(Value) +
            UIStrings::Get(L"citizen_info.unit.megawatt");
    }

    std::wstring FormatSignedMegawattValue(int Value)
    {
        return std::wstring(Value > 0 ? L"+" : L"") +
            FormatMegawattValue(Value);
    }

    std::wstring FormatPowerCoverageValue(float Ratio)
    {
        return std::to_wstring(static_cast<int>(roundf(
            Clamp<float>(Ratio, 0.f, 1.f) * 100.f))) + L"%";
    }

    std::wstring FormatDayCount(int Value)
    {
        return std::to_wstring(Value) +
            UIStrings::Get(L"citizen_info.unit.day_suffix");
    }

    int ParseLeadingInteger(const std::wstring& Text, int DefaultValue = 0)
    {
        bool Negative = false;
        bool FoundDigit = false;
        int Value = 0;

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const wchar_t Ch = Text[Index];

            if (!FoundDigit && Ch == L'-')
            {
                Negative = true;
                continue;
            }

            if (Ch < L'0' || Ch > L'9')
            {
                if (FoundDigit)
                    break;

                continue;
            }

            FoundDigit = true;
            Value = Value * 10 + static_cast<int>(Ch - L'0');
        }

        if (!FoundDigit)
            return DefaultValue;

        return Negative ? -Value : Value;
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

    int ExtractPowerValueMW(
        const std::wstring& DetailText,
        const wchar_t* Prefix)
    {
        return ParseLeadingInteger(
            ExtractDetailValue(DetailText, Prefix),
            0);
    }

    std::wstring NormalizeWealthRequirementText(const std::wstring& Value)
    {
        if (Value == UIStrings::Get(L"citizen.wealth.well_off"))
            return UIStrings::Get(L"citizen_info.wealth_profile.well_off");
        if (Value == UIStrings::Get(L"citizen.wealth.rich"))
            return UIStrings::Get(L"citizen_info.wealth_profile.rich");
        if (Value == UIStrings::Get(L"citizen.wealth.poor"))
            return UIStrings::Get(L"citizen_info.wealth_profile.poor");

        return Value;
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

    #if 0
    bool HasBuildingId(
        const FBuildingUiSnapshot& Snapshot,
        const char* BuildingId)
    {
        return BuildingId &&
            Snapshot.BuildingId == BuildingId;
    }

    bool IsHydroponicFarmBuilding(const FBuildingUiSnapshot& Snapshot)
    {
        return HasBuildingId(Snapshot, "build_2_10") ||
            Snapshot.DisplayName == L"대규모 수경 농장";
    }

    bool IsCustomsOfficeBuilding(const FBuildingUiSnapshot& Snapshot)
    {
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

    int ResolveCustomsBudgetModifierPercent(
        const FBuildingUiSnapshot& Snapshot)
    {
        return static_cast<int>(std::lround(
            (Snapshot.BudgetScale - 1.f) * 100.f));
    }

    int ResolveCustomsEfficiencyPercent(
        const FBuildingUiSnapshot& Snapshot)
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
    #endif

    int ResolveOverviewHousingQuality(const FBuildingUiSnapshot& Snapshot)
    {
        const int Parsed =
            ParseLeadingInteger(Snapshot.HousingQualityText, Snapshot.HousingCap);
        return (std::max)(0, Parsed);
    }

    long long ResolveOverviewMonthlyIncome(const FBuildingUiSnapshot& Snapshot)
    {
        return static_cast<long long>((std::max)(0, Snapshot.Capacity)) * 3LL;
    }

    int ResolveOverviewRequiredPower(const FBuildingUiSnapshot& Snapshot)
    {
        return (std::max)(0, Snapshot.RequiredPowerMW);
    }

    bool UseGenericBuildingWorkOverview(const FBuildingUiSnapshot& Snapshot)
    {
        return !Snapshot.Residential &&
            Snapshot.WorkProvider &&
            !Snapshot.IsRoad;
    }

    struct FOverviewMetricWriter
    {
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot;
        int WriteIndex = 0;

        void Add(
            const std::wstring& Label,
            const std::wstring& Value,
            bool Accent = false)
        {
            if (WriteIndex < 0 ||
                WriteIndex >=
                    static_cast<int>(Snapshot.OverviewMetricLabels.size()) ||
                Label.empty())
            {
                return;
            }

            Snapshot.OverviewMetricLabels[static_cast<size_t>(WriteIndex)] =
                Label;
            Snapshot.OverviewMetricValues[static_cast<size_t>(WriteIndex)] =
                Value;
            Snapshot.OverviewMetricAccentValues[
                static_cast<size_t>(WriteIndex)] = Accent;
            ++WriteIndex;
        }

        void AddHeader(const std::wstring& Label)
        {
            Add(Label, std::wstring(), false);
        }
    };

    std::wstring ResolveOverviewBudgetValue(const FBuildingUiSnapshot& Snapshot)
    {
        return FormatMoney(Snapshot.MonthlyUpkeepCost);
    }

    std::wstring ResolveWorkerOverviewEfficiency(
        const FBuildingUiSnapshot& Snapshot)
    {
        const int EfficiencyPercent = static_cast<int>(roundf(
            Snapshot.BudgetScale *
            (Snapshot.RequiredPowerMW > 0 ?
                Snapshot.PowerSupplyRatio :
                1.f) *
            100.f));
        return std::to_wstring((std::max)(0, EfficiencyPercent)) + L"%";
    }

    std::wstring ResolveWorkerOverviewJobQuality(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!Snapshot.JobQualityText.empty())
            return Snapshot.JobQualityText;

        if (Snapshot.JobCap > 0)
            return std::to_wstring(Snapshot.JobCap);

        return L"-";
    }

    std::wstring ResolveWorkerOverviewVisitorValue(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.ServiceCapacity > 0)
        {
            return std::to_wstring(Snapshot.AssignedVisitors.size()) +
                L" / " +
                std::to_wstring((std::max)(0, Snapshot.ServiceCapacity));
        }

        if (!Snapshot.AssignedVisitors.empty())
            return std::to_wstring(Snapshot.AssignedVisitors.size());

        return std::wstring();
    }

    const wchar_t* GetServiceCapacityLabelKey(
        const FBuildingUiSnapshot& Snapshot)
    {
        return Snapshot.ServiceCapacityUsesHouseholds ?
            L"citizen_info.label.household_capacity" :
            L"citizen_info.label.service_capacity";
    }

    std::wstring ResolveWorkerOverviewPreferredType(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!Snapshot.TouristPreferenceText.empty())
            return Snapshot.TouristPreferenceText;

        if (Snapshot.CatalogEntry &&
            Snapshot.CatalogEntry->PrimaryTouristPreference !=
                ETouristPreference::None)
        {
            return GetTouristPreferenceDisplayName(
                Snapshot.CatalogEntry->PrimaryTouristPreference);
        }

        return std::wstring();
    }

    std::wstring ResolveWorkerOverviewPowerValue(
        const FBuildingUiSnapshot& Snapshot)
    {
        const int RequiredPowerMW = (std::max)(0, Snapshot.RequiredPowerMW);

        if (RequiredPowerMW > 0)
        {
            std::wstring Value = FormatMegawattValue(-RequiredPowerMW);
            Value += L" (";
            Value += FormatPowerCoverageValue(Snapshot.PowerSupplyRatio);
            Value += L")";
            return Value;
        }

        if (!Snapshot.RequiredPowerText.empty())
            return L"-" + Snapshot.RequiredPowerText;

        const int ProducedPowerMW = (std::max)(0, Snapshot.ProducedPowerMW);

        if (ProducedPowerMW > 0)
            return L"+" + FormatMegawattValue(ProducedPowerMW);

        if (!Snapshot.ProducedPowerText.empty())
            return L"+" + Snapshot.ProducedPowerText;

        return std::wstring();
    }

    std::wstring ResolveWorkerOverviewStorageValue(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!Snapshot.Harbor &&
            !Snapshot.Warehouse &&
            !Snapshot.CanGenerateWorkOutput &&
            Snapshot.ResourceStock <= 0)
        {
            return std::wstring();
        }

        return FormatInteger(Snapshot.ResourceStock) +
            L" / " +
            FormatInteger(Snapshot.MaxResourceStock);
    }

    void PopulateGenericWorkOverview(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result)
    {
        Result.OverviewWorkModeLabel =
            Ui(L"citizen_info.label.work_mode");
        Result.OverviewWorkModeValue =
            !Snapshot.ActiveOperationModeText.empty() ?
                Snapshot.ActiveOperationModeText :
                (!Snapshot.OperationModes.empty() ?
                    Snapshot.OperationModes.front() :
                    Ui(L"citizen_info.work_mode.general_control"));
        Result.OverviewBudgetLabel = Ui(L"citizen_info.label.budget");
        Result.OverviewBudgetValue = ResolveOverviewBudgetValue(Snapshot);
        Result.OverviewOccupancyLabel = Ui(L"citizen_info.label.workers");
        Result.OverviewOccupancyValue =
            std::to_wstring(Snapshot.AssignedEmployees.size()) +
            L" / " +
            std::to_wstring((std::max)(0, Snapshot.Capacity));
        Result.OverviewResidentCount =
            static_cast<int>(Snapshot.AssignedEmployees.size());
        Result.OverviewResidentCapacity =
            (std::max)(0, Snapshot.Capacity);

        const bool HasServiceBlock =
            Snapshot.ServiceCapacity > 0 ||
            !Snapshot.ServiceQualityText.empty() ||
            !Snapshot.WealthRequirementText.empty() ||
            !ResolveWorkerOverviewPreferredType(Snapshot).empty();

        if (HasServiceBlock)
        {
            Result.ShowBuildingVisitorIcons = Snapshot.ServiceCapacity > 0;
            Result.OverviewVisitorCount =
                static_cast<int>(Snapshot.AssignedVisitors.size());
            Result.OverviewVisitorCapacity =
                (std::max)(
                    static_cast<int>(Snapshot.AssignedVisitors.size()),
                    (std::max)(0, Snapshot.ServiceCapacity));
        }

        FOverviewMetricWriter Writer{ Result };
        Writer.Add(
            Ui(L"citizen_info.label.job_quality"),
            ResolveWorkerOverviewJobQuality(Snapshot));
        Writer.Add(
            Ui(L"citizen_info.label.required_education"),
            GetCitizenEducationDisplayName(
                Snapshot.RequiredEducationLevel));
        Writer.Add(
            Ui(L"citizen_info.label.monthly_wage_cost"),
            FormatMoney(Snapshot.MonthlyWageCost));
        Writer.Add(
            Ui(L"citizen_info.label.efficiency"),
            ResolveWorkerOverviewEfficiency(Snapshot));

        const std::wstring PowerValue =
            ResolveWorkerOverviewPowerValue(Snapshot);

        if (!PowerValue.empty() || !Snapshot.ProducedPowerText.empty())
        {
            Writer.Add(Ui(L"citizen_info.label.electricity"), PowerValue);
            Writer.Add(Ui(L"citizen_info.label.power_network"), L"#1");

            const int PowerSurplusMW =
                Snapshot.TotalProducedPowerMW -
                Snapshot.TotalRequiredPowerMW;
            Writer.Add(
                Ui(L"citizen_info.label.power_grid_status"),
                FormatSignedMegawattValue(PowerSurplusMW),
                PowerSurplusMW >= 0);
        }
        else
        {
            Writer.Add(
                Ui(L"citizen_info.label.road_accessibility"),
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.AccessibilityScore * 100.f))) +
                    L"%");
        }

        if (HasServiceBlock)
        {
            const std::wstring VisitorValue =
                ResolveWorkerOverviewVisitorValue(Snapshot);

            if (!VisitorValue.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.visitors"),
                    VisitorValue);
            }

            if (!Snapshot.ServiceQualityText.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.service_quality"),
                    Snapshot.ServiceQualityText);
            }

            if (!Snapshot.WealthRequirementText.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.required_wealth"),
                    NormalizeWealthRequirementText(
                        Snapshot.WealthRequirementText));
            }

            const std::wstring PreferredType =
                ResolveWorkerOverviewPreferredType(Snapshot);

            if (!PreferredType.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.preferred_type"),
                    PreferredType);
            }
        }

        const std::wstring StorageValue =
            ResolveWorkerOverviewStorageValue(Snapshot);

        if (!StorageValue.empty())
        {
            Writer.AddHeader(Ui(L"citizen_info.label.storage"));
            Writer.Add(Ui(L"citizen_info.label.stock"), StorageValue);

            if (Snapshot.Harbor)
            {
                Writer.Add(
                    Ui(L"citizen_info.label.next_arrival_time"),
                    FormatDayCount((std::max)(
                        0,
                        static_cast<int>(roundf(
                            (1.f - Snapshot.HarborShipProgressPercent) *
                            3.f *
                            static_cast<float>((std::max)(
                                1,
                                Snapshot.DaysInMonth)))))));
                Writer.Add(
                    Ui(L"citizen_info.label.exportable_stock"),
                    FormatInteger(Snapshot.ExportableStock));
            }
        }
        else if (Snapshot.CanGenerateWorkOutput)
        {
            Writer.Add(
                Ui(L"citizen_info.label.current_stock"),
                FormatInteger(Snapshot.ResourceStock));
        }
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
                    UIStrings::Get(L"citizen_info.section.main_effect") + L": " +
                    Trim(Line.substr(wcslen(L"효과:"))));
                continue;
            }

            if (StartsWith(Line, L"비고:"))
            {
                Result.push_back(
                    UIStrings::Get(L"citizen_info.section.note") + L": " +
                    Trim(Line.substr(wcslen(L"비고:"))));
                continue;
            }

            if (Line.find(L':') != std::wstring::npos)
                continue;

            Result.push_back(Line);
        }

        return Result;
    }

    void PushUnique(std::vector<std::string>& Names, const std::string& Name)
    {
        if (Name.empty())
            return;

        if (std::find(Names.begin(), Names.end(), Name) == Names.end())
            Names.push_back(Name);
    }

    std::wstring SummarizeNames(const std::vector<std::string>& Names)
    {
        if (Names.empty())
            return L"-";

        constexpr size_t GMaxShownNames = 5;
        const size_t CountToShow = (std::min)(Names.size(), GMaxShownNames);
        std::wstring Summary;

        for (size_t Index = 0; Index < CountToShow; ++Index)
        {
            if (Index > 0)
                Summary += L", ";

            Summary += Utf8ToWide(Names[Index]);
        }

        if (Names.size() > GMaxShownNames)
        {
            Summary += L" (+";
            Summary += std::to_wstring(Names.size() - GMaxShownNames);
            Summary += L")";
        }

        return Summary;
    }

    const wchar_t* GetHousingClassDisplayName(
        EBuildingHousingClass HousingClass)
    {
        switch (HousingClass)
        {
        case EBuildingHousingClass::Collective:
            return UiText(L"citizen_info.building.housing_class.collective");
        case EBuildingHousingClass::Standard:
            return UiText(L"citizen_info.building.housing_class.standard");
        case EBuildingHousingClass::Elite:
            return UiText(L"citizen_info.building.housing_class.elite");
        default:
            return UiText(L"citizen_info.building.housing_class.default");
        }
    }

    const wchar_t* GetLeisureClassDisplayName(
        EBuildingLeisureClass LeisureClass)
    {
        switch (LeisureClass)
        {
        case EBuildingLeisureClass::Cultural:
            return UiText(L"citizen_info.building.leisure_class.cultural");
        case EBuildingLeisureClass::Luxury:
            return UiText(L"citizen_info.building.leisure_class.luxury");
        case EBuildingLeisureClass::General:
            return UiText(L"citizen_info.building.leisure_class.general");
        default:
            return UiText(L"citizen_info.building.leisure_class.unknown");
        }
    }

    std::wstring ResolveRoleSummary(const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.Residential)
        {
            std::wstring Summary = UIStrings::Format(
                L"citizen_info.building.role.residential",
                {
                    Snapshot.DisplayName,
                    std::to_wstring((std::max)(0, Snapshot.Capacity))
                });

            if (Snapshot.CatalogEntry)
            {
                Summary += L" ";
                Summary += UIStrings::Format(
                    L"citizen_info.building.role.residential_class_suffix",
                    {
                        GetHousingClassDisplayName(
                            Snapshot.CatalogEntry->HousingClass)
                    });
            }

            return Summary;
        }

        if (Snapshot.Harbor)
            return UIStrings::Get(L"citizen_info.building.role.harbor");

        if (Snapshot.EntertainmentProvider && Snapshot.FoodProvider)
            return UIStrings::Get(
                L"citizen_info.building.role.food_entertainment");

        if (Snapshot.EntertainmentProvider)
        {
            std::wstring Summary =
                UIStrings::Get(L"citizen_info.building.role.entertainment");

            if (Snapshot.CatalogEntry)
            {
                Summary += L" ";
                Summary += UIStrings::Format(
                    L"citizen_info.building.role.entertainment_class_suffix",
                    {
                        GetLeisureClassDisplayName(
                            Snapshot.CatalogEntry->LeisureClass)
                    });
            }

            return Summary;
        }

        if (Snapshot.FoodProvider)
            return UIStrings::Get(L"citizen_info.building.role.food");

        if (Snapshot.CanGenerateWorkOutput)
            return UIStrings::Get(L"citizen_info.building.role.work_output");

        return UIStrings::Get(L"citizen_info.building.role.support");
    }

    void AppendKeyValue(
        std::wstring& Body,
        const wchar_t* Key,
        const std::wstring& Value)
    {
        if (Value.empty())
            return;

        AppendLine(Body, std::wstring(Key) + L": " + Value);
    }

    void AppendKeyValueByKey(
        std::wstring& Body,
        const wchar_t* LabelKey,
        const std::wstring& Value)
    {
        if (Value.empty())
            return;

        AppendLine(
            Body,
            UIStrings::Get(LabelKey) + L": " + Value);
    }

    bool HasTradeUnitPrice(EResourceType Type)
    {
        return IsExportableResourceType(Type);
    }

    std::wstring FormatTradeUnitPriceInline(EResourceType Type)
    {
        if (!HasTradeUnitPrice(Type))
            return std::wstring();

        return Ui(L"citizen_info.label.export_short") +
            L" " +
            FormatMoneyDollarFirst(
                ResourceTradePricing::GetExportPricePerStockUnit(Type)) +
            L" / " +
            Ui(L"citizen_info.label.import_short") +
            L" " +
            FormatMoneyDollarFirst(
                ResourceTradePricing::GetImportPricePerStockUnit(Type));
    }

    void AppendProducedResourceTradeLines(
        std::wstring& Body,
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!HasTradeUnitPrice(Snapshot.ProducedResourceType))
            return;

        const std::wstring ProducedResourceDisplayName =
            Snapshot.CatalogEntry ?
                GetBuildingProducedResourceDisplayName(
                    *Snapshot.CatalogEntry) :
                std::wstring(GetResourceTypeDisplayName(
                    Snapshot.ProducedResourceType));

        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.produced_resource",
            ProducedResourceDisplayName);
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.export_unit_price",
            FormatMoneyDollarFirst(
                ResourceTradePricing::GetExportPricePerStockUnit(
                    Snapshot.ProducedResourceType)));
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.import_unit_price",
            FormatMoneyDollarFirst(
                ResourceTradePricing::GetImportPricePerStockUnit(
                    Snapshot.ProducedResourceType)));
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.market_price",
            BuildMarketPriceSummary(Snapshot.ProducedResourceType));
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.instant_export_value",
            FormatMoney(
                ResourceTradePricing::ComputeExportValue(
                    Snapshot.ProducedResourceType,
                    Snapshot.ProducedResourceStock)));
    }

    void AppendHarborTradePriceReference(std::wstring& Body)
    {
        AppendLine(Body, Ui(L"citizen_info.section.trade_prices"));

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsExportableResourceType(ResourceType))
                continue;

            AppendLine(
                Body,
                std::wstring(GetResourceTypeDisplayName(ResourceType)) +
                    L": " +
                    FormatTradeUnitPriceInline(ResourceType));
        }
    }

    void AppendHarborPolicyReference(
        std::wstring& Body,
        const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.HarborPolicyLines.empty())
            return;

        AppendLine(Body, Ui(L"citizen_info.section.export_policy"));

        for (size_t Index = 0; Index < Snapshot.HarborPolicyLines.size(); ++Index)
            AppendLine(Body, Snapshot.HarborPolicyLines[Index]);
    }

    void AppendHarborPriorityReference(
        std::wstring& Body,
        const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.HarborPriorityLines.empty())
            return;

        AppendLine(Body, Ui(L"citizen_info.section.export_priority"));

        for (size_t Index = 0;
            Index < Snapshot.HarborPriorityLines.size();
            ++Index)
        {
            AppendLine(Body, Snapshot.HarborPriorityLines[Index]);
        }
    }

    #if 0
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

        OutSnapshot.RequiredPowerText =
            OutSnapshot.RequiredPowerMW > 0 ?
                FormatMegawattValue(OutSnapshot.RequiredPowerMW) :
                (OutSnapshot.CatalogEntry &&
                        OutSnapshot.CatalogEntry->BaseRequiredPowerMW > 0 ?
                    FormatMegawattValue(
                        OutSnapshot.CatalogEntry->BaseRequiredPowerMW) :
                    ExtractDetailValue(OutSnapshot.DetailText, L"필요 전력:"));
        OutSnapshot.ProducedPowerText =
            OutSnapshot.ProducedPowerMW > 0 ?
                FormatMegawattValue(OutSnapshot.ProducedPowerMW) :
                (OutSnapshot.CatalogEntry &&
                        OutSnapshot.CatalogEntry->BaseProducedPowerMW > 0 ?
                    FormatMegawattValue(
                        OutSnapshot.CatalogEntry->BaseProducedPowerMW) :
                    ExtractDetailValue(OutSnapshot.DetailText, L"생산 전력:"));

        if (OutSnapshot.ProducedPowerText.empty() &&
            OutSnapshot.ProducedPowerMW <= 0)
        {
            const int FallbackProducedPowerMW =
                OutSnapshot.CatalogEntry &&
                    OutSnapshot.CatalogEntry->BaseProducedPowerMW > 0 ?
                    OutSnapshot.CatalogEntry->BaseProducedPowerMW :
                    ExtractPowerValueMW(OutSnapshot.DetailText, L"발전량:");

            if (FallbackProducedPowerMW > 0)
                OutSnapshot.ProducedPowerText =
                    FormatMegawattValue(FallbackProducedPowerMW);
        }
        OutSnapshot.JobQualityText =
            ExtractDetailValue(OutSnapshot.DetailText, L"직업 품질:");
        OutSnapshot.ServiceQualityText =
            ExtractDetailValue(OutSnapshot.DetailText, L"서비스 품질:");
        OutSnapshot.HousingQualityText =
            ExtractDetailValue(OutSnapshot.DetailText, L"주거 품질:");
        OutSnapshot.HouseholdCapacityText =
            OutSnapshot.HouseholdCapacity > 0 ?
                std::to_wstring(OutSnapshot.HouseholdCapacity) :
                ExtractDetailValue(OutSnapshot.DetailText, L"수용 가구:");
        OutSnapshot.WealthRequirementText =
            ExtractDetailValue(OutSnapshot.DetailText, L"재산 요구치:");

        if (OutSnapshot.WealthRequirementText.empty())
        {
            OutSnapshot.WealthRequirementText =
                ExtractDetailValue(OutSnapshot.DetailText, L"필요 재산:");
        }

        if (OutSnapshot.WealthRequirementText.empty())
        {
            OutSnapshot.WealthRequirementText =
                ExtractDetailValue(OutSnapshot.DetailText, L"관광객 재산:");
        }

        if (OutSnapshot.WealthRequirementText.empty() &&
            OutSnapshot.CatalogEntry)
        {
            OutSnapshot.WealthRequirementText =
                BuildAllowedWealthRequirementText(
                    OutSnapshot.CatalogEntry->AllowedWealthMask);
        }

        OutSnapshot.TouristPreferenceText =
            ExtractDetailValue(OutSnapshot.DetailText, L"선호 관광객:");
        OutSnapshot.EffectText =
            ExtractDetailValue(OutSnapshot.DetailText, L"효과:");
        OutSnapshot.NoteText =
            ExtractDetailValue(OutSnapshot.DetailText, L"비고:");
        std::wstring RawServiceCapacityText;
        bool RawServiceCapacityUsesHouseholds = false;
        if (!OutSnapshot.Residential)
        {
            RawServiceCapacityText =
                ExtractDetailValue(OutSnapshot.DetailText, L"수용 인원:");

            if (RawServiceCapacityText.empty())
            {
                RawServiceCapacityText =
                    ExtractDetailValue(OutSnapshot.DetailText, L"수용 가구:");
                RawServiceCapacityUsesHouseholds =
                    !RawServiceCapacityText.empty();
            }
        }

        const int CatalogServiceCapacity = OutSnapshot.CatalogEntry ?
            (std::max)(0, OutSnapshot.CatalogEntry->ServiceCapacity) :
            0;
        OutSnapshot.ServiceCapacity = (std::max)(
            BuildingRecord.ServiceCapacity,
            CatalogServiceCapacity);

        if (OutSnapshot.ServiceCapacity <= 0)
        {
            OutSnapshot.ServiceCapacity =
                ParseLeadingInteger(RawServiceCapacityText, 0);
        }

        OutSnapshot.ServiceCapacityText =
            OutSnapshot.ServiceCapacity > 0 ?
                std::to_wstring(OutSnapshot.ServiceCapacity) :
                RawServiceCapacityText;
        if (!OutSnapshot.ServiceCapacityUsesHouseholds)
        {
            OutSnapshot.ServiceCapacityUsesHouseholds =
                RawServiceCapacityUsesHouseholds;
        }
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
            OutSnapshot.HousingQualityText = std::to_wstring(OutSnapshot.HousingCap);

        if (OutSnapshot.WorkProvider)
            OutSnapshot.JobQualityText = std::to_wstring(OutSnapshot.JobCap);

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

        OutSnapshot.UpgradeHints = OutSnapshot.CatalogEntry ?
            OutSnapshot.CatalogEntry->UpgradeHints :
            ExtractBulletSection(OutSnapshot.DetailText, L"업그레이드");
        OutSnapshot.NarrativeLines =
            ExtractNarrativeLines(OutSnapshot.DetailText);
        return true;
    }
    #endif

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
            const wchar_t* const ProductionStageText =
                GetProductionChainStageDisplayName(
                    Snapshot.CatalogEntry->ProductionChainStage);

            if (ProductionStageText && *ProductionStageText)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.production_stage",
                    ProductionStageText);
            }

            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.production",
                BuildProductionChainSummary(*Snapshot.CatalogEntry));
        }

        if (Snapshot.Residential && Snapshot.CatalogEntry)
        {
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.housing_class",
                GetHousingClassDisplayName(
                    Snapshot.CatalogEntry->HousingClass));
        }

        if (Snapshot.ServiceCapacity > 0 ||
            !Snapshot.ServiceQualityText.empty() ||
            !Snapshot.WealthRequirementText.empty() ||
            !Snapshot.TouristPreferenceText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.service"));

            if (Snapshot.ServiceCapacity > 0)
            {
                AppendLine(
                    Body,
                    Ui(L"citizen_info.label.visitors") + L": " +
                    std::to_wstring(Snapshot.AssignedVisitors.size()) +
                    L" / " +
                    std::to_wstring(Snapshot.ServiceCapacity));
            }

            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.service_quality",
                Snapshot.ServiceQualityText);
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.wealth_requirement",
                Snapshot.WealthRequirementText);
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.tourist_preference",
                Snapshot.TouristPreferenceText);
        }

        if (!Snapshot.RequiredPowerText.empty() ||
            !Snapshot.ProducedPowerText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.power"));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.required_power",
                Snapshot.RequiredPowerText);
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.produced_power",
                Snapshot.ProducedPowerText);
            if (Snapshot.RequiredPowerMW > 0)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.power_coverage",
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

        if (Snapshot.UsesResourceStock || Snapshot.Harbor)
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.storage_logistics"));

            if (Snapshot.UsesResourceStock)
            {
                AppendLine(
                    Body,
                    Ui(L"citizen_info.label.stock") + L": " +
                    FormatInteger(Snapshot.ResourceStock) +
                    L" / " +
                    FormatInteger(Snapshot.MaxResourceStock));
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
                const wchar_t* const ProductionStageText =
                    GetProductionChainStageDisplayName(
                        Snapshot.CatalogEntry->ProductionChainStage);

                if (ProductionStageText && *ProductionStageText)
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
                            BuildProductionChainSummary(
                                *Snapshot.CatalogEntry));
                }
            }

            AppendLine(
                Body,
                Ui(L"citizen_info.label.current_stock") + L": " +
                FormatInteger(Snapshot.ResourceStock) +
                L" / " +
                FormatInteger(Snapshot.MaxResourceStock));
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
                std::to_wstring(Snapshot.WorkingEmployees.size()));
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

                if (IsActiveMode &&
                    !Snapshot.ActiveOperationModeEffectSummary.empty())
                {
                    Line += L" (";
                    Line += Snapshot.ActiveOperationModeEffectSummary;
                    Line += L")";
                }

                AppendLine(Body, Line);
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
        const int BudgetPercent =
            static_cast<int>(roundf(Snapshot.BudgetScale * 100.f));
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
            std::to_wstring(BudgetPercent) +
            L"%");
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

        if (!Snapshot.RequiredPowerText.empty())
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.power_demand",
                Snapshot.RequiredPowerText);

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
            Snapshot.RequiredPowerText);
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.produced_power",
            Snapshot.ProducedPowerText);
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

            const wchar_t* const ProductionStageText =
                GetProductionChainStageDisplayName(
                    Snapshot.CatalogEntry->ProductionChainStage);

            if (ProductionStageText && *ProductionStageText)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.production_stage",
                    ProductionStageText);
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.production",
                    BuildProductionChainSummary(*Snapshot.CatalogEntry));
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
            L"해당 건물의 근무 형태를 선택하십시오.";
        const std::wstring Description =
            CitizenInfoBuildingRuntime::ResolveCustomsModeDescription(
                Snapshot,
                Snapshot.ActiveOperationModeIndex);

        if (!Description.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Description);
        }

        return Body;
    }

    std::wstring BuildCustomsUpgradesBody(
        const FBuildingUiSnapshot& Snapshot)
    {
        return BuildUpgradesBody(Snapshot);
    }

    const wchar_t* GetCitizenProfileWealthDisplayName(
        ECitizenWealthLevel Level)
    {
        switch (Level)
        {
        case ECitizenWealthLevel::WellOff:
            return UiText(L"citizen_info.wealth_profile.well_off");
        case ECitizenWealthLevel::Rich:
            return UiText(L"citizen_info.wealth_profile.rich");
        default:
            return UiText(L"citizen_info.wealth_profile.poor");
        }
    }

    const wchar_t* GetCitizenActivityDisplayName(ECitizenState State)
    {
        switch (State)
        {
        case ECitizenState::GoingToWork:
        case ECitizenState::AtWork:
        case ECitizenState::GoingToTeamsterSource:
        case ECitizenState::GoingToTeamsterHarbor:
        case ECitizenState::GoingToTeamsterConsumerSource:
        case ECitizenState::GoingToTeamsterConsumerTarget:
        case ECitizenState::GoingToTeamsterOffice:
            return UiText(L"citizen_info.activity.work");
        case ECitizenState::GoingToFood:
        case ECitizenState::AtFood:
            return UiText(L"citizen_info.activity.food");
        case ECitizenState::GoingToHealth:
        case ECitizenState::AtHealth:
        case ECitizenState::GoingToFaith:
        case ECitizenState::AtFaith:
        case ECitizenState::GoingHome:
        case ECitizenState::AtHome:
        case ECitizenState::GoingToFun:
        case ECitizenState::AtFun:
            return UiText(L"citizen_info.activity.leisure");
        default:
            return UIStrings::Get(L"citizen_info.activity.move").c_str();
        }
    }

    std::wstring ResolveBuildingDisplayName(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>&
            QuerySource,
        const std::string& BuildingName)
    {
        if (BuildingName.empty())
            return L"-";

        if (!QuerySource)
            return Utf8ToWide(BuildingName);

        return QuerySource->ResolveBuildingDisplayName(BuildingName);
    }

    std::wstring ResolveCitizenLocationText(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>&
            QuerySource,
        const CitizenInfoDataProvider::FCitizenInfoCitizenRecord& Citizen)
    {
        if (!Citizen.Valid)
            return L"-";

        auto MakeInteriorText =
            [&](const std::string& BuildingName)
        {
            const std::wstring DisplayName =
                ResolveBuildingDisplayName(QuerySource, BuildingName);

            if (DisplayName.empty() || DisplayName == L"-")
                return std::wstring(L"-");

            return DisplayName +
                UIStrings::Get(L"citizen_info.location.interior_suffix");
        };

        switch (Citizen.State)
        {
        case ECitizenState::AtHome:
        case ECitizenState::GoingHome:
            return MakeInteriorText(Citizen.HomeBuildingName);
        case ECitizenState::AtWork:
        case ECitizenState::GoingToWork:
        case ECitizenState::GoingToTeamsterSource:
        case ECitizenState::GoingToTeamsterHarbor:
        case ECitizenState::GoingToTeamsterConsumerSource:
        case ECitizenState::GoingToTeamsterConsumerTarget:
        case ECitizenState::GoingToTeamsterOffice:
            return MakeInteriorText(Citizen.WorkBuildingName);
        case ECitizenState::AtFood:
        case ECitizenState::GoingToFood:
            return MakeInteriorText(
                Citizen.FoodVisitBuildingName.empty() ?
                    Citizen.FoodBuildingName :
                    Citizen.FoodVisitBuildingName);
        case ECitizenState::AtFun:
        case ECitizenState::GoingToFun:
            return MakeInteriorText(
                Citizen.FunVisitBuildingName.empty() ?
                    Citizen.FunBuildingName :
                    Citizen.FunVisitBuildingName);
        case ECitizenState::AtHealth:
        case ECitizenState::GoingToHealth:
            return MakeInteriorText(
                Citizen.HealthVisitBuildingName.empty() ?
                    Citizen.HealthBuildingName :
                    Citizen.HealthVisitBuildingName);
        case ECitizenState::AtFaith:
        case ECitizenState::GoingToFaith:
            return MakeInteriorText(
                Citizen.FaithVisitBuildingName.empty() ?
                    Citizen.FaithBuildingName :
                    Citizen.FaithVisitBuildingName);
        default:
            return UIStrings::Get(L"citizen_info.location.outside_tropico");
        }
    }

    const wchar_t* GetCitizenPoliticalIntensityDisplayName(
        EPoliticalAxis Axis,
        EPoliticalSupportLevel Support)
    {
        if (Support == EPoliticalSupportLevel::Strong)
        {
            return Axis == EPoliticalAxis::ReligionMilitarism ?
                UiText(L"citizen_info.politics.intensity.fervent") :
                UiText(L"citizen_info.politics.intensity.strong");
        }

        return GetPoliticalSupportDisplayName(Support);
    }

    std::array<std::wstring, 3> BuildCitizenOpinionLines(
        const FNpcPoliticalProfile& PoliticalProfile)
    {
        struct FOpinionEntry
        {
            EPoliticalAxis Axis;
            FNpcPoliticalChoice Choice;
        };

        const std::array<FOpinionEntry, 4> OrderedEntries =
        {{
            { EPoliticalAxis::ReligionMilitarism,
                PoliticalProfile.ReligionMilitarism },
            { EPoliticalAxis::EnvironmentIndustry,
                PoliticalProfile.EnvironmentIndustry },
            { EPoliticalAxis::IntellectualConservative,
                PoliticalProfile.IntellectualConservative },
            { EPoliticalAxis::Economy,
                PoliticalProfile.Economy }
        }};

        std::array<std::wstring, 3> Lines = {};
        int WriteIndex = 0;

        for (const FOpinionEntry& Entry : OrderedEntries)
        {
            if (WriteIndex >= static_cast<int>(Lines.size()))
                break;

            if (Entry.Choice.Stance == EPoliticalStance::Neutral)
                continue;

            Lines[static_cast<size_t>(WriteIndex)] =
                std::wstring(GetPoliticalFactionDisplayName(
                    Entry.Axis,
                    Entry.Choice.Stance)) +
                L" (" +
                GetCitizenPoliticalIntensityDisplayName(
                    Entry.Axis,
                    Entry.Choice.Support) +
                L")";
            ++WriteIndex;
        }

        return Lines;
    }

    float BuildCitizenSupportRatio(
        const FNpcSatisfaction& Satisfaction)
    {
        const float Overall =
            (std::max)(0.f, (std::min)(100.f, Satisfaction.Overall));
        const float Ratio = (Overall + 12.f) / 112.f;
        return (std::max)(0.f, (std::min)(1.f, Ratio));
    }
}

namespace CitizenInfoDataProvider
{
    FCitizenInfoSnapshot BuildCitizenSnapshot(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FCitizenIdentityProfile& IdentityProfile,
        const FNpcPoliticalProfile& PoliticalProfile,
        int TabIndex)
    {
        const int ClampedTab =
            (std::max)(0, (std::min)(GCitizenTabCount - 1, TabIndex));

        auto ToPercent = [](float Value)
        {
            const float Clamped = (std::max)(0.f, (std::min)(100.f, Value));
            return static_cast<int>(roundf(Clamped));
        };

        FCitizenInfoSnapshot Result;
        Result.Valid = true;
        Result.Mode = EPanelMode::Citizen;
        Result.SelectedTabIndex = ClampedTab;
        Result.Title = Utf8ToWide(CitizenName);
        Result.ShowTabButtons = true;
        Result.ShowBudgetControls = false;
        Result.ShowActionButtons = false;
        Result.ShowSectionRibbon = (ClampedTab > 0);
        Result.ShowTitleIcon = false;

        {
            if (IdentityProfile.IsTourist)
            {
                Result.Subtitle =
                    UIStrings::Get(L"citizen_info.subtitle.tourist") +
                    L"  |  " +
                    GetTouristPreferenceDisplayName(
                        IdentityProfile.TouristProfile);
            }
            else
            {
                Result.Subtitle = UIStrings::Format(
                    L"citizen_info.subtitle.citizen_identity_template",
                    {
                        GetCitizenWealthDisplayName(
                            IdentityProfile.WealthLevel),
                        GetCitizenEducationDisplayName(
                            IdentityProfile.EducationLevel),
                        IdentityProfile.IsImmigrant ?
                            Ui(L"citizen_info.fragment.immigrant") :
                            std::wstring()
                    });
            }
        }

        if (ClampedTab == 0)
        {
            Result.PageTitle =
                CitizenInfoConstants::GetCitizenPageTitle(ClampedTab);
            Result.BodyText = UIStrings::Format(
                L"citizen_info.body.citizen_overview",
                {
                    GetCitizenEducationDisplayName(
                        IdentityProfile.EducationLevel),
                    GetCitizenWealthDisplayName(
                        IdentityProfile.WealthLevel),
                    IdentityProfile.IsImmigrant ?
                        Ui(L"citizen_info.fragment.immigrant_parenthetical") :
                        std::wstring(),
                    std::to_wstring(ToPercent(Satisfaction.Food)),
                    std::to_wstring(ToPercent(Satisfaction.Health)),
                    std::to_wstring(ToPercent(Satisfaction.Fun)),
                    std::to_wstring(ToPercent(Satisfaction.Faith)),
                    std::to_wstring(ToPercent(Satisfaction.Housing)),
                    std::to_wstring(ToPercent(Satisfaction.Job)),
                    std::to_wstring(ToPercent(Satisfaction.CommuteTimePenalty)),
                    std::to_wstring(ToPercent(Satisfaction.Freedom)),
                    std::to_wstring(ToPercent(Satisfaction.Security)),
                    std::to_wstring(ToPercent(Satisfaction.Overall))
                });
        }
        else if (ClampedTab == 1)
        {
            Result.PageTitle =
                CitizenInfoConstants::GetCitizenPageTitle(ClampedTab);
            Result.BodyText = UIStrings::Format(
                L"citizen_info.body.citizen_politics",
                {
                    std::to_wstring(ToPercent(Satisfaction.Food)),
                    std::to_wstring(ToPercent(Satisfaction.Health)),
                    std::to_wstring(ToPercent(Satisfaction.Fun)),
                    std::to_wstring(ToPercent(Satisfaction.Faith)),
                    std::to_wstring(ToPercent(Satisfaction.Housing)),
                    std::to_wstring(ToPercent(Satisfaction.Job)),
                    std::to_wstring(ToPercent(Satisfaction.Freedom)),
                    std::to_wstring(ToPercent(Satisfaction.Security)),
                    std::to_wstring(ToPercent(Satisfaction.Overall)),
                    GetPoliticalFactionDisplayName(
                        EPoliticalAxis::Economy,
                        PoliticalProfile.Economy.Stance),
                    GetPoliticalSupportDisplayName(
                        PoliticalProfile.Economy.Support),
                    GetPoliticalFactionDisplayName(
                        EPoliticalAxis::ReligionMilitarism,
                        PoliticalProfile.ReligionMilitarism.Stance),
                    GetPoliticalSupportDisplayName(
                        PoliticalProfile.ReligionMilitarism.Support),
                    GetPoliticalFactionDisplayName(
                        EPoliticalAxis::EnvironmentIndustry,
                        PoliticalProfile.EnvironmentIndustry.Stance),
                    GetPoliticalSupportDisplayName(
                        PoliticalProfile.EnvironmentIndustry.Support),
                    GetPoliticalFactionDisplayName(
                        EPoliticalAxis::IntellectualConservative,
                        PoliticalProfile.IntellectualConservative.Stance),
                    GetPoliticalSupportDisplayName(
                        PoliticalProfile.IntellectualConservative.Support)
                });
        }
        else
        {
            Result.PageTitle =
                CitizenInfoConstants::GetCitizenPageTitle(ClampedTab);
            const int Overall = ToPercent(Satisfaction.Overall);

            const wchar_t* SatisfactionDesc =
                Overall >= 75 ?
                    UiText(L"citizen_info.thoughts.satisfaction.very_high") :
                Overall >= 55 ?
                    UiText(L"citizen_info.thoughts.satisfaction.high") :
                Overall >= 35 ?
                    UiText(L"citizen_info.thoughts.satisfaction.low") :
                    UiText(L"citizen_info.thoughts.satisfaction.very_low");

            const wchar_t* DominantStance = nullptr;
            int MaxSupport = -1;
            auto CheckAxis =
                [&](EPoliticalAxis Axis, const FNpcPoliticalChoice& Profile)
            {
                const int Sup = static_cast<int>(Profile.Support);
                if (Sup > MaxSupport)
                {
                    MaxSupport = Sup;
                    DominantStance = GetPoliticalFactionDisplayName(
                        Axis, Profile.Stance);
                }
            };
            CheckAxis(EPoliticalAxis::Economy,
                PoliticalProfile.Economy);
            CheckAxis(EPoliticalAxis::ReligionMilitarism,
                PoliticalProfile.ReligionMilitarism);
            CheckAxis(EPoliticalAxis::EnvironmentIndustry,
                PoliticalProfile.EnvironmentIndustry);
            CheckAxis(EPoliticalAxis::IntellectualConservative,
                PoliticalProfile.IntellectualConservative);

            Result.BodyText = UIStrings::Format(
                L"citizen_info.body.citizen_thoughts",
                {
                    Result.Title,
                    GetCitizenWealthDisplayName(
                        IdentityProfile.WealthLevel),
                    GetCitizenEducationDisplayName(
                        IdentityProfile.EducationLevel),
                    IdentityProfile.IsImmigrant ?
                        Ui(L"citizen_info.fragment.immigrant_parenthetical") :
                        std::wstring(),
                    SatisfactionDesc,
                    DominantStance ? DominantStance : L"-"
                });
        }

        return Result;
    }

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex)
    {
        if (!QuerySource || CitizenName.empty())
            return FCitizenInfoSnapshot();

        FCitizenInfoCitizenRecord Citizen;

        if (!QuerySource->TryGetCitizenRecord(CitizenName, Citizen) ||
            !Citizen.Valid)
        {
            return FCitizenInfoSnapshot();
        }

        FCitizenInfoSnapshot Result = BuildCitizenSnapshot(
            Citizen.Name,
            Citizen.Satisfaction,
            Citizen.IdentityProfile,
            Citizen.PoliticalProfile,
            SelectedCitizenTabIndex);

        if (Result.Valid && Result.SelectedTabIndex == 0)
        {
            const bool IsTourist = Citizen.IdentityProfile.IsTourist;
            const std::wstring TouristProfileText =
                IsTourist &&
                    HasTouristPreference(
                        Citizen.IdentityProfile.TouristProfile) ?
                    GetTouristPreferenceDisplayName(
                        Citizen.IdentityProfile.TouristProfile) :
                    std::wstring(L"-");
            const std::wstring TouristVenueText =
                ResolveBuildingDisplayName(
                    QuerySource,
                    Citizen.FunVisitBuildingName.empty() ?
                        Citizen.FunBuildingName :
                        Citizen.FunVisitBuildingName);
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText =
                UIStrings::Get(
                    IsTourist ?
                        L"citizen_info.subtitle.tourist" :
                        L"citizen_info.subtitle.tropican");
            Result.ShowCitizenProfileOverview = true;
            Result.ShowCitizenActionButtons = true;
            Result.ShowSectionDivider = true;
            Result.CitizenPortraitSlotCount = 11;
            Result.CitizenPortraitOccupiedSlot = 4;
            Result.CitizenPortraitVariant =
                static_cast<int>(Citizen.Name.size() % 4);
            Result.CitizenFooterText = L"TROPICO EXEC. 16FA-923";

            Result.OverviewMetricLabels[0] =
                Ui(L"citizen_info.metric.activity");
            Result.OverviewMetricValues[0] =
                GetCitizenActivityDisplayName(Citizen.State);
            Result.OverviewMetricLabels[1] =
                Ui(L"citizen_info.metric.location");
            Result.OverviewMetricValues[1] =
                ResolveCitizenLocationText(QuerySource, Citizen);
            Result.OverviewMetricLabels[2] =
                Ui(
                    IsTourist ?
                        L"citizen_info.metric.tourist_profile" :
                        L"citizen_info.metric.age");
            Result.OverviewMetricValues[2] =
                IsTourist ?
                    TouristProfileText :
                    Ui(L"citizen_info.value.age_30_adult");
            Result.OverviewMetricLabels[3] =
                Ui(L"citizen_info.metric.origin");
            Result.OverviewMetricValues[3] =
                IsTourist ?
                    Ui(L"citizen_info.origin.tourist") :
                Citizen.IdentityProfile.IsImmigrant ?
                    Ui(L"citizen_info.origin.france") :
                    Ui(L"citizen_info.origin.tropico");
            Result.OverviewMetricLabels[4] =
                Ui(L"citizen_info.metric.wealth");
            Result.OverviewMetricValues[4] =
                GetCitizenProfileWealthDisplayName(
                    Citizen.IdentityProfile.WealthLevel);
            Result.OverviewMetricLabels[5] =
                Ui(L"citizen_info.metric.education");
            Result.OverviewMetricValues[5] =
                GetCitizenEducationDisplayName(
                    Citizen.IdentityProfile.EducationLevel);
            Result.OverviewMetricLabels[6] =
                Ui(
                    IsTourist ?
                        L"citizen_info.metric.visit" :
                        L"citizen_info.metric.job");
            Result.OverviewMetricValues[6] =
                IsTourist ?
                    TouristVenueText :
                    ResolveBuildingDisplayName(
                        QuerySource,
                        Citizen.WorkBuildingName);
            Result.OverviewMetricLabels[7] =
                Ui(L"citizen_info.metric.home");
            Result.OverviewMetricValues[7] =
                ResolveBuildingDisplayName(
                    QuerySource,
                    Citizen.HomeBuildingName);
            Result.OverviewMetricAccentValues[6] =
                IsTourist ?
                    !Citizen.FunBuildingName.empty() ||
                        !Citizen.FunVisitBuildingName.empty() :
                    !Citizen.WorkBuildingName.empty();
            Result.OverviewMetricAccentValues[7] =
                !Citizen.HomeBuildingName.empty();

            Result.CitizenActionLabels[0] =
                Ui(L"citizen_info.action.bribe");
            Result.CitizenActionLabels[1] =
                Ui(L"citizen_info.action.kill");
            Result.CitizenActionLabels[2] =
                Ui(L"citizen_info.action.stage_accident");
            Result.CitizenActionLabels[3] =
                Ui(L"citizen_info.action.arrest");
            Result.CitizenActionLabels[4] =
                Ui(L"citizen_info.action.isolate");
            Result.CitizenActionLabels[5] =
                Ui(L"citizen_info.action.send_to_space");
        }
        else if (Result.Valid && Result.SelectedTabIndex == 1)
        {
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText =
                UIStrings::Get(
                    Citizen.IdentityProfile.IsTourist ?
                        L"citizen_info.subtitle.tourist" :
                        L"citizen_info.subtitle.tropican");
            Result.ShowCitizenPoliticsOverview = true;
            Result.ShowCitizenProfileOverview = false;
            Result.ShowCitizenActionButtons = false;
            Result.ShowSectionDivider = false;
            Result.CitizenFooterText.clear();

            Result.CitizenPoliticsSatisfactionLabels =
            {{
                Ui(L"almanac.satisfaction.overall"),
                Ui(L"almanac.satisfaction.food"),
                Ui(L"almanac.satisfaction.health"),
                Ui(L"almanac.satisfaction.fun"),
                Ui(L"almanac.satisfaction.faith"),
                Ui(L"almanac.satisfaction.housing"),
                Ui(L"almanac.satisfaction.job"),
                Ui(L"almanac.satisfaction.freedom"),
                Ui(L"almanac.satisfaction.security")
            }};
            Result.CitizenPoliticsSatisfactionRatios =
            {{
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Overall / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Food / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Health / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Fun / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Faith / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Housing / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Job / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Freedom / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Security / 100.f))
            }};
            Result.CitizenPoliticsOpinionLines =
                BuildCitizenOpinionLines(Citizen.PoliticalProfile);
            Result.CitizenPoliticsSupportRatio =
                BuildCitizenSupportRatio(Citizen.Satisfaction);
        }
        else if (Result.Valid && Result.SelectedTabIndex == 2)
        {
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText =
                UIStrings::Get(
                    Citizen.IdentityProfile.IsTourist ?
                        L"citizen_info.subtitle.tourist" :
                        L"citizen_info.subtitle.tropican");
            Result.ShowCitizenThoughtsOverview = true;
            Result.ShowCitizenProfileOverview = false;
            Result.ShowCitizenPoliticsOverview = false;
            Result.ShowCitizenActionButtons = false;
            Result.ShowSectionDivider = false;
            Result.CitizenFooterText.clear();
            Result.CitizenThoughtLines =
            {{
                Ui(L"citizen_info.thought.line1"),
                Ui(L"citizen_info.thought.line2"),
                Ui(L"citizen_info.thought.line3"),
                Ui(L"citizen_info.thought.line4"),
                Ui(L"citizen_info.thought.line5")
            }};
        }

        return Result;
    }

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex)
    {
        return BuildTrackedCitizenSnapshot(
            CitizenInfoQueryService::CreateWorldQuerySource(World),
            CitizenName,
            SelectedCitizenTabIndex);
    }

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex,
        bool ShowCustomsModeSelection)
    {
        FBuildingUiSnapshot BuildingSnapshot;

        if (!CitizenInfoBuildingRuntime::BuildBuildingUiSnapshot(
            QuerySource,
            BuildingName,
            BuildingSnapshot))
        {
            return FCitizenInfoSnapshot();
        }

        FCitizenInfoSnapshot Result;
        Result.Valid = true;
        Result.Mode = EPanelMode::Building;
        Result.SelectedTabIndex =
            (std::max)(
                0,
                (std::min)(
                    GBuildingTabCount - 1,
                    SelectedBuildingTabIndex));
        Result.BudgetLevel = BuildingSnapshot.BudgetLevel;
        Result.Title = BuildingSnapshot.DisplayName.empty() ?
            BuildingSnapshot.ObjectName :
            BuildingSnapshot.DisplayName;
        const bool IsCustomsOffice =
            CitizenInfoBuildingRuntime::IsCustomsOfficeBuilding(
                BuildingSnapshot);
        const bool ShowCustomsModePage =
            IsCustomsOffice &&
            Result.SelectedTabIndex == 0 &&
            ShowCustomsModeSelection;

        if (BuildingSnapshot.CatalogEntry)
        {
            Result.Subtitle =
                std::wstring(GetBuildingEraDisplayName(
                    BuildingSnapshot.CatalogEntry->UnlockEra)) +
                L"  |  " +
                BuildingSnapshot.CategoryName;
        }
        else
        {
            Result.Subtitle = BuildingSnapshot.CategoryName;
        }

        Result.PageTitle =
            CitizenInfoConstants::GetBuildingPageTitle(
                Result.SelectedTabIndex);
        Result.ShowTabButtons = true;
        Result.ShowSectionRibbon = Result.SelectedTabIndex != 0;
        Result.ShowBudgetControls = Result.SelectedTabIndex == 0;
        Result.ShowActionButtons =
            Result.SelectedTabIndex == 0 &&
            !ShowCustomsModePage;
        Result.ShowDemolishButton = Result.ShowActionButtons;
        Result.ShowMoveButton =
            Result.ShowActionButtons &&
            !IsCustomsOffice;
        Result.ShowCloneButton =
            Result.ShowActionButtons &&
            !IsCustomsOffice;
        Result.ShowBuildingOverview =
            Result.SelectedTabIndex == 0 &&
            BuildingSnapshot.Residential;
        Result.ShowBuildingWorkOverview =
            Result.SelectedTabIndex == 0 &&
            !ShowCustomsModePage &&
            (IsCustomsOffice ||
                UseGenericBuildingWorkOverview(BuildingSnapshot));
        Result.ShowBuildingMetricRows =
            Result.SelectedTabIndex == 1 &&
            BuildingSnapshot.Residential;
        if (IsCustomsOffice &&
            (Result.SelectedTabIndex == 1 ||
                Result.SelectedTabIndex == 3))
        {
            Result.ShowBuildingMetricRows = true;
        }
        Result.ShowBuildingUpgradeCard = false;
        Result.ShowBuildingInformationParagraphs = false;
        Result.ShowSectionRibbon =
            Result.SelectedTabIndex != 0 &&
            !Result.ShowBuildingInformationParagraphs;
        if (ShowCustomsModePage)
            Result.ShowSectionRibbon = false;
        const bool ShowHydroponicCommand =
            Result.SelectedTabIndex == 0 &&
            CitizenInfoBuildingRuntime::IsHydroponicFarmBuilding(
                BuildingSnapshot);
        const bool ShowOperationModeCommand =
            Result.SelectedTabIndex == 0 &&
            !BuildingSnapshot.Harbor &&
            !IsCustomsOffice &&
            !BuildingSnapshot.OperationModes.empty();
        const bool ShowWarehousePolicyCommand =
            Result.SelectedTabIndex == 0 &&
            BuildingSnapshot.Warehouse &&
            BuildingSnapshot.OperationModes.empty();
        const bool ShowRuntimeUpgradeCommand =
            Result.SelectedTabIndex == 2 &&
            !BuildingSnapshot.Harbor &&
            !IsCustomsOffice &&
            BuildingSnapshot.CatalogEntry &&
            !BuildingSnapshot.CatalogEntry->RuntimeUpgradeDefs.empty();
        const bool ShowWarehousePriorityCommand =
            Result.SelectedTabIndex == 4 &&
            BuildingSnapshot.Warehouse;
        const bool ShowHarborImportCommand =
            Result.SelectedTabIndex == 0 &&
            BuildingSnapshot.Harbor;
        const bool ShowHarborReserveCommand =
            Result.SelectedTabIndex == 1 &&
            BuildingSnapshot.Harbor;
        const bool ShowHarborImportCapCommand =
            Result.SelectedTabIndex == 2 &&
            BuildingSnapshot.Harbor;
        const bool ShowHarborImportBudgetCommand =
            Result.SelectedTabIndex == 3 &&
            BuildingSnapshot.Harbor;
        const bool ShowHarborExportCommand =
            Result.SelectedTabIndex == 4 &&
            BuildingSnapshot.Harbor;
        const bool ShowCustomsTradeCommand =
            IsCustomsOffice &&
            Result.SelectedTabIndex == 0 &&
            !ShowCustomsModePage;
        const bool ShowCustomsBackCommand =
            ShowCustomsModePage;
        Result.ShowOverviewCommandButton =
            ShowCustomsTradeCommand ||
            ShowCustomsBackCommand ||
            ShowOperationModeCommand ||
            ShowWarehousePolicyCommand ||
            ShowRuntimeUpgradeCommand ||
            ShowWarehousePriorityCommand ||
            ShowHydroponicCommand ||
            ShowHarborImportCommand ||
            ShowHarborReserveCommand ||
            ShowHarborImportCapCommand ||
            ShowHarborImportBudgetCommand ||
            ShowHarborExportCommand;
        Result.OverviewCommandButtonText.clear();
        Result.ShowBudgetText = !ShowCustomsModePage;

        for (size_t Index = 0; Index < Result.BudgetButtonLabels.size(); ++Index)
        {
            Result.BudgetButtonLabels[Index] =
                std::to_wstring(static_cast<int>(Index) + 1);
            Result.BudgetButtonEnabled[Index] = Result.ShowBudgetControls;
        }

        if (ShowCustomsTradeCommand)
        {
            Result.OverviewCommandButtonText = L"무역 화면 열기";
        }
        else if (ShowCustomsBackCommand)
        {
            Result.OverviewCommandButtonText = L"뒤로";
        }
        else if (ShowOperationModeCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.operation_mode_cycle") +
                L": " +
                (BuildingSnapshot.ActiveOperationModeText.empty() ?
                    L"-" :
                    BuildingSnapshot.ActiveOperationModeText);
        }
        else if (ShowWarehousePolicyCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.warehouse_policy_cycle") +
                L": " +
                (BuildingSnapshot.WarehousePolicySelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.WarehousePolicySelectionText);
        }
        else if (ShowRuntimeUpgradeCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.runtime_upgrade_cycle") +
                L": " +
                (BuildingSnapshot.ActiveRuntimeUpgradeText.empty() ?
                    L"-" :
                    BuildingSnapshot.ActiveRuntimeUpgradeText);
        }
        else if (ShowWarehousePriorityCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.warehouse_priority_cycle") +
                L": " +
                (BuildingSnapshot.WarehousePrioritySelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.WarehousePrioritySelectionText);
        }
        else if (ShowHarborImportCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.auto_import_cycle") +
                L": " +
                (BuildingSnapshot.HarborImportSelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.HarborImportSelectionText);
        }
        else if (ShowHarborReserveCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.domestic_reserve_cycle") +
                L": " +
                (BuildingSnapshot.HarborDomesticReserveSelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.HarborDomesticReserveSelectionText);
        }
        else if (ShowHarborImportCapCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.import_cap_cycle") +
                L": " +
                (BuildingSnapshot.HarborImportCapSelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.HarborImportCapSelectionText);
        }
        else if (ShowHarborImportBudgetCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.import_budget_cycle") +
                L": " +
                (BuildingSnapshot.HarborImportBudgetSelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.HarborImportBudgetSelectionText);
        }
        else if (ShowHarborExportCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.export_block_cycle") +
                L": " +
                (BuildingSnapshot.HarborExportSelectionText.empty() ?
                    L"-" :
                    BuildingSnapshot.HarborExportSelectionText);
        }
        else if (ShowHydroponicCommand)
        {
            Result.OverviewCommandButtonText =
                Ui(L"citizen_info.action.change_resource");
        }

        if (ShowCustomsModePage)
        {
            Result.BudgetLevel =
                (std::max)(1, BuildingSnapshot.ActiveOperationModeIndex + 1);

            for (size_t Index = 0;
                Index < Result.BudgetButtonLabels.size();
                ++Index)
            {
                if (Index < BuildingSnapshot.OperationModes.size())
                {
                    Result.BudgetButtonLabels[Index] =
                        BuildingSnapshot.OperationModes[Index];
                    Result.BudgetButtonEnabled[Index] = true;
                }
                else
                {
                    Result.BudgetButtonLabels[Index].clear();
                    Result.BudgetButtonEnabled[Index] = false;
                }
            }
        }

        const long long TotalMonthlyCost =
            static_cast<long long>(BuildingSnapshot.MonthlyWageCost) +
            static_cast<long long>(BuildingSnapshot.MonthlyUpkeepCost);
        Result.BudgetText = UIStrings::Format(
            L"citizen_info.budget.summary_template",
            {
                std::to_wstring(BuildingSnapshot.BudgetLevel),
                FormatMultiplier(BuildingSnapshot.BudgetScale),
                FormatMoney(TotalMonthlyCost)
            });

        if (BuildingSnapshot.CatalogEntry)
        {
            Result.TitleIconPath = GetCatalogEntryIconPath(
                *BuildingSnapshot.CatalogEntry);

            if (Result.TitleIconPath)
            {
                Result.ShowTitleIcon = true;
                Result.TitleIconTextureKey =
                    BuildCatalogIconTextureKey(
                        *BuildingSnapshot.CatalogEntry);
            }
        }

        if (Result.ShowBuildingOverview)
        {
            const int PowerSurplusMW =
                BuildingSnapshot.TotalProducedPowerMW -
                BuildingSnapshot.TotalRequiredPowerMW;
            const int HousingQuality =
                ResolveOverviewHousingQuality(BuildingSnapshot);
            const long long MonthlyIncome =
                ResolveOverviewMonthlyIncome(BuildingSnapshot);
            const int RequiredPowerMW =
                ResolveOverviewRequiredPower(BuildingSnapshot);

            Result.OverviewBudgetLabel =
                Ui(L"citizen_info.label.budget");
            Result.OverviewBudgetValue =
                FormatMoney(BuildingSnapshot.MonthlyUpkeepCost);
            Result.OverviewOccupancyLabel =
                Ui(L"citizen_info.label.residence");
            Result.OverviewOccupancyValue =
                std::to_wstring(BuildingSnapshot.Residents.size()) +
                L" / " +
                std::to_wstring((std::max)(0, BuildingSnapshot.Capacity));
            Result.OverviewResidentCount =
                static_cast<int>(BuildingSnapshot.Residents.size());
            Result.OverviewResidentCapacity =
                (std::max)(0, BuildingSnapshot.Capacity);
            Result.OverviewMetricLabels =
            {
                Ui(L"citizen_info.label.housing_quality"),
                Ui(L"citizen_info.label.required_wealth"),
                Ui(L"citizen_info.label.monthly_income"),
                Ui(L"citizen_info.label.electricity"),
                Ui(L"citizen_info.label.power_network"),
                Ui(L"citizen_info.label.power_grid_status")
            };
            Result.OverviewMetricValues =
            {
                std::to_wstring(HousingQuality),
                NormalizeWealthRequirementText(
                    BuildingSnapshot.WealthRequirementText),
                FormatMoney(MonthlyIncome),
                FormatMegawattValue(-RequiredPowerMW),
                L"#1",
                FormatSignedMegawattValue(PowerSurplusMW)
            };
        }

        if (Result.ShowBuildingWorkOverview)
        {
            if (IsCustomsOffice)
            {
                const int WorkerCapacity =
                    (std::max)(1, BuildingSnapshot.Capacity);
                const int WorkerCount = (std::min)(
                    WorkerCapacity,
                    static_cast<int>(BuildingSnapshot.AssignedEmployees.size()));
                Result.OverviewWorkModeLabel =
                    Ui(L"citizen_info.label.work_mode");
                Result.OverviewWorkModeValue =
                    !BuildingSnapshot.ActiveOperationModeText.empty() ?
                        BuildingSnapshot.ActiveOperationModeText :
                        L"수입세 감소";
                Result.OverviewBudgetLabel =
                    Ui(L"citizen_info.label.budget");
                Result.OverviewBudgetValue =
                    BuildingSnapshot.MonthlyUpkeepCost > 0 ?
                        FormatMoney(BuildingSnapshot.MonthlyUpkeepCost) :
                        L"$118";
                Result.OverviewOccupancyLabel =
                    Ui(L"citizen_info.label.workers");
                Result.OverviewOccupancyValue =
                    std::to_wstring(WorkerCount) +
                    L" / " +
                    std::to_wstring(WorkerCapacity);
                Result.OverviewResidentCount = WorkerCount;
                Result.OverviewResidentCapacity = WorkerCapacity;
                Result.OverviewMetricLabels =
                {
                    Ui(L"citizen_info.label.job_quality"),
                    Ui(L"citizen_info.label.required_education"),
                    Ui(L"citizen_info.label.wage"),
                    Ui(L"citizen_info.label.efficiency")
                };
                Result.OverviewMetricValues =
                {
                    !BuildingSnapshot.JobQualityText.empty() ?
                        BuildingSnapshot.JobQualityText :
                        L"70",
                    GetCitizenEducationDisplayName(
                        BuildingSnapshot.RequiredEducationLevel),
                    CitizenInfoBuildingRuntime::ResolveCustomsPerWorkerWage(
                        BuildingSnapshot),
                    std::to_wstring(
                    CitizenInfoBuildingRuntime::ResolveCustomsEfficiencyPercent(
                        BuildingSnapshot)) +
                        L"%"
                };
            }
            else
            {
                PopulateGenericWorkOverview(BuildingSnapshot, Result);
            }
        }

        if (Result.ShowBuildingMetricRows && Result.SelectedTabIndex == 1)
        {
            if (IsCustomsOffice)
            {
                const long long BuildingExpense =
                    static_cast<long long>(
                        (std::max)(
                            118,
                            BuildingSnapshot.MonthlyUpkeepCost));
                Result.OverviewMetricLabels =
                {
                    L"수입 (전체)",
                    L"수입 (건물)",
                    L"수출량",
                    L"수입량",
                    L"전체 수출량 (무역로)",
                    L"관광객 도착"
                };
                Result.OverviewMetricValues =
                {
                    FormatMoneyDollarFirst(
                        -BuildingSnapshot.LastDailyImportExpense),
                    FormatMoneyDollarFirst(-BuildingExpense),
                    FormatInteger(
                        BuildingSnapshot.TradeRouteExportFulfilledUnits),
                    FormatInteger(
                        BuildingSnapshot.TradeRouteImportFulfilledUnits),
                    FormatInteger(
                        BuildingSnapshot.TradeRouteExportContractUnits),
                    FormatInteger(BuildingSnapshot.TourismArrivalCount)
                };
            }
            else
            {
                const long long IncomeTotal =
                    ResolveOverviewMonthlyIncome(BuildingSnapshot) * 12LL;
                const long long IncomePrevious =
                    ResolveOverviewMonthlyIncome(BuildingSnapshot) -
                    static_cast<long long>(BuildingSnapshot.MonthlyUpkeepCost);
                Result.OverviewMetricLabels =
                {
                    Ui(L"citizen_info.label.total_income"),
                    Ui(L"citizen_info.label.last_month_income"),
                    L"",
                    L"",
                    L"",
                    L""
                };
                Result.OverviewMetricValues =
                {
                    FormatMoney(IncomeTotal),
                    FormatMoneyDollarFirst(IncomePrevious),
                    L"",
                    L"",
                    L"",
                    L""
                };
            }
        }

        if (Result.ShowBuildingMetricRows && Result.SelectedTabIndex == 3)
        {
            if (IsCustomsOffice)
            {
                const int DiplomacyModifier =
                    CitizenInfoBuildingRuntime::
                        ComputeAverageCustomsDiplomacyExportBiasPercent();
                const int BudgetModifier =
                    CitizenInfoBuildingRuntime::
                        ResolveCustomsBudgetModifierPercent(BuildingSnapshot);
                Result.ShowHeaderNote = true;
                Result.ShowSectionDivider = true;
                Result.HeaderNoteText =
                    L"수출 가격 보너스는 효율에 따라 변합니다.";
                Result.OverviewMetricLabels =
                {
                    Ui(L"citizen_info.label.efficiency"),
                    L"경제부 장관",
                    L"예산 수정치"
                };
                Result.OverviewMetricValues =
                {
                    std::to_wstring(
                        CitizenInfoBuildingRuntime::
                            ResolveCustomsEfficiencyPercent(
                                BuildingSnapshot)) +
                        L"%",
                    std::wstring(
                        DiplomacyModifier > 0 ? L"+" : L"") +
                        std::to_wstring(DiplomacyModifier) +
                        L"%",
                    std::wstring(
                        BudgetModifier > 0 ? L"+" : L"") +
                        std::to_wstring(BudgetModifier) +
                        L"%"
                };
                Result.OverviewMetricAccentValues[1] = true;
                Result.OverviewMetricAccentValues[2] = true;
            }
            else
            {
                Result.ShowHeaderNote = true;
                Result.ShowSectionDivider = true;
                Result.HeaderNoteText =
                    Ui(L"citizen_info.note.housing_quality_efficiency");
                Result.OverviewMetricLabels =
                {
                    Ui(L"citizen_info.label.efficiency"),
                    L"",
                    L"",
                    L"",
                    L"",
                    L""
                };
                Result.OverviewMetricValues =
                {
                    L"100%",
                    L"",
                    L"",
                    L"",
                    L"",
                    L""
                };
            }
        }

        switch (Result.SelectedTabIndex)
        {
        case 1:
            Result.BodyText = Result.ShowBuildingMetricRows ?
                std::wstring() :
                BuildStatisticsBody(BuildingSnapshot);
            break;
        case 2:
            Result.BodyText = IsCustomsOffice ?
                BuildCustomsUpgradesBody(BuildingSnapshot) :
                BuildUpgradesBody(BuildingSnapshot);
            break;
        case 3:
            Result.BodyText = Result.ShowBuildingMetricRows ?
                std::wstring() :
                BuildEfficiencyBody(BuildingSnapshot);
            break;
        case 4:
            Result.BodyText = BuildInformationBody(BuildingSnapshot);
            break;
        case 0:
        default:
            Result.BodyText =
                ShowCustomsModePage ?
                    BuildCustomsModeSelectionBody(BuildingSnapshot) :
                Result.ShowBuildingOverview ?
                std::wstring() :
                BuildOverviewBody(BuildingSnapshot);
            break;
        }

        return Result;
    }

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex,
        bool ShowCustomsModeSelection)
    {
        return BuildTrackedBuildingSnapshot(
            CitizenInfoQueryService::CreateWorldQuerySource(World),
            BuildingName,
            SelectedBuildingTabIndex,
            ShowCustomsModeSelection);
    }
}
