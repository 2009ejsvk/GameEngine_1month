#include "CitizenInfoPresentationInternal.h"
#include "UIStrings.h"
#include "../StringUtils.h"
#include <array>

using namespace CitizenInfoPresentationInternal;

namespace CitizenInfoPresentation
{
    const wchar_t* GetCitizenProfileWealthDisplayName(
        ECitizenWealthLevel Level)
    {
        switch (Level)
        {
        case ECitizenWealthLevel::Broke:
            return UiText(L"citizen_info.wealth_profile.broke");
        case ECitizenWealthLevel::WellOff:
            return UiText(L"citizen_info.wealth_profile.well_off");
        case ECitizenWealthLevel::Rich:
            return UiText(L"citizen_info.wealth_profile.rich");
        case ECitizenWealthLevel::FilthyRich:
            return UiText(L"citizen_info.wealth_profile.filthy_rich");
        case ECitizenWealthLevel::Poor:
        default:
            return UiText(L"citizen_info.wealth_profile.poor");
        }
    }

    const wchar_t* GetCitizenActivityDisplayName(ECitizenState State)
    {
        switch (State)
        {
        case ECitizenState::GoingToWork:
        case ECitizenState::AtWork:
        case ECitizenState::GoingToTeamsterSource:
        case ECitizenState::GoingToTeamsterHarbor:
        case ECitizenState::GoingToTeamsterConsumerSource:
        case ECitizenState::GoingToTeamsterConsumerTarget:
        case ECitizenState::GoingToTeamsterOffice:
            return UiText(L"citizen_info.activity.work");
        case ECitizenState::GoingToFood:
        case ECitizenState::AtFood:
            return UiText(L"citizen_info.activity.food");
        case ECitizenState::GoingToHealth:
        case ECitizenState::AtHealth:
        case ECitizenState::GoingToFaith:
        case ECitizenState::AtFaith:
        case ECitizenState::GoingHome:
        case ECitizenState::AtHome:
        case ECitizenState::GoingToFun:
        case ECitizenState::AtFun:
            return UiText(L"citizen_info.activity.leisure");
        default:
            return UIStrings::Get(L"citizen_info.activity.move").c_str();
        }
    }

    std::wstring ResolveBuildingDisplayName(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>&
            QuerySource,
        const std::string& BuildingName)
    {
        if (BuildingName.empty())
            return L"-";

        if (!QuerySource)
            return StringUtils::Utf8ToWide(BuildingName);

        return QuerySource->ResolveBuildingDisplayName(BuildingName);
    }

    std::wstring ResolveCitizenLocationText(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>&
            QuerySource,
        const CitizenInfoDataProvider::FCitizenInfoCitizenRecord& Citizen)
    {
        if (!Citizen.Valid)
            return L"-";

        auto MakeInteriorText =
            [&](const std::string& BuildingName)
        {
            const std::wstring DisplayName =
                ResolveBuildingDisplayName(QuerySource, BuildingName);

            if (DisplayName.empty() || DisplayName == L"-")
                return std::wstring(L"-");

            return DisplayName +
                UIStrings::Get(L"citizen_info.location.interior_suffix");
        };

        switch (Citizen.State)
        {
        case ECitizenState::AtHome:
        case ECitizenState::GoingHome:
            return MakeInteriorText(Citizen.HomeBuildingName);
        case ECitizenState::AtWork:
        case ECitizenState::GoingToWork:
        case ECitizenState::GoingToTeamsterSource:
        case ECitizenState::GoingToTeamsterHarbor:
        case ECitizenState::GoingToTeamsterConsumerSource:
        case ECitizenState::GoingToTeamsterConsumerTarget:
        case ECitizenState::GoingToTeamsterOffice:
            return MakeInteriorText(Citizen.WorkBuildingName);
        case ECitizenState::AtFood:
        case ECitizenState::GoingToFood:
            return MakeInteriorText(
                Citizen.FoodVisitBuildingName.empty() ?
                    Citizen.FoodBuildingName :
                    Citizen.FoodVisitBuildingName);
        case ECitizenState::AtFun:
        case ECitizenState::GoingToFun:
            return MakeInteriorText(
                Citizen.FunVisitBuildingName.empty() ?
                    Citizen.FunBuildingName :
                    Citizen.FunVisitBuildingName);
        case ECitizenState::AtHealth:
        case ECitizenState::GoingToHealth:
            return MakeInteriorText(
                Citizen.HealthVisitBuildingName.empty() ?
                    Citizen.HealthBuildingName :
                    Citizen.HealthVisitBuildingName);
        case ECitizenState::AtFaith:
        case ECitizenState::GoingToFaith:
            return MakeInteriorText(
                Citizen.FaithVisitBuildingName.empty() ?
                    Citizen.FaithBuildingName :
                    Citizen.FaithVisitBuildingName);
        default:
            return UIStrings::Get(L"citizen_info.location.outside_tropico");
        }
    }

    std::array<std::wstring, 3> BuildCitizenOpinionLines(
        const FNpcPoliticalProfile& PoliticalProfile)
    {
        struct FOpinionEntry
        {
            EPoliticalAxis Axis;
            FNpcPoliticalChoice Choice;
        };

        const std::array<FOpinionEntry, 4> OrderedEntries =
        {{
            { EPoliticalAxis::ReligionMilitarism,
                PoliticalProfile.ReligionMilitarism },
            { EPoliticalAxis::EnvironmentIndustry,
                PoliticalProfile.EnvironmentIndustry },
            { EPoliticalAxis::IntellectualConservative,
                PoliticalProfile.IntellectualConservative },
            { EPoliticalAxis::Economy,
                PoliticalProfile.Economy }
        }};

        std::array<std::wstring, 3> Lines = {};
        int WriteIndex = 0;

        for (const FOpinionEntry& Entry : OrderedEntries)
        {
            if (WriteIndex >= static_cast<int>(Lines.size()))
                break;

            if (Entry.Choice.Stance == EPoliticalStance::Neutral)
                continue;

            Lines[static_cast<size_t>(WriteIndex)] =
                std::wstring(GetPoliticalFactionDisplayName(
                    Entry.Axis,
                    Entry.Choice.Stance)) +
                L" (" +
                GetCitizenPoliticalIntensityDisplayName(
                    Entry.Axis,
                    Entry.Choice.Support) +
                L")";
            ++WriteIndex;
        }

        return Lines;
    }

    float BuildCitizenSupportRatio(
        const FNpcSatisfaction& Satisfaction)
    {
        const float Overall =
            (std::max)(0.f, (std::min)(100.f, Satisfaction.Overall));
        const float Ratio = (Overall + 12.f) / 112.f;
        return (std::max)(0.f, (std::min)(1.f, Ratio));
    }
}
