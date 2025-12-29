#include "CBufferAnimation2D.h"
#include "ConstantBuffer.h"

CCBufferAnimation2D::CCBufferAnimation2D()
{
}

CCBufferAnimation2D::CCBufferAnimation2D(const CCBufferAnimation2D& ref) :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferAnimation2D::CCBufferAnimation2D(CCBufferAnimation2D&& ref) noexcept :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferAnimation2D::~CCBufferAnimation2D()
{
}

bool CCBufferAnimation2D::Init()
{
	SetConstantBuffer("Animation2D");

	return true;
}

void CCBufferAnimation2D::UpdateBuffer()
{
	std::shared_ptr<CConstantBuffer>	CBuffer = mBuffer.lock();

	if (CBuffer)
		CBuffer->Update(&mData);
}

CCBufferAnimation2D* CCBufferAnimation2D::Clone()
{
	return new CCBufferAnimation2D(*this);
}

