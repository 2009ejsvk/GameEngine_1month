#pragma once

#include "ConstantBufferData.h"

class CCBufferAnimation2D :
    public CConstantBufferData
{
public:
	CCBufferAnimation2D();
	CCBufferAnimation2D(const CCBufferAnimation2D& ref);
	CCBufferAnimation2D(CCBufferAnimation2D&& ref)	noexcept;
	virtual ~CCBufferAnimation2D();

private:
	FCBufferAnimation2DData	mData;

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferAnimation2D* Clone();

public:
	void SetLTUV(const FVector2& uv)
	{
		mData.LTUV = uv;
	}

	void SetLTUV(float u, float v)
	{
		mData.LTUV.x = u;
		mData.LTUV.y = v;
	}

	void SetRBUV(const FVector2& uv)
	{
		mData.RBUV = uv;
	}

	void SetRBUV(float u, float v)
	{
		mData.RBUV.x = u;
		mData.RBUV.y = v;
	}

	void SetAnimation2DEnable(bool Enable)
	{
		mData.Anim2DEnable = Enable ? 1 : 0;
	}

	void SetAnimation2DTextureType(
		EAnimation2DTextureType Type)
	{
		mData.Anim2DTextureType = (int)Type;
	}

	void SetTextureSymmetry(bool Symmetry)
	{
		mData.TextureSymmetry = Symmetry ? 1 : 0;
	}
};

