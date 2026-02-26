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
	std::shared_ptr<CWorld>	mAsyncWorld;

public:
	std::weak_ptr<CWorld> GetWorld()	const;

public:
	void InputActive();
	void InputDeactive();
	bool Init();
	bool Update(float DeltaTime);
	bool PostUpdate(float DeltaTime);
	void Render();
	void RenderUI();
	void PostRender();

private:
	bool ChangeWorld();

public:
	template <typename T>
	std::weak_ptr<T> CreateWorld(bool Next = true)
	{
		if (Next)
		{
			mNextWorld = std::make_shared<T>();

			mNextWorld->SetSelf(mNextWorld);

			if (mWorld)
				mWorld->ClearWorld();

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

	template <typename T>
	std::weak_ptr<T> CreateAsyncWorld()
	{
		mAsyncWorld = std::make_shared<T>();

		mAsyncWorld->SetSelf(mAsyncWorld);

		if (!mAsyncWorld->Init())
			return std::weak_ptr<T>();

		//mNextWorld = mAsyncWorld;
		//mAsyncWorld.reset();

		return std::dynamic_pointer_cast<T>(mAsyncWorld);
	}

	void CompleteAsyncWorld()
	{
		if (mWorld)
			mWorld->ClearWorld();

		mNextWorld = mAsyncWorld;
		mAsyncWorld.reset();
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

