# PlacementAreaObject.cpp — 건물 배치 시스템 완전 해설

> C++를 처음 배우는 학생을 위한 학습 노트입니다.
> 코드보다 개념을 먼저 이해하고, 그 다음 코드를 읽는 순서로 구성했습니다.

---

## 이 파일은 무엇을 하는가

게임에서 건물 배치 버튼을 누르면 무슨 일이 생기는지 생각해보자.

마우스를 움직이면 건물 모양이 따라다닌다. 놓을 수 있는 땅이면 초록색, 다른 건물과 겹치면 빨간색으로 바뀐다. 회전 키를 누르면 방향이 돌아간다. 마우스를 클릭하면 건물이 확정되고, ESC를 누르면 취소된다.

이 모든 과정이 `PlacementAreaObject.cpp` 한 파일 안에 들어있다. 단순해 보이는 UI 뒤에 얼마나 많은 계산이 숨어 있는지, 지금부터 하나씩 풀어보겠다.

---

# 1장. C++ 기초 — 코드를 읽기 전에 알아야 할 것들

## 1-1. 클래스와 객체

C++에서 **클래스(class)** 는 설계도다. `CPlacementAreaObject`라는 클래스는 "건물 배치 오브젝트가 어떤 데이터를 갖고 어떻게 동작하는지"를 정의한다.

그 설계도로 실제로 만들어진 것이 **객체(Object)** 다. 게임 맵에 건물이 10개 있으면 `CPlacementAreaObject` 객체가 10개 존재하는 것이다. 각 객체는 자기 자신의 위치, 크기, 점유 타일 목록을 따로 갖는다.

클래스 이름 앞의 `C`는 이 프로젝트의 명명 규칙이다. `C`로 시작하면 클래스, `F`로 시작하면 구조체(struct), `E`로 시작하면 열거형(enum)이라고 생각하면 된다.

## 1-2. 멤버 변수 — `m`으로 시작하는 것들

클래스 안에 저장되는 데이터를 **멤버 변수**라고 한다. 이 코드에서는 `m`으로 시작하는 이름이 전부 멤버 변수다.

```cpp
mPrimaryPlacedIndices   // 이 건물이 점유 중인 타일 목록
mPreviewIndices         // 미리보기 중인 타일 목록
mPreviewCanPlace        // 현재 위치에 배치 가능한지 (true/false)
mMovePreviewActive      // 지금 미리보기 모드인지
mPreviewDirection       // 회전 방향 (0, 1, 2, 3)
```

`m`은 "member"의 약자다. 전역 변수(어디서든 접근 가능한 변수)와 구분하기 위해 붙인다. 전역 변수는 이 코드에서 `G`로 시작한다 (`GPrimaryOverlayState` 등).

## 1-3. 함수와 메서드

클래스 안에 정의된 함수를 **메서드**라고 부른다. `Update()`, `ConfirmPlacement()`, `ClearPreview()` 같은 것들이다. 이 파일 안의 모든 함수는 `CPlacementAreaObject`의 메서드다.

```cpp
void CPlacementAreaObject::ConfirmPlacement()
{
    // ...
}
```

`CPlacementAreaObject::` 앞에 붙은 것은 "이 함수가 어느 클래스 소속인지"를 명시하는 것이다. `::`는 "의"라고 읽으면 된다. "CPlacementAreaObject의 ConfirmPlacement 함수."

## 1-4. 포인터와 스마트 포인터

C++에서 **포인터**는 "어떤 데이터가 메모리 어느 위치에 있는지를 가리키는 주소값"이다. 도서관 책 번호처럼, 번호를 알면 책을 찾아갈 수 있다.

문제는 일반 포인터가 위험하다는 것이다. 책이 이미 반납됐는데도 번호를 들고 가면 엉뚱한 책이 나온다. 게임 엔진에서 객체는 언제든 삭제될 수 있으므로, 삭제된 객체를 가리키는 포인터가 생겨버릴 수 있다.

이 문제를 해결하기 위해 C++11부터 **스마트 포인터**가 도입됐다. 이 코드에서는 두 종류를 쓴다.

### shared_ptr — 강한 참조

`shared_ptr`는 "내가 이 객체를 쓰고 있으므로 절대 삭제하지 말아라"는 강한 약속이다. `shared_ptr`를 들고 있는 사람이 한 명이라도 있으면 객체는 절대 삭제되지 않는다. 여러 명이 같은 객체를 `shared_ptr`로 가리키면 내부의 참조 카운터가 올라간다. 마지막 사람이 놓아줄 때 비로소 삭제된다.

```cpp
std::shared_ptr<CTileMapComponent> TileMap;
// TileMap이 살아있는 동안 CTileMapComponent 객체는 삭제되지 않는다.
```

### weak_ptr — 약한 참조

`weak_ptr`는 "혹시 살아있으면 쓸 텐데, 없어도 괜찮다"는 약한 참조다. 이것만으로는 객체가 삭제되는 것을 막지 못한다. 도서관에서 책 번호를 적어뒀지만, 책이 반납됐는지 확인해야 꺼낼 수 있는 상황과 같다.

```cpp
std::weak_ptr<CWorld> mWorld;
// mWorld 혼자서는 월드를 살려두지 못한다.
```

### lock() — 약한 참조를 강한 참조로 변환

`weak_ptr`에서 실제 객체를 꺼내려면 `.lock()`을 호출한다. 객체가 살아있으면 유효한 `shared_ptr`가 나오고, 이미 사라졌으면 비어있는 `shared_ptr`(nullptr처럼 동작)가 나온다.

```cpp
auto World = mWorld.lock();   // weak_ptr에서 shared_ptr 꺼내기
if (!World)                   // 꺼내기 실패(객체가 없음)
    return;                   // 그냥 돌아감
// 여기서부터는 World가 유효하다는 게 보장됨
```

이 패턴이 이 파일 전체에서 수십 번 반복된다. "사용하기 전에 살아있는지 확인하고, 없으면 돌아간다"는 방어적 프로그래밍 습관이다.

## 1-5. vector — 크기가 변하는 배열

`std::vector<int>`는 정수를 담는 배열인데, 크기를 실행 중에 자유롭게 늘리거나 줄일 수 있다. C의 배열과 달리 처음부터 크기를 정하지 않아도 된다.

```cpp
std::vector<int> mPrimaryPlacedIndices;
// 처음엔 비어있다.
mPrimaryPlacedIndices.push_back(42);  // 42 추가
mPrimaryPlacedIndices.push_back(43);  // 43 추가
mPrimaryPlacedIndices.clear();        // 전부 지우기
mPrimaryPlacedIndices.size();         // 현재 원소 개수
mPrimaryPlacedIndices.empty();        // 비어있는지 여부
```

이 코드에서 타일 인덱스 목록(`mPrimaryPlacedIndices`, `mPreviewIndices` 등)은 모두 `vector<int>`로 관리된다.

## 1-6. bool — 참/거짓 값

`bool`은 `true` 또는 `false` 중 하나만 담는 변수다.

```cpp
bool mPreviewCanPlace = false;   // 배치 가능한지
bool mMovePreviewActive = false; // 미리보기 모드 켜짐 여부
bool mTileMapPrepared = false;   // 초기화 완료 여부
```

`if (!mPreviewCanPlace)` 는 "배치가 불가능하면"이라는 뜻이다. `!`는 부정 연산자다.

## 1-7. 열거형(enum) — 이름 붙인 숫자

열거형은 의미 있는 이름을 숫자에 붙여주는 것이다.

```cpp
enum class ETileType { Normal, UnableToMove, ... };
```

코드에서 `ETileType::UnableToMove`는 "이동 불가 타일"을 의미한다. 숫자 `1`이라고 쓰는 것보다 훨씬 읽기 쉽다. `E`로 시작하는 이름이 나오면 열거형이라고 생각하면 된다.

## 1-8. 람다 — 함수 안의 함수

람다는 함수 안에서 즉석으로 만드는 작은 함수다. `[&]` 또는 `[]`로 시작하는 부분이 람다다.

```cpp
auto ClampTo100 = [](int Value)
{
    return (std::max)(0, (std::min)(100, Value));
};
```

이 람다는 `Value`를 0~100 사이로 강제하는 역할이다. `ClampTo100(150)`을 호출하면 100이 나온다. 함수를 따로 만들 필요 없이 그 자리에서 쓰고 버릴 수 있어서 코드가 간결해진다.

`[&]`로 시작하는 람다는 바깥 함수의 변수들을 그대로 가져다 쓸 수 있다. `[]`는 외부 변수를 가져다 쓰지 않는 람다다.

## 1-9. const — 바꾸지 않겠다는 약속

`const`는 "이 값은 변경하지 않는다"는 약속이다.

```cpp
const int TileCount = TileMap->GetTileCountX() * TileMap->GetTileCountY();
```

`TileCount`는 한 번 계산한 뒤 바뀌지 않는 값이다. `const`를 붙이면 실수로 값을 바꾸는 버그를 컴파일러가 미리 잡아준다.

## 1-10. static 지역 변수 — 딱 한 번만 초기화

함수 안에 `static` 키워드가 붙은 지역 변수는 특별하다. 함수가 끝나도 사라지지 않고, 다음에 함수를 다시 호출해도 그 값이 남아있다.

```cpp
static CTileMapComponent* sInitializedTileMap = nullptr;

if (sInitializedTileMap != TileMap.get())
{
    // 타일맵 초기화 작업 (한 번만 실행됨)
    sInitializedTileMap = TileMap.get();
}
```

`sInitializedTileMap`은 "어느 타일맵을 이미 초기화했는지" 기억하는 변수다. 처음 호출될 때만 초기화 작업이 실행되고, 그 이후에는 `sInitializedTileMap`이 바뀌지 않았으면 건너뛴다.

---

# 2장. 게임 엔진 기초 — 구조를 이해해야 코드가 보인다

## 2-1. 게임 루프와 Update()

게임 엔진은 매 프레임마다 모든 오브젝트의 `Update()` 함수를 호출한다. 초당 60프레임이면 1초에 60번 호출되는 것이다.

`Update(float DeltaTime)`의 `DeltaTime`은 "이전 프레임에서 지금까지 걸린 시간(초)"이다. 60fps라면 대략 0.016초다. 이 값을 이용하면 프레임레이트가 달라도 같은 속도로 움직이는 오브젝트를 만들 수 있다. 이 파일에서는 DeltaTime을 시간 계산에 직접 쓰지는 않지만, 매 프레임마다 상태를 갱신하는 데 사용된다.

## 2-2. 컴포넌트 시스템

게임 오브젝트는 여러 **컴포넌트**로 구성된다. 렌더링을 담당하는 컴포넌트, 물리 충돌을 담당하는 컴포넌트, 위치/회전을 담당하는 씬 컴포넌트 등이 붙어서 오브젝트의 기능을 만든다.

`Init()` 함수에서 `CreateComponent<CSceneComponent>("Root")`로 루트 컴포넌트(위치의 기준점)를 만든다.

## 2-3. 월드(World)

**월드**는 게임의 모든 오브젝트가 존재하는 공간이다. `mWorld`는 이 오브젝트가 속한 월드를 가리키는 `weak_ptr`다. 타일맵을 찾으려면 월드에서 이름으로 검색한다:

```cpp
auto World = mWorld.lock();
mTileMapObject = World->FindObject<CTileMapObject>(GTileMapObjectName);
```

`FindObject<CTileMapObject>`는 "CTileMapObject 타입의 오브젝트 중 이름이 GTileMapObjectName인 것을 찾아라"는 뜻이다. `<>` 안에 타입을 넣어서 어떤 종류를 찾을지 알려주는 것이 C++ **템플릿** 문법이다.

## 2-4. 타일맵과 타일

**타일맵**은 바둑판처럼 생긴 2차원 격자다. 각 칸이 **타일**이다. 타일 하나는:

- `GetType()` : 현재 타일 타입 (Normal, UnableToMove 등)
- `SetTileType()` : 타일 타입 변경
- `SetOutLineColor()` : 외곽선 색 변경
- `GetCenter()` : 타일 중심의 2D 좌표
- `GetIndexX()`, `GetIndexY()` : 격자 상 x, y 좌표

이런 정보를 갖고 있다. `CTileMapComponent`가 이 타일들을 배열로 들고 있고, `GetTile(Index)`로 특정 타일에 접근한다.

`GetTile(Index)`는 `weak_ptr<CTile>`을 반환한다. 따라서 `.lock()`으로 꺼내야 실제 타일에 접근할 수 있다:

```cpp
auto Tile = TileMap->GetTile(Index).lock();
if (!Tile) continue;   // 타일이 없으면 건너뜀
Tile->SetOutLineColor(FVector4::Blue);
```

---

# 3장. 타일 인덱스 — 2D를 1D로

## 타일 좌표와 인덱스의 관계

타일맵은 2차원이지만, 타일 하나를 가리킬 때 코드는 정수 하나(Index)를 사용한다. 이렇게 하면 배열에 바로 접근할 수 있어서 빠르다.

```
가로 10칸 타일맵의 경우:

  x=0  x=1  x=2  ...  x=9
y=0 [0]  [1]  [2]  ...  [9]
y=1 [10] [11] [12] ...  [19]
y=2 [20] [21] [22] ...  [29]
```

공식: `Index = y * CountX + x`

반대로, 인덱스에서 좌표 복원:
- `x = Index % CountX`
- `y = Index / CountX`

이 공식은 코드 전체에서 암묵적으로 사용된다. 예를 들어:

```cpp
const int CenterIndex = CenterY * CountX + CenterX;
```

---

# 4장. 아이소메트릭 격자 — 왜 좌표 계산이 복잡한가

## 아이소메트릭이란

일반적인 탑다운 게임은 타일이 직사각형이고 위에서 수직으로 내려다본다. 아이소메트릭은 45도 비스듬히 기울여 내려다보는 시점이다. 타일이 마름모처럼 보인다.

```
일반 직교 격자:        아이소메트릭 격자:
□□□□□               ◇ ◇ ◇
□□□□□              ◇ ◇ ◇ ◇
□□□□□               ◇ ◇ ◇
```

비스듬한 격자에서는 짝수 행과 홀수 행이 서로 반 칸씩 어긋나 있다. 예를 들어 y=0 행의 타일들은 x=0, 1, 2, 3... 에 있지만, y=1 행의 타일들은 실제 화면에서 반 칸 오른쪽으로 밀려있다.

## 논리 좌표 — 어긋남을 보정한 좌표

이 코드는 이 어긋남을 보정하기 위해 **논리 좌표(Logical Coordinate)** 를 사용한다.

```cpp
// 홀수 행이면 x에 0.5 더함 (반 칸 보정)
const float LogicalX = x + (y % 2 == 0 ? 0.f : 0.5f);
// y축은 실제 화면 높이의 절반이므로 0.5 곱함
const float LogicalY = y * 0.5f;
```

이렇게 논리 좌표로 변환하면 "직교 격자처럼" 거리를 계산할 수 있다. `(y % 2 == 0 ? 0.f : 0.5f)`는 "y가 짝수면 0, 홀수면 0.5"라는 3항 연산자다.

## 맨해튼 거리

두 점 사이의 **맨해튼 거리**는 `|dx| + |dy|`다. 직선 거리(유클리드 거리)와 달리, 격자 위에서 상하좌우로만 이동할 때의 최단 거리다. 이름은 뉴욕 맨해튼의 바둑판 도로에서 왔다.

```cpp
const float DistX = fabs(LogicalX - CenterLogicalX);
const float DistY = fabs(LogicalY - CenterLogicalY);

if (DistX + DistY > DiamondRadius)
    continue;  // 다이아몬드 바깥이면 건너뜀
```

맨해튼 거리가 반지름 이하인 타일들이 다이아몬드 모양을 만든다. 이것이 `BuildDiamondAreaIndices()`의 핵심 원리다.

---

# 5장. 다이아몬드 영역 — 건물이 차지하는 땅

## 왜 다이아몬드인가

아이소메트릭 게임에서 건물 영역이 화면에 직사각형으로 보이려면, 실제 격자에서는 다이아몬드(마름모) 모양으로 타일을 선택해야 한다. 아이소메트릭 변환 때문에 격자 다이아몬드가 화면에서는 직사각형처럼 보이게 된다.

## BuildDiamondAreaIndices() 단계별 해설

```cpp
bool CPlacementAreaObject::BuildDiamondAreaIndices(
    const std::shared_ptr<CTileMapComponent>& TileMap,
    int CenterIndex,
    std::vector<int>& OutIndices) const
```

인자 설명:
- `TileMap` : 타일맵 컴포넌트
- `CenterIndex` : 다이아몬드의 중심 타일 인덱스
- `OutIndices` : 결과 인덱스 목록 (참조로 받아서 채운다)
- `const` 뒤에 붙은 것은 이 함수가 멤버 변수를 바꾸지 않는다는 뜻

**단계 1: 중심 타일 정보 추출**

```cpp
auto CenterTile = TileMap->GetTile(CenterIndex).lock();
const int CenterX = CenterTile->GetIndexX();
const int CenterY = CenterTile->GetIndexY();
const int Radius = mTemplate.DiamondRadius;
```

**단계 2: 탐색 범위 설정**

다이아몬드 반지름이 2라면, 중심에서 상하좌우로 최대 4칸까지 탐색한다 (`SearchRange = Radius * 2`). 넉넉하게 잡는 이유는 아이소메트릭 어긋남 때문에 조금 더 멀리까지 볼 필요가 있기 때문이다.

**단계 3: 사각 범위를 훑으면서 다이아몬드 내부만 선택**

```cpp
for (int y = CenterY - SearchRange; y <= CenterY + SearchRange; ++y)
{
    for (int x = CenterX - SearchRange; x <= CenterX + SearchRange; ++x)
    {
        // 맵 경계 벗어나면 건너뜀
        // 논리 좌표 계산
        // 맨해튼 거리 > 반지름이면 건너뜀
        // 통과한 타일만 OutIndices에 추가
    }
}
```

2중 반복문으로 사각 범위 전체를 훑는다. 각 타일의 논리 좌표와 중심의 논리 좌표 사이의 맨해튼 거리가 반지름 이하면 다이아몬드 안이다.

**단계 4: 입구 만들기 (HasDirectionalGap)**

`HasDirectionalGap`이 `true`인 템플릿은 다이아몬드에서 타일 1개를 뚫어 입구처럼 만든다. `Diamond3x3SingleMarker`가 여기에 해당한다. `FindPreviewOpenTileIndex()`로 현재 회전 방향에 해당하는 외곽 타일을 찾아서 목록에서 제거한다.

## 템플릿 종류

```
Diamond3x3SingleMarker (반지름 1, 기본값):
      ◇
    ◇ ◇ ◇      (중심에서 맨해튼 거리 1 이하)
      ◇
    단, 방향에 따라 1칸이 뚫린다.

Diamond5x5TwoMarker (반지름 2):
        ◇
      ◇ ◇ ◇
    ◇ ◇ ◇ ◇ ◇   (중심에서 맨해튼 거리 2 이하)
      ◇ ◇ ◇
        ◇
```

`GetExpectedTileCount()`는 이 템플릿에서 정확히 몇 개의 타일이 나와야 하는지 반환한다. 계산 결과 개수가 예상과 다르면 맵 경계에 걸린 것으로 판단해 배치 불가로 처리한다.

---

# 6장. 오버레이 시스템 — 색상이 겹쳐도 꼬이지 않는 이유

## 오버레이 타일맵이란

화면에는 타일맵이 겹겹이 쌓여있다.

```
[노랑 오버레이 레이어]   ← 마커(입구) 표시
[파랑 오버레이 레이어]   ← 건물 점유 영역 표시
[기본 타일맵 레이어]     ← 실제 지형
```

건물을 배치하면 그 건물이 차지하는 타일 위에 파랑 오버레이가 켜지고, 입구 타일 위에는 노랑 오버레이가 켜진다.

## 문제 상황 — 여러 건물이 있을 때

건물 A와 건물 B가 인접해서, 두 건물 모두 타일 #50번을 파랑으로 칠하고 싶다고 하자. (이것이 가능한 경우는 오버레이 맵에서만 — 기본 타일맵에서는 점유 타일이 겹치면 배치 불가다.)

A가 먼저 칠했다. B도 칠했다. 나중에 A가 철거됐다. "A가 #50을 더 이상 쓰지 않으니 파랑을 지워라"고 하면, B도 아직 #50을 쓰고 있는데 색이 지워져버린다.

## 해결책 — 참조 카운트(Reference Count)

이 문제를 **참조 카운트**로 해결한다. 각 타일마다 "몇 명이 이 타일을 사용 중인지"를 세는 숫자를 유지한다.

```
타일 #50의 RefCount:
  A가 켬: 0 → 1  (처음이므로 실제로 색을 칠함)
  B가 켬: 1 → 2  (이미 색이 있으므로 그냥 숫자만 올림)
  A가 끔: 2 → 1  (아직 B가 사용 중이므로 색은 유지)
  B가 끔: 1 → 0  (마지막이므로 실제로 색을 지움)
```

이 카운터 배열이 `FOverlayTileState.RefCounts`다:

```cpp
struct FOverlayTileState
{
    CTileMapComponent* TileMap = nullptr;
    std::vector<int> RefCounts;   // 타일 개수만큼의 배열
};
```

파랑/노랑 두 오버레이를 따로 관리하기 위해 전역으로 두 개를 선언한다:

```cpp
FOverlayTileState GPrimaryOverlayState;   // 파랑 (점유 영역)
FOverlayTileState GMarkerOverlayState;    // 노랑 (마커)
```

## UpdateOverlayTileRefs() 상세 해설

이 함수가 참조 카운트 전체를 관리한다.

```cpp
void UpdateOverlayTileRefs(
    FOverlayTileState& State,          // 파랑 또는 노랑 상태
    const shared_ptr<CTileMapComponent>& TileMap,
    std::vector<int>& InOutAppliedIndices,  // 현재 적용된 타일 목록 (입출력)
    const std::vector<int>& NextIndices,    // 앞으로 적용할 타일 목록 (입력)
    const FVector4& VisibleColor)           // 칠할 색상
```

**1단계: 이전 목록의 참조를 모두 해제**

`InOutAppliedIndices`(지난 프레임에 칠했던 타일들)를 순회하며 각 타일의 RefCount를 1 내린다. 0이 되면 색을 지운다 (알파를 0으로 만들어 투명하게).

**2단계: 새 목록으로 교체**

`InOutAppliedIndices = NextIndices;`로 내부 목록을 갱신한다.

**3단계: 새 목록의 참조를 모두 추가**

`NextIndices`(이번 프레임에 칠해야 할 타일들)를 순회하며 각 타일의 RefCount를 1 올린다. 0에서 1이 된 타일(처음 사용하는 것)만 실제로 색을 칠한다.

이전/다음 목록이 같으면 아무것도 하지 않는다:
```cpp
if (InOutAppliedIndices == NextIndices)
    return;
```
이 최적화 덕에 건물이 움직이지 않는 매 프레임마다 불필요한 타일 처리를 건너뛸 수 있다.

---

# 7장. 배치 상태 머신 — 건물이 올라가는 전 과정

"상태 머신"이란 "어떤 상태인지에 따라 다르게 동작하는 시스템"이다. 이 오브젝트는 크게 네 가지 상태를 거친다.

```
[미준비] → [준비 완료] → [미리보기 중] → [확정 배치]
             ↑                               ↓
             └──────────── (이동 시) ←───────┘
```

## 상태 1: 미준비 → 준비 완료 (EnsurePlacementObject)

`mTileMapPrepared`가 `false`인 상태. `Update()`가 처음 호출될 때 `EnsurePlacementObject()`가 준비 작업을 한다.

**타일맵 초기화 (최초 1회)**

```cpp
static CTileMapComponent* sInitializedTileMap = nullptr;
if (sInitializedTileMap != TileMap.get())
{
    // 전체 타일 순회: 이동불가 → 파랑, 그 외 → 흰색
    sInitializedTileMap = TileMap.get();
}
```

`static` 지역 변수는 함수가 몇 번 호출되어도 딱 한 번만 초기화된다. 따라서 타일맵 전체의 외곽선 색을 정리하는 작업이 딱 한 번만 일어난다.

**자동 배치 시도**

`mAutoPlaceOnPrepare`가 켜져 있으면 맵 중심에 자동으로 건물을 놓는다. 중심이 안 되면 맵 전체를 좌상단부터 훑어서 첫 번째 유효한 위치를 찾는다.

```cpp
for (int y = 0; y < CountY && !Found; ++y)
    for (int x = 0; x < CountX; ++x)
    {
        // 다이아몬드 영역 계산
        // 모든 타일이 비어있으면 여기에 놓기로 결정
        if (IsAreaPlaceable(TileMap, StartPrimaryIndices))
        {
            Found = true;
            break;
        }
    }
```

`&& !Found`는 이미 자리를 찾았으면 더 이상 탐색하지 않도록 하는 조기 종료 조건이다.

## 상태 2: 미리보기 (StartMovePreview / UpdatePlacementPreviewFromMouse)

건물 배치 버튼을 누르면 `StartMovePreview()`를 호출한다. `mMovePreviewActive = true`가 되고, 이후 `Update()` 매 프레임마다 마우스 위치로 미리보기를 갱신한다.

**매 프레임 흐름:**

```
1. ClearPreview()          - 이전 프레임 프리뷰 색 지우기
2. GetTileIndex(마우스좌표) - 마우스 아래 타일 인덱스 계산
3. BuildDiamondAreaIndices - 그 타일 중심으로 다이아몬드 계산
4. IsAreaPlaceable()       - 배치 가능 여부 판단
5. SetAreaColor()          - 가능하면 초록, 불가면 빨강
6. 중심 타일은 노란색으로  - 강조 표시
```

**IsAreaPlaceable() 판단 기준:**

```cpp
for (size_t i = 0; i < Indices.size(); ++i)
{
    auto Tile = TileMap->GetTile(Indices[i]).lock();

    // UnableToMove 타일인데 내가 점유한 게 아니면 → 배치 불가
    if (Tile->GetType() == ETileType::UnableToMove &&
        !IsPlacedIndex(Indices[i]))
    {
        return false;
    }
}
return true;
```

핵심은 `!IsPlacedIndex(Indices[i])` 조건이다. "이 타일이 이미 이동불가지만, 내가 기존에 점유한 타일이라면 괜찮다"는 뜻이다. 건물을 제자리 또는 기존 영역과 겹치는 위치로 이동할 때를 허용하기 위해서다.

**회전 처리:**

`RotatePreviewCW()`는 시계 방향:
```cpp
mPreviewDirection = (mPreviewDirection + 1) % 4;
```

`RotatePreviewCCW()`는 반시계 방향:
```cpp
mPreviewDirection = (mPreviewDirection + 3) % 4;
```

왜 반시계가 `-1` 대신 `+3`인가? `mPreviewDirection`이 0일 때 0-1 = -1이 되는데, 0~3 사이를 유지하려면 -1을 3으로 바꿔야 한다. `(-1 + 4) % 4 = 3`이고, `(0 + 3) % 4 = 3`이므로 같은 결과다. 음수를 피하기 위한 C++ 관용구다.

## 상태 3: 확정 (ConfirmPlacement)

마우스 클릭 시 호출. `mPreviewCanPlace`가 `true`일 때만 진행한다.

```
1. 마커 초기화       - mMarkerTileIndices.clear()
2. 기존 점유 해제    - 기존 타일 → ETileType::Normal, 흰색
3. 새 영역 계산      - BuildDiamondAreaIndices(프리뷰 중심 기준)
4. 새 영역 점유      - 새 타일 → ETileType::UnableToMove
5. 상태 확정         - mPrimaryPlacedIndices = 새 목록
6. 프리뷰 초기화     - mPreviewIndices.clear(), mMovePreviewActive = false
7. 위치 동기화       - SyncWorldPosFromCenter()로 오브젝트 위치 맞춤
8. 색상 갱신         - ApplyPlacedAreaColor()로 파랑/노랑 오버레이 갱신
```

**SyncWorldPosFromCenter() — 왜 필요한가**

오브젝트의 월드 위치(게임 공간에서의 실제 좌표)는 타일 위치와 별개로 관리된다. 건물을 새 위치에 확정하면 오브젝트의 위치를 새 중심 타일의 화면 좌표에 맞춰야 한다. 그렇지 않으면 오브젝트는 다른 곳에 있는데 타일만 강조되는 상황이 생긴다.

```cpp
const FVector2 Center = CenterTile->GetCenter();          // 타일 중심 좌표
const FVector3 TileMapWorldPos = TileMapObj->GetWorldPos(); // 타일맵 월드 위치
SetWorldPos(Center.x + TileMapWorldPos.x,
            Center.y + TileMapWorldPos.y,
            GetWorldPos().z);  // z는 유지
```

타일 좌표는 타일맵 기준 로컬 좌표다. 타일맵 자체가 월드 어딘가에 있으므로, 타일맵의 월드 위치를 더해야 실제 월드 좌표가 된다.

## 상태 4: 소멸 (Destroy)

오브젝트가 삭제될 때 반드시 타일 상태를 원상복구해야 한다. 이걸 빠뜨리면 건물이 사라져도 타일이 계속 이동불가 상태로 남아 길이 영영 막힌다.

```cpp
// 1. 확정 타일 원상복구
for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
{
    Tile->SetTileType(ETileType::Normal);
    Tile->SetOutLineColor(FVector4::White);
}

// 2. 오버레이 참조 전부 해제 (빈 목록 전달 = 모든 참조 내리기)
UpdatePrimaryOverlayTiles(std::vector<int>());
UpdateMarkerOverlayTiles(std::vector<int>());
```

`std::vector<int>()`는 비어있는 벡터를 만드는 표현이다. "오버레이에 아무것도 적용하지 않겠다"고 선언하면, 내부에서 기존에 올려뒀던 참조 카운트를 전부 내린다.

---

# 8장. 마커 타일 — 입구이자 길찾기 목표

## 마커란

건물의 "입구" 역할을 하는 타일이다. 노란색으로 표시된다. 주민이나 차량은 이 타일 앞에 도착하면 건물에 도착한 것으로 간주된다.

## ApplyPlacedAreaColor() — 마커 위치를 어떻게 계산하는가

마커 위치는 템플릿의 `MarkerAnchors`에 저장된 논리 오프셋으로 정한다.

```cpp
// 예: Diamond3x3SingleMarker의 앵커
Template.MarkerAnchors.push_back({ 0.5f, 0.5f });
```

`{ 0.5f, 0.5f }`는 중심에서 논리 좌표로 `(+0.5, +0.5)` 방향에 있는 타일이 마커라는 뜻이다.

**회전 적용:**

회전 방향(`mPreviewDirection`)에 따라 오프셋을 변환한다. 90도씩 회전하는 수식은:

```cpp
for (int r = 0; r < Rotation; ++r)
{
    const float PrevX = OffsetX;
    OffsetX = OffsetY;
    OffsetY = -PrevX;
}
```

90도 회전 변환 공식 `(x, y) → (y, -x)`를 Rotation 횟수만큼 반복 적용하는 것이다. 행렬 회전의 단순화 버전이다.

**가장 가까운 타일 찾기:**

변환된 논리 오프셋 방향으로 `FindMarkerTileIndexByLogicalOffset()`을 호출해, 그 방향에 가장 가까운 **외곽 타일**을 찾는다.

**외곽 타일이란:** 8방향 이웃 중 하나 이상이 배치 영역 밖에 있는 타일. 즉, 영역의 가장자리.

```cpp
for (int Dir = 0; Dir < 8; ++Dir)
{
    const int Neighbor = GetIsoNeighborIndexByDir(TileMap, CandidateIndex, Dir);
    if (Neighbor < 0 || !IsInArea(Neighbor))
    {
        IsEdge = true;
        break;
    }
}
```

## 길찾기 연동

네비게이션 시스템이 이 오브젝트에 두 가지를 물어본다.

**막힌 타일 (`GetNavigationBlockedTiles`):**
건물이 점유한 모든 타일을 막힌 타일로 반환한다. 단, 마커 타일은 제외한다.

```cpp
for (각 점유 타일)
{
    if (IsGoalTile(Index))
        continue;   // 마커는 제외
    OutIndices.push_back(Index);
}
```

마커까지 막아버리면 아무도 건물에 접근할 수 없게 되기 때문이다. 마커는 통과 가능하지만 "도착 지점"으로 등록된다.

**목표 타일 (`GetNavigationGoalTiles`):**
마커 타일 인덱스만 반환한다. 에이전트는 이 타일에 도달하면 건물 도착으로 처리된다.

---

# 9장. 아이소메트릭 이웃 탐색 — GetIsoNeighborIndexByDir

외곽 타일을 판별하려면 임의의 타일의 8방향 이웃을 알아야 한다. 아이소메트릭 격자에서는 홀짝 행이 어긋나 있어서 단순 ±1로는 안 된다.

## 보조 격자(Grid) 변환

이 함수는 타일 좌표 `(x, y)`를 "보조 격자 좌표 `(GridX, GridY)`"로 변환한다.

```cpp
const int GridX = x + ((y + (y & 1)) / 2);
const int GridY = x - (y / 2);
```

- `y & 1`은 비트 AND 연산이다. y가 홀수면 1, 짝수면 0이다. `y % 2`와 같은 결과지만 더 빠르다.

이 보조 격자에서는 아이소메트릭 어긋남이 제거되어, 8방향 이웃이 단순한 ±1로 계산된다.

```cpp
const int DirX[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
const int DirY[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
// 시계 방향: 위, 우상, 우, 우하, 아래, 좌하, 좌, 좌상
```

이웃의 보조 격자 좌표를 구한 뒤 역변환으로 원래 타일 좌표로 돌아온다:

```cpp
const int NextY = NextGridX - NextGridY;
const int NextX = NextGridY + (NextY / 2);
```

변환과 역변환 공식이 왜 이렇게 되는지는 아이소메트릭 기하학에서 나오는 내용이다. 지금은 "비스듬한 격자와 직교 격자 사이를 오가는 공식"이라고만 이해해도 충분하다.

---

# 10장. 건물 정보 — SetBuildingDisplayInfo

배치와 직접 관련은 없지만, 같은 오브젝트가 건물의 경제 데이터도 담당한다.

## 매개변수들

```
DisplayName          - 건물 표시 이름
CategoryName         - 분류 (주거, 농업, 상업 등)
Residential          - 주거 건물 여부
Capacity             - 수용 인원
FoodProvider         - 식량 제공 여부
EntertainmentProvider - 오락 제공 여부
HousingSatisfactionCap - 주거 만족도 최댓값 (0~100)
JobSatisfactionCap   - 직업 만족도 최댓값 (0~100)
FoodSatisfactionCap  - 식량 만족도 최댓값 (0~100)
FunSatisfactionCap   - 오락 만족도 최댓값 (0~100)
BaseMonthlyWage      - 월간 기본 임금 (-1이면 자동 계산)
BaseMonthlyUpkeep    - 월간 기본 유지비 (-1이면 자동 계산)
```

## 자동 계산 로직

`BaseMonthlyWage`나 `BaseMonthlyUpkeep`에 -1이 들어오면 수용 인원(`Capacity`)을 기반으로 적절한 값을 계산한다.

```cpp
if (BaseMonthlyWage < 0)
{
    if (mResidential)
        mBaseMonthlyWage = 0;             // 주거 건물은 임금 없음
    else
        mBaseMonthlyWage = Capacity * 120; // 일반 건물은 인원당 120
}
```

항구(`IsHarbor()`)나 운송 사무소(`IsTransportOffice()`)는 최솟값을 더 높게 보장해 준다. `(std::max)(a, b)`는 a와 b 중 큰 값을 반환하는 표준 함수다.

만족도 수치는 람다로 0~100으로 강제 클램핑된다:

```cpp
auto ClampTo100 = [](int Value)
{
    return (std::max)(0, (std::min)(100, Value));
};
mHousingSatisfactionCap = ClampTo100(HousingSatisfactionCap);
```

`(std::min)(100, Value)`로 100 이하로 자르고, `(std::max)(0, ...)`로 0 이상으로 자른다. 두 번 자르면 0~100 범위가 된다.

---

# 11장. 색상 복원 — RestoreTileColor

프리뷰가 지워지거나 철거 hover가 끝났을 때, 타일을 "원래 보여야 할 색"으로 되돌린다. 단순히 흰색으로 돌리면 안 되는 경우가 있기 때문에 세 가지 경우를 구분한다.

```
경우 1: 이 타일에 오버레이 참조가 있다
        (파랑 또는 노랑 오버레이가 켜진 상태)
        → 흰색 (오버레이가 위에 칠해지므로 흰색이 맞음)

경우 2: 오버레이 없고, 타일 타입이 UnableToMove
        (다른 건물이 점유하고 있는 상태)
        → 파란색 (해당 건물이 없어도 이 건물이 여기 있음을 표시)

경우 3: 오버레이 없고, 빈 타일
        → 흰색
```

```cpp
if (HasOverlayRef(GPrimaryOverlayState, Index) ||
    HasOverlayRef(GMarkerOverlayState, Index))
{
    Tile->SetOutLineColor(FVector4::White);
}
else if (Tile->GetType() == ETileType::UnableToMove)
    Tile->SetOutLineColor(FVector4::Blue);
else
    Tile->SetOutLineColor(FVector4::White);
```

`||`는 OR 연산자다. 둘 중 하나라도 참이면 참이다.

---

# 12장. 전체 상태 변수 요약

| 변수명 | 타입 | 의미 |
|---|---|---|
| `mPrimaryPlacedIndices` | `vector<int>` | 확정 배치된 타일 인덱스 목록 |
| `mPlacedCenterIndex` | `int` | 확정 배치의 중심 타일 인덱스 (-1이면 없음) |
| `mMarkerTileIndices` | `vector<int>` | 마커(노랑) 타일 인덱스 목록 |
| `mPreviewIndices` | `vector<int>` | 미리보기 중인 타일 인덱스 목록 |
| `mPreviewCenterIndex` | `int` | 미리보기 중심 타일 인덱스 |
| `mPreviewCanPlace` | `bool` | 현재 위치에 배치 가능한지 여부 |
| `mMovePreviewActive` | `bool` | 미리보기 모드가 켜져 있는지 |
| `mPreviewDirection` | `int` | 회전 방향 0=아래, 1=오른쪽, 2=위, 3=왼쪽 |
| `mTileMapPrepared` | `bool` | 준비 과정 완료 여부 (한 번만 실행됨) |
| `mAutoPlaceOnPrepare` | `bool` | 준비 시 자동 배치 여부 |
| `mDemolitionHoverActive` | `bool` | 철거 모드에서 마우스가 이 건물 위인지 |
| `mAppliedPrimaryOverlayIndices` | `vector<int>` | 현재 파랑 오버레이에 실제 적용된 타일 목록 |
| `mAppliedMarkerOverlayIndices` | `vector<int>` | 현재 노랑 오버레이에 실제 적용된 타일 목록 |
| `mTemplate` | `FPlacementTemplate` | 현재 건물의 모양 템플릿 |
| `mInitialCenterOffsetX/Y` | `int` | 자동 배치 시 맵 중심에서의 오프셋 |

---

# 마치며

처음 이 파일을 읽으면 1500줄이 넘는 코드에 압도된다. 하지만 결국 이 파일이 하는 일은 하나다. **"마우스가 가리키는 타일을 중심으로 다이아몬드를 그리고, 그 다이아몬드를 색으로 표현하고, 클릭하면 확정한다."**

복잡해 보이는 이유는 네 가지다.

1. **아이소메트릭 좌표** — 직교 격자가 아니라서 이웃 계산에 변환이 필요하다.
2. **weak_ptr 방어 코드** — 객체가 사라졌을 때를 항상 대비해야 한다.
3. **오버레이 RefCount** — 여러 건물이 화면을 공유할 때 색이 꼬이지 않게 한다.
4. **상태 일관성 유지** — 미리보기 → 확정 → 소멸 각 단계에서 타일 타입과 색을 올바르게 유지해야 한다.

이 네 가지 이유를 이해하면, 나머지 코드는 그 해결책들이 쌓인 것에 불과하다.
