#include "AlmanacDataProvider.h"
#include "AlmanacQueryService.h"
#include "UIStrings.h"
#include "../Politics/EdictSystem.h"
#include "../World/WorldStatsSnapshot.h"
#include <cassert>
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <string>
#include <vector>

namespace
{
    double Clamp01(double Value)
    {
        return (std::max)(0.0, (std::min)(1.0, Value));
    }

    std::wstring FormatFixed1(double Value)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%.1f", Value);
        return Buffer;
    }

    void AppendLine(std::wstring& Body, const std::wstring& Line)
    {
        Body += Line;
        Body += L"\n";
    }

    std::wstring MakeLabeledValue(
        const wchar_t* LabelKey,
        const std::wstring& Value)
    {
        return UIStrings::Get(LabelKey) + L": " + Value;
    }

    std::wstring MakeCountLine(
        const wchar_t* LabelKey,
        int Count)
    {
        return MakeLabeledValue(
            LabelKey,
            std::to_wstring(Count) +
                UIStrings::Get(L"almanac.unit.person_suffix"));
    }

    void ApplyWorldStatsSnapshot(
        const WorldStats::FWorldStatsSnapshot& WorldSnapshot,
        AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
        Snapshot.ActiveCitizenCount = WorldSnapshot.ActiveCitizenCount;
        Snapshot.ActiveTouristCount = WorldSnapshot.ActiveTouristCount;
        Snapshot.ActiveHouseholdCount = WorldSnapshot.ActiveHouseholdCount;
        Snapshot.HomelessCount = WorldSnapshot.HomelessCount;
        Snapshot.HomelessHouseholdCount =
            WorldSnapshot.HomelessHouseholdCount;
        Snapshot.UnemployedCount = WorldSnapshot.UnemployedCount;
        Snapshot.AssignedHomeCount = WorldSnapshot.AssignedHomeCount;
        Snapshot.AssignedJobCount = WorldSnapshot.AssignedJobCount;
        Snapshot.ResidentialCapacity = WorldSnapshot.ResidentialCapacity;
        Snapshot.JobCapacity = WorldSnapshot.JobCapacity;
        Snapshot.TotalBuildingCount = WorldSnapshot.TotalBuildingCount;
        Snapshot.TourismBuildingCount = WorldSnapshot.TourismBuildingCount;
        Snapshot.FoodProviderCount = WorldSnapshot.FoodProviderCount;
        Snapshot.EntertainmentBuildingCount =
            WorldSnapshot.EntertainmentBuildingCount;
        Snapshot.FaithBuildingCount = WorldSnapshot.FaithBuildingCount;
        Snapshot.ResidentialVacancyBuildingCount =
            WorldSnapshot.ResidentialVacancyBuildingCount;
        Snapshot.WorkVacancyBuildingCount =
            WorldSnapshot.WorkVacancyBuildingCount;
        Snapshot.FreedomInfluenceBuildingCount =
            WorldSnapshot.FreedomInfluenceBuildingCount;
        Snapshot.SecurityInfluenceBuildingCount =
            WorldSnapshot.SecurityInfluenceBuildingCount;
        Snapshot.HarborCount = WorldSnapshot.HarborCount;
        Snapshot.TourismVisitCapacity = WorldSnapshot.TourismVisitCapacity;
        Snapshot.TourismVisitOccupancy = WorldSnapshot.TourismVisitOccupancy;
        Snapshot.TouristPreferenceMatchedCount =
            WorldSnapshot.TouristPreferenceMatchedCount;
        Snapshot.TotalProducedPowerMW = WorldSnapshot.TotalProducedPowerMW;
        Snapshot.TotalRequiredPowerMW = WorldSnapshot.TotalRequiredPowerMW;
        Snapshot.DisconnectedConsumerCount =
            WorldSnapshot.DisconnectedConsumerCount;
        Snapshot.AnyNeutralAxisCitizenCount =
            WorldSnapshot.AnyNeutralAxisCitizenCount;
        Snapshot.FullyNeutralCitizenCount =
            WorldSnapshot.FullyNeutralCitizenCount;
        for (int WealthIndex = 0; WealthIndex < 3; ++WealthIndex)
        {
            Snapshot.HomelessWealthCount[WealthIndex] =
                WorldSnapshot.HomelessWealthCount[WealthIndex];
            Snapshot.CitizenWealthCount[WealthIndex] =
                WorldSnapshot.CitizenWealthCount[WealthIndex];
        }
        for (int PreferenceIndex = 0;
            PreferenceIndex < GTouristPreferenceCount;
            ++PreferenceIndex)
        {
            Snapshot.TouristProfileCount[PreferenceIndex] =
                WorldSnapshot.TouristProfileCount[PreferenceIndex];
        }
        for (int TierIndex = 0; TierIndex < 5; ++TierIndex)
        {
            Snapshot.OverallSatisfactionCitizenCount[TierIndex] =
                WorldSnapshot.OverallSatisfactionCitizenCount[TierIndex];
        }
        for (int EducationIndex = 0; EducationIndex < 3; ++EducationIndex)
        {
            Snapshot.EducationCount[EducationIndex] =
                WorldSnapshot.EducationCount[EducationIndex];
            Snapshot.UnemployedEducationCount[EducationIndex] =
                WorldSnapshot.UnemployedEducationCount[EducationIndex];
            Snapshot.WorkVacancyEducationCount[EducationIndex] =
                WorldSnapshot.WorkVacancyEducationCount[EducationIndex];
        }
        for (int WealthIndex = 0; WealthIndex < 5; ++WealthIndex)
        {
            Snapshot.ResidentialVacancyWealthCount[WealthIndex] =
                WorldSnapshot.ResidentialVacancyWealthCount[WealthIndex];
        }
        for (int ResourceIndex = 0;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const auto& WorldResource =
                WorldSnapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)];
            auto& Resource =
                Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)];
            Resource.Type = WorldResource.Type;
            Resource.TotalStock = WorldResource.TotalStock;
            Resource.AvailableStock = WorldResource.AvailableStock;
            Resource.ReservedPickup = WorldResource.ReservedPickup;
            Resource.ReservedIncoming = WorldResource.ReservedIncoming;
            Resource.Capacity = WorldResource.Capacity;
            Resource.AvailableIncomingCapacity =
                WorldResource.AvailableIncomingCapacity;
            Resource.ShortagePressure = WorldResource.ShortagePressure;
            Resource.ProducerAvailableStock =
                WorldResource.ProducerAvailableStock;
            Resource.WarehouseBufferedStock =
                WorldResource.WarehouseBufferedStock;
            Resource.ConsumerCoveredStock =
                WorldResource.ConsumerCoveredStock;
            Resource.HarborExportableStock =
                WorldResource.HarborExportableStock;
            Resource.HarborReservedPickup =
                WorldResource.HarborReservedPickup;
            Resource.ProducerBuildingCount =
                WorldResource.ProducerBuildingCount;
            Resource.ConsumerBuildingCount =
                WorldResource.ConsumerBuildingCount;
            Resource.StorageBuildingCount =
                WorldResource.StorageBuildingCount;
            Resource.WarehouseBuildingCount =
                WorldResource.WarehouseBuildingCount;
            Resource.HarborBuildingCount =
                WorldResource.HarborBuildingCount;
            Resource.TopStockBuildings = WorldResource.TopStockBuildings;
            Resource.TopProducerBuildings =
                WorldResource.TopProducerBuildings;
            Resource.TopWarehouseBuildings =
                WorldResource.TopWarehouseBuildings;
            Resource.TopConsumerBuildings =
                WorldResource.TopConsumerBuildings;
            Resource.TopHarborBuildings =
                WorldResource.TopHarborBuildings;
            Resource.TopShortageBuildings =
                WorldResource.TopShortageBuildings;
            Resource.TopReservedBuildings =
                WorldResource.TopReservedBuildings;
            Resource.TopOverflowBuildings =
                WorldResource.TopOverflowBuildings;
        }
        for (int CategoryIndex = 0;
            CategoryIndex < BuildingCategoryInfo::GBuildingCategoryCount;
            ++CategoryIndex)
        {
            const auto& WorldCategory =
                WorldSnapshot.BuildingCategories[
                    static_cast<size_t>(CategoryIndex)];
            auto& Category =
                Snapshot.BuildingCategories[
                    static_cast<size_t>(CategoryIndex)];
            Category.Category = WorldCategory.Category;
            Category.Count = WorldCategory.Count;
            Category.TopBuildings = WorldCategory.TopBuildings;
        }
        Snapshot.MonthlyWageCost = WorldSnapshot.MonthlyWageCost;
        Snapshot.MonthlyUpkeepCost = WorldSnapshot.MonthlyUpkeepCost;
        Snapshot.TotalResourceStock = WorldSnapshot.TotalResourceStock;
        Snapshot.AverageFood = WorldSnapshot.AverageFood;
        Snapshot.AverageHealth = WorldSnapshot.AverageHealth;
        Snapshot.AverageFun = WorldSnapshot.AverageFun;
        Snapshot.AverageFaith = WorldSnapshot.AverageFaith;
        Snapshot.AverageHousing = WorldSnapshot.AverageHousing;
        Snapshot.AverageJob = WorldSnapshot.AverageJob;
        Snapshot.AverageFreedom = WorldSnapshot.AverageFreedom;
        Snapshot.AverageSecurity = WorldSnapshot.AverageSecurity;
        Snapshot.AverageResidentialFreedom =
            WorldSnapshot.AverageResidentialFreedom;
        Snapshot.AverageResidentialSecurity =
            WorldSnapshot.AverageResidentialSecurity;
        Snapshot.AverageResidentialPollution =
            WorldSnapshot.AverageResidentialPollution;
        Snapshot.AverageOverall = WorldSnapshot.AverageOverall;
        Snapshot.TopBuildings = WorldSnapshot.TopBuildings;
        Snapshot.TopResourceBuildings = WorldSnapshot.TopResourceBuildings;

        for (int AxisIndex = 0;
            AxisIndex < static_cast<int>(EPoliticalAxis::Count);
            ++AxisIndex)
        {
            for (int StanceIndex = 0; StanceIndex < 3; ++StanceIndex)
            {
                Snapshot.PoliticalCount[AxisIndex][StanceIndex] =
                    WorldSnapshot.PoliticalCount[AxisIndex][StanceIndex];
            }
        }

        for (int CategoryIndex = 0;
            CategoryIndex < BuildingCategoryInfo::GBuildingCategoryCount;
            ++CategoryIndex)
        {
            Snapshot.BuildingCategoryCount[CategoryIndex] =
                WorldSnapshot.BuildingCategoryCount[CategoryIndex];
        }
    }

    void ApplyMainWorldRecord(
        const AlmanacDataProvider::FAlmanacMainWorldRecord& MainWorldRecord,
        AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
        Snapshot.HasMainWorld = MainWorldRecord.Available;

        if (!MainWorldRecord.Available)
            return;

        Snapshot.NationalBudget = MainWorldRecord.NationalBudget;
        Snapshot.DailyExportIncome = MainWorldRecord.DailyExportIncome;
        Snapshot.DailyTaxIncome = MainWorldRecord.DailyTaxIncome;
        Snapshot.DailyConsumptionTaxIncome =
            MainWorldRecord.DailyConsumptionTaxIncome;
        Snapshot.DailyIncomeTaxIncome =
            MainWorldRecord.DailyIncomeTaxIncome;
        Snapshot.DailyPropertyTaxIncome =
            MainWorldRecord.DailyPropertyTaxIncome;
        Snapshot.DailyEdictCost = MainWorldRecord.DailyEdictCost;
        Snapshot.DailyImportExpense = MainWorldRecord.DailyImportExpense;
        Snapshot.DailyNetChange = MainWorldRecord.DailyNetChange;
        Snapshot.TaxCollectionEfficiency =
            MainWorldRecord.TaxCollectionEfficiency;
        Snapshot.PoliticalSnapshot = MainWorldRecord.PoliticalSnapshot;
        Snapshot.GovernmentProfile = MainWorldRecord.GovernmentProfile;
        Snapshot.ElectionStatus = MainWorldRecord.ElectionStatus;
        Snapshot.DaysUntilNextElection =
            MainWorldRecord.DaysUntilNextElection;
        Snapshot.ElectionWarningScore =
            MainWorldRecord.ElectionWarningScore;
        Snapshot.TaxEventStatus = MainWorldRecord.TaxEventStatus;
        Snapshot.WorldCrisisStatus = MainWorldRecord.WorldCrisisStatus;
        Snapshot.PoliticalDemandNotice = MainWorldRecord.PoliticalDemandNotice;
        Snapshot.FactionDemandStates = MainWorldRecord.FactionDemandStates;
        Snapshot.ForeignDemandStates = MainWorldRecord.ForeignDemandStates;
        Snapshot.ForeignPowerStates = MainWorldRecord.ForeignPowerStates;

        for (size_t i = 0; i < MainWorldRecord.GovernmentEdictStates.size(); ++i)
        {
            const FGovernmentEdictState& EdictState =
                MainWorldRecord.GovernmentEdictStates[i];

            if (EdictState.Active)
                ++Snapshot.ActiveEdictCount;

            if (EdictState.Type == EGovernmentEdictType::MartialLaw &&
                EdictState.Active)
            {
                Snapshot.MartialLawActive = true;
            }

            if (!EdictState.Active)
                continue;

            const FGovernmentEdictDefinition* Definition =
                EdictSystem::FindGovernmentEdictDefinition(
                    EdictState.Type);

            if (!Definition)
                continue;

            std::wstring Line = Definition->DisplayName;

            if (Definition->Mode == EGovernmentEdictMode::Active)
            {
                Line += UIStrings::Format(
                    L"almanac.edict.remaining_days",
                    { std::to_wstring(EdictState.RemainingDays) });
            }

            Snapshot.ActiveEdictLines.push_back(std::move(Line));
        }
    }

    void ValidateSnapshotForDebug(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
#ifndef NDEBUG
        auto IsPercent = [](double Value)
        {
            return std::isfinite(Value) &&
                Value >= -0.1 &&
                Value <= 100.1;
        };

        assert(Snapshot.ActiveCitizenCount >= 0);
        assert(Snapshot.HomelessCount >= 0);
        assert(Snapshot.UnemployedCount >= 0);
        assert(Snapshot.ActiveEdictCount >= 0);
        assert(Snapshot.TotalBuildingCount >= 0);
        assert(Snapshot.ResidentialCapacity >= 0);
        assert(Snapshot.JobCapacity >= 0);
        assert(IsPercent(Snapshot.SupportPercent));
        assert(IsPercent(Snapshot.OppositionPercent));
        assert(IsPercent(Snapshot.AbstainPercent));
        assert(IsPercent(Snapshot.RebelRiskScore));
        assert(IsPercent(Snapshot.AverageFood));
        assert(IsPercent(Snapshot.AverageHealth));
        assert(IsPercent(Snapshot.AverageFun));
        assert(IsPercent(Snapshot.AverageFaith));
        assert(IsPercent(Snapshot.AverageHousing));
        assert(IsPercent(Snapshot.AverageJob));
        assert(IsPercent(Snapshot.AverageFreedom));
        assert(IsPercent(Snapshot.AverageSecurity));
        assert(IsPercent(Snapshot.AverageOverall));

        for (const auto& Resource : Snapshot.ResourceTypes)
        {
            assert(Resource.TotalStock >= 0);
            assert(Resource.AvailableStock >= 0);
            assert(Resource.ReservedIncoming >= 0);
            assert(Resource.Capacity >= 0);
            assert(Resource.ProducerBuildingCount >= 0);
            assert(Resource.ConsumerBuildingCount >= 0);
            assert(Resource.StorageBuildingCount >= 0);
            assert(Resource.HarborBuildingCount >= 0);
        }

        for (const auto& Category : Snapshot.BuildingCategories)
        {
            assert(Category.Count >= 0);
        }
#else
        (void)Snapshot;
#endif
    }
}

namespace AlmanacDataProvider
{
    FAlmanacSnapshot BuildSnapshot(
        const std::shared_ptr<IAlmanacQuerySource>& QuerySource)
    {
        FAlmanacSnapshot Snapshot;

        if (!QuerySource)
        {
            ValidateSnapshotForDebug(Snapshot);
            return Snapshot;
        }

        ApplyMainWorldRecord(QuerySource->QueryMainWorldRecord(), Snapshot);

        WorldStats::FWorldStatsSnapshot WorldSnapshot;
        QuerySource->QueryWorldStats(WorldSnapshot);
        ApplyWorldStatsSnapshot(WorldSnapshot, Snapshot);

        if (Snapshot.PoliticalSnapshot.ActiveCitizenCount > 0)
        {
            const double Denominator =
                static_cast<double>(
                    Snapshot.PoliticalSnapshot.ActiveCitizenCount);

            Snapshot.SupportPercent =
                static_cast<double>(Snapshot.PoliticalSnapshot.IncumbentCount) /
                Denominator * 100.0;
            Snapshot.OppositionPercent =
                static_cast<double>(Snapshot.PoliticalSnapshot.OppositionCount) /
                Denominator * 100.0;
            Snapshot.AbstainPercent =
                static_cast<double>(Snapshot.PoliticalSnapshot.AbstainCount) /
                Denominator * 100.0;
        }

        const double FreedomPressure =
            1.0 - Clamp01(Snapshot.AverageFreedom / 100.0);
        const double SecurityVulnerability =
            1.0 - Clamp01(Snapshot.AverageSecurity / 100.0);
        const double ResidentialFreedomPressure =
            1.0 - Clamp01(Snapshot.AverageResidentialFreedom / 100.0);
        const double ResidentialSecurityPressure =
            1.0 - Clamp01(Snapshot.AverageResidentialSecurity / 100.0);
        const double ResidentialPollutionPressure =
            Clamp01(Snapshot.AverageResidentialPollution / 100.0);
        const double OppositionRatio =
            Clamp01(Snapshot.OppositionPercent / 100.0);
        const double FoodPressure =
            1.0 - Clamp01(Snapshot.AverageFood / 100.0);
        const double HealthPressure =
            1.0 - Clamp01(Snapshot.AverageHealth / 100.0);
        const double HousingPressure =
            1.0 - Clamp01(Snapshot.AverageHousing / 100.0);
        const double JobPressure =
            1.0 - Clamp01(Snapshot.AverageJob / 100.0);
        const double CitizenCount =
            static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount));
        const double HomelessRatio =
            static_cast<double>(Snapshot.HomelessCount) / CitizenCount;
        const double UnemploymentRatio =
            static_cast<double>(Snapshot.UnemployedCount) / CitizenCount;
        const double DailyOperatingCost =
            static_cast<double>(
                Snapshot.MonthlyWageCost + Snapshot.MonthlyUpkeepCost) / 30.0 +
            (std::max)(0.0, static_cast<double>(Snapshot.DailyEdictCost));
        const double FiscalStress =
            DailyOperatingCost > 0.0 && Snapshot.DailyNetChange < 0 ?
            Clamp01(
                static_cast<double>(-Snapshot.DailyNetChange) /
                DailyOperatingCost) :
            0.0;
        double TaxEventPressure = 0.0;
        double WorldCrisisPressure = 0.0;

        if (Snapshot.TaxEventStatus.Active)
        {
            const double DurationPressure = Clamp01(
                static_cast<double>(Snapshot.TaxEventStatus.DaysActive + 1) /
                6.0);

            switch (Snapshot.TaxEventStatus.Type)
            {
            case ETaxPolicyEventType::WorkerTaxStrike:
                TaxEventPressure = 0.12 + DurationPressure * 0.14;
                break;
            case ETaxPolicyEventType::PropertyTaxBacklash:
                TaxEventPressure = 0.10 + DurationPressure * 0.12;
                break;
            case ETaxPolicyEventType::BudgetCrisis:
                TaxEventPressure = 0.16 + DurationPressure * 0.16;
                break;
            default:
                break;
            }
        }

        if (Snapshot.WorldCrisisStatus.Active)
        {
            WorldCrisisPressure =
                Clamp01(
                    0.14 +
                    static_cast<double>(
                        Snapshot.WorldCrisisStatus.DaysActive + 1) * 0.03);
        }
        else if (Snapshot.WorldCrisisStatus.NotificationDays > 0 &&
            !Snapshot.WorldCrisisStatus.Summary.empty())
        {
            WorldCrisisPressure = 0.05;
        }

        const double MaterialPressure =
            Clamp01(
                FoodPressure * 0.22 +
                HealthPressure * 0.18 +
                HousingPressure * 0.14 +
                JobPressure * 0.12 +
                HomelessRatio * 0.10 +
                UnemploymentRatio * 0.12 +
                FiscalStress * 0.12 +
                ResidentialPollutionPressure * 0.10);

        Snapshot.RebelRiskScore =
            Clamp01(
                FreedomPressure * 0.18 +
                ResidentialFreedomPressure * 0.08 +
                SecurityVulnerability * 0.10 +
                ResidentialSecurityPressure * 0.08 +
                OppositionRatio * 0.24 +
                MaterialPressure * 0.24 +
                TaxEventPressure * 0.10 +
                WorldCrisisPressure * 0.08) * 100.0;

        if (Snapshot.MartialLawActive)
        {
            Snapshot.RebelRiskScore =
                (std::max)(0.0, Snapshot.RebelRiskScore - 8.0);
        }

        if (Snapshot.RebelRiskScore >= 66.0)
            Snapshot.RebelRiskLabel = UIStrings::Get(L"almanac.rebel_risk.high");
        else if (Snapshot.RebelRiskScore >= 33.0)
            Snapshot.RebelRiskLabel = UIStrings::Get(L"almanac.rebel_risk.medium");
        else
            Snapshot.RebelRiskLabel = UIStrings::Get(L"almanac.rebel_risk.low");

        ValidateSnapshotForDebug(Snapshot);
        return Snapshot;
    }

    FAlmanacSnapshot BuildSnapshot(
        const std::shared_ptr<CWorld>& World)
    {
        return BuildSnapshot(
            AlmanacQueryService::CreateWorldQuerySource(World));
    }

    std::wstring BuildYearbookSummaryText(
        const FAlmanacSnapshot& Snapshot)
    {
        std::wstring Body;

        if (Snapshot.ActiveCitizenCount > 0)
        {
            if (Snapshot.WorldCrisisStatus.Active)
            {
                AppendLine(
                    Body,
                    L"진행 중 위기: " + Snapshot.WorldCrisisStatus.Title);
            }
            else if (Snapshot.WorldCrisisStatus.NotificationDays > 0 &&
                !Snapshot.WorldCrisisStatus.Summary.empty())
            {
                AppendLine(
                    Body,
                    L"최근 위기: " + Snapshot.WorldCrisisStatus.Summary);
            }

            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.overall_satisfaction",
                    FormatFixed1(Snapshot.AverageOverall) + L" / 100"));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.food",
                    FormatFixed1(Snapshot.AverageFood)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.health",
                    FormatFixed1(Snapshot.AverageHealth)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.fun",
                    FormatFixed1(Snapshot.AverageFun)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.faith",
                    FormatFixed1(Snapshot.AverageFaith)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.housing",
                    FormatFixed1(Snapshot.AverageHousing)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.job",
                    FormatFixed1(Snapshot.AverageJob)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.freedom",
                    FormatFixed1(Snapshot.AverageFreedom)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.security",
                    FormatFixed1(Snapshot.AverageSecurity)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.residential_freedom",
                    FormatFixed1(Snapshot.AverageResidentialFreedom)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.residential_security",
                    FormatFixed1(Snapshot.AverageResidentialSecurity)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.residential_pollution",
                    FormatFixed1(Snapshot.AverageResidentialPollution)));
            AppendLine(
                Body,
                MakeCountLine(
                    L"almanac.yearbook.homeless_count",
                    Snapshot.HomelessCount));
            AppendLine(
                Body,
                MakeCountLine(
                    L"almanac.yearbook.unemployed_count",
                    Snapshot.UnemployedCount));
        }
        else
        {
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.overall_satisfaction",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.food",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.health",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.fun",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.faith",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.housing",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.job",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.freedom",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.security",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.residential_freedom",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.residential_security",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.satisfaction.residential_pollution",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeCountLine(L"almanac.yearbook.homeless_count", 0));
            AppendLine(
                Body,
                MakeCountLine(L"almanac.yearbook.unemployed_count", 0));
        }

        Body += L"\n";
        AppendLine(
            Body,
            UIStrings::Get(L"almanac.yearbook.section.government_assessment"));

        if (Snapshot.PoliticalSnapshot.ActiveCitizenCount > 0)
        {
            AppendLine(
                Body,
                MakeCountLine(
                    L"almanac.yearbook.incumbent_support",
                    Snapshot.PoliticalSnapshot.IncumbentCount));
            AppendLine(
                Body,
                MakeCountLine(
                    L"almanac.yearbook.opposition_support",
                    Snapshot.PoliticalSnapshot.OppositionCount));
            AppendLine(
                Body,
                MakeCountLine(
                    L"almanac.yearbook.abstain_support",
                    Snapshot.PoliticalSnapshot.AbstainCount));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.average_support_score",
                    FormatFixed1(Snapshot.PoliticalSnapshot.AverageSupportScore) +
                        L" / 100"));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.life_score",
                    FormatFixed1(Snapshot.PoliticalSnapshot.AverageLifeScore)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.ideology_alignment",
                    FormatFixed1(
                        Snapshot.PoliticalSnapshot.
                            AverageGovernmentIdeologyScore)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.building_preference_effect",
                    FormatFixed1(Snapshot.PoliticalSnapshot.AverageBuildingScore)));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.recent_action_effect",
                    FormatFixed1(Snapshot.PoliticalSnapshot.AverageActionScore)));
        }
        else
        {
            AppendLine(
                Body,
                MakeCountLine(L"almanac.yearbook.incumbent_support", 0));
            AppendLine(
                Body,
                MakeCountLine(L"almanac.yearbook.opposition_support", 0));
            AppendLine(
                Body,
                MakeCountLine(L"almanac.yearbook.abstain_support", 0));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.average_support_score",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.life_score",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.ideology_alignment",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.building_preference_effect",
                    UIStrings::Get(L"almanac.value.empty")));
            AppendLine(
                Body,
                MakeLabeledValue(
                    L"almanac.yearbook.recent_action_effect",
                    UIStrings::Get(L"almanac.value.empty")));
        }

        Body += L"\n";
        AppendLine(
            Body,
            UIStrings::Get(L"almanac.yearbook.section.government_line"));

        for (int AxisIndex = 0;
            AxisIndex < static_cast<int>(EPoliticalAxis::Count);
            ++AxisIndex)
        {
            const EPoliticalAxis Axis =
                static_cast<EPoliticalAxis>(AxisIndex);
            const FNpcPoliticalChoice& Choice =
                Snapshot.GovernmentProfile.Ideology.Get(Axis);

            AppendLine(
                Body,
                std::wstring(GetPoliticalAxisDisplayName(Axis)) + L": " +
                GetPoliticalFactionDisplayName(Axis, Choice.Stance) +
                L" (" + GetPoliticalSupportDisplayName(Choice.Support) + L")");
        }

        Body += L"\n";
        AppendLine(
            Body,
            UIStrings::Get(L"almanac.yearbook.section.active_edicts"));

        if (Snapshot.ActiveEdictLines.empty())
        {
            Body += L"- ";
            AppendLine(Body, UIStrings::Get(L"almanac.yearbook.none"));
        }
        else
        {
            for (size_t i = 0; i < Snapshot.ActiveEdictLines.size(); ++i)
            {
                Body += L"- ";
                AppendLine(Body, Snapshot.ActiveEdictLines[i]);
            }
        }

        Body += L"\n";
        AppendLine(
            Body,
            UIStrings::Get(L"almanac.yearbook.section.political_counts"));

        for (int AxisIndex = 0;
            AxisIndex < static_cast<int>(EPoliticalAxis::Count);
            ++AxisIndex)
        {
            const EPoliticalAxis Axis =
                static_cast<EPoliticalAxis>(AxisIndex);

            Body += std::wstring(GetPoliticalAxisDisplayName(Axis));
            Body += L": ";
            Body += GetPoliticalFactionDisplayName(
                Axis, EPoliticalStance::Left);
            Body += L" ";
            Body += std::to_wstring(
                Snapshot.PoliticalCount[AxisIndex][
                    static_cast<int>(EPoliticalStance::Left)]);
            Body += UIStrings::Get(L"almanac.yearbook.people_separator");
            Body += GetPoliticalFactionDisplayName(
                Axis, EPoliticalStance::Neutral);
            Body += L" ";
            Body += std::to_wstring(
                Snapshot.PoliticalCount[AxisIndex][
                    static_cast<int>(EPoliticalStance::Neutral)]);
            Body += UIStrings::Get(L"almanac.yearbook.people_separator");
            Body += GetPoliticalFactionDisplayName(
                Axis, EPoliticalStance::Right);
            Body += L" ";
            Body += std::to_wstring(
                Snapshot.PoliticalCount[AxisIndex][
                    static_cast<int>(EPoliticalStance::Right)]);
            Body += UIStrings::Get(L"almanac.yearbook.people_suffix_newline");
        }

        return Body;
    }
}
