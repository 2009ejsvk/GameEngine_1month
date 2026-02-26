#pragma once

#include "Texture.h"

class CRenderTarget :
    public CTexture
{
protected:
    CRenderTarget();

public:
    virtual ~CRenderTarget();

public:
    static std::shared_ptr<CRenderTarget> Create(
        const std::string& Name, unsigned int Width,
        unsigned int Height, DXGI_FORMAT Format,
        bool DepthEnable = false);

private:
    ID3D11Texture2D* mTexture = nullptr;
    ID3D11RenderTargetView* mTarget = nullptr;
    ID3D11RenderTargetView* mPrevTarget = nullptr;

    ID3D11Texture2D* mDepthTexture = nullptr;
    ID3D11DepthStencilView* mDepth = nullptr;
    ID3D11DepthStencilView* mPrevDepth = nullptr;

    D3D11_VIEWPORT      mVP = {};
    D3D11_VIEWPORT      mPrevVP = {};
    unsigned int        mPrevVPCount = 1;
    float               mClearColor[4] = {};

public:
    void SetClearColor(float r, float g, float b, float a)
    {
        mClearColor[0] = r;
        mClearColor[1] = g;
        mClearColor[2] = b;
        mClearColor[3] = a;
    }

    bool Init(unsigned int Width, unsigned int Height,
        DXGI_FORMAT Format, bool DepthEnable = false);
    void SetTarget();
    void ResetTarget();
};

