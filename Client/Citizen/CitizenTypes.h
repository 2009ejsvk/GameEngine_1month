#pragma once

#include "../Building/BuildingTypes.h"
#include "../UI/UIStrings.h"
#include <algorithm>
#include <string>

// 시민 FSM 상태
enum class ECitizenState
{
    Wander,                  // 핵심 건물(집/직장/음식) 미배정
    GoingToWork,
    AtWork,                  // 직장 체류 타이머
    GoingHome,
    AtHome,                  // 집 체류 타이머
    GoingToFood,
    AtFood,
    GoingToFun,
    AtFun,
    GoingToHealth,
    AtHealth,
    GoingToFaith,
    AtFaith,
    GoingToTeamsterSource,
    GoingToTeamsterHarbor,
    GoingToTeamsterConsumerSource,
    GoingToTeamsterConsumerTarget,
    GoingToTeamsterOffice
};

enum class ECitizenEducationLevel
{
    Uneducated = 0,
    HighSchool,
    College
};

inline const wchar_t* GetCitizenEducationDisplayName(
    ECitizenEducationLevel Level)
{
    switch (Level)
    {
    case ECitizenEducationLevel::HighSchool:
        return UIStrings::Get(L"citizen.education.high_school").c_str();
    case ECitizenEducationLevel::College:
        return UIStrings::Get(L"citizen.education.college").c_str();
    default:
        return UIStrings::Get(L"citizen.education.uneducated").c_str();
    }
}

enum class ECitizenWealthLevel
{
    Broke = 0,
    Poor,
    WellOff,
    Rich,
    FilthyRich
};

constexpr int GCitizenWealthLevelCount =
    static_cast<int>(ECitizenWealthLevel::FilthyRich) + 1;

inline int GetCitizenWealthRank(ECitizenWealthLevel Level)
{
    const int Rank = static_cast<int>(Level);
    return (std::max)(0, (std::min)(GCitizenWealthLevelCount - 1, Rank));
}

inline bool IsLowWealthCitizen(ECitizenWealthLevel WealthLevel)
{
    return GetCitizenWealthRank(WealthLevel) <=
        GetCitizenWealthRank(ECitizenWealthLevel::Poor);
}

inline bool IsAffluentCitizen(ECitizenWealthLevel WealthLevel)
{
    return GetCitizenWealthRank(WealthLevel) >=
        GetCitizenWealthRank(ECitizenWealthLevel::Rich);
}

inline bool IsLuxuryWealthCitizen(ECitizenWealthLevel WealthLevel)
{
    return GetCitizenWealthRank(WealthLevel) >=
        GetCitizenWealthRank(ECitizenWealthLevel::FilthyRich);
}

inline const wchar_t* GetCitizenWealthDisplayName(ECitizenWealthLevel Level)
{
    switch (Level)
    {
    case ECitizenWealthLevel::Broke:
        return UIStrings::Get(L"citizen.wealth.broke").c_str();
    case ECitizenWealthLevel::WellOff:
        return UIStrings::Get(L"citizen.wealth.well_off").c_str();
    case ECitizenWealthLevel::Rich:
        return UIStrings::Get(L"citizen.wealth.rich").c_str();
    case ECitizenWealthLevel::FilthyRich:
        return UIStrings::Get(L"citizen.wealth.filthy_rich").c_str();
    case ECitizenWealthLevel::Poor:
    default:
        return UIStrings::Get(L"citizen.wealth.poor").c_str();
    }
}

struct FCitizenIdentityProfile
{
    ECitizenEducationLevel EducationLevel =
        ECitizenEducationLevel::Uneducated;
    ECitizenWealthLevel WealthLevel = ECitizenWealthLevel::Poor;
    int HouseholdId = -1;
    int HouseholdSize = 1;
    bool IsImmigrant = true;
    bool IsTourist = false;
    ETouristPreference TouristProfile = ETouristPreference::None;
    std::string DisplayName;
};

// 시민 만족도 (행복 5요소 + 부가 항목)
struct FNpcSatisfaction
{
    float Food     = 70.f;
    float Health   = 70.f;
    float Fun      = 70.f;
    float Faith    = 70.f;
    float Housing  = 70.f;
    float Job      = 70.f;
    float CommuteTimePenalty = 0.f;
    float Freedom  = 70.f;
    float Security = 70.f;
    float Overall  = 70.f;
};

// 정치 성향 축 (4개 파벌 대립 구도)
enum class EPoliticalAxis
{
    Economy = 0,
    ReligionMilitarism,
    EnvironmentIndustry,
    IntellectualConservative,
    Count
};

enum class EPoliticalStance
{
    Left = 0,
    Neutral,
    Right
};

enum class EPoliticalSupportLevel
{
    Weak = 0,
    Normal,
    Strong
};

struct FNpcPoliticalChoice
{
    EPoliticalStance     Stance  = EPoliticalStance::Neutral;
    EPoliticalSupportLevel Support = EPoliticalSupportLevel::Normal;
};

struct FNpcPoliticalProfile
{
    FNpcPoliticalChoice Economy;
    FNpcPoliticalChoice ReligionMilitarism;
    FNpcPoliticalChoice EnvironmentIndustry;
    FNpcPoliticalChoice IntellectualConservative;

    const FNpcPoliticalChoice& Get(EPoliticalAxis Axis) const
    {
        switch (Axis)
        {
        case EPoliticalAxis::Economy:              return Economy;
        case EPoliticalAxis::ReligionMilitarism:   return ReligionMilitarism;
        case EPoliticalAxis::EnvironmentIndustry:  return EnvironmentIndustry;
        case EPoliticalAxis::IntellectualConservative: return IntellectualConservative;
        default: return Economy;
        }
    }

    FNpcPoliticalChoice& Get(EPoliticalAxis Axis)
    {
        switch (Axis)
        {
        case EPoliticalAxis::Economy:              return Economy;
        case EPoliticalAxis::ReligionMilitarism:   return ReligionMilitarism;
        case EPoliticalAxis::EnvironmentIndustry:  return EnvironmentIndustry;
        case EPoliticalAxis::IntellectualConservative: return IntellectualConservative;
        default: return Economy;
        }
    }
};

struct FCitizenAssignmentProfile
{
    std::string HomeBuildingName;
    std::string WorkBuildingName;
    std::string FoodBuildingName;
    std::string FunBuildingName;
    std::string HealthBuildingName;
    std::string FaithBuildingName;

    bool HasCoreAssignments() const
    {
        return !HomeBuildingName.empty() &&
            !WorkBuildingName.empty() &&
            !FoodBuildingName.empty();
    }
};

struct FCitizenProfile
{
    FCitizenIdentityProfile Identity;
    FCitizenAssignmentProfile Assignments;
    FNpcSatisfaction Satisfaction;
    FNpcPoliticalProfile Politics;
};

// 표시 이름 헬퍼
inline const wchar_t* GetPoliticalAxisDisplayName(EPoliticalAxis Axis)
{
    switch (Axis)
    {
    case EPoliticalAxis::Economy:
        return UIStrings::Get(L"citizen.politics.axis.economy").c_str();
    case EPoliticalAxis::ReligionMilitarism:
        return UIStrings::Get(
            L"citizen.politics.axis.religion_militarism").c_str();
    case EPoliticalAxis::EnvironmentIndustry:
        return UIStrings::Get(
            L"citizen.politics.axis.environment_industry").c_str();
    case EPoliticalAxis::IntellectualConservative:
        return UIStrings::Get(
            L"citizen.politics.axis.intellectual_conservative").c_str();
    default:
        return UIStrings::Get(L"citizen.politics.axis.default").c_str();
    }
}

inline const wchar_t* GetPoliticalFactionDisplayName(
    EPoliticalAxis Axis, EPoliticalStance Stance)
{
    switch (Axis)
    {
    case EPoliticalAxis::Economy:
        switch (Stance)
        {
        case EPoliticalStance::Left:
            return UIStrings::Get(
                L"citizen.politics.faction.economy.left").c_str();
        case EPoliticalStance::Right:
            return UIStrings::Get(
                L"citizen.politics.faction.economy.right").c_str();
        default:
            return UIStrings::Get(
                L"citizen.politics.faction.neutral").c_str();
        }
    case EPoliticalAxis::ReligionMilitarism:
        switch (Stance)
        {
        case EPoliticalStance::Left:
            return UIStrings::Get(
                L"citizen.politics.faction.religion_militarism.left").c_str();
        case EPoliticalStance::Right:
            return UIStrings::Get(
                L"citizen.politics.faction.religion_militarism.right").c_str();
        default:
            return UIStrings::Get(
                L"citizen.politics.faction.neutral").c_str();
        }
    case EPoliticalAxis::EnvironmentIndustry:
        switch (Stance)
        {
        case EPoliticalStance::Left:
            return UIStrings::Get(
                L"citizen.politics.faction.environment_industry.left").c_str();
        case EPoliticalStance::Right:
            return UIStrings::Get(
                L"citizen.politics.faction.environment_industry.right").c_str();
        default:
            return UIStrings::Get(
                L"citizen.politics.faction.neutral").c_str();
        }
    case EPoliticalAxis::IntellectualConservative:
        switch (Stance)
        {
        case EPoliticalStance::Left:
            return UIStrings::Get(
                L"citizen.politics.faction.intellectual_conservative.left")
                .c_str();
        case EPoliticalStance::Right:
            return UIStrings::Get(
                L"citizen.politics.faction.intellectual_conservative.right")
                .c_str();
        default:
            return UIStrings::Get(
                L"citizen.politics.faction.neutral").c_str();
        }
    default:
        return UIStrings::Get(L"citizen.politics.faction.neutral").c_str();
    }
}

inline const wchar_t* GetPoliticalSupportDisplayName(EPoliticalSupportLevel Support)
{
    switch (Support)
    {
    case EPoliticalSupportLevel::Weak:
        return UIStrings::Get(L"citizen.politics.support.weak").c_str();
    case EPoliticalSupportLevel::Strong:
        return UIStrings::Get(L"citizen.politics.support.strong").c_str();
    default:
        return UIStrings::Get(L"citizen.politics.support.normal").c_str();
    }
}
