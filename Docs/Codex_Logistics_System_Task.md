# Codex 작업 지시서 — 물류(Logistics) 시스템 데이터 정비

## 목표
T6 기준 운송업자 사무소 / 화물창고 / 창고의 운영 모드 및 업그레이드 데이터를
`BuildingOperationModes.tsv`, `BuildingUpgrades.tsv`에 채운다.

---

## 배경 — T6 물류 시스템 구조

### 핵심 3종 건물
| 건물ID     | 한국어명        | 역할 |
|-----------|--------------|------|
| build_1_5 | 운송업자 사무소 | 자원 픽업 → 소비자/항구 배달. 커버리지 반경 기반. |
| build_1_7 | 화물창고       | 섬 간 자원 수송. 선박이 자원을 다른 섬 선착장에 전달. |
| build_1_8 | 창고           | 중간 저장소. 최대 3종 자원 대량 보관, 물류 우선순위 설정. |

### 현재 프로젝트 코드 상태 (이미 구현된 것)
- `FBuildingOperationModeEffect.TeamsterTransferMultiplier` — 운송업자 배달량 배율
- `FBuildingOperationModeEffect.TeamsterCargoLossPercent` — 화물 손실률
- `FBuildingOperationModeEffect.HarborProgressMultiplier` — 화물선 속도 배율
- `BuildingCatalog.cpp:4396~4423` — EffectSummary 텍스트 파싱:
  - `"적하량 +N%"` → `TeamsterTransferMultiplier`
  - `"화물 손실 N%"` → `TeamsterCargoLossPercent`
  - `"화물선 속도 +N%"` → `HarborProgressMultiplier`
- 창고 업그레이드 2개 이미 있음 (`BuildingUpgrades.tsv` build_1_8)

### 현재 누락 데이터
1. `BuildingOperationModes.tsv` — Teamster 운영 모드 없음 (build_1_5)
2. `BuildingOperationModes.tsv` — 화물창고 운영 모드 없음 (build_1_7)
3. `BuildingUpgrades.tsv` — 화물창고 갑판원 업그레이드 없음 (build_1_7)

---

## 작업 1 — `BuildingOperationModes.tsv` 에 Teamster 모드 추가

### 파일 위치
```
E:\GameEngine_1month\Client\Building\Data\BuildingOperationModes.tsv
```

### 헤더 (참고용)
```
# BuildingId	ModeIndex	DisplayName	EffectSummary	ProductionMultiplier
InputConsumptionMultiplier	ServiceThroughputMultiplier	PollutionMultiplier
WageMultiplier	UpkeepMultiplier	ExportPriceDeltaPercent	ImportPriceDeltaPercent
CapacityDelta	ServiceCapacityDelta	HousingQualityDelta	JobQualityDelta
ServiceQualityDelta	RequiredResearch	UnlockEra	SourceDisplayName
```
(실제 파일은 탭 구분, 한 행)

### 추가할 행 (build_1_5 — 운송업자 사무소)

T6 원본 운영 모드 2개:
```
build_1_5	0	안전 하중	기본 설정												운송업자 사무소
build_1_5	1	느슨한 하중 제한	적하량 +50%, 최대 10% 화물 손실												운송업자 사무소
```
- Mode 0: 기본값 (효과 없음)
- Mode 1: `EffectSummary = "적하량 +50%, 최대 10% 화물 손실"`
  - → 파서가 자동으로 `TeamsterTransferMultiplier = 1.5f`, `TeamsterCargoLossPercent = 10` 적용

### 추가할 행 (build_1_7 — 화물창고)

T6 원본 운영 모드 2개:
```
build_1_7	0	안전 하중	기본 설정												화물창고
build_1_7	1	느슨한 속도 제한	화물선 속도 +50%, 최대 5% 화물 손실												화물창고
```
- Mode 0: 기본값 (효과 없음)
- Mode 1: `EffectSummary = "화물선 속도 +50%, 최대 5% 화물 손실"`
  - → 파서가 자동으로 `HarborProgressMultiplier = 1.5f`, `TeamsterCargoLossPercent = 5` 적용

### 추가 위치
`BuildingOperationModes.tsv` 파일 첫 번째 섹션 `# Infrastructure` 다음에 삽입.
다른 건물 행들과 동일한 형식 유지.

---

## 작업 2 — `BuildingUpgrades.tsv` 에 화물창고 갑판원 업그레이드 추가

### 파일 위치
```
E:\GameEngine_1month\Client\Building\Data\BuildingUpgrades.tsv
```

### 헤더 컬럼 순서 (참고용, 탭 구분 20개 필드)
```
BuildingId | UpgradeIndex | DisplayName | EffectSummary | UnlockEra | Cost |
ProductionMultiplier | InputConsumptionMultiplier | UpkeepMultiplier |
WarehouseSlotCapacityMultiplier | CapacityDelta | ServiceCapacityDelta |
PerWorkerServiceCapacityDelta | HousingQualityDelta | JobQualityDelta |
ServiceQualityDelta | RequiredPowerDeltaMW | PollutionMultiplier |
UpkeepFlatDelta | WarehouseSlotCapacityDelta | SourceDisplayName
```

### 추가할 행 (build_1_7 — 화물창고)

T6 원본: 갑판원(Dock Crew) 업그레이드 — Colonial 시대, 비용 1200, 일자리 +6
```
build_1_7	0	갑판원	일자리 +6		1200					6										화물창고
```
- UpgradeIndex: 0
- UnlockEra: 비어있음 (Colonial)
- Cost: 1200
- CapacityDelta (11번째 필드): 6 (인력 슬롯 +6)
- 나머지 필드: 비어있음

### 추가 위치
`BuildingUpgrades.tsv`의 `# Infrastructure` 섹션에서
`build_1_5` 행 다음, `build_1_8` 행 이전에 삽입.

---

## 작업 3 — 운송업자 기본 파라미터 확인 (GameConstants)

### 현재 기본값 (`GameConstants.cpp`)
```cpp
Orb::TeamsterSpeedMultiplier = 3.f;        // 운송 이동속도 = 시민 속도 × 3
Orb::TeamsterCoverageRadiusTiles = 25.f;   // 커버리지 반경 (타일 단위)
Orb::TeamsterTransferUnit = 1000;          // 1회 배달량 (단위)
Orb::TeamsterConsumerRestockThreshold = 250; // 보충 트리거 재고 수준
Orb::TeamsterConsumerTargetStock = 1000;   // 목표 재고 수준
```

### T6 참고값 (BuildingCatalog.tsv DetailText 기반)
T6 운송업자 사무소:
- 커버리지: T6는 타일 20~30 범위 → 현재 25 타일 적절
- 배달량: T6 표준 약 1000단위 → 현재 1000 일치
- 속도: T6 트럭 이동 속도 ≈ 보행 3배 → 현재 3.0x 일치

**보정 불필요** — 현재 기본값이 T6 수준과 일치함.

---

## T6 vs 현재 프로젝트 비교 요약

| 기능                           | T6 ModKit | 현재 프로젝트 | 상태 |
|------------------------------|-----------|------------|------|
| 운송업자 커버리지 반경            | ~25타일    | 25타일       | ✅ 일치 |
| 운송업자 배달량                  | ~1000단위  | 1000단위     | ✅ 일치 |
| 운송업자 속도 배율               | ~3x       | 3.0x        | ✅ 일치 |
| 안전 하중 운영 모드              | 있음       | ❌ 누락       | 작업 1 |
| 느슨한 하중 모드 (+50% 적재, 10% 손실) | 있음  | ❌ 누락       | 작업 1 |
| 화물창고 속도 모드 (느슨한 속도 제한) | 있음   | ❌ 누락       | 작업 1 |
| 화물창고 갑판원 업그레이드         | 있음      | ❌ 누락       | 작업 2 |
| 창고 고층 진열대 업그레이드        | 있음      | ✅ 구현       | —    |
| 창고 무작위 보관 업그레이드        | 있음      | ✅ 구현       | —    |
| TeamsterCargoLossPercent 파서 | —         | ✅ 구현       | —    |
| HarborProgressMultiplier     | —         | ✅ 구현       | —    |
| 창고 3슬롯 보관                 | 있음       | ✅ 구현       | —    |
| 창고 전용/균형 정책              | 있음       | ✅ 구현       | —    |
| 섬 간 다중 홉 라우팅             | 있음       | 미구현(싱글맵) | 향후 |
| 건물별 instock/outstock 분리    | 있음      | 단일 재고 버퍼 | 향후 |

---

## 최종 변경 파일 요약

| 파일 | 변경 내용 |
|------|---------|
| `Client/Building/Data/BuildingOperationModes.tsv` | build_1_5 모드 2개 추가, build_1_7 모드 2개 추가 (총 4행) |
| `Client/Building/Data/BuildingUpgrades.tsv` | build_1_7 갑판원 업그레이드 1행 추가 |

---

## T6 참고 문자열 (BuildingCatalog.tsv DetailText 발췌)

```
build_1_5 운송업자 사무소:
  "운영 모드:\n- 안전 하중\n- 느슨한 하중 제한 (적하량 +50%, 최대 10% 화물 손실)\n
   업그레이드:\n- 교대 근무 (세계대전 시대, 2500, 일자리 +6)"

build_1_7 화물창고:
  "운영 모드:\n- 안전 하중\n- 느슨한 속도 제한 (화물선 속도 +50%, 최대 5% 화물 손실)\n
   업그레이드:\n- 갑판원 (1200, 일자리 +6)"

build_1_8 창고:
  "효과: 최대 3종류 상품 대량 보관 및 물류 지침 설정\n
   업그레이드:\n- 고층 진열대 (냉전, 5000, 슬롯당 보관량 +5000)\n
             - 무작위 창고 보관 (현대, 15000, 슬롯 보관량 +50%)"
```
