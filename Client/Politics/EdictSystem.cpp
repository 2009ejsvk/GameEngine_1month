#include "EdictSystem.h"
#include "EdictSystemRuntime.h"
#include "EdictSystemSerialization.h"
#include "../RuntimeConfigRegistry.h"
#include <Windows.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <initializer_list>
#include <string>

namespace
{
    constexpr const wchar_t* GConfigId = L"Game.Edicts";
    constexpr const wchar_t* GPlaceholderSummary =
        L"아이콘과 시대 배치만 연결된 참고용 칙령입니다.";
    constexpr const wchar_t* GPlaceholderEffect =
        L"실제 효과와 적용 로직은 아직 연결되지 않았습니다.";
    std::vector<FGovernmentEdictDefinition> GDefinitions;
    bool GDefinitionsInitialized = false;

    bool TryBuildExpandedImplementedEdict(
        EGovernmentEdictType Type,
        EEdictEra Era,
        const wchar_t* DisplayName,
        const wchar_t* IconPath,
        FGovernmentEdictDefinition& OutDefinition);

    std::vector<FGovernmentEdictDefinition>
        BuildDefaultGovernmentEdictDefinitions();

    void AddFactionApproval(
        FGovernmentEdictEffectDefinition& Effect,
        EPoliticalFaction Faction,
        int Delta)
    {
        const int Index = static_cast<int>(Faction);

        if (Index < 0 || Index >= GPoliticalFactionCount)
            return;

        Effect.FactionApprovalDelta[static_cast<size_t>(Index)] += Delta;
    }

    FGovernmentEdictEffectDefinition BuildDefaultEdictEffectDefinition(
        EGovernmentEdictType Type)
    {
        FGovernmentEdictEffectDefinition Effect;

        switch (Type)
        {
        case EGovernmentEdictType::FoodForThePeople:
            Effect.FoodConsumptionPerVisitMin = 2;
            Effect.FoodGainMultiplier = 1.18f;
            Effect.DailyFoodDelta = 3.5f;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 8);
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, -5);
            break;
        case EGovernmentEdictType::ChurchFee:
            Effect.TaxRevenueMultiplier = 1.10f;
            Effect.DailyFreedomDelta = -0.45f;
            Effect.DailySecurityDelta = 0.25f;
            AddFactionApproval(Effect, EPoliticalFaction::Religious, 6);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, -4);
            break;
        case EGovernmentEdictType::NoFreeLunch:
            Effect.DailyBudgetDeltaFlat = 140;
            Effect.DailyFoodDelta = -1.50f;
            Effect.DailyFreedomDelta = -0.20f;
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Communists, -6);
            break;
        case EGovernmentEdictType::NothingBeatsSiesta:
            Effect.ProductionMultiplier = 0.88f;
            Effect.DailyJobDelta = 0.55f;
            Effect.DailyFreedomDelta = 0.60f;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 3);
            AddFactionApproval(Effect, EPoliticalFaction::Industrialists, -4);
            break;
        case EGovernmentEdictType::UrbanDevelopment:
            Effect.DailyHousingDelta = 2.20f;
            Effect.DailyFreedomDelta = -0.30f;
            Effect.DailyBudgetDeltaFlat = -120;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, 2);
            break;
        case EGovernmentEdictType::TaxCut:
            Effect.DailyFreedomDelta = 1.5f;
            Effect.TaxRevenueMultiplier = 0.65f;
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 9);
            AddFactionApproval(Effect, EPoliticalFaction::Communists, -7);
            break;
        case EGovernmentEdictType::DiplomaticSuperParty:
            Effect.DailyFreedomDelta = 0.25f;
            Effect.DailyBudgetDeltaFlat = -80;
            Effect.TouristChanceBonusPercent = 10;
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 3);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, 4);
            break;
        case EGovernmentEdictType::AgriculturalSubsidies:
            Effect.ProductionMultiplier = 1.12f;
            Effect.DailyFoodDelta = 1.35f;
            Effect.DailyBudgetDeltaFlat = -160;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 2);
            AddFactionApproval(Effect, EPoliticalFaction::Religious, 2);
            AddFactionApproval(Effect, EPoliticalFaction::Industrialists, 2);
            break;
        case EGovernmentEdictType::BuildingPermit:
            Effect.TaxRevenueMultiplier = 1.10f;
            Effect.DailyFreedomDelta = -0.80f;
            Effect.DailySecurityDelta = 0.20f;
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 3);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, -3);
            break;
        case EGovernmentEdictType::WealthTax:
            Effect.TaxRevenueMultiplier = 1.14f;
            Effect.DailyFreedomDelta = -0.35f;
            Effect.DailyHousingDelta = 0.45f;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 6);
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, -7);
            break;
        case EGovernmentEdictType::Industrialization:
            Effect.ProductionMultiplier = 1.18f;
            Effect.DailyJobDelta = 0.90f;
            Effect.DailyFreedomDelta = -0.35f;
            AddFactionApproval(Effect, EPoliticalFaction::Industrialists, 7);
            AddFactionApproval(Effect, EPoliticalFaction::Environmentalists, -6);
            break;
        case EGovernmentEdictType::LiteracyProgram:
            Effect.ProductionMultiplier = 1.06f;
            Effect.DailyJobDelta = 0.95f;
            Effect.DailyFreedomDelta = 0.45f;
            Effect.DailyBudgetDeltaFlat = -110;
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, 7);
            break;
        case EGovernmentEdictType::MartialLaw:
            Effect.DailyFreedomDelta = -4.0f;
            Effect.DailySecurityDelta = 5.0f;
            Effect.TouristChanceBonusPercent = -10;
            AddFactionApproval(Effect, EPoliticalFaction::Militarists, 9);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, -9);
            AddFactionApproval(Effect, EPoliticalFaction::Environmentalists, -4);
            break;
        case EGovernmentEdictType::Prohibition:
            Effect.DailySecurityDelta = 2.10f;
            Effect.DailyFreedomDelta = -1.40f;
            Effect.DailyJobDelta = -0.20f;
            AddFactionApproval(Effect, EPoliticalFaction::Religious, 6);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, -5);
            break;
        case EGovernmentEdictType::MilitaryPolice:
            Effect.DailySecurityDelta = 2.90f;
            Effect.DailyFreedomDelta = -2.10f;
            Effect.DailyBudgetDeltaFlat = -90;
            Effect.TouristChanceBonusPercent = -6;
            AddFactionApproval(Effect, EPoliticalFaction::Militarists, 8);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, 3);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, -7);
            break;
        case EGovernmentEdictType::RightToArms:
            Effect.DailyFreedomDelta = 1.45f;
            Effect.DailySecurityDelta = -1.10f;
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 5);
            AddFactionApproval(Effect, EPoliticalFaction::Militarists, 3);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Communists, -4);
            break;
        case EGovernmentEdictType::FreeHousing:
            Effect.DailyHousingDelta = 3.5f;
            Effect.DailyBudgetDeltaPerCitizen = -1;
            Effect.DailyBudgetDeltaPerCitizenAbsMinimum = 80;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 10);
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, -8);
            break;
        case EGovernmentEdictType::EmployeeOfTheMonth:
            Effect.ProductionMultiplier = 1.35f;
            Effect.DailyJobDelta = 1.5f;
            AddFactionApproval(Effect, EPoliticalFaction::Industrialists, 6);
            break;
        case EGovernmentEdictType::TaxHeaven:
            Effect.TaxRevenueMultiplier = 1.06f;
            Effect.DailyFreedomDelta = -0.20f;
            Effect.DailyBudgetDeltaFlat = 110;
            Effect.TouristChanceBonusPercent = 4;
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 8);
            AddFactionApproval(Effect, EPoliticalFaction::Industrialists, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Communists, -6);
            break;
        case EGovernmentEdictType::PolicyOfDetente:
            Effect.DailyFreedomDelta = 0.15f;
            Effect.DailySecurityDelta = 0.20f;
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, 5);
            AddFactionApproval(Effect, EPoliticalFaction::Environmentalists, 3);
            AddFactionApproval(Effect, EPoliticalFaction::Militarists, -4);
            break;
        case EGovernmentEdictType::CTPA:
            Effect.ProductionMultiplier = 1.03f;
            Effect.DailyBudgetDeltaFlat = 90;
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Industrialists, 7);
            AddFactionApproval(Effect, EPoliticalFaction::Communists, -3);
            break;
        case EGovernmentEdictType::TourismState:
            Effect.DailyFreedomDelta = 0.20f;
            Effect.DailyBudgetDeltaFlat = 120;
            Effect.TouristChanceBonusPercent = 14;
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, 2);
            break;
        case EGovernmentEdictType::LaborTaxRelief:
            Effect.DailyJobDelta = 1.25f;
            Effect.DailyFreedomDelta = 0.75f;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 5);
            AddFactionApproval(Effect, EPoliticalFaction::Industrialists, -3);
            break;
        case EGovernmentEdictType::PropertyTaxRelief:
            Effect.DailyHousingDelta = 1.40f;
            Effect.DailyFreedomDelta = 0.35f;
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Conservatives, 2);
            AddFactionApproval(Effect, EPoliticalFaction::Communists, -3);
            break;
        case EGovernmentEdictType::EmergencyAusterity:
            Effect.DailyBudgetDeltaFlat = 450;
            Effect.DailyJobDelta = -1.0f;
            Effect.DailyFreedomDelta = -0.85f;
            Effect.DailySecurityDelta = 0.45f;
            Effect.TouristChanceBonusPercent = -8;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, -8);
            AddFactionApproval(Effect, EPoliticalFaction::Intellectuals, -4);
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 3);
            break;
        case EGovernmentEdictType::LaborNegotiation:
            Effect.DailyJobDelta = 1.5f;
            Effect.DailyFreedomDelta = 0.5f;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 6);
            AddFactionApproval(Effect, EPoliticalFaction::Industrialists, -3);
            break;
        case EGovernmentEdictType::EmergencyWelfareSupport:
            Effect.DailyFoodDelta = 2.0f;
            Effect.DailyHousingDelta = 1.5f;
            Effect.DailyFunDelta = 1.5f;
            AddFactionApproval(Effect, EPoliticalFaction::Communists, 5);
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, -3);
            break;
        case EGovernmentEdictType::AdvancedBoatServices:
            Effect.HarborUpkeepMultiplier = 1.20f;
            Effect.HarborShipProgressMultiplier = 1.25f;
            AddFactionApproval(Effect, EPoliticalFaction::Capitalists, 4);
            AddFactionApproval(Effect, EPoliticalFaction::Industrialists, 3);
            break;
        default:
            break;
        }

        return Effect;
    }

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

    FGovernmentEdictDefinition MakePlaceholderEdict(
        EGovernmentEdictType Type,
        EEdictEra Era,
        const wchar_t* DisplayName,
        const wchar_t* IconPath)
    {
        FGovernmentEdictDefinition Definition;

        if (TryBuildExpandedImplementedEdict(
                Type,
                Era,
                DisplayName,
                IconPath,
                Definition))
        {
            return Definition;
        }

        Definition.Type = Type;
        Definition.Era = Era;
        Definition.DisplayName = DisplayName ? DisplayName : L"칙령";
        Definition.Summary = GPlaceholderSummary;
        Definition.EffectText = GPlaceholderEffect;
        Definition.IconPath = IconPath ? IconPath : L"";
        Definition.Effect = BuildDefaultEdictEffectDefinition(Type);
        return Definition;
    }

    FGovernmentEdictDefinition MakeImplementedEdict(
        EGovernmentEdictType Type,
        EEdictEra Era,
        EGovernmentEdictMode Mode,
        EPoliticalActionType ActionType,
        const wchar_t* DisplayName,
        const wchar_t* Summary,
        const wchar_t* EffectText,
        const wchar_t* IconPath,
        long long BaseCost,
        long long CostPerCitizen,
        long long MonthlyUpkeep,
        int DurationDays,
        int CooldownDays,
        std::initializer_list<FPoliticalSignalDef> Signals = {})
    {
        FGovernmentEdictDefinition Definition;
        Definition.Type = Type;
        Definition.Era = Era;
        Definition.Mode = Mode;
        Definition.ActionType = ActionType;
        Definition.DisplayName = DisplayName ? DisplayName : L"칙령";
        Definition.Summary = Summary ? Summary : L"";
        Definition.EffectText = EffectText ? EffectText : L"";
        Definition.IconPath = IconPath ? IconPath : L"";
        Definition.Implemented = true;
        Definition.BaseCost = BaseCost;
        Definition.CostPerCitizen = CostPerCitizen;
        Definition.MonthlyUpkeep = MonthlyUpkeep;
        Definition.DurationDays = DurationDays;
        Definition.CooldownDays = CooldownDays;
        Definition.Effect = BuildDefaultEdictEffectDefinition(Type);
        Definition.Signals.assign(Signals.begin(), Signals.end());
        return Definition;
    }

    bool TryBuildExpandedImplementedEdict(
        EGovernmentEdictType Type,
        EEdictEra Era,
        const wchar_t* DisplayName,
        const wchar_t* IconPath,
        FGovernmentEdictDefinition& OutDefinition)
    {
        switch (Type)
        {
        case EGovernmentEdictType::ChurchFee:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"예배 비용을 징수해 재정을 늘리지만 자유가 낮아집니다.",
                L"세수 증가, 자유 소폭 하락, 치안 소폭 상승",
                IconPath,
                800,
                0,
                120,
                0,
                0);
            return true;
        case EGovernmentEdictType::NoFreeLunch:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"복지 부담을 줄여 예산을 아끼지만 음식 불만이 커집니다.",
                L"일일 재정 개선, 음식 만족도 하락, 자유 소폭 하락",
                IconPath,
                500,
                0,
                0,
                0,
                0);
            return true;
        case EGovernmentEdictType::NothingBeatsSiesta:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"근무 중 휴식을 보장해 여유를 늘리지만 생산량은 줄어듭니다.",
                L"생산량 감소, 직업 만족도 상승, 자유 상승",
                IconPath,
                600,
                0,
                50,
                0,
                0);
            return true;
        case EGovernmentEdictType::UrbanDevelopment:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"도시 확장에 예산을 투입해 주거 환경을 개선합니다.",
                L"주거 만족도 상승, 자유 소폭 하락, 월간 유지비 발생",
                IconPath,
                1400,
                0,
                280,
                0,
                0);
            return true;
        case EGovernmentEdictType::AgriculturalSubsidies:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"농업과 식량 공급에 보조금을 투입해 생산을 끌어올립니다.",
                L"생산량 증가, 음식 만족도 상승, 예산 부담 증가",
                IconPath,
                1800,
                0,
                520,
                0,
                0);
            return true;
        case EGovernmentEdictType::BuildingPermit:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"허가 수수료를 통해 세수를 늘리는 대신 절차적 자유를 낮춥니다.",
                L"세수 증가, 자유 하락, 치안 소폭 상승",
                IconPath,
                900,
                0,
                0,
                0,
                0);
            return true;
        case EGovernmentEdictType::WealthTax:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"부유층에 더 무거운 세금을 매겨 국가 재정을 강화합니다.",
                L"세수 증가, 자유 소폭 하락, 주거 만족도 소폭 상승",
                IconPath,
                1300,
                0,
                0,
                0,
                0);
            return true;
        case EGovernmentEdictType::Industrialization:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"산업 확장에 집중해 생산성과 고용 만족을 높입니다.",
                L"생산량 상승, 직업 만족도 상승, 자유 소폭 하락",
                IconPath,
                2600,
                0,
                450,
                0,
                0);
            return true;
        case EGovernmentEdictType::LiteracyProgram:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"기초 교육을 확장해 노동 품질과 시민 의식을 개선합니다.",
                L"생산량 소폭 상승, 직업 만족도 상승, 자유 상승",
                IconPath,
                1700,
                0,
                360,
                0,
                0);
            return true;
        case EGovernmentEdictType::Prohibition:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"음주를 강하게 제한해 치안을 높이지만 자유는 떨어집니다.",
                L"치안 상승, 자유 하락, 직업 만족도 소폭 하락",
                IconPath,
                950,
                0,
                80,
                0,
                0);
            return true;
        case EGovernmentEdictType::MilitaryPolice:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"군 병력을 동원해 질서를 강화하지만 자유 억압이 커집니다.",
                L"치안 크게 상승, 자유 하락, 월간 유지비 발생",
                IconPath,
                2400,
                0,
                620,
                0,
                0);
            return true;
        case EGovernmentEdictType::RightToArms:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"개인 무장 권리를 보장해 자유를 높이지만 치안은 흔들립니다.",
                L"자유 상승, 치안 하락",
                IconPath,
                1100,
                0,
                0,
                0,
                0);
            return true;
        case EGovernmentEdictType::AdvancedBoatServices:
            OutDefinition = MakeImplementedEdict(
                Type,
                Era,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                DisplayName,
                L"항구 시설의 유지비가 오르지만 선박 속도가 빨라져 수출이 빈번해집니다.",
                L"항구 건물 유지비 +20%, 수출 선박 속도 +25%",
                IconPath,
                2500,
                0,
                180,
                0,
                0);
            return true;
        default:
            break;
        }

        return false;
    }
}

namespace
{
    std::vector<FGovernmentEdictDefinition> BuildDefaultGovernmentEdictDefinitions()
    {
        std::vector<FGovernmentEdictDefinition> Definitions =
        {
            MakeImplementedEdict(
                EGovernmentEdictType::FoodForThePeople,
                EEdictEra::Colonial,
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
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::ChurchFee,
                EEdictEra::Colonial,
                L"종교세",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_ChurchFee.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::NoFreeLunch,
                EEdictEra::Colonial,
                L"공짜 점심 없음",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_NoFreeLunch.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::NothingBeatsSiesta,
                EEdictEra::Colonial,
                L"의무 낮잠",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_NothingBeatsSiesta.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::UrbanDevelopment,
                EEdictEra::Colonial,
                L"도시 계획",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_urbanDevelopment.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::PenalColony,
                EEdictEra::Colonial,
                L"유형지",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_australianStyle.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::ChildAllowances,
                EEdictEra::Colonial,
                L"아동 수당",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_childAllowances.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::EmployeeOfTheMonth,
                EEdictEra::Colonial,
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
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::AdvancedBoatServices,
                EEdictEra::Colonial,
                L"고급 보트 서비스",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_advancedBoatServices.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::FreeHousing,
                EEdictEra::Colonial,
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
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::SpecialTreats,
                EEdictEra::Colonial,
                L"펜하우저 스페셜",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_specialTreats.png")),

            MakePlaceholderEdict(
                EGovernmentEdictType::Audience,
                EEdictEra::WorldWars,
                L"청문회",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_Audience.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::AgriculturalSubsidies,
                EEdictEra::WorldWars,
                L"농업보조금",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_agriculturalSubsidies.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::BuildingPermit,
                EEdictEra::WorldWars,
                L"건축 허가제",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_buildingPermit.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::StateLoans,
                EEdictEra::WorldWars,
                L"국가 대출",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_stateLoans.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::WealthTax,
                EEdictEra::WorldWars,
                L"부유세",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_wealthTax.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::Industrialization,
                EEdictEra::WorldWars,
                L"산업화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_industrialization.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::EarlyElections,
                EEdictEra::WorldWars,
                L"조기 선거",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_earlyElections.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::LiteracyProgram,
                EEdictEra::WorldWars,
                L"장학제도",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_literacyProgram.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::MartialLaw,
                EEdictEra::WorldWars,
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
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::Prohibition,
                EEdictEra::WorldWars,
                L"금주법",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_prohibition.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::BellsToBullets,
                EEdictEra::WorldWars,
                L"총알로 만든 종",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_bellsToBullets.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MilitaryPolice,
                EEdictEra::WorldWars,
                L"헌병대",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_militaryPolice.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::Volkswagen,
                EEdictEra::WorldWars,
                L"국민차 보급",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_volksWagen.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::RightToArms,
                EEdictEra::WorldWars,
                L"총기 소지 권리",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_rightToArms.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::FestivalBoost,
                EEdictEra::WorldWars,
                L"축제 부스트",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_festivalboost.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MardiGras,
                EEdictEra::WorldWars,
                L"마르디 그라",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_mardiGras.png")),

            MakePlaceholderEdict(
                EGovernmentEdictType::NuclearTesting,
                EEdictEra::ColdWar,
                L"핵 실험",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_nuclearTesting.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::GoodOldDays,
                EEdictEra::ColdWar,
                L"좋았던 옛 시절",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_goodOldDays.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::ExperimentalGroundTreatment,
                EEdictEra::ColdWar,
                L"실험적인 토양 처리법",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_experimentalGroundTreatment.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MandatoryWasteSorting,
                EEdictEra::ColdWar,
                L"쓰레기 분리수거 의무화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_mandatoryWasteSorting.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::DiplomaticSuperParty,
                EEdictEra::ColdWar,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                L"외교적 슈퍼 파티",
                L"대외 이미지를 개선해 주요 강대국과의 교류 창구를 넓힙니다.",
                L"외교 무역 보정 상승, 수출 단가 소폭 상승, 수입 단가 소폭 완화, 월 유지비 발생",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_diplomaticSuperParty.png"),
                3500,
                0,
                180,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, 4.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 2.0f)
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::HappyMeat,
                EEdictEra::ColdWar,
                L"자연방목 육류",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_happyMeal.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::TaxCut,
                EEdictEra::ColdWar,
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
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::AlternativeFoodSource,
                EEdictEra::ColdWar,
                L"대체 식량원",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_alternativeFoodSource.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::SpellingBee,
                EEdictEra::ColdWar,
                L"철자 대회",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_spellingBee.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::AssemblyBan,
                EEdictEra::ColdWar,
                L"집회 금지",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_assemblyBan.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::NationalDay,
                EEdictEra::ColdWar,
                L"국경일",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_naionalDay.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MadeInTropico,
                EEdictEra::ColdWar,
                L"트로피코산 제품",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_madeInTropico.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::SocialSecurity,
                EEdictEra::ColdWar,
                L"사회 보장",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_socialSecurity.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MoneyLaundering,
                EEdictEra::ColdWar,
                L"자금 세탁",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_moneyLaundering.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::HearTheCall,
                EEdictEra::Modern,
                L"부름에 응하라",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_hearTheCall.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::LaborTaxRelief,
                EEdictEra::ColdWar,
                L"근로세 경감",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_taxCut.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::PropertyTaxRelief,
                EEdictEra::ColdWar,
                L"재산세 유예",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_freeHousing.png")),

            MakePlaceholderEdict(
                EGovernmentEdictType::ContraceptionBan,
                EEdictEra::Modern,
                L"피임 금지",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_contraceptionBan.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::LegalizedSubstances,
                EEdictEra::Modern,
                L"마약 합법화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_legalizedSubstances.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::CulturalDiversity,
                EEdictEra::Modern,
                L"문화 다양성",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_culturalDiversity.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::TaxHeaven,
                EEdictEra::Modern,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                L"조세 피난처",
                L"해외 자본과 고부가 상품 무역을 끌어들여 수출 수익을 높입니다.",
                L"가공재/사치재 수출 단가 상승, 일부 수입 단가 완화, 세수 소폭 증가, 월 유지비 발생",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_taxHeaven.png"),
                6000,
                0,
                260,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, -4.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, 6.5f)
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::KnowledgioSinco,
                EEdictEra::Modern,
                L"놀리지오 신코",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_knowledgioSinco.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::Speedway,
                EEdictEra::Modern,
                L"과속 도로",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_speedway.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::CompulsoryVaccination,
                EEdictEra::Modern,
                L"예방 접종 의무화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_compulsoryVaccination.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::LightBulbBan,
                EEdictEra::Modern,
                L"전구 금지",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_lightBulbBan.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::SeaDisposal,
                EEdictEra::Modern,
                L"해양처분",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_seaDisposal.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::PolicyOfDetente,
                EEdictEra::Modern,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                L"데탕트 정책",
                L"긴장을 완화하고 교역 조건을 개선해 원자재와 공산품 거래를 유리하게 만듭니다.",
                L"외교 관계 완화, 원자재/가공재 수출입 보정, 치안 압박 소폭 완화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_policyOfDetente.png"),
                4200,
                0,
                160,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::ReligionMilitarism,
                        EPoliticalStance::Right, -3.5f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, 3.0f)
                }),
            MakeImplementedEdict(
                EGovernmentEdictType::CTPA,
                EEdictEra::Modern,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                L"카리브해 무역 조약 연합",
                L"지역 무역 협정에 참여해 전반적인 수출입 조건을 개선합니다.",
                L"전반적 수출입 우대, 제조·사치재 수출 보너스, 월 협정 비용 발생",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_CTPA.png"),
                5200,
                0,
                220,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 4.0f),
                    MakeSignal(EPoliticalAxis::EnvironmentIndustry,
                        EPoliticalStance::Right, 2.0f)
                }),
            MakeImplementedEdict(
                EGovernmentEdictType::TourismState,
                EEdictEra::Modern,
                EGovernmentEdictMode::Passive,
                EPoliticalActionType::None,
                L"관광 국가",
                L"관광 친화 노선으로 외화를 끌어들이지만 소비재 수요가 함께 늘어납니다.",
                L"사치재 수출 단가 상승, 관광 소비재 수입 비용 상승, 월 유지비 발생",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_tourismstate.png"),
                4800,
                0,
                190,
                0,
                0,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 2.5f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, 2.5f)
                }),
            MakePlaceholderEdict(
                EGovernmentEdictType::SoapOpera,
                EEdictEra::Modern,
                L"막장 드라마",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_soapOpera.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::EmergencyAusterity,
                EEdictEra::Modern,
                L"긴축 예산",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_printMoney.png")),

            MakePlaceholderEdict(
                EGovernmentEdictType::LaborNegotiation,
                EEdictEra::ColdWar,
                L"노동자 협상",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_taxCut.png")),

            MakePlaceholderEdict(
                EGovernmentEdictType::EmergencyWelfareSupport,
                EEdictEra::WorldWars,
                L"긴급 민생 지원",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_freeHousing.png"))
        };

        return Definitions;
    }

    void ResetEdictDefinitionsToDefaults()
    {
        GDefinitions = BuildDefaultGovernmentEdictDefinitions();
        GDefinitionsInitialized = true;
    }

    bool LoadEdictDefinitionsFromRuntimeConfig(const std::wstring& Path)
    {
        if (!GDefinitionsInitialized)
            ResetEdictDefinitionsToDefaults();

        return EdictSystemSerialization::LoadEdictDefinitionsFromFile(
            Path,
            GDefinitions);
    }

    void EnsureDefaultEdictConfigFileExists(const std::wstring& Path)
    {
        const DWORD Attributes = GetFileAttributesW(Path.c_str());

        if (Attributes != INVALID_FILE_ATTRIBUTES &&
            (Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            return;
        }

        if (!GDefinitionsInitialized)
            ResetEdictDefinitionsToDefaults();

        EdictSystemSerialization::WriteEdictDefinitionsToFile(
            Path,
            GDefinitions);
    }
}

namespace EdictSystem
{
    const std::vector<FGovernmentEdictDefinition>&
        GetGovernmentEdictDefinitions()
    {
        if (!GDefinitionsInitialized)
        {
            GDefinitions = BuildDefaultGovernmentEdictDefinitions();
            GDefinitionsInitialized = true;
        }

        return GDefinitions;
    }

    void RegisterRuntimeConfig()
    {
        const std::wstring ConfigPath =
            RuntimeConfigRegistry::BuildExeRelativePath(L"Edicts.ini");
        EnsureDefaultEdictConfigFileExists(ConfigPath);
        RuntimeConfigRegistry::RegisterSource(
            {
                GConfigId,
                ConfigPath,
                0.5f,
                &ResetEdictDefinitionsToDefaults,
                &LoadEdictDefinitionsFromRuntimeConfig,
                nullptr
            });
    }

    unsigned long long GetRuntimeConfigGeneration()
    {
        return RuntimeConfigRegistry::GetSourceGeneration(GConfigId);
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
        EdictSystemRuntime::InitializeGovernmentEdictStates(
            GetGovernmentEdictDefinitions(),
            OutStates);
    }

    void SynchronizeGovernmentEdictStates(
        std::vector<FGovernmentEdictState>& InOutStates)
    {
        EdictSystemRuntime::SynchronizeGovernmentEdictStates(
            GetGovernmentEdictDefinitions(),
            InOutStates);
    }

    long long ResolveEdictActivationCost(
        const FGovernmentEdictDefinition& Definition,
        int ActiveCitizenCount)
    {
        return EdictSystemRuntime::ResolveEdictActivationCost(
            Definition,
            ActiveCitizenCount);
    }

    long long CalculateEdictDailyUpkeep(
        const std::vector<FGovernmentEdictState>& States,
        int DaysInMonth)
    {
        return EdictSystemRuntime::CalculateEdictDailyUpkeep(
            GetGovernmentEdictDefinitions(),
            States,
            DaysInMonth);
    }

    FGovernmentEdictModifiers CalculateEdictModifiers(
        const std::vector<FGovernmentEdictState>& States,
        int ActiveCitizenCount)
    {
        return EdictSystemRuntime::CalculateEdictModifiers(
            GetGovernmentEdictDefinitions(),
            States,
            ActiveCitizenCount);
    }

    FGovernmentActionRecord MakeGovernmentActionFromEdict(
        const FGovernmentEdictDefinition& Definition)
    {
        return EdictSystemRuntime::MakeGovernmentActionFromEdict(Definition);
    }
}
