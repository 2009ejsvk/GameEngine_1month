#include "MeshManager.h"
#include "Mesh.h"

CMeshManager::CMeshManager()
{
}

CMeshManager::~CMeshManager()
{
}

// -1 ~ 1 -> 0 ~ 1280 -> -640 ~ 640 -> 0 ~ 1280
bool CMeshManager::Init()
{
	// ColorMesh 사각형 생성
	FVertexColor	CenterRectColor[4] =
	{
		FVertexColor(-0.5f, 0.5f, 0.f, 1.f, 0.f, 0.f, 1.f),
		FVertexColor(0.5f, 0.5f, 0.f, 0.f, 1.f, 0.f, 1.f),
		FVertexColor(-0.5f, -0.5f, 0.f, 0.f, 0.f, 1.f, 1.f),
		FVertexColor(0.5f, -0.5f, 0.f, 1.f, 1.f, 0.f, 1.f)
	};

	unsigned short	CenterRectColorIdx[6] = { 0, 1, 3, 0, 3, 2 };

	if (!CreateMesh("Mesh_CenterRectColor",
		true, CenterRectColor, sizeof(FVertexColor),
		4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		CenterRectColorIdx, 2, 6, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	FVector3	CenterFrameRect[4] =
	{
		FVector3(-0.5f, 0.5f, 0.f),
		FVector3(0.5f, 0.5f, 0.f),
		FVector3(-0.5f, -0.5f, 0.f),
		FVector3(0.5f, -0.5f, 0.f)
	};

	unsigned short	CenterFrameRectIdx[5] = { 0, 1, 3, 2, 0 };

	if (!CreateMesh("Mesh_CenterFrameRect",
		true, CenterFrameRect, sizeof(FVector3),
		4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		CenterFrameRectIdx, 2, 5, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	FVector3	LBFrameRect[4] =
	{
		FVector3(0.f, 1.f, 0.f),
		FVector3(1.f, 1.f, 0.f),
		FVector3(0.f, 0.f, 0.f),
		FVector3(1.f, 0.f, 0.f)
	};

	unsigned short	LBFrameRectIdx[5] = { 0, 1, 3, 2, 0 };

	if (!CreateMesh("Mesh_LBFrameRect",
		true, LBFrameRect, sizeof(FVector3),
		4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		LBFrameRectIdx, 2, 5, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;


	FVector3	IsmFrameRect[4] =
	{
		FVector3(0.5f, 1.f, 0.f),
		FVector3(1.f, 0.5f, 0.f),
		FVector3(0.5f, 0.f, 0.f),
		FVector3(0.f, 0.5f, 0.f)
	};

	unsigned short	IsmFrameRectIdx[5] = { 0, 1, 2, 3, 0 };

	if (!CreateMesh("Mesh_IstFrameRect",
		true, IsmFrameRect, sizeof(FVector3),
		4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		IsmFrameRectIdx, 2, 5, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	// FrameSphere2D 생성
	std::vector<FVector3>	FrameSphere2D;

	for (int i = 0; i < 360; i += 12)
	{
		FVector3	Pos;
		Pos.x = cosf(DirectX::XMConvertToRadians((float)i));
		Pos.y = sinf(DirectX::XMConvertToRadians((float)i));

		FrameSphere2D.push_back(Pos);
	}

	std::vector<unsigned short>	FrameSphere2DIdx;

	size_t	PosCount = FrameSphere2D.size();

	for (size_t i = 0; i < PosCount; ++i)
	{
		FrameSphere2DIdx.push_back((unsigned short)i);
	}

	FrameSphere2DIdx.push_back(0);

	if (!CreateMesh("Mesh_FrameSphere2D",
		true, &FrameSphere2D[0],
		sizeof(FVector3),
		(int)PosCount, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		&FrameSphere2DIdx[0], 2, (int)FrameSphere2DIdx.size(),
		DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	// FrameSphere2DColor 생성
	std::vector<FVertexColor>	FrameSphere2DColor;

	for (int i = 0; i < 360; i += 12)
	{
		FVertexColor	Pos;
		Pos.Pos.x = cosf(DirectX::XMConvertToRadians((float)i));
		Pos.Pos.y = sinf(DirectX::XMConvertToRadians((float)i));
		Pos.Pos.z = 0.f;
		Pos.Color = FVector4::White;

		FrameSphere2DColor.push_back(Pos);
	}

	if (!CreateMesh("Mesh_FrameSphere2DColor",
		true, &FrameSphere2DColor[0],
		sizeof(FVertexColor),
		(int)FrameSphere2DColor.size(), D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		&FrameSphere2DIdx[0], 2, (int)FrameSphere2DIdx.size(),
		DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;


	// TextureMesh 사각형 생성
	FVertexTex	CenterRectTexture[4] =
	{
		FVertexTex(-0.5f, 0.5f, 0.f, 0.f, 0.f),
		FVertexTex(0.5f, 0.5f, 0.f, 1.f, 0.f),
		FVertexTex(-0.5f, -0.5f, 0.f, 0.f, 1.f),
		FVertexTex(0.5f, -0.5f, 0.f, 1.f, 1.f)
	};

	if (!CreateMesh("Mesh_CenterRectTex",
		true, CenterRectTexture,
		sizeof(FVertexTex),
		4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		CenterRectColorIdx, 2, 6, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	FVertexTex	RectTexture[4] =
	{
		FVertexTex(0.f, 1.f, 0.f, 0.f, 0.f),
		FVertexTex(1.f, 1.f, 0.f, 1.f, 0.f),
		FVertexTex(0.f, 0.f, 0.f, 0.f, 1.f),
		FVertexTex(1.f, 0.f, 0.f, 1.f, 1.f)
	};

	if (!CreateMesh("Mesh_RectTex",
		true, RectTexture,
		sizeof(FVertexTex),
		4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		CenterRectColorIdx, 2, 6, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	FVertexTex	UIRectTexture[4] =
	{
		FVertexTex(0.f, 0.f, 0.f, 0.f, 0.f),
		FVertexTex(1.f, 0.f, 0.f, 1.f, 0.f),
		FVertexTex(0.f, 1.f, 0.f, 0.f, 1.f),
		FVertexTex(1.f, 1.f, 0.f, 1.f, 1.f)
	};

	if (!CreateMesh("Mesh_UIRectTex",
		true, UIRectTexture,
		sizeof(FVertexTex),
		4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		CenterRectColorIdx, 2, 6, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	FVertexColor	CenterCubeColor[8] =
	{
		FVertexColor(-0.5f, 0.5f, -0.5f, 1.f, 0.f, 0.f, 1.f),
		FVertexColor(0.5f, 0.5f, -0.5f, 0.f, 1.f, 0.f, 1.f),
		FVertexColor(-0.5f, -0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f),
		FVertexColor(0.5f, -0.5f, -0.5f, 1.f, 1.f, 0.f, 1.f),
		FVertexColor(-0.5f, 0.5f, 0.5f, 1.f, 0.f, 0.f, 1.f),
		FVertexColor(0.5f, 0.5f, 0.5f, 0.f, 1.f, 0.f, 1.f),
		FVertexColor(-0.5f, -0.5f, 0.5f, 0.f, 0.f, 1.f, 1.f),
		FVertexColor(0.5f, -0.5f, 0.5f, 1.f, 1.f, 0.f, 1.f)
	};

	unsigned short	CenterCubeColorIdx[36] = { 0, 1, 3, 0, 3, 2,
		1, 5, 7, 1, 7, 3, 5, 4, 6, 5, 6, 7, 4, 0, 2, 4, 2, 6,
		4, 5, 1, 4, 1, 0, 2, 3, 7, 2, 7, 6 };

	if (!CreateMesh("Mesh_CenterCubeColor",
		true, CenterCubeColor,
		sizeof(FVertexColor),
		8, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		CenterCubeColorIdx, 2, 36, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	FVector3	LineUp[2] =
	{
		FVector3(0.f, 0.f, 0.f),
		FVector3(0.f, 1.f, 0.f)
	};

	if (!CreateMesh("Mesh_LineUP2D",
		true, LineUp,
		sizeof(FVector3),
		2, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP))
		return false;

	FVector3	LineRight[2] =
	{
		FVector3(0.f, 0.f, 0.f),
		FVector3(1.f, 0.f, 0.f)
	};

	if (!CreateMesh("Mesh_LineRight2D",
		true, LineRight,
		sizeof(FVector3),
		2, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP))
		return false;

	// ── 아이소메트릭 건물 비주얼 메시 ──────────────────────────────
	// 공용 인덱스 (CW × 2 삼각형)
	unsigned short IsoQuadIdx[6] = { 0, 1, 3, 0, 3, 2 };

	// Mesh_IsoWallLeft — 좌측 평행사변형 (Left-Bottom 엣지)
	// Scale(TileW, TileH) 적용 시: V2→Left_floor, V3→Bottom_floor
	//                               V0→Left_roof,  V1→Bottom_roof
	FVertexColor IsoWallLeft[4] =
	{
		FVertexColor(-0.5f,  1.0f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V0 Left_roof
		FVertexColor( 0.0f,  0.5f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V1 Bottom_roof
		FVertexColor(-0.5f,  0.0f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V2 Left_floor
		FVertexColor( 0.0f, -0.5f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V3 Bottom_floor
	};

	if (!CreateMesh("Mesh_IsoWallLeft",
		true, IsoWallLeft, sizeof(FVertexColor),
		4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		IsoQuadIdx, 2, 6, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	// Mesh_IsoWallRight — 우측 평행사변형 (Bottom-Right 엣지)
	// V0→Bottom_roof, V1→Right_roof, V2→Bottom_floor, V3→Right_floor
	FVertexColor IsoWallRight[4] =
	{
		FVertexColor( 0.0f,  0.5f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V0 Bottom_roof
		FVertexColor( 0.5f,  1.0f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V1 Right_roof
		FVertexColor( 0.0f, -0.5f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V2 Bottom_floor
		FVertexColor( 0.5f,  0.0f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V3 Right_floor
	};

	if (!CreateMesh("Mesh_IsoWallRight",
		true, IsoWallRight, sizeof(FVertexColor),
		4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		IsoQuadIdx, 2, 6, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	// Mesh_IsoDiamondColor — 채워진 다이아몬드 (옥상)
	// V0→Top, V1→Right, V2→Left, V3→Bottom
	FVertexColor IsoDiamond[4] =
	{
		FVertexColor( 0.0f,  0.5f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V0 Top
		FVertexColor( 0.5f,  0.0f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V1 Right
		FVertexColor(-0.5f,  0.0f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V2 Left
		FVertexColor( 0.0f, -0.5f, 0.f, 1.f, 1.f, 1.f, 1.f),	// V3 Bottom
	};

	if (!CreateMesh("Mesh_IsoDiamondColor",
		true, IsoDiamond, sizeof(FVertexColor),
		4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		IsoQuadIdx, 2, 6, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	return true;
}

bool CMeshManager::CreateMesh(const std::string& Name,
	bool Keep, void* VertexData, int VertexSize,
	int VertexCount, D3D11_USAGE VertexUsage,
	D3D11_PRIMITIVE_TOPOLOGY Primitive, void* IndexData,
	int IndexSize, int IndexCount, DXGI_FORMAT Fmt,
	D3D11_USAGE IndexUsage)
{
	std::weak_ptr<CMesh>	Check = FindMesh(Name);

	// 있을 경우
	if (!Check.expired())
		return true;

	std::shared_ptr<CMesh> Mesh;

	Mesh.reset(new CMesh);

	Mesh->SetKeep(Keep);

	if (!Mesh->CreateMesh(VertexData, VertexSize, VertexCount, VertexUsage,
		Primitive, IndexData, IndexSize, IndexCount, Fmt, IndexUsage))
	{
		return false;
	}

	Mesh->SetName(Name);

	mMeshMap.insert(std::make_pair(Name, Mesh));

	return true;
}

std::weak_ptr<CMesh> CMeshManager::FindMesh(
	const std::string& Name)
{
	// 못찾으면 end() 가 반환된다.
	auto iter = mMeshMap.find(Name);

	if (iter == mMeshMap.end())
		return std::weak_ptr<CMesh>();

	return iter->second;
}

void CMeshManager::KeepMesh(const std::string& Name, bool Keep)
{
	auto iter = mMeshMap.find(Name);

	if (iter == mMeshMap.end())
		return;

	iter->second->SetKeep(Keep);
}

void CMeshManager::ReleaseAsset(const std::string& Name)
{
	auto	iter = mMeshMap.find(Name);

	if (iter != mMeshMap.end())
	{
		// 다른 월드에서 더이상 들고 있지 않을 경우
		if (iter->second.use_count() == 1)
		{
			mMeshMap.erase(iter);
		}
	}
}
