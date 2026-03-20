#pragma once

#include "../Building/BuildingTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <array>
#include <vector>

namespace WorldStats
{
    struct FWorldStatsSnapshot;
}

namespace FactionDemandTuning
{
    struct FFactionDefinition
    {
        EPoliticalFaction Faction = EPoliticalFaction::Communists;
        const wchar_t* DisplayName = L"세력";
        const wchar_t* CompactLabel = L"세력";
        const wchar_t* WarningSummaryKey = L"escalation.warning.generic";
        EBuildingEra FirstAvailableEra = EBuildingEra::Modern;
    };

    struct FFactionDemandContext
    {
        EPoliticalFaction Faction = EPoliticalFaction::Communists;
        const WorldStats::FWorldStatsSnapshot& Snapshot;
        const FPoliticalWorldSnapshot& PoliticalSnapshot;
        const FGovernmentProfile& GovernmentProfile;
        long long LastDailyExportIncome = 0;
        EBuildingEra CurrentEra = EBuildingEra::Modern;
    };

    struct FPoliticalEscalationSummary
    {
        bool Active = false;
        bool Critical = false;
        EPoliticalDemandStage HighestStage = EPoliticalDemandStage::Demand;
        std::vector<EPoliticalFaction> Factions;
    };

    extern const std::array<FFactionDefinition, GPoliticalFactionCount>
        GFactionDefinitions;

    const FFactionDefinition& GetFactionDefinition(EPoliticalFaction Faction);
    const wchar_t* GetPoliticalFactionName(EPoliticalFaction Faction);
    const wchar_t* GetFactionCompactLabel(EPoliticalFaction Faction);
    const wchar_t* GetFactionWarningSummaryKey(EPoliticalFaction Faction);
    bool IsFactionAvailableInEra(
        EPoliticalFaction Faction,
        EBuildingEra Era);
    int ResolveFaithDemandTargetMax(EBuildingEra Era);
    bool TryBuildFactionDemand(
        const FFactionDemandContext& Context,
        FPoliticalDemandState& OutDemand,
        double& OutPriority);
    FPoliticalEscalationSummary BuildPoliticalEscalationSummary(
        const std::array<int, GPoliticalFactionCount>& PressureDays,
        const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
            DemandStates);
}
