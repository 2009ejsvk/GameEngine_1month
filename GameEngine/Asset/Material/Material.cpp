#include "Material.h"
#include "../Shader/CBufferMaterial.h"
#include "../Texture/Texture.h"
#include "../Texture/TextureManager.h"
#include "../AssetManager.h"
#include "../../Render/RenderManager.h"
#include "../../Render/RenderState.h"

CMaterial::CMaterial()
{
	mAssetType = EAssetType::Material;
}

CMaterial::CMaterial(const CMaterial& ref)	:
	CAsset(ref)
{
	mAssetType = EAssetType::Material;
	mBaseColor = ref.mBaseColor;
	mOpacity = ref.mOpacity;

	mMaterialCBuffer.reset(new CCBufferMaterial);

	mMaterialCBuffer->Init();

	mMaterialCBuffer->SetBaseColor(mBaseColor);
	mMaterialCBuffer->SetOpacity(mOpacity);
}

CMaterial::~CMaterial()
{
}

void CMaterial::SetBlendState(const std::string& Name)
{
	mBlendState = CRenderManager::GetInst()->FindRenderState(Name);
}

void CMaterial::SetBlendState(const std::weak_ptr<class CRenderState>& State)
{
	mBlendState = State;
}

void CMaterial::AddTexture(const std::weak_ptr<CTexture>& Texture, int Register, int ShaderBufferType, int Index)
{
	FMaterialTextureInfo* Origin = new FMaterialTextureInfo;

	std::shared_ptr<FMaterialTextureInfo>	TexInfo(Origin);

	std::shared_ptr<CTexture>	_Texture = Texture.lock();

	TexInfo->Texture = Texture;
	TexInfo->Name = _Texture->GetName();
	TexInfo->Register = Register;
	TexInfo->ShaderBufferType = ShaderBufferType;
	TexInfo->Index = Index;

	mTextureArray.push_back(TexInfo);
}

void CMaterial::AddTexture(const std::string& Name, int Register,
	int ShaderBufferType, int Index)
{
	FMaterialTextureInfo* Origin = new FMaterialTextureInfo;

	std::shared_ptr<FMaterialTextureInfo>	TexInfo(Origin);

	auto	TextureManager = CAssetManager::GetInst()->GetTextureManager().lock();

	TexInfo->Texture = TextureManager->FindTexture(Name);
	TexInfo->Name = Name;
	TexInfo->Register = Register;
	TexInfo->ShaderBufferType = ShaderBufferType;
	TexInfo->Index = Index;

	mTextureArray.push_back(TexInfo);
}

void CMaterial::AddTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName, int Register, int ShaderBufferType, int Index)
{
	FMaterialTextureInfo* Origin = new FMaterialTextureInfo;

	std::shared_ptr<FMaterialTextureInfo>	TexInfo(Origin);

	auto	TextureManager = CAssetManager::GetInst()->GetTextureManager().lock();

	TextureManager->LoadTexture(Name, FileName, PathName);

	TexInfo->Texture = TextureManager->FindTexture(Name);
	TexInfo->Name = Name;
	TexInfo->Register = Register;
	TexInfo->ShaderBufferType = ShaderBufferType;
	TexInfo->Index = Index;

	mTextureArray.push_back(TexInfo);
}

void CMaterial::AddTextureFullPath(const std::string& Name, const TCHAR* FullPath, int Register, int ShaderBufferType, int Index)
{
	FMaterialTextureInfo* Origin = new FMaterialTextureInfo;

	std::shared_ptr<FMaterialTextureInfo>	TexInfo(Origin);

	auto	TextureManager = CAssetManager::GetInst()->GetTextureManager().lock();

	TextureManager->LoadTextureFullPath(Name, FullPath);

	TexInfo->Texture = TextureManager->FindTexture(Name);
	TexInfo->Name = Name;
	TexInfo->Register = Register;
	TexInfo->ShaderBufferType = ShaderBufferType;
	TexInfo->Index = Index;

	mTextureArray.push_back(TexInfo);
}

void CMaterial::AddTexture(const std::string& Name, const std::vector<const TCHAR*>& FileName, const std::string& PathName, int Register, int ShaderBufferType, int Index)
{
	FMaterialTextureInfo* Origin = new FMaterialTextureInfo;

	std::shared_ptr<FMaterialTextureInfo>	TexInfo(Origin);

	auto	TextureManager = CAssetManager::GetInst()->GetTextureManager().lock();

	TextureManager->LoadTexture(Name, FileName, PathName);

	TexInfo->Texture = TextureManager->FindTexture(Name);
	TexInfo->Name = Name;
	TexInfo->Register = Register;
	TexInfo->ShaderBufferType = ShaderBufferType;
	TexInfo->Index = Index;

	mTextureArray.push_back(TexInfo);
}

void CMaterial::AddTextureFullPath(const std::string& Name, const std::vector<const TCHAR*>& FullPath, int Register, int ShaderBufferType, int Index)
{
	FMaterialTextureInfo* Origin = new FMaterialTextureInfo;

	std::shared_ptr<FMaterialTextureInfo>	TexInfo(Origin);

	auto	TextureManager = CAssetManager::GetInst()->GetTextureManager().lock();

	TextureManager->LoadTextureFullPath(Name, FullPath);

	TexInfo->Texture = TextureManager->FindTexture(Name);
	TexInfo->Name = Name;
	TexInfo->Register = Register;
	TexInfo->ShaderBufferType = ShaderBufferType;
	TexInfo->Index = Index;

	mTextureArray.push_back(TexInfo);
}

bool CMaterial::SetTexture(int TextureIndex, 
	const std::weak_ptr<class CTexture>& Texture)
{
	if (mTextureArray.size() <= TextureIndex)
	{
		FMaterialTextureInfo* Origin = new FMaterialTextureInfo;

		std::shared_ptr<FMaterialTextureInfo>	TexInfo(Origin);

		auto	_Tex = Texture.lock();

		TexInfo->Name = _Tex->GetName();
		TexInfo->Register = 0;
		TexInfo->ShaderBufferType = EShaderBufferType::Pixel;
		TexInfo->Index = 0;

		mTextureArray.push_back(TexInfo);
	}

	mTextureArray[TextureIndex]->Texture = Texture;

	return true;
}

void CMaterial::SetBaseColor(float r, float g, float b, 
	float a)
{
	mBaseColor = FVector4(r, g, b, a);

	mMaterialCBuffer->SetBaseColor(mBaseColor);
}

void CMaterial::SetBaseColor(int r, int g,
	int b, int a)
{
	mBaseColor = FVector4(r / 255.f, g / 255.f, b / 255.f, a / 255.f);

	mMaterialCBuffer->SetBaseColor(mBaseColor);
}

void CMaterial::SetBaseColor(const FVector4& Color)
{
	mBaseColor = Color;

	mMaterialCBuffer->SetBaseColor(mBaseColor);
}

void CMaterial::SetOpacity(float Opacity)
{
	mOpacity = Opacity;

	mMaterialCBuffer->SetOpacity(mOpacity);
}

bool CMaterial::Init()
{
	mMaterialCBuffer.reset(new CCBufferMaterial);

	mMaterialCBuffer->Init();

	mMaterialCBuffer->SetBaseColor(mBaseColor);
	mMaterialCBuffer->SetOpacity(mOpacity);

	return true;
}

void CMaterial::UpdateConstantBuffer()
{
	size_t	Size = mTextureArray.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto	Texture = mTextureArray[i]->Texture.lock();

		if (Texture)
		{
			Texture->SetShader(mTextureArray[i]->Register,
				mTextureArray[i]->ShaderBufferType,
				0);
		}
	}

	mMaterialCBuffer->UpdateBuffer();

	if (!mBlendState.expired())
	{
		auto	State = mBlendState.lock();

		State->SetState();
	}
}

void CMaterial::UpdateConstantBuffer(int TextureIndex)
{
	size_t	Size = mTextureArray.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto	Texture = mTextureArray[i]->Texture.lock();

		if (Texture)
		{
			Texture->SetShader(mTextureArray[i]->Register,
				mTextureArray[i]->ShaderBufferType,
				TextureIndex);
		}
	}

	mMaterialCBuffer->UpdateBuffer();

	if (!mBlendState.expired())
	{
		auto	State = mBlendState.lock();

		State->SetState();
	}
}

void CMaterial::Reset()
{
	if (!mBlendState.expired())
	{
		auto	State = mBlendState.lock();

		State->ResetState();
	}
}

CMaterial* CMaterial::Clone()	const
{
	return new CMaterial(*this);
}
