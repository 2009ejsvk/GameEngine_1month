#pragma once

#include "../../EngineInfo.h"
#include "Texture.h"
#include "../Shader/CBufferContainer.h"

class CTextureManager
{
	friend class CAssetManager;

private:
	CTextureManager();

public:
	~CTextureManager();

private:
    std::unordered_map<std::string, std::shared_ptr<CTexture>>	mTextureMap;
    std::unordered_map<std::string, ID3D11SamplerState*>	mSamplerMap;

public:
	bool Init();
    bool LoadTexture(const std::string& Name, const TCHAR* FileName,
        const std::string& PathName = "Texture");
    bool LoadTextureFullPath(const std::string& Name, const TCHAR* FullPath);
    bool LoadTexture(const std::string& Name, 
        const std::vector<const TCHAR*>& FileName,
        const std::string& PathName = "Texture");
    bool LoadTextureFullPath(const std::string& Name,
        const std::vector<const TCHAR*> FullPath);
    std::weak_ptr<CTexture> FindTexture(const std::string& Name);

    bool CreateSampler(const std::string& Name, D3D11_FILTER Filter,
        D3D11_TEXTURE_ADDRESS_MODE u = D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_TEXTURE_ADDRESS_MODE v = D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_TEXTURE_ADDRESS_MODE w = D3D11_TEXTURE_ADDRESS_WRAP,
        UINT MaxAnisotropy = 0,
        D3D11_COMPARISON_FUNC Func = D3D11_COMPARISON_NEVER);
    ID3D11SamplerState* FindSampler(const std::string& Name);
    void SetSampler(const std::string& Name, int Register,
        int ShaderBufferType = EShaderBufferType::Pixel);
    void ReleaseAsset(const std::string& Name);
};

