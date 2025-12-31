#pragma once

#include "Object/GameObject.h"

class CBullet :
    public CGameObject
{
	friend class CWorld;
	friend class CObject;

protected:
	CBullet();
	CBullet(const CBullet& ref);
	CBullet(CBullet&& ref)	noexcept;

public:
	virtual ~CBullet();

private:
	std::weak_ptr<class CMeshComponent>	mMeshComponent;
	float		mDistance = 600.f;
	FVector3	mMoveDir;
	bool		mMoveDirEnable = false;
	std::weak_ptr<CGameObject>	mTarget;

	std::string	mCollisionTargetName;
	float		mCollisionRange = 0.f;
	bool		mMoveEnable = true;

public:
	void SetMoveEnable(bool Enable)
	{
		mMoveEnable = Enable;
	}

	void ComputeCollisionRange()
	{
		FVector3	Scale = GetWorldScale();

		Scale /= 2.f;

		mCollisionRange = sqrtf(Scale.x * Scale.x + Scale.y * Scale.y);
	}

	void SetCollisionTargetName(const std::string& Name)
	{
		mCollisionTargetName = Name;
	}

	void SetMoveDir(const FVector3& MoveDir)
	{
		mMoveDirEnable = true;
		mMoveDir = MoveDir;
	}

	void SetNearTarget(const std::string& Name);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);

protected:
	virtual CBullet* Clone();
};

