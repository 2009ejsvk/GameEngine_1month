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

## 5-3. Line2D 충돌 알고리즘

### 5-3-1. Collision.h에 추가된 함수들

```cpp
public:
    // ============================================
    // Box vs Line 충돌 검사
    // ============================================
    static bool CollisionBox2DToLine2D(FVector3& HitPoint,
        CColliderBox2D* Src, CColliderLine2D* Dest);
    static bool CollisionBox2DToLine2D(FVector3& HitPoint,
        const FBox2DInfo& Box, const FLine2DInfo& Line);

    // ============================================
    // Sphere vs Line 충돌 검사
    // ============================================
    static bool CollisionSphere2DToLine2D(FVector3& HitPoint,
        CColliderSphere2D* Src, CColliderLine2D* Dest);
    static bool CollisionSphere2DToLine2D(FVector3& HitPoint,
        const FSphere2DInfo& Sphere, const FLine2DInfo& Line);

    // ============================================
    // Line vs Line 충돌 검사
    // ============================================
    static bool CollisionLine2DToLine2D(FVector3& HitPoint,
        CColliderLine2D* Src, CColliderLine2D* Dest);
    static bool CollisionLine2DToLine2D(FVector3& HitPoint,
        const FLine2DInfo& Src, const FLine2DInfo& Dest);

    // ============================================
    // Box vs Point 충돌 검사 (Line 충돌에서 사용)
    // ============================================
    static bool CollisionBox2DToPoint(FVector3& HitPoint,
        const FBox2DInfo& Box, const FVector3& Point);

private:
    // ============================================
    // CCW (Counter Clock Wise) 알고리즘
    // 세 점이 이루는 방향 판정
    // ============================================
    static ECCWResult::Type CCW2D(const FVector3& p1,
        const FVector3& p2,
        const FVector3& p3);

    // ============================================
    // 점이 선분 위에 있는지 검사
    // ============================================
    static bool PointOnLine2D(const FVector3& LineStart,
        const FVector3& LineEnd,
        const FVector3& Point);
```

### 5-3-2. Box vs Line 충돌 검사 (Collision.cpp)

```cpp
// ============================================
// Box vs Line 충돌 검사 (Collider 객체)
// ============================================
bool CCollision::CollisionBox2DToLine2D(FVector3& HitPoint,
    CColliderBox2D* Src, CColliderLine2D* Dest)
{
    if (!Src || !Dest)
        return false;

    // 구조체 버전 호출
    if (!CollisionBox2DToLine2D(HitPoint, Src->GetInfo(),
        Dest->GetInfo()))
        return false;

    return true;
}

// ============================================
// Box vs Line 충돌 검사 (구조체)
// ============================================
bool CCollision::CollisionBox2DToLine2D(FVector3& HitPoint,
    const FBox2DInfo& Box, const FLine2DInfo& Line)
{
    // ============================================
    // 1. 선분의 시작/끝점이 박스 안에 있는지 검사
    // ============================================
    // 하나라도 박스 안에 있으면 무조건 충돌
    if (CollisionBox2DToPoint(HitPoint, Box, Line.Start))
        return true;

    else if (CollisionBox2DToPoint(HitPoint, Box, Line.End))
        return true;

    // ============================================
    // 2. 박스의 4개 변과 선분의 교차 검사
    // ============================================
    // 박스를 구성하는 4개의 꼭지점 계산
    FVector3 Pos[4];

    // 좌하단
    Pos[0] = Box.Center - Box.Axis[EAxis::X] * Box.HalfSize.x -
        Box.Axis[EAxis::Y] * Box.HalfSize.y;
    // 좌상단
    Pos[1] = Box.Center - Box.Axis[EAxis::X] * Box.HalfSize.x +
        Box.Axis[EAxis::Y] * Box.HalfSize.y;
    // 우하단
    Pos[2] = Box.Center + Box.Axis[EAxis::X] * Box.HalfSize.x -
        Box.Axis[EAxis::Y] * Box.HalfSize.y;
    // 우상단
    Pos[3] = Box.Center + Box.Axis[EAxis::X] * Box.HalfSize.x +
        Box.Axis[EAxis::Y] * Box.HalfSize.y;

    // 박스의 4개 변 정의
    FLine2DInfo BoxLine[4];

    // 왼쪽 변
    BoxLine[0].Start = Pos[0];
    BoxLine[0].End = Pos[1];

    // 위쪽 변
    BoxLine[1].Start = Pos[1];
    BoxLine[1].End = Pos[3];

    // 오른쪽 변
    BoxLine[2].Start = Pos[3];
    BoxLine[2].End = Pos[2];

    // 아래쪽 변
    BoxLine[3].Start = Pos[0];
    BoxLine[3].End = Pos[2];

    // ============================================
    // 3. 각 변과 선분의 교차점 찾기
    // ============================================
    bool Result = false;
    float Dist = FLT_MAX;  // 최소 거리 추적
    FVector3 HitResult;

    for (int i = 0; i < 4; ++i)
    {
        // 선분과 박스 변이 교차하는지 검사
        if (CollisionLine2DToLine2D(HitPoint, Line, BoxLine[i]))
        {
            Result = true;

            // 선분 시작점에서 교차점까지의 거리
            float Dist1 = Line.Start.Distance(HitPoint);

            // 가장 가까운 교차점 찾기
            if (Dist1 < Dist)
            {
                HitResult = HitPoint;
                Dist = Dist1;
            }
        }
    }

    // 충돌했으면 가장 가까운 교차점을 HitPoint로 설정
    if (Result)
    {
        HitPoint = HitResult;
    }

    return Result;
}
```

### 5-3-3. Sphere vs Line 충돌 검사

```cpp
// ============================================
// Sphere vs Line 충돌 검사 (Collider 객체)
// ============================================
bool CCollision::CollisionSphere2DToLine2D(FVector3& HitPoint,
    CColliderSphere2D* Src, CColliderLine2D* Dest)
{
    if (!Src || !Dest)
        return false;

    // 구조체 버전 호출
    if (!CollisionSphere2DToLine2D(HitPoint, Src->GetInfo(),
        Dest->GetInfo()))
        return false;

    return true;
}

// ============================================
// Sphere vs Line 충돌 검사 (구조체)
// 직선과 원의 교점을 이차방정식으로 계산
// ============================================
bool CCollision::CollisionSphere2DToLine2D(FVector3& HitPoint,
    const FSphere2DInfo& Sphere, const FLine2DInfo& Line)
{
    /*
    ============================================
    원과 직선의 교점 공식 유도
    ============================================
    원의 방정식: ||P - C|| = r
    직선의 방정식: P = S + Dt
      (S: 시작점, D: 방향, t: 거리)

    ||S - C + Dt|| = r
    M = S - C로 치환
    ||M + Dt|| = r
    (M + Dt)·(M + Dt) = r²
    M·M + 2M·Dt + D·D·t² = r²

    D는 단위벡터이므로 D·D = 1
    t² + 2(M·D)t + (M·M - r²) = 0

    이차방정식: at² + bt + c = 0
    a = 1, b = 2(M·D), c = M·M - r²

    해: t = (-b ± √(b² - 4ac)) / 2a
    a = 1이므로: t = (-b ± √(b² - 4c)) / 2
    ============================================
    */

    // 선분의 방향 벡터
    FVector3 Dir = Line.End - Line.Start;

    // 선분의 길이 저장
    float LineLength = Dir.Length();

    // 방향 벡터를 단위 벡터로 정규화
    Dir.Normalize();

    // M = S - C (선 시작점에서 원 중심으로의 벡터)
    FVector3 M = Line.Start - Sphere.Center;

    // 이차방정식 계수 계산
    float b = 2.f * M.Dot(Dir);
    float c = M.Dot(M) - Sphere.Radius * Sphere.Radius;

    // 판별식 (Determinant)
    float Det = b * b - 4.f * c;

    // 판별식이 음수면 교점 없음 (선이 원과 떨어져 있음)
    if (Det < 0.f)
        return false;

    // 판별식의 제곱근
    Det = sqrtf(Det);

    // 두 교점까지의 거리 계산
    float t1, t2;
    t1 = (-b + Det) / 2.f;
    t2 = (-b - Det) / 2.f;

    // 두 교점이 모두 선분 시작점 뒤에 있으면 충돌 아님
    if (t1 < 0.f && t2 < 0.f)
        return false;

    // ============================================
    // 교점이 선분 범위 안에 있는지 검사
    // ============================================
    bool Result = false;

    // 교점 중 하나라도 [0, LineLength] 범위에 있으면 충돌
    if (t1 > 0.f && t1 <= LineLength || t2 > 0.f && t2 <= LineLength)
        Result = true;

    else
    {
        // 선분의 시작과 끝이 모두 원 안에 있는지 검사
        float Length1 = Line.Start.Distance(Sphere.Center);
        float Length2 = Line.End.Distance(Sphere.Center);

        if (Length1 <= Sphere.Radius && Length2 <= Sphere.Radius)
        {
            Result = true;
        }
    }

    // ============================================
    // HitPoint 계산 (가장 가까운 교점)
    // ============================================
    if (Result)
    {
        // 더 가까운 교점 선택
        float HitDist = t1 < t2 ? t1 : t2;

        // 음수면 더 먼 교점 선택
        if (HitDist < 0.f)
            HitDist = t1 > t2 ? t1 : t2;

        // HitPoint = Start + Dir * t
        HitPoint = Line.Start + Dir * HitDist;
    }

    return Result;
}
```

### 5-3-4. Line vs Line 충돌 검사 (CCW 알고리즘)

```cpp
// ============================================
// Line vs Line 충돌 검사 (Collider 객체)
// ============================================
bool CCollision::CollisionLine2DToLine2D(FVector3& HitPoint,
    CColliderLine2D* Src, CColliderLine2D* Dest)
{
    if (!Src || !Dest)
        return false;

    // 구조체 버전 호출
    if (!CollisionLine2DToLine2D(HitPoint, Src->GetInfo(),
        Dest->GetInfo()))
        return false;

    return true;
}

// ============================================
// Line vs Line 충돌 검사 (구조체)
// CCW 알고리즘 사용
// ============================================
bool CCollision::CollisionLine2DToLine2D(FVector3& HitPoint,
    const FLine2DInfo& Src, const FLine2DInfo& Dest)
{
    // ============================================
    // CCW 알고리즘으로 선분 교차 판정
    // ============================================
    /*
    두 선분 AB, CD가 교차하려면:
    1. C와 D가 AB를 기준으로 반대편에 있어야 함
       → CCW(A,B,C) * CCW(A,B,D) < 0
    2. A와 B가 CD를 기준으로 반대편에 있어야 함
       → CCW(C,D,A) * CCW(C,D,B) < 0
    */

    int ccw1 = CCW2D(Src.Start, Src.End, Dest.Start);
    int ccw2 = CCW2D(Src.Start, Src.End, Dest.End);
    int ccw3 = CCW2D(Dest.Start, Dest.End, Src.Start);
    int ccw4 = CCW2D(Dest.Start, Dest.End, Src.End);

    // ============================================
    // 일반적인 교차 (두 선분이 X자로 교차)
    // ============================================
    if (ccw1 * ccw2 < 0 && ccw3 * ccw4 < 0)
    {
        // ============================================
        // 교점 계산 (크래머 공식)
        // ============================================
        /*
        직선의 방정식: ax + by = c
        점 A(x1,y1), B(x2,y2)가 있을 때:
        a = y1 - y2
        b = x2 - x1
        c = a*x1 + b*y1

        두 직선의 교점:
        Det = v.x * w.y - v.y * w.x (행렬식)
        x = ((x1*y2 - y1*x2)(x3 - x4) - (x3*y4 - y3*x4)(x1 - x2)) / Det
        y = ((x1*y2 - y1*x2)(y3 - y4) - (x3*y4 - y3*x4)(y1 - y2)) / Det
        */

        FVector3 v = Src.Start - Src.End;
        FVector3 w = Dest.Start - Dest.End;

        // 행렬식 계산
        float Det = v.x * w.y - v.y * w.x;

        // 교점 x좌표
        HitPoint.x = ((Src.Start.x * Src.End.y -
            Src.Start.y * Src.End.x) * (Dest.Start.x - Dest.End.x) -
            (Dest.Start.x * Dest.End.y - Dest.Start.y * Dest.End.x) *
            (Src.Start.x - Src.End.x)) / Det;

        // 교점 y좌표
        HitPoint.y = ((Src.Start.x * Src.End.y -
            Src.Start.y * Src.End.x) * (Dest.Start.y - Dest.End.y) -
            (Dest.Start.x * Dest.End.y - Dest.Start.y * Dest.End.x) *
            (Src.Start.y - Src.End.y)) / Det;

        return true;
    }

    // ============================================
    // 특수 케이스: 점이 선분 위에 있는 경우
    // ============================================
    // Dest.Start가 Src 선분 위에 있을 경우
    if (ccw1 == 0 && PointOnLine2D(Src.Start, Src.End, Dest.Start))
    {
        HitPoint = Dest.Start;
        return true;
    }

    // Dest.End가 Src 선분 위에 있을 경우
    else if (ccw2 == 0 && PointOnLine2D(Src.Start, Src.End, Dest.End))
    {
        HitPoint = Dest.End;
        return true;
    }

    // Src.Start가 Dest 선분 위에 있을 경우
    else if (ccw3 == 0 && PointOnLine2D(Dest.Start, Dest.End, Src.Start))
    {
        HitPoint = Src.Start;
        return true;
    }

    // Src.End가 Dest 선분 위에 있을 경우
    else if (ccw4 == 0 && PointOnLine2D(Dest.Start, Dest.End, Src.End))
    {
        HitPoint = Src.End;
        return true;
    }

    return false;
}
```

### 5-3-5. 보조 함수들

```cpp
// ============================================
// Box vs Point 충돌 검사
// ============================================
bool CCollision::CollisionBox2DToPoint(FVector3& HitPoint,
    const FBox2DInfo& Box, const FVector3& Point)
{
    // 점에서 박스 중심으로의 벡터
    FVector3 CenterLine = Point - Box.Center;

    // ============================================
    // 박스의 X축에 투영하여 검사
    // ============================================
    float Dist = abs(CenterLine.Dot(Box.Axis[EAxis::X]));

    // 투영 거리가 박스 반너비보다 크면 밖에 있음
    if (Dist > Box.HalfSize.x)
        return false;

    // ============================================
    // 박스의 Y축에 투영하여 검사
    // ============================================
    Dist = abs(CenterLine.Dot(Box.Axis[EAxis::Y]));

    if (Dist > Box.HalfSize.y)
        return false;

    // 두 축 모두 범위 안에 있으면 충돌
    HitPoint = Point;
    return true;
}

// ============================================
// CCW (Counter Clock Wise) 알고리즘
// 세 점이 이루는 방향 판정
// ============================================
/*
외적을 이용한 방향 판정:
- 양수: 반시계방향 (CCW)
- 0: 일직선
- 음수: 시계방향 (CW)

2D 외적: (B-A) × (C-A) = (B.x-A.x)(C.y-A.y) - (B.y-A.y)(C.x-A.x)
*/
ECCWResult::Type CCollision::CCW2D(const FVector3& p1,
    const FVector3& p2, const FVector3& p3)
{
    // 벡터 v = p2 - p1
    FVector3 v = p2 - p1;
    // 벡터 w = p3 - p1
    FVector3 w = p3 - p1;

    // 외적의 z 성분 계산
    float Cross = v.x * w.y - v.y * w.x;

    if (Cross < 0.f)
        return ECCWResult::CW;      // 시계방향

    else if (Cross > 0.f)
        return ECCWResult::CCW;     // 반시계방향

    return ECCWResult::Line;        // 일직선
}

// ============================================
// 점이 선분 위에 있는지 검사
// ============================================
bool CCollision::PointOnLine2D(const FVector3& LineStart,
    const FVector3& LineEnd, const FVector3& Point)
{
    // 선분의 최소/최대 x, y 좌표 계산
    float MinX, MinY, MaxX, MaxY;

    if (LineStart.x < LineEnd.x)
    {
        MinX = LineStart.x;
        MaxX = LineEnd.x;
    }
    else
    {
        MinX = LineEnd.x;
        MaxX = LineStart.x;
    }

    if (LineStart.y < LineEnd.y)
    {
        MinY = LineStart.y;
        MaxY = LineEnd.y;
    }
    else
    {
        MinY = LineEnd.y;
        MaxY = LineStart.y;
    }

    // 점이 선분의 AABB 범위 안에 있는지 검사
    if (MinX > Point.x)
        return false;

    else if (MaxX < Point.x)
        return false;

    else if (MinY > Point.y)
        return false;

    else if (MaxY < Point.y)
        return false;

    return true;
}
```

---

  계속해서 WorldCollision과 사용 예제를 작성하겠습니다!

> 계속

● 계속 진행하겠습니다!

  ---
