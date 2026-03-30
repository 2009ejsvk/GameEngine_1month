#include "CitizenInfoPresentation.h"
#include "CitizenInfoPresentationInternal.h"
#include "CitizenInfoConstants.h"
#include "UIEnumLabels.h"
#include "UIStrings.h"
#include "../Building/BuildingCatalog.h"
#include "../GameConstants.h"
#include "../Economy/ResourceTradePricing.h"
#include "../StringUtils.h"
#include "Vector4.h"
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <cwctype>
#include <vector>

namespace CitizenInfoPresentation
{
    std::wstring FormatFlowRateValue(float Value, const wchar_t* SuffixKey);
    std::wstring FormatFlowVolumeValue(int Value, const wchar_t* SuffixKey);
}

namespace CitizenInfoPresentationInternal
{
    std::wstring JoinInlineSegments(
        const std::vector<std::wstring>& Segments);

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

    std::wstring FormatPowerCoverageValue(float Ratio)
    {
        const float ClampedRatio = (std::max)(0.f, (std::min)(1.f, Ratio));
        return std::to_wstring(static_cast<int>(roundf(
            ClampedRatio * 100.f))) + L"%";
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

    void AppendScrollableOverviewMetricLines(
        FOverviewMetricWriter& Writer,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result,
        const std::vector<std::wstring>& Lines,
        int RequestedOffset)
    {
        const int FirstRowIndex = Writer.WriteIndex;
        const int VisibleLineCount = (std::max)(
            0,
            static_cast<int>(Result.OverviewMetricLabels.size()) -
                FirstRowIndex);
        const int TotalLineCount = static_cast<int>(Lines.size());
        const int MaxOffset = (std::max)(0, TotalLineCount - VisibleLineCount);
        const int ClampedOffset = (std::max)(
            0,
            (std::min)(RequestedOffset, MaxOffset));

        Result.ShowOverviewMetricScroll =
            TotalLineCount > VisibleLineCount &&
            VisibleLineCount > 0;
        Result.OverviewMetricScrollOffset = ClampedOffset;
        Result.OverviewMetricScrollVisibleLineCount = VisibleLineCount;
        Result.OverviewMetricScrollTotalLineCount = TotalLineCount;
        Result.OverviewMetricScrollFirstRowIndex = FirstRowIndex;

        const int VisibleEnd = (std::min)(
            TotalLineCount,
            ClampedOffset + VisibleLineCount);

        for (int Index = ClampedOffset; Index < VisibleEnd; ++Index)
        {
            Writer.Add(
                L"  " + Lines[static_cast<size_t>(Index)],
                std::wstring());
        }
    }

    std::wstring ResolveOverviewBudgetValue(const FBuildingUiSnapshot& Snapshot)
    {
        return CitizenInfoPresentation::FormatMoney(Snapshot.MonthlyUpkeepCost);
    }

    bool UsesProductionEfficiencyMetric(const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.CanGenerateWorkOutput ||
            Snapshot.ProducedResourceType != EResourceType::None ||
            Snapshot.ChainStage != EProductionChainStage::None ||
            !Snapshot.ProductionChainStageText.empty())
        {
            return true;
        }

        for (size_t Index = 0; Index < Snapshot.ProductionInputs.size(); ++Index)
        {
            if (Snapshot.ProductionInputs[Index].Type != EResourceType::None &&
                Snapshot.ProductionInputs[Index].RequiredAmount > 0)
            {
                return true;
            }
        }

        return false;
    }

    float ResolveOperationalEfficiencyRatio(const FBuildingUiSnapshot& Snapshot)
    {
        return (std::max)(
            0.f,
            Snapshot.BudgetScale *
                Snapshot.DamageEfficiencyMultiplier *
                (Snapshot.RequiredPowerMW > 0 ?
                    Snapshot.PowerSupplyRatio :
                    1.f));
    }

    float ResolveProductionCoverageEfficiencyRatio(
        const FBuildingUiSnapshot& Snapshot)
    {
        return (std::max)(0.f, Snapshot.LastProductionEfficiency);
    }

    float ResolveDisplayedEfficiencyRatio(const FBuildingUiSnapshot& Snapshot)
    {
        if (!UsesProductionEfficiencyMetric(Snapshot))
            return ResolveOperationalEfficiencyRatio(Snapshot);

        return ResolveProductionCoverageEfficiencyRatio(Snapshot);
    }

    std::wstring ResolveWorkerOverviewEfficiency(
        const FBuildingUiSnapshot& Snapshot)
    {
        const int EfficiencyPercent = static_cast<int>(roundf(
            ResolveDisplayedEfficiencyRatio(Snapshot) *
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

    int ResolveDisplayRequiredPowerMW(const FBuildingUiSnapshot& Snapshot)
    {
        const int RuntimeRequiredPowerMW =
            (std::max)(0, Snapshot.RequiredPowerMW);

        if (RuntimeRequiredPowerMW > 0)
            return RuntimeRequiredPowerMW;

        return Snapshot.CatalogEntry ?
            (std::max)(0, Snapshot.CatalogEntry->BaseRequiredPowerMW) :
            0;
    }

    int ResolveDisplayProducedPowerMW(const FBuildingUiSnapshot& Snapshot)
    {
        const int RuntimeProducedPowerMW =
            (std::max)(0, Snapshot.ProducedPowerMW);

        if (RuntimeProducedPowerMW > 0)
            return RuntimeProducedPowerMW;

        return Snapshot.CatalogEntry ?
            (std::max)(0, Snapshot.CatalogEntry->BaseProducedPowerMW) :
            0;
    }

    std::wstring ResolveRequiredPowerDisplayText(
        const FBuildingUiSnapshot& Snapshot)
    {
        const int RequiredPowerMW = ResolveDisplayRequiredPowerMW(Snapshot);
        return RequiredPowerMW > 0 ?
            CitizenInfoPresentation::FormatMegawattValue(RequiredPowerMW) :
            std::wstring();
    }

    std::wstring ResolveProducedPowerDisplayText(
        const FBuildingUiSnapshot& Snapshot)
    {
        const int ProducedPowerMW = ResolveDisplayProducedPowerMW(Snapshot);
        return ProducedPowerMW > 0 ?
            CitizenInfoPresentation::FormatMegawattValue(ProducedPowerMW) :
            std::wstring();
    }

    std::wstring ResolveWorkerOverviewPowerValue(
        const FBuildingUiSnapshot& Snapshot)
    {
        const int ActualRequiredPowerMW = (std::max)(0, Snapshot.RequiredPowerMW);

        if (ActualRequiredPowerMW > 0)
        {
            std::wstring Value =
                CitizenInfoPresentation::FormatMegawattValue(
                    -ActualRequiredPowerMW);
            Value += L" (";
            Value += FormatPowerCoverageValue(Snapshot.PowerSupplyRatio);
            Value += L")";
            return Value;
        }

        const int DisplayRequiredPowerMW = ResolveDisplayRequiredPowerMW(Snapshot);

        if (DisplayRequiredPowerMW > 0)
        {
            return L"-" +
                CitizenInfoPresentation::FormatMegawattValue(
                    DisplayRequiredPowerMW);
        }

        const int ProducedPowerMW = ResolveDisplayProducedPowerMW(Snapshot);

        if (ProducedPowerMW > 0)
        {
            return L"+" +
                CitizenInfoPresentation::FormatMegawattValue(ProducedPowerMW);
        }

        return std::wstring();
    }

    bool IsValidProductionInputRecord(const FProductionInputSlotView& Input)
    {
        return Input.Type != EResourceType::None && Input.RequiredAmount > 0;
    }

    bool HasProductionInputRecords(const FBuildingUiSnapshot& Snapshot)
    {
        for (size_t Index = 0; Index < Snapshot.ProductionInputs.size(); ++Index)
        {
            if (IsValidProductionInputRecord(Snapshot.ProductionInputs[Index]))
                return true;
        }

        return false;
    }

    bool HasProductionInputShortage(const FBuildingUiSnapshot& Snapshot)
    {
        for (size_t Index = 0; Index < Snapshot.ProductionInputs.size(); ++Index)
        {
            const FProductionInputSlotView& Input =
                Snapshot.ProductionInputs[Index];

            if (!IsValidProductionInputRecord(Input))
                continue;

            if (Input.CurrentStock <= 0)
                return true;
        }

        return false;
    }

    bool HasLowProductionInputStock(const FBuildingUiSnapshot& Snapshot)
    {
        for (size_t Index = 0; Index < Snapshot.ProductionInputs.size(); ++Index)
        {
            const FProductionInputSlotView& Input =
                Snapshot.ProductionInputs[Index];

            if (!IsValidProductionInputRecord(Input))
                continue;

            if (Input.CurrentStock < (std::max)(1, Input.RequiredAmount))
            {
                return true;
            }
        }

        return false;
    }

    std::wstring BuildProductionInputRequirementText(
        const FProductionInputSlotView& Input)
    {
        if (!IsValidProductionInputRecord(Input))
            return std::wstring();

        return std::wstring(GetResourceTypeDisplayName(Input.Type)) +
            L" x " +
            std::to_wstring((std::max)(1, Input.RequiredAmount));
    }

    std::wstring BuildProductionInputStockText(
        const FProductionInputSlotView& Input)
    {
        if (!IsValidProductionInputRecord(Input))
            return std::wstring();

        return CitizenInfoPresentation::FormatInteger(Input.CurrentStock);
    }

    std::wstring BuildProductionInputMaxStockText(
        const FProductionInputSlotView& Input)
    {
        if (!IsValidProductionInputRecord(Input) || Input.MaxStock <= 0)
            return std::wstring();

        return CitizenInfoPresentation::FormatInteger(Input.MaxStock);
    }

    std::wstring BuildProductionInputRequirementSummary(
        const FBuildingUiSnapshot& Snapshot)
    {
        std::vector<std::wstring> Segments;

        for (size_t Index = 0; Index < Snapshot.ProductionInputs.size(); ++Index)
        {
            const std::wstring Segment =
                BuildProductionInputRequirementText(
                    Snapshot.ProductionInputs[Index]);

            if (!Segment.empty())
                Segments.push_back(Segment);
        }

        return JoinInlineSegments(Segments);
    }

    std::wstring BuildProductionInputConsumptionText(
        const FProductionInputSlotView& Input)
    {
        if (!IsValidProductionInputRecord(Input))
            return std::wstring();

        return CitizenInfoPresentation::FormatFlowRateValue(
                Input.ConsumptionUnitsPerSecond,
                L"citizen_info.unit.per_second_suffix") +
            L", " +
            CitizenInfoPresentation::FormatFlowVolumeValue(
                Input.EstimatedDailyConsumptionUnits,
                L"citizen_info.unit.per_day_suffix") +
            L", " +
            CitizenInfoPresentation::FormatFlowVolumeValue(
                Input.EstimatedMonthlyConsumptionUnits,
                L"citizen_info.unit.per_month_suffix");
    }

    std::wstring BuildProductionInputConsumptionSummary(
        const FBuildingUiSnapshot& Snapshot)
    {
        std::vector<std::wstring> Segments;
        int HiddenCount = 0;

        for (size_t Index = 0; Index < Snapshot.ProductionInputs.size(); ++Index)
        {
            const FProductionInputSlotView& Input =
                Snapshot.ProductionInputs[Index];

            if (!IsValidProductionInputRecord(Input))
                continue;

            const std::wstring Segment =
                std::wstring(GetResourceTypeDisplayName(Input.Type)) +
                L" " +
                CitizenInfoPresentation::FormatFlowRateValue(
                    Input.ConsumptionUnitsPerSecond,
                    L"citizen_info.unit.per_second_suffix");

            if (Segments.size() < 2)
                Segments.push_back(Segment);
            else
                ++HiddenCount;
        }

        std::wstring Summary = JoinInlineSegments(Segments);

        if (HiddenCount > 0)
        {
            Summary += L" +";
            Summary += std::to_wstring(HiddenCount);
        }

        return Summary;
    }

    std::wstring BuildProductionInputStatisticsLabel(int SlotIndex)
    {
        return Ui(L"citizen_info.label.input_slot") +
            L" " +
            std::to_wstring((std::max)(1, SlotIndex + 1));
    }

    std::wstring BuildProductionInputStatisticsValue(
        const FProductionInputSlotView& Input)
    {
        if (!IsValidProductionInputRecord(Input))
            return std::wstring();

        const std::wstring MaxStockStr = BuildProductionInputMaxStockText(Input);
        return L"[IN] " +
            std::wstring(GetResourceTypeDisplayName(Input.Type)) +
            L": " +
            BuildProductionInputStockText(Input) +
            (MaxStockStr.empty() ? std::wstring() : L" / " + MaxStockStr);
    }

    std::wstring BuildProductionInputOverviewValue(
        const FProductionInputSlotView& Input)
    {
        if (!IsValidProductionInputRecord(Input))
            return std::wstring();

        const std::wstring MaxStockStr = BuildProductionInputMaxStockText(Input);
        if (MaxStockStr.empty())
            return BuildProductionInputStockText(Input);
        return BuildProductionInputStockText(Input) + L" / " + MaxStockStr;
    }

    int ExtractIntegerAfterToken(
        const std::wstring& Text,
        const wchar_t* Token)
    {
        if (!Token)
            return 0;

        const size_t TokenPos = Text.find(Token);

        if (TokenPos == std::wstring::npos)
            return 0;

        const std::wstring Segment =
            Text.substr(TokenPos + wcslen(Token));
        bool Negative = false;
        bool FoundDigit = false;
        int Value = 0;

        for (size_t Index = 0; Index < Segment.size(); ++Index)
        {
            const wchar_t Ch = Segment[Index];

            if (!FoundDigit && Ch == L'-')
            {
                Negative = true;
                continue;
            }

            if (Ch == L',' && FoundDigit)
                continue;

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
            return 0;

        return Negative ? -Value : Value;
    }

    std::wstring BuildTeamsterDeliverySummary(
        const FBuildingUiSnapshot& Snapshot)
    {
        int TotalShortage = 0;
        int TotalPickupWaiting = 0;
        bool HasRelevantLine = false;

        for (size_t Index = 0; Index < Snapshot.LogisticsLines.size(); ++Index)
        {
            const std::wstring& Line = Snapshot.LogisticsLines[Index];

            if (StartsWith(Line, L"투입 보급:") ||
                StartsWith(Line, L"소비 보급:"))
            {
                HasRelevantLine = true;
                TotalShortage += (std::max)(
                    0,
                    ExtractIntegerAfterToken(Line, L"부족 "));
            }

            if (StartsWith(Line, L"생산 대기:") ||
                StartsWith(Line, L"선적 대기:"))
            {
                HasRelevantLine = true;
                TotalPickupWaiting += (std::max)(
                    0,
                    ExtractIntegerAfterToken(Line, L"사용 가능 "));
            }
        }

        if (!HasRelevantLine)
            return std::wstring();

        if (TotalShortage <= 0 && TotalPickupWaiting <= 0)
            return Ui(L"citizen_info.value.stable");

        std::wstring Summary;

        if (TotalShortage > 0)
        {
            Summary += Ui(L"citizen_info.fragment.shortage");
            Summary += L" ";
            Summary +=
                CitizenInfoPresentation::FormatInteger(TotalShortage);
        }

        if (TotalPickupWaiting > 0)
        {
            if (!Summary.empty())
                Summary += L" / ";

            Summary += Ui(L"citizen_info.fragment.pickup_waiting");
            Summary += L" ";
            Summary +=
                CitizenInfoPresentation::FormatInteger(TotalPickupWaiting);
        }

        return Summary;
    }

    bool HasProductionFlowEstimate(const FBuildingUiSnapshot& Snapshot)
    {
        return Snapshot.CanGenerateWorkOutput ||
            Snapshot.ProducedResourceType != EResourceType::None;
    }

    std::wstring ResolveProductionInputStatus(const FBuildingUiSnapshot& Snapshot)
    {
        if (!HasProductionInputRecords(Snapshot))
            return std::wstring();

        if (HasProductionInputShortage(Snapshot))
        {
            const bool HasSufficientPower =
                Snapshot.RequiredPowerMW <= 0 ||
                Snapshot.PowerSupplyRatio >= 0.95f;

            if (!Snapshot.AssignedEmployees.empty() &&
                HasSufficientPower &&
                ResolveDisplayedEfficiencyRatio(Snapshot) <= 0.001f)
            {
                return Ui(L"citizen_info.input_status.stopped");
            }

            return Ui(L"citizen_info.input_status.low");
        }

        if (HasLowProductionInputStock(Snapshot))
            return Ui(L"citizen_info.input_status.low");

        return Ui(L"citizen_info.input_status.ready");
    }

    void AppendProductionInputLines(
        std::wstring& Body,
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!HasProductionInputRecords(Snapshot))
            return;

        AppendLine(Body, Ui(L"citizen_info.section.production_inputs"));
        const std::wstring RequiredInputs =
            BuildProductionInputRequirementSummary(Snapshot);

        if (!RequiredInputs.empty())
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.required_inputs") +
                    L": " +
                    RequiredInputs);
        }

        for (size_t Index = 0; Index < Snapshot.ProductionInputs.size(); ++Index)
        {
            const FProductionInputSlotView& Input =
                Snapshot.ProductionInputs[Index];

            if (!IsValidProductionInputRecord(Input))
                continue;

            const std::wstring MaxStockStr =
                BuildProductionInputMaxStockText(Input);
            AppendLine(
                Body,
                L"- " +
                    BuildProductionInputRequirementText(Input) +
                    L"  |  " +
                    Ui(L"citizen_info.label.current_stock") +
                    L" " +
                    BuildProductionInputStockText(Input) +
                    (MaxStockStr.empty() ? std::wstring() :
                        L" / " + MaxStockStr) +
                    L"  |  " +
                    Ui(L"citizen_info.label.input_consumption") +
                    L" " +
                    BuildProductionInputConsumptionText(Input));
        }
    }

    void AppendProductionFlowLines(
        std::wstring& Body,
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!HasProductionFlowEstimate(Snapshot))
            return;

        AppendLine(
            Body,
            Ui(L"citizen_info.label.output_per_second") +
                L": " +
                CitizenInfoPresentation::FormatFlowRateValue(
                    Snapshot.CurrentProductionUnitsPerSecond,
                    L"citizen_info.unit.per_second_suffix"));
        AppendLine(
            Body,
            Ui(L"citizen_info.label.expected_daily_output") +
                L": " +
                CitizenInfoPresentation::FormatFlowVolumeValue(
                    Snapshot.EstimatedDailyProductionUnits,
                    L"citizen_info.unit.per_day_suffix"));
        AppendLine(
            Body,
            Ui(L"citizen_info.label.expected_monthly_output") +
                L": " +
                CitizenInfoPresentation::FormatFlowVolumeValue(
                    Snapshot.EstimatedMonthlyProductionUnits,
                    L"citizen_info.unit.per_month_suffix"));
    }

    std::wstring ResolveWorkerOverviewStorageValue(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.ProducedResourceType != EResourceType::None)
        {
            return CitizenInfoPresentation::FormatInteger(
                       Snapshot.ProducedResourceStock) +
                L" / " +
                CitizenInfoPresentation::FormatInteger(Snapshot.MaxResourceStock);
        }

        if (Snapshot.Harbor)
        {
            return CitizenInfoPresentation::FormatInteger(
                Snapshot.ExportableStock);
        }

        if (!Snapshot.Warehouse &&
            !Snapshot.CanGenerateWorkOutput &&
            Snapshot.ResourceStock <= 0)
        {
            return std::wstring();
        }

        return CitizenInfoPresentation::FormatInteger(Snapshot.ResourceStock) +
            L" / " +
            CitizenInfoPresentation::FormatInteger(Snapshot.MaxResourceStock);
    }

    std::wstring ResolveProductionChainStageText(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!Snapshot.ProductionChainStageText.empty())
            return Snapshot.ProductionChainStageText;

        if (Snapshot.ChainStage != EProductionChainStage::None)
        {
            const wchar_t* const StageText =
                GetProductionChainStageDisplayName(Snapshot.ChainStage);
            return StageText ? std::wstring(StageText) : std::wstring();
        }

        return std::wstring();
    }

    EProductionChainStage ResolveProductionChainStage(
        const FBuildingUiSnapshot& Snapshot)
    {
        return Snapshot.ChainStage;
    }

    std::wstring ResolveProductionChainStageBadgeText(
        EProductionChainStage Stage)
    {
        switch (Stage)
        {
        case EProductionChainStage::Primary:
            return L"원자재 생산";
        case EProductionChainStage::Intermediate:
            return L"중간재 가공";
        case EProductionChainStage::Final:
            return L"완제품";
        default:
            break;
        }

        return std::wstring();
    }

    FVector4 ResolveProductionChainStageBadgeColor(
        EProductionChainStage Stage)
    {
        switch (Stage)
        {
        case EProductionChainStage::Primary:
            return FVector4(0.24f, 0.62f, 0.30f, 1.f);
        case EProductionChainStage::Intermediate:
            return FVector4(0.78f, 0.58f, 0.10f, 1.f);
        case EProductionChainStage::Final:
            return FVector4(0.24f, 0.50f, 0.86f, 1.f);
        default:
            break;
        }

        return FVector4(0.26f, 0.62f, 0.82f, 1.f);
    }

    std::wstring ResolveSupplyChainSummaryText(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!Snapshot.SupplyChainSummaryText.empty())
            return Snapshot.SupplyChainSummaryText;

        return std::wstring();
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

            Summary += StringUtils::Utf8ToWide(Names[Index]);
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
        return UIEnumLabels::GetHousingClassDisplayName(HousingClass).c_str();
    }

    const wchar_t* GetLeisureClassDisplayName(
        EBuildingLeisureClass LeisureClass)
    {
        return UIEnumLabels::GetLeisureClassDisplayName(LeisureClass).c_str();
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
            CitizenInfoPresentation::FormatMoneyDollarFirst(
                ResourceTradePricing::GetExportPricePerStockUnit(Type)) +
            L" / " +
            Ui(L"citizen_info.label.import_short") +
            L" " +
            CitizenInfoPresentation::FormatMoneyDollarFirst(
                ResourceTradePricing::GetImportPricePerStockUnit(Type));
    }

    std::wstring ResolveProducedResourceDisplayName(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.ProducedResourceType != EResourceType::None)
        {
            if (Snapshot.CatalogEntry &&
                Snapshot.CatalogEntry->ProducedResourceType ==
                    Snapshot.ProducedResourceType)
            {
                const std::wstring CatalogDisplayName =
                    GetBuildingProducedResourceDisplayName(
                        *Snapshot.CatalogEntry);
                if (!CatalogDisplayName.empty())
                    return CatalogDisplayName;
            }

            return std::wstring(
                GetResourceTypeDisplayName(Snapshot.ProducedResourceType));
        }

        if (Snapshot.CatalogEntry)
            return GetBuildingProducedResourceDisplayName(*Snapshot.CatalogEntry);

        return std::wstring();
    }

    std::wstring ResolveProducedResourceStockLabel(
        const FBuildingUiSnapshot& Snapshot,
        const wchar_t* FallbackLabelKey)
    {
        const std::wstring ProducedResourceDisplayName =
            ResolveProducedResourceDisplayName(Snapshot);
        if (!ProducedResourceDisplayName.empty())
            return ProducedResourceDisplayName;

        return Ui(FallbackLabelKey);
    }

    void AppendProducedResourceTradeLines(
        std::wstring& Body,
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!HasTradeUnitPrice(Snapshot.ProducedResourceType))
            return;

        const std::wstring ProducedResourceDisplayName =
            ResolveProducedResourceDisplayName(Snapshot);

        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.produced_resource",
            ProducedResourceDisplayName);
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.export_unit_price",
            CitizenInfoPresentation::FormatMoneyDollarFirst(
                ResourceTradePricing::GetExportPricePerStockUnit(
                    Snapshot.ProducedResourceType)));
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.import_unit_price",
            CitizenInfoPresentation::FormatMoneyDollarFirst(
                ResourceTradePricing::GetImportPricePerStockUnit(
                    Snapshot.ProducedResourceType)));
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.market_price",
            BuildMarketPriceSummary(Snapshot.ProducedResourceType));
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.instant_export_value",
            CitizenInfoPresentation::FormatMoney(
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
}

using namespace CitizenInfoPresentationInternal;

namespace CitizenInfoPresentation
{
    std::wstring NormalizeWealthRequirementText(const std::wstring& Value)
    {
        if (Value == UIStrings::Get(L"citizen.wealth.broke"))
            return UIStrings::Get(L"citizen_info.wealth_profile.broke");
        if (Value == UIStrings::Get(L"citizen.wealth.well_off"))
            return UIStrings::Get(L"citizen_info.wealth_profile.well_off");
        if (Value == UIStrings::Get(L"citizen.wealth.rich"))
            return UIStrings::Get(L"citizen_info.wealth_profile.rich");
        if (Value == UIStrings::Get(L"citizen.wealth.filthy_rich"))
            return UIStrings::Get(L"citizen_info.wealth_profile.filthy_rich");
        if (Value == UIStrings::Get(L"citizen.wealth.poor"))
            return UIStrings::Get(L"citizen_info.wealth_profile.poor");

        return Value;
    }

    int ResolveOverviewHousingQuality(const FBuildingUiSnapshot& Snapshot)
    {
        const int Parsed =
            ParseLeadingInteger(Snapshot.HousingQualityText, Snapshot.HousingCap);
        return (std::max)(0, Parsed);
    }

    long long ResolveCurrentProducedStockExportValue(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.ProducedResourceType == EResourceType::None ||
            Snapshot.ProducedResourceStock <= 0 ||
            !IsExportableResourceType(Snapshot.ProducedResourceType))
        {
            return 0;
        }

        return ResourceTradePricing::ComputeExportValue(
            Snapshot.ProducedResourceType,
            Snapshot.ProducedResourceStock);
    }

    long long ResolveRealizedMonthlyTradeIncome(
        const FBuildingUiSnapshot& Snapshot)
    {
        const long long DailyTradeIncome =
            Snapshot.LastDailyExportIncome -
            Snapshot.LastDailyImportExpense;

        if (DailyTradeIncome == 0)
            return 0;

        return DailyTradeIncome *
            static_cast<long long>((std::max)(1, Snapshot.DaysInMonth));
    }

    double ResolveResidentialRentHousingClassMultiplier(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!Snapshot.CatalogEntry)
            return 1.0;

        switch (Snapshot.CatalogEntry->HousingClass)
        {
        case EBuildingHousingClass::Collective:
            return 0.85;
        case EBuildingHousingClass::Elite:
            return 1.40;
        case EBuildingHousingClass::Standard:
        case EBuildingHousingClass::None:
        default:
            return 1.0;
        }
    }

    double ResolveResidentialRentWealthMultiplier(
        const FBuildingUiSnapshot& Snapshot)
    {
        const unsigned int EffectiveMask =
            Snapshot.CatalogEntry &&
                Snapshot.CatalogEntry->AllowedWealthMask !=
                    GBuildingWealthMaskNone ?
                Snapshot.CatalogEntry->AllowedWealthMask :
                GBuildingWealthMaskAll;

        switch (EffectiveMask)
        {
        case GBuildingWealthMaskBroke:
            return 0.35;
        case GBuildingWealthMaskPoor:
            return 0.55;
        case GBuildingWealthMaskWellOff:
            return 1.00;
        case GBuildingWealthMaskRich:
            return 1.45;
        case GBuildingWealthMaskFilthyRich:
            return 1.85;
        case GBuildingWealthMaskWellOff |
            GBuildingWealthMaskRich |
            GBuildingWealthMaskFilthyRich:
            return 1.20;
        case GBuildingWealthMaskRich | GBuildingWealthMaskFilthyRich:
            return 1.55;
        case GBuildingWealthMaskPoor |
            GBuildingWealthMaskWellOff |
            GBuildingWealthMaskRich |
            GBuildingWealthMaskFilthyRich:
            return 0.70;
        case GBuildingWealthMaskAll:
        default:
            return 0.85;
        }
    }

    long long ResolveResidentialMonthlyRentIncome(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!Snapshot.Residential)
            return 0;

        const int OccupiedResidents =
            (std::max)(0, static_cast<int>(Snapshot.Residents.size()));

        if (OccupiedResidents <= 0)
            return 0;

        const double DailyRentPerResident =
            GameConstants::Economy::DailyResidenceValueBase *
            ResolveResidentialRentHousingClassMultiplier(Snapshot) *
            ResolveResidentialRentWealthMultiplier(Snapshot);

        if (DailyRentPerResident <= 0.0)
            return 0;

        return static_cast<long long>(std::llround(
            DailyRentPerResident *
            static_cast<double>(OccupiedResidents) *
            static_cast<double>((std::max)(1, Snapshot.DaysInMonth))));
    }

    long long ResolveProductionMonthlyIncomeEstimate(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (!UsesProductionEfficiencyMetric(Snapshot) ||
            Snapshot.ProducedResourceType == EResourceType::None ||
            !IsExportableResourceType(Snapshot.ProducedResourceType))
        {
            return 0;
        }

        const int DailyProductionUnits =
            (std::max)(0, Snapshot.EstimatedDailyProductionUnits);

        if (DailyProductionUnits <= 0)
            return 0;

        return static_cast<long long>(
            ResourceTradePricing::GetExportPricePerStockUnit(
                Snapshot.ProducedResourceType)) *
            static_cast<long long>(DailyProductionUnits) *
            static_cast<long long>((std::max)(1, Snapshot.DaysInMonth));
    }

    long long ResolveOverviewMonthlyIncome(const FBuildingUiSnapshot& Snapshot)
    {
        const long long RealizedMonthlyTradeIncome =
            ResolveRealizedMonthlyTradeIncome(Snapshot);

        if (RealizedMonthlyTradeIncome != 0)
            return RealizedMonthlyTradeIncome;

        const long long ResidentialMonthlyRentIncome =
            ResolveResidentialMonthlyRentIncome(Snapshot);

        if (ResidentialMonthlyRentIncome != 0)
            return ResidentialMonthlyRentIncome;

        const long long ProductionMonthlyIncome =
            ResolveProductionMonthlyIncomeEstimate(Snapshot);

        if (ProductionMonthlyIncome != 0)
            return ProductionMonthlyIncome;

        return 0;
    }

    int ResolveOverviewRequiredPower(const FBuildingUiSnapshot& Snapshot)
    {
        return ResolveDisplayRequiredPowerMW(Snapshot);
    }

    std::wstring FormatCountPair(size_t Current, int Capacity)
    {
        if (Capacity <= 0)
            return std::to_wstring(Current);

        return std::to_wstring(Current) +
            L" / " +
            std::to_wstring((std::max)(0, Capacity));
    }

    std::wstring FormatPercentPair(size_t Current, int Capacity)
    {
        if (Capacity <= 0)
            return L"-";

        const float Ratio =
            static_cast<float>(Current) /
            static_cast<float>((std::max)(1, Capacity));
        return std::to_wstring(static_cast<int>(roundf(
            (std::max)(0.f, Ratio) * 100.f))) + L"%";
    }

    std::wstring FormatStockSummary(const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.ProducedResourceType != EResourceType::None)
        {
            return CitizenInfoPresentation::FormatInteger(
                       Snapshot.ProducedResourceStock) +
                L" / " +
                CitizenInfoPresentation::FormatInteger(
                    (std::max)(0, Snapshot.MaxResourceStock));
        }

        if (Snapshot.MaxResourceStock <= 0 &&
            Snapshot.ResourceStock <= 0 &&
            !Snapshot.Warehouse)
        {
            return std::wstring();
        }

        return CitizenInfoPresentation::FormatInteger(Snapshot.ResourceStock) +
            L" / " +
            CitizenInfoPresentation::FormatInteger(
                (std::max)(0, Snapshot.MaxResourceStock));
    }

    std::wstring FormatShipProgressValue(const FBuildingUiSnapshot& Snapshot)
    {
        return std::to_wstring(static_cast<int>(roundf(
            (std::max)(0.f, Snapshot.HarborShipProgressPercent) * 100.f))) +
            L"%";
    }

    std::wstring BuildDetailExcerpt(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Candidate;

        if (!Snapshot.NarrativeLines.empty())
        {
            Candidate = JoinLines(Snapshot.NarrativeLines);
        }
        else if (!Snapshot.DetailText.empty())
        {
            Candidate = Snapshot.DetailText;
        }
        else if (!Snapshot.EffectText.empty())
        {
            Candidate = Snapshot.EffectText;
        }
        else if (!Snapshot.NoteText.empty())
        {
            Candidate = Snapshot.NoteText;
        }

        Candidate = StringUtils::Trim(Candidate);

        if (Candidate.empty())
            return Candidate;

        Candidate.erase(
            std::remove(Candidate.begin(), Candidate.end(), L'\r'),
            Candidate.end());
        std::replace(Candidate.begin(), Candidate.end(), L'\n', L' ');

        constexpr size_t GMaxExcerptLength = 150;

        if (Candidate.size() > GMaxExcerptLength)
        {
            Candidate.resize(GMaxExcerptLength);
            Candidate += L"...";
        }

        return Candidate;
    }

    void ResetOverviewMetricOutput(
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result)
    {
        Result.OverviewMetricLabels = {};
        Result.OverviewMetricValues = {};
        Result.OverviewMetricAccentValues = {};
        Result.ShowOverviewMetricScroll = false;
        Result.OverviewMetricScrollOffset = 0;
        Result.OverviewMetricScrollVisibleLineCount = 0;
        Result.OverviewMetricScrollTotalLineCount = 0;
        Result.OverviewMetricScrollFirstRowIndex = 0;
        Result.ShowHeaderNote = false;
        Result.HeaderNoteText.clear();
        Result.ShowSectionDivider = false;
    }

    std::wstring BuildInformationTopText(
        const FBuildingUiSnapshot& Snapshot,
        EBuildingUiProfile Profile)
    {
        std::wstring Text;
        AppendLine(Text, ResolveRoleSummary(Snapshot));

        switch (Profile)
        {
        case EBuildingUiProfile::Residential:
            AppendLine(
                Text,
                Ui(L"citizen_info.label.residents_status") + L": " +
                    FormatCountPair(Snapshot.Residents.size(), Snapshot.Capacity));
            AppendLine(
                Text,
                Ui(L"citizen_info.label.housing_quality") + L": " +
                    (Snapshot.HousingQualityText.empty() ?
                        L"-" :
                        Snapshot.HousingQualityText) +
                    L"  |  " +
                    Ui(L"citizen_info.label.required_wealth") + L": " +
                    NormalizeWealthRequirementText(
                        Snapshot.WealthRequirementText));
            break;
        case EBuildingUiProfile::Customs:
            AppendLine(
                Text,
                Ui(L"citizen_info.label.workers") + L": " +
                    FormatCountPair(
                        Snapshot.AssignedEmployees.size(),
                        Snapshot.Capacity));
            AppendLine(
                Text,
                Ui(L"citizen_info.label.efficiency") + L": " +
                    std::to_wstring(
                        CitizenInfoBuildingRuntime::ResolveCustomsEfficiencyPercent(
                            Snapshot)) +
                    L"%");
            break;
        case EBuildingUiProfile::Logistics:
            if (Snapshot.Harbor)
            {
                AppendLine(
                    Text,
                    Ui(L"citizen_info.label.exportable_stock") + L": " +
                        CitizenInfoPresentation::FormatInteger(
                            Snapshot.ExportableStock));
                AppendLine(
                    Text,
                    Ui(L"citizen_info.label.ship_arrival_progress") + L": " +
                        FormatShipProgressValue(Snapshot));
            }
            else
            {
                AppendLine(
                    Text,
                    Ui(L"citizen_info.label.current_stock") + L": " +
                        FormatStockSummary(Snapshot));
                AppendLine(
                    Text,
                    Ui(L"citizen_info.label.workers") + L": " +
                        FormatCountPair(
                            Snapshot.AssignedEmployees.size(),
                            Snapshot.Capacity));
            }
            break;
        case EBuildingUiProfile::Power:
        {
            const std::wstring ProducedPowerText =
                ResolveProducedPowerDisplayText(Snapshot);
            const std::wstring RequiredPowerText =
                ResolveRequiredPowerDisplayText(Snapshot);
            AppendLine(
                Text,
                Ui(L"citizen_info.label.produced_power") + L": " +
                    (ProducedPowerText.empty() ?
                        L"-" :
                        ProducedPowerText));
            AppendLine(
                Text,
                Ui(L"citizen_info.label.required_power") + L": " +
                    (RequiredPowerText.empty() ?
                        L"-" :
                        RequiredPowerText));
            break;
        }
        case EBuildingUiProfile::Tourism:
            AppendLine(
                Text,
                Ui(L"citizen_info.label.visitors") + L": " +
                    ResolveWorkerOverviewVisitorValue(Snapshot));
            AppendLine(
                Text,
                Ui(L"citizen_info.label.primary_tourist") + L": " +
                    (ResolveWorkerOverviewPreferredType(Snapshot).empty() ?
                        L"-" :
                        ResolveWorkerOverviewPreferredType(Snapshot)));
            break;
        case EBuildingUiProfile::Service:
            AppendLine(
                Text,
                UIStrings::Get(GetServiceCapacityLabelKey(Snapshot)) + L": " +
                    (Snapshot.ServiceCapacityText.empty() ?
                        L"-" :
                        Snapshot.ServiceCapacityText));
            AppendLine(
                Text,
                Ui(L"citizen_info.label.service_quality") + L": " +
                    (Snapshot.ServiceQualityText.empty() ?
                        L"-" :
                        Snapshot.ServiceQualityText));
            break;
        case EBuildingUiProfile::Production:
            AppendLine(
                Text,
                ResolveProducedResourceStockLabel(
                    Snapshot,
                    L"citizen_info.label.current_stock") +
                    L": " +
                    FormatStockSummary(Snapshot));
            if (Snapshot.ProducedResourceType != EResourceType::None &&
                IsExportableResourceType(Snapshot.ProducedResourceType))
            {
                AppendLine(
                    Text,
                    Ui(L"citizen_info.label.instant_export_value") + L": " +
                        FormatMoney(
                            ResolveCurrentProducedStockExportValue(Snapshot)));
            }
            AppendLine(
                Text,
                Ui(L"citizen_info.label.workers") + L": " +
                    FormatCountPair(
                        Snapshot.AssignedEmployees.size(),
                        Snapshot.Capacity));
            break;
        case EBuildingUiProfile::Generic:
        default:
            AppendLine(
                Text,
                Ui(L"citizen_info.label.monthly_upkeep_cost") + L": " +
                    CitizenInfoPresentation::FormatMoney(
                        Snapshot.MonthlyUpkeepCost));
            AppendLine(
                Text,
                Ui(L"citizen_info.label.budget_scale") + L": " +
                    CitizenInfoPresentation::FormatMultiplier(
                        Snapshot.BudgetScale));
            break;
        }

        return Text;
    }

    std::wstring BuildInformationBottomText(
        const FBuildingUiSnapshot& Snapshot,
        EBuildingUiProfile Profile)
    {
        std::wstring Text;

        switch (Profile)
        {
        case EBuildingUiProfile::Customs:
            AppendLine(
                Text,
                L"현재 근무 형태: " +
                    (Snapshot.ActiveOperationModeText.empty() ?
                        L"-" :
                        Snapshot.ActiveOperationModeText));
            if (!Snapshot.ActiveOperationModeEffectSummary.empty())
            {
                AppendLine(
                    Text,
                    L"효과: " + Snapshot.ActiveOperationModeEffectSummary);
            }
            break;
        case EBuildingUiProfile::Logistics:
            if (!Snapshot.WarehousePolicySelectionText.empty())
            {
                AppendLine(
                    Text,
                    L"창고 정책: " + Snapshot.WarehousePolicySelectionText);
            }
            if (!Snapshot.WarehousePrioritySelectionText.empty())
            {
                AppendLine(
                    Text,
                    L"정렬 우선순위: " +
                        Snapshot.WarehousePrioritySelectionText);
            }
            if (!Snapshot.HarborExportSelectionText.empty())
            {
                AppendLine(
                    Text,
                    L"수출 차단: " + Snapshot.HarborExportSelectionText);
            }
            break;
        case EBuildingUiProfile::Power:
            AppendLine(
                Text,
                Ui(L"citizen_info.label.power_grid_status") + L": " +
                    CitizenInfoPresentation::FormatSignedMegawattValue(
                        Snapshot.TotalProducedPowerMW -
                        Snapshot.TotalRequiredPowerMW));
            if (Snapshot.RequiredPowerMW > 0)
            {
                AppendLine(
                    Text,
                    Ui(L"citizen_info.label.power_network") + L": " +
                        FormatPowerCoverageValue(Snapshot.PowerSupplyRatio));
            }
            break;
        case EBuildingUiProfile::Tourism:
            AppendLine(
                Text,
                Ui(L"citizen_info.label.service_quality") + L": " +
                    (Snapshot.ServiceQualityText.empty() ?
                        L"-" :
                        Snapshot.ServiceQualityText));
            AppendLine(
                Text,
                Ui(L"citizen_info.label.last_month_income") + L": " +
                    CitizenInfoPresentation::FormatMoneyDollarFirst(
                        ResolveOverviewMonthlyIncome(Snapshot) -
                        static_cast<long long>(Snapshot.MonthlyUpkeepCost)));
            break;
        case EBuildingUiProfile::Service:
            AppendLine(
                Text,
                Ui(L"citizen_info.label.required_wealth") + L": " +
                    NormalizeWealthRequirementText(
                        Snapshot.WealthRequirementText));
            AppendLine(
                Text,
                Ui(L"citizen_info.label.workers") + L": " +
                    FormatCountPair(
                        Snapshot.AssignedEmployees.size(),
                        Snapshot.Capacity));
            break;
        case EBuildingUiProfile::Production:
        {
            const std::wstring ProducedResourceDisplayName =
                ResolveProducedResourceDisplayName(Snapshot);
            if (!ProducedResourceDisplayName.empty())
            {
                AppendLine(
                    Text,
                    Ui(L"citizen_info.label.produced_resource") + L": " +
                        ProducedResourceDisplayName);
            }
            AppendLine(
                Text,
                Ui(L"citizen_info.label.required_education") + L": " +
                    std::wstring(
                        GetCitizenEducationDisplayName(
                            Snapshot.RequiredEducationLevel)));
            break;
        }
        case EBuildingUiProfile::Residential:
        case EBuildingUiProfile::Generic:
        default:
            break;
        }

        const std::wstring Excerpt = BuildDetailExcerpt(Snapshot);

        if (!Excerpt.empty())
        {
            if (!Text.empty())
                AppendLine(Text, L"");

            AppendLine(Text, Excerpt);
        }

        return Text;
    }

    void PopulateResidentialOverview(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result)
    {
        const int PowerSurplusMW =
            Snapshot.TotalProducedPowerMW -
            Snapshot.TotalRequiredPowerMW;
        const int HousingQuality =
            ResolveOverviewHousingQuality(Snapshot);
        const long long MonthlyIncome =
            ResolveOverviewMonthlyIncome(Snapshot);
        const int RequiredPowerMW =
            ResolveOverviewRequiredPower(Snapshot);

        Result.OverviewBudgetLabel =
            Ui(L"citizen_info.label.budget");
        Result.OverviewBudgetValue =
            FormatMoney(Snapshot.MonthlyUpkeepCost);
        Result.OverviewOccupancyLabel =
            Ui(L"citizen_info.label.residence");
        Result.OverviewOccupancyValue =
            std::to_wstring(Snapshot.Residents.size()) +
            L" / " +
            std::to_wstring((std::max)(0, Snapshot.Capacity));
        Result.OverviewWorkModeLabel =
            Ui(L"citizen_info.action.operation_mode_cycle");
        Result.OverviewWorkModeValue =
            !Snapshot.ActiveOperationModeText.empty() ?
                Snapshot.ActiveOperationModeText :
                (!Snapshot.OperationModes.empty() ?
                    Snapshot.OperationModes.front() :
                    L"");
        Result.OverviewResidentCount =
            static_cast<int>(Snapshot.Residents.size());
        Result.OverviewResidentCapacity =
            (std::max)(0, Snapshot.Capacity);
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
            NormalizeWealthRequirementText(Snapshot.WealthRequirementText),
            FormatMoney(MonthlyIncome),
            FormatMegawattValue(-RequiredPowerMW),
            L"#1",
            FormatSignedMegawattValue(PowerSurplusMW)
        };
    }

    void PopulateCustomsWorkOverview(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result)
    {
        const int WorkerCapacity =
            (std::max)(1, Snapshot.Capacity);
        const int WorkerCount = (std::min)(
            WorkerCapacity,
            static_cast<int>(Snapshot.AssignedEmployees.size()));
        Result.OverviewWorkModeLabel =
            Ui(L"citizen_info.label.work_mode");
        Result.OverviewWorkModeValue =
            !Snapshot.ActiveOperationModeText.empty() ?
                Snapshot.ActiveOperationModeText :
                (!Snapshot.OperationModes.empty() ?
                    Snapshot.OperationModes.front() :
                    L"-");
        Result.OverviewBudgetLabel =
            Ui(L"citizen_info.label.budget");
        Result.OverviewBudgetValue =
            ResolveOverviewBudgetValue(Snapshot);
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
            ResolveWorkerOverviewJobQuality(Snapshot),
            GetCitizenEducationDisplayName(
                Snapshot.RequiredEducationLevel),
            CitizenInfoBuildingRuntime::ResolveCustomsPerWorkerWage(
                Snapshot),
            std::to_wstring(
                CitizenInfoBuildingRuntime::ResolveCustomsEfficiencyPercent(
                    Snapshot)) +
                L"%"
        };
    }

    void PopulateGenericWorkOverview(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result,
        int OverviewMetricScrollOffset)
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
        const std::wstring ProducedResourceDisplayName =
            ResolveProducedResourceDisplayName(Snapshot);
        const std::wstring SupplyChainSummaryText =
            ResolveSupplyChainSummaryText(Snapshot);
        Writer.Add(
            Ui(L"citizen_info.label.job_quality"),
            ResolveWorkerOverviewJobQuality(Snapshot));
        Writer.Add(
            Ui(L"citizen_info.label.required_education"),
            GetCitizenEducationDisplayName(
                Snapshot.RequiredEducationLevel));
        if (!ProducedResourceDisplayName.empty())
        {
            Writer.AddHeader(ProducedResourceDisplayName);
        }
        else if (!SupplyChainSummaryText.empty())
        {
            Writer.AddHeader(SupplyChainSummaryText);
        }
        Writer.Add(
            Ui(L"citizen_info.label.monthly_wage_cost"),
            FormatMoney(Snapshot.MonthlyWageCost));
        Writer.Add(
            Ui(L"citizen_info.label.efficiency"),
            ResolveWorkerOverviewEfficiency(Snapshot));

        if (HasProductionInputRecords(Snapshot))
        {
            Writer.AddHeader(Ui(L"citizen_info.section.production_inputs"));

            for (size_t Index = 0; Index < Snapshot.ProductionInputs.size(); ++Index)
            {
                const FProductionInputSlotView& Input =
                    Snapshot.ProductionInputs[Index];

                if (!IsValidProductionInputRecord(Input))
                    continue;

                Writer.Add(
                    BuildProductionInputRequirementText(Input),
                    BuildProductionInputOverviewValue(Input),
                    Input.CurrentStock >=
                        (std::max)(1, Input.RequiredAmount));
            }
        }

        const std::wstring PowerValue =
            ResolveWorkerOverviewPowerValue(Snapshot);

        if (!PowerValue.empty())
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

            if (Snapshot.ProducedResourceType != EResourceType::None)
            {
                Writer.Add(
                    ResolveProducedResourceStockLabel(
                        Snapshot,
                        L"citizen_info.label.output_stock"),
                    StorageValue,
                    Snapshot.ProducedResourceStock > 0);
            }
            else if (Snapshot.Harbor)
            {
                Writer.Add(
                    Ui(L"citizen_info.label.harbor_cargo"),
                    StorageValue,
                    Snapshot.ExportableStock > 0);
                Writer.Add(
                    Ui(L"citizen_info.label.next_arrival_time"),
                    FormatDayCount((std::max)(
                        0,
                        static_cast<int>(roundf(
                            (1.f - Snapshot.HarborShipProgressPercent) *
                            2.f *
                            static_cast<float>((std::max)(
                                1,
                                Snapshot.DaysInMonth)))))));
                if (!Snapshot.HarborResourceLines.empty())
                {
                    AppendScrollableOverviewMetricLines(
                        Writer,
                        Result,
                        Snapshot.HarborResourceLines,
                        OverviewMetricScrollOffset);
                }
            }
            else
            {
                Writer.Add(Ui(L"citizen_info.label.stock"), StorageValue);

                if (Snapshot.Warehouse)
                {
                    for (size_t Index = 0;
                        Index < Snapshot.WarehouseSlotLines.size();
                        ++Index)
                    {
                        Writer.Add(
                            L"  " + Snapshot.WarehouseSlotLines[Index],
                            std::wstring());
                    }
                }
            }
        }
        else if (Snapshot.CanGenerateWorkOutput)
        {
            Writer.Add(
                Ui(L"citizen_info.label.current_stock"),
                FormatInteger(Snapshot.ResourceStock));
        }
    }

    void PopulateBuildingStatisticsMetrics(
        const FBuildingUiSnapshot& Snapshot,
        bool IsCustomsOffice,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result,
        int OverviewMetricScrollOffset)
    {
        ResetOverviewMetricOutput(Result);

        if (IsCustomsOffice)
        {
            const long long BuildingExpense =
                static_cast<long long>(Snapshot.MonthlyUpkeepCost);
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
                    -Snapshot.LastDailyImportExpense),
                FormatMoneyDollarFirst(-BuildingExpense),
                FormatInteger(
                    Snapshot.TradeRouteExportFulfilledUnits),
                FormatInteger(
                    Snapshot.TradeRouteImportFulfilledUnits),
                FormatInteger(
                    Snapshot.TradeRouteExportContractUnits),
                FormatInteger(Snapshot.TourismArrivalCount)
            };
            return;
        }

        const EBuildingUiProfile Profile =
            ResolveBuildingUiProfileInternal(Snapshot);
        const long long RealizedMonthlyTradeIncome =
            ResolveRealizedMonthlyTradeIncome(Snapshot);
        const long long CurrentOutputExportValue =
            ResolveCurrentProducedStockExportValue(Snapshot);
        const long long AnnualIncome =
            ResolveOverviewMonthlyIncome(Snapshot) * 12LL;
        const long long NetMonthlyIncome =
            ResolveOverviewMonthlyIncome(Snapshot) -
            static_cast<long long>(Snapshot.MonthlyUpkeepCost);
        FOverviewMetricWriter Writer{ Result };

        switch (Profile)
        {
        case EBuildingUiProfile::Residential:
            Writer.Add(
                Ui(L"citizen_info.label.residents_status"),
                FormatCountPair(Snapshot.Residents.size(), Snapshot.Capacity));
            Writer.Add(
                Ui(L"citizen_info.label.household_capacity"),
                Snapshot.HouseholdCapacityText.empty() ?
                    std::to_wstring((std::max)(0, Snapshot.HouseholdCapacity)) :
                    Snapshot.HouseholdCapacityText);
            Writer.Add(
                Ui(L"citizen_info.label.monthly_total_cost"),
                FormatMoney(
                    static_cast<long long>(Snapshot.MonthlyWageCost) +
                    static_cast<long long>(Snapshot.MonthlyUpkeepCost)));
            Writer.Add(
                Ui(L"citizen_info.label.monthly_income"),
                FormatMoney(ResolveOverviewMonthlyIncome(Snapshot)),
                true);
            Writer.Add(
                Ui(L"citizen_info.label.required_power"),
                ResolveRequiredPowerDisplayText(Snapshot));
            Writer.Add(
                Ui(L"citizen_info.label.housing_quality"),
                Snapshot.HousingQualityText,
                true);
            break;
        case EBuildingUiProfile::Logistics:
            Writer.Add(
                Ui(L"citizen_info.label.current_stock"),
                FormatStockSummary(Snapshot),
                true);
            if (Snapshot.Warehouse)
            {
                for (size_t Index = 0;
                    Index < Snapshot.WarehouseSlotLines.size();
                    ++Index)
                {
                    Writer.Add(
                        L"  " + Snapshot.WarehouseSlotLines[Index],
                        std::wstring());
                }
            }
            if (Snapshot.Harbor)
            {
                Writer.Add(
                    Ui(L"citizen_info.label.exportable_stock"),
                    FormatInteger(Snapshot.ExportableStock),
                    true);
                Writer.Add(
                    Ui(L"citizen_info.label.ship_arrival_progress"),
                    FormatShipProgressValue(Snapshot));
                if (!Snapshot.HarborResourceLines.empty())
                {
                    AppendScrollableOverviewMetricLines(
                        Writer,
                        Result,
                        Snapshot.HarborResourceLines,
                        OverviewMetricScrollOffset);
                }
            }
            Writer.Add(
                Ui(L"citizen_info.label.assigned_workers"),
                FormatCountPair(
                    Snapshot.AssignedEmployees.size(),
                    Snapshot.Capacity));
            Writer.Add(
                Ui(L"citizen_info.label.last_month_income"),
                FormatMoneyDollarFirst(NetMonthlyIncome),
                NetMonthlyIncome >= 0);
            break;
        case EBuildingUiProfile::Power:
        {
            const std::wstring ProducedPowerText =
                ResolveProducedPowerDisplayText(Snapshot);
            const std::wstring RequiredPowerText =
                ResolveRequiredPowerDisplayText(Snapshot);

            if (!ProducedPowerText.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.produced_power"),
                    ProducedPowerText,
                    true);
            }
            if (!RequiredPowerText.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.required_power"),
                    RequiredPowerText);
            }
            Writer.Add(
                Ui(L"citizen_info.label.power_grid_status"),
                FormatSignedMegawattValue(
                    Snapshot.TotalProducedPowerMW -
                    Snapshot.TotalRequiredPowerMW),
                Snapshot.TotalProducedPowerMW >= Snapshot.TotalRequiredPowerMW);
            Writer.Add(
                Ui(L"citizen_info.label.assigned_workers"),
                FormatCountPair(
                    Snapshot.AssignedEmployees.size(),
                    Snapshot.Capacity));
            Writer.Add(
                Ui(L"citizen_info.label.monthly_upkeep_cost"),
                FormatMoney(Snapshot.MonthlyUpkeepCost));
            Writer.Add(
                Ui(L"citizen_info.label.pollution_output"),
                Snapshot.PollutionOutput > 0 ?
                    std::to_wstring(Snapshot.PollutionOutput) :
                    std::wstring(L"-"));
            break;
        }
        case EBuildingUiProfile::Tourism:
            Writer.Add(
                Ui(L"citizen_info.label.visitors"),
                ResolveWorkerOverviewVisitorValue(Snapshot),
                true);
            Writer.Add(
                UIStrings::Get(GetServiceCapacityLabelKey(Snapshot)),
                Snapshot.ServiceCapacityText);
            Writer.Add(
                Ui(L"citizen_info.label.service_quality"),
                Snapshot.ServiceQualityText,
                true);
            Writer.Add(
                Ui(L"citizen_info.label.primary_tourist"),
                ResolveWorkerOverviewPreferredType(Snapshot));
            Writer.Add(
                Ui(L"citizen_info.label.assigned_workers"),
                FormatCountPair(
                    Snapshot.AssignedEmployees.size(),
                    Snapshot.Capacity));
            Writer.Add(
                Ui(L"citizen_info.label.last_month_income"),
                FormatMoneyDollarFirst(NetMonthlyIncome),
                NetMonthlyIncome >= 0);
            break;
        case EBuildingUiProfile::Service:
            Writer.Add(
                UIStrings::Get(GetServiceCapacityLabelKey(Snapshot)),
                Snapshot.ServiceCapacityText);
            Writer.Add(
                Ui(L"citizen_info.label.visitors"),
                ResolveWorkerOverviewVisitorValue(Snapshot),
                true);
            Writer.Add(
                Ui(L"citizen_info.label.service_quality"),
                Snapshot.ServiceQualityText,
                true);
            Writer.Add(
                Ui(L"citizen_info.label.required_wealth"),
                NormalizeWealthRequirementText(
                    Snapshot.WealthRequirementText));
            Writer.Add(
                Ui(L"citizen_info.label.assigned_workers"),
                FormatCountPair(
                    Snapshot.AssignedEmployees.size(),
                    Snapshot.Capacity));
            Writer.Add(
                Ui(L"citizen_info.label.monthly_upkeep_cost"),
                FormatMoney(Snapshot.MonthlyUpkeepCost));
            break;
        case EBuildingUiProfile::Production:
        {
            Writer.Add(
                ResolveProducedResourceStockLabel(
                    Snapshot,
                    L"citizen_info.label.current_stock"),
                FormatStockSummary(Snapshot),
                true);
            Writer.Add(
                Ui(L"citizen_info.label.assigned_workers"),
                FormatCountPair(
                    Snapshot.AssignedEmployees.size(),
                    Snapshot.Capacity));
            Writer.Add(
                Ui(L"citizen_info.label.working_now"),
                FormatCountPair(
                    Snapshot.WorkingNowOccupancy,
                    Snapshot.Capacity));
            Writer.Add(
                Ui(L"citizen_info.label.output_per_second"),
                CitizenInfoPresentation::FormatFlowRateValue(
                    Snapshot.CurrentProductionUnitsPerSecond,
                    L"citizen_info.unit.per_second_suffix"),
                Snapshot.CurrentProductionUnitsPerSecond > 0.f);
            Writer.Add(
                Ui(L"citizen_info.label.expected_daily_output"),
                CitizenInfoPresentation::FormatFlowVolumeValue(
                    Snapshot.EstimatedDailyProductionUnits,
                    L"citizen_info.unit.per_day_suffix"),
                Snapshot.EstimatedDailyProductionUnits > 0);
            Writer.Add(
                Ui(L"citizen_info.label.expected_monthly_output"),
                CitizenInfoPresentation::FormatFlowVolumeValue(
                    Snapshot.EstimatedMonthlyProductionUnits,
                    L"citizen_info.unit.per_month_suffix"),
                Snapshot.EstimatedMonthlyProductionUnits > 0);
            for (size_t Index = 0; Index < Snapshot.ProductionInputs.size(); ++Index)
            {
                const FProductionInputSlotView& Input =
                    Snapshot.ProductionInputs[Index];
                const std::wstring InputValue =
                    BuildProductionInputStatisticsValue(Input);

                if (InputValue.empty())
                    continue;

                Writer.Add(
                    BuildProductionInputStatisticsLabel(
                        static_cast<int>(Index)),
                    InputValue,
                    Input.CurrentStock > 0);
            }

            const std::wstring TeamsterDeliverySummary =
                BuildTeamsterDeliverySummary(Snapshot);

            if (!TeamsterDeliverySummary.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.teamster_delivery"),
                    TeamsterDeliverySummary,
                    TeamsterDeliverySummary ==
                        Ui(L"citizen_info.value.stable"));
            }
            if (RealizedMonthlyTradeIncome != 0)
            {
                Writer.Add(
                    Ui(L"citizen_info.label.last_month_income"),
                    FormatMoneyDollarFirst(NetMonthlyIncome),
                    NetMonthlyIncome >= 0);
                Writer.Add(
                    Ui(L"citizen_info.label.total_income"),
                    FormatMoney(AnnualIncome));
            }
            else
            {
                Writer.Add(
                    Ui(L"citizen_info.label.instant_export_value"),
                    FormatMoney(CurrentOutputExportValue),
                    CurrentOutputExportValue > 0);

                if (Snapshot.ProducedResourceType != EResourceType::None &&
                    IsExportableResourceType(Snapshot.ProducedResourceType))
                {
                    Writer.Add(
                        Ui(L"citizen_info.label.export_unit_price"),
                        FormatMoneyDollarFirst(
                            ResourceTradePricing::GetExportPricePerStockUnit(
                                Snapshot.ProducedResourceType)));
                }
                else
                {
                    Writer.Add(
                        Ui(L"citizen_info.label.total_income"),
                        FormatMoney(AnnualIncome));
                }
            }
            break;
        }
        case EBuildingUiProfile::Generic:
        default:
            Writer.Add(
                Ui(L"citizen_info.label.total_income"),
                FormatMoney(AnnualIncome));
            Writer.Add(
                Ui(L"citizen_info.label.last_month_income"),
                FormatMoneyDollarFirst(NetMonthlyIncome),
                NetMonthlyIncome >= 0);
            Writer.Add(
                Ui(L"citizen_info.label.monthly_total_cost"),
                FormatMoney(
                    static_cast<long long>(Snapshot.MonthlyWageCost) +
                    static_cast<long long>(Snapshot.MonthlyUpkeepCost)));
            Writer.Add(
                Ui(L"citizen_info.label.assigned_workers"),
                FormatCountPair(
                    Snapshot.AssignedEmployees.size(),
                    Snapshot.Capacity));
            break;
        }
    }

    void PopulateBuildingEfficiencyMetrics(
        const FBuildingUiSnapshot& Snapshot,
        bool IsCustomsOffice,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result)
    {
        ResetOverviewMetricOutput(Result);

        if (IsCustomsOffice)
        {
            const int DiplomacyModifier =
                CitizenInfoBuildingRuntime::
                    ComputeAverageCustomsDiplomacyExportBiasPercent();
            const int BudgetModifier =
                CitizenInfoBuildingRuntime::
                    ResolveCustomsBudgetModifierPercent(Snapshot);
            Result.ShowHeaderNote = true;
            Result.ShowSectionDivider = false;
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
                            Snapshot)) +
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
            return;
        }

        const EBuildingUiProfile Profile =
            ResolveBuildingUiProfileInternal(Snapshot);
        FOverviewMetricWriter Writer{ Result };

        Result.ShowHeaderNote = true;
        Result.ShowSectionDivider = false;

        switch (Profile)
        {
        case EBuildingUiProfile::Residential:
            Result.HeaderNoteText =
                Ui(L"citizen_info.note.housing_quality_efficiency");
            Writer.Add(
                Ui(L"citizen_info.label.efficiency"),
                ResolveWorkerOverviewEfficiency(Snapshot),
                true);
            Writer.Add(
                Ui(L"citizen_info.label.housing_fill_rate"),
                FormatPercentPair(
                    Snapshot.Residents.size(),
                    Snapshot.Capacity));
            Writer.Add(
                Ui(L"citizen_info.label.housing_quality"),
                Snapshot.HousingQualityText,
                true);
            Writer.Add(
                Ui(L"citizen_info.label.power_network"),
                Snapshot.RequiredPowerMW > 0 ?
                    FormatPowerCoverageValue(Snapshot.PowerSupplyRatio) :
                    L"-");
            Writer.Add(
                Ui(L"citizen_info.label.local_pollution"),
                std::to_wstring((std::max)(0, Snapshot.LocalPollutionExposure)));
            break;
        case EBuildingUiProfile::Logistics:
            Result.HeaderNoteText =
                L"물류 건물은 저장량과 도로 접근성에 따라 체감 효율이 달라집니다.";
            Writer.Add(
                Ui(L"citizen_info.label.current_efficiency"),
                ResolveWorkerOverviewEfficiency(Snapshot),
                true);
            Writer.Add(
                Ui(L"citizen_info.label.worker_fill_rate"),
                FormatPercentPair(
                    Snapshot.AssignedEmployees.size(),
                    Snapshot.Capacity));
            Writer.Add(
                Ui(L"citizen_info.label.current_stock"),
                FormatStockSummary(Snapshot));
            if (Snapshot.Harbor)
            {
                Writer.Add(
                    Ui(L"citizen_info.label.exportable_stock"),
                    FormatInteger(Snapshot.ExportableStock),
                    true);
            }
            else if (!Snapshot.WarehousePrioritySelectionText.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.action.warehouse_priority_cycle"),
                    Snapshot.WarehousePrioritySelectionText);
            }
            break;
        case EBuildingUiProfile::Power:
        {
            const std::wstring ProducedPowerText =
                ResolveProducedPowerDisplayText(Snapshot);
            const std::wstring RequiredPowerText =
                ResolveRequiredPowerDisplayText(Snapshot);
            Result.HeaderNoteText =
                L"전력 생산량과 전력망 여유분이 섬 전체 운영 효율에 직접 반영됩니다.";
            Writer.Add(
                Ui(L"citizen_info.label.current_efficiency"),
                ResolveWorkerOverviewEfficiency(Snapshot),
                true);
            if (!ProducedPowerText.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.produced_power"),
                    ProducedPowerText,
                    true);
            }
            if (!RequiredPowerText.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.power_demand"),
                    RequiredPowerText);
            }
            Writer.Add(
                Ui(L"citizen_info.label.power_network"),
                Snapshot.RequiredPowerMW > 0 ?
                    FormatPowerCoverageValue(Snapshot.PowerSupplyRatio) :
                    L"-");
            Writer.Add(
                Ui(L"citizen_info.label.power_grid_status"),
                FormatSignedMegawattValue(
                    Snapshot.TotalProducedPowerMW -
                    Snapshot.TotalRequiredPowerMW),
                Snapshot.TotalProducedPowerMW >= Snapshot.TotalRequiredPowerMW);
            Writer.Add(
                Ui(L"citizen_info.label.pollution_output"),
                Snapshot.PollutionOutput > 0 ?
                    std::to_wstring(Snapshot.PollutionOutput) :
                    std::wstring(L"-"));
            break;
        }
        case EBuildingUiProfile::Tourism:
            Result.HeaderNoteText =
                L"관광 시설은 방문객 수용률과 서비스 품질이 핵심 효율 지표입니다.";
            Writer.Add(
                Ui(L"citizen_info.label.current_efficiency"),
                ResolveWorkerOverviewEfficiency(Snapshot),
                true);
            Writer.Add(
                Ui(L"citizen_info.label.visitor_utilization"),
                FormatPercentPair(
                    Snapshot.AssignedVisitors.size(),
                    Snapshot.ServiceCapacity));
            Writer.Add(
                Ui(L"citizen_info.label.service_quality"),
                Snapshot.ServiceQualityText,
                true);
            Writer.Add(
                Ui(L"citizen_info.label.primary_tourist"),
                ResolveWorkerOverviewPreferredType(Snapshot));
            Writer.Add(
                Ui(L"citizen_info.label.worker_fill_rate"),
                FormatPercentPair(
                    Snapshot.AssignedEmployees.size(),
                    Snapshot.Capacity));
            break;
        case EBuildingUiProfile::Service:
            Result.HeaderNoteText =
                L"서비스 건물은 이용률과 서비스 품질이 체감 성능을 좌우합니다.";
            Writer.Add(
                Ui(L"citizen_info.label.current_efficiency"),
                ResolveWorkerOverviewEfficiency(Snapshot),
                true);
            Writer.Add(
                Ui(L"citizen_info.label.visitors"),
                ResolveWorkerOverviewVisitorValue(Snapshot));
            Writer.Add(
                Ui(L"citizen_info.label.service_quality"),
                Snapshot.ServiceQualityText,
                true);
            Writer.Add(
                Ui(L"citizen_info.label.required_wealth"),
                NormalizeWealthRequirementText(
                    Snapshot.WealthRequirementText));
            Writer.Add(
                Ui(L"citizen_info.label.worker_fill_rate"),
                FormatPercentPair(
                    Snapshot.AssignedEmployees.size(),
                    Snapshot.Capacity));
            break;
        case EBuildingUiProfile::Production:
            Result.HeaderNoteText =
                L"현재 효율은 생산 커버리지 기준이며 예산과 전력은 별도 지표로 표시됩니다.";
            Writer.Add(
                Ui(L"citizen_info.label.current_efficiency"),
                ResolveWorkerOverviewEfficiency(Snapshot),
                true);
            Writer.Add(
                Ui(L"citizen_info.label.job_quality"),
                ResolveWorkerOverviewJobQuality(Snapshot));
            Writer.Add(
                Ui(L"citizen_info.label.worker_fill_rate"),
                FormatPercentPair(
                    Snapshot.AssignedEmployees.size(),
                    Snapshot.Capacity));
            Writer.Add(
                ResolveProducedResourceStockLabel(
                    Snapshot,
                    L"citizen_info.label.current_stock"),
                FormatStockSummary(Snapshot));
            if (HasProductionInputRecords(Snapshot))
            {
                Writer.Add(
                    Ui(L"citizen_info.label.required_inputs"),
                    BuildProductionInputRequirementSummary(Snapshot));
            }
            Writer.Add(
                Ui(L"citizen_info.label.electricity"),
                ResolveWorkerOverviewPowerValue(Snapshot));
            break;
        case EBuildingUiProfile::Generic:
        default:
            Result.HeaderNoteText =
                L"운영 예산과 접근성이 건물의 기본 효율을 좌우합니다.";
            Writer.Add(
                Ui(L"citizen_info.label.current_efficiency"),
                ResolveWorkerOverviewEfficiency(Snapshot),
                true);
            Writer.Add(
                Ui(L"citizen_info.label.budget_scale"),
                CitizenInfoPresentation::FormatMultiplier(
                    Snapshot.BudgetScale));
            Writer.Add(
                Ui(L"citizen_info.label.electricity"),
                ResolveWorkerOverviewPowerValue(Snapshot));
            break;
        }
    }

    bool HasOverviewMetrics(
        const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot)
    {
        for (const std::wstring& Label : Snapshot.OverviewMetricLabels)
        {
            if (!Label.empty())
                return true;
        }

        return false;
    }

    bool PopulateBuildingInformationPanel(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result)
    {
        const EBuildingUiProfile Profile =
            ResolveBuildingUiProfileInternal(Snapshot);
        Result.InformationAccentText.clear();
        Result.InformationTopText = BuildInformationTopText(Snapshot, Profile);
        Result.InformationBottomText = BuildInformationBottomText(
            Snapshot,
            Profile);
        Result.ShowSectionDivider = true;

        return !Result.InformationTopText.empty() ||
            !Result.InformationBottomText.empty();
    }

}

