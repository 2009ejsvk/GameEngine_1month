#include "ShaderBlur.h"

CShaderBlur::CShaderBlur()
{
}

CShaderBlur::~CShaderBlur()
{
}

bool CShaderBlur::Init(const std::string& PathName)
{
	if (!LoadVertexShader("NullTexVS",
		TEXT("Share.hlsli"),
		PathName))
		return false;

	if (!LoadPixelShader("BlurPS",
		TEXT("EnginePostProcess.hlsl"),
		PathName))
		return false;

	return true;
}
