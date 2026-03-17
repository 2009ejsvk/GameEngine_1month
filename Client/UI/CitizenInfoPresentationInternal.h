#pragma once

#include "CitizenInfoBuildingRuntime.h"
#include "CitizenInfoPresentation.h"
#include <vector>

namespace CitizenInfoPresentationInternal
{
    using FBuildingUiSnapshot = CitizenInfoBuildingRuntime::FBuildingUiSnapshot;
    using FProductionInputSlotView =
        CitizenInfoDataProvider::FProductionInputSlotView;
    using EProductionChainStage =
        CitizenInfoDataProvider::EProductionChainStage;

    const std::wstring& Ui(const wchar_t* Key);
    const wchar_t* UiText(const wchar_t* Key);
    void AppendLine(std::wstring& Body, const std::wstring& Line);
    std::wstring JoinLines(const std::vector<std::wstring>& Lines);
    bool HasLockedOperationModeResearch(const FBuildingUiSnapshot& Snapshot);
    std::wstring BuildKnowledgeSummaryText(
        const FBuildingUiSnapshot& Snapshot);
    std::wstring BuildOperationModeResearchSuffix(
        const FBuildingUiSnapshot& Snapshot,
        size_t Index);
    std::wstring GetDamageLevelDisplayName(EBuildingDamageLevel Level);
    std::wstring FormatCatalogCostValue(
        EBuildingCostState State,
        int Value);
    const FBuildingRuntimeUpgradeDef* ResolveActiveRuntimeUpgradeDef(
        const FBuildingUiSnapshot& Snapshot);
    std::wstring BuildRuntimeUpgradeSummary(
        const FBuildingRuntimeUpgradeDef& UpgradeDef,
        const std::wstring* OverrideEffectSummary = nullptr);
    std::wstring BuildRuntimeUpgradeLine(
        const FBuildingRuntimeUpgradeDef& UpgradeDef,
        bool Active,
        const std::wstring* OverrideEffectSummary = nullptr);
    std::wstring FormatPowerCoverageValue(float Ratio);
    const wchar_t* GetServiceCapacityLabelKey(
        const FBuildingUiSnapshot& Snapshot);
    std::wstring ResolveRequiredPowerDisplayText(
        const FBuildingUiSnapshot& Snapshot);
    std::wstring ResolveProducedPowerDisplayText(
        const FBuildingUiSnapshot& Snapshot);
    bool HasProductionInputRecords(const FBuildingUiSnapshot& Snapshot);
    bool HasProductionFlowEstimate(const FBuildingUiSnapshot& Snapshot);
    void AppendProductionInputLines(
        std::wstring& Body,
        const FBuildingUiSnapshot& Snapshot);
    void AppendProductionFlowLines(
        std::wstring& Body,
        const FBuildingUiSnapshot& Snapshot);
    std::wstring ResolveSupplyChainSummaryText(
        const FBuildingUiSnapshot& Snapshot);
    std::wstring ResolveProductionChainStageText(
        const FBuildingUiSnapshot& Snapshot);
    float ResolveDisplayedEfficiencyRatio(
        const FBuildingUiSnapshot& Snapshot);
    std::wstring SummarizeNames(const std::vector<std::string>& Names);
    const wchar_t* GetHousingClassDisplayName(
        EBuildingHousingClass HousingClass);
    const wchar_t* GetLeisureClassDisplayName(
        EBuildingLeisureClass LeisureClass);
    std::wstring ResolveRoleSummary(const FBuildingUiSnapshot& Snapshot);
    void AppendKeyValue(
        std::wstring& Body,
        const wchar_t* Key,
        const std::wstring& Value);
    void AppendKeyValueByKey(
        std::wstring& Body,
        const wchar_t* LabelKey,
        const std::wstring& Value);
    std::wstring ResolveProducedResourceDisplayName(
        const FBuildingUiSnapshot& Snapshot);
    std::wstring ResolveProducedResourceStockLabel(
        const FBuildingUiSnapshot& Snapshot,
        const wchar_t* FallbackLabelKey);
    void AppendProducedResourceTradeLines(
        std::wstring& Body,
        const FBuildingUiSnapshot& Snapshot);
    void AppendHarborTradePriceReference(std::wstring& Body);
    void AppendHarborPolicyReference(
        std::wstring& Body,
        const FBuildingUiSnapshot& Snapshot);
    void AppendHarborPriorityReference(
        std::wstring& Body,
        const FBuildingUiSnapshot& Snapshot);
    const wchar_t* GetCitizenPoliticalIntensityDisplayName(
        EPoliticalAxis Axis,
        EPoliticalSupportLevel Support);
}
