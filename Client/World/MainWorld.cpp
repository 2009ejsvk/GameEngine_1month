#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "MainWorldInfrastructureRuntime.h"
#include "MainWorldTradeRuntime.h"
#include "RuntimeConfigRegistry.h"
#include "World/WorldUIManager.h"
#include "WorldStatsSnapshot.h"
#include "../GameConstants.h"
#include "../ObjectNames.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../Politics/ConstitutionSystem.h"
#include "../Politics/EdictSystem.h"
#include "../Politics/PoliticsSystem.h"
#include "../UI/EventWidget.h"
#include "../UI/ResultWidget.h"
#include "../Economy/EconomySystem.h"
#include "../Economy/ResourceTradePricing.h"
#include "../Economy/TradeDiplomacyRuntime.h"
#include "../StringUtils.h"
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
    namespace MWTrade = GameConstants::MainWorld::Trade;
    namespace MWTradeDiplomacy = GameConstants::MainWorld::TradeDiplomacy;

    constexpr const wchar_t* GEraConfigId = L"MainWorld.EraUnlockRequirements";
    constexpr int GEraTransitionNotificationDays = 6;
    std::array<
        FEraUnlockRequirement,
        static_cast<size_t>(EBuildingEra::Modern) + 1> GEraUnlockRequirements;

    std::wstring FormatPercentText(double Value, int DecimalPlaces)
    {
        wchar_t Buffer[64] = {};

        if (DecimalPlaces <= 0)
        {
            swprintf_s(Buffer, L"%d%%", static_cast<int>(std::lround(Value)));
            return Buffer;
        }

        swprintf_s(Buffer, L"%.1f%%", Value);
        return Buffer;
    }

    std::wstring BuildTenureText(
        int StartYear,
        int StartMonth,
        int StartDay,
        int CurrentYear,
        int CurrentMonth,
        int CurrentDay)
    {
        int TotalMonths =
            (CurrentYear - StartYear) * 12 +
            (CurrentMonth - StartMonth);

        if (CurrentDay < StartDay)
            --TotalMonths;

        TotalMonths = (std::max)(0, TotalMonths);
        const int Years = TotalMonths / 12;
        const int Months = TotalMonths % 12;

        return
            L"재임 기간: " +
            std::to_wstring(Years) +
            L"년 " +
            std::to_wstring(Months) +
            L"개월";
    }

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

    const wchar_t* GetTradeForeignPowerName(int Index, EBuildingEra Era)
    {
        return MainWorldTradeRuntime::GetForeignPowerName(Index, Era);
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

    bool IsEligibleRevoltDamageTarget(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        return Building &&
            Building->GetAlive() &&
            Building->GetEnable() &&
            Building->HasPlacedArea() &&
            !Building->IsRoad() &&
            Building->GetDamageLevel() != EBuildingDamageLevel::Critical;
    }

    bool IsFactionRevoltPriorityBuilding(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        EPoliticalFaction Faction)
    {
        if (!Building)
            return false;

        switch (Faction)
        {
        case EPoliticalFaction::Communists:
            return Building->IsResidential() ||
                Building->GetBuildingCategory() == EBuildingCategory::Housing;
        case EPoliticalFaction::Capitalists:
            return Building->GetBuildingCategory() == EBuildingCategory::Industry ||
                Building->GetBuildingCategory() ==
                    EBuildingCategory::GovernmentFinance;
        case EPoliticalFaction::Religious:
            return Building->IsFaithProvider();
        case EPoliticalFaction::Militarists:
            return Building->GetBuildingCategory() == EBuildingCategory::Military;
        case EPoliticalFaction::Environmentalists:
            return Building->IsHealthProvider() ||
                Building->GetBuildingCategory() ==
                    EBuildingCategory::PublicService ||
                Building->GetBuildingCategory() == EBuildingCategory::FoodResource;
        case EPoliticalFaction::Industrialists:
            return Building->GetBuildingCategory() == EBuildingCategory::Industry ||
                Building->CanGenerateWorkOutput();
        case EPoliticalFaction::Intellectuals:
            return Building->GetBuildingCategory() ==
                EBuildingCategory::MediaEducation;
        case EPoliticalFaction::Conservatives:
            return Building->GetBuildingCategory() ==
                    EBuildingCategory::GovernmentFinance ||
                Building->IsResidential();
        default:
            return false;
        }
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
                MWTrade::DefaultDurationDays,
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
            MWTrade::MinDailyTransferUnits,
            (std::min)(
                MWTrade::MaxDailyTransferUnits,
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
        case ETradeRouteEndReason::EraTransitioned:
            return 0;
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

        if (EndReason == ETradeRouteEndReason::EraTransitioned)
            return 0;

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
        case ETradeRouteEndReason::EraTransitioned:
            return 0;
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
        else if (Record.EndReason != ETradeRouteEndReason::EraTransitioned)
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

        if (InOutState.IdleDays %
                (std::max)(1, MWTradeDiplomacy::StandingIdleDecayIntervalDays) ==
                0 &&
            InOutState.Standing != 0)
        {
            InOutState.Standing += InOutState.Standing > 0 ? -1 : 1;
        }

        if (InOutState.IdleDays %
                (std::max)(1, MWTradeDiplomacy::RelationIdleDecayIntervalDays) ==
                0 &&
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

    std::wstring BuildEraTransitionTitle(EBuildingEra TargetEra)
    {
        switch (TargetEra)
        {
        case EBuildingEra::WorldWars:
            return L"독립 선언";
        case EBuildingEra::ColdWar:
            return L"전후 질서 편입";
        case EBuildingEra::Modern:
            return L"현대화 선언";
        case EBuildingEra::Colonial:
        default:
            return L"시대 전환";
        }
    }

    std::wstring BuildEraTransitionConfirmText(EBuildingEra TargetEra)
    {
        switch (TargetEra)
        {
        case EBuildingEra::WorldWars:
            return L"독립 승인";
        case EBuildingEra::ColdWar:
            return L"냉전 진입";
        case EBuildingEra::Modern:
            return L"현대화 승인";
        case EBuildingEra::Colonial:
        default:
            return L"전환 승인";
        }
    }

    std::wstring BuildEraTransitionAvailableSummary(EBuildingEra TargetEra)
    {
        switch (TargetEra)
        {
        case EBuildingEra::WorldWars:
            return
                L"독립을 선포할 준비가 끝났습니다.\n"
                L"승인하면 세계대전 시대 건물과 칙령이 즉시 열립니다.";
        case EBuildingEra::ColdWar:
            return
                L"전후 체제로 편입할 준비가 끝났습니다.\n"
                L"승인하면 냉전 시대 건물과 칙령이 즉시 열립니다.";
        case EBuildingEra::Modern:
            return
                L"현대화 전환 준비가 끝났습니다.\n"
                L"승인하면 현대 시대 건물과 칙령이 즉시 열립니다.";
        case EBuildingEra::Colonial:
        default:
            return L"다음 시대로 전환할 수 있습니다.";
        }
    }

    std::wstring BuildEraTransitionCompletionSummary(
        EBuildingEra PreviousEra,
        EBuildingEra CurrentEra)
    {
        return
            std::wstring(GetBuildingEraDisplayName(PreviousEra)) +
            L"에서 " +
            GetBuildingEraDisplayName(CurrentEra) +
            L"(으)로 전환되었습니다. 새 시대 건물과 칙령을 확인하세요.";
    }

    void PopulateEraTransitionAvailableState(
        FEraTransitionState& OutState,
        const FEraProgressState& EraProgress,
        int Year,
        int Month,
        int Day)
    {
        const bool KeepAvailableDate =
            OutState.Stage == EEraTransitionStage::Available &&
            OutState.CurrentEra == EraProgress.CurrentEra &&
            OutState.TargetEra == EraProgress.NextEra;

        const int AvailableYear = KeepAvailableDate ?
            OutState.AvailableSinceYear :
            Year;
        const int AvailableMonth = KeepAvailableDate ?
            OutState.AvailableSinceMonth :
            Month;
        const int AvailableDay = KeepAvailableDate ?
            OutState.AvailableSinceDay :
            Day;

        OutState = FEraTransitionState();
        OutState.Stage = EEraTransitionStage::Available;
        OutState.Choice = EEraTransitionChoice::Confirm;
        OutState.CurrentEra = EraProgress.CurrentEra;
        OutState.TargetEra = EraProgress.NextEra;
        OutState.CanStart = true;
        OutState.AvailableSinceYear = AvailableYear;
        OutState.AvailableSinceMonth = AvailableMonth;
        OutState.AvailableSinceDay = AvailableDay;
        OutState.Title = BuildEraTransitionTitle(EraProgress.NextEra);
        OutState.Summary =
            BuildEraTransitionAvailableSummary(EraProgress.NextEra);
        OutState.ConfirmText =
            BuildEraTransitionConfirmText(EraProgress.NextEra);
    }

    bool ShouldEnableElectionsForEra(EBuildingEra Era)
    {
        return IsBuildingEraUnlocked(Era, EBuildingEra::WorldWars);
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
                !IsImmediateProductionScopeResourceType(ResourceType) ||
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

    bool TryResolveKnowledgeProducerBaseDailyGeneration(
        const FBuildingCatalogEntry& Entry,
        int& OutBaseDailyGeneration)
    {
        OutBaseDailyGeneration = 0;

        if (Entry.Id == "build_6_2")
        {
            OutBaseDailyGeneration = 3;
            return true;
        }

        if (Entry.Id == "build_6_3")
        {
            OutBaseDailyGeneration = 4;
            return true;
        }

        if (Entry.Id == "build_6_4")
        {
            OutBaseDailyGeneration = 6;
            return true;
        }

        if (Entry.Id == "build_6_11")
        {
            OutBaseDailyGeneration = 10;
            return true;
        }

        return false;
    }

    int ResolveKnowledgeGenerationForBuilding(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable() ||
            !Building->HasPlacedArea())
        {
            return 0;
        }

        const FBuildingCatalogEntry* const Entry =
            FindBuildingCatalogEntry(Building->GetBuildingId());

        if (!Entry)
            return 0;

        int BaseDailyGeneration = 0;

        if (!TryResolveKnowledgeProducerBaseDailyGeneration(
                *Entry,
                BaseDailyGeneration))
        {
            return 0;
        }

        const int WorkerCapacity = Building->GetCapacity();
        const int CurrentWorkers = Building->GetCurrentWorkerOccupancy();

        if (WorkerCapacity <= 0 || CurrentWorkers <= 0)
            return 0;

        const float WorkerRatio = (std::max)(
            0.f,
            (std::min)(
                1.f,
                static_cast<float>(CurrentWorkers) /
                    static_cast<float>(WorkerCapacity)));
        const float EffectiveGeneration =
            static_cast<float>(BaseDailyGeneration) *
            WorkerRatio *
            (std::max)(0.f, Building->GetBudgetSatisfactionScale()) *
            (std::max)(0.f, Building->GetPowerSupplyRatio());
        return (std::max)(
            0,
            static_cast<int>(roundf(EffectiveGeneration)));
    }

    int GetBuildingDamageLevelRank(EBuildingDamageLevel Level)
    {
        switch (Level)
        {
        case EBuildingDamageLevel::Critical:
            return 2;
        case EBuildingDamageLevel::Damaged:
            return 1;
        case EBuildingDamageLevel::None:
        default:
            return 0;
        }
    }

    int ResolveBuildingRepairCost(
        const FBuildingCatalogEntry& Entry,
        EBuildingDamageLevel Level)
    {
        if (Level == EBuildingDamageLevel::None)
            return 0;

        int BaseCost = 1200;

        if (Entry.ConstructionCostState == EBuildingCostState::Known &&
            Entry.ConstructionCost > 0)
        {
            BaseCost = Entry.ConstructionCost;
        }
        else if (Entry.BlueprintCostState == EBuildingCostState::Known &&
            Entry.BlueprintCost > 0)
        {
            BaseCost = Entry.BlueprintCost;
        }

        switch (Level)
        {
        case EBuildingDamageLevel::Damaged:
            return (std::max)(
                250,
                static_cast<int>(roundf(static_cast<float>(BaseCost) * 0.18f)));
        case EBuildingDamageLevel::Critical:
            return (std::max)(
                600,
                static_cast<int>(roundf(static_cast<float>(BaseCost) * 0.42f)));
        case EBuildingDamageLevel::None:
        default:
            return 0;
        }
    }

}

int CMainWorld::GetDaysUntilNextElection() const
{
    return mPolitics->GetDaysUntilNextElection(
        mSimulation->Year,
        mSimulation->Month,
        mSimulation->Day);
}

double CMainWorld::GetElectionWarningScore() const
{
    return mPolitics->GetElectionWarningScore(
        mEconomy->TaxEventStatus,
        mSimulation->Year,
        mSimulation->Month,
        mSimulation->Day);
}

void CMainWorld::RefreshRuntimeBuildingState()
{
    mBuildings->RefreshRuntimeBuildingState();
}

bool CMainWorld::TryUnlockResearch(
    const std::wstring& Key,
    int Cost)
{
    return mKnowledgeState->TryUnlockResearch(Key, Cost);
}

bool CMainWorld::TrySelectConstitutionOption(
    EConstitutionOptionId Id)
{
    return mKnowledgeState->TrySelectConstitutionOption(Id);
}

bool CMainWorld::DamageBuilding(
    const std::string& BuildingName,
    EBuildingDamageLevel Level)
{
    return mBuildings->DamageBuilding(BuildingName, Level);
}

bool CMainWorld::TryRepairBuilding(
    const std::string& BuildingName,
    std::wstring& OutMessage)
{
    return mBuildings->TryRepairBuilding(BuildingName, OutMessage);
}

bool CMainWorld::TryApplyEdict(
    EGovernmentEdictType Type,
    std::wstring& OutMessage)
{
    return mEdictState->TryApplyEdict(Type, OutMessage);
}

bool CMainWorld::TryExecuteEraTransition(EEraTransitionChoice Choice)
{
    const bool Success = mEraState->TryExecuteEraTransition(Choice);

    if (Success)
        mScenario->NotifyEraTransitioned(mEraState->EraProgress.CurrentEra);

    return Success;
}

bool CMainWorld::TryExecutePeacePayment(std::wstring& OutMessage)
{
    if (!mScenario)
    {
        OutMessage = L"시나리오가 활성화되지 않았습니다.";
        return false;
    }
    return mScenario->TryExecutePeacePayment(OutMessage);
}

bool CMainWorld::AdjustTaxPolicy(
    ETaxPolicyType Type,
    int DeltaPercent,
    std::wstring& OutMessage)
{
    return mEconomy->AdjustTaxPolicy(Type, DeltaPercent, OutMessage);
}

bool CMainWorld::CycleExportBlockedResource(
    std::wstring& OutMessage)
{
    return mTrade->CycleExportBlockedResource(OutMessage);
}

bool CMainWorld::ExecuteTradeProposal(
    bool ImportRoute,
    EResourceType ResourceType,
    int ForeignPowerIndex,
    int PricePerThousandUnits,
    int Amount,
    std::wstring& OutMessage)
{
    return mTrade->ExecuteTradeProposal(
        ImportRoute,
        ResourceType,
        ForeignPowerIndex,
        PricePerThousandUnits,
        Amount,
        OutMessage);
}

bool CMainWorld::CancelTradeRoute(
    int RouteId,
    std::wstring& OutMessage)
{
    return mTrade->CancelTradeRoute(RouteId, OutMessage);
}

bool CMainWorld::RespondPoliticalDemand(
    EPoliticalDemandIssuerType IssuerType,
    int IssuerIndex,
    bool Accept,
    std::wstring& OutMessage)
{
    return mPolitics->RespondPoliticalDemand(
        IssuerType,
        IssuerIndex,
        Accept,
        OutMessage);
}

int CMainWorld::GetCustomsExportTradePriceModifierPercent() const
{
    return mTrade->GetCustomsExportTradePriceModifierPercent();
}

int CMainWorld::GetCustomsImportTradePriceModifierPercent() const
{
    return mTrade->GetCustomsImportTradePriceModifierPercent();
}

const FGovernmentEdictState* CMainWorld::GetGovernmentEdictState(
    EGovernmentEdictType Type) const
{
    for (size_t i = 0; i < mEdictState->GovernmentEdicts.size(); ++i)
    {
        if (mEdictState->GovernmentEdicts[i].Type == Type)
            return &mEdictState->GovernmentEdicts[i];
    }

    return nullptr;
}


