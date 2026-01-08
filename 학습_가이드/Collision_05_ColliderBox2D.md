  4. 충돌체 구체 클래스 (ColliderBox2D)

  4-1. 헤더 파일 (ColliderBox2D.h)

  #pragma once
  #include "Collider.h"

  // ============================================
  // 2D 박스 충돌체 클래스
  // - AABB (Axis-Aligned Bounding Box): 회전 없을 때
  // - OBB (Oriented Bounding Box): 회전 있을 때
  // ============================================
  class CColliderBox2D : public CCollider
  {
      friend class CGameObject;
      friend class CObject;

  protected:
      CColliderBox2D();
      CColliderBox2D(const CColliderBox2D& ref);
      CColliderBox2D(CColliderBox2D&& ref) noexcept;

  public:
      virtual ~CColliderBox2D();

  protected:
      // 박스 충돌체의 정보
      // Center, Axis[2], HalfSize를 포함
      FBox2DInfo mInfo;

  public:
      // Getter: 박스 정보 얻기
      const FBox2DInfo& GetInfo() const
      {
          return mInfo;
      }

  public:
      // 박스 크기 설정 (벡터로)
      void SetBoxSize(const FVector2& Size)
      {
          // 전체 크기를 받아서 반 크기로 저장
          // 예: SetBoxSize(100, 100) → HalfSize = (50, 50)
          mInfo.HalfSize = Size / 2.f;
      }

      // 박스 크기 설정 (x, y로)
      void SetBoxSize(float x, float y)
      {
          mInfo.HalfSize = FVector2(x / 2.f, y / 2.f);
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
      virtual CColliderBox2D* Clone() const;

  public:
      // 충돌 검사 함수 (오버라이드)
      virtual bool Collision(FVector3& HitPoint,
          std::shared_ptr<CCollider> Dest);
  };

  4-2. 구현 파일 (ColliderBox2D.cpp)

  #include "ColliderBox2D.h"
  #include "../Asset/AssetManager.h"
  #include "../Asset/Shader/ShaderManager.h"
  #include "../Asset/Shader/Shader.h"
  #include "../Asset/Shader/CBufferCollider.h"
  #include "../Asset/Mesh/MeshManager.h"
  #include "../Asset/Mesh/Mesh.h"
  #include "../World/World.h"
  #include "../World/WorldAssetManager.h"
  #include "Collision.h"
  #include "ColliderSphere2D.h"
  #include "ColliderLine2D.h"

  CColliderBox2D::CColliderBox2D()
  {
      // 충돌체 타입 설정
      mColliderType = EColliderType::Box2D;
  }

  CColliderBox2D::CColliderBox2D(const CColliderBox2D& ref) :
      CCollider(ref)
  {
      mColliderType = EColliderType::Box2D;
  }

  CColliderBox2D::CColliderBox2D(CColliderBox2D&& ref) noexcept :
      CCollider(std::move(ref))
  {
      mColliderType = EColliderType::Box2D;
  }

  CColliderBox2D::~CColliderBox2D()
  {
  }

  // ============================================
  // 디버그 드로우 설정
  // ============================================
  void CColliderBox2D::SetDebugDraw(bool DebugDraw)
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

          // 메시 로드 (중심이 0,0인 사각형 프레임)
          auto World = mWorld.lock();

          if (World)
          {
              auto AssetMgr = World->GetWorldAssetManager().lock();
              mMesh = AssetMgr->FindMesh("CenterFrameRect");
          }
          else
          {
              std::weak_ptr<CMeshManager> Weak_MeshMgr =
                  CAssetManager::GetInst()->GetMeshManager();
              std::shared_ptr<CMeshManager> MeshMgr = Weak_MeshMgr.lock();
              mMesh = MeshMgr->FindMesh("CenterFrameRect");
          }

          // 충돌체 색상 버퍼 생성
          mColliderCBuffer.reset(new CCBufferCollider);
          mColliderCBuffer->Init();
      }
  }

  // ============================================
  // 초기화
  // ============================================
  bool CColliderBox2D::Init()
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
              mMesh = AssetMgr->FindMesh("Mesh_CenterFrameRect");
          }
          else
          {
              std::weak_ptr<CMeshManager> Weak_MeshMgr =
                  CAssetManager::GetInst()->GetMeshManager();
              std::shared_ptr<CMeshManager> MeshMgr = Weak_MeshMgr.lock();
              mMesh = MeshMgr->FindMesh("Mesh_CenterFrameRect");
          }

          mColliderCBuffer.reset(new CCBufferCollider);
          mColliderCBuffer->Init();
      }

      return true;
  }

  void CColliderBox2D::Update(float DeltaTime)
  {
      CCollider::Update(DeltaTime);
  }

  // ============================================
  // PostUpdate: 충돌체 정보 갱신
  // ============================================
  void CColliderBox2D::PostUpdate(float DeltaTime)
  {
      CCollider::PostUpdate(DeltaTime);

      // 박스 중심점 = 오브젝트의 월드 위치
      mInfo.Center = mWorldPos;

      // 박스의 로컬 축 = 오브젝트의 월드 축
      // 오브젝트가 회전하면 이 축도 함께 회전됨
      mInfo.Axis[EAxis::X] = mWorldAxis[EAxis::X];
      mInfo.Axis[EAxis::Y] = mWorldAxis[EAxis::Y];

      // ============================================
      // AABB 바운딩 박스 계산
      // OBB를 감싸는 최소/최대 점 계산
      // ============================================

      // 박스를 구성하는 4개의 꼭지점 계산
      FVector3 Pos[4];

      // 좌하단: Center - X축*반너비 - Y축*반높이
      Pos[0] = mInfo.Center - mInfo.Axis[EAxis::X] * mInfo.HalfSize.x -
          mInfo.Axis[EAxis::Y] * mInfo.HalfSize.y;

      // 좌상단: Center - X축*반너비 + Y축*반높이
      Pos[1] = mInfo.Center - mInfo.Axis[EAxis::X] * mInfo.HalfSize.x +
          mInfo.Axis[EAxis::Y] * mInfo.HalfSize.y;

      // 우하단: Center + X축*반너비 - Y축*반높이
      Pos[2] = mInfo.Center + mInfo.Axis[EAxis::X] * mInfo.HalfSize.x -
          mInfo.Axis[EAxis::Y] * mInfo.HalfSize.y;

      // 우상단: Center + X축*반너비 + Y축*반높이
      Pos[3] = mInfo.Center + mInfo.Axis[EAxis::X] * mInfo.HalfSize.x +
          mInfo.Axis[EAxis::Y] * mInfo.HalfSize.y;

      // 첫 번째 점을 초기 최소/최대로 설정
      mMin = Pos[0];
      mMax = Pos[0];

      // 나머지 점들과 비교하여 최소/최대 갱신
      for (int i = 1; i < 4; ++i)
      {
          // 최소값 갱신: 현재 최소값보다 점의 x가 작으면 교체
          mMin.x = mMin.x > Pos[i].x ? Pos[i].x : mMin.x;
          mMin.y = mMin.y > Pos[i].y ? Pos[i].y : mMin.y;

          // 최대값 갱신: 현재 최대값보다 점의 x가 크면 교체
          mMax.x = mMax.x < Pos[i].x ? Pos[i].x : mMax.x;
          mMax.y = mMax.y < Pos[i].y ? Pos[i].y : mMax.y;
      }

      // ============================================
      // 렌더링 크기 계산 (디버그 표시용)
      // ============================================
      mRenderScale.x = mWorldScale.x * mInfo.HalfSize.x * 2.f;  // 전체 너비
      mRenderScale.y = mWorldScale.y * mInfo.HalfSize.y * 2.f;  // 전체 높이
      mRenderScale.z = 1.f;
  }

  // ============================================
  // 복제 함수
  // ============================================
  CColliderBox2D* CColliderBox2D::Clone() const
  {
      return new CColliderBox2D(*this);
  }

  // ============================================
  // 충돌 검사 함수
  // ============================================
  bool CColliderBox2D::Collision(FVector3& HitPoint,
      std::shared_ptr<CCollider> Dest)
  {
      // 상대방의 충돌체 모양에 따라 충돌 알고리즘 선택
      switch (Dest->GetColliderType())
      {
      case EColliderType::Box2D:
          // Box vs Box 충돌
          // 둘 다 회전이 0이면 AABB, 하나라도 회전 있으면 OBB
          return CCollision::CollisionBox2DToBox2D(HitPoint, this,
              dynamic_cast<CColliderBox2D*>(Dest.get()));

      case EColliderType::Sphere2D:
          // Box vs Sphere 충돌
          return CCollision::CollisionBox2DToSphere2D(HitPoint,
              this, dynamic_cast<CColliderSphere2D*>(Dest.get()));

      case EColliderType::Line2D:
          // Box vs Line 충돌
          return CCollision::CollisionBox2DToLine2D(HitPoint,
              this, dynamic_cast<CColliderLine2D*>(Dest.get()));
      }

      return false;
  }

  ---
