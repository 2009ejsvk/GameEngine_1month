#pragma once

#include "ConstantBufferData.h"

class CCBufferCollider :
    public CConstantBufferData
{
public:
	CCBufferCollider();
	CCBufferCollider(const CCBufferCollider& ref);
	CCBufferCollider(CCBufferCollider&& ref)	noexcept;
	virtual ~CCBufferCollider();

private:
	FCBufferColliderData	mData;

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferCollider* Clone();

public:
	void SetColor(const FVector4& Color)
	{
		mData.Color = Color;
	}
};

