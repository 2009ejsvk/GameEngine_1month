#include "BlendState.h"
#include "../Device.h"

CBlendState::CBlendState()
{
}

CBlendState::~CBlendState()
{
}

void CBlendState::AddRenderTargetDesc(bool BlendEnable, 
	D3D11_BLEND SrcBlend, D3D11_BLEND DestBlend,
	D3D11_BLEND_OP BlendOp, D3D11_BLEND SrcBlendAlpha,
	D3D11_BLEND DestBlendAlpha, D3D11_BLEND_OP BlendOpAlpha,
	UINT8 RenderTargetWriteMask)
{
	D3D11_RENDER_TARGET_BLEND_DESC	Desc = {};

	Desc.BlendEnable = BlendEnable;
	Desc.SrcBlend = SrcBlend;
	Desc.DestBlend = DestBlend;
	Desc.BlendOp = BlendOp;
	Desc.SrcBlendAlpha = SrcBlendAlpha;
	Desc.DestBlendAlpha = DestBlendAlpha;
	Desc.BlendOpAlpha = BlendOpAlpha;
	Desc.RenderTargetWriteMask = RenderTargetWriteMask;

	mTargetDesc.push_back(Desc);
}

void CBlendState::SetBlendFactor(float r, float g, float b, float a)
{
	mBlendFactor[0] = r;
	mBlendFactor[1] = g;
	mBlendFactor[2] = b;
	mBlendFactor[3] = a;
}

void CBlendState::SetSampleMask(UINT SampleMask)
{
	mSampleMask = SampleMask;
}

bool CBlendState::CreateState(bool AlphaToCoverageEnable,
	bool IndependentBlendEnable)
{
	if (mTargetDesc.empty())
		return false;

	D3D11_BLEND_DESC	Desc = {};

	Desc.AlphaToCoverageEnable = AlphaToCoverageEnable;
	Desc.IndependentBlendEnable = IndependentBlendEnable;

	size_t	Size = mTargetDesc.size();

	memcpy(Desc.RenderTarget, &mTargetDesc[0],
		sizeof(D3D11_RENDER_TARGET_BLEND_DESC) * Size);

	for (size_t i = Size; i < 8; ++i)
	{
		Desc.RenderTarget[i].BlendEnable = false;
		Desc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		Desc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
		Desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		Desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		Desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		Desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		Desc.RenderTarget[i].RenderTargetWriteMask = 
			D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	if (FAILED(CDevice::GetInst()->GetDevice()->CreateBlendState(
		&Desc, (ID3D11BlendState**)&mState)))
		return false;

	return true;
}

void CBlendState::SetState()
{
	// BlendFactor : float 4개 배열로 구성되며 0 ~ 1로 구성된다.
	// 블렌드 설정에서 D3D11_BLEND_FACTOR 또는 
	// D3D11_BLEND_INV_BLEND_FACTOR를 썼을 때 사용한다.
	CDevice::GetInst()->GetContext()->OMGetBlendState(
		(ID3D11BlendState**)&mPrevState, mPrevBlendFactor,
		&mPrevSampleMask);

	CDevice::GetInst()->GetContext()->OMSetBlendState(
		(ID3D11BlendState*)mState, mBlendFactor, mSampleMask);
}

void CBlendState::ResetState()
{
	CDevice::GetInst()->GetContext()->OMSetBlendState(
		(ID3D11BlendState*)mPrevState, mPrevBlendFactor, 
		mPrevSampleMask);

	SAFE_RELEASE(mPrevState);
}
