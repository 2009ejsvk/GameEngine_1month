# 07. 건물 카탈로그 & 데이터 설계

## 7-1. 건물 카탈로그 개요

모든 건물 유형의 메타데이터는 **카탈로그(Catalog)** 에 등록됩니다.
카탈로그는 게임 시작 시 로드되고, 이후 읽기 전용으로 사용됩니다.

```
BuildingCatalogTypes.h     — 타입 정의 (enum, 기본 구조체)
BuildingCatalogData.h/.cpp — 기본 데이터 구조체
BuildingCatalogEntry.h     — 개별 건물 항목
BuildingCatalogDerived.h   — 파생 데이터 (계산된 값들)
BuildingCatalogAspectData.h — 측면별 데이터 (경제/복지/관광 등)
BuildingCatalogQuery.h     — 카탈로그 쿼리 인터페이스
BuildingCatalogLoader.h    — 파일에서 로드
BuildingCatalogRegistry.cpp — 전역 레지스트리
```

---

## 7-2. 건물 카테고리

```cpp
enum class EBuildingCategory
{
    Residential,   // 주거 시설
    Food,          // 식량 생산/서비스
    Industry,      // 산업/생산
    Entertainment, // 유흥/관광
    Civic,         // 공공 시설 (병원, 학교 등)
    Religion,      // 종교 시설
    Military,      // 군사 시설
    Infrastructure // 도로, 항구, 발전소 등
    // Count = 8
};
```

---

## 7-3. 건물 항목 구조 (FBuildingCatalogEntry)

```cpp
struct FBuildingCatalogEntry
{
    std::string       ObjectName;        // 고유 식별자
    std::wstring      DisplayName;       // 화면에 표시되는 이름
    EBuildingCategory Category;          // 카테고리
    EBuildingEra      UnlockEra;         // 시대 잠금 (Colonial/WorldWars/ColdWar/Modern)
    int               BuildCost;         // 건설 비용
    int               DailyUpkeep;       // 일일 유지비
    int               WorkerCapacity;    // 최대 직원 수
    int               ResidentCapacity;  // 최대 거주자 수 (주거 건물)
    bool              IsResidential;     // 주거 건물 여부
    // ... 각 복지 서비스 반경, 효율 등
};
```

---

## 7-4. 파생 데이터 패턴 (BuildingCatalogDerived)

기본 데이터에서 계산되는 값들을 **별도 구조체**로 분리합니다.

```cpp
// 원본 데이터 (입력)
int BuildCost = 5000;
int DailyUpkeep = 50;
int WorkerCapacity = 10;

// 파생 데이터 (계산 결과)
float DailyWageCostAtFullCapacity = WorkerCapacity * AverageWage;
float BreakevenDays = BuildCost / DailyProfit;
```

**왜 분리하는가?**
- 원본 데이터가 바뀌면 파생 데이터도 재계산합니다.
- UI 표시 시 매번 계산하지 않고 파생 데이터를 읽기만 합니다.
- 계산 로직이 한 곳에 집중됩니다.

---

## 7-5. 측면별 데이터 (Aspect Data)

건물은 여러 "측면(Aspect)"으로 기능을 가집니다.

```
경제 측면: 수출 가능 자원, 생산량
복지 측면: 제공하는 서비스 (건강/교육 등), 서비스 반경
관광 측면: 관광 점수, 유흥 점수
군사 측면: 치안 반경, 방위 값
```

각 측면이 없는 건물은 해당 AspectData가 null/default입니다.

```cpp
// 예: 병원은 복지 측면만 있음
BuildingEntry.WelfareAspect.HealthServiceRadius = 500;
BuildingEntry.EconomyAspect = {};  // 경제 측면 없음
```

---

## 7-6. 건물 배치 시스템

건물 배치는 타일맵 위에서 이루어집니다.

```
PlacementController       — 배치 흐름 제어 (시작/확인/취소)
PlacementAreaObject       — 배치 가능 영역 시각화
PlacementBuildingVisual   — 배치 중인 건물 시각 피드백
PlacementBuildingRoleResolver — 건물 역할 지정
```

**배치 흐름**
```
1. 플레이어가 건설 메뉴에서 건물 선택
2. PlacementController → 배치 모드 시작
3. 마우스 이동 → PlacementAreaObject 로 유효 위치 시각화
4. 클릭 → 위치 확정 → BuildingSubsystem 에 건설 명령
5. 예산에서 BuildCost 차감
```

---

## 7-7. 건물 인스턴스 vs 카탈로그

| | 카탈로그 (CatalogEntry) | 인스턴스 (RuntimeObject) |
|--|----------------------|----------------------|
| 개수 | 유형별 1개 | 월드에 N개 |
| 내용 | 설계 스펙 (불변) | 현재 상태 (가변) |
| 예 | "농장" 의 최대 직원 = 10 | 농장 #3 의 현재 직원 = 7 |

```cpp
// 인스턴스는 카탈로그를 참조합니다
class CBuildingInstance
{
    std::string CatalogEntryId;   // 카탈로그 참조
    int CurrentWorkers;           // 런타임 상태
    int BudgetLevel;              // 런타임 상태
    bool IsOperating;             // 런타임 상태
};
```

---

## 7-8. 건물 정보 패널 탭 구성

건물 클릭 시 5탭으로 정보를 표시합니다.

| 탭 | 내용 |
|----|------|
| 기본 | 개요 (직원, 예산, 운영 모드, 입주자/방문자) |
| 통계 | 수익성, 효율성 수치 |
| 업글 | 업그레이드 카드 (이름, 설명, 아이콘) |
| 효율 | 효율성 지표 상세 |
| 정보 | 텍스트 설명 (역사적 배경 등) |

---

## 7-9. 건설 메뉴 (BuildMenu)

```
BuildMenuWidget          — UI 위젯 (탭, 버튼)
BuildMenuDataProvider    — 카탈로그에서 메뉴 데이터 조합
BuildMenuRenderer        — 화면 렌더링
BuildMenuQueryService    — 건물 목록 쿼리
BuildMenuWorldQuerySource — 월드에서 건설 가능 여부 확인
```

건설 메뉴는 카테고리별 탭으로 구성되고,
각 탭 안에 해당 카테고리 건물들이 아이콘 그리드로 표시됩니다.

---

## 학습 포인트 요약

1. **카탈로그 패턴**: 불변 메타데이터를 카탈로그로 분리하면 인스턴스 코드가 단순해집니다.
2. **파생 데이터 캐싱**: 자주 사용되는 계산 결과를 미리 계산해두면 런타임 성능이 향상됩니다.
3. **Aspect 패턴**: 건물이 가진 기능을 측면별로 분리하면 기능 추가가 기존 구조를 건드리지 않습니다.
4. **카탈로그 vs 인스턴스 분리**: 타입 정보(불변)와 런타임 상태(가변)를 분리하면 메모리 효율과 코드 명확성이 높아집니다.
5. **계층적 쿼리**: QueryService → WorldQuerySource의 계층으로 월드 데이터 접근을 추상화합니다.
