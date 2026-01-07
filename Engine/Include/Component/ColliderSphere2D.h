#pragma once

#include "Collider.h"

class CColliderSphere2D :
    public CCollider
{
	friend class CGameObject;
	friend class CObject;

protected:
	CColliderSphere2D();
	CColliderSphere2D(const CColliderSphere2D& ref);
	CColliderSphere2D(CColliderSphere2D&& ref)	noexcept;

public:
	virtual ~CColliderSphere2D();

protected:
	FSphere2DInfo		mInfo;

public:
	const FSphere2DInfo& GetInfo()	const
	{
		return mInfo;
	}

public:
	void SetRadius(float Radius)
	{
		mInfo.Radius = Radius;
	}

public:
	virtual void SetDebugDraw(bool DebugDraw);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);

protected:
	virtual CColliderSphere2D* Clone()	const;

public:
	virtual bool Collision(FVector3& HitPoint,
		std::shared_ptr<CCollider> Dest);
};

