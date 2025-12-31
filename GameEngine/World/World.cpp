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

	mObjList.reserve(10000);

	return true;
}

void CWorld::Update(float DeltaTime)
{
	// 입력은 오브젝트 업데이트 전에 해야 한다.
	mInput->Update(DeltaTime);

	auto	iter = mObjList.begin();
	auto	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
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
	auto	iter = mObjList.begin();
	auto	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			continue;
		}

		iter->second->PostUpdate(DeltaTime);
		++iter;
	}
}

void CWorld::Render()
{
	auto	iter = mObjList.begin();
	auto	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			continue;
		}

		iter->second->Render();
		++iter;
	}
}