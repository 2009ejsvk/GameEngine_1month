#include "PostProcessHit.h"
#include "Render/RenderManager.h"
#include "Asset/Texture/RenderTarget.h"
#include "Asset/Texture/TextureManager.h"
#include "Asset/AssetManager.h"
#include "../Shader/CBufferHit.h"

CPostProcessHit::CPostProcessHit()
{
}

CPostProcessHit::~CPostProcessHit()
{
}

void CPostProcessHit::SetHitColor(float r, float g, 
	float b, float a)
{
	mHitColor = FVector4(r, g, b, a);

	mHitCBuffer->SetColor(mHitColor);
}

bool CPostProcessHit::Init()
{
	CPostProcess::Init();

	mHitCBuffer.reset(new CCBufferHit);

	mHitCBuffer->Init();

	CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, true);

	SetBlendTargetInfo(0);

	mHitColor.x = 1.f;
	mHitColor.w = 1.f;

	mHitCBuffer->SetColor(mHitColor);

	SetShader("Hit");
	SetOrder(20);
	//SetBlendState("AlphaBlend");

	auto	TexMgr = CAssetManager::GetInst()->GetTextureManager().lock();

	TexMgr->LoadTexture("Hit", TEXT("TestEffect.png"));

	mHitTexture = TexMgr->FindTexture("Hit");

	return true;
}

void CPostProcessHit::Update(float DeltaTime)
{
	mTime += DeltaTime;

	if (mTime >= mEnableTime)
	{
		mTime = 0.f;
		SetEnable(false);
	}
}

void CPostProcessHit::Render()
{
	auto	HitTex = mHitTexture.lock();

	if (HitTex)
		HitTex->SetShader(1, EShaderBufferType::Pixel, 0);

	mHitCBuffer->UpdateBuffer();
}
