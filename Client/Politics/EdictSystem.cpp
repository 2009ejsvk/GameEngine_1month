#include "EdictSystem.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>

namespace
{
    FPoliticalSignalDef MakeSignal(
        EPoliticalAxis Axis,
        EPoliticalStance Stance,
        float Strength,
        EPoliticalScope Scope = EPoliticalScope::Global)
    {
        FPoliticalSignalDef Signal;
        Signal.Axis = Axis;
        Signal.FavoredStance = Stance;
        Signal.Strength = Strength;
        Signal.Scope = Scope;
        return Signal;
    }
}

namespace EdictSystem
{
    const std::vector<FGovernmentEdictDefinition>&
        GetGovernmentEdictDefinitions()
    {
        static const std::vector<FGovernmentEdictDefinition> GDefinitions =
        {
            {
                EGovernmentEdictType::FoodForThePeople,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::WelfareProgram,
                L"국민에게 식량을",
                L"음식 만족도를 올리지만 식사 시 식량을 더 소비합니다.",
                L"음식 회복량 증가, 식사 시 음식 2개 소비, 공산주의자 호감 상승",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_FoodForThePeople.png"),
                500,
                0,
                0,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, 7.5f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, -4.5f)
                }
            },
            {
                EGovernmentEdictType::TaxCut,
                EGovernmentEdictMode::Active,
                EPoliticalActionType::TaxCut,
                L"감세",
                L"시민 수에 비례한 비용을 지불하고 단기 지지율을 끌어올립니다.",
                L"1년간 자유/지지 상승, 자본주의자 호감, 5년 재사용 대기시간",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_taxCut.png"),
                0,
                5,
                0,
                365,
                365 * 5,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 8.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, -4.0f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, 2.5f)
                }
            },
            {
                EGovernmentEdictType::MartialLaw,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::MartialLaw,
                L"계엄령",
                L"자유를 희생해 치안과 공포를 높입니다.",
                L"자유 하락, 치안 상승, 군국주의자 호감, 지식인 반발",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_martialLaw.png"),
                7500,
                0,
                0,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::ReligionMilitarism,
                        EPoliticalStance::Right, 10.0f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, -8.0f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Right, 3.5f)
                }
            },
            {
                EGovernmentEdictType::FreeHousing,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::HousingInitiative,
                L"여유 주택",
                L"임대 부담을 정부가 떠안아 주거 만족도를 크게 올립니다.",
                L"주거 만족도 상승, 재정 부담 증가, 공산주의자 호감 상승",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_freeHousing.png"),
                500,
                0,
                1200,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, 9.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, -6.5f)
                }
            },
            {
                EGovernmentEdictType::EmployeeOfTheMonth,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::IndustrialSubsidy,
                L"이달의 직원",
                L"운영비를 더 들여 생산성을 끌어올립니다.",
                L"생산량 증가, 직업 만족도 소폭 상승, 월간 운영비 추가",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_employeeOfTheMonth.png"),
                0,
                0,
                350,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::EnvironmentIndustry,
                        EPoliticalStance::Right, 8.5f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 3.0f)
                }
            }
        };

        return GDefinitions;
    }

    const FGovernmentEdictDefinition* FindGovernmentEdictDefinition(
        EGovernmentEdictType Type)
    {
        const auto& Definitions = GetGovernmentEdictDefinitions();

        for (size_t i = 0; i < Definitions.size(); ++i)
        {
            if (Definitions[i].Type == Type)
                return &Definitions[i];
        }

        return nullptr;
    }

    void InitializeGovernmentEdictStates(
        std::vector<FGovernmentEdictState>& OutStates)
    {
        OutStates.clear();

        const auto& Definitions = GetGovernmentEdictDefinitions();
        OutStates.reserve(Definitions.size());

        for (size_t i = 0; i < Definitions.size(); ++i)
        {
            FGovernmentEdictState State;
            State.Type = Definitions[i].Type;
            OutStates.push_back(State);
        }
    }

    long long ResolveEdictActivationCost(
        const FGovernmentEdictDefinition& Definition,
        int ActiveCitizenCount)
    {
        const int SafeCitizenCount = (std::max)(0, ActiveCitizenCount);
        return Definition.BaseCost +
            Definition.CostPerCitizen * static_cast<long long>(SafeCitizenCount);
    }

    long long CalculateEdictDailyUpkeep(
        const std::vector<FGovernmentEdictState>& States,
        int DaysInMonth)
    {
        const int SafeDays = (std::max)(1, DaysInMonth);
        long long Total = 0;

        for (size_t i = 0; i < States.size(); ++i)
        {
            if (!States[i].Active)
                continue;

            const FGovernmentEdictDefinition* Definition =
                FindGovernmentEdictDefinition(States[i].Type);

            if (!Definition || Definition->MonthlyUpkeep <= 0)
                continue;

            const double DailyCost =
                static_cast<double>(Definition->MonthlyUpkeep) /
                static_cast<double>(SafeDays);
            Total += static_cast<long long>(std::round(DailyCost));
        }

        return Total;
    }

    FGovernmentEdictModifiers CalculateEdictModifiers(
        const std::vector<FGovernmentEdictState>& States,
        int ActiveCitizenCount)
    {
        FGovernmentEdictModifiers Modifiers;
        const int SafeCitizenCount = (std::max)(0, ActiveCitizenCount);

        for (size_t i = 0; i < States.size(); ++i)
        {
            if (!States[i].Active)
                continue;

            switch (States[i].Type)
            {
            case EGovernmentEdictType::FoodForThePeople:
                Modifiers.FoodConsumptionPerVisit =
                    (std::max)(Modifiers.FoodConsumptionPerVisit, 2);
                Modifiers.FoodGainMultiplier *= 1.18f;
                Modifiers.DailyFoodDelta += 3.5f;
                break;
            case EGovernmentEdictType::TaxCut:
                Modifiers.DailyFreedomDelta += 1.5f;
                Modifiers.DailyBudgetDelta -=
                    static_cast<long long>((std::max)(120, SafeCitizenCount * 2));
                break;
            case EGovernmentEdictType::MartialLaw:
                Modifiers.DailyFreedomDelta -= 4.0f;
                Modifiers.DailySecurityDelta += 5.0f;
                break;
            case EGovernmentEdictType::FreeHousing:
                Modifiers.DailyHousingDelta += 3.5f;
                Modifiers.DailyBudgetDelta -=
                    static_cast<long long>((std::max)(80, SafeCitizenCount));
                break;
            case EGovernmentEdictType::EmployeeOfTheMonth:
                Modifiers.ProductionMultiplier *= 1.35f;
                Modifiers.DailyJobDelta += 1.5f;
                break;
            default:
                break;
            }
        }

        return Modifiers;
    }

    FGovernmentActionRecord MakeGovernmentActionFromEdict(
        const FGovernmentEdictDefinition& Definition)
    {
        FGovernmentActionRecord Record;
        Record.Type = Definition.ActionType;
        Record.Label = Definition.DisplayName;
        Record.Strength = 1.f;
        Record.RemainingDays = -1;
        Record.Signals = Definition.Signals;
        return Record;
    }
}
