#include "ConstantBufferData.h"
#include "../AssetManager.h"
#include "ShaderManager.h"

CConstantBufferData::CConstantBufferData()
{
}

CConstantBufferData::CConstantBufferData(
	const CConstantBufferData& ref)
{
	mBuffer = ref.mBuffer;
}

CConstantBufferData::CConstantBufferData(CConstantBufferData&& ref)	noexcept
{
	mBuffer = ref.mBuffer;
	ref.mBuffer.reset();
}

CConstantBufferData::~CConstantBufferData()
{
}

void CConstantBufferData::SetConstantBuffer(const std::string& Name)
{
	std::weak_ptr<CShaderManager>	Weak_ShaderMgr =
		CAssetManager::GetInst()->GetShaderManager();

	std::shared_ptr<CShaderManager>	ShaderMgr = Weak_ShaderMgr.lock();

	if (ShaderMgr)
		mBuffer = ShaderMgr->FindCBuffer(Name);
}
