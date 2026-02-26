#pragma once
#include "ConstantBufferData.h"
class CCBufferTileMap :
    public CConstantBufferData
{
public:
	CCBufferTileMap();
	CCBufferTileMap(const CCBufferTileMap& ref);
	CCBufferTileMap(CCBufferTileMap&& ref)	noexcept;
	virtual ~CCBufferTileMap();

private:
	FCBufferTileMapData	mData;

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferTileMap* Clone();

public:
	void SetUV(const FVector2& LT, const FVector2& RB)
	{
		mData.LTUV = LT;
		mData.RBUV = RB;
	}
};

