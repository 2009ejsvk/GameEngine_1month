#pragma once

#include "../Asset.h"
#include "DirectXTex.h"
#include "../Shader/CBufferContainer.h"

#ifdef _DEBUG

#pragma comment(lib, "DirectXTex_Debug.lib")

#else

#pragma comment(lib, "DirectXTex.lib")

#endif // _DEBUG

struct FTextureInfo
{
    // Texture의 Pixel 정보를 저장하기 위한 객체이다.
    DirectX::ScratchImage* Image = nullptr;

    // 위의 Pixel 정보를 이용하여 화면에 출력하기 위해 Shader에
    // Texture 정보를 보내주기 위한 객체이다.
    ID3D11ShaderResourceView* SRV = nullptr;

    unsigned int    Width = 0;
    unsigned int    Height = 0;

    ~FTextureInfo()
    {
        SAFE_DELETE(Image);
        SAFE_RELEASE(SRV);
    }
};


class CTexture :
    public CAsset
{
    friend class CTextureManager;

protected:
    CTexture();

public:
    virtual ~CTexture();

protected:
    // 이미지 여러장을 하나의 Texture로 구성 가능하도록 제작.
    std::vector<FTextureInfo*>  mTextureList;

public:
    const FTextureInfo* GetTexture(int Index = 0)   const
    {
        return mTextureList[Index];
    }

    int GetTextureCount()   const
    {
        return (int)mTextureList.size();
    }

public:
    bool LoadTexture(const TCHAR* FileName,
        const std::string& PathName);
    bool LoadTexture(const TCHAR* FullPath);
    bool LoadTexture(const std::vector<const TCHAR*>& FileName,
        const std::string& PathName);
    bool LoadTexture(const std::vector<const TCHAR*> FullPath);
    void SetShader(int Register, int ShaderBufferType,
        int TextureIndex);


private:
    bool CreateResourceView(int Index = 0);
};

