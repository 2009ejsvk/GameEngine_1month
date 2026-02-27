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
	std::string		mPathTargetObjectName;
	unsigned int	mPathRequestId = 0;

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

	void SetPathTargetObjectName(const std::string& TargetObjectName)
	{
		mPathTargetObjectName = TargetObjectName;
	}

	const std::string& GetPathTargetObjectName()	const
	{
		return mPathTargetObjectName;
	}

	unsigned int AdvancePathRequestId()
	{
		++mPathRequestId;

		if (mPathRequestId == 0)
			mPathRequestId = 1;

		return mPathRequestId;
	}

	bool IsExpectedPathRequestId(unsigned int RequestId)	const
	{
		return RequestId == mPathRequestId;
	}

	virtual void StartPath() = 0;
};

