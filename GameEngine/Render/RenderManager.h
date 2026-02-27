#pragma once

#include "../EngineInfo.h"
#include "../UI/MouseWidget.h"
#include "PostProcess.h"
#include "../Sync.h"
#include "RenderInstancing.h"

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
	std::unordered_multimap<size_t,
		std::shared_ptr<CRenderInstancing>>	InstancingMap;
	ERenderListSort	SortType = ERenderListSort::Y;
};

class CRenderManager
{
public:
	static bool SortYRenderList(
		const std::weak_ptr<class CSceneComponent>& Src,
		const std::weak_ptr<class CSceneComponent>& Dest);

private:
	CRenderManager();
	~CRenderManager();

private:
	std::unordered_map<std::string, std::shared_ptr<class CRenderState>>	  mRenderStateMap;
	std::map<int, FRenderLayer>	mRenderLayerMap;
	std::vector<std::shared_ptr<CPostProcess>>	mPostProcessArray;

	std::shared_ptr<CPostProcess>	mBlur;

	EMouseState::Type			mMouseState = EMouseState::Normal;
	std::shared_ptr<CMouseWidget>	mMouseWidget[EMouseState::End];

	std::unordered_map<std::string,
		std::shared_ptr<class CRenderTarget>>	mRenderTargetMap;
	std::weak_ptr<class CShader>			mNullBufferShader;

	std::weak_ptr<class CMesh>		mTargetMesh;
	std::weak_ptr<class CShader>	mTargetShader;
	std::shared_ptr<class CCBufferUIDefault>	mTargetCBuffer;
	std::shared_ptr<class CCBufferTransform>	mTargetTR;

	bool		mDebugTarget = true;

	CRITICAL_SECTION	mCrt;

public:
	void SetDebugTarget(bool Debug)
	{
		mDebugTarget = Debug;
	}

	void SetMouseState(EMouseState::Type State)
	{
		mMouseState = State;
	}

	void SetMouseWidget(EMouseState::Type State,
		CMouseWidget* Widget);

	void SetBlurEnable(bool Enable);

public:
	bool CreateLayer(const std::string& Name, int RenderOrder,
		ERenderListSort SortType);
	int GetLayerOrder(const std::string& Name);
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
	std::weak_ptr<class CRenderTarget> FindRenderTarget(const std::string& Name);
	void EnablePostProcess(const std::string& Name);
	bool CheckPostProcess(const std::string& Name);
	bool Init();
	void Update(float DeltaTime);
	void Render();

private:
	void RenderFullScreenQuad();
	void CheckInstancing(std::shared_ptr<class CSceneComponent> Com, FRenderLayer& Layer);

public:
	template <typename T>
	std::weak_ptr<T> SetMouseWidget(EMouseState::Type State,
		const std::string& Name)
	{
		CSync	sync(&mCrt);

		mMouseWidget[State].reset(new T);

		mMouseWidget[State]->SetSelf(mMouseWidget[State]);
		mMouseWidget[State]->SetName(Name);

		if (!mMouseWidget[State]->Init())
		{
			return std::weak_ptr<T>();
		}

		ShowCursor(FALSE);

		// dynamic_pointer_cast 를 통해 T 타입으로 변환한
		// shared_ptr이 나오고 그걸 weak_ptr로 변환해서 반환한다.
		return std::dynamic_pointer_cast<T>(mMouseWidget[State]);
	}

	template <typename T>
	std::weak_ptr<T> CreatePostProcess(const std::string& Name,
		int Order = 0)
	{
		std::shared_ptr<CPostProcess>	Obj;

		Obj.reset(new T);

		Obj->SetName(Name);
		Obj->SetOrder(Order);

		if (!Obj->Init())
		{
			return std::weak_ptr<T>();
		}

		CSync	sync(&mCrt);

		mPostProcessArray.push_back(Obj);

		std::sort(mPostProcessArray.begin(),
			mPostProcessArray.end(),
			CRenderManager::SortPostProcess);

		return std::dynamic_pointer_cast<T>(Obj);
	}

private:
	static bool SortPostProcess(
		const std::shared_ptr<CPostProcess>& Src,
		const std::shared_ptr<CPostProcess>& Dest)
	{
		return Src->GetOrder() > Dest->GetOrder();
	}


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

