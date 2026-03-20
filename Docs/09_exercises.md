# 09. 실습 과제

학습 자료를 바탕으로 실제 코드를 수정해보는 과제입니다.
각 과제는 난이도 순서로 배치되어 있습니다.

---

## 과제 1 — INI 핫리로드 체험 (난이도: 하)

**목표**: INI 파일을 수정해 게임 실행 중 UI가 변경되는 것을 확인한다.

**단계**:
1. 게임을 실행한다.
2. `Binary/UILayout/07_Almanac.ini` 를 텍스트 편집기로 연다.
3. `AlmanacMetricRowHeight` 값을 `46` → `80` 으로 변경한다.
4. 파일을 저장하고 게임의 알마낙을 열어본다.
5. 0.5초 이내에 행 높이가 변하는 것을 확인한다.

**확인 포인트**:
- 게임 재시작 없이 변경이 반영되었는가?
- `UILayoutConfig.cpp` 의 기본값과 INI 값 중 어느 것이 적용되는가?

---

## 과제 2 — 새 세금 유형 추가 (난이도: 중)

**목표**: `ETaxPolicyType` 에 새로운 `LuxuryTax`(사치세)를 추가한다.

**단계**:
1. `Client/Politics/PoliticalTypes.h` 를 열고 `ETaxPolicyType` 열거형에 `LuxuryTax` 를 추가한다.
2. `FTaxPolicy` 구조체에 `int LuxuryRatePercent = 0;` 필드를 추가한다.
3. `GetTaxPolicyDisplayName()` 에 L"사치세" 케이스를 추가한다.
4. `GetTaxPolicyDefaultPercent()`, `GetTaxPolicyMinPercent()`, `GetTaxPolicyMaxPercent()` 에 적절한 값을 추가한다.
5. `GetCitizenTaxBurdenNormalized()` 에서 사치세가 재산이 많은 시민에게 더 큰 가중치를 갖도록 수정한다.

**고민할 것**:
- 사치세는 어떤 시민에게 영향을 주어야 하는가?
- 가중치는 어떻게 결정하는가?

---

## 과제 3 — 새 파벌 요구 유형 추가 (난이도: 중)

**목표**: `EPoliticalDemandObjectiveType` 에 `Education`(교육 수준) 요구를 추가한다.

**단계**:
1. `Client/Politics/PoliticalTypes.h` 의 `EPoliticalDemandObjectiveType` 에 `Education` 추가
2. 관련 요구 생성 로직(`MainWorldPoliticalDemandService`)에서 교육 수준 달성 조건 추가
3. 목표 텍스트(`ObjectiveText`)를 적절히 설정
4. 알마낙 또는 `WorldStatsSnapshot` 에서 현재 교육 수준을 읽어 `CurrentValue` 갱신

**고민할 것**:
- 교육 수준은 어떻게 측정하는가? (학교 건물 수? 고학력 시민 비율?)
- 어떤 파벌이 이 요구를 낼 것인가?

---

## 과제 4 — 알마낙에 새 페이지 추가 (난이도: 중-상)

**목표**: 알마낙에 "교육" 페이지를 추가해 학력별 시민 분포를 표시한다.

**단계**:
1. `Client/UI/AlmanacPageData.h` 에서 페이지 유형 열거형에 `Education` 추가
2. `AlmanacDataProvider` 에서 교육 페이지 데이터를 조합
3. `AlmanacRendererPages*.cpp` 에서 교육 페이지 렌더 함수 구현
4. INI 파일(`07_Almanac.ini`)에 교육 페이지 레이아웃 값 추가

**참고할 파일**:
- `Client/UI/AlmanacRendererPagesPopulation.cpp` — 국민 페이지 구현 참고

---

## 과제 5 — 시민 만족도에 교육 인프라 반영 (난이도: 상)

**목표**: 학교/대학 건물과의 거리에 따라 시민의 만족도에 영향을 주도록 구현한다.

**단계**:
1. `CitizenCommuteCalc` 나 `CitizenSatisfaction` 에서 교육 건물과의 거리 계산 로직 추가
2. `EBuildingCategory::Civic` 중 교육 건물을 식별하는 필드 추가
3. 거리가 짧을수록 자유/교육 만족도 보너스, 멀수록 패널티 적용
4. `FCitizenInfoSnapshot` 에 교육 접근성 수치 추가해 UI에 표시

**시스템 이해 확인**:
- 만족도 변경이 즉시 반영되는가, 아니면 `DailyDelta` 를 통해 천천히 반영되는가?
- 어떤 경로로 건물 위치 정보를 시민 시스템이 접근하는가?

---

## 과제 6 — 미니 기능: 선거 여론조사 UI (난이도: 상)

**목표**: 선거 N일 전부터 TopHUD에 여론조사 수치를 표시한다.

**단계**:
1. `MainWorldElectionService` 에서 현재 지지율 추정치 계산 (일부 시민 샘플링)
2. `FElectionStatus` 에 `PollEstimate` 필드 추가
3. `TopHudDataProvider` 에서 선거가 임박했을 때 여론조사 수치를 제공
4. `TopHudRenderer` 에서 이를 표시

**고민할 것**:
- 여론조사가 항상 정확하면 재미가 없을 수 있습니다. 약간의 노이즈를 어떻게 추가할까요?
- 선거 얼마 전부터 여론조사를 보여줄 것인가? `GameBalanceTuning` 에 이 값을 두는 것이 좋을까요?

---

## 코드 리뷰 질문 (토론용)

다음 설계 결정에 대해 각자 의견을 작성하고 토론해보세요.

### Q1. 싱글턴 vs 의존성 주입
이 엔진은 `CEngine::GetInst()`, `CRenderManager::GetInst()` 등 싱글턴을 많이 씁니다.
- 싱글턴의 장단점은 무엇인가?
- 의존성 주입(DI)으로 바꾼다면 어떻게 달라지는가?
- 게임 엔진에서 싱글턴이 특히 많은 이유는 무엇인가?

### Q2. weak_ptr vs raw pointer
UI 위젯 멤버가 모두 `weak_ptr` 로 선언되어 있습니다.
- 왜 `unique_ptr` 나 `shared_ptr` 가 아닌 `weak_ptr` 인가?
- `.lock()` 을 매 프레임 호출하는 비용은 얼마나 될까?
- 이 패턴 대신 쓸 수 있는 다른 방법은?

### Q3. 스냅샷 캐싱의 적절한 주기
`WorldStatsSnapshot` 이 얼마나 자주 갱신되어야 하는가?
- 너무 자주 갱신하면? 너무 드물게 갱신하면?
- 어떤 데이터는 매 프레임 갱신이 필요하고, 어떤 데이터는 1초에 한 번으로 충분한가?

### Q4. 데이터 파일 포맷 선택
이 프로젝트는 INI 파일을 사용합니다.
- INI 대신 JSON, XML, YAML, 커스텀 바이너리 포맷의 장단점은?
- 런타임 핫리로드 지원 여부에 포맷 선택이 영향을 주는가?
- 대규모 게임에서는 어떤 포맷이 주로 쓰이는가?

---

## 심화 과제 — 나만의 칙령 설계

**목표**: 새 칙령을 처음부터 설계하고 구현한다.

1. `EGovernmentEdictType` 에 새 이름 추가 (예: `UniversalBasicIncome`)
2. 시대, 비용, 효과를 `Edicts.ini` 에 정의
3. `FGovernmentEdictModifiers` 에서 효과 적용 로직 구현
4. 어떤 파벌이 좋아하고 싫어하는지 `FactionApprovalModifiers` 설정
5. 칙령이 발동될 때 이벤트 메시지 작성

**평가 기준**:
- 기존 칙령들과 일관된 밸런스인가?
- 파벌별 반응이 파벌의 성격과 맞는가?
- 이 칙령이 플레이어에게 흥미로운 전략적 선택을 만드는가?
