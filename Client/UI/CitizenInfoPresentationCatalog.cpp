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
    std::wstring FormatInteger(long long Value)
    {
        return StringUtils::FormatIntegerWithCommas(Value);
    }

    std::wstring FormatMoney(long long Value)
    {
        return StringUtils::FormatCurrency(Value);
    }

    std::wstring FormatMoneyDollarFirst(long long Value)
    {
        return StringUtils::FormatCurrencyDollarFirst(Value);
    }

    std::wstring FormatCatalogCostValue(
        EBuildingCostState State,
        int Value)
    {
        switch (State)
        {
        case EBuildingCostState::Known:
            return Value <= 0 ?
                UIStrings::Get(L"citizen_info.value.free") :
                FormatMoney(Value);
        case EBuildingCostState::Unknown:
            return UIStrings::Get(L"citizen_info.value.unknown");
        case EBuildingCostState::None:
        default:
            return std::wstring();
        }
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

    std::wstring FormatFlowRateValue(float Value, const wchar_t* SuffixKey)
    {
        const float SafeValue = (std::max)(0.f, Value);
        const std::wstring Suffix = UIStrings::Get(SuffixKey);

        if (SafeValue < 0.05f)
            return L"0" + Suffix;

        const float RoundedValue = roundf(SafeValue * 10.f) / 10.f;
        wchar_t Buffer[64] = {};

        if (fabsf(RoundedValue - roundf(RoundedValue)) < 0.05f)
        {
            swprintf_s(
                Buffer,
                L"%d%s",
                static_cast<int>(roundf(RoundedValue)),
                Suffix.c_str());
        }
        else
        {
            swprintf_s(Buffer, L"%.1f%s", RoundedValue, Suffix.c_str());
        }

        return Buffer;
    }

    std::wstring FormatFlowVolumeValue(int Value, const wchar_t* SuffixKey)
    {
        return CitizenInfoPresentation::FormatInteger((std::max)(0, Value)) +
            UIStrings::Get(SuffixKey);
    }
}

namespace CitizenInfoPresentationInternal
{
    using FBuildingUiSnapshot = CitizenInfoBuildingRuntime::FBuildingUiSnapshot;
    using FProductionInputSlotView =
        CitizenInfoDataProvider::FProductionInputSlotView;
    using EProductionChainStage =
        CitizenInfoDataProvider::EProductionChainStage;
    using StringUtils::SplitLines;
    using StringUtils::Trim;

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
        const std::wstring* OverrideEffectSummary)
    {
        std::vector<std::wstring> Segments;

        if (UpgradeDef.HasUnlockEra)
            Segments.push_back(GetBuildingEraDisplayName(UpgradeDef.UnlockEra));

        const std::wstring CostText =
            CitizenInfoPresentation::FormatCatalogCostValue(
                UpgradeDef.CostState,
                UpgradeDef.Cost);

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
        const std::wstring* OverrideEffectSummary)
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

    bool HasLockedOperationModeResearch(const FBuildingUiSnapshot& Snapshot)
    {
        for (size_t Index = 0;
            Index < Snapshot.OperationModeResearchLocked.size();
            ++Index)
        {
            if (Snapshot.OperationModeResearchLocked[Index])
                return true;
        }

        return false;
    }

    std::wstring BuildKnowledgeSummaryText(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Result =
            L"연구 지식: " +
            StringUtils::FormatIntegerWithCommas(Snapshot.KnowledgePoints);

        if (Snapshot.DailyKnowledgeGeneration > 0)
        {
            Result += L" (+";
            Result += StringUtils::FormatIntegerWithCommas(
                Snapshot.DailyKnowledgeGeneration);
            Result += L"/일)";
        }

        return Result;
    }

    std::wstring BuildOperationModeResearchSuffix(
        const FBuildingUiSnapshot& Snapshot,
        size_t Index)
    {
        if (Index >= Snapshot.OperationModeResearchLocked.size() ||
            !Snapshot.OperationModeResearchLocked[Index])
        {
            return std::wstring();
        }

        const int ResearchCost =
            Index < Snapshot.OperationModeResearchCosts.size() ?
                (std::max)(0, Snapshot.OperationModeResearchCosts[Index]) :
                0;
        const bool CanAffordResearch =
            ResearchCost <= 0 || Snapshot.KnowledgePoints >= ResearchCost;
        std::wstring Result = L" [잠김";

        if (Index < Snapshot.OperationModeResearchLabels.size())
        {
            const std::wstring& ResearchLabel =
                Snapshot.OperationModeResearchLabels[Index];
            const std::wstring DefaultLabel =
                Index < Snapshot.OperationModes.size() ?
                    Snapshot.OperationModes[Index] + L" 연구" :
                    std::wstring();

            if (!ResearchLabel.empty() && ResearchLabel != DefaultLabel)
            {
                Result += L" | ";
                Result += ResearchLabel;
            }
        }

        if (ResearchCost > 0)
        {
            Result += L" | 지식 ";
            Result += StringUtils::FormatIntegerWithCommas(ResearchCost);
        }

        Result += CanAffordResearch ? L" | 즉시 해금]" : L" | 지식 부족]";
        return Result;
    }

}

