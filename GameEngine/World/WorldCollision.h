#pragma once

#include "../EngineInfo.h"

class CWorldCollision
{
	friend class CWorld;

private:
	CWorldCollision();

public:
	~CWorldCollision();

private:
	std::list<std::weak_ptr<class CCollider>>	mColliderList;
	float	mInterval = 0.f;
	float	mIntervalTime = 0.f;

public:
	void SetInterval(float Interval)
	{
		mInterval = Interval;
	}
	void AddCollider(const std::weak_ptr<class CCollider>& Collider);

public:
	bool Init();
	void Update(float DeltaTime);
};

