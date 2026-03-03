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
		float R, float G, float B)
	{
		Out = CreateComponent<CMeshComponent>(Name);
		auto Face = Out.lock();

		if (!Face)
			return;

		Face->SetShader("MaterialColor2D");
		Face->SetMesh(MeshName);
		Face->SetBlendState(0, "AlphaBlend");
		Face->SetMaterialBaseColor(0, R, G, B, 1.f);
		Face->SetMaterialOpacity(0, 0.85f);
		Face->SetRenderLayer("BuildingVisual");
		Face->SetEnable(false);
	};

	// 좌측 벽 — 밝은 파란 계열
	MakeFace(mLeftWall,  "LeftWall",  "IsoWallLeft",      0.20f, 0.45f, 0.90f);
	// 우측 벽 — 약간 어두운 파란 계열 (면 구분)
	MakeFace(mRightWall, "RightWall", "IsoWallRight",     0.10f, 0.30f, 0.70f);
	// 옥상 — 회색
	MakeFace(mRoof,      "Roof",      "IsoDiamondColor",  0.55f, 0.55f, 0.55f);

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

		if (!mVisible)
			LeftWall->SetEnable(true);
	}

	// ─ 우측 벽 ─ (중심 기준, 회전 없음)
	auto RightWall = mRightWall.lock();

	if (RightWall)
	{
		RightWall->SetRelativePos(0.f, 0.f, 0.f);
		RightWall->SetRelativeScale(ScaleX, ScaleY);

		if (!mVisible)
			RightWall->SetEnable(true);
	}

	// ─ 옥상 ─ (ScaleY 픽셀 위에 배치, 회전 없음)
	auto Roof = mRoof.lock();

	if (Roof)
	{
		Roof->SetRelativePos(0.f, ScaleY, 0.f);
		Roof->SetRelativeScale(ScaleX, ScaleY);

		if (!mVisible)
			Roof->SetEnable(true);
	}

	mVisible = true;
}
