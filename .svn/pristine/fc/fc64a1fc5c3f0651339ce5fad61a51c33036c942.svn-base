#pragma once

#include "../EngineInfo.h"
#include "World.h"

class CWorldManager
{
private:
	CWorldManager();
	~CWorldManager();

private:
	std::shared_ptr<CWorld>	mWorld;
	std::shared_ptr<CWorld>	mNextWorld;

public:
	std::weak_ptr<CWorld> GetWorld()	const;

public:
	bool Init();
	void Update(float DeltaTime);
	void PostUpdate(float DeltaTime);
	void Render();

public:
	template <typename T>
	std::weak_ptr<T> CreateWorld(bool Next = true)
	{
		if (Next)
		{
			mNextWorld = std::make_shared<T>();

			mNextWorld->SetSelf(mNextWorld);

			if (!mNextWorld->Init())
				return std::weak_ptr<T>();

			return std::dynamic_pointer_cast<T>(mNextWorld);
		}

		mWorld = std::make_shared<T>();

		mWorld->SetSelf(mWorld);

		if (!mWorld->Init())
			return std::weak_ptr<T>();

		return std::dynamic_pointer_cast<T>(mWorld);
	}

private:
	static CWorldManager* mInst;

public:
	static CWorldManager* GetInst()
	{
		if (!mInst)
			mInst = new CWorldManager;
		return mInst;
	}

	static void DestroyInst()
	{
		SAFE_DELETE(mInst);
	}
};

