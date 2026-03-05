#include "PlacementBuildingVisual.h"
#include "PlacementAreaObject.h"
#include "../Player/MainCamera.h"
#include "Component/MeshComponent.h"
#include "Component/SceneComponent.h"
#include "Render/RenderManager.h"
#include "World/World.h"
#include "World/WorldAssetManager.h"
#include <string>
#include <vector>

namespace
{
	constexpr const TCHAR* GDefaultBuildingSpriteFile = TEXT("Floors.png");
	constexpr float GSpriteScaleXMultiplier = 1.0f;
	constexpr float GSpriteScaleYMultiplier = 2.0f;
	constexpr float GSpriteOffsetYMultiplier = 0.0f;
	constexpr float GSpriteSortYBiasMultiplier = -0.5f;

	std::vector<std::string> BuildTextureCandidates(
		const CPlacementAreaObject& Building)
	{
		std::vector<std::string> Candidates;
		const std::string& BuildingId = Building.GetBuildingId();

		if (!BuildingId.empty())
			Candidates.push_back(BuildingId + ".png");

		// 항구는 추후 방향 텍스처를 추가할 수 있도록 후보를 미리 넣어둔다.
		if (Building.IsHarbor())
		{
			Candidates.push_back("harbor_up.png");
			Candidates.push_back("harbor_right.png");
			Candidates.push_back("harbor_down.png");
			Candidates.push_back("harbor_left.png");
			Candidates.push_back("harbor.png");
		}

		Candidates.push_back("building_default.png");
		Candidates.push_back("Floors.png");
		return Candidates;
	}
}

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

	CreateComponent<CSceneComponent>("VisualRoot");
	mSprite = CreateComponent<CMeshComponent>("BuildingSprite");
	auto Sprite = mSprite.lock();

	if (Sprite)
	{
		Sprite->SetShader("DefaultTexture2D");
		Sprite->SetMesh("CenterRectTex");
		Sprite->AddTexture(0, "BuildingDefaultTex",
			GDefaultBuildingSpriteFile, "Texture");
		Sprite->SetBlendState(0, "AlphaBlend");
		Sprite->SetMaterialBaseColor(0, 1.f, 1.f, 1.f, 1.f);
		Sprite->SetMaterialOpacity(0, 1.f);
		Sprite->SetRenderLayer("BuildingVisual");
		Sprite->SetEnable(false);
	}

	return true;
}

void CBuildingVisual::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);
	SyncVisuals();
}

bool CBuildingVisual::BindSpriteTexture(
	const CPlacementAreaObject& Building)
{
	auto Sprite = mSprite.lock();
	auto World = mWorld.lock();

	if (!Sprite || !World)
		return false;

	auto AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr)
		return false;

	const std::string& BuildingId = Building.GetBuildingId();
	const std::string& ExplicitTexturePath =
		Building.GetBuildingSpriteTexturePath();
	const std::vector<std::string> Candidates =
		BuildTextureCandidates(Building);
	const std::string TexturePrefix =
		"BuildingSprite_" +
		(BuildingId.empty() ? "Unknown" : BuildingId) + "_";

	if (!ExplicitTexturePath.empty())
	{
		const std::wstring WideTexturePath(
			ExplicitTexturePath.begin(),
			ExplicitTexturePath.end());
		const std::string ExplicitTextureKey =
			TexturePrefix + "explicit";

		if (AssetMgr->LoadTexture(
			ExplicitTextureKey, WideTexturePath.c_str(), "Texture"))
		{
			auto ExplicitTexture =
				AssetMgr->FindTexture(ExplicitTextureKey);

			if (!ExplicitTexture.expired() &&
				Sprite->SetTexture(0, 0, ExplicitTexture))
			{
				mLoadedBuildingId = BuildingId;
				mLoadedTextureFile = ExplicitTexturePath;
				return true;
			}
		}
	}

	for (size_t i = 0; i < Candidates.size(); ++i)
	{
		const std::string& FileName = Candidates[i];
		const std::wstring WideFileName(
			FileName.begin(), FileName.end());
		const std::string TextureKey =
			TexturePrefix + std::to_string(i);

		if (!AssetMgr->LoadTexture(
			TextureKey, WideFileName.c_str(), "Texture"))
		{
			continue;
		}

		auto Texture = AssetMgr->FindTexture(TextureKey);

		if (Texture.expired())
			continue;

		if (Sprite->SetTexture(0, 0, Texture))
		{
			mLoadedBuildingId = BuildingId;
			mLoadedTextureFile = FileName;
			return true;
		}
	}

	mLoadedBuildingId = BuildingId;
	mLoadedTextureFile.clear();
	return false;
}

void CBuildingVisual::SyncVisuals()
{
	auto Building = mBuilding.lock();
	auto Sprite = mSprite.lock();

	if (!Building || !Sprite)
		return;

	if (!Building->HasPlacedArea())
	{
		Sprite->SetEnable(false);
		mVisible = false;
		return;
	}

	// 건물 ID 기준 파일명 규칙: "<building_id>.png"
	// 예: build_1_1.png, starter_harbor.png
	if (mLoadedBuildingId != Building->GetBuildingId())
	{
		BindSpriteTexture(*Building);
	}

	FVector2 TileSize;

	if (!Building->GetTileSize(TileSize))
		return;

	bool IsAnyMovePreviewActive = false;
	bool IsDemolitionModeActive = false;
	auto World = mWorld.lock();

	if (World)
	{
		std::vector<std::weak_ptr<CPlacementAreaObject>> PlacementObjects;
		World->FindObjectListByType<CPlacementAreaObject>(PlacementObjects);

		for (size_t i = 0; i < PlacementObjects.size(); ++i)
		{
			auto PlacementObject = PlacementObjects[i].lock();

			if (!PlacementObject)
				continue;

			if (PlacementObject->IsMovePreviewActive())
			{
				IsAnyMovePreviewActive = true;
				break;
			}
		}

		auto MainCamera =
			World->FindObject<CMainCamera>("MainCamera").lock();

		if (MainCamera && MainCamera->IsDemolitionMode())
			IsDemolitionModeActive = true;
	}

	const float BuildingFaceOpacity =
		(IsAnyMovePreviewActive || IsDemolitionModeActive) ?
		0.35f : 1.f;

	const FVector3 Center = Building->GetWorldPos();
	const int Radius = Building->GetDiamondRadius();
	const int Direction = Building->GetPlacementDirection();
	const float FootprintTiles = Radius * 2.f + 1.f;
	const float BaseScaleX = TileSize.x * FootprintTiles;
	const float BaseScaleY = TileSize.y * FootprintTiles;
	const float SpriteScaleX = BaseScaleX * GSpriteScaleXMultiplier;
	const float SpriteScaleY = BaseScaleY * GSpriteScaleYMultiplier;
	const float UpwardStretchOffsetY =
		BaseScaleY * (GSpriteScaleYMultiplier - 1.f) * 0.5f;
	const float SpriteRotationZ = -90.f * static_cast<float>(Direction);

	SetWorldPos(Center.x, Center.y, Center.z);

	Sprite->SetRelativePos(
		0.f,
		BaseScaleY * GSpriteOffsetYMultiplier + UpwardStretchOffsetY,
		0.f);
	Sprite->SetRelativeRotation(0.f, 0.f, SpriteRotationZ);
	Sprite->SetRelativeScale(SpriteScaleX, SpriteScaleY);
	Sprite->SetRenderSortYBias(
		BaseScaleY * GSpriteSortYBiasMultiplier);
	Sprite->SetRenderSortPriority(0);
	Sprite->SetMaterialOpacity(0, BuildingFaceOpacity);

	if (!mVisible)
		Sprite->SetEnable(true);

	mVisible = true;
}
