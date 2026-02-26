#include "ShaderNullBuffer.h"

CShaderNullBuffer::CShaderNullBuffer()
{
}

CShaderNullBuffer::~CShaderNullBuffer()
{
}

bool CShaderNullBuffer::Init(const std::string& PathName)
{
	if (!LoadVertexShader("NullTexVS", 
		TEXT("Share.hlsli"),
		PathName))
		return false;

	if (!LoadPixelShader("TexPS", 
		TEXT("GlobalShader2D.hlsl"),
		PathName))
		return false;

	return true;
}
