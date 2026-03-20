# 02. 월드 시스템 & 서브시스템 분리

## 2-1. MainWorld의 역할

`CMainWorld`는 이 게임의 핵심 월드입니다.
인게임의 모든 시스템(경제, 정치, 시민, 건물, UI)을 초기화하고 매 프레임 업데이트합니다.

부트스트랩 파일(`MainWorldBootstrap.cpp`)을 보면 월드 초기화 순서를 알 수 있습니다.

```cpp
// MainWorldBootstrap.cpp 에서 월드 초기화 시 처리되는 것들
- 렌더 레이어 생성 (타일맵, 건물, UI)
- 타일맵 설정 (기본 + 오버레이)
- UI 위젯 생성 (TopHud, BuildMenu, Almanac, Edict, ...)
- 서브시스템 등록 (Economy, Politics, Simulation, ...)
- INI 핫리로드 등록
```

---

## 2-2. 서브시스템 패턴

`MainWorld`는 거대한 단일 클래스가 아니라, **서브시스템으로 분리**됩니다.

```
CMainWorld
  ├── SimulationSubsystem     — 날짜/시간 진행, 이민자 생성
  ├── PopulationSubsystem     — 시민 인구 관리
  ├── BuildingSubsystem       — 건물 인스턴스 관리
  ├── EconomySubsystem        — 매일 경제 정산
  ├── PoliticsSubsystem       — 파벌, 선거, 요구 사항
  ├── EdictSubsystem          — 칙령 활성화/만료
  ├── InfrastructureSubsystem — 도로, 버스 노선
  ├── TradeDiplomacySubsystem — 외교, 무역 노선
  ├── KnowledgeSubsystem      — 시대 전환 조건 추적
  └── CrisisSubsystem         — 위기 이벤트 (파업, 습격 등)
```

**왜 서브시스템인가?**
- 단일 책임 원칙(SRP): 각 서브시스템이 하나의 관심사만 담당합니다.
- 코드 규모가 커질수록 거대한 단일 클래스는 유지보수가 어렵습니다.
- 서브시스템별 독립 테스트가 가능합니다.

---

## 2-3. 월드 접근 계층

외부 코드가 `MainWorld` 내부 데이터에 직접 접근하지 못하도록
**접근 인터페이스를 역할별로 분리**합니다.

```
MainWorldAccess              — 일반 읽기 (날짜, 예산, 시민 수)
MainWorldSystemAccess        — 서브시스템 접근
MainWorldInfrastructureAccess — 도로/버스 인프라
MainWorldBuildingControlAccess — 건물 건설/철거 명령
MainWorldTradeAccess         — 무역 노선 접근
MainWorldUiReadAccess        — UI 전용 읽기 접근
```

```cpp
// 예시: UI에서 예산 읽기 (읽기 전용 인터페이스만 사용)
auto WorldAccess = MainWorldUiReadAccess::Get(World);
long long Budget = WorldAccess.GetBudget();
```

**이 패턴의 장점**
- UI 코드가 시뮬레이션을 직접 수정하는 것을 방지합니다.
- 어떤 코드가 어떤 데이터를 접근하는지 명확합니다.
- "읽기 전용 접근"을 컴파일 타임에 강제합니다.

---

## 2-4. 서비스 레이어 패턴

서브시스템 외에 **서비스(Service)** 클래스가 따로 존재합니다.

```
GovernmentCommandService      — 플레이어 명령 처리 (칙령 발동, 세금 변경)
MainWorldElectionService      — 선거 진행 로직
MainWorldPoliticalDemandService — 파벌 요구 처리
MainWorldWorldCrisisService   — 위기 이벤트 처리
```

**서브시스템 vs 서비스**
| | 서브시스템 | 서비스 |
|--|-----------|--------|
| 역할 | 상태 보유 + 매 틱 업데이트 | 특정 연산/명령 처리 |
| 상태 | O (데이터 소유) | 주로 X (무상태) |
| 호출 | Update() 루프에서 자동 | 이벤트 발생 시 직접 호출 |

---

## 2-5. 시나리오 시스템

게임 목표와 승패 조건은 `ScenarioRunner` / `ScenarioSubsystem` 이 담당합니다.

```cpp
// ScenarioConfig.h — 시나리오 구성
struct FScenarioConfig
{
    std::wstring Title;
    int          StartYear;
    int          StartMonth;
    long long    StartBudget;
    int          TargetElectionWinCount;
    // ...
};
```

- 시나리오 설정으로 시작 조건과 승리 조건을 데이터로 정의합니다.
- 하드코딩 대신 구조체로 관리하면 새 시나리오 추가가 쉽습니다.

---

## 2-6. 세계 통계 스냅샷 (WorldStatsSnapshot)

매 틱마다 계산하면 비용이 크므로, **스냅샷 패턴**을 사용합니다.

```cpp
// WorldStatsSnapshot.h
struct FWorldStatsSnapshot
{
    int   TotalPopulation;
    int   TotalWorkers;
    int   UnemployedCount;
    double AverageSatisfaction;
    long long Budget;
    // ... (알마낙 전체 통계)
};
```

```
매 N 틱마다:
  WorldStatsSnapshot 재계산 → 캐싱

UI/알마낙이 데이터 요청 시:
  시뮬레이션 직접 순회 X
  → 캐싱된 스냅샷에서 읽기
```

**왜 스냅샷인가?**
- 매 프레임 전체 시민을 순회하면 성능이 저하됩니다.
- UI는 약간 오래된 데이터여도 괜찮습니다.
- 렌더 스레드와 시뮬레이션 스레드 분리 시에도 유용합니다.

---

## 학습 포인트 요약

1. **서브시스템 패턴**: 큰 월드 클래스를 역할별로 분리하면 유지보수성이 높아집니다.
2. **접근 계층 분리**: 읽기/쓰기 인터페이스를 분리하면 코드의 의도가 명확해집니다.
3. **스냅샷 캐싱**: 비싼 연산 결과를 캐싱해 UI 조회 비용을 낮춥니다.
4. **서비스 패턴**: 상태 없는 연산은 서비스 클래스로 분리해 테스트를 쉽게 합니다.
