#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "MainWorldInfrastructureRuntime.h"
#include "MainWorldTradeRuntime.h"
#include "RuntimeConfigRegistry.h"
#include "WorldStatsSnapshot.h"
#include "../GameConstants.h"
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
    namespace MWTrade = GameConstants::MainWorld::Trade;
    namespace MWTradeDiplomacy = GameConstants::MainWorld::TradeDiplomacy;

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

}

void CMainWorld::InitializeElectionSchedule()
{
    mElectionService->InitializeSchedule(
        mSimulationYear,
        MainWorldConfig::GInitialElectionLeadYears,
        MainWorldConfig::GElectionMonth,
        MainWorldConfig::GElectionDay);
}

void CMainWorld::TickElectionPromises()
{
    CMainWorldElectionService::FPromiseContext Context;
    Context.World = mSelf.lock();
    Context.SimulationYear = mSimulationYear;
    Context.SimulationMonth = mSimulationMonth;
    Context.SimulationDay = mSimulationDay;
    Context.LastDailyExportIncome = mLastDailyExportIncome;
    mElectionService->TickPromises(Context);
}

void CMainWorld::ResolveScheduledElection()
{
    RefreshPoliticalSnapshot();
    mElectionService->ResolveScheduledElection(
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
    return mElectionService->GetDaysUntilNextElection(
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay);
}

double CMainWorld::GetElectionWarningScore() const
{
    return mElectionService->GetElectionWarningScore(
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
        static_cast<size_t>(MWTrade::MaxCompletedTradeRouteRecordCount))
    {
        mTradeDiplomacyState.CompletedTradeRoutes.resize(
            static_cast<size_t>(MWTrade::MaxCompletedTradeRouteRecordCount));
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
        mWorldCrisisService->GetStatus(),
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

CMainWorldPoliticalDemandService::FContext
    CMainWorld::BuildPoliticalDemandContext()
{
    return
    {
        mSelf.lock(),
        mPoliticalSnapshot,
        mGovernmentProfile,
        mLastDailyExportIncome,
        mNationalBudget,
        mLastDailyNetChange,
        mTradeDiplomacyState.ForeignPowerStandingStates,
        mTradeDiplomacyState.ForeignPowerStates,
        mTradeDiplomacyState.ActiveTradeRoutes
    };
}

void CMainWorld::ApplyPoliticalDemandRefreshRequests(
    const CMainWorldPoliticalDemandService::FRefreshRequests& RefreshRequests)
{
    if (RefreshRequests.RefreshPoliticalSnapshot)
        RefreshPoliticalSnapshot();

    if (RefreshRequests.RefreshForeignTradeDiplomacy)
        RefreshForeignTradeDiplomacy(false);

    if (RefreshRequests.RefreshWorldMarketPrices)
        RefreshWorldMarketPrices();
}

bool CMainWorld::RespondPoliticalDemand(
    EPoliticalDemandIssuerType IssuerType,
    int IssuerIndex,
    bool Accept,
    std::wstring& OutMessage)
{
    CMainWorldPoliticalDemandService::FRefreshRequests RefreshRequests;

    if (!mPoliticalDemandService->RespondPoliticalDemand(
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
    const CMainWorldWorldCrisisService::FDailyContext Context =
    {
        mSelf.lock(),
        mNationalBudget,
        mLastDailyNetChange,
        mLastDailyTaxIncome,
        mLastDailyConsumptionTaxIncome,
        mLastDailyIncomeTaxIncome,
        mLastDailyPropertyTaxIncome
    };
    mWorldCrisisService->ApplyDailyEffects(Context);
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
    const CMainWorldWorldCrisisService::FTickContext Context =
    {
        mSelf.lock(),
        mPoliticalSnapshot,
        mGovernmentEdicts,
        mGovernmentProfile.TaxPolicy,
        mTaxEventStatus,
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay,
        mNationalBudget,
        mLastDailyNetChange,
        mLastDailyTaxCollectionEfficiency
    };
    mWorldCrisisService->Tick(Context);
}

void CMainWorld::TickPoliticalDemands()
{
    const CMainWorldPoliticalDemandService::FRefreshRequests RefreshRequests =
        mPoliticalDemandService->Tick(BuildPoliticalDemandContext());

    ApplyPoliticalDemandRefreshRequests(RefreshRequests);
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
