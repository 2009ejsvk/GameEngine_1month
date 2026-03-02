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
	std::weak_ptr<class CTileMapComponent>	mTileMap;

	struct FNavCacheEntry
	{
		std::vector<unsigned char>	BlockedMask;
		std::vector<int>			GoalIndices;
		std::string					ResolvedTargetObjectName;
	};
	std::unordered_map<std::string, FNavCacheEntry>	mFrameCache;

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
		std::weak_ptr<class CComponent>* Agent,
		const std::string& TargetObjectName = "",
		unsigned int RequestId = 0);

private:
	void NavigationComplete(int Size, unsigned char* Data);
};

