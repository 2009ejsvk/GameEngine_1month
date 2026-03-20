# 03. UI 아키텍처 (Widget / Renderer 분리)

## 3-1. 핵심 설계 원칙

이 프로젝트의 UI는 세 가지 역할로 명확히 분리됩니다.

```
Widget          — 상태(State) 보유, 입력 처리, 위젯 트리 소유
DataProvider    — 게임 데이터 → UI 표시용 데이터 변환
Renderer        — DataProvider 결과를 받아 실제 화면에 그리기
```

**왜 분리하는가?**
- `Widget`이 렌더링까지 직접 하면 수백 줄의 거대한 클래스가 됩니다.
- 렌더링 로직만 따로 테스트하거나 교체하기 어렵습니다.
- Renderer는 Widget의 내부 구조를 몰라도 됩니다.

---

## 3-2. CitizenInfoWidget 구조 분석

NPC/건물 정보 패널은 이 분리 패턴의 대표적인 예입니다.

```
CCitizenInfoWidget              (Widget — 상태, 입력)
  ├── FCitizenInfoState         (현재 선택 상태, 탭 상태)
  ├── FPanelChromeWidgets       (공통 UI 요소: 타이틀, 탭 버튼 등)
  ├── FBuildingPanelWidgets     (건물 모드 전용 위젯들)
  └── FCitizenPanelWidgets      (시민 모드 전용 위젯들)

CitizenInfoDataProvider         (DataProvider — 데이터 변환)
  └── FCitizenInfoSnapshot      (스냅샷: 표시할 모든 데이터)

FCitizenInfoRenderer            (Renderer — 스냅샷 → 화면)
```

---

## 3-3. FRendererView 패턴

`CCitizenInfoWidget::FRendererView` 는 흥미로운 패턴입니다.

```cpp
struct FRendererView
{
    CCitizenInfoWidget& Owner;          // Widget 역참조
    EPanelMode&         mPanelMode;     // 레퍼런스로 상태 공유
    WImage&             mPanelImage;    // 위젯 멤버를 레퍼런스로 노출
    WText&              mTitleText;
    // ... 모든 하위 위젯에 대한 레퍼런스
};
```

- Widget의 멤버를 **레퍼런스로 묶은 뷰(View) 객체**입니다.
- Renderer는 이 View를 통해 Widget 멤버에 접근합니다.
- Widget 내부 구조가 바뀌어도 View 인터페이스가 유지되면 Renderer는 변경이 없습니다.

```cpp
// Widget이 Renderer에게 View를 넘겨주는 방식
FRendererView View = GetRendererView();
FCitizenInfoRenderer::Render(View, Snapshot);
```

---

## 3-4. 이중 모드 위젯

`CCitizenInfoWidget`은 **시민 모드**와 **건물 모드** 두 가지를 하나의 위젯으로 처리합니다.

```cpp
enum class ECitizenInfoPanelMode
{
    Citizen,
    Building
};
```

```
탭 구성:
  건물 모드: 기본 | 통계 | 업글 | 효율 | 정보  (5탭)
  시민 모드: 기본 | 정치 | 성향              (3탭)
```

- 탭 버튼은 동일한 위젯 배열을 재사용합니다.
- 모드에 따라 레이블과 가시성만 동적으로 변경합니다.
- 위젯 수를 줄여 메모리와 생성 비용을 절약합니다.

---

## 3-5. UI 레이아웃 시스템

모든 UI 수치는 코드에 하드코딩되지 않고, **INI 파일 + 전역 변수**로 관리됩니다.

```cpp
// UILayoutValues.h — 전역 변수 선언
namespace UIConfig
{
    extern float StatusBarHeight;      // 상단 상태바 높이
    extern float EdictHeaderHeight;    // 칙령 헤더 높이
    extern float AlmanacLeftPanelRatio; // 알마낙 좌측 비율
    // ... 수백 개의 레이아웃 변수
}
```

```
UILayoutConfig.cpp  — 기본값 설정
UILayoutLoader.cpp  — INI 파일 읽어서 전역 변수에 반영
UILayoutApplier.cpp — 변경된 값을 실행 중인 위젯에 적용
```

**핫리로드 흐름**
```
Binary/UILayout/*.ini 파일 수정
        ↓ (약 0.5초 후 자동 감지)
UILayoutLoader 가 변경 감지
        ↓
UIConfig 전역 변수 갱신
        ↓
UILayoutApplier 가 실행 중인 위젯 재배치
```

게임을 재시작하지 않고 UI 크기/위치를 조정할 수 있습니다.

---

## 3-6. 스케일 팩터

화면 해상도에 따라 UI 크기가 자동 조정됩니다.

```cpp
float Scale = std::min(1.0f, screenFit);
// 모든 UI 수치에 Scale을 곱해 적용
```

- `screenFit`: 현재 해상도 / 기준 해상도
- `min(1, screenFit)`: 기준보다 크면 확대하지 않고 기준 크기 유지
- 작은 화면에서는 비율에 맞게 축소합니다.

---

## 3-7. 주요 UI 구성 요소

| 위젯 | 역할 |
|------|------|
| `TopHudWidget` | 상단 HUD (예산, 인구, 날짜, 지지율) |
| `BuildMenuWidget` | 건물 건설 메뉴 |
| `CitizenInfoWidget` | NPC/건물 정보 패널 |
| `AlmanacWidget` | 국가 통계 백과사전 (9페이지) |
| `EdictWidget` | 칙령 발동 UI |
| `TradeWidget` | 무역 노선 설정 |
| `EventWidget` | 이벤트 알림 팝업 |
| `TaskWidget` | 임무/목표 UI |
| `ResultWidget` | 게임 결과 화면 |

---

## 3-8. 약한 포인터(weak_ptr) 사용

```cpp
using WImage  = std::weak_ptr<class CImage>;
using WText   = std::weak_ptr<class CTextBlock>;
using WButton = std::weak_ptr<class CButton>;

WImage mTitleIcon;
WText  mTitleText;
```

- 위젯 트리의 **소유권은 WidgetContainer**가 가집니다.
- Widget 클래스는 자식 위젯을 `weak_ptr`로 참조합니다.
- 자식 위젯이 외부에서 삭제되어도 Widget이 댕글링 포인터를 갖지 않습니다.
- `.lock()`으로 유효성을 확인 후 사용합니다.

```cpp
if (auto Text = mTitleText.lock())
{
    Text->SetText(L"건물 이름");
}
```

---

## 학습 포인트 요약

1. **Widget/DataProvider/Renderer 분리**: 관심사 분리로 코드를 작고 명확하게 유지합니다.
2. **View 패턴**: 레퍼런스 묶음으로 인터페이스를 노출해 결합도를 낮춥니다.
3. **INI 기반 레이아웃**: 코드 수정 없이 UI를 조정할 수 있습니다.
4. **약한 포인터**: 소유권과 참조를 분리해 안전한 메모리 관리를 합니다.
5. **이중 모드 위젯**: 유사한 UI는 재사용해 위젯 수를 최소화합니다.
