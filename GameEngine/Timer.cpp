#include "Timer.h"
#include "Device.h"

LARGE_INTEGER CTimer::mFrequency;
LARGE_INTEGER CTimer::mPrevCount;
float CTimer::mDeltaTime = 0.f;
float CTimer::mFPS = 0.f;
float CTimer::mFPSTime = 0.f;
int CTimer::mFPSTick = 0;
float CTimer::mFrameMsAccum = 0.f;
float CTimer::mPresentMsAccum = 0.f;
float CTimer::mCPUFPS = 0.f;
float CTimer::mCPUFrameMs = 0.f;
float CTimer::mPresentMs = 0.f;

void CTimer::Init()
{
	// 1초당 카운트 수를 구한다.
	QueryPerformanceFrequency(&mFrequency);

	// 이전 프레임의 카운트를 구한다.
	QueryPerformanceCounter(&mPrevCount);
}

float CTimer::Update(HWND hWnd)
{
	LARGE_INTEGER	Count;

	// 현재 프레임의 카운트를 구한다.
	QueryPerformanceCounter(&Count);

	// 현재 프레임의 카운트와 이전 프레임의 카운트의 차이를 구하고
	// 이 차이를 초당 카운트로 나누어서 흐른 시간을 구한다.
	mDeltaTime = (Count.QuadPart - mPrevCount.QuadPart) / (float)mFrequency.QuadPart;

	// 현재 프레임의 카운트를 이전 프레임의 카운트 저장하여 다음 프레임에서
	// 시간을 구할 수 있게 해준다.
	mPrevCount = Count;

	mFPSTime += mDeltaTime;
	mFrameMsAccum += mDeltaTime * 1000.f;
	mPresentMsAccum += CDevice::GetInst()->GetLastPresentTimeMs();

	++mFPSTick;

	if (mFPSTick == 60)
	{
		const float AvgFrameMs = mFrameMsAccum / (float)mFPSTick;
		const float AvgPresentMs = mPresentMsAccum / (float)mFPSTick;
		float AvgCPUFrameMs = AvgFrameMs - AvgPresentMs;

		if (AvgCPUFrameMs < 0.f)
		{
			AvgCPUFrameMs = 0.f;
		}

		mFPS = AvgFrameMs > 0.f ? (1000.f / AvgFrameMs) : 0.f;
		mCPUFPS = AvgCPUFrameMs > 0.0001f ?
			(1000.f / AvgCPUFrameMs) : 0.f;
		mCPUFrameMs = AvgCPUFrameMs;
		mPresentMs = AvgPresentMs;
		mFPSTime = 0.f;
		mFPSTick = 0;
		mFrameMsAccum = 0.f;
		mPresentMsAccum = 0.f;

		char	FPSText[160] = {};

		if (mCPUFPS > 0.f)
		{
			sprintf_s(FPSText,
				"FPS: %.2f | CPU FPS: %.2f | CPU: %.2fms | Present: %.2fms",
				mFPS, mCPUFPS, mCPUFrameMs, mPresentMs);
		}

		else
		{
			sprintf_s(FPSText,
				"FPS: %.2f | CPU: %.2fms | Present: %.2fms",
				mFPS, mCPUFrameMs, mPresentMs);
		}

		if (hWnd)
		{
			SetWindowTextA(hWnd, FPSText);
		}

#ifdef _DEBUG
		OutputDebugStringA(FPSText);
		OutputDebugStringA("\n");
#endif // _DEBUG
	}


	return mDeltaTime;
}

