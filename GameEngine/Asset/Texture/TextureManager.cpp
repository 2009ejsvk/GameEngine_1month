#include "TextureManager.h"
#include "../../Device.h"

CTextureManager::CTextureManager()
{
}

CTextureManager::~CTextureManager()
{
	auto	iter = mSamplerMap.begin();
	auto	iterEnd = mSamplerMap.end();

	for (; iter != iterEnd; ++iter)
	{
		SAFE_RELEASE(iter->second);
	}
}

bool CTextureManager::Init()
{
	CreateSampler("Point", D3D11_FILTER_MIN_MAG_MIP_POINT);
	CreateSampler("Linear", D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	SetSampler("Point", 0, EShaderBufferType::All);
	SetSampler("Linear", 1, EShaderBufferType::All);

	return true;
}

bool CTextureManager::LoadTexture(const std::string& Name, 
	const TCHAR* FileName, const std::string& PathName)
{
	std::weak_ptr<CTexture>	Check = FindTexture(Name);

	// 있을 경우
	if (!Check.expired())
		return true;

	CTexture* Origin = new CTexture;

	std::shared_ptr<CTexture> Texture;

	Texture.reset(Origin);

	Texture->mName = Name;

	if (!Texture->LoadTexture(FileName, PathName))
	{
		return false;
	}

	mTextureMap.insert(std::make_pair(Name, Texture));

	return true;
}

bool CTextureManager::LoadTextureFullPath(const std::string& Name,
	const TCHAR* FullPath)
{
	std::weak_ptr<CTexture>	Check = FindTexture(Name);

	// 있을 경우
	if (!Check.expired())
		return true;

	CTexture* Origin = new CTexture;

	std::shared_ptr<CTexture> Texture;

	Texture.reset(Origin);

	Texture->mName = Name;

	if (!Texture->LoadTexture(FullPath))
	{
		return false;
	}

	mTextureMap.insert(std::make_pair(Name, Texture));

	return true;
}

bool CTextureManager::LoadTexture(const std::string& Name, 
	const std::vector<const TCHAR*>& FileName,
	const std::string& PathName)
{
	std::weak_ptr<CTexture>	Check = FindTexture(Name);

	// 있을 경우
	if (!Check.expired())
		return true;

	CTexture* Origin = new CTexture;

	std::shared_ptr<CTexture> Texture;

	Texture.reset(Origin);

	Texture->mName = Name;

	if (!Texture->LoadTexture(FileName, PathName))
	{
		return false;
	}

	mTextureMap.insert(std::make_pair(Name, Texture));

	return true;
}

bool CTextureManager::LoadTextureFullPath(const std::string& Name,
	const std::vector<const TCHAR*> FullPath)
{
	std::weak_ptr<CTexture>	Check = FindTexture(Name);

	// 있을 경우
	if (!Check.expired())
		return true;

	CTexture* Origin = new CTexture;

	std::shared_ptr<CTexture> Texture;

	Texture.reset(Origin);

	Texture->mName = Name;

	if (!Texture->LoadTexture(FullPath))
	{
		return false;
	}

	mTextureMap.insert(std::make_pair(Name, Texture));

	return true;
}

std::weak_ptr<CTexture> CTextureManager::FindTexture(
	const std::string& Name)
{
	auto	iter = mTextureMap.find(Name);

	if (iter == mTextureMap.end())
		return std::weak_ptr<CTexture>();

	return iter->second;
}

bool CTextureManager::CreateSampler(const std::string& Name,
	D3D11_FILTER Filter,
	D3D11_TEXTURE_ADDRESS_MODE u,
	D3D11_TEXTURE_ADDRESS_MODE v,
	D3D11_TEXTURE_ADDRESS_MODE w,
	UINT MaxAnisotropy, D3D11_COMPARISON_FUNC Func)
{
	if (FindSampler(Name))
		return false;

	D3D11_SAMPLER_DESC	Desc = {};

	Desc.Filter = Filter;
	Desc.AddressU = u;
	Desc.AddressV = v;
	Desc.AddressW = w;
	Desc.MipLODBias = 0.f;
	Desc.MaxAnisotropy = MaxAnisotropy;
	Desc.ComparisonFunc = Func;
	Desc.BorderColor[0] = 0.f;
	Desc.BorderColor[1] = 0.f;
	Desc.BorderColor[2] = 0.f;
	Desc.BorderColor[3] = 1.f;
	Desc.MinLOD = 0.f;
	Desc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* Sampler = nullptr;

	if (FAILED(CDevice::GetInst()->GetDevice()->CreateSamplerState(
		&Desc, &Sampler)))
		return false;

	mSamplerMap.insert(std::make_pair(Name, Sampler));

	return true;
}

ID3D11SamplerState* CTextureManager::FindSampler(
	const std::string& Name)
{
	auto	iter = mSamplerMap.find(Name);

	if (iter == mSamplerMap.end())
		return nullptr;

	return iter->second;
}

void CTextureManager::SetSampler(const std::string& Name,
	int Register, int ShaderBufferType)
{
	ID3D11SamplerState* Sampler = FindSampler(Name);

	if (!Sampler)
		return;

	if (ShaderBufferType & EShaderBufferType::Vertex)
	{
		CDevice::GetInst()->GetContext()->VSSetSamplers(Register, 1,
			&Sampler);
	}

	if (ShaderBufferType & EShaderBufferType::Pixel)
	{
		CDevice::GetInst()->GetContext()->PSSetSamplers(Register, 1,
			&Sampler);
	}

	if (ShaderBufferType & EShaderBufferType::Domain)
	{
		CDevice::GetInst()->GetContext()->DSSetSamplers(Register, 1,
			&Sampler);
	}

	if (ShaderBufferType & EShaderBufferType::Hull)
	{
		CDevice::GetInst()->GetContext()->HSSetSamplers(Register, 1,
			&Sampler);
	}

	if (ShaderBufferType & EShaderBufferType::Geometry)
	{
		CDevice::GetInst()->GetContext()->GSSetSamplers(Register, 1,
			&Sampler);
	}

	if (ShaderBufferType & EShaderBufferType::Compute)
	{
		CDevice::GetInst()->GetContext()->CSSetSamplers(Register, 1,
			&Sampler);
	}
}

void CTextureManager::ReleaseAsset(const std::string& Name)
{
	auto	iter = mTextureMap.find(Name);

	if (iter != mTextureMap.end())
	{
		// 다른 월드에서 더이상 들고 있지 않을 경우
		if (iter->second.use_count() == 1)
		{
			mTextureMap.erase(iter);
		}
	}
}
