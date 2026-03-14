#include "CitizenInfoPresentation.h"
#include "CitizenInfoConstants.h"
#include "UIStrings.h"
#include "../Building/BuildingCatalog.h"
#include "../Economy/ResourceTradePricing.h"
#include "../StringUtils.h"
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <cwctype>
#include <vector>

namespace
{
    using FBuildingUiSnapshot = CitizenInfoBuildingRuntime::FBuildingUiSnapshot;

    const std::wstring& Ui(const wchar_t* Key)
    {
        return UIStrings::Get(Key);
    }

    const wchar_t* UiText(const wchar_t* Key)
    {
        return UIStrings::Get(Key).c_str();
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

    std::wstring FormatCatalogCostValue(
        EBuildingCostState State,
        int Value)
    {
        switch (State)
        {
        case EBuildingCostState::Known:
            return Value <= 0 ?
                Ui(L"citizen_info.value.free") :
                CitizenInfoPresentation::FormatMoney(Value);
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

    std::wstring ResolveOverviewBudgetValue(const FBuildingUiSnapshot& Snapshot)
    {
        return CitizenInfoPresentation::FormatMoney(Snapshot.MonthlyUpkeepCost);
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
            std::wstring Value =
                CitizenInfoPresentation::FormatMegawattValue(-RequiredPowerMW);
            Value += L" (";
            Value += FormatPowerCoverageValue(Snapshot.PowerSupplyRatio);
            Value += L")";
            return Value;
        }

        if (!Snapshot.RequiredPowerText.empty())
            return L"-" + Snapshot.RequiredPowerText;

        const int ProducedPowerMW = (std::max)(0, Snapshot.ProducedPowerMW);

        if (ProducedPowerMW > 0)
            return L"+" +
                CitizenInfoPresentation::FormatMegawattValue(ProducedPowerMW);

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

        return CitizenInfoPresentation::FormatInteger(Snapshot.ResourceStock) +
            L" / " +
            CitizenInfoPresentation::FormatInteger(Snapshot.MaxResourceStock);
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
            CitizenInfoPresentation::FormatMoneyDollarFirst(
                ResourceTradePricing::GetExportPricePerStockUnit(Type)) +
            L" / " +
            Ui(L"citizen_info.label.import_short") +
            L" " +
            CitizenInfoPresentation::FormatMoneyDollarFirst(
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

namespace CitizenInfoPresentation
{
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

    std::wstring FormatMultiplier(float Value)
    {
        wchar_t Buffer[32] = {};
        swprintf_s(Buffer, L"x%.2f", Value);
        return Buffer;
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

    enum class EBuildingUiProfile
    {
        Residential,
        Customs,
        Logistics,
        Power,
        Tourism,
        Service,
        Production,
        Generic
    };

    EBuildingUiProfile ResolveBuildingUiProfileInternal(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (CitizenInfoBuildingRuntime::IsCustomsOfficeBuilding(Snapshot))
            return EBuildingUiProfile::Customs;

        if (Snapshot.Residential)
            return EBuildingUiProfile::Residential;

        if (Snapshot.Harbor || Snapshot.Warehouse)
            return EBuildingUiProfile::Logistics;

        const bool ProducesPower =
            Snapshot.ProducedPowerMW > 0 ||
            (Snapshot.CatalogEntry &&
                Snapshot.CatalogEntry->BaseProducedPowerMW > 0);
        const bool UsesPower =
            Snapshot.RequiredPowerMW > 0 ||
            (Snapshot.CatalogEntry &&
                Snapshot.CatalogEntry->BaseRequiredPowerMW > 0);
        const bool TourismIdentity =
            (Snapshot.CatalogEntry &&
                Snapshot.CatalogEntry->Category == EBuildingCategory::Tourism) ||
            Snapshot.TourismArrivalCount > 0 ||
            !ResolveWorkerOverviewPreferredType(Snapshot).empty();

        if (ProducesPower)
            return EBuildingUiProfile::Power;

        if (TourismIdentity)
            return EBuildingUiProfile::Tourism;

        if (Snapshot.FoodProvider ||
            Snapshot.EntertainmentProvider ||
            Snapshot.HealthProvider ||
            Snapshot.FaithProvider)
        {
            return EBuildingUiProfile::Service;
        }

        if (Snapshot.CanGenerateWorkOutput ||
            Snapshot.UsesResourceStock ||
            Snapshot.ProducedResourceType != EResourceType::None)
        {
            return EBuildingUiProfile::Production;
        }

        if (UsesPower)
            return EBuildingUiProfile::Power;

        return EBuildingUiProfile::Generic;
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

        Candidate = Trim(Candidate);

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
            AppendLine(
                Text,
                Ui(L"citizen_info.label.produced_power") + L": " +
                    (Snapshot.ProducedPowerText.empty() ?
                        L"-" :
                        Snapshot.ProducedPowerText));
            AppendLine(
                Text,
                Ui(L"citizen_info.label.required_power") + L": " +
                    (Snapshot.RequiredPowerText.empty() ?
                        L"-" :
                        Snapshot.RequiredPowerText));
            break;
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
                Ui(L"citizen_info.label.current_stock") + L": " +
                    FormatStockSummary(Snapshot));
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
            if (!Snapshot.HarborImportSelectionText.empty())
            {
                AppendLine(
                    Text,
                    L"자동 수입: " + Snapshot.HarborImportSelectionText);
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
            if (Snapshot.ProducedResourceType != EResourceType::None)
            {
                AppendLine(
                    Text,
                    Ui(L"citizen_info.label.produced_resource") + L": " +
                        std::wstring(
                            GetResourceTypeDisplayName(
                                Snapshot.ProducedResourceType)));
            }
            AppendLine(
                Text,
                Ui(L"citizen_info.label.required_education") + L": " +
                    std::wstring(
                        GetCitizenEducationDisplayName(
                            Snapshot.RequiredEducationLevel)));
            break;
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

    std::wstring ResolveProfileTabTitle(
        EBuildingUiProfile Profile,
        int TabIndex,
        bool ShowCustomsModeSelection)
    {
        if (TabIndex == 0 && ShowCustomsModeSelection)
            return L"근무 형태";

        switch (TabIndex)
        {
        case 1:
            switch (Profile)
            {
            case EBuildingUiProfile::Residential:
                return L"거주 상태";
            case EBuildingUiProfile::Customs:
                return L"무역 상태";
            case EBuildingUiProfile::Logistics:
                return L"물류 상태";
            case EBuildingUiProfile::Power:
                return L"전력 상태";
            case EBuildingUiProfile::Tourism:
                return L"관광 상태";
            case EBuildingUiProfile::Service:
                return L"서비스 상태";
            case EBuildingUiProfile::Production:
                return L"생산 상태";
            default:
                break;
            }
            break;
        case 2:
            switch (Profile)
            {
            case EBuildingUiProfile::Customs:
                return L"무역 설정";
            case EBuildingUiProfile::Power:
                return L"설비 업그레이드";
            case EBuildingUiProfile::Logistics:
                return L"물류 업그레이드";
            default:
                return L"업그레이드";
            }
        case 3:
            switch (Profile)
            {
            case EBuildingUiProfile::Residential:
                return L"주거 효율";
            case EBuildingUiProfile::Customs:
                return L"수출 효율";
            case EBuildingUiProfile::Logistics:
                return L"물류 효율";
            case EBuildingUiProfile::Power:
                return L"전력 효율";
            case EBuildingUiProfile::Tourism:
                return L"관광 효율";
            case EBuildingUiProfile::Service:
                return L"서비스 효율";
            case EBuildingUiProfile::Production:
                return L"생산 효율";
            default:
                break;
            }
            break;
        case 4:
            switch (Profile)
            {
            case EBuildingUiProfile::Residential:
                return L"거주 설명";
            case EBuildingUiProfile::Customs:
                return L"세관 설명";
            case EBuildingUiProfile::Logistics:
                return L"물류 설명";
            case EBuildingUiProfile::Power:
                return L"전력 설명";
            case EBuildingUiProfile::Tourism:
                return L"관광 설명";
            case EBuildingUiProfile::Service:
                return L"서비스 설명";
            case EBuildingUiProfile::Production:
                return L"생산 설명";
            default:
                break;
            }
            break;
        default:
            break;
        }

        return CitizenInfoConstants::GetBuildingPageTitle(TabIndex);
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
                L"수입세 감소";
        Result.OverviewBudgetLabel =
            Ui(L"citizen_info.label.budget");
        Result.OverviewBudgetValue =
            Snapshot.MonthlyUpkeepCost > 0 ?
                FormatMoney(Snapshot.MonthlyUpkeepCost) :
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
            !Snapshot.JobQualityText.empty() ?
                Snapshot.JobQualityText :
                L"70",
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

    void PopulateBuildingStatisticsMetrics(
        const FBuildingUiSnapshot& Snapshot,
        bool IsCustomsOffice,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result)
    {
        ResetOverviewMetricOutput(Result);

        if (IsCustomsOffice)
        {
            const long long BuildingExpense =
                static_cast<long long>(
                    (std::max)(
                        118,
                        Snapshot.MonthlyUpkeepCost));
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
                Snapshot.RequiredPowerText);
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
                Writer.Add(
                    Ui(L"citizen_info.label.storage"),
                    std::to_wstring(Snapshot.WarehouseSlotLines.size()) +
                        L" 슬롯");
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
            if (!Snapshot.ProducedPowerText.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.produced_power"),
                    Snapshot.ProducedPowerText,
                    true);
            }
            if (!Snapshot.RequiredPowerText.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.required_power"),
                    Snapshot.RequiredPowerText);
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
            if (Snapshot.ProducedResourceType != EResourceType::None)
            {
                Writer.Add(
                    Ui(L"citizen_info.label.produced_resource"),
                    std::wstring(
                        GetResourceTypeDisplayName(
                            Snapshot.ProducedResourceType)));
            }
            Writer.Add(
                Ui(L"citizen_info.label.current_stock"),
                FormatStockSummary(Snapshot),
                true);
            Writer.Add(
                Ui(L"citizen_info.label.assigned_workers"),
                FormatCountPair(
                    Snapshot.AssignedEmployees.size(),
                    Snapshot.Capacity));
            Writer.Add(
                Ui(L"citizen_info.label.required_education"),
                std::wstring(
                    GetCitizenEducationDisplayName(
                        Snapshot.RequiredEducationLevel)));
            Writer.Add(
                Ui(L"citizen_info.label.last_month_income"),
                FormatMoneyDollarFirst(NetMonthlyIncome),
                NetMonthlyIncome >= 0);
            Writer.Add(
                Ui(L"citizen_info.label.total_income"),
                FormatMoney(AnnualIncome));
            break;
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
                L"100%",
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
            Writer.Add(
                Ui(L"citizen_info.label.road_accessibility"),
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.AccessibilityScore * 100.f))) +
                    L"%");
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
            Result.HeaderNoteText =
                L"전력 생산량과 전력망 여유분이 섬 전체 운영 효율에 직접 반영됩니다.";
            Writer.Add(
                Ui(L"citizen_info.label.current_efficiency"),
                ResolveWorkerOverviewEfficiency(Snapshot),
                true);
            if (!Snapshot.ProducedPowerText.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.produced_power"),
                    Snapshot.ProducedPowerText,
                    true);
            }
            if (!Snapshot.RequiredPowerText.empty())
            {
                Writer.Add(
                    Ui(L"citizen_info.label.power_demand"),
                    Snapshot.RequiredPowerText);
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
            Writer.Add(
                Ui(L"citizen_info.label.road_accessibility"),
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.AccessibilityScore * 100.f))) +
                    L"%");
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
            Writer.Add(
                Ui(L"citizen_info.label.road_accessibility"),
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.AccessibilityScore * 100.f))) +
                    L"%");
            break;
        case EBuildingUiProfile::Production:
            Result.HeaderNoteText =
                L"생산 건물은 인력, 전력, 재고 상태가 동시에 맞물려 효율을 결정합니다.";
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
                Ui(L"citizen_info.label.current_stock"),
                FormatStockSummary(Snapshot));
            Writer.Add(
                Ui(L"citizen_info.label.electricity"),
                ResolveWorkerOverviewPowerValue(Snapshot));
            Writer.Add(
                Ui(L"citizen_info.label.road_accessibility"),
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.AccessibilityScore * 100.f))) +
                    L"%");
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
                Ui(L"citizen_info.label.road_accessibility"),
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.AccessibilityScore * 100.f))) +
                    L"%");
            Writer.Add(
                Ui(L"citizen_info.label.electricity"),
                ResolveWorkerOverviewPowerValue(Snapshot));
            break;
        }
    }

    std::wstring ResolveBuildingPageTitle(
        const FBuildingUiSnapshot& Snapshot,
        int TabIndex,
        bool ShowCustomsModeSelection)
    {
        return ResolveProfileTabTitle(
            ResolveBuildingUiProfileInternal(Snapshot),
            TabIndex,
            ShowCustomsModeSelection);
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

    bool PopulateBuildingUpgradeCard(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result)
    {
        Result.UpgradeCardTitle.clear();
        Result.UpgradeCardDescription.clear();
        Result.UpgradeCardIconPath = nullptr;
        Result.UpgradeCardIconTextureKey.clear();

        if (const FBuildingRuntimeUpgradeDef* const ActiveUpgradeDef =
            ResolveActiveRuntimeUpgradeDef(Snapshot))
        {
            Result.UpgradeCardTitle = ActiveUpgradeDef->DisplayName;
            Result.UpgradeCardDescription =
                !Snapshot.ActiveRuntimeUpgradeEffectSummary.empty() ?
                    Snapshot.ActiveRuntimeUpgradeEffectSummary :
                    BuildRuntimeUpgradeSummary(
                        *ActiveUpgradeDef,
                        Snapshot.ActiveRuntimeUpgradeEffectSummary.empty() ?
                            nullptr :
                            &Snapshot.ActiveRuntimeUpgradeEffectSummary);
            return !Result.UpgradeCardTitle.empty();
        }

        if (!Snapshot.ActiveOperationModeText.empty())
        {
            Result.UpgradeCardTitle = L"현재 근무 형태";
            Result.UpgradeCardDescription = Snapshot.ActiveOperationModeText;

            if (!Snapshot.ActiveOperationModeEffectSummary.empty())
            {
                Result.UpgradeCardDescription += L"\n";
                Result.UpgradeCardDescription +=
                    Snapshot.ActiveOperationModeEffectSummary;
            }

            return true;
        }

        const EBuildingUiProfile Profile =
            ResolveBuildingUiProfileInternal(Snapshot);

        switch (Profile)
        {
        case EBuildingUiProfile::Residential:
            if (Snapshot.CatalogEntry &&
                Snapshot.CatalogEntry->HousingClass !=
                    EBuildingHousingClass::None)
            {
                Result.UpgradeCardTitle = L"주거 등급";
                Result.UpgradeCardDescription =
                    GetHousingClassDisplayName(
                        Snapshot.CatalogEntry->HousingClass);
            }
            break;
        case EBuildingUiProfile::Logistics:
            if (!Snapshot.WarehousePolicySelectionText.empty() ||
                !Snapshot.WarehousePrioritySelectionText.empty() ||
                !Snapshot.HarborImportSelectionText.empty() ||
                !Snapshot.HarborExportSelectionText.empty())
            {
                Result.UpgradeCardTitle = L"물류 정책";
                if (!Snapshot.WarehousePolicySelectionText.empty())
                {
                    AppendLine(
                        Result.UpgradeCardDescription,
                        L"창고 정책: " +
                            Snapshot.WarehousePolicySelectionText);
                }
                if (!Snapshot.WarehousePrioritySelectionText.empty())
                {
                    AppendLine(
                        Result.UpgradeCardDescription,
                        L"정렬 우선순위: " +
                            Snapshot.WarehousePrioritySelectionText);
                }
                if (!Snapshot.HarborImportSelectionText.empty())
                {
                    AppendLine(
                        Result.UpgradeCardDescription,
                        L"자동 수입: " + Snapshot.HarborImportSelectionText);
                }
                if (!Snapshot.HarborExportSelectionText.empty())
                {
                    AppendLine(
                        Result.UpgradeCardDescription,
                        L"수출 차단: " + Snapshot.HarborExportSelectionText);
                }
            }
            break;
        case EBuildingUiProfile::Power:
            Result.UpgradeCardTitle = L"전력 요약";
            Result.UpgradeCardDescription =
                Ui(L"citizen_info.label.produced_power") + L": " +
                (Snapshot.ProducedPowerText.empty() ?
                    L"-" :
                    Snapshot.ProducedPowerText) +
                L"\n" +
                Ui(L"citizen_info.label.power_grid_status") + L": " +
                FormatSignedMegawattValue(
                    Snapshot.TotalProducedPowerMW -
                    Snapshot.TotalRequiredPowerMW);
            break;
        case EBuildingUiProfile::Tourism:
            if (!ResolveWorkerOverviewPreferredType(Snapshot).empty())
            {
                Result.UpgradeCardTitle = L"주요 고객";
                Result.UpgradeCardDescription =
                    ResolveWorkerOverviewPreferredType(Snapshot);
            }
            break;
        case EBuildingUiProfile::Service:
            if (!Snapshot.ServiceQualityText.empty())
            {
                Result.UpgradeCardTitle = L"서비스 품질";
                Result.UpgradeCardDescription = Snapshot.ServiceQualityText;
            }
            break;
        case EBuildingUiProfile::Production:
            if (Snapshot.ProducedResourceType != EResourceType::None)
            {
                Result.UpgradeCardTitle = L"주요 생산품";
                Result.UpgradeCardDescription =
                    GetResourceTypeDisplayName(
                        Snapshot.ProducedResourceType);
            }
            break;
        case EBuildingUiProfile::Customs:
        case EBuildingUiProfile::Generic:
        default:
            break;
        }

        return !Result.UpgradeCardTitle.empty();
    }

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
            return StringUtils::Utf8ToWide(BuildingName);

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
