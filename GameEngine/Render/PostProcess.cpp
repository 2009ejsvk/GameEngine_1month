#include "PostProcess.h"
#include "../Asset/Texture/RenderTarget.h"
#include "RenderManager.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Shader/ShaderManager.h"
#include "../Asset/AssetManager.h"
#include "../Device.h"
#include "RenderState.h"

CPostProcess::CPostProcess()
{
}

CPostProcess::~CPostProcess()
{
}

void CPostProcess::SetShader(const std::string& Name)
{
	auto	ShaderMgr = CAssetManager::GetInst()->GetShaderManager().lock();
	mShader = ShaderMgr->FindShader(Name);
}

void CPostProcess::CreateRenderTarget(DXGI_FORMAT Fmt,
	bool DepthEnable)
{
	FResolution	RS = CDevice::GetInst()->GetResolution();

	mTarget = CRenderTarget::Create(mName, RS.Width, RS.Height,
		Fmt, DepthEnable);
}

void CPostProcess::SetBlendState(const std::string& Name)
{
	mBlendState = CRenderManager::GetInst()->FindRenderState(Name).lock();
}

bool CPostProcess::Init()
{
	/*mTarget = CRenderManager::GetInst()->FindRenderTarget("FinalTarget");*/
	SetShader("NullBuffer");

	return true;
}

void CPostProcess::RenderFullScreenQuad()
{
	// null버퍼 출력
	mShader.lock()->SetShader();

	ID3D11DeviceContext* Context =
		CDevice::GetInst()->GetContext();

	UINT	Offset = 0;

	Context->IASetVertexBuffers(0, 0, nullptr, nullptr,
		&Offset);
	Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Context->Draw(4, 0);
}

void CPostProcess::RenderPostProcess()
{
	mTarget->SetTarget();

	if (mBlendState)
		mBlendState->SetState();

	Render();

	auto BlendTarget = mBlendTarget.BlendTarget.lock();

	if (BlendTarget)
	{
		BlendTarget->SetShader(mBlendTarget.Register,
			mBlendTarget.ShaderBufferType, 0);
	}

	RenderFullScreenQuad();

	if (mBlendState)
		mBlendState->ResetState();

	mTarget->ResetTarget();
}
