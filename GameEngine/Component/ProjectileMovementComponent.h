#pragma once

#include "MovementComponent.h"

class CProjectileMovementComponent :
    public CMovementComponent
{
	friend class CGameObject;

protected:
	CProjectileMovementComponent();
	CProjectileMovementComponent(const CProjectileMovementComponent& ref);
	CProjectileMovementComponent(CProjectileMovementComponent&& ref)	noexcept;

public:
	virtual ~CProjectileMovementComponent();

protected:
	FVector3		mMoveDir = FVector3::Zero;
	float			mSpeed = 200.f;
	FVector3		mVelocity;
	float			mRange = -1.f;

	// 사정거리 끝까지 날아간 뒤에 호출될 함수를 등록.
	std::function<void()>	mRangeFunction;

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
	void SetRange(float Range)
	{
		mRange = Range;
	}

	void SetSpeed(float Speed)
	{
		mSpeed = Speed;
	}

	void SetMoveDir(const FVector3& Dir)
	{
		mMoveDir = Dir;
		mMoveDir.Normalize();
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void PostRender();
	virtual void Destroy();

protected:
	virtual CProjectileMovementComponent* Clone()	const;

public:
	template <typename T>
	void SetRangeFunction(T* Obj, void(T::*Func)())
	{
		mRangeFunction = std::bind(Func, Obj);
	}
};

