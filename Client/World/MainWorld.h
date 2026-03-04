#pragma once

#include "World/World.h"
#include <string>
#include <vector>

class CMainWorld :
    public CWorld
{
public:
	CMainWorld();
	virtual ~CMainWorld();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);

private:
	int mSpawnedNpcCount = 0;
	float mNpcSpawnAccum = 0.f;
	float mCitizenReassignAccum = 0.f;

private:
	void SpawnCitizenOrb();
	void ReassignCitizenNeeds();
	void CollectCurrentBuildingNames(
		std::vector<std::string>& OutBuildingNames);
	void CollectHomeBuildingNames(std::vector<std::string>& OutNames);
	void CollectWorkBuildingNames(std::vector<std::string>& OutNames);
	void CollectFoodBuildingNames(std::vector<std::string>& OutNames);
	void CollectEntertainmentBuildingNames(std::vector<std::string>& OutNames);
	void LoadCitizenAnimation2D();
	void LoadAnimation2D();
	void LoadSound();
	void CreateUI();
};
