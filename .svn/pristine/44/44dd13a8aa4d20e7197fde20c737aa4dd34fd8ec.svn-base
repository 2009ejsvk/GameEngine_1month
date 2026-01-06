#pragma once

#include "ColliderBox2D.h"

class CCollision
{
public:
	static bool CollisionBox2DToBox2D(FVector3& HitPoint,
		CColliderBox2D* Src, CColliderBox2D* Dest);
	static bool CollisionAABB2DToAABB2D(FVector3& HitPoint,
		const FBox2DInfo& Src, const FBox2DInfo& Dest);
	static bool CollisionOBB2DToOBB2D(FVector3& HitPoint,
		const FBox2DInfo& Src, const FBox2DInfo& Dest);


private:
	static bool AxisProjection(const FVector3& CenterLine,
		const FVector3& Axis, float SrcHalfSize,
		const FVector3* DestAxis, const FVector2& DestHalfSize);
};

