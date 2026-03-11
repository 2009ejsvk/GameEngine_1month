# 🎨 렌더링 파이프라인 상세 분석 가이드

본 문서는 `GameEngine/Render/RenderManager.cpp` 코드를 바탕으로 이 게임 엔진의 시각적 출력 과정(렌더링 파이프라인)을 심층 분석합니다.

---

## 1. 개요 (Overview)

이 엔진의 렌더링 파이프라인은 **"지연된 2D/2.5D 합성 방식"**을 채택하고 있습니다. 모든 오브젝트를 즉시 화면에 그리는 것이 아니라, `MainTarget`이라는 중간 버퍼에 먼저 그리고, 각종 후처리(Post-process)를 거친 뒤 최종적으로 `BackBuffer`에 출력합니다.

---

## 2. 주요 구성 요소

### 2.1. 레이어 시스템 (Layer System)
- **주요 함수**: `CRenderManager::CreateLayer`, `AddRenderLayer`
- **구조**: `mRenderLayerMap`을 통해 여러 렌더 레이어를 순서대로 관리합니다.
- **정렬 (`SortYRenderList`)**: 2D 게임에서 깊이(Depth)를 표현하기 위해 Y축 좌표를 기준으로 오브젝트를 정렬하는 로직이 핵심입니다. (`SrcY > DestY`)

### 2.2. 렌더 타겟 (Render Targets)
- **MainTarget**: 게임 월드의 모든 오브젝트(배경, 캐릭터 등)가 그려지는 1차 버퍼.
- **FinalTarget**: 후처리가 완료된 결과가 담기는 최종 버퍼.
- **UI/Mouse**: 최종 결과 위에 별도로 덮어씌워지는 최상위 레이어.

---

## 3. 매 프레임 실행 흐름 (Execution Flow)

### 단계 1: 업데이트 (`CRenderManager::Update`)
1.  **레이어 관리**: 유효하지 않은(Expired) 컴포넌트를 정리하고, 런타임에 변경된 레이어 정보를 반영합니다.
2.  **정렬**: 각 레이어의 설정(`ERenderListSort`)에 따라 오브젝트를 정렬합니다. (예: `Y-Sort`)
3.  **인스턴싱(Instancing) 준비**: 동일한 메쉬와 텍스처를 사용하는 오브젝트들을 그룹화하여 성능을 최적화(Draw Call 절감)합니다.
4.  **포스트 프로세스 업데이트**: 델타 타임에 맞춰 후처리 효과(예: Blur)의 애니메이션이나 상태를 갱신합니다.

### 단계 2: 렌더링 (`CRenderManager::Render`)
1.  **MainTarget 설정**: 모든 드로우 명령이 `MainTarget`으로 전송되도록 설정합니다.
2.  **레이어 순차 렌더링**: 정렬된 레이어 리스트를 순회하며 `GameObject->Render()`를 호출합니다. 인스턴싱 그룹도 한꺼번에 그려집니다.
3.  **포스트 프로세스 (Post-processing)**:
    - 등록된 효과가 있다면 `MainTarget`의 결과를 입력으로 받아 변환을 수행합니다.
    - 결과물은 `FinalTarget`으로 전송됩니다.
4.  **최종 출력 및 UI**: 
    - `FinalTarget`의 내용을 화면 전역 사각형(FullScreen Quad)에 입혀 출력합니다.
    - 그 위에 가독성을 위해 별도의 알파 블렌딩 상태(`AlphaBlend`)로 UI와 마우스 커서를 그립니다.

---

## 4. 코드 핵심 로직 분석 (Deep Dive)

### 인스턴싱 로직 (`CheckInstancing`)
- 같은 종류의 스프라이트가 많을 때(예: 숲의 나무들), 이를 한 번의 Draw Call로 처리하기 위해 `InstancingMap`을 사용합니다. `Mesh`, `Shader`, `Texture`의 ID를 조합한 키값을 사용하여 그룹화합니다.

### Y-정렬 알고리즘 (`SortYRenderList`)
```cpp
if (fabsf(SrcY - DestY) > Epsilon)
    return SrcY > DestY; // Y축 값이 클수록(화면 아래쪽) 나중에 그려짐
```
- 아이소메트릭(Isometric) 뷰에서 캐릭터가 건물 뒤로 숨거나 앞으로 나오는 자연스러운 깊이 표현을 가능하게 합니다.

### 풀스크린 쿼드 렌더링 (`RenderFullScreenQuad`)
- 렌더 타겟의 결과물을 화면에 꽉 차게 그릴 때 정점 버퍼 없이(Null Buffer) 쉐이더 내에서 4개의 정점을 생성하여 그리는 방식을 사용합니다.

---

## 5. 교육 및 활용 포인트
- **Draw Call 최적화**: 왜 인스턴싱이 필요한지, 그리고 어떻게 구현되어 있는지 학생들에게 설명하기 좋습니다.
- **2.5D 깊이 구현**: Z-Buffer를 사용하지 않고 소프트웨어적으로 정렬(Y-Sorting)하는 방식의 원리를 학습할 수 있습니다.
- **포스트 프로세스 구조**: 하나의 결과물이 다음 프로세스의 입력이 되는 체이닝(Chaining) 기법을 이해할 수 있습니다.
