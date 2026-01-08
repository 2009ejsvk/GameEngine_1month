> 현재 프로젝트는 게임 엔진 개발을 공부하기 위한 학습용 레벨의 프로젝트입니다. 이 관점에서 프로젝트 내의 충돌 처리(Collision) 시스템의 전체적인 구조를
 분석해 주세요. '실제 코드(Syntax)'와 '구현 로직'을 중심으로 분석해 주길 원합니다. 설명할 때는 제가 이 코드를 보고 직접 따라 칠 수 있을 정도로
상세하게 주석을 달아주세요.

● 게임 엔진 프로젝트의 충돌 처리 시스템을 분석해드리겠습니다. 먼저 프로젝트 구조와 충돌 관련 파일들을 탐색하겠습니다.

● Explore(Explore collision system structure)
  ⎿  Done (38 tool uses · 48.3k tokens · 6m 0s)

● 완벽합니다! 이제 각 파일의 실제 코드를 읽고 상세한 주석을 달아 분석해드리겠습니다. 중요한 파일들을 순서대로 읽어보겠습니다.

● Read(Engine\Include\EngineInfo.h)
  ⎿  Read 103 lines

● Read(Engine\Include\Component\Collider.h)
  ⎿  Read 113 lines

● Read(GameEngine\Component\Collider.cpp)
  ⎿  Read 211 lines

● Read(Engine\Include\Component\ColliderBox2D.h)
  ⎿  Read 55 lines

● Read(GameEngine\Component\ColliderBox2D.cpp)
  ⎿  Read 181 lines

● Read(Engine\Include\Component\Collision.h)
  ⎿  Read 35 lines

● Read(GameEngine\Component\Collision.cpp)
  ⎿  Read 245 lines

● Read(Engine\Include\World\WorldCollision.h)
  ⎿  Read 32 lines

● Read(GameEngine\World\WorldCollision.cpp)
  ⎿  Read 162 lines

● Read(Engine\Include\CollisionInfoManager.h)
  ⎿  Read 58 lines

● Read(GameEngine\CollisionInfoManager.cpp)
  ⎿  Read 196 lines

● 완벽합니다! 이제 충돌 처리 시스템의 전체 구조를 상세하게 분석해드리겠습니다. 코드를 직접 따라 칠 수 있도록 매우 상세한 주석을 달아 설명하겠습니다.

  ---
  🎮 게임 엔진 충돌 처리(Collision) 시스템 완전 분석

  📋 목차

  1. #시스템-개요
  2. #1-데이터-구조-engineinfoh
  3. #2-충돌-프로파일-관리자-collisioninfomanager
  4. #3-충돌체-기본-클래스-collider
  5. #4-충돌체-구체-클래스-colliderbox2d
  6. #5-충돌-알고리즘-collision
  7. #6-월드-충돌-관리자-worldcollision
  8. #7-사용-예제

  ---
  시스템 개요

  이 충돌 시스템은 언리얼 엔진의 충돌 시스템을 모방한 구조입니다:
  - 채널(Channel) 기반 충돌 분류
  - 프로파일(Profile) 시스템으로 복잡한 충돌 관계 관리
  - 다형성을 활용한 여러 충돌체 타입 지원
  - 콜백 함수를 통한 이벤트 처리

  동작 흐름:
  1. Collider 생성 → WorldCollision에 자동 등록
  2. 매 프레임 WorldCollision::Update() 실행
  3. 모든 Collider 쌍을 브루트포스(O(n²))로 검사
  4. 프로파일 설정에 따라 충돌 필터링
  5. 충돌 알고리즘 실행 → HitPoint 계산
  6. CollisionBegin/End 콜백 호출

  ---
  1. 데이터 구조 (EngineInfo.h)

  // ============================================
  // 1-1. 충돌체 타입 열거형
  // ============================================
  enum class EColliderType
  {
      Box2D,      // 2D 박스 충돌체 (사각형)
      Sphere2D,   // 2D 원형 충돌체
      Line2D      // 2D 선분 충돌체 (현재 미구현)
  };

  // ============================================
  // 1-2. 박스 충돌체 정보 구조체
  // ============================================
  struct FBox2DInfo
  {
      FVector3 Center;        // 박스의 중심점 (월드 좌표)

      // 박스의 로컬 축 (회전 정보 포함)
      // Axis[0] = X축 방향 벡터 (기본: 1,0,0)
      // Axis[1] = Y축 방향 벡터 (기본: 0,1,0)
      FVector3 Axis[2] = {
          FVector3::Axis[EAxis::X],   // (1, 0, 0)
          FVector3::Axis[EAxis::Y]    // (0, 1, 0)
      };

      // 박스의 반 크기 (중심에서 각 축 방향으로의 거리)
      // 실제 박스 크기 = HalfSize * 2
      FVector2 HalfSize = FVector2(1.f, 1.f);
  };

  // ============================================
  // 1-3. 충돌 채널 열거형
  // 충돌 객체를 분류하는 카테고리
  // ============================================
  namespace ECollisionChannel
  {
      enum Type
      {
          Static,     // 정적 오브젝트 (벽, 바닥 등)
          Player,     // 플레이어
          Monster,    // 몬스터
          Custom1,    // 사용자 정의 채널 1
          Custom2,    // 사용자 정의 채널 2
          Custom3,    // ...
          Custom4,
          Custom5,
          Custom6,
          Custom7,
          Custom8,
          Custom9,
          Custom10,
          End         // 채널 개수 계산용
      };
  };

  // ============================================
  // 1-4. 충돌 채널 구조체
  // 이름과 타입을 매핑
  // ============================================
  struct FCollisionChannel
  {
      std::string Name;                           // 채널 이름 (예: "Player")
      ECollisionChannel::Type Channel = ECollisionChannel::Static;  // 채널 타입
  };

  // ============================================
  // 1-5. 충돌 상호작용 타입
  // 두 객체가 만났을 때 어떻게 처리할지
  // ============================================
  namespace ECollisionInteraction
  {
      enum Type
      {
          Ignore,     // 무시 (충돌 검사하지 않음)
          Collision,  // 충돌 (충돌 이벤트 발생)
          End
      };
  }

  // ============================================
  // 1-6. 충돌 프로파일 구조체
  // 특정 채널의 충돌 규칙을 정의
  // ============================================
  /*
  예시:
  Player 프로파일:
  - Channel: Player
  - Enable: true
  - Interaction[Static] = Collision    → Static과 충돌
  - Interaction[Player] = Ignore       → 다른 Player와 충돌 안함
  - Interaction[Monster] = Collision   → Monster와 충돌
  */
  struct FCollisionProfile
  {
      std::string Name;                   // 프로파일 이름 (예: "Player")
      FCollisionChannel* Channel;         // 이 프로파일이 속한 채널
      bool Enable = true;                 // 충돌 활성화 여부

      // 각 채널과의 상호작용 설정
      // Interaction[ECollisionChannel::Monster] = Collision
      // → Monster 채널과 충돌한다
      ECollisionInteraction::Type Interaction[ECollisionChannel::End] = {};
  };

  // ============================================
  // 1-7. 원형 충돌체 정보 구조체
  // ============================================
  struct FSphere2DInfo
  {
      FVector3 Center;    // 원의 중심점 (월드 좌표)
      float Radius = 0.f; // 원의 반지름
  };

  // ============================================
  // 1-8. 선분 충돌체 정보 구조체
  // ============================================
  struct FLine2DInfo
  {
      FVector3 Start;     // 선분의 시작점
      FVector3 End;       // 선분의 끝점
  };

  ---
  2. 충돌 프로파일 관리자 (CollisionInfoManager)

  2-1. 헤더 파일 (CollisionInfoManager.h)

  #pragma once
  #include "EngineInfo.h"

  // ============================================
  // 충돌 프로파일 관리자 (싱글톤)
  // - 채널(Channel) 생성 및 관리
  // - 프로파일(Profile) 생성 및 관리
  // - 충돌 규칙 설정
  // ============================================
  class CCollisionInfoManager
  {
  private:
      // 싱글톤 패턴: 생성자를 private으로
      CCollisionInfoManager();
      ~CCollisionInfoManager();

  private:
      // 채널 이름으로 검색하는 맵
      // key: 채널 이름(string), value: 채널 정보(FCollisionChannel*)
      std::unordered_map<std::string, FCollisionChannel*> mChannelMap;

      // 다음에 생성할 채널의 타입
      // Static부터 시작해서 하나씩 증가
      ECollisionChannel::Type mCreateChannel = ECollisionChannel::Static;

      // 프로파일 이름으로 검색하는 맵
      // key: 프로파일 이름(string), value: 프로파일 정보(FCollisionProfile*)
      std::unordered_map<std::string, FCollisionProfile*> mProfileMap;

  public:
      // 기본 채널과 프로파일 초기화
      bool Init();

      // 새로운 채널 생성 (Custom1~Custom10 사용)
      void CreateChannel(const std::string& Name);

      // 새로운 프로파일 생성 (채널 이름으로)
      // Name: 프로파일 이름
      // ChannelName: 속할 채널 이름
      // Enable: 충돌 활성화 여부
      // DefaultInteraction: 모든 채널에 대한 기본 상호작용
      bool CreateProfile(const std::string& Name,
          const std::string& ChannelName, bool Enable,
          ECollisionInteraction::Type DefaultInteraction =
          ECollisionInteraction::Collision);

      // 새로운 프로파일 생성 (채널 타입으로)
      bool CreateProfile(const std::string& Name,
          ECollisionChannel::Type Channel, bool Enable,
          ECollisionInteraction::Type DefaultInteraction =
          ECollisionInteraction::Collision);

      // 프로파일의 특정 채널과의 상호작용 설정 (채널 이름으로)
      // 예: SetProfileInteraction("Player", "Monster", Collision)
      // → Player 프로파일이 Monster 채널과 충돌하도록 설정
      bool SetProfileInteraction(const std::string& Name,
          const std::string& ChannelName,
          ECollisionInteraction::Type Interaction);

      // 프로파일의 특정 채널과의 상호작용 설정 (채널 타입으로)
      bool SetProfileInteraction(const std::string& Name,
          ECollisionChannel::Type Channel,
          ECollisionInteraction::Type Interaction);

      // 프로파일 활성화/비활성화
      bool SetProfileEnable(const std::string& Name, bool Enable);

      // 프로파일 검색
      FCollisionProfile* FindProfile(const std::string& Name);

  private:
      // 채널 검색 (내부용)
      FCollisionChannel* FindChannel(const std::string& Name);

  private:
      // 싱글톤 인스턴스
      static CCollisionInfoManager* mInst;

  public:
      // 싱글톤 인스턴스 얻기
      static CCollisionInfoManager* GetInst()
      {
          if (!mInst)
              mInst = new CCollisionInfoManager;
          return mInst;
      }

      // 싱글톤 인스턴스 파괴
      static void DestroyInst()
      {
          SAFE_DELETE(mInst);
      }
  };

  2-2. 구현 파일 (CollisionInfoManager.cpp)

  #include "CollisionInfoManager.h"

  // 싱글톤 인스턴스 초기화
  CCollisionInfoManager* CCollisionInfoManager::mInst = nullptr;

  CCollisionInfoManager::CCollisionInfoManager()
  {
  }

  CCollisionInfoManager::~CCollisionInfoManager()
  {
      // 모든 프로파일 메모리 해제
      auto iter = mProfileMap.begin();
      auto iterEnd = mProfileMap.end();
      for (; iter != iterEnd; ++iter)
      {
          SAFE_DELETE(iter->second);
      }

      // 모든 채널 메모리 해제
      auto iter1 = mChannelMap.begin();
      auto iter1End = mChannelMap.end();
      for (; iter1 != iter1End; ++iter1)
      {
          SAFE_DELETE(iter1->second);
      }
  }

  // ============================================
  // 초기화: 기본 채널과 프로파일 생성
  // ============================================
  bool CCollisionInfoManager::Init()
  {
      // 기본 채널 3개 생성
      CreateChannel("Static");    // 정적 오브젝트
      CreateChannel("Player");    // 플레이어
      CreateChannel("Monster");   // 몬스터

      // 기본 프로파일 3개 생성
      // 기본적으로 모든 채널과 충돌하도록 설정됨
      CreateProfile("Static", "Static", true);
      CreateProfile("Player", "Player", true);
      CreateProfile("Monster", "Monster", true);

      return true;
  }

  // ============================================
  // 새로운 채널 생성
  // ============================================
  void CCollisionInfoManager::CreateChannel(const std::string& Name)
  {
      // 더 이상 생성할 수 있는 채널이 없으면 종료
      if (mCreateChannel == ECollisionChannel::End)
          return;

      // 이미 존재하는 채널이면 종료
      FCollisionChannel* Channel = FindChannel(Name);
      if (Channel)
          return;

      // 새 채널 생성
      Channel = new FCollisionChannel;
      Channel->Name = Name;
      Channel->Channel = mCreateChannel;

      // 다음 채널 타입으로 증가 (Static → Player → Monster → Custom1 ...)
      mCreateChannel = (ECollisionChannel::Type)(mCreateChannel + 1);

      // 맵에 등록
      mChannelMap.insert(std::make_pair(Name, Channel));
  }

  // ============================================
  // 새로운 프로파일 생성 (채널 이름으로)
  // ============================================
  bool CCollisionInfoManager::CreateProfile(const std::string& Name,
      const std::string& ChannelName, bool Enable,
      ECollisionInteraction::Type DefaultInteraction)
  {
      // 이미 존재하는 프로파일이면 성공 반환
      FCollisionProfile* Profile = FindProfile(Name);
      if (Profile)
          return true;

      // 채널 찾기
      FCollisionChannel* Channel = FindChannel(ChannelName);
      if (!Channel)
          return false;  // 채널이 없으면 실패

      // 새 프로파일 생성
      Profile = new FCollisionProfile;
      Profile->Name = Name;
      Profile->Channel = Channel;
      Profile->Enable = Enable;

      // 모든 채널에 대해 기본 상호작용 설정
      // 예: DefaultInteraction이 Collision이면
      // 모든 채널과 충돌하도록 초기화
      for (int i = 0; i < ECollisionChannel::End; ++i)
      {
          Profile->Interaction[i] = DefaultInteraction;
      }

      // 맵에 등록
      mProfileMap.insert(std::make_pair(Name, Profile));

      return true;
  }

  // ============================================
  // 새로운 프로파일 생성 (채널 타입으로)
  // ============================================
  bool CCollisionInfoManager::CreateProfile(const std::string& Name,
      ECollisionChannel::Type Channel, bool Enable,
      ECollisionInteraction::Type DefaultInteraction)
  {
      // 이미 존재하는 프로파일이면 성공 반환
      FCollisionProfile* Profile = FindProfile(Name);
      if (Profile)
          return true;

      // 채널 타입으로 채널 객체 찾기
      FCollisionChannel* ChannelObject = nullptr;
      auto iter = mChannelMap.begin();
      auto iterEnd = mChannelMap.end();
      for (; iter != iterEnd; ++iter)
      {
          if (iter->second->Channel == Channel)
          {
              ChannelObject = iter->second;
              break;
          }
      }

      if (!ChannelObject)
          return false;  // 채널이 없으면 실패

      // 새 프로파일 생성 (위와 동일)
      Profile = new FCollisionProfile;
      Profile->Name = Name;
      Profile->Channel = ChannelObject;
      Profile->Enable = Enable;

      for (int i = 0; i < ECollisionChannel::End; ++i)
      {
          Profile->Interaction[i] = DefaultInteraction;
      }

      mProfileMap.insert(std::make_pair(Name, Profile));

      return true;
  }

  // ============================================
  // 프로파일의 특정 채널과의 상호작용 설정
  // ============================================
  bool CCollisionInfoManager::SetProfileInteraction(const std::string& Name,
      const std::string& ChannelName, ECollisionInteraction::Type Interaction)
  {
      // 프로파일 찾기
      FCollisionProfile* Profile = FindProfile(Name);
      if (!Profile)
          return false;

      // 채널 찾기
      FCollisionChannel* Channel = FindChannel(ChannelName);
      if (!Channel)
          return false;

      // 상호작용 설정
      // 예: Profile->Interaction[Monster] = Ignore
      // → 이 프로파일은 Monster 채널과 충돌하지 않음
      Profile->Interaction[Channel->Channel] = Interaction;

      return true;
  }

  bool CCollisionInfoManager::SetProfileInteraction(const std::string& Name,
      ECollisionChannel::Type Channel, ECollisionInteraction::Type Interaction)
  {
      FCollisionProfile* Profile = FindProfile(Name);
      if (!Profile)
          return false;

      Profile->Interaction[Channel] = Interaction;

      return true;
  }

  // ============================================
  // 프로파일 활성화/비활성화
  // ============================================
  bool CCollisionInfoManager::SetProfileEnable(const std::string& Name,
      bool Enable)
  {
      FCollisionProfile* Profile = FindProfile(Name);
      if (!Profile)
          return false;

      Profile->Enable = Enable;

      return true;
  }

  // ============================================
  // 프로파일 검색
  // ============================================
  FCollisionProfile* CCollisionInfoManager::FindProfile(const std::string& Name)
  {
      auto iter = mProfileMap.find(Name);

      if (iter == mProfileMap.end())
          return nullptr;

      return iter->second;
  }

  // ============================================
  // 채널 검색
  // ============================================
  FCollisionChannel* CCollisionInfoManager::FindChannel(const std::string& Name)
  {
      auto iter = mChannelMap.find(Name);

      if (iter == mChannelMap.end())
          return nullptr;

      return iter->second;
  }

  ---
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
          // Box vs Line 충돌 (현재 미구현)
          break;
      }

      return false;
  }

  ---
  5. 충돌 알고리즘 (Collision)

  5-1. 헤더 파일 (Collision.h)

  #pragma once
  #include "ColliderBox2D.h"
  #include "ColliderSphere2D.h"

  // ============================================
  // 충돌 알고리즘 정적 클래스
  // - 각 충돌체 타입별 충돌 검사 함수 제공
  // - 분리축 정리(SAT) 알고리즘 구현
  // ============================================
  class CCollision
  {
  public:
      // ============================================
      // Box vs Box 충돌 검사
      // ============================================

      // Collider 객체로 검사
      static bool CollisionBox2DToBox2D(FVector3& HitPoint,
          CColliderBox2D* Src, CColliderBox2D* Dest);

      // AABB vs AABB 충돌 검사 (회전 없음)
      static bool CollisionAABB2DToAABB2D(FVector3& HitPoint,
          const FBox2DInfo& Src, const FBox2DInfo& Dest);

      // OBB vs OBB 충돌 검사 (회전 있음)
      static bool CollisionOBB2DToOBB2D(FVector3& HitPoint,
          const FBox2DInfo& Src, const FBox2DInfo& Dest);

  public:
      // ============================================
      // Sphere vs Sphere 충돌 검사
      // ============================================

      // Collider 객체로 검사
      static bool CollisionSphere2DToSphere2D(FVector3& HitPoint,
          CColliderSphere2D* Src, CColliderSphere2D* Dest);

      // 구조체로 검사
      static bool CollisionSphere2DToSphere2D(FVector3& HitPoint,
          const FSphere2DInfo& Src, const FSphere2DInfo& Dest);

  public:
      // ============================================
      // Box vs Sphere 충돌 검사
      // ============================================

      // Collider 객체로 검사
      static bool CollisionBox2DToSphere2D(FVector3& HitPoint,
          CColliderBox2D* Src, CColliderSphere2D* Dest);

      // 구조체로 검사
      static bool CollisionBox2DToSphere2D(FVector3& HitPoint,
          const FBox2DInfo& Box, const FSphere2DInfo& Sphere);

  private:
      // ============================================
      // 분리축 투영 (Separating Axis Projection)
      // OBB 충돌 검사에 사용
      // ============================================
      // CenterLine: 두 도형 중심을 연결하는 벡터
      // Axis: 분리축 후보 (투영할 축)
      // SrcHalfSize: 첫 번째 도형의 해당 축 방향 반 크기
      // DestAxis: 두 번째 도형의 로컬 축 배열
      // DestHalfSize: 두 번째 도형의 반 크기
      // 반환값: 겹치면 true, 분리되면 false
      static bool AxisProjection(const FVector3& CenterLine,
          const FVector3& Axis, float SrcHalfSize,
          const FVector3* DestAxis, const FVector2& DestHalfSize);
  };

  5-2. 구현 파일 (Collision.cpp)

  #include "Collision.h"

  // ============================================
  // Box vs Box 충돌 검사 (자동 선택)
  // ============================================
  bool CCollision::CollisionBox2DToBox2D(FVector3& HitPoint,
      CColliderBox2D* Src, CColliderBox2D* Dest)
  {
      if (!Src || !Dest)
          return false;

      // 둘 다 회전이 0이면 AABB 충돌 검사 (빠름)
      if (Src->GetWorldRot().IsZero() && Dest->GetWorldRot().IsZero())
      {
          return CollisionAABB2DToAABB2D(HitPoint, Src->GetInfo(),
              Dest->GetInfo());
      }

      // 하나라도 회전이 있으면 OBB 충돌 검사 (느림)
      return CollisionOBB2DToOBB2D(HitPoint, Src->GetInfo(),
          Dest->GetInfo());
  }

  // ============================================
  // AABB vs AABB 충돌 검사
  // Axis-Aligned Bounding Box (축 정렬 경계 상자)
  // ============================================
  bool CCollision::CollisionAABB2DToAABB2D(FVector3& HitPoint,
      const FBox2DInfo& Src, const FBox2DInfo& Dest)
  {
      // ============================================
      // 1. 각 박스의 최소/최대 점 계산
      // ============================================
      FVector3 SrcMin, SrcMax, DestMin, DestMax;

      // Src 박스의 최소점 (좌하단)
      SrcMin = Src.Center - Src.Axis[EAxis::X] * Src.HalfSize.x -
          Src.Axis[EAxis::Y] * Src.HalfSize.y;

      // Src 박스의 최대점 (우상단)
      SrcMax = Src.Center + Src.Axis[EAxis::X] * Src.HalfSize.x +
          Src.Axis[EAxis::Y] * Src.HalfSize.y;

      // Dest 박스의 최소점
      DestMin = Dest.Center - Dest.Axis[EAxis::X] * Dest.HalfSize.x -
          Dest.Axis[EAxis::Y] * Dest.HalfSize.y;

      // Dest 박스의 최대점
      DestMax = Dest.Center + Dest.Axis[EAxis::X] * Dest.HalfSize.x +
          Dest.Axis[EAxis::Y] * Dest.HalfSize.y;

      // ============================================
      // 2. 분리축 정리 적용 (X축, Y축 검사)
      // ============================================

      // X축 검사: Src의 최소 x가 Dest의 최대 x보다 크면 분리
      if (SrcMin.x > DestMax.x)
          return false;

      // X축 검사: Dest의 최소 x가 Src의 최대 x보다 크면 분리
      else if (DestMin.x > SrcMax.x)
          return false;

      // Y축 검사: Src의 최소 y가 Dest의 최대 y보다 크면 분리
      else if (SrcMin.y > DestMax.y)
          return false;

      // Y축 검사: Dest의 최소 y가 Src의 최대 y보다 크면 분리
      else if (DestMin.y > SrcMax.y)
          return false;

      // ============================================
      // 3. 충돌! - 충돌 지점(HitPoint) 계산
      // ============================================

      // 교집합 영역의 최소/최대 점 계산
      FVector3 IntersectMin, IntersectMax;

      // 교집합 최소점 = 두 최소점 중 더 큰 값
      IntersectMin.x = SrcMin.x > DestMin.x ? SrcMin.x : DestMin.x;
      IntersectMin.y = SrcMin.y > DestMin.y ? SrcMin.y : DestMin.y;

      // 교집합 최대점 = 두 최대점 중 더 작은 값
      IntersectMax.x = SrcMax.x < DestMax.x ? SrcMax.x : DestMax.x;
      IntersectMax.y = SrcMax.y < DestMax.y ? SrcMax.y : DestMax.y;

      // HitPoint = 교집합 영역의 중심점
      HitPoint = (IntersectMin + IntersectMax) * 0.5f;

      return true;
  }

  // ============================================
  // OBB vs OBB 충돌 검사
  // Oriented Bounding Box (회전된 경계 상자)
  // 분리축 정리(Separating Axis Theorem) 사용
  // ============================================
  bool CCollision::CollisionOBB2DToOBB2D(FVector3& HitPoint,
      const FBox2DInfo& Src, const FBox2DInfo& Dest)
  {
      // ============================================
      // 분리축 정리(SAT):
      // 두 볼록 도형이 분리되어 있으면, 그들을 분리하는 축이 존재한다.
      // 2D OBB의 경우: 각 박스의 X축, Y축 (총 4개 축) 검사
      // ============================================

      // 두 박스 중심을 연결하는 벡터
      FVector3 CenterLine = Src.Center - Dest.Center;

      // ============================================
      // 1. Src의 X축을 분리축으로 검사
      // ============================================
      if (!AxisProjection(CenterLine, Src.Axis[EAxis::X],
          Src.HalfSize.x, Dest.Axis, Dest.HalfSize))
          return false;  // 분리축 발견 → 충돌 안함

      // ============================================
      // 2. Src의 Y축을 분리축으로 검사
      // ============================================
      if (!AxisProjection(CenterLine, Src.Axis[EAxis::Y],
          Src.HalfSize.y, Dest.Axis, Dest.HalfSize))
          return false;

      // ============================================
      // 3. Dest의 X축을 분리축으로 검사
      // ============================================
      if (!AxisProjection(CenterLine, Dest.Axis[EAxis::X],
          Dest.HalfSize.x, Src.Axis, Src.HalfSize))
          return false;

      // ============================================
      // 4. Dest의 Y축을 분리축으로 검사
      // ============================================
      if (!AxisProjection(CenterLine, Dest.Axis[EAxis::Y],
          Dest.HalfSize.y, Src.Axis, Src.HalfSize))
          return false;

      // ============================================
      // 모든 축에서 겹침 → 충돌!
      // ============================================

      // HitPoint 계산 (AABB와 동일한 방식)
      FVector3 SrcMin, SrcMax, DestMin, DestMax;

      SrcMin = Src.Center - Src.Axis[EAxis::X] * Src.HalfSize.x -
          Src.Axis[EAxis::Y] * Src.HalfSize.y;
      SrcMax = Src.Center + Src.Axis[EAxis::X] * Src.HalfSize.x +
          Src.Axis[EAxis::Y] * Src.HalfSize.y;

      DestMin = Dest.Center - Dest.Axis[EAxis::X] * Dest.HalfSize.x -
          Dest.Axis[EAxis::Y] * Dest.HalfSize.y;
      DestMax = Dest.Center + Dest.Axis[EAxis::X] * Dest.HalfSize.x +
          Dest.Axis[EAxis::Y] * Dest.HalfSize.y;

      FVector3 IntersectMin, IntersectMax;

      IntersectMin.x = SrcMin.x > DestMin.x ? SrcMin.x : DestMin.x;
      IntersectMin.y = SrcMin.y > DestMin.y ? SrcMin.y : DestMin.y;

      IntersectMax.x = SrcMax.x < DestMax.x ? SrcMax.x : DestMax.x;
      IntersectMax.y = SrcMax.y < DestMax.y ? SrcMax.y : DestMax.y;

      HitPoint = (IntersectMin + IntersectMax) * 0.5f;

      return true;
  }

  // ============================================
  // Sphere vs Sphere 충돌 검사 (Collider 객체)
  // ============================================
  bool CCollision::CollisionSphere2DToSphere2D(FVector3& HitPoint,
      CColliderSphere2D* Src, CColliderSphere2D* Dest)
  {
      if (!Src || !Dest)
          return false;

      // 구조체 버전 호출
      if (!CollisionSphere2DToSphere2D(HitPoint, Src->GetInfo(),
          Dest->GetInfo()))
          return false;

      return true;
  }

  // ============================================
  // Sphere vs Sphere 충돌 검사 (구조체)
  // ============================================
  bool CCollision::CollisionSphere2DToSphere2D(FVector3& HitPoint,
      const FSphere2DInfo& Src, const FSphere2DInfo& Dest)
  {
      // ============================================
      // 원-원 충돌: 중심 간 거리 vs 반지름 합
      // ============================================

      // 두 원 중심 사이의 거리 계산
      float Distance = Src.Center.Distance(Dest.Center);

      // 거리가 반지름의 합보다 크면 충돌 안함
      if (Distance > Src.Radius + Dest.Radius)
          return false;

      // ============================================
      // 충돌 지점 계산
      // ============================================

      // 겹친 거리 (침투 깊이)
      float Gap = Src.Radius + Dest.Radius - Distance;
      Gap *= 0.5f;  // 절반만 사용

      // Src에서 Dest로 향하는 방향 벡터
      FVector3 Dir = Dest.Center - Src.Center;
      Dir.Normalize();

      // HitPoint = Src 중심 + 방향 * (반지름 - Gap)
      // Src 표면에서 Gap만큼 안쪽 지점
      HitPoint = Src.Center + Dir * (Src.Radius - Gap);

      return true;
  }

  // ============================================
  // Box vs Sphere 충돌 검사 (Collider 객체)
  // ============================================
  bool CCollision::CollisionBox2DToSphere2D(FVector3& HitPoint,
      CColliderBox2D* Src, CColliderSphere2D* Dest)
  {
      if (!Src || !Dest)
          return false;

      // 구조체 버전 호출
      if (!CollisionBox2DToSphere2D(HitPoint, Src->GetInfo(),
          Dest->GetInfo()))
          return false;

      return true;
  }

  // ============================================
  // Box vs Sphere 충돌 검사 (구조체)
  // ============================================
  bool CCollision::CollisionBox2DToSphere2D(FVector3& HitPoint,
      const FBox2DInfo& Box, const FSphere2DInfo& Sphere)
  {
      // ============================================
      // 분리축 검사 (3개 축)
      // ============================================

      // 박스에서 원으로 향하는 벡터
      FVector3 CenterLine = Box.Center - Sphere.Center;

      // 1. 원 중심에서 박스 중심으로 향하는 축 검사
      FVector3 Axis = CenterLine;
      Axis.Normalize();

      if (!AxisProjection(CenterLine, Axis, Sphere.Radius,
          Box.Axis, Box.HalfSize))
          return false;

      // 2. 박스 X축으로 투영
      // 중심선을 박스 X축에 투영한 거리
      float CenterProjectionDist = abs(CenterLine.Dot(Box.Axis[EAxis::X]));

      // 투영 거리가 (반지름 + 박스 반너비)보다 크면 분리
      if (CenterProjectionDist > Sphere.Radius + Box.HalfSize.x)
          return false;

      // 3. 박스 Y축으로 투영
      CenterProjectionDist = abs(CenterLine.Dot(Box.Axis[EAxis::Y]));

      if (CenterProjectionDist > Sphere.Radius + Box.HalfSize.y)
          return false;

      // ============================================
      // 충돌! - HitPoint 계산
      // ============================================

      // 원을 AABB로 근사하여 교집합 계산
      FVector3 SrcMin, SrcMax, DestMin, DestMax;

      SrcMin = Sphere.Center - FVector3(Sphere.Radius, Sphere.Radius, 0.f);
      SrcMax = Sphere.Center + FVector3(Sphere.Radius, Sphere.Radius, 0.f);

      DestMin = Box.Center - Box.Axis[EAxis::X] * Box.HalfSize.x -
          Box.Axis[EAxis::Y] * Box.HalfSize.y;
      DestMax = Box.Center + Box.Axis[EAxis::X] * Box.HalfSize.x +
          Box.Axis[EAxis::Y] * Box.HalfSize.y;

      FVector3 IntersectMin, IntersectMax;

      IntersectMin.x = SrcMin.x > DestMin.x ? SrcMin.x : DestMin.x;
      IntersectMin.y = SrcMin.y > DestMin.y ? SrcMin.y : DestMin.y;

      IntersectMax.x = SrcMax.x < DestMax.x ? SrcMax.x : DestMax.x;
      IntersectMax.y = SrcMax.y < DestMax.y ? SrcMax.y : DestMax.y;

      HitPoint = (IntersectMin + IntersectMax) * 0.5f;

      return true;
  }

  // ============================================
  // 분리축 투영 (SAT 핵심 알고리즘)
  // ============================================
  bool CCollision::AxisProjection(const FVector3& CenterLine,
      const FVector3& Axis, float SrcHalfSize,
      const FVector3* DestAxis, const FVector2& DestHalfSize)
  {
      /*
      분리축 정리:
      - CenterLine을 특정 축(Axis)에 투영
      - 두 도형을 해당 축에 투영
      - 투영된 구간이 겹치는지 검사

      예시:
           Src          Dest
      |----●----|    |----●----|
           투영 →  |-----|  |-----|
                      ↑ 겹침?
      */

      // ============================================
      // 1. 중심선을 분리축에 투영
      // ============================================
      // 절대값 사용 (방향은 상관없고 거리만 필요)
      float CenterProjectionDist = abs(CenterLine.Dot(Axis));

      // ============================================
      // 2. Dest를 분리축에 투영한 구간 길이 계산
      // ============================================
      // Dest의 각 축을 분리축에 투영하고 크기를 곱함
      // |Axis · DestAxis[X]| * HalfSize.x
      // |Axis · DestAxis[Y]| * HalfSize.y
      float DestProjectionDist =
          abs(Axis.Dot(DestAxis[EAxis::X])) * DestHalfSize.x +
          abs(Axis.Dot(DestAxis[EAxis::Y])) * DestHalfSize.y;

      // ============================================
      // 3. 겹침 검사
      // ============================================
      // 두 투영 구간의 합이 중심 거리보다 크면 겹침
      // SrcHalfSize + DestProjectionDist > CenterProjectionDist
      if (SrcHalfSize + DestProjectionDist > CenterProjectionDist)
          return true;  // 겹침 (이 축으로는 분리 안됨)

      return false;  // 분리됨 (이 축으로 분리 가능)
  }

  ---
  계속해서 WorldCollision과 사용 예제를 작성하겠습니다!

> 계속

● 계속 진행하겠습니다!

  ---
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

  7-2. 총알 오브젝트에서 충돌체 사용

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

  7-3. 커스텀 충돌 프로파일 설정

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

  7-4. 실시간 충돌 프로파일 변경

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
  8. 시스템 동작 흐름 요약

  // ============================================
  // 전체 흐름 다이어그램
  // ============================================

  /*
  [게임 시작]
      ↓
  [CollisionInfoManager::Init()]
      - 기본 채널 생성 (Static, Player, Monster)
      - 기본 프로파일 생성
      ↓
  [Collider 생성] (예: Player의 Body Collider)
      ↓
  [CCollider::Init()]
      - WorldCollision에 자동 등록
      - 기본 프로파일 설정
      ↓
  [매 프레임]
      ↓
  [WorldCollision::Update(DeltaTime)]
      ↓
      ┌─────────────────────────────────┐
      │ for (각 Collider 쌍)            │
      │   ├─ 유효성 검사                │
      │   ├─ 프로파일 필터링            │
      │   ├─ Collision() 호출           │
      │   │   └─ CCollision 알고리즘   │
      │   │       (AABB/OBB/Sphere)    │
      │   ├─ 충돌 성공?                 │
      │   │   ├─ Yes: CollisionBegin   │
      │   │   │   또는 계속 충돌        │
      │   │   └─ No: CollisionEnd      │
      │   │       또는 계속 분리        │
      └─────────────────────────────────┘
      ↓
  [사용자 콜백 함수 호출]
      - OnCollisionBegin()
      - OnCollisionEnd()
      ↓
  [다음 프레임으로]
  */

  ---
  9. 최적화 포인트 및 개선 사항

  // ============================================
  // 현재 시스템의 한계와 개선 방안
  // ============================================

  /*
  1. 성능 문제:
     - 브루트포스 O(n²) 알고리즘
     - 충돌체가 많아지면 급격히 느려짐

     개선 방안:
     - Spatial Partitioning (공간 분할)
       * Quadtree
       * Grid-based
     - Broad Phase / Narrow Phase 분리
       * Broad Phase: AABB로 빠른 검사
       * Narrow Phase: 정밀 충돌 검사

  2. Line2D 미구현:
     - CollisionBox2DToLine2D
     - CollisionLine2DToLine2D

     구현 방법:
     - 선분-선분: 매개변수 방정식
     - 선분-박스: 4개 변과의 교차 검사

  3. 충돌 응답 없음:
     - 현재: 충돌 감지만
     - 개선: 물리적 응답 (밀어내기, 튕김 등)

  4. 3D 지원 없음:
     - 현재: 2D 전용
     - 개선: Box3D, Sphere3D 추가
  */

  ---
  10. 학습 가이드

  10-1. 코드를 따라 칠 때 순서

  1단계: 데이터 구조 (EngineInfo.h)
     → enum, struct 타입 정의

  2단계: CollisionInfoManager
     → 싱글톤 패턴 이해
     → 채널과 프로파일 관리

  3단계: Collider 기본 클래스
     → 추상 클래스 설계
     → 콜백 함수 패턴

  4단계: ColliderBox2D 구체 클래스
     → 상속 구조 이해
     → AABB 계산 로직

  5단계: Collision 알고리즘
     → AABB 충돌 (간단)
     → OBB 충돌 (분리축 정리)
     → Sphere 충돌
     → Box-Sphere 충돌

  6단계: WorldCollision
     → 이중 루프 구조
     → 충돌 상태 관리

  7단계: 사용 예제
     → 실제 게임 오브젝트 적용

  10-2. 핵심 개념 정리

  // ============================================
  // 1. 충돌 채널(Channel)
  // ============================================
  // - 오브젝트를 카테고리로 분류
  // - 예: Player, Monster, Item, Projectile
  // - 언리얼 엔진과 동일한 개념

  // ============================================
  // 2. 충돌 프로파일(Profile)
  // ============================================
  // - 특정 채널의 충돌 규칙 정의
  // - Interaction 배열로 다른 채널과의 관계 설정
  // - 예: Player는 Monster와 충돌, Player끼리는 무시

  // ============================================
  // 3. AABB (Axis-Aligned Bounding Box)
  // ============================================
  // - 축에 정렬된 바운딩 박스
  // - 회전 없을 때 사용
  // - 매우 빠름 (단순 좌표 비교)

  // ============================================
  // 4. OBB (Oriented Bounding Box)
  // ============================================
  // - 회전된 바운딩 박스
  // - 분리축 정리(SAT) 사용
  // - AABB보다 느림

  // ============================================
  // 5. 분리축 정리 (Separating Axis Theorem)
  // ============================================
  // - 두 볼록 도형이 분리되어 있으면
  //   그들을 분리하는 축이 존재한다
  // - 모든 후보 축에서 투영이 겹치면 충돌

  // ============================================
  // 6. 충돌 이벤트
  // ============================================
  // - CollisionBegin: 충돌 시작 (Enter)
  // - 계속 충돌: 매 프레임 충돌 중 (Stay)
  // - CollisionEnd: 충돌 종료 (Exit)

  ---
  11. 디버깅 팁

  // ============================================
  // 충돌 디버깅 체크리스트
  // ============================================

  /*
  1. 충돌이 감지 안될 때:
     □ SetDebugDraw(true) 했는가?
     □ 충돌체가 초록색인가? (빨간색이면 충돌 중)
     □ 프로파일이 Enable인가?
     □ 상대방 채널과 Interaction이 Collision인가?
     □ 충돌체 크기가 적절한가?
     □ 오프셋이 잘못 설정되지 않았나?

  2. 충돌 콜백이 안 불릴 때:
     □ SetCollisionBeginFunction() 호출했는가?
     □ 콜백 함수 시그니처가 정확한가?
     □ WorldCollision에 등록되었는가?

  3. 성능 문제:
     □ 충돌체 개수가 너무 많지 않은가? (100개 이상)
     □ SetInterval()로 검사 빈도 줄이기
     □ 불필요한 충돌체는 Disable하기

  4. 디버그 표시:
     - 초록색: 충돌 안함
     - 빨간색: 충돌 중
     - 안 보임: DebugDraw가 false
  */

  ---
  이것으로 게임 엔진의 충돌 처리 시스템 완전 분석을 마칩니다!

  핵심 요약:
  1. 채널 & 프로파일 시스템으로 충돌 관계 관리
  2. 다형성으로 여러 충돌체 타입 지원
  3. **분리축 정리(SAT)**로 OBB 충돌 검사
  4. 콜백 함수로 충돌 이벤트 처리
  5. WorldCollision이 매 프레임 모든 쌍 검사

  이 코드를 직접 따라 치면서 각 부분의 역할을 이해하시면 충돌 시스템의 작동 원리를 완전히 파악할 수 있을 것입니다!