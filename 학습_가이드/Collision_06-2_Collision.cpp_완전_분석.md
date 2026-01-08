# Collision.cpp 완전 분석 (학습용)

이 파일은 2D 충돌 검사 알고리즘의 핵심 구현체입니다. 각 함수를 수학적 배경과 함께 상세히 설명합니다.

---

## 📚 목차
1. [Box2D vs Box2D 충돌](#1-box2d-vs-box2d-충돌)
2. [Sphere2D vs Sphere2D 충돌](#2-sphere2d-vs-sphere2d-충돌)
3. [Box2D vs Sphere2D 충돌](#3-box2d-vs-sphere2d-충돌)
4. [Box2D vs Line2D 충돌](#4-box2d-vs-line2d-충돌)
5. [Sphere2D vs Line2D 충돌](#5-sphere2d-vs-line2d-충돌)
6. [Line2D vs Line2D 충돌](#6-line2d-vs-line2d-충돌)
7. [유틸리티 함수들](#7-유틸리티-함수들)

---

## 1. Box2D vs Box2D 충돌

### 1.1 CollisionBox2DToBox2D (라인 3-17)

```cpp
bool CCollision::CollisionBox2DToBox2D(FVector3& HitPoint,
    CColliderBox2D* Src, CColliderBox2D* Dest)
{
    if (!Src || !Dest)
        return false;

    if (Src->GetWorldRot().IsZero() && Dest->GetWorldRot().IsZero())
    {
        return CollisionAABB2DToAABB2D(HitPoint, Src->GetInfo(),
            Dest->GetInfo());
    }

    return CollisionOBB2DToOBB2D(HitPoint, Src->GetInfo(),
        Dest->GetInfo());
}
```

**핵심 아이디어:**
- 두 박스가 **회전이 없으면** → AABB (Axis-Aligned Bounding Box) 알고리즘
- 두 박스 중 **하나라도 회전이 있으면** → OBB (Oriented Bounding Box) 알고리즘

**성능 최적화 포인트:** AABB는 단순 좌표 비교로 매우 빠르지만, OBB는 분리축 정리(SAT)가 필요해 복잡함

---

### 1.2 CollisionAABB2DToAABB2D (라인 19-57)

**수학적 배경:**
AABB는 축과 정렬된 사각형이므로, Min-Max 좌표 비교만으로 충돌 판정 가능

```cpp
bool CCollision::CollisionAABB2DToAABB2D(FVector3& HitPoint,
    const FBox2DInfo& Src, const FBox2DInfo& Dest)
{
    FVector3 SrcMin, SrcMax, DestMin, DestMax;

    // 1단계: Min/Max 좌표 계산
    SrcMin = Src.Center - Src.Axis[EAxis::X] * Src.HalfSize.x -
        Src.Axis[EAxis::Y] * Src.HalfSize.y;
    SrcMax = Src.Center + Src.Axis[EAxis::X] * Src.HalfSize.x +
        Src.Axis[EAxis::Y] * Src.HalfSize.y;
```

**왜 이렇게 계산하는가?**
- `Center`는 사각형의 중심점
- `HalfSize.x`, `HalfSize.y`는 중심에서 각 축까지의 반너비/반높이
- X축 방향으로 `HalfSize.x`만큼, Y축 방향으로 `HalfSize.y`만큼 빼면 왼쪽 하단 모서리
- 더하면 오른쪽 상단 모서리

```cpp
    // 2단계: 분리축 테스트 (SAT - Separating Axis Theorem)
    if (SrcMin.x > DestMax.x)  // Src가 Dest보다 완전히 오른쪽
        return false;
    else if (DestMin.x > SrcMax.x)  // Dest가 Src보다 완전히 오른쪽
        return false;
    else if (SrcMin.y > DestMax.y)  // Src가 Dest보다 완전히 위
        return false;
    else if (DestMin.y > SrcMax.y)  // Dest가 Src보다 완전히 위
        return false;
```

**분리축 정리 (SAT) 핵심:**
> 두 도형이 충돌하지 않으면, 반드시 그 사이를 분리하는 축(Separating Axis)이 존재한다.

AABB의 경우 X축과 Y축만 검사하면 충분합니다.

```cpp
    // 3단계: 교차 영역 계산
    FVector3 IntersectMin, IntersectMax;

    IntersectMin.x = SrcMin.x > DestMin.x ? SrcMin.x : DestMin.x;  // 더 큰 Min
    IntersectMin.y = SrcMin.y > DestMin.y ? SrcMin.y : DestMin.y;

    IntersectMax.x = SrcMax.x < DestMax.x ? SrcMax.x : DestMax.x;  // 더 작은 Max
    IntersectMax.y = SrcMax.y < DestMax.y ? SrcMax.y : DestMax.y;

    HitPoint = (IntersectMin + IntersectMax) * 0.5f;  // 교차 영역의 중심
```

**충돌 지점 계산 원리:**
- 교차 영역의 Min = `max(SrcMin, DestMin)`
- 교차 영역의 Max = `min(SrcMax, DestMax)`
- 충돌 지점 = 교차 영역의 중심점

---

### 1.3 CollisionOBB2DToOBB2D (라인 59-106)

**수학적 배경: 분리축 정리 (SAT) 완전 이해**

회전된 사각형의 충돌은 다음 4개의 축에 대해 검사해야 합니다:
1. Src의 X축
2. Src의 Y축
3. Dest의 X축
4. Dest의 Y축

```cpp
bool CCollision::CollisionOBB2DToOBB2D(FVector3& HitPoint,
    const FBox2DInfo& Src, const FBox2DInfo& Dest)
{
    // 두 상자의 센터를 연결하는 벡터
    FVector3 CenterLine = Src.Center - Dest.Center;
```

**CenterLine의 의미:**
- 두 사각형의 중심을 연결하는 벡터
- 이 벡터를 각 분리축에 투영하여 거리 계산

```cpp
    // Src의 X축을 분리축으로 테스트
    if (!AxisProjection(CenterLine, Src.Axis[EAxis::X],
        Src.HalfSize.x, Dest.Axis, Dest.HalfSize))
        return false;

    // Src의 Y축을 분리축으로 테스트
    if (!AxisProjection(CenterLine, Src.Axis[EAxis::Y],
        Src.HalfSize.y, Dest.Axis, Dest.HalfSize))
        return false;

    // Dest의 X축을 분리축으로 테스트
    if (!AxisProjection(CenterLine, Dest.Axis[EAxis::X],
        Dest.HalfSize.x, Src.Axis, Src.HalfSize))
        return false;

    // Dest의 Y축을 분리축으로 테스트
    if (!AxisProjection(CenterLine, Dest.Axis[EAxis::Y],
        Dest.HalfSize.y, Src.Axis, Src.HalfSize))
        return false;
```

**왜 4개 축을 모두 검사해야 하는가?**
- 2D OBB는 각각 2개의 방향 축을 가짐
- 어느 한 축이라도 분리되면 충돌 X
- 모든 축에서 겹치면 충돌 O

**AxisProjection 함수의 역할 (라인 509-526 참고):**
```
1. CenterLine을 분리축에 투영 → 중심 간 거리
2. 두 사각형의 HalfSize를 분리축에 투영 → 각 사각형의 투영 반경
3. 투영 반경의 합 > 중심 간 거리 → 충돌 가능
4. 투영 반경의 합 <= 중심 간 거리 → 분리됨
```

---

## 2. Sphere2D vs Sphere2D 충돌

### 2.1 CollisionSphere2DToSphere2D (라인 121-139)

**수학적 배경: 거리 기반 충돌**

원-원 충돌의 수식:
```
|Center1 - Center2| <= Radius1 + Radius2
```

```cpp
bool CCollision::CollisionSphere2DToSphere2D(FVector3& HitPoint,
    const FSphere2DInfo& Src, const FSphere2DInfo& Dest)
{
    // 1단계: 두 중심 사이의 거리 계산
    float Distance = Src.Center.Distance(Dest.Center);

    // 2단계: 충돌 검사
    if (Distance > Src.Radius + Dest.Radius)
        return false;  // 분리됨
```

**충돌 지점 계산 (라인 130-136):**

```cpp
    // Gap: 겹친 깊이의 절반
    float Gap = Src.Radius + Dest.Radius - Distance;
    Gap *= 0.5f;

    // Dir: Src에서 Dest로 향하는 방향 벡터
    FVector3 Dir = Dest.Center - Src.Center;
    Dir.Normalize();

    // HitPoint: Src의 표면에서 Gap만큼 안쪽으로 들어간 지점
    HitPoint = Src.Center + Dir * (Src.Radius - Gap);
```

**시각화:**
```
    Src              Dest
     O ----Gap---> | <---Gap---- O
     |<--Radius--->|<--Radius-->|
     |             |             |
   Center      HitPoint       Center
```

---

## 3. Box2D vs Sphere2D 충돌

### 3.1 CollisionBox2DToSphere2D (라인 154-199)

**수학적 배경: 분리축 + 특수 케이스**

원과 사각형의 충돌은 3가지 축에서 검사:
1. 두 중심을 연결하는 축 (원의 방향축)
2. 사각형의 X축
3. 사각형의 Y축

```cpp
bool CCollision::CollisionBox2DToSphere2D(FVector3& HitPoint,
    const FBox2DInfo& Box, const FSphere2DInfo& Sphere)
{
    FVector3 CenterLine = Box.Center - Sphere.Center;
    FVector3 Axis = CenterLine;
    Axis.Normalize();

    // 1단계: 원의 방향축으로 투영 테스트
    if (!AxisProjection(CenterLine, Axis, Sphere.Radius,
        Box.Axis, Box.HalfSize))
        return false;
```

**왜 원의 방향축이 필요한가?**
- 사각형의 모서리가 원과 충돌하는 경우를 검출
- 원은 모든 방향에 대해 대칭이므로, 중심을 연결하는 축만 검사하면 충분

```cpp
    // 2단계: 상자 X축으로 투영
    float CenterProjectionDist = abs(CenterLine.Dot(Box.Axis[EAxis::X]));

    if (CenterProjectionDist > Sphere.Radius + Box.HalfSize.x)
        return false;

    // 3단계: 상자 Y축으로 투영
    CenterProjectionDist = abs(CenterLine.Dot(Box.Axis[EAxis::Y]));

    if (CenterProjectionDist > Sphere.Radius + Box.HalfSize.y)
        return false;
```

**내적(Dot Product)의 의미:**
```
CenterLine.Dot(Box.Axis[X]) = CenterLine을 Box의 X축에 투영한 길이
```

---

## 4. Box2D vs Line2D 충돌

### 4.1 CollisionBox2DToLine2D (라인 214-287)

**알고리즘 전략:**
1. 선의 양 끝점이 사각형 안에 있는지 검사
2. 없다면, 사각형의 4개 변과 선이 교차하는지 검사

```cpp
bool CCollision::CollisionBox2DToLine2D(FVector3& HitPoint,
    const FBox2DInfo& Box, const FLine2DInfo& Line)
{
    // 전략 1: 선의 끝점이 박스 안에 있는가?
    if (CollisionBox2DToPoint(HitPoint, Box, Line.Start))
        return true;
    else if (CollisionBox2DToPoint(HitPoint, Box, Line.End))
        return true;
```

**CollisionBox2DToPoint (라인 488-507):**
점이 사각형 안에 있는지 검사하는 방법:

```cpp
bool CCollision::CollisionBox2DToPoint(FVector3& HitPoint,
    const FBox2DInfo& Box, const FVector3& Point)
{
    FVector3 CenterLine = Point - Box.Center;

    // X축에 투영하여 검사
    float Dist = abs(CenterLine.Dot(Box.Axis[EAxis::X]));
    if (Dist > Box.HalfSize.x)
        return false;

    // Y축에 투영하여 검사
    Dist = abs(CenterLine.Dot(Box.Axis[EAxis::Y]));
    if (Dist > Box.HalfSize.y)
        return false;

    HitPoint = Point;
    return true;
}
```

**원리:**
- 점에서 사각형 중심까지의 벡터를 사각형의 각 축에 투영
- 투영된 길이가 HalfSize보다 작으면 내부

**전략 2: 사각형의 4개 변과 교차 검사 (라인 228-287):**

```cpp
    // 사각형의 4개 꼭지점 계산
    FVector3 Pos[4];

    // 왼쪽 하단
    Pos[0] = Box.Center - Box.Axis[EAxis::X] * Box.HalfSize.x -
        Box.Axis[EAxis::Y] * Box.HalfSize.y;
    // 왼쪽 상단
    Pos[1] = Box.Center - Box.Axis[EAxis::X] * Box.HalfSize.x +
        Box.Axis[EAxis::Y] * Box.HalfSize.y;
    // 오른쪽 하단
    Pos[2] = Box.Center + Box.Axis[EAxis::X] * Box.HalfSize.x -
        Box.Axis[EAxis::Y] * Box.HalfSize.y;
    // 오른쪽 상단
    Pos[3] = Box.Center + Box.Axis[EAxis::X] * Box.HalfSize.x +
        Box.Axis[EAxis::Y] * Box.HalfSize.y;
```

**사각형 구성 (시각화):**
```
    Pos[1] -------- Pos[3]
      |              |
      |    Center    |
      |              |
    Pos[0] -------- Pos[2]
```

```cpp
    FLine2DInfo BoxLine[4];

    BoxLine[0].Start = Pos[0]; BoxLine[0].End = Pos[1];  // 왼쪽 변
    BoxLine[1].Start = Pos[1]; BoxLine[1].End = Pos[3];  // 위 변
    BoxLine[2].Start = Pos[3]; BoxLine[2].End = Pos[2];  // 오른쪽 변
    BoxLine[3].Start = Pos[0]; BoxLine[3].End = Pos[2];  // 아래 변
```

**최단 거리 충돌점 찾기 (라인 261-286):**

```cpp
    bool Result = false;
    float Dist = FLT_MAX;
    FVector3 HitResult;

    for (int i = 0; i < 4; ++i)
    {
        if (CollisionLine2DToLine2D(HitPoint, Line, BoxLine[i]))
        {
            Result = true;

            // 선의 시작점에서 충돌점까지의 거리
            float Dist1 = Line.Start.Distance(HitPoint);

            // 가장 가까운 충돌점 선택
            if (Dist1 < Dist)
            {
                HitResult = HitPoint;
                Dist = Dist1;
            }
        }
    }
```

**왜 가장 가까운 충돌점을 선택하는가?**
- 선이 사각형을 관통하면 2개 이상의 충돌점 발생
- 첫 번째 충돌점(가장 가까운)이 실제 충돌 지점

---

## 5. Sphere2D vs Line2D 충돌

### 5.1 CollisionSphere2DToLine2D (라인 302-382)

**수학적 배경: 직선-원 교점 공식 (이차방정식)**

이 알고리즘은 고등학교 수학의 백미입니다. 주석에 전체 수식이 나와있습니다.

**문제 정의:**
- 원의 중심: `C`, 반지름: `r`
- 선의 시작점: `S`, 방향: `D`, 거리: `t`
- 교점: `P`

**1단계: 원 위의 점 조건**
```
||P - C|| = r
P = S + D*t
```

**2단계: 대입 및 정리**
```
||S - C + D*t|| = r
M = S - C  (치환)
||M + D*t|| = r
루트((M + D*t) · (M + D*t)) = r
(M + D*t) · (M + D*t) = r²
D²t² + 2(M·D)t + M² - r² = 0
```

**3단계: 이차방정식 형태**
```
At² + Bt + C = 0
A = D·D = 1 (D는 정규화된 방향벡터)
B = 2(M·D)
C = M·M - r²
```

**4단계: 근의 공식**
```
t = (-B ± √(B² - 4AC)) / 2A
D·D = 1이므로 A = 1
t = (-B ± √(B² - 4C)) / 2
```

**코드 구현:**

```cpp
bool CCollision::CollisionSphere2DToLine2D(FVector3& HitPoint,
    const FSphere2DInfo& Sphere, const FLine2DInfo& Line)
{
    FVector3 Dir = Line.End - Line.Start;
    float LineLength = Dir.Length();  // 선의 실제 길이
    Dir.Normalize();  // 단위 벡터로 정규화

    FVector3 M = Line.Start - Sphere.Center;

    // 이차방정식 계수 계산
    float b = 2.f * M.Dot(Dir);
    float c = M.Dot(M) - Sphere.Radius * Sphere.Radius;

    // 판별식
    float Det = b * b - 4.f * c;

    if (Det < 0.f)
        return false;  // 실근 없음 = 교점 없음
```

**판별식의 의미:**
- `Det > 0`: 2개의 교점 (선이 원을 관통)
- `Det = 0`: 1개의 교점 (선이 원에 접함)
- `Det < 0`: 교점 없음

```cpp
    Det = sqrtf(Det);

    float t1, t2;
    t1 = (-b + Det) / 2.f;  // 먼 교점
    t2 = (-b - Det) / 2.f;  // 가까운 교점

    // 두 교점이 모두 선의 시작점 뒤에 있으면 충돌 X
    if (t1 < 0.f && t2 < 0.f)
        return false;
```

**t값의 의미:**
- `t < 0`: 선의 시작점 뒤쪽
- `0 <= t <= LineLength`: 선분 위
- `t > LineLength`: 선의 끝점 너머

```cpp
    // 교점이 선분 범위 내에 있는지 검사
    bool Result = false;

    if (t1 > 0.f && t1 <= LineLength || t2 > 0.f && t2 <= LineLength)
        Result = true;
```

**특수 케이스: 선분이 원 안에 완전히 들어간 경우 (라인 359-369):**

```cpp
    else
    {
        // 선의 시작과 끝점이 원 안에 있는지 검사
        float Length1 = Line.Start.Distance(Sphere.Center);
        float Length2 = Line.End.Distance(Sphere.Center);

        if (Length1 <= Sphere.Radius && Length2 <= Sphere.Radius)
        {
            Result = true;
        }
    }
```

**충돌점 계산 (라인 371-379):**

```cpp
    if (Result)
    {
        // 더 가까운 교점 선택
        float HitDist = t1 < t2 ? t1 : t2;

        // 둘 다 음수면 더 큰 값 (덜 음수인 값) 선택
        if (HitDist < 0.f)
            HitDist = t1 > t2 ? t1 : t2;

        HitPoint = Line.Start + Dir * HitDist;
    }
```

---

## 6. Line2D vs Line2D 충돌

### 6.1 CollisionLine2DToLine2D (라인 396-486)

**수학적 배경: CCW + 크래머 공식**

이 알고리즘은 2단계로 구성됩니다:
1. **CCW 알고리즘**으로 교차 여부 판단
2. **크래머 공식**으로 교점 계산

**1단계: CCW(Counter Clock Wise) 검사 (라인 400-405):**

```cpp
    int ccw1 = CCW2D(Src.Start, Src.End, Dest.Start);
    int ccw2 = CCW2D(Src.Start, Src.End, Dest.End);
    int ccw3 = CCW2D(Dest.Start, Dest.End, Src.Start);
    int ccw4 = CCW2D(Dest.Start, Dest.End, Src.End);

    if (ccw1 * ccw2 < 0 && ccw3 * ccw4 < 0)
```

**CCW 알고리즘 원리 (라인 528-551):**

```cpp
ECCWResult::Type CCollision::CCW2D(const FVector3& p1,
    const FVector3& p2, const FVector3& p3)
{
    // 점 3개가 이루는 방향 계산
    FVector3 v = p2 - p1;
    FVector3 w = p3 - p1;

    // 2D 외적의 z 성분
    float Cross = v.x * w.y - v.y * w.x;

    if (Cross < 0.f)
        return ECCWResult::CW;       // 시계방향
    else if (Cross > 0.f)
        return ECCWResult::CCW;      // 반시계방향
    return ECCWResult::Line;         // 일직선
}
```

**외적(Cross Product)의 기하학적 의미:**
```
v × w = |v||w|sin(θ)

Cross > 0: p3가 p1→p2 벡터의 왼쪽 (반시계방향)
Cross < 0: p3가 p1→p2 벡터의 오른쪽 (시계방향)
Cross = 0: 일직선상에 위치
```

**교차 판정 조건:**
```
ccw1 * ccw2 < 0: Dest의 양 끝이 Src 직선의 양쪽에 위치
ccw3 * ccw4 < 0: Src의 양 끝이 Dest 직선의 양쪽에 위치
→ 두 조건 모두 만족하면 교차
```

**2단계: 교점 계산 - 크래머 공식 (라인 407-455):**

**수학적 배경:**
두 직선의 방정식:
```
직선1: a1*x + b1*y = c1
직선2: a2*x + b2*y = c2
```

행렬 형태:
```
|a1  b1|   |x|   |c1|
|a2  b2| × |y| = |c2|
```

크래머 공식:
```
Det = a1*b2 - a2*b1 (행렬식)

x = |c1  b1| / Det
    |c2  b2|

y = |a1  c1| / Det
    |a2  c2|
```

**코드 구현:**

```cpp
    if (ccw1 * ccw2 < 0 && ccw3 * ccw4 < 0)
    {
        // 직선의 방향 벡터
        FVector3 v = Src.Start - Src.End;
        FVector3 w = Dest.Start - Dest.End;

        // 행렬식 계산
        float Det = v.x * w.y - v.y * w.x;
```

**Det = 0이면?**
- 두 직선이 평행 → 교점 없음 (무한 or 0개)

```cpp
        // 크래머 공식으로 x 좌표 계산
        HitPoint.x = ((Src.Start.x * Src.End.y -
            Src.Start.y * Src.End.x) * (Dest.Start.x - Dest.End.x) -
            (Dest.Start.x * Dest.End.y - Dest.Start.y * Dest.End.x) *
            (Src.Start.x - Src.End.x)) / Det;

        // 크래머 공식으로 y 좌표 계산
        HitPoint.y = ((Src.Start.x * Src.End.y -
            Src.Start.y * Src.End.x) * (Dest.Start.y - Dest.End.y) -
            (Dest.Start.x * Dest.End.y - Dest.Start.y * Dest.End.x) *
            (Src.Start.y - Src.End.y)) / Det;

        return true;
    }
```

**3단계: 특수 케이스 - 끝점이 직선 위에 있는 경우 (라인 458-484):**

```cpp
    // Dest.Start가 Src 직선 위에 존재할 경우
    if (ccw1 == 0 && PointOnLine2D(Src.Start, Src.End, Dest.Start))
    {
        HitPoint = Dest.Start;
        return true;
    }

    else if (ccw2 == 0 && PointOnLine2D(Src.Start, Src.End, Dest.End))
    {
        HitPoint = Dest.End;
        return true;
    }

    else if (ccw3 == 0 && PointOnLine2D(Dest.Start, Dest.End, Src.Start))
    {
        HitPoint = Src.Start;
        return true;
    }

    else if (ccw4 == 0 && PointOnLine2D(Dest.Start, Dest.End, Src.End))
    {
        HitPoint = Src.End;
        return true;
    }
```

**PointOnLine2D (라인 553-595):**
점이 선분 위에 있는지 검사 (AABB 방식):

```cpp
bool CCollision::PointOnLine2D(const FVector3& LineStart,
    const FVector3& LineEnd, const FVector3& Point)
{
    float MinX, MinY, MaxX, MaxY;

    // X 범위 계산
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

    // Y 범위 계산
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

    // 점이 AABB 범위 내에 있는지 검사
    if (MinX > Point.x || MaxX < Point.x)
        return false;
    if (MinY > Point.y || MaxY < Point.y)
        return false;

    return true;
}
```

---

## 7. 유틸리티 함수들

### 7.1 AxisProjection (라인 509-526)

**분리축 정리의 핵심 계산 함수**

```cpp
bool CCollision::AxisProjection(const FVector3& CenterLine,
    const FVector3& Axis, float SrcHalfSize,
    const FVector3* DestAxis, const FVector2& DestHalfSize)
{
    // 중심선을 분리축에 투영
    float CenterProjectionDist = abs(CenterLine.Dot(Axis));

    // Dest의 투영 반경 계산
    float DestProjectionDist =
        abs(Axis.Dot(DestAxis[EAxis::X])) * DestHalfSize.x +
        abs(Axis.Dot(DestAxis[EAxis::Y])) * DestHalfSize.y;

    // 충돌 조건: (Src 반경 + Dest 반경) > 중심 거리
    if (SrcHalfSize + DestProjectionDist > CenterProjectionDist)
        return true;

    return false;
}
```

**수학적 의미:**

1. **CenterProjectionDist**: 두 중심 사이의 거리를 분리축에 투영
   ```
   = |CenterLine · Axis|
   ```

2. **DestProjectionDist**: Dest 사각형을 분리축에 투영한 반경
   ```
   = |Axis · DestAxis[X]| * HalfSize.x + |Axis · DestAxis[Y]| * HalfSize.y
   ```

   **왜 두 축의 투영을 더하는가?**
   - 사각형을 임의의 축에 투영하면 평행사변형이 됨
   - 그 평행사변형의 너비 = X축 기여분 + Y축 기여분

3. **충돌 조건:**
   ```
   SrcHalfSize + DestProjectionDist > CenterProjectionDist
   ```

   **시각화:**
   ```
   [----Src----][중심거리][----Dest----]

   충돌: [----Src----]
              [중심거리]
                    [----Dest----]
   ```

---

## 📊 성능 최적화 포인트

1. **AABB vs OBB 분기**: 회전 없는 경우 빠른 AABB 사용
2. **조기 탈출**: 분리축 발견 즉시 `false` 리턴
3. **내적/외적 활용**: 복잡한 삼각함수 대신 벡터 연산 사용
4. **판별식 검사**: 제곱근 계산 전에 판별식으로 먼저 필터링

## 🎯 학습 체크리스트

- [ ] AABB와 OBB의 차이를 설명할 수 있다
- [ ] 분리축 정리(SAT)의 원리를 이해했다
- [ ] 내적과 외적의 기하학적 의미를 안다
- [ ] CCW 알고리즘으로 교차 판정하는 원리를 안다
- [ ] 크래머 공식으로 교점을 계산할 수 있다
- [ ] 이차방정식으로 원-직선 교점을 구할 수 있다
- [ ] AxisProjection 함수의 수학적 의미를 이해했다

---

## 📚 참고 자료

- **분리축 정리(SAT)**: [Wikipedia - Hyperplane separation theorem](https://en.wikipedia.org/wiki/Hyperplane_separation_theorem)
- **CCW 알고리즘**: [Wikipedia - Cross product](https://en.wikipedia.org/wiki/Cross_product)
- **크래머 공식**: [Wikipedia - Cramer's rule](https://en.wikipedia.org/wiki/Cramer%27s_rule)
