#pragma once

#include "../Asset.h"
#include "../Shader/CBufferContainer.h"

struct FMaterialTextureInfo
{
	std::string		Name;
	std::weak_ptr<class CTexture>	Texture;
	int				Register = 0;
	int				ShaderBufferType = EShaderBufferType::Pixel;
	int				Index = 0;
};

class CMaterial :
    public CAsset
{
	friend class CMaterialManager;
	friend class CMesh;

protected:
	CMaterial();
	CMaterial(const CMaterial& ref);

public:
	virtual ~CMaterial();

protected:
	std::vector<std::shared_ptr<FMaterialTextureInfo>>	mTextureArray;
	FVector4		mBaseColor = FVector4::White;
	// 불투명도. 1 : 불투명 0 : 투명
	float			mOpacity = 1.f;
	std::shared_ptr<class CCBufferMaterial>	mMaterialCBuffer;
	std::weak_ptr<class CRenderState>	mBlendState;

public:
	void SetBlendState(const std::string& Name);
	void SetBlendState(const std::weak_ptr<class CRenderState>& State);
	void AddTexture(const std::weak_ptr<CTexture>& Texture,
		int Register, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	void AddTexture(const std::string& Name,
		int Register, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	void AddTexture(const std::string& Name,
		const TCHAR* FileName, 
		const std::string& PathName = "Texture",
		int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	void AddTextureFullPath(const std::string& Name,
		const TCHAR* FullPath,
		int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	void AddTexture(const std::string& Name,
		const std::vector<const TCHAR*>& FileName,
		const std::string& PathName = "Texture",
		int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	void AddTextureFullPath(const std::string& Name,
		const std::vector<const TCHAR*>& FullPath,
		int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	bool SetTexture(int TextureIndex, 
		const std::weak_ptr<class CTexture>& Texture);
	bool SetTextureIndex(int TextureIndex);
	void SetBaseColor(float r, float g, float b, float a);
	void SetBaseColor(int r, int g,
		int b, int a);
	void SetBaseColor(const FVector4& Color);
	void SetOpacity(float Opacity);

public:
	bool Init();
	void UpdateConstantBuffer();
	void UpdateConstantBuffer(int TextureIndex);
	void Reset();
	CMaterial* Clone()	const;
};

