#pragma once

#include "../EngineInfo.h"
#include "ThreadNavigation.h"

class CWorldNavigation
{
	friend class CWorld;

private:
	CWorldNavigation();

public:
	~CWorldNavigation();

private:
	std::weak_ptr<class CWorld>	mWorld;
	std::weak_ptr<CWorldNavigation>	mSelf;
	std::vector<std::shared_ptr<CThreadNavigation>>	mThreadList;
	std::shared_ptr<CThreadQueue>	mNavQueue;

public:
	void AddData(int Header, int Size, unsigned char* Data);

private:
	void Begin();

public:
	bool Init();
	void Update(float DeltaTime);
	void CreateNavigationThread(int Count,
		const std::weak_ptr<class CTileMapComponent>& TileMap);
	void FindPath(const FVector3& Start, const FVector3& End,
		std::weak_ptr<class CComponent>* Agent);

private:
	void NavigationComplete(int Size, unsigned char* Data);
};

