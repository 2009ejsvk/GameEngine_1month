#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "MainWorldInfrastructureRuntime.h"
#include "MainWorldTradeRuntime.h"
#include "RuntimeConfigRegistry.h"
#include "WorldStatsSnapshot.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../Politics/EdictSystem.h"
#include "../Politics/PoliticsSystem.h"
#include "../Economy/EconomySystem.h"
#include "../Economy/ResourceTradePricing.h"
#include "../Economy/TradeDiplomacyRuntime.h"
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <string>
#include <vector>

namespace
{
    constexpr int GMaxActiveTradeRouteCount = 10;
    constexpr int GMaxCompletedTradeRouteRecordCount = 12;
    constexpr int GTradeRouteMinAmountUnits = 1000;
    constexpr int GTradeRouteMaxAmountUnits = 24000;
    constexpr int GTradeRouteMinDailyTransferUnits = 150;
    constexpr int GTradeRouteMaxDailyTransferUnits = 1200;
    constexpr int GTradeRouteDefaultDurationDays = 1500;
    constexpr int GMaxActiveFactionDemandCount = 2;
    constexpr int GMaxActiveForeignDemandCount = 1;
    constexpr int GFactionDemandCooldownDays = 90;
    constexpr int GForeignDemandCooldownDays = 105;
    constexpr int GFactionDemandModifierDurationDays = 120;
    constexpr int GDemandNoticeDurationDays = 10;
    constexpr int GCampaignPromiseLeadDays = 240;
    constexpr const wchar_t* GEraConfigId = L"MainWorld.EraUnlockRequirements";
    std::array<
        FEraUnlockRequirement,
        static_cast<size_t>(EBuildingEra::Modern) + 1> GEraUnlockRequirements;

    size_t GetEraRequirementIndex(EBuildingEra Era)
    {
        return static_cast<size_t>(Era);
    }

    void ResetEraUnlockRequirementsToDefaults()
    {
        GEraUnlockRequirements.fill(FEraUnlockRequirement());

        FEraUnlockRequirement& WorldWars =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::WorldWars)];
        WorldWars.MinPopulation = 40;
        WorldWars.MinTotalBuildings = 10;
        WorldWars.MinFoodProviders = 3;

        FEraUnlockRequirement& ColdWar =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::ColdWar)];
        ColdWar.MinPopulation = 80;
        ColdWar.MinTotalBuildings = 22;
        ColdWar.MinIndustryBuildings = 5;
        ColdWar.MinPowerMW = 25;

        FEraUnlockRequirement& Modern =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::Modern)];
        Modern.MinPopulation = 150;
        Modern.MinTotalBuildings = 38;
        Modern.MinPublicServiceBuildings = 6;
        Modern.MinEntertainmentBuildings = 5;
        Modern.MinPowerMW = 60;
    }

    bool LoadEraUnlockRequirementsFromFile(const std::wstring& Path)
    {
        auto LoadRequirement =
            [&](EBuildingEra Era, const wchar_t* Section)
            {
                FEraUnlockRequirement& Requirement =
                    GEraUnlockRequirements[GetEraRequirementIndex(Era)];
                Requirement.MinPopulation = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinPopulation",
                        Requirement.MinPopulation,
                        Path.c_str()));
                Requirement.MinTotalBuildings = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinTotalBuildings",
                        Requirement.MinTotalBuildings,
                        Path.c_str()));
                Requirement.MinFoodProviders = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinFoodProviders",
                        Requirement.MinFoodProviders,
                        Path.c_str()));
                Requirement.MinIndustryBuildings = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinIndustryBuildings",
                        Requirement.MinIndustryBuildings,
                        Path.c_str()));
                Requirement.MinPublicServiceBuildings = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinPublicServiceBuildings",
                        Requirement.MinPublicServiceBuildings,
                        Path.c_str()));
                Requirement.MinEntertainmentBuildings = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinEntertainmentBuildings",
                        Requirement.MinEntertainmentBuildings,
                        Path.c_str()));
                Requirement.MinPowerMW = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinPowerMW",
                        Requirement.MinPowerMW,
                        Path.c_str()));
            };

        LoadRequirement(EBuildingEra::WorldWars, L"WorldWars");
        LoadRequirement(EBuildingEra::ColdWar, L"ColdWar");
        LoadRequirement(EBuildingEra::Modern, L"Modern");
        return true;
    }

    void WriteEraUnlockRequirementsToFile(const std::wstring& Path)
    {
        const FEraUnlockRequirement& WorldWars =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::WorldWars)];
        const FEraUnlockRequirement& ColdWar =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::ColdWar)];
        const FEraUnlockRequirement& Modern =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::Modern)];

        const std::string Buffer =
            "; Runtime-tunable era unlock requirements.\r\n"
            "; Adjust values and save to hot-reload in-game.\r\n\r\n"
            "[WorldWars]\r\n"
            "MinPopulation=" + std::to_string(WorldWars.MinPopulation) +
            "\r\n"
            "MinTotalBuildings=" +
            std::to_string(WorldWars.MinTotalBuildings) + "\r\n"
            "MinFoodProviders=" +
            std::to_string(WorldWars.MinFoodProviders) + "\r\n\r\n"
            "[ColdWar]\r\n"
            "MinPopulation=" + std::to_string(ColdWar.MinPopulation) +
            "\r\n"
            "MinTotalBuildings=" +
            std::to_string(ColdWar.MinTotalBuildings) + "\r\n"
            "MinIndustryBuildings=" +
            std::to_string(ColdWar.MinIndustryBuildings) + "\r\n"
            "MinPowerMW=" + std::to_string(ColdWar.MinPowerMW) + "\r\n\r\n"
            "[Modern]\r\n"
            "MinPopulation=" + std::to_string(Modern.MinPopulation) +
            "\r\n"
            "MinTotalBuildings=" +
            std::to_string(Modern.MinTotalBuildings) + "\r\n"
            "MinPublicServiceBuildings=" +
            std::to_string(Modern.MinPublicServiceBuildings) + "\r\n"
            "MinEntertainmentBuildings=" +
            std::to_string(Modern.MinEntertainmentBuildings) + "\r\n"
            "MinPowerMW=" + std::to_string(Modern.MinPowerMW) + "\r\n";

        FILE* File = nullptr;

        if (_wfopen_s(&File, Path.c_str(), L"wb") != 0 || !File)
            return;

        static const unsigned char Bom[] = { 0xEF, 0xBB, 0xBF };
        fwrite(Bom, 1, sizeof(Bom), File);
        fwrite(Buffer.data(), 1, Buffer.size(), File);
        fclose(File);
    }

    void EnsureDefaultEraConfigFileExists(const std::wstring& Path)
    {
        const DWORD Attributes = GetFileAttributesW(Path.c_str());

        if (Attributes != INVALID_FILE_ATTRIBUTES &&
            (Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            return;
        }

        ResetEraUnlockRequirementsToDefaults();
        WriteEraUnlockRequirementsToFile(Path);
    }

    void EnsureEraRuntimeConfigRegistered()
    {
        if (RuntimeConfigRegistry::GetSourceGeneration(GEraConfigId) != 0)
            return;

        const std::wstring ConfigPath =
            RuntimeConfigRegistry::BuildExeRelativePath(L"EraConfig.ini");
        EnsureDefaultEraConfigFileExists(ConfigPath);
        RuntimeConfigRegistry::RegisterSource(
            {
                GEraConfigId,
                ConfigPath,
                0.5f,
                &ResetEraUnlockRequirementsToDefaults,
                &LoadEraUnlockRequirementsFromFile,
                nullptr
            });
    }

    const wchar_t* GetTradeForeignPowerName(int Index)
    {
        static const wchar_t* Names[TradeDiplomacyRuntime::GForeignPowerCount] =
        {
            L"중국",
            L"러시아",
            L"미국",
            L"중동",
            L"유럽연합"
        };

        if (Index < 0 ||
            Index >= TradeDiplomacyRuntime::GForeignPowerCount)
        {
            return L"해외";
        }

        return Names[Index];
    }

    std::wstring FormatTradeCurrency(long long Value)
    {
        const bool Negative = Value < 0;
        const unsigned long long AbsValue = Negative ?
            static_cast<unsigned long long>(-Value) :
            static_cast<unsigned long long>(Value);
        std::wstring Digits = std::to_wstring(AbsValue);

        for (int Index = static_cast<int>(Digits.size()) - 3;
            Index > 0;
            Index -= 3)
        {
            Digits.insert(static_cast<size_t>(Index), 1, L',');
        }

        if (Negative)
            Digits.insert(Digits.begin(), L'-');

        return L"$" + Digits;
    }

    std::wstring FormatTradeUnits(int Value)
    {
        std::wstring Digits = std::to_wstring((std::max)(0, Value));

        for (int Index = static_cast<int>(Digits.size()) - 3;
            Index > 0;
            Index -= 3)
        {
            Digits.insert(static_cast<size_t>(Index), 1, L',');
        }

        return Digits;
    }

    const wchar_t* GetPoliticalFactionName(EPoliticalFaction Faction)
    {
        switch (Faction)
        {
        case EPoliticalFaction::Communists:
            return L"공산주의자";
        case EPoliticalFaction::Capitalists:
            return L"자본가";
        case EPoliticalFaction::Religious:
            return L"종교인";
        case EPoliticalFaction::Militarists:
            return L"군부";
        case EPoliticalFaction::Environmentalists:
            return L"환경주의자";
        case EPoliticalFaction::Industrialists:
            return L"산업주의자";
        case EPoliticalFaction::Intellectuals:
            return L"지식인";
        case EPoliticalFaction::Conservatives:
            return L"보수주의자";
        default:
            return L"세력";
        }
    }

    std::wstring FormatSignedInt(int Value)
    {
        if (Value > 0)
            return L"+" + std::to_wstring(Value);

        return std::to_wstring(Value);
    }

    std::wstring FormatBudgetDelta(long long Value)
    {
        if (Value > 0)
            return L"+" + MainWorldTradeRuntime::FormatCurrency(Value);
        if (Value < 0)
            return L"-" + MainWorldTradeRuntime::FormatCurrency(-Value);

        return L"$0";
    }

    std::wstring BuildPoliticalDemandEffectText(
        long long BudgetDelta,
        int FactionApprovalDelta,
        int RelationDelta,
        int StandingDelta,
        int DurationDays)
    {
        std::wstring Result;
        const auto AppendPart =
            [&](const std::wstring& Part)
            {
                if (Part.empty())
                    return;

                if (!Result.empty())
                    Result += L" / ";

                Result += Part;
            };

        if (BudgetDelta != 0)
            AppendPart(L"예산 " + FormatBudgetDelta(BudgetDelta));

        if (FactionApprovalDelta != 0)
        {
            std::wstring Part =
                L"승인도 " + FormatSignedInt(FactionApprovalDelta);

            if (DurationDays > 0)
                Part += L" (" + std::to_wstring(DurationDays) + L"일)";

            AppendPart(Part);
        }

        if (RelationDelta != 0)
            AppendPart(L"관계 " + FormatSignedInt(RelationDelta));

        if (StandingDelta != 0)
            AppendPart(L"standing " + FormatSignedInt(StandingDelta));

        if (Result.empty())
            Result = L"직접 변화 없음";

        return Result;
    }

    int CountActiveTradeRoutesForPower(
        const std::vector<FTradeRouteRuntimeState>& ActiveRoutes,
        int ForeignPowerIndex)
    {
        int Count = 0;

        for (size_t Index = 0; Index < ActiveRoutes.size(); ++Index)
        {
            if (ActiveRoutes[Index].ForeignPowerIndex == ForeignPowerIndex)
                ++Count;
        }

        return Count;
    }

    int CountActiveFactionDemands(
        const std::array<FPoliticalDemandState, GPoliticalFactionCount>& Demands)
    {
        int Count = 0;

        for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
        {
            if (Demands[static_cast<size_t>(Index)].Active)
                ++Count;
        }

        return Count;
    }

    int CountActiveForeignDemands(
        const std::array<
            FPoliticalDemandState,
            TradeDiplomacyRuntime::GForeignPowerCount>& Demands)
    {
        int Count = 0;

        for (int Index = 0;
            Index < TradeDiplomacyRuntime::GForeignPowerCount;
            ++Index)
        {
            if (Demands[static_cast<size_t>(Index)].Active)
                ++Count;
        }

        return Count;
    }

    int EvaluatePoliticalDemandCurrentValue(
        const FPoliticalDemandState& Demand,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FGovernmentProfile& GovernmentProfile,
        long long LastDailyExportIncome,
        const std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount>& ForeignPowerStates,
        const std::vector<FTradeRouteRuntimeState>& ActiveTradeRoutes)
    {
        switch (Demand.ObjectiveType)
        {
        case EPoliticalDemandObjectiveType::Housing:
            return static_cast<int>(std::lround(Snapshot.AverageHousing));
        case EPoliticalDemandObjectiveType::Food:
            return static_cast<int>(std::lround(Snapshot.AverageFood));
        case EPoliticalDemandObjectiveType::Faith:
            return static_cast<int>(std::lround(Snapshot.AverageFaith));
        case EPoliticalDemandObjectiveType::Security:
            return static_cast<int>(std::lround(Snapshot.AverageSecurity));
        case EPoliticalDemandObjectiveType::Freedom:
            return static_cast<int>(std::lround(Snapshot.AverageFreedom));
        case EPoliticalDemandObjectiveType::Health:
            return static_cast<int>(std::lround(Snapshot.AverageHealth));
        case EPoliticalDemandObjectiveType::ExportIncome:
            return (std::max)(0, static_cast<int>(LastDailyExportIncome));
        case EPoliticalDemandObjectiveType::IncomeTaxCeiling:
            return GovernmentProfile.TaxPolicy.IncomeRatePercent;
        case EPoliticalDemandObjectiveType::PropertyTaxCeiling:
            return GovernmentProfile.TaxPolicy.PropertyRatePercent;
        case EPoliticalDemandObjectiveType::ActiveTradeRoutes:
            if (Demand.IssuerType == EPoliticalDemandIssuerType::ForeignPower &&
                Demand.IssuerIndex >= 0 &&
                Demand.IssuerIndex < TradeDiplomacyRuntime::GForeignPowerCount)
            {
                return MainWorldTradeRuntime::CountActiveTradeRoutesForPower(
                    ActiveTradeRoutes,
                    Demand.IssuerIndex);
            }

            return static_cast<int>(ActiveTradeRoutes.size());
        case EPoliticalDemandObjectiveType::None:
        default:
            break;
        }

        (void)ForeignPowerStates;
        return 0;
    }

    bool IsPoliticalDemandSatisfied(const FPoliticalDemandState& Demand)
    {
        switch (Demand.ObjectiveType)
        {
        case EPoliticalDemandObjectiveType::IncomeTaxCeiling:
        case EPoliticalDemandObjectiveType::PropertyTaxCeiling:
            return Demand.CurrentValue <= Demand.TargetValue;
        case EPoliticalDemandObjectiveType::Housing:
        case EPoliticalDemandObjectiveType::Food:
        case EPoliticalDemandObjectiveType::Faith:
        case EPoliticalDemandObjectiveType::Security:
        case EPoliticalDemandObjectiveType::Freedom:
        case EPoliticalDemandObjectiveType::Health:
        case EPoliticalDemandObjectiveType::ExportIncome:
        case EPoliticalDemandObjectiveType::ActiveTradeRoutes:
            return Demand.CurrentValue >= Demand.TargetValue;
        case EPoliticalDemandObjectiveType::None:
        default:
            return false;
        }
    }

    struct FElectionPromiseCandidate
    {
        FElectionPromiseState Promise;
        double Priority = 0.0;
    };

    const wchar_t* GetElectionPromiseName(EElectionPromiseType Type)
    {
        switch (Type)
        {
        case EElectionPromiseType::Housing:
            return L"주거 개선";
        case EElectionPromiseType::Food:
            return L"식량 안정";
        case EElectionPromiseType::Health:
            return L"보건 확충";
        case EElectionPromiseType::Job:
            return L"고용 확대";
        case EElectionPromiseType::Freedom:
            return L"자유 확대";
        case EElectionPromiseType::Security:
            return L"치안 강화";
        case EElectionPromiseType::Faith:
            return L"신앙 지원";
        case EElectionPromiseType::ExportIncome:
            return L"수출 확대";
        case EElectionPromiseType::None:
        default:
            return L"공약";
        }
    }

    int EvaluateElectionPromiseCurrentValue(
        const FElectionPromiseState& Promise,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        long long LastDailyExportIncome)
    {
        switch (Promise.Type)
        {
        case EElectionPromiseType::Housing:
            return static_cast<int>(std::lround(Snapshot.AverageHousing));
        case EElectionPromiseType::Food:
            return static_cast<int>(std::lround(Snapshot.AverageFood));
        case EElectionPromiseType::Health:
            return static_cast<int>(std::lround(Snapshot.AverageHealth));
        case EElectionPromiseType::Job:
            return static_cast<int>(std::lround(Snapshot.AverageJob));
        case EElectionPromiseType::Freedom:
            return static_cast<int>(std::lround(Snapshot.AverageFreedom));
        case EElectionPromiseType::Security:
            return static_cast<int>(std::lround(Snapshot.AverageSecurity));
        case EElectionPromiseType::Faith:
            return static_cast<int>(std::lround(Snapshot.AverageFaith));
        case EElectionPromiseType::ExportIncome:
            return (std::max)(0, static_cast<int>(LastDailyExportIncome));
        case EElectionPromiseType::None:
        default:
            return 0;
        }
    }

    FElectionPromiseState BuildElectionPromiseState(
        EElectionPromiseType Type,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        long long LastDailyExportIncome)
    {
        FElectionPromiseState Promise;
        Promise.Active = true;
        Promise.Type = Type;
        Promise.Title = std::wstring(GetElectionPromiseName(Type)) + L" 공약";
        Promise.BaselineValue =
            EvaluateElectionPromiseCurrentValue(
                Promise,
                Snapshot,
                LastDailyExportIncome);
        Promise.CurrentValue = Promise.BaselineValue;

        switch (Type)
        {
        case EElectionPromiseType::Housing:
            Promise.TargetValue = (std::min)(100, Promise.BaselineValue + 8);
            Promise.SuccessVoteModifierPercent = 4;
            Promise.FailureVoteModifierPercent = 5;
            Promise.Summary =
                L"평균 주거 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        case EElectionPromiseType::Food:
            Promise.TargetValue = (std::min)(100, Promise.BaselineValue + 7);
            Promise.SuccessVoteModifierPercent = 3;
            Promise.FailureVoteModifierPercent = 4;
            Promise.Summary =
                L"평균 식량 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        case EElectionPromiseType::Health:
            Promise.TargetValue = (std::min)(100, Promise.BaselineValue + 7);
            Promise.SuccessVoteModifierPercent = 3;
            Promise.FailureVoteModifierPercent = 4;
            Promise.Summary =
                L"평균 보건 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        case EElectionPromiseType::Job:
            Promise.TargetValue = (std::min)(100, Promise.BaselineValue + 8);
            Promise.SuccessVoteModifierPercent = 4;
            Promise.FailureVoteModifierPercent = 5;
            Promise.Summary =
                L"평균 직업 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        case EElectionPromiseType::Freedom:
            Promise.TargetValue = (std::min)(100, Promise.BaselineValue + 8);
            Promise.SuccessVoteModifierPercent = 3;
            Promise.FailureVoteModifierPercent = 4;
            Promise.Summary =
                L"평균 자유 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        case EElectionPromiseType::Security:
            Promise.TargetValue = (std::min)(100, Promise.BaselineValue + 8);
            Promise.SuccessVoteModifierPercent = 3;
            Promise.FailureVoteModifierPercent = 4;
            Promise.Summary =
                L"평균 치안 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        case EElectionPromiseType::Faith:
            Promise.TargetValue = (std::min)(100, Promise.BaselineValue + 7);
            Promise.SuccessVoteModifierPercent = 3;
            Promise.FailureVoteModifierPercent = 4;
            Promise.Summary =
                L"평균 신앙 " +
                std::to_wstring(Promise.TargetValue) +
                L" 이상";
            break;
        case EElectionPromiseType::ExportIncome:
        {
            const int BaselineIncome = (std::max)(0, Promise.BaselineValue);
            Promise.TargetValue = BaselineIncome +
                (std::max)(1400, BaselineIncome / 5);
            Promise.SuccessVoteModifierPercent = 3;
            Promise.FailureVoteModifierPercent = 4;
            Promise.Summary =
                L"일일 수출 " +
                MainWorldTradeRuntime::FormatCurrency(Promise.TargetValue) +
                L" 이상";
            break;
        }
        case EElectionPromiseType::None:
        default:
            Promise.Active = false;
            break;
        }

        return Promise;
    }

    double EvaluateElectionPromisePriority(
        EElectionPromiseType Type,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        long long LastDailyExportIncome)
    {
        const double CitizenCount =
            static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount));
        const double UnemploymentRatio =
            static_cast<double>(Snapshot.UnemployedCount) / CitizenCount;

        switch (Type)
        {
        case EElectionPromiseType::Housing:
            return
                (std::max)(0.0, 62.0 - Snapshot.AverageHousing) * 1.25 +
                static_cast<double>(Snapshot.HomelessHouseholdCount) * 2.6;
        case EElectionPromiseType::Food:
            return (std::max)(0.0, 60.0 - Snapshot.AverageFood) * 1.20;
        case EElectionPromiseType::Health:
            return (std::max)(0.0, 60.0 - Snapshot.AverageHealth) * 1.15;
        case EElectionPromiseType::Job:
            return
                (std::max)(0.0, 60.0 - Snapshot.AverageJob) * 1.05 +
                UnemploymentRatio * 55.0;
        case EElectionPromiseType::Freedom:
            return (std::max)(0.0, 60.0 - Snapshot.AverageFreedom) * 1.10;
        case EElectionPromiseType::Security:
            return (std::max)(0.0, 60.0 - Snapshot.AverageSecurity) * 1.10;
        case EElectionPromiseType::Faith:
            return (std::max)(0.0, 58.0 - Snapshot.AverageFaith) * 1.05;
        case EElectionPromiseType::ExportIncome:
            return static_cast<double>((std::max)(
                0,
                5600 - static_cast<int>(LastDailyExportIncome))) / 240.0;
        case EElectionPromiseType::None:
        default:
            return 0.0;
        }
    }

    int CountActiveElectionPromises(const FElectionStatus& ElectionStatus)
    {
        int Count = 0;

        for (int Index = 0; Index < GElectionPromiseCount; ++Index)
        {
            if (ElectionStatus.ActivePromises[static_cast<size_t>(Index)].Active)
                ++Count;
        }

        return Count;
    }

    bool TryBuildFactionDemand(
        EPoliticalFaction Faction,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FPoliticalWorldSnapshot& PoliticalSnapshot,
        const FGovernmentProfile& GovernmentProfile,
        long long LastDailyExportIncome,
        FPoliticalDemandState& OutDemand,
        double& OutPriority)
    {
        const int FactionIndex = static_cast<int>(Faction);
        const auto& FactionSnapshot =
            PoliticalSnapshot.Factions[static_cast<size_t>(FactionIndex)];

        if (FactionSnapshot.MemberCount < 6 ||
            FactionSnapshot.AverageApproval >= 60.0)
        {
            return false;
        }

        const double ApprovalPressure =
            (std::max)(0.0, 60.0 - FactionSnapshot.AverageApproval);
        FPoliticalDemandState Demand;
        Demand.Active = true;
        Demand.IssuerType = EPoliticalDemandIssuerType::Faction;
        Demand.IssuerIndex = FactionIndex;
        Demand.Status = EPoliticalDemandStatus::PendingResponse;
        Demand.DurationDays = 90;
        Demand.RemainingDays = Demand.DurationDays;
        Demand.ModifierDurationDays = GFactionDemandModifierDurationDays;
        Demand.PenaltyBudgetDelta = 0;
        Demand.Title = std::wstring(GetPoliticalFactionName(Faction)) + L" 요구";
        OutPriority = 0.0;

        switch (Faction)
        {
        case EPoliticalFaction::Communists:
        {
            const int CurrentHousing =
                static_cast<int>(std::lround(Snapshot.AverageHousing));
            if (CurrentHousing >= 58 && Snapshot.HomelessHouseholdCount <= 1)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Housing;
            Demand.TargetValue = (std::max)(
                58,
                (std::min)(72, CurrentHousing + 10));
            Demand.CurrentValue = CurrentHousing;
            Demand.Summary = L"무주택과 저질 주거를 줄이라고 압박합니다.";
            Demand.ObjectiveText =
                L"평균 주거 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            Demand.RewardBudgetDelta =
                1200 + static_cast<long long>(FactionSnapshot.MemberCount) * 18LL;
            Demand.RewardFactionApprovalDelta = 9;
            Demand.PenaltyFactionApprovalDelta = -12;
            OutPriority =
                ApprovalPressure * 1.2 +
                (std::max)(0, 58 - CurrentHousing) +
                Snapshot.HomelessHouseholdCount * 1.6;
            break;
        }
        case EPoliticalFaction::Capitalists:
        {
            const int CurrentIncomeTax =
                GovernmentProfile.TaxPolicy.IncomeRatePercent;
            if (CurrentIncomeTax <= 12)
                return false;

            Demand.ObjectiveType =
                EPoliticalDemandObjectiveType::IncomeTaxCeiling;
            Demand.TargetValue = 12;
            Demand.CurrentValue = CurrentIncomeTax;
            Demand.Summary = L"소득세 인하와 투자 여건 개선을 요구합니다.";
            Demand.ObjectiveText = L"소득세 12% 이하";
            Demand.RewardBudgetDelta = 2000;
            Demand.RewardFactionApprovalDelta = 8;
            Demand.PenaltyFactionApprovalDelta = -10;
            OutPriority =
                ApprovalPressure * 1.1 +
                static_cast<double>(CurrentIncomeTax - Demand.TargetValue) * 1.8;
            break;
        }
        case EPoliticalFaction::Religious:
        {
            const int CurrentFaith =
                static_cast<int>(std::lround(Snapshot.AverageFaith));
            if (CurrentFaith >= 58)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Faith;
            Demand.TargetValue = (std::max)(58, (std::min)(72, CurrentFaith + 10));
            Demand.CurrentValue = CurrentFaith;
            Demand.Summary = L"신앙 만족도 회복과 종교 서비스 강화를 요구합니다.";
            Demand.ObjectiveText =
                L"평균 신앙 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            Demand.RewardBudgetDelta = 1500;
            Demand.RewardFactionApprovalDelta = 8;
            Demand.PenaltyFactionApprovalDelta = -11;
            OutPriority =
                ApprovalPressure * 1.1 + (std::max)(0, 58 - CurrentFaith) * 1.2;
            break;
        }
        case EPoliticalFaction::Militarists:
        {
            const int CurrentSecurity =
                static_cast<int>(std::lround(Snapshot.AverageSecurity));
            if (CurrentSecurity >= 60)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Security;
            Demand.TargetValue =
                (std::max)(60, (std::min)(76, CurrentSecurity + 10));
            Demand.CurrentValue = CurrentSecurity;
            Demand.Summary = L"치안 안정과 군사 통제를 강화하라고 압박합니다.";
            Demand.ObjectiveText =
                L"평균 치안 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            Demand.RewardBudgetDelta = 1700;
            Demand.RewardFactionApprovalDelta = 8;
            Demand.PenaltyFactionApprovalDelta = -11;
            OutPriority =
                ApprovalPressure * 1.1 +
                (std::max)(0, 60 - CurrentSecurity) * 1.3;
            break;
        }
        case EPoliticalFaction::Environmentalists:
        {
            const int CurrentHealth =
                static_cast<int>(std::lround(Snapshot.AverageHealth));
            if (CurrentHealth >= 60)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Health;
            Demand.TargetValue =
                (std::max)(60, (std::min)(74, CurrentHealth + 9));
            Demand.CurrentValue = CurrentHealth;
            Demand.Summary = L"보건과 환경 악화를 바로잡으라고 요구합니다.";
            Demand.ObjectiveText =
                L"평균 보건 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            Demand.RewardBudgetDelta = 1600;
            Demand.RewardFactionApprovalDelta = 9;
            Demand.PenaltyFactionApprovalDelta = -12;
            OutPriority =
                ApprovalPressure * 1.1 +
                (std::max)(0, 60 - CurrentHealth) * 1.2;
            break;
        }
        case EPoliticalFaction::Industrialists:
        {
            const int CurrentExportIncome =
                (std::max)(0, static_cast<int>(LastDailyExportIncome));
            if (CurrentExportIncome >= 5200)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::ExportIncome;
            Demand.TargetValue = (std::max)(5200, CurrentExportIncome + 1800);
            Demand.CurrentValue = CurrentExportIncome;
            Demand.Summary = L"산업 생산을 끌어올려 수출 실적을 내라고 요구합니다.";
            Demand.ObjectiveText =
                L"일일 수출 " +
                MainWorldTradeRuntime::FormatCurrency(Demand.TargetValue) +
                L" 이상";
            Demand.RewardBudgetDelta = 2200;
            Demand.RewardFactionApprovalDelta = 8;
            Demand.PenaltyFactionApprovalDelta = -10;
            OutPriority =
                ApprovalPressure * 1.0 +
                static_cast<double>((std::max)(0, Demand.TargetValue - CurrentExportIncome)) /
                    700.0;
            break;
        }
        case EPoliticalFaction::Intellectuals:
        {
            const int CurrentFreedom =
                static_cast<int>(std::lround(Snapshot.AverageFreedom));
            if (CurrentFreedom >= 60)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Freedom;
            Demand.TargetValue =
                (std::max)(60, (std::min)(76, CurrentFreedom + 10));
            Demand.CurrentValue = CurrentFreedom;
            Demand.Summary = L"자유와 개방성을 회복하라고 요구합니다.";
            Demand.ObjectiveText =
                L"평균 자유 " + std::to_wstring(Demand.TargetValue) + L" 이상";
            Demand.RewardBudgetDelta = 1500;
            Demand.RewardFactionApprovalDelta = 9;
            Demand.PenaltyFactionApprovalDelta = -12;
            OutPriority =
                ApprovalPressure * 1.15 +
                (std::max)(0, 60 - CurrentFreedom) * 1.3;
            break;
        }
        case EPoliticalFaction::Conservatives:
        {
            const int CurrentPropertyTax =
                GovernmentProfile.TaxPolicy.PropertyRatePercent;
            if (CurrentPropertyTax <= 35)
                return false;

            Demand.ObjectiveType =
                EPoliticalDemandObjectiveType::PropertyTaxCeiling;
            Demand.TargetValue = 35;
            Demand.CurrentValue = CurrentPropertyTax;
            Demand.Summary = L"재산세 완화와 질서 회복을 동시에 요구합니다.";
            Demand.ObjectiveText = L"재산세 35% 이하";
            Demand.RewardBudgetDelta = 1800;
            Demand.RewardFactionApprovalDelta = 8;
            Demand.PenaltyFactionApprovalDelta = -10;
            OutPriority =
                ApprovalPressure * 1.0 +
                static_cast<double>(CurrentPropertyTax - Demand.TargetValue) * 1.5;
            break;
        }
        default:
            return false;
        }

        if (Demand.ObjectiveType == EPoliticalDemandObjectiveType::None)
            return false;

        Demand.RewardText = BuildPoliticalDemandEffectText(
            Demand.RewardBudgetDelta,
            Demand.RewardFactionApprovalDelta,
            0,
            0,
            Demand.ModifierDurationDays);
        Demand.PenaltyText = BuildPoliticalDemandEffectText(
            Demand.PenaltyBudgetDelta,
            Demand.PenaltyFactionApprovalDelta,
            0,
            0,
            Demand.ModifierDurationDays);
        OutDemand = std::move(Demand);
        return OutPriority > 0.0;
    }

    bool TryBuildForeignDemand(
        int ForeignPowerIndex,
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const std::array<
            TradeDiplomacyRuntime::FForeignPowerWorldState,
            TradeDiplomacyRuntime::GForeignPowerCount>& ForeignPowerStates,
        const std::vector<FTradeRouteRuntimeState>& ActiveTradeRoutes,
        FPoliticalDemandState& OutDemand,
        double& OutPriority)
    {
        if (ForeignPowerIndex < 0 ||
            ForeignPowerIndex >= TradeDiplomacyRuntime::GForeignPowerCount)
        {
            return false;
        }

        const auto& ForeignState =
            ForeignPowerStates[static_cast<size_t>(ForeignPowerIndex)];
        FPoliticalDemandState Demand;
        Demand.Active = true;
        Demand.IssuerType = EPoliticalDemandIssuerType::ForeignPower;
        Demand.IssuerIndex = ForeignPowerIndex;
        Demand.Status = EPoliticalDemandStatus::PendingResponse;
        Demand.DurationDays = 105;
        Demand.RemainingDays = Demand.DurationDays;
        Demand.Title =
            std::wstring(MainWorldTradeRuntime::GetForeignPowerName(ForeignPowerIndex)) +
            L" 요구";
        Demand.RewardBudgetDelta = 2400;
        Demand.PenaltyBudgetDelta = 0;
        Demand.RewardForeignRelationDelta = 9;
        Demand.RewardForeignStandingDelta = 4;
        Demand.PenaltyForeignRelationDelta = -10;
        Demand.PenaltyForeignStandingDelta = -4;
        OutPriority =
            static_cast<double>((std::max)(0, 72 - ForeignState.Relation)) * 0.7 +
            static_cast<double>((std::max)(0, 10 - ForeignState.Standing)) * 0.8;

        switch (ForeignPowerIndex)
        {
        case 0:
        {
            const int ActiveRouteCount =
                MainWorldTradeRuntime::CountActiveTradeRoutesForPower(
                    ActiveTradeRoutes,
                    ForeignPowerIndex);
            if (ActiveRouteCount >= 2)
                return false;

            Demand.ObjectiveType =
                EPoliticalDemandObjectiveType::ActiveTradeRoutes;
            Demand.TargetValue = 2;
            Demand.CurrentValue = ActiveRouteCount;
            Demand.Summary = L"무역 계약을 늘려 교역 규모를 키우라고 요구합니다.";
            Demand.ObjectiveText = L"중국과 활성 무역로 2개 이상";
            OutPriority += static_cast<double>(Demand.TargetValue - ActiveRouteCount) * 6.0;
            break;
        }
        case 1:
        {
            const int CurrentSecurity =
                static_cast<int>(std::lround(Snapshot.AverageSecurity));
            if (CurrentSecurity >= 60)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Security;
            Demand.TargetValue = 60;
            Demand.CurrentValue = CurrentSecurity;
            Demand.Summary = L"군사 협력을 위해 치안 확보를 요구합니다.";
            Demand.ObjectiveText = L"평균 치안 60 이상";
            OutPriority += static_cast<double>((std::max)(0, 60 - CurrentSecurity)) * 1.4;
            break;
        }
        case 2:
        {
            const int CurrentFreedom =
                static_cast<int>(std::lround(Snapshot.AverageFreedom));
            if (CurrentFreedom >= 60)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Freedom;
            Demand.TargetValue = 60;
            Demand.CurrentValue = CurrentFreedom;
            Demand.Summary = L"대외 협력 조건으로 자유 확대를 요구합니다.";
            Demand.ObjectiveText = L"평균 자유 60 이상";
            OutPriority += static_cast<double>((std::max)(0, 60 - CurrentFreedom)) * 1.4;
            break;
        }
        case 3:
        {
            const int CurrentFaith =
                static_cast<int>(std::lround(Snapshot.AverageFaith));
            if (CurrentFaith >= 58)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Faith;
            Demand.TargetValue = 58;
            Demand.CurrentValue = CurrentFaith;
            Demand.Summary = L"종교 기반 안정을 위해 신앙 서비스를 요구합니다.";
            Demand.ObjectiveText = L"평균 신앙 58 이상";
            OutPriority += static_cast<double>((std::max)(0, 58 - CurrentFaith)) * 1.3;
            break;
        }
        case 4:
        default:
        {
            const int CurrentHealth =
                static_cast<int>(std::lround(Snapshot.AverageHealth));
            if (CurrentHealth >= 60)
                return false;

            Demand.ObjectiveType = EPoliticalDemandObjectiveType::Health;
            Demand.TargetValue = 60;
            Demand.CurrentValue = CurrentHealth;
            Demand.Summary = L"협력 유지를 위해 보건 수준 개선을 요구합니다.";
            Demand.ObjectiveText = L"평균 보건 60 이상";
            OutPriority += static_cast<double>((std::max)(0, 60 - CurrentHealth)) * 1.4;
            break;
        }
        }

        Demand.RewardText = BuildPoliticalDemandEffectText(
            Demand.RewardBudgetDelta,
            0,
            Demand.RewardForeignRelationDelta,
            Demand.RewardForeignStandingDelta,
            0);
        Demand.PenaltyText = BuildPoliticalDemandEffectText(
            Demand.PenaltyBudgetDelta,
            0,
            Demand.PenaltyForeignRelationDelta,
            Demand.PenaltyForeignStandingDelta,
            0);
        OutDemand = std::move(Demand);
        return OutPriority > 0.0;
    }

    void ApplyPoliticalDemandBudgetDelta(
        long long Delta,
        long long& InOutBudget,
        long long& InOutLastDailyNetChange)
    {
        if (Delta == 0)
            return;

        InOutBudget += Delta;
        InOutLastDailyNetChange += Delta;
    }

    void ApplyForeignDemandStandingDelta(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState,
        int RelationDelta,
        int StandingDelta)
    {
        InOutState.LastRelationChange = RelationDelta;
        InOutState.LastStandingChange = StandingDelta;
        InOutState.RelationModifier = TradeDiplomacyRuntime::ClampInt(
            InOutState.RelationModifier + RelationDelta,
            -35,
            35);
        InOutState.Standing = TradeDiplomacyRuntime::ClampStanding(
            InOutState.Standing + StandingDelta);
        InOutState.IdleDays = 0;
    }

    void SetPoliticalDemandResolutionNotice(
        FPoliticalDemandNotice& OutNotice,
        const FPoliticalDemandState& Demand,
        bool Positive,
        const wchar_t* StatusLabel)
    {
        OutNotice = FPoliticalDemandNotice();
        OutNotice.ActiveDemand = false;
        OutNotice.Positive = Positive;
        OutNotice.RemainingDays = GDemandNoticeDurationDays;
        OutNotice.Title = Demand.Title;
        OutNotice.Summary =
            std::wstring(StatusLabel ? StatusLabel : L"처리") +
            L" / " +
            (Positive ? Demand.RewardText : Demand.PenaltyText);
    }

    FPoliticalDemandNotice BuildPriorityDemandNotice(
        const std::array<FPoliticalDemandState, GPoliticalFactionCount>&
            FactionDemands,
        const std::array<
            FPoliticalDemandState,
            TradeDiplomacyRuntime::GForeignPowerCount>& ForeignDemands)
    {
        const FPoliticalDemandState* BestDemand = nullptr;

        const auto ConsiderDemand =
            [&](const FPoliticalDemandState& Demand)
            {
                if (!Demand.Active)
                    return;

                if (!BestDemand)
                {
                    BestDemand = &Demand;
                    return;
                }

                const bool DemandPending =
                    Demand.Status == EPoliticalDemandStatus::PendingResponse;
                const bool BestPending =
                    BestDemand->Status == EPoliticalDemandStatus::PendingResponse;

                if (DemandPending != BestPending)
                {
                    if (DemandPending)
                        BestDemand = &Demand;
                    return;
                }

                if (Demand.RemainingDays < BestDemand->RemainingDays)
                {
                    BestDemand = &Demand;
                    return;
                }

                if (Demand.RemainingDays == BestDemand->RemainingDays &&
                    Demand.Title < BestDemand->Title)
                {
                    BestDemand = &Demand;
                }
            };

        for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
        {
            ConsiderDemand(FactionDemands[static_cast<size_t>(Index)]);
        }

        for (int Index = 0;
            Index < TradeDiplomacyRuntime::GForeignPowerCount;
            ++Index)
        {
            ConsiderDemand(ForeignDemands[static_cast<size_t>(Index)]);
        }

        if (!BestDemand)
            return FPoliticalDemandNotice();

        FPoliticalDemandNotice Notice;
        Notice.ActiveDemand = true;
        Notice.Positive = false;
        Notice.RemainingDays = BestDemand->RemainingDays;
        Notice.Title = BestDemand->Title;
        Notice.Summary =
            (BestDemand->Status == EPoliticalDemandStatus::Accepted ?
                L"수행 중 / " :
                L"응답 대기 / ") +
            BestDemand->ObjectiveText +
            L" / " +
            std::to_wstring((std::max)(0, BestDemand->RemainingDays)) +
            L"일";
        return Notice;
    }

    int ComputeTradeRouteSignedStandardPricePerThousand(EResourceType Type, bool ImportRoute)
    {
        const int PricePerUnit = ImportRoute ?
            ResourceTradePricing::GetImportPricePerStockUnit(Type) :
            ResourceTradePricing::GetExportPricePerStockUnit(Type);
        return (std::max)(1000, PricePerUnit * 1000);
    }

    int ResolveTradeRouteDurationDays(int ContractUnits)
    {
        const int CargoScaleDays =
            (std::max)(0, ContractUnits / 20);
        return (std::max)(
            540,
            (std::min)(
                GTradeRouteDefaultDurationDays,
                420 + CargoScaleDays));
    }

    int ResolveTradeRouteDailyTransferUnits(
        const FTradeRouteRuntimeState& Route)
    {
        const int RemainingUnits = (std::max)(
            0,
            Route.ContractUnits - Route.FulfilledUnits);
        const int RemainingDays = (std::max)(1, Route.RemainingDays);
        const int TimedTarget = static_cast<int>(std::ceil(
            static_cast<double>(RemainingUnits) /
            static_cast<double>((std::min)(RemainingDays, 120))));

        return (std::max)(
            GTradeRouteMinDailyTransferUnits,
            (std::min)(
                GTradeRouteMaxDailyTransferUnits,
                TimedTarget));
    }

    int ResolveTradeRouteCompletionRewardModifier(
        const FTradeRouteRuntimeState& Route,
        ETradeRouteEndReason EndReason)
    {
        const int SizeTier = (std::max)(1, Route.ContractUnits / 1500);

        switch (EndReason)
        {
        case ETradeRouteEndReason::Completed:
            return 5 + SizeTier;
        case ETradeRouteEndReason::Cancelled:
            return -(2 + SizeTier / 2);
        case ETradeRouteEndReason::Expired:
        default:
            return -(1 + SizeTier / 3);
        }
    }

    int ResolveTradeRouteSecondaryRelationModifier(
        const FTradeRouteRuntimeState& Route,
        ETradeRouteEndReason EndReason)
    {
        const int BaseModifier =
            ResolveTradeRouteCompletionRewardModifier(Route, EndReason);

        if (EndReason == ETradeRouteEndReason::Completed)
            return BaseModifier;

        return BaseModifier / 2;
    }

    int ResolveTradeRouteStandingModifier(
        const FTradeRouteRuntimeState& Route,
        ETradeRouteEndReason EndReason)
    {
        const int SizeTier = (std::max)(1, Route.ContractUnits / 1500);

        switch (EndReason)
        {
        case ETradeRouteEndReason::Completed:
            return (std::max)(2, SizeTier);
        case ETradeRouteEndReason::Cancelled:
            return -(std::max)(1, SizeTier / 2);
        case ETradeRouteEndReason::Expired:
        default:
            return -(std::max)(1, SizeTier / 3);
        }
    }

    int ResolveTradeRouteActivationRelationModifier(
        const FTradeRouteRuntimeState& Route)
    {
        const int SizeTier = (std::max)(1, Route.ContractUnits / 1500);
        return TradeDiplomacyRuntime::ClampInt(
            1 + SizeTier / 2 + (Route.ImportRoute ? 0 : 1),
            1,
            5);
    }

    void ApplyTradeRouteActivationState(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState,
        const FTradeRouteRuntimeState& Route)
    {
        ++InOutState.SignedContractCount;
        InOutState.LastStandingChange = 0;
        InOutState.LastRelationChange =
            ResolveTradeRouteActivationRelationModifier(Route);
        InOutState.RelationModifier = TradeDiplomacyRuntime::ClampInt(
            InOutState.RelationModifier + InOutState.LastRelationChange,
            -35,
            35);
        InOutState.IdleDays = 0;
    }

    void ApplyTradeRouteCompletionState(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState,
        const FTradeRouteCompletionRecord& Record)
    {
        if (Record.EndReason == ETradeRouteEndReason::Completed)
            ++InOutState.CompletedContractCount;
        else
            ++InOutState.FailedContractCount;

        InOutState.LastRelationChange = Record.SecondaryRelationModifier;
        InOutState.LastStandingChange = Record.StandingModifier;
        InOutState.RelationModifier = TradeDiplomacyRuntime::ClampInt(
            InOutState.RelationModifier + Record.SecondaryRelationModifier,
            -35,
            35);
        InOutState.Standing = TradeDiplomacyRuntime::ClampStanding(
            InOutState.Standing + Record.StandingModifier);
        InOutState.IdleDays = 0;
    }

    void ApplyForeignPowerIdleDecay(
        TradeDiplomacyRuntime::FForeignPowerStandingState& InOutState)
    {
        if (InOutState.ActiveContractCount > 0)
        {
            InOutState.IdleDays = 0;
            return;
        }

        ++InOutState.IdleDays;

        if (InOutState.IdleDays % 20 == 0 && InOutState.Standing != 0)
        {
            InOutState.Standing += InOutState.Standing > 0 ? -1 : 1;
        }

        if (InOutState.IdleDays % 30 == 0 &&
            InOutState.RelationModifier != 0)
        {
            InOutState.RelationModifier +=
                InOutState.RelationModifier > 0 ? -1 : 1;
        }
    }

    std::vector<std::shared_ptr<CPlacementAreaObject>> CollectOperationalHarbors(
        const std::shared_ptr<CWorld>& World)
    {
        std::vector<std::shared_ptr<CPlacementAreaObject>> Harbors;

        if (!World)
            return Harbors;

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
            return Harbors;

        for (size_t Index = 0; Index < BuildingList.size(); ++Index)
        {
            auto Building = BuildingList[Index].lock();

            if (!Building ||
                !Building->GetAlive() ||
                !Building->GetEnable() ||
                !Building->HasPlacedArea() ||
                !Building->IsHarbor())
            {
                continue;
            }

            Harbors.push_back(Building);
        }

        return Harbors;
    }

    struct FCustomsTradeModifierSummary
    {
        int ExportPricePercent = 0;
        int ImportPricePercent = 0;
    };

    FCustomsTradeModifierSummary CollectCustomsTradeModifierSummary(
        CWorld* World)
    {
        FCustomsTradeModifierSummary Result;

        if (!World)
            return Result;

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
            return Result;

        for (size_t Index = 0; Index < BuildingList.size(); ++Index)
        {
            auto Building = BuildingList[Index].lock();

            if (!Building ||
                !Building->GetAlive() ||
                !Building->GetEnable() ||
                !Building->HasPlacedArea() ||
                !IsCustomsOfficeBuildingId(Building->GetBuildingId()))
            {
                continue;
            }

            const int ExportModifier =
                Building->GetTradeRouteExportPriceModifierPercent();

            if (ExportModifier > 0)
            {
                Result.ExportPricePercent = (std::max)(
                    Result.ExportPricePercent,
                    ExportModifier);
            }
            else if (Result.ExportPricePercent <= 0)
            {
                Result.ExportPricePercent = (std::min)(
                    Result.ExportPricePercent,
                    ExportModifier);
            }

            const int ImportModifier =
                Building->GetTradeRouteImportPriceModifierPercent();

            if (ImportModifier < 0)
            {
                Result.ImportPricePercent = (std::min)(
                    Result.ImportPricePercent,
                    ImportModifier);
            }
            else if (Result.ImportPricePercent >= 0)
            {
                Result.ImportPricePercent = (std::max)(
                    Result.ImportPricePercent,
                    ImportModifier);
            }
        }

        return Result;
    }

    EBuildingEra ConvertEdictEra(EEdictEra Era)
    {
        switch (Era)
        {
        case EEdictEra::WorldWars:
            return EBuildingEra::WorldWars;
        case EEdictEra::ColdWar:
            return EBuildingEra::ColdWar;
        case EEdictEra::Modern:
            return EBuildingEra::Modern;
        case EEdictEra::Colonial:
        default:
            return EBuildingEra::Colonial;
        }
    }

    FEraUnlockRequirement ResolveEraUnlockRequirement(EBuildingEra TargetEra)
    {
        EnsureEraRuntimeConfigRegistered();

        if (TargetEra < EBuildingEra::Colonial ||
            TargetEra > EBuildingEra::Modern)
        {
            return FEraUnlockRequirement();
        }

        return GEraUnlockRequirements[GetEraRequirementIndex(TargetEra)];
    }

    int ResolveIndustryBuildingCount(
        const WorldStats::FWorldStatsSnapshot& Snapshot)
    {
        return Snapshot.BuildingCategoryCount[
            static_cast<int>(EBuildingCategory::Industry)];
    }

    int ResolvePublicServiceBuildingCount(
        const WorldStats::FWorldStatsSnapshot& Snapshot)
    {
        return Snapshot.BuildingCategoryCount[
            static_cast<int>(EBuildingCategory::PublicService)];
    }

    int ResolveEntertainmentBuildingCount(
        const WorldStats::FWorldStatsSnapshot& Snapshot)
    {
        return Snapshot.BuildingCategoryCount[
            static_cast<int>(EBuildingCategory::Entertainment)];
    }

    bool MeetsEraUnlockRequirement(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FEraUnlockRequirement& Requirement)
    {
        if (Snapshot.ActiveCitizenCount < Requirement.MinPopulation)
            return false;
        if (Snapshot.TotalBuildingCount < Requirement.MinTotalBuildings)
            return false;
        if (Snapshot.FoodProviderCount < Requirement.MinFoodProviders)
            return false;
        if (ResolveIndustryBuildingCount(Snapshot) <
            Requirement.MinIndustryBuildings)
        {
            return false;
        }
        if (ResolvePublicServiceBuildingCount(Snapshot) <
            Requirement.MinPublicServiceBuildings)
        {
            return false;
        }
        if (ResolveEntertainmentBuildingCount(Snapshot) <
            Requirement.MinEntertainmentBuildings)
        {
            return false;
        }
        if (Snapshot.TotalProducedPowerMW < Requirement.MinPowerMW)
            return false;

        return true;
    }

    struct FWorldCrisisPressureSnapshot
    {
        double RaidRisk = 0.0;
        double LaborStrikeRisk = 0.0;
        double CrimeWaveRisk = 0.0;
        double FiscalEmergencyRisk = 0.0;
        double OppositionRatio = 0.0;
        double UnemploymentRatio = 0.0;
        double HomelessRatio = 0.0;
        bool MartialLawActive = false;
    };

    double NormalizeShortfall(double Value, double SafeValue, double CriticalValue)
    {
        if (Value >= SafeValue)
            return 0.0;
        if (Value <= CriticalValue)
            return 1.0;

        return Clamp<double>(
            (SafeValue - Value) / (SafeValue - CriticalValue),
            0.0,
            1.0);
    }

    double NormalizeOverflow(double Value, double SafeValue, double CriticalValue)
    {
        if (Value <= SafeValue)
            return 0.0;
        if (Value >= CriticalValue)
            return 1.0;

        return Clamp<double>(
            (Value - SafeValue) / (CriticalValue - SafeValue),
            0.0,
            1.0);
    }

    bool IsOperationalBuilding(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        return Building &&
            Building->GetAlive() &&
            Building->GetEnable() &&
            Building->HasPlacedArea();
    }

    bool HasMartialLaw(const std::vector<FGovernmentEdictState>& States)
    {
        for (size_t Index = 0; Index < States.size(); ++Index)
        {
            if (States[Index].Type == EGovernmentEdictType::MartialLaw &&
                States[Index].Active)
            {
                return true;
            }
        }

        return false;
    }

    const wchar_t* GetWorldCrisisTitle(EWorldCrisisType Type)
    {
        switch (Type)
        {
        case EWorldCrisisType::Raid:
            return L"습격 사태";
        case EWorldCrisisType::LaborStrike:
            return L"총파업";
        case EWorldCrisisType::CrimeWave:
            return L"범죄 파동";
        case EWorldCrisisType::FiscalEmergency:
            return L"재정 위기";
        case EWorldCrisisType::None:
        default:
            return L"안정";
        }
    }

    std::wstring BuildWorldCrisisWarningSummary(
        EWorldCrisisType Type,
        int DaysActive)
    {
        const bool Escalated = DaysActive >= 4;

        switch (Type)
        {
        case EWorldCrisisType::Raid:
            return Escalated ?
                L"반정부 세력이 창고와 항구를 집중 습격하고 있습니다." :
                L"외곽 지역에서 무장 세력의 습격 조짐이 포착되었습니다.";
        case EWorldCrisisType::LaborStrike:
            return Escalated ?
                L"노동자 불만이 총파업으로 번져 주요 생산이 흔들리고 있습니다." :
                L"노동 현장에 파업 움직임이 퍼지고 있습니다.";
        case EWorldCrisisType::CrimeWave:
            return Escalated ?
                L"절도와 폭력 사건이 급증하며 치안이 붕괴 직전입니다." :
                L"도시 전역에서 범죄 발생이 빠르게 늘고 있습니다.";
        case EWorldCrisisType::FiscalEmergency:
            return Escalated ?
                L"국고 압박이 심화되어 운영비와 징세 체계가 붕괴하고 있습니다." :
                L"재정 적자가 누적되어 긴급 지출 조정이 필요합니다.";
        case EWorldCrisisType::None:
        default:
            return L"도시는 안정 상태입니다.";
        }
    }

    std::wstring BuildWorldCrisisResolvedSummary(
        EWorldCrisisType Type,
        bool Success)
    {
        switch (Type)
        {
        case EWorldCrisisType::Raid:
            return Success ?
                L"습격 세력이 밀려나며 항구와 창고 질서가 회복되었습니다." :
                L"습격은 잦아들었지만 도시 곳곳에 피해가 남았습니다.";
        case EWorldCrisisType::LaborStrike:
            return Success ?
                L"파업 지도부가 복귀하며 생산 라인이 다시 움직입니다." :
                L"파업은 소강됐지만 작업장 불신이 남아 있습니다.";
        case EWorldCrisisType::CrimeWave:
            return Success ?
                L"치안 단속이 효과를 내며 범죄 파동이 진정되었습니다." :
                L"범죄 파동은 가라앉았지만 시민 불안은 남아 있습니다.";
        case EWorldCrisisType::FiscalEmergency:
            return Success ?
                L"재정 위기가 완화되어 국고 흐름이 정상화되었습니다." :
                L"재정 위기는 일단락됐지만 국고 신뢰는 아직 약합니다.";
        case EWorldCrisisType::None:
        default:
            return Success ? L"상황이 진정되었습니다." : L"상황이 일단락되었습니다.";
        }
    }

    int GetWorldCrisisDurationDays(EWorldCrisisType Type)
    {
        switch (Type)
        {
        case EWorldCrisisType::Raid:
            return 7;
        case EWorldCrisisType::LaborStrike:
            return 8;
        case EWorldCrisisType::CrimeWave:
            return 9;
        case EWorldCrisisType::FiscalEmergency:
            return 8;
        case EWorldCrisisType::None:
        default:
            return 0;
        }
    }

    int GetWorldCrisisCooldownDays(bool Success)
    {
        return Success ? 20 : 28;
    }

    double GetWorldCrisisSeverity(const FWorldCrisisStatus& Status)
    {
        if (!Status.Active || Status.Type == EWorldCrisisType::None)
            return 0.0;

        return Clamp<double>(
            static_cast<double>(Status.DaysActive + 1) / 6.0,
            0.0,
            1.0);
    }

    double GetWorldCrisisPrimaryRisk(
        EWorldCrisisType Type,
        const FWorldCrisisPressureSnapshot& Pressure)
    {
        switch (Type)
        {
        case EWorldCrisisType::Raid:
            return Pressure.RaidRisk;
        case EWorldCrisisType::LaborStrike:
            return Pressure.LaborStrikeRisk;
        case EWorldCrisisType::CrimeWave:
            return Pressure.CrimeWaveRisk;
        case EWorldCrisisType::FiscalEmergency:
            return Pressure.FiscalEmergencyRisk;
        case EWorldCrisisType::None:
        default:
            return 0.0;
        }
    }

    long long ResolveWorldCrisisImmediateBudgetDelta(
        EWorldCrisisType Type,
        double Risk)
    {
        switch (Type)
        {
        case EWorldCrisisType::Raid:
            return
                -(900LL + static_cast<long long>(llround(1600.0 * Risk)));
        case EWorldCrisisType::LaborStrike:
            return
                -(250LL + static_cast<long long>(llround(500.0 * Risk)));
        case EWorldCrisisType::CrimeWave:
            return
                -(700LL + static_cast<long long>(llround(1200.0 * Risk)));
        case EWorldCrisisType::FiscalEmergency:
            return
                -(1600LL + static_cast<long long>(llround(2600.0 * Risk)));
        case EWorldCrisisType::None:
        default:
            return 0;
        }
    }

    EWorldCrisisType ResolveWorldCrisisFollowupType(
        EWorldCrisisType CurrentType,
        const FWorldCrisisPressureSnapshot& Pressure,
        bool Failure)
    {
        auto SelectHigherRisk = [&](EWorldCrisisType A, EWorldCrisisType B)
        {
            return GetWorldCrisisPrimaryRisk(A, Pressure) >=
                GetWorldCrisisPrimaryRisk(B, Pressure) ?
                A :
                B;
        };

        switch (CurrentType)
        {
        case EWorldCrisisType::Raid:
            return SelectHigherRisk(
                EWorldCrisisType::CrimeWave,
                EWorldCrisisType::FiscalEmergency);
        case EWorldCrisisType::LaborStrike:
            return Failure ?
                SelectHigherRisk(
                    EWorldCrisisType::FiscalEmergency,
                    EWorldCrisisType::CrimeWave) :
                SelectHigherRisk(
                    EWorldCrisisType::CrimeWave,
                    EWorldCrisisType::FiscalEmergency);
        case EWorldCrisisType::CrimeWave:
            return SelectHigherRisk(
                EWorldCrisisType::Raid,
                EWorldCrisisType::FiscalEmergency);
        case EWorldCrisisType::FiscalEmergency:
            return SelectHigherRisk(
                EWorldCrisisType::LaborStrike,
                EWorldCrisisType::CrimeWave);
        case EWorldCrisisType::None:
        default:
            return EWorldCrisisType::None;
        }
    }

    double ResolveWorldCrisisFollowupRisk(
        EWorldCrisisType CurrentType,
        const FWorldCrisisPressureSnapshot& Pressure,
        double Severity,
        bool Failure)
    {
        const EWorldCrisisType FollowupType =
            ResolveWorldCrisisFollowupType(CurrentType, Pressure, Failure);

        if (FollowupType == EWorldCrisisType::None)
            return 0.0;

        const double BaseRisk =
            GetWorldCrisisPrimaryRisk(FollowupType, Pressure);
        const double Carryover =
            (Failure ? 0.18 : 0.06) + Severity * 0.18;

        return Clamp<double>(BaseRisk + Carryover, 0.0, 1.0);
    }

    std::wstring BuildWorldCrisisFollowupSummary(
        EWorldCrisisType Type,
        double Risk,
        int DelayDays)
    {
        if (Type == EWorldCrisisType::None || Risk <= 0.0)
            return std::wstring();

        std::wstring Result =
            L" / 후속 위기 징후: " +
            std::wstring(GetWorldCrisisTitle(Type)) +
            L" " +
            std::to_wstring(
                static_cast<int>(llround(Clamp<double>(Risk, 0.0, 1.0) * 100.0))) +
            L"%";

        if (DelayDays > 0)
        {
            Result +=
                L" (" +
                std::to_wstring(DelayDays) +
                L"일 내)";
        }

        return Result;
    }

    void ApplyWorldCrisisPressureTransfer(
        EWorldCrisisType Type,
        double Severity,
        bool Failure,
        int& InOutRaidPressureDays,
        int& InOutLaborStrikePressureDays,
        int& InOutCrimeWavePressureDays,
        int& InOutFiscalEmergencyPressureDays)
    {
        const int LightPressure = Severity >= 0.35 ? 1 : 0;
        const int HeavyPressure = Severity >= 0.72 ? 1 : 0;
        const int FailureBonus = Failure ? 1 : 0;

        switch (Type)
        {
        case EWorldCrisisType::Raid:
            InOutCrimeWavePressureDays += 1 + LightPressure + FailureBonus;
            InOutFiscalEmergencyPressureDays +=
                LightPressure + HeavyPressure + FailureBonus;
            break;
        case EWorldCrisisType::LaborStrike:
            InOutFiscalEmergencyPressureDays +=
                1 + LightPressure + FailureBonus;
            InOutCrimeWavePressureDays +=
                LightPressure + HeavyPressure + FailureBonus;
            break;
        case EWorldCrisisType::CrimeWave:
            InOutRaidPressureDays += 1 + LightPressure + FailureBonus;
            InOutFiscalEmergencyPressureDays +=
                LightPressure + HeavyPressure + FailureBonus;
            break;
        case EWorldCrisisType::FiscalEmergency:
            InOutLaborStrikePressureDays +=
                1 + LightPressure + FailureBonus;
            InOutCrimeWavePressureDays +=
                LightPressure + HeavyPressure + FailureBonus;
            break;
        case EWorldCrisisType::None:
        default:
            break;
        }
    }

    FWorldCrisisPressureSnapshot BuildWorldCrisisPressureSnapshot(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FPoliticalWorldSnapshot& PoliticalSnapshot,
        long long NationalBudget,
        long long LastDailyNetChange,
        double TaxCollectionEfficiency,
        const std::vector<FGovernmentEdictState>& GovernmentEdicts,
        const FTaxPolicy& TaxPolicy,
        const FTaxPolicyEventStatus& TaxEventStatus)
    {
        FWorldCrisisPressureSnapshot Result;
        const double CitizenCount =
            static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount));
        const double OppositionRatio =
            CitizenCount > 0.0 ?
                static_cast<double>(PoliticalSnapshot.OppositionCount) /
                    CitizenCount :
                0.0;
        const double UnemploymentRatio =
            static_cast<double>(Snapshot.UnemployedCount) / CitizenCount;
        const double HomelessRatio =
            static_cast<double>(Snapshot.HomelessCount) / CitizenCount;
        const double SecurityCollapse =
            NormalizeShortfall(Snapshot.AverageSecurity, 58.0, 30.0);
        const double ResidentialSecurityCollapse =
            NormalizeShortfall(
                Snapshot.AverageResidentialSecurity,
                56.0,
                26.0);
        const double FreedomCollapse =
            NormalizeShortfall(Snapshot.AverageFreedom, 56.0, 28.0);
        const double ResidentialFreedomCollapse =
            NormalizeShortfall(
                Snapshot.AverageResidentialFreedom,
                54.0,
                24.0);
        const double JobStress =
            NormalizeShortfall(Snapshot.AverageJob, 56.0, 32.0);
        const double FoodStress =
            NormalizeShortfall(Snapshot.AverageFood, 54.0, 28.0);
        const double HousingStress =
            NormalizeShortfall(Snapshot.AverageHousing, 54.0, 28.0);
        const double ResidentialPollutionStress =
            NormalizeOverflow(
                Snapshot.AverageResidentialPollution,
                28.0,
                68.0);
        const double BudgetDeficit =
            NationalBudget < 0 ?
                NormalizeOverflow(
                    static_cast<double>(-NationalBudget),
                    5000.0,
                    70000.0) :
                0.0;
        const double NetLossPressure =
            LastDailyNetChange < 0 ?
                NormalizeOverflow(
                    static_cast<double>(-LastDailyNetChange),
                    1200.0,
                    9000.0) :
                0.0;
        const double TaxCollectionBreakdown =
            NormalizeShortfall(TaxCollectionEfficiency, 0.82, 0.48);
        const double IncomeTaxPressure =
            (std::max)(
                0.0,
                static_cast<double>(
                    GetTaxPolicyDeviationNormalized(
                        TaxPolicy,
                        ETaxPolicyType::Income)));

        Result.MartialLawActive = HasMartialLaw(GovernmentEdicts);
        Result.OppositionRatio = Clamp<double>(OppositionRatio, 0.0, 1.0);
        Result.UnemploymentRatio = Clamp<double>(UnemploymentRatio, 0.0, 1.0);
        Result.HomelessRatio = Clamp<double>(HomelessRatio, 0.0, 1.0);

        Result.RaidRisk = Clamp<double>(
            SecurityCollapse * 0.26 +
            ResidentialSecurityCollapse * 0.14 +
            FreedomCollapse * 0.14 +
            ResidentialFreedomCollapse * 0.08 +
            Result.OppositionRatio * 0.20 +
            FoodStress * 0.08 +
            HousingStress * 0.06 +
            ResidentialPollutionStress * 0.04 -
            (Result.MartialLawActive ? 0.12 : 0.0),
            0.0,
            1.0);

        Result.LaborStrikeRisk = Clamp<double>(
            Result.UnemploymentRatio * 0.34 +
            JobStress * 0.26 +
            IncomeTaxPressure * 0.16 +
            Result.OppositionRatio * 0.12 +
            (TaxEventStatus.Active &&
                TaxEventStatus.Type == ETaxPolicyEventType::WorkerTaxStrike ?
                    0.12 :
                    0.0),
            0.0,
            1.0);

        Result.CrimeWaveRisk = Clamp<double>(
            SecurityCollapse * 0.28 +
            ResidentialSecurityCollapse * 0.18 +
            Result.HomelessRatio * 0.14 +
            Result.UnemploymentRatio * 0.12 +
            HousingStress * 0.10 +
            ResidentialPollutionStress * 0.10 +
            NetLossPressure * 0.08 -
            (Result.MartialLawActive ? 0.10 : 0.0),
            0.0,
            1.0);

        Result.FiscalEmergencyRisk = Clamp<double>(
            BudgetDeficit * 0.42 +
            NetLossPressure * 0.26 +
            TaxCollectionBreakdown * 0.20 +
            ResidentialPollutionStress * 0.04 +
            (TaxEventStatus.Active &&
                TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis ?
                    0.08 :
                    0.0) +
            (Snapshot.ActiveCitizenCount >= 80 ? 0.04 : 0.0),
            0.0,
            1.0);

        return Result;
    }

    void ResetWorldCrisisPressureCounters(
        int& InOutRaidPressureDays,
        int& InOutLaborStrikePressureDays,
        int& InOutCrimeWavePressureDays,
        int& InOutFiscalEmergencyPressureDays)
    {
        InOutRaidPressureDays = 0;
        InOutLaborStrikePressureDays = 0;
        InOutCrimeWavePressureDays = 0;
        InOutFiscalEmergencyPressureDays = 0;
    }

    void StartWorldCrisis(
        FWorldCrisisStatus& InOutStatus,
        EWorldCrisisType Type,
        int SimulationYear,
        int SimulationMonth,
        int SimulationDay,
        long long ImmediateBudgetDelta,
        long long& InOutNationalBudget,
        long long& InOutLastDailyNetChange,
        int& InOutRaidPressureDays,
        int& InOutLaborStrikePressureDays,
        int& InOutCrimeWavePressureDays,
        int& InOutFiscalEmergencyPressureDays,
        bool IgnoreCooldown)
    {
        if (Type == EWorldCrisisType::None ||
            InOutStatus.Active ||
            (!IgnoreCooldown && InOutStatus.CooldownDays > 0))
        {
            return;
        }

        InOutStatus = FWorldCrisisStatus();
        InOutStatus.Type = Type;
        InOutStatus.Active = true;
        InOutStatus.RemainingDays = GetWorldCrisisDurationDays(Type);
        InOutStatus.NotificationDays = 6;
        InOutStatus.TriggerYear = SimulationYear;
        InOutStatus.TriggerMonth = SimulationMonth;
        InOutStatus.TriggerDay = SimulationDay;
        InOutStatus.Title = GetWorldCrisisTitle(Type);
        InOutStatus.Summary = BuildWorldCrisisWarningSummary(Type, 0);
        ResetWorldCrisisPressureCounters(
            InOutRaidPressureDays,
            InOutLaborStrikePressureDays,
            InOutCrimeWavePressureDays,
            InOutFiscalEmergencyPressureDays);

        if (ImmediateBudgetDelta != 0)
        {
            InOutNationalBudget += ImmediateBudgetDelta;
            InOutLastDailyNetChange += ImmediateBudgetDelta;
        }
    }

    void ResolveWorldCrisisState(
        FWorldCrisisStatus& InOutStatus,
        bool Success)
    {
        if (InOutStatus.Type == EWorldCrisisType::None)
            return;

        InOutStatus.Active = false;
        InOutStatus.RemainingDays = 0;
        InOutStatus.CooldownDays = GetWorldCrisisCooldownDays(Success);
        InOutStatus.NotificationDays = 8;
        InOutStatus.Summary = BuildWorldCrisisResolvedSummary(
            InOutStatus.Type,
            Success);
        InOutStatus.DaysActive = 0;
    }

    std::wstring BuildAutoImportSelectionText(
        const TradePolicy::FImportTradePolicy& Policy)
    {
        switch (Policy.Mode)
        {
        case TradePolicy::EImportPolicyMode::None:
            return L"없음";
        case TradePolicy::EImportPolicyMode::SingleResource:
            if (Policy.SelectedResourceType != EResourceType::None)
                return GetResourceTypeDisplayName(Policy.SelectedResourceType);
            return L"없음";
        case TradePolicy::EImportPolicyMode::AllResources:
        default:
            return L"전체";
        }
    }

    std::wstring BuildImportCapSelectionText(
        const TradePolicy::FImportTradePolicy& Policy)
    {
        switch (TradePolicy::GetImportMaxUnitsPerResource(Policy))
        {
        case 500:
            return L"낮음 (500)";
        case 1500:
            return L"표준 (1,500)";
        case 3000:
            return L"확대 (3,000)";
        case 6000:
            return L"최대 (6,000)";
        default:
            return L"사용자 지정";
        }
    }

    std::wstring BuildImportBudgetSelectionText(
        const TradePolicy::FImportTradePolicy& Policy)
    {
        const int BudgetCap = TradePolicy::GetDailyImportBudgetCap(Policy);

        if (BudgetCap <= 0)
            return L"무제한";

        const std::wstring BudgetLabel =
            L"$" + std::to_wstring(BudgetCap);

        if (TradePolicy::AllowsEmergencyImports(Policy))
            return L"긴급 대응 (" + BudgetLabel + L")";

        switch (BudgetCap)
        {
        case 12000:
            return L"절약 (" + BudgetLabel + L")";
        case 24000:
            return L"표준 (" + BudgetLabel + L")";
        case 36000:
            return L"대량 (" + BudgetLabel + L")";
        default:
            return L"사용자 지정 (" + BudgetLabel + L")";
        }
    }

    std::wstring BuildExportBlockedSelectionText(
        const TradePolicy::FExportTradePolicy& Policy)
    {
        std::vector<std::wstring> BlockedResources;

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsExportableResourceType(ResourceType) ||
                TradePolicy::IsResourceExportAllowed(Policy, ResourceType))
            {
                continue;
            }

            BlockedResources.push_back(
                GetResourceTypeDisplayName(ResourceType));
        }

        if (BlockedResources.empty())
            return L"없음";

        std::wstring Result;

        for (size_t Index = 0; Index < BlockedResources.size(); ++Index)
        {
            if (Index > 0)
                Result += L", ";

            Result += BlockedResources[Index];
        }

        return Result;
    }

    std::wstring BuildDomesticReserveSelectionText(
        const TradePolicy::FExportTradePolicy& Policy)
    {
        switch (TradePolicy::GetDomesticReserveBufferUnits(Policy))
        {
        case 0:
            return L"부족분만";
        case 1000:
            return L"표준 (+1,000)";
        case 3000:
            return L"강화 (+3,000)";
        case 6000:
            return L"최우선 (+6,000)";
        default:
            return L"사용자 지정";
        }
    }

    int ConsumeRaidResourcesFromBuildings(
        const std::vector<std::weak_ptr<CPlacementAreaObject>>& BuildingList,
        bool RequirePriorityTarget,
        int RemainingTarget)
    {
        if (RemainingTarget <= 0)
            return 0;

        int StolenAmount = 0;

        auto ConsumeFromBuilding = [&](const std::shared_ptr<CPlacementAreaObject>& Building)
        {
            if (!IsOperationalBuilding(Building) ||
                Building->GetAvailableExportableResourceStock() <= 0)
            {
                return;
            }

            while (RemainingTarget > 0)
            {
                const int AttemptAmount = (std::min)(
                    RemainingTarget,
                    (std::min)(10, Building->GetAvailableExportableResourceStock()));

                if (AttemptAmount <= 0)
                    break;

                bool Consumed = false;

                for (int Amount = AttemptAmount; Amount >= 1; --Amount)
                {
                    if (!Building->TryConsumeExportableResources(Amount))
                        continue;

                    RemainingTarget -= Amount;
                    StolenAmount += Amount;
                    Consumed = true;
                    break;
                }

                if (!Consumed)
                    break;
            }
        };

        for (size_t Index = 0; Index < BuildingList.size() && RemainingTarget > 0; ++Index)
        {
            auto Building = BuildingList[Index].lock();

            if (!IsOperationalBuilding(Building))
                continue;

            const bool PriorityTarget =
                Building->IsHarbor() || Building->IsWarehouse();

            if (RequirePriorityTarget != PriorityTarget)
                continue;

            ConsumeFromBuilding(Building);
        }

        return StolenAmount;
    }

    int ApplyRaidResourceTheft(CWorld* World, int TargetAmount)
    {
        if (!World || TargetAmount <= 0)
            return 0;

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
            return 0;

        int StolenAmount =
            ConsumeRaidResourcesFromBuildings(BuildingList, true, TargetAmount);

        if (StolenAmount < TargetAmount)
        {
            StolenAmount += ConsumeRaidResourcesFromBuildings(
                BuildingList,
                false,
                TargetAmount - StolenAmount);
        }

        return StolenAmount;
    }
}

void CMainWorld::InitializeElectionSchedule()
{
    PoliticsSystem::InitializeElectionSchedule(
        mElectionStatus,
        mSimulationYear,
        MainWorldConfig::GInitialElectionLeadYears,
        MainWorldConfig::GElectionMonth,
        MainWorldConfig::GElectionDay);
}

void CMainWorld::TickElectionPromises()
{
    if (mElectionStatus.GameLost)
        return;

    auto World = mSelf.lock();

    if (!World)
        return;

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(World);

    for (int Index = 0; Index < GElectionPromiseCount; ++Index)
    {
        FElectionPromiseState& Promise =
            mElectionStatus.ActivePromises[static_cast<size_t>(Index)];

        if (!Promise.Active)
            continue;

        Promise.CurrentValue =
            EvaluateElectionPromiseCurrentValue(
                Promise,
                Snapshot,
                mLastDailyExportIncome);
    }

    const int DaysUntilElection = GetDaysUntilNextElection();

    if (DaysUntilElection <= 0 ||
        DaysUntilElection > GCampaignPromiseLeadDays ||
        mElectionStatus.CampaignPromisesIssued ||
        CountActiveElectionPromises(mElectionStatus) > 0)
    {
        return;
    }

    static const EElectionPromiseType GPromiseTypes[] =
    {
        EElectionPromiseType::Housing,
        EElectionPromiseType::Food,
        EElectionPromiseType::Health,
        EElectionPromiseType::Job,
        EElectionPromiseType::Freedom,
        EElectionPromiseType::Security,
        EElectionPromiseType::Faith,
        EElectionPromiseType::ExportIncome
    };

    std::vector<FElectionPromiseCandidate> Candidates;
    Candidates.reserve(sizeof(GPromiseTypes) / sizeof(GPromiseTypes[0]));

    for (EElectionPromiseType Type : GPromiseTypes)
    {
        FElectionPromiseCandidate Candidate;
        Candidate.Promise =
            BuildElectionPromiseState(Type, Snapshot, mLastDailyExportIncome);
        Candidate.Priority =
            EvaluateElectionPromisePriority(
                Type,
                Snapshot,
                mLastDailyExportIncome);
        Candidates.push_back(Candidate);
    }

    std::sort(
        Candidates.begin(),
        Candidates.end(),
        [](const FElectionPromiseCandidate& A,
            const FElectionPromiseCandidate& B)
        {
            if (A.Priority != B.Priority)
                return A.Priority > B.Priority;

            return A.Promise.Title < B.Promise.Title;
        });

    mElectionStatus.ActivePromises = {};

    for (int Index = 0;
        Index < GElectionPromiseCount &&
            Index < static_cast<int>(Candidates.size());
        ++Index)
    {
        mElectionStatus.ActivePromises[static_cast<size_t>(Index)] =
            Candidates[static_cast<size_t>(Index)].Promise;
    }

    mElectionStatus.CampaignPromisesIssued =
        CountActiveElectionPromises(mElectionStatus) > 0;

    if (!mElectionStatus.CampaignPromisesIssued)
        return;

    mElectionStatus.PromiseIssueYear = mSimulationYear;
    mElectionStatus.PromiseIssueMonth = mSimulationMonth;
    mElectionStatus.PromiseIssueDay = mSimulationDay;
    mElectionStatus.HasPromiseEvaluation = false;
    mElectionStatus.LastPromiseMetCount = 0;
    mElectionStatus.LastPromiseFailedCount = 0;
    mElectionStatus.LastPromiseVoteModifierPercent = 0;
}

void CMainWorld::ResolveScheduledElection()
{
    RefreshPoliticalSnapshot();
    PoliticsSystem::ResolveScheduledElection(
        mElectionStatus,
        mPoliticalSnapshot,
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay,
        MainWorldConfig::GElectionIntervalYears,
        MainWorldConfig::GElectionMonth,
        MainWorldConfig::GElectionDay);
}

int CMainWorld::GetDaysUntilNextElection() const
{
    return PoliticsSystem::GetDaysUntilNextElection(
        mElectionStatus,
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay);
}

double CMainWorld::GetElectionWarningScore() const
{
    return PoliticsSystem::GetElectionWarningScore(
        mElectionStatus,
        mPoliticalSnapshot,
        mTaxEventStatus,
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay);
}

void CMainWorld::ApplyDailyEconomySettlement()
{
    const int DaysInMonth = GetDaysInMonth(mSimulationYear, mSimulationMonth);
    const auto Result = EconomySystem::ApplyDailyWorldSettlement(
        this,
        DaysInMonth,
        mGovernmentProfile,
        mTaxEventStatus,
        mGovernmentEdicts,
        mEdictModifiers);
    mLastDailyWageCost     = Result.BaseResult.WageCost;
    mLastDailyUpkeepCost   = Result.BaseResult.UpkeepCost;
    mLastDailyExportIncome = Result.BaseResult.ExportIncome;
    mLastDailyTaxIncome    = Result.AdjustedTaxIncome;
    mLastDailyConsumptionTaxIncome = Result.AdjustedConsumptionTaxIncome;
    mLastDailyIncomeTaxIncome = Result.AdjustedIncomeTaxIncome;
    mLastDailyPropertyTaxIncome = Result.AdjustedPropertyTaxIncome;
    mLastDailyEdictCost    = Result.DailyEdictCost;
    mLastDailyImportExpense = Result.BaseResult.ImportExpense;
    mLastDailyTaxCollectionEfficiency =
        Result.BaseResult.TaxCollectionEfficiency;
    mLastDailyNetChange = Result.NetBudgetChange;
    mNationalBudget += mLastDailyNetChange;
}

void CMainWorld::RecordFinishedTradeRoute(
    const FTradeRouteRuntimeState& Route,
    ETradeRouteEndReason EndReason)
{
    FTradeRouteCompletionRecord Record;
    Record.RecordId = mTradeDiplomacyState.NextTradeRouteCompletionRecordId++;
    Record.RouteId = Route.RouteId;
    Record.ImportRoute = Route.ImportRoute;
    Record.ResourceType = Route.ResourceType;
    Record.MarketClass = Route.MarketClass;
    Record.ForeignPowerIndex = Route.ForeignPowerIndex;
    Record.ContractUnits = Route.ContractUnits;
    Record.FulfilledUnits = Route.FulfilledUnits;
    Record.ElapsedDays = (std::max)(
        0,
        Route.TotalDurationDays - Route.RemainingDays);
    Record.TotalDurationDays = Route.TotalDurationDays;
    Record.SettledValue =
        static_cast<long long>(Route.RoutePricePerThousandUnits) *
        static_cast<long long>(Route.FulfilledUnits) / 1000LL;
    Record.EndReason = EndReason;
    Record.CompletionRewardModifier =
        MainWorldTradeRuntime::ResolveTradeRouteCompletionRewardModifier(
            Route,
            EndReason);
    Record.SecondaryRelationModifier =
        MainWorldTradeRuntime::ResolveTradeRouteSecondaryRelationModifier(
            Route,
            EndReason);
    Record.StandingModifier =
        MainWorldTradeRuntime::ResolveTradeRouteStandingModifier(
            Route,
            EndReason);
    MainWorldTradeRuntime::ApplyTradeRouteCompletionState(
        mTradeDiplomacyState.ForeignPowerStandingStates[static_cast<size_t>(
            TradeDiplomacyRuntime::ClampInt(
                Route.ForeignPowerIndex,
                0,
                TradeDiplomacyRuntime::GForeignPowerCount - 1))],
        Record);

    mTradeDiplomacyState.CompletedTradeRoutes.insert(
        mTradeDiplomacyState.CompletedTradeRoutes.begin(),
        Record);

    if (mTradeDiplomacyState.CompletedTradeRoutes.size() >
        static_cast<size_t>(GMaxCompletedTradeRouteRecordCount))
    {
        mTradeDiplomacyState.CompletedTradeRoutes.resize(
            static_cast<size_t>(GMaxCompletedTradeRouteRecordCount));
    }

    ++mTradeDiplomacyState.TradeRouteCompletionNotificationVersion;
}

void CMainWorld::ProcessActiveTradeRoutes()
{
    if (mTradeDiplomacyState.ActiveTradeRoutes.empty())
        return;

    auto World = mSelf.lock();

    if (!World)
        return;

    std::vector<std::shared_ptr<CPlacementAreaObject>> Harbors =
        MainWorldTradeRuntime::CollectOperationalHarbors(World);
    std::vector<FTradeRouteRuntimeState> RemainingRoutes;
    RemainingRoutes.reserve(mTradeDiplomacyState.ActiveTradeRoutes.size());
    bool BudgetChanged = false;

    for (size_t RouteIndex = 0;
        RouteIndex < mTradeDiplomacyState.ActiveTradeRoutes.size();
        ++RouteIndex)
    {
        FTradeRouteRuntimeState Route =
            mTradeDiplomacyState.ActiveTradeRoutes[RouteIndex];
        const int RemainingUnits = (std::max)(
            0,
            Route.ContractUnits - Route.FulfilledUnits);

        if (RemainingUnits <= 0)
        {
            RecordFinishedTradeRoute(Route, ETradeRouteEndReason::Completed);
            continue;
        }

        int DailyTransferUnits = (std::min)(
            RemainingUnits,
            MainWorldTradeRuntime::ResolveTradeRouteDailyTransferUnits(Route));

        if (Route.ImportRoute)
        {
            if (Route.RoutePricePerThousandUnits > 0)
            {
                const long long MaxAffordableUnits =
                    mNationalBudget > 0 ?
                        (mNationalBudget * 1000LL) /
                        static_cast<long long>(Route.RoutePricePerThousandUnits) :
                        0LL;
                DailyTransferUnits = (std::min)(
                    DailyTransferUnits,
                    static_cast<int>((std::max)(0LL, MaxAffordableUnits)));
            }

            struct FHarborImportAllocation
            {
                std::shared_ptr<CPlacementAreaObject> Harbor;
                int Capacity = 0;
            };

            std::vector<FHarborImportAllocation> Allocations;
            Allocations.reserve(Harbors.size());

            for (size_t HarborIndex = 0; HarborIndex < Harbors.size(); ++HarborIndex)
            {
                FHarborImportAllocation Allocation;
                Allocation.Harbor = Harbors[HarborIndex];
                Allocation.Capacity =
                    Harbors[HarborIndex]->GetAvailableIncomingCapacity(
                        Route.ResourceType);

                if (Allocation.Capacity > 0)
                    Allocations.push_back(std::move(Allocation));
            }

            std::sort(
                Allocations.begin(),
                Allocations.end(),
                [](const FHarborImportAllocation& A,
                    const FHarborImportAllocation& B)
                {
                    return A.Capacity > B.Capacity;
                });

            int ImportedUnits = 0;
            int RemainingTransferUnits = DailyTransferUnits;

            for (size_t HarborIndex = 0;
                HarborIndex < Allocations.size() && RemainingTransferUnits > 0;
                ++HarborIndex)
            {
                const int AssignedUnits = (std::min)(
                    RemainingTransferUnits,
                    Allocations[HarborIndex].Capacity);

                if (AssignedUnits <= 0)
                    continue;

                if (!Allocations[HarborIndex].Harbor->TryAddResourceStock(
                        Route.ResourceType,
                        AssignedUnits))
                {
                    continue;
                }

                ImportedUnits += AssignedUnits;
                RemainingTransferUnits -= AssignedUnits;
            }

            if (ImportedUnits > 0)
            {
                const long long ImportCost =
                    static_cast<long long>(Route.RoutePricePerThousandUnits) *
                    static_cast<long long>(ImportedUnits) / 1000LL;
                mNationalBudget -= ImportCost;
                mLastDailyImportExpense += ImportCost;
                mLastDailyNetChange -= ImportCost;
                Route.FulfilledUnits += ImportedUnits;
                BudgetChanged = true;
            }
        }
        else
        {
            std::sort(
                Harbors.begin(),
                Harbors.end(),
                [&Route](const std::shared_ptr<CPlacementAreaObject>& A,
                    const std::shared_ptr<CPlacementAreaObject>& B)
                {
                    return A->GetAvailableResourceStock(Route.ResourceType) >
                        B->GetAvailableResourceStock(Route.ResourceType);
                });

            int ExportedUnits = 0;
            int RemainingTransferUnits = DailyTransferUnits;

            for (size_t HarborIndex = 0;
                HarborIndex < Harbors.size() && RemainingTransferUnits > 0;
                ++HarborIndex)
            {
                const int AvailableUnits =
                    Harbors[HarborIndex]->GetAvailableResourceStock(
                        Route.ResourceType);
                const int ExportUnits = (std::min)(
                    RemainingTransferUnits,
                    AvailableUnits);

                if (ExportUnits <= 0)
                    continue;

                if (!Harbors[HarborIndex]->TryConsumeResource(
                        Route.ResourceType,
                        ExportUnits))
                {
                    continue;
                }

                ExportedUnits += ExportUnits;
                RemainingTransferUnits -= ExportUnits;
            }

            if (ExportedUnits > 0)
            {
                const long long ExportIncome =
                    static_cast<long long>(Route.RoutePricePerThousandUnits) *
                    static_cast<long long>(ExportedUnits) / 1000LL;
                mNationalBudget += ExportIncome;
                mLastDailyExportIncome += ExportIncome;
                mLastDailyNetChange += ExportIncome;
                Route.FulfilledUnits += ExportedUnits;
                BudgetChanged = true;
            }
        }

        Route.RemainingDays = (std::max)(0, Route.RemainingDays - 1);

        if (Route.FulfilledUnits < Route.ContractUnits &&
            Route.RemainingDays > 0)
        {
            RemainingRoutes.push_back(std::move(Route));
        }
        else
        {
            RecordFinishedTradeRoute(
                Route,
                Route.FulfilledUnits >= Route.ContractUnits ?
                    ETradeRouteEndReason::Completed :
                    ETradeRouteEndReason::Expired);
        }
    }

    mTradeDiplomacyState.ActiveTradeRoutes.swap(RemainingRoutes);

    if (BudgetChanged)
    {
        RefreshWorldMarketPrices();
        RefreshPoliticalSnapshot();
    }
}

void CMainWorld::RefreshForeignTradeDiplomacy(bool ApplyIdleDecay)
{
    auto World = mSelf.lock();

    if (!World)
    {
        mTradeDiplomacyState.ForeignPowerStates = {};
        return;
    }

    std::array<int, TradeDiplomacyRuntime::GForeignPowerCount> ActiveCounts = {};

    for (size_t RouteIndex = 0;
        RouteIndex < mTradeDiplomacyState.ActiveTradeRoutes.size();
        ++RouteIndex)
    {
        const int SafeIndex = TradeDiplomacyRuntime::ClampInt(
            mTradeDiplomacyState.ActiveTradeRoutes[RouteIndex].ForeignPowerIndex,
            0,
            TradeDiplomacyRuntime::GForeignPowerCount - 1);
        ++ActiveCounts[static_cast<size_t>(SafeIndex)];
    }

    for (int Index = 0;
        Index < TradeDiplomacyRuntime::GForeignPowerCount;
        ++Index)
    {
        auto& StandingState =
            mTradeDiplomacyState.ForeignPowerStandingStates[
                static_cast<size_t>(Index)];
        StandingState.ActiveContractCount =
            ActiveCounts[static_cast<size_t>(Index)];

        if (ApplyIdleDecay)
            MainWorldTradeRuntime::ApplyForeignPowerIdleDecay(StandingState);
    }

    mTradeDiplomacyState.ForeignPowerStates =
        TradeDiplomacyRuntime::BuildForeignPowerWorldStates(
            WorldStats::BuildSnapshot(World),
            mGovernmentProfile,
            mTaxEventStatus,
            mGovernmentEdicts,
            mTradeDiplomacyState.ForeignPowerStandingStates);
}

void CMainWorld::RefreshPowerGridCoverage()
{
    MainWorldInfrastructureRuntime::RefreshPowerGridCoverage(
        mSelf.lock().get());
}

void CMainWorld::RefreshWorldMarketPrices()
{
    auto World = mSelf.lock();

    if (!World)
        return;

    ResourceTradePricing::UpdateWorldMarketPrices(
        WorldStats::BuildSnapshot(World),
        mGovernmentProfile,
        mGovernmentEdicts,
        mTaxEventStatus,
        mWorldCrisisStatus,
        mTradeDiplomacyState.ForeignPowerStates,
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay);
}

void CMainWorld::RefreshBuildingPollutionExposure()
{
    MainWorldInfrastructureRuntime::RefreshBuildingPollutionExposure(
        mSelf.lock().get());
}

void CMainWorld::RefreshRuntimeBuildingState()
{
    RefreshPowerGridCoverage();
    RefreshBuildingPollutionExposure();
    ReassignCitizenNeeds();
    RefreshEraProgress();
    RefreshPoliticalSnapshot();
    RefreshForeignTradeDiplomacy(false);
    RefreshWorldMarketPrices();
}

bool CMainWorld::TryApplyEdict(
    EGovernmentEdictType Type,
    std::wstring& OutMessage)
{
    const FGovernmentEdictDefinition* Definition =
        EdictSystem::FindGovernmentEdictDefinition(Type);

    if (!Definition)
    {
        OutMessage = L"정의되지 않은 칙령입니다.";
        return false;
    }

    if (!IsBuildingEraUnlocked(
            mEraProgress.CurrentEra,
            ConvertEdictEra(Definition->Era)))
    {
        OutMessage =
            std::wstring(GetBuildingEraDisplayName(
                ConvertEdictEra(Definition->Era))) +
            L" 이후에 시행할 수 있습니다.";
        return false;
    }

    if (!Definition->Implemented)
    {
        OutMessage = L"아직 구현되지 않은 칙령입니다.";
        return false;
    }

    FGovernmentEdictState* TargetState = nullptr;

    for (size_t i = 0; i < mGovernmentEdicts.size(); ++i)
    {
        if (mGovernmentEdicts[i].Type == Type)
        {
            TargetState = &mGovernmentEdicts[i];
            break;
        }
    }

    if (!TargetState)
    {
        OutMessage = L"칙령 상태를 찾을 수 없습니다.";
        return false;
    }

    const int ActiveCitizenCount =
        (std::max)(0, mPoliticalSnapshot.ActiveCitizenCount);
    const ETaxPolicyEventType RequiredTaxEvent =
        EconomySystem::GetRequiredTaxPolicyEventForEdict(Type);

    if (RequiredTaxEvent != ETaxPolicyEventType::None)
    {
        if (!mTaxEventStatus.Active || mTaxEventStatus.Type != RequiredTaxEvent)
        {
            OutMessage =
                Definition->DisplayName +
                L"은(는) " +
                EconomySystem::GetTaxPolicyEventTitle(RequiredTaxEvent) +
                L" 발생 중에만 시행할 수 있습니다.";
            return false;
        }
    }

    if (Definition->Mode == EGovernmentEdictMode::Passive &&
        TargetState->Active)
    {
        TargetState->Active = false;
        TargetState->RemainingDays = 0;
        PoliticsSystem::SyncGovernmentActionFromEdict(
            mGovernmentProfile,
            Type,
            false);
        RefreshEdictModifiers();
        RefreshPoliticalSnapshot();
        RefreshForeignTradeDiplomacy(false);
        RefreshWorldMarketPrices();
        OutMessage = Definition->DisplayName + L" 해제";
        return true;
    }

    if (TargetState->Active)
    {
        OutMessage = Definition->DisplayName + L" 시행 중";
        return false;
    }

    if (TargetState->CooldownDays > 0)
    {
        OutMessage = Definition->DisplayName + L" 재사용 대기 중";
        return false;
    }

    const long long ActivationCost =
        EdictSystem::ResolveEdictActivationCost(
            *Definition,
            ActiveCitizenCount);

    if (ActivationCost > mNationalBudget)
    {
        OutMessage = L"예산이 부족합니다.";
        return false;
    }

    mNationalBudget -= ActivationCost;
    TargetState->Active = true;

    if (Definition->Mode == EGovernmentEdictMode::Active)
    {
        TargetState->RemainingDays = (std::max)(1, Definition->DurationDays);
        TargetState->CooldownDays = (std::max)(1, Definition->CooldownDays);
    }
    else
    {
        TargetState->RemainingDays = -1;
        TargetState->CooldownDays = 0;
    }

    std::wstring ResponseMessage;

    switch (Type)
    {
    case EGovernmentEdictType::LaborTaxRelief:
    {
        const int RateDelta = EconomySystem::ApplyTaxPolicyRateDelta(
            mGovernmentProfile.TaxPolicy,
            ETaxPolicyType::Income,
            -4);
        EconomySystem::ResolveTaxPolicyEvent(mTaxEventStatus, true);
        ResponseMessage =
            L"소득세 " +
            std::to_wstring((std::max)(0, -RateDelta)) +
            L"%p 인하";
        break;
    }
    case EGovernmentEdictType::PropertyTaxRelief:
    {
        const int RateDelta = EconomySystem::ApplyTaxPolicyRateDelta(
            mGovernmentProfile.TaxPolicy,
            ETaxPolicyType::Property,
            -10);
        EconomySystem::ResolveTaxPolicyEvent(mTaxEventStatus, true);
        ResponseMessage =
            L"재산세 " +
            std::to_wstring((std::max)(0, -RateDelta)) +
            L"%p 인하";
        break;
    }
    case EGovernmentEdictType::EmergencyAusterity:
    {
        const long long EmergencyFunds = 12000;
        mNationalBudget += EmergencyFunds;
        mLastDailyNetChange += EmergencyFunds;
        EconomySystem::ResolveTaxPolicyEvent(mTaxEventStatus, true);
        ResponseMessage = L"긴급 자금 $12,000 투입";
        break;
    }
    default:
        break;
    }

    PoliticsSystem::SyncGovernmentActionFromEdict(
        mGovernmentProfile,
        Type,
        true);
    RefreshEdictModifiers();
    RefreshPoliticalSnapshot();
    RefreshForeignTradeDiplomacy(false);
    RefreshWorldMarketPrices();

    OutMessage = Definition->DisplayName + L" 시행";

    if (!ResponseMessage.empty())
    {
        OutMessage += L" / ";
        OutMessage += ResponseMessage;
    }

    return true;
}

bool CMainWorld::AdjustTaxPolicy(
    ETaxPolicyType Type,
    int DeltaPercent,
    std::wstring& OutMessage)
{
    const bool Adjusted = EconomySystem::AdjustTaxPolicy(
        mGovernmentProfile.TaxPolicy,
        Type,
        DeltaPercent,
        OutMessage);

    if (Adjusted)
    {
        RefreshForeignTradeDiplomacy(false);
        RefreshWorldMarketPrices();
    }

    return Adjusted;
}

bool CMainWorld::CycleAutoImportResource(
    std::wstring& OutMessage)
{
    TradePolicy::AdvanceImportPolicySelection(
        mGovernmentProfile.ImportTradePolicy);
    OutMessage =
        L"자동 수입 대상: " +
        BuildAutoImportSelectionText(
            mGovernmentProfile.ImportTradePolicy);
    return true;
}

bool CMainWorld::CycleImportPerResourceCap(
    std::wstring& OutMessage)
{
    TradePolicy::AdvanceImportResourceCapSelection(
        mGovernmentProfile.ImportTradePolicy);
    OutMessage =
        L"자원별 수입 한도: " +
        BuildImportCapSelectionText(
            mGovernmentProfile.ImportTradePolicy);
    return true;
}

bool CMainWorld::CycleImportBudgetPolicy(
    std::wstring& OutMessage)
{
    TradePolicy::AdvanceImportBudgetSelection(
        mGovernmentProfile.ImportTradePolicy);
    OutMessage =
        L"일일 수입 예산: " +
        BuildImportBudgetSelectionText(
            mGovernmentProfile.ImportTradePolicy);
    return true;
}

bool CMainWorld::CycleDomesticReservePolicy(
    std::wstring& OutMessage)
{
    TradePolicy::AdvanceDomesticReservePolicySelection(
        mGovernmentProfile.ExportTradePolicy);
    OutMessage =
        L"내수 비축 기준: " +
        BuildDomesticReserveSelectionText(
            mGovernmentProfile.ExportTradePolicy);
    return true;
}

bool CMainWorld::CycleExportBlockedResource(
    std::wstring& OutMessage)
{
    TradePolicy::AdvanceExportBlockedResourceSelection(
        mGovernmentProfile.ExportTradePolicy);
    OutMessage =
        L"수출 금지 자원: " +
        BuildExportBlockedSelectionText(
            mGovernmentProfile.ExportTradePolicy);
    return true;
}

bool CMainWorld::ExecuteTradeProposal(
    bool ImportRoute,
    EResourceType ResourceType,
    int ForeignPowerIndex,
    int PricePerThousandUnits,
    int Amount,
    std::wstring& OutMessage)
{
    auto World = mSelf.lock();

    if (!World)
    {
        OutMessage = L"월드 상태를 확인할 수 없습니다.";
        return false;
    }

    if (static_cast<int>(mTradeDiplomacyState.ActiveTradeRoutes.size()) >=
        GMaxActiveTradeRouteCount)
    {
        OutMessage = L"활성화할 수 있는 무역로가 가득 찼습니다.";
        return false;
    }

    if (!IsExportableResourceType(ResourceType))
    {
        OutMessage = L"유효하지 않은 자원 제안입니다.";
        return false;
    }

    const int SafeAmount = (std::max)(
        GTradeRouteMinAmountUnits,
        (std::min)(GTradeRouteMaxAmountUnits, Amount));

    if (SafeAmount < GTradeRouteMinAmountUnits)
    {
        OutMessage = L"제안 물량이 너무 작습니다.";
        return false;
    }

    const int SafePricePerThousand = (std::max)(1000, PricePerThousandUnits);
    const std::vector<std::shared_ptr<CPlacementAreaObject>> Harbors =
        MainWorldTradeRuntime::CollectOperationalHarbors(World);

    if (Harbors.empty())
    {
        OutMessage = L"운영 중인 항구가 없어 무역로를 활성화할 수 없습니다.";
        return false;
    }

    FTradeRouteRuntimeState Route;
    Route.RouteId = mTradeDiplomacyState.NextTradeRouteId++;
    Route.ImportRoute = ImportRoute;
    Route.ResourceType = ResourceType;
    Route.MarketClass = GetResourceMarketClass(ResourceType);
    Route.ForeignPowerIndex =
        (std::max)(0, (std::min)(4, ForeignPowerIndex));
    Route.ContractUnits = SafeAmount;
    Route.FulfilledUnits = 0;
    Route.TotalDurationDays =
        MainWorldTradeRuntime::ResolveTradeRouteDurationDays(SafeAmount);
    Route.RemainingDays = Route.TotalDurationDays;
    Route.RoutePricePerThousandUnits = SafePricePerThousand;
    Route.SignedStandardPricePerThousandUnits =
        MainWorldTradeRuntime::ComputeTradeRouteSignedStandardPricePerThousand(
            ResourceType,
            ImportRoute);
    mTradeDiplomacyState.ActiveTradeRoutes.push_back(Route);
    MainWorldTradeRuntime::ApplyTradeRouteActivationState(
        mTradeDiplomacyState.ForeignPowerStandingStates[static_cast<size_t>(
            Route.ForeignPowerIndex)],
        Route);

    RefreshForeignTradeDiplomacy(false);
    RefreshWorldMarketPrices();
    RefreshPoliticalSnapshot();
    OutMessage =
        std::wstring(MainWorldTradeRuntime::GetForeignPowerName(ForeignPowerIndex)) +
        L"과(와) " +
        GetResourceTypeDisplayName(ResourceType) +
        L" " +
        MainWorldTradeRuntime::FormatUnits(SafeAmount) +
        L" 단위 무역로 활성화";
    return true;
}

bool CMainWorld::CancelTradeRoute(
    int RouteId,
    std::wstring& OutMessage)
{
    const auto RouteIt = std::find_if(
        mTradeDiplomacyState.ActiveTradeRoutes.begin(),
        mTradeDiplomacyState.ActiveTradeRoutes.end(),
        [RouteId](const FTradeRouteRuntimeState& Route)
        {
            return Route.RouteId == RouteId;
        });

    if (RouteIt == mTradeDiplomacyState.ActiveTradeRoutes.end())
    {
        OutMessage = L"취소할 수 있는 무역 계약이 없습니다.";
        return false;
    }

    const std::wstring DirectionText =
        RouteIt->ImportRoute ? L"수입" : L"수출";
    const std::wstring ResourceName =
        GetResourceTypeDisplayName(RouteIt->ResourceType);

    RecordFinishedTradeRoute(*RouteIt, ETradeRouteEndReason::Cancelled);
    mTradeDiplomacyState.ActiveTradeRoutes.erase(RouteIt);
    RefreshForeignTradeDiplomacy(false);
    RefreshWorldMarketPrices();
    RefreshPoliticalSnapshot();
    OutMessage =
        DirectionText +
        L": " +
        ResourceName +
        L" 무역 계약 취소";
    return true;
}

bool CMainWorld::RespondPoliticalDemand(
    EPoliticalDemandIssuerType IssuerType,
    int IssuerIndex,
    bool Accept,
    std::wstring& OutMessage)
{
    auto World = mSelf.lock();

    if (!World)
    {
        OutMessage = L"월드 상태를 확인할 수 없습니다.";
        return false;
    }

    FPoliticalDemandState* Demand = nullptr;
    int* CooldownDays = nullptr;
    const int SafeFactionIndex =
        (std::max)(0, (std::min)(GPoliticalFactionCount - 1, IssuerIndex));
    const int SafeForeignIndex =
        (std::max)(
            0,
            (std::min)(TradeDiplomacyRuntime::GForeignPowerCount - 1, IssuerIndex));

    switch (IssuerType)
    {
    case EPoliticalDemandIssuerType::Faction:
        Demand = &mFactionDemands[static_cast<size_t>(SafeFactionIndex)];
        CooldownDays =
            &mFactionDemandCooldownDays[static_cast<size_t>(SafeFactionIndex)];
        break;
    case EPoliticalDemandIssuerType::ForeignPower:
        Demand = &mTradeDiplomacyState.ForeignPowerDemands[
            static_cast<size_t>(SafeForeignIndex)];
        CooldownDays =
            &mTradeDiplomacyState.ForeignDemandCooldownDays[
                static_cast<size_t>(SafeForeignIndex)];
        break;
    case EPoliticalDemandIssuerType::None:
    default:
        OutMessage = L"요구 발신자를 확인할 수 없습니다.";
        return false;
    }

    if (!Demand || !Demand->Active)
    {
        OutMessage = L"현재 응답할 수 있는 요구가 없습니다.";
        return false;
    }

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(World);

    auto ApplyFactionModifier =
        [&](int FactionIndex, int Delta, int DurationDays)
        {
            if (Delta == 0 ||
                FactionIndex < 0 ||
                FactionIndex >= GPoliticalFactionCount)
            {
                return;
            }

            int& Modifier =
                mGovernmentProfile.FactionApprovalModifiers[
                    static_cast<size_t>(FactionIndex)];
            Modifier = (std::max)(-25, (std::min)(25, Modifier + Delta));
            mFactionDemandModifierDays[static_cast<size_t>(FactionIndex)] =
                (std::max)(
                    mFactionDemandModifierDays[
                        static_cast<size_t>(FactionIndex)],
                    (std::max)(1, DurationDays));
        };

    auto ApplyDemandOutcome =
        [&](bool Success, const wchar_t* StatusLabel)
        {
            const long long BudgetDelta =
                Success ? Demand->RewardBudgetDelta : Demand->PenaltyBudgetDelta;
            const int FactionApprovalDelta =
                Success ?
                    Demand->RewardFactionApprovalDelta :
                    Demand->PenaltyFactionApprovalDelta;
            const int RelationDelta =
                Success ?
                    Demand->RewardForeignRelationDelta :
                    Demand->PenaltyForeignRelationDelta;
            const int StandingDelta =
                Success ?
                    Demand->RewardForeignStandingDelta :
                    Demand->PenaltyForeignStandingDelta;

            ApplyPoliticalDemandBudgetDelta(
                BudgetDelta,
                mNationalBudget,
                mLastDailyNetChange);

            if (Demand->IssuerType == EPoliticalDemandIssuerType::Faction)
            {
                ApplyFactionModifier(
                    Demand->IssuerIndex,
                    FactionApprovalDelta,
                    Demand->ModifierDurationDays > 0 ?
                        Demand->ModifierDurationDays :
                        GFactionDemandModifierDurationDays);
            }
            else if (Demand->IssuerType == EPoliticalDemandIssuerType::ForeignPower &&
                Demand->IssuerIndex >= 0 &&
                Demand->IssuerIndex < TradeDiplomacyRuntime::GForeignPowerCount)
            {
                MainWorldTradeRuntime::ApplyForeignDemandStandingDelta(
                    mTradeDiplomacyState.ForeignPowerStandingStates[
                        static_cast<size_t>(Demand->IssuerIndex)],
                    RelationDelta,
                    StandingDelta);
            }

            if (CooldownDays)
            {
                *CooldownDays =
                    Demand->IssuerType == EPoliticalDemandIssuerType::Faction ?
                        GFactionDemandCooldownDays :
                        GForeignDemandCooldownDays;
            }

            SetPoliticalDemandResolutionNotice(
                mPoliticalDemandNotice,
                *Demand,
                Success,
                StatusLabel);
        *Demand = FPoliticalDemandState();
        RefreshPoliticalSnapshot();
        RefreshForeignTradeDiplomacy(false);
        RefreshWorldMarketPrices();
        };

    if (!Accept)
    {
        const std::wstring DemandTitle = Demand->Title;
        ApplyDemandOutcome(
            false,
            Demand->Status == EPoliticalDemandStatus::Accepted ?
                L"요구 포기" :
                L"요구 거절");
        OutMessage =
            DemandTitle.empty() ?
                L"요구를 거절했습니다." :
                DemandTitle + L" 거절";
        return true;
    }

    if (Demand->Status == EPoliticalDemandStatus::Accepted)
    {
        OutMessage = L"이미 수락한 요구입니다.";
        return false;
    }

    Demand->Status = EPoliticalDemandStatus::Accepted;
    Demand->CurrentValue = EvaluatePoliticalDemandCurrentValue(
        *Demand,
        Snapshot,
        mGovernmentProfile,
        mLastDailyExportIncome,
        mTradeDiplomacyState.ForeignPowerStates,
        mTradeDiplomacyState.ActiveTradeRoutes);
    OutMessage = Demand->Title + L" 수락";

    if (IsPoliticalDemandSatisfied(*Demand))
    {
        ApplyDemandOutcome(true, L"요구 완료");
        OutMessage += L" / 즉시 완료";
        return true;
    }

    mPoliticalDemandNotice =
        BuildPriorityDemandNotice(
            mFactionDemands,
            mTradeDiplomacyState.ForeignPowerDemands);
    return true;
}

int CMainWorld::GetCustomsExportTradePriceModifierPercent() const
{
    return MainWorldTradeRuntime::CollectCustomsTradeModifierSummary(
        const_cast<CMainWorld*>(this)).ExportPricePercent;
}

int CMainWorld::GetCustomsImportTradePriceModifierPercent() const
{
    return MainWorldTradeRuntime::CollectCustomsTradeModifierSummary(
        const_cast<CMainWorld*>(this)).ImportPricePercent;
}

const FGovernmentEdictState* CMainWorld::GetGovernmentEdictState(
    EGovernmentEdictType Type) const
{
    for (size_t i = 0; i < mGovernmentEdicts.size(); ++i)
    {
        if (mGovernmentEdicts[i].Type == Type)
            return &mGovernmentEdicts[i];
    }

    return nullptr;
}

void CMainWorld::TickGovernmentEdicts()
{
    bool ModifiersChanged = false;

    for (size_t i = 0; i < mGovernmentEdicts.size(); ++i)
    {
        FGovernmentEdictState& State = mGovernmentEdicts[i];
        const FGovernmentEdictDefinition* Definition =
            EdictSystem::FindGovernmentEdictDefinition(State.Type);

        if (!Definition)
            continue;

        if (State.Active &&
            Definition->Mode == EGovernmentEdictMode::Active &&
            State.RemainingDays > 0)
        {
            --State.RemainingDays;

            if (State.RemainingDays <= 0)
            {
                State.Active = false;
                State.RemainingDays = 0;
                PoliticsSystem::SyncGovernmentActionFromEdict(
                    mGovernmentProfile,
                    State.Type,
                    false);
                ModifiersChanged = true;
            }
        }

        if (!State.Active && State.CooldownDays > 0)
            --State.CooldownDays;
    }

    if (ModifiersChanged)
    {
        RefreshEdictModifiers();
        RefreshPoliticalSnapshot();
        RefreshForeignTradeDiplomacy(false);
    }
}

void CMainWorld::RefreshEdictModifiers()
{
    mEdictModifiers = EdictSystem::CalculateEdictModifiers(
        mGovernmentEdicts,
        mPoliticalSnapshot.ActiveCitizenCount);
    mGovernmentProfile.EdictFactionApprovalModifiers =
        mEdictModifiers.FactionApprovalModifiers;
    RefreshWorldMarketPrices();
}

void CMainWorld::ApplyDailyEdictCitizenEffects()
{
    PoliticsSystem::ApplyDailyEdictCitizenEffects(
        this,
        mEdictModifiers);
}

void CMainWorld::ApplyDailyTaxPolicyEventEffects()
{
    EconomySystem::ApplyDailyTaxPolicyEventEffects(
        this,
        mTaxEventStatus);
}

void CMainWorld::ApplyDailyWorldCrisisEffects()
{
    if (!mWorldCrisisStatus.Active ||
        mWorldCrisisStatus.Type == EWorldCrisisType::None)
    {
        return;
    }

    const double Severity = GetWorldCrisisSeverity(mWorldCrisisStatus);
    const double ChainIntensity =
        1.0 + static_cast<double>(mActiveWorldCrisisChainDepth) * 0.14;
    auto World = mSelf.lock();

    if (World)
    {
        std::vector<std::weak_ptr<CBuildingMarkerOrb>> CitizenList;

        if (World->FindObjectListByType<CBuildingMarkerOrb>(CitizenList))
        {
            float FoodDelta = 0.f;
            float HealthDelta = 0.f;
            float FunDelta = 0.f;
            float FaithDelta = 0.f;
            float HousingDelta = 0.f;
            float JobDelta = 0.f;
            float FreedomDelta = 0.f;
            float SecurityDelta = 0.f;

            switch (mWorldCrisisStatus.Type)
            {
            case EWorldCrisisType::Raid:
                FoodDelta = static_cast<float>(
                    (-0.35 - 0.55 * Severity) * ChainIntensity);
                FunDelta = static_cast<float>(
                    (-0.45 - 0.45 * Severity) * ChainIntensity);
                HousingDelta = static_cast<float>(
                    (-0.30 - 0.40 * Severity) * ChainIntensity);
                JobDelta = static_cast<float>(
                    (-0.70 - 0.90 * Severity) * ChainIntensity);
                FreedomDelta = static_cast<float>(
                    (-0.20 - 0.30 * Severity) * ChainIntensity);
                SecurityDelta = static_cast<float>(
                    (-2.20 - 2.80 * Severity) * ChainIntensity);
                break;
            case EWorldCrisisType::LaborStrike:
                FunDelta = static_cast<float>(
                    (-0.30 - 0.35 * Severity) * ChainIntensity);
                JobDelta = static_cast<float>(
                    (-1.40 - 1.60 * Severity) * ChainIntensity);
                FreedomDelta =
                    0.08f + static_cast<float>(0.02 * mActiveWorldCrisisChainDepth);
                SecurityDelta = static_cast<float>(
                    (-0.80 - 0.90 * Severity) * ChainIntensity);
                break;
            case EWorldCrisisType::CrimeWave:
                FunDelta = static_cast<float>(
                    (-0.35 - 0.30 * Severity) * ChainIntensity);
                HousingDelta = static_cast<float>(
                    (-0.55 - 0.45 * Severity) * ChainIntensity);
                JobDelta = static_cast<float>(
                    (-0.60 - 0.55 * Severity) * ChainIntensity);
                FreedomDelta = static_cast<float>(
                    (-0.30 - 0.30 * Severity) * ChainIntensity);
                SecurityDelta = static_cast<float>(
                    (-2.60 - 2.60 * Severity) * ChainIntensity);
                break;
            case EWorldCrisisType::FiscalEmergency:
                FoodDelta = static_cast<float>(
                    (-0.35 - 0.40 * Severity) * ChainIntensity);
                FunDelta = static_cast<float>(
                    (-0.45 - 0.40 * Severity) * ChainIntensity);
                HousingDelta = static_cast<float>(
                    (-0.40 - 0.40 * Severity) * ChainIntensity);
                JobDelta = static_cast<float>(
                    (-0.90 - 1.00 * Severity) * ChainIntensity);
                FreedomDelta = static_cast<float>(
                    (-0.18 - 0.18 * Severity) * ChainIntensity);
                SecurityDelta = static_cast<float>(
                    (-0.60 - 0.70 * Severity) * ChainIntensity);
                break;
            case EWorldCrisisType::None:
            default:
                break;
            }

            for (size_t Index = 0; Index < CitizenList.size(); ++Index)
            {
                auto Citizen = CitizenList[Index].lock();

                if (!Citizen || !Citizen->GetAlive())
                    continue;

                Citizen->ApplySatisfactionDelta(
                    FoodDelta,
                    HealthDelta,
                    FunDelta,
                    FaithDelta,
                    HousingDelta,
                    JobDelta,
                    FreedomDelta,
                    SecurityDelta);
            }
        }
    }

    auto ApplyBudgetDelta = [&](long long Delta)
    {
        if (Delta == 0)
            return;

        mNationalBudget += Delta;
        mLastDailyNetChange += Delta;
    };

    auto ApplyTaxLoss = [&](long long& TaxField, double LossRatio)
    {
        if (TaxField <= 0 || LossRatio <= 0.0)
            return;

        const long long LossAmount = static_cast<long long>(llround(
            static_cast<double>(TaxField) * LossRatio));
        const long long ClampedLoss = (std::min)(TaxField, LossAmount);

        if (ClampedLoss <= 0)
            return;

        TaxField -= ClampedLoss;
        mLastDailyTaxIncome = (std::max)(0LL, mLastDailyTaxIncome - ClampedLoss);
        ApplyBudgetDelta(-ClampedLoss);
    };

    switch (mWorldCrisisStatus.Type)
    {
    case EWorldCrisisType::Raid:
    {
        const int TargetAmount =
            8 + static_cast<int>(round(10.0 * Severity));
        const int StolenAmount =
            ApplyRaidResourceTheft(World.get(), TargetAmount);
        const long long BudgetDamage =
            -(1200LL +
                static_cast<long long>(llround(1800.0 * Severity * ChainIntensity)) +
                static_cast<long long>(StolenAmount) * 42LL);
        ApplyBudgetDelta(BudgetDamage);
        break;
    }
    case EWorldCrisisType::LaborStrike:
        ApplyBudgetDelta(
            -(250LL +
                static_cast<long long>(llround(550.0 * Severity * ChainIntensity))));
        break;
    case EWorldCrisisType::CrimeWave:
        ApplyTaxLoss(mLastDailyPropertyTaxIncome, 0.08 + 0.12 * Severity);
        ApplyBudgetDelta(
            -(550LL +
                static_cast<long long>(llround(1450.0 * Severity * ChainIntensity))));
        break;
    case EWorldCrisisType::FiscalEmergency:
        ApplyTaxLoss(mLastDailyConsumptionTaxIncome, 0.06 + 0.08 * Severity);
        ApplyTaxLoss(mLastDailyIncomeTaxIncome, 0.08 + 0.10 * Severity);
        ApplyTaxLoss(mLastDailyPropertyTaxIncome, 0.05 + 0.08 * Severity);
        ApplyBudgetDelta(
            -(1200LL +
                static_cast<long long>(llround(2600.0 * Severity * ChainIntensity))));
        break;
    case EWorldCrisisType::None:
    default:
        break;
    }
}

void CMainWorld::TickTaxPolicyEvents()
{
    EconomySystem::TickTaxPolicyEvents(
        mPoliticalSnapshot,
        mGovernmentProfile,
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay,
        mNationalBudget,
        mLastDailyNetChange,
        mWorkerTaxPressureDays,
        mPropertyTaxPressureDays,
        mBudgetCrisisPressureDays,
        mTaxEventStatus);
}

void CMainWorld::TickWorldCrises()
{
    if (mWorldCrisisStatus.NotificationDays > 0)
        --mWorldCrisisStatus.NotificationDays;

    if (!mWorldCrisisStatus.Active && mWorldCrisisStatus.CooldownDays > 0)
        --mWorldCrisisStatus.CooldownDays;

    if (mQueuedWorldCrisisType != EWorldCrisisType::None &&
        mQueuedWorldCrisisDelayDays > 0)
    {
        --mQueuedWorldCrisisDelayDays;
    }

    auto World = mSelf.lock();

    if (!World)
        return;

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(World);
    const FWorldCrisisPressureSnapshot Pressure =
        BuildWorldCrisisPressureSnapshot(
            Snapshot,
            mPoliticalSnapshot,
            mNationalBudget,
            mLastDailyNetChange,
            mLastDailyTaxCollectionEfficiency,
            mGovernmentEdicts,
            mGovernmentProfile.TaxPolicy,
            mTaxEventStatus);
    auto ClearQueuedWorldCrisis = [&]()
    {
        mQueuedWorldCrisisType = EWorldCrisisType::None;
        mQueuedWorldCrisisRisk = 0.0;
        mQueuedWorldCrisisDelayDays = 0;
        mQueuedWorldCrisisChainDepth = 0;
    };

    if (mWorldCrisisStatus.Active)
    {
        ++mWorldCrisisStatus.DaysActive;
        const double Severity = GetWorldCrisisSeverity(mWorldCrisisStatus);
        const EWorldCrisisType FollowupType =
            ResolveWorldCrisisFollowupType(
                mWorldCrisisStatus.Type,
                Pressure,
                false);
        const double FollowupRisk =
            ResolveWorldCrisisFollowupRisk(
                mWorldCrisisStatus.Type,
                Pressure,
                Severity,
                false);
        mWorldCrisisStatus.Summary = BuildWorldCrisisWarningSummary(
            mWorldCrisisStatus.Type,
            mWorldCrisisStatus.DaysActive);

        if (mActiveWorldCrisisChainDepth > 0)
        {
            mWorldCrisisStatus.Summary +=
                L" / 연쇄 단계 " +
                std::to_wstring(mActiveWorldCrisisChainDepth + 1);
        }

        if (mWorldCrisisStatus.DaysActive >= 2 && FollowupRisk >= 0.54)
        {
            mWorldCrisisStatus.Summary +=
                BuildWorldCrisisFollowupSummary(
                    FollowupType,
                    FollowupRisk,
                    2);
        }

        ApplyWorldCrisisPressureTransfer(
            mWorldCrisisStatus.Type,
            Severity,
            false,
            mRaidPressureDays,
            mLaborStrikePressureDays,
            mCrimeWavePressureDays,
            mFiscalEmergencyPressureDays);

        if (mWorldCrisisStatus.RemainingDays > 0)
            --mWorldCrisisStatus.RemainingDays;

        const bool CanResolveEarly = mWorldCrisisStatus.DaysActive >= 3;
        bool Recovered = false;

        switch (mWorldCrisisStatus.Type)
        {
        case EWorldCrisisType::Raid:
            Recovered =
                Snapshot.AverageSecurity >=
                    (Pressure.MartialLawActive ? 52.0 : 58.0) &&
                Pressure.RaidRisk < 0.38 &&
                Pressure.OppositionRatio < 0.44;
            break;
        case EWorldCrisisType::LaborStrike:
            Recovered =
                Pressure.UnemploymentRatio < 0.10 &&
                Snapshot.AverageJob >= 54.0 &&
                Pressure.LaborStrikeRisk < 0.40 &&
                (!mTaxEventStatus.Active ||
                    mTaxEventStatus.Type != ETaxPolicyEventType::WorkerTaxStrike);
            break;
        case EWorldCrisisType::CrimeWave:
            Recovered =
                Snapshot.AverageSecurity >=
                    (Pressure.MartialLawActive ? 50.0 : 55.0) &&
                Pressure.HomelessRatio < 0.09 &&
                Pressure.CrimeWaveRisk < 0.40;
            break;
        case EWorldCrisisType::FiscalEmergency:
            Recovered =
                mNationalBudget >= 15000 &&
                mLastDailyNetChange >= -400 &&
                mLastDailyTaxCollectionEfficiency >= 0.72 &&
                Pressure.FiscalEmergencyRisk < 0.38 &&
                (!mTaxEventStatus.Active ||
                    mTaxEventStatus.Type != ETaxPolicyEventType::BudgetCrisis);
            break;
        case EWorldCrisisType::None:
        default:
            break;
        }

        if (CanResolveEarly && Recovered)
        {
            ResolveWorldCrisisState(mWorldCrisisStatus, true);
            ClearQueuedWorldCrisis();
            mActiveWorldCrisisChainDepth = 0;
            return;
        }

        if (mWorldCrisisStatus.RemainingDays <= 0)
        {
            const EWorldCrisisType QueuedType =
                ResolveWorldCrisisFollowupType(
                    mWorldCrisisStatus.Type,
                    Pressure,
                    true);
            const double QueuedRisk =
                ResolveWorldCrisisFollowupRisk(
                    mWorldCrisisStatus.Type,
                    Pressure,
                    Severity,
                    true);

            if (QueuedType != EWorldCrisisType::None && QueuedRisk >= 0.48)
            {
                mQueuedWorldCrisisType = QueuedType;
                mQueuedWorldCrisisRisk = QueuedRisk;
                mQueuedWorldCrisisDelayDays =
                    (std::max)(1, 3 - (std::min)(2, mActiveWorldCrisisChainDepth));
                mQueuedWorldCrisisChainDepth =
                    mActiveWorldCrisisChainDepth + 1;
                ApplyWorldCrisisPressureTransfer(
                    mWorldCrisisStatus.Type,
                    Severity,
                    true,
                    mRaidPressureDays,
                    mLaborStrikePressureDays,
                    mCrimeWavePressureDays,
                    mFiscalEmergencyPressureDays);
            }
            else
            {
                ClearQueuedWorldCrisis();
            }

            ResolveWorldCrisisState(mWorldCrisisStatus, false);

            if (mQueuedWorldCrisisType != EWorldCrisisType::None)
            {
                mWorldCrisisStatus.Summary +=
                    BuildWorldCrisisFollowupSummary(
                        mQueuedWorldCrisisType,
                        mQueuedWorldCrisisRisk,
                        mQueuedWorldCrisisDelayDays);
            }

            mActiveWorldCrisisChainDepth = 0;
        }

        return;
    }

    if (mQueuedWorldCrisisType != EWorldCrisisType::None &&
        mQueuedWorldCrisisDelayDays <= 0)
    {
        const EWorldCrisisType QueuedType = mQueuedWorldCrisisType;
        const double QueuedRisk =
            Clamp<double>(mQueuedWorldCrisisRisk, 0.0, 1.0);
        const int QueuedChainDepth = (std::max)(1, mQueuedWorldCrisisChainDepth);
        const long long ImmediateBudgetDelta =
            ResolveWorldCrisisImmediateBudgetDelta(
                QueuedType,
                QueuedRisk);

        ClearQueuedWorldCrisis();
        StartWorldCrisis(
            mWorldCrisisStatus,
            QueuedType,
            mSimulationYear,
            mSimulationMonth,
            mSimulationDay,
            ImmediateBudgetDelta,
            mNationalBudget,
            mLastDailyNetChange,
            mRaidPressureDays,
            mLaborStrikePressureDays,
            mCrimeWavePressureDays,
            mFiscalEmergencyPressureDays,
            true);
        mActiveWorldCrisisChainDepth = QueuedChainDepth;

        if (mWorldCrisisStatus.Active && mActiveWorldCrisisChainDepth > 0)
        {
            mWorldCrisisStatus.Title +=
                L" (연쇄 " +
                std::to_wstring(mActiveWorldCrisisChainDepth + 1) +
                L")";
            mWorldCrisisStatus.Summary +=
                L" / 이전 위기의 후폭풍이 이어지고 있습니다.";
        }

        return;
    }

    if (mWorldCrisisStatus.CooldownDays > 0)
        return;

    auto TickPressure = [](double Risk, double TriggerRisk, int& InOutDays)
    {
        if (Risk >= TriggerRisk)
            ++InOutDays;
        else
            InOutDays = (std::max)(0, InOutDays - (Risk < TriggerRisk * 0.65 ? 2 : 1));
    };

    TickPressure(Pressure.RaidRisk, 0.58, mRaidPressureDays);
    TickPressure(Pressure.LaborStrikeRisk, 0.56, mLaborStrikePressureDays);
    TickPressure(Pressure.CrimeWaveRisk, 0.55, mCrimeWavePressureDays);
    TickPressure(Pressure.FiscalEmergencyRisk, 0.58, mFiscalEmergencyPressureDays);

    if (Snapshot.ActiveCitizenCount < 24 || Snapshot.TotalBuildingCount < 8)
        mRaidPressureDays = 0;
    if (Snapshot.ActiveCitizenCount < 28 || Snapshot.AssignedJobCount < 12)
        mLaborStrikePressureDays = 0;
    if (Snapshot.ActiveCitizenCount < 20 || Snapshot.TotalBuildingCount < 10)
        mCrimeWavePressureDays = 0;
    if (Snapshot.ActiveCitizenCount < 32 &&
        mNationalBudget >= 0 &&
        mLastDailyNetChange >= 0)
    {
        mFiscalEmergencyPressureDays = 0;
    }

    if (mTaxEventStatus.Active)
    {
        if (mTaxEventStatus.Type == ETaxPolicyEventType::WorkerTaxStrike)
            mLaborStrikePressureDays = 0;
        if (mTaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis)
            mFiscalEmergencyPressureDays = 0;
    }

    EWorldCrisisType NextType = EWorldCrisisType::None;
    double NextRisk = 0.0;

    auto ConsiderCrisis = [&](EWorldCrisisType Type, int PressureDays, int RequiredDays, double Risk)
    {
        if (PressureDays < RequiredDays || Risk <= NextRisk)
            return;

        NextType = Type;
        NextRisk = Risk;
    };

    ConsiderCrisis(EWorldCrisisType::Raid, mRaidPressureDays, 4, Pressure.RaidRisk);
    ConsiderCrisis(
        EWorldCrisisType::LaborStrike,
        mLaborStrikePressureDays,
        3,
        Pressure.LaborStrikeRisk);
    ConsiderCrisis(
        EWorldCrisisType::CrimeWave,
        mCrimeWavePressureDays,
        3,
        Pressure.CrimeWaveRisk);
    ConsiderCrisis(
        EWorldCrisisType::FiscalEmergency,
        mFiscalEmergencyPressureDays,
        4,
        Pressure.FiscalEmergencyRisk);

    if (NextType == EWorldCrisisType::None)
        return;

    const long long ImmediateBudgetDelta =
        ResolveWorldCrisisImmediateBudgetDelta(NextType, NextRisk);

    ClearQueuedWorldCrisis();
    StartWorldCrisis(
        mWorldCrisisStatus,
        NextType,
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay,
        ImmediateBudgetDelta,
        mNationalBudget,
        mLastDailyNetChange,
        mRaidPressureDays,
        mLaborStrikePressureDays,
        mCrimeWavePressureDays,
        mFiscalEmergencyPressureDays,
        false);
    mActiveWorldCrisisChainDepth = 0;
}

void CMainWorld::TickPoliticalDemands()
{
    auto World = mSelf.lock();

    if (!World)
        return;

    if (!mPoliticalDemandNotice.ActiveDemand &&
        mPoliticalDemandNotice.RemainingDays > 0)
    {
        --mPoliticalDemandNotice.RemainingDays;

        if (mPoliticalDemandNotice.RemainingDays <= 0)
            mPoliticalDemandNotice = FPoliticalDemandNotice();
    }

    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        if (mFactionDemandCooldownDays[static_cast<size_t>(Index)] > 0)
            --mFactionDemandCooldownDays[static_cast<size_t>(Index)];
    }

    for (int Index = 0;
        Index < TradeDiplomacyRuntime::GForeignPowerCount;
        ++Index)
    {
        if (mTradeDiplomacyState.ForeignDemandCooldownDays[
                static_cast<size_t>(Index)] > 0)
        {
            --mTradeDiplomacyState.ForeignDemandCooldownDays[
                static_cast<size_t>(Index)];
        }
    }

    bool PoliticalRefreshNeeded = false;
    bool ForeignRefreshNeeded = false;

    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        int& RemainingDays =
            mFactionDemandModifierDays[static_cast<size_t>(Index)];

        if (RemainingDays <= 0)
            continue;

        --RemainingDays;

        if (RemainingDays <= 0 &&
            mGovernmentProfile.FactionApprovalModifiers[
                static_cast<size_t>(Index)] != 0)
        {
            mGovernmentProfile.FactionApprovalModifiers[
                static_cast<size_t>(Index)] = 0;
            PoliticalRefreshNeeded = true;
        }
    }

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(World);

    const auto ApplyFactionModifier =
        [&](int FactionIndex, int Delta, int DurationDays)
        {
            if (Delta == 0 ||
                FactionIndex < 0 ||
                FactionIndex >= GPoliticalFactionCount)
            {
                return;
            }

            int& Modifier =
                mGovernmentProfile.FactionApprovalModifiers[
                    static_cast<size_t>(FactionIndex)];
            Modifier = (std::max)(-25, (std::min)(25, Modifier + Delta));
            mFactionDemandModifierDays[static_cast<size_t>(FactionIndex)] =
                (std::max)(
                    mFactionDemandModifierDays[
                        static_cast<size_t>(FactionIndex)],
                    (std::max)(1, DurationDays));
        };

    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        FPoliticalDemandState& Demand =
            mFactionDemands[static_cast<size_t>(Index)];

        if (!Demand.Active)
            continue;

        Demand.CurrentValue = EvaluatePoliticalDemandCurrentValue(
            Demand,
            Snapshot,
            mGovernmentProfile,
            mLastDailyExportIncome,
            mTradeDiplomacyState.ForeignPowerStates,
            mTradeDiplomacyState.ActiveTradeRoutes);

        if (Demand.Status == EPoliticalDemandStatus::Accepted &&
            IsPoliticalDemandSatisfied(Demand))
        {
            ApplyPoliticalDemandBudgetDelta(
                Demand.RewardBudgetDelta,
                mNationalBudget,
                mLastDailyNetChange);
            ApplyFactionModifier(
                Demand.IssuerIndex,
                Demand.RewardFactionApprovalDelta,
                Demand.ModifierDurationDays > 0 ?
                    Demand.ModifierDurationDays :
                    GFactionDemandModifierDurationDays);
            mFactionDemandCooldownDays[static_cast<size_t>(Index)] =
                GFactionDemandCooldownDays;
            SetPoliticalDemandResolutionNotice(
                mPoliticalDemandNotice,
                Demand,
                true,
                L"요구 완료");
            Demand = FPoliticalDemandState();
            PoliticalRefreshNeeded = true;
            continue;
        }

        if (Demand.RemainingDays > 0)
            --Demand.RemainingDays;

        if (Demand.RemainingDays > 0)
            continue;

        ApplyPoliticalDemandBudgetDelta(
            Demand.PenaltyBudgetDelta,
            mNationalBudget,
            mLastDailyNetChange);
        ApplyFactionModifier(
            Demand.IssuerIndex,
            Demand.PenaltyFactionApprovalDelta,
            Demand.ModifierDurationDays > 0 ?
                Demand.ModifierDurationDays :
                GFactionDemandModifierDurationDays);
        mFactionDemandCooldownDays[static_cast<size_t>(Index)] =
            GFactionDemandCooldownDays;
        SetPoliticalDemandResolutionNotice(
            mPoliticalDemandNotice,
            Demand,
            false,
            Demand.Status == EPoliticalDemandStatus::Accepted ?
                L"요구 실패" :
                L"요구 만료");
        Demand = FPoliticalDemandState();
        PoliticalRefreshNeeded = true;
    }

    for (int Index = 0;
        Index < TradeDiplomacyRuntime::GForeignPowerCount;
        ++Index)
    {
        FPoliticalDemandState& Demand =
            mTradeDiplomacyState.ForeignPowerDemands[
                static_cast<size_t>(Index)];

        if (!Demand.Active)
            continue;

        Demand.CurrentValue = EvaluatePoliticalDemandCurrentValue(
            Demand,
            Snapshot,
            mGovernmentProfile,
            mLastDailyExportIncome,
            mTradeDiplomacyState.ForeignPowerStates,
            mTradeDiplomacyState.ActiveTradeRoutes);

        if (Demand.Status == EPoliticalDemandStatus::Accepted &&
            IsPoliticalDemandSatisfied(Demand))
        {
            ApplyPoliticalDemandBudgetDelta(
                Demand.RewardBudgetDelta,
                mNationalBudget,
                mLastDailyNetChange);
            MainWorldTradeRuntime::ApplyForeignDemandStandingDelta(
                mTradeDiplomacyState.ForeignPowerStandingStates[
                    static_cast<size_t>(Index)],
                Demand.RewardForeignRelationDelta,
                Demand.RewardForeignStandingDelta);
            mTradeDiplomacyState.ForeignDemandCooldownDays[
                static_cast<size_t>(Index)] =
                GForeignDemandCooldownDays;
            SetPoliticalDemandResolutionNotice(
                mPoliticalDemandNotice,
                Demand,
                true,
                L"요구 완료");
            Demand = FPoliticalDemandState();
            ForeignRefreshNeeded = true;
            continue;
        }

        if (Demand.RemainingDays > 0)
            --Demand.RemainingDays;

        if (Demand.RemainingDays > 0)
            continue;

        ApplyPoliticalDemandBudgetDelta(
            Demand.PenaltyBudgetDelta,
            mNationalBudget,
            mLastDailyNetChange);
        MainWorldTradeRuntime::ApplyForeignDemandStandingDelta(
            mTradeDiplomacyState.ForeignPowerStandingStates[
                static_cast<size_t>(Index)],
            Demand.PenaltyForeignRelationDelta,
            Demand.PenaltyForeignStandingDelta);
        mTradeDiplomacyState.ForeignDemandCooldownDays[
            static_cast<size_t>(Index)] =
            GForeignDemandCooldownDays;
        SetPoliticalDemandResolutionNotice(
            mPoliticalDemandNotice,
            Demand,
            false,
            Demand.Status == EPoliticalDemandStatus::Accepted ?
                L"요구 실패" :
                L"요구 만료");
        Demand = FPoliticalDemandState();
        ForeignRefreshNeeded = true;
    }

    if (CountActiveFactionDemands(mFactionDemands) <
        GMaxActiveFactionDemandCount)
    {
        bool FoundDemand = false;
        FPoliticalDemandState BestDemand;
        int BestIndex = -1;
        double BestPriority = 0.0;

        for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
        {
            if (mFactionDemands[static_cast<size_t>(Index)].Active ||
                mFactionDemandCooldownDays[static_cast<size_t>(Index)] > 0)
            {
                continue;
            }

            FPoliticalDemandState CandidateDemand;
            double CandidatePriority = 0.0;

            if (!TryBuildFactionDemand(
                    static_cast<EPoliticalFaction>(Index),
                    Snapshot,
                    mPoliticalSnapshot,
                    mGovernmentProfile,
                    mLastDailyExportIncome,
                    CandidateDemand,
                    CandidatePriority))
            {
                continue;
            }

            if (!FoundDemand || CandidatePriority > BestPriority)
            {
                FoundDemand = true;
                BestDemand = std::move(CandidateDemand);
                BestIndex = Index;
                BestPriority = CandidatePriority;
            }
        }

        if (FoundDemand && BestIndex >= 0)
        {
            mFactionDemands[static_cast<size_t>(BestIndex)] =
                std::move(BestDemand);
        }
    }

    if (CountActiveForeignDemands(
            mTradeDiplomacyState.ForeignPowerDemands) <
        GMaxActiveForeignDemandCount)
    {
        bool FoundDemand = false;
        FPoliticalDemandState BestDemand;
        int BestIndex = -1;
        double BestPriority = 0.0;

        for (int Index = 0;
            Index < TradeDiplomacyRuntime::GForeignPowerCount;
            ++Index)
        {
            if (mTradeDiplomacyState.ForeignPowerDemands[
                    static_cast<size_t>(Index)].Active ||
                mTradeDiplomacyState.ForeignDemandCooldownDays[
                    static_cast<size_t>(Index)] > 0)
            {
                continue;
            }

            FPoliticalDemandState CandidateDemand;
            double CandidatePriority = 0.0;

            if (!TryBuildForeignDemand(
                    Index,
                    Snapshot,
                    mTradeDiplomacyState.ForeignPowerStates,
                    mTradeDiplomacyState.ActiveTradeRoutes,
                    CandidateDemand,
                    CandidatePriority))
            {
                continue;
            }

            if (!FoundDemand || CandidatePriority > BestPriority)
            {
                FoundDemand = true;
                BestDemand = std::move(CandidateDemand);
                BestIndex = Index;
                BestPriority = CandidatePriority;
            }
        }

        if (FoundDemand && BestIndex >= 0)
        {
            mTradeDiplomacyState.ForeignPowerDemands[
                static_cast<size_t>(BestIndex)] =
                std::move(BestDemand);
        }
    }

    if (PoliticalRefreshNeeded)
        RefreshPoliticalSnapshot();

    if (ForeignRefreshNeeded)
    {
        RefreshForeignTradeDiplomacy(false);
        RefreshWorldMarketPrices();
    }

    const FPoliticalDemandNotice ActiveNotice =
        BuildPriorityDemandNotice(
            mFactionDemands,
            mTradeDiplomacyState.ForeignPowerDemands);

    if (ActiveNotice.ActiveDemand)
    {
        mPoliticalDemandNotice = ActiveNotice;
    }
    else if (mPoliticalDemandNotice.ActiveDemand)
    {
        mPoliticalDemandNotice = FPoliticalDemandNotice();
    }
}

void CMainWorld::RefreshEraProgress()
{
    const std::shared_ptr<CWorld> World = mSelf.lock();

    if (!World)
        return;

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(World);

    EBuildingEra CurrentEra = mEraProgress.CurrentEra;

    while (HasNextBuildingEra(CurrentEra))
    {
        const EBuildingEra NextEra = GetNextBuildingEra(CurrentEra);
        const FEraUnlockRequirement Requirement =
            ResolveEraUnlockRequirement(NextEra);

        if (!MeetsEraUnlockRequirement(Snapshot, Requirement))
            break;

        CurrentEra = NextEra;
    }

    mEraProgress = FEraProgressState();
    mEraProgress.CurrentEra = CurrentEra;
    mEraProgress.Population = Snapshot.ActiveCitizenCount;
    mEraProgress.TotalBuildings = Snapshot.TotalBuildingCount;
    mEraProgress.FoodProviders = Snapshot.FoodProviderCount;
    mEraProgress.IndustryBuildings =
        ResolveIndustryBuildingCount(Snapshot);
    mEraProgress.PublicServiceBuildings =
        ResolvePublicServiceBuildingCount(Snapshot);
    mEraProgress.EntertainmentBuildings =
        ResolveEntertainmentBuildingCount(Snapshot);
    mEraProgress.PowerMW = Snapshot.TotalProducedPowerMW;
    mEraProgress.HasNextEra = HasNextBuildingEra(CurrentEra);

    if (mEraProgress.HasNextEra)
    {
        mEraProgress.NextEra = GetNextBuildingEra(CurrentEra);
        mEraProgress.NextRequirement =
            ResolveEraUnlockRequirement(mEraProgress.NextEra);
    }
    else
    {
        mEraProgress.NextEra = EBuildingEra::Modern;
        mEraProgress.NextRequirement = FEraUnlockRequirement();
    }
}

void CMainWorld::RefreshPoliticalSnapshot()
{
    mPoliticalSnapshot = PoliticsSystem::EvaluateWorld(
        this,
        mGovernmentProfile,
        &mTaxEventStatus);
}
