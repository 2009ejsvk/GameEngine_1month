#pragma once

#include "ConstantBufferData.h"

class CCBufferTransform :
    public CConstantBufferData
{
public:
	CCBufferTransform();
	CCBufferTransform(const CCBufferTransform& ref);
	CCBufferTransform(CCBufferTransform&& ref)	noexcept;
	virtual ~CCBufferTransform();

private:
	FCBufferTransformData	mData;

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferTransform* Clone();

public:
	void SetWorldMatrix(const FMatrix& World)
	{
		mData.World = World;
	}

	void SetViewMatrix(const FMatrix& View)
	{
		mData.View = View;
	}

	void SetProjMatrix(const FMatrix& Proj)
	{
		mData.Proj = Proj;
	}
};

