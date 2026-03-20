# 01. 엔진 구조 & 게임 루프

## 1-1. 진입점 (main.cpp)

```cpp
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, ...)
{
    CEngine::GetInst()->CreateEngineSetting<CGlobalSetting>();
    CEngine::GetInst()->Init(hInstance, TEXT("GameClient"), ..., 1280, 720, true);

    CEngine::CreateCDO<CMainCamera>();

    // 시작 월드 지정
    CWorldManager::GetInst()->CreateWorld<CStartWorld>(false);

    int Ret = CEngine::GetInst()->Run();   // 게임 루프 시작
    CEngine::DestroyInst();
    return Ret;
}
```

**핵심 개념**
- `CEngine::GetInst()` : 싱글턴 패턴으로 엔진 인스턴스 접근
- `CreateEngineSetting<T>()` : 전역 설정 객체 등록
- `CreateCDO<T>()` : Class Default Object — UE4와 유사한 클래스 기본값 시스템
- `CWorldManager` : 월드 생명 주기 관리 (생성 / 전환 / 소멸)

---

## 1-2. 엔진 싱글턴 패턴

이 엔진은 **매니저 싱글턴** 패턴을 광범위하게 사용합니다.

```
CEngine           — 최상위 엔진 객체 (게임 루프, 윈도우)
CWorldManager     — 월드 생성 / 전환
CRenderManager    — 렌더 레이어 관리
CAssetManager     — 텍스처, 폰트 등 에셋 관리
```

```cpp
// 싱글턴 접근 패턴 (엔진 전반에서 동일)
CRenderManager::GetInst()->CreateLayer("UI", 10, ERenderListSort::None);
```

**왜 싱글턴인가?**
- 게임 엔진의 핵심 서비스(렌더, 입력, 오디오)는 전역 접근이 필요합니다.
- 생명 주기 관리가 명확합니다 (`GetInst` / `DestroyInst`).

---

## 1-3. 월드 전환 흐름

```
wWinMain
  └─ CreateWorld<CStartWorld>()    시작 화면
       └─ (게임 시작 버튼 클릭)
            └─ CreateWorld<CMainWorld>()   실제 게임 월드
```

| 월드 클래스 | 역할 |
|-------------|------|
| `CStartWorld` | 시작 메뉴 화면 |
| `CMainWorld` | 인게임 메인 월드 (시뮬레이션 전체) |
| `CEditorWorld` | 맵 에디터 |
| `CLoadingWorld` | 로딩 화면 |

---

## 1-4. 게임 루프 구조

```
CEngine::Run()
  ├── ProcessInput()      — 키보드/마우스 입력 처리
  ├── Update(DeltaTime)   — 모든 월드/오브젝트 업데이트
  │    ├── World->Update()
  │    │    ├── 시뮬레이션 Tick (Economy, Politics, Citizens ...)
  │    │    └── UI Widget Update
  │    └── Camera Update
  └── Render()            — 화면 출력
       ├── 타일맵 렌더
       ├── 건물 / 유닛 렌더
       └── UI 렌더 (위젯 트리 순회)
```

**DeltaTime 기반 업데이트**
- 모든 `Update(float DeltaTime)` 함수는 프레임 독립적으로 동작합니다.
- 게임 내 날짜/시뮬레이션은 누적 시간으로 진행됩니다.

---

## 1-5. 디버그 모드 특이점

`main.cpp` 의 `#ifdef _DEBUG` 블록을 보면 흥미로운 패턴이 있습니다.

```cpp
#ifdef _DEBUG
if (ShouldLaunchMainWorldValidation())
    CWorldManager::GetInst()->CreateWorld<CMainWorld>(false);
else
    CWorldManager::GetInst()->CreateWorld<CStartWorld>(false);
#endif
```

- `ConstitutionValidation.request` 파일이 실행 파일 옆에 존재하면
  시작 화면을 건너뛰고 바로 `MainWorld` 를 로드합니다.
- **개발자 편의 기능**: 특정 기능만 빠르게 테스트할 때 사용합니다.
- 파일을 생성/삭제하는 것만으로 동작을 바꾸는 "파일 플래그" 기법입니다.

---

## 1-6. 라이브러리 링크 분기

```cpp
#ifdef _DEBUG
#pragma comment(lib, "GameEngine_Debug.lib")
#else
#pragma comment(lib, "GameEngine.lib")
#endif
```

- Debug / Release 빌드에 따라 다른 엔진 라이브러리를 링크합니다.
- `#pragma comment(lib, ...)` : MSVC 전용 링크 지시자 (CMake 없이 간단하게 처리)

---

## 학습 포인트 요약

1. **싱글턴 패턴**은 편리하지만 테스트 어려움과 전역 상태 문제가 있습니다.
2. **월드 시스템**은 게임 상태(화면)를 객체로 추상화합니다.
3. **DeltaTime 기반 게임 루프**는 프레임레이트에 독립적인 시뮬레이션을 가능하게 합니다.
4. **파일 플래그**처럼 작은 개발자 편의 기능을 코드에 통합하는 방법을 눈여겨봅시다.
