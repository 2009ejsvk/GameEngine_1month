#pragma once

#include "ConstantBufferData.h"

class CCBufferBlur :
    public CConstantBufferData
{
public:
	CCBufferBlur();
	CCBufferBlur(const CCBufferBlur& ref);
	CCBufferBlur(CCBufferBlur&& ref)	noexcept;
	virtual ~CCBufferBlur();

private:
	FCBufferBlur	mData;

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferBlur* Clone();

public:
	void SetTexelSize(const FVector2& TexelSize)
	{
		mData.TexelSize = TexelSize;
	}

	void SetCount(int Count)
	{
		mData.Count = Count;
	}

	void SetWeight(const std::vector<float>& WeightArray)
	{
		if (WeightArray.size() > MAX_BLUR_COUNT + 1)
			return;

		memcpy(mData.BlurWeight, &WeightArray[0],
			sizeof(float) * WeightArray.size());
	}
};

