#include "ShaderCollider.h"

CShaderCollider::CShaderCollider()
{
}

CShaderCollider::~CShaderCollider()
{
}

bool CShaderCollider::Init(const std::string& PathName)
{
	if (!LoadVertexShader("FrameVS", TEXT("GlobalShader2D.hlsl"),
		PathName))
		return false;

	if (!LoadPixelShader("main",
		TEXT("Collider.hlsl"), PathName))
		return false;

	AddInputDesc("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
		D3D11_INPUT_PER_VERTEX_DATA, 0);

	// 입력 레이아웃을 생성한다.
	if (!CreateInputLayout())
		return false;

	return true;
}
