#include "World.h"

CWorld::CWorld()
{
}

CWorld::~CWorld()
{
}

void CWorld::InputActive()
{
	mInput->DeviceAcquire();
}

void CWorld::InputDeactive()
{
	mInput->DeviceUnAcquire();
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

	mInput->mWorld = mSelf;

	if (!mInput->Init())
		return false;

	mCollision.reset(new CWorldCollision);

	mCollision->SetWorld(mSelf);

	if (!mCollision->Init())
		return false;

	mUIManager.reset(new CWorldUIManager);

	mUIManager->mWorld = mSelf;
	mUIManager->mSelf = mUIManager;

	if (!mUIManager->Init())
		return false;

	mNavigation.reset(new CWorldNavigation);

	mNavigation->mWorld = mSelf;
	mNavigation->mSelf = mNavigation;

	if (!mNavigation->Init())
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

	mNavigation->Update(DeltaTime);

	mUIManager->Update(DeltaTime);
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

	iter = mObjList.begin();
	iterEnd = mObjList.end();

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

		iter->second->UpdateTransform();
		++iter;
	}
}

void CWorld::Render()
{
	Begin();

#ifdef _DEBUG

	mCollision->Render();

#endif // _DEBUG

	mCollision->ReturnNodePool();
}

void CWorld::PostRender()
{
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

		iter->second->PostRender();
		++iter;
	}
}

void CWorld::RenderUI()
{
	mUIManager->Render();
}

void CWorld::ClearWorld()
{
	mWorldAssetManager->ClearAsset();
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

void CWorld::BeginManager()
{
	mNavigation->Begin();
}
