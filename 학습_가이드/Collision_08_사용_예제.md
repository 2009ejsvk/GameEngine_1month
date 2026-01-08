  7. 사용 예제

  7-1. 플레이어 캐릭터에서 충돌체 사용

  // ============================================
  // Player.h
  // ============================================
  class CPlayer : public CGameObject
  {
  private:
      // 충돌체 멤버 변수
      std::weak_ptr<class CColliderBox2D> mBody;        // 몸체 충돌체
      std::weak_ptr<class CColliderSphere2D> mSphere2D; // 원형 충돌체
      std::weak_ptr<class CColliderLine2D> mLine2D;     // 선분 충돌체

  public:
      virtual bool Init();

      // 충돌 콜백 함수 (사용자 정의)
      void OnCollisionBegin(const FVector3& HitPoint, CCollider* Dest);
      void OnCollisionEnd(CCollider* Dest);
  };

  // ============================================
  // Player.cpp - 초기화
  // ============================================
  bool CPlayer::Init()
  {
      CGameObject::Init();

      // ============================================
      // 1. Box Collider 생성 및 설정
      // ============================================

      // "Body"라는 이름으로 Box2D Collider 생성
      mBody = CreateComponent<CColliderBox2D>("Body");

      auto Body = mBody.lock();

      // 충돌 프로파일 설정 (Player 채널)
      Body->SetCollisionProfile("Player");

      // 박스 크기 설정 (100x100)
      Body->SetBoxSize(100.f, 100.f);

      // 디버그 표시 활성화 (초록/빨강 테두리)
      Body->SetDebugDraw(true);

      // 중심 오프셋 설정 (오브젝트 위 50만큼)
      Body->SetCenterOffset(0.f, 50.f, 0.f);

      // 충돌 콜백 함수 등록
      Body->SetCollisionBeginFunction(this, &CPlayer::OnCollisionBegin);
      Body->SetCollisionEndFunction(this, &CPlayer::OnCollisionEnd);

      // ============================================
      // 2. Sphere Collider 생성 및 설정
      // ============================================

      mSphere2D = CreateComponent<CColliderSphere2D>("Sphere2D");

      auto Sphere = mSphere2D.lock();

      // 반지름 설정 (대각선 길이의 절반)
      // sqrt(100² + 100²) / 2 = sqrt(20000) / 2
      Sphere->SetRadius(sqrtf(20000.f) * 0.5f);

      Sphere->SetCollisionProfile("Player");
      Sphere->SetDebugDraw(true);

      // ============================================
      // 3. Line Collider 생성 및 설정
      // ============================================

      mLine2D = CreateComponent<CColliderLine2D>("Line2D");

      auto Line = mLine2D.lock();

      // 선분 길이 설정
      Line->SetLineDistance(200.f);

      Line->SetCollisionProfile("Player");
      Line->SetDebugDraw(true);

      // 스케일 상속 비활성화
      Line->SetInheritScale(false);

      // 선분 위치 오프셋 (오브젝트 위 100만큼)
      Line->SetRelativePos(0.f, 100.f);

      return true;
  }

  // ============================================
  // 충돌 시작 콜백
  // ============================================
  void CPlayer::OnCollisionBegin(const FVector3& HitPoint, CCollider* Dest)
  {
      // HitPoint: 충돌 지점 (월드 좌표)
      // Dest: 충돌한 상대 Collider

      // 상대방이 어떤 프로파일인지 확인
      FCollisionProfile* DestProfile = Dest->GetCollisionProfile();

      if (DestProfile->Name == "Monster")
      {
          // 몬스터와 충돌!
          // 예: 데미지 받기, 넉백 등
          OutputDebugString(L"플레이어가 몬스터와 충돌!\n");
      }
      else if (DestProfile->Name == "Item")
      {
          // 아이템과 충돌!
          // 예: 아이템 획득
          OutputDebugString(L"플레이어가 아이템 획득!\n");
      }
  }

  // ============================================
  // 충돌 종료 콜백
  // ============================================
  void CPlayer::OnCollisionEnd(CCollider* Dest)
  {
      // Dest: 충돌이 끝난 상대 Collider

      FCollisionProfile* DestProfile = Dest->GetCollisionProfile();

      if (DestProfile->Name == "Monster")
      {
          OutputDebugString(L"몬스터와 떨어짐!\n");
      }
  }

  7-2. 몬스터 캐릭터에서 충돌체 사용

  // ============================================
  // Monster.h
  // ============================================
  class CMonster : public CGameObject
  {
  private:
      // 충돌체 멤버 변수
      std::weak_ptr<class CColliderSphere2D> mBody;     // Sphere 충돌체
      std::weak_ptr<class CColliderLine2D> mLine2D;     // 선분 충돌체

  public:
      virtual bool Init();
  };

  // ============================================
  // Monster.cpp - 초기화
  // ============================================
  bool CMonster::Init()
  {
      CGameObject::Init();

      // ============================================
      // 1. Sphere Collider 생성 및 설정
      // ============================================

      // "Body"라는 이름으로 Sphere2D Collider 생성
      mBody = CreateComponent<CColliderSphere2D>("Body");

      auto Body = mBody.lock();

      // 충돌 프로파일 설정 (Monster 채널)
      Body->SetCollisionProfile("Monster");

      // 반지름 설정 (대각선 길이의 절반)
      // sqrt(100² + 100²) / 2 = sqrt(20000) / 2
      Body->SetRadius(sqrtf(100.f * 100.f + 100.f * 100.f) * 0.5f);

      // 디버그 표시 활성화
      Body->SetDebugDraw(true);

      // 스케일 상속 비활성화
      Body->SetInheritScale(false);

      // ============================================
      // 2. Line Collider 생성 및 설정
      // ============================================

      mLine2D = CreateComponent<CColliderLine2D>("Line2D");

      auto Line = mLine2D.lock();

      // 충돌 프로파일 설정
      Line->SetCollisionProfile("Monster");

      // 선분 길이 설정
      Line->SetLineDistance(200.f);

      // 디버그 표시 활성화
      Line->SetDebugDraw(true);

      // 스케일 상속 비활성화
      Line->SetInheritScale(false);

      return true;
  }

  7-3. 총알 오브젝트에서 충돌체 사용

  // ============================================
  // Bullet.h
  // ============================================
  class CBullet : public CGameObject
  {
  private:
      std::weak_ptr<class CColliderBox2D> mBody;
      float mSpeed = 500.f;
      bool mIsActive = true;

  public:
      virtual bool Init();
      virtual void Update(float DeltaTime);

      void OnCollisionBegin(const FVector3& HitPoint, CCollider* Dest);
  };

  // ============================================
  // Bullet.cpp
  // ============================================
  bool CBullet::Init()
  {
      CGameObject::Init();

      // 작은 Box Collider 생성
      mBody = CreateComponent<CColliderBox2D>("Body");

      auto Body = mBody.lock();

      Body->SetCollisionProfile("Player");  // 플레이어 총알
      Body->SetBoxSize(10.f, 10.f);         // 작은 크기
      Body->SetDebugDraw(true);

      // 충돌 시작 콜백만 등록 (종료는 필요 없음)
      Body->SetCollisionBeginFunction(this, &CBullet::OnCollisionBegin);

      return true;
  }

  void CBullet::Update(float DeltaTime)
  {
      CGameObject::Update(DeltaTime);

      if (mIsActive)
      {
          // 앞으로 이동
          AddWorldPos(GetWorldAxis(EAxis::Y) * mSpeed * DeltaTime);
      }
  }

  void CBullet::OnCollisionBegin(const FVector3& HitPoint, CCollider* Dest)
  {
      FCollisionProfile* DestProfile = Dest->GetCollisionProfile();

      if (DestProfile->Name == "Monster")
      {
          // 몬스터에 맞음!
          // 총알 비활성화
          mIsActive = false;
          Destroy();  // 오브젝트 삭제

          // 몬스터에게 데미지 전달
          // CMonster* Monster = dynamic_cast<CMonster*>(Dest->GetOwner());
          // Monster->TakeDamage(10);
      }
      else if (DestProfile->Name == "Static")
      {
          // 벽에 맞음
          mIsActive = false;
          Destroy();
      }
  }

  7-4. 커스텀 충돌 프로파일 설정

  // ============================================
  // GameMode.cpp 또는 초기화 코드
  // ============================================

  // 1. 커스텀 채널 생성
  CCollisionInfoManager* CollisionMgr = CCollisionInfoManager::GetInst();

  CollisionMgr->CreateChannel("Item");      // 아이템 채널
  CollisionMgr->CreateChannel("Projectile"); // 투사체 채널

  // 2. 커스텀 프로파일 생성
  // 아이템 프로파일: 플레이어와만 충돌
  CollisionMgr->CreateProfile("Item", "Item", true, ECollisionInteraction::Ignore);
  CollisionMgr->SetProfileInteraction("Item", "Player", ECollisionInteraction::Collision);

  // 투사체 프로파일: 몬스터와만 충돌
  CollisionMgr->CreateProfile("Projectile", "Projectile", true, ECollisionInteraction::Ignore);
  CollisionMgr->SetProfileInteraction("Projectile", "Monster", ECollisionInteraction::Collision);

  // 3. Player 프로파일 수정
  // 플레이어는 Static, Monster, Item과 충돌
  CollisionMgr->SetProfileInteraction("Player", "Static", ECollisionInteraction::Collision);
  CollisionMgr->SetProfileInteraction("Player", "Monster", ECollisionInteraction::Collision);
  CollisionMgr->SetProfileInteraction("Player", "Item", ECollisionInteraction::Collision);
  CollisionMgr->SetProfileInteraction("Player", "Player", ECollisionInteraction::Ignore);

  // 4. Monster 프로파일 수정
  // 몬스터는 Static, Player, Projectile과 충돌
  CollisionMgr->SetProfileInteraction("Monster", "Static", ECollisionInteraction::Collision);
  CollisionMgr->SetProfileInteraction("Monster", "Player", ECollisionInteraction::Collision);
  CollisionMgr->SetProfileInteraction("Monster", "Projectile", ECollisionInteraction::Collision);
  CollisionMgr->SetProfileInteraction("Monster", "Monster", ECollisionInteraction::Ignore);

  7-5. 실시간 충돌 프로파일 변경

  // ============================================
  // 무적 상태 구현 예제
  // ============================================
  class CPlayer : public CGameObject
  {
  private:
      bool mIsInvincible = false;
      float mInvincibleTime = 0.f;

  public:
      void SetInvincible(float Duration)
      {
          mIsInvincible = true;
          mInvincibleTime = Duration;

          // 몬스터와의 충돌 무시
          auto Body = mBody.lock();
          if (Body)
          {
              FCollisionProfile* Profile = Body->GetCollisionProfile();
              Profile->Interaction[ECollisionChannel::Monster] = ECollisionInteraction::Ignore;
          }
      }

      void Update(float DeltaTime)
      {
          if (mIsInvincible)
          {
              mInvincibleTime -= DeltaTime;

              if (mInvincibleTime <= 0.f)
              {
                  // 무적 해제
                  mIsInvincible = false;

                  // 몬스터와의 충돌 복구
                  auto Body = mBody.lock();
                  if (Body)
                  {
                      FCollisionProfile* Profile = Body->GetCollisionProfile();
                      Profile->Interaction[ECollisionChannel::Monster] = ECollisionInteraction::Collision;
                  }
              }
          }
      }
  };

  ---
