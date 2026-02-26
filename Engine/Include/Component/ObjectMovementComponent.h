#pragma once

#include "MovementComponent.h"
#include "../World/NavAgent.h"

class CObjectMovementComponent :
	public CMovementComponent,
	public CNavAgent
{
	friend class CGameObject;

protected:
	CObjectMovementComponent();
	CObjectMovementComponent(const CObjectMovementComponent& ref);
	CObjectMovementComponent(CObjectMovementComponent&& ref)	noexcept;

public:
	virtual ~CObjectMovementComponent();

protected:
	FVector3		mMoveDir = FVector3::Zero;
	float			mSpeed = 200.f;
	FVector3		mVelocity;

public:
	const FVector3& GetVelocity()	const
	{
		return mVelocity;
	}

	const FVector3& GetMoveDir()	const
	{
		return mMoveDir;
	}

	float GetSpeed()	const
	{
		return mSpeed;
	}

	float GetVelocityFrame()	const
	{
		if (mMoveDir.IsZero())
			return 0.f;

		return mSpeed * CTimer::GetDeltaTime();
	}

public:
	void SetSpeed(float Speed)
	{
		mSpeed = Speed;
	}

	void AddMove(const FVector3& Dir)
	{
		mMoveDir += Dir;
	}

	void Move(const FVector3& Pos);
	void Move(const FVector2& Pos);
	void MovePath(const FVector3& Pos);
	void MovePath(const FVector2& Pos);
	virtual void StartPath();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void PostRender();
	virtual void Destroy();

protected:
	virtual CObjectMovementComponent* Clone()	const;
};

