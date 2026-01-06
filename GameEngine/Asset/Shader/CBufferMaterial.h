#pragma once

#include "ConstantBufferData.h"

class CCBufferMaterial :
    public CConstantBufferData
{
public:
	CCBufferMaterial();
	CCBufferMaterial(const CCBufferMaterial& ref);
	CCBufferMaterial(CCBufferMaterial&& ref)	noexcept;
	virtual ~CCBufferMaterial();

private:
	FCBufferMaterialData	mData;

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferMaterial* Clone();

public:
	void SetBaseColor(const FVector4& Color)
	{
		mData.BaseColor = Color;
	}

	void SetOpacity(float Opacity)
	{
		mData.Opacity = Opacity;
	}
};

