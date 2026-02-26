#include "WorldManager.h"

CWorldManager* CWorldManager::mInst = nullptr;

CWorldManager::CWorldManager()
{
}

CWorldManager::~CWorldManager()
{
}

std::weak_ptr<class CWorld> CWorldManager::GetWorld() const
{
	return mWorld;
}

void CWorldManager::InputActive()
{
	mWorld->InputActive();
}

void CWorldManager::InputDeactive()
{
	mWorld->InputDeactive();
}

bool CWorldManager::Init()
{
	mWorld = std::make_shared<CWorld>();

	mWorld->SetSelf(mWorld);

	if (!mWorld->Init())
		return false;

	mWorld->BeginManager();

	return true;
}

bool CWorldManager::Update(float DeltaTime)
{
	mWorld->Update(DeltaTime);

	return ChangeWorld();
}

bool CWorldManager::PostUpdate(float DeltaTime)
{
	mWorld->PostUpdate(DeltaTime);

	return ChangeWorld();
}

void CWorldManager::Render()
{
	mWorld->Render();
}

void CWorldManager::RenderUI()
{
	mWorld->RenderUI();
}

void CWorldManager::PostRender()
{
	mWorld->PostRender();
}

bool CWorldManager::ChangeWorld()
{
	if (mNextWorld)
	{
		mWorld = mNextWorld;

		mNextWorld.reset();

		mWorld->BeginManager();

		return true;
	}

	return false;
}
