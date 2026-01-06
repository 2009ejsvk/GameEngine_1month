#pragma once

#include "SceneComponent.h"

enum class ECameraProjectionType : unsigned char
{
	Perspective,
	Ortho
};

class CCameraComponent :
    public CSceneComponent
{
	friend class CGameObject;
	friend class CObject;

protected:
	CCameraComponent();
	CCameraComponent(const CCameraComponent& ref);
	CCameraComponent(CCameraComponent&& ref)	noexcept;

public:
	virtual ~CCameraComponent();

protected:
	ECameraProjectionType	mProjType = ECameraProjectionType::Perspective;
	FMatrix	mViewMatrix;
	FMatrix	mProjMatrix;

	float	mViewAngle = 90.f;
	float	mWidth = 1280.f;
	float	mHeight = 720.f;
	float	mViewDistance = 1000.f;

public:
	const FMatrix& GetViewMatrix()	const
	{
		return mViewMatrix;
	}

	const FMatrix& GetProjMatrix()	const
	{
		return mProjMatrix;
	}

	void SetProjection(ECameraProjectionType Type, float ViewAngle,
		float Width, float Height, float ViewDistance);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);

protected:
	virtual CCameraComponent* Clone()	const;
};

