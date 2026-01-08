## LineUP2D 메시 생성과 로드 과정 상세 분석

이 문서는 ColliderLine2D의 디버그 렌더링에 사용되는 **LineUP2D 메시**가 어떻게 생성되고, 어떻게 로드되는지를 학습 레벨에서 상세하게 설명합니다.

---

## 1. LineUP2D 메시란?

### 1-1. 기본 정보

**LineUP2D**는 ColliderLine2D의 디버그 렌더링을 위한 **선분 메시**입니다.

**특징:**
- **정점 개수:** 2개
- **시작점:** (0, 0, 0) - 원점
- **끝점:** (0, 1, 0) - Y축 위쪽 1 단위
- **방향:** Y축 방향 (위쪽)
- **길이:** 1.0 (단위 길이)
- **렌더링 방식:** LINE_STRIP (선분)

**시각화:**
```
Y
↑
|  ● (0, 1, 0) - 끝점
|  |
|  |  ← 길이 1.0
|  |
|  ● (0, 0, 0) - 시작점
└──────→ X
```

### 1-2. 왜 Y축 방향인가?

**이유:**
1. **직관적인 회전**: 2D 게임에서 "위"를 기준으로 회전하는 것이 자연스러움
2. **일관성**: 다른 컴포넌트들도 Y축을 "전방(Forward)" 방향으로 사용
3. **스케일 적용 용이**: Y 스케일만 조절하면 길이가 변함

**사용 예시:**
```cpp
// 길이 200인 선분을 만들고 싶다면
mRenderScale.y = 200.f;  // Y축 스케일만 조절

// 메시의 (0, 1, 0)이 (0, 200, 0)으로 확대됨
```

---

## 2. 메시 생성 과정

### 2-1. 엔진 초기화 흐름

```
CEngine::Init()
    ↓
CAssetManager::Init()
    ↓
CMeshManager::Init()
    ↓
CreateMesh("Mesh_LineUP2D", ...)
    ↓
GPU 버퍼 생성 및 등록
```

### 2-2. MeshManager.cpp에서의 생성 코드

**파일:** `E:\GameEngine\GameEngine\Asset\Mesh\MeshManager.cpp` (156-159줄)

```cpp
bool CMeshManager::Init()
{
    // ... 다른 메시 생성 코드 ...

    // ============================================
    // LineUP2D 메시 생성
    // ============================================

    // STEP 1: 정점 데이터 정의
    FVector3 LineUp[2] =
    {
        FVector3(0.f, 0.f, 0.f),  // 시작점
        FVector3(0.f, 1.f, 0.f)   // 끝점
    };

    // STEP 2: 메시 생성 및 등록
    if (!CreateMesh("Mesh_LineUP2D",                    // 메시 이름
                    LineUp,                             // 정점 데이터
                    sizeof(FVector3),                   // 정점 크기 (12바이트)
                    2,                                  // 정점 개수
                    D3D11_USAGE_IMMUTABLE,             // 사용 방식 (변경 불가)
                    D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP))// 렌더링 방식 (선분)
        return false;

    // ... 다른 메시 생성 코드 ...

    return true;
}
```

### 2-3. CreateMesh 함수 상세

```cpp
bool CMeshManager::CreateMesh(
    const std::string& Name,              // "Mesh_LineUP2D"
    void* VertexData,                     // LineUp 배열 주소
    int VertexSize,                       // sizeof(FVector3) = 12 바이트
    int VertexCount,                      // 2개
    D3D11_USAGE VertexUsage,             // D3D11_USAGE_IMMUTABLE
    D3D11_PRIMITIVE_TOPOLOGY Primitive,   // D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
    void* IndexData,                      // nullptr (인덱스 사용 안함)
    int IndexSize,                        // 0
    int IndexCount,                       // 0
    DXGI_FORMAT Fmt,                     // DXGI_FORMAT_UNKNOWN
    D3D11_USAGE IndexUsage)              // D3D11_USAGE_DEFAULT
{
    // ============================================
    // 1. 중복 검사
    // ============================================
    std::weak_ptr<CMesh> Check = FindMesh(Name);

    // 이미 존재하면 생성하지 않음
    if (!Check.expired())
        return true;

    // ============================================
    // 2. CMesh 객체 생성
    // ============================================
    std::shared_ptr<CMesh> Mesh;
    Mesh.reset(new CMesh);

    // ============================================
    // 3. GPU 버퍼 생성 (CMesh::CreateMesh 호출)
    // ============================================
    if (!Mesh->CreateMesh(VertexData, VertexSize, VertexCount, VertexUsage,
                         Primitive, IndexData, IndexSize, IndexCount, Fmt, IndexUsage))
    {
        return false;
    }

    // ============================================
    // 4. 메시 이름 설정
    // ============================================
    Mesh->SetName(Name);

    // ============================================
    // 5. MeshMap에 등록
    // ============================================
    mMeshMap.insert(std::make_pair(Name, Mesh));

    return true;
}
```

### 2-4. 정점 데이터 구조

**FVector3 구조체:**
```cpp
// Vector3.h
struct FVector3
{
    float x = 0.f;  // X 좌표 (4바이트)
    float y = 0.f;  // Y 좌표 (4바이트)
    float z = 0.f;  // Z 좌표 (4바이트)

    // 전체 크기: 12바이트
};
```

**LineUp 배열의 메모리 구조:**
```
메모리 주소:    내용:
[0x0000]       0.0f  (LineUp[0].x)
[0x0004]       0.0f  (LineUp[0].y)
[0x0008]       0.0f  (LineUp[0].z)
[0x000C]       0.0f  (LineUp[1].x)
[0x0010]       1.0f  (LineUp[1].y)  ← Y 좌표만 1.0
[0x0014]       0.0f  (LineUp[1].z)

총 크기: 24바이트 (12바이트 × 2개)
```

### 2-5. GPU 버퍼 생성

**CMesh::CreateMesh 내부:**
```cpp
bool CMesh::CreateMesh(void* VertexData, int VertexSize, int VertexCount,
                      D3D11_USAGE VertexUsage,
                      D3D11_PRIMITIVE_TOPOLOGY Primitive,
                      ...)
{
    // ============================================
    // 1. 정점 버퍼 생성
    // ============================================
    D3D11_BUFFER_DESC Desc = {};
    Desc.ByteWidth = VertexSize * VertexCount;  // 12 × 2 = 24 바이트
    Desc.Usage = VertexUsage;                    // D3D11_USAGE_IMMUTABLE
    Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA Data = {};
    Data.pSysMem = VertexData;  // LineUp 배열 주소

    // GPU에 버퍼 생성
    ID3D11Buffer* VertexBuffer = nullptr;
    if (FAILED(CDevice::GetInst()->GetDevice()->CreateBuffer(
        &Desc, &Data, &VertexBuffer)))
        return false;

    // ============================================
    // 2. 메시 정보 저장
    // ============================================
    FMeshContainer Container;
    Container.VertexBuffer = VertexBuffer;
    Container.VertexSize = VertexSize;      // 12
    Container.VertexCount = VertexCount;    // 2
    Container.Primitive = Primitive;        // LINE_STRIP

    mMeshContainerVec.push_back(Container);

    return true;
}
```

**D3D11_USAGE_IMMUTABLE의 의미:**
- **불변 버퍼**: 생성 후 수정 불가
- **최적화**: GPU가 읽기 전용으로 최적화 가능
- **사용 케이스**: 정적인 형태의 메시 (LineUP2D는 항상 같은 형태)

**D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP의 의미:**
- **선분 렌더링**: 정점들을 연결하여 선으로 그림
- **연결 방식**: 정점 0 → 정점 1 (직선)
- **대안:**
  - `LINE_LIST`: 2개씩 묶어서 독립적인 선분
  - `LINE_STRIP`: 연속된 선분 (LineUP2D는 2개뿐이라 동일)

---

## 3. 메시 저장 구조

### 3-1. MeshManager의 저장소

```cpp
// MeshManager.h
class CMeshManager
{
private:
    // 메시 저장 맵
    std::unordered_map<std::string, std::shared_ptr<class CMesh>> mMeshMap;
};
```

**저장 방식:**
```
mMeshMap:
┌─────────────────┬──────────────────┐
│ 키 (string)     │ 값 (shared_ptr) │
├─────────────────┼──────────────────┤
│ "Mesh_RectColor"│ → CMesh*         │
│ "Mesh_RectTex"  │ → CMesh*         │
│ "Mesh_LineUP2D" │ → CMesh*         │ ← 우리의 메시
│ ...             │ ...              │
└─────────────────┴──────────────────┘
```

### 3-2. FindMesh 함수

```cpp
std::weak_ptr<CMesh> CMeshManager::FindMesh(const std::string& Name)
{
    auto iter = mMeshMap.find(Name);

    // 못 찾으면 빈 weak_ptr 반환
    if (iter == mMeshMap.end())
        return std::weak_ptr<CMesh>();

    // 찾으면 shared_ptr → weak_ptr 변환하여 반환
    return iter->second;
}
```

**weak_ptr을 반환하는 이유:**
- **순환 참조 방지**: 메시를 사용하는 객체가 메시를 소유하지 않음
- **수명 관리**: MeshManager가 메시의 수명을 관리
- **안전성**: 메시가 삭제되었는지 `expired()`로 확인 가능

---

## 4. 메시 로드 과정

### 4-1. ColliderLine2D에서의 로드

**파일:** `E:\GameEngine\GameEngine\Component\ColliderLine2D.cpp`

```cpp
void CColliderLine2D::SetDebugDraw(bool DebugDraw)
{
    CCollider::SetDebugDraw(DebugDraw);

    if (DebugDraw && mShader.expired())
    {
        // ============================================
        // 1. 셰이더 로드
        // ============================================
        std::shared_ptr<CShaderManager> ShaderMgr =
            CAssetManager::GetInst()->GetShaderManager().lock();

        mShader = ShaderMgr->FindShader("Collider");

        // ============================================
        // 2. 메시 로드
        // ============================================
        auto World = mWorld.lock();

        if (World)
        {
            // 케이스 1: World가 있으면 WorldAssetManager에서 로드
            auto AssetMgr = World->GetWorldAssetManager().lock();
            mMesh = AssetMgr->FindMesh("LineUP2D");
        }
        else
        {
            // 케이스 2: World가 없으면 MeshManager에서 직접 로드
            std::weak_ptr<CMeshManager> Weak_MeshMgr =
                CAssetManager::GetInst()->GetMeshManager();
            std::shared_ptr<CMeshManager> MeshMgr = Weak_MeshMgr.lock();
            mMesh = MeshMgr->FindMesh("Mesh_LineUP2D");
        }

        // ============================================
        // 3. 색상 버퍼 생성
        // ============================================
        mColliderCBuffer.reset(new CCBufferCollider);
        mColliderCBuffer->Init();
    }
}
```

### 4-2. 두 가지 로드 경로

**경로 1: WorldAssetManager 사용**
```cpp
// World가 있을 때
auto AssetMgr = World->GetWorldAssetManager().lock();
mMesh = AssetMgr->FindMesh("LineUP2D");
```

**왜 "LineUP2D"인가?**
- WorldAssetManager는 "Mesh_" 접두사 없이 등록
- 내부적으로 MeshManager의 "Mesh_LineUP2D"를 참조

**경로 2: MeshManager 직접 사용**
```cpp
// World가 없을 때 (드물게 발생)
std::shared_ptr<CMeshManager> MeshMgr =
    CAssetManager::GetInst()->GetMeshManager().lock();
mMesh = MeshMgr->FindMesh("Mesh_LineUP2D");
```

**왜 "Mesh_LineUP2D"인가?**
- MeshManager의 실제 키 이름 사용

### 4-3. WorldAssetManager의 역할

**WorldAssetManager.cpp:**
```cpp
std::weak_ptr<CMesh> CWorldAssetManager::FindMesh(const std::string& Name)
{
    // 1. 월드 내 캐시에서 먼저 검색
    auto iter = mAssetMap.find(Name);

    if (iter != mAssetMap.end())
    {
        // 캐시에 있으면 바로 반환
        return std::static_pointer_cast<CMesh>(iter->second);
    }

    // 2. 캐시에 없으면 MeshManager에서 로드
    std::weak_ptr<CMeshManager> MeshMgr =
        CAssetManager::GetInst()->GetMeshManager();

    std::shared_ptr<CMeshManager> Mgr = MeshMgr.lock();

    if (!Mgr)
        return std::weak_ptr<CMesh>();

    // "LineUP2D" → "Mesh_LineUP2D"로 변환
    std::string FullName = "Mesh_" + Name;
    std::weak_ptr<CMesh> Mesh = Mgr->FindMesh(FullName);

    if (Mesh.expired())
        return std::weak_ptr<CMesh>();

    // 3. 월드 캐시에 저장
    mAssetMap.insert(std::make_pair(Name, Mesh.lock()));

    return Mesh;
}
```

**장점:**
- **월드별 관리**: 각 월드가 사용하는 메시를 추적
- **빠른 접근**: 두 번째 호출부터는 캐시에서 바로 반환
- **자동 정리**: 월드가 삭제되면 참조 카운트 감소

---

## 5. 메시 사용 흐름 전체 도식

```
[엔진 시작]
    ↓
CEngine::Init()
    ↓
CAssetManager::Init()
    ↓
CMeshManager::Init()
    ↓
CreateMesh("Mesh_LineUP2D", LineUp, ...)
    ↓
CMesh::CreateMesh()
    ↓
GPU에 정점 버퍼 생성
    ↓
mMeshMap["Mesh_LineUP2D"] = shared_ptr<CMesh>


[런타임 - Collider 생성]
    ↓
Line2D->SetDebugDraw(true)
    ↓
WorldAssetManager::FindMesh("LineUP2D")
    ↓
MeshManager::FindMesh("Mesh_LineUP2D")
    ↓
mMeshMap에서 찾아서 반환
    ↓
mMesh = weak_ptr<CMesh> (참조만 저장)


[렌더링]
    ↓
Collider::Render()
    ↓
auto Mesh = mMesh.lock()
    ↓
Shader->SetShader()
    ↓
Mesh->Render()
    ↓
GPU에서 정점 버퍼 읽어서 선분 그리기
```

---

## 6. 메모리 관리

### 6-1. shared_ptr / weak_ptr 사용

**MeshManager (소유자):**
```cpp
std::shared_ptr<CMesh> mesh;  // 강한 참조 (소유)
mMeshMap["Mesh_LineUP2D"] = mesh;
```

**ColliderLine2D (사용자):**
```cpp
std::weak_ptr<CMesh> mMesh;  // 약한 참조 (소유 안함)
```

**참조 카운팅:**
```
MeshManager의 shared_ptr: 참조 카운트 = 1
    ↓
WorldAssetManager도 참조: 참조 카운트 = 2
    ↓
ColliderLine2D의 weak_ptr: 참조 카운트 변화 없음
```

### 6-2. 수명 관리

**메시 생성:**
- CEngine::Init() → CMeshManager::Init()에서 생성
- 프로그램 종료까지 유지

**메시 삭제:**
- CEngine 소멸자 → CMeshManager 소멸자
- mMeshMap 해제 → shared_ptr 참조 카운트 0
- CMesh 소멸자 → GPU 버퍼 해제

---

## 7. 디버깅 팁

### 7-1. 메시 로드 실패 디버깅

```cpp
void CColliderLine2D::SetDebugDraw(bool DebugDraw)
{
    // ... 코드 ...

    if (World)
    {
        auto AssetMgr = World->GetWorldAssetManager().lock();

        // 디버깅: AssetManager가 null인지 확인
        if (!AssetMgr)
        {
            OutputDebugString(L"WorldAssetManager가 null!\n");
            return;
        }

        mMesh = AssetMgr->FindMesh("LineUP2D");

        // 디버깅: 메시 로드 실패 확인
        if (mMesh.expired())
        {
            OutputDebugString(L"LineUP2D 메시 로드 실패!\n");

            // MeshManager에서 직접 시도
            auto MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();
            if (MeshMgr)
            {
                mMesh = MeshMgr->FindMesh("Mesh_LineUP2D");

                if (!mMesh.expired())
                    OutputDebugString(L"MeshManager에서 직접 로드 성공!\n");
            }
        }
    }
}
```

### 7-2. 메시 존재 확인

```cpp
// MeshManager에 등록된 모든 메시 출력
void DebugPrintMeshList()
{
    auto MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();

    if (!MeshMgr)
        return;

    // MeshManager의 mMeshMap을 순회 (구현 필요)
    // 예상 출력:
    // "Mesh_RectColor"
    // "Mesh_RectTex"
    // "Mesh_LineUP2D"  ← 여기 있어야 함
    // ...
}
```

### 7-3. GPU 버퍼 확인

```cpp
// Render에서 확인
void CCollider::Render()
{
    if (mDebugDraw)
    {
        auto Mesh = mMesh.lock();

        // 메시가 유효한지 확인
        if (!Mesh)
        {
            OutputDebugString(L"메시가 만료됨 (expired)!\n");
            return;
        }

        // 메시 정보 출력
        char DebugMsg[256];
        sprintf_s(DebugMsg, "Mesh VertexCount: %d\n",
                  Mesh->GetVertexCount());
        OutputDebugStringA(DebugMsg);

        // 정상적이면 VertexCount = 2

        // ... 렌더링 코드 ...
    }
}
```

---

## 8. 요약

### 생성 (엔진 초기화)
1. `CMeshManager::Init()`에서 정점 데이터 정의
2. `CreateMesh()`로 GPU 버퍼 생성
3. `mMeshMap`에 "Mesh_LineUP2D" 키로 등록

### 로드 (Collider 생성 시)
1. `SetDebugDraw(true)` 호출
2. `WorldAssetManager` 또는 `MeshManager`에서 검색
3. `weak_ptr`로 메시 참조 저장

### 사용 (렌더링)
1. `Render()`에서 `mMesh.lock()`으로 메시 얻기
2. 변환 행렬 적용 (스케일, 회전, 이동)
3. `Mesh->Render()`로 GPU에 그리기 명령

### 삭제 (엔진 종료)
1. `CMeshManager` 소멸자
2. `mMeshMap` 해제
3. GPU 버퍼 해제

---
