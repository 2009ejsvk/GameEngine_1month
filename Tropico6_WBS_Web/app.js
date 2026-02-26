/* =========================================================
   Tropico Engine Plan (2D) — Portfolio Pitch Deck
   App Logic + Dynamic Rendering + Animations
   v2 — Critical fixes + Polish
   ========================================================= */

const dataModel = {
  heroMetrics: [
    { label: "개발 기간", value: "2026.02.26 ~ 03.26", icon: "📅" },
    { label: "목표 플레이", value: "30~45분", icon: "🎮" },
    { label: "참조 스타일", value: "Tropico 1 (2D)", icon: "🧭" },
    { label: "구현 환경", value: "2D 고정 격자", icon: "🔲" }
  ],
  portfolioBadges: [
    { title: "Playable First", desc: "1개월 안에 실제 플레이 가능 상태를 증명", tone: "teal" },
    { title: "Engine Base", desc: "학습용으로 제공된 언리얼 엔진 구조의 DirectX11 구현 버전을 기반으로 개발", tone: "gold" },
    { title: "2D Visual Direction", desc: "Tropico 1 스타일 2D 시각 구성을 기준으로 UI/배치 규칙을 통일", tone: "orange" }
  ],
  coreLoop: ["건설", "생산/물류", "시민 생활/행복", "정치 대응"],
  doneCriteria: [
    "2개 섬에서 건설/생산/물류 루프가 끊기지 않는다.",
    "시민 이동이 행복도와 생산성에 실질적으로 반영된다.",
    "50~200 NPC가 개별 상태(FSM) 기반으로 경로를 따라 이동/행동한다.",
    "파벌/선거/칙령으로 정치적 압박이 재현된다.",
    "식민지 → 세계대전 전환이 플레이 흐름에 적용된다.",
    "2D 고정 격자 환경에서 주요 시스템 플레이가 가능하다.",
    "30~45분 플레이에서 흑자/적자/정치 위기를 모두 경험한다."
  ],
  successMeters: [
    { label: "루프 연동도", value: 88, detail: "건설-경제-정치가 한 세션에서 연결" },
    { label: "시스템 가시성", value: 82, detail: "HUD/오버레이에서 상태 즉시 확인" },
    { label: "데모 완성도", value: 80, detail: "포트폴리오 데모 발표 기준 충족" }
  ],
  mvpScope: [
    { title: "군도 2개 섬 + 연결 인프라", note: "도로/운송항 단순화 포함" },
    { title: "건설/철거/예산 조정 UI", note: "실시간 운영 피드백 중심" },
    { title: "시민 시뮬레이션", note: "직업/주거/서비스 이동 FSM" },
    { title: "경제 루프", note: "원자재 → 가공 → 수출 체인 2개" },
    { title: "정치 루프", note: "파벌 4개 + 선거 + 칙령 3종" },
    { title: "시대 전환 2단계", note: "식민지 → 세계대전" },
    { title: "습격 최소형", note: "해적 소굴 + 임무 3종" },
    { title: "오버레이 3종", note: "고용/교통/서비스 커버리지" }
  ],
  expansionCandidates: [
    { title: "4시대 완전 구현", trigger: "월간 KPI 달성 및 인력 여유 확보 시" },
    { title: "세계 불가사의 풀셋", trigger: "습격 메타 검증 완료 시" },
    { title: "관광 메타 전체", trigger: "경제 밸런스 고정 이후" },
    { title: "브로커/스위스 계좌 고도화", trigger: "정치 루프 안정 후 2차 확장" },
    { title: "15개 미션 캠페인 구조", trigger: "콘텐츠 제작 파이프라인 확보 시" }
  ],
  constructionOriginal: [
    "Tropico 1 방식의 2D 아이소메트릭 타일 기반 시점",
    "건물/도로가 타일 단위로 배치되고 연결 상태가 즉시 표현",
    "지면→도로→건물 순 레이어 렌더링으로 가시성 확보"
  ],
  constructionMvp: [
    "Tropico 1 스타일 아이소메트릭 타일맵(마름모) 좌표 변환으로 고정 시점 구현",
    "건물 배치: 그리드 스냅 + 크기별 타일 점유(1x1, 2x2, 3x3)",
    "도로: 타일 배치 후 8방향 인접 체크로 자동 스프라이트 선택",
    "도로 연결 그래프를 길찾기(A*) 입력으로 사용"
  ],
  constructionRoadCards: [
    { title: "직선/커브 자동화", note: "도로 인접 패턴에 따라 시각 자산 자동 교체" },
    { title: "교차로 처리", note: "T자/4방 교차를 인접도 기반으로 자동 분기" },
    { title: "건물 연결 검증", note: "도로 미연결 건물은 물류/시민 접근 불가 처리" },
    { title: "배치 즉시 반영", note: "건설/철거 직후 그래프 갱신으로 경로 재계산" }
  ],
  productionOriginal: [
    "4개 시대: 식민지 → 세계대전 → 냉전 → 현대",
    "원자재 → 가공 → 고급품 단계형 생산 체인",
    "건물별 작업 모드/효율/예산(5단계) 운영",
    "자원 비옥도/고갈 메카닉으로 장기 운영 압박"
  ],
  productionMvp: [
    "2단계 체인: 원자재(농장/광산) → 가공(공장) → 수출(항구)",
    "자원 4종: 사탕수수/목재/광물/면화",
    "건물 패널에 작업자/효율/재고량 표시",
    "시대는 식민지 1종만 구현 후 확장 후보로 분리"
  ],
  productionChainFlow: ["원자재 생산", "가공", "항구 집하", "수출"],
  productionResourceCards: [
    { title: "사탕수수 → 럼", note: "농장 → 럼 공장 → 항구" },
    { title: "목재 → 판자", note: "벌목 캠프 → 판자 공장 → 항구" },
    { title: "광물 → 무기", note: "광산 → 무기 공장 → 항구" },
    { title: "면화 → 직물", note: "면화 농장 → 직물 공장 → 항구" }
  ],
  logisticsOriginal: [
    "Teamster 건물이 트럭 NPC를 생성해 자원 수거/배달",
    "Teamster 부족 시 생산 라인이 정지하며 경제가 붕괴",
    "항구가 수출/수입 허브 역할 수행",
    "시민은 집→직장→서비스→집 경로로 이동"
  ],
  logisticsMvp: [
    "기존 엔진 CThreadNavigation 기반 A* 경로탐색 재사용",
    "시민 경로(집↔직장↔서비스)를 도로 그래프에서 계산",
    "Teamster 경로(생산→가공→항구) 배달 루프 구현",
    "도로 미연결 시 이동 불가: 도로 그래프가 곧 이동 권한"
  ],
  logisticsKeyPoints: ["A* 경로탐색", "도로 그래프", "Teamster 큐", "항구 자동 수출"],
  logisticsRouteCards: [
    { title: "시민 이동", note: "생활 루프를 우선순위 없는 상시 경로로 운용" },
    { title: "물류 운송", note: "생산물 우선순위 큐로 정체 시 핵심 자원 보호" },
    { title: "항구 처리", note: "도착 자원은 즉시 수출 금액으로 환산" },
    { title: "교통 병목", note: "재경로 최소 간격으로 과도한 재탐색 방지" }
  ],
  happinessOriginal: [
    "행복도는 음식/의료/오락/신앙/주거/직업/자유/치안 평균값",
    "주거 품질과 직장 거리, 서비스 접근성이 생활 만족에 영향",
    "행복도는 지지율과 선거 결과로 직결",
    "불만 누적 시 반란/정치 위기로 이어짐"
  ],
  happinessMvp: [
    "행복 5요소: 음식/주거/의료/오락/치안 우선 구현",
    "각 요소는 서비스 건물 커버리지 x 품질 점수로 계산",
    "NPC 개별 행복도 평균을 전체 지지율로 반영",
    "행복 하락 시 반란 위험도 상승 이벤트 트리거"
  ],
  happinessFactors: ["음식", "주거", "의료", "오락", "치안"],
  happinessMeters: [
    { label: "음식 커버리지", value: 82, detail: "식량 공급과 접근성 기반" },
    { label: "주거 만족도", value: 74, detail: "주거 품질과 직장 거리 반영" },
    { label: "서비스 접근성", value: 79, detail: "의료/오락/치안 시설 거리 반영" }
  ],
  politicsOriginal: [
    "8개 파벌(자본가/공산/군사/자유/환경/지식인/보수/종교) 운영",
    "파벌은 건물/정책 선호에 따라 지지도를 변화",
    "선거는 지지율 기반이며 연설/뇌물/약속으로 개입 가능",
    "파벌 불만 누적 시 최후통첩과 반란으로 확산"
  ],
  politicsMvp: [
    "파벌 4개: 자본가, 공산주의, 군사, 종교",
    "건물/칙령에 따른 파벌별 선호도 modifier 적용",
    "4년 주기 선거: 전체 행복도 + 파벌 지지 합산 투표",
    "임계값 미만 파벌은 최후통첩 후 반란 이벤트 진입"
  ],
  factions: [
    { title: "자본가", note: "세금 감면, 산업 건물 확장을 선호" },
    { title: "공산주의", note: "복지/분배 정책, 음식 배급 칙령 선호" },
    { title: "군사", note: "치안 강화, 계엄령 등 강경 통치 선호" },
    { title: "종교", note: "종교 시설 투자와 도덕 규범 정책 선호" }
  ],
  decrees: [
    { title: "음식 배급", note: "공산 지지 상승, 재정 비용 증가" },
    { title: "세금 감면", note: "자본 지지 상승, 단기 세수 감소" },
    { title: "계엄령", note: "군사 지지 상승, 자유·행복 패널티" }
  ],
  reuseList: [
    "학습용 언리얼 엔진 구조를 DirectX11로 구현한 월드 프레임(CWorld)",
    "CTileMapObject / CTileMapComponent 기반 2D 고정 격자",
    "CWorldNavigation / CThreadNavigation",
    "CWidgetContainer / CButton / CTextBlock",
    "CInput 바인딩 시스템"
  ],
  newSystems: [
    "SimulationClockSystem (틱/배속)",
    "BuildingSystem (배치/철거/예산/전력 요구)",
    "CitizenSystem (FSM, 배정, 이동 요청)",
    "LogisticsSystem (팀스터 운송 큐)",
    "EconomySystem (생산/소비/수출/재정)",
    "PoliticsSystem (파벌/선거/칙령)",
    "RaidSystem (습격 점수/임무 큐)",
    "OverlaySystem (히트맵/상태 시각화)"
  ],
  architectureFlow: [
    { title: "Engine Core", sub: "학습용 언리얼 구조의 DirectX11 구현 (World / TileMap / Nav / UI)" },
    { title: "MVP Systems", sub: "2D 고정 격자 기반 Citizen / Economy / Politics / Raid" },
    { title: "Playable Outcome", sub: "30~45분 통치 시뮬레이션 데모" }
  ],
  principles: [
    "데이터 우선(테이블 기반)",
    "렌더·시뮬 틱 분리",
    "Tropico 1 스타일 2D 고정 격자/고정 카메라",
    "원거리 시민 저빈도 업데이트",
    "경로탐색 호출 수 제한",
    "플레이 긴장감 우선"
  ],
  risks: [
    "경로탐색 과부하 → 이동 요청 배치 처리 + 재경로 최소 간격",
    "물류 병목 → 운송 우선순위(원자재/필수재) 도입",
    "UI 복잡도 증가 → 운영 HUD와 건설 패널 분리",
    "범위 초과 → 3주차 말 기능 동결, 4주차 안정화 전용"
  ],
  riskMap: [
    { short: "NAV", title: "경로탐색 과부하", probability: 72, impact: 82, tone: "warn" },
    { short: "LOG", title: "물류 병목", probability: 64, impact: 76, tone: "warn" },
    { short: "UI", title: "UI 복잡도", probability: 48, impact: 52, tone: "mid" },
    { short: "SCP", title: "범위 초과", probability: 58, impact: 88, tone: "risk" }
  ],
  webPages: [
    "개요: 목표, 범위, KPI",
    "시스템 설계: 경제/시민/정치/습격",
    "WBS & 일정: 2026.02.26 ~ 2026.03.26",
    "테스트 리포트: 주차별 성능/버그/밸런싱"
  ],
  portfolioProof: [
    { label: "기획-구현 연결", value: "문서 → WBS → 데모" },
    { label: "기술 설득력", value: "학습용 언리얼 구조(DX11) 기반 + 신규 시스템" },
    { label: "확장성", value: "여건 확보 시 확장 후보 즉시 투입" }
  ],
  weeklyGoals: {
    1: "월드 골격 + 건설 + 군도 맵 + 데이터 로딩",
    2: "시민/물류/생산/재정 루프 완성",
    3: "행복-정치-선거-시대전환-습격 연결",
    4: "HUD/오버레이 + 밸런싱 + 성능 + 데모 고정"
  },
  wbs: [
    { id: "1.1", title: "범위 확정, KPI 정의", days: 0.5, dep: "-", done: "MVP 체크리스트 동결", week: 1 },
    { id: "1.2", title: "성능 스파이크(시민 200명+생산틱)", days: 0.5, dep: "1.1", done: "성능 목표 기준 수립", week: 1 },
    { id: "1.3", title: "CTropicoWorld/시뮬레이션 루프 뼈대", days: 0.5, dep: "1.1", done: "월드 생성 및 틱 동작", week: 1 },
    { id: "1.4", title: "데이터 스키마 + 로더", days: 1.0, dep: "1.3", done: "초기 데이터 로드 성공", week: 1 },
    { id: "1.5", title: "건설 모드(배치/철거/유효성)", days: 1.5, dep: "1.4", done: "기본 건물 배치 가능", week: 1 },
    { id: "1.6", title: "군도 타일 메타(섬 ID/연결/도로)", days: 1.5, dep: "1.5", done: "2개 섬 맵 운용", week: 1 },
    { id: "1.7", title: "저장/불러오기(시뮬레이션 상태)", days: 1.0, dep: "1.6", done: "세션 복원 성공", week: 1 },
    { id: "2.1", title: "시민 FSM(집-직장-서비스-집)", days: 1.0, dep: "1.6", done: "시민 루프 가동", week: 2 },
    { id: "2.2", title: "직업/주거 매칭 로직", days: 1.0, dep: "2.1", done: "무직/노숙 지표 계산", week: 2 },
    { id: "2.3", title: "이동 요청-내비 연동 최적화", days: 0.5, dep: "2.1", done: "프레임 저하 없는 이동", week: 2 },
    { id: "2.4", title: "물류 시스템(팀스터 큐)", days: 1.0, dep: "2.2", done: "생산물 운송 가능", week: 2 },
    { id: "2.5", title: "생산 체인 2개 구현", days: 1.0, dep: "2.4", done: "수출 매출 발생", week: 2 },
    { id: "2.6", title: "재정/건물 예산 슬라이더", days: 0.5, dep: "2.5", done: "예산 변경 시 효율 변화", week: 2 },
    { id: "3.1", title: "행복도 지표 계산", days: 1.0, dep: "2.6", done: "행복 분포 출력", week: 3 },
    { id: "3.2", title: "파벌 4개 지지도/요구 처리", days: 1.0, dep: "3.1", done: "파벌 영향 반영", week: 3 },
    { id: "3.3", title: "선거 시스템(주기/승패)", days: 1.0, dep: "3.2", done: "선거 이벤트 동작", week: 3 },
    { id: "3.4", title: "칙령 3종(효과/페널티)", days: 0.5, dep: "3.3", done: "칙령 발동/쿨다운", week: 3 },
    { id: "3.5", title: "시대 전환(식민지→세계대전)", days: 0.5, dep: "3.3", done: "전환 조건/효과 적용", week: 3 },
    { id: "3.6", title: "습격 최소형(해적 소굴+임무3종)", days: 1.0, dep: "3.5", done: "임무 큐/보상 동작", week: 3 },
    { id: "4.1", title: "HUD/오버레이(고용/물류/행복)", days: 1.0, dep: "3.6", done: "디버그 시각화", week: 4 },
    { id: "4.2", title: "밸런싱 1차", days: 1.0, dep: "4.1", done: "30분 플레이 성립", week: 4 },
    { id: "4.3", title: "성능/안정화", days: 1.0, dep: "4.2", done: "목표 FPS 달성", week: 4 },
    { id: "4.4", title: "QA + 데모 빌드 + 문서 정리", days: 0.5, dep: "4.3", done: "데모 패키지 완료", week: 4 }
  ]
};

/* ── State ─────────────────────────────────── */
const state = { activeIndex: 0, weekFilter: "all" };
const byId = (id) => document.getElementById(id);

/* ── D-Day Calculation ─────────────────────── */
function getDDay() {
  const deadline = new Date("2026-03-26T23:59:59+09:00");
  const now = new Date();
  const diff = Math.ceil((deadline - now) / (1000 * 60 * 60 * 24));
  return diff > 0 ? `D-${diff}` : diff === 0 ? "D-Day" : `D+${Math.abs(diff)}`;
}

/* ── Count-Up Animation ────────────────────── */
function animateCountUp(el, target, suffix = "", duration = 1200) {
  const startTime = performance.now();
  function tick(now) {
    const p = Math.min((now - startTime) / duration, 1);
    const eased = 1 - Math.pow(1 - p, 3);
    el.textContent = Math.round(target * eased) + suffix;
    if (p < 1) requestAnimationFrame(tick);
  }
  requestAnimationFrame(tick);
}

/* ── Render Functions ──────────────────────── */
function renderList(targetId, items) {
  const t = byId(targetId);
  if (t) t.innerHTML = items.map(i => `<li>${i}</li>`).join("");
}

function renderFlow(targetId, steps) {
  const t = byId(targetId);
  if (!t) return;
  t.innerHTML = steps.map((step, i) => `
    <div class="loop-step">
      <span class="loop-index">${String(i + 1).padStart(2, "0")}</span>
      <strong>${step}</strong>
    </div>
    ${i < steps.length - 1 ? '<div class="loop-arrow">→</div>' : ""}
  `).join("");
}

function renderMeters(targetId, meters) {
  const t = byId(targetId);
  if (!t) return;
  t.innerHTML = meters.map(m => `
    <article class="meter-card">
      <div class="meter-head">
        <strong>${m.label}</strong>
        <span class="meter-value" data-target="${m.value}">0%</span>
      </div>
      <div class="meter-track">
        <div class="meter-fill" data-width="${m.value}" style="width:0%"></div>
      </div>
      <p>${m.detail}</p>
    </article>
  `).join("");
}

function renderChips(targetId, items) {
  const t = byId(targetId);
  if (t) t.innerHTML = items.map(item => `<span class="chip">${item}</span>`).join("");
}

function renderInfoCards(targetId, items, tag) {
  const t = byId(targetId);
  if (!t) return;
  t.innerHTML = items.map((item, i) => `
    <article class="scope-card">
      <div class="scope-card-top">
        <span class="scope-index">${String(i + 1).padStart(2, "0")}</span>
        <span class="scope-tag">${tag}</span>
      </div>
      <h3>${item.title}</h3>
      <p>${item.note}</p>
    </article>
  `).join("");
}

function renderHeroMetrics() {
  const t = byId("heroMetrics");
  if (!t) return;
  t.innerHTML = dataModel.heroMetrics.map(m => `
    <article class="metric-card">
      <span>${m.icon} ${m.label}</span>
      <strong>${m.value}</strong>
    </article>
  `).join("");
}

function renderDDay() {
  const t = byId("ddayBadge");
  if (t) t.textContent = getDDay();
}

function renderSlideTitle(slides) {
  const t = byId("slideTitle");
  if (!t || !slides[state.activeIndex]) return;
  const title = slides[state.activeIndex].dataset.title || "";
  t.textContent = title;
}

function renderPortfolioBadges() {
  const t = byId("portfolioBadges");
  if (!t) return;
  t.innerHTML = dataModel.portfolioBadges.map(b => `
    <article class="portfolio-badge tone-${b.tone}">
      <h3>${b.title}</h3>
      <p>${b.desc}</p>
    </article>
  `).join("");
}

function renderLoopFlow() {
  renderFlow("loopFlow", dataModel.coreLoop);
}

function renderSuccessMeters() {
  renderMeters("successMeters", dataModel.successMeters);
}

function renderScopeCards(targetId, items, mode) {
  const t = byId(targetId);
  if (!t) return;
  t.innerHTML = items.map((item, i) => `
    <article class="scope-card ${mode}">
      <div class="scope-card-top">
        <span class="scope-index">${String(i + 1).padStart(2, "0")}</span>
        <span class="scope-tag">${mode === "mvp" ? "MVP" : "EXPAND"}</span>
      </div>
      <h3>${item.title}</h3>
      <p>${mode === "mvp" ? item.note : item.trigger}</p>
    </article>
  `).join("");
}

function renderPrinciples() {
  renderChips("principles", dataModel.principles);
}

function renderSystemSlides() {
  renderList("constructionOriginal", dataModel.constructionOriginal);
  renderList("constructionMvp", dataModel.constructionMvp);
  renderInfoCards("constructionRoadCards", dataModel.constructionRoadCards, "ROAD");

  renderList("productionOriginal", dataModel.productionOriginal);
  renderList("productionMvp", dataModel.productionMvp);
  renderFlow("productionChainFlow", dataModel.productionChainFlow);
  renderInfoCards("productionResourceCards", dataModel.productionResourceCards, "CHAIN");

  renderList("logisticsOriginal", dataModel.logisticsOriginal);
  renderList("logisticsMvp", dataModel.logisticsMvp);
  renderChips("logisticsKeyPoints", dataModel.logisticsKeyPoints);
  renderInfoCards("logisticsRouteCards", dataModel.logisticsRouteCards, "ROUTE");

  renderList("happinessOriginal", dataModel.happinessOriginal);
  renderList("happinessMvp", dataModel.happinessMvp);
  renderChips("happinessFactors", dataModel.happinessFactors);
  renderMeters("happinessMeters", dataModel.happinessMeters);

  renderList("politicsOriginal", dataModel.politicsOriginal);
  renderList("politicsMvp", dataModel.politicsMvp);
  renderInfoCards("factionCards", dataModel.factions, "FACTION");
  renderInfoCards("decreeCards", dataModel.decrees, "DECREE");
}

function renderEngineMix() {
  const t = byId("engineMix");
  if (!t) return;
  const r = dataModel.reuseList.length, n = dataModel.newSystems.length, tot = r + n;
  const rr = Math.round((r / tot) * 100), nr = 100 - rr;
  t.innerHTML = `
    <article class="mix-card">
      <h3>구현 구성 비율</h3>
      <div class="mix-row"><span>🔧 기반 엔진</span><strong>${r}</strong></div>
      <div class="mix-row"><span>✨ 신규</span><strong>${n}</strong></div>
      <div class="mix-track">
        <div class="mix-fill reuse" data-width="${rr}" style="width:0%"></div>
        <div class="mix-fill fresh" data-width="${nr}" style="width:0%"></div>
      </div>
      <p>기반 엔진 ${rr}% · 신규 ${nr}%</p>
    </article>
  `;
}

function renderArchitectureFlow() {
  const t = byId("architectureFlow");
  if (!t) return;
  t.innerHTML = dataModel.architectureFlow.map((node, i) => `
    <article class="arch-node">
      <h3>${node.title}</h3>
      <p>${node.sub}</p>
    </article>
    ${i < dataModel.architectureFlow.length - 1 ? '<div class="arch-link">→</div>' : ""}
  `).join("");
}

function renderRoadmap() {
  const w = byId("roadmap");
  if (!w) return;
  const icons = { 1: "🏗️", 2: "👥", 3: "⚖️", 4: "🎯" };
  w.innerHTML = [1, 2, 3, 4].map(week => {
    const tasks = dataModel.wbs.filter(t => t.week === week);
    const days = tasks.reduce((s, t) => s + t.days, 0).toFixed(1);
    const cards = tasks.map(t => `<div class="roadmap-task">${t.id} ${t.title}</div>`).join("");
    return `<article class="roadmap-col">
      <h3>${icons[week]} ${week}주차 · ${days}일</h3>
      <p class="roadmap-goal">${dataModel.weeklyGoals[week]}</p>
      ${cards}
    </article>`;
  }).join("");
}

function renderWeekHeat() {
  const t = byId("weekHeat");
  if (!t) return;
  const icons = { 1: "🏗️", 2: "👥", 3: "⚖️", 4: "🎯" };
  const weeks = [1, 2, 3, 4].map(w => ({
    week: w,
    days: dataModel.wbs.filter(x => x.week === w).reduce((s, c) => s + c.days, 0),
    goal: dataModel.weeklyGoals[w]
  }));
  const max = Math.max(...weeks.map(w => w.days));
  t.innerHTML = weeks.map(w => `
    <article class="heat-card">
      <div class="heat-top">
        <strong>${icons[w.week]} ${w.week}주차</strong>
        <span>${w.days.toFixed(1)}일</span>
      </div>
      <div class="heat-track">
        <div class="heat-fill" data-width="${Math.round((w.days / max) * 100)}" style="width:0%"></div>
      </div>
      <p>${w.goal}</p>
    </article>
  `).join("");
}

function buildWeekTabs() {
  const tabs = byId("weekTabs");
  if (!tabs) return;
  const cfg = [
    { value: "all", label: "전체" },
    { value: "1", label: "1주차" },
    { value: "2", label: "2주차" },
    { value: "3", label: "3주차" },
    { value: "4", label: "4주차" }
  ];
  tabs.innerHTML = cfg.map(t =>
    `<button class="week-btn ${t.value === "all" ? "is-active" : ""}" data-week="${t.value}">${t.label}</button>`
  ).join("");
  tabs.querySelectorAll(".week-btn").forEach(btn => {
    btn.addEventListener("click", () => {
      state.weekFilter = btn.dataset.week;
      tabs.querySelectorAll(".week-btn").forEach(b => b.classList.remove("is-active"));
      btn.classList.add("is-active");
      renderWbsRows();
    });
  });
}

function renderWbsRows() {
  const rows = state.weekFilter === "all"
    ? dataModel.wbs
    : dataModel.wbs.filter(t => String(t.week) === state.weekFilter);
  const body = byId("wbsRows"), summary = byId("wbsSummary");
  if (!body || !summary) return;
  body.innerHTML = rows.map((t, i) => `
    <tr style="animation: tableRowIn 0.35s ${i * 0.025}s both">
      <td class="id">${t.id}</td>
      <td>${t.title}</td>
      <td>${t.days}</td>
      <td>${t.dep}</td>
      <td>${t.done}</td>
      <td><span class="week-pill">${t.week}주차</span></td>
    </tr>
  `).join("");
  const sum = rows.reduce((a, c) => a + c.days, 0);
  summary.innerHTML = `표시 작업 <strong>${rows.length}개</strong> · 기간 합계 <strong>${sum.toFixed(1)}일</strong> · 전체 기간 <strong>2026.02.26 ~ 2026.03.26</strong>`;
}

function renderWbsKpis() {
  const t = byId("wbsKpis");
  if (!t) return;
  const totalDays = dataModel.wbs.reduce((s, c) => s + c.days, 0);
  const wc = [1, 2, 3, 4].map(w => ({ week: w, count: dataModel.wbs.filter(x => x.week === w).length }));
  const busiest = wc.sort((a, b) => b.count - a.count)[0];
  const kpis = [
    { label: "전체 작업", value: `${dataModel.wbs.length}개`, icon: "📋" },
    { label: "프로젝트 기간", value: "02.26~03.26", icon: "📆" },
    { label: "예상 공수", value: `${totalDays.toFixed(1)}일`, icon: "⏱️" },
    { label: "집중 주차", value: `${busiest.week}주차`, icon: "🔥" }
  ];
  t.innerHTML = kpis.map(k => `
    <article class="kpi-mini">
      <span>${k.icon} ${k.label}</span>
      <strong>${k.value}</strong>
    </article>
  `).join("");
}

function renderRiskMatrix() {
  const t = byId("riskMatrix");
  if (!t) return;
  const dots = dataModel.riskMap.map(r => `
    <button class="risk-dot ${r.tone}" style="left:${r.probability}%;bottom:${r.impact}%"
            title="${r.title} — 확률 ${r.probability}% / 영향 ${r.impact}%">${r.short}</button>
  `).join("");
  const legends = dataModel.riskMap.map(r =>
    `<li><strong>${r.short}</strong> ${r.title} <span style="opacity:0.5">(${r.probability}%/${r.impact}%)</span></li>`
  ).join("");
  t.innerHTML = `
    <div class="risk-grid">
      <span class="risk-axis x">확률 →</span>
      <span class="risk-axis y">↑ 영향도</span>
      ${dots}
    </div>
    <ul class="risk-legend">${legends}</ul>
  `;
}

function renderProofStrip() {
  const t = byId("portfolioProof");
  if (!t) return;
  t.innerHTML = dataModel.portfolioProof.map(p => `
    <article class="proof-card">
      <span>${p.label}</span>
      <strong>${p.value}</strong>
    </article>
  `).join("");
}

function renderIfPossiblePlan() {
  const t = byId("ifPossiblePlan");
  if (!t) return;
  t.innerHTML = dataModel.expansionCandidates.slice(0, 3).map(c => `
    <article class="if-card">
      <strong>${c.title}</strong>
      <p>${c.trigger}</p>
    </article>
  `).join("");
}

/* ── Animated Bars on Scroll ───────────────── */
function setupBarAnimations() {
  const obs = new IntersectionObserver((entries) => {
    entries.forEach(entry => {
      if (!entry.isIntersecting) return;
      entry.target.querySelectorAll("[data-width]").forEach(bar => {
        const w = bar.getAttribute("data-width");
        setTimeout(() => { bar.style.width = w + "%"; }, 250);
      });
      entry.target.querySelectorAll(".meter-value[data-target]").forEach(el => {
        const v = parseInt(el.dataset.target, 10);
        setTimeout(() => animateCountUp(el, v, "%", 1200), 250);
      });
      obs.unobserve(entry.target);
    });
  }, { threshold: 0.2 });
  document.querySelectorAll(".slide").forEach(s => obs.observe(s));
}

/* ── Table Row Animation ───────────────────── */
function injectAnimStyles() {
  if (document.getElementById("deckDynamicStyles")) return;
  const s = document.createElement("style");
  s.id = "deckDynamicStyles";
  s.textContent = `
    @keyframes tableRowIn {
      from { opacity: 0; transform: translateX(-10px); }
      to { opacity: 1; transform: translateX(0); }
    }
  `;
  document.head.appendChild(s);
}

/* ══════════════════════════════════════════════
   NAVIGATION — Fixed: no more blank slides
   ══════════════════════════════════════════════ */

function buildSlideNav(slides) {
  const outline = byId("outlineList"), dotNav = byId("dotNav");
  if (!outline || !dotNav) return;

  outline.innerHTML = slides.map((s, i) => {
    const title = s.dataset.title || `슬라이드 ${i + 1}`;
    return `<li><button class="outline-btn" data-index="${i}"><span>${i + 1}.</span>${title}</button></li>`;
  }).join("");

  dotNav.innerHTML = slides.map((s, i) =>
    `<button class="dot-btn" data-index="${i}" title="${s.dataset.title || 'slide'}"></button>`
  ).join("");

  [...outline.querySelectorAll(".outline-btn"), ...dotNav.querySelectorAll(".dot-btn")].forEach(btn => {
    btn.addEventListener("click", () => goToSlide(slides, Number(btn.dataset.index)));
  });
}

function updateNavState(slides, idx) {
  const counter = byId("slideCounter"), fill = byId("progressFill");
  if (counter) counter.textContent = `${idx + 1} / ${slides.length}`;
  if (fill) fill.style.width = `${((idx + 1) / slides.length) * 100}%`;
  document.querySelectorAll(".outline-btn").forEach(b =>
    b.classList.toggle("is-active", Number(b.dataset.index) === idx));
  document.querySelectorAll(".dot-btn").forEach(b =>
    b.classList.toggle("is-active", Number(b.dataset.index) === idx));
  renderSlideTitle(slides);
}

/* FIX: Always reveal immediately after navigation */
function revealSlide(slide) {
  if (!slide) return;
  const items = slide.querySelectorAll(".reveal");
  items.forEach((item, i) => {
    item.classList.remove("is-visible");
    setTimeout(() => item.classList.add("is-visible"), 60 * i + 50);
  });
}

/* FIX: goToSlide manually sets index + reveals to prevent blank slides */
function goToSlide(slides, index) {
  const target = Math.max(0, Math.min(slides.length - 1, index));
  state.activeIndex = target;
  updateNavState(slides, target);

  /* Use instant scroll to avoid snap conflicts, then reveal */
  slides[target].scrollIntoView({ behavior: "smooth", block: "start" });

  /* Ensure reveal fires even if IntersectionObserver misses it */
  setTimeout(() => revealSlide(slides[target]), 120);
}

function bindDeckControls(slides) {
  const prevBtn = byId("prevBtn"), nextBtn = byId("nextBtn"), fullBtn = byId("fullBtn");
  if (prevBtn) prevBtn.addEventListener("click", () => goToSlide(slides, state.activeIndex - 1));
  if (nextBtn) nextBtn.addEventListener("click", () => goToSlide(slides, state.activeIndex + 1));
  if (fullBtn) fullBtn.addEventListener("click", async () => {
    if (!document.fullscreenElement) await document.documentElement.requestFullscreen();
    else await document.exitFullscreen();
  });

  document.addEventListener("keydown", (e) => {
    const k = e.key.toLowerCase();
    if (["arrowdown", "pagedown", " ", "arrowright"].includes(k)) {
      e.preventDefault(); goToSlide(slides, state.activeIndex + 1);
    } else if (["arrowup", "pageup", "arrowleft"].includes(k)) {
      e.preventDefault(); goToSlide(slides, state.activeIndex - 1);
    } else if (k === "home") {
      e.preventDefault(); goToSlide(slides, 0);
    } else if (k === "end") {
      e.preventDefault(); goToSlide(slides, slides.length - 1);
    } else if (k === "f") {
      e.preventDefault();
      if (!document.fullscreenElement) document.documentElement.requestFullscreen();
      else document.exitFullscreen();
    }
  });
}

/* FIX: Lower threshold + also force reveal on intersection */
function watchSlides(slides) {
  const deck = byId("deck");
  if (!deck) return;
  const obs = new IntersectionObserver((entries) => {
    entries.forEach(entry => {
      if (!entry.isIntersecting || entry.intersectionRatio < 0.3) return;
      const idx = slides.indexOf(entry.target);
      if (idx === -1) return;
      state.activeIndex = idx;
      updateNavState(slides, idx);
      revealSlide(slides[idx]);
    });
  }, { root: deck, threshold: [0.3, 0.5, 0.7] });
  slides.forEach(s => obs.observe(s));
}

/* ── Bootstrap ─────────────────────────────── */
function bootstrap() {
  injectAnimStyles();

  renderHeroMetrics();
  renderDDay();
  renderPortfolioBadges();
  renderLoopFlow();
  renderList("doneCriteria", dataModel.doneCriteria);
  renderSuccessMeters();
  renderScopeCards("inScopeCards", dataModel.mvpScope, "mvp");
  renderScopeCards("outScopeCards", dataModel.expansionCandidates, "expand");
  renderSystemSlides();
  renderList("reuseList", dataModel.reuseList);
  renderList("newSystems", dataModel.newSystems);
  renderPrinciples();
  renderEngineMix();
  renderArchitectureFlow();
  renderWeekHeat();
  renderRoadmap();
  buildWeekTabs();
  renderWbsRows();
  renderWbsKpis();
  renderList("risks", dataModel.risks);
  renderRiskMatrix();
  renderList("webPages", dataModel.webPages);
  renderProofStrip();
  renderIfPossiblePlan();

  const slides = [...document.querySelectorAll(".slide")];
  buildSlideNav(slides);
  updateNavState(slides, 0);

  /* Reveal first slide immediately */
  revealSlide(slides[0]);

  bindDeckControls(slides);
  watchSlides(slides);
  setupBarAnimations();

  /* Update D-Day every minute */
  setInterval(renderDDay, 60000);
}

bootstrap();
