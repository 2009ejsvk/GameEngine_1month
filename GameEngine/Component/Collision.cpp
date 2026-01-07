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

bool CCollision::CollisionBox2DToLine2D(FVector3& HitPoint, CColliderBox2D* Src, CColliderLine2D* Dest)
{
    return false;
}

bool CCollision::CollisionBox2DToLine2D(FVector3& HitPoint,
    const FBox2DInfo& Box, const FLine2DInfo& Line)
{
    // 선을 구성하는 점 2개중 사각형 안에 들어오는 점이 있다면
    // 무조건 충돌이다.
    // 하지만 들어오지 않을 경우 사각형을 구성하는 4개의 변을 만들고
    // 선이 교차하는 변이 있는지 체크하여 검사한다.

    return false;
}

bool CCollision::CollisionLine2DToLine2D(FVector3& HitPoint, CColliderLine2D* Src, CColliderLine2D* Dest)
{
    return false;
}

bool CCollision::CollisionLine2DToLine2D(FVector3& HitPoint, const FLine2DInfo& Src, const FLine2DInfo& Dest)
{
    return false;
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
