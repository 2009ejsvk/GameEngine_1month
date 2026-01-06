#pragma once

#include "../EngineInfo.h"

enum class ERenderListSort
{
	None,
	Y,
	Alpha
};

struct FRenderLayer
{
	std::string		Name;
	std::list<std::weak_ptr<class CSceneComponent>>	RenderList;
	ERenderListSort	SortType = ERenderListSort::Y;
};

class CRenderManager
{
private:
	CRenderManager();
	~CRenderManager();

private:
	std::unordered_map<std::string, std::shared_ptr<class CRenderState>>	  mRenderStateMap;
	std::map<int, FRenderLayer>	mRenderLayerMap;

public:
	bool CreateLayer(const std::string& Name, int RenderOrder,
		ERenderListSort SortType);
	void AddRenderLayer(
		const std::weak_ptr<class CSceneComponent>& Component);
	void ClearRenderLayer(int RenderLayer);

public:
	void AddBlendRenderTargetDesc(const std::string& Name,
		bool BlendEnable = true,
		D3D11_BLEND SrcBlend = D3D11_BLEND_ONE,
		D3D11_BLEND DestBlend = D3D11_BLEND_ZERO,
		D3D11_BLEND_OP BlendOp = D3D11_BLEND_OP_ADD,
		D3D11_BLEND SrcBlendAlpha = D3D11_BLEND_ONE,
		D3D11_BLEND DestBlendAlpha = D3D11_BLEND_ZERO,
		D3D11_BLEND_OP BlendOpAlpha = D3D11_BLEND_OP_ADD,
		UINT8 RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL);
	void SetBlendFactor(const std::string& Name, 
		float r, float g, float b, float a);
	void SetBlendSampleMask(const std::string& Name, UINT SampleMask);
	bool CreateBlendState(const std::string& Name, 
		bool AlphaToCoverageEnable = false,
		bool IndependentBlendEnable = false);

	bool CreateDepthStencilState(const std::string& Name, 
		bool DepthEnable = true,
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

	void SetState(const std::string& Name);
	void ResetState(const std::string& Name);

	std::weak_ptr<class CRenderState> FindRenderState(
		const std::string& Name);

public:
	bool Init();
	void Render();

private:
	static CRenderManager* mInst;

public:
	static CRenderManager* GetInst()
	{
		if (!mInst)
			mInst = new CRenderManager;
		return mInst;
	}

	static void DestroyInst()
	{
		SAFE_DELETE(mInst);
	}
};

