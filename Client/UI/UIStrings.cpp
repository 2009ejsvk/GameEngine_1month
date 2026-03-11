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
            { L"citizen_info.tab.building.overview", L"기본" },
            { L"citizen_info.tab.building.statistics", L"통계" },
            { L"citizen_info.tab.building.upgrades", L"업글" },
            { L"citizen_info.tab.building.efficiency", L"효율" },
            { L"citizen_info.tab.building.information", L"정보" },
            { L"citizen_info.page.building.overview", L"" },
            { L"citizen_info.page.building.statistics", L"통계" },
            { L"citizen_info.page.building.upgrades", L"업그레이드" },
            { L"citizen_info.page.building.efficiency", L"효율" },
            { L"citizen_info.page.building.information", L"정보" },
            { L"citizen_info.politics.section.satisfaction", L"만족도" },
            { L"citizen_info.politics.section.opinion", L"견해" },
            { L"citizen_info.politics.section.support", L"지지도" },
            { L"citizen_info.thought.title", L"생각" },
            { L"citizen_info.action.demolish", L"철거" },
            { L"citizen_info.action.move", L"이동" },
            { L"citizen_info.action.clone", L"복제" },
            { L"citizen_info.building.data_pending", L"건물 데이터 준비 중입니다." },
            { L"citizen_info.activity.move", L"이동" },
            { L"citizen_info.subtitle.tropican", L"↠ 트로피코인 ↞" }
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
