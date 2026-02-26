#pragma once

#include "ConstantBufferData.h"

class CCBufferUIDefault :
    public CConstantBufferData
{
public:
	CCBufferUIDefault();
	CCBufferUIDefault(const CCBufferUIDefault& ref);
	CCBufferUIDefault(CCBufferUIDefault&& ref)	noexcept;
	virtual ~CCBufferUIDefault();

private:
	FCBufferUIDefaultData	mData;

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferUIDefault* Clone();

public:
	void SetBrusnTint(const FVector4& Color)
	{
		mData.BrushTint = Color;
	}

	void SetWidgetColor(const FVector4& Color)
	{
		mData.WidgetColor = Color;
	}

	void SetBrushLTUV(const FVector2& UV)
	{
		mData.BrushLTUV = UV;
	}

	void SetBrushRBUV(const FVector2& UV)
	{
		mData.BrushRBUV = UV;
	}

	void SetAnimEnable(bool AnimEnable)
	{
		mData.BrushAnimEnable = AnimEnable ? 1 : 0;
	}

	void SetTextureEnable(bool AnimEnable)
	{
		mData.BrushTextureEnable = AnimEnable ? 1 : 0;
	}
};

