#include "CitizenInfoDataProviderInternal.h"
#include "CitizenInfoConstants.h"
#include "CitizenInfoPresentation.h"
#include "CitizenInfoQueryService.h"
#include "UIStrings.h"
#include "../StringUtils.h"
#include <algorithm>
#include <cmath>

using namespace CitizenInfoDataProviderInternal;

namespace CitizenInfoDataProvider
{
    FCitizenInfoSnapshot BuildCitizenSnapshot(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FCitizenIdentityProfile& IdentityProfile,
        const FNpcPoliticalProfile& PoliticalProfile,
        int TabIndex)
    {
        const int ClampedTab =
            (std::max)(
                0,
                (std::min)(CitizenInfoConstants::GCitizenTabCount - 1, TabIndex));

        auto ToPercent = [](float Value)
        {
            const float Clamped = (std::max)(0.f, (std::min)(100.f, Value));
            return static_cast<int>(roundf(Clamped));
        };

        FCitizenInfoSnapshot Result;
        Result.Valid = true;
        Result.Mode = EPanelMode::Citizen;
        Result.SelectedTabIndex = ClampedTab;
        Result.Title = StringUtils::Utf8ToWide(CitizenName);
        Result.ShowTabButtons = true;
        Result.ShowBudgetControls = false;
        Result.ShowActionButtons = false;
        Result.ShowSectionRibbon = (ClampedTab > 0);
        Result.ShowTitleIcon = false;

        if (IdentityProfile.IsTourist)
        {
            Result.Subtitle =
                UIStrings::Get(L"citizen_info.subtitle.tourist") +
                L"  |  " +
                GetTouristPreferenceDisplayName(
                    IdentityProfile.TouristProfile);
        }
        else
        {
            Result.Subtitle = UIStrings::Format(
                L"citizen_info.subtitle.citizen_identity_template",
                {
                    GetCitizenWealthDisplayName(
                        IdentityProfile.WealthLevel),
                    GetCitizenEducationDisplayName(
                        IdentityProfile.EducationLevel),
                    IdentityProfile.IsImmigrant ?
                        Ui(L"citizen_info.fragment.immigrant") :
                        std::wstring()
                });
        }

        if (ClampedTab == 0)
        {
            Result.PageTitle =
                CitizenInfoConstants::GetCitizenPageTitle(ClampedTab);
            Result.BodyText = UIStrings::Format(
                L"citizen_info.body.citizen_overview",
                {
                    GetCitizenEducationDisplayName(
                        IdentityProfile.EducationLevel),
                    GetCitizenWealthDisplayName(
                        IdentityProfile.WealthLevel),
                    IdentityProfile.IsImmigrant ?
                        Ui(L"citizen_info.fragment.immigrant_parenthetical") :
                        std::wstring(),
                    std::to_wstring(ToPercent(Satisfaction.Food)),
                    std::to_wstring(ToPercent(Satisfaction.Health)),
                    std::to_wstring(ToPercent(Satisfaction.Fun)),
                    std::to_wstring(ToPercent(Satisfaction.Faith)),
                    std::to_wstring(ToPercent(Satisfaction.Housing)),
                    std::to_wstring(ToPercent(Satisfaction.Job)),
                    std::to_wstring(ToPercent(Satisfaction.CommuteTimePenalty)),
                    std::to_wstring(ToPercent(Satisfaction.Freedom)),
                    std::to_wstring(ToPercent(Satisfaction.Security)),
                    std::to_wstring(ToPercent(Satisfaction.Overall))
                });
        }
        else if (ClampedTab == 1)
        {
            Result.PageTitle =
                CitizenInfoConstants::GetCitizenPageTitle(ClampedTab);
            Result.BodyText = UIStrings::Format(
                L"citizen_info.body.citizen_politics",
                {
                    std::to_wstring(ToPercent(Satisfaction.Food)),
                    std::to_wstring(ToPercent(Satisfaction.Health)),
                    std::to_wstring(ToPercent(Satisfaction.Fun)),
                    std::to_wstring(ToPercent(Satisfaction.Faith)),
                    std::to_wstring(ToPercent(Satisfaction.Housing)),
                    std::to_wstring(ToPercent(Satisfaction.Job)),
                    std::to_wstring(ToPercent(Satisfaction.Freedom)),
                    std::to_wstring(ToPercent(Satisfaction.Security)),
                    std::to_wstring(ToPercent(Satisfaction.Overall)),
                    GetPoliticalFactionDisplayName(
                        EPoliticalAxis::Economy,
                        PoliticalProfile.Economy.Stance),
                    GetPoliticalSupportDisplayName(
                        PoliticalProfile.Economy.Support),
                    GetPoliticalFactionDisplayName(
                        EPoliticalAxis::ReligionMilitarism,
                        PoliticalProfile.ReligionMilitarism.Stance),
                    GetPoliticalSupportDisplayName(
                        PoliticalProfile.ReligionMilitarism.Support),
                    GetPoliticalFactionDisplayName(
                        EPoliticalAxis::EnvironmentIndustry,
                        PoliticalProfile.EnvironmentIndustry.Stance),
                    GetPoliticalSupportDisplayName(
                        PoliticalProfile.EnvironmentIndustry.Support),
                    GetPoliticalFactionDisplayName(
                        EPoliticalAxis::IntellectualConservative,
                        PoliticalProfile.IntellectualConservative.Stance),
                    GetPoliticalSupportDisplayName(
                        PoliticalProfile.IntellectualConservative.Support)
                });
        }
        else
        {
            Result.PageTitle =
                CitizenInfoConstants::GetCitizenPageTitle(ClampedTab);
            const int Overall = ToPercent(Satisfaction.Overall);

            const wchar_t* SatisfactionDesc =
                Overall >= 75 ?
                    UiText(L"citizen_info.thoughts.satisfaction.very_high") :
                Overall >= 55 ?
                    UiText(L"citizen_info.thoughts.satisfaction.high") :
                Overall >= 35 ?
                    UiText(L"citizen_info.thoughts.satisfaction.low") :
                    UiText(L"citizen_info.thoughts.satisfaction.very_low");

            const wchar_t* DominantStance = nullptr;
            int MaxSupport = -1;
            auto CheckAxis =
                [&](EPoliticalAxis Axis, const FNpcPoliticalChoice& Profile)
            {
                const int Sup = static_cast<int>(Profile.Support);
                if (Sup > MaxSupport)
                {
                    MaxSupport = Sup;
                    DominantStance = GetPoliticalFactionDisplayName(
                        Axis,
                        Profile.Stance);
                }
            };
            CheckAxis(EPoliticalAxis::Economy, PoliticalProfile.Economy);
            CheckAxis(
                EPoliticalAxis::ReligionMilitarism,
                PoliticalProfile.ReligionMilitarism);
            CheckAxis(
                EPoliticalAxis::EnvironmentIndustry,
                PoliticalProfile.EnvironmentIndustry);
            CheckAxis(
                EPoliticalAxis::IntellectualConservative,
                PoliticalProfile.IntellectualConservative);

            Result.BodyText = UIStrings::Format(
                L"citizen_info.body.citizen_thoughts",
                {
                    Result.Title,
                    GetCitizenWealthDisplayName(
                        IdentityProfile.WealthLevel),
                    GetCitizenEducationDisplayName(
                        IdentityProfile.EducationLevel),
                    IdentityProfile.IsImmigrant ?
                        Ui(L"citizen_info.fragment.immigrant_parenthetical") :
                        std::wstring(),
                    SatisfactionDesc,
                    DominantStance ? DominantStance : L"-"
                });
        }

        return Result;
    }

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex)
    {
        if (!QuerySource || CitizenName.empty())
            return FCitizenInfoSnapshot();

        FCitizenInfoCitizenRecord Citizen;

        if (!QuerySource->TryGetCitizenRecord(CitizenName, Citizen) ||
            !Citizen.Valid)
        {
            return FCitizenInfoSnapshot();
        }

        FCitizenInfoSnapshot Result = BuildCitizenSnapshot(
            Citizen.Name,
            Citizen.Satisfaction,
            Citizen.IdentityProfile,
            Citizen.PoliticalProfile,
            SelectedCitizenTabIndex);

        if (Result.Valid && Result.SelectedTabIndex == 0)
        {
            const bool IsTourist = Citizen.IdentityProfile.IsTourist;
            const std::wstring TouristProfileText =
                IsTourist &&
                    HasTouristPreference(
                        Citizen.IdentityProfile.TouristProfile) ?
                    GetTouristPreferenceDisplayName(
                        Citizen.IdentityProfile.TouristProfile) :
                    std::wstring(L"-");
            const std::wstring TouristVenueText =
                CitizenInfoPresentation::ResolveBuildingDisplayName(
                    QuerySource,
                    Citizen.FunVisitBuildingName.empty() ?
                        Citizen.FunBuildingName :
                        Citizen.FunVisitBuildingName);
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText =
                UIStrings::Get(
                    IsTourist ?
                        L"citizen_info.subtitle.tourist" :
                        L"citizen_info.subtitle.tropican");
            Result.ShowCitizenProfileOverview = true;
            Result.ShowCitizenActionButtons = true;
            Result.ShowSectionDivider = true;
            Result.CitizenPortraitSlotCount = 11;
            Result.CitizenPortraitOccupiedSlot = 4;
            Result.CitizenPortraitVariant =
                static_cast<int>(Citizen.Name.size() % 4);
            Result.CitizenFooterText = L"TROPICO EXEC. 16FA-923";

            Result.OverviewMetricLabels[0] =
                Ui(L"citizen_info.metric.activity");
            Result.OverviewMetricValues[0] =
                CitizenInfoPresentation::GetCitizenActivityDisplayName(
                    Citizen.State);
            Result.OverviewMetricLabels[1] =
                Ui(L"citizen_info.metric.location");
            Result.OverviewMetricValues[1] =
                CitizenInfoPresentation::ResolveCitizenLocationText(
                    QuerySource,
                    Citizen);
            Result.OverviewMetricLabels[2] =
                Ui(
                    IsTourist ?
                        L"citizen_info.metric.tourist_profile" :
                        L"citizen_info.metric.age");
            Result.OverviewMetricValues[2] =
                IsTourist ?
                    TouristProfileText :
                    Ui(L"citizen_info.value.age_30_adult");
            Result.OverviewMetricLabels[3] =
                Ui(L"citizen_info.metric.origin");
            Result.OverviewMetricValues[3] =
                IsTourist ?
                    Ui(L"citizen_info.origin.tourist") :
                Citizen.IdentityProfile.IsImmigrant ?
                    Ui(L"citizen_info.origin.france") :
                    Ui(L"citizen_info.origin.tropico");
            Result.OverviewMetricLabels[4] =
                Ui(L"citizen_info.metric.wealth");
            Result.OverviewMetricValues[4] =
                CitizenInfoPresentation::GetCitizenProfileWealthDisplayName(
                    Citizen.IdentityProfile.WealthLevel);
            Result.OverviewMetricLabels[5] =
                Ui(L"citizen_info.metric.education");
            Result.OverviewMetricValues[5] =
                GetCitizenEducationDisplayName(
                    Citizen.IdentityProfile.EducationLevel);
            Result.OverviewMetricLabels[6] =
                Ui(
                    IsTourist ?
                        L"citizen_info.metric.visit" :
                        L"citizen_info.metric.job");
            Result.OverviewMetricValues[6] =
                IsTourist ?
                    TouristVenueText :
                    CitizenInfoPresentation::ResolveBuildingDisplayName(
                        QuerySource,
                        Citizen.WorkBuildingName);
            Result.OverviewMetricLabels[7] =
                Ui(L"citizen_info.metric.home");
            Result.OverviewMetricValues[7] =
                CitizenInfoPresentation::ResolveBuildingDisplayName(
                    QuerySource,
                    Citizen.HomeBuildingName);
            Result.OverviewMetricAccentValues[6] =
                IsTourist ?
                    !Citizen.FunBuildingName.empty() ||
                        !Citizen.FunVisitBuildingName.empty() :
                    !Citizen.WorkBuildingName.empty();
            Result.OverviewMetricAccentValues[7] =
                !Citizen.HomeBuildingName.empty();

            Result.CitizenActionLabels[0] =
                Ui(L"citizen_info.action.bribe");
            Result.CitizenActionLabels[1] =
                Ui(L"citizen_info.action.kill");
            Result.CitizenActionLabels[2] =
                Ui(L"citizen_info.action.stage_accident");
            Result.CitizenActionLabels[3] =
                Ui(L"citizen_info.action.arrest");
            Result.CitizenActionLabels[4] =
                Ui(L"citizen_info.action.isolate");
            Result.CitizenActionLabels[5] =
                Ui(L"citizen_info.action.send_to_space");
        }
        else if (Result.Valid && Result.SelectedTabIndex == 1)
        {
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText =
                UIStrings::Get(
                    Citizen.IdentityProfile.IsTourist ?
                        L"citizen_info.subtitle.tourist" :
                        L"citizen_info.subtitle.tropican");
            Result.ShowCitizenPoliticsOverview = true;
            Result.ShowCitizenProfileOverview = false;
            Result.ShowCitizenActionButtons = false;
            Result.ShowSectionDivider = false;
            Result.CitizenFooterText.clear();

            Result.CitizenPoliticsSatisfactionLabels =
            {{
                Ui(L"almanac.satisfaction.overall"),
                Ui(L"almanac.satisfaction.food"),
                Ui(L"almanac.satisfaction.health"),
                Ui(L"almanac.satisfaction.fun"),
                Ui(L"almanac.satisfaction.faith"),
                Ui(L"almanac.satisfaction.housing"),
                Ui(L"almanac.satisfaction.job"),
                Ui(L"almanac.satisfaction.freedom"),
                Ui(L"almanac.satisfaction.security")
            }};
            Result.CitizenPoliticsSatisfactionRatios =
            {{
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Overall / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Food / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Health / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Fun / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Faith / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Housing / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Job / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Freedom / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Security / 100.f))
            }};
            Result.CitizenPoliticsOpinionLines =
                CitizenInfoPresentation::BuildCitizenOpinionLines(
                    Citizen.PoliticalProfile);
            Result.CitizenPoliticsSupportRatio =
                CitizenInfoPresentation::BuildCitizenSupportRatio(
                    Citizen.Satisfaction);
        }
        else if (Result.Valid && Result.SelectedTabIndex == 2)
        {
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText =
                UIStrings::Get(
                    Citizen.IdentityProfile.IsTourist ?
                        L"citizen_info.subtitle.tourist" :
                        L"citizen_info.subtitle.tropican");
            Result.ShowCitizenThoughtsOverview = true;
            Result.ShowCitizenProfileOverview = false;
            Result.ShowCitizenPoliticsOverview = false;
            Result.ShowCitizenActionButtons = false;
            Result.ShowSectionDivider = false;
            Result.CitizenFooterText.clear();
            Result.CitizenThoughtLines =
            {{
                Ui(L"citizen_info.thought.line1"),
                Ui(L"citizen_info.thought.line2"),
                Ui(L"citizen_info.thought.line3"),
                Ui(L"citizen_info.thought.line4"),
                Ui(L"citizen_info.thought.line5")
            }};
        }

        return Result;
    }

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex)
    {
        return BuildTrackedCitizenSnapshot(
            CitizenInfoQueryService::CreateWorldQuerySource(World),
            CitizenName,
            SelectedCitizenTabIndex);
    }
}
