#include "CBufferCollider.h"
#include "ConstantBuffer.h"

CCBufferCollider::CCBufferCollider()
{
}

CCBufferCollider::CCBufferCollider(const CCBufferCollider& ref) :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferCollider::CCBufferCollider(CCBufferCollider&& ref) noexcept :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferCollider::~CCBufferCollider()
{
}

bool CCBufferCollider::Init()
{
	SetConstantBuffer("Collider");

	return true;
}

void CCBufferCollider::UpdateBuffer()
{
	std::shared_ptr<CConstantBuffer>	CBuffer = mBuffer.lock();

	if (CBuffer)
		CBuffer->Update(&mData);
}

CCBufferCollider* CCBufferCollider::Clone()
{
	return new CCBufferCollider(*this);
}

