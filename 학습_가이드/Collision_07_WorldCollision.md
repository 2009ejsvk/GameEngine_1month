  6. 월드 충돌 관리자 (WorldCollision)

  6-1. 헤더 파일 (WorldCollision.h)

  #pragma once
  #include "../EngineInfo.h"

  // ============================================
  // 월드 충돌 관리자
  // - 월드 내 모든 Collider 관리
  // - 매 프레임 모든 Collider 쌍 검사 (브루트포스)
  // - 충돌 프로파일 기반 필터링
  // - CollisionBegin/End 이벤트 발생
  // ============================================
  class CWorldCollision
  {
      friend class CWorld;

  private:
      // World에서만 생성 가능
      CWorldCollision();

  public:
      ~CWorldCollision();

  private:
      // 월드에 등록된 모든 충돌체 리스트
      // weak_ptr 사용: Collider가 삭제되면 자동으로 expired됨
      std::list<std::weak_ptr<class CCollider>> mColliderList;

      // 충돌 검사 간격 (초 단위)
      // 0이면 매 프레임 검사, 0보다 크면 해당 시간마다 검사
      // 예: 0.016f → 약 60fps로 충돌 검사
      float mInterval = 0.f;

      // 경과 시간 누적
      float mIntervalTime = 0.f;

  public:
      // 충돌 검사 간격 설정
      void SetInterval(float Interval)
      {
          mInterval = Interval;
      }

      // 새로운 Collider 등록
      void AddCollider(const std::weak_ptr<class CCollider>& Collider);

  public:
      // 초기화
      bool Init();

      // 매 프레임 충돌 검사
      void Update(float DeltaTime);
  };

  6-2. 구현 파일 (WorldCollision.cpp)

  #include "WorldCollision.h"
  #include "../Component/Collider.h"

  CWorldCollision::CWorldCollision()
  {
  }

  CWorldCollision::~CWorldCollision()
  {
  }

  // ============================================
  // Collider 등록
  // ============================================
  void CWorldCollision::AddCollider(
      const std::weak_ptr<class CCollider>& Collider)
  {
      // 리스트에 추가만 하면 됨
      mColliderList.push_back(Collider);
  }

  bool CWorldCollision::Init()
  {
      return true;
  }

  // ============================================
  // 매 프레임 충돌 검사 (핵심 함수!)
  // ============================================
  void CWorldCollision::Update(float DeltaTime)
  {
      // ============================================
      // 1. 충돌 검사 간격 확인
      // ============================================
      if (mInterval > 0.f)
      {
          mIntervalTime += DeltaTime;

          // 아직 간격 시간이 안됐으면 검사 건너뛰기
          if (mIntervalTime < mInterval)
              return;

          // 간격 시간 초과분 유지
          mIntervalTime -= mInterval;
      }

      // ============================================
      // 2. 이중 루프로 모든 Collider 쌍 검사 (O(n²))
      // ============================================

      auto iter = mColliderList.begin();
      auto iterEnd = mColliderList.end();
      --iterEnd;  // 마지막 요소 전까지 (외부 루프)

      auto iterEnd1 = mColliderList.end();  // 마지막 요소까지 (내부 루프)

      // 외부 루프: 첫 번째 Collider
      for (; iter != iterEnd;)
      {
          // ============================================
          // 2-1. 첫 번째 Collider 유효성 검사
          // ============================================

          // weak_ptr이 만료됨 → Collider가 삭제됨
          if ((*iter).expired())
          {
              iter = mColliderList.erase(iter);
              iterEnd = mColliderList.end();
              iterEnd1 = iterEnd;
              --iterEnd;
              continue;
          }

          auto SrcCollider = (*iter).lock();

          // Alive가 false → 삭제 예정
          if (!SrcCollider->GetAlive())
          {
              iter = mColliderList.erase(iter);
              iterEnd = mColliderList.end();
              iterEnd1 = iterEnd;
              --iterEnd;
              continue;
          }

          // Enable이 false → 비활성화됨 (검사 건너뛰기)
          else if (!SrcCollider->GetEnable())
          {
              ++iter;
              continue;
          }

          // ============================================
          // 2-2. 첫 번째 Collider의 프로파일 확인
          // ============================================

          FCollisionProfile* SrcProfile = SrcCollider->GetCollisionProfile();

          // 프로파일이 비활성화됨 (검사 건너뛰기)
          if (!SrcProfile->Enable)
          {
              ++iter;
              continue;
          }

          // ============================================
          // 2-3. 내부 루프: 두 번째 Collider
          // ============================================

          auto iter1 = iter;
          ++iter1;  // 자기 자신과는 검사 안함

          for (; iter1 != iterEnd1;)
          {
              // ============================================
              // 2-3-1. 두 번째 Collider 유효성 검사
              // ============================================

              if ((*iter1).expired())
              {
                  iter1 = mColliderList.erase(iter1);
                  iterEnd = mColliderList.end();
                  iterEnd1 = iterEnd;
                  --iterEnd;
                  continue;
              }

              auto DestCollider = (*iter1).lock();

              if (!DestCollider->GetAlive())
              {
                  iter1 = mColliderList.erase(iter1);
                  iterEnd = mColliderList.end();
                  iterEnd1 = iterEnd;
                  --iterEnd;
                  continue;
              }

              else if (!DestCollider->GetEnable())
              {
                  ++iter1;
                  continue;
              }

              // ============================================
              // 2-3-2. 두 번째 Collider의 프로파일 확인
              // ============================================

              FCollisionProfile* DestProfile = DestCollider->GetCollisionProfile();

              if (!DestProfile->Enable)
              {
                  ++iter1;
                  continue;
              }

              // ============================================
              // 2-4. 충돌 프로파일 필터링
              // ============================================

              // 상대방 채널에 대해 Ignore이면 검사 건너뛰기
              // 예: Src가 Player, Dest가 Monster
              //     Player 프로파일의 Interaction[Monster] == Ignore
              //     또는 Monster 프로파일의 Interaction[Player] == Ignore
              //     → 충돌 검사 안함
              if (SrcProfile->Interaction[DestProfile->Channel->Channel] == ECollisionInteraction::Ignore ||
                  DestProfile->Interaction[SrcProfile->Channel->Channel] == ECollisionInteraction::Ignore)
              {
                  ++iter1;
                  continue;
              }

              // ============================================
              // 2-5. 실제 충돌 검사 수행
              // ============================================

              FVector3 HitPoint;  // 충돌 지점 저장

              // Collision 함수 호출 (다형성 - 각 타입별 구현)
              // Box2D::Collision → CCollision::CollisionBox2DToBox2D 등
              if (SrcCollider->Collision(HitPoint, DestCollider))
              {
                  // ============================================
                  // 충돌 성공!
                  // ============================================

                  // 이전 프레임에도 충돌 중이었는지 확인
                  if (!SrcCollider->CheckCollisionObject(DestCollider.get()))
                  {
                      // 이번 프레임에 처음 충돌 → CollisionBegin
                      SrcCollider->CallCollisionBegin(HitPoint, *iter1);
                      DestCollider->CallCollisionBegin(HitPoint, *iter);
                  }
                  // else: 계속 충돌 중 → 아무것도 안함
              }
              else
              {
                  // ============================================
                  // 충돌 안함
                  // ============================================

                  // 이전 프레임에는 충돌 중이었는지 확인
                  if (SrcCollider->CheckCollisionObject(DestCollider.get()))
                  {
                      // 이번 프레임에 떨어짐 → CollisionEnd
                      SrcCollider->CallCollisionEnd(DestCollider.get());
                      DestCollider->CallCollisionEnd(SrcCollider.get());
                  }
              }

              ++iter1;
          }

          ++iter;
      }
  }

  ---
