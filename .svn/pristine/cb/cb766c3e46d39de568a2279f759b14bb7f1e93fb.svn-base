#include "ShaderColor2D.h"

CShaderColor2D::CShaderColor2D()
{
}

CShaderColor2D::~CShaderColor2D()
{
}

bool CShaderColor2D::Init(const std::string& PathName)
{
	if (!LoadVertexShader("Color2DVS", TEXT("GlobalShader2D.hlsl"),
		PathName))
		return false;

	if (!LoadPixelShader("Color2DPS", TEXT("GlobalShader2D.hlsl"),
		PathName))
		return false;

	AddInputDesc("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
		D3D11_INPUT_PER_VERTEX_DATA, 0);
	AddInputDesc("COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16,
		D3D11_INPUT_PER_VERTEX_DATA, 0);

	// 입력 레이아웃을 생성한다.
	if (!CreateInputLayout())
		return false;

	return true;
}
