#pragma once

#include "RenderState.h"

class CDepthStencilState :
    public CRenderState
{
	friend class CRenderManager;

protected:
	CDepthStencilState();

public:
	virtual ~CDepthStencilState();

private:
	unsigned int	mStencilRef = 0xffffffff;
	unsigned int	mPrevStencilRef = 0xffffffff;

public:
	// DepthEnable : 깊이를 쓸지 말지 결정한다.
	// DepthWriteMask : ZERO, ALL 로 구성. ZERO를 하면 깊이버퍼
	// 에 깊이값을 안쓰고 ALL로 하면 깊이값을 쓴다.
	// DepthFunc : 깊이값을 비교할 함수를 지정한다.
	// StencilEnable : 스텐실을 사용할지 여부.
	// StencilReadMask : 스텐실을 테스트할 때 읽어서 비교할 비트 마스크
	bool CreateState(bool DepthEnable = true,
		D3D11_DEPTH_WRITE_MASK DepthWriteMask =
		D3D11_DEPTH_WRITE_MASK_ALL,
		D3D11_COMPARISON_FUNC DepthFunc = D3D11_COMPARISON_LESS,
		bool StencilEnable = false, UINT8 StencilReadMask = 0xff,
		UINT8 StencilWriteMask = 0xff,
		D3D11_DEPTH_STENCILOP_DESC FrontFace =
		{ D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP , D3D11_COMPARISON_ALWAYS },
		D3D11_DEPTH_STENCILOP_DESC BackFace =
		{ D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP , D3D11_COMPARISON_ALWAYS });

public:
	virtual void SetState();
	virtual void ResetState();
};

