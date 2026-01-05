#include "RenderManager.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "../Component/SceneComponent.h"

CRenderManager* CRenderManager::mInst = nullptr;

bool SortYRenderList(const std::weak_ptr<class CSceneComponent>& Src,
	const std::weak_ptr<class CSceneComponent>& Dest)
{
	auto	_Src = Src.lock();
	auto	_Dest = Dest.lock();

	float	SrcY = _Src->GetWorldPos().y;
	float	DestY = _Dest->GetWorldPos().y;

	return SrcY > DestY;
}

CRenderManager::CRenderManager()
{
}

CRenderManager::~CRenderManager()
{
	ResetState("DepthDisable");
}

bool CRenderManager::CreateLayer(const std::string& Name, 
	int RenderOrder,
	ERenderListSort SortType)
{
	auto	iter = mRenderLayerMap.find(RenderOrder);

	if (iter != mRenderLayerMap.end())
		return false;

	FRenderLayer	Layer;
	Layer.Name = Name;
	Layer.SortType = SortType;

	mRenderLayerMap.insert(std::make_pair(RenderOrder, Layer));

	return true;
}

void CRenderManager::AddRenderLayer(
	const std::weak_ptr<class CSceneComponent>& Component)
{
	auto	RenderComponent = Component.lock();

	int	RenderLayer = RenderComponent->GetRenderLayer();

	auto	iter = mRenderLayerMap.find(RenderLayer);

	if (iter == mRenderLayerMap.end())
		return;

	iter->second.RenderList.push_back(Component);

}

void CRenderManager::ClearRenderLayer(int RenderLayer)
{
	auto	iter = mRenderLayerMap.begin();
	auto	iterEnd = mRenderLayerMap.end();

	for (; iter != iterEnd; ++iter)
	{
		iter->second.RenderList.clear();
	}
}

void CRenderManager::AddBlendRenderTargetDesc(
	const std::string& Name, bool BlendEnable, 
	D3D11_BLEND SrcBlend, D3D11_BLEND DestBlend,
	D3D11_BLEND_OP BlendOp, D3D11_BLEND SrcBlendAlpha,
	D3D11_BLEND DestBlendAlpha, D3D11_BLEND_OP BlendOpAlpha,
	UINT8 RenderTargetWriteMask)
{
	auto	iter = mRenderStateMap.find(Name);

	std::shared_ptr<CBlendState>	BlendState;

	if (iter == mRenderStateMap.end())
	{
		std::shared_ptr<CRenderState>	State(new CBlendState);

		mRenderStateMap.insert(std::make_pair(Name, State));

		BlendState = std::dynamic_pointer_cast<CBlendState>(State);
	}

	else
	{
		BlendState = std::dynamic_pointer_cast<CBlendState>(
			iter->second);
	}

	BlendState->AddRenderTargetDesc(BlendEnable, SrcBlend, DestBlend,
		BlendOp, SrcBlendAlpha, DestBlendAlpha, BlendOpAlpha,
		RenderTargetWriteMask);
}

void CRenderManager::SetBlendFactor(const std::string& Name,
	float r, float g, float b, float a)
{
	auto	iter = mRenderStateMap.find(Name);

	std::shared_ptr<CBlendState>	BlendState;

	if (iter == mRenderStateMap.end())
	{
		std::shared_ptr<CRenderState>	State(new CBlendState);

		mRenderStateMap.insert(std::make_pair(Name, State));

		BlendState = std::dynamic_pointer_cast<CBlendState>(State);
	}

	else
	{
		BlendState = std::dynamic_pointer_cast<CBlendState>(
			iter->second);
	}

	BlendState->SetBlendFactor(r, g, b, a);
}

void CRenderManager::SetBlendSampleMask(const std::string& Name,
	UINT SampleMask)
{
	auto	iter = mRenderStateMap.find(Name);

	std::shared_ptr<CBlendState>	BlendState;

	if (iter == mRenderStateMap.end())
	{
		std::shared_ptr<CRenderState>	State(new CBlendState);

		mRenderStateMap.insert(std::make_pair(Name, State));

		BlendState = std::dynamic_pointer_cast<CBlendState>(State);
	}

	else
	{
		BlendState = std::dynamic_pointer_cast<CBlendState>(
			iter->second);
	}

	BlendState->SetSampleMask(SampleMask);
}

bool CRenderManager::CreateBlendState(const std::string& Name,
	bool AlphaToCoverageEnable, bool IndependentBlendEnable)
{
	auto	iter = mRenderStateMap.find(Name);

	std::shared_ptr<CBlendState>	BlendState;

	if (iter == mRenderStateMap.end())
	{
		std::shared_ptr<CRenderState>	State(new CBlendState);

		mRenderStateMap.insert(std::make_pair(Name, State));

		BlendState = std::dynamic_pointer_cast<CBlendState>(State);
	}

	else
	{
		BlendState = std::dynamic_pointer_cast<CBlendState>(
			iter->second);
	}

	if (!BlendState->CreateState(AlphaToCoverageEnable,
		IndependentBlendEnable))
	{
		mRenderStateMap.erase(Name);

		return false;
	}

	return true;
}

bool CRenderManager::CreateDepthStencilState(const std::string& Name,
	bool DepthEnable, D3D11_DEPTH_WRITE_MASK DepthWriteMask, 
	D3D11_COMPARISON_FUNC DepthFunc, bool StencilEnable, 
	UINT8 StencilReadMask, UINT8 StencilWriteMask,
	D3D11_DEPTH_STENCILOP_DESC FrontFace, 
	D3D11_DEPTH_STENCILOP_DESC BackFace)
{
	auto	iter = mRenderStateMap.find(Name);

	std::shared_ptr<CDepthStencilState>	DepthState;

	if (iter == mRenderStateMap.end())
	{
		std::shared_ptr<CRenderState>	State(new CDepthStencilState);

		mRenderStateMap.insert(std::make_pair(Name, State));

		DepthState = std::dynamic_pointer_cast<CDepthStencilState>(
			State);
	}

	else
	{
		DepthState = std::dynamic_pointer_cast<CDepthStencilState>(
			iter->second);
	}

	if (!DepthState->CreateState(DepthEnable, DepthWriteMask,
		DepthFunc, StencilEnable, StencilReadMask, StencilWriteMask,
		FrontFace, BackFace))
	{
		mRenderStateMap.erase(Name);
		return false;
	}

	return true;
}

void CRenderManager::SetState(const std::string& Name)
{
	auto	iter = mRenderStateMap.find(Name);

	if (iter == mRenderStateMap.end())
		return;

	iter->second->SetState();
}

void CRenderManager::ResetState(const std::string& Name)
{
	auto	iter = mRenderStateMap.find(Name);

	if (iter == mRenderStateMap.end())
		return;

	iter->second->ResetState();
}

std::weak_ptr<CRenderState> CRenderManager::FindRenderState(
	const std::string& Name)
{
	auto	iter = mRenderStateMap.find(Name);

	if (iter == mRenderStateMap.end())
		return std::weak_ptr<CRenderState>();

	return iter->second;
}

bool CRenderManager::Init()
{
	// 기본 알파블렌딩 생성
	AddBlendRenderTargetDesc("AlphaBlend", true,
		D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND_OP_ADD);

	CreateBlendState("AlphaBlend");

	// Depth 끄기
	CreateDepthStencilState("DepthDisable", false);

	CreateLayer("Default", 0, ERenderListSort::Y);

	SetState("DepthDisable");

	return true;
}

void CRenderManager::Render()
{
	auto	iter = mRenderLayerMap.begin();
	auto	iterEnd = mRenderLayerMap.end();

	for (; iter != iterEnd; ++iter)
	{
		// 렌더링할 목록을 정렬한다.
		auto	Com = iter->second.RenderList.begin();
		auto	ComEnd = iter->second.RenderList.end();

		for (; Com != ComEnd;)
		{
			if ((*Com).expired())
			{
				Com = iter->second.RenderList.erase(Com);
				continue;
			}

			++Com;
		}

		// 출력할 물체를 정렬한다.
		switch (iter->second.SortType)
		{
		case ERenderListSort::None:
			break;
		case ERenderListSort::Y:
			if (iter->second.RenderList.size() > 1)
				iter->second.RenderList.sort(SortYRenderList);
			break;
		case ERenderListSort::Alpha:
			break;
		}

		Com = iter->second.RenderList.begin();
		ComEnd = iter->second.RenderList.end();

		for (; Com != ComEnd; ++Com)
		{
			auto	_Com = (*Com).lock();

			if (!_Com->GetEnable())
				continue;

			_Com->Render();

			_Com->PostRender();
		}
	}
}
