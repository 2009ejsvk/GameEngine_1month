# 08. 데이터 주도 설계 & INI 핫리로드

## 8-1. 데이터 주도 설계(Data-Driven Design) 란?

게임의 수치와 동작을 **코드가 아닌 데이터 파일**로 정의하는 설계 방식입니다.

```
하드코딩 방식:
  코드에 AlmanacMetricRowHeight = 46; 을 박아 넣는다
  → 수정 시 재컴파일 필요

데이터 주도 방식:
  INI 파일에 AlmanacMetricRowHeight=60 을 쓴다
  → 런타임에 파일 읽기, 재시작이나 재컴파일 불필요
```

---

## 8-2. 이 프로젝트의 INI 파일 구조

```
Binary/UILayout/
├── 06_Edict.ini           — 칙령 UI 레이아웃
├── 07_Almanac.ini         — 알마낙 UI 레이아웃
├── 08_BuildingPanel.ini   — 건물 패널 레이아웃
├── 10_WidgetOverrides_Guide.ini
├── 11_WidgetOverrides_CitizenInfo.ini
├── 13_WidgetOverrides_TaskWidget.ini
├── 14_WidgetOverrides_Constitution.ini
├── 15_WidgetOverrides_Constitution_Integrated.ini
├── 16_WidgetOverrides_Event_BuildMenu.ini
├── 17_WidgetOverrides_TopHud_Left.ini
└── 18_WidgetOverrides_Harbor.ini

Binary/
├── Edicts.ini             — 칙령 정의 데이터
├── EraConfig.ini          — 시대 전환 조건
└── PopupConfig.ini        — 팝업 동작 설정
```

---

## 8-3. UI 레이아웃 핫리로드 시스템

```cpp
// UILayoutLoader.h
class UILayoutLoader
{
    // 파일 수정 시간을 추적
    std::unordered_map<std::string, FILETIME> mFileTimestamps;
public:
    bool ReloadIfChanged(float DeltaTime);  // 매 프레임 변경 감지
    void ApplyToWidgets();                  // 변경된 값을 위젯에 반영
};
```

**핫리로드 흐름**
```
매 프레임 ReloadIfChanged() 호출
  ↓
INI 파일들의 수정 시간 확인 (파일시스템 API)
  ↓
변경 감지 시 → 파일 재파싱
  ↓
UIConfig::* 전역 변수 갱신
  ↓
UILayoutApplier::Apply() → 실행 중인 위젯 위치/크기 갱신
```

약 **0.5초 이내**에 반영됩니다.

---

## 8-4. UIConfig 전역 변수 패턴

```cpp
// UILayoutValues.h — 선언만 (extern)
namespace UIConfig
{
    extern float AlmanacMetricRowHeight;
    extern float AlmanacDetailRowHeight;
    extern float BuildingIconSize;
    // ...
}

// UILayoutConfig.cpp — 기본값 정의
namespace UIConfig
{
    float AlmanacMetricRowHeight = 46.f;  // 기본값
    float AlmanacDetailRowHeight = 46.f;
    float BuildingIconSize       = 22.f;
}
```

- `extern` 선언으로 모든 파일에서 접근 가능합니다.
- 기본값은 코드에 있으므로 INI 파일이 없어도 동작합니다.
- INI 값이 기본값을 덮어씁니다.

---

## 8-5. GameBalanceTuning — 런타임 수치 조정

UI 레이아웃뿐 아니라 **게임 밸런스 수치**도 런타임 조정이 가능합니다.

```cpp
// GameBalanceTuning.h
namespace GameBalanceTuning
{
    void RegisterRuntimeConfig();
    bool ReloadIfChanged(float DeltaTime);

    namespace Almanac
    {
        extern double SatisfactionClampMin;   // 만족도 최솟값
        extern double SatisfactionClampMax;   // 만족도 최댓값
        extern double SatisfactionLiftScale;  // 만족도 상승 배율
    }

    namespace Politics
    {
        extern double ElectionWarningCriticalThreshold;
        extern double ElectionWarningCautionThreshold;
    }
}
```

개발 중에:
- 만족도 상승 속도를 파일만 수정해서 빠르게 테스트할 수 있습니다.
- 선거 경고 임계값을 조정해 게임 난이도를 튜닝합니다.
- **코드 재컴파일 없이** 밸런스 패치가 가능합니다.

---

## 8-6. RuntimeConfigRegistry

여러 모듈의 런타임 설정을 통합 관리합니다.

```cpp
// RuntimeConfigRegistry.h
class RuntimeConfigRegistry
{
public:
    void Register(const std::string& ConfigId, IRuntimeConfig* Config);
    void ReloadAll(float DeltaTime);  // 등록된 모든 설정 파일 체크
};
```

```
GameBalanceTuning::RegisterRuntimeConfig()
  → RuntimeConfigRegistry에 파일 감시 등록
UILayoutLoader::Register()
  → RuntimeConfigRegistry에 등록

MainWorld::Update(DeltaTime)
  → RuntimeConfigRegistry::ReloadAll(DeltaTime)
     → 변경된 파일만 선택적으로 다시 읽기
```

---

## 8-7. 칙령 데이터 파일 (Edicts.ini)

칙령의 효과, 비용, 조건 등도 데이터 파일에 정의됩니다.

```ini
[FoodForThePeople]
Era=Colonial
DailyFoodDelta=+0.5
DailyBudgetDelta=-200
FactionApproval_Communists=+5
FactionApproval_Capitalists=-3
```

- 새 칙령 추가 시 코드 수정 없이 INI에 항목만 추가합니다.
- 수치 밸런싱을 디자이너가 코드 몰라도 할 수 있습니다.

---

## 8-8. 데이터 주도 설계의 장단점

**장점**
| 장점 | 설명 |
|------|------|
| 빠른 이터레이션 | 컴파일 없이 수치 변경 확인 가능 |
| 역할 분리 | 프로그래머와 기획자/아티스트가 독립 작업 가능 |
| 모딩 지원 | 파일만 수정하면 커스텀 콘텐츠 제작 가능 |
| 런타임 패치 | 라이브 서비스 게임에서 핫픽스 적용 가능 |

**단점**
| 단점 | 설명 |
|------|------|
| 타입 안전성 부족 | 잘못된 키 이름을 컴파일 타임에 잡을 수 없음 |
| 디버그 어려움 | 수치가 어디서 바뀐지 추적이 복잡해질 수 있음 |
| 파일 관리 | 파일 수가 많아지면 버전 관리가 복잡해짐 |
| 초기 구현 비용 | 로더/적용기 시스템을 처음에 만들어야 함 |

---

## 8-9. 실제 변경 예시

```
Binary/UILayout/07_Almanac.ini 열기

변경 전:
AlmanacMetricRowHeight=46

변경 후:
AlmanacMetricRowHeight=60

→ 게임 실행 중 약 0.5초 후 알마낙의 행 높이가 즉시 변경됨
→ 편집기와 게임을 오가며 빠르게 확인 가능
```

---

## 학습 포인트 요약

1. **핫리로드의 핵심**: 파일 수정 시간 비교 → 변경 시에만 파싱 → 비용 최소화
2. **extern 전역 변수 패턴**: 선언(Values.h)과 정의(Config.cpp)를 분리하면 모든 코드에서 같은 변수를 공유합니다.
3. **기본값 보장**: INI 없이도 기본값으로 동작하도록 설계하면 강건한 시스템이 됩니다.
4. **레지스트리 패턴**: 여러 설정 모듈을 하나의 레지스트리에 등록해 일괄 업데이트합니다.
5. **데이터와 코드 분리**: 수치를 데이터 파일로 이동하면 프로그래머 외 팀원도 게임 콘텐츠를 조정할 수 있습니다.
