#pragma once

#include "Component.h"

enum class EComponentRender : unsigned char
{
	None,
	Render
};

class CSceneComponent :
    public CComponent
{
	friend class CGameObject;
	friend class CObject;

protected:
	CSceneComponent();
	CSceneComponent(const CSceneComponent& ref);
	CSceneComponent(CSceneComponent&& ref)	noexcept;

public:
	virtual ~CSceneComponent();

protected:
	std::weak_ptr<CSceneComponent>	mParent;
	EComponentRender	mRenderType = EComponentRender::None;
	std::vector<std::weak_ptr<CSceneComponent>>	mChildList;
	int	mRenderLayer = 0;

public:
	void SetRenderLayer(int Layer)
	{
		mRenderLayer = Layer;
	}

public:
	EComponentRender GetRenderType()	const
	{
		return mRenderType;
	}

	int GetRenderLayer()	const
	{
		return mRenderLayer;
	}

public:
	virtual void Begin();
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void PostRender();
	virtual void Destroy();

protected:
	virtual CSceneComponent* Clone()	const;

protected:
	bool		mInheritScale = true;
	bool		mInheritRot = true;

	FVector3	mPivot;

	FVector3	mRelativeScale = FVector3::One;
	FVector3	mRelativeRot;
	FVector3	mRelativePos;

	FVector3	mWorldScale = FVector3::One;
	FVector3	mWorldRot;
	FVector3	mWorldPos;
	FVector3	mVelocity;

	FVector3	mWorldAxis[EAxis::End] =
	{
		FVector3::Axis[EAxis::X],
		FVector3::Axis[EAxis::Y],
		FVector3::Axis[EAxis::Z],
	};

	FMatrix	mScaleMatrix;
	FMatrix	mRotMatrix;
	FMatrix	mTranslateMatrix;
	FMatrix	mWorldMatrix;

public:
	void AddChild(std::weak_ptr<CSceneComponent> Child);
	void UpdateTransform();

public:
	const FVector3& GetPivot()	const
	{
		return mPivot;
	}

	const FVector3& GetVelocity()	const
	{
		return mVelocity;
	}

	float GetSpeed()	const
	{
		return mVelocity.Length();
	}

	const FVector3& GetAxis(EAxis::Type Axis)	const
	{
		return mWorldAxis[Axis];
	}

	const FVector3& GetRelativeScale()	const
	{
		return mRelativeScale;
	}

	const FVector3& GetRelativeRot()	const
	{
		return mRelativeRot;
	}

	const FVector3& GetRelativePos()	const
	{
		return mRelativePos;
	}

	const FVector3& GetWorldScale()	const
	{
		return mWorldScale;
	}

	const FVector3& GetWorldRot()	const
	{
		return mWorldRot;
	}

	const FVector3& GetWorldPos()	const
	{
		return mWorldPos;
	}

public:
	void SetInheritScale(bool Scale);
	void SetInheritRot(bool Rot);

	void SetPivot(const FVector3& Pivot)
	{
		mPivot = Pivot;
	}

	void SetPivot(float x, float y, float z)
	{
		mPivot = FVector3(x, y, z);
	}

	void SetPivot(float x, float y)
	{
		mPivot = FVector3(x, y, 0.f);
	}

	void SetRelativeScale(const FVector3& Scale);
	void SetRelativeScale(const FVector2& Scale);
	void SetRelativeScale(float x, float y, float z);
	void SetRelativeScale(float x, float y);

	void AddRelativeScale(const FVector3& Scale);
	void AddRelativeScale(const FVector2& Scale);
	void AddRelativeScale(float x, float y, float z);
	void AddRelativeScale(float x, float y);

	void SetRelativeRotation(const FVector3& Rot);
	void SetRelativeRotation(const FVector2& Rot);
	void SetRelativeRotation(float x, float y, float z);
	void SetRelativeRotation(float x, float y);
	void SetRelativeRotationX(float x);
	void SetRelativeRotationY(float y);
	void SetRelativeRotationZ(float z);

	void AddRelativeRotation(const FVector3& Rot);
	void AddRelativeRotation(const FVector2& Rot);
	void AddRelativeRotation(float x, float y, float z);
	void AddRelativeRotation(float x, float y);
	void AddRelativeRotationX(float x);
	void AddRelativeRotationY(float y);
	void AddRelativeRotationZ(float z);

	void SetRelativePos(const FVector3& Pos);
	void SetRelativePos(const FVector2& Pos);
	void SetRelativePos(float x, float y, float z);
	void SetRelativePos(float x, float y);

	void AddRelativePos(const FVector3& Pos);
	void AddRelativePos(const FVector2& Pos);
	void AddRelativePos(float x, float y, float z);
	void AddRelativePos(float x, float y);

	void InheritScale();
	void InheritRotation();
	void InheritPos();

	void SetWorldScale(const FVector3& Scale);
	void SetWorldScale(const FVector2& Scale);
	void SetWorldScale(float x, float y, float z);
	void SetWorldScale(float x, float y);

	void AddWorldScale(const FVector3& Scale);
	void AddWorldScale(const FVector2& Scale);
	void AddWorldScale(float x, float y, float z);
	void AddWorldScale(float x, float y);

	void SetWorldRotation(const FVector3& Rot);
	void SetWorldRotation(const FVector2& Rot);
	void SetWorldRotation(float x, float y, float z);
	void SetWorldRotation(float x, float y);
	void SetWorldRotationX(float x);
	void SetWorldRotationY(float y);
	void SetWorldRotationZ(float z);

	void AddWorldRotation(const FVector3& Rot);
	void AddWorldRotation(const FVector2& Rot);
	void AddWorldRotation(float x, float y, float z);
	void AddWorldRotation(float x, float y);
	void AddWorldRotationX(float x);
	void AddWorldRotationY(float y);
	void AddWorldRotationZ(float z);

	void SetWorldPos(const FVector3& Pos);
	void SetWorldPos(const FVector2& Pos);
	void SetWorldPos(float x, float y, float z);
	void SetWorldPos(float x, float y);

	void AddWorldPos(const FVector3& Pos);
	void AddWorldPos(const FVector2& Pos);
	void AddWorldPos(float x, float y, float z);
	void AddWorldPos(float x, float y);

	void UpdateChildWorldPos(const FVector3& Dist);

	void InheritWorldScale();
	void InheritWorldRotation();
	void InheritWorldPos();
};

