#pragma once

#include "Asset/Shader/ConstantBufferData.h"

struct FCBufferHitData
{
	FVector4	Color;
};

class CCBufferHit :
    public CConstantBufferData
{
public:
	CCBufferHit();
	CCBufferHit(const CCBufferHit& ref);
	CCBufferHit(CCBufferHit&& ref)	noexcept;
	virtual ~CCBufferHit();

private:
	FCBufferHitData	mData;

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferHit* Clone();

public:
	void SetColor(const FVector4& Color)
	{
		mData.Color = Color;
	}
};

