#include "PostProcessBlur.h"
#include "RenderManager.h"
#include "../Asset/Texture/RenderTarget.h"
#include "../Asset/Texture/TextureManager.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Shader/CBufferBlur.h"
#include "../Device.h"

CPostProcessBlur::CPostProcessBlur()
{
}

CPostProcessBlur::~CPostProcessBlur()
{
}

void CPostProcessBlur::ComputeGaussianWeight(int Count)
{
	mBlurWeight.clear();

	mBlurWeight.resize(Count + 1);

	float	Sigma = Count * 0.5f;

	float Denom = 2.f * Sigma * Sigma;

	mBlurWeight[0] = 1.f;

	float	Sum = mBlurWeight[0];

	for (int i = 1; i <= Count; ++i)
	{
		float Weight = std::exp(-(i * i) / Denom);
		mBlurWeight[i] = Weight;
		Sum += 2.f * Weight;
	}

	for (int i = 0; i <= Count; ++i)
	{
		mBlurWeight[i] /= Sum;
	}

	mBlurCBuffer->SetWeight(mBlurWeight);
}

void CPostProcessBlur::SetBlurCount(int Count)
{
	if (Count > MAX_BLUR_COUNT)
		Count = MAX_BLUR_COUNT;

	mBlurCBuffer->SetCount(Count);

	ComputeGaussianWeight(Count);
}

bool CPostProcessBlur::Init()
{
	CPostProcess::Init();

	mBlurCBuffer.reset(new CCBufferBlur);

	mBlurCBuffer->Init();

	FVector2	TexelSize = CDevice::GetInst()->GetTexelSize();

	mBlurCBuffer->SetTexelSize(TexelSize);
	SetBlurCount(5);

	CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, true);

	SetBlendTargetInfo(0);

	SetShader("Blur");
	//SetBlendState("AlphaBlend");

	return true;
}

void CPostProcessBlur::Update(float DeltaTime)
{
}

void CPostProcessBlur::Render()
{
	mBlurCBuffer->UpdateBuffer();
}

