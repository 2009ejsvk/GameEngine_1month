#pragma once

#include "ColliderBox2D.h"
#include "ColliderSphere2D.h"
#include "ColliderLine2D.h"

namespace ECCWResult
{
	enum Type
	{
		CW = -1,
		Line,
		CCW
	};
};

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

public:
	static bool CollisionBox2DToLine2D(FVector3& HitPoint,
		CColliderBox2D* Src, CColliderLine2D* Dest);
	static bool CollisionBox2DToLine2D(FVector3& HitPoint,
		const FBox2DInfo& Box, const FLine2DInfo& Line);

	static bool CollisionSphere2DToLine2D(FVector3& HitPoint,
		CColliderSphere2D* Src, CColliderLine2D* Dest);
	static bool CollisionSphere2DToLine2D(FVector3& HitPoint,
		const FSphere2DInfo& Sphere, const FLine2DInfo& Line);

	static bool CollisionLine2DToLine2D(FVector3& HitPoint,
		CColliderLine2D* Src, CColliderLine2D* Dest);
	static bool CollisionLine2DToLine2D(FVector3& HitPoint,
		const FLine2DInfo& Src, const FLine2DInfo& Dest);


	static bool CollisionBox2DToPoint(FVector3& HitPoint,
		const FBox2DInfo& Box, const FVector3& Point);


private:
	static bool AxisProjection(const FVector3& CenterLine,
		const FVector3& Axis, float SrcHalfSize,
		const FVector3* DestAxis, const FVector2& DestHalfSize);

	static ECCWResult::Type CCW2D(const FVector3& p1,
		const FVector3& p2,
		const FVector3& p3);
	static bool PointOnLine2D(const FVector3& LineStart,
		const FVector3& LineEnd,
		const FVector3& Point);
};

