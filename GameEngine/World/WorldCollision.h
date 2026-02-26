#pragma once

#include "../EngineInfo.h"
#include "CollisionQuadTree.h"

class CWorldCollision
{
	friend class CWorld;

private:
	CWorldCollision();

public:
	~CWorldCollision();

private:
	std::weak_ptr<class CWorld>	mWorld;
	std::shared_ptr<CCollisionQuadTree>	mQuadTree;
	std::list<std::weak_ptr<class CCollider>>	mColliderList;
	float	mInterval = 0.f;
	float	mIntervalTime = 0.f;
	std::weak_ptr<class CCollider>	mHoveredObject;

public:
	void SetWorld(const std::weak_ptr<class CWorld>& World)
	{
		mWorld = World;
	}

	void SetInterval(float Interval)
	{
		mInterval = Interval;
	}
	void AddCollider(const std::weak_ptr<class CCollider>& Collider);

public:
	bool Init();
	void Update(float DeltaTime);
	void Render();
	void ReturnNodePool();

private:
	void UpdateInfo();
};

