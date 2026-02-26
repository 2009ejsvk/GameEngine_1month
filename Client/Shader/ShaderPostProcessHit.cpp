#include "ShaderPostProcessHit.h"

CShaderPostProcessHit::CShaderPostProcessHit()
{
}

CShaderPostProcessHit::~CShaderPostProcessHit()
{
}

bool CShaderPostProcessHit::Init(const std::string& PathName)
{
	if (!LoadVertexShader("HitVS",
		TEXT("PostProcess.hlsl"),
		PathName))
		return false;

	if (!LoadPixelShader("HitPS",
		TEXT("PostProcess.hlsl"),
		PathName))
		return false;

	return true;
}
