#include "CBufferTileMap.h"
#include "ConstantBuffer.h"

CCBufferTileMap::CCBufferTileMap()
{
}

CCBufferTileMap::CCBufferTileMap(const CCBufferTileMap& ref) :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferTileMap::CCBufferTileMap(CCBufferTileMap&& ref) noexcept :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferTileMap::~CCBufferTileMap()
{
}

bool CCBufferTileMap::Init()
{
	SetConstantBuffer("TileMap");

	return true;
}

void CCBufferTileMap::UpdateBuffer()
{
	std::shared_ptr<CConstantBuffer>	CBuffer = mBuffer.lock();

	if (CBuffer)
		CBuffer->Update(&mData);
}

CCBufferTileMap* CCBufferTileMap::Clone()
{
	return new CCBufferTileMap(*this);
}




