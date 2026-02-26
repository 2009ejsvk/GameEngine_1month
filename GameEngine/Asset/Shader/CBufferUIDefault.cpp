#include "CBufferUIDefault.h"
#include "ConstantBuffer.h"

CCBufferUIDefault::CCBufferUIDefault()
{
}

CCBufferUIDefault::CCBufferUIDefault(const CCBufferUIDefault& ref) :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferUIDefault::CCBufferUIDefault(CCBufferUIDefault&& ref) noexcept :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferUIDefault::~CCBufferUIDefault()
{
}

bool CCBufferUIDefault::Init()
{
	SetConstantBuffer("UIDefault");

	return true;
}

void CCBufferUIDefault::UpdateBuffer()
{
	std::shared_ptr<CConstantBuffer>	CBuffer = mBuffer.lock();

	if (CBuffer)
		CBuffer->Update(&mData);
}

CCBufferUIDefault* CCBufferUIDefault::Clone()
{
	return new CCBufferUIDefault(*this);
}


