#include "PlacementBuildingVisual.h"
#include "PlacementAreaObject.h"
#include "Component/MeshComponent.h"
#include "Component/SceneComponent.h"
#include "Render/RenderManager.h"

CBuildingVisual::CBuildingVisual()
{
	SetClassType<CBuildingVisual>();
}

CBuildingVisual::CBuildingVisual(const CBuildingVisual& ref) :
	CGameObject(ref)
{
}

CBuildingVisual::CBuildingVisual(CBuildingVisual&& ref) noexcept :
	CGameObject(std::move(ref))
{
}

CBuildingVisual::~CBuildingVisual()
{
}

bool CBuildingVisual::Init()
{
	CGameObject::Init();

	auto RenderMgr = CRenderManager::GetInst();

	if (RenderMgr->GetLayerOrder("BuildingVisual") < 0)
	{
		for (int Order = 3; Order <= 100; ++Order)
		{
			RenderMgr->CreateLayer(
				"BuildingVisual", Order, ERenderListSort::Y);

			if (RenderMgr->GetLayerOrder("BuildingVisual") >= 0)
				break;
		}
	}

	// 컴포넌트 계층: root → 각 face
	CreateComponent<CSceneComponent>("VisualRoot");

	auto MakeFace = [&](
		std::weak_ptr<CMeshComponent>& Out,
		const char* Name,
		const char* MeshName,
		const char* TextureName,
		const TCHAR* TextureFileName)
	{
		Out = CreateComponent<CMeshComponent>(Name);
		auto Face = Out.lock();

		if (!Face)
			return;

		Face->SetShader("DefaultTexture2D");
		Face->SetMesh(MeshName);
		Face->AddTexture(0, TextureName, TextureFileName, "Texture");
		Face->SetBlendState(0, "AlphaBlend");
		Face->SetMaterialBaseColor(0, 1.f, 1.f, 1.f, 1.f);
		Face->SetMaterialOpacity(0, 1.f);
		Face->SetRenderLayer("BuildingVisual");
		Face->SetEnable(false);
	};

	// 면별 텍스처를 개별 바인딩한다.
	MakeFace(mLeftWall,  "LeftWall",  "IsoWallLeftTex",
		"BuildingWallLeftTex",  TEXT("wall.png"));
	MakeFace(mRightWall, "RightWall", "IsoWallRightTex",
		"BuildingWallRightTex", TEXT("wall.png"));
	// 지붕은 전용 텍스처를 사용한다.
	MakeFace(mRoof,      "Roof",      "IsoDiamondTex",
		"BuildingRoofTex",      TEXT("Floors.png"));

	return true;
}

void CBuildingVisual::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);
	SyncVisuals();
}

void CBuildingVisual::SyncVisuals()
{
	auto Building = mBuilding.lock();

	if (!Building)
		return;

	// 건물이 타일맵에 배치 완료될 때까지 대기
	FVector2 TileSize;

	if (!Building->GetTileSize(TileSize))
		return;

	const FVector3 Center = Building->GetWorldPos();
	const int      Radius = Building->GetDiamondRadius();

	// 이 오브젝트 자체를 건물 중심에 맞춘다
	SetWorldPos(Center.x, Center.y, Center.z);

	// Radius는 "점유 다이아몬드 반경"이며 실제 폭/높이 타일 수는 (2R + 1)이다.
	// 정점 좌표는 이미 아이소 형태이므로 footprint 타일 수만 스케일에 반영한다.
	const float FootprintTiles = Radius * 2.f + 1.f;
	const float ScaleX = TileSize.x * FootprintTiles;
	const float ScaleY = TileSize.y * FootprintTiles;

	// ─ 좌측 벽 ─ (중심 기준, 회전 없음)
	auto LeftWall = mLeftWall.lock();

	if (LeftWall)
	{
		LeftWall->SetRelativePos(0.f, 0.f, 0.f);
		LeftWall->SetRelativeScale(ScaleX, ScaleY);
		// 아이소 기준 깊이 앵커를 중심이 아닌 footprint 하단점으로 맞춘다.
		LeftWall->SetRenderSortYBias(-ScaleY * 0.5f);
		LeftWall->SetRenderSortPriority(0);

		if (!mVisible)
			LeftWall->SetEnable(true);
	}

	// ─ 우측 벽 ─ (중심 기준, 회전 없음)
	auto RightWall = mRightWall.lock();

	if (RightWall)
	{
		RightWall->SetRelativePos(0.f, 0.f, 0.f);
		RightWall->SetRelativeScale(ScaleX, ScaleY);
		RightWall->SetRenderSortYBias(-ScaleY * 0.5f);
		RightWall->SetRenderSortPriority(1);

		if (!mVisible)
			RightWall->SetEnable(true);
	}

	// ─ 옥상 ─ (ScaleY 픽셀 위에 배치, 회전 없음)
	auto Roof = mRoof.lock();

	if (Roof)
	{
		Roof->SetRelativePos(0.f, ScaleY, 0.f);
		Roof->SetRelativeScale(ScaleX, ScaleY);
		// 지붕은 실제 위치를 올리되, 정렬 키는 벽과 동일한 footprint 하단점에 고정한다.
		Roof->SetRenderSortYBias(-ScaleY * 1.5f);
		Roof->SetRenderSortPriority(2);

		if (!mVisible)
			Roof->SetEnable(true);
	}

	mVisible = true;
}

