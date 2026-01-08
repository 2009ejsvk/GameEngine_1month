  3. 충돌체 기본 클래스 (Collider)

  3-1. 헤더 파일 (Collider.h)

  #pragma once
  #include "SceneComponent.h"

  // ============================================
  // 모든 충돌체의 부모 클래스 (추상 클래스)
  // - Box2D, Sphere2D, Line2D의 공통 기능 정의
  // - 충돌 프로파일 관리
  // - 충돌 이벤트 콜백 관리
  // - 디버그 렌더링
  // ============================================
  class CCollider abstract : public CSceneComponent
  {
      friend class CGameObject;
      friend class CObject;

  protected:
      CCollider();
      CCollider(const CCollider& ref);
      CCollider(CCollider&& ref) noexcept;

  public:
      virtual ~CCollider();

  protected:
      // 충돌체 타입 (Box2D / Sphere2D / Line2D)
      EColliderType mColliderType;

      // AABB 바운딩 박스의 최소/최대 점
      // 충돌 검사 최적화에 사용 (브로드 페이즈)
      FVector3 mMin;
      FVector3 mMax;

      // 렌더링 크기 (디버그 표시용)
      FVector3 mRenderScale;

      // 디버그 드로우 활성화 여부
      bool mDebugDraw = false;

      // 현재 충돌 중인지 여부
      // true면 빨간색, false면 초록색으로 표시
      bool mCollision = false;

      // 이 충돌체의 충돌 프로파일
      FCollisionProfile* mProfile = nullptr;

      // 충돌체 중심 오프셋
      // 오브젝트 위치에서 얼마나 떨어진 곳에 충돌체를 배치할지
      // 예: mOffset = (0, 50, 0) → 오브젝트 위 50만큼에 충돌체 배치
      FVector3 mOffset;

      // 현재 충돌 중인 다른 충돌체들의 맵
      // key: 충돌체 포인터, value: weak_ptr로 충돌체 관리
      // 충돌이 끝나면 이 맵에서 제거
      std::unordered_map<CCollider*, std::weak_ptr<CCollider>> mCollisionObjectMap;

      // 디버그 렌더링용 셰이더와 메시
      std::weak_ptr<class CShader> mShader;
      std::weak_ptr<class CMesh> mMesh;

      // GPU에 전달할 변환 행렬 버퍼
      std::shared_ptr<class CCBufferTransform> mTransformCBuffer;

      // GPU에 전달할 충돌체 색상 버퍼
      std::shared_ptr<class CCBufferCollider> mColliderCBuffer;

      // 충돌 시작 시 호출될 콜백 함수
      // 매개변수: HitPoint(충돌 지점), Collider*(충돌한 상대)
      std::function<void(const FVector3&, CCollider*)> mCollisionBeginFunc;

      // 충돌 종료 시 호출될 콜백 함수
      // 매개변수: Collider*(충돌했던 상대)
      std::function<void(CCollider*)> mCollisionEndFunc;

  public:
      // Getter 함수들
      EColliderType GetColliderType() const
      {
          return mColliderType;
      }

      bool GetDebugDraw() const
      {
          return mDebugDraw;
      }

      FCollisionProfile* GetCollisionProfile() const
      {
          return mProfile;
      }

      // 특정 충돌체가 충돌 중인지 확인
      // 충돌 맵에 해당 충돌체가 있으면 true
      bool CheckCollisionObject(CCollider* Collider);

      // 충돌 맵에서 특정 충돌체 제거
      void EraseCollisionObject(CCollider* Collider);

  public:
      // 디버그 드로우 활성화/비활성화
      virtual void SetDebugDraw(bool DebugDraw);

      // 충돌 프로파일 설정 (이름으로)
      void SetCollisionProfile(const std::string& Name);

      // 충돌체 중심 오프셋 설정
      void SetCenterOffset(const FVector3& Offset)
      {
          mOffset = Offset;
      }

      void SetCenterOffset(float x, float y, float z)
      {
          mOffset = FVector3(x, y, z);
      }

  public:
      // 생명주기 함수들
      virtual bool Init();
      virtual void Update(float DeltaTime);
      virtual void PostUpdate(float DeltaTime);
      virtual void Render();

  protected:
      // 복제 함수 (순수 가상 함수 - 자식 클래스에서 구현)
      virtual CCollider* Clone() const = 0;

  public:
      // 충돌 시작 콜백 호출 (내부용)
      // HitPoint: 충돌 지점
      // Dest: 충돌한 상대 충돌체
      void CallCollisionBegin(const FVector3& HitPoint,
          const std::weak_ptr<CCollider>& Dest);

      // 충돌 종료 콜백 호출 (내부용)
      void CallCollisionEnd(CCollider* Dest);

  public:
      // 실제 충돌 검사 함수 (순수 가상 함수)
      // HitPoint: 충돌 지점을 저장할 변수 (출력)
      // Dest: 충돌 대상
      // 반환값: 충돌했으면 true, 아니면 false
      virtual bool Collision(FVector3& HitPoint,
          std::shared_ptr<CCollider> Dest) = 0;

  public:
      // 충돌 시작 콜백 함수 등록
      // 사용 예: SetCollisionBeginFunction(this, &CPlayer::OnCollisionBegin)
      template <typename T>
      void SetCollisionBeginFunction(T* Obj,
          void(T::* Func)(const FVector3&, CCollider*))
      {
          mCollisionBeginFunc = std::bind(Func, Obj,
              std::placeholders::_1, std::placeholders::_2);
      }

      // 충돌 종료 콜백 함수 등록
      template <typename T>
      void SetCollisionEndFunction(T* Obj,
          void(T::* Func)(CCollider*))
      {
          mCollisionEndFunc = std::bind(Func, Obj,
              std::placeholders::_1);
      }
  };

  3-2. 구현 파일 (Collider.cpp) - 주요 함수만

  #include "Collider.h"
  #include "../Asset/Shader/Shader.h"
  #include "../Asset/Shader/CBufferCollider.h"
  #include "../World/World.h"
  #include "../World/WorldCollision.h"
  #include "../CollisionInfoManager.h"

  CCollider::CCollider()
  {
  }

  CCollider::~CCollider()
  {
      // 소멸자: 충돌 중인 모든 상대방에게 알림
      auto iter = mCollisionObjectMap.begin();
      auto iterEnd = mCollisionObjectMap.end();

      for (; iter != iterEnd; ++iter)
      {
          // weak_ptr이 만료되지 않았으면
          if (iter->second.expired())
              continue;

          auto Dest = iter->second.lock();

          // 상대방의 충돌 맵에서 나를 제거
          Dest->EraseCollisionObject(this);
      }
  }

  // ============================================
  // 충돌 맵에 특정 충돌체가 있는지 확인
  // ============================================
  bool CCollider::CheckCollisionObject(CCollider* Collider)
  {
      auto iter = mCollisionObjectMap.find(Collider);

      // 맵에 없으면 false (충돌 중이 아님)
      if (iter == mCollisionObjectMap.end())
          return false;

      return true;  // 맵에 있으면 true (충돌 중)
  }

  // ============================================
  // 충돌 맵에서 특정 충돌체 제거
  // ============================================
  void CCollider::EraseCollisionObject(CCollider* Collider)
  {
      auto iter = mCollisionObjectMap.find(Collider);

      if (iter == mCollisionObjectMap.end())
          return;  // 없으면 그냥 종료

      // 맵에서 제거
      mCollisionObjectMap.erase(Collider);

      // 충돌 맵이 비었으면 충돌 상태 해제
      if (mCollisionObjectMap.empty())
          mCollision = false;
  }

  // ============================================
  // 디버그 드로우 설정
  // ============================================
  void CCollider::SetDebugDraw(bool DebugDraw)
  {
      mDebugDraw = DebugDraw;

      if (mDebugDraw)
      {
          // 렌더링 타입을 Render로 설정
          mRenderType = EComponentRender::Render;

          // 렌더 매니저에 등록
          auto self = std::dynamic_pointer_cast<CSceneComponent>(mSelf.lock());
          CRenderManager::GetInst()->AddRenderLayer(self);

          // Transform 버퍼 생성
          mTransformCBuffer.reset(new CCBufferTransform);
          mTransformCBuffer->Init();
      }
  }

  // ============================================
  // 충돌 프로파일 설정
  // ============================================
  void CCollider::SetCollisionProfile(const std::string& Name)
  {
      // CollisionInfoManager에서 프로파일 찾아서 설정
      mProfile = CCollisionInfoManager::GetInst()->FindProfile(Name);
  }

  // ============================================
  // 초기화
  // ============================================
  bool CCollider::Init()
  {
      CSceneComponent::Init();

      // 디버그 드로우 활성화 시 버퍼 생성
      if (mDebugDraw)
      {
          mTransformCBuffer.reset(new CCBufferTransform);
          mTransformCBuffer->Init();
      }

      // 기본 프로파일을 "Static"으로 설정
      mProfile = CCollisionInfoManager::GetInst()->FindProfile("Static");

      // WorldCollision에 자동 등록
      auto World = mWorld.lock();
      if (World)
      {
          auto CollisionMgr = World->GetCollision().lock();
          if (CollisionMgr)
          {
              // 이 충돌체를 월드 충돌 관리자에 추가
              CollisionMgr->AddCollider(
                  std::dynamic_pointer_cast<CCollider>(mSelf.lock())
              );
          }
      }

      return true;
  }

  // ============================================
  // 렌더링 (디버그 표시)
  // ============================================
  void CCollider::Render()
  {
      CSceneComponent::Render();

      if (mDebugDraw)
      {
          // 카메라 행렬 얻기
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

          // 월드 행렬 계산
          FMatrix ScaleMatrix, RotMatrix, TranslateMatrix, WorldMatrix;

          // 오프셋 적용한 렌더 위치
          FVector3 RenderPos = mWorldPos + mOffset;

          ScaleMatrix.Scaling(mRenderScale);
          RotMatrix.Rotation(mWorldRot);
          TranslateMatrix.Translation(RenderPos);

          WorldMatrix = ScaleMatrix * RotMatrix * TranslateMatrix;

          // Transform 버퍼 업데이트
          mTransformCBuffer->SetWorldMatrix(WorldMatrix);
          mTransformCBuffer->SetViewMatrix(ViewMat);
          mTransformCBuffer->SetProjMatrix(ProjMat);

          FVector3 PivotSize = mPivot * Mesh->GetMeshSize();
          mTransformCBuffer->SetPivotSize(PivotSize);
          mTransformCBuffer->UpdateBuffer();

          // 충돌 상태에 따라 색상 변경
          if (mCollision)
              mColliderCBuffer->SetColor(FVector4::Red);    // 충돌 중: 빨간색
          else
              mColliderCBuffer->SetColor(FVector4::Green);  // 미충돌: 초록색

          mColliderCBuffer->UpdateBuffer();

          // 셰이더와 메시로 렌더링
          auto Shader = mShader.lock();
          Shader->SetShader();
          Mesh->Render();
      }
  }

  // ============================================
  // 충돌 시작 콜백 호출
  // ============================================
  void CCollider::CallCollisionBegin(const FVector3& HitPoint,
      const std::weak_ptr<CCollider>& Dest)
  {
      auto _Dest = Dest.lock();

      // 충돌 맵에 추가
      mCollisionObjectMap.insert(std::make_pair(_Dest.get(), Dest));

      // 충돌 상태로 변경
      mCollision = true;

      // 사용자 정의 콜백 함수 호출
      if (mCollisionBeginFunc)
          mCollisionBeginFunc(HitPoint, _Dest.get());
  }

  // ============================================
  // 충돌 종료 콜백 호출
  // ============================================
  void CCollider::CallCollisionEnd(CCollider* Dest)
  {
      // 충돌 맵에서 제거
      mCollisionObjectMap.erase(Dest);

      // 충돌 맵이 비었으면 충돌 상태 해제
      if (mCollisionObjectMap.empty())
          mCollision = false;

      // 사용자 정의 콜백 함수 호출
      if (mCollisionEndFunc)
          mCollisionEndFunc(Dest);
  }

    ---
  이어서 ColliderBox2D, Collision 알고리즘, WorldCollision 부분을 계속 작성하겠습니다. 계속 진행할까요?

  > 계속 진행

● 계속 진행하겠습니다!

  ---
