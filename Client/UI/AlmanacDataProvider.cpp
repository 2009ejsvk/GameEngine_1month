#include "AlmanacDataProvider.h"
#include "../Politics/EdictSystem.h"
#include "../World/WorldStatsSnapshot.h"
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

    void ApplyWorldStatsSnapshot(
        const WorldStats::FWorldStatsSnapshot& WorldSnapshot,
        AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
        Snapshot.ActiveCitizenCount = WorldSnapshot.ActiveCitizenCount;
        Snapshot.HomelessCount = WorldSnapshot.HomelessCount;
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
        Snapshot.HarborCount = WorldSnapshot.HarborCount;
        Snapshot.AnyNeutralAxisCitizenCount =
            WorldSnapshot.AnyNeutralAxisCitizenCount;
        Snapshot.FullyNeutralCitizenCount =
            WorldSnapshot.FullyNeutralCitizenCount;
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
            CategoryIndex < AlmanacDataProvider::GBuildingCategoryCount;
            ++CategoryIndex)
        {
            Snapshot.BuildingCategoryCount[CategoryIndex] =
                WorldSnapshot.BuildingCategoryCount[CategoryIndex];
        }
    }
}

namespace AlmanacDataProvider
{
    FAlmanacSnapshot BuildSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::shared_ptr<IMainWorldAlmanacAccess>& MainWorld)
    {
        FAlmanacSnapshot Snapshot;

        if (!World)
            return Snapshot;

        Snapshot.HasMainWorld = MainWorld != nullptr;

        if (MainWorld)
        {
            Snapshot.NationalBudget = MainWorld->GetNationalBudget();
            Snapshot.DailyExportIncome = MainWorld->GetLastDailyExportIncome();
            Snapshot.DailyTaxIncome = MainWorld->GetLastDailyTaxIncome();
            Snapshot.DailyConsumptionTaxIncome =
                MainWorld->GetLastDailyConsumptionTaxIncome();
            Snapshot.DailyIncomeTaxIncome =
                MainWorld->GetLastDailyIncomeTaxIncome();
            Snapshot.DailyPropertyTaxIncome =
                MainWorld->GetLastDailyPropertyTaxIncome();
            Snapshot.DailyEdictCost = MainWorld->GetLastDailyEdictCost();
            Snapshot.DailyNetChange = MainWorld->GetLastDailyNetChange();
            Snapshot.TaxCollectionEfficiency =
                MainWorld->GetLastDailyTaxCollectionEfficiency();
            Snapshot.PoliticalSnapshot = MainWorld->GetPoliticalSnapshot();
            Snapshot.GovernmentProfile = MainWorld->GetGovernmentProfile();
            Snapshot.ElectionStatus = MainWorld->GetElectionStatus();
            Snapshot.DaysUntilNextElection =
                MainWorld->GetDaysUntilNextElection();
            Snapshot.ElectionWarningScore =
                MainWorld->GetElectionWarningScore();
            Snapshot.TaxEventStatus = MainWorld->GetTaxPolicyEventStatus();

            const auto& EdictStates = MainWorld->GetGovernmentEdictStates();

            for (size_t i = 0; i < EdictStates.size(); ++i)
            {
                if (EdictStates[i].Active)
                    ++Snapshot.ActiveEdictCount;

                if (EdictStates[i].Type == EGovernmentEdictType::MartialLaw &&
                    EdictStates[i].Active)
                {
                    Snapshot.MartialLawActive = true;
                }

                if (!EdictStates[i].Active)
                    continue;

                const FGovernmentEdictDefinition* Definition =
                    EdictSystem::FindGovernmentEdictDefinition(
                        EdictStates[i].Type);

                if (!Definition)
                    continue;

                std::wstring Line = Definition->DisplayName;

                if (Definition->Mode == EGovernmentEdictMode::Active)
                {
                    Line += L" (";
                    Line += std::to_wstring(EdictStates[i].RemainingDays);
                    Line += L"일 남음)";
                }

                Snapshot.ActiveEdictLines.push_back(std::move(Line));
            }
        }

        const WorldStats::FWorldStatsSnapshot WorldSnapshot =
            WorldStats::BuildSnapshot(World);
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

        const double MaterialPressure =
            Clamp01(
                FoodPressure * 0.22 +
                HealthPressure * 0.18 +
                HousingPressure * 0.14 +
                JobPressure * 0.12 +
                HomelessRatio * 0.10 +
                UnemploymentRatio * 0.12 +
                FiscalStress * 0.12);

        Snapshot.RebelRiskScore =
            Clamp01(
                FreedomPressure * 0.24 +
                SecurityVulnerability * 0.14 +
                OppositionRatio * 0.24 +
                MaterialPressure * 0.28 +
                TaxEventPressure * 0.10) * 100.0;

        if (Snapshot.MartialLawActive)
        {
            Snapshot.RebelRiskScore =
                (std::max)(0.0, Snapshot.RebelRiskScore - 8.0);
        }

        if (Snapshot.RebelRiskScore >= 66.0)
            Snapshot.RebelRiskLabel = L"높음";
        else if (Snapshot.RebelRiskScore >= 33.0)
            Snapshot.RebelRiskLabel = L"중간";
        else
            Snapshot.RebelRiskLabel = L"낮음";

        return Snapshot;
    }

    std::wstring BuildYearbookSummaryText(
        const FAlmanacSnapshot& Snapshot)
    {
        std::wstring Body;

        if (Snapshot.ActiveCitizenCount > 0)
        {
            AppendLine(
                Body,
                L"종합 만족도: " + FormatFixed1(Snapshot.AverageOverall) +
                L" / 100");
            AppendLine(Body, L"음식: " + FormatFixed1(Snapshot.AverageFood));
            AppendLine(Body, L"보건: " + FormatFixed1(Snapshot.AverageHealth));
            AppendLine(Body, L"유흥: " + FormatFixed1(Snapshot.AverageFun));
            AppendLine(Body, L"신앙: " + FormatFixed1(Snapshot.AverageFaith));
            AppendLine(Body, L"주거: " + FormatFixed1(Snapshot.AverageHousing));
            AppendLine(Body, L"직업: " + FormatFixed1(Snapshot.AverageJob));
            AppendLine(Body, L"자유: " + FormatFixed1(Snapshot.AverageFreedom));
            AppendLine(
                Body,
                L"치안: " + FormatFixed1(Snapshot.AverageSecurity));
            AppendLine(
                Body,
                L"무주택자 수: " + std::to_wstring(Snapshot.HomelessCount) +
                L"명");
            AppendLine(
                Body,
                L"실업자 수: " + std::to_wstring(Snapshot.UnemployedCount) +
                L"명");
        }
        else
        {
            Body +=
                L"종합 만족도: -\n"
                L"음식: -\n"
                L"보건: -\n"
                L"유흥: -\n"
                L"신앙: -\n"
                L"주거: -\n"
                L"직업: -\n"
                L"자유: -\n"
                L"치안: -\n"
                L"무주택자 수: 0명\n"
                L"실업자 수: 0명\n";
        }

        Body += L"\n정권 평가\n";

        if (Snapshot.PoliticalSnapshot.ActiveCitizenCount > 0)
        {
            AppendLine(
                Body,
                L"현 정권 지지: " +
                std::to_wstring(Snapshot.PoliticalSnapshot.IncumbentCount) +
                L"명");
            AppendLine(
                Body,
                L"야권 지지: " +
                std::to_wstring(Snapshot.PoliticalSnapshot.OppositionCount) +
                L"명");
            AppendLine(
                Body,
                L"기권/부동층: " +
                std::to_wstring(Snapshot.PoliticalSnapshot.AbstainCount) +
                L"명");
            AppendLine(
                Body,
                L"평균 지지 점수: " +
                FormatFixed1(Snapshot.PoliticalSnapshot.AverageSupportScore) +
                L" / 100");
            AppendLine(
                Body,
                L"생활 평가: " +
                FormatFixed1(Snapshot.PoliticalSnapshot.AverageLifeScore));
            AppendLine(
                Body,
                L"정부 이념 일치: " +
                FormatFixed1(
                    Snapshot.PoliticalSnapshot.
                        AverageGovernmentIdeologyScore));
            AppendLine(
                Body,
                L"건물 선호 효과: " +
                FormatFixed1(Snapshot.PoliticalSnapshot.AverageBuildingScore));
            AppendLine(
                Body,
                L"최근 행동 효과: " +
                FormatFixed1(Snapshot.PoliticalSnapshot.AverageActionScore));
        }
        else
        {
            Body +=
                L"현 정권 지지: 0명\n"
                L"야권 지지: 0명\n"
                L"기권/부동층: 0명\n"
                L"평균 지지 점수: -\n"
                L"생활 평가: -\n"
                L"정부 이념 일치: -\n"
                L"건물 선호 효과: -\n"
                L"최근 행동 효과: -\n";
        }

        Body += L"\n정부 노선\n";

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

        Body += L"\n활성 칙령\n";

        if (Snapshot.ActiveEdictLines.empty())
        {
            Body += L"- 없음\n";
        }
        else
        {
            for (size_t i = 0; i < Snapshot.ActiveEdictLines.size(); ++i)
            {
                Body += L"- ";
                AppendLine(Body, Snapshot.ActiveEdictLines[i]);
            }
        }

        Body += L"\n정치 성향 인원\n";

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
            Body += L"명 / ";
            Body += GetPoliticalFactionDisplayName(
                Axis, EPoliticalStance::Neutral);
            Body += L" ";
            Body += std::to_wstring(
                Snapshot.PoliticalCount[AxisIndex][
                    static_cast<int>(EPoliticalStance::Neutral)]);
            Body += L"명 / ";
            Body += GetPoliticalFactionDisplayName(
                Axis, EPoliticalStance::Right);
            Body += L" ";
            Body += std::to_wstring(
                Snapshot.PoliticalCount[AxisIndex][
                    static_cast<int>(EPoliticalStance::Right)]);
            Body += L"명\n";
        }

        return Body;
    }
}
