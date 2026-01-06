#include "CBufferMaterial.h"
#include "ConstantBuffer.h"

CCBufferMaterial::CCBufferMaterial()
{
}

CCBufferMaterial::CCBufferMaterial(const CCBufferMaterial& ref) :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferMaterial::CCBufferMaterial(CCBufferMaterial&& ref) noexcept :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferMaterial::~CCBufferMaterial()
{
}

bool CCBufferMaterial::Init()
{
	SetConstantBuffer("Material");

	return true;
}

void CCBufferMaterial::UpdateBuffer()
{
	std::shared_ptr<CConstantBuffer>	CBuffer = mBuffer.lock();

	if (CBuffer)
		CBuffer->Update(&mData);
}

CCBufferMaterial* CCBufferMaterial::Clone()
{
	return new CCBufferMaterial(*this);
}

