#pragma once

#include "Object/GameObject.h"

class CMonsterSpawnPoint :
    public CGameObject
{
	friend class CWorld;
	friend class CObject;

protected:
	CMonsterSpawnPoint();
	CMonsterSpawnPoint(const CMonsterSpawnPoint& ref);
	CMonsterSpawnPoint(CMonsterSpawnPoint&& ref)	noexcept;

public:
	virtual ~CMonsterSpawnPoint();

private:
	size_t	mSpawnClass = 0;
	float	mTime = 0.f;
	float	mSpawnTime = 1.f;
	std::weak_ptr<class CMonster>	mSpawnMonster;


public:
	void SetSpawnTime(float SpawnTime)
	{
		mSpawnTime = SpawnTime;
	}

	template <typename T>
	void SetSpawnClass()
	{
		mSpawnClass = typeid(T).hash_code();
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
};

