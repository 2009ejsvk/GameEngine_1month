#include "RenderManager.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "../Component/SceneComponent.h"
#include "../World/WorldManager.h"
#include "../Asset/Texture/RenderTarget.h"
#include "../Device.h"
#include "../Asset/Shader/ShaderManager.h"
#include "../Asset/Shader/Shader.h"
#include "PostProcessBlur.h"
#include "../Asset/Shader/CBufferUIDefault.h"
#include "../UI/Widget.h"
#include "../Asset/Shader/CBufferTransform.h"

CRenderManager* CRenderManager::mInst = nullptr;

bool CRenderManager::SortYRenderList(const std::weak_ptr<class CSceneComponent>& Src,
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
	InitializeCriticalSection(&mCrt);
}

CRenderManager::~CRenderManager()
{
	DeleteCriticalSection(&mCrt);

	ResetState("DepthDisable");
}

void CRenderManager::SetMouseWidget(EMouseState::Type State,
	CMouseWidget* Widget)
{
	mMouseWidget[State].reset(Widget);

	ShowCursor(FALSE);
}

void CRenderManager::SetBlurEnable(bool Enable)
{
	mBlur->SetEnable(Enable);
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

int CRenderManager::GetLayerOrder(const std::string& Name)
{
	auto	iter = mRenderLayerMap.begin();
	auto	iterEnd = mRenderLayerMap.end();

	for (; iter != iterEnd; ++iter)
	{
		if (iter->second.Name == Name)
			return iter->first;
	}

	return -1;
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

	BlendState->SetName(Name);

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

	DepthState->SetName(Name);

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

std::weak_ptr<CRenderTarget> CRenderManager::FindRenderTarget(const std::string& Name)
{
	auto	iter = mRenderTargetMap.find(Name);

	if (iter == mRenderTargetMap.end())
		return std::weak_ptr<CRenderTarget>();

	return iter->second;
}

void CRenderManager::EnablePostProcess(const std::string& Name)
{
	auto	iter = mPostProcessArray.begin();
	auto	iterEnd = mPostProcessArray.end();

	for (; iter != iterEnd; ++iter)
	{
		if ((*iter)->GetName() == Name)
		{
			(*iter)->SetEnable(true);
			return;
		}
	}
}

bool CRenderManager::CheckPostProcess(const std::string& Name)
{
	auto	iter = mPostProcessArray.begin();
	auto	iterEnd = mPostProcessArray.end();

	for (; iter != iterEnd; ++iter)
	{
		if ((*iter)->GetName() == Name)
		{
			return true;
		}
	}

	return false;
}

bool CRenderManager::Init()
{
	// 기본 알파블렌딩 생성
	AddBlendRenderTargetDesc("AlphaBlend", true,
		D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE,
		D3D11_BLEND_INV_SRC_ALPHA);

	CreateBlendState("AlphaBlend");

	// Depth 끄기
	CreateDepthStencilState("DepthDisable", false);

	CreateLayer("Map", 0, ERenderListSort::None);
	CreateLayer("Default", 1, ERenderListSort::Y);

	SetState("DepthDisable");

	FResolution	RS = CDevice::GetInst()->GetResolution();

	// 렌더타겟 생성
	std::shared_ptr<CRenderTarget> Target  = 
		CRenderTarget::Create("MainTarget",
		RS.Width, RS.Height, DXGI_FORMAT_R8G8B8A8_UNORM,
		true);

	mRenderTargetMap.insert(std::make_pair("MainTarget",
		Target));

	Target =
		CRenderTarget::Create("FinalTarget",
			RS.Width, RS.Height, DXGI_FORMAT_R8G8B8A8_UNORM,
			true);

	mRenderTargetMap.insert(std::make_pair("FinalTarget",
		Target));

	mNullBufferShader = CAssetManager::GetInst()->GetShaderManager().lock()->FindShader("NullBuffer");

	mBlur =
		CreatePostProcess<CPostProcessBlur>("Blur", 10).lock();

	mBlur->SetEnable(false);

	auto	MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();

	mTargetMesh = MeshMgr->FindMesh("Mesh_UIRectTex");

	mTargetShader = CAssetManager::GetInst()->GetShaderManager().lock()->FindShader("UIDefault");

	mTargetCBuffer.reset(new CCBufferUIDefault);

	mTargetCBuffer->Init();

	mTargetCBuffer->SetWidgetColor(FVector4::White);
	mTargetCBuffer->SetAnimEnable(false);
	mTargetCBuffer->SetBrusnTint(FVector4::White);
	mTargetCBuffer->SetTextureEnable(true);

	mTargetTR.reset(new CCBufferTransform);

	mTargetTR->Init();

	return true;
}

void CRenderManager::Update(float DeltaTime)
{
	if (mMouseWidget[mMouseState])
		mMouseWidget[mMouseState]->Update(DeltaTime);

	++mInstancingFrameToken;

	if (mInstancingFrameToken == 0)
	{
		mInstancingFrameToken = 1;
	}

	// 가지고 있는 렌더 목록을 이용해서 인스턴싱 객체인지 판단한다.
	auto	iterLayer = mRenderLayerMap.begin();
	auto	iterLayerEnd = mRenderLayerMap.end();

	for (; iterLayer != iterLayerEnd; ++iterLayer)
	{
		// 1-pass: 목록 유효성 정리 + 런타임 레이어 이동 반영
		auto	Com = iterLayer->second.RenderList.begin();
		auto	ComEnd = iterLayer->second.RenderList.end();

		for (; Com != ComEnd;)
		{
			if ((*Com).expired())
			{
				Com = iterLayer->second.RenderList.erase(Com);
				ComEnd = iterLayer->second.RenderList.end();
				continue;
			}

			auto	_Com = (*Com).lock();

			if (!_Com)
			{
				Com = iterLayer->second.RenderList.erase(Com);
				ComEnd = iterLayer->second.RenderList.end();
				continue;
			}

			// 런타임에 레이어가 바뀌면 실제 레이어 리스트도 이동시킨다.
			const int CurrentLayer = _Com->GetRenderLayer();

			if (CurrentLayer != iterLayer->first)
			{
				auto MoveCom = *Com;
				Com = iterLayer->second.RenderList.erase(Com);
				ComEnd = iterLayer->second.RenderList.end();

				auto DestLayer = mRenderLayerMap.find(CurrentLayer);

				if (DestLayer != mRenderLayerMap.end())
				{
					bool ExistsInDest = false;

					auto DestCom = DestLayer->second.RenderList.begin();
					auto DestComEnd = DestLayer->second.RenderList.end();

					for (; DestCom != DestComEnd; ++DestCom)
					{
						auto DestObj = DestCom->lock();

						if (DestObj && DestObj == _Com)
						{
							ExistsInDest = true;
							break;
						}
					}

					if (!ExistsInDest)
						DestLayer->second.RenderList.push_back(MoveCom);
				}

				continue;
			}

			++Com;
		}

		// 2-pass: 레이어 정렬을 프레임당 1회만 수행한다.
		switch (iterLayer->second.SortType)
		{
		case ERenderListSort::None:
			break;
		case ERenderListSort::Y:
			if (iterLayer->second.RenderList.size() > 1)
				iterLayer->second.RenderList.sort(SortYRenderList);
			break;
		case ERenderListSort::Alpha:
			break;
		}

		// 기존 인스턴싱 그룹의 프레임 빌드 시작
		auto	iterIst = iterLayer->second.InstancingMap.begin();
		auto	iterIstEnd = iterLayer->second.InstancingMap.end();

		for (; iterIst != iterIstEnd; ++iterIst)
		{
			iterIst->second->BeginFrameBuild(mInstancingFrameToken);
		}

		// 3-pass: 정렬된 리스트 기준으로 인스턴싱 그룹 구성
		Com = iterLayer->second.RenderList.begin();
		ComEnd = iterLayer->second.RenderList.end();

		for (; Com != ComEnd; ++Com)
		{
			auto	_Com = (*Com).lock();

			if (!_Com || !_Com->GetEnable())
				continue;

			CheckInstancing(_Com, iterLayer->second, mInstancingFrameToken);
		}

		// 4-pass: 미참조 컴포넌트 정리 후 인스턴싱 업데이트
		iterIst = iterLayer->second.InstancingMap.begin();
		iterIstEnd = iterLayer->second.InstancingMap.end();

		for (; iterIst != iterIstEnd;)
		{
			auto Instancing = iterIst->second;
			Instancing->FinalizeFrame(mInstancingFrameToken);

			if (Instancing->IsEmpty())
			{
				iterIst = iterLayer->second.InstancingMap.erase(iterIst);
				iterIstEnd = iterLayer->second.InstancingMap.end();
				continue;
			}

			Instancing->Update(DeltaTime);
			++iterIst;
		}
	}

	CSync	sync(&mCrt);

	auto	iter = mPostProcessArray.begin();
	auto	iterEnd = mPostProcessArray.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->GetActive())
		{
			iter = mPostProcessArray.erase(iter);
			iterEnd = mPostProcessArray.end();
			continue;
		}

		else if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Update(DeltaTime);

		++iter;
	}
}

void CRenderManager::Render()
{
	auto MainTarget = FindRenderTarget("MainTarget").lock();

	MainTarget->SetTarget();

	auto	iter = mRenderLayerMap.begin();
	auto	iterEnd = mRenderLayerMap.end();

	for (; iter != iterEnd; ++iter)
	{
		// 목록 유효성 정리만 수행한다.
		auto	Com = iter->second.RenderList.begin();
		auto	ComEnd = iter->second.RenderList.end();

		for (; Com != ComEnd;)
		{
			if ((*Com).expired())
			{
				Com = iter->second.RenderList.erase(Com);
				ComEnd = iter->second.RenderList.end();
				continue;
			}

			++Com;
		}

		Com = iter->second.RenderList.begin();
		ComEnd = iter->second.RenderList.end();

		for (; Com != ComEnd; ++Com)
		{
			auto	_Com = (*Com).lock();

			if (!_Com)
				continue;

			if (!_Com->GetEnable())
				continue;

			else if (_Com->GetRenderOption() == EComponentRenderOption::Instancing)
				continue;

			_Com->Render();

			//_Com->PostRender();
		}

		// 인스턴싱 객체 반복.
		auto	iterIst = iter->second.InstancingMap.begin();
		auto	iterIstEnd = iter->second.InstancingMap.end();

		for (; iterIst != iterIstEnd; ++iterIst)
		{
			iterIst->second->Render();
			iterIst->second->RenderClear();
		}
	}

	MainTarget->ResetTarget();

	auto	FinalTarget = FindRenderTarget("FinalTarget").lock();

	{
		CSync	sync(&mCrt);

		// 포스트 프로세스 적용
		if (mPostProcessArray.empty())
		{
			FinalTarget->SetTarget();

			MainTarget->SetShader(0, EShaderBufferType::Pixel, 0);

			RenderFullScreenQuad();

			FinalTarget->ResetTarget();
		}

		else
		{
			bool	RenderPostProcess = false;
			std::weak_ptr<CRenderTarget>	PrevTarget = MainTarget;

			auto	iter1 = mPostProcessArray.begin();
			auto	iter1End = mPostProcessArray.end();

			for (; iter1 != iter1End;)
			{
				if (!(*iter1)->GetActive())
				{
					iter1 = mPostProcessArray.erase(iter1);
					iter1End = mPostProcessArray.end();
					continue;
				}

				else if (!(*iter1)->GetEnable())
				{
					++iter1;
					continue;
				}

				(*iter1)->SetBlendTarget(PrevTarget);

				(*iter1)->RenderPostProcess();

				RenderPostProcess = true;

				PrevTarget = (*iter1)->GetTarget();
				FinalTarget = (*iter1)->GetTarget().lock();

				++iter1;
			}

			if (!RenderPostProcess)
			{
				FinalTarget->SetTarget();

				MainTarget->SetShader(0, EShaderBufferType::Pixel, 0);

				RenderFullScreenQuad();

				FinalTarget->ResetTarget();
			}
		}
	}

	// 최종 타겟 출력	
	FinalTarget->SetShader(0, EShaderBufferType::Pixel, 0);

	RenderFullScreenQuad();

	// UI 출력
	SetState("AlphaBlend");
		
	CWorldManager::GetInst()->RenderUI();

	ResetState("AlphaBlend");

	// 디버깅용 렌더타겟 출력
	if (mDebugTarget)
	{
		FVector3	Pos, Scale(400.f, 200.f, 1.f);

		FMatrix	ScaleMat, TranslateMat, WorldMat;

		ScaleMat.Scaling(Scale);
		TranslateMat.Translation(Pos);

		WorldMat = ScaleMat * TranslateMat;

		mTargetTR->SetWorldMatrix(WorldMat);
		mTargetTR->SetProjMatrix(CWidget::GetProjMatrix());

		mTargetTR->UpdateBuffer();

		mTargetCBuffer->UpdateBuffer();

		MainTarget->SetShader(0, EShaderBufferType::Pixel, 0);

		mTargetShader.lock()->SetShader();

		mTargetMesh.lock()->Render();

		{
			CSync	sync(&mCrt);

			auto	iter1 = mPostProcessArray.begin();
			auto	iter1End = mPostProcessArray.end();

			for (; iter1 != iter1End; ++iter1)
			{
				Pos.y += Scale.y;
				TranslateMat.Translation(Pos);

				WorldMat = ScaleMat * TranslateMat;

				mTargetTR->SetWorldMatrix(WorldMat);
				mTargetTR->SetProjMatrix(CWidget::GetProjMatrix());

				mTargetTR->UpdateBuffer();

				mTargetCBuffer->UpdateBuffer();

				auto	PostTarget = (*iter1)->GetTarget().lock();

				PostTarget->SetShader(0, EShaderBufferType::Pixel, 0);

				mTargetShader.lock()->SetShader();

				mTargetMesh.lock()->Render();
			}
		}
	}

	SetState("AlphaBlend");

	// 마우스 출력
	if (mMouseWidget[mMouseState])
		mMouseWidget[mMouseState]->Render();

	ResetState("AlphaBlend");
}

void CRenderManager::RenderFullScreenQuad()
{
	// null버퍼 출력
	mNullBufferShader.lock()->SetShader();

	ID3D11DeviceContext* Context =
		CDevice::GetInst()->GetContext();

	UINT	Offset = 0;

	Context->IASetVertexBuffers(0, 0, nullptr, nullptr,
		&Offset);
	Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Context->Draw(4, 0);
}

void CRenderManager::CheckInstancing(
	std::shared_ptr<class CSceneComponent> Com,
	FRenderLayer& Layer, unsigned int FrameToken)
{
	auto Mesh = Com->GetMesh().lock();
	auto Texture = Com->GetTexture().lock();
	auto Shader = Com->GetShader().lock();

	if (!Mesh || !Shader)
		return;

	size_t	Key = Mesh->GetID() + Shader->GetID();

	if (Texture)
		Key += Texture->GetID();

	auto Range = Layer.InstancingMap.equal_range(Key);

	std::shared_ptr<CRenderInstancing>	Instancing;

	auto	iter = Range.first;
	auto	iterEnd = Range.second;

	// 없을 경우
	if (iter == iterEnd)
	{
		Instancing.reset(new CRenderInstancing);

		Instancing->SetMesh(Com->GetMesh());
		Instancing->SetTexture(Com->GetTexture());

		Layer.InstancingMap.insert(std::make_pair(Key, Instancing));
	}

	// 있을경우
	else
	{
		for (; iter != iterEnd; ++iter)
		{
			if(iter->second->CheckMesh(Com->GetMesh()) &&
				iter->second->CheckTexture(Com->GetTexture()))
			{
				Instancing = iter->second;
				break;
			}
		}
	}

	if (!Instancing)
	{
		Instancing.reset(new CRenderInstancing);
		Instancing->SetMesh(Com->GetMesh());
		Instancing->SetTexture(Com->GetTexture());
		Layer.InstancingMap.insert(std::make_pair(Key, Instancing));
	}

	Instancing->BeginFrameBuild(FrameToken);
	Instancing->AddRenderList(Com);
}
