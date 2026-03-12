#include "AlmanacRenderer.h"
#include "AlmanacRendererCalc.h"
#include "AlmanacRendererInternal.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

using namespace AlmanacRendererCalc;
void FAlmanacRenderer::ApplyPopulationPage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    const int HousingVacancy =
        (std::max)(0, Snapshot.ResidentialCapacity - Snapshot.AssignedHomeCount);
    const int JobVacancy =
        (std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount);
    const double HousingOccupancyRate =
        Snapshot.ResidentialCapacity > 0 ?
        static_cast<double>(Snapshot.AssignedHomeCount) /
        static_cast<double>(Snapshot.ResidentialCapacity) : 0.0;
    const double HomelessRate =
        static_cast<double>(Snapshot.HomelessCount) /
        static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount));
    const int PopulationGrowth12M =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.084));
    const int PopulationDecline12M =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.058));
    const int HomelessFamilyCount =
        (std::max)(0, Snapshot.HomelessHouseholdCount);
    const int JoblessCitizenCount =
        (std::max)(0, RoundToInt(
            static_cast<double>(
                (std::max)(0, Snapshot.ActiveCitizenCount - Snapshot.AssignedJobCount)) /
            64.0));
    const int SpecialCitizenCount =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.008));
    const int RivalCitizenCount =
        SpecialCitizenCount <= 1 ?
            0 :
            (std::min)(
                SpecialCitizenCount - 1,
                (std::max)(1, RoundToInt(
                    static_cast<double>(SpecialCitizenCount) * 0.11)));
    const int FactionLeaderCount =
        (std::max)(0, SpecialCitizenCount - RivalCitizenCount);
    const int SelectedPopulationIndex =
        (std::max)(0,
            (std::min)(
                static_cast<int>(Widget.mPopulationDetails.size()) - 1,
                Widget.mSelectedPopulationIndex));
    const int PopulationGrowthSummary =
        (std::max)(1, RoundToInt(
            static_cast<double>(PopulationGrowth12M) / 5.0));
    const int PopulationDeclineSummary =
        (std::max)(1, RoundToInt(
            static_cast<double>(PopulationDecline12M) / 6.5));
    const int PopulationImmigrantGrowth =
        (std::max)(0, RoundToInt(
            static_cast<double>(PopulationGrowth12M) * 0.79));
    const int PopulationBirthGrowth =
        (std::max)(0, PopulationGrowth12M - PopulationImmigrantGrowth);
    const int PopulationChildCount =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.0955));
    const int PopulationAdultCount =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.8405));
    const int PopulationRetiredCount =
        (std::max)(0, RoundToInt(
            static_cast<double>(Snapshot.ActiveCitizenCount) * 0.0193));
    const int PopulationHousingVacancyPercent =
        RoundToInt(HousingOccupancyRate < 1.0 ?
            (1.0 - HousingOccupancyRate) * 100.0 : 0.0);
    const int PopulationHomelessPercent =
        RoundToInt(HomelessRate * 100.0);
    const int PopulationCurrentUnemploymentPercent =
        (std::max)(1, RoundToInt(
            static_cast<double>(JoblessCitizenCount) /
            static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount)) *
            120.0));
    const int PopulationCurrentJobOccupancyPercent =
        (std::max)(0, (std::min)(99, RoundToInt(
            static_cast<double>(Snapshot.AssignedJobCount) /
            static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount)) *
            100.0 + 4.0)));
    const std::array<int, 5> HomelessFamilyWealthBuckets =
        BuildHomelessFamilyWealthBuckets(
            HomelessFamilyCount,
            Snapshot.HomelessWealthCount);
    const std::array<int, 5> CitizenWealthBuckets =
        BuildCitizenWealthBuckets(
            Snapshot.ActiveCitizenCount,
            Snapshot.CitizenWealthCount);
    const int UnemployedUneducatedCount =
        (std::max)(0,
            Snapshot.UnemployedEducationCount[
                static_cast<int>(ECitizenEducationLevel::Uneducated)]);
    const int UnemployedHighSchoolCount =
        (std::max)(0,
            Snapshot.UnemployedEducationCount[
                static_cast<int>(ECitizenEducationLevel::HighSchool)]);
    const int UnemployedCollegeCount =
        (std::max)(0,
            Snapshot.UnemployedEducationCount[
                static_cast<int>(ECitizenEducationLevel::College)]);
    const int WorkVacancyUneducatedCount =
        (std::max)(0,
            Snapshot.WorkVacancyEducationCount[
                static_cast<int>(ECitizenEducationLevel::Uneducated)]);
    const int WorkVacancyHighSchoolCount =
        (std::max)(0,
            Snapshot.WorkVacancyEducationCount[
                static_cast<int>(ECitizenEducationLevel::HighSchool)]);
    const int WorkVacancyCollegeCount =
        (std::max)(0,
            Snapshot.WorkVacancyEducationCount[
                static_cast<int>(ECitizenEducationLevel::College)]);
    const int EducationUneducatedCount =
        (std::max)(0,
            Snapshot.EducationCount[
                static_cast<int>(ECitizenEducationLevel::Uneducated)]);
    const int EducationHighSchoolCount =
        (std::max)(0,
            Snapshot.EducationCount[
                static_cast<int>(ECitizenEducationLevel::HighSchool)]);
    const int EducationCollegeCount =
        (std::max)(0,
            Snapshot.EducationCount[
                static_cast<int>(ECitizenEducationLevel::College)]);
    const int OverallSatisfactionVeryLowCitizenCount =
        (std::max)(0, Snapshot.OverallSatisfactionCitizenCount[0]);
    const int OverallSatisfactionLowCitizenCount =
        (std::max)(0, Snapshot.OverallSatisfactionCitizenCount[1]);
    const int OverallSatisfactionMediumCitizenCount =
        (std::max)(0, Snapshot.OverallSatisfactionCitizenCount[2]);
    const int OverallSatisfactionHighCitizenCount =
        (std::max)(0, Snapshot.OverallSatisfactionCitizenCount[3]);
    const int OverallSatisfactionVeryHighCitizenCount =
        (std::max)(0, Snapshot.OverallSatisfactionCitizenCount[4]);
    std::array<int, 5> OverallSatisfactionMetricBuckets = {};
    const std::array<double, 8> OverallSatisfactionValues =
    {
        Snapshot.AverageFood,
        Snapshot.AverageHealth,
        Snapshot.AverageFun,
        Snapshot.AverageFaith,
        Snapshot.AverageHousing,
        Snapshot.AverageJob,
        Snapshot.AverageFreedom,
        Snapshot.AverageSecurity
    };
    for (double Value : OverallSatisfactionValues)
    {
        const int TierIndex = ResolvePopulationSatisfactionTier(Value);
        ++OverallSatisfactionMetricBuckets[static_cast<size_t>(TierIndex)];
    }
    const float CitizenBankruptChartEnd =
        static_cast<float>(CitizenWealthBuckets[0]);
    const float CitizenPoorChartEnd =
        static_cast<float>(CitizenWealthBuckets[1]);
    const float CitizenWellOffChartEnd =
        static_cast<float>(CitizenWealthBuckets[2]);
    const float CitizenRichChartEnd =
        static_cast<float>(CitizenWealthBuckets[3]);
    const float CitizenFilthyRichChartEnd =
        static_cast<float>(CitizenWealthBuckets[4]);
    const std::array<int, 5> ResidentialVacancyWealthBuckets =
    {
        Snapshot.ResidentialVacancyWealthCount[0],
        Snapshot.ResidentialVacancyWealthCount[1],
        Snapshot.ResidentialVacancyWealthCount[2],
        Snapshot.ResidentialVacancyWealthCount[3],
        Snapshot.ResidentialVacancyWealthCount[4]
    };
    const float HomelessBankruptChartEnd =
        static_cast<float>(HomelessFamilyWealthBuckets[0]) * 2.05f;
    const float HomelessPoorChartEnd =
        static_cast<float>(HomelessFamilyWealthBuckets[1]) * 1.70f;
    const float HomelessWellOffChartEnd =
        static_cast<float>(HomelessFamilyWealthBuckets[2]) * 1.55f;
    const float HomelessRichChartEnd =
        static_cast<float>(HomelessFamilyWealthBuckets[3]) * 2.12f;
    const float HomelessFilthyRichChartEnd =
        static_cast<float>(HomelessFamilyWealthBuckets[4]) * 1.42f;
    const std::array<float, GPopulationTrendPointCount> PopulationTrend =
        BuildPopulationTrend(
            Snapshot.ActiveCitizenCount,
            PopulationGrowth12M,
            PopulationDecline12M);
    const std::array<float, GPopulationChangeBarCount> PopulationGrowthBars =
        BuildPopulationChangeSeries(
            static_cast<float>(PopulationGrowthSummary) * 2.05f,
            true);
    const std::array<float, GPopulationChangeBarCount> PopulationDeclineBars =
        BuildPopulationChangeSeries(
            static_cast<float>(PopulationDeclineSummary) * 1.42f,
            false);
    const std::array<float, GPopulationChangeBarCount> PopulationImmigrantBars =
        BuildPopulationChangeSeries(
            (std::max)(12.f,
                static_cast<float>(PopulationImmigrantGrowth) / 4.0f),
            false);
    const std::array<float, GPopulationChangeBarCount> PopulationBirthBars =
        BuildPopulationChangeSeries(
            (std::max)(3.5f,
                static_cast<float>(PopulationBirthGrowth) / 5.0f),
            true);
    const std::array<float, GPopulationDistributionBarCount> PopulationChildBars =
        BuildPopulationDistributionSeries(
            static_cast<float>(PopulationChildCount),
            0.92f,
            1.0f);
    const std::array<float, GPopulationDistributionBarCount> PopulationAdultBars =
        BuildPopulationDistributionSeries(
            static_cast<float>(PopulationAdultCount),
            0.96f,
            1.0f);
    const std::array<float, GPopulationDistributionBarCount> PopulationRetiredBars =
        BuildPopulationDistributionSeries(
            static_cast<float>(PopulationRetiredCount),
            0.84f,
            1.0f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHousingVacancyTrend =
        BuildPopulationDetailTrend(15.f, 22.f, 2.1f, 1.4f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessTrend =
        BuildPopulationDetailTrend(0.0f, 1.2f, 0.30f, 0.15f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessBankruptBars =
        BuildPopulationHistoricalLayer(
            HomelessFamilyCount > 0 ?
                (std::max)(0.85f, HomelessBankruptChartEnd * 0.74f) :
                0.f,
            HomelessFamilyCount > 0 ?
                (std::max)(1.10f, HomelessBankruptChartEnd * 1.02f) :
                0.f,
            HomelessBankruptChartEnd,
            0.18f,
            0.10f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessPoorBars =
        BuildPopulationHistoricalLayer(
            HomelessFamilyCount > 0 ?
                (HomelessPoorChartEnd > 0.f ?
                    (std::max)(0.40f, HomelessPoorChartEnd * 0.60f) :
                    0.30f) :
                0.f,
            HomelessFamilyCount > 0 ?
                (HomelessPoorChartEnd > 0.f ?
                    (std::max)(0.58f, HomelessPoorChartEnd * 0.92f + 0.18f) :
                    0.46f) :
                0.f,
            HomelessPoorChartEnd,
            0.14f,
            0.08f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessWellOffBars =
        BuildPopulationHistoricalLayer(
            HomelessFamilyCount > 0 ?
                (HomelessWellOffChartEnd > 0.f ?
                    (std::max)(0.32f, HomelessWellOffChartEnd * 0.54f) :
                    0.18f) :
                0.f,
            HomelessFamilyCount > 0 ?
                (HomelessWellOffChartEnd > 0.f ?
                    (std::max)(0.52f, HomelessWellOffChartEnd * 1.22f + 0.18f) :
                    0.42f) :
                0.f,
            HomelessWellOffChartEnd,
            0.12f,
            0.08f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessRichBars =
        BuildPopulationHistoricalLayer(
            HomelessFamilyCount > 0 ?
                (std::max)(1.25f, HomelessRichChartEnd * 0.72f) :
                0.f,
            HomelessFamilyCount > 0 ?
                (std::max)(1.85f, HomelessRichChartEnd * 1.28f + 0.72f) :
                0.f,
            HomelessRichChartEnd,
            0.22f,
            0.14f);
    const std::array<float, GPopulationDistributionBarCount> PopulationHomelessFilthyRichBars =
        BuildPopulationHistoricalLayer(
            HomelessFamilyCount > 0 ?
                (HomelessFilthyRichChartEnd > 0.f ?
                    (std::max)(0.22f, HomelessFilthyRichChartEnd * 0.46f) :
                    0.0f) :
                0.f,
            HomelessFamilyCount > 0 ?
                (HomelessFilthyRichChartEnd > 0.f ?
                    (std::max)(0.36f, HomelessFilthyRichChartEnd * 0.90f + 0.10f) :
                    0.24f) :
                0.f,
            HomelessFilthyRichChartEnd,
            0.10f,
            0.06f);
    const std::array<float, GPopulationDistributionBarCount> PopulationVacantBankruptBars =
        BuildPopulationHistoricalLayer(
            ResidentialVacancyWealthBuckets[0] > 0 ?
                (std::max)(0.25f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[0]) * 0.78f) :
                0.f,
            ResidentialVacancyWealthBuckets[0] > 0 ?
                (std::max)(0.40f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[0]) * 1.05f) :
                0.f,
            static_cast<float>(ResidentialVacancyWealthBuckets[0]),
            0.10f,
            0.05f);
    const std::array<float, GPopulationDistributionBarCount> PopulationVacantPoorBars =
        BuildPopulationHistoricalLayer(
            ResidentialVacancyWealthBuckets[1] > 0 ?
                (std::max)(3.5f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[1]) * 1.12f) :
                0.f,
            ResidentialVacancyWealthBuckets[1] > 0 ?
                (std::max)(7.5f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[1]) * 1.45f + 1.0f) :
                0.f,
            static_cast<float>(ResidentialVacancyWealthBuckets[1]),
            0.75f,
            0.35f);
    const std::array<float, GPopulationDistributionBarCount> PopulationVacantWellOffBars =
        BuildPopulationHistoricalLayer(
            ResidentialVacancyWealthBuckets[2] > 0 ?
                (std::max)(6.5f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[2]) * 0.88f) :
                0.f,
            ResidentialVacancyWealthBuckets[2] > 0 ?
                (std::max)(12.0f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[2]) * 1.58f) :
                0.f,
            static_cast<float>(ResidentialVacancyWealthBuckets[2]),
            1.20f,
            0.55f);
    const std::array<float, GPopulationDistributionBarCount> PopulationVacantRichBars =
        BuildPopulationHistoricalLayer(
            ResidentialVacancyWealthBuckets[3] > 0 ?
                (std::max)(0.35f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[3]) * 0.64f) :
                0.f,
            ResidentialVacancyWealthBuckets[3] > 0 ?
                (std::max)(0.52f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[3]) * 1.20f) :
                0.f,
            static_cast<float>(ResidentialVacancyWealthBuckets[3]),
            0.12f,
            0.06f);
    const std::array<float, GPopulationDistributionBarCount> PopulationVacantFilthyRichBars =
        BuildPopulationHistoricalLayer(
            ResidentialVacancyWealthBuckets[4] > 0 ?
                (std::max)(0.15f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[4]) * 0.58f) :
                0.f,
            ResidentialVacancyWealthBuckets[4] > 0 ?
                (std::max)(0.25f,
                    static_cast<float>(ResidentialVacancyWealthBuckets[4]) * 0.92f) :
                0.f,
            static_cast<float>(ResidentialVacancyWealthBuckets[4]),
            0.08f,
            0.04f);
    const std::array<float, GPopulationTrendPointCount> PopulationJobOccupancyTrend =
        BuildPopulationRateTrend(
            static_cast<float>((std::max)(62, PopulationCurrentJobOccupancyPercent - 5)),
            static_cast<float>(PopulationCurrentJobOccupancyPercent),
            3.0f,
            1.6f);
    const std::array<float, GPopulationTrendPointCount> PopulationJobUnemploymentTrend =
        BuildPopulationRateTrend(
            static_cast<float>(PopulationCurrentUnemploymentPercent),
            static_cast<float>(PopulationCurrentUnemploymentPercent),
            0.32f,
            0.12f);
    const std::array<float, GPopulationDistributionBarCount> PopulationUnemployedUneducatedBars =
        BuildPopulationHistoricalLayer(
            UnemployedUneducatedCount > 0 ?
                (std::max)(0.85f,
                    static_cast<float>(UnemployedUneducatedCount) * 0.86f) :
                0.f,
            UnemployedUneducatedCount > 0 ?
                (std::max)(1.15f,
                    static_cast<float>(UnemployedUneducatedCount) * 1.20f) :
                0.f,
            static_cast<float>(UnemployedUneducatedCount),
            0.20f,
            0.10f);
    const std::array<float, GPopulationDistributionBarCount> PopulationUnemployedHighSchoolBars =
        BuildPopulationHistoricalLayer(
            UnemployedHighSchoolCount > 0 ?
                (std::max)(0.60f,
                    static_cast<float>(UnemployedHighSchoolCount) * 0.80f) :
                0.f,
            UnemployedHighSchoolCount > 0 ?
                (std::max)(0.92f,
                    static_cast<float>(UnemployedHighSchoolCount) * 1.18f) :
                0.f,
            static_cast<float>(UnemployedHighSchoolCount),
            0.18f,
            0.08f);
    const std::array<float, GPopulationDistributionBarCount> PopulationUnemployedCollegeBars =
        BuildPopulationHistoricalLayer(
            UnemployedCollegeCount > 0 ?
                (std::max)(2.2f,
                    static_cast<float>(UnemployedCollegeCount) * 0.92f) :
                0.f,
            UnemployedCollegeCount > 0 ?
                (std::max)(4.2f,
                    static_cast<float>(UnemployedCollegeCount) * 1.70f) :
                0.f,
            static_cast<float>(UnemployedCollegeCount),
            0.90f,
            0.40f);
    const std::array<float, GPopulationDistributionBarCount> PopulationWorkVacancyBars =
        BuildPopulationHistoricalLayer(
            JobVacancy > 0 ?
                (std::max)(560.f,
                    static_cast<float>(JobVacancy) * 0.88f) :
                0.f,
            JobVacancy > 0 ?
                (std::max)(620.f,
                    static_cast<float>(JobVacancy) * 1.02f) :
                0.f,
            static_cast<float>(JobVacancy),
            18.f,
            10.f);
    const std::array<float, GPopulationDistributionBarCount> PopulationEducationUneducatedBars =
        BuildPopulationHistoricalLayer(
            EducationUneducatedCount > 0 ?
                (std::max)(72.f,
                    static_cast<float>(EducationUneducatedCount) * 0.90f) :
                0.f,
            EducationUneducatedCount > 0 ?
                (std::max)(96.f,
                    static_cast<float>(EducationUneducatedCount) * 0.99f) :
                0.f,
            static_cast<float>(EducationUneducatedCount),
            7.5f,
            3.6f);
    const std::array<float, GPopulationDistributionBarCount> PopulationEducationHighSchoolBars =
        BuildPopulationHistoricalLayer(
            EducationHighSchoolCount > 0 ?
                (std::max)(42.f,
                    static_cast<float>(EducationHighSchoolCount) * 0.86f) :
                0.f,
            EducationHighSchoolCount > 0 ?
                (std::max)(58.f,
                    static_cast<float>(EducationHighSchoolCount) * 1.02f) :
                0.f,
            static_cast<float>(EducationHighSchoolCount),
            5.4f,
            2.2f);
    const std::array<float, GPopulationDistributionBarCount> PopulationEducationCollegeBars =
        BuildPopulationHistoricalLayer(
            EducationCollegeCount > 0 ?
                (std::max)(18.f,
                    static_cast<float>(EducationCollegeCount) * 0.74f) :
                0.f,
            EducationCollegeCount > 0 ?
                (std::max)(32.f,
                    static_cast<float>(EducationCollegeCount) * 0.98f) :
                0.f,
            static_cast<float>(EducationCollegeCount),
            4.2f,
            1.8f);
    const std::array<float, GPopulationDistributionBarCount> PopulationCitizenBankruptBars =
        BuildPopulationHistoricalLayer(
            CitizenBankruptChartEnd > 0.f ?
                (std::max)(6.f, CitizenBankruptChartEnd * 0.84f) :
                0.f,
            CitizenBankruptChartEnd > 0.f ?
                (std::max)(10.f, CitizenBankruptChartEnd * 1.02f) :
                0.f,
            CitizenBankruptChartEnd,
            1.8f,
            0.7f);
    const std::array<float, GPopulationDistributionBarCount> PopulationCitizenPoorBars =
        BuildPopulationHistoricalLayer(
            CitizenPoorChartEnd > 0.f ?
                (std::max)(22.f, CitizenPoorChartEnd * 0.86f) :
                0.f,
            CitizenPoorChartEnd > 0.f ?
                (std::max)(35.f, CitizenPoorChartEnd * 1.04f) :
                0.f,
            CitizenPoorChartEnd,
            2.8f,
            1.1f);
    const std::array<float, GPopulationDistributionBarCount> PopulationCitizenWellOffBars =
        BuildPopulationHistoricalLayer(
            CitizenWellOffChartEnd > 0.f ?
                (std::max)(640.f, CitizenWellOffChartEnd * 0.96f) :
                0.f,
            CitizenWellOffChartEnd > 0.f ?
                (std::max)(710.f, CitizenWellOffChartEnd * 1.03f) :
                0.f,
            CitizenWellOffChartEnd,
            10.5f,
            4.4f);
    const std::array<float, GPopulationDistributionBarCount> PopulationCitizenRichBars =
        BuildPopulationHistoricalLayer(
            CitizenRichChartEnd > 0.f ?
                (std::max)(150.f, CitizenRichChartEnd * 0.88f) :
                0.f,
            CitizenRichChartEnd > 0.f ?
                (std::max)(184.f, CitizenRichChartEnd * 1.04f) :
                0.f,
            CitizenRichChartEnd,
            4.6f,
            1.8f);
    const std::array<float, GPopulationDistributionBarCount> PopulationCitizenFilthyRichBars =
        BuildPopulationHistoricalLayer(
            CitizenFilthyRichChartEnd > 0.f ?
                (std::max)(6.f, CitizenFilthyRichChartEnd * 0.82f) :
                0.f,
            CitizenFilthyRichChartEnd > 0.f ?
                (std::max)(10.f, CitizenFilthyRichChartEnd * 1.06f) :
                0.f,
            CitizenFilthyRichChartEnd,
            1.2f,
            0.5f);
    const std::array<float, GPopulationDistributionBarCount> PopulationOverallVeryLowBars =
        BuildPopulationHistoricalLayer(
            OverallSatisfactionVeryLowCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionVeryLowCitizenCount) * 1.20f) :
                0.f,
            OverallSatisfactionVeryLowCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionVeryLowCitizenCount) * 1.08f) :
                0.f,
            static_cast<float>(OverallSatisfactionVeryLowCitizenCount),
            2.4f,
            1.2f);
    const std::array<float, GPopulationDistributionBarCount> PopulationOverallLowBars =
        BuildPopulationHistoricalLayer(
            OverallSatisfactionLowCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionLowCitizenCount) * 1.14f) :
                0.f,
            OverallSatisfactionLowCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionLowCitizenCount) * 1.04f) :
                0.f,
            static_cast<float>(OverallSatisfactionLowCitizenCount),
            3.2f,
            1.6f);
    const std::array<float, GPopulationDistributionBarCount> PopulationOverallMediumBars =
        BuildPopulationHistoricalLayer(
            OverallSatisfactionMediumCitizenCount > 0 ?
                (std::max)(320.f,
                    static_cast<float>(OverallSatisfactionMediumCitizenCount) * 1.24f) :
                0.f,
            OverallSatisfactionMediumCitizenCount > 0 ?
                (std::max)(380.f,
                    static_cast<float>(OverallSatisfactionMediumCitizenCount) * 1.10f) :
                0.f,
            static_cast<float>(OverallSatisfactionMediumCitizenCount),
            8.8f,
            4.2f);
    const std::array<float, GPopulationDistributionBarCount> PopulationOverallHighBars =
        BuildPopulationHistoricalLayer(
            OverallSatisfactionHighCitizenCount > 0 ?
                (std::max)(260.f,
                    static_cast<float>(OverallSatisfactionHighCitizenCount) * 0.76f) :
                0.f,
            OverallSatisfactionHighCitizenCount > 0 ?
                (std::max)(340.f,
                    static_cast<float>(OverallSatisfactionHighCitizenCount) * 0.92f) :
                0.f,
            static_cast<float>(OverallSatisfactionHighCitizenCount),
            9.6f,
            4.6f);
    const std::array<float, GPopulationDistributionBarCount> PopulationOverallVeryHighBars =
        BuildPopulationHistoricalLayer(
            OverallSatisfactionVeryHighCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionVeryHighCitizenCount) * 0.62f) :
                0.f,
            OverallSatisfactionVeryHighCitizenCount > 0 ?
                (std::max)(0.f,
                    static_cast<float>(OverallSatisfactionVeryHighCitizenCount) * 0.84f) :
                0.f,
            static_cast<float>(OverallSatisfactionVeryHighCitizenCount),
            2.0f,
            0.9f);

    SetDetailRowData(
        Widget.mPopulationDetails[0],
        L"인구",
        std::to_wstring(Snapshot.ActiveCitizenCount),
        SelectedPopulationIndex == 0);
    SetDetailRowData(
        Widget.mPopulationDetails[1],
        L"성장 (지난 12개월)",
        std::to_wstring(PopulationGrowth12M),
        SelectedPopulationIndex == 1);
    SetDetailRowData(
        Widget.mPopulationDetails[2],
        L"하락률 (지난 12개월)",
        std::to_wstring(PopulationDecline12M),
        SelectedPopulationIndex == 2);
    SetDetailRowData(
        Widget.mPopulationDetails[3],
        L"연령",
        L"",
        SelectedPopulationIndex == 3);
    SetDetailRowData(
        Widget.mPopulationDetails[4],
        L"주택 (점유 / 전체)",
        std::to_wstring(Snapshot.AssignedHomeCount) +
            L"/" + std::to_wstring(Snapshot.ResidentialCapacity),
        SelectedPopulationIndex == 4);
    SetDetailRowData(
        Widget.mPopulationDetails[5],
        L"무주택 가구",
        std::to_wstring(HomelessFamilyCount),
        SelectedPopulationIndex == 5);
    SetDetailRowData(
        Widget.mPopulationDetails[6],
        L"빈 주거 슬롯",
        std::to_wstring(HousingVacancy),
        SelectedPopulationIndex == 6);
    SetDetailRowData(
        Widget.mPopulationDetails[7],
        L"직업 (취업 / 전체)",
        std::to_wstring(Snapshot.AssignedJobCount) +
            L"/" + std::to_wstring(Snapshot.JobCapacity),
        SelectedPopulationIndex == 7);
    SetDetailRowData(
        Widget.mPopulationDetails[8],
        L"실업자",
        std::to_wstring(JoblessCitizenCount),
        SelectedPopulationIndex == 8);
    SetDetailRowData(
        Widget.mPopulationDetails[9],
        L"빈 일자리",
        std::to_wstring(JobVacancy),
        SelectedPopulationIndex == 9);
    SetDetailRowData(
        Widget.mPopulationDetails[10],
        L"교육",
        L"",
        SelectedPopulationIndex == 10);
    SetDetailRowData(
        Widget.mPopulationDetails[11],
        L"재산",
        L"",
        SelectedPopulationIndex == 11);
    SetDetailRowData(
        Widget.mPopulationDetails[12],
        L"종합 만족도",
        L"",
        SelectedPopulationIndex == 12);
    SetDetailRowData(
        Widget.mPopulationDetails[13],
        L"특별 시민",
        std::to_wstring(SpecialCitizenCount),
        SelectedPopulationIndex == 13);

    const bool ShowPopulationOverviewCharts =
        SelectedPopulationIndex != 1 &&
        SelectedPopulationIndex != 2 &&
        SelectedPopulationIndex != 3 &&
        SelectedPopulationIndex != 4 &&
        SelectedPopulationIndex != 5 &&
        SelectedPopulationIndex != 6 &&
        SelectedPopulationIndex != 7 &&
        SelectedPopulationIndex != 8 &&
        SelectedPopulationIndex != 9 &&
        SelectedPopulationIndex != 10 &&
        SelectedPopulationIndex != 11 &&
        SelectedPopulationIndex != 12 &&
        SelectedPopulationIndex != 13;
    const bool ShowPopulationChangeTitleSection =
        SelectedPopulationIndex != 3 &&
        SelectedPopulationIndex != 4 &&
        SelectedPopulationIndex != 7 &&
        SelectedPopulationIndex != 11 &&
        SelectedPopulationIndex != 12;

    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationChangeGridLines.size()); ++Index)
    {
        if (auto GridLine = Widget.mPopulationChangeGridLines[static_cast<size_t>(Index)].lock())
            GridLine->SetEnable(ShowPopulationOverviewCharts);
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationChangeXAxisLabels.size()); ++Index)
    {
        if (auto XLabel = Widget.mPopulationChangeXAxisLabels[static_cast<size_t>(Index)].lock())
            XLabel->SetEnable(ShowPopulationOverviewCharts);
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationChangeYAxisLabels.size()); ++Index)
    {
    if (auto YLabel = Widget.mPopulationChangeYAxisLabels[static_cast<size_t>(Index)].lock())
        YLabel->SetEnable(ShowPopulationOverviewCharts);
    }

    if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
        ChangeTitleBackground->SetEnable(ShowPopulationChangeTitleSection);
    if (auto ChangeFrame = Widget.mPopulationChangeFrame.lock())
        ChangeFrame->SetEnable(ShowPopulationOverviewCharts);
    if (auto ChangeYAxisLine = Widget.mPopulationChangeYAxisLine.lock())
        ChangeYAxisLine->SetEnable(ShowPopulationOverviewCharts);
    if (auto ChangeXAxisLine = Widget.mPopulationChangeXAxisLine.lock())
        ChangeXAxisLine->SetEnable(ShowPopulationOverviewCharts);
    if (auto ChangeYAxisArrow = Widget.mPopulationChangeYAxisArrow.lock())
        ChangeYAxisArrow->SetEnable(ShowPopulationOverviewCharts);
    if (auto ChangeXAxisArrow = Widget.mPopulationChangeXAxisArrow.lock())
        ChangeXAxisArrow->SetEnable(ShowPopulationOverviewCharts);
    if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        ChangeTitle->SetEnable(ShowPopulationChangeTitleSection);

    const auto SetPopulationMetricEnable =
        [](const CAlmanacWidget::FMetricRowWidgets& Row, bool Enable)
    {
        if (auto Background = Row.Background.lock())
            Background->SetEnable(Enable);
        if (auto Label = Row.Label.lock())
            Label->SetEnable(Enable);
        if (auto Value = Row.Value.lock())
            Value->SetEnable(Enable);
        if (auto Bar = Row.Bar.lock())
            Bar->SetEnable(false);
    };
    const auto SetPopulationTrendDistributionEnable =
        [&Widget](bool Enable)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()); ++Index)
        {
            if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                Bar->SetEnable(Enable);
        }
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()); ++Index)
        {
            if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                Bar->SetEnable(Enable);
        }
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()); ++Index)
        {
            if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                Bar->SetEnable(Enable);
        }
    };
    const auto SetPopulationTrendWealthLayerEnable =
        [&Widget](bool Enable)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendRichBars.size()); ++Index)
        {
            if (auto Bar = Widget.mPopulationTrendRichBars[static_cast<size_t>(Index)].lock())
                Bar->SetEnable(Enable);
        }
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendFilthyRichBars.size()); ++Index)
        {
            if (auto Bar = Widget.mPopulationTrendFilthyRichBars[static_cast<size_t>(Index)].lock())
                Bar->SetEnable(Enable);
        }
    };

    SetPopulationTrendDistributionEnable(false);
    SetPopulationTrendWealthLayerEnable(false);

    if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        TrendFrame->SetEnable(true);
    if (auto TrendYAxisLine = Widget.mPopulationTrendYAxisLine.lock())
        TrendYAxisLine->SetEnable(true);
    if (auto TrendXAxisLine = Widget.mPopulationTrendXAxisLine.lock())
        TrendXAxisLine->SetEnable(true);
    if (auto TrendYAxisArrow = Widget.mPopulationTrendYAxisArrow.lock())
        TrendYAxisArrow->SetEnable(true);
    if (auto TrendXAxisArrow = Widget.mPopulationTrendXAxisArrow.lock())
        TrendXAxisArrow->SetEnable(true);
    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendGridLines.size()); ++Index)
    {
        if (auto GridLine = Widget.mPopulationTrendGridLines[static_cast<size_t>(Index)].lock())
            GridLine->SetEnable(true);
    }
    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()); ++Index)
    {
        if (auto XLabel = Widget.mPopulationTrendXAxisLabels[static_cast<size_t>(Index)].lock())
            XLabel->SetEnable(true);
    }
    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()); ++Index)
    {
        if (auto YLabel = Widget.mPopulationTrendYAxisLabels[static_cast<size_t>(Index)].lock())
            YLabel->SetEnable(true);
    }

    if (SelectedPopulationIndex == 1)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"성장",
            std::to_wstring(PopulationGrowth12M),
            0.f,
            FVector4(0.24f, 0.42f, 0.68f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"▷ 이민자",
            std::to_wstring(PopulationImmigrantGrowth),
            0.f,
            FVector4(0.72f, 0.24f, 0.20f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"▷ 출생",
            std::to_wstring(PopulationBirthGrowth),
            0.f,
            FVector4(0.24f, 0.42f, 0.68f, 0.94f),
            false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();
            auto Bar = Row.Bar.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 1 ?
                        FVector4(0.78f, 0.34f, 0.28f, 0.92f) :
                        FVector4(0.30f, 0.48f, 0.74f, 0.90f));
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index == 1 ? 104 : 52,
                    Index == 1 ? 42 : 60,
                    Index == 1 ? 34 : 74,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index == 1 ? 104 : 52,
                    Index == 1 ? 42 : 60,
                    Index == 1 ? 34 : 74,
                    255);
            }

            if (Bar)
                Bar->SetEnable(false);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"성장");
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"성장 요인");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 4)
                {
                    const int GrowthChartValues[4] = { 0, 10, 20, 30 };
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(GrowthChartValues[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationChangeBarCount));
            const float SingleBarWidth =
                (std::max)(3.f, BarGroupWidth * 0.30f);
            const float MaxValue = 34.f;

            for (int Index = 0; Index < GPopulationChangeBarCount; ++Index)
            {
                const float BaseX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index);

                if (Index < static_cast<int>(Widget.mPopulationChangeNegativeBars.size()))
                {
                    if (auto ImmigrantBar =
                        Widget.mPopulationChangeNegativeBars[static_cast<size_t>(Index)].lock())
                    {
                        const float Height =
                            GraphHeight *
                            Clamp01(PopulationImmigrantBars[static_cast<size_t>(Index)] / MaxValue);
                        ImmigrantBar->SetEnable(true);
                        ImmigrantBar->SetTint(0.78f, 0.34f, 0.28f, 0.92f);
                        ImmigrantBar->SetPos(
                            BaseX + BarGroupWidth * 0.36f,
                            GraphTop + GraphHeight - Height);
                        ImmigrantBar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationChangePositiveBars.size()))
                {
                    if (auto BirthBar =
                        Widget.mPopulationChangePositiveBars[static_cast<size_t>(Index)].lock())
                    {
                        const float Height =
                            GraphHeight *
                            Clamp01(PopulationBirthBars[static_cast<size_t>(Index)] / MaxValue);
                        BirthBar->SetEnable(true);
                        BirthBar->SetTint(0.28f, 0.48f, 0.82f, 0.94f);
                        BirthBar->SetPos(
                            BaseX + BarGroupWidth * 0.08f,
                            GraphTop + GraphHeight - Height);
                        BirthBar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                    }
                }
            }
        }

        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);
    }
    else if (SelectedPopulationIndex == 2)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"하락률",
            std::to_wstring(PopulationDecline12M),
            0.f,
            FVector4(0.24f, 0.42f, 0.68f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"▷ 사망",
            std::to_wstring(PopulationDecline12M),
            0.f,
            FVector4(0.24f, 0.42f, 0.68f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"해외이주자",
            L"0",
            0.f,
            FVector4(0.72f, 0.24f, 0.20f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"게릴라",
            L"0",
            0.f,
            FVector4(0.40f, 0.56f, 0.22f, 0.92f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 || Index == 1 ?
                        FVector4(0.30f, 0.48f, 0.74f, 0.90f) :
                    Index == 2 ?
                        FVector4(0.78f, 0.34f, 0.28f, 0.92f) :
                        FVector4(0.42f, 0.55f, 0.22f, 0.92f));
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index == 2 ? 104 : (Index == 3 ? 74 : 52),
                    Index == 2 ? 42 : (Index == 3 ? 74 : 60),
                    Index == 2 ? 34 : (Index == 3 ? 32 : 74),
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index == 2 ? 104 : (Index == 3 ? 74 : 52),
                    Index == 2 ? 42 : (Index == 3 ? 74 : 60),
                    Index == 2 ? 34 : (Index == 3 ? 32 : 74),
                    255);
            }
        }

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"하락률");
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"하락 요인");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 2)
                {
                    const int DeclineChartValues[2] = { 0, 10 };
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(DeclineChartValues[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationChangeBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.56f);
            const float MaxValue = 10.f;

            for (int Index = 0; Index < GPopulationChangeBarCount; ++Index)
            {
                if (Index < static_cast<int>(Widget.mPopulationChangeNegativeBars.size()))
                {
                    if (auto HiddenBar =
                        Widget.mPopulationChangeNegativeBars[static_cast<size_t>(Index)].lock())
                    {
                        HiddenBar->SetEnable(false);
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationChangePositiveBars.size()))
                {
                    if (auto DeclineBar =
                        Widget.mPopulationChangePositiveBars[static_cast<size_t>(Index)].lock())
                    {
                        const float ChartValue =
                            (std::min)(
                                10.f,
                                (std::max)(1.0f,
                                    PopulationDeclineBars[static_cast<size_t>(Index)] * 0.58f));
                        const float Height =
                            GraphHeight * Clamp01(ChartValue / MaxValue);
                        DeclineBar->SetEnable(true);
                        DeclineBar->SetTint(0.34f, 0.50f, 0.78f, 0.94f);
                        DeclineBar->SetPos(
                            GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                                (BarGroupWidth - SingleBarWidth) * 0.5f,
                            GraphTop + GraphHeight - Height);
                        DeclineBar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 3)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"▷ 아동",
            std::to_wstring(PopulationChildCount),
            0.f,
            FVector4(0.31f, 0.48f, 0.80f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"▷ 성인",
            std::to_wstring(PopulationAdultCount),
            0.f,
            FVector4(0.80f, 0.34f, 0.28f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"▷ 은퇴",
            std::to_wstring(PopulationRetiredCount),
            0.f,
            FVector4(0.50f, 0.64f, 0.24f, 0.92f),
            false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.31f, 0.48f, 0.80f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.80f, 0.34f, 0.28f, 0.92f) :
                        FVector4(0.50f, 0.64f, 0.24f, 0.92f));
            }

            if (Label)
                Label->SetTextColor(58, 56, 42, 255);

            if (Value)
                Value->SetTextColor(58, 56, 42, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"연령 분포");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(false);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
            ChangeTitle->SetEnable(false);

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        const int PopulationDistributionLabels[GPopulationTrendYAxisLabelCount] =
        {
            0, 210, 420, 630, 840, 1050
        };
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(std::to_wstring(PopulationDistributionLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.72f);
            const float MaxValue = 1260.f;

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float ChildHeight =
                    GraphHeight *
                    Clamp01(PopulationChildBars[static_cast<size_t>(Index)] / MaxValue);
                const float AdultHeight =
                    GraphHeight *
                    Clamp01(PopulationAdultBars[static_cast<size_t>(Index)] / MaxValue);
                const float RetiredHeight =
                    GraphHeight *
                    Clamp01(PopulationRetiredBars[static_cast<size_t>(Index)] / MaxValue);
                const float ChildTop =
                    GraphTop + GraphHeight - ChildHeight;
                const float AdultTop = ChildTop - AdultHeight;
                const float RetiredTop = AdultTop - RetiredHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(true);
                        Bar->SetPos(BarX, ChildTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, ChildHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(true);
                        Bar->SetPos(BarX, AdultTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, AdultHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(true);
                        Bar->SetPos(BarX, RetiredTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, RetiredHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 4)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"현재 주택 공실률",
            std::to_wstring(PopulationHousingVacancyPercent) + L"%",
            0.f,
            FVector4(0.78f, 0.34f, 0.28f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"현재 노숙자 비율",
            std::to_wstring(PopulationHomelessPercent) + L"%",
            0.f,
            FVector4(0.30f, 0.48f, 0.74f, 0.90f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"▷ 빈 방이 있는 주거 건물",
            std::to_wstring(Snapshot.ResidentialVacancyBuildingCount),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"무주택자 가족",
            L"0",
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                if (Index <= 1)
                {
                    Background->SetTexture(
                        Background->GetName() + "_summary",
                        GBarFillTexture);
                    Background->SetTint(
                        Index == 0 ?
                            FVector4(0.78f, 0.34f, 0.28f, 0.92f) :
                            FVector4(0.30f, 0.48f, 0.74f, 0.90f));
                }
                else
                {
                    ApplySelectableBackground(Background, false);
                }
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index <= 1 ? 58 : 76,
                    Index <= 1 ? 56 : 70,
                    Index <= 1 ? 42 : 60,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index <= 1 ? 58 : 76,
                    Index <= 1 ? 56 : 70,
                    Index <= 1 ? 42 : 60,
                    255);
            }
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"가구");
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
            ChangeTitle->SetEnable(false);

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        const int HousingChartLabels[4] = { 0, 10, 20, 30 };
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 4)
                {
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(HousingChartLabels[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;

            for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()); ++Index)
            {
                if (Index < GPopulationDistributionBarCount - 1)
                {
                    if (auto Segment = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        const float X0 =
                            GraphLeft +
                            GraphWidth *
                                static_cast<float>(Index) /
                                static_cast<float>(GPopulationDistributionBarCount - 1);
                        const float X1 =
                            GraphLeft +
                            GraphWidth *
                                static_cast<float>(Index + 1) /
                                static_cast<float>(GPopulationDistributionBarCount - 1);
                        const float Y0 =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                PopulationHousingVacancyTrend[static_cast<size_t>(Index)],
                                0.f,
                                30.f);
                        const float Y1 =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                PopulationHousingVacancyTrend[static_cast<size_t>(Index + 1)],
                                0.f,
                                30.f);
                        SetLineSegment(
                            Segment,
                            X0,
                            Y0,
                            X1,
                            Y1,
                            2.4f,
                            FVector4(0.82f, 0.34f, 0.28f, 0.94f));
                    }
                }
                else if (auto Segment = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                {
                    Segment->SetEnable(false);
                }
            }

            for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()); ++Index)
            {
                if (Index < GPopulationDistributionBarCount - 1)
                {
                    if (auto Segment = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        const float X0 =
                            GraphLeft +
                            GraphWidth *
                                static_cast<float>(Index) /
                                static_cast<float>(GPopulationDistributionBarCount - 1);
                        const float X1 =
                            GraphLeft +
                            GraphWidth *
                                static_cast<float>(Index + 1) /
                                static_cast<float>(GPopulationDistributionBarCount - 1);
                        const float Y0 =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                PopulationHomelessTrend[static_cast<size_t>(Index)],
                                0.f,
                                30.f);
                        const float Y1 =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                PopulationHomelessTrend[static_cast<size_t>(Index + 1)],
                                0.f,
                                30.f);
                        SetLineSegment(
                            Segment,
                            X0,
                            Y0,
                            X1,
                            Y1,
                            2.0f,
                            FVector4(0.30f, 0.48f, 0.74f, 0.94f));
                    }
                }
                else if (auto Segment = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                {
                    Segment->SetEnable(false);
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 5)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);
        SetPopulationTrendWealthLayerEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"파산",
            std::to_wstring(HomelessFamilyWealthBuckets[0]),
            0.f,
            FVector4(0.28f, 0.46f, 0.78f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"가난함",
            std::to_wstring(HomelessFamilyWealthBuckets[1]),
            0.f,
            FVector4(0.76f, 0.25f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"유복함",
            std::to_wstring(HomelessFamilyWealthBuckets[2]),
            0.f,
            FVector4(0.56f, 0.66f, 0.24f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"부유함",
            std::to_wstring(HomelessFamilyWealthBuckets[3]),
            0.f,
            FVector4(0.92f, 0.76f, 0.22f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[4],
            L"더럽게 부유함",
            std::to_wstring(HomelessFamilyWealthBuckets[4]),
            0.f,
            FVector4(0.58f, 0.30f, 0.66f, 0.94f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.28f, 0.46f, 0.78f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.76f, 0.25f, 0.22f, 0.92f) :
                    Index == 2 ?
                        FVector4(0.56f, 0.66f, 0.24f, 0.92f) :
                    Index == 3 ?
                        FVector4(0.92f, 0.76f, 0.22f, 0.94f) :
                        FVector4(0.58f, 0.30f, 0.66f, 0.94f));
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"무주택자 가족");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"무주택자 (재산 순)");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 3)
                {
                    const int HomelessChartLabels[3] = { 0, 10, 20 };
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(HomelessChartLabels[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.76f);
            const float MaxValue = 20.f;

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float BankruptHeight =
                    GraphHeight *
                    Clamp01(PopulationHomelessBankruptBars[static_cast<size_t>(Index)] / MaxValue);
                const float PoorHeight =
                    GraphHeight *
                    Clamp01(PopulationHomelessPoorBars[static_cast<size_t>(Index)] / MaxValue);
                const float WellOffHeight =
                    GraphHeight *
                    Clamp01(PopulationHomelessWellOffBars[static_cast<size_t>(Index)] / MaxValue);
                const float RichHeight =
                    GraphHeight *
                    Clamp01(PopulationHomelessRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float FilthyRichHeight =
                    GraphHeight *
                    Clamp01(PopulationHomelessFilthyRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float BankruptTop =
                    GraphTop + GraphHeight - BankruptHeight;
                const float PoorTop = BankruptTop - PoorHeight;
                const float WellOffTop = PoorTop - WellOffHeight;
                const float RichTop = WellOffTop - RichHeight;
                const float FilthyRichTop = RichTop - FilthyRichHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(BankruptHeight > 0.f);
                        Bar->SetPos(BarX, BankruptTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, BankruptHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(PoorHeight > 0.f);
                        Bar->SetPos(BarX, PoorTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, PoorHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(WellOffHeight > 0.f);
                        Bar->SetPos(BarX, WellOffTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, WellOffHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(RichHeight > 0.f);
                        Bar->SetPos(BarX, RichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, RichHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendFilthyRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendFilthyRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(FilthyRichHeight > 0.f);
                        Bar->SetPos(BarX, FilthyRichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, FilthyRichHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 6)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);
        SetPopulationTrendWealthLayerEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"파산",
            std::to_wstring(ResidentialVacancyWealthBuckets[0]),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"가난함",
            std::to_wstring(ResidentialVacancyWealthBuckets[1]),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"유복함",
            std::to_wstring(ResidentialVacancyWealthBuckets[2]),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"부유함",
            std::to_wstring(ResidentialVacancyWealthBuckets[3]),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[4],
            L"더럽게 부유함",
            std::to_wstring(ResidentialVacancyWealthBuckets[4]),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
                ApplySelectableBackground(Background, false);

            if (Label)
                Label->SetTextColor(76, 70, 60, 255);

            if (Value)
                Value->SetTextColor(76, 70, 60, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"빈 주거 슬롯");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"빈 주택 (재산 순)");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 4)
                {
                    const int VacancyChartLabels[4] = { 0, 10, 20, 30 };
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(VacancyChartLabels[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.76f);
            const float MaxValue = 30.f;

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float BankruptHeight =
                    GraphHeight *
                    Clamp01(PopulationVacantBankruptBars[static_cast<size_t>(Index)] / MaxValue);
                const float PoorHeight =
                    GraphHeight *
                    Clamp01(PopulationVacantPoorBars[static_cast<size_t>(Index)] / MaxValue);
                const float WellOffHeight =
                    GraphHeight *
                    Clamp01(PopulationVacantWellOffBars[static_cast<size_t>(Index)] / MaxValue);
                const float RichHeight =
                    GraphHeight *
                    Clamp01(PopulationVacantRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float FilthyRichHeight =
                    GraphHeight *
                    Clamp01(PopulationVacantFilthyRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float BankruptTop =
                    GraphTop + GraphHeight - BankruptHeight;
                const float PoorTop = BankruptTop - PoorHeight;
                const float WellOffTop = PoorTop - WellOffHeight;
                const float RichTop = WellOffTop - RichHeight;
                const float FilthyRichTop = RichTop - FilthyRichHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(BankruptHeight > 0.f);
                        Bar->SetPos(BarX, BankruptTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, BankruptHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(PoorHeight > 0.f);
                        Bar->SetPos(BarX, PoorTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, PoorHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(WellOffHeight > 0.f);
                        Bar->SetPos(BarX, WellOffTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, WellOffHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(RichHeight > 0.f);
                        Bar->SetPos(BarX, RichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, RichHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendFilthyRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendFilthyRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(FilthyRichHeight > 0.f);
                        Bar->SetPos(BarX, FilthyRichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, FilthyRichHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 7)
    {
        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"현재 실업률",
            std::to_wstring(PopulationCurrentUnemploymentPercent) + L"%",
            0.f,
            FVector4(0.76f, 0.25f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"현재 직장 점유율",
            std::to_wstring(PopulationCurrentJobOccupancyPercent) + L"%",
            0.f,
            FVector4(0.24f, 0.42f, 0.68f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"▷ 빈 일자리",
            std::to_wstring(JobVacancy),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"▷ 폐쇄된 직업",
            L"0",
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[4],
            L"▷ 실업자 시민",
            std::to_wstring(JoblessCitizenCount),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                if (Index <= 1)
                {
                    Background->SetTexture(
                        Background->GetName() + "_summary",
                        GBarFillTexture);
                    Background->SetTint(
                        Index == 0 ?
                            FVector4(0.76f, 0.25f, 0.22f, 0.92f) :
                            FVector4(0.24f, 0.42f, 0.68f, 0.94f));
                }
                else
                {
                    ApplySelectableBackground(Background, false);
                }
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index <= 1 ? 58 : 76,
                    Index <= 1 ? 56 : 70,
                    Index <= 1 ? 42 : 60,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index <= 1 ? 58 : 76,
                    Index <= 1 ? 56 : 70,
                    Index <= 1 ? 42 : 60,
                    255);
            }
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"일자리");
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
            ChangeTitle->SetEnable(false);

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                const int JobChartLabels[GPopulationTrendYAxisLabelCount] =
                {
                    0, 20, 40, 60, 80, 100
                };
                YLabel->SetEnable(true);
                YLabel->SetText(
                    (std::to_wstring(JobChartLabels[Index]) + L"%").c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;

            for (int SegmentIndex = 0;
                SegmentIndex < GPopulationTrendSegmentCount;
                ++SegmentIndex)
            {
                if (SegmentIndex < static_cast<int>(Widget.mPopulationTrendLines.size()))
                {
                    const float X0 =
                        GraphLeft +
                        GraphWidth *
                            static_cast<float>(SegmentIndex) /
                            static_cast<float>(GPopulationTrendSegmentCount);
                    const float X1 =
                        GraphLeft +
                        GraphWidth *
                            static_cast<float>(SegmentIndex + 1) /
                            static_cast<float>(GPopulationTrendSegmentCount);
                    const float Y0 =
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            PopulationJobOccupancyTrend[static_cast<size_t>(SegmentIndex)],
                            0.f,
                            100.f);
                    const float Y1 =
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            PopulationJobOccupancyTrend[static_cast<size_t>(SegmentIndex + 1)],
                            0.f,
                            100.f);
                    SetLineSegment(
                        Widget.mPopulationTrendLines[static_cast<size_t>(SegmentIndex)].lock(),
                        X0,
                        Y0,
                        X1,
                        Y1,
                        2.6f,
                        FVector4(0.24f, 0.42f, 0.68f, 0.94f));
                }
            }

            for (int SegmentIndex = 0;
                SegmentIndex < static_cast<int>(Widget.mPopulationTrendChildBars.size());
                ++SegmentIndex)
            {
                if (SegmentIndex < GPopulationTrendSegmentCount)
                {
                    const float X0 =
                        GraphLeft +
                        GraphWidth *
                            static_cast<float>(SegmentIndex) /
                            static_cast<float>(GPopulationTrendSegmentCount);
                    const float X1 =
                        GraphLeft +
                        GraphWidth *
                            static_cast<float>(SegmentIndex + 1) /
                            static_cast<float>(GPopulationTrendSegmentCount);
                    const float Y0 =
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            PopulationJobUnemploymentTrend[static_cast<size_t>(SegmentIndex)],
                            0.f,
                            100.f);
                    const float Y1 =
                        ResolveGraphYInRange(
                            GraphTop,
                            GraphHeight,
                            PopulationJobUnemploymentTrend[static_cast<size_t>(SegmentIndex + 1)],
                            0.f,
                            100.f);
                    SetLineSegment(
                        Widget.mPopulationTrendChildBars[static_cast<size_t>(SegmentIndex)].lock(),
                        X0,
                        Y0,
                        X1,
                        Y1,
                        2.1f,
                        FVector4(0.78f, 0.26f, 0.22f, 0.92f));
                }
                else if (auto Segment = Widget.mPopulationTrendChildBars[static_cast<size_t>(SegmentIndex)].lock())
                {
                    Segment->SetEnable(false);
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 8)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"무학력",
            std::to_wstring(UnemployedUneducatedCount),
            0.f,
            FVector4(0.30f, 0.48f, 0.78f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"고등학교",
            std::to_wstring(UnemployedHighSchoolCount),
            0.f,
            FVector4(0.78f, 0.26f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"대학교",
            std::to_wstring(UnemployedCollegeCount),
            0.f,
            FVector4(0.56f, 0.68f, 0.24f, 0.92f),
            false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);
        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.30f, 0.48f, 0.78f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.78f, 0.26f, 0.22f, 0.92f) :
                        FVector4(0.56f, 0.68f, 0.24f, 0.92f));
            }

            if (Label)
                Label->SetTextColor(58, 56, 42, 255);

            if (Value)
                Value->SetTextColor(58, 56, 42, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"실업자");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"학력별 실업자");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                if (Index < 3)
                {
                    const int UnemploymentChartLabels[3] = { 0, 10, 20 };
                    YLabel->SetEnable(true);
                    YLabel->SetText(std::to_wstring(UnemploymentChartLabels[Index]).c_str());
                }
                else
                {
                    YLabel->SetEnable(false);
                }
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.76f);
            const float MaxValue = 20.f;

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float UneducatedHeight =
                    GraphHeight *
                    Clamp01(PopulationUnemployedUneducatedBars[static_cast<size_t>(Index)] / MaxValue);
                const float HighSchoolHeight =
                    GraphHeight *
                    Clamp01(PopulationUnemployedHighSchoolBars[static_cast<size_t>(Index)] / MaxValue);
                const float CollegeHeight =
                    GraphHeight *
                    Clamp01(PopulationUnemployedCollegeBars[static_cast<size_t>(Index)] / MaxValue);
                const float UneducatedTop =
                    GraphTop + GraphHeight - UneducatedHeight;
                const float HighSchoolTop = UneducatedTop - HighSchoolHeight;
                const float CollegeTop = HighSchoolTop - CollegeHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(UneducatedHeight > 0.f);
                        Bar->SetPos(BarX, UneducatedTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, UneducatedHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(HighSchoolHeight > 0.f);
                        Bar->SetPos(BarX, HighSchoolTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, HighSchoolHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(CollegeHeight > 0.f);
                        Bar->SetPos(BarX, CollegeTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, CollegeHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 9)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"▷ 무학력",
            std::to_wstring(WorkVacancyUneducatedCount),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"▷ 고등학교",
            std::to_wstring(WorkVacancyHighSchoolCount),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"▷ 대학교",
            std::to_wstring(WorkVacancyCollegeCount),
            0.f,
            FVector4(0.40f, 0.40f, 0.40f, 0.92f),
            false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);
        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
                ApplySelectableBackground(Background, false);

            if (Label)
                Label->SetTextColor(76, 70, 60, 255);

            if (Value)
                Value->SetTextColor(76, 70, 60, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"빈 일자리");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"학력별 빈 일자리");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        const int WorkVacancyChartLabels[GPopulationTrendYAxisLabelCount] =
        {
            0, 150, 300, 450, 600, 750
        };
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(
                    std::to_wstring(WorkVacancyChartLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.72f);
            const float MaxValue = 750.f;

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float Height =
                    GraphHeight *
                    Clamp01(PopulationWorkVacancyBars[static_cast<size_t>(Index)] / MaxValue);

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(true);
                        Bar->SetTint(0.30f, 0.48f, 0.78f, 0.94f);
                        Bar->SetPos(BarX, GraphTop + GraphHeight - Height);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, Height));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 10)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"무학력",
            std::to_wstring(EducationUneducatedCount),
            0.f,
            FVector4(0.30f, 0.48f, 0.78f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"고등학교",
            std::to_wstring(EducationHighSchoolCount),
            0.f,
            FVector4(0.78f, 0.26f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"대학교",
            std::to_wstring(EducationCollegeCount),
            0.f,
            FVector4(0.56f, 0.68f, 0.24f, 0.92f),
            false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);
        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < 3 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.30f, 0.48f, 0.78f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.78f, 0.26f, 0.22f, 0.92f) :
                        FVector4(0.56f, 0.68f, 0.24f, 0.92f));
            }

            if (Label)
                Label->SetTextColor(58, 56, 42, 255);

            if (Value)
                Value->SetTextColor(58, 56, 42, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"교육");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"학교 건물");
        }

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        const int EducationChartMax =
            (std::max)(250,
                ((Snapshot.ActiveCitizenCount + 49) / 50) * 50);
        const int EducationChartStep =
            EducationChartMax / (GPopulationTrendYAxisLabelCount - 1);
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(
                    std::to_wstring(EducationChartStep * Index).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.72f);
            const float MaxValue = static_cast<float>(EducationChartMax);

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float UneducatedHeight =
                    GraphHeight *
                    Clamp01(PopulationEducationUneducatedBars[static_cast<size_t>(Index)] / MaxValue);
                const float HighSchoolHeight =
                    GraphHeight *
                    Clamp01(PopulationEducationHighSchoolBars[static_cast<size_t>(Index)] / MaxValue);
                const float CollegeHeight =
                    GraphHeight *
                    Clamp01(PopulationEducationCollegeBars[static_cast<size_t>(Index)] / MaxValue);
                const float UneducatedTop =
                    GraphTop + GraphHeight - UneducatedHeight;
                const float HighSchoolTop = UneducatedTop - HighSchoolHeight;
                const float CollegeTop = HighSchoolTop - CollegeHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(UneducatedHeight > 0.f);
                        Bar->SetTint(0.30f, 0.48f, 0.78f, 0.94f);
                        Bar->SetPos(BarX, UneducatedTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, UneducatedHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(HighSchoolHeight > 0.f);
                        Bar->SetTint(0.78f, 0.26f, 0.22f, 0.92f);
                        Bar->SetPos(BarX, HighSchoolTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, HighSchoolHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(CollegeHeight > 0.f);
                        Bar->SetTint(0.56f, 0.68f, 0.24f, 0.92f);
                        Bar->SetPos(BarX, CollegeTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, CollegeHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 11)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);
        SetPopulationTrendWealthLayerEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"파산",
            std::to_wstring(CitizenWealthBuckets[0]),
            0.f,
            FVector4(0.28f, 0.46f, 0.78f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"가난함",
            std::to_wstring(CitizenWealthBuckets[1]),
            0.f,
            FVector4(0.76f, 0.25f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"유복함",
            std::to_wstring(CitizenWealthBuckets[2]),
            0.f,
            FVector4(0.56f, 0.66f, 0.24f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"부유함",
            std::to_wstring(CitizenWealthBuckets[3]),
            0.f,
            FVector4(0.92f, 0.76f, 0.22f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[4],
            L"더럽게 부유함",
            std::to_wstring(CitizenWealthBuckets[4]),
            0.f,
            FVector4(0.58f, 0.30f, 0.66f, 0.94f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.28f, 0.46f, 0.78f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.76f, 0.25f, 0.22f, 0.92f) :
                    Index == 2 ?
                        FVector4(0.56f, 0.66f, 0.24f, 0.92f) :
                    Index == 3 ?
                        FVector4(0.92f, 0.76f, 0.22f, 0.94f) :
                        FVector4(0.58f, 0.30f, 0.66f, 0.94f));
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"재산");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(false);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
            ChangeTitle->SetEnable(false);

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        const int WealthChartMax =
            (std::max)(250,
                ((Snapshot.ActiveCitizenCount + 49) / 50) * 50);
        const int WealthChartStep =
            WealthChartMax / (GPopulationTrendYAxisLabelCount - 1);
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(
                    std::to_wstring(WealthChartStep * Index).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.76f);
            const float MaxValue = static_cast<float>(WealthChartMax);

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float BankruptHeight =
                    GraphHeight *
                    Clamp01(PopulationCitizenBankruptBars[static_cast<size_t>(Index)] / MaxValue);
                const float PoorHeight =
                    GraphHeight *
                    Clamp01(PopulationCitizenPoorBars[static_cast<size_t>(Index)] / MaxValue);
                const float WellOffHeight =
                    GraphHeight *
                    Clamp01(PopulationCitizenWellOffBars[static_cast<size_t>(Index)] / MaxValue);
                const float RichHeight =
                    GraphHeight *
                    Clamp01(PopulationCitizenRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float FilthyRichHeight =
                    GraphHeight *
                    Clamp01(PopulationCitizenFilthyRichBars[static_cast<size_t>(Index)] / MaxValue);
                const float BankruptTop =
                    GraphTop + GraphHeight - BankruptHeight;
                const float PoorTop = BankruptTop - PoorHeight;
                const float WellOffTop = PoorTop - WellOffHeight;
                const float RichTop = WellOffTop - RichHeight;
                const float FilthyRichTop = RichTop - FilthyRichHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(BankruptHeight > 0.f);
                        Bar->SetTint(0.28f, 0.46f, 0.78f, 0.94f);
                        Bar->SetPos(BarX, BankruptTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, BankruptHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(PoorHeight > 0.f);
                        Bar->SetTint(0.76f, 0.25f, 0.22f, 0.92f);
                        Bar->SetPos(BarX, PoorTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, PoorHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(WellOffHeight > 0.f);
                        Bar->SetTint(0.56f, 0.66f, 0.24f, 0.92f);
                        Bar->SetPos(BarX, WellOffTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, WellOffHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(RichHeight > 0.f);
                        Bar->SetPos(BarX, RichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, RichHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendFilthyRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendFilthyRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(FilthyRichHeight > 0.f);
                        Bar->SetPos(BarX, FilthyRichTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, FilthyRichHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 12)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(true);
        SetPopulationTrendWealthLayerEnable(true);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"제일 낮음",
            std::to_wstring(OverallSatisfactionMetricBuckets[0]),
            0.f,
            FVector4(0.28f, 0.46f, 0.78f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"낮음",
            std::to_wstring(OverallSatisfactionMetricBuckets[1]),
            0.f,
            FVector4(0.76f, 0.25f, 0.22f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[2],
            L"중간",
            std::to_wstring(OverallSatisfactionMetricBuckets[2]),
            0.f,
            FVector4(0.56f, 0.66f, 0.24f, 0.92f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[3],
            L"높음",
            std::to_wstring(OverallSatisfactionMetricBuckets[3]),
            0.f,
            FVector4(0.92f, 0.76f, 0.22f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[4],
            L"제일 높음",
            std::to_wstring(OverallSatisfactionMetricBuckets[4]),
            0.f,
            FVector4(0.58f, 0.30f, 0.66f, 0.94f),
            false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[static_cast<size_t>(Index)], true);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.28f, 0.46f, 0.78f, 0.94f) :
                    Index == 1 ?
                        FVector4(0.76f, 0.25f, 0.22f, 0.92f) :
                    Index == 2 ?
                        FVector4(0.56f, 0.66f, 0.24f, 0.92f) :
                    Index == 3 ?
                        FVector4(0.92f, 0.76f, 0.22f, 0.94f) :
                        FVector4(0.58f, 0.30f, 0.66f, 0.94f));
            }

            if (Label)
            {
                Label->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }

            if (Value)
            {
                Value->SetTextColor(
                    Index == 4 ? 68 : 58,
                    Index == 4 ? 44 : 56,
                    Index == 4 ? 78 : 42,
                    255);
            }
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"종합 만족도");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(false);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
            ChangeTitle->SetEnable(false);

        for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
                break;

            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            {
                XLabel->SetEnable(true);
                XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
            }
        }

        const int OverallSatisfactionChartMax =
            (std::max)(1260,
                ((Snapshot.ActiveCitizenCount + 209) / 210) * 210);
        const int OverallSatisfactionChartLabels[GPopulationTrendYAxisLabelCount] =
        {
            0, 210, 420, 630, 840, 1050
        };
        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                YLabel->SetEnable(true);
                YLabel->SetText(
                    std::to_wstring(OverallSatisfactionChartLabels[Index]).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;
            const float BarGroupWidth =
                GraphWidth /
                static_cast<float>((std::max)(1, GPopulationDistributionBarCount));
            const float SingleBarWidth =
                (std::max)(4.f, BarGroupWidth * 0.76f);
            const float MaxValue = static_cast<float>(OverallSatisfactionChartMax);

            for (int Index = 0; Index < GPopulationDistributionBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft + BarGroupWidth * static_cast<float>(Index) +
                        (BarGroupWidth - SingleBarWidth) * 0.5f;
                const float VeryLowHeight =
                    GraphHeight *
                    Clamp01(PopulationOverallVeryLowBars[static_cast<size_t>(Index)] / MaxValue);
                const float LowHeight =
                    GraphHeight *
                    Clamp01(PopulationOverallLowBars[static_cast<size_t>(Index)] / MaxValue);
                const float MediumHeight =
                    GraphHeight *
                    Clamp01(PopulationOverallMediumBars[static_cast<size_t>(Index)] / MaxValue);
                const float HighHeight =
                    GraphHeight *
                    Clamp01(PopulationOverallHighBars[static_cast<size_t>(Index)] / MaxValue);
                const float VeryHighHeight =
                    GraphHeight *
                    Clamp01(PopulationOverallVeryHighBars[static_cast<size_t>(Index)] / MaxValue);
                const float VeryLowTop =
                    GraphTop + GraphHeight - VeryLowHeight;
                const float LowTop = VeryLowTop - LowHeight;
                const float MediumTop = LowTop - MediumHeight;
                const float HighTop = MediumTop - HighHeight;
                const float VeryHighTop = HighTop - VeryHighHeight;

                if (Index < static_cast<int>(Widget.mPopulationTrendChildBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendChildBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(VeryLowHeight > 0.f);
                        Bar->SetTint(0.28f, 0.46f, 0.78f, 0.94f);
                        Bar->SetPos(BarX, VeryLowTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, VeryLowHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendAdultBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendAdultBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(LowHeight > 0.f);
                        Bar->SetTint(0.76f, 0.25f, 0.22f, 0.92f);
                        Bar->SetPos(BarX, LowTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, LowHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRetiredBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRetiredBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(MediumHeight > 0.f);
                        Bar->SetTint(0.56f, 0.66f, 0.24f, 0.92f);
                        Bar->SetPos(BarX, MediumTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, MediumHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(HighHeight > 0.f);
                        Bar->SetPos(BarX, HighTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, HighHeight));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationTrendFilthyRichBars.size()))
                {
                    if (auto Bar = Widget.mPopulationTrendFilthyRichBars[static_cast<size_t>(Index)].lock())
                    {
                        Bar->SetEnable(VeryHighHeight > 0.f);
                        Bar->SetPos(BarX, VeryHighTop);
                        Bar->SetSize(SingleBarWidth, (std::max)(2.f, VeryHighHeight));
                    }
                }
            }
        }
    }
    else if (SelectedPopulationIndex == 13)
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendGridLines.size()); ++Index)
        {
            if (auto GridLine = Widget.mPopulationTrendGridLines[static_cast<size_t>(Index)].lock())
                GridLine->SetEnable(false);
        }
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()); ++Index)
        {
            if (auto XLabel = Widget.mPopulationTrendXAxisLabels[static_cast<size_t>(Index)].lock())
                XLabel->SetEnable(false);
        }
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()); ++Index)
        {
            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[static_cast<size_t>(Index)].lock())
                YLabel->SetEnable(false);
        }
        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
            TrendFrame->SetEnable(false);
        if (auto TrendYAxisLine = Widget.mPopulationTrendYAxisLine.lock())
            TrendYAxisLine->SetEnable(false);
        if (auto TrendXAxisLine = Widget.mPopulationTrendXAxisLine.lock())
            TrendXAxisLine->SetEnable(false);
        if (auto TrendYAxisArrow = Widget.mPopulationTrendYAxisArrow.lock())
            TrendYAxisArrow->SetEnable(false);
        if (auto TrendXAxisArrow = Widget.mPopulationTrendXAxisArrow.lock())
            TrendXAxisArrow->SetEnable(false);

        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendLines.size()); ++Index)
        {
            if (auto TrendLine = Widget.mPopulationTrendLines[static_cast<size_t>(Index)].lock())
                TrendLine->SetEnable(false);
        }

        SetPopulationTrendDistributionEnable(false);
        SetPopulationTrendWealthLayerEnable(false);

        SetMetricRowData(
            Widget.mPopulationMetrics[0],
            L"세력 지도자",
            std::to_wstring(FactionLeaderCount),
            0.f,
            FVector4(0.76f, 0.31f, 0.28f, 0.94f),
            false);
        SetMetricRowData(
            Widget.mPopulationMetrics[1],
            L"경쟁자",
            std::to_wstring(RivalCitizenCount),
            0.f,
            FVector4(0.26f, 0.78f, 0.80f, 0.94f),
            false);

        if (Widget.mPopulationMetrics.size() > 0)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[0], true);
        if (Widget.mPopulationMetrics.size() > 1)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[1], true);
        if (Widget.mPopulationMetrics.size() > 2)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[2], false);
        if (Widget.mPopulationMetrics.size() > 3)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
        if (Widget.mPopulationMetrics.size() > 4)
            SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

        for (int Index = 0; Index < 2 && Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
        {
            auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
            auto Background = Row.Background.lock();
            auto Label = Row.Label.lock();
            auto Value = Row.Value.lock();

            if (Background)
            {
                Background->SetTexture(
                    Background->GetName() + "_summary",
                    GBarFillTexture);
                Background->SetTint(
                    Index == 0 ?
                        FVector4(0.76f, 0.31f, 0.28f, 0.94f) :
                        FVector4(0.26f, 0.78f, 0.80f, 0.94f));
            }

            if (Label)
                Label->SetTextColor(58, 56, 42, 255);

            if (Value)
                Value->SetTextColor(58, 56, 42, 255);
        }

        if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
            TrendTitle->SetText(L"특별 시민");
        if (auto ChangeTitleBackground = Widget.mPopulationChangeTitleBackground.lock())
            ChangeTitleBackground->SetEnable(true);
        if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        {
            ChangeTitle->SetEnable(true);
            ChangeTitle->SetText(L"이름이 바뀐 사람");
        }

        std::vector<std::weak_ptr<CImage>> PieSegments;
        PieSegments.reserve(
            Widget.mPopulationTrendLines.size() +
            Widget.mPopulationTrendChildBars.size() +
            Widget.mPopulationTrendAdultBars.size() +
            Widget.mPopulationTrendRetiredBars.size() +
            Widget.mPopulationTrendRichBars.size() +
            Widget.mPopulationTrendFilthyRichBars.size());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendLines.begin(),
            Widget.mPopulationTrendLines.end());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendChildBars.begin(),
            Widget.mPopulationTrendChildBars.end());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendAdultBars.begin(),
            Widget.mPopulationTrendAdultBars.end());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendRetiredBars.begin(),
            Widget.mPopulationTrendRetiredBars.end());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendRichBars.begin(),
            Widget.mPopulationTrendRichBars.end());
        PieSegments.insert(
            PieSegments.end(),
            Widget.mPopulationTrendFilthyRichBars.begin(),
            Widget.mPopulationTrendFilthyRichBars.end());

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float FrameLeft = TrendFrame->GetPos().x;
            const float FrameTop = TrendFrame->GetPos().y;
            const float FrameWidth = TrendFrame->GetSize().x;
            const float FrameHeight = TrendFrame->GetSize().y;
            const float CenterX = FrameLeft + FrameWidth * 0.50f;
            const float CenterY = FrameTop + FrameHeight * 0.54f;
            const float Radius = (std::min)(FrameWidth, FrameHeight) * 0.34f;
            const int SegmentCount =
                (std::max)(24, static_cast<int>(PieSegments.size()));
            const int RivalSegmentCount =
                SpecialCitizenCount > 0 ?
                    (std::max)(0, (std::min)(
                        SegmentCount,
                        RoundToInt(
                            static_cast<double>(SegmentCount) *
                            static_cast<double>(RivalCitizenCount) /
                            static_cast<double>(SpecialCitizenCount)))) :
                    0;
            const float SegmentThickness =
                (std::max)(4.4f,
                    6.28318530718f * Radius /
                    static_cast<float>((std::max)(1, SegmentCount)));
            const float StartAngleRadians =
                -130.f * 3.14159265358979323846f / 180.f;
            const float AngleStepRadians =
                6.28318530718f /
                static_cast<float>((std::max)(1, SegmentCount));

            if (SpecialCitizenCount <= 0)
            {
                for (int Index = 0; Index < static_cast<int>(PieSegments.size()); ++Index)
                {
                    if (auto Segment = PieSegments[static_cast<size_t>(Index)].lock())
                        Segment->SetEnable(false);
                }
            }
            else
            {
                for (int Index = 0; Index < static_cast<int>(PieSegments.size()); ++Index)
                {
                    auto Segment = PieSegments[static_cast<size_t>(Index)].lock();
                    if (!Segment)
                        continue;

                    if (Index >= SegmentCount)
                    {
                        Segment->SetEnable(false);
                        continue;
                    }

                    const float Angle =
                        StartAngleRadians +
                        AngleStepRadians * static_cast<float>(Index);
                    const float EndX =
                        CenterX + std::cos(Angle) * Radius;
                    const float EndY =
                        CenterY + std::sin(Angle) * Radius;
                    const FVector4 Tint =
                        Index < RivalSegmentCount ?
                            FVector4(0.26f, 0.78f, 0.80f, 0.94f) :
                            FVector4(0.76f, 0.31f, 0.28f, 0.94f);
                    SetLineSegment(
                        Segment,
                        CenterX,
                        CenterY,
                        EndX,
                        EndY,
                        SegmentThickness,
                        Tint);
                }
            }
        }
    }
    else
    {
        for (int Index = 0; Index < static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()); ++Index)
        {
            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[static_cast<size_t>(Index)].lock())
                YLabel->SetEnable(true);
        }

    SetMetricRowData(
        Widget.mPopulationMetrics[0],
        L"인구",
        std::to_wstring(Snapshot.ActiveCitizenCount),
        0.f,
        FVector4(0.24f, 0.42f, 0.68f, 0.94f),
        false);
    SetMetricRowData(
        Widget.mPopulationMetrics[1],
        L"성장",
        std::to_wstring(PopulationGrowthSummary),
        0.f,
        FVector4(0.24f, 0.42f, 0.68f, 0.94f),
        false);
    SetMetricRowData(
        Widget.mPopulationMetrics[2],
        L"하락률",
        std::to_wstring(PopulationDeclineSummary),
        0.f,
        FVector4(0.72f, 0.24f, 0.20f, 0.92f),
        false);

    for (int Index = 0; Index < static_cast<int>(Widget.mPopulationMetrics.size()); ++Index)
    {
        auto& Row = Widget.mPopulationMetrics[static_cast<size_t>(Index)];
        auto Background = Row.Background.lock();
        auto Label = Row.Label.lock();
        auto Value = Row.Value.lock();
        auto Bar = Row.Bar.lock();

        if (Background)
        {
            Background->SetTexture(
                Background->GetName() + "_summary",
                GBarFillTexture);
            Background->SetTint(
                Index == 2 ?
                    FVector4(0.80f, 0.34f, 0.30f, 0.90f) :
                    FVector4(0.30f, 0.48f, 0.74f, 0.90f));
        }

        if (Label)
        {
            Label->SetTextColor(
                Index == 2 ? 96 : 54,
                Index == 2 ? 42 : 60,
                Index == 2 ? 38 : 74,
                255);
        }

        if (Value)
        {
            Value->SetTextColor(
                Index == 2 ? 96 : 54,
                Index == 2 ? 42 : 60,
                Index == 2 ? 38 : 74,
                255);
        }

        if (Bar)
            Bar->SetEnable(false);
    }

    if (Widget.mPopulationMetrics.size() > 0)
        SetPopulationMetricEnable(Widget.mPopulationMetrics[0], true);
    if (Widget.mPopulationMetrics.size() > 1)
        SetPopulationMetricEnable(Widget.mPopulationMetrics[1], true);
    if (Widget.mPopulationMetrics.size() > 2)
        SetPopulationMetricEnable(Widget.mPopulationMetrics[2], true);
    if (Widget.mPopulationMetrics.size() > 3)
        SetPopulationMetricEnable(Widget.mPopulationMetrics[3], false);
    if (Widget.mPopulationMetrics.size() > 4)
        SetPopulationMetricEnable(Widget.mPopulationMetrics[4], false);

    if (auto TrendTitle = Widget.mPopulationTrendTitle.lock())
        TrendTitle->SetText(L"인구");

    if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        ChangeTitle->SetEnable(true);

    for (int Index = 0; Index < GPopulationTrendXAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPopulationTrendXAxisLabels.size()))
            break;

        if (auto XLabel = Widget.mPopulationTrendXAxisLabels[Index].lock())
            XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
    }

    {
        const float MaxTrendValue =
            *std::max_element(PopulationTrend.begin(), PopulationTrend.end());
        const int PopulationTrendMax =
            (std::max)(1250,
                ((RoundToInt(MaxTrendValue) + 249) / 250) * 250);

        for (int Index = 0; Index < GPopulationTrendYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationTrendYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationTrendYAxisLabels[Index].lock())
            {
                const int Value =
                    (PopulationTrendMax / (GPopulationTrendYAxisLabelCount - 1)) * Index;
                YLabel->SetText(std::to_wstring(Value).c_str());
            }
        }

        if (auto TrendFrame = Widget.mPopulationTrendFrame.lock())
        {
            const float GraphLeft = TrendFrame->GetPos().x + 36.f;
            const float GraphTop = TrendFrame->GetPos().y + 18.f;
            const float GraphWidth = TrendFrame->GetSize().x - 54.f;
            const float GraphHeight = TrendFrame->GetSize().y - 46.f;

            for (int SegmentIndex = 0;
                SegmentIndex < GPopulationTrendSegmentCount;
                ++SegmentIndex)
            {
                if (SegmentIndex >= static_cast<int>(Widget.mPopulationTrendLines.size()))
                    break;

                const float X0 =
                    GraphLeft +
                    GraphWidth *
                        static_cast<float>(SegmentIndex) /
                        static_cast<float>(GPopulationTrendSegmentCount);
                const float X1 =
                    GraphLeft +
                    GraphWidth *
                        static_cast<float>(SegmentIndex + 1) /
                        static_cast<float>(GPopulationTrendSegmentCount);
                const float Y0 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        PopulationTrend[static_cast<size_t>(SegmentIndex)],
                        0.f,
                        static_cast<float>(PopulationTrendMax));
                const float Y1 =
                    ResolveGraphYInRange(
                        GraphTop,
                        GraphHeight,
                        PopulationTrend[static_cast<size_t>(SegmentIndex + 1)],
                        0.f,
                        static_cast<float>(PopulationTrendMax));
                SetLineSegment(
                    Widget.mPopulationTrendLines[static_cast<size_t>(SegmentIndex)].lock(),
                    X0,
                    Y0,
                    X1,
                    Y1,
                    2.6f,
                    FVector4(0.24f, 0.44f, 0.80f, 0.96f));
            }
        }
    }

    if (auto ChangeTitle = Widget.mPopulationChangeTitle.lock())
        ChangeTitle->SetText(L"인구 변화");

    for (int Index = 0; Index < GPopulationChangeXAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPopulationChangeXAxisLabels.size()))
            break;

        if (auto XLabel = Widget.mPopulationChangeXAxisLabels[Index].lock())
            XLabel->SetText(GetSatisfactionTrendLabel(Index).c_str());
    }

    {
        constexpr int PopulationChangeMin = -40;
        constexpr int PopulationChangeMax = 60;
        constexpr int PopulationChangeStep = 20;

        for (int Index = 0; Index < GPopulationChangeYAxisLabelCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPopulationChangeYAxisLabels.size()))
                break;

            if (auto YLabel = Widget.mPopulationChangeYAxisLabels[Index].lock())
            {
                const int Value =
                    PopulationChangeMin +
                    PopulationChangeStep * Index;
                YLabel->SetText(std::to_wstring(Value).c_str());
            }
        }

        if (auto ChangeFrame = Widget.mPopulationChangeFrame.lock())
        {
            const float GraphLeft = ChangeFrame->GetPos().x + 36.f;
            const float GraphTop = ChangeFrame->GetPos().y + 14.f;
            const float GraphWidth = ChangeFrame->GetSize().x - 54.f;
            const float GraphHeight = ChangeFrame->GetSize().y - 32.f;
            const float ZeroY =
                ResolveGraphYInRange(
                    GraphTop,
                    GraphHeight,
                    0.f,
                    static_cast<float>(PopulationChangeMin),
                    static_cast<float>(PopulationChangeMax));
            const float BarWidth =
                GraphWidth /
                    static_cast<float>((std::max)(1, GPopulationChangeBarCount)) * 0.64f;
            const float StepWidth =
                GraphWidth /
                    static_cast<float>((std::max)(1, GPopulationChangeBarCount));

            for (int Index = 0; Index < GPopulationChangeBarCount; ++Index)
            {
                const float BarX =
                    GraphLeft +
                    StepWidth * static_cast<float>(Index) +
                    (StepWidth - BarWidth) * 0.5f;

                if (Index < static_cast<int>(Widget.mPopulationChangePositiveBars.size()))
                {
                    if (auto PositiveBar =
                        Widget.mPopulationChangePositiveBars[static_cast<size_t>(Index)].lock())
                    {
                        const float PositiveTop =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                PopulationGrowthBars[static_cast<size_t>(Index)],
                                static_cast<float>(PopulationChangeMin),
                                static_cast<float>(PopulationChangeMax));
                        PositiveBar->SetEnable(true);
                        PositiveBar->SetPos(BarX, PositiveTop);
                        PositiveBar->SetSize(BarWidth, (std::max)(2.f, ZeroY - PositiveTop));
                    }
                }

                if (Index < static_cast<int>(Widget.mPopulationChangeNegativeBars.size()))
                {
                    if (auto NegativeBar =
                        Widget.mPopulationChangeNegativeBars[static_cast<size_t>(Index)].lock())
                    {
                        const float NegativeBottom =
                            ResolveGraphYInRange(
                                GraphTop,
                                GraphHeight,
                                -PopulationDeclineBars[static_cast<size_t>(Index)],
                                static_cast<float>(PopulationChangeMin),
                                static_cast<float>(PopulationChangeMax));
                        NegativeBar->SetEnable(true);
                        NegativeBar->SetPos(BarX, ZeroY);
                        NegativeBar->SetSize(
                            BarWidth,
                            (std::max)(2.f, NegativeBottom - ZeroY));
                    }
                }
            }
        }
    }
    }

}


