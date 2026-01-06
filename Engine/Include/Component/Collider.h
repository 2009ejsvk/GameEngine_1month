#pragma once

#include "SceneComponent.h"

class CCollider abstract :
    public CSceneComponent
{
	friend class CGameObject;
	friend class CObject;

protected:
	CCollider();
	CCollider(const CCollider& ref);
	CCollider(CCollider&& ref)	noexcept;

public:
	virtual ~CCollider();

protected:
	EColliderType	mColliderType;
	FVector3		mMin;
	FVector3		mMax;
	FVector3		mRenderScale;
	bool			mDebugDraw = false;
	bool			mCollision = false;
	FCollisionProfile* mProfile = nullptr;

	std::weak_ptr<class CShader>	mShader;
	std::weak_ptr<class CMesh>		mMesh;
	std::shared_ptr<class CCBufferTransform>	mTransformCBuffer;
	std::shared_ptr<class CCBufferCollider>	mColliderCBuffer;

public:
	EColliderType GetColliderType()	const
	{
		return mColliderType;
	}

	bool GetDebugDraw()	const
	{
		return mDebugDraw;
	}

	FCollisionProfile* GetCollisionProfile()	const
	{
		return mProfile;
	}

public:
	virtual void SetDebugDraw(bool DebugDraw);
	void SetCollisionProfile(const std::string& Name);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();

protected:
	virtual CCollider* Clone()	const = 0;

public:
	virtual bool Collision(FVector3& HitPoint,
		std::shared_ptr<CCollider> Dest) = 0;
};

