#pragma once

#include "Collider.h"

class CColliderLine2D :
    public CCollider
{
	friend class CGameObject;
	friend class CObject;

protected:
	CColliderLine2D();
	CColliderLine2D(const CColliderLine2D& ref);
	CColliderLine2D(CColliderLine2D&& ref)	noexcept;

public:
	virtual ~CColliderLine2D();

protected:
	FLine2DInfo		mInfo;
	FVector3		mLineDir = FVector3::Axis[EAxis::Y];
	float			mDistance = 100.f;

public:
	const FLine2DInfo& GetInfo()	const
	{
		return mInfo;
	}

public:
	void SetLineDir(const FVector3& Dir)
	{
		mLineDir = Dir;
		mLineDir.Normalize();
	}

	void SetLineDir(float x, float y, float z)
	{
		mLineDir = FVector3(x, y, z);
		mLineDir.Normalize();
	}

	void SetLineDir(float x, float y)
	{
		mLineDir = FVector3(x, y, 0.f);
		mLineDir.Normalize();
	}

	void SetLineDistance(float Dist)
	{
		mDistance = Dist;
	}

public:
	virtual void SetDebugDraw(bool DebugDraw);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void UpdateInfo();

protected:
	virtual CColliderLine2D* Clone()	const;

public:
	virtual bool Collision(FVector3& HitPoint,
		std::shared_ptr<CCollider> Dest);
	virtual bool CollisionManifold(FCollisionManifold& Result,
		std::shared_ptr<CCollider> Dest);
	virtual bool CollisionMouse(const FVector2& MousePos);
};

