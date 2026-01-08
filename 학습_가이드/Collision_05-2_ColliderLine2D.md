## 4-2. 충돌체 구체 클래스 (ColliderLine2D)

### 4-2-1. 헤더 파일 (ColliderLine2D.h)

```cpp
#pragma once
#include "Collider.h"

// ============================================
// 2D 선분 충돌체 클래스
// - 시작점과 끝점을 가진 선분
// - 방향과 거리로 선분 정의
// - 레이캐스트, 트리거 영역 등에 사용
// ============================================
class CColliderLine2D : public CCollider
{
    friend class CGameObject;
    friend class CObject;

protected:
    CColliderLine2D();
    CColliderLine2D(const CColliderLine2D& ref);
    CColliderLine2D(CColliderLine2D&& ref) noexcept;

public:
    virtual ~CColliderLine2D();

protected:
    // 선분 충돌체의 정보
    // Start, End를 포함
    FLine2DInfo mInfo;

    // 선분의 방향 벡터 (기본값: Y축 방향)
    // 이 방향으로 mDistance만큼 선분이 그려짐
    FVector3 mLineDir = FVector3::Axis[EAxis::Y];

    // 선분의 길이
    float mDistance = 100.f;

public:
    // Getter: 선분 정보 얻기
    const FLine2DInfo& GetInfo() const
    {
        return mInfo;
    }

public:
    // 선분 방향 설정 (벡터로)
    void SetLineDir(const FVector3& Dir)
    {
        mLineDir = Dir;
        mLineDir.Normalize();  // 단위 벡터로 정규화
    }

    // 선분 방향 설정 (x, y, z로)
    void SetLineDir(float x, float y, float z)
    {
        mLineDir = FVector3(x, y, z);
        mLineDir.Normalize();
    }

    // 선분 방향 설정 (x, y만, 2D)
    void SetLineDir(float x, float y)
    {
        mLineDir = FVector3(x, y, 0.f);
        mLineDir.Normalize();
    }

    // 선분 길이 설정
    void SetLineDistance(float Dist)
    {
        mDistance = Dist;
    }

public:
    // 디버그 드로우 설정 (오버라이드)
    virtual void SetDebugDraw(bool DebugDraw);

public:
    // 생명주기 함수들
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void PostUpdate(float DeltaTime);

protected:
    // 복제 함수
    virtual CColliderLine2D* Clone() const;

public:
    // 충돌 검사 함수 (오버라이드)
    virtual bool Collision(FVector3& HitPoint,
        std::shared_ptr<CCollider> Dest);
};
```

### 4-2-2. 구현 파일 (ColliderLine2D.cpp)

```cpp
#include "ColliderLine2D.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Shader/ShaderManager.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Shader/CBufferCollider.h"
#include "../Asset/Mesh/MeshManager.h"
#include "../Asset/Mesh/Mesh.h"
#include "../World/World.h"
#include "../World/WorldAssetManager.h"
#include "Collision.h"
#include "ColliderBox2D.h"
#include "ColliderSphere2D.h"

CColliderLine2D::CColliderLine2D()
{
    // 충돌체 타입 설정
    mColliderType = EColliderType::Line2D;
}

CColliderLine2D::CColliderLine2D(const CColliderLine2D& ref) :
    CCollider(ref)
{
    mColliderType = EColliderType::Line2D;
}

CColliderLine2D::CColliderLine2D(CColliderLine2D&& ref) noexcept :
    CCollider(std::move(ref))
{
    mColliderType = EColliderType::Line2D;
}

CColliderLine2D::~CColliderLine2D()
{
}

// ============================================
// 디버그 드로우 설정
// ============================================
void CColliderLine2D::SetDebugDraw(bool DebugDraw)
{
    // 부모 클래스의 SetDebugDraw 호출
    CCollider::SetDebugDraw(DebugDraw);

    // 디버그 드로우 활성화 시 셰이더와 메시 로드
    if (DebugDraw && mShader.expired())
    {
        // 셰이더 매니저에서 "Collider" 셰이더 찾기
        std::shared_ptr<CShaderManager> ShaderMgr =
            CAssetManager::GetInst()->GetShaderManager().lock();

        mShader = ShaderMgr->FindShader("Collider");

        // 메시 로드 (Y축 방향으로 그려지는 선분 메시)
        auto World = mWorld.lock();

        if (World)
        {
            auto AssetMgr = World->GetWorldAssetManager().lock();
            mMesh = AssetMgr->FindMesh("LineUP2D");
        }
        else
        {
            std::weak_ptr<CMeshManager> Weak_MeshMgr =
                CAssetManager::GetInst()->GetMeshManager();
            std::shared_ptr<CMeshManager> MeshMgr = Weak_MeshMgr.lock();
            mMesh = MeshMgr->FindMesh("Mesh_LineUP2D");
        }

        // 충돌체 색상 버퍼 생성
        mColliderCBuffer.reset(new CCBufferCollider);
        mColliderCBuffer->Init();
    }
}

// ============================================
// 초기화
// ============================================
bool CColliderLine2D::Init()
{
    // 부모 클래스 초기화
    CCollider::Init();

    // 디버그 드로우 활성화 시 리소스 로드
    if (mDebugDraw)
    {
        std::shared_ptr<CShaderManager> ShaderMgr =
            CAssetManager::GetInst()->GetShaderManager().lock();

        mShader = ShaderMgr->FindShader("Collider");

        auto World = mWorld.lock();

        if (World)
        {
            auto AssetMgr = World->GetWorldAssetManager().lock();
            mMesh = AssetMgr->FindMesh("LineUP2D");
        }
        else
        {
            std::weak_ptr<CMeshManager> Weak_MeshMgr =
                CAssetManager::GetInst()->GetMeshManager();
            std::shared_ptr<CMeshManager> MeshMgr = Weak_MeshMgr.lock();
            mMesh = MeshMgr->FindMesh("Mesh_LineUP2D");
        }

        mColliderCBuffer.reset(new CCBufferCollider);
        mColliderCBuffer->Init();
    }

    return true;
}

void CColliderLine2D::Update(float DeltaTime)
{
    CCollider::Update(DeltaTime);
}

// ============================================
// PostUpdate: 충돌체 정보 갱신
// ============================================
void CColliderLine2D::PostUpdate(float DeltaTime)
{
    CCollider::PostUpdate(DeltaTime);

    // ============================================
    // 선분의 시작점 = 오브젝트의 월드 위치
    // ============================================
    mInfo.Start = mWorldPos;

    // ============================================
    // 회전된 선분 방향 계산
    // ============================================
    // 로컬 방향(mLineDir)을 월드 회전 행렬로 변환
    FVector3 Dir;
    Dir = mLineDir.TransformNormal(mRotMatrix);
    Dir.Normalize();

    // ============================================
    // 선분의 끝점 = 시작점 + 방향 * 거리
    // ============================================
    mInfo.End = mInfo.Start + Dir * mDistance;

    // ============================================
    // AABB 바운딩 박스 계산
    // 선분을 감싸는 최소/최대 점 계산
    // ============================================
    mMin.x = mInfo.Start.x < mInfo.End.x ? mInfo.Start.x : mInfo.End.x;
    mMin.y = mInfo.Start.y < mInfo.End.y ? mInfo.Start.y : mInfo.End.y;

    mMax.x = mInfo.Start.x > mInfo.End.x ? mInfo.Start.x : mInfo.End.x;
    mMax.y = mInfo.Start.y > mInfo.End.y ? mInfo.Start.y : mInfo.End.y;

    // ============================================
    // 렌더링 크기 계산 (디버그 표시용)
    // ============================================
    // X 스케일은 1 (선의 두께)
    // Y 스케일은 선분의 길이
    mRenderScale.x = 1.f;
    mRenderScale.y = mDistance;
    mRenderScale.z = 1.f;
}

// ============================================
// 복제 함수
// ============================================
CColliderLine2D* CColliderLine2D::Clone() const
{
    return new CColliderLine2D(*this);
}

// ============================================
// 충돌 검사 함수
// ============================================
bool CColliderLine2D::Collision(FVector3& HitPoint,
    std::shared_ptr<CCollider> Dest)
{
    // 상대방의 충돌체 모양에 따라 충돌 알고리즘 선택
    switch (Dest->GetColliderType())
    {
    case EColliderType::Box2D:
        // Line vs Box 충돌
        // Box의 충돌 함수를 호출 (인자 순서만 반대)
        return CCollision::CollisionBox2DToLine2D(HitPoint,
            dynamic_cast<CColliderBox2D*>(Dest.get()), this);

    case EColliderType::Sphere2D:
        // Line vs Sphere 충돌
        return CCollision::CollisionSphere2DToLine2D(HitPoint,
            dynamic_cast<CColliderSphere2D*>(Dest.get()), this);

    case EColliderType::Line2D:
        // Line vs Line 충돌
        return CCollision::CollisionLine2DToLine2D(HitPoint,
            dynamic_cast<CColliderLine2D*>(Dest.get()), this);
    }

    return false;
}
```

---

### 주요 특징

1. **방향과 거리로 선분 정의**
   - `mLineDir`: 선분의 방향 (단위 벡터)
   - `mDistance`: 선분의 길이
   - `mInfo.Start`: 시작점 (오브젝트 위치)
   - `mInfo.End`: 끝점 (시작점 + 방향 * 거리)

2. **회전 지원**
   - `mLineDir`을 로컬 방향으로 저장
   - `PostUpdate`에서 회전 행렬(`mRotMatrix`)로 변환하여 월드 방향 계산
   - 오브젝트가 회전하면 선분도 함께 회전

3. **디버그 렌더링**
   - "LineUP2D" 메시 사용 (Y축 방향 선분)
   - `mRenderScale.y`에 거리를 설정하여 길이 표현
   - 충돌 시 빨간색, 미충돌 시 초록색으로 표시

4. **사용 예시**
   ```cpp
   // 플레이어에 Line Collider 추가
   mLine2D = CreateComponent<CColliderLine2D>("Line2D");
   auto Line = mLine2D.lock();

   // 위쪽 방향으로 200 길이의 선분
   Line->SetLineDir(0.f, 1.f);      // Y축 방향
   Line->SetLineDistance(200.f);    // 길이 200
   Line->SetDebugDraw(true);
   Line->SetCollisionProfile("Player");
   ```

---
