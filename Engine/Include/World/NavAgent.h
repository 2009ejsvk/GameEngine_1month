#pragma once

#include "../EngineInfo.h"

class CNavAgent
{
public:
	CNavAgent();
	virtual ~CNavAgent() = 0;

protected:
	std::vector<FVector3>	mPathList;
	float			mAcceptDistance = 10.f;
	int				mPathIndex = -1;
	float			mMoveDist = 0.f;

public:
	void SetAcceptDistance(float Dist)
	{
		mAcceptDistance = Dist;
	}

	void StartPathPoint()
	{
		mPathList.clear();
		mPathIndex = 0;
	}

	void AddPathPoint(const FVector3& Pos)
	{
		mPathList.push_back(Pos);
	}

	virtual void StartPath() = 0;
};

