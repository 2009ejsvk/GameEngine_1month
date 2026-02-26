#include "RenderTarget.h"
#include "../../Device.h"

CRenderTarget::CRenderTarget()
{
}

CRenderTarget::~CRenderTarget()
{
    SAFE_RELEASE(mDepthTexture);
    SAFE_RELEASE(mDepth);
    SAFE_RELEASE(mPrevDepth);

    SAFE_RELEASE(mTexture);
    SAFE_RELEASE(mTarget);
    SAFE_RELEASE(mPrevTarget);
}

std::shared_ptr<CRenderTarget> CRenderTarget::Create(
    const std::string& Name, unsigned int Width, unsigned int Height,
    DXGI_FORMAT Format, bool DepthEnable)
{
    static std::shared_ptr<CRenderTarget>   Target;

    Target.reset(new CRenderTarget);

    Target->SetName(Name);

    if (!Target->Init(Width, Height, Format, DepthEnable))
    {
        Target.reset();
        return std::shared_ptr<CRenderTarget>();
    }

    return Target;
}

bool CRenderTarget::Init(unsigned int Width, unsigned int Height,
    DXGI_FORMAT Format, bool DepthEnable)
{
    FTextureInfo* Info = new FTextureInfo;

    Info->Width = Width;
    Info->Height = Height;

    // Target Texture를 생성한다.
    D3D11_TEXTURE2D_DESC    Desc = {};
    Desc.Width = Width;
    Desc.Height = Height;
    Desc.MipLevels = 1;
    Desc.ArraySize = 1;
    Desc.Format = Format;
    Desc.SampleDesc.Count = 1;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_RENDER_TARGET |
        D3D11_BIND_SHADER_RESOURCE;

    if (FAILED(CDevice::GetInst()->GetDevice()->CreateTexture2D(&Desc,
        nullptr, &mTexture)))
        return false;

    // 렌더타겟 뷰를 생성한다.
    if (FAILED(CDevice::GetInst()->GetDevice()->CreateRenderTargetView(
        mTexture, nullptr, &mTarget)))
        return false;

    // Shader Resource View 생성
    if (FAILED(CDevice::GetInst()->GetDevice()->CreateShaderResourceView(
        mTexture, nullptr, &Info->SRV)))
        return false;

    mVP.Width = (float)Width;
    mVP.Height = (float)Height;
    mVP.MaxDepth = 1.f;

    mTextureList.push_back(Info);

    if (DepthEnable)
    {
        D3D11_TEXTURE2D_DESC    DepthDesc = {};
        DepthDesc.Width = Width;
        DepthDesc.Height = Height;
        DepthDesc.MipLevels = 1;
        DepthDesc.ArraySize = 1;
        DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        DepthDesc.SampleDesc.Count = 1;
        DepthDesc.Usage = D3D11_USAGE_DEFAULT;
        DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        if (FAILED(CDevice::GetInst()->GetDevice()->CreateTexture2D(
            &DepthDesc, nullptr, &mDepthTexture)))
            return false;

        if (FAILED(CDevice::GetInst()->GetDevice()->CreateDepthStencilView(
            mDepthTexture, nullptr, &mDepth)))
            return false;
    }

    return true;
}

void CRenderTarget::SetTarget()
{
    // 출력하기 위한 타겟을 지정한다.
    ID3D11DeviceContext* Context = CDevice::GetInst()->GetContext();

    ID3D11ShaderResourceView* nullSRV = nullptr;

    // 픽셀 셰이더를 nullptr로 초기화해준다.
    Context->PSSetShaderResources(0, 1, &nullSRV);

    // 지정되어 있던 타겟뷰를 얻어온다.
    // 현재 디바이스에 지정되어 있는 렌더 타겟을 얻어온다.
    // 만약 백버퍼 타겟이 지정되어 있다면 백버퍼 타겟을 얻어오는
    // 개념이다.
    Context->OMGetRenderTargets(1, &mPrevTarget, &mPrevDepth);

    // 현재 뷰포트 정보를 얻어온다.
    Context->RSGetViewports(&mPrevVPCount, &mPrevVP);

    // 이 렌더 타겟에 지정한 뷰포트로 새롭게 지정한다.
    // 현재 렌더타겟에 출력을 해야하기 때문에 뷰포트를 현재 렌더타겟
    // 에 맞는 크기로 지정한 뷰포트로 지정해 주는것이다.
    Context->RSSetViewports(1, &mVP);

    // 만약 렌더타겟을 만들때 깊이 버퍼를 만들어 주었다면 만들어준
    // 깊이 버퍼로 지정하고 아니라면 기존에 지정되어 있던 깊이버퍼
    // 를 그대로 사용하게 한다.
    ID3D11DepthStencilView* Depth = mDepth;

    if (!mDepth)
        Depth = mPrevDepth;

    // 디바이스에 이 타겟을 렌더링할 대상 타겟으로 지정해준다.
    Context->OMSetRenderTargets(1, &mTarget, Depth);

    // 지정된 색상으로 이 타겟을 클리어한다.
    Context->ClearRenderTargetView(mTarget, mClearColor);

    if (mDepth)
    {
        Context->ClearDepthStencilView(Depth,
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
    }
}

void CRenderTarget::ResetTarget()
{
    // 원래 타겟으로 복구시킨다.
    ID3D11DeviceContext* Context = CDevice::GetInst()->GetContext();

    Context->RSSetViewports(1, &mPrevVP);
    Context->OMSetRenderTargets(1, &mPrevTarget, mPrevDepth);
    SAFE_RELEASE(mPrevTarget);
    SAFE_RELEASE(mPrevDepth);
}
