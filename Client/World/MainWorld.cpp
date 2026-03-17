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

    std::wstring BuildAutoImportSelectionText(
        const TradePolicy::FImportTradePolicy& Policy)
    {
        return TradePolicy::BuildImportPolicySelectionDisplayText(Policy);
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

void CMainWorld::InitializeElectionSchedule()
{
    if (!ShouldEnableElectionsForEra(mPolicy.EraProgress.CurrentEra))
        return;

    const FElectionStatus& ElectionStatus = mServices.ElectionService->GetElectionStatus();

    if (ElectionStatus.GameLost ||
        ElectionStatus.NextElectionYear > 0)
    {
        return;
    }

    mServices.ElectionService->InitializeSchedule(
        mSimulation.Year,
        MainWorldConfig::GInitialElectionLeadYears,
        MainWorldConfig::GElectionMonth,
        MainWorldConfig::GElectionDay);
}

void CMainWorld::TickElectionPromises()
{
    CMainWorldElectionService::FPromiseContext Context;
    Context.World = mSelf.lock();
    Context.SimulationYear = mSimulation.Year;
    Context.SimulationMonth = mSimulation.Month;
    Context.SimulationDay = mSimulation.Day;
    Context.LastDailyExportIncome = mBudget.LastDailyExportIncome;
    mServices.ElectionService->TickPromises(Context);
}

void CMainWorld::ResolveScheduledElection()
{
    const FElectionStatus PreviousStatus =
        mServices.ElectionService->GetElectionStatus();

    RefreshPoliticalSnapshot();
    mServices.ElectionService->ResolveScheduledElection(
        mPolicy.PoliticalSnapshot,
        mSimulation.Year,
        mSimulation.Month,
        mSimulation.Day,
        MainWorldConfig::GElectionIntervalYears,
        MainWorldConfig::GElectionMonth,
        MainWorldConfig::GElectionDay);

    const FElectionStatus& ElectionStatus =
        mServices.ElectionService->GetElectionStatus();
    const bool RecordedNewElection =
        ElectionStatus.HasRecordedElection &&
        (!PreviousStatus.HasRecordedElection ||
            ElectionStatus.LastElectionYear != PreviousStatus.LastElectionYear ||
            ElectionStatus.LastElectionMonth != PreviousStatus.LastElectionMonth ||
            ElectionStatus.LastElectionDay != PreviousStatus.LastElectionDay);

    if (RecordedNewElection)
        ShowResultWidget(ElectionStatus.IncumbentWonLastElection);
}

int CMainWorld::GetDaysUntilNextElection() const
{
    return mServices.ElectionService->GetDaysUntilNextElection(
        mSimulation.Year,
        mSimulation.Month,
        mSimulation.Day);
}

double CMainWorld::GetElectionWarningScore() const
{
    return mServices.ElectionService->GetElectionWarningScore(
        mPolicy.PoliticalSnapshot,
        mPolicy.TaxEventStatus,
        mSimulation.Year,
        mSimulation.Month,
        mSimulation.Day);
}

void CMainWorld::ApplyDailyEconomySettlement()
{
    const int DaysInMonth = GetDaysInMonth(mSimulation.Year, mSimulation.Month);
    const auto Result = EconomySystem::ApplyDailyWorldSettlement(
        this,
        DaysInMonth,
        mPolicy.GovernmentProfile,
        mPolicy.TaxEventStatus,
        mPolicy.GovernmentEdicts,
        mPolicy.EdictModifiers);
    mBudget.LastDailyWageCost     = Result.BaseResult.WageCost;
    mBudget.LastDailyUpkeepCost   = Result.BaseResult.UpkeepCost;
    mBudget.LastDailyExportIncome = Result.BaseResult.ExportIncome;
    mBudget.LastDailyTaxIncome    = Result.AdjustedTaxIncome;
    mBudget.LastDailyConsumptionTaxIncome = Result.AdjustedConsumptionTaxIncome;
    mBudget.LastDailyIncomeTaxIncome = Result.AdjustedIncomeTaxIncome;
    mBudget.LastDailyPropertyTaxIncome = Result.AdjustedPropertyTaxIncome;
    mBudget.LastDailyEdictCost    = Result.DailyEdictCost;
    mBudget.LastDailyImportExpense = Result.BaseResult.ImportExpense;
    mBudget.LastDailyTaxCollectionEfficiency =
        Result.BaseResult.TaxCollectionEfficiency;
    mBudget.LastDailyNetChange = Result.NetBudgetChange;
    mBudget.NationalBudget += mBudget.LastDailyNetChange;
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
        static_cast<size_t>(MWTrade::MaxCompletedTradeRouteRecordCount))
    {
        mTradeDiplomacyState.CompletedTradeRoutes.resize(
            static_cast<size_t>(MWTrade::MaxCompletedTradeRouteRecordCount));
    }

    ++mTradeDiplomacyState.TradeRouteCompletionNotificationVersion;
}

void CMainWorld::CancelTradeRoutesForInactivePowers(EBuildingEra Era)
{
    (void)Era;

    if (mTradeDiplomacyState.ActiveTradeRoutes.empty())
        return;

    for (const FTradeRouteRuntimeState& Route :
        mTradeDiplomacyState.ActiveTradeRoutes)
    {
        RecordFinishedTradeRoute(
            Route,
            ETradeRouteEndReason::EraTransitioned);
    }

    mTradeDiplomacyState.ActiveTradeRoutes.clear();
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
                    mBudget.NationalBudget > 0 ?
                        (mBudget.NationalBudget * 1000LL) /
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
                mBudget.NationalBudget -= ImportCost;
                mBudget.LastDailyImportExpense += ImportCost;
                mBudget.LastDailyNetChange -= ImportCost;
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
                mBudget.NationalBudget += ExportIncome;
                mBudget.LastDailyExportIncome += ExportIncome;
                mBudget.LastDailyNetChange += ExportIncome;
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
            mPolicy.GovernmentProfile,
            mPolicy.TaxEventStatus,
            mPolicy.GovernmentEdicts,
            mPolicy.EraProgress.CurrentEra,
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
        mPolicy.GovernmentProfile,
        mPolicy.GovernmentEdicts,
        mPolicy.TaxEventStatus,
        mServices.WorldCrisisService->GetStatus(),
        mTradeDiplomacyState.ForeignPowerStates,
        mSimulation.Year,
        mSimulation.Month,
        mSimulation.Day);
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
    RefreshBuildingRepairCosts();
    RefreshKnowledgeGeneration();
    ReassignCitizenNeeds();
    RefreshEraProgress();
    RefreshPoliticalSnapshot();
    RefreshForeignTradeDiplomacy(false);
    RefreshWorldMarketPrices();
}

bool CMainWorld::TryUnlockResearch(
    const std::wstring& Key,
    int Cost)
{
    if (Key.empty())
        return false;

    if (::IsResearchUnlocked(mPolicy.KnowledgeState, Key))
        return true;

    if (!CanUnlockResearch(mPolicy.KnowledgeState, Key, Cost))
        return false;

    return ::TryUnlockResearch(mPolicy.KnowledgeState, Key, Cost);
}

bool CMainWorld::TrySelectConstitutionOption(
    EConstitutionOptionId Id)
{
    if (!ConstitutionSystem::TrySelectConstitutionOption(
            mPolicy.ConstitutionState,
            Id))
    {
        return false;
    }

    RefreshPoliticalSnapshot();
    return true;
}

void CMainWorld::RefreshKnowledgeGeneration()
{
    mPolicy.KnowledgeState.DailyGeneration = 0;
    auto World = mSelf.lock();

    if (!World)
        return;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return;

    int TotalDailyGeneration = 0;

    for (size_t Index = 0; Index < BuildingList.size(); ++Index)
    {
        const auto Building = BuildingList[Index].lock();
        TotalDailyGeneration += ResolveKnowledgeGenerationForBuilding(Building);
    }

    mPolicy.KnowledgeState.DailyGeneration = (std::max)(0, TotalDailyGeneration);
}

void CMainWorld::ApplyDailyKnowledgeGain()
{
    if (mPolicy.KnowledgeState.DailyGeneration <= 0)
        return;

    mPolicy.KnowledgeState.Points += (std::max)(0, mPolicy.KnowledgeState.DailyGeneration);
}

void CMainWorld::RefreshBuildingRepairCosts()
{
    auto World = mSelf.lock();

    if (!World)
        return;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return;

    for (size_t Index = 0; Index < BuildingList.size(); ++Index)
    {
        auto Building = BuildingList[Index].lock();

        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable())
        {
            continue;
        }

        const FBuildingCatalogEntry* const Entry =
            FindBuildingCatalogEntry(Building->GetBuildingId());

        Building->SetRepairCost(
            Entry ?
                ResolveBuildingRepairCost(*Entry, Building->GetDamageLevel()) :
                0);
    }
}

bool CMainWorld::DamageBuilding(
    const std::string& BuildingName,
    EBuildingDamageLevel Level)
{
    if (BuildingName.empty() || Level == EBuildingDamageLevel::None)
        return false;

    auto World = mSelf.lock();

    if (!World)
        return false;

    auto Building =
        World->FindObject<CPlacementAreaObject>(BuildingName).lock();

    if (!Building ||
        !Building->GetAlive() ||
        !Building->GetEnable() ||
        !Building->HasPlacedArea() ||
        Building->IsRoad())
    {
        return false;
    }

    if (GetBuildingDamageLevelRank(Building->GetDamageLevel()) >=
        GetBuildingDamageLevelRank(Level))
    {
        return false;
    }

    Building->SetDamageLevel(Level);
    const FBuildingCatalogEntry* const Entry =
        FindBuildingCatalogEntry(Building->GetBuildingId());
    Building->SetRepairCost(
        Entry ?
            ResolveBuildingRepairCost(*Entry, Level) :
            0);
    return true;
}

bool CMainWorld::TryRepairBuilding(
    const std::string& BuildingName,
    std::wstring& OutMessage)
{
    OutMessage.clear();
    auto World = mSelf.lock();

    if (!World || BuildingName.empty())
    {
        OutMessage = L"건물을 찾을 수 없습니다.";
        return false;
    }

    auto Building =
        World->FindObject<CPlacementAreaObject>(BuildingName).lock();

    if (!Building ||
        !Building->GetAlive() ||
        !Building->GetEnable() ||
        !Building->HasPlacedArea())
    {
        OutMessage = L"건물을 찾을 수 없습니다.";
        return false;
    }

    if (!Building->HasBuildingDamage())
    {
        OutMessage = L"수리가 필요한 피해가 없습니다.";
        return false;
    }

    const FBuildingCatalogEntry* const Entry =
        FindBuildingCatalogEntry(Building->GetBuildingId());
    const int RepairCost =
        Building->GetRepairCost() > 0 ?
            Building->GetRepairCost() :
            (Entry ?
                ResolveBuildingRepairCost(*Entry, Building->GetDamageLevel()) :
                0);

    if (RepairCost > mBudget.NationalBudget)
    {
        OutMessage =
            StringUtils::Utf8ToWide(Building->GetBuildingDisplayName()) +
            L": 수리비 " +
            FormatTradeCurrency(RepairCost) +
            L" 필요";
        return false;
    }

    mBudget.NationalBudget -= RepairCost;
    mBudget.LastDailyNetChange -= RepairCost;
    Building->SetDamageLevel(EBuildingDamageLevel::None);
    Building->SetRepairCost(0);
    RefreshRuntimeBuildingState();
    OutMessage =
        StringUtils::Utf8ToWide(Building->GetBuildingDisplayName()) +
        L": 긴급 수리 완료 (-" +
        FormatTradeCurrency(RepairCost) +
        L")";
    return true;
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
            mPolicy.EraProgress.CurrentEra,
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

    for (size_t i = 0; i < mPolicy.GovernmentEdicts.size(); ++i)
    {
        if (mPolicy.GovernmentEdicts[i].Type == Type)
        {
            TargetState = &mPolicy.GovernmentEdicts[i];
            break;
        }
    }

    if (!TargetState)
    {
        OutMessage = L"칙령 상태를 찾을 수 없습니다.";
        return false;
    }

    const int ActiveCitizenCount =
        (std::max)(0, mPolicy.PoliticalSnapshot.ActiveCitizenCount);
    const ETaxPolicyEventType RequiredTaxEvent =
        EconomySystem::GetRequiredTaxPolicyEventForEdict(Type);

    if (RequiredTaxEvent != ETaxPolicyEventType::None)
    {
        if (!mPolicy.TaxEventStatus.Active || mPolicy.TaxEventStatus.Type != RequiredTaxEvent)
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
            mPolicy.GovernmentProfile,
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

    if (ActivationCost > mBudget.NationalBudget)
    {
        OutMessage = L"예산이 부족합니다.";
        return false;
    }

    mBudget.NationalBudget -= ActivationCost;
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
            mPolicy.GovernmentProfile.TaxPolicy,
            ETaxPolicyType::Income,
            -4);
        EconomySystem::ResolveTaxPolicyEvent(mPolicy.TaxEventStatus, true);
        ResponseMessage =
            L"소득세 " +
            std::to_wstring((std::max)(0, -RateDelta)) +
            L"%p 인하";
        break;
    }
    case EGovernmentEdictType::PropertyTaxRelief:
    {
        const int RateDelta = EconomySystem::ApplyTaxPolicyRateDelta(
            mPolicy.GovernmentProfile.TaxPolicy,
            ETaxPolicyType::Property,
            -10);
        EconomySystem::ResolveTaxPolicyEvent(mPolicy.TaxEventStatus, true);
        ResponseMessage =
            L"재산세 " +
            std::to_wstring((std::max)(0, -RateDelta)) +
            L"%p 인하";
        break;
    }
    case EGovernmentEdictType::EmergencyAusterity:
    {
        const long long EmergencyFunds = 12000;
        mBudget.NationalBudget += EmergencyFunds;
        mBudget.LastDailyNetChange += EmergencyFunds;
        EconomySystem::ResolveTaxPolicyEvent(mPolicy.TaxEventStatus, true);
        ResponseMessage = L"긴급 자금 $12,000 투입";
        break;
    }
    default:
        break;
    }

    PoliticsSystem::SyncGovernmentActionFromEdict(
        mPolicy.GovernmentProfile,
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

bool CMainWorld::TryExecuteEraTransition(EEraTransitionChoice Choice)
{
    if (Choice != EEraTransitionChoice::Confirm ||
        mPolicy.EraTransition.Stage != EEraTransitionStage::Available ||
        !mPolicy.EraTransition.CanStart)
    {
        return false;
    }

    const EBuildingEra PreviousEra = mPolicy.EraProgress.CurrentEra;
    const EBuildingEra TargetEra = mPolicy.EraTransition.TargetEra;

    mPolicy.EraProgress.CurrentEra = TargetEra;
    CancelTradeRoutesForInactivePowers(TargetEra);
    mTradeDiplomacyState.ForeignPowerStandingStates = {};
    RefreshEraProgress();
    InitializeElectionSchedule();

    mPolicy.EraTransition = FEraTransitionState();
    mPolicy.EraTransition.Stage = EEraTransitionStage::Cooldown;
    mPolicy.EraTransition.Choice = Choice;
    mPolicy.EraTransition.CurrentEra = mPolicy.EraProgress.CurrentEra;
    mPolicy.EraTransition.TargetEra = TargetEra;
    mPolicy.EraTransition.NotificationDays = GEraTransitionNotificationDays;
    mPolicy.EraTransition.Title = BuildEraTransitionTitle(TargetEra);
    mPolicy.EraTransition.Summary =
        BuildEraTransitionCompletionSummary(
            PreviousEra,
            mPolicy.EraProgress.CurrentEra);
    mPolicy.EraTransition.ConfirmText =
        BuildEraTransitionConfirmText(TargetEra);

    ConstitutionSystem::OnEraTransitioned(
        mPolicy.ConstitutionState,
        TargetEra);
    RefreshPoliticalSnapshot();
    RefreshForeignTradeDiplomacy(false);
    RefreshWorldMarketPrices();
    return true;
}

bool CMainWorld::AdjustTaxPolicy(
    ETaxPolicyType Type,
    int DeltaPercent,
    std::wstring& OutMessage)
{
    const bool Adjusted = EconomySystem::AdjustTaxPolicy(
        mPolicy.GovernmentProfile.TaxPolicy,
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
        mPolicy.GovernmentProfile.ImportTradePolicy);
    OutMessage =
        L"자동 수입 대상: " +
        BuildAutoImportSelectionText(
            mPolicy.GovernmentProfile.ImportTradePolicy);
    return true;
}

bool CMainWorld::CycleImportPerResourceCap(
    std::wstring& OutMessage)
{
    TradePolicy::AdvanceImportResourceCapSelection(
        mPolicy.GovernmentProfile.ImportTradePolicy);
    OutMessage =
        L"자원별 수입 한도: " +
        BuildImportCapSelectionText(
            mPolicy.GovernmentProfile.ImportTradePolicy);
    return true;
}

bool CMainWorld::CycleImportBudgetPolicy(
    std::wstring& OutMessage)
{
    TradePolicy::AdvanceImportBudgetSelection(
        mPolicy.GovernmentProfile.ImportTradePolicy);
    OutMessage =
        L"일일 수입 예산: " +
        BuildImportBudgetSelectionText(
            mPolicy.GovernmentProfile.ImportTradePolicy);
    return true;
}

bool CMainWorld::CycleDomesticReservePolicy(
    std::wstring& OutMessage)
{
    TradePolicy::AdvanceDomesticReservePolicySelection(
        mPolicy.GovernmentProfile.ExportTradePolicy);
    OutMessage =
        L"내수 비축 기준: " +
        BuildDomesticReserveSelectionText(
            mPolicy.GovernmentProfile.ExportTradePolicy);
    return true;
}

bool CMainWorld::CycleExportBlockedResource(
    std::wstring& OutMessage)
{
    TradePolicy::AdvanceExportBlockedResourceSelection(
        mPolicy.GovernmentProfile.ExportTradePolicy);
    OutMessage =
        L"수출 금지 자원: " +
        BuildExportBlockedSelectionText(
            mPolicy.GovernmentProfile.ExportTradePolicy);
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
        MWTrade::MaxActiveTradeRouteCount)
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
        MWTrade::MinAmountUnits,
        (std::min)(MWTrade::MaxAmountUnits, Amount));

    if (SafeAmount < MWTrade::MinAmountUnits)
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
        std::wstring(GetTradeForeignPowerName(
            ForeignPowerIndex,
            mPolicy.EraProgress.CurrentEra)) +
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

CMainWorldPoliticalDemandService::FContext
    CMainWorld::BuildPoliticalDemandContext()
{
    return
    {
        mSelf.lock(),
        mPolicy.EraProgress.CurrentEra,
        mPolicy.PoliticalSnapshot,
        mPolicy.GovernmentProfile,
        mBudget.LastDailyExportIncome,
        mBudget.NationalBudget,
        mBudget.LastDailyNetChange,
        mTradeDiplomacyState.ForeignPowerStandingStates,
        mTradeDiplomacyState.ForeignPowerStates,
        mTradeDiplomacyState.ActiveTradeRoutes
    };
}

void CMainWorld::ApplyPoliticalDemandRefreshRequests(
    const CMainWorldPoliticalDemandService::FRefreshRequests& RefreshRequests)
{
    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        if (!RefreshRequests.TriggerFactionRevolts[static_cast<size_t>(Index)])
            continue;

        TriggerFactionRevoltConsequences(static_cast<EPoliticalFaction>(Index));
    }

    if (RefreshRequests.RefreshPoliticalSnapshot)
        RefreshPoliticalSnapshot();

    if (RefreshRequests.RefreshForeignTradeDiplomacy)
        RefreshForeignTradeDiplomacy(false);

    if (RefreshRequests.RefreshWorldMarketPrices)
        RefreshWorldMarketPrices();
}

void CMainWorld::TriggerFactionRevoltConsequences(EPoliticalFaction Faction)
{
    auto World = mSelf.lock();

    if (!World)
        return;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return;

    std::vector<std::shared_ptr<CPlacementAreaObject>> PriorityTargets;
    std::vector<std::shared_ptr<CPlacementAreaObject>> FallbackTargets;

    for (size_t Index = 0; Index < BuildingList.size(); ++Index)
    {
        auto Building = BuildingList[Index].lock();

        if (!IsEligibleRevoltDamageTarget(Building))
            continue;

        if (IsFactionRevoltPriorityBuilding(Building, Faction))
            PriorityTargets.push_back(Building);
        else
            FallbackTargets.push_back(Building);
    }

    const int TotalCandidateCount = static_cast<int>(
        PriorityTargets.size() + FallbackTargets.size());
    const int TargetHitCount =
        TotalCandidateCount <= 0 ? 0 : (std::min)(2, 1 + rand() % 2);
    int AppliedHits = 0;

    auto ApplySingleHit =
        [&](std::vector<std::shared_ptr<CPlacementAreaObject>>& Targets)
        {
            while (AppliedHits < TargetHitCount && !Targets.empty())
            {
                const size_t PickIndex = static_cast<size_t>(
                    rand() % static_cast<int>(Targets.size()));
                const auto Target = Targets[PickIndex];
                Targets.erase(Targets.begin() + static_cast<int>(PickIndex));

                if (!Target)
                    continue;

                const EBuildingDamageLevel NextLevel =
                    Target->GetDamageLevel() == EBuildingDamageLevel::Damaged ?
                        EBuildingDamageLevel::Critical :
                        EBuildingDamageLevel::Damaged;

                if (DamageBuilding(Target->GetName(), NextLevel))
                    ++AppliedHits;
            }
        };

    ApplySingleHit(PriorityTargets);
    ApplySingleHit(FallbackTargets);

    if (mServices.WorldCrisisService)
    {
        const CMainWorldWorldCrisisService::FTickContext CrisisContext =
        {
            mSelf.lock(),
            mPolicy.PoliticalSnapshot,
            mPolicy.GovernmentEdicts,
            mPolicy.GovernmentProfile.TaxPolicy,
            mPolicy.TaxEventStatus,
            mSimulation.Year,
            mSimulation.Month,
            mSimulation.Day,
            mBudget.NationalBudget,
            mBudget.LastDailyNetChange,
            mBudget.LastDailyTaxCollectionEfficiency
        };
        mServices.WorldCrisisService->TriggerForcedRaid(CrisisContext);
    }
}

bool CMainWorld::RespondPoliticalDemand(
    EPoliticalDemandIssuerType IssuerType,
    int IssuerIndex,
    bool Accept,
    std::wstring& OutMessage)
{
    CMainWorldPoliticalDemandService::FRefreshRequests RefreshRequests;

    if (!mServices.PoliticalDemandService->RespondPoliticalDemand(
            IssuerType,
            IssuerIndex,
            Accept,
            OutMessage,
            BuildPoliticalDemandContext(),
            RefreshRequests))
    {
        return false;
    }

    ApplyPoliticalDemandRefreshRequests(RefreshRequests);
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
    for (size_t i = 0; i < mPolicy.GovernmentEdicts.size(); ++i)
    {
        if (mPolicy.GovernmentEdicts[i].Type == Type)
            return &mPolicy.GovernmentEdicts[i];
    }

    return nullptr;
}

void CMainWorld::TickGovernmentEdicts()
{
    bool ModifiersChanged = false;

    for (size_t i = 0; i < mPolicy.GovernmentEdicts.size(); ++i)
    {
        FGovernmentEdictState& State = mPolicy.GovernmentEdicts[i];
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
                    mPolicy.GovernmentProfile,
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
    mPolicy.EdictModifiers = EdictSystem::CalculateEdictModifiers(
        mPolicy.GovernmentEdicts,
        mPolicy.PoliticalSnapshot.ActiveCitizenCount);
    mPolicy.GovernmentProfile.EdictFactionApprovalModifiers =
        mPolicy.EdictModifiers.FactionApprovalModifiers;
    RefreshWorldMarketPrices();
}

void CMainWorld::ApplyDailyEdictCitizenEffects()
{
    PoliticsSystem::ApplyDailyEdictCitizenEffects(
        this,
        mPolicy.EdictModifiers);
}

void CMainWorld::ApplyDailyTaxPolicyEventEffects()
{
    EconomySystem::ApplyDailyTaxPolicyEventEffects(
        this,
        mPolicy.TaxEventStatus);
}

void CMainWorld::ApplyDailyWorldCrisisEffects()
{
    const CMainWorldWorldCrisisService::FDailyContext Context =
    {
        mSelf.lock(),
        mBudget.NationalBudget,
        mBudget.LastDailyNetChange,
        mBudget.LastDailyTaxIncome,
        mBudget.LastDailyConsumptionTaxIncome,
        mBudget.LastDailyIncomeTaxIncome,
        mBudget.LastDailyPropertyTaxIncome
    };
    mServices.WorldCrisisService->ApplyDailyEffects(Context);
}

void CMainWorld::TickTaxPolicyEvents()
{
    EconomySystem::TickTaxPolicyEvents(
        mPolicy.PoliticalSnapshot,
        mPolicy.GovernmentProfile,
        mSimulation.Year,
        mSimulation.Month,
        mSimulation.Day,
        mBudget.NationalBudget,
        mBudget.LastDailyNetChange,
        mPolicy.WorkerTaxPressureDays,
        mPolicy.PropertyTaxPressureDays,
        mPolicy.BudgetCrisisPressureDays,
        mPolicy.TaxEventStatus);
}

void CMainWorld::TickWorldCrises()
{
    const CMainWorldWorldCrisisService::FTickContext Context =
    {
        mSelf.lock(),
        mPolicy.PoliticalSnapshot,
        mPolicy.GovernmentEdicts,
        mPolicy.GovernmentProfile.TaxPolicy,
        mPolicy.TaxEventStatus,
        mSimulation.Year,
        mSimulation.Month,
        mSimulation.Day,
        mBudget.NationalBudget,
        mBudget.LastDailyNetChange,
        mBudget.LastDailyTaxCollectionEfficiency
    };
    mServices.WorldCrisisService->Tick(Context);
}

void CMainWorld::TickPoliticalDemands()
{
    const CMainWorldPoliticalDemandService::FRefreshRequests RefreshRequests =
        mServices.PoliticalDemandService->Tick(BuildPoliticalDemandContext());

    ApplyPoliticalDemandRefreshRequests(RefreshRequests);
}

void CMainWorld::TickEraTransitionState()
{
    if (mPolicy.EraTransition.Stage != EEraTransitionStage::Cooldown ||
        mPolicy.EraTransition.NotificationDays <= 0)
    {
        return;
    }

    --mPolicy.EraTransition.NotificationDays;

    if (mPolicy.EraTransition.NotificationDays <= 0)
        mPolicy.EraTransition = FEraTransitionState();
}

void CMainWorld::RefreshEraProgress()
{
    const std::shared_ptr<CWorld> World = mSelf.lock();

    if (!World)
        return;

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(World);

    const EBuildingEra CurrentEra = mPolicy.EraProgress.CurrentEra;

    mPolicy.EraProgress = FEraProgressState();
    mPolicy.EraProgress.CurrentEra = CurrentEra;
    mPolicy.EraProgress.Population = Snapshot.ActiveCitizenCount;
    mPolicy.EraProgress.TotalBuildings = Snapshot.TotalBuildingCount;
    mPolicy.EraProgress.FoodProviders = Snapshot.FoodProviderCount;
    mPolicy.EraProgress.IndustryBuildings =
        ResolveIndustryBuildingCount(Snapshot);
    mPolicy.EraProgress.PublicServiceBuildings =
        ResolvePublicServiceBuildingCount(Snapshot);
    mPolicy.EraProgress.EntertainmentBuildings =
        ResolveEntertainmentBuildingCount(Snapshot);
    mPolicy.EraProgress.PowerMW = Snapshot.TotalProducedPowerMW;
    mPolicy.EraProgress.HasNextEra = HasNextBuildingEra(CurrentEra);

    if (mPolicy.EraProgress.HasNextEra)
    {
        mPolicy.EraProgress.NextEra = GetNextBuildingEra(CurrentEra);
        mPolicy.EraProgress.NextRequirement =
            ResolveEraUnlockRequirement(mPolicy.EraProgress.NextEra);
        mPolicy.EraProgress.NextEraReady =
            MeetsEraUnlockRequirement(
                Snapshot,
                mPolicy.EraProgress.NextRequirement);
    }
    else
    {
        mPolicy.EraProgress.NextEra = EBuildingEra::Modern;
        mPolicy.EraProgress.NextRequirement = FEraUnlockRequirement();
        mPolicy.EraProgress.NextEraReady = false;
    }

    RefreshEraTransitionState();
}

void CMainWorld::RefreshEraTransitionState()
{
    if (mPolicy.EraTransition.Stage == EEraTransitionStage::Cooldown &&
        mPolicy.EraTransition.NotificationDays > 0)
    {
        return;
    }

    if (!mPolicy.EraProgress.HasNextEra || !mPolicy.EraProgress.NextEraReady)
    {
        if (mPolicy.EraTransition.Stage == EEraTransitionStage::Available)
            mPolicy.EraTransition = FEraTransitionState();

        return;
    }

    PopulateEraTransitionAvailableState(
        mPolicy.EraTransition,
        mPolicy.EraProgress,
        mSimulation.Year,
        mSimulation.Month,
        mSimulation.Day);
}

void CMainWorld::RefreshPoliticalSnapshot()
{
    mPolicy.PoliticalSnapshot = PoliticsSystem::EvaluateWorld(
        this,
        mPolicy.GovernmentProfile,
        &mPolicy.TaxEventStatus,
        &mPolicy.ConstitutionState.ActiveEffects);

    constexpr int GWarningPressureThreshold = 10;
    const auto& FactionPressureDays =
        mServices.PoliticalDemandService->GetFactionPressureDays();
    const auto& FactionDemandStates =
        mServices.PoliticalDemandService->GetFactionDemandStates();

    for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
    {
        FPoliticalFactionSnapshot& FactionSnapshot =
            mPolicy.PoliticalSnapshot.Factions[static_cast<size_t>(Index)];
        const int PressureDays =
            (std::max)(0, FactionPressureDays[static_cast<size_t>(Index)]);
        const FPoliticalDemandState& Demand =
            FactionDemandStates[static_cast<size_t>(Index)];

        FactionSnapshot.PressureDays = PressureDays;

        if (Demand.Active)
        {
            FactionSnapshot.DemandStage = Demand.Stage;
        }
        else if (PressureDays >= GWarningPressureThreshold)
        {
            FactionSnapshot.DemandStage = EPoliticalDemandStage::Warning;
        }
        else
        {
            FactionSnapshot.DemandStage = EPoliticalDemandStage::Demand;
        }
    }

    mResultRuntime.PeakSupportPercent =
        (std::max)(
            mResultRuntime.PeakSupportPercent,
            mPolicy.PoliticalSnapshot.AverageSupportScore);
}

void CMainWorld::ShowResultWidget(bool Victory)
{
    if (mResultRuntime.ResultShown)
        return;

    auto UiManager = mUIManager;
    const std::shared_ptr<CWorld> World = mSelf.lock();

    if (!UiManager || !World)
        return;

    auto ResultWidget =
        UiManager->FindWidget<CResultWidget>(GResultWidgetName).lock();

    if (!ResultWidget)
        return;

    if (auto EventWidget = UiManager->FindWidget<CEventWidget>(GEventWidgetName).lock())
        EventWidget->GetMutableState().Visible = false;

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(World);
    const FElectionStatus& ElectionStatus =
        mServices.ElectionService->GetElectionStatus();
    const int BuildingsBuilt =
        (std::max)(0, Snapshot.TotalBuildingCount - mResultRuntime.InitialBuildingCount);
    const std::wstring TenureText =
        BuildTenureText(
            mResultRuntime.TermStartYear,
            mResultRuntime.TermStartMonth,
            mResultRuntime.TermStartDay,
            mSimulation.Year,
            mSimulation.Month,
            mSimulation.Day);
    FResultWidgetState& State = ResultWidget->GetMutableState();
    State.Visible = true;
    State.Victory = Victory;

    if (Victory)
    {
        State.Title = L"재선 성공!";
        State.Summary = L"트로피코 시민들이 다시 한번 당신을 선택했습니다.";
        State.DetailPrimary =
            L"득표율: " + FormatPercentText(ElectionStatus.LastVoteShare, 1);
        State.DetailSecondary = L"임기 연장: 4년";
        State.DetailTertiary =
            L"임기 중 건설: " +
            std::to_wstring(BuildingsBuilt) +
            L"개 건물";
        State.DetailQuaternary =
            L"최고 지지율: " +
            FormatPercentText(mResultRuntime.PeakSupportPercent, 0);
    }
    else
    {
        State.Title = L"쿠데타 발생";
        State.Summary = L"지지율 붕괴로 군부가 관저를 점령했습니다.";
        State.DetailPrimary =
            L"최종 지지율: " +
            FormatPercentText(mPolicy.PoliticalSnapshot.AverageSupportScore, 0);
        State.DetailSecondary = TenureText;
        State.DetailTertiary =
            L"임기 중 건설: " +
            std::to_wstring(BuildingsBuilt) +
            L"개 건물";
        State.DetailQuaternary =
            L"최고 지지율: " +
            FormatPercentText(mResultRuntime.PeakSupportPercent, 0);
    }

    mSimulation.Paused = true;
    mResultRuntime.ResultShown = true;
}

