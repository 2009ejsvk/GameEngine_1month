#pragma once

#include "RenderState.h"

class CBlendState :
    public CRenderState
{
	friend class CRenderManager;

protected:
	CBlendState();

public:
	virtual ~CBlendState();

private:
	std::vector<D3D11_RENDER_TARGET_BLEND_DESC>	mTargetDesc;
	float	mBlendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
	float	mPrevBlendFactor[4] = {};
	UINT	mSampleMask = 0xffffffff;
	UINT	mPrevSampleMask = 0xffffffff;

public:
	// SrcRGB * SrcBlend BlendOp DestRGB * DestBlend = 최종 RGB
	// SrcA * SrcBlendAlpha BlendOpAlpha DestA * DestBlendAlpha = 
	// 최종 Alpha
	// src : 출력할 물체의 색
	// dest : 렌더타겟에 출력되어 있는 색
	// src : 0, 1, 0, 0.5
	// dest : 1, 0, 0, 1
	// 0, 1, 0 * 0.5 + 1, 0, 0 * (1 - 0.5) = 0.5, 0.5, 0
	void AddRenderTargetDesc(bool BlendEnable = true,
		D3D11_BLEND SrcBlend = D3D11_BLEND_ONE, 
		D3D11_BLEND DestBlend = D3D11_BLEND_ZERO,
		D3D11_BLEND_OP BlendOp = D3D11_BLEND_OP_ADD,
		D3D11_BLEND SrcBlendAlpha = D3D11_BLEND_ONE,
		D3D11_BLEND DestBlendAlpha = D3D11_BLEND_ZERO,
		D3D11_BLEND_OP BlendOpAlpha = D3D11_BLEND_OP_ADD,
		UINT8 RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL);
	void SetBlendFactor(float r, float g, float b, float a);
	void SetSampleMask(UINT SampleMask);

	// AlphaToCoverageEnable : MSAA 사용 시 알파값을 샘플 마스크로
	// 바꿔서 투명 경계를 부드럽게 해준다. 나뭇잎, 풀 같은 곳에서
	// 많이 활용된다.
	// IndependentBlendEnable : false로 하면 RenderTarget[0]의 값이
	// 1 ~ 7번까지 모두 복제되고 true를 하면 각자 따로 지정이 가능하다.
	bool CreateState(bool AlphaToCoverageEnable = false,
		bool IndependentBlendEnable = false);

	/*
	BOOL AlphaToCoverageEnable;
    BOOL IndependentBlendEnable;
    D3D11_RENDER_TARGET_BLEND_DESC RenderTarget[ 8 ];
	*/

public:
	virtual void SetState();
	virtual void ResetState();
};

