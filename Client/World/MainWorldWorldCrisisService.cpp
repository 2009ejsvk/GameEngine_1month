#include "MainWorldWorldCrisisService.h"
#include "MainWorldBuildingControlAccess.h"
#include "WorldStatsSnapshot.h"
#include "../GameConstants.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "World/World.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace
{
    namespace MWWorldCrisis = GameConstants::MainWorld::WorldCrisis;

    int GetEraIndex(EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::WorldWars: return 1;
        case EBuildingEra::ColdWar:   return 2;
        case EBuildingEra::Modern:    return 3;
        case EBuildingEra::Colonial:
        default:                      return 0;
        }
    }

    void ApplyEraCooldownMultiplier(
        FWorldCrisisStatus& InOutStatus,
        EBuildingEra Era)
    {
        if (InOutStatus.CooldownDays <= 0)
            return;

        const int EraIndex = GetEraIndex(Era);
        const float Multiplier =
            GameConstants::MainWorld::EraCrisisCooldownMultipliers[EraIndex];

        if (Multiplier != 1.0f)
        {
            InOutStatus.CooldownDays = static_cast<int>(
                static_cast<float>(InOutStatus.CooldownDays) * Multiplier + 0.5f);
        }
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
            return MWWorldCrisis::RaidDurationDays;
        case EWorldCrisisType::LaborStrike:
            return MWWorldCrisis::LaborStrikeDurationDays;
        case EWorldCrisisType::CrimeWave:
            return MWWorldCrisis::CrimeWaveDurationDays;
        case EWorldCrisisType::FiscalEmergency:
            return MWWorldCrisis::FiscalEmergencyDurationDays;
        case EWorldCrisisType::None:
        default:
            return 0;
        }
    }

    int GetWorldCrisisCooldownDays(bool Success)
    {
        return Success ?
            MWWorldCrisis::SuccessCooldownDays :
            MWWorldCrisis::FailureCooldownDays;
    }

    double GetWorldCrisisSeverity(const FWorldCrisisStatus& Status)
    {
        if (!Status.Active || Status.Type == EWorldCrisisType::None)
            return 0.0;

        return Clamp<double>(
            static_cast<double>(Status.DaysActive + 1) /
                (std::max)(
                    1.0,
                    static_cast<double>(
                        MWWorldCrisis::SeverityDurationDivisorDays)),
            0.0,
            1.0);
    }

    long long ResolveWorldCrisisImmediateBudgetDeltaFromTuning(
        const MWWorldCrisis::FImmediateBudgetTuning& Tuning,
        double Risk)
    {
        return -(static_cast<long long>(Tuning.BasePenalty) +
            static_cast<long long>(std::llround(
                static_cast<double>(Tuning.RiskPenaltyScale) * Risk)));
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
            return ResolveWorldCrisisImmediateBudgetDeltaFromTuning(
                MWWorldCrisis::RaidImmediateBudget,
                Risk);
        case EWorldCrisisType::LaborStrike:
            return ResolveWorldCrisisImmediateBudgetDeltaFromTuning(
                MWWorldCrisis::LaborStrikeImmediateBudget,
                Risk);
        case EWorldCrisisType::CrimeWave:
            return ResolveWorldCrisisImmediateBudgetDeltaFromTuning(
                MWWorldCrisis::CrimeWaveImmediateBudget,
                Risk);
        case EWorldCrisisType::FiscalEmergency:
            return ResolveWorldCrisisImmediateBudgetDeltaFromTuning(
                MWWorldCrisis::FiscalEmergencyImmediateBudget,
                Risk);
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
            (Failure ?
                static_cast<double>(MWWorldCrisis::FollowupFailureBaseCarryover) :
                static_cast<double>(MWWorldCrisis::FollowupSuccessBaseCarryover)) +
            Severity *
                static_cast<double>(MWWorldCrisis::FollowupSeverityCarryoverWeight);

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
                static_cast<int>(std::llround(
                    Clamp<double>(Risk, 0.0, 1.0) * 100.0))) +
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
        const int LightPressure =
            Severity >=
                static_cast<double>(
                    MWWorldCrisis::LightPressureSeverityThreshold) ?
                1 :
                0;
        const int HeavyPressure =
            Severity >=
                static_cast<double>(
                    MWWorldCrisis::HeavyPressureSeverityThreshold) ?
                1 :
                0;
        const int FailureBonus = Failure ? 1 : 0;

        switch (Type)
        {
        case EWorldCrisisType::Raid:
            InOutCrimeWavePressureDays +=
                MWWorldCrisis::RaidPressureTransfer.PrimaryBaseDays +
                LightPressure +
                FailureBonus;
            InOutFiscalEmergencyPressureDays +=
                MWWorldCrisis::RaidPressureTransfer.SecondaryBaseDays +
                LightPressure +
                HeavyPressure +
                FailureBonus;
            break;
        case EWorldCrisisType::LaborStrike:
            InOutFiscalEmergencyPressureDays +=
                MWWorldCrisis::LaborStrikePressureTransfer.PrimaryBaseDays +
                LightPressure +
                FailureBonus;
            InOutCrimeWavePressureDays +=
                MWWorldCrisis::LaborStrikePressureTransfer.SecondaryBaseDays +
                LightPressure +
                HeavyPressure +
                FailureBonus;
            break;
        case EWorldCrisisType::CrimeWave:
            InOutRaidPressureDays +=
                MWWorldCrisis::CrimeWavePressureTransfer.PrimaryBaseDays +
                LightPressure +
                FailureBonus;
            InOutFiscalEmergencyPressureDays +=
                MWWorldCrisis::CrimeWavePressureTransfer.SecondaryBaseDays +
                LightPressure +
                HeavyPressure +
                FailureBonus;
            break;
        case EWorldCrisisType::FiscalEmergency:
            InOutLaborStrikePressureDays +=
                MWWorldCrisis::FiscalEmergencyPressureTransfer.PrimaryBaseDays +
                LightPressure +
                FailureBonus;
            InOutCrimeWavePressureDays +=
                MWWorldCrisis::FiscalEmergencyPressureTransfer.SecondaryBaseDays +
                LightPressure +
                HeavyPressure +
                FailureBonus;
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
        const auto& RaidRiskTuning = MWWorldCrisis::RaidRisk;
        const auto& LaborStrikeRiskTuning = MWWorldCrisis::LaborStrikeRisk;
        const auto& CrimeWaveRiskTuning = MWWorldCrisis::CrimeWaveRisk;
        const auto& FiscalEmergencyRiskTuning =
            MWWorldCrisis::FiscalEmergencyRisk;
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
            NormalizeShortfall(
                Snapshot.AverageSecurity,
                static_cast<double>(MWWorldCrisis::SecurityCollapse.StartValue),
                static_cast<double>(MWWorldCrisis::SecurityCollapse.FullValue));
        const double ResidentialSecurityCollapse =
            NormalizeShortfall(
                Snapshot.AverageResidentialSecurity,
                static_cast<double>(
                    MWWorldCrisis::ResidentialSecurityCollapse.StartValue),
                static_cast<double>(
                    MWWorldCrisis::ResidentialSecurityCollapse.FullValue));
        const double FreedomCollapse =
            NormalizeShortfall(
                Snapshot.AverageFreedom,
                static_cast<double>(MWWorldCrisis::FreedomCollapse.StartValue),
                static_cast<double>(MWWorldCrisis::FreedomCollapse.FullValue));
        const double ResidentialFreedomCollapse =
            NormalizeShortfall(
                Snapshot.AverageResidentialFreedom,
                static_cast<double>(
                    MWWorldCrisis::ResidentialFreedomCollapse.StartValue),
                static_cast<double>(
                    MWWorldCrisis::ResidentialFreedomCollapse.FullValue));
        const double JobStress =
            NormalizeShortfall(
                Snapshot.AverageJob,
                static_cast<double>(MWWorldCrisis::JobStress.StartValue),
                static_cast<double>(MWWorldCrisis::JobStress.FullValue));
        const double FoodStress =
            NormalizeShortfall(
                Snapshot.AverageFood,
                static_cast<double>(MWWorldCrisis::FoodStress.StartValue),
                static_cast<double>(MWWorldCrisis::FoodStress.FullValue));
        const double HousingStress =
            NormalizeShortfall(
                Snapshot.AverageHousing,
                static_cast<double>(MWWorldCrisis::HousingStress.StartValue),
                static_cast<double>(MWWorldCrisis::HousingStress.FullValue));
        const double ResidentialPollutionStress =
            NormalizeOverflow(
                Snapshot.AverageResidentialPollution,
                static_cast<double>(
                    MWWorldCrisis::ResidentialPollutionStress.StartValue),
                static_cast<double>(
                    MWWorldCrisis::ResidentialPollutionStress.FullValue));
        const double BudgetDeficit =
            NationalBudget < 0 ?
                NormalizeOverflow(
                    static_cast<double>(-NationalBudget),
                    static_cast<double>(MWWorldCrisis::BudgetDeficit.StartValue),
                    static_cast<double>(MWWorldCrisis::BudgetDeficit.FullValue)) :
                0.0;
        const double NetLossPressure =
            LastDailyNetChange < 0 ?
                NormalizeOverflow(
                    static_cast<double>(-LastDailyNetChange),
                    static_cast<double>(MWWorldCrisis::NetLossPressure.StartValue),
                    static_cast<double>(MWWorldCrisis::NetLossPressure.FullValue)) :
                0.0;
        const double TaxCollectionBreakdown =
            NormalizeShortfall(
                TaxCollectionEfficiency,
                static_cast<double>(
                    MWWorldCrisis::TaxCollectionBreakdown.StartValue),
                static_cast<double>(
                    MWWorldCrisis::TaxCollectionBreakdown.FullValue));
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
            SecurityCollapse *
                static_cast<double>(RaidRiskTuning.SecurityCollapseWeight) +
            ResidentialSecurityCollapse * static_cast<double>(
                RaidRiskTuning.ResidentialSecurityCollapseWeight) +
            FreedomCollapse *
                static_cast<double>(RaidRiskTuning.FreedomCollapseWeight) +
            ResidentialFreedomCollapse * static_cast<double>(
                RaidRiskTuning.ResidentialFreedomCollapseWeight) +
            Result.OppositionRatio *
                static_cast<double>(RaidRiskTuning.OppositionRatioWeight) +
            FoodStress *
                static_cast<double>(RaidRiskTuning.FoodStressWeight) +
            HousingStress *
                static_cast<double>(RaidRiskTuning.HousingStressWeight) +
            ResidentialPollutionStress * static_cast<double>(
                RaidRiskTuning.ResidentialPollutionStressWeight) -
            (Result.MartialLawActive ?
                static_cast<double>(RaidRiskTuning.MartialLawReduction) :
                0.0),
            0.0,
            1.0);

        Result.LaborStrikeRisk = Clamp<double>(
            Result.UnemploymentRatio * static_cast<double>(
                LaborStrikeRiskTuning.UnemploymentRatioWeight) +
            JobStress *
                static_cast<double>(LaborStrikeRiskTuning.JobStressWeight) +
            IncomeTaxPressure * static_cast<double>(
                LaborStrikeRiskTuning.IncomeTaxPressureWeight) +
            Result.OppositionRatio * static_cast<double>(
                LaborStrikeRiskTuning.OppositionRatioWeight) +
            (TaxEventStatus.Active &&
                TaxEventStatus.Type == ETaxPolicyEventType::WorkerTaxStrike ?
                    static_cast<double>(
                        LaborStrikeRiskTuning.WorkerTaxStrikeBonus) :
                    0.0),
            0.0,
            1.0);

        Result.CrimeWaveRisk = Clamp<double>(
            SecurityCollapse * static_cast<double>(
                CrimeWaveRiskTuning.SecurityCollapseWeight) +
            ResidentialSecurityCollapse * static_cast<double>(
                CrimeWaveRiskTuning.ResidentialSecurityCollapseWeight) +
            Result.HomelessRatio *
                static_cast<double>(CrimeWaveRiskTuning.HomelessRatioWeight) +
            Result.UnemploymentRatio * static_cast<double>(
                CrimeWaveRiskTuning.UnemploymentRatioWeight) +
            HousingStress *
                static_cast<double>(CrimeWaveRiskTuning.HousingStressWeight) +
            ResidentialPollutionStress * static_cast<double>(
                CrimeWaveRiskTuning.ResidentialPollutionStressWeight) +
            NetLossPressure *
                static_cast<double>(CrimeWaveRiskTuning.NetLossPressureWeight) -
            (Result.MartialLawActive ?
                static_cast<double>(CrimeWaveRiskTuning.MartialLawReduction) :
                0.0),
            0.0,
            1.0);

        Result.FiscalEmergencyRisk = Clamp<double>(
            BudgetDeficit *
                static_cast<double>(FiscalEmergencyRiskTuning.BudgetDeficitWeight) +
            NetLossPressure *
                static_cast<double>(FiscalEmergencyRiskTuning.NetLossPressureWeight) +
            TaxCollectionBreakdown * static_cast<double>(
                FiscalEmergencyRiskTuning.TaxCollectionBreakdownWeight) +
            ResidentialPollutionStress * static_cast<double>(
                FiscalEmergencyRiskTuning.ResidentialPollutionStressWeight) +
            (TaxEventStatus.Active &&
                TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis ?
                    static_cast<double>(
                        FiscalEmergencyRiskTuning.BudgetCrisisBonus) :
                    0.0) +
            (Snapshot.ActiveCitizenCount >=
                    FiscalEmergencyRiskTuning.LargePopulationThreshold ?
                static_cast<double>(
                    FiscalEmergencyRiskTuning.LargePopulationBonus) :
                0.0),
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
        InOutStatus.NotificationDays = MWWorldCrisis::StartNotificationDays;
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
        InOutStatus.NotificationDays =
            MWWorldCrisis::ResolvedNotificationDays;
        InOutStatus.Summary = BuildWorldCrisisResolvedSummary(
            InOutStatus.Type,
            Success);
        InOutStatus.DaysActive = 0;
    }

    int ConsumeRaidResourcesFromBuildings(
        const std::vector<std::weak_ptr<CPlacementAreaObject>>& BuildingList,
        bool RequirePriorityTarget,
        int RemainingTarget)
    {
        if (RemainingTarget <= 0)
            return 0;

        int StolenAmount = 0;

        auto ConsumeFromBuilding =
            [&](const std::shared_ptr<CPlacementAreaObject>& Building)
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

        for (size_t Index = 0;
            Index < BuildingList.size() && RemainingTarget > 0;
            ++Index)
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

    bool IsRaidPriorityBuilding(const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        return Building &&
            (Building->IsHarbor() ||
                Building->IsWarehouse() ||
                Building->CanGenerateWorkOutput());
    }

    int ApplyRaidBuildingDamage(
        const std::shared_ptr<CWorld>& World,
        double Severity,
        double ChainIntensity)
    {
        if (!World)
            return 0;

        auto DamageAccess =
            ResolveMainWorldBuildingConditionAccess(World);

        if (!DamageAccess)
            return 0;

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
            return 0;

        std::vector<std::shared_ptr<CPlacementAreaObject>> PriorityBuildings;
        std::vector<std::shared_ptr<CPlacementAreaObject>> SecondaryBuildings;

        for (size_t Index = 0; Index < BuildingList.size(); ++Index)
        {
            auto Building = BuildingList[Index].lock();

            if (!IsOperationalBuilding(Building) ||
                Building->IsRoad() ||
                Building->GetDamageLevel() == EBuildingDamageLevel::Critical)
            {
                continue;
            }

            if (IsRaidPriorityBuilding(Building))
                PriorityBuildings.push_back(Building);
            else
                SecondaryBuildings.push_back(Building);
        }

        auto ApplySingleHit =
            [&](std::vector<std::shared_ptr<CPlacementAreaObject>>& Targets)
            {
                if (Targets.empty())
                    return false;

                const size_t PickIndex =
                    static_cast<size_t>(rand() % static_cast<int>(Targets.size()));
                const auto Target = Targets[PickIndex];
                Targets.erase(Targets.begin() + static_cast<int>(PickIndex));

                if (!Target)
                    return false;

                const EBuildingDamageLevel NextLevel =
                    Target->GetDamageLevel() == EBuildingDamageLevel::None ?
                        EBuildingDamageLevel::Damaged :
                        EBuildingDamageLevel::Critical;
                return DamageAccess->DamageBuilding(Target->GetName(), NextLevel);
            };

        int TargetHits = 1;

        if (Severity >= 0.35 || ChainIntensity >= 1.1)
            ++TargetHits;

        if (Severity >= 0.72 || ChainIntensity >= 1.28)
            ++TargetHits;

        int AppliedHits = 0;

        for (int HitIndex = 0; HitIndex < TargetHits; ++HitIndex)
        {
            if (ApplySingleHit(PriorityBuildings) ||
                ApplySingleHit(SecondaryBuildings))
            {
                ++AppliedHits;
            }
            else
            {
                break;
            }
        }

        if (AppliedHits > 0)
        {
            if (auto RefreshAccess =
                    ResolveMainWorldRuntimeRefreshAccess(World))
            {
                RefreshAccess->RefreshRuntimeBuildingState();
            }
        }

        return AppliedHits;
    }
} // namespace

void CMainWorldWorldCrisisService::Reset()
{
    mRaidPressureDays = 0;
    mLaborStrikePressureDays = 0;
    mCrimeWavePressureDays = 0;
    mFiscalEmergencyPressureDays = 0;
    mActiveWorldCrisisChainDepth = 0;
    mQueuedWorldCrisisType = EWorldCrisisType::None;
    mQueuedWorldCrisisRisk = 0.0;
    mQueuedWorldCrisisDelayDays = 0;
    mQueuedWorldCrisisChainDepth = 0;
    mWorldCrisisStatus = FWorldCrisisStatus();
    mLaborStrikeWarningActive = false;
    mLaborStrikeImminent = false;
}

void CMainWorldWorldCrisisService::TriggerForcedCrisis(
    EWorldCrisisType Type,
    const FTickContext& Context)
{
    if (!Context.World || Type == EWorldCrisisType::None)
        return;

    const double ForcedRisk =
        Type == EWorldCrisisType::FiscalEmergency ? 0.84 :
        Type == EWorldCrisisType::Raid ? 0.82 :
        Type == EWorldCrisisType::LaborStrike ? 0.80 :
        Type == EWorldCrisisType::CrimeWave ? 0.78 :
        0.76;

    if (mWorldCrisisStatus.Active)
    {
        if (mWorldCrisisStatus.Type == Type)
        {
            mWorldCrisisStatus.NotificationDays = (std::max)(
                mWorldCrisisStatus.NotificationDays,
                MWWorldCrisis::StartNotificationDays);
            mWorldCrisisStatus.RemainingDays = (std::max)(
                mWorldCrisisStatus.RemainingDays,
                3);
            mWorldCrisisStatus.Summary =
                BuildWorldCrisisWarningSummary(Type, 0);
            return;
        }

        mQueuedWorldCrisisType = Type;
        mQueuedWorldCrisisRisk = (std::max)(
            Clamp<double>(mQueuedWorldCrisisRisk, 0.0, 1.0),
            ForcedRisk);
        mQueuedWorldCrisisDelayDays = 1;
        mQueuedWorldCrisisChainDepth =
            (std::max)(1, mActiveWorldCrisisChainDepth + 1);
        return;
    }

    const long long ImmediateBudgetDelta =
        ResolveWorldCrisisImmediateBudgetDelta(
            Type,
            ForcedRisk);
    mQueuedWorldCrisisType = EWorldCrisisType::None;
    mQueuedWorldCrisisRisk = 0.0;
    mQueuedWorldCrisisDelayDays = 0;
    mQueuedWorldCrisisChainDepth = 0;
    StartWorldCrisis(
        mWorldCrisisStatus,
        Type,
        Context.SimulationYear,
        Context.SimulationMonth,
        Context.SimulationDay,
        ImmediateBudgetDelta,
        Context.NationalBudget,
        Context.LastDailyNetChange,
        mRaidPressureDays,
        mLaborStrikePressureDays,
        mCrimeWavePressureDays,
        mFiscalEmergencyPressureDays,
        true);
    mActiveWorldCrisisChainDepth = 0;
}

void CMainWorldWorldCrisisService::TriggerForcedRaid(
    const FTickContext& Context)
{
    TriggerForcedCrisis(EWorldCrisisType::Raid, Context);
}

bool CMainWorldWorldCrisisService::ForceEndActiveCrisis(
    bool Success,
    EBuildingEra CurrentEra)
{
    if (!mWorldCrisisStatus.Active ||
        mWorldCrisisStatus.Type == EWorldCrisisType::None)
    {
        return false;
    }

    ResolveWorldCrisisState(mWorldCrisisStatus, Success);
    ApplyEraCooldownMultiplier(mWorldCrisisStatus, CurrentEra);

    mQueuedWorldCrisisType = EWorldCrisisType::None;
    mQueuedWorldCrisisRisk = 0.0;
    mQueuedWorldCrisisDelayDays = 0;
    mQueuedWorldCrisisChainDepth = 0;
    mActiveWorldCrisisChainDepth = 0;

    mLaborStrikeWarningActive = false;
    mLaborStrikeImminent = false;

    return true;
}

void CMainWorldWorldCrisisService::ApplyDailyEffects(
    const FDailyContext& Context)
{
    if (!mWorldCrisisStatus.Active ||
        mWorldCrisisStatus.Type == EWorldCrisisType::None)
    {
        return;
    }

    const double Severity = GetWorldCrisisSeverity(mWorldCrisisStatus);
    const double ChainIntensity =
        1.0 + static_cast<double>(mActiveWorldCrisisChainDepth) * 0.14;

    if (Context.World)
    {
        std::vector<std::weak_ptr<CBuildingMarkerOrb>> CitizenList;

        if (Context.World->FindObjectListByType<CBuildingMarkerOrb>(CitizenList))
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

    auto ApplyBudgetDelta =
        [&](long long Delta)
        {
            if (Delta == 0)
                return;

            Context.NationalBudget += Delta;
            Context.LastDailyNetChange += Delta;
        };

    auto ApplyTaxLoss =
        [&](long long& TaxField, long long& TotalTaxField, double LossRatio)
        {
            if (TaxField <= 0 || LossRatio <= 0.0)
                return;

            const long long LossAmount = static_cast<long long>(std::llround(
                static_cast<double>(TaxField) * LossRatio));
            const long long ClampedLoss = (std::min)(TaxField, LossAmount);

            if (ClampedLoss <= 0)
                return;

            TaxField -= ClampedLoss;
            TotalTaxField = (std::max)(0LL, TotalTaxField - ClampedLoss);
            ApplyBudgetDelta(-ClampedLoss);
        };

    switch (mWorldCrisisStatus.Type)
    {
    case EWorldCrisisType::Raid:
    {
        const int TargetAmount =
            8 + static_cast<int>(std::round(10.0 * Severity));
        const int StolenAmount =
            ApplyRaidResourceTheft(Context.World.get(), TargetAmount);
        const int DamagedBuildingCount =
            ApplyRaidBuildingDamage(Context.World, Severity, ChainIntensity);
        const long long BudgetDamage =
            -(1200LL +
                static_cast<long long>(std::llround(
                    1800.0 * Severity * ChainIntensity)) +
                static_cast<long long>(StolenAmount) * 42LL +
                static_cast<long long>(DamagedBuildingCount) * 180LL);
        ApplyBudgetDelta(BudgetDamage);
        break;
    }
    case EWorldCrisisType::LaborStrike:
        ApplyBudgetDelta(
            -(150LL +
                static_cast<long long>(std::llround(
                    300.0 * Severity * ChainIntensity))));
        break;
    case EWorldCrisisType::CrimeWave:
        ApplyTaxLoss(
            Context.LastDailyPropertyTaxIncome,
            Context.LastDailyTaxIncome,
            0.08 + 0.12 * Severity);
        ApplyBudgetDelta(
            -(550LL +
                static_cast<long long>(std::llround(
                    1450.0 * Severity * ChainIntensity))));
        break;
    case EWorldCrisisType::FiscalEmergency:
        ApplyTaxLoss(
            Context.LastDailyConsumptionTaxIncome,
            Context.LastDailyTaxIncome,
            0.06 + 0.08 * Severity);
        ApplyTaxLoss(
            Context.LastDailyIncomeTaxIncome,
            Context.LastDailyTaxIncome,
            0.08 + 0.10 * Severity);
        ApplyTaxLoss(
            Context.LastDailyPropertyTaxIncome,
            Context.LastDailyTaxIncome,
            0.05 + 0.08 * Severity);
        ApplyBudgetDelta(
            -(1200LL +
                static_cast<long long>(std::llround(
                    2600.0 * Severity * ChainIntensity))));
        break;
    case EWorldCrisisType::None:
    default:
        break;
    }
}

void CMainWorldWorldCrisisService::Tick(const FTickContext& Context)
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

    if (!Context.World)
        return;

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(Context.World);
    const FWorldCrisisPressureSnapshot Pressure =
        BuildWorldCrisisPressureSnapshot(
            Snapshot,
            Context.PoliticalSnapshot,
            Context.NationalBudget,
            Context.LastDailyNetChange,
            Context.LastDailyTaxCollectionEfficiency,
            Context.GovernmentEdicts,
            Context.TaxPolicy,
            Context.TaxEventStatus);
    auto ClearQueuedWorldCrisis = [&]()
    {
        mQueuedWorldCrisisType = EWorldCrisisType::None;
        mQueuedWorldCrisisRisk = 0.0;
        mQueuedWorldCrisisDelayDays = 0;
        mQueuedWorldCrisisChainDepth = 0;
    };

    if (mWorldCrisisStatus.Active)
    {
        mLaborStrikeWarningActive = false;
        mLaborStrikeImminent = false;
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
                (!Context.TaxEventStatus.Active ||
                    Context.TaxEventStatus.Type != ETaxPolicyEventType::WorkerTaxStrike);
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
                Context.NationalBudget >= 15000 &&
                Context.LastDailyNetChange >= -400 &&
                Context.LastDailyTaxCollectionEfficiency >= 0.72 &&
                Pressure.FiscalEmergencyRisk < 0.38 &&
                (!Context.TaxEventStatus.Active ||
                    Context.TaxEventStatus.Type != ETaxPolicyEventType::BudgetCrisis);
            break;
        case EWorldCrisisType::None:
        default:
            break;
        }

        if (CanResolveEarly && Recovered)
        {
            ResolveWorldCrisisState(mWorldCrisisStatus, true);
            ApplyEraCooldownMultiplier(mWorldCrisisStatus, Context.CurrentEra);
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
            ApplyEraCooldownMultiplier(mWorldCrisisStatus, Context.CurrentEra);

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
            Context.SimulationYear,
            Context.SimulationMonth,
            Context.SimulationDay,
            ImmediateBudgetDelta,
            Context.NationalBudget,
            Context.LastDailyNetChange,
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

    // Labor strike pre-warning flags (LaborStrike TriggerThreshold = 3 days)
    {
        const bool NoActiveLaborStrike =
            !mWorldCrisisStatus.Active ||
            mWorldCrisisStatus.Type != EWorldCrisisType::LaborStrike;
        const bool NoCooldown = mWorldCrisisStatus.CooldownDays <= 0;
        mLaborStrikeWarningActive =
            mLaborStrikePressureDays >= 1 &&
            NoActiveLaborStrike &&
            NoCooldown;
        mLaborStrikeImminent =
            mLaborStrikePressureDays >= 2 &&
            NoActiveLaborStrike &&
            NoCooldown;
    }

    if (Snapshot.ActiveCitizenCount < 24 || Snapshot.TotalBuildingCount < 8)
        mRaidPressureDays = 0;
    if (Snapshot.ActiveCitizenCount < 28 || Snapshot.AssignedJobCount < 12)
        mLaborStrikePressureDays = 0;
    if (Snapshot.ActiveCitizenCount < 20 || Snapshot.TotalBuildingCount < 10)
        mCrimeWavePressureDays = 0;
    if (Snapshot.ActiveCitizenCount < 32 &&
        Context.NationalBudget >= 0 &&
        Context.LastDailyNetChange >= 0)
    {
        mFiscalEmergencyPressureDays = 0;
    }

    if (Context.TaxEventStatus.Active)
    {
        if (Context.TaxEventStatus.Type == ETaxPolicyEventType::WorkerTaxStrike)
            mLaborStrikePressureDays = 0;
        if (Context.TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis)
            mFiscalEmergencyPressureDays = 0;
    }

    EWorldCrisisType NextType = EWorldCrisisType::None;
    double NextRisk = 0.0;

    auto ConsiderCrisis =
        [&](EWorldCrisisType Type, int PressureDays, int RequiredDays, double Risk)
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
        Context.SimulationYear,
        Context.SimulationMonth,
        Context.SimulationDay,
        ImmediateBudgetDelta,
        Context.NationalBudget,
        Context.LastDailyNetChange,
        mRaidPressureDays,
        mLaborStrikePressureDays,
        mCrimeWavePressureDays,
        mFiscalEmergencyPressureDays,
        false);
    mActiveWorldCrisisChainDepth = 0;
}
