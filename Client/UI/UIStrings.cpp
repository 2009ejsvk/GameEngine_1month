#include "UIStrings.h"
#include "../StringUtils.h"
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
        using StringUtils::ReplaceAll;
        using StringUtils::Trim;
        using StringUtils::Utf8ToWide;

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
            { L"top_hud.menu.era_transition", L"시대 전환" },
            { L"top_hud.menu.trade", L"무역" },
            { L"top_hud.menu.raid", L"원정" },
            { L"top_hud.menu.research", L"연구" },
            { L"top_hud.menu.almanac", L"연감" },
            { L"top_hud.label.budget", L"예산" },
            { L"top_hud.label.population", L"인구" },
            { L"top_hud.label.support", L"지지율" },
            { L"top_hud.label.knowledge", L"지식" },
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
            { L"top_hud.fragment.era_ready_prefix", L"시대 전환 가능 | " },
            { L"top_hud.fragment.era_complete_prefix", L"시대 전환 완료 | " },
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
            { L"top_hud.era_transition.cancel", L"나중에" },
            { L"top_hud.era_transition.unavailable", L"아직 전환 조건이 충족되지 않았습니다." },
            { L"constitution.popup.title_prefix", L"헌법 선택: " },
            { L"constitution.popup.body_prompt", L"헌법을 선택하십시오." },
            { L"constitution.panel.title", L"트로피코 헌법" },
            { L"constitution.panel.subtitle", L"개요 화면에서 현재 채택한 조항을 확인하고, 상단 탭으로 주제별 세부 내용을 볼 수 있습니다." },
            { L"constitution.panel.topic_subtitle", L"현재 주제의 선택지와 효과를 확인하십시오." },
            { L"constitution.panel.overview_button", L"개요" },
            { L"constitution.panel.topic_pending", L"이 주제는 지금 바로 결정할 수 있습니다." },
            { L"constitution.panel.current_selection", L"현재 채택안: " },
            { L"constitution.summary.pending_prefix", L"선택 필요: " },
            { L"constitution.summary.review_all", L"현재 헌법 전체를 검토할 수 있습니다." },
            { L"constitution.summary.pending", L"선택 필요" },
            { L"constitution.summary.active", L"채택됨" },
            { L"constitution.summary.waiting", L"검토 가능" },
            { L"constitution.summary.selection_required", L"아직 선택되지 않음" },
            { L"constitution.summary.research_required", L"연구 필요" },
            { L"constitution.summary.locked", L"잠김" },
            { L"constitution.option.placeholder_title", L"추가 선택지 준비 중" },
            { L"constitution.option.placeholder_summary", L"새 조항이 해금되면 이 자리가 채워집니다." },
            { L"constitution.option.placeholder_body", L"현재 구현된 헌법 시스템은 주제별 핵심 조항을 먼저 지원합니다. 이후 연구 및 시대 확장에 맞춰 추가 선택지가 연결될 수 있습니다." },
            { L"constitution.effect.liberty", L"자유" },
            { L"constitution.effect.security", L"치안" },
            { L"constitution.effect.job_quality", L"직업 품질" },
            { L"constitution.effect.immigration", L"이민" },
            { L"constitution.effect.no_major_change", L"주요 수치 변화 없음" },
            { L"constitution.topic.voting_rights", L"투표권" },
            { L"constitution.topic.labor_policy", L"노동 정책" },
            { L"constitution.topic.religion_and_state", L"종교와 국가" },
            { L"constitution.topic.media_independence", L"언론 독립" },
            { L"constitution.topic.armed_forces", L"군대" },
            { L"constitution.topic.default", L"헌법" },
            { L"constitution.option.all_citizens_vote.name", L"보통선거" },
            { L"constitution.option.all_citizens_vote.summary", L"자유와 이민이 증가하지만 상류층 팩션은 반발합니다." },
            { L"constitution.option.wealthy_citizens_vote.name", L"재산 제한 선거" },
            { L"constitution.option.wealthy_citizens_vote.summary", L"치안과 기득권 안정에 유리하지만 자유가 감소합니다." },
            { L"constitution.option.early_retirement.name", L"조기 은퇴" },
            { L"constitution.option.early_retirement.summary", L"노동자 삶의 질이 높아지지만 기업가 지지는 줄어듭니다." },
            { L"constitution.option.lifes_work.name", L"종신 노동" },
            { L"constitution.option.lifes_work.summary", L"직업 만족은 떨어지지만 산업계와 자본가가 선호합니다." },
            { L"constitution.option.theocracy.name", L"신정국가" },
            { L"constitution.option.theocracy.summary", L"신앙과 질서는 강화되지만 시민 자유는 감소합니다." },
            { L"constitution.option.official_separation.name", L"정교 분리" },
            { L"constitution.option.official_separation.summary", L"자유와 이민은 늘지만 종교 세력은 반발합니다." },
            { L"constitution.option.free_media.name", L"언론 자유" },
            { L"constitution.option.free_media.summary", L"자유와 개방성이 늘지만 국가 통제력은 약해집니다." },
            { L"constitution.option.state_controlled_media.name", L"국가 통제 언론" },
            { L"constitution.option.state_controlled_media.summary", L"통제와 치안은 오르지만 자유는 크게 감소합니다." },
            { L"constitution.option.professional_soldiers.name", L"직업 군인" },
            { L"constitution.option.professional_soldiers.summary", L"상비군 중심으로 치안을 크게 강화합니다." },
            { L"constitution.option.militia.name", L"민병대" },
            { L"constitution.option.militia.summary", L"시민 무장을 허용해 자유는 오르지만 치안 대응력은 낮아집니다." },
            { L"escalation.stage.stable", L"안정" },
            { L"escalation.stage.warning", L"경고" },
            { L"escalation.stage.demand", L"요구" },
            { L"escalation.stage.ultimatum", L"최후통첩" },
            { L"escalation.stage.revolt", L"봉기" },
            { L"escalation.pressure_suffix_template", L" / 압력 {0}일" },
            { L"escalation.warning.title_template", L"{0} 경고" },
            { L"escalation.warning.generic",
                L"승인도가 낮은 상태가 {0}일째 지속되고 있습니다. 방치하면 최후통첩으로 번질 수 있습니다." },
            { L"escalation.warning.communists",
                L"주거와 복지에 대한 불만이 {0}일째 누적되고 있습니다. 방치하면 주택가 소요와 최후통첩으로 번질 수 있습니다." },
            { L"escalation.warning.capitalists",
                L"세금과 투자 여건에 대한 불만이 {0}일째 누적되고 있습니다. 방치하면 투자 위축과 최후통첩으로 번질 수 있습니다." },
            { L"escalation.warning.religious",
                L"신앙 질서에 대한 반발이 {0}일째 이어지고 있습니다. 방치하면 종교 세력의 강경 요구로 번질 수 있습니다." },
            { L"escalation.warning.militarists",
                L"치안과 군사 대비 태세에 대한 불만이 {0}일째 누적되고 있습니다. 방치하면 군부의 최후통첩과 봉기 준비로 번질 수 있습니다." },
            { L"escalation.warning.environmentalists",
                L"환경과 보건 악화에 대한 분노가 {0}일째 누적되고 있습니다. 방치하면 생산 차질과 최후통첩으로 번질 수 있습니다." },
            { L"escalation.warning.industrialists",
                L"생산성과 수출 부진에 대한 불만이 {0}일째 누적되고 있습니다. 방치하면 산업 압박과 최후통첩으로 번질 수 있습니다." },
            { L"escalation.warning.intellectuals",
                L"자유와 개방성 후퇴에 대한 반발이 {0}일째 누적되고 있습니다. 방치하면 여론 악화와 최후통첩으로 번질 수 있습니다." },
            { L"escalation.warning.conservatives",
                L"질서와 재산 안정에 대한 불만이 {0}일째 누적되고 있습니다. 방치하면 보수 세력의 강경 요구로 번질 수 있습니다." },

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

            { L"citizen_info.tab.citizen.overview", L"정보" },
            { L"citizen_info.tab.citizen.politics", L"만족도" },
            { L"citizen_info.tab.citizen.thoughts", L"생각" },
            { L"citizen_info.page.citizen.overview", L"정보" },
            { L"citizen_info.page.citizen.politics", L"만족도" },
            { L"citizen_info.page.citizen.thoughts", L"생각" },
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
            { L"citizen_info.action.repair_damage", L"수리" },
            { L"citizen_info.building.data_pending", L"건물 데이터 준비 중입니다." },
            { L"citizen_info.activity.move", L"이동" },
            { L"citizen_info.subtitle.tropican", L"↠ 트로피코인 ↞" },
            { L"citizen_info.subtitle.tourist", L"↠ 관광객 ↞" },
            { L"citizen_info.damage.none", L"정상" },
            { L"citizen_info.damage.damaged", L"손상" },
            { L"citizen_info.damage.critical", L"치명적 손상" },
            { L"citizen_info.label.damage_status", L"피해 상태" },
            { L"citizen_info.label.damage_efficiency", L"피해 효율" },
            { L"citizen_info.label.repair_cost", L"수리 비용" },

            { L"citizen.education.uneducated", L"무학력" },
            { L"citizen.education.high_school", L"고등학교" },
            { L"citizen.education.college", L"대학" },
            { L"citizen.wealth.broke", L"파산" },
            { L"citizen.wealth.poor", L"가난" },
            { L"citizen.wealth.well_off", L"유복" },
            { L"citizen.wealth.rich", L"부유" },
            { L"citizen.wealth.filthy_rich", L"더럽게 부유" },
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
            { L"building.resource_type.feed_crops", L"사료 작물" },
            { L"building.resource_type.banana", L"바나나" },
            { L"building.resource_type.cocoa", L"코코아" },
            { L"building.resource_type.coffee", L"커피" },
            { L"building.resource_type.corn", L"옥수수" },
            { L"building.resource_type.cotton", L"면화" },
            { L"building.resource_type.pineapple", L"파인애플" },
            { L"building.resource_type.rubber", L"고무" },
            { L"building.resource_type.sugar", L"설탕" },
            { L"building.resource_type.tobacco", L"담배" },
            { L"building.resource_type.meat", L"고기" },
            { L"building.resource_type.milk", L"우유" },
            { L"building.resource_type.hides", L"생가죽" },
            { L"building.resource_type.wool", L"양모" },
            { L"building.resource_type.coal", L"석탄" },
            { L"building.resource_type.iron", L"철" },
            { L"building.resource_type.gold", L"금" },
            { L"building.resource_type.nickel", L"니켈" },
            { L"building.resource_type.aluminum", L"알루미늄" },
            { L"building.resource_type.uranium", L"우라늄" },
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
            { L"building.resource_type.special_chocolate", L"특수 초콜릿" },
            { L"building.resource_type.goldnuts", L"골드넛" },
            { L"building.resource_type.bs", L"BS" },

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

            { L"citizen_info.wealth_profile.broke", L"파산 상태" },
            { L"citizen_info.wealth_profile.poor", L"가난함" },
            { L"citizen_info.wealth_profile.well_off", L"유복함" },
            { L"citizen_info.wealth_profile.rich", L"부유함" },
            { L"citizen_info.wealth_profile.filthy_rich", L"더럽게 부유함" },
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
            { L"citizen_info.action.export_block_cycle", L"수출 금지" },
            { L"citizen_info.subtitle.cocoa", L"< 코코아 >" },
            { L"citizen_info.budget.summary_template",
                L"예산 단계 {0}  |  {1}  |  월 비용 {2}" },
            { L"citizen_info.unit.megawatt", L"메가와트" },
            { L"citizen_info.unit.day_suffix", L"일" },
            { L"citizen_info.unit.per_second_suffix", L"/초" },
            { L"citizen_info.unit.per_day_suffix", L"/일" },
            { L"citizen_info.unit.per_month_suffix", L"/월" },
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
            { L"citizen_info.label.harbor_cargo", L"선적 화물" },
            { L"citizen_info.label.output_stock", L"생산물 재고" },
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
            { L"citizen_info.label.input_slot", L"투입" },
            { L"citizen_info.label.output_per_second", L"초당 생산량" },
            { L"citizen_info.label.expected_daily_output", L"예상 일일 산출량" },
            { L"citizen_info.label.expected_monthly_output", L"예상 월간 산출량" },
            { L"citizen_info.label.required_inputs", L"필요 입력" },
            { L"citizen_info.label.input_status", L"입력 상태" },
            { L"citizen_info.label.input_consumption", L"입력 소비율" },
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
            { L"citizen_info.label.teamster_delivery", L"팀스터 배달" },
            { L"citizen_info.label.tourist_only_checkbox", L"□ 관광객 전용" },
            { L"citizen_info.label.wage", L"임금" },
            { L"citizen_info.label.work_mode", L"근무 형태" },
            { L"citizen_info.section.production_inputs", L"생산 입력" },
            { L"citizen_info.input_status.stopped", L"입력 부족으로 생산 중단" },
            { L"citizen_info.input_status.low", L"입력 부족으로 생산 저하" },
            { L"citizen_info.input_status.ready", L"입력 재고 정상" },
            { L"citizen_info.value.free", L"무료" },
            { L"citizen_info.value.stable", L"안정" },
            { L"citizen_info.value.unknown", L"미확인" },
            { L"citizen_info.fragment.shortage", L"부족" },
            { L"citizen_info.fragment.pickup_waiting", L"수거 대기" },
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
                L"제정신이 아닌 듯한 월세를 제시합니다." },
            { L"citizen_info.wealth.at_least_template", L"{0} 이상" },
            { L"citizen_info.detail.prefix.construction_cost", L"건설 비용:" },
            { L"citizen_info.detail.prefix.blueprint_cost", L"설계도 비용:" },
            { L"citizen_info.detail.prefix.unlock_era", L"등장 시기:" },
            { L"citizen_info.detail.prefix.required_workers", L"필요 인력:" },
            { L"citizen_info.detail.prefix.required_power", L"필요 전력:" },
            { L"citizen_info.detail.prefix.produced_power", L"생산 전력:" },
            { L"citizen_info.detail.prefix.size", L"크기:" },
            { L"citizen_info.detail.prefix.job_quality", L"직업 품질:" },
            { L"citizen_info.detail.prefix.service_quality", L"서비스 품질:" },
            { L"citizen_info.detail.prefix.service_capacity", L"수용 인원:" },
            { L"citizen_info.detail.prefix.household_capacity", L"수용 가구:" },
            { L"citizen_info.detail.prefix.wealth_requirement", L"재산 요구치:" },
            { L"citizen_info.detail.prefix.required_wealth", L"필요 재산:" },
            { L"citizen_info.detail.prefix.tourist_wealth", L"관광객 재산:" },
            { L"citizen_info.detail.prefix.tourist_preference", L"선호 관광객:" },
            { L"citizen_info.detail.prefix.operation_mode", L"운영 모드:" },
            { L"citizen_info.detail.prefix.upgrade", L"업그레이드" },
            { L"citizen_info.detail.prefix.effect", L"효과:" },
            { L"citizen_info.detail.prefix.note", L"비고:" },
            { L"task_widget.title", L"과제" },
            { L"task_widget.subtitle",
                L"세력 요구, 외교 요청, 국정 과제를 수락하고 처리할 수 있습니다." },
            { L"task_widget.empty", L"현재 활성화된 과제가 없습니다." },
            { L"task_widget.placeholder.none", L"-" },
            { L"task_widget.value.none", L"없음" },
            { L"task_widget.action.accept", L"수락" },
            { L"task_widget.action.decline", L"거절" },
            { L"task_widget.action.close", L"닫기" },
            { L"task_widget.action.abandon", L"포기" },
            { L"task_widget.action.abandon_with_penalty_template",
                L"포기 ({0})" },
            { L"task_widget.faction.communists", L"공산주의자" },
            { L"task_widget.faction.capitalists", L"자본가" },
            { L"task_widget.faction.religious", L"종교인" },
            { L"task_widget.faction.militarists", L"군부" },
            { L"task_widget.faction.environmentalists", L"환경주의자" },
            { L"task_widget.faction.industrialists", L"산업주의자" },
            { L"task_widget.faction.intellectuals", L"지식인" },
            { L"task_widget.faction.conservatives", L"보수주의자" },
            { L"task_widget.faction.generic", L"세력" },
            { L"task_widget.objective.completed_template",
                L"[완료] {0} {1}/{2}" },
            { L"task_widget.objective.progress_template",
                L"[진행] {0} {1}/{2}" },
            { L"task_widget.objective.population", L"인구" },
            { L"task_widget.objective.buildings", L"건물" },
            { L"task_widget.objective.food_providers", L"식량 공급처" },
            { L"task_widget.objective.industry_buildings", L"산업 건물" },
            { L"task_widget.objective.public_service_buildings",
                L"공공서비스 건물" },
            { L"task_widget.objective.entertainment_buildings",
                L"오락 건물" },
            { L"task_widget.objective.power_mw", L"전력(MW)" },
            { L"task_widget.demand.row_subtitle_template", L"{0} / {1}일" },
            { L"task_widget.demand.status.in_progress", L"수행 중" },
            { L"task_widget.demand.status.awaiting_response", L"응답 대기" },
            { L"task_widget.penultimo.issuer_label", L"페놀티코의 과제" },
            { L"task_widget.penultimo.name", L"페놀티코" },
            { L"task_widget.era_mission.row_title_template",
                L"{0} → {1} 시대 진행" },
            { L"task_widget.era_mission.row_subtitle", L"페놀티코 / 조건 달성 중" },
            { L"task_widget.era_mission.detail_body_template",
                L"페놀티코가 보고합니다. {0} 시대로 전환하려면 아래 조건을 달성해야 합니다." },
            { L"task_widget.era_mission.stage_line", L"국정 과제 / 진행 중" },
            { L"task_widget.era_mission.reward_template",
                L"{0} 시대 건물과 칙령 즉시 해금 / 외교 및 시대 진행 상태 갱신" },
            { L"task_widget.era_transition.default_title", L"시대 전환 과제" },
            { L"task_widget.era_transition.row_subtitle", L"페놀티코 / 즉시 처리 가능" },
            { L"task_widget.era_transition.detail_body_template",
                L"페놀티코가 보고합니다. {0}에서 {1}(으)로 넘어갈 준비가 끝났습니다." },
            { L"task_widget.era_transition.objective_template", L"결재: {0}" },
            { L"task_widget.era_transition.stage_line", L"국정 과제 / 준비 완료" },
            { L"task_widget.era_transition.penalty_text",
                L"불이익 없음 / 나중에 다시 결재 가능" },
            { L"task_widget.era_transition.confirm_default", L"시대 전환 승인" },
            { L"task_widget.demand.issuer_label_template", L"{0} 요구" },
            { L"task_widget.demand.speaker_label_template", L"{0} 대표" },
            { L"task_widget.demand.detail_body_template",
                L"{0} 측이 프레지덴테에게 직접 요구를 전달했습니다." },
            { L"task_widget.demand.stage_line_template",
                L"{0} / 압력 {1}일 / 남은 {2}일" },
            { L"task_widget.foreign.issuer_label_template", L"{0} 요청" },
            { L"task_widget.foreign.detail_body_template",
                L"{0} 관련 외교 채널을 통해 새로운 요청이 도착했습니다." },
            { L"task_widget.foreign.stage_line_template", L"외교 요청 / 남은 {0}일" },
            { L"task_widget.detail.header.approval", L"결재" },
            { L"task_widget.detail.header.conditions", L"달성 조건" },
            { L"task_widget.detail.header.objective", L"목표" },
            { L"task_widget.detail.header.reward", L"보상" },
            { L"task_widget.detail.header.effect", L"효과" },
            { L"task_widget.detail.header.penalty_abandon", L"포기 시 불이익" },
            { L"task_widget.detail.header.penalty_reject_fail",
                L"거절/실패 시 불이익" },
            { L"task_widget.detail.header.defer", L"보류" },
            { L"task_widget.feedback.no_era_transition_command",
                L"시대 전환 명령을 확인할 수 없습니다." },
            { L"task_widget.feedback.era_transition_unavailable",
                L"아직 시대 전환을 진행할 수 없습니다." },
            { L"task_widget.feedback.no_admin_command",
                L"행정 명령을 확인할 수 없습니다." },
            { L"task_widget.feedback.accepted", L"과제를 수락했습니다." },
            { L"task_widget.feedback.cleared", L"과제를 정리했습니다." }
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
                Key = Trim(Key);
                Value = Trim(Value);

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
