#include "EconomySystem.h"
#include "../Politics/EdictSystem.h"
#include "World/World.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace
{
    constexpr int GExportPricePerStockUnit = 2;
    constexpr double GDailyConsumptionSpendBase = 30.0;
    constexpr double GDailyWorkerIncomeBase = 66.6666666667;
    constexpr double GDailyResidenceValueBase = 11.4285714286;

    struct FTaxEventEconomyEffects
    {
        double ExportMultiplier = 1.0;
        double ConsumptionTaxLeakage = 0.0;
        double IncomeTaxLeakage = 0.0;
        double PropertyTaxLeakage = 0.0;
        double ResidentialUpkeepMultiplier = 1.0;
        double GlobalUpkeepMultiplier = 1.0;
        double CollectionEfficiencyPenalty = 0.0;
    };

    int* ResolveTaxRatePercent(
        FTaxPolicy& TaxPolicy,
        ETaxPolicyType Type)
    {
        switch (Type)
        {
        case ETaxPolicyType::Consumption:
            return &TaxPolicy.ConsumptionRatePercent;
        case ETaxPolicyType::Income:
            return &TaxPolicy.IncomeRatePercent;
        case ETaxPolicyType::Property:
            return &TaxPolicy.PropertyRatePercent;
        default:
            return nullptr;
        }
    }

    FTaxEventEconomyEffects ResolveTaxEventEconomyEffects(
        const FTaxPolicyEventStatus* TaxEventStatus)
    {
        FTaxEventEconomyEffects Effects;

        if (!TaxEventStatus ||
            !TaxEventStatus->Active ||
            TaxEventStatus->Type == ETaxPolicyEventType::None)
        {
            return Effects;
        }

        const double Severity = Clamp<double>(
            static_cast<double>(TaxEventStatus->DaysActive + 1) / 6.0,
            0.0,
            1.0);

        switch (TaxEventStatus->Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            Effects.ExportMultiplier = 0.86 - 0.18 * Severity;
            Effects.IncomeTaxLeakage = 0.08 + 0.14 * Severity;
            Effects.GlobalUpkeepMultiplier = 1.03 + 0.05 * Severity;
            Effects.CollectionEfficiencyPenalty = 0.03 + 0.05 * Severity;
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            Effects.PropertyTaxLeakage = 0.18 + 0.24 * Severity;
            Effects.ResidentialUpkeepMultiplier = 1.10 + 0.18 * Severity;
            Effects.CollectionEfficiencyPenalty = 0.02 + 0.03 * Severity;
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            Effects.ExportMultiplier = 0.82 - 0.18 * Severity;
            Effects.ConsumptionTaxLeakage = 0.06 + 0.08 * Severity;
            Effects.IncomeTaxLeakage = 0.05 + 0.08 * Severity;
            Effects.PropertyTaxLeakage = 0.08 + 0.10 * Severity;
            Effects.GlobalUpkeepMultiplier = 1.08 + 0.12 * Severity;
            Effects.CollectionEfficiencyPenalty = 0.08 + 0.08 * Severity;
            break;
        default:
            break;
        }

        Effects.ExportMultiplier =
            Clamp<double>(Effects.ExportMultiplier, 0.45, 1.0);
        Effects.ConsumptionTaxLeakage =
            Clamp<double>(Effects.ConsumptionTaxLeakage, 0.0, 0.75);
        Effects.IncomeTaxLeakage =
            Clamp<double>(Effects.IncomeTaxLeakage, 0.0, 0.75);
        Effects.PropertyTaxLeakage =
            Clamp<double>(Effects.PropertyTaxLeakage, 0.0, 0.75);
        Effects.ResidentialUpkeepMultiplier =
            (std::max)(1.0, Effects.ResidentialUpkeepMultiplier);
        Effects.GlobalUpkeepMultiplier =
            (std::max)(1.0, Effects.GlobalUpkeepMultiplier);
        Effects.CollectionEfficiencyPenalty =
            Clamp<double>(Effects.CollectionEfficiencyPenalty, 0.0, 0.35);
        return Effects;
    }

    std::wstring BuildTaxPolicyEventWarningSummary(
        ETaxPolicyEventType Type,
        int DaysActive)
    {
        const bool Escalated = DaysActive >= 4;

        switch (Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            return Escalated ?
                L"자본주의자와 지식인이 근로세 경감을 최후통첩합니다." :
                L"자본주의자와 지식인이 근로세 경감을 요구합니다.";
        case ETaxPolicyEventType::PropertyTaxBacklash:
            return Escalated ?
                L"보수주의자와 자본주의자가 재산세 유예를 강하게 압박합니다." :
                L"보수주의자와 자본주의자가 재산세 유예를 요구합니다.";
        case ETaxPolicyEventType::BudgetCrisis:
            return Escalated ?
                L"보수주의자와 공산주의자가 재정 안정 대책을 최후통첩합니다." :
                L"보수주의자와 공산주의자가 재정 안정 대책을 요구합니다.";
        default:
            return L"정치 경고가 감지되었습니다.";
        }
    }

    std::wstring BuildTaxPolicyEventResolvedSummary(
        ETaxPolicyEventType Type,
        bool Success)
    {
        switch (Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            return Success ?
                L"자본주의자와 지식인이 한발 물러섰습니다." :
                L"자본주의자와 지식인의 조세 시위가 소강 상태로 돌아섰습니다.";
        case ETaxPolicyEventType::PropertyTaxBacklash:
            return Success ?
                L"보수주의자와 자본주의자의 재산권 압박이 진정되었습니다." :
                L"보수주의자와 자본주의자의 반발이 소강 상태에 들어갔습니다.";
        case ETaxPolicyEventType::BudgetCrisis:
            return Success ?
                L"보수주의자와 공산주의자의 재정 압박이 진정되었습니다." :
                L"재정 압박 연대가 일단락되었지만 경계는 남아 있습니다.";
        default:
            return Success ?
                L"정치 경고가 진정되었습니다." :
                L"정치 경고가 일단락되었습니다.";
        }
    }

    int GetTaxPolicyEventDurationDays(ETaxPolicyEventType Type)
    {
        switch (Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            return 8;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            return 7;
        case ETaxPolicyEventType::BudgetCrisis:
            return 9;
        default:
            return 0;
        }
    }

    int GetTaxPolicyEventCooldownDays(bool Success)
    {
        return Success ? 18 : 24;
    }

    void StartTaxPolicyEvent(
        FTaxPolicyEventStatus& InOutTaxEventStatus,
        ETaxPolicyEventType Type,
        int SimulationYear,
        int SimulationMonth,
        int SimulationDay,
        long long ImmediateBudgetDelta,
        long long& InOutNationalBudget,
        long long& InOutLastDailyNetChange,
        int& InOutWorkerTaxPressureDays,
        int& InOutPropertyTaxPressureDays,
        int& InOutBudgetCrisisPressureDays)
    {
        if (Type == ETaxPolicyEventType::None || InOutTaxEventStatus.Active)
            return;

        InOutTaxEventStatus = FTaxPolicyEventStatus();
        InOutTaxEventStatus.Type = Type;
        InOutTaxEventStatus.Active = true;
        InOutTaxEventStatus.RemainingDays = GetTaxPolicyEventDurationDays(Type);
        InOutTaxEventStatus.NotificationDays = 6;
        InOutTaxEventStatus.DaysActive = 0;
        InOutTaxEventStatus.TriggerYear = SimulationYear;
        InOutTaxEventStatus.TriggerMonth = SimulationMonth;
        InOutTaxEventStatus.TriggerDay = SimulationDay;
        InOutTaxEventStatus.Title = EconomySystem::GetTaxPolicyEventTitle(Type);
        InOutTaxEventStatus.Summary = BuildTaxPolicyEventWarningSummary(Type, 0);
        InOutWorkerTaxPressureDays = 0;
        InOutPropertyTaxPressureDays = 0;
        InOutBudgetCrisisPressureDays = 0;

        if (ImmediateBudgetDelta != 0)
        {
            InOutNationalBudget += ImmediateBudgetDelta;
            InOutLastDailyNetChange += ImmediateBudgetDelta;
        }
    }

    void ResolveTaxPolicyEventState(
        FTaxPolicyEventStatus& InOutTaxEventStatus,
        bool Success)
    {
        if (InOutTaxEventStatus.Type == ETaxPolicyEventType::None)
            return;

        InOutTaxEventStatus.Active = false;
        InOutTaxEventStatus.RemainingDays = 0;
        InOutTaxEventStatus.CooldownDays =
            GetTaxPolicyEventCooldownDays(Success);
        InOutTaxEventStatus.NotificationDays = 8;
        InOutTaxEventStatus.Summary = BuildTaxPolicyEventResolvedSummary(
            InOutTaxEventStatus.Type,
            Success);
        InOutTaxEventStatus.DaysActive = 0;
    }
}

EconomySystem::FDailyResult EconomySystem::ApplyDailySettlement(
    CWorld* World,
    int DaysInMonth,
    const FGovernmentProfile& GovernmentProfile,
    const FTaxPolicyEventStatus* TaxEventStatus)
{
    FDailyResult Result;
    const FTaxEventEconomyEffects EventEffects =
        ResolveTaxEventEconomyEffects(TaxEventStatus);

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return Result;

    double PropertyTaxIncome = 0.0;

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        auto Building = BuildingList[i].lock();

        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable() ||
            !Building->HasPlacedArea())
        {
            continue;
        }

        Result.WageCost += Building->GetDailyWageCost(DaysInMonth);
        const double BaseDailyUpkeep =
            static_cast<double>(Building->GetDailyUpkeepCost(DaysInMonth));
        double EffectiveUpkeepMultiplier = EventEffects.GlobalUpkeepMultiplier;

        if (Building->IsResidential())
        {
            EffectiveUpkeepMultiplier *=
                EventEffects.ResidentialUpkeepMultiplier;
        }

        Result.UpkeepCost += static_cast<long long>(std::llround(
            BaseDailyUpkeep * EffectiveUpkeepMultiplier));
        PropertyTaxIncome +=
            (std::max)(
                2.0,
                BaseDailyUpkeep *
                static_cast<double>(
                    GovernmentProfile.TaxPolicy.PropertyRatePercent) / 100.0);

        if (Building->IsHarbor())
        {
            const bool ShipArrived =
                Building->AdvanceHarborShipProgressAndCheckArrival(
                    DaysInMonth);

            if (!ShipArrived)
                continue;

            const int ExportStock = Building->GetExportableResourceStock();
            const int EffectiveExportStock = static_cast<int>(std::floor(
                static_cast<double>(ExportStock) *
                EventEffects.ExportMultiplier));

            if (EffectiveExportStock > 0 &&
                Building->TryConsumeExportableResources(
                    EffectiveExportStock))
            {
                Result.ExportIncome += static_cast<long long>(
                    EffectiveExportStock) * GExportPricePerStockUnit;
            }
        }
    }

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> CitizenList;
    double ConsumptionTaxIncome = 0.0;
    double IncomeTaxIncome = 0.0;
    double ResidenceTaxIncome = 0.0;
    double SecuritySum = 0.0;
    double OverallSum = 0.0;
    int ActiveCitizenCount = 0;

    if (World->FindObjectListByType<CBuildingMarkerOrb>(CitizenList))
    {
        for (size_t i = 0; i < CitizenList.size(); ++i)
        {
            auto Citizen = CitizenList[i].lock();

            if (!Citizen || !Citizen->GetAlive() || !Citizen->GetEnable())
                continue;

            ++ActiveCitizenCount;
            const FNpcSatisfaction& Satisfaction = Citizen->GetSatisfaction();
            SecuritySum += Satisfaction.Security;
            OverallSum += Satisfaction.Overall;

            ConsumptionTaxIncome +=
                GDailyConsumptionSpendBase *
                static_cast<double>(
                    GovernmentProfile.TaxPolicy.ConsumptionRatePercent) /
                100.0;

            if (!Citizen->GetWorkBuilding().empty())
            {
                IncomeTaxIncome +=
                    GDailyWorkerIncomeBase *
                    static_cast<double>(
                        GovernmentProfile.TaxPolicy.IncomeRatePercent) /
                    100.0;
            }

            if (!Citizen->GetHomeBuilding().empty())
            {
                ResidenceTaxIncome +=
                    GDailyResidenceValueBase *
                    static_cast<double>(
                        GovernmentProfile.TaxPolicy.PropertyRatePercent) /
                    100.0;
            }
        }
    }

    ConsumptionTaxIncome *= 1.0 - EventEffects.ConsumptionTaxLeakage;
    IncomeTaxIncome *= 1.0 - EventEffects.IncomeTaxLeakage;

    double CollectionEfficiency = 0.80;

    if (ActiveCitizenCount > 0)
    {
        const double Denominator = static_cast<double>(ActiveCitizenCount);
        const double AverageSecurity = SecuritySum / Denominator;
        const double AverageOverall = OverallSum / Denominator;

        CollectionEfficiency =
            0.45 +
            AverageSecurity / 200.0 +
            AverageOverall / 400.0;
        CollectionEfficiency =
            (std::max)(0.45, (std::min)(1.10, CollectionEfficiency));
    }

    PropertyTaxIncome += ResidenceTaxIncome;
    PropertyTaxIncome *= 1.0 - EventEffects.PropertyTaxLeakage;
    CollectionEfficiency -= EventEffects.CollectionEfficiencyPenalty;
    CollectionEfficiency =
        (std::max)(0.35, (std::min)(1.10, CollectionEfficiency));
    const double GrossTaxIncome =
        ConsumptionTaxIncome +
        IncomeTaxIncome +
        PropertyTaxIncome;

    Result.TaxCollectionEfficiency = CollectionEfficiency;
    Result.TaxIncome = static_cast<long long>(std::llround(
        GrossTaxIncome * CollectionEfficiency));
    Result.ConsumptionTaxIncome = static_cast<long long>(std::llround(
        ConsumptionTaxIncome * CollectionEfficiency));
    Result.IncomeTaxIncome = static_cast<long long>(std::llround(
        IncomeTaxIncome * CollectionEfficiency));
    Result.PropertyTaxIncome =
        Result.TaxIncome -
        Result.ConsumptionTaxIncome -
        Result.IncomeTaxIncome;

    Result.NetChange = Result.ExportIncome +
        Result.TaxIncome -
        Result.WageCost - Result.UpkeepCost;

    return Result;
}

EconomySystem::FWorldSettlementResult EconomySystem::ApplyDailyWorldSettlement(
    CWorld* World,
    int DaysInMonth,
    const FGovernmentProfile& GovernmentProfile,
    const FTaxPolicyEventStatus& TaxEventStatus,
    const std::vector<FGovernmentEdictState>& GovernmentEdicts,
    const FGovernmentEdictModifiers& EdictModifiers)
{
    FWorldSettlementResult Result;
    Result.BaseResult = ApplyDailySettlement(
        World,
        DaysInMonth,
        GovernmentProfile,
        &TaxEventStatus);

    Result.AdjustedTaxIncome = static_cast<long long>(std::llround(
        static_cast<double>(Result.BaseResult.TaxIncome) *
        static_cast<double>(EdictModifiers.TaxRevenueMultiplier)));

    if (Result.BaseResult.TaxIncome > 0 && Result.AdjustedTaxIncome > 0)
    {
        const double BaseTaxIncome =
            static_cast<double>(Result.BaseResult.TaxIncome);
        Result.AdjustedConsumptionTaxIncome = static_cast<long long>(
            std::llround(
                static_cast<double>(Result.AdjustedTaxIncome) *
                static_cast<double>(Result.BaseResult.ConsumptionTaxIncome) /
                BaseTaxIncome));
        Result.AdjustedIncomeTaxIncome = static_cast<long long>(
            std::llround(
                static_cast<double>(Result.AdjustedTaxIncome) *
                static_cast<double>(Result.BaseResult.IncomeTaxIncome) /
                BaseTaxIncome));
        Result.AdjustedPropertyTaxIncome =
            Result.AdjustedTaxIncome -
            Result.AdjustedConsumptionTaxIncome -
            Result.AdjustedIncomeTaxIncome;
    }

    const long long TaxRevenueDelta =
        Result.AdjustedTaxIncome - Result.BaseResult.TaxIncome;
    const long long DailyEdictUpkeep =
        EdictSystem::CalculateEdictDailyUpkeep(
            GovernmentEdicts,
            DaysInMonth);
    const long long DailyEdictBudgetDelta =
        EdictModifiers.DailyBudgetDelta;

    Result.DailyEdictCost = DailyEdictUpkeep - DailyEdictBudgetDelta;
    Result.NetBudgetChange =
        Result.BaseResult.NetChange +
        TaxRevenueDelta -
        DailyEdictUpkeep +
        DailyEdictBudgetDelta;
    return Result;
}

int EconomySystem::ApplyTaxPolicyRateDelta(
    FTaxPolicy& TaxPolicy,
    ETaxPolicyType Type,
    int DeltaPercent)
{
    int* TargetRatePercent = ResolveTaxRatePercent(TaxPolicy, Type);

    if (!TargetRatePercent)
        return 0;

    const int PreviousRatePercent = *TargetRatePercent;
    const int NewRatePercent = (std::max)(
        GetTaxPolicyMinPercent(Type),
        (std::min)(
            GetTaxPolicyMaxPercent(Type),
            PreviousRatePercent + DeltaPercent));
    *TargetRatePercent = NewRatePercent;
    return NewRatePercent - PreviousRatePercent;
}

bool EconomySystem::AdjustTaxPolicy(
    FTaxPolicy& TaxPolicy,
    ETaxPolicyType Type,
    int DeltaPercent,
    std::wstring& OutMessage)
{
    int* TargetRatePercent = ResolveTaxRatePercent(TaxPolicy, Type);

    if (!TargetRatePercent)
    {
        OutMessage = L"정의되지 않은 세율 정책입니다.";
        return false;
    }

    const int PreviousRatePercent = *TargetRatePercent;
    const int NewRatePercent = (std::max)(
        GetTaxPolicyMinPercent(Type),
        (std::min)(
            GetTaxPolicyMaxPercent(Type),
            PreviousRatePercent + DeltaPercent));

    if (PreviousRatePercent == NewRatePercent)
    {
        OutMessage =
            std::wstring(GetTaxPolicyDisplayName(Type)) +
            (DeltaPercent < 0 ?
                L"는 이미 최저 세율입니다." :
                L"는 이미 최고 세율입니다.");
        return false;
    }

    *TargetRatePercent = NewRatePercent;
    OutMessage =
        std::wstring(GetTaxPolicyDisplayName(Type)) +
        L" " +
        std::to_wstring(PreviousRatePercent) +
        L"% -> " +
        std::to_wstring(NewRatePercent) +
        L"%";
    return true;
}

ETaxPolicyEventType EconomySystem::GetRequiredTaxPolicyEventForEdict(
    EGovernmentEdictType Type)
{
    switch (Type)
    {
    case EGovernmentEdictType::LaborTaxRelief:
        return ETaxPolicyEventType::WorkerTaxStrike;
    case EGovernmentEdictType::PropertyTaxRelief:
        return ETaxPolicyEventType::PropertyTaxBacklash;
    case EGovernmentEdictType::EmergencyAusterity:
        return ETaxPolicyEventType::BudgetCrisis;
    default:
        return ETaxPolicyEventType::None;
    }
}

const wchar_t* EconomySystem::GetTaxPolicyEventTitle(ETaxPolicyEventType Type)
{
    switch (Type)
    {
    case ETaxPolicyEventType::WorkerTaxStrike:
        return L"자본주의자·지식인 조세 시위";
    case ETaxPolicyEventType::PropertyTaxBacklash:
        return L"보수주의자·자본주의자 재산권 반발";
    case ETaxPolicyEventType::BudgetCrisis:
        return L"보수주의자·공산주의자 재정 압박";
    default:
        return L"정치 경고";
    }
}

void EconomySystem::ResolveTaxPolicyEvent(
    FTaxPolicyEventStatus& InOutTaxEventStatus,
    bool Success)
{
    ResolveTaxPolicyEventState(InOutTaxEventStatus, Success);
}

void EconomySystem::ApplyDailyTaxPolicyEventEffects(
    CWorld* World,
    const FTaxPolicyEventStatus& TaxEventStatus)
{
    if (!World ||
        !TaxEventStatus.Active ||
        TaxEventStatus.Type == ETaxPolicyEventType::None)
    {
        return;
    }

    const float Escalation =
        1.0f +
        (std::min)(1.35f,
            static_cast<float>(TaxEventStatus.DaysActive) / 4.0f);
    const float ControlBreakdown =
        1.0f +
        (std::min)(0.80f,
            static_cast<float>(TaxEventStatus.DaysActive) / 6.0f);
    float FoodDelta = 0.f;
    float HealthDelta = 0.f;
    float FunDelta = 0.f;
    float FaithDelta = 0.f;
    float HousingDelta = 0.f;
    float JobDelta = 0.f;
    float FreedomDelta = 0.f;
    float SecurityDelta = 0.f;

    switch (TaxEventStatus.Type)
    {
    case ETaxPolicyEventType::WorkerTaxStrike:
        JobDelta = -0.80f * Escalation;
        FreedomDelta = -0.55f * ControlBreakdown;
        SecurityDelta = -0.25f * ControlBreakdown;
        FunDelta = -0.16f * Escalation;
        break;
    case ETaxPolicyEventType::PropertyTaxBacklash:
        HousingDelta = -0.85f * Escalation;
        FreedomDelta = -0.45f * ControlBreakdown;
        SecurityDelta = -0.20f * ControlBreakdown;
        HealthDelta = -0.10f * Escalation;
        break;
    case ETaxPolicyEventType::BudgetCrisis:
        FoodDelta = -0.70f * Escalation;
        JobDelta = -0.55f * Escalation;
        SecurityDelta = -0.40f * ControlBreakdown;
        HealthDelta = -0.18f * Escalation;
        FunDelta = -0.18f * Escalation;
        break;
    default:
        return;
    }

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

    if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        return;

    for (size_t i = 0; i < OrbList.size(); ++i)
    {
        auto Orb = OrbList[i].lock();

        if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
            continue;

        Orb->ApplySatisfactionDelta(
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

void EconomySystem::TickTaxPolicyEvents(
    const FPoliticalWorldSnapshot& Snapshot,
    const FGovernmentProfile& GovernmentProfile,
    int SimulationYear,
    int SimulationMonth,
    int SimulationDay,
    long long& InOutNationalBudget,
    long long& InOutLastDailyNetChange,
    int& InOutWorkerTaxPressureDays,
    int& InOutPropertyTaxPressureDays,
    int& InOutBudgetCrisisPressureDays,
    FTaxPolicyEventStatus& InOutTaxEventStatus)
{
    if (InOutTaxEventStatus.NotificationDays > 0)
        --InOutTaxEventStatus.NotificationDays;

    if (!InOutTaxEventStatus.Active && InOutTaxEventStatus.CooldownDays > 0)
        --InOutTaxEventStatus.CooldownDays;

    const int ActiveCitizenCount =
        (std::max)(0, Snapshot.ActiveCitizenCount);
    const double SupportPercent =
        ActiveCitizenCount > 0 ?
        static_cast<double>(Snapshot.IncumbentCount) /
            static_cast<double>(ActiveCitizenCount) * 100.0 :
        50.0;
    const float WorkerTaxBurden = GetCitizenTaxBurdenNormalized(
        GovernmentProfile.TaxPolicy,
        true,
        false);
    const float ResidentTaxBurden = GetCitizenTaxBurdenNormalized(
        GovernmentProfile.TaxPolicy,
        false,
        true);
    const float OverallTaxBurden = GetCitizenTaxBurdenNormalized(
        GovernmentProfile.TaxPolicy,
        true,
        true);
    const bool WorkerPressure =
        WorkerTaxBurden >= 0.45f ||
        (WorkerTaxBurden >= 0.30f && SupportPercent <= 48.0);
    const bool PropertyPressure =
        ResidentTaxBurden >= 0.50f ||
        (ResidentTaxBurden >= 0.35f && SupportPercent <= 50.0);
    const bool BudgetPressure =
        OverallTaxBurden <= -0.35f &&
        InOutLastDailyNetChange <= -5000;

    if (WorkerPressure)
        ++InOutWorkerTaxPressureDays;
    else
        InOutWorkerTaxPressureDays =
            (std::max)(0, InOutWorkerTaxPressureDays - 2);

    if (PropertyPressure)
        ++InOutPropertyTaxPressureDays;
    else
        InOutPropertyTaxPressureDays =
            (std::max)(0, InOutPropertyTaxPressureDays - 2);

    if (BudgetPressure)
        ++InOutBudgetCrisisPressureDays;
    else
        InOutBudgetCrisisPressureDays =
            (std::max)(0, InOutBudgetCrisisPressureDays - 2);

    if (InOutTaxEventStatus.Active)
    {
        ++InOutTaxEventStatus.DaysActive;
        InOutTaxEventStatus.Summary = BuildTaxPolicyEventWarningSummary(
            InOutTaxEventStatus.Type,
            InOutTaxEventStatus.DaysActive);

        if (InOutTaxEventStatus.RemainingDays > 0)
            --InOutTaxEventStatus.RemainingDays;

        const bool CanResolveEarly = InOutTaxEventStatus.DaysActive >= 3;

        switch (InOutTaxEventStatus.Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            if (CanResolveEarly && !WorkerPressure)
            {
                ResolveTaxPolicyEventState(InOutTaxEventStatus, true);
                return;
            }
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            if (CanResolveEarly && !PropertyPressure)
            {
                ResolveTaxPolicyEventState(InOutTaxEventStatus, true);
                return;
            }
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            if (CanResolveEarly && !BudgetPressure)
            {
                ResolveTaxPolicyEventState(InOutTaxEventStatus, true);
                return;
            }
            break;
        default:
            break;
        }

        if (InOutTaxEventStatus.RemainingDays <= 0)
        {
            ResolveTaxPolicyEventState(InOutTaxEventStatus, false);
            return;
        }

        long long DailyEventPenalty = 0;
        const float Escalation =
            1.0f +
            (std::min)(1.50f,
                static_cast<float>(InOutTaxEventStatus.DaysActive) / 4.0f);

        switch (InOutTaxEventStatus.Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            DailyEventPenalty = static_cast<long long>(std::llround(
                900.0 + 450.0 * static_cast<double>(Escalation)));
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            DailyEventPenalty = static_cast<long long>(std::llround(
                750.0 + 360.0 * static_cast<double>(Escalation)));
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            DailyEventPenalty = static_cast<long long>(std::llround(
                1400.0 + 650.0 * static_cast<double>(Escalation)));
            break;
        default:
            break;
        }

        if (DailyEventPenalty > 0)
        {
            InOutNationalBudget -= DailyEventPenalty;
            InOutLastDailyNetChange -= DailyEventPenalty;
        }

        return;
    }

    if (InOutTaxEventStatus.CooldownDays > 0)
        return;

    if (InOutBudgetCrisisPressureDays >= 4)
    {
        StartTaxPolicyEvent(
            InOutTaxEventStatus,
            ETaxPolicyEventType::BudgetCrisis,
            SimulationYear,
            SimulationMonth,
            SimulationDay,
            -6000,
            InOutNationalBudget,
            InOutLastDailyNetChange,
            InOutWorkerTaxPressureDays,
            InOutPropertyTaxPressureDays,
            InOutBudgetCrisisPressureDays);
        InOutBudgetCrisisPressureDays = 0;
        return;
    }

    if (InOutWorkerTaxPressureDays >= 5 &&
        InOutWorkerTaxPressureDays >= InOutPropertyTaxPressureDays)
    {
        StartTaxPolicyEvent(
            InOutTaxEventStatus,
            ETaxPolicyEventType::WorkerTaxStrike,
            SimulationYear,
            SimulationMonth,
            SimulationDay,
            -4000,
            InOutNationalBudget,
            InOutLastDailyNetChange,
            InOutWorkerTaxPressureDays,
            InOutPropertyTaxPressureDays,
            InOutBudgetCrisisPressureDays);
        InOutWorkerTaxPressureDays = 0;
        return;
    }

    if (InOutPropertyTaxPressureDays >= 5)
    {
        StartTaxPolicyEvent(
            InOutTaxEventStatus,
            ETaxPolicyEventType::PropertyTaxBacklash,
            SimulationYear,
            SimulationMonth,
            SimulationDay,
            -3500,
            InOutNationalBudget,
            InOutLastDailyNetChange,
            InOutWorkerTaxPressureDays,
            InOutPropertyTaxPressureDays,
            InOutBudgetCrisisPressureDays);
        InOutPropertyTaxPressureDays = 0;
    }
}
