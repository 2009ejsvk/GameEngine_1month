#include "CBufferTransform.h"
#include "ConstantBuffer.h"

CCBufferTransform::CCBufferTransform()
{
}

CCBufferTransform::CCBufferTransform(const CCBufferTransform& ref)	:
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferTransform::CCBufferTransform(CCBufferTransform&& ref) noexcept :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferTransform::~CCBufferTransform()
{
}

bool CCBufferTransform::Init()
{
	SetConstantBuffer("Transform");

	return true;
}

void CCBufferTransform::UpdateBuffer()
{
	mData.WV = mData.World * mData.View;
	mData.WVP = mData.WV * mData.Proj;

	// Shader에서 이 행렬을 이용하여 연산을 할 수 있게 하기 위해
	// 행과 열을 바꾼다.
	mData.World.Transpose();
	mData.View.Transpose();
	mData.Proj.Transpose();
	mData.WV.Transpose();
	mData.WVP.Transpose();

	std::shared_ptr<CConstantBuffer>	CBuffer = mBuffer.lock();

	if (CBuffer)
		CBuffer->Update(&mData);
}

CCBufferTransform* CCBufferTransform::Clone()
{
	return new CCBufferTransform(*this);
}
