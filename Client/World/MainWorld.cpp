#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "WorldStatsSnapshot.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../Politics/EdictSystem.h"
#include "../Politics/PoliticsSystem.h"
#include "../Economy/EconomySystem.h"
#include "../Economy/ResourceTradePricing.h"
#include "../Economy/TradeDiplomacyRuntime.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cwchar>
#include <string>
#include <vector>

namespace
{
    constexpr float GBuildingPollutionRadiusTiles = 18.f;
    constexpr int GPowerPriorityBandCount = 3;
    constexpr int GMaxActiveTradeRouteCount = 10;
    constexpr int GMaxCompletedTradeRouteRecordCount = 12;
    constexpr int GTradeRouteMinAmountUnits = 1000;
    constexpr int GTradeRouteMaxAmountUnits = 24000;
    constexpr int GTradeRouteMinDailyTransferUnits = 150;
    constexpr int GTradeRouteMaxDailyTransferUnits = 1200;
    constexpr int GTradeRouteDefaultDurationDays = 1500;

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

    enum class EPowerPriorityBand
    {
        Industry = 0,
        Service,
        Housing
    };

    int ResolvePowerPriorityBandIndex(EBuildingCategory Category)
    {
        switch (Category)
        {
        case EBuildingCategory::FoodResource:
        case EBuildingCategory::Industry:
            return static_cast<int>(EPowerPriorityBand::Industry);
        case EBuildingCategory::Housing:
            return static_cast<int>(EPowerPriorityBand::Housing);
        case EBuildingCategory::Infrastructure:
        case EBuildingCategory::Entertainment:
        case EBuildingCategory::MediaEducation:
        case EBuildingCategory::Tourism:
        case EBuildingCategory::PublicService:
        default:
            return static_cast<int>(EPowerPriorityBand::Service);
        }
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
        FEraUnlockRequirement Requirement;

        switch (TargetEra)
        {
        case EBuildingEra::WorldWars:
            Requirement.MinPopulation = 24;
            Requirement.MinTotalBuildings = 8;
            Requirement.MinFoodProviders = 3;
            break;
        case EBuildingEra::ColdWar:
            Requirement.MinPopulation = 60;
            Requirement.MinTotalBuildings = 18;
            Requirement.MinIndustryBuildings = 4;
            Requirement.MinPowerMW = 20;
            break;
        case EBuildingEra::Modern:
            Requirement.MinPopulation = 120;
            Requirement.MinTotalBuildings = 32;
            Requirement.MinPublicServiceBuildings = 5;
            Requirement.MinEntertainmentBuildings = 4;
            Requirement.MinPowerMW = 45;
            break;
        case EBuildingEra::Colonial:
        default:
            break;
        }

        return Requirement;
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
        const double FreedomCollapse =
            NormalizeShortfall(Snapshot.AverageFreedom, 56.0, 28.0);
        const double JobStress =
            NormalizeShortfall(Snapshot.AverageJob, 56.0, 32.0);
        const double FoodStress =
            NormalizeShortfall(Snapshot.AverageFood, 54.0, 28.0);
        const double HousingStress =
            NormalizeShortfall(Snapshot.AverageHousing, 54.0, 28.0);
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
            SecurityCollapse * 0.34 +
            FreedomCollapse * 0.22 +
            Result.OppositionRatio * 0.22 +
            FoodStress * 0.12 +
            HousingStress * 0.10 -
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
            SecurityCollapse * 0.42 +
            Result.HomelessRatio * 0.18 +
            Result.UnemploymentRatio * 0.16 +
            HousingStress * 0.12 +
            NetLossPressure * 0.12 -
            (Result.MartialLawActive ? 0.10 : 0.0),
            0.0,
            1.0);

        Result.FiscalEmergencyRisk = Clamp<double>(
            BudgetDeficit * 0.42 +
            NetLossPressure * 0.26 +
            TaxCollectionBreakdown * 0.20 +
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
        int& InOutFiscalEmergencyPressureDays)
    {
        if (Type == EWorldCrisisType::None ||
            InOutStatus.Active ||
            InOutStatus.CooldownDays > 0)
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
    Record.RecordId = mNextTradeRouteCompletionRecordId++;
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
        ResolveTradeRouteCompletionRewardModifier(Route, EndReason);
    Record.SecondaryRelationModifier =
        ResolveTradeRouteSecondaryRelationModifier(Route, EndReason);
    Record.StandingModifier =
        ResolveTradeRouteStandingModifier(Route, EndReason);

    mCompletedTradeRoutes.insert(mCompletedTradeRoutes.begin(), Record);

    if (mCompletedTradeRoutes.size() >
        static_cast<size_t>(GMaxCompletedTradeRouteRecordCount))
    {
        mCompletedTradeRoutes.resize(
            static_cast<size_t>(GMaxCompletedTradeRouteRecordCount));
    }

    ++mTradeRouteCompletionNotificationVersion;
}

void CMainWorld::ProcessActiveTradeRoutes()
{
    if (mActiveTradeRoutes.empty())
        return;

    auto World = mSelf.lock();

    if (!World)
        return;

    std::vector<std::shared_ptr<CPlacementAreaObject>> Harbors =
        CollectOperationalHarbors(World);
    std::vector<FTradeRouteRuntimeState> RemainingRoutes;
    RemainingRoutes.reserve(mActiveTradeRoutes.size());
    bool BudgetChanged = false;

    for (size_t RouteIndex = 0; RouteIndex < mActiveTradeRoutes.size(); ++RouteIndex)
    {
        FTradeRouteRuntimeState Route = mActiveTradeRoutes[RouteIndex];
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
            ResolveTradeRouteDailyTransferUnits(Route));

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

    mActiveTradeRoutes.swap(RemainingRoutes);

    if (BudgetChanged)
    {
        RefreshWorldMarketPrices();
        RefreshPoliticalSnapshot();
    }
}

void CMainWorld::RefreshPowerGridCoverage()
{
    auto World = mSelf.lock();

    if (!World)
        return;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return;

    int TotalProducedPowerMW = 0;

    struct FPowerConsumerNode
    {
        std::shared_ptr<CPlacementAreaObject> Building;
        int RequiredPowerMW = 0;
        int PriorityBandIndex = 0;
    };

    std::vector<FPowerConsumerNode> PowerConsumers;
    PowerConsumers.reserve(BuildingList.size());
    std::array<int, GPowerPriorityBandCount> RequiredPowerByBand = {};

    for (size_t Index = 0; Index < BuildingList.size(); ++Index)
    {
        auto Building = BuildingList[Index].lock();

        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable() ||
            !Building->HasPlacedArea())
        {
            continue;
        }

        TotalProducedPowerMW +=
            (std::max)(0, Building->GetProducedPowerMW());
        const int RequiredPowerMW = Building->GetRequiredPowerMW();

        if (RequiredPowerMW <= 0)
        {
            Building->SetPowerSupplyRatio(1.f);
            continue;
        }

        FPowerConsumerNode Node;
        Node.Building = std::move(Building);
        Node.RequiredPowerMW = RequiredPowerMW;
        Node.PriorityBandIndex =
            ResolvePowerPriorityBandIndex(
                Node.Building->GetBuildingCategory());
        PowerConsumers.push_back(std::move(Node));
        RequiredPowerByBand[static_cast<size_t>(PowerConsumers.back().PriorityBandIndex)] +=
            RequiredPowerMW;
    }

    int RemainingProducedPowerMW = (std::max)(0, TotalProducedPowerMW);
    std::array<float, GPowerPriorityBandCount> CoverageByBand =
    {
        1.f,
        1.f,
        1.f
    };

    for (int BandIndex = 0;
        BandIndex < GPowerPriorityBandCount;
        ++BandIndex)
    {
        const int BandDemandMW =
            RequiredPowerByBand[static_cast<size_t>(BandIndex)];

        if (BandDemandMW <= 0)
            continue;

        CoverageByBand[static_cast<size_t>(BandIndex)] =
            Clamp<float>(
                static_cast<float>(RemainingProducedPowerMW) /
                    static_cast<float>(BandDemandMW),
                0.f,
                1.f);
        RemainingProducedPowerMW = (std::max)(
            0,
            RemainingProducedPowerMW -
                (std::min)(RemainingProducedPowerMW, BandDemandMW));
    }

    for (size_t Index = 0; Index < PowerConsumers.size(); ++Index)
    {
        const FPowerConsumerNode& Node = PowerConsumers[Index];
        Node.Building->SetPowerSupplyRatio(
            CoverageByBand[static_cast<size_t>(Node.PriorityBandIndex)]);
    }
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
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay);
}

void CMainWorld::RefreshBuildingPollutionExposure()
{
    auto World = mSelf.lock();

    if (!World)
        return;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return;

    struct FPollutionNode
    {
        std::shared_ptr<CPlacementAreaObject> Building;
        int GridX = 0;
        int GridY = 0;
        int NetInfluence = 0;
    };

    std::vector<FPollutionNode> PollutionNodes;
    PollutionNodes.reserve(BuildingList.size());

    for (size_t Index = 0; Index < BuildingList.size(); ++Index)
    {
        auto Building = BuildingList[Index].lock();

        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable() ||
            !Building->HasPlacedArea())
        {
            continue;
        }

        int GridX = 0;
        int GridY = 0;

        if (!Building->GetPlacedCenterGridCoords(GridX, GridY))
            continue;

        FPollutionNode Node;
        Node.Building = Building;
        Node.GridX = GridX;
        Node.GridY = GridY;
        Node.NetInfluence =
            Building->GetPollutionOutput() -
            Building->GetPollutionMitigation();
        PollutionNodes.push_back(std::move(Node));
    }

    const float RadiusSq =
        GBuildingPollutionRadiusTiles * GBuildingPollutionRadiusTiles;

    for (size_t TargetIndex = 0;
        TargetIndex < PollutionNodes.size();
        ++TargetIndex)
    {
        FPollutionNode& TargetNode = PollutionNodes[TargetIndex];
        float TotalExposure = 0.f;

        for (size_t SourceIndex = 0;
            SourceIndex < PollutionNodes.size();
            ++SourceIndex)
        {
            const FPollutionNode& SourceNode = PollutionNodes[SourceIndex];

            if (SourceNode.NetInfluence == 0)
                continue;

            const float dx =
                static_cast<float>(TargetNode.GridX - SourceNode.GridX);
            const float dy =
                static_cast<float>(TargetNode.GridY - SourceNode.GridY);
            const float DistanceSq = dx * dx + dy * dy;

            if (DistanceSq > RadiusSq)
                continue;

            const float Distance =
                DistanceSq > 0.f ? std::sqrt(DistanceSq) : 0.f;
            const float Weight =
                (std::max)(
                    0.f,
                    1.f - Distance / GBuildingPollutionRadiusTiles);
            TotalExposure +=
                static_cast<float>(SourceNode.NetInfluence) * Weight;
        }

        TargetNode.Building->SetLocalPollutionExposure(
            (std::max)(
                0,
                (std::min)(100, static_cast<int>(roundf(TotalExposure)))));
    }
}

void CMainWorld::RefreshRuntimeBuildingState()
{
    RefreshPowerGridCoverage();
    RefreshBuildingPollutionExposure();
    ReassignCitizenNeeds();
    RefreshEraProgress();
    RefreshPoliticalSnapshot();
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
    return EconomySystem::AdjustTaxPolicy(
        mGovernmentProfile.TaxPolicy,
        Type,
        DeltaPercent,
        OutMessage);
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

    if (static_cast<int>(mActiveTradeRoutes.size()) >=
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
        CollectOperationalHarbors(World);

    if (Harbors.empty())
    {
        OutMessage = L"운영 중인 항구가 없어 무역로를 활성화할 수 없습니다.";
        return false;
    }

    FTradeRouteRuntimeState Route;
    Route.RouteId = mNextTradeRouteId++;
    Route.ImportRoute = ImportRoute;
    Route.ResourceType = ResourceType;
    Route.MarketClass = GetResourceMarketClass(ResourceType);
    Route.ForeignPowerIndex =
        (std::max)(0, (std::min)(4, ForeignPowerIndex));
    Route.ContractUnits = SafeAmount;
    Route.FulfilledUnits = 0;
    Route.TotalDurationDays = ResolveTradeRouteDurationDays(SafeAmount);
    Route.RemainingDays = Route.TotalDurationDays;
    Route.RoutePricePerThousandUnits = SafePricePerThousand;
    Route.SignedStandardPricePerThousandUnits =
        ComputeTradeRouteSignedStandardPricePerThousand(
            ResourceType,
            ImportRoute);
    mActiveTradeRoutes.push_back(Route);

    RefreshWorldMarketPrices();
    RefreshPoliticalSnapshot();
    OutMessage =
        std::wstring(GetTradeForeignPowerName(ForeignPowerIndex)) +
        L"과(와) " +
        GetResourceTypeDisplayName(ResourceType) +
        L" " +
        FormatTradeUnits(SafeAmount) +
        L" 단위 무역로 활성화";
    return true;
}

bool CMainWorld::CancelTradeRoute(
    int RouteId,
    std::wstring& OutMessage)
{
    const auto RouteIt = std::find_if(
        mActiveTradeRoutes.begin(),
        mActiveTradeRoutes.end(),
        [RouteId](const FTradeRouteRuntimeState& Route)
        {
            return Route.RouteId == RouteId;
        });

    if (RouteIt == mActiveTradeRoutes.end())
    {
        OutMessage = L"취소할 수 있는 무역 계약이 없습니다.";
        return false;
    }

    const std::wstring DirectionText =
        RouteIt->ImportRoute ? L"수입" : L"수출";
    const std::wstring ResourceName =
        GetResourceTypeDisplayName(RouteIt->ResourceType);

    RecordFinishedTradeRoute(*RouteIt, ETradeRouteEndReason::Cancelled);
    mActiveTradeRoutes.erase(RouteIt);
    RefreshWorldMarketPrices();
    RefreshPoliticalSnapshot();
    OutMessage =
        DirectionText +
        L": " +
        ResourceName +
        L" 무역 계약 취소";
    return true;
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
        RefreshEdictModifiers();
}

void CMainWorld::RefreshEdictModifiers()
{
    mEdictModifiers = EdictSystem::CalculateEdictModifiers(
        mGovernmentEdicts,
        mPoliticalSnapshot.ActiveCitizenCount);
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
                FoodDelta = -0.35f - static_cast<float>(0.55 * Severity);
                FunDelta = -0.45f - static_cast<float>(0.45 * Severity);
                HousingDelta = -0.30f - static_cast<float>(0.40 * Severity);
                JobDelta = -0.70f - static_cast<float>(0.90 * Severity);
                FreedomDelta = -0.20f - static_cast<float>(0.30 * Severity);
                SecurityDelta = -2.20f - static_cast<float>(2.80 * Severity);
                break;
            case EWorldCrisisType::LaborStrike:
                FunDelta = -0.30f - static_cast<float>(0.35 * Severity);
                JobDelta = -1.40f - static_cast<float>(1.60 * Severity);
                FreedomDelta = 0.08f;
                SecurityDelta = -0.80f - static_cast<float>(0.90 * Severity);
                break;
            case EWorldCrisisType::CrimeWave:
                FunDelta = -0.35f - static_cast<float>(0.30 * Severity);
                HousingDelta = -0.55f - static_cast<float>(0.45 * Severity);
                JobDelta = -0.60f - static_cast<float>(0.55 * Severity);
                FreedomDelta = -0.30f - static_cast<float>(0.30 * Severity);
                SecurityDelta = -2.60f - static_cast<float>(2.60 * Severity);
                break;
            case EWorldCrisisType::FiscalEmergency:
                FoodDelta = -0.35f - static_cast<float>(0.40 * Severity);
                FunDelta = -0.45f - static_cast<float>(0.40 * Severity);
                HousingDelta = -0.40f - static_cast<float>(0.40 * Severity);
                JobDelta = -0.90f - static_cast<float>(1.00 * Severity);
                FreedomDelta = -0.18f - static_cast<float>(0.18 * Severity);
                SecurityDelta = -0.60f - static_cast<float>(0.70 * Severity);
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
                static_cast<long long>(llround(1800.0 * Severity)) +
                static_cast<long long>(StolenAmount) * 42LL);
        ApplyBudgetDelta(BudgetDamage);
        break;
    }
    case EWorldCrisisType::LaborStrike:
        ApplyBudgetDelta(
            -(250LL +
                static_cast<long long>(llround(550.0 * Severity))));
        break;
    case EWorldCrisisType::CrimeWave:
        ApplyTaxLoss(mLastDailyPropertyTaxIncome, 0.08 + 0.12 * Severity);
        ApplyBudgetDelta(
            -(550LL +
                static_cast<long long>(llround(1450.0 * Severity))));
        break;
    case EWorldCrisisType::FiscalEmergency:
        ApplyTaxLoss(mLastDailyConsumptionTaxIncome, 0.06 + 0.08 * Severity);
        ApplyTaxLoss(mLastDailyIncomeTaxIncome, 0.08 + 0.10 * Severity);
        ApplyTaxLoss(mLastDailyPropertyTaxIncome, 0.05 + 0.08 * Severity);
        ApplyBudgetDelta(
            -(1200LL +
                static_cast<long long>(llround(2600.0 * Severity))));
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

    if (mWorldCrisisStatus.Active)
    {
        ++mWorldCrisisStatus.DaysActive;
        mWorldCrisisStatus.Summary = BuildWorldCrisisWarningSummary(
            mWorldCrisisStatus.Type,
            mWorldCrisisStatus.DaysActive);

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
            return;
        }

        if (mWorldCrisisStatus.RemainingDays <= 0)
            ResolveWorldCrisisState(mWorldCrisisStatus, false);

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

    long long ImmediateBudgetDelta = 0;

    switch (NextType)
    {
    case EWorldCrisisType::Raid:
        ImmediateBudgetDelta =
            -(900LL + static_cast<long long>(llround(1600.0 * NextRisk)));
        break;
    case EWorldCrisisType::LaborStrike:
        ImmediateBudgetDelta =
            -(250LL + static_cast<long long>(llround(500.0 * NextRisk)));
        break;
    case EWorldCrisisType::CrimeWave:
        ImmediateBudgetDelta =
            -(700LL + static_cast<long long>(llround(1200.0 * NextRisk)));
        break;
    case EWorldCrisisType::FiscalEmergency:
        ImmediateBudgetDelta =
            -(1600LL + static_cast<long long>(llround(2600.0 * NextRisk)));
        break;
    case EWorldCrisisType::None:
    default:
        break;
    }

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
        mFiscalEmergencyPressureDays);
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
