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

bool CWorldManager::Init()
{
	mWorld = std::make_shared<CWorld>();

	mWorld->SetSelf(mWorld);

	if (!mWorld->Init())
		return false;

	return true;
}

void CWorldManager::Update(float DeltaTime)
{
	mWorld->Update(DeltaTime);
}

void CWorldManager::PostUpdate(float DeltaTime)
{
	mWorld->PostUpdate(DeltaTime);
}

void CWorldManager::Render()
{
	mWorld->Render();
}
