#pragma once

#include "ColliderBox2D.h"
#include "ColliderSphere2D.h"

class CCollision
{
public:
	static bool CollisionBox2DToBox2D(FVector3& HitPoint,
		CColliderBox2D* Src, CColliderBox2D* Dest);
	static bool CollisionAABB2DToAABB2D(FVector3& HitPoint,
		const FBox2DInfo& Src, const FBox2DInfo& Dest);
	static bool CollisionOBB2DToOBB2D(FVector3& HitPoint,
		const FBox2DInfo& Src, const FBox2DInfo& Dest);

public:
	static bool CollisionSphere2DToSphere2D(FVector3& HitPoint,
		CColliderSphere2D* Src, CColliderSphere2D* Dest);
	static bool CollisionSphere2DToSphere2D(FVector3& HitPoint,
		const FSphere2DInfo& Src, const FSphere2DInfo& Dest);

public:
	static bool CollisionBox2DToSphere2D(FVector3& HitPoint,
		CColliderBox2D* Src, CColliderSphere2D* Dest);
	static bool CollisionBox2DToSphere2D(FVector3& HitPoint,
		const FBox2DInfo& Box, const FSphere2DInfo& Sphere);


private:
	static bool AxisProjection(const FVector3& CenterLine,
		const FVector3& Axis, float SrcHalfSize,
		const FVector3* DestAxis, const FVector2& DestHalfSize);
};

