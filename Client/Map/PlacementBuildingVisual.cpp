#include "PlacementBuildingVisual.h"
#include "PlacementAreaObject.h"
#include "PlacementController.h"
#include "../ObjectNames.h"
#include "Component/CameraComponent.h"
#include "Component/MeshComponent.h"
#include "Component/SceneComponent.h"
#include "Device.h"
#include "Render/RenderManager.h"
#include "World/CameraManager.h"
#include "World/World.h"
#include "World/WorldAssetManager.h"
#include "../Building/BuildingCatalog.h"
#include "../StringUtils.h"
#include <DirectXMath.h>
#include <array>
#include <cmath>
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
		switch (Building.GetBuildingKind())
		{
		case EPlacementBuildingKind::Harbor:
			// 항구는 방향별 대체 텍스처를 둘 수 있다.
			Candidates.push_back("harbor_up.png");
			Candidates.push_back("harbor_right.png");
			Candidates.push_back("harbor_down.png");
			Candidates.push_back("harbor_left.png");
			Candidates.push_back("harbor.png");
			Candidates.push_back("harbor_default.png");
			break;
		case EPlacementBuildingKind::TransportOffice:
			Candidates.push_back("transport_office_default.png");
			break;
		case EPlacementBuildingKind::Road:
			Candidates.push_back("road_default.png");
			break;
		case EPlacementBuildingKind::Structure:
		default:
			Candidates.push_back("building_default.png");
			break;
		}

		Candidates.push_back("Floors.png");
		return Candidates;
	}

	std::string ResolveSpriteTexturePath(
		const CPlacementAreaObject& Building)
	{
		const FBuildingCatalogEntry* CatalogEntry =
			FindBuildingCatalogEntry(Building.GetBuildingId());

		if (CatalogEntry)
		{
			const std::string CatalogTexturePath =
				GetCatalogEntrySpriteTexturePathUtf8(*CatalogEntry);

			if (!CatalogTexturePath.empty())
				return CatalogTexturePath;
		}

		return Building.GetBuildingSpriteTexturePath();
	}

	bool ProjectToScreen(
		const FVector3& LocalPos,
		const FMatrix& WorldMatrix,
		const FMatrix& ViewMatrix,
		const FMatrix& ProjMatrix,
		float ScreenWidth,
		float ScreenHeight,
		FVector2& OutScreenPos)
	{
		const DirectX::XMVECTOR Projected =
			DirectX::XMVector3Project(
				DirectX::XMVectorSet(
					LocalPos.x,
					LocalPos.y,
					LocalPos.z,
					1.f),
				0.f,
				0.f,
				ScreenWidth,
				ScreenHeight,
				0.f,
				1.f,
				ProjMatrix.m,
				ViewMatrix.m,
				WorldMatrix.m);
		OutScreenPos.x = DirectX::XMVectorGetX(Projected);
		OutScreenPos.y = DirectX::XMVectorGetY(Projected);
		return
			std::isfinite(OutScreenPos.x) &&
			std::isfinite(OutScreenPos.y);
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

bool CBuildingVisual::TryGetProjectedScreenBounds(
	FVector2& OutMin,
	FVector2& OutMax,
	FVector2& OutCenter) const
{
	OutMin = FVector2();
	OutMax = FVector2();
	OutCenter = FVector2();

	auto Sprite = mSprite.lock();
	auto World = mWorld.lock();

	if (!Sprite || !World || !Sprite->GetEnable())
		return false;

	auto CameraManager = World->GetCameraManager().lock();

	if (!CameraManager)
		return false;

	auto MainCamera = CameraManager->GetMainCamera().lock();

	if (!MainCamera)
		return false;

	const FResolution& Resolution = CDevice::GetInst()->GetResolution();
	const float ScreenWidth = static_cast<float>(Resolution.Width);
	const float ScreenHeight = static_cast<float>(Resolution.Height);
	FMatrix WorldMatrix =
		FMatrix::StaticScaling(Sprite->GetWorldScale()) *
		FMatrix::StaticRotation(Sprite->GetWorldRot()) *
		FMatrix::StaticTranslation(Sprite->GetWorldPos());
	const FMatrix& ViewMatrix = MainCamera->GetViewMatrix();
	const FMatrix& ProjMatrix = MainCamera->GetProjMatrix();
	const std::array<FVector3, 4> LocalCorners =
	{
		FVector3(-0.5f, -0.5f, 0.f),
		FVector3(0.5f, -0.5f, 0.f),
		FVector3(0.5f, 0.5f, 0.f),
		FVector3(-0.5f, 0.5f, 0.f)
	};
	std::array<FVector2, 4> ScreenCorners;

	for (size_t Index = 0; Index < LocalCorners.size(); ++Index)
	{
		if (!ProjectToScreen(
				LocalCorners[Index],
				WorldMatrix,
				ViewMatrix,
				ProjMatrix,
				ScreenWidth,
				ScreenHeight,
				ScreenCorners[Index]))
		{
			return false;
		}
	}

	OutMin = ScreenCorners[0];
	OutMax = ScreenCorners[0];

	for (size_t Index = 1; Index < ScreenCorners.size(); ++Index)
	{
		OutMin.x = (std::min)(OutMin.x, ScreenCorners[Index].x);
		OutMin.y = (std::min)(OutMin.y, ScreenCorners[Index].y);
		OutMax.x = (std::max)(OutMax.x, ScreenCorners[Index].x);
		OutMax.y = (std::max)(OutMax.y, ScreenCorners[Index].y);
	}

	OutCenter.x = (OutMin.x + OutMax.x) * 0.5f;
	OutCenter.y = (OutMin.y + OutMax.y) * 0.5f;
	return true;
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
	const std::string ExplicitTexturePath =
		ResolveSpriteTexturePath(Building);
	const std::vector<std::string> Candidates =
		BuildTextureCandidates(Building);
	const unsigned long long CatalogGeneration =
		::GetRuntimeConfigGeneration();
	const std::string TexturePrefix =
		"BuildingSprite_" +
		(BuildingId.empty() ? "Unknown" : BuildingId) + "_";
	const std::string GenerationSuffix =
		"Gen_" + std::to_string(CatalogGeneration) + "_";

	if (!ExplicitTexturePath.empty())
	{
		const std::wstring WideTexturePath =
			ResolveCatalogTextureFullPath(
				StringUtils::Utf8ToWide(ExplicitTexturePath).c_str());
		const std::string ExplicitTextureKey =
			TexturePrefix + GenerationSuffix + "explicit";

		if (!WideTexturePath.empty() &&
			AssetMgr->LoadTextureFullPath(
				ExplicitTextureKey,
				WideTexturePath.c_str()))
		{
			auto ExplicitTexture =
				AssetMgr->FindTexture(ExplicitTextureKey);

			if (!ExplicitTexture.expired() &&
				Sprite->SetTexture(0, 0, ExplicitTexture))
			{
				mLoadedBuildingId = BuildingId;
				mLoadedTextureFile = ExplicitTexturePath;
				mLoadedCatalogGeneration = CatalogGeneration;
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
			TexturePrefix +
			GenerationSuffix +
			std::to_string(i);

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
			mLoadedCatalogGeneration = CatalogGeneration;
			return true;
		}
	}

	mLoadedBuildingId.clear();
	mLoadedTextureFile.clear();
	mLoadedCatalogGeneration = 0;
	return false;
}

void CBuildingVisual::SyncVisuals()
{
	auto Building = mBuilding.lock();
	auto Sprite = mSprite.lock();

	if (!Building || !Sprite)
		return;

	if (!Building->HasPlacedArea() ||
		Building->IsRoad())
	{
		Sprite->SetEnable(false);
		mVisible = false;
		return;
	}

	if (mLoadedBuildingId != Building->GetBuildingId() ||
		mLoadedCatalogGeneration != ::GetRuntimeConfigGeneration())
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

		auto PlacementCtrl =
			World->FindObject<CPlacementController>(GPlacementControllerName).lock();

		if (PlacementCtrl && PlacementCtrl->IsDemolitionMode())
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
