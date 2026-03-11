#pragma once

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
        return L"고졸";
    case ECitizenEducationLevel::College:
        return L"대졸";
    default:
        return L"무학력";
    }
}

enum class ECitizenWealthLevel
{
    Poor = 0,
    WellOff,
    Rich
};

inline const wchar_t* GetCitizenWealthDisplayName(ECitizenWealthLevel Level)
{
    switch (Level)
    {
    case ECitizenWealthLevel::WellOff:
        return L"유복";
    case ECitizenWealthLevel::Rich:
        return L"부유";
    default:
        return L"가난";
    }
}

struct FCitizenIdentityProfile
{
    ECitizenEducationLevel EducationLevel =
        ECitizenEducationLevel::Uneducated;
    ECitizenWealthLevel WealthLevel = ECitizenWealthLevel::Poor;
    bool IsImmigrant = true;
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
    case EPoliticalAxis::Economy:                 return L"경제";
    case EPoliticalAxis::ReligionMilitarism:      return L"신념";
    case EPoliticalAxis::EnvironmentIndustry:     return L"산업";
    case EPoliticalAxis::IntellectualConservative: return L"사회";
    default: return L"정치";
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
        case EPoliticalStance::Left:  return L"자본주의자";
        case EPoliticalStance::Right: return L"공산주의자";
        default: return L"무관심";
        }
    case EPoliticalAxis::ReligionMilitarism:
        switch (Stance)
        {
        case EPoliticalStance::Left:  return L"종교인";
        case EPoliticalStance::Right: return L"군국주의자";
        default: return L"무관심";
        }
    case EPoliticalAxis::EnvironmentIndustry:
        switch (Stance)
        {
        case EPoliticalStance::Left:  return L"환경주의자";
        case EPoliticalStance::Right: return L"실업가";
        default: return L"무관심";
        }
    case EPoliticalAxis::IntellectualConservative:
        switch (Stance)
        {
        case EPoliticalStance::Left:  return L"지식인";
        case EPoliticalStance::Right: return L"보수주의자";
        default: return L"무관심";
        }
    default: return L"무관심";
    }
}

inline const wchar_t* GetPoliticalSupportDisplayName(EPoliticalSupportLevel Support)
{
    switch (Support)
    {
    case EPoliticalSupportLevel::Weak:   return L"약함";
    case EPoliticalSupportLevel::Strong: return L"강함";
    default: return L"보통";
    }
}
