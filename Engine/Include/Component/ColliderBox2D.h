#pragma once

#include "Collider.h"

class CColliderBox2D :
    public CCollider
{
	friend class CGameObject;
	friend class CObject;

protected:
	CColliderBox2D();
	CColliderBox2D(const CColliderBox2D& ref);
	CColliderBox2D(CColliderBox2D&& ref)	noexcept;

public:
	virtual ~CColliderBox2D();

protected:
	FBox2DInfo		mInfo;

public:
	const FBox2DInfo& GetInfo()	const
	{
		return mInfo;
	}

public:
	void SetBoxSize(const FVector2& Size)
	{
		mInfo.HalfSize = Size / 2.f;
	}

	void SetBoxSize(float x, float y)
	{
		mInfo.HalfSize = FVector2(x / 2.f, y / 2.f);
	}

public:
	virtual void SetDebugDraw(bool DebugDraw);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);

protected:
	virtual CColliderBox2D* Clone()	const;

public:
	virtual bool Collision(FVector3& HitPoint,
		std::shared_ptr<CCollider> Dest);
};

