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

	// 이 충돌체와 부딪친 충돌체는 서로 떨어지기 전까지
	// 이 맵에 보관해둔다.
	std::unordered_map<CCollider*, std::weak_ptr<CCollider>>	mCollisionObjectMap;

	std::weak_ptr<class CShader>	mShader;
	std::weak_ptr<class CMesh>		mMesh;
	std::shared_ptr<class CCBufferTransform>	mTransformCBuffer;
	std::shared_ptr<class CCBufferCollider>	mColliderCBuffer;

	std::function<void(const FVector3&, CCollider*)>	
		mCollisionBeginFunc;
	std::function<void(CCollider*)>	mCollisionEndFunc;

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

	bool CheckCollisionObject(CCollider* Collider);
	void EraseCollisionObject(CCollider* Collider);

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
	void CallCollisionBegin(const FVector3& HitPoint,
		const std::weak_ptr<CCollider>& Dest);
	void CallCollisionEnd(CCollider* Dest);

public:
	virtual bool Collision(FVector3& HitPoint,
		std::shared_ptr<CCollider> Dest) = 0;

public:
	template <typename T>
	void SetCollisionBeginFunction(T* Obj,
		void(T::* Func)(const FVector3&, CCollider*))
	{
		mCollisionBeginFunc = std::bind(Func, Obj,
			std::placeholders::_1, std::placeholders::_2);
	}

	template <typename T>
	void SetCollisionEndFunction(T* Obj,
		void(T::* Func)(CCollider*))
	{
		mCollisionEndFunc = std::bind(Func, Obj,
			std::placeholders::_1);
	}
};

