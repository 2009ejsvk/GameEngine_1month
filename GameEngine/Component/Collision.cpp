#include "Collision.h"

bool CCollision::CollisionBox2DToBox2D(FVector3& HitPoint, 
    CColliderBox2D* Src, CColliderBox2D* Dest)
{
    if (!Src || !Dest)
        return false;

    if (Src->GetWorldRot().IsZero() && Dest->GetWorldRot().IsZero())
    {
        return CollisionAABBToAABB(HitPoint, Src->GetInfo(),
            Dest->GetInfo());
    }

    return CollisionOBBToOBB(HitPoint, Src->GetInfo(),
        Dest->GetInfo());
}

bool CCollision::CollisionAABBToAABB(FVector3& HitPoint,
    const FBox2DInfo& Src, const FBox2DInfo& Dest)
{
    return false;
}

bool CCollision::CollisionOBBToOBB(FVector3& HitPoint,
    const FBox2DInfo& Src, const FBox2DInfo& Dest)
{
    return false;
}
