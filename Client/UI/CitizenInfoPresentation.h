#pragma once

#include "CitizenInfoBuildingRuntime.h"
#include <array>
#include <memory>
#include <string>

namespace CitizenInfoPresentation
{
    using FBuildingUiSnapshot = CitizenInfoBuildingRuntime::FBuildingUiSnapshot;

    std::wstring FormatInteger(long long Value);
    std::wstring FormatMoney(long long Value);
    std::wstring FormatMoneyDollarFirst(long long Value);
    std::wstring FormatMegawattValue(int Value);
    std::wstring FormatSignedMegawattValue(int Value);
    std::wstring FormatMultiplier(float Value);

    std::wstring NormalizeWealthRequirementText(const std::wstring& Value);
    int ResolveOverviewHousingQuality(const FBuildingUiSnapshot& Snapshot);
    long long ResolveOverviewMonthlyIncome(const FBuildingUiSnapshot& Snapshot);
    int ResolveOverviewRequiredPower(const FBuildingUiSnapshot& Snapshot);
    bool UseGenericBuildingWorkOverview(const FBuildingUiSnapshot& Snapshot);

    void PopulateResidentialOverview(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result);
    void PopulateCustomsWorkOverview(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result);
    void PopulateGenericWorkOverview(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result);
    void PopulateBuildingStatisticsMetrics(
        const FBuildingUiSnapshot& Snapshot,
        bool IsCustomsOffice,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result);
    void PopulateBuildingEfficiencyMetrics(
        const FBuildingUiSnapshot& Snapshot,
        bool IsCustomsOffice,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result);
    std::wstring ResolveBuildingPageTitle(
        const FBuildingUiSnapshot& Snapshot,
        int TabIndex,
        bool ShowCustomsModeSelection);
    bool HasOverviewMetrics(
        const CitizenInfoDataProvider::FCitizenInfoSnapshot& Snapshot);
    bool PopulateBuildingInformationPanel(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result);
    bool PopulateBuildingUpgradeCard(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result);

    std::wstring BuildOverviewBody(const FBuildingUiSnapshot& Snapshot);
    std::wstring BuildStatisticsBody(const FBuildingUiSnapshot& Snapshot);
    std::wstring BuildUpgradesBody(const FBuildingUiSnapshot& Snapshot);
    std::wstring BuildEfficiencyBody(const FBuildingUiSnapshot& Snapshot);
    std::wstring BuildInformationBody(const FBuildingUiSnapshot& Snapshot);
    std::wstring BuildCustomsModeSelectionBody(
        const FBuildingUiSnapshot& Snapshot);
    std::wstring BuildCustomsUpgradesBody(
        const FBuildingUiSnapshot& Snapshot);

    const wchar_t* GetCitizenProfileWealthDisplayName(
        ECitizenWealthLevel Level);
    const wchar_t* GetCitizenActivityDisplayName(ECitizenState State);
    std::wstring ResolveBuildingDisplayName(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>&
            QuerySource,
        const std::string& BuildingName);
    std::wstring ResolveCitizenLocationText(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>&
            QuerySource,
        const CitizenInfoDataProvider::FCitizenInfoCitizenRecord& Citizen);
    std::array<std::wstring, 3> BuildCitizenOpinionLines(
        const FNpcPoliticalProfile& PoliticalProfile);
    float BuildCitizenSupportRatio(
        const FNpcSatisfaction& Satisfaction);
}
