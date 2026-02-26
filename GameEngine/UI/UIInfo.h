#pragma once

#include "../EngineInfo.h"

struct FUIBrush
{
	std::weak_ptr<class CTexture>	Texture;
	FVector4	Tint = FVector4::White;
	std::vector<FTextureFrame>	AnimationFrames;
	bool		AnimationEnable = false;
	bool		FrameAnimationEnable = false;
	int			Frame = 0;
	float		Time = 0.f;
	float		FrameTime = 1.f;
	float		PlayRate = 1.f;
	float		PlayTime = 1.f;

public:
	void PlayAnimation(float DeltaTime)
	{
		if (!AnimationEnable)
			return;

		Time += DeltaTime * PlayRate;

		if (Time >= FrameTime)
		{
			Time -= FrameTime;

			++Frame;

			if (Frame == (int)AnimationFrames.size())
				Frame = 0;
		}
	}
};
