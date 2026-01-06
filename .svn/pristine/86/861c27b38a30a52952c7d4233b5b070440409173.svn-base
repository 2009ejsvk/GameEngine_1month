#pragma once

#include "EngineInfo.h"

class CTimer
{
private:
	static LARGE_INTEGER	mFrequency;	// 초당 카운트 수
	static LARGE_INTEGER	mPrevCount;	// 이전 프레임의 카운트 수
	static float	mDeltaTime;	// 이전프레임 시작에서 이전 프레임 끝까지의 시간

public:
	static float GetDeltaTime()
	{
		return mDeltaTime;
	}

public:
	static void Init();
	static float Update();
};

