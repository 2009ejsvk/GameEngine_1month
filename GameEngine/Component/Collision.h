#pragma once

#include "ColliderBox2D.h"

class CCollision
{
public:
	static bool CollisionBox2DToBox2D(FVector3& HitPoint,
		CColliderBox2D* Src, CColliderBox2D* Dest);
	static bool CollisionAABBToAABB(FVector3& HitPoint,
		const FBox2DInfo& Src, const FBox2DInfo& Dest);
	static bool CollisionOBBToOBB(FVector3& HitPoint,
		const FBox2DInfo& Src, const FBox2DInfo& Dest);
};

