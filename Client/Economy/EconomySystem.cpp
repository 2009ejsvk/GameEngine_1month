#include "EconomySystem.h"
#include "World/World.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../World/MainWorld.h"
#include <cmath>
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
}

EconomySystem::FDailyResult EconomySystem::ApplyDailySettlement(
    CWorld* World,
    int DaysInMonth,
    const FGovernmentProfile& GovernmentProfile)
{
    FDailyResult Result;
    const CMainWorld* MainWorld =
        World ? dynamic_cast<CMainWorld*>(World) : nullptr;
    const FTaxPolicyEventStatus* TaxEventStatus =
        MainWorld ? &MainWorld->GetTaxPolicyEventStatus() : nullptr;
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

            const int ExportStock = Building->GetResourceStock();
            const int EffectiveExportStock = static_cast<int>(std::floor(
                static_cast<double>(ExportStock) *
                EventEffects.ExportMultiplier));

            if (EffectiveExportStock > 0 &&
                Building->TryConsumeResource(EffectiveExportStock))
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
