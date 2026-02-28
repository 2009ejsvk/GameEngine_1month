#pragma once

#include "EngineInfo.h"

class CTimer
{
private:
	static LARGE_INTEGER	mFrequency;	// 초당 카운트 수
	static LARGE_INTEGER	mPrevCount;	// 이전 프레임의 카운트 수
	static float	mDeltaTime;	// 이전프레임 시작에서 이전 프레임 끝까지의 시간

	static float	mFPS;
	static float	mFPSTime;
	static int		mFPSTick;
	static float	mFrameMsAccum;
	static float	mPresentMsAccum;
	static float	mCPUFPS;
	static float	mCPUFrameMs;
	static float	mPresentMs;

public:
	static float GetDeltaTime()
	{
		return mDeltaTime;
	}

	static float GetFPS()
	{
		return mFPS;
	}

	static float GetCPUFPS()
	{
		return mCPUFPS;
	}

	static float GetCPUFrameMs()
	{
		return mCPUFrameMs;
	}

	static float GetPresentMs()
	{
		return mPresentMs;
	}

public:
	static void Init();
	static float Update(HWND hWnd);
};

