#include "Collision.h"

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

bool CCollision::CollisionAABB2DToAABB2D(FVector3& HitPoint,
    const FBox2DInfo& Src, const FBox2DInfo& Dest)
{
    FVector3    SrcMin, SrcMax, DestMin, DestMax;

    SrcMin = Src.Center - Src.Axis[EAxis::X] * Src.HalfSize.x -
        Src.Axis[EAxis::Y] * Src.HalfSize.y;
    SrcMax = Src.Center + Src.Axis[EAxis::X] * Src.HalfSize.x +
        Src.Axis[EAxis::Y] * Src.HalfSize.y;

    DestMin = Dest.Center - Dest.Axis[EAxis::X] * Dest.HalfSize.x -
        Dest.Axis[EAxis::Y] * Dest.HalfSize.y;
    DestMax = Dest.Center + Dest.Axis[EAxis::X] * Dest.HalfSize.x +
        Dest.Axis[EAxis::Y] * Dest.HalfSize.y;

    if (SrcMin.x > DestMax.x)
        return false;

    else if (DestMin.x > SrcMax.x)
        return false;

    else if (SrcMin.y > DestMax.y)
        return false;

    else if (DestMin.y > SrcMax.y)
        return false;

    FVector3    IntersectMin, IntersectMax;

    IntersectMin.x = SrcMin.x > DestMin.x ? SrcMin.x : DestMin.x;
    IntersectMin.y = SrcMin.y > DestMin.y ? SrcMin.y : DestMin.y;

    IntersectMax.x = SrcMax.x < DestMax.x ? SrcMax.x : DestMax.x;
    IntersectMax.y = SrcMax.y < DestMax.y ? SrcMax.y : DestMax.y;

    HitPoint = (IntersectMin + IntersectMax) * 0.5f;

    return true;
}

bool CCollision::CollisionOBB2DToOBB2D(FVector3& HitPoint,
    const FBox2DInfo& Src, const FBox2DInfo& Dest)
{
    // 두 상자의 센터끼리 빼서 센터에서 센터를 향하는 벡터를 구한다.
    FVector3    CenterLine = Src.Center - Dest.Center;

    // 분리축을 Src의 X축을 기준으로 검사해본다.
    if (!AxisProjection(CenterLine, Src.Axis[EAxis::X],
        Src.HalfSize.x, Dest.Axis, Dest.HalfSize))
        return false;

    if (!AxisProjection(CenterLine, Src.Axis[EAxis::Y],
        Src.HalfSize.y, Dest.Axis, Dest.HalfSize))
        return false;

    // Dest 축을 분리축으로 하여 검사.
    if (!AxisProjection(CenterLine, Dest.Axis[EAxis::X],
        Dest.HalfSize.x, Src.Axis, Src.HalfSize))
        return false;

    if (!AxisProjection(CenterLine, Dest.Axis[EAxis::Y],
        Dest.HalfSize.y, Src.Axis, Src.HalfSize))
        return false;

    FVector3    SrcMin, SrcMax, DestMin, DestMax;

    SrcMin = Src.Center - Src.Axis[EAxis::X] * Src.HalfSize.x -
        Src.Axis[EAxis::Y] * Src.HalfSize.y;
    SrcMax = Src.Center + Src.Axis[EAxis::X] * Src.HalfSize.x +
        Src.Axis[EAxis::Y] * Src.HalfSize.y;

    DestMin = Dest.Center - Dest.Axis[EAxis::X] * Dest.HalfSize.x -
        Dest.Axis[EAxis::Y] * Dest.HalfSize.y;
    DestMax = Dest.Center + Dest.Axis[EAxis::X] * Dest.HalfSize.x +
        Dest.Axis[EAxis::Y] * Dest.HalfSize.y;

    FVector3    IntersectMin, IntersectMax;

    IntersectMin.x = SrcMin.x > DestMin.x ? SrcMin.x : DestMin.x;
    IntersectMin.y = SrcMin.y > DestMin.y ? SrcMin.y : DestMin.y;

    IntersectMax.x = SrcMax.x < DestMax.x ? SrcMax.x : DestMax.x;
    IntersectMax.y = SrcMax.y < DestMax.y ? SrcMax.y : DestMax.y;

    HitPoint = (IntersectMin + IntersectMax) * 0.5f;

    return true;
}

bool CCollision::CollisionSphere2DToSphere2D(FVector3& HitPoint,
    CColliderSphere2D* Src, CColliderSphere2D* Dest)
{
    if (!Src || !Dest)
        return false;

    if (!CollisionSphere2DToSphere2D(HitPoint, Src->GetInfo(),
        Dest->GetInfo()))
        return false;

    return true;
}

bool CCollision::CollisionSphere2DToSphere2D(FVector3& HitPoint,
    const FSphere2DInfo& Src, const FSphere2DInfo& Dest)
{
    // 센터와 센터 사이의 거리를 구한다.
    float Distance = Src.Center.Distance(Dest.Center);

    if (Distance > Src.Radius + Dest.Radius)
        return false;

    float   Gap = Src.Radius + Dest.Radius - Distance;
    Gap *= 0.5f;

    FVector3    Dir = Dest.Center - Src.Center;
    Dir.Normalize();

    HitPoint = Src.Center + Dir * (Src.Radius - Gap);

    return true;
}

bool CCollision::CollisionBox2DToSphere2D(FVector3& HitPoint, 
    CColliderBox2D* Src, CColliderSphere2D* Dest)
{
    if (!Src || !Dest)
        return false;

    if (!CollisionBox2DToSphere2D(HitPoint, Src->GetInfo(),
        Dest->GetInfo()))
        return false;

    return true;
}

bool CCollision::CollisionBox2DToSphere2D(FVector3& HitPoint,
    const FBox2DInfo& Box, const FSphere2DInfo& Sphere)
{
    FVector3    CenterLine = Box.Center - Sphere.Center;

    FVector3    Axis = CenterLine;
    Axis.Normalize();

    if (!AxisProjection(CenterLine, Axis, Sphere.Radius,
        Box.Axis, Box.HalfSize))
        return false;

    // 상자 X축으로 투영
    float   CenterProjectionDist = abs(CenterLine.Dot(Box.Axis[EAxis::X]));

    if (CenterProjectionDist > Sphere.Radius + Box.HalfSize.x)
        return false;

    // 상자 Y축으로 투영
    CenterProjectionDist = abs(CenterLine.Dot(Box.Axis[EAxis::Y]));

    if (CenterProjectionDist > Sphere.Radius + Box.HalfSize.y)
        return false;

    FVector3    SrcMin, SrcMax, DestMin, DestMax;

    SrcMin = Sphere.Center - FVector3(Sphere.Radius, Sphere.Radius, 0.f);
    SrcMax = Sphere.Center + FVector3(Sphere.Radius, Sphere.Radius, 0.f);

    DestMin = Box.Center - Box.Axis[EAxis::X] * Box.HalfSize.x -
        Box.Axis[EAxis::Y] * Box.HalfSize.y;
    DestMax = Box.Center + Box.Axis[EAxis::X] * Box.HalfSize.x +
        Box.Axis[EAxis::Y] * Box.HalfSize.y;

    FVector3    IntersectMin, IntersectMax;

    IntersectMin.x = SrcMin.x > DestMin.x ? SrcMin.x : DestMin.x;
    IntersectMin.y = SrcMin.y > DestMin.y ? SrcMin.y : DestMin.y;

    IntersectMax.x = SrcMax.x < DestMax.x ? SrcMax.x : DestMax.x;
    IntersectMax.y = SrcMax.y < DestMax.y ? SrcMax.y : DestMax.y;

    HitPoint = (IntersectMin + IntersectMax) * 0.5f;

    return true;
}

bool CCollision::CollisionBox2DToLine2D(FVector3& HitPoint, 
    CColliderBox2D* Src, CColliderLine2D* Dest)
{
    if (!Src || !Dest)
        return false;

    if (!CollisionBox2DToLine2D(HitPoint, Src->GetInfo(),
        Dest->GetInfo()))
        return false;

    return true;
}

bool CCollision::CollisionBox2DToLine2D(FVector3& HitPoint,
    const FBox2DInfo& Box, const FLine2DInfo& Line)
{
    // 선을 구성하는 점 2개중 사각형 안에 들어오는 점이 있다면
    // 무조건 충돌이다.
    if (CollisionBox2DToPoint(HitPoint, Box, Line.Start))
        return true;

    else if (CollisionBox2DToPoint(HitPoint, Box, Line.End))
        return true;     
    
    // 하지만 들어오지 않을 경우 사각형을 구성하는 4개의 변을 만들고
    // 선이 교차하는 변이 있는지 체크하여 검사한다.
    // 사각형을 구성하는 4개의 꼭지점을 구한다.
    FVector3	Pos[4];

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

    FLine2DInfo BoxLine[4];

    // 왼쪽
    BoxLine[0].Start = Pos[0];
    BoxLine[0].End = Pos[1];

    // 위
    BoxLine[1].Start = Pos[1];
    BoxLine[1].End = Pos[3];

    // 오른쪽
    BoxLine[2].Start = Pos[3];
    BoxLine[2].End = Pos[2];

    // 아래
    BoxLine[3].Start = Pos[0];
    BoxLine[3].End = Pos[2];

    bool    Result = false;
    float   Dist = FLT_MAX;
    FVector3    HitResult;

    for (int i = 0; i < 4; ++i)
    {
        if (CollisionLine2DToLine2D(HitPoint, Line, BoxLine[i]))
        {
            Result = true;

            float   Dist1 = Line.Start.Distance(HitPoint);

            if (Dist1 < Dist)
            {
                HitResult = HitPoint;
                Dist = Dist1;
            }
        }
    }

    if (Result)
    {
        HitPoint = HitResult;
    }

    return Result;
}

bool CCollision::CollisionSphere2DToLine2D(FVector3& HitPoint,
    CColliderSphere2D* Src, CColliderLine2D* Dest)
{
    if (!Src || !Dest)
        return false;

    if (!CollisionSphere2DToLine2D(HitPoint, Src->GetInfo(),
        Dest->GetInfo()))
        return false;

    return true;
}

bool CCollision::CollisionSphere2DToLine2D(FVector3& HitPoint, 
    const FSphere2DInfo& Sphere, const FLine2DInfo& Line)
{
    /*
    교점 P는 2개가 나올 수 있다.
    원 센터 : C 반지름 : r 교점 : P
    ||P - C|| - r = 0
    직선의 시작점 : S 직선의 방향 : D 임의의 거리 : t
    교점 P = S + Dt
    ||S - C + Dt|| - r = 0
    S - C : M
    ||M + Dt|| - r = 0
    루트((M + Dt) * (M + Dt)) - r = 0
    (M + Dt) * (M + Dt) - r * r = 0
    Dt^2 + 2MDt + M^2 - r^2 = 0
    이차방정식 : Ax^2 + Bx + C = 0
    A : D, B : 2MD, C : M^2 - r^2
    -B +- 루트(B^2 - 4AC) / 2A
    A에 해당하는 D는 방향벡터이므로 제곱해도 1이다 무시한다.
    -B +- 루트(B^2 - 4C) / 2
    */
    FVector3    Dir = Line.End - Line.Start;

    // 선의 길이를 구한다.
    float   LineLength = Dir.Length();

    // 방향만 남긴다.
    Dir.Normalize();

    FVector3    M = Line.Start - Sphere.Center;

    float   b = 2.f * M.Dot(Dir);
    float   c = M.Dot(M) - Sphere.Radius * Sphere.Radius;

    float   Det = b * b - 4.f * c;

    if (Det < 0.f)
        return false;

    Det = sqrtf(Det);

    float   t1, t2;

    t1 = (-b + Det) / 2.f;
    t2 = (-b - Det) / 2.f;

    // 교점이 둘 다 뒤에 있을 경우 충돌이 아니다.
    if (t1 < 0.f && t2 < 0.f)
        return false;

    // 구해준 두 길이중 하나라도 선의 길이보다 짧아야 한다.
    // 단, 음수는 제외한다.
    bool    Result = false;

    if (t1 > 0.f && t1 <= LineLength || t2 > 0.f && t2 <= LineLength)
        Result = true;

    else
    {
        // 선의 시작과 도착점이 원 안에 들어왔는지 판단한다.
        float   Length1 = Line.Start.Distance(Sphere.Center);
        float   Length2 = Line.End.Distance(Sphere.Center);

        if (Length1 <= Sphere.Radius && Length2 <= Sphere.Radius)
        {
            Result = true;
        }
    }

    if (Result)
    {
        float   HitDist = t1 < t2 ? t1 : t2;

        if (HitDist < 0.f)
            HitDist = t1 > t2 ? t1 : t2;

        HitPoint = Line.Start + Dir * HitDist;
    }

    return Result;
}

bool CCollision::CollisionLine2DToLine2D(FVector3& HitPoint, CColliderLine2D* Src, CColliderLine2D* Dest)
{
    if (!Src || !Dest)
        return false;

    if (!CollisionLine2DToLine2D(HitPoint, Src->GetInfo(),
        Dest->GetInfo()))
        return false;

    return true;
}

bool CCollision::CollisionLine2DToLine2D(FVector3& HitPoint,
    const FLine2DInfo& Src, const FLine2DInfo& Dest)
{
    // 두 선이 교차하는지 판단해야 한다.
    int ccw1 = CCW2D(Src.Start, Src.End, Dest.Start);
    int ccw2 = CCW2D(Src.Start, Src.End, Dest.End);
    int ccw3 = CCW2D(Dest.Start, Dest.End, Src.Start);
    int ccw4 = CCW2D(Dest.Start, Dest.End, Src.End);

    if (ccw1 * ccw2 < 0 && ccw3 * ccw4 < 0)
    {
        // 직선의 방정식 : ax + by = c
        // 점 A, B가 있을 경우
        // a1 = y1 - y2;
        // b1 = x2 - x1;
        // c1 = a1 * x1 + b1 * y1
        // 두 직선이 평행인지를 구하는 행렬식을 구한다.
        // 두 직선을 외적한다.
        FVector3    v = Src.Start - Src.End;
        FVector3    w = Dest.Start - Dest.End;

        // Det 값이 0일 경우 두 직선은 평행하다는 의미이다.
        // 직선1 : a1 * x + b1 * y = c1
        // 직선2 : a2 * x + b2 * y = c2
        /*
        |a1 b1|   |x| = |c1|
        |a2 b2|   |y|   |c2|

        2x2 행렬을 A, 미지수 벡터를 p = (x, y)
        c = (c1, c2)

        A * p = c

        p = A역행렬 * c

        크래머 공식을 이용해서 역행렬을 직접 만들지 않고 행렬식 만으로
        x, y를 구하는 방식이다.
        행렬식 Det가 0이면 두 점은 평행하여 무한한 교점이 나온다.
        행렬식 Det가 0이 아니면 두 직선의 방향이 달라서 한 점에서
        교차한다.

        a1 * x + b1 * y = c1
        a2 * x + b2 * y = c2

        a1 * b2 * x + b1 * b2 * y = c1 * b2
        a2 * b1 * x + b2 * b1 * y = c2  *b1
        */
        float   Det = v.x * w.y - v.y * w.x;

        HitPoint.x = ((Src.Start.x * Src.End.y -
            Src.Start.y * Src.End.x) * (Dest.Start.x - Dest.End.x) -
            (Dest.Start.x - Dest.End.y - Dest.Start.y * Dest.End.x) *
            (Src.Start.x - Src.End.x)) / Det;

        HitPoint.y = ((Src.Start.x * Src.End.y -
            Src.Start.y * Src.End.x) * (Dest.Start.y - Dest.End.y) -
            (Dest.Start.x - Dest.End.y - Dest.Start.y * Dest.End.x) *
            (Src.Start.y - Src.End.y)) / Det;

        return true;
    }

    // Dest.Start가 Src 직선 위에 존재할 경우.
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

    else if (ccw3 == 0 && PointOnLine2D(Dest.Start, Dest.End, 
        Src.Start))
    {
        HitPoint = Src.Start;
        return true;
    }

    else if (ccw4 == 0 && PointOnLine2D(Dest.Start, Dest.End,
        Src.End))
    {
        HitPoint = Src.End;
        return true;
    }

    return false;
}

bool CCollision::CollisionBox2DToPoint(FVector3& HitPoint,
    const FBox2DInfo& Box, const FVector3& Point)
{
    // CenterLine을 상자의 X, Y축에 투영하여 구간을 비교한다.
    FVector3    CenterLine = Point - Box.Center;

    float   Dist = abs(CenterLine.Dot(Box.Axis[EAxis::X]));

    if (Dist > Box.HalfSize.x)
        return false;

    Dist = abs(CenterLine.Dot(Box.Axis[EAxis::Y]));

    if (Dist > Box.HalfSize.y)
        return false;

    HitPoint = Point;

    return true;
}

bool CCollision::AxisProjection(const FVector3& CenterLine,
    const FVector3& Axis, float SrcHalfSize,
    const FVector3* DestAxis, const FVector2& DestHalfSize)
{
    // 센터 사이를 연결하는 벡터를 분리축 후보에 투영하여 구간의
    // 길이를 구한다. 단, 음수값은 필요 없으므로 절대값으로 처리한다.
    // abs : 절대값을 구해준다.
    float CenterProjectionDist = abs(CenterLine.Dot(Axis));

    float DestProjectionDist =
        abs(Axis.Dot(DestAxis[EAxis::X])) * DestHalfSize.x +
        abs(Axis.Dot(DestAxis[EAxis::Y])) * DestHalfSize.y;

    if (SrcHalfSize + DestProjectionDist > CenterProjectionDist)
        return true;

    return false;
}

ECCWResult::Type CCollision::CCW2D(const FVector3& p1, 
    const FVector3& p2, const FVector3& p3)
{
    // CCW(Counter Clock Wise) 알고리즘
    // 점 3개가 이루는 방향을 계산하는 알고리즘.
    // 점 A, B, C가 있을 경우 세 점이 이루는 방향을 계산.
    // CCW(A, B, C)
    // CCW(A, C, B)
    // 2D 벡터는 외적을 할 경우 x, y 값은 0, 0이고 z값만 의미를 가진다.
    // z값이 1이나 -1이냐에 따라 외적 방향이 결정되기 때문이다.
    // (A X B).z = Ax * By - Ay * Bx;
    FVector3    v = p2 - p1;
    FVector3    w = p3 - p1;

    float   Cross = v.x * w.y - v.y * w.x;

    if (Cross < 0.f)
        return ECCWResult::CW;

    else if (Cross > 0.f)
        return ECCWResult::CCW;

    return ECCWResult::Line;
}

bool CCollision::PointOnLine2D(const FVector3& LineStart,
    const FVector3& LineEnd, const FVector3& Point)
{
    float   MinX, MinY, MaxX, MaxY;

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
