#pragma once

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
    GoingToTeamsterOffice
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
