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
	bool			mStatic = false;
	FCollisionProfile* mProfile = nullptr;

	// 이 충돌체와 부딪친 충돌체는 서로 떨어지기 전까지
	// 이 맵에 보관해둔다.
	std::unordered_map<CCollider*, std::weak_ptr<CCollider>>	mCollisionObjectMap;

	std::weak_ptr<class CShader>	mShader;
	std::weak_ptr<class CMesh>		mMesh;
	std::shared_ptr<class CCBufferTransform>	mTransformCBuffer;
	std::shared_ptr<class CCBufferCollider>	mColliderCBuffer;

	// Overlap일 경우 충돌 시작과 충돌 종료 시 호출될 함수
	std::function<void(const FVector3&, CCollider*)>	
		mCollisionBeginFunc;
	std::function<void(CCollider*)>	mCollisionEndFunc;

	// Block일 경우 충돌 시 호출될 함수.
	std::function<void(const FVector3&, CCollider*)>	mCollisionHitFunc;
	
	// 마우스와 충돌 시 호출될 함수
	std::function<void(const FVector3&)>	mCollisionMouseBeginFunc;
	std::function<void(const FVector3&)>	mCollisionMouseEndFunc;

public:
	const FVector3& GetMin()	const
	{
		return mMin;
	}

	const FVector3& GetMax()	const
	{
		return mMax;
	}

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

	bool GetStatic()	const
	{
		return mStatic;
	}

	void SetStatic(bool Static)
	{
		mStatic = Static;
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
	// 충돌 시 정보가 변경될 수도 있기 때문에 충돌 시 정보가 변경되면
	// 이 함수를 호출해서 충돌체의 정보도 변경시켜줄 수 있게 한다.
	virtual void UpdateInfo() = 0;

protected:
	virtual CCollider* Clone()	const = 0;

public:
	void CallCollisionBegin(const FVector3& HitPoint,
		const std::weak_ptr<CCollider>& Dest);
	void CallCollisionEnd(CCollider* Dest);
	void CallCollisionHit(const FVector3& HitPoint,
		const std::weak_ptr<CCollider>& Dest);
	void CollisionHitEnd();
	void CallCollisionMouseBegin(const FVector3& MousePos);
	void CallCollisionMouseEnd(const FVector3& MousePos);

public:
	virtual bool Collision(FVector3& HitPoint,
		std::shared_ptr<CCollider> Dest) = 0;
	virtual bool CollisionManifold(FCollisionManifold& Result,
		std::shared_ptr<CCollider> Dest) = 0;
	virtual bool CollisionMouse(const FVector2& MousePos) = 0;

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

	template <typename T>
	void SetCollisionHitFunction(T* Obj,
		void(T::* Func)(const FVector3&, CCollider*))
	{
		mCollisionHitFunc = std::bind(Func, Obj,
			std::placeholders::_1, std::placeholders::_2);
	}

	template <typename T>
	void SetCollisionMouseBeginFunction(T* Obj,
		void(T::* Func)(const FVector3&))
	{
		mCollisionMouseBeginFunc = std::bind(Func, Obj,
			std::placeholders::_1);
	}

	template <typename T>
	void SetCollisionMouseEndFunction(T* Obj,
		void(T::* Func)(const FVector3&))
	{
		mCollisionMouseEndFunc = std::bind(Func, Obj,
			std::placeholders::_1);
	}
};

