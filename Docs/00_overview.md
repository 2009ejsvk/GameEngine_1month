# GameEngine_1month 학습 자료

## 프로젝트 개요

이 프로젝트는 **Tropico 시리즈**에서 영감을 받은 도시 건설 시뮬레이션 게임 엔진입니다.
Win32 + C++ 로 작성된 커스텀 엔진 위에 게임 로직과 UI 시스템이 구현되어 있습니다.

---

## 학습 문서 목차

| 파일 | 주제 |
|------|------|
| [01_engine_architecture.md](01_engine_architecture.md) | 엔진 구조 & 게임 루프 |
| [02_world_system.md](02_world_system.md) | 월드 시스템 & 서브시스템 분리 |
| [03_ui_system.md](03_ui_system.md) | UI 아키텍처 (Widget / Renderer 분리) |
| [04_economy_system.md](04_economy_system.md) | 경제 시스템 |
| [05_politics_system.md](05_politics_system.md) | 정치 & 선거 시스템 |
| [06_citizen_system.md](06_citizen_system.md) | 시민 시스템 & 만족도 |
| [07_building_catalog.md](07_building_catalog.md) | 건물 카탈로그 & 데이터 설계 |
| [08_data_driven_design.md](08_data_driven_design.md) | 데이터 주도 설계 & INI 핫리로드 |
| [09_exercises.md](09_exercises.md) | 실습 과제 |

---

## 프로젝트 구조 한눈에 보기

```
GameEngine_1month/
├── Client/                  # 게임 클라이언트 코드 (C++)
│   ├── main.cpp             # 진입점
│   ├── Citizen/             # 시민 AI, 만족도, 정치 성향
│   ├── Economy/             # 경제 정산, 무역, 세금
│   ├── Politics/            # 칙령, 헌법, 선거
│   ├── Building/            # 건물 카탈로그 & 메타데이터
│   ├── Map/                 # 타일맵, 건물 배치
│   ├── UI/                  # 위젯, 렌더러, 데이터 프로바이더
│   └── World/               # 게임 월드, 서브시스템
├── Binary/                  # 실행 파일 + 설정 파일 (INI)
│   └── UILayout/            # UI 레이아웃 INI (핫리로드)
└── docs/                    # 학습 자료 (이 폴더)
```

---

## 핵심 기술 스택

- **언어**: C++17
- **플랫폼**: Windows (Win32 API)
- **빌드**: Visual Studio (vcxproj)
- **그래픽**: 커스텀 렌더 레이어 (DirectX 기반 엔진)
- **설정**: INI 파일 (런타임 핫리로드)

---

## 게임 콘셉트

| 요소 | 설명 |
|------|------|
| 플레이어 역할 | 열대 섬나라 독재자 (El Presidente) |
| 목표 | 시민 만족도 유지 + 선거 승리 |
| 핵심 자원 | 예산(Budget), 시민 지지율, 파벌 지지도 |
| 시대 구분 | 식민지 → 세계대전 → 냉전 → 현대 (4 Era) |
| 건물 카테고리 | 8개 (주거, 식량, 산업, 관광, 복지 등) |
| 정치 파벌 | 8개 (공산주의자, 자본주의자, 종교, 군사, 환경, 산업, 지식인, 보수) |
