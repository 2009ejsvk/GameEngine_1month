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
