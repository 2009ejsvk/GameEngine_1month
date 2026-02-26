#include "ShaderMaterialColor2DInstancing.h"

CShaderMaterialColor2DInstancing::CShaderMaterialColor2DInstancing()
{
}

CShaderMaterialColor2DInstancing::~CShaderMaterialColor2DInstancing()
{
}

bool CShaderMaterialColor2DInstancing::Init(const std::string& PathName)
{
	if (!LoadVertexShader("Color2DInstancingVS", TEXT("GlobalShader2D.hlsl"),
		PathName))
		return false;

	if (!LoadPixelShader("Color2DPS",
		TEXT("GlobalShader2D.hlsl"), PathName))
		return false;

	AddInputDesc("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
		D3D11_INPUT_PER_VERTEX_DATA, 0);
	AddInputDesc("COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16,
		D3D11_INPUT_PER_VERTEX_DATA, 0);

	AddInputDesc("INSTANCE_WVP", 0,
		DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16,
		D3D11_INPUT_PER_INSTANCE_DATA, 1);
	AddInputDesc("INSTANCE_WVP", 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16,
		D3D11_INPUT_PER_INSTANCE_DATA, 1);
	AddInputDesc("INSTANCE_WVP", 2,
		DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16,
		D3D11_INPUT_PER_INSTANCE_DATA, 1);
	AddInputDesc("INSTANCE_WVP", 3,
		DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16,
		D3D11_INPUT_PER_INSTANCE_DATA, 1);

	AddInputDesc("INSTANCE_UV", 0,
		DXGI_FORMAT_R32G32_FLOAT, 1, 8,
		D3D11_INPUT_PER_INSTANCE_DATA, 1);
	AddInputDesc("INSTANCE_UV", 1,
		DXGI_FORMAT_R32G32_FLOAT, 1, 8,
		D3D11_INPUT_PER_INSTANCE_DATA, 1);

	AddInputDesc("INSTANCE_COLOR", 0,
		DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16,
		D3D11_INPUT_PER_INSTANCE_DATA, 1);

	AddInputDesc("INSTANCE_PIVOT", 0,
		DXGI_FORMAT_R32G32B32_FLOAT, 1, 12,
		D3D11_INPUT_PER_INSTANCE_DATA, 1);

	AddInputDesc("INSTANCE_TEXTURETYPE", 0,
		DXGI_FORMAT_R32_SINT, 1, 4,
		D3D11_INPUT_PER_INSTANCE_DATA, 1);

	AddInputDesc("INSTANCE_ANIMFRAME", 0,
		DXGI_FORMAT_R32_SINT, 1, 4,
		D3D11_INPUT_PER_INSTANCE_DATA, 1);

	// 입력 레이아웃을 생성한다.
	if (!CreateInputLayout())
		return false;

	return true;
}
