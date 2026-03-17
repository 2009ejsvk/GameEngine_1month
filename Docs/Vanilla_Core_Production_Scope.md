# Vanilla Core Production Scope

현재 즉시 지원 범위는 `바닐라 핵심 생산경제 + source-backed major mode switch`다.

포함:

- 1차 생산: 플랜테이션, 수경 플랜테이션, 목장, 공장식 목장, 광산, 자동 광산
- 주요 가공 체인: 럼, 가죽, 치즈, 시가, 강철, 옷감, 무기, 초콜릿, 가구, 보석, 플라스틱, 자동차, 전자 제품, 의류, 의약품, 주스
- 주요 mode switch:
  - exact 산출물 전환: 플랜테이션, 수경 플랜테이션, 목장, 공장식 목장, 광산, 자동 광산
  - exact 입력 전환: 주스 공장, 제약회사

제외:

- `Goldnuts`
- `SpecialChocolate`
- `BS`
- Future/DLC 특수 goods 기반 mode

원칙:

- 경제, 팀스터, 수입/수출, 정책 선택은 exact concrete resource만 실자원으로 취급한다.
- `Crops`, `AnimalProducts`, `Ore`, `HydroponicProduce`, `FactoryLivestock`, `FeedCrops`는 family/summary 용도에만 쓴다.
- 제외된 특수 goods는 enum/코드에 흔적이 남아 있어도 현재 플레이 표면에서는 노출하지 않는다.
