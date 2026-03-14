#include "UIStrings.h"
#include <Windows.h>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>

namespace UIStrings
{
    namespace
    {
        struct FDefaultEntry
        {
            const wchar_t* Key;
            const wchar_t* Value;
        };

        const FDefaultEntry GDefaultEntries[] =
        {
            { L"top_hud.menu.mission", L"임무" },
            { L"top_hud.menu.construction", L"건설" },
            { L"top_hud.menu.edict", L"칙령" },
            { L"top_hud.menu.constitution", L"헌법" },
            { L"top_hud.menu.trade", L"무역" },
            { L"top_hud.menu.raid", L"원정" },
            { L"top_hud.menu.research", L"연구" },
            { L"top_hud.menu.almanac", L"연감" },
            { L"top_hud.label.budget", L"예산" },
            { L"top_hud.label.population", L"인구" },
            { L"top_hud.label.support", L"지지율" },
            { L"top_hud.placeholder.election", L"차기 선거 -" },
            { L"top_hud.placeholder.tax_policy", L"세금 -" },
            { L"top_hud.placeholder.event_stable", L"현재 상태 안정" },
            { L"top_hud.game_over.title", L"정권 상실" },
            { L"top_hud.election.defeat", L"선거 패배" },
            { L"top_hud.fragment.next_election_prefix", L"차기 선거 " },
            { L"top_hud.fragment.tax_policy_prefix", L"세금 " },
            { L"top_hud.fragment.separator", L" | " },
            { L"top_hud.fragment.day_suffix", L"일" },
            { L"top_hud.fragment.last_win", L"직전 승리" },
            { L"top_hud.fragment.last_loss", L"직전 패배" },
            { L"top_hud.fragment.warning_prefix", L"경고 " },
            { L"top_hud.fragment.recent_warning_prefix", L"최근 경고 | " },
            { L"top_hud.event.election_warning", L"선거 경고 | 지지 기반 흔들림" },
            { L"top_hud.warning.high", L"재선 위험 높음" },
            { L"top_hud.warning.caution", L"재선 주의" },
            { L"top_hud.warning.check", L"선거 점검" },
            { L"top_hud.warning.stable", L"안정" },
            { L"top_hud.game_over.body_template",
                L"{0} 선거에서 재집권에 실패했습니다.\n"
                L"지지 {1} / 야당 {2} / 기권 {3}\n"
                L"득표율 {4}% / 투표율 {5}%\n"
                L"시뮬레이션이 정지되었습니다." },

            { L"edict.category.colonial", L"식민지 시대" },
            { L"edict.category.world_wars", L"세계대전 시대" },
            { L"edict.category.cold_war", L"냉전 시대" },
            { L"edict.category.modern", L"현대 시대" },
            { L"edict.category.fallback", L"칙령" },
            { L"edict.detail.default_title", L"칙령 선택" },
            { L"edict.detail.default_info", L"왼쪽 카드에서 칙령을 선택하세요." },
            { L"edict.detail.default_body",
                L"카드를 고르면 효과와 제약 조건이 아래 패널에 표시됩니다.\n"
                L"현재 시행 중인 칙령은 금색 강조와 체크 표시로 구분됩니다." },
            { L"edict.detail.panel_title", L"칙령 정보" },
            { L"edict.detail.placeholder", L"칙령을 클릭하면 상세 정보가 표시됩니다." },
            { L"edict.action.apply", L"시행" },
            { L"edict.action.active", L"활성" },
            { L"edict.action.waiting", L"대기" },
            { L"edict.action.preparing", L"준비 중" },
            { L"edict.action.requirement", L"조건 필요" },
            { L"edict.action.budget_shortage", L"예산 부족" },
            { L"edict.action.no_info", L"정보 없음" },
            { L"edict.tax_policy.title", L"세금 정책" },
            { L"edict.tax_policy.summary_pending", L"세금 보고 준비 중" },
            { L"edict.tax_policy.summary_apply_next", L"다음 일일 정산부터 반영" },
            { L"edict.tax_policy.daily_income_template",
                L"오늘 세수 {0}\n다음 일일 정산부터 반영" },
            { L"edict.tax_policy.active_event_template",
                L"\n활성 사건: {0} ({1}일)" },
            { L"edict.feedback.no_admin", L"지금은 행정 정보를 확인할 수 없습니다." },
            { L"edict.feedback.select_edict", L"먼저 칙령을 선택하세요." },
            { L"edict.status.checking", L"상태 확인 중" },
            { L"edict.status.preparing", L"준비 중" },
            { L"edict.status.active", L"활성" },
            { L"edict.status.available", L"사용 가능" },
            { L"edict.status.can_apply", L"시행 가능" },
            { L"edict.status.requirement_missing", L"조건 미충족" },
            { L"edict.status.budget_shortage", L"예산 부족" },
            { L"edict.status.active_template", L"시행 중 ({0}일 남음)" },
            { L"edict.status.cooldown_template", L"재사용 대기 ({0}일)" },
            { L"edict.requirement.not_implemented", L"아직 구현되지 않은 칙령입니다." },
            { L"edict.requirement.event_prefix", L"대응 사건 필요: " },
            { L"edict.requirement.unmet_prefix", L"미충족: " },
            { L"edict.requirement.current_event_prefix", L"미충족: 현재 사건은 " },
            { L"edict.requirement.event_needed_suffix", L" 발생 필요" },
            { L"edict.detail.reference_only", L"준비 중  |  참고용 칙령" },
            { L"edict.detail.reference_body",
                L"아이콘과 시대 배치만 연결된 칙령입니다.\n"
                L"실제 효과와 적용 로직은 아직 연결되지 않았습니다." },
            { L"edict.detail.not_implemented", L"미구현: 아직 게임 로직이 연결되지 않았습니다." },
            { L"edict.detail.passive", L"상시 칙령" },
            { L"edict.detail.active", L"기간 칙령" },
            { L"edict.detail.monthly_upkeep_prefix", L"\n\n매달 유지비 " },
            { L"edict.detail.monthly_upkeep_suffix", L"이 소요됩니다." },
            { L"edict.detail.duration_prefix", L"  |  지속 " },
            { L"edict.detail.duration_suffix", L"개월" },
            { L"edict.detail.cooldown_prefix", L"  |  재사용 " },
            { L"edict.detail.cooldown_suffix", L"개월" },
            { L"edict.detail.required_event_prefix", L"\n\n필요 사건: " },
            { L"edict.detail.event_ready", L"  |  대응 가능" },
            { L"edict.tax_policy.active_event_prefix", L"\n활성 사건: " },
            { L"edict.tax_policy.active_event_suffix", L"일)" },
            { L"edict.tax_event.worker_tax_strike", L"근로층 세금 파업" },
            { L"edict.tax_event.property_tax_backlash", L"재산세 반발" },
            { L"edict.tax_event.budget_crisis", L"국고 위기" },
            { L"edict.tax_event.generic", L"세금 사건" },

            { L"citizen_info.tab.citizen.overview", L"기본" },
            { L"citizen_info.tab.citizen.politics", L"정치" },
            { L"citizen_info.tab.citizen.thoughts", L"성향" },
            { L"citizen_info.page.citizen.overview", L"기본" },
            { L"citizen_info.page.citizen.politics", L"정치" },
            { L"citizen_info.page.citizen.thoughts", L"성향" },
            { L"citizen_info.tab.building.overview", L"설정" },
            { L"citizen_info.tab.building.statistics", L"상태" },
            { L"citizen_info.tab.building.upgrades", L"개선" },
            { L"citizen_info.tab.building.efficiency", L"효율" },
            { L"citizen_info.tab.building.information", L"설명" },
            { L"citizen_info.page.building.overview", L"설정" },
            { L"citizen_info.page.building.statistics", L"상태" },
            { L"citizen_info.page.building.upgrades", L"개선" },
            { L"citizen_info.page.building.efficiency", L"효율" },
            { L"citizen_info.page.building.information", L"설명" },
            { L"citizen_info.politics.section.satisfaction", L"만족도" },
            { L"citizen_info.politics.section.opinion", L"견해" },
            { L"citizen_info.politics.section.support", L"지지도" },
            { L"citizen_info.thought.title", L"생각" },
            { L"citizen_info.action.demolish", L"철거" },
            { L"citizen_info.action.move", L"이동" },
            { L"citizen_info.action.relocate", L"재배치" },
            { L"citizen_info.action.focus", L"포커스" },
            { L"citizen_info.building.data_pending", L"건물 데이터 준비 중입니다." },
            { L"citizen_info.activity.move", L"이동" },
            { L"citizen_info.subtitle.tropican", L"↠ 트로피코인 ↞" },
            { L"citizen_info.subtitle.tourist", L"↠ 관광객 ↞" },

            { L"citizen.education.uneducated", L"무학력" },
            { L"citizen.education.high_school", L"고등학교" },
            { L"citizen.education.college", L"대학" },
            { L"citizen.wealth.poor", L"가난" },
            { L"citizen.wealth.well_off", L"유복" },
            { L"citizen.wealth.rich", L"부유" },
            { L"citizen.politics.axis.economy", L"경제" },
            { L"citizen.politics.axis.religion_militarism", L"종교·군국" },
            { L"citizen.politics.axis.environment_industry", L"환경·산업" },
            { L"citizen.politics.axis.intellectual_conservative", L"지식·보수" },
            { L"citizen.politics.axis.default", L"정치" },
            { L"citizen.politics.faction.economy.left", L"공산" },
            { L"citizen.politics.faction.economy.right", L"자본" },
            { L"citizen.politics.faction.religion_militarism.left", L"종교" },
            { L"citizen.politics.faction.religion_militarism.right", L"군부" },
            { L"citizen.politics.faction.environment_industry.left", L"환경" },
            { L"citizen.politics.faction.environment_industry.right", L"산업" },
            { L"citizen.politics.faction.intellectual_conservative.left", L"지식" },
            { L"citizen.politics.faction.intellectual_conservative.right", L"보수" },
            { L"citizen.politics.faction.neutral", L"중립" },
            { L"citizen.politics.support.weak", L"약함" },
            { L"citizen.politics.support.normal", L"보통" },
            { L"citizen.politics.support.strong", L"강함" },

            { L"building.era.colonial", L"식민지 시대" },
            { L"building.era.world_wars", L"세계대전 시대" },
            { L"building.era.cold_war", L"냉전 시대" },
            { L"building.era.modern", L"현대 시대" },
            { L"building.era.unknown", L"미분류 시대" },
            { L"building.tourist_preference.cultural", L"문화" },
            { L"building.tourist_preference.family", L"가족" },
            { L"building.tourist_preference.backpacker", L"배낭여행" },
            { L"building.tourist_preference.relaxation", L"휴양" },
            { L"building.tourist_preference.thrill_seeker", L"스릴" },
            { L"building.tourist_preference.celebrity", L"VIP" },
            { L"building.tourist_preference.unknown", L"미분류" },
            { L"building.resource_type.none", L"없음" },
            { L"building.resource_type.coconuts", L"코코넛" },
            { L"building.resource_type.logs", L"통나무" },
            { L"building.resource_type.fish", L"생선" },
            { L"building.resource_type.crops", L"농작물" },
            { L"building.resource_type.animal_products", L"축산물" },
            { L"building.resource_type.ore", L"광석" },
            { L"building.resource_type.oil", L"석유" },
            { L"building.resource_type.farmed_fish", L"양식 수산물" },
            { L"building.resource_type.hydroponic_produce", L"수경 작물" },
            { L"building.resource_type.factory_livestock", L"공장식 축산물" },
            { L"building.resource_type.planks", L"널빤지" },
            { L"building.resource_type.rum", L"럼주" },
            { L"building.resource_type.leather", L"가죽" },
            { L"building.resource_type.canned_goods", L"통조림" },
            { L"building.resource_type.cheese", L"치즈" },
            { L"building.resource_type.cigars", L"시가" },
            { L"building.resource_type.boats", L"보트" },
            { L"building.resource_type.steel", L"강철" },
            { L"building.resource_type.textiles", L"옷감" },
            { L"building.resource_type.weapons", L"무기" },
            { L"building.resource_type.chocolate", L"초콜릿" },
            { L"building.resource_type.furniture", L"가구" },
            { L"building.resource_type.jewelry", L"보석류" },
            { L"building.resource_type.plastic", L"플라스틱" },
            { L"building.resource_type.cars", L"차량" },
            { L"building.resource_type.electronics", L"전자 제품" },
            { L"building.resource_type.apparel", L"의류" },
            { L"building.resource_type.pharmaceuticals", L"의약품" },
            { L"building.resource_type.juice", L"주스" },

            { L"almanac.page.overview", L"개요" },
            { L"almanac.page.satisfaction", L"만족도" },
            { L"almanac.page.population", L"인구" },
            { L"almanac.page.economy", L"경제" },
            { L"almanac.page.resources", L"자원" },
            { L"almanac.page.politics", L"정치" },
            { L"almanac.page.foreign", L"외교" },
            { L"almanac.page.buildings", L"건물" },
            { L"almanac.page.conflict", L"분쟁" },
            { L"almanac.satisfaction.title", L"만족도" },
            { L"almanac.satisfaction.overall", L"종합 만족도" },
            { L"almanac.satisfaction.food", L"음식" },
            { L"almanac.satisfaction.health", L"보건" },
            { L"almanac.satisfaction.fun", L"유흥" },
            { L"almanac.satisfaction.faith", L"신앙" },
            { L"almanac.satisfaction.housing", L"주거" },
            { L"almanac.satisfaction.job", L"직업" },
            { L"almanac.satisfaction.freedom", L"자유" },
            { L"almanac.satisfaction.security", L"치안" },
            { L"almanac.satisfaction.residential_freedom", L"주거 지역 자유" },
            { L"almanac.satisfaction.residential_security", L"주거 지역 치안" },
            { L"almanac.satisfaction.residential_pollution", L"주거 지역 공해" },
            { L"almanac.trend.years_ago_3", L"3년 전" },
            { L"almanac.trend.years_ago_2", L"2년 전" },
            { L"almanac.trend.years_ago_1", L"1년 전" },
            { L"almanac.trend.current_year", L"올해" },
            { L"almanac.overview.section.population", L"인구" },
            { L"almanac.overview.section.satisfaction", L"만족도" },
            { L"almanac.overview.section.economy", L"경제" },
            { L"almanac.overview.section.politics", L"정권 평가" },
            { L"almanac.overview.section.conflict", L"분쟁" },
            { L"almanac.overview.section.foreign", L"행정" },
            { L"almanac.resource.title", L"자원" },
            { L"almanac.resource.filter.all", L"모든 상품" },
            { L"almanac.resource.filter.period_24m", L"마지막 24개월" },
            { L"almanac.politics.title.factions", L"세력" },
            { L"almanac.politics.title.support", L"지지율" },
            { L"almanac.format.currency.million_suffix", L"백만" },
            { L"almanac.format.count_with_percent", L"{0}명 ({1})" },
            { L"almanac.tax_policy.summary_template",
                L"소비세 {0}% / 소득세 {1}% / 재산세 {2}%" },
            { L"almanac.warning.high", L"위험 높음" },
            { L"almanac.warning.caution", L"주의" },
            { L"almanac.warning.check", L"점검" },
            { L"almanac.warning.stable", L"안정" },
            { L"almanac.warning.game_lost", L"정권 상실" },
            { L"almanac.warning.no_schedule", L"선거 일정 없음" },
            { L"almanac.warning.days_remaining_suffix", L"일 남음" },
            { L"almanac.warning.fragment.support_collapse", L" / 지지 기반 붕괴" },
            { L"almanac.warning.fragment.opposition_rally", L" / 야권 결집" },
            { L"almanac.warning.fragment.close_race", L" / 접전 예상" },
            { L"almanac.politics.compact.economy.left", L"공산" },
            { L"almanac.politics.compact.economy.right", L"자본" },
            { L"almanac.politics.compact.religion_militarism.left", L"종교" },
            { L"almanac.politics.compact.religion_militarism.right", L"군부" },
            { L"almanac.politics.compact.environment_industry.left", L"환경" },
            { L"almanac.politics.compact.environment_industry.right", L"산업" },
            { L"almanac.politics.compact.intellectual_conservative.left", L"지식" },
            { L"almanac.politics.compact.intellectual_conservative.right", L"보수" },
            { L"almanac.politics.compact.neutral", L"중립" },
            { L"almanac.politics.verbose.economy.left", L"공산주의자" },
            { L"almanac.politics.verbose.economy.right", L"자본가" },
            { L"almanac.politics.verbose.religion_militarism.left", L"종교인" },
            { L"almanac.politics.verbose.religion_militarism.right", L"군부" },
            { L"almanac.politics.verbose.environment_industry.left", L"환경주의자" },
            { L"almanac.politics.verbose.environment_industry.right", L"산업주의자" },
            { L"almanac.politics.verbose.intellectual_conservative.left", L"지식인" },
            { L"almanac.politics.verbose.intellectual_conservative.right", L"보수주의자" },
            { L"almanac.tax_event_effect.worker_tax_strike",
                L"근로층 조세 반발이 치안과 노동 효율을 압박합니다." },
            { L"almanac.tax_event_effect.property_tax_backlash",
                L"재산세 반발이 지지율과 투자 심리를 약화시킵니다." },
            { L"almanac.tax_event_effect.budget_crisis",
                L"국고 위기가 공공 서비스와 통치 안정성을 흔듭니다." },
            { L"almanac.tax_event_effect.recovering",
                L"세금 사건 여파가 점차 진정되고 있습니다." },
            { L"almanac.tax_event_effect.none", L"직접적인 월드 영향 없음" },
            { L"almanac.edict.remaining_days", L" ({0}일 남음)" },
            { L"almanac.rebel_risk.high", L"높음" },
            { L"almanac.rebel_risk.medium", L"보통" },
            { L"almanac.rebel_risk.low", L"낮음" },
            { L"almanac.unit.person_suffix", L"명" },
            { L"almanac.value.empty", L"-" },
            { L"almanac.yearbook.overall_satisfaction", L"종합 만족도" },
            { L"almanac.yearbook.homeless_count", L"무주택자" },
            { L"almanac.yearbook.unemployed_count", L"실업자" },
            { L"almanac.yearbook.section.government_assessment", L"[ 정권 평가 ]" },
            { L"almanac.yearbook.incumbent_support", L"여당 지지" },
            { L"almanac.yearbook.opposition_support", L"야당 지지" },
            { L"almanac.yearbook.abstain_support", L"기권" },
            { L"almanac.yearbook.average_support_score", L"평균 지지 점수" },
            { L"almanac.yearbook.life_score", L"생활 점수" },
            { L"almanac.yearbook.ideology_alignment", L"이념 정렬도" },
            { L"almanac.yearbook.building_preference_effect", L"건물 선호 영향" },
            { L"almanac.yearbook.recent_action_effect", L"최근 조치 영향" },
            { L"almanac.yearbook.section.government_line", L"[ 정권 노선 ]" },
            { L"almanac.yearbook.section.active_edicts", L"[ 활성 칙령 ]" },
            { L"almanac.yearbook.none", L"없음" },
            { L"almanac.yearbook.section.political_counts", L"[ 정치 성향 분포 ]" },
            { L"almanac.yearbook.people_separator", L"명 / " },
            { L"almanac.yearbook.people_suffix_newline", L"명\n" },

            { L"citizen_info.wealth_profile.poor", L"가난함" },
            { L"citizen_info.wealth_profile.well_off", L"유복함" },
            { L"citizen_info.wealth_profile.rich", L"부유함" },
            { L"citizen_info.activity.work", L"근무" },
            { L"citizen_info.activity.food", L"식사" },
            { L"citizen_info.activity.leisure", L"여가" },
            { L"citizen_info.fragment.immigrant", L" · 이민자" },
            { L"citizen_info.fragment.immigrant_parenthetical", L" (이민자)" },
            { L"citizen_info.subtitle.citizen_identity_template", L"{0} · {1}{2}" },
            { L"citizen_info.body.citizen_overview",
                L"학력: {0}  재산: {1}{2}\n\n"
                L"[ 여가 ]\n"
                L"음식: {3}    보건: {4}\n"
                L"유흥: {5}    신앙: {6}\n\n"
                L"[ 복지 서비스 ]\n"
                L"주거: {7}    직업: {8}\n"
                L"통근 부담: {9}\n"
                L"자유: {10}    치안: {11}\n\n"
                L"종합 만족도: {12}" },
            { L"citizen_info.body.citizen_politics",
                L"[ 만족도 현황 ]\n"
                L"음식: {0}  보건: {1}  유흥: {2}\n"
                L"신앙: {3}  주거: {4}  직업: {5}\n"
                L"자유: {6}  치안: {7}  종합: {8}\n\n"
                L"[ 신념 ]\n"
                L"경제: {9} · {10}\n"
                L"종교·군국: {11} · {12}\n"
                L"환경·산업: {13} · {14}\n"
                L"지식·보수: {15} · {16}" },
            { L"citizen_info.body.citizen_thoughts",
                L"{0}은/는 {1} {2} 시민입니다{3}.\n\n"
                L"{4}\n\n"
                L"지배적 정치 성향: {5}" },
            { L"citizen_info.thoughts.satisfaction.very_high",
                L"트로피코 생활에 크게 만족하고 있습니다." },
            { L"citizen_info.thoughts.satisfaction.high",
                L"트로피코 생활에 어느 정도 만족하고 있습니다." },
            { L"citizen_info.thoughts.satisfaction.low",
                L"트로피코 생활에 불만이 있습니다." },
            { L"citizen_info.thoughts.satisfaction.very_low",
                L"트로피코 생활에 크게 불만족하고 있습니다." },
            { L"citizen_info.metric.activity", L"활동" },
            { L"citizen_info.metric.location", L"위치" },
            { L"citizen_info.metric.age", L"연령" },
            { L"citizen_info.metric.origin", L"출신" },
            { L"citizen_info.metric.tourist_profile", L"관광 유형" },
            { L"citizen_info.metric.visit", L"관광지" },
            { L"citizen_info.metric.wealth", L"재산" },
            { L"citizen_info.metric.education", L"교육" },
            { L"citizen_info.metric.job", L"직장" },
            { L"citizen_info.metric.home", L"집" },
            { L"citizen_info.value.age_30_adult", L"30 (성인)" },
            { L"citizen_info.origin.france", L"프랑스" },
            { L"citizen_info.origin.tropico", L"트로피코" },
            { L"citizen_info.origin.tourist", L"해외 관광객" },
            { L"citizen_info.action.bribe", L"매수" },
            { L"citizen_info.action.kill", L"살해" },
            { L"citizen_info.action.stage_accident", L"사고사로 위장" },
            { L"citizen_info.action.arrest", L"체포" },
            { L"citizen_info.action.isolate", L"보호 시설 격리" },
            { L"citizen_info.action.send_to_space", L"우주 파견" },
            { L"citizen_info.thought.line1", L"우리 군대의 군복이 아주 끝내줘!" },
            { L"citizen_info.thought.line2",
                L"잠을 자면서 돈을 받는 게 내가 바라는\n직업이지." },
            { L"citizen_info.thought.line3",
                L"내가 무슨 생각을 하고 싶은지 생각할 수\n있다니!" },
            { L"citizen_info.thought.line4",
                L"난 노숙자가 아니야. 집을 찾아다니는 게\n직업일 뿐..." },
            { L"citizen_info.thought.line5",
                L"강국이라면 군대에 아무도 사용법을\n모르는 무기도 많아야 마땅하지." },
            { L"citizen_info.location.interior_suffix", L" 내부" },
            { L"citizen_info.location.outside_tropico", L"트로피코 외부" },
            { L"citizen_info.politics.intensity.fervent", L"열렬함" },
            { L"citizen_info.politics.intensity.strong", L"강함" },
            { L"citizen_info.action.change_resource", L"자원 변경" },
            { L"citizen_info.action.operation_mode_cycle", L"운영 모드" },
            { L"citizen_info.action.runtime_upgrade_cycle", L"업그레이드" },
            { L"citizen_info.action.warehouse_policy_cycle", L"보관 정책" },
            { L"citizen_info.action.warehouse_priority_cycle", L"보관 우선" },
            { L"citizen_info.action.auto_import_cycle", L"자동 수입" },
            { L"citizen_info.action.domestic_reserve_cycle", L"내수 비축" },
            { L"citizen_info.action.import_cap_cycle", L"수입 한도" },
            { L"citizen_info.action.import_budget_cycle", L"수입 예산" },
            { L"citizen_info.action.export_block_cycle", L"수출 금지" },
            { L"citizen_info.subtitle.cocoa", L"< 코코아 >" },
            { L"citizen_info.budget.summary_template",
                L"예산 단계 {0}  |  {1}  |  월 비용 {2}" },
            { L"citizen_info.unit.megawatt", L"메가와트" },
            { L"citizen_info.unit.day_suffix", L"일" },
            { L"citizen_info.work_mode.mostly_natural", L"대체로 천연" },
            { L"citizen_info.work_mode.fresh_and_clean", L"산뜻하고 깔끔하게" },
            { L"citizen_info.work_mode.thousand_plus", L"천 넘긴" },
            { L"citizen_info.work_mode.general_control", L"일반 통제" },
            { L"citizen_info.commodity.cocoa", L"코코아" },
            { L"citizen_info.commodity.sugar", L"설탕" },
            { L"citizen_info.commodity.corn", L"옥수수" },

            { L"citizen_info.building.housing_class.collective", L"서민 주거" },
            { L"citizen_info.building.housing_class.standard", L"중산층 주거" },
            { L"citizen_info.building.housing_class.elite", L"고급 주거" },
            { L"citizen_info.building.housing_class.default", L"일반" },
            { L"citizen_info.building.leisure_class.cultural", L"문화" },
            { L"citizen_info.building.leisure_class.luxury", L"고급" },
            { L"citizen_info.building.leisure_class.general", L"일반" },
            { L"citizen_info.building.leisure_class.unknown", L"미분류" },
            { L"citizen_info.building.role.residential",
                L"{0}은(는) {1}명을 수용하는 주거 건물입니다." },
            { L"citizen_info.building.role.residential_class_suffix",
                L"{0} 계층을 주로 상대합니다." },
            { L"citizen_info.building.role.harbor",
                L"수입/수출과 입국 관문 역할을 담당하는 기반시설입니다." },
            { L"citizen_info.building.role.food_entertainment",
                L"음식과 유흥을 동시에 제공하는 서비스 건물입니다." },
            { L"citizen_info.building.role.entertainment",
                L"관광객과 시민에게 여가 서비스를 제공하는 시설입니다." },
            { L"citizen_info.building.role.entertainment_class_suffix",
                L"{0} 성향의 방문객에게 적합합니다." },
            { L"citizen_info.building.role.food",
                L"시민 소비와 외식 흐름을 담당하는 공급 건물입니다." },
            { L"citizen_info.building.role.work_output",
                L"인력을 배치해 자원이나 상품을 생산하는 작업 시설입니다." },
            { L"citizen_info.building.role.support",
                L"도시 운영에 필요한 기능을 담당하는 보조 건물입니다." },
            { L"citizen_info.building.warehouse.empty", L"비어 있음" },
            { L"citizen_info.building.warehouse.slot_line", L"슬롯 {0}: {1}" },
            { L"citizen_info.section.operation_modes", L"운영 모드" },
            { L"citizen_info.section.operation_mode_candidates", L"운영 모드 후보" },
            { L"citizen_info.section.active_upgrades", L"활성 업그레이드" },
            { L"citizen_info.section.runtime_upgrades", L"적용 가능한 업그레이드" },
            { L"citizen_info.section.service", L"서비스" },
            { L"citizen_info.section.power", L"전력" },
            { L"citizen_info.section.environment", L"환경" },
            { L"citizen_info.section.storage_logistics", L"보관 및 물류" },
            { L"citizen_info.section.export_policy", L"수출 정책" },
            { L"citizen_info.section.export_priority", L"선적 우선순위" },
            { L"citizen_info.section.trade_prices", L"자원 거래 단가" },
            { L"citizen_info.section.main_effect", L"주요 효과" },
            { L"citizen_info.section.note", L"비고" },
            { L"citizen_info.section.warehouse_slots", L"창고 슬롯" },
            { L"citizen_info.section.registered_upgrades", L"등록된 업그레이드" },
            { L"citizen_info.upgrades.none", L"등록된 업그레이드가 없습니다." },
            { L"citizen_info.label.budget", L"예산" },
            { L"citizen_info.label.residence", L"거주지" },
            { L"citizen_info.label.residents_status", L"거주 현황" },
            { L"citizen_info.label.workers", L"노동자" },
            { L"citizen_info.label.required_education", L"요구 학력" },
            { L"citizen_info.label.job_quality", L"직업 품질" },
            { L"citizen_info.label.housing_class", L"주거 계층" },
            { L"citizen_info.label.visitors", L"방문객" },
            { L"citizen_info.label.service_quality", L"서비스 품질" },
            { L"citizen_info.label.wealth_requirement", L"재산 요구치" },
            { L"citizen_info.label.tourist_preference", L"선호 관광객" },
            { L"citizen_info.label.required_power", L"필요 전력" },
            { L"citizen_info.label.produced_power", L"생산 전력" },
            { L"citizen_info.label.power_coverage", L"전력 충족" },
            { L"citizen_info.label.pollution_output", L"공해 배출" },
            { L"citizen_info.label.pollution_mitigation", L"공해 정화" },
            { L"citizen_info.label.local_pollution", L"주변 공해" },
            { L"citizen_info.label.stock", L"재고" },
            { L"citizen_info.label.slot_capacity", L"슬롯당 용량" },
            { L"citizen_info.label.warehouse_policy", L"보관 정책" },
            { L"citizen_info.label.warehouse_priority", L"보관 우선" },
            { L"citizen_info.label.storage_loss", L"보관 손실" },
            { L"citizen_info.label.produced_resource", L"생산 자원" },
            { L"citizen_info.label.market_price", L"시장 시세" },
            { L"citizen_info.label.exportable_stock", L"수출 가능 재고" },
            { L"citizen_info.label.export_unit_price", L"수출 단가" },
            { L"citizen_info.label.import_unit_price", L"수입 단가" },
            { L"citizen_info.label.instant_export_value", L"즉시 수출 가치" },
            { L"citizen_info.label.export_short", L"수출" },
            { L"citizen_info.label.import_short", L"수입" },
            { L"citizen_info.label.ship_progress", L"선박 진행" },
            { L"citizen_info.label.monthly_wage_cost", L"월 인건비" },
            { L"citizen_info.label.monthly_upkeep_cost", L"월 유지비" },
            { L"citizen_info.label.monthly_total_cost", L"월 총비용" },
            { L"citizen_info.label.daily_wage_cost", L"일 인건비" },
            { L"citizen_info.label.daily_upkeep_cost", L"일 유지비" },
            { L"citizen_info.label.current_stock", L"현재 재고" },
            { L"citizen_info.label.ship_arrival_progress", L"선박 도착 진행" },
            { L"citizen_info.label.exportable_total", L"수출 가능 총량" },
            { L"citizen_info.label.resident_assignment", L"거주자 배정" },
            { L"citizen_info.label.representative_resident", L"대표 거주자" },
            { L"citizen_info.label.assigned_workers", L"배정 노동자" },
            { L"citizen_info.label.working_now", L"근무 중" },
            { L"citizen_info.label.representative_worker", L"대표 노동자" },
            { L"citizen_info.label.assigned_visitors", L"배정 방문객" },
            { L"citizen_info.label.on_site_visitors", L"현장 체류" },
            { L"citizen_info.label.incoming_visitors", L"도착 임박" },
            { L"citizen_info.label.representative_visitor", L"대표 방문객" },
            { L"citizen_info.label.current_efficiency", L"현재 효율" },
            { L"citizen_info.label.budget_scale", L"예산 보정" },
            { L"citizen_info.label.road_accessibility", L"도로 접근성" },
            { L"citizen_info.label.housing_fill_rate", L"거주 충원율" },
            { L"citizen_info.label.housing_satisfaction_cap", L"주거 만족도 상한" },
            { L"citizen_info.label.worker_fill_rate", L"인력 충원율" },
            { L"citizen_info.label.job_satisfaction_cap", L"직업 만족도 상한" },
            { L"citizen_info.label.food_satisfaction_cap", L"음식 만족도 상한" },
            { L"citizen_info.label.fun_satisfaction_cap", L"유흥 만족도 상한" },
            { L"citizen_info.label.visitor_utilization", L"방문 활용도" },
            { L"citizen_info.label.power_demand", L"전력 요구" },
            { L"citizen_info.label.category", L"카테고리" },
            { L"citizen_info.label.blueprint_cost", L"설계도 비용" },
            { L"citizen_info.label.construction_cost", L"건설 비용" },
            { L"citizen_info.label.unlock_era", L"등장 시대" },
            { L"citizen_info.label.housing_grade", L"주거 등급" },
            { L"citizen_info.label.leisure_grade", L"여가 등급" },
            { L"citizen_info.label.primary_tourist", L"대표 관광객" },
            { L"citizen_info.label.efficiency", L"효율" },
            { L"citizen_info.label.electricity", L"전기" },
            { L"citizen_info.label.expected_revenue", L"예상 수익" },
            { L"citizen_info.label.fee_income", L"요금/수입" },
            { L"citizen_info.label.fee_revenue", L"요금 수입" },
            { L"citizen_info.label.harbor", L"항구" },
            { L"citizen_info.label.housing_quality", L"주거 품질" },
            { L"citizen_info.label.last_month_income", L"수입 (전월)" },
            { L"citizen_info.label.monthly_income", L"월간 수입" },
            { L"citizen_info.label.next_arrival_time", L"다음 도착 시각" },
            { L"citizen_info.label.power_grid_status", L"전력망 현황" },
            { L"citizen_info.label.power_network", L"전력망" },
            { L"citizen_info.label.preferred_type", L"선호하는 유형" },
            { L"citizen_info.label.production", L"생산" },
            { L"citizen_info.label.production_stage", L"생산 단계" },
            { L"citizen_info.label.required_wealth", L"필요 재산" },
            { L"citizen_info.label.household_capacity", L"수용 가구" },
            { L"citizen_info.label.service_capacity", L"수용 인원" },
            { L"citizen_info.label.storage", L"보관소" },
            { L"citizen_info.label.total_cost", L"전체 비용" },
            { L"citizen_info.label.total_income", L"수입 (전체)" },
            { L"citizen_info.label.tourist_only_checkbox", L"□ 관광객 전용" },
            { L"citizen_info.label.wage", L"임금" },
            { L"citizen_info.label.work_mode", L"근무 형태" },
            { L"citizen_info.value.free", L"무료" },
            { L"citizen_info.value.unknown", L"미확인" },
            { L"citizen_info.note.housing_quality_efficiency",
                L"주거 품질은 효율에 따라 변합니다." },
            { L"citizen_info.upgrade.solar_window.title", L"태양광 패널 창문" },
            { L"citizen_info.upgrade.solar_window.description",
                L"건물의 전력 필요량이 -30메가와트\n감소합니다." },
            { L"citizen_info.information.modern_apartment.top",
                L"가구를 수용할 수 있는 고급 주거\n"
                L"건물입니다. 입주자의 재산 수준이\n"
                L"최소 {0} 이상이어야 합니다. 적은\n"
                L"공해를 배출합니다." },
            { L"citizen_info.information.modern_apartment.bottom",
                L"콘크리트와 강철로 이루어진 괴물,\n"
                L"현대식 아파트는 '현대적인 편의 시설\n"
                L"완비'라는 광고 문구를 이용해\n"
                L"제정신이 아닌 듯한 월세를 제시합니다." }
        };

        std::unordered_map<std::wstring, std::wstring> GTable;
        bool GLoaded = false;

        std::wstring GetConfigPath()
        {
            wchar_t ExePath[MAX_PATH] = {};
            GetModuleFileNameW(nullptr, ExePath, MAX_PATH);

            std::wstring Path(ExePath);
            const size_t Slash = Path.rfind(L'\\');

            if (Slash != std::wstring::npos)
                Path = Path.substr(0, Slash + 1);

            return Path + L"UIStrings.ini";
        }

        void TrimString(std::string& Value)
        {
            Value.erase(
                Value.begin(),
                std::find_if(
                    Value.begin(),
                    Value.end(),
                    [](unsigned char Character)
                    {
                        return !std::isspace(Character);
                    }));
            Value.erase(
                std::find_if(
                    Value.rbegin(),
                    Value.rend(),
                    [](unsigned char Character)
                    {
                        return !std::isspace(Character);
                    }).base(),
                Value.end());
        }

        std::wstring Utf8ToWide(const std::string& Value)
        {
            if (Value.empty())
                return std::wstring();

            const int RequiredLength = MultiByteToWideChar(
                CP_UTF8,
                0,
                Value.c_str(),
                static_cast<int>(Value.size()),
                nullptr,
                0);

            if (RequiredLength <= 0)
                return std::wstring(Value.begin(), Value.end());

            std::wstring Result(static_cast<size_t>(RequiredLength), L'\0');
            MultiByteToWideChar(
                CP_UTF8,
                0,
                Value.c_str(),
                static_cast<int>(Value.size()),
                &Result[0],
                RequiredLength);
            return Result;
        }

        std::wstring ParseTextValue(const std::string& Value)
        {
            std::string Unescaped;
            Unescaped.reserve(Value.size());

            for (size_t Index = 0; Index < Value.size(); ++Index)
            {
                if (Value[Index] != '\\' || Index + 1 >= Value.size())
                {
                    Unescaped.push_back(Value[Index]);
                    continue;
                }

                const char Escaped = Value[Index + 1];

                if (Escaped == 'n')
                {
                    Unescaped.push_back('\n');
                    ++Index;
                    continue;
                }

                if (Escaped == 't')
                {
                    Unescaped.push_back('\t');
                    ++Index;
                    continue;
                }

                Unescaped.push_back(Escaped);
                ++Index;
            }

            return Utf8ToWide(Unescaped);
        }

        void LoadDefaults()
        {
            for (const FDefaultEntry& Entry : GDefaultEntries)
            {
                GTable.emplace(Entry.Key, Entry.Value);
            }
        }

        void LoadOverrides()
        {
            std::ifstream File(GetConfigPath(), std::ios::binary);

            if (!File.is_open())
                return;

            std::string Content(
                (std::istreambuf_iterator<char>(File)),
                std::istreambuf_iterator<char>());

            if (Content.size() >= 3 &&
                static_cast<unsigned char>(Content[0]) == 0xEF &&
                static_cast<unsigned char>(Content[1]) == 0xBB &&
                static_cast<unsigned char>(Content[2]) == 0xBF)
            {
                Content.erase(0, 3);
            }

            std::istringstream Stream(Content);
            std::string Line;

            while (std::getline(Stream, Line))
            {
                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (Line.empty() || Line[0] == '#' || Line[0] == ';')
                    continue;

                const size_t EqPos = Line.find('=');

                if (EqPos == std::string::npos)
                    continue;

                std::string Key = Line.substr(0, EqPos);
                std::string Value = Line.substr(EqPos + 1);
                TrimString(Key);
                TrimString(Value);

                if (Key.empty())
                    continue;

                GTable[Utf8ToWide(Key)] = ParseTextValue(Value);
            }
        }

        void EnsureLoaded()
        {
            if (GLoaded)
                return;

            LoadDefaults();
            LoadOverrides();
            GLoaded = true;
        }

        void ReplaceAll(
            std::wstring& Text,
            const std::wstring& Pattern,
            const std::wstring& Replacement)
        {
            if (Pattern.empty())
                return;

            size_t Position = 0;

            while ((Position = Text.find(Pattern, Position)) != std::wstring::npos)
            {
                Text.replace(Position, Pattern.size(), Replacement);
                Position += Replacement.size();
            }
        }
    }

    const std::wstring& Get(const wchar_t* Key)
    {
        EnsureLoaded();

        auto Iter = GTable.find(Key);

        if (Iter != GTable.end())
            return Iter->second;

        auto Result = GTable.emplace(Key, Key);
        return Result.first->second;
    }

    std::wstring Format(
        const wchar_t* Key,
        const std::vector<std::wstring>& Args)
    {
        std::wstring Result = Get(Key);

        for (size_t Index = 0; Index < Args.size(); ++Index)
        {
            ReplaceAll(
                Result,
                L"{" + std::to_wstring(Index) + L"}",
                Args[Index]);
        }

        return Result;
    }
}
