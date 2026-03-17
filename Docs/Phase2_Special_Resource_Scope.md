# Phase 2 Special Resource Scope

이 문서는 `바닐라 핵심 생산경제 + 주요 mode switch` 범위 밖의 특수 자원군을 별도 스코프로 묶어 둔다.

대상 자원:

- `SpecialChocolate`
- `Goldnuts`
- `BS`

대상 건물/계열:

- `Vertical Farm`
- `Synthetic Food Lab`
- Future/DLC 특수 goods 기반 workmode

현재 원칙:

- enum 값은 저장 호환과 데이터 키 안정성을 위해 유지한다.
- 가격표 자리값은 남겨 두되, 현재 무역/정책/UI 표면에는 노출하지 않는다.
- `BuildingOperationModes.tsv`에는 source-backed 바닐라 핵심 mode만 유지한다.
- `BuildingTypes.h`의 `IsImmediateProductionScopeResourceType(...)`가 1차 범위를 결정한다.

2차 작업에서 열어야 할 층:

1. `BuildingTypes.h`
   exact 자원 의미, market class, edible/summary 규칙 재정의
2. `ResourceTradePricing.h`
   실제 가격 밸런스와 무역 편향 재조정
3. `BuildingOperationModes.tsv`
   source-backed 특수 mode 행 복원
4. 무역/UI/정책
   selection, filter, 설명 문구, 통계 집계 재노출
5. 경제/물류
   생산, 재고, shortage, reserve, export/import 경로 검증
