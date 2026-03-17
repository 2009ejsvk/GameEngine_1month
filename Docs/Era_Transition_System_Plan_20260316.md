# Era Transition System Plan

## 목적

현재 프로젝트에는 `다음 시대로 넘어가기`를 플레이어가 인지하고 선택하는 시스템이 없다.
실제 상태는 `MainWorld::RefreshEraProgress()`가 조건을 만족하면 내부 `CurrentEra`를 즉시 올리는
자동 해금 방식이며, 전환 이벤트, 선택지, 연출, 외교 세력 전환, 후속 시스템 잠금 해제 단계가 없다.

이 문서는 다음 두 가지를 목표로 한다.

1. 현재 코드베이스에서 시대 전환을 어떤 구조로 구현할지 정리한다.
2. `C:\Program Files\Epic Games\Tropico6ModKit`에서 확인한 T6 자산 구조를 참고해,
   우리 프로젝트에 맞는 최소 구현(MVP)과 확장 방향을 구분한다.

## 현황 요약

### 이미 있는 것

- 시대 enum과 해금 판정
  - `Client/Building/BuildingTypes.h`
- 시대별 요구 조건 데이터
  - `Client/World/MainWorld.cpp`
  - `Binary/EraConfig.ini` 런타임 설정 패턴 존재
- 현재 시대/다음 시대 진행도 UI
  - `Client/UI/BuildMenuDataProvider.cpp`
- 시대에 따른 건물/칙령 잠금
  - `Client/UI/BuildMenuDataProvider.cpp`
  - `Client/UI/EdictDataProvider.cpp`

### 없는 것

- 플레이어가 전환을 승인하거나 거부하는 단계
- 시대 전환 시 발동하는 보상/패널티/세력 선택
- 시대 전환 알림, 팝업, 축하 연출
- 시대별 외교 세력 로스터
- 시대별 제도 잠금 해제 순서
  - 예: 식민지에서는 선거/헌법 비활성, 독립 이후 활성

### 지금 구조의 문제

- `Client/World/MainWorld.cpp`의 `RefreshEraProgress()`는 조건을 만족하면 즉시 `CurrentEra`를 올린다.
- 이 함수는 월드 갱신, 정치 갱신, 일일 시뮬레이션 루프에서 자주 호출된다.
- 결과적으로 시대 전환이 "이벤트"가 아니라 "숫자 조건 충족 시 내부 값 변경"으로 끝난다.

## Mod Kit에서 확인한 T6식 단서

텍스트 소스는 제한적이지만, 자산 이름만으로도 시대 전환이 다음 요소와 결합되어 있음을 확인할 수 있었다.

### 1. 시대 전환은 세력 전환과 묶여 있다

- `Tropico6/Content/Blueprints/Politics/Superpowers/BP_T6SuperpowerTheCrown.uasset`
- `Tropico6/Content/Blueprints/Politics/Superpowers/BP_T6SuperpowerAllies.uasset`
- `Tropico6/Content/Blueprints/Politics/Superpowers/BP_T6SuperpowerAxis.uasset`
- `Tropico6/Content/Blueprints/Trade/BP_T6TradePartnerTheCrown.uasset`
- `Tropico6/Content/Blueprints/Trade/BP_T6TradepartnerAllies.uasset`
- `Tropico6/Content/Blueprints/Trade/BP_T6TradepartnerAxis.uasset`
- `Tropico6/Content/Visuals/UI/Icons/SuperpowerIcons/Era_transition/*`

해석:
시대가 바뀌면 단순히 건물만 열리는 것이 아니라, 상대하는 외교 세력 묶음도 바뀐다.

### 2. 식민지 탈출은 독립 이벤트 체인이다

- `Tropico6/Content/Maps/Episodes/Episode_01/Quests/EP01_1_10_IndependenceP1_Q.uasset`
- `Tropico6/Content/Maps/Episodes/Episode_15/Scripts/EP15_1_7_Independence_Q.uasset`
- `Tropico6/Content/Audio/Sounds/G4F/Events/Voiceover/Tropico6_VO/Play_M5_FailedIndependance_.uasset`
- `Tropico6/Content/Audio/Sounds/G4F/Events/Voiceover/Tropico6_VO/Play_M5_MandateExp_.uasset`

해석:
식민지 -> 세계대전은 "조건 달성 즉시 전환"보다 독립 승인/실패/기한과 연관된 퀘스트성 흐름이다.

### 3. 헌법은 시대 진행과 함께 열리는 별도 시스템이다

- `Tropico6/Content/Blueprints/Constitution/BP_T6ConstitutionManager.uasset`
- `Tropico6/Content/UI/PopUps/Constitution/BP_T6UIConstitutionPopUp.uasset`
- `Tropico6/Content/Blueprints/Constitution/WorldWars/*`
- `Tropico6/Content/Blueprints/Constitution/ColdWar/*`
- `Tropico6/Content/Blueprints/Constitution/ModernTimes/*`

해석:
헌법은 시대별로 옵션 풀이 증가하는 별도 progression layer다.

### 4. 세계대전 이후에는 초강대국 요구/최후통첩 루프가 있다

- `Tropico6/Content/Blueprints/Demands/Superpowers/Allies/*`
- `Tropico6/Content/Blueprints/Demands/Superpowers/Axis/*`
- `Tropico6/Content/Blueprints/Demands/Superpowers/EasternBloc_Demands/*`
- `Tropico6/Content/Blueprints/Ultimatums/Superpowers/*`
- `Tropico6/Content/Blueprints/Quest/EraProgression/WorldWars/*`

해석:
시대 전환 이후 gameplay loop 자체가 외교 요구와 연결된다.

## 프로젝트 기준 설계 원칙

### 원칙 1. 자동 승급을 중단하고 "대기 상태 + 승인" 구조로 바꾼다

요구 조건을 만족해도 즉시 시대가 바뀌면 안 된다.
대신 `전환 가능` 상태를 만들고, 플레이어가 버튼이나 팝업에서 승인해야 한다.

### 원칙 2. T6의 전체 퀘스트 체인을 그대로 복제하지 않고, 3단계로 축소한다

1. 전환 조건 충족
2. 전환 제안 팝업
3. 확정 시 시대 변경 + 후속 시스템 적용

이 구조만 있어도 현재 프로젝트에서는 체감 품질이 크게 올라간다.

### 원칙 3. 기존 서브시스템을 최대한 재사용한다

- 알림/요약: `WorldCrisisStatus`, `PoliticalDemandNotice` 패턴 재사용
- 진행 표시: `BuildMenu` 연감/상단 HUD
- 런타임 조정: `EraConfig.ini` 패턴 유지

### 원칙 4. 시대 전환은 시스템 번들 전환이어야 한다

다음 항목이 함께 바뀌어야 한다.

- 현재 시대 값
- 건물/칙령/운영모드 해금 범위
- 외교 세력 집합
- 선거/헌법 활성 상태
- 전환 보상/패널티
- UI 알림/연감 기록

## 제안 구조

### 새 상태 모델

`Client/Building/BuildingTypes.h` 또는 별도 정치/월드 타입 헤더에 다음 구조를 추가한다.

```cpp
enum class EEraTransitionStage
{
    None = 0,
    Available,
    PendingChoice,
    InCeremony,
    Cooldown
};

enum class EEraTransitionChoice
{
    None = 0,
    Independence,
    CrownLoyalist,
    Revolutionary,
    Allies,
    Axis,
    WesternPowers,
    EasternBloc,
    Modernization
};

struct FEraTransitionState
{
    EEraTransitionStage Stage = EEraTransitionStage::None;
    EBuildingEra CurrentEra = EBuildingEra::Colonial;
    EBuildingEra TargetEra = EBuildingEra::WorldWars;
    bool CanStart = false;
    int AvailableSinceYear = 0;
    int AvailableSinceMonth = 0;
    int AvailableSinceDay = 0;
    int CooldownDays = 0;
    std::wstring Title;
    std::wstring Summary;
    std::vector<EEraTransitionChoice> Choices;
};
```

핵심은 `EraProgress`와 `EraTransition`을 분리하는 것이다.

- `EraProgress`: 숫자 충족 상태
- `EraTransition`: 플레이어 승인/선택/연출 상태

### MainWorld 역할 분리

`Client/World/MainWorld.cpp`

기존:

- `RefreshEraProgress()`가 진행도 계산과 실제 승급을 동시에 수행

변경:

1. `RefreshEraProgress()`
   - 현재 수치와 `CanAdvanceToNextEra`만 계산
   - 실제 승급은 하지 않음
2. `RefreshEraTransitionState()`
   - 조건 충족 시 전환 제안 상태 생성
3. `BeginEraTransition(Choice)`
   - 플레이어 선택 확정
4. `ApplyEraTransitionResults()`
   - 시대 변경, 보상, 외교/선거/헌법 상태 갱신

즉, 자동 루프는 "승급"이 아니라 "전환 가능 상태 생성"까지만 한다.

## 시대별 기획안

### 1. Colonial -> WorldWars

이 구간은 T6 감성을 가장 강하게 살릴 필요가 있다.

#### MVP

- 이름: `독립 선언`
- 조건: 기존 `WorldWars` 해금 조건 사용
- UI: "식민 통치를 끝내고 독립 국가로 전환할 준비가 되었습니다."
- 선택지: `독립 선언`
- 확정 효과:
  - 시대를 `WorldWars`로 변경
  - 선거 시스템 활성화
  - 헌법 시스템 1단계 활성화
  - 외교 세력을 `The Crown` 단일 식민 세력에서 `Allies / Axis` 2세력 구조로 교체
  - 보상금, 즉시 지지율 보너스, 1회성 축하 연감 문구 지급

#### 확장안

- 선택지 2개:
  - `독립 협상` : 돈/관계 보너스
  - `혁명` : 군사/혁명 지지 보너스, 초기 불안정도 증가
- 실패 리스크:
  - 제안 후 일정 기간 무시하면 세금/외교 패널티

### 2. WorldWars -> ColdWar

#### MVP

- 이름: `전후 질서 편입`
- 조건: 기존 `ColdWar` 해금 조건 사용
- 선택지:
  - `서방 노선`
  - `군사 노선`

초기 구현에서는 실제 외교 세력 이름만 다르게 두고,
세부 이념 효과는 작은 수치 차이로 시작한다.

예시:

- `서방 노선`
  - 수출 가격 보너스
  - 자유도 소폭 상승
- `군사 노선`
  - 치안/군사 효율 보너스
  - 자유도 소폭 하락

#### 확장안

- 선택지를 실제 T6처럼 `Allies / Axis` 전시 정산 + `WesternPowers / EasternBloc` 냉전 체제로 분리
- 외교 요구 시스템을 `PoliticalDemandService`와 연결

### 3. ColdWar -> Modern

#### MVP

- 이름: `현대화 선언`
- 조건: 기존 `Modern` 해금 조건 사용
- 선택지:
  - `시장 개방`
  - `국가 주도 현대화`

효과 예시:

- `시장 개방`
  - 무역/관광 보너스
  - 일부 공공만족도 관리 난이도 상승
- `국가 주도 현대화`
  - 전력/공공서비스 보너스
  - 수입 비용 또는 자유도 관리 부담 증가

#### 확장안

- 헌법 옵션 추가 해금
- 현대 시대 뉴스/연감/외교 요구 강화

## 서브시스템 연계 계획

### 1. 외교 세력 로스터를 시대 기반으로 전환

현재 `Client/World/MainWorld.cpp`의 외교 세력 이름은 고정 5개다.

```cpp
중국 / 러시아 / 미국 / 중동 / 유럽연합
```

이 구조는 T6식 시대 전환과 맞지 않는다.

권장 변경:

```cpp
struct FEraForeignPowerSet
{
    int Count;
    const wchar_t* Names[5];
};
```

예시:

- Colonial: `The Crown`
- WorldWars: `Allies`, `Axis`
- ColdWar: `Western Powers`, `Eastern Bloc`
- Modern: `Western Powers`, `Eastern Bloc`, `중립 시장`

기존 `TradeDiplomacyRuntime::GForeignPowerCount`가 5 고정이라면,
당장 상수는 유지하되 비활성 슬롯을 두는 방식이 가장 안전하다.

### 2. 선거를 시대 잠금 시스템으로 바꾼다

현재는 월드 시작 직후 `InitializeElectionSchedule()`가 실행된다.
이건 T6 흐름과 맞지 않는다.

권장 변경:

- Colonial: 선거 비활성
- WorldWars 진입 시: 선거 스케줄 초기화
- 이후 시대: 기존 선거 시스템 재사용

즉, `InitializeElectionSchedule()`은 부트스트랩이 아니라
`ApplyEraTransitionResults(Colonial -> WorldWars)`에서 호출하는 것이 더 자연스럽다.

### 3. 헌법은 2단계로 도입한다

현재 프로젝트에는 헌법 아이콘만 있고 실체가 없다.

권장:

- 1단계: 전환 팝업 내부의 간단한 시대 노선 선택으로 대체
- 2단계: 별도 `Constitution` 시스템 도입
  - 주제별 슬롯
  - 시대별 옵션 해금
  - 정치 성향/외교/만족도 수정치 적용

### 4. 알림/연감/상단 HUD 연결

이미 있는 패턴을 활용한다.

- Top HUD 이벤트 줄
- BuildMenu 연감
- Almanac 요약 카드

시대 전환 시 다음 정보가 남아야 한다.

- 전환 날짜
- 선택한 노선
- 받은 보너스/패널티
- 새로 열린 컨텐츠

### 5. 연출은 작은 것부터

MVP 연출:

- 시뮬레이션 일시 정지
- 중앙 팝업
- 상단 이벤트 문구
- 연감에 1회성 기록 추가

추후:

- 음성/배경 텍스처 교체
- 시대별 초상화/국기 사용
- 외교 아이콘 교체

## 권장 구현 순서

### Phase 1. 구조 분리

목표:
자동 승급을 제거하고 수동 전환으로 바꾼다.

작업:

- `RefreshEraProgress()`에서 실제 승급 제거
- `FEraTransitionState` 추가
- `CanAdvanceToNextEra()`와 `StartEraTransition()` 추가
- BuildMenu 또는 Top HUD에 `시대 전환` 버튼 노출

완료 기준:

- 조건을 만족해도 시대가 자동으로 바뀌지 않음
- 플레이어가 버튼을 눌러야 다음 시대로 넘어감

### Phase 2. 전환 효과 번들화

목표:
시대 변경 시 관련 시스템이 함께 전환되도록 만든다.

작업:

- 외교 세력 세트 전환
- 선거 시스템 시대 잠금
- 1회성 보상/패널티 적용
- 연감/알림 기록

완료 기준:

- 시대가 바뀌면 단순 이름 변경이 아니라 플레이 감각이 달라짐

### Phase 3. 선택지와 경로 차등화

목표:
각 시대 전환에 최소 2개 선택지를 둔다.

작업:

- 독립 방식 분기
- 냉전/현대화 노선 선택
- 선택지별 외교/정치/경제 보정

완료 기준:

- 같은 시대 진입이라도 선택에 따라 게임 양상이 달라짐

### Phase 4. 헌법/요구 시스템 확장

목표:
T6 감성의 장기 progression layer를 붙인다.

작업:

- Constitution 시스템
- 초강대국 요구/최후통첩
- 시대별 외교 과제

## 파일별 적용 포인트

### 핵심 로직

- `Client/World/MainWorld.cpp`
  - 진행도 계산과 전환 실행 분리
- `Client/World/MainWorld.h`
  - 전환 상태 조회/실행 API 추가
- `Client/Building/BuildingTypes.h`
  - 전환 상태 타입 추가

### UI

- `Client/UI/BuildMenuDataProvider.cpp`
  - 현재 연감 상단에 있는 시대 진행 텍스트에 전환 가능 상태/버튼 연결
- `Client/UI/TopHudDataProvider.cpp`
  - 이벤트 문구 노출
- `Client/UI/TopHudRenderer.cpp`
  - 시대 전환 버튼 또는 팝업 진입 버튼 배치

### 정치/외교

- `Client/Economy/TradeDiplomacyRuntime.h`
  - 시대별 외교 세력 세트 계산
- `Client/World/MainWorldElectionService.cpp`
  - 선거 시스템 활성 시점 이동
- `Client/World/MainWorldPoliticalDemandService.cpp`
  - 후속 확장에서 시대별 요구 생성에 재사용

### 데이터

- `Binary/EraConfig.ini`
  - 전환 조건 유지
- 신규 제안: `Binary/EraTransitionConfig.ini`
  - 선택지별 보상, 패널티, 쿨다운, 설명 문구

## MVP 권장안

현재 프로젝트 상태를 기준으로 가장 비용 대비 효과가 큰 안은 다음이다.

1. 자동 승급 제거
2. 연감 또는 상단 HUD에 `시대 전환` 버튼 추가
3. 버튼 클릭 시 간단한 중앙 팝업 표시
4. 팝업에서 선택지 1~2개 제시
5. 확정 시
   - `CurrentEra` 변경
   - 선거/외교 세트 갱신
   - 알림/연감 기록
   - 새 시대 건물/칙령 해금

이 안은 T6의 전체 복잡도를 복제하지 않으면서도,
플레이어 체감상 "내가 다음 시대로 넘어갔다"는 경험을 만들어 준다.

## 결론

현재 프로젝트에 필요한 것은 더 많은 해금 조건이 아니라,
`시대 전환을 하나의 플레이 이벤트로 만드는 오케스트레이션 레이어`다.

따라서 첫 구현은 다음 원칙으로 가는 것이 가장 적절하다.

- 숫자 조건 충족은 전환 가능 상태만 만든다.
- 실제 전환은 플레이어 승인으로 진행한다.
- 전환 시 외교/선거/알림을 함께 바꾼다.
- 헌법과 초강대국 요구는 후속 단계로 확장한다.

이 방향이 현재 코드 구조와 가장 잘 맞고,
Tropico 6 Mod Kit에서 확인된 시대 진행 감성도 가장 자연스럽게 흡수할 수 있다.
