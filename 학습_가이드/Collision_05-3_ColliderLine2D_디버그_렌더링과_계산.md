## ColliderLine2D 디버그 렌더링과 선분 계산 상세 분석

이 문서는 ColliderLine2D의 **디버그 렌더링(선 그리기)**과 **선분 계산** 과정을 학습 레벨에서 상세하게 설명합니다.

### 📚 관련 문서

**필수 참고 문서:**
- **`Collision_05-4_LineUP2D_메시_생성과_로드.md`**: LineUP2D 메시의 생성/저장/로드 전체 과정
- **`Collision_05-5_왜_Mesh_접두사를_사용하는가.md`**: "Mesh_LineUP2D" vs "LineUP2D" 네이밍 규칙 설명

**기본 문서:**
- **`Collision_05-2_ColliderLine2D.md`**: ColliderLine2D 클래스 구조와 사용법

### 📝 이 문서에서 다루는 내용

1. **선분 계산** (PostUpdate)
   - 시작점/끝점 계산
   - 회전 행렬 적용 (`TransformNormal`)
   - AABB 바운딩 박스 계산

2. **디버그 렌더링** (Render)
   - LineUP2D 메시 구조
   - 행렬 변환 과정 (스케일 → 회전 → 이동)
   - GPU 렌더링 파이프라인

3. **디버깅 팁**
   - 메시 로드 실패 해결
   - 접두사 문제 해결
   - 회전/길이/색상 문제 디버깅

---

## 1. 선분 계산 과정 (PostUpdate)

### 1-1. 전체 흐름

```cpp
void CColliderLine2D::PostUpdate(float DeltaTime)
{
    CCollider::PostUpdate(DeltaTime);

    // ============================================
    // STEP 1: 시작점 설정
    // ============================================
    mInfo.Start = mWorldPos;

    // ============================================
    // STEP 2: 회전된 방향 벡터 계산
    // ============================================
    FVector3 Dir;
    Dir = mLineDir.TransformNormal(mRotMatrix);
    Dir.Normalize();

    // ============================================
    // STEP 3: 끝점 계산
    // ============================================
    mInfo.End = mInfo.Start + Dir * mDistance;

    // ============================================
    // STEP 4: AABB 바운딩 박스 계산
    // ============================================
    mMin.x = mInfo.Start.x < mInfo.End.x ? mInfo.Start.x : mInfo.End.x;
    mMin.y = mInfo.Start.y < mInfo.End.y ? mInfo.Start.y : mInfo.End.y;

    mMax.x = mInfo.Start.x > mInfo.End.x ? mInfo.Start.x : mInfo.End.x;
    mMax.y = mInfo.Start.y > mInfo.End.y ? mInfo.Start.y : mInfo.End.y;

    // ============================================
    // STEP 5: 렌더링 스케일 설정
    // ============================================
    mRenderScale.x = 1.f;           // 선의 두께 (고정)
    mRenderScale.y = mDistance;     // 선의 길이
    mRenderScale.z = 1.f;
}
```

### 1-2. STEP 1: 시작점 설정

```cpp
mInfo.Start = mWorldPos;
```

**설명:**
- 선분의 시작점 = 오브젝트의 월드 위치
- `mWorldPos`는 부모 클래스 `CSceneComponent`에서 계산된 최종 월드 좌표
- 만약 부모 오브젝트가 있다면, 부모의 위치도 반영된 값

**예시:**
```cpp
// 플레이어가 (100, 200, 0)에 있고
// Line2D의 상대 위치가 (0, 50, 0)이면
// mWorldPos = (100, 250, 0)
// → 선분 시작점 = (100, 250, 0)
```

### 1-3. STEP 2: 회전된 방향 벡터 계산

```cpp
FVector3 Dir;
Dir = mLineDir.TransformNormal(mRotMatrix);
Dir.Normalize();
```

**배경 지식: TransformNormal이란?**
- 벡터를 회전 행렬로 변환하는 함수
- **Normal**: 법선, 방향을 의미 (위치가 아닌 방향만 변환)
- 이동(Translation)은 적용하지 않고, **회전(Rotation)만 적용**

**TransformNormal 내부 동작:**
```cpp
// FVector3::TransformNormal(const FMatrix& Matrix)
FVector3 Result;
Result.x = x * Matrix.m[0][0] + y * Matrix.m[1][0] + z * Matrix.m[2][0];
Result.y = x * Matrix.m[0][1] + y * Matrix.m[1][1] + z * Matrix.m[2][1];
Result.z = x * Matrix.m[0][2] + y * Matrix.m[1][2] + z * Matrix.m[2][2];
return Result;
```

**왜 이렇게 하는가?**
1. `mLineDir`은 **로컬 방향** (기본값: Y축 = (0, 1, 0))
2. 오브젝트가 회전하면 선분도 함께 회전해야 함
3. `mRotMatrix`에 오브젝트의 월드 회전 정보가 담겨있음
4. 로컬 방향 × 회전 행렬 = 월드 방향

**실제 예시:**
```cpp
// 초기 설정
Line->SetLineDir(0.f, 1.f, 0.f);  // Y축 방향 (위쪽)
// mLineDir = (0, 1, 0)

// 오브젝트가 90도 회전 (Z축 기준 반시계방향)
// 회전 행렬:
// [  0  -1   0 ]
// [  1   0   0 ]
// [  0   0   1 ]

// 변환 결과:
// Dir.x = 0*0 + 1*1 + 0*0 = 1
// Dir.y = 0*(-1) + 1*0 + 0*0 = 0
// Dir.z = 0*0 + 1*0 + 0*1 = 0
// → Dir = (1, 0, 0)  // X축 방향 (오른쪽)

// Normalize() 후에도 (1, 0, 0) (이미 단위 벡터)
```

### 1-4. STEP 3: 끝점 계산

```cpp
mInfo.End = mInfo.Start + Dir * mDistance;
```

**벡터 연산:**
- `Dir * mDistance`: 방향 벡터에 거리를 곱함 → 변위 벡터
- `Start + 변위`: 시작점에서 변위만큼 이동한 점 = 끝점

**예시:**
```cpp
// Start = (100, 250, 0)
// Dir = (0, 1, 0)  // 위쪽 방향
// Distance = 200

// 계산:
// Dir * Distance = (0, 200, 0)
// End = Start + (0, 200, 0) = (100, 450, 0)

// 만약 45도 회전했다면:
// Dir = (0.707, 0.707, 0)  // 대각선 방향
// Dir * 200 = (141.4, 141.4, 0)
// End = (100, 250, 0) + (141.4, 141.4, 0) = (241.4, 391.4, 0)
```

### 1-5. STEP 4: AABB 바운딩 박스 계산

```cpp
mMin.x = mInfo.Start.x < mInfo.End.x ? mInfo.Start.x : mInfo.End.x;
mMin.y = mInfo.Start.y < mInfo.End.y ? mInfo.Start.y : mInfo.End.y;

mMax.x = mInfo.Start.x > mInfo.End.x ? mInfo.Start.x : mInfo.End.x;
mMax.y = mInfo.Start.y > mInfo.End.y ? mInfo.Start.y : mInfo.End.y;
```

**AABB (Axis-Aligned Bounding Box)란?**
- 축 정렬 경계 상자
- 회전하지 않는 박스로 선분을 감싸는 최소/최대 점

**왜 필요한가?**
- 충돌 최적화: 브로드 페이즈(Broad Phase)에서 빠른 필터링
- AABB끼리 먼저 검사 → 겹치지 않으면 정밀 충돌 검사 생략

**예시:**
```cpp
// Start = (100, 250, 0)
// End = (241.4, 391.4, 0)

// Min 계산:
// mMin.x = min(100, 241.4) = 100
// mMin.y = min(250, 391.4) = 250

// Max 계산:
// mMax.x = max(100, 241.4) = 241.4
// mMax.y = max(250, 391.4) = 391.4

// AABB = [(100, 250) ~ (241.4, 391.4)]
```

**시각화:**
```
        End (241.4, 391.4)
           ●
          /|
         / |
        /  |
       /   |  ← AABB 박스
      /    |
     /     |
    ●------+
Start
(100,250)
```

### 1-6. STEP 5: 렌더링 스케일 설정

```cpp
mRenderScale.x = 1.f;           // 선의 두께
mRenderScale.y = mDistance;     // 선의 길이
mRenderScale.z = 1.f;
```

**왜 Y축에 거리를 넣는가?**
- `LineUP2D` 메시는 **Y축 방향으로 길이 1의 선분**
- Y 스케일을 200으로 설정 → 길이 200인 선분이 됨
- 회전 행렬이 적용되어 실제 방향으로 그려짐

E:\GameEngine\학습_가이드\Collision_05-4_LineUP2D_메시_생성과_로드.md
로 보충 설명

---

## 2. 디버그 렌더링 (선 그리기)

### 2-1. 렌더링 파이프라인 개요

```
┌─────────────────┐
│  LineUP2D 메시  │  (Y축 방향 길이 1의 선분)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  스케일 행렬    │  (Y축 mDistance로 스케일)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  회전 행렬      │  (오브젝트 회전 적용)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  이동 행렬      │  (월드 위치로 이동)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  화면에 그려짐  │
└─────────────────┘
```

### 2-2. LineUP2D 메시 구조

> **상세 설명:** `Collision_05-4_LineUP2D_메시_생성과_로드.md` 참조
> **접두사 이유:** `Collision_05-5_왜_Mesh_접두사를_사용하는가.md` 참조

**LineUP2D 메시는 엔진 초기화 시 자동으로 생성됩니다.**

**생성 위치:** `MeshManager.cpp` (라인 156-159)
```cpp
// MeshManager::Init()에서 생성
FVector3 LineUp[2] =
{
    FVector3(0.f, 0.f, 0.f),  // 시작점 (원점)
    FVector3(0.f, 1.f, 0.f)   // 끝점 (Y축 위쪽 1 단위)
};

// "Mesh_LineUP2D" 이름으로 저장됨 (내부 구현)
if (!CreateMesh("Mesh_LineUP2D", LineUp,
    sizeof(FVector3),                   // 정점 크기: 12바이트
    2,                                  // 정점 개수: 2개
    D3D11_USAGE_IMMUTABLE,             // 불변 버퍼 (변경 불가)
    D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP)) // 선분 렌더링
    return false;
```

**정점 데이터 구조:**
```cpp
// FVector3 구조체 (Vector3.h)
struct FVector3
{
    float x = 0.f;  // 4바이트
    float y = 0.f;  // 4바이트
    float z = 0.f;  // 4바이트
    // 전체: 12바이트
};

// LineUp 배열의 메모리 구조
// [0x00] 0.0f, 0.0f, 0.0f  ← 시작점 (12바이트)
// [0x0C] 0.0f, 1.0f, 0.0f  ← 끝점 (12바이트)
// 총 24바이트 → GPU 버퍼로 복사됨
```

**특징:**
- **길이:** 1.0 (단위 길이)
- **방향:** Y축 (위쪽)
- **시작점:** 원점 (0, 0, 0)
- **끝점:** (0, 1, 0)
- **렌더링:** LINE_STRIP (정점 연결해서 선분 그림)
- **변경 불가:** D3D11_USAGE_IMMUTABLE (성능 최적화)

**저장 위치:**
- `MeshManager`의 `mMeshMap`에 **"Mesh_LineUP2D"** 키로 저장
- WorldAssetManager를 통해 로드 시에는 **"LineUP2D"** (접두사 자동 추가됨)

### 2-3. Render 함수 상세 분석

```cpp
void CCollider::Render()
{
    CSceneComponent::Render();

    if (mDebugDraw)
    {
        // ============================================
        // 1. 카메라 행렬 가져오기
        // ============================================
        FMatrix ViewMat;
        FMatrix ProjMat;

        auto World = mWorld.lock();
        if (World)
        {
            auto CameraMgr = World->GetCameraManager().lock();
            if (CameraMgr)
            {
                ViewMat = CameraMgr->GetViewMatrix();
                ProjMat = CameraMgr->GetProjMatrix();
            }
        }

        auto Mesh = mMesh.lock();

        // ============================================
        // 2. 월드 행렬 계산
        // ============================================
        FMatrix ScaleMatrix, RotMatrix, TranslateMatrix, WorldMatrix;

        // 오프셋 적용한 렌더 위치
        FVector3 RenderPos = mWorldPos + mOffset;

        // 각 변환 행렬 생성
        ScaleMatrix.Scaling(mRenderScale);          // (1, 200, 1)
        RotMatrix.Rotation(mWorldRot);              // 오브젝트 회전
        TranslateMatrix.Translation(RenderPos);     // 월드 위치

        // 월드 행렬 = 스케일 × 회전 × 이동
        WorldMatrix = ScaleMatrix * RotMatrix * TranslateMatrix;

        // ============================================
        // 3. Transform 버퍼 업데이트
        // ============================================
        mTransformCBuffer->SetWorldMatrix(WorldMatrix);
        mTransformCBuffer->SetViewMatrix(ViewMat);
        mTransformCBuffer->SetProjMatrix(ProjMat);

        FVector3 PivotSize = mPivot * Mesh->GetMeshSize();
        mTransformCBuffer->SetPivotSize(PivotSize);
        mTransformCBuffer->UpdateBuffer();

        // ============================================
        // 4. 색상 버퍼 업데이트
        // ============================================
        if (mCollision)
            mColliderCBuffer->SetColor(FVector4::Red);    // 충돌 중: 빨간색
        else
            mColliderCBuffer->SetColor(FVector4::Green);  // 미충돌: 초록색

        mColliderCBuffer->UpdateBuffer();

        // ============================================
        // 5. 셰이더와 메시로 렌더링
        // ============================================
        auto Shader = mShader.lock();
        Shader->SetShader();
        Mesh->Render();
    }
}
```

### 2-4. 행렬 변환 과정 상세 설명

**STEP 1: 스케일 행렬**
```cpp
ScaleMatrix.Scaling(mRenderScale);  // (1, 200, 1)

// 스케일 행렬:
// [  1    0    0    0 ]
// [  0  200    0    0 ]
// [  0    0    1    0 ]
// [  0    0    0    1 ]

// 메시의 끝점 (0, 1, 0)에 적용:
// [  1    0    0    0 ]   [ 0 ]   [ 0 ]
// [  0  200    0    0 ] × [ 1 ] = [200]
// [  0    0    1    0 ]   [ 0 ]   [ 0 ]
// [  0    0    0    1 ]   [ 1 ]   [ 1 ]

// 결과: (0, 200, 0) - 길이 200인 선분
```

**STEP 2: 회전 행렬**
```cpp
RotMatrix.Rotation(mWorldRot);  // 예: Z축 45도 회전

// 45도 회전 행렬 (Z축):
// [ 0.707 -0.707   0    0 ]
// [ 0.707  0.707   0    0 ]
// [   0      0     1    0 ]
// [   0      0     0    1 ]

// 끝점 (0, 200, 0)에 적용:
// [ 0.707 -0.707   0    0 ]   [  0  ]   [-141.4]
// [ 0.707  0.707   0    0 ] × [200 ] = [ 141.4]
// [   0      0     1    0 ]   [  0  ]   [   0  ]
// [   0      0     0    1 ]   [  1  ]   [   1  ]

// 결과: (-141.4, 141.4, 0) - 대각선 방향
```

**STEP 3: 이동 행렬**
```cpp
TranslateMatrix.Translation(RenderPos);  // 예: (100, 250, 0)

// 이동 행렬:
// [  1    0    0   100 ]
// [  0    1    0   250 ]
// [  0    0    1    0  ]
// [  0    0    0    1  ]

// 끝점 (-141.4, 141.4, 0)에 적용:
// [  1    0    0   100 ]   [-141.4]   [-41.4 ]
// [  0    1    0   250 ] × [ 141.4] = [ 391.4]
// [  0    0    1    0  ]   [   0  ]   [   0  ]
// [  0    0    0    1  ]   [   1  ]   [   1  ]

// 결과: (-41.4, 391.4, 0) - 월드 좌표
```

**STEP 4: 최종 변환**
```cpp
WorldMatrix = ScaleMatrix * RotMatrix * TranslateMatrix;

// 시작점 (0, 0, 0):
// (0,0,0) → 스케일 → (0,0,0) → 회전 → (0,0,0) → 이동 → (100,250,0)

// 끝점 (0, 1, 0):
// (0,1,0) → 스케일 → (0,200,0) → 회전 → (-141.4,141.4,0) → 이동 → (-41.4,391.4,0)

// 최종 선분: (100,250,0) ~ (-41.4,391.4,0)
```

### 2-5. 실제 렌더링 예시

**코드:**
```cpp
// Player.cpp
mLine2D = CreateComponent<CColliderLine2D>("Line2D");
auto Line = mLine2D.lock();

Line->SetLineDir(0.f, 1.f, 0.f);   // Y축 방향
Line->SetLineDistance(200.f);       // 길이 200
Line->SetDebugDraw(true);
Line->SetRelativePos(0.f, 100.f);   // 플레이어 위 100

// Player가 (500, 300)에 있고, 45도 회전했다면:
```

**계산 과정:**
```cpp
// 1. 월드 위치 계산
mWorldPos = PlayerPos + RelativePos = (500, 300) + (0, 100) = (500, 400)

// 2. 시작점
mInfo.Start = (500, 400)

// 3. 방향 계산 (45도 회전)
mLineDir = (0, 1, 0)
Dir = mLineDir × RotMatrix(45°) = (0.707, 0.707, 0)

// 4. 끝점
mInfo.End = (500, 400) + (0.707, 0.707, 0) * 200
         = (500, 400) + (141.4, 141.4, 0)
         = (641.4, 541.4, 0)

// 5. 렌더링
// - LineUP2D 메시: (0,0,0) ~ (0,1,0)
// - 스케일: Y축 200배 → (0,0,0) ~ (0,200,0)
// - 회전: 45도 → (0,0,0) ~ (141.4,141.4,0)
// - 이동: (500,400) 더함 → (500,400,0) ~ (641.4,541.4,0)
```

**화면에 그려지는 선:**
```
              End (641.4, 541.4)
                 ●
                /
               /
              /  ← 45도 기울어진 선
             /
            /
           ●
        Start (500, 400)
```

---

## 3. 디버깅 팁

### 3-1. 선이 안 보이는 경우

**문제 1: 메시가 로드되지 않음**
```cpp
// 확인 방법
if (mMesh.expired())
{
    OutputDebugString(L"LineUP2D 메시 로드 실패!\n");
}

// 원인: 메시 이름이 잘못되었을 가능성
```

**해결책 1: WorldAssetManager 사용 (권장)**
```cpp
// ColliderLine2D.cpp - SetDebugDraw()
auto World = mWorld.lock();
if (World)
{
    auto AssetMgr = World->GetWorldAssetManager().lock();

    // ✅ 올바른 사용 (접두사 없이)
    mMesh = AssetMgr->FindMesh("LineUP2D");

    // ❌ 잘못된 사용 (접두사 포함하면 안됨)
    // mMesh = AssetMgr->FindMesh("Mesh_LineUP2D");  // 찾을 수 없음!
}
```

**해결책 2: MeshManager 직접 사용 (예외 상황)**
```cpp
// World가 없을 때
else
{
    auto MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();

    // ✅ 올바른 사용 (접두사 포함)
    mMesh = MeshMgr->FindMesh("Mesh_LineUP2D");

    // ❌ 잘못된 사용 (접두사 없으면 찾을 수 없음)
    // mMesh = MeshMgr->FindMesh("LineUP2D");  // 찾을 수 없음!
}
```

**핵심 규칙:**
- **WorldAssetManager**: `"LineUP2D"` (접두사 없음)
- **MeshManager**: `"Mesh_LineUP2D"` (접두사 포함)
- 상세 설명: `Collision_05-5_왜_Mesh_접두사를_사용하는가.md` 참조

**디버깅 체크리스트:**
```cpp
// 1. World가 유효한지 확인
auto World = mWorld.lock();
if (!World)
{
    OutputDebugString(L"World가 null! MeshManager 직접 사용해야 함\n");
}

// 2. AssetManager가 유효한지 확인
auto AssetMgr = World->GetWorldAssetManager().lock();
if (!AssetMgr)
{
    OutputDebugString(L"WorldAssetManager가 null!\n");
}

// 3. 메시 로드 결과 확인
mMesh = AssetMgr->FindMesh("LineUP2D");
if (mMesh.expired())
{
    OutputDebugString(L"메시 로드 실패! 이름 확인 필요\n");

    // 대안: MeshManager에서 직접 시도
    auto MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();
    mMesh = MeshMgr->FindMesh("Mesh_LineUP2D");

    if (!mMesh.expired())
    {
        OutputDebugString(L"MeshManager에서 로드 성공!\n");
    }
}
```

**문제 2: 셰이더가 없음**
```cpp
// 확인 방법
if (mShader.expired())
{
    OutputDebugString(L"Collider 셰이더 로드 실패!\n");
}

// 해결책: "Collider" 셰이더가 ShaderManager에 등록되어 있는지 확인
```

**문제 3: DebugDraw가 false**
```cpp
// 확인
Line->SetDebugDraw(true);  // 반드시 true로 설정
```

### 3-2. 선 방향이 이상한 경우

**원인: 회전 행렬 문제**
```cpp
// 디버깅 코드
char DebugMsg[256];
sprintf_s(DebugMsg, "Dir: (%.2f, %.2f, %.2f)\n", Dir.x, Dir.y, Dir.z);
OutputDebugStringA(DebugMsg);

sprintf_s(DebugMsg, "Start: (%.2f, %.2f) End: (%.2f, %.2f)\n",
    mInfo.Start.x, mInfo.Start.y, mInfo.End.x, mInfo.End.y);
OutputDebugStringA(DebugMsg);
```

**체크 포인트:**
- `mLineDir`이 단위 벡터인지 확인
- `mRotMatrix`가 올바른지 확인
- `TransformNormal` 결과가 예상과 맞는지 확인

### 3-3. 선 길이가 이상한 경우

**디버깅:**
```cpp
// mRenderScale 확인
char DebugMsg[256];
sprintf_s(DebugMsg, "RenderScale: (%.2f, %.2f, %.2f)\n",
    mRenderScale.x, mRenderScale.y, mRenderScale.z);
OutputDebugStringA(DebugMsg);

// mDistance 확인
sprintf_s(DebugMsg, "Distance: %.2f\n", mDistance);
OutputDebugStringA(DebugMsg);
```

**예상 결과:**
- `mRenderScale.y`는 `mDistance`와 같아야 함
- `mRenderScale.x`와 `z`는 1.f이어야 함

### 3-4. 색상이 안 변하는 경우

**문제: Collider 버퍼가 업데이트 안 됨**
```cpp
// 확인
if (!mColliderCBuffer)
{
    OutputDebugString(L"ColliderCBuffer가 null!\n");
}

// 충돌 상태 확인
char DebugMsg[256];
sprintf_s(DebugMsg, "Collision: %s\n", mCollision ? "TRUE" : "FALSE");
OutputDebugStringA(DebugMsg);
```

---

## 4. 학습 실습 과제

### 실습 1: 회전하는 선분
```cpp
// 선분이 계속 회전하도록 만들기
void CPlayer::Update(float DeltaTime)
{
    CGameObject::Update(DeltaTime);

    auto Line = mLine2D.lock();
    if (Line)
    {
        // Z축 기준으로 매 프레임 90도씩 회전
        Line->AddRelativeRotationZ(90.f * DeltaTime);
    }
}

// 예상 결과: 선분이 빙글빙글 회전
```

### 실습 2: 선분 길이 변화
```cpp
// 선분이 늘어났다 줄어들었다 반복
void CPlayer::Update(float DeltaTime)
{
    CGameObject::Update(DeltaTime);

    static float Time = 0.f;
    Time += DeltaTime;

    auto Line = mLine2D.lock();
    if (Line)
    {
        // 사인파로 길이 조절 (100 ~ 300 사이)
        float Distance = 200.f + sinf(Time * 2.f) * 100.f;
        Line->SetLineDistance(Distance);
    }
}

// 예상 결과: 선분이 주기적으로 길어졌다 짧아짐
```

### 실습 3: 마우스 방향으로 선분
```cpp
void CPlayer::Update(float DeltaTime)
{
    CGameObject::Update(DeltaTime);

    // 마우스 위치 가져오기
    POINT MousePos;
    GetCursorPos(&MousePos);
    ScreenToClient(GetActiveWindow(), &MousePos);

    // 월드 좌표로 변환 (구현 필요)
    FVector3 MouseWorldPos = ScreenToWorld(MousePos);

    // 플레이어 → 마우스 방향 계산
    FVector3 Dir = MouseWorldPos - GetWorldPos();
    Dir.Normalize();

    auto Line = mLine2D.lock();
    if (Line)
    {
        Line->SetLineDir(Dir.x, Dir.y, 0.f);
    }
}

// 예상 결과: 선분이 항상 마우스 커서를 가리킴
```

---

## 5. 요약

### 선분 계산 핵심
1. **시작점**: 오브젝트 월드 위치
2. **방향**: 로컬 방향 × 회전 행렬
3. **끝점**: 시작점 + 방향 × 거리
4. **AABB**: 최소/최대 점 계산

### 디버그 렌더링 핵심
1. **메시**: LineUP2D (Y축 길이 1)
2. **스케일**: Y축을 `mDistance`로 조절
3. **회전**: 오브젝트 회전 적용
4. **이동**: 월드 위치로 이동
5. **색상**: 충돌 시 빨강, 아니면 초록

### 디버깅 체크리스트
- [ ] DebugDraw가 true인지
- [ ] LineUP2D 메시가 로드되었는지
- [ ] Collider 셰이더가 있는지
- [ ] mDistance가 0이 아닌지
- [ ] 회전 행렬이 올바른지
- [ ] 색상 버퍼가 업데이트되는지

---
