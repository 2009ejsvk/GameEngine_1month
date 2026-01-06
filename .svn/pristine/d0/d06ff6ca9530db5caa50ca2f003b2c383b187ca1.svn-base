#pragma once

#include "../Asset/Animation2D/Animation2D.h"

struct FAnimation2DNotify
{
	std::string	Name;
	bool	Call = false;
	int		Frame = 0;
	std::function<void()>	Function;
};

class CAnimation2DSequence
{
	friend class CAnimation2DComponent;

private:
	CAnimation2DSequence();
	CAnimation2DSequence(const CAnimation2DSequence& ref);
	CAnimation2DSequence(CAnimation2DSequence&& ref);

public:
	~CAnimation2DSequence();

private:
	std::weak_ptr<CAnimation2D>	mAnimation;

	// 여러 프레임중 몇번째 프레임인지를 판단.
	int		mFrame = 0;

	// 프레임을 증가 혹은 감소시키기 위해 누적되는 시간.
	float	mTime = 0.f;

	// 한 프레임이 변경되어야 하는 시간
	float	mFrameTime = 0.f;

	// 전체 프레임이 동작할 시간.
	float	mPlayTime = 1.f;

	// 재생속도 배율
	float	mPlayRate = 1.f;

	// 애니메이션을 반복재생 시킬 것인지 여부
	bool	mLoop = false;

	// 역재생
	bool	mReverse = false;

	// 좌/우 대칭
	bool	mSymmetry = false;

	std::vector<FAnimation2DNotify>	mNotifyArray;

	std::function<void()>	mFinishNotify;

public:
	EAnimation2DTextureType GetAnimationTextureType()	const
	{
		auto	Anim = mAnimation.lock();

		if (Anim)
			return Anim->GetAnimationTextureType();

		return EAnimation2DTextureType::None;
	}

	bool GetReverse()	const
	{
		return mReverse;
	}

	int GetFrameCount()	const
	{
		auto	Anim = mAnimation.lock();

		if (Anim)
			return Anim->GetFrameCount();

		return 0;
	}

	const std::weak_ptr<CAnimation2D>& GetAsset()	const
	{
		return mAnimation;
	}

	bool GetSymmetry()	const
	{
		return mSymmetry;
	}

	int GetFrame()	const
	{
		return mFrame;
	}

	float GetTime()	const
	{
		return mTime;
	}

	float GetFrameTime()	const
	{
		return mFrameTime;
	}

	const std::string& GetName()	const;

public:
	void SetAnimation2D(const std::weak_ptr<CAnimation2D>& Anim)
	{
		mAnimation = Anim;

		auto	Animation = mAnimation.lock();

		if (Animation)
			mFrameTime = mPlayTime / Animation->GetFrameCount();
	}

	void SetPlayTime(float Time)
	{
		mPlayTime = Time;

		auto	Anim = mAnimation.lock();

		if (Anim)
			mFrameTime = mPlayTime / Anim->GetFrameCount();
	}

	void SetPlayRate(float Rate)
	{
		mPlayRate = Rate;
	}

	void SetLoop(bool Loop)
	{
		mLoop = Loop;
	}

	void SetReverse(bool Reverse)
	{
		mReverse = Reverse;

		auto	Anim = mAnimation.lock();

		if (Anim)
		{
			if (mReverse)
				mFrame = Anim->GetFrameCount() - 1;

			else
				mFrame = 0;
		}
	}

	void SetAnimationSymmetry(bool Symmetry)
	{
		mSymmetry = Symmetry;
	}

public:
	void Update(float DeltaTime);
	CAnimation2DSequence* Clone()	const;
	void Clear();

private:
	void CallNotify();

public:
	template <typename T>
	void AddNotify(const std::string& Name, int Frame,
		T* Obj, void(T::* Func)())
	{
		FAnimation2DNotify	Notify;
		Notify.Name = Name;
		Notify.Frame = Frame;
		Notify.Function = std::bind(Func, Obj);

		mNotifyArray.push_back(Notify);
	}

	template <typename T>
	void SetFinishNotify(T* Obj, void (T::* Func)())
	{
		mFinishNotify = std::bind(Func, Obj);
	}
};

