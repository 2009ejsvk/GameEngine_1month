# Codex 작업 지시서 — 시대(Era) 시스템 데이터 정비

## 목표
현재 프로젝트의 시대 전환 조건(Threshold)과 건물별 시대 잠금 해제(UnlockEra) 데이터를
트로피코 6 원본 데이터와 맞게 보정한다.

---

## 배경

### T6 시대 구분 (4단계)
| 코드 이름    | 표시 이름       | T6 원본 건물 수 |
|------------|---------------|--------------|
| Colonial   | 식민지 시대      | 53           |
| WorldWars  | 세계대전 시대     | 45           |
| ColdWar    | 냉전 시대        | 58           |
| Modern     | 현대 시대        | 51           |
(DLC 제외 기준. 출처: `Tropico6SourceBuildingInventory.tsv` SourceEra 집계)

### 현재 프로젝트 문제
1. **건물 UnlockEra 누락**: 전체 159개 건물 중 `Tropico6SourceMetadataOverrides.tsv`에
   등록된 건물은 24개뿐. 나머지 135개는 전부 `EBuildingEra::Colonial` 기본값.
2. **전환 조건**: `MainWorld.cpp` 내 `ResolveEraUnlockRequirement` 함수의 수치가
   T6 건물 분포를 반영하지 않아 실제 게임 밸런스와 차이가 있음.

---

## 작업 1 — 건물 UnlockEra 일괄 보정

### T6 참고 파일
```
E:\GameEngine_1month\Client\Building\Data\Tropico6SourceBuildingInventory.tsv
```
- 컬럼: `DisplayName`, `AssetName`, `SourceEra`, `ProjectEraHint`
- `ProjectEraHint` 값: `Colonial` / `WorldWars` / `ColdWar` / `Modern`

### 수정 대상 파일
```
E:\GameEngine_1month\Client\Building\Data\Tropico6SourceMetadataOverrides.tsv
```
- 컬럼: `BuildingId`, `UnlockEra`, `BuildMenuCategory`, `SourceAssetName`, `SourceDisplayName`
- `UnlockEra` 허용값: `Colonial` / `WorldWars` / `ColdWar` / `Modern`

### 크로스레퍼런스 방법
1. `Tropico6SourceMetadataOverrides.tsv`의 `SourceAssetName` 컬럼이
   `Tropico6SourceBuildingInventory.tsv`의 `AssetName` 컬럼과 일치하는 항목을 찾는다.
2. 일치하면 인벤토리의 `ProjectEraHint`를 오버라이드 파일의 `UnlockEra`로 설정한다.
3. 아직 `Tropico6SourceMetadataOverrides.tsv`에 없는 건물을 인벤토리에서 찾아 추가한다.
   - 단, `BuildingCatalog.tsv`에 실제로 존재하는 건물만 추가한다.
   - `BuildingId`는 `BuildingCatalog.tsv`의 `build_{카테고리}_{로컬인덱스}` 형식.

### 매핑 기준 (AssetName 기준 직접 매핑이 불가능한 경우)
아래 목록을 수동 보완용으로 사용한다.

#### 세계대전 (WorldWars) 건물 (현재 Colonial로 잘못 분류된 항목 다수)
T6 WorldWars 팩 대표 건물 (총 45종):
- Cabaret, Brewery, Tobacco Farm, Tobacco Factory, Weapons Factory, Army Garrison
- Car Factory, Commando Garrison, Power Plant (Coal), Steel Mill
- Customs Office, Immigration Office, Embassy, Bank
- Cinema, Newspaper, Radio Station
- Sanatorium, School (고급), Fire Station
- Dock Crane, Road extension (교각)

#### 냉전 (ColdWar) 건물 (총 58종):
- Casino, Cocktail Bar, Golf Course, Gourmet Restaurant, Night Club, Snorkel Bay
- Arcade, Fast Food Joint, Dungeon Entrance
- Nuclear Power Plant, Missile Defense
- Secret Police HQ, Watchtower
- Childhood Museum, Mausoleum, TV Station
- Research Lab, College, Skyscraper
- Chemical Plant, Oil Refinery, Textile Mill
- Supermarket, Department Store

#### 현대 (Modern) 건물 (총 51종):
- Beach Resort, Hang Gliding, Museum of Modern Art, Yacht Club
- Airport, Spaceport
- Electronics Factory, Robotics Laboratory
- Solar Farm, Wind Farm, Offshore Wind Turbine
- High School (현대형), Modern Watchtower
- Office Building, Modern Residential Block

---

## 작업 2 — 시대 전환 조건 값 보정

### 수정 대상 파일
```
E:\GameEngine_1month\Client\World\MainWorld.cpp
```
- 함수: `ResolveEraUnlockRequirement` (약 line 1411)
- 파일이 너무 크므로 오프셋 1411 근방 100줄만 읽어 해당 함수를 찾는다.

### 현재 값 (변경 전)
```cpp
case EBuildingEra::WorldWars:
    Requirement.MinPopulation = 24;
    Requirement.MinTotalBuildings = 8;
    Requirement.MinFoodProviders = 3;
    break;
case EBuildingEra::ColdWar:
    Requirement.MinPopulation = 60;
    Requirement.MinTotalBuildings = 18;
    Requirement.MinIndustryBuildings = 4;
    Requirement.MinPowerMW = 20;
    break;
case EBuildingEra::Modern:
    Requirement.MinPopulation = 120;
    Requirement.MinTotalBuildings = 32;
    Requirement.MinPublicServiceBuildings = 5;
    Requirement.MinEntertainmentBuildings = 4;
    Requirement.MinPowerMW = 45;
    break;
```

### 목표 값 (변경 후)
T6 건물 분포(Colonial 53개, WorldWars 45개, ColdWar 58개, Modern 51개)를 기준으로
각 시대 전환 시점에 해당 시대 건물의 약 15~20%를 보유해야 하도록 설정.

```cpp
case EBuildingEra::WorldWars:
    // Colonial 건물 ~53종 중 약 15% 건설 = 8개
    // 인구: T6 WorldWars 진입 시점 기준 약 40~60명 규모 마을
    Requirement.MinPopulation = 40;
    Requirement.MinTotalBuildings = 10;
    Requirement.MinFoodProviders = 3;
    break;
case EBuildingEra::ColdWar:
    // Colonial+WorldWars 합산 ~98종 중 약 18% 건설 = 18개
    // 전력 조건: WorldWars 말기에 전력망이 구축됨 (발전소 1~2기 = 20~30MW)
    // 산업: WorldWars 대표 산업 건물 확보
    Requirement.MinPopulation = 80;
    Requirement.MinTotalBuildings = 22;
    Requirement.MinIndustryBuildings = 5;
    Requirement.MinPowerMW = 25;
    break;
case EBuildingEra::Modern:
    // 전 시대 합산 ~156종 중 약 20% 건설 = 32개
    // 전력: ColdWar 말기 원자력/화력 확장 = 50~60MW
    // 공공서비스+오락: 도시 완성도 조건
    Requirement.MinPopulation = 150;
    Requirement.MinTotalBuildings = 38;
    Requirement.MinPublicServiceBuildings = 6;
    Requirement.MinEntertainmentBuildings = 5;
    Requirement.MinPowerMW = 60;
    break;
```

### 수정 근거
| 전환 | 변경 내용 | 이유 |
|------|----------|------|
| Colonial→WorldWars | 인구 24→40 | T6 WorldWars 초반 마을 규모에 맞춤 |
| Colonial→WorldWars | 총건물 8→10 | 소규모 증가 (농장+인프라 기반) |
| WorldWars→ColdWar | 인구 60→80 | 산업화 시작 단계 인구 반영 |
| WorldWars→ColdWar | 총건물 18→22 | WorldWars 팩 비중 반영 |
| WorldWars→ColdWar | 산업건물 4→5 | WorldWars 산업건물(철강·무기·발전) 반영 |
| WorldWars→ColdWar | 전력 20→25 | 발전소 1기(20MW)+서브스테이션 기준 |
| ColdWar→Modern | 인구 120→150 | Modern 진입 시 도시 규모 |
| ColdWar→Modern | 총건물 32→38 | 3개 시대 건물 20% 기준 |
| ColdWar→Modern | 공공서비스 5→6 | 병원·학교·경찰서·소방서 확보 |
| ColdWar→Modern | 오락건물 4→5 | 바·카지노·클럽 등 냉전 오락 확보 |
| ColdWar→Modern | 전력 45→60 | 원자력(60MW) 또는 화력(40MW)×2 기준 |

---

## 작업 3 — 시대 전환 조건을 INI 설정으로 분리 (선택 사항)

현재 `ResolveEraUnlockRequirement`의 값이 코드에 하드코딩되어 있어
런타임 조정이 불가능하다. 칙령 시스템(`Binary/Edicts.ini`)처럼
`Binary/EraConfig.ini` 파일로 분리하면 게임 중 밸런싱이 가능해진다.

이 작업은 선택 사항이므로, 작업 1·2가 완료된 후 시간 여유가 있을 때 진행한다.
구현 방식은 `EdictSystem.cpp`의 INI 파싱 패턴을 참고한다.

---

## 참고: T6 ModKit 직접 접근 가능 파일

| 목적 | 파일 경로 |
|------|---------|
| 건물별 시대 힌트 | `E:\GameEngine_1month\Client\Building\Data\Tropico6SourceBuildingInventory.tsv` |
| 기존 시대 오버라이드 | `E:\GameEngine_1month\Client\Building\Data\Tropico6SourceMetadataOverrides.tsv` |
| 건물 카탈로그 | `E:\GameEngine_1month\Client\Building\Data\BuildingCatalog.tsv` |
| 시대 전환 로직 | `E:\GameEngine_1month\Client\World\MainWorld.cpp` (line ~1411) |
| 시대 관련 타입 | `E:\GameEngine_1month\Client\Building\BuildingTypes.h` (line ~87) |

> **주의**: T6 ModKit의 실제 시대 전환 수치(인구·건물 수)는 바이너리 `.uasset`에만 존재하여
> 직접 읽기가 불가능하다. 위 목표값은 T6의 에라별 건물 분포(53/45/58/51개)를 근거로
> 현재 프로젝트 규모에 맞게 역산한 값이다.

---

## 작업 우선순위

1. **작업 2** (MainWorld.cpp 수치 보정) — 코드 1줄 교체, 위험도 낮음, 즉시 효과
2. **작업 1** (Tropico6SourceMetadataOverrides.tsv 확장) — 데이터 작업, 위험도 낮음
3. **작업 3** (INI 분리) — 선택 사항, 구현 공수 있음
