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
