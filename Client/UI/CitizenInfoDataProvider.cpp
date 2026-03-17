#include "CitizenInfoDataProvider.h"
#include "CitizenInfoDataProviderInternal.h"
#include "CitizenInfoBuildingRuntime.h"
#include "CitizenInfoConstants.h"
#include "CitizenInfoPresentation.h"
#include "CitizenInfoQueryService.h"
#include "UIStrings.h"
#include "../Building/BuildingCatalog.h"
#include "../Economy/ResourceTradePricing.h"
#include "../StringUtils.h"
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <cwctype>
#include <vector>

namespace CitizenInfoDataProviderInternal
{
    using CitizenInfoConstants::GBuildingTabCount;
    using CitizenInfoConstants::GCitizenTabCount;
    using FBuildingUiSnapshot = CitizenInfoBuildingRuntime::FBuildingUiSnapshot;
    using CitizenInfoPresentation::PopulateBuildingEfficiencyMetrics;
    using CitizenInfoPresentation::PopulateBuildingInformationPanel;
    using CitizenInfoPresentation::PopulateBuildingStatisticsMetrics;
    using CitizenInfoPresentation::PopulateBuildingUpgradeCard;
    using CitizenInfoPresentation::PopulateCustomsWorkOverview;
    using CitizenInfoPresentation::PopulateResidentialOverview;
    using CitizenInfoPresentation::ResolveBuildingPageTitle;
    using CitizenInfoPresentation::HasOverviewMetrics;
    using StringUtils::Utf8ToWide;

    std::string BuildCatalogIconTextureKey(
        const FBuildingCatalogEntry& Entry)
    {
        return
            "BuildingCatalogIcon_" +
            Entry.Id +
            "_Gen_" +
            std::to_string(::GetRuntimeConfigGeneration());
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

    std::wstring GetDamageLevelDisplayName(EBuildingDamageLevel Level)
    {
        switch (Level)
        {
        case EBuildingDamageLevel::Damaged:
            return Ui(L"citizen_info.damage.damaged");
        case EBuildingDamageLevel::Critical:
            return Ui(L"citizen_info.damage.critical");
        case EBuildingDamageLevel::None:
        default:
            return Ui(L"citizen_info.damage.none");
        }
    }

    std::wstring FormatMoney(long long Value)
    {
        if (Value < 0)
            return L"-$" +
                StringUtils::FormatUnsignedIntegerWithCommas(
                    StringUtils::AbsToUnsigned(Value));

        return L"$" + StringUtils::FormatIntegerWithCommas(Value);
    }

    std::wstring FormatMoneyDollarFirst(long long Value)
    {
        if (Value < 0)
            return L"$-" +
                StringUtils::FormatUnsignedIntegerWithCommas(
                    StringUtils::AbsToUnsigned(Value));

        return L"$" + StringUtils::FormatIntegerWithCommas(Value);
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
            return UIStrings::Get(L"citizen.wealth.well_off") + L" 이상";
        case GBuildingWealthMaskRich | GBuildingWealthMaskFilthyRich:
            return UIStrings::Get(L"citizen.wealth.rich") + L" 이상";
        case GBuildingWealthMaskPoor |
            GBuildingWealthMaskWellOff |
            GBuildingWealthMaskRich |
            GBuildingWealthMaskFilthyRich:
            return UIStrings::Get(L"citizen.wealth.poor") + L" 이상";
        default:
            return std::wstring();
        }
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

}

using namespace CitizenInfoDataProviderInternal;
