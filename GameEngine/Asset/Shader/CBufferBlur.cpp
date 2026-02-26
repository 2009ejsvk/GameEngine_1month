#include "CBufferBlur.h"
#include "ConstantBuffer.h"

CCBufferBlur::CCBufferBlur()
{
}

CCBufferBlur::CCBufferBlur(const CCBufferBlur& ref) :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferBlur::CCBufferBlur(CCBufferBlur&& ref) noexcept :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferBlur::~CCBufferBlur()
{
}

bool CCBufferBlur::Init()
{
	SetConstantBuffer("Blur");

	return true;
}

void CCBufferBlur::UpdateBuffer()
{
	std::shared_ptr<CConstantBuffer>	CBuffer = mBuffer.lock();

	if (CBuffer)
		CBuffer->Update(&mData);
}

CCBufferBlur* CCBufferBlur::Clone()
{
	return new CCBufferBlur(*this);
}



