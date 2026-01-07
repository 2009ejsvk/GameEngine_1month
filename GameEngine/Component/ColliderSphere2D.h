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

	// 충돌체 위치로부터 얼마나 떨어져서 Center를 만들어줄지에 대한 값.
	FVector3			mOffset;

public:
	const FSphere2DInfo& GetInfo()	const
	{
		return mInfo;
	}

public:
	void SetCenterOffset(const FVector3& Offset)
	{
		mOffset = Offset;
	}

	void SetCenterOffset(float x, float y, float z)
	{
		mOffset = FVector3(x, y, z);
	}

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

