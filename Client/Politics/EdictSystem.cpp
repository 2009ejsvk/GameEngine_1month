#include "EdictSystem.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <initializer_list>

namespace
{
    constexpr const wchar_t* GPlaceholderSummary =
        L"아이콘과 시대 배치만 연결된 참고용 칙령입니다.";
    constexpr const wchar_t* GPlaceholderEffect =
        L"실제 효과와 적용 로직은 아직 연결되지 않았습니다.";

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
        Definition.Type = Type;
        Definition.Era = Era;
        Definition.DisplayName = DisplayName ? DisplayName : L"칙령";
        Definition.Summary = GPlaceholderSummary;
        Definition.EffectText = GPlaceholderEffect;
        Definition.IconPath = IconPath;
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
        Definition.IconPath = IconPath;
        Definition.Implemented = true;
        Definition.BaseCost = BaseCost;
        Definition.CostPerCitizen = CostPerCitizen;
        Definition.MonthlyUpkeep = MonthlyUpkeep;
        Definition.DurationDays = DurationDays;
        Definition.CooldownDays = CooldownDays;
        Definition.Signals.assign(Signals.begin(), Signals.end());
        return Definition;
    }
}

namespace EdictSystem
{
    const std::vector<FGovernmentEdictDefinition>&
        GetGovernmentEdictDefinitions()
    {
        static const std::vector<FGovernmentEdictDefinition> GDefinitions =
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
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_printMoney.png")),
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
                L"조기선거",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_earlyElections.png")),
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
                L"종을 총알로",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_bellsToBullets.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MilitaryPolice,
                EEdictEra::WorldWars,
                L"헌병대",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_militaryPolice.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::Volkswagen,
                EEdictEra::WorldWars,
                L"공짜 자동차",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_volksWagen.png")),
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
                L"핵실험",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_nuclearTesting.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::GoodOldDays,
                EEdictEra::ColdWar,
                L"옛날이 좋았지",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_goodOldDays.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::ExperimentalGroundTreatment,
                EEdictEra::ColdWar,
                L"실험적인 토양 처리법",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_experimentalGroundTreatment.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MandatoryWasteSorting,
                EEdictEra::ColdWar,
                L"분리수거 의무화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_mandatoryWasteSorting.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::DiplomaticSuperParty,
                EEdictEra::ColdWar,
                L"외교적 슈퍼 파티",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_diplomaticSuperParty.png")),
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
                L"트로피코산",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_MadeIn.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::SocialSecurity,
                EEdictEra::ColdWar,
                L"사회보장",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_socialSecurity.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::MoneyLaundering,
                EEdictEra::ColdWar,
                L"자금 세탁",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_moneylaundering3.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::HearTheCall,
                EEdictEra::ColdWar,
                L"부름을 들어라!",
                TEXT("TROPICO_ASSET\\Blueprints\\Edict\\DLC_Future\\T_ICO_edict_HearTheCall.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::LaborTaxRelief,
                EEdictEra::ColdWar,
                EGovernmentEdictMode::Active,
                EPoliticalActionType::LaborTaxRelief,
                L"근로세 경감",
                L"근로층 세금 파업에 대응해 소득세를 즉시 낮추고 고용 불만을 진정시킵니다.",
                L"근로층 세금 파업 발생 중에만 시행 가능, 소득세 4%p 인하, 파업 즉시 진정, 4개월 동안 직업/자유 완화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_taxCut.png"),
                2500,
                6,
                0,
                120,
                360,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 6.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, -2.5f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Left, 1.5f)
                }),
            MakeImplementedEdict(
                EGovernmentEdictType::PropertyTaxRelief,
                EEdictEra::ColdWar,
                EGovernmentEdictMode::Active,
                EPoliticalActionType::PropertyTaxRelief,
                L"재산세 유예",
                L"주거층 재산세 반발에 대응해 재산세를 낮추고 주거 불만을 완화합니다.",
                L"재산세 반발 발생 중에만 시행 가능, 재산세 10%p 인하, 반발 즉시 진정, 4개월 동안 주거 완화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_freeHousing.png"),
                3200,
                5,
                0,
                120,
                420,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 7.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, -3.0f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Right, 2.0f)
                }),

            MakePlaceholderEdict(
                EGovernmentEdictType::ContraceptionBan,
                EEdictEra::Modern,
                L"피임 금지",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_contraceptionBan.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::LegalizedSubstances,
                EEdictEra::Modern,
                L"약물 합법화",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_legalizedSubstances.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::CulturalDiversity,
                EEdictEra::Modern,
                L"다문화 정책",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_culturalDiversity.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::TaxHeaven,
                EEdictEra::Modern,
                L"조세 피난처",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_taxHeaven.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::KnowledgioSinco,
                EEdictEra::Modern,
                L"놀리지오 신코",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_knowledgioSinco.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::Speedway,
                EEdictEra::Modern,
                L"경주장",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_speedway.png")),
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
            MakePlaceholderEdict(
                EGovernmentEdictType::PolicyOfDetente,
                EEdictEra::Modern,
                L"데탕트 정책",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_policyOfDetente.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::CTPA,
                EEdictEra::Modern,
                L"카리브해 무역 조약 연합",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_CTPA.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::TourismState,
                EEdictEra::Modern,
                L"관광 국가",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_tourismstate.png")),
            MakePlaceholderEdict(
                EGovernmentEdictType::SoapOpera,
                EEdictEra::Modern,
                L"우주 연속극",
                TEXT("TROPICO_ASSET\\Blueprints\\Edict\\DLC_Future\\T_ICO_edict_SoapOpera.png")),
            MakeImplementedEdict(
                EGovernmentEdictType::EmergencyAusterity,
                EEdictEra::Modern,
                EGovernmentEdictMode::Active,
                EPoliticalActionType::AusterityProgram,
                L"긴축 예산",
                L"국고 위기 경보에 대응해 긴급 자금을 투입하고 단기 긴축 체제로 전환합니다.",
                L"국고 위기 발생 중에만 시행 가능, 즉시 자금 투입, 위기 즉시 진정, 4개월 동안 일일 재정 개선과 자유/직업 압박",
                TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\Edicts\\T_ICO_edicts_printMoney.png"),
                4500,
                4,
                0,
                120,
                480,
                {
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Left, 2.0f),
                    MakeSignal(EPoliticalAxis::Economy,
                        EPoliticalStance::Right, -4.5f),
                    MakeSignal(EPoliticalAxis::IntellectualConservative,
                        EPoliticalStance::Right, 5.5f)
                })
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
                Modifiers.TaxRevenueMultiplier *= 0.65f;
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
            case EGovernmentEdictType::LaborTaxRelief:
                Modifiers.DailyJobDelta += 1.25f;
                Modifiers.DailyFreedomDelta += 0.75f;
                break;
            case EGovernmentEdictType::PropertyTaxRelief:
                Modifiers.DailyHousingDelta += 1.40f;
                Modifiers.DailyFreedomDelta += 0.35f;
                break;
            case EGovernmentEdictType::EmergencyAusterity:
                Modifiers.DailyBudgetDelta += 450;
                Modifiers.DailyJobDelta -= 1.00f;
                Modifiers.DailyFreedomDelta -= 0.85f;
                Modifiers.DailySecurityDelta += 0.45f;
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
