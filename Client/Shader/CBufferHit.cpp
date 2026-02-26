#include "CBufferHit.h"
#include "Asset/Shader/ConstantBuffer.h"
#include "Asset/AssetManager.h"
#include "Asset/Shader/ShaderManager.h"

CCBufferHit::CCBufferHit()
{
}

CCBufferHit::CCBufferHit(const CCBufferHit& ref) :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferHit::CCBufferHit(CCBufferHit&& ref) noexcept :
	CConstantBufferData(ref)
{
	mData = ref.mData;
}

CCBufferHit::~CCBufferHit()
{
}

bool CCBufferHit::Init()
{
	auto	ShaderMgr = CAssetManager::GetInst()->GetShaderManager().lock();

	ShaderMgr->CreateCBuffer("Hit", sizeof(FCBufferHitData),
		10);

	SetConstantBuffer("Hit");

	return true;
}

void CCBufferHit::UpdateBuffer()
{
	std::shared_ptr<CConstantBuffer>	CBuffer = mBuffer.lock();

	if (CBuffer)
		CBuffer->Update(&mData);
}

CCBufferHit* CCBufferHit::Clone()
{
	return new CCBufferHit(*this);
}
