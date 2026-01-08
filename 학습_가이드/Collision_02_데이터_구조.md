  1. 데이터 구조 (EngineInfo.h)

  // ============================================
  // 1-1. 충돌체 타입 열거형
  // ============================================
  enum class EColliderType
  {
      Box2D,      // 2D 박스 충돌체 (사각형)
      Sphere2D,   // 2D 원형 충돌체
      Line2D      // 2D 선분 충돌체
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
