#include "ShaderTileMapInstancing.h"

CShaderTileMapInstancing::CShaderTileMapInstancing()
{
}

CShaderTileMapInstancing::~CShaderTileMapInstancing()
{
}

bool CShaderTileMapInstancing::Init(
	const std::string& PathName)
{
	if (!LoadVertexShader("TileMapInstancingVS",
		TEXT("TileMap.hlsl"),
		PathName))
		return false;

	if (!LoadPixelShader("TileMapInstancingPS",
		TEXT("TileMap.hlsl"),
		PathName))
		return false;

	AddInputDesc("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
		0, 12,
		D3D11_INPUT_PER_VERTEX_DATA, 0);
	AddInputDesc("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8,
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

	// 입력 레이아웃을 생성한다.
	if (!CreateInputLayout())
		return false;

	return true;
}

