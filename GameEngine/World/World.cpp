#include "World.h"

CWorld::CWorld()
{
}

CWorld::~CWorld()
{
}

bool CWorld::Init()
{
	mCameraManager.reset(new CCameraManager);

	if (!mCameraManager->Init())
		return false;

	mWorldAssetManager.reset(new CWorldAssetManager);

	if (!mWorldAssetManager->Init())
		return false;

	mInput.reset(new CInput);

	if (!mInput->Init())
		return false;

	mCollision.reset(new CWorldCollision);

	if (!mCollision->Init())
		return false;

	mObjList.reserve(10000);

	mStartObjList.reserve(200);

	return true;
}

void CWorld::Update(float DeltaTime)
{
	Begin();

	// 입력은 오브젝트 업데이트 전에 해야 한다.
	mInput->Update(DeltaTime);

	auto	iter = mObjList.begin();
	auto	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (iter->second.use_count() == 0)
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetEnable())
		{
			++iter;
			continue;
		}

		iter->second->Update(DeltaTime);
		++iter;
	}

	mCameraManager->Update(DeltaTime);

	mWorldAssetManager->Update(DeltaTime);
}

void CWorld::PostUpdate(float DeltaTime)
{
	Begin();

	auto	iter = mObjList.begin();
	auto	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (iter->second.use_count() == 0)
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetEnable())
		{
			++iter;
			continue;
		}

		iter->second->PostUpdate(DeltaTime);
		++iter;
	}

	// 모든 데이터 업데이트가 완료된 후에 충돌을 진행한다.
	mCollision->Update(DeltaTime);
}

void CWorld::Render()
{
	Begin();

	auto	iter = mObjList.begin();
	auto	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (iter->second.use_count() == 0)
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetEnable())
		{
			++iter;
			continue;
		}

		iter->second->Render();
		++iter;
	}
}

void CWorld::Begin()
{
	if (!mStartObjList.empty())
	{
		size_t	Size = mStartObjList.size();

		for (size_t i = 0; i < Size; ++i)
		{
			auto	Obj = mStartObjList[i].lock();

			Obj->Begin();
		}

		mStartObjList.clear();
	}
}
