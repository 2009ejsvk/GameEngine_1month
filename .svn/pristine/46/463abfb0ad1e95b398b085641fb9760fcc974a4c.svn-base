#include "Animation2DSequence.h"

CAnimation2DSequence::CAnimation2DSequence()
{
}

CAnimation2DSequence::CAnimation2DSequence(
	const CAnimation2DSequence& ref)
{
}

CAnimation2DSequence::CAnimation2DSequence(CAnimation2DSequence&& ref)
{
}

CAnimation2DSequence::~CAnimation2DSequence()
{
}

const std::string& CAnimation2DSequence::GetName()	const
{
	auto	Anim = mAnimation.lock();

	if (!Anim)
		return std::string();

	return Anim->GetName();
}

void CAnimation2DSequence::Update(float DeltaTime)
{
	if (mAnimation.expired())
		return;

	auto	Anim = mAnimation.lock();

	mTime += DeltaTime * mPlayRate;

	if (mTime >= mFrameTime)
	{
		mTime -= mFrameTime;

		if (mReverse)
		{
			--mFrame;

			if (mFrame < 0)
			{
				if (mFinishNotify)
					mFinishNotify();

				if (mLoop)
				{
					mFrame = Anim->GetFrameCount() - 1;

					size_t	Size = mNotifyArray.size();

					for (size_t i = 0; i < Size; ++i)
					{
						mNotifyArray[i].Call = false;
					}
				}

				else
					mFrame = 0;
			}
		}

		else
		{
			// 프레임을 증가시킨다.
			++mFrame;

			// 프레임이 프레임 수만큼 도달했다면
			if (mFrame == Anim->GetFrameCount())
			{
				if (mFinishNotify)
					mFinishNotify();

				// 반복재생 해야 할 경우
				if (mLoop)
				{
					mFrame = 0;

					size_t	Size = mNotifyArray.size();

					for (size_t i = 0; i < Size; ++i)
					{
						mNotifyArray[i].Call = false;
					}
				}

				// 반복 재생이 아닐 경우 마지막 프레임으로 고정.
				else
					mFrame = Anim->GetFrameCount() - 1;
			}
		}
	}

	CallNotify();
}

CAnimation2DSequence* CAnimation2DSequence::Clone()	const
{
	return new CAnimation2DSequence(*this);
}

void CAnimation2DSequence::Clear()
{
	size_t	Size = mNotifyArray.size();

	for (size_t i = 0; i < Size; ++i)
	{
		mNotifyArray[i].Call = false;
	}

	auto	Anim = mAnimation.lock();

	if (Anim)
	{
		if (mReverse)
			mFrame = Anim->GetFrameCount() - 1;

		else
			mFrame = 0;

		mTime = 0.f;
	}
}

void CAnimation2DSequence::CallNotify()
{
	size_t	Size = mNotifyArray.size();

	for (size_t i = 0; i < Size; ++i)
	{
		if (mNotifyArray[i].Frame == mFrame &&
			!mNotifyArray[i].Call)
		{
			mNotifyArray[i].Call = true;

			if (mNotifyArray[i].Function)
				mNotifyArray[i].Function();
		}
	}
}
