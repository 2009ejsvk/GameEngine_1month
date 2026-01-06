#include "Timer.h"

LARGE_INTEGER CTimer::mFrequency;
LARGE_INTEGER CTimer::mPrevCount;
float CTimer::mDeltaTime = 0.f;

void CTimer::Init()
{
	// 1초당 카운트 수를 구한다.
	QueryPerformanceFrequency(&mFrequency);

	// 이전 프레임의 카운트를 구한다.
	QueryPerformanceCounter(&mPrevCount);
}

float CTimer::Update()
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


	return mDeltaTime;
}

