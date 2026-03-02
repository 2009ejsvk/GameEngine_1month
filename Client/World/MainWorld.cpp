#include "MainWorld.h"
#include "../Player/Player.h"
#include "../Monster/Monster.h"
#include "../Monster/MonsterSpawnPoint.h"
#include "Asset/AssetManager.h"
#include "Asset/Animation2D/Animation2DManager.h"
#include "Component/ColliderBox2D.h"
#include "../UI/MainWidget.h"
#include "World/WorldUIManager.h"
#include "Render/RenderManager.h"
#include "../PostProcess/PostProcessHit.h"
#include "../Map/TileMapMain.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Player/MainCamera.h"
#include <algorithm>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace
{
	struct FBuildingDefinition
	{
		std::string Id;
		EPlacementBuildingKind Kind =
			EPlacementBuildingKind::BuildingB;
		EPlacementTemplateType TemplateType =
			EPlacementTemplateType::Diamond3x3SingleMarker;
	};

	struct FBuildingSpawnData
	{
		std::string Name;
		std::string DefinitionId;
		int OffsetX = 0;
		int OffsetY = 0;
	};

	const FBuildingDefinition* FindBuildingDefinition(
		const std::vector<FBuildingDefinition>& Definitions,
		const std::string& Id)
	{
		const auto It = std::find_if(
			Definitions.begin(), Definitions.end(),
			[&](const FBuildingDefinition& Definition)
			{
				return Definition.Id == Id;
			});

		if (It == Definitions.end())
			return nullptr;

		return &(*It);
	}

	std::vector<FBuildingDefinition> BuildMainWorldBuildingDefinitions()
	{
		std::vector<FBuildingDefinition> Definitions;
		const int DefinitionCount = 30;
		Definitions.reserve(DefinitionCount);

		const EPlacementTemplateType TemplateCycle[4] =
		{
			EPlacementTemplateType::Diamond3x3SingleMarker,
			EPlacementTemplateType::Diamond5x5TwoMarker,
			EPlacementTemplateType::Diamond5x5FourMarker,
			EPlacementTemplateType::Diamond7x7ThreeMarker
		};

		for (int i = 0; i < DefinitionCount; ++i)
		{
			Definitions.push_back({
				"building_" + std::to_string(i + 1),
				(i % 2 == 0) ?
					EPlacementBuildingKind::BuildingA :
					EPlacementBuildingKind::BuildingB,
				TemplateCycle[i % 4]
				});
		}

		return Definitions;
	}

	std::vector<FBuildingSpawnData> BuildMainWorldBuildingSpawns(
		const std::vector<FBuildingDefinition>& Definitions)
	{
		std::vector<FBuildingSpawnData> Spawns;

		const std::pair<int, int> SpawnOffsets[10] =
		{
			{ -18, 0 },
			{ -12, -8 },
			{ -12, 8 },
			{ -6, -14 },
			{ -6, 14 },
			{ 0, -18 },
			{ 0, 18 },
			{ 6, -14 },
			{ 6, 14 },
			{ 12, 0 }
		};

		if (Definitions.empty())
			return Spawns;

		std::vector<size_t> DefinitionIndices(Definitions.size());
		std::iota(
			DefinitionIndices.begin(), DefinitionIndices.end(), 0);

		std::random_device RandomDevice;
		std::mt19937 RandomEngine(RandomDevice());
		std::shuffle(
			DefinitionIndices.begin(),
			DefinitionIndices.end(),
			RandomEngine);

		const size_t SpawnCount = (std::min)(
			static_cast<size_t>(10),
			Definitions.size());
		Spawns.reserve(SpawnCount);

		for (size_t i = 0; i < SpawnCount; ++i)
		{
			const FBuildingDefinition& Definition =
				Definitions[DefinitionIndices[i]];

			Spawns.push_back({
				"Building" + std::to_string(i + 1),
				Definition.Id,
				SpawnOffsets[i].first,
				SpawnOffsets[i].second
				});
		}

		return Spawns;
	}
}

CMainWorld::CMainWorld()
{
}

CMainWorld::~CMainWorld()
{
}

bool CMainWorld::Init()
{
	CWorld::Init();

	// 왼쪽 상단 디버그 렌더타겟(카메라 미리보기) 출력 비활성화.
	CRenderManager::GetInst()->SetDebugTarget(false);

	// LoadAnimation2D();

	// LoadSound();

	// CreateUI();

	// MainWorld 화면 출력 요소는 우선 모두 주석 처리하고
	// 이동 가능한 카메라만 별도로 배치한다.
	auto MainCamera = CreateGameObject<CMainCamera>("MainCamera");

	// 카메라 이동/줌 확인용 더미 오브젝트는 비활성화한다.
	//auto CreateDummyObject = [&](const std::string& Name,
	//	const FVector3& Pos, const FVector3& Scale)
	//{
	//	auto Obj = CreateGameObject<CGameObject>(Name).lock();
	//
	//	if (!Obj)
	//		return;
	//
	//	auto Mesh = Obj->CreateComponent<CMeshComponent>("Mesh").lock();
	//
	//	if (!Mesh)
	//		return;
	//
	//	Mesh->SetShader("Color2D");
	//	Mesh->SetMesh("CenterRectColor");
	//	Mesh->SetWorldScale(Scale);
	//	Mesh->SetWorldPos(Pos);
	//};
	//
	//CreateDummyObject("Dummy_Origin", FVector3(0.f, 0.f, 0.f),
	//	FVector3(220.f, 220.f, 1.f));
	//CreateDummyObject("Dummy_East", FVector3(800.f, 0.f, 0.f),
	//	FVector3(150.f, 150.f, 1.f));
	//CreateDummyObject("Dummy_West", FVector3(-800.f, 0.f, 0.f),
	//	FVector3(150.f, 150.f, 1.f));
	//CreateDummyObject("Dummy_North", FVector3(0.f, 600.f, 0.f),
	//	FVector3(150.f, 150.f, 1.f));
	//CreateDummyObject("Dummy_South", FVector3(0.f, -600.f, 0.f),
	//	FVector3(150.f, 150.f, 1.f));
	//CreateDummyObject("Dummy_Far", FVector3(1800.f, 1200.f, 0.f),
	//	FVector3(240.f, 240.f, 1.f));

	auto TileMap = CreateGameObject<CTileMapMain>("TileMap");
	auto TileMapObj = std::dynamic_pointer_cast<CTileMapObject>(
		TileMap.lock());
	auto FloorBlueTileMap =
		CreateGameObject<CTileMapObject>("TileMapFloorBlue");
	auto FloorBlueTileMapObj = FloorBlueTileMap.lock();
	auto FloorYellowTileMap =
		CreateGameObject<CTileMapObject>("TileMapFloorYellow");
	auto FloorYellowTileMapObj = FloorYellowTileMap.lock();
	auto MainCameraObj = MainCamera.lock();

	if (TileMapObj &&
		FloorBlueTileMapObj &&
		FloorYellowTileMapObj)
	{
		auto BaseTileMap = TileMapObj->GetTileMap().lock();

		auto ResolveLayerOrder = [](
			const std::string& LayerName,
			int PreferredOrder,
			ERenderListSort SortType)
		{
			auto RenderMgr = CRenderManager::GetInst();
			int LayerOrder = RenderMgr->GetLayerOrder(LayerName);

			if (LayerOrder >= 0)
				return LayerOrder;

			for (int Order = PreferredOrder; Order <= 100; ++Order)
			{
				RenderMgr->CreateLayer(LayerName, Order, SortType);
				LayerOrder = RenderMgr->GetLayerOrder(LayerName);

				if (LayerOrder >= 0)
					return LayerOrder;
			}

			return -1;
		};

		auto ConfigureOverlayTileMap = [&](const std::shared_ptr<CTileMapObject>& OverlayObj,
			const std::string& LayerName,
			int PreferredOrder,
			ERenderListSort SortType,
			const FVector4& InitialColor)
		{
			if (!BaseTileMap || !OverlayObj)
				return;

			auto OverlayRender =
				OverlayObj->FindComponent<CTileMapRender>().lock();
			auto OverlayMap = OverlayObj->GetTileMap().lock();

			if (!OverlayRender || !OverlayMap)
				return;

			const int OverlayLayerOrder = ResolveLayerOrder(
				LayerName, PreferredOrder, SortType);

			if (OverlayLayerOrder >= 0)
				OverlayRender->SetRenderLayer(LayerName);

			OverlayRender->EnableTileAlphaBlend();
			OverlayRender->AddTileFrame(0.f, 0.f, 1.f, 1.f);

			OverlayMap->CreateTile(
				BaseTileMap->GetTileShape(),
				BaseTileMap->GetTileCountX(),
				BaseTileMap->GetTileCountY(),
				BaseTileMap->GetTileSize());
			OverlayMap->SetViewCulling(true);
			OverlayMap->SetTileTextureSize(1.f, 1.f);
			OverlayMap->SetTileFrameAll(0);
			OverlayMap->SetTileOutLineRender(false);

			const int TileCount = OverlayMap->GetTileCountX() *
				OverlayMap->GetTileCountY();

			for (int i = 0; i < TileCount; ++i)
			{
				auto Tile = OverlayMap->GetTile(i).lock();

				if (!Tile)
					continue;

				Tile->SetTileType(ETileType::Normal);
				Tile->SetOutLineColor(
					InitialColor.x, InitialColor.y, InitialColor.z, 0.f);
			}

			OverlayObj->SetWorldPos(TileMapObj->GetWorldPos());
		};

		if (BaseTileMap)
		{
			ResolveLayerOrder("MapFloorBlue", 2, ERenderListSort::None);
			ResolveLayerOrder("MarkerOrb", 4, ERenderListSort::Y);

			ConfigureOverlayTileMap(FloorBlueTileMapObj,
				"MapFloorBlue",
				2, ERenderListSort::None,
				FVector4::Blue);
			ConfigureOverlayTileMap(FloorYellowTileMapObj,
				"MapFloorBlue",
				2, ERenderListSort::None,
				FVector4(1.f, 1.f, 0.f, 1.f));
		}
	}

	if (MainCameraObj && TileMapObj)
	{
		auto TileMapComp = TileMapObj->GetTileMap().lock();

		if (TileMapComp)
		{
			const int CountX = TileMapComp->GetTileCountX();
			const int CountY = TileMapComp->GetTileCountY();

			if (CountX > 0 && CountY > 0)
			{
				const int CenterX = CountX / 2;
				const int CenterY = CountY / 2;
				const int CenterIndex = CenterY * CountX + CenterX;
				auto CenterTile = TileMapComp->GetTile(CenterIndex).lock();

				if (CenterTile)
				{
					const FVector2 Center = CenterTile->GetCenter();
					const FVector3 TileMapWorldPos = TileMapObj->GetWorldPos();
					const float CameraZ = MainCameraObj->GetWorldPos().z;

					MainCameraObj->SetWorldPos(
						Center.x + TileMapWorldPos.x,
						Center.y + TileMapWorldPos.y,
						CameraZ);
				}
			}
		}
	}

	const std::vector<FBuildingDefinition> BuildingDefinitions =
		BuildMainWorldBuildingDefinitions();
	const std::vector<FBuildingSpawnData> BuildingSpawns =
		BuildMainWorldBuildingSpawns(BuildingDefinitions);

	std::vector<std::string> BuildingNames;
	BuildingNames.reserve(BuildingSpawns.size());

	auto CreatePlacementBuilding = [&](const FBuildingSpawnData& Spawn)
	{
		const FBuildingDefinition* Definition = FindBuildingDefinition(
			BuildingDefinitions, Spawn.DefinitionId);

		if (!Definition)
			return;

		auto Building = CreateGameObject<CPlacementAreaObject>(Spawn.Name);
		auto BuildingObj = Building.lock();

		if (!BuildingObj)
			return;

		BuildingObj->SetTileMapObject(TileMapObj);
		BuildingObj->SetInitialCenterOffset(Spawn.OffsetX, Spawn.OffsetY);
		BuildingObj->SetBuildingId(Definition->Id);
		BuildingObj->SetBuildingKind(Definition->Kind);
		BuildingObj->SetPlacementTemplateType(Definition->TemplateType);
		BuildingNames.push_back(Spawn.Name);
	};

	for (size_t i = 0; i < BuildingSpawns.size(); ++i)
	{
		CreatePlacementBuilding(BuildingSpawns[i]);
	}

	const int MarkerOrbCount = 200;

	for (int i = 0; i < MarkerOrbCount; ++i)
	{
		const std::string OrbName = i == 0 ?
			"BuildingMarkerOrb" :
			"BuildingMarkerOrb" + std::to_string(i + 1);

		auto MarkerOrb = CreateGameObject<CBuildingMarkerOrb>(OrbName);
		auto MarkerOrbObj = MarkerOrb.lock();

		if (!MarkerOrbObj)
			continue;

		MarkerOrbObj->SetRandomTargetNames(BuildingNames);

		const float SpeedBase = 280.f;
		const float SpeedVariance = 42.f;
		const float Speed = SpeedBase +
			((float)(rand() % 1001) / 500.f - 1.f) * SpeedVariance;
		MarkerOrbObj->SetMoveSpeed(Speed);
	}
	// TileMap.lock()->LoadTileMap(TEXT("Map/MainMap.tlm"), "Asset");
	// std::weak_ptr<CPlayer>	Player = CreateGameObject<CPlayer>("Player");

	//for (int i = 0; i < 30; ++i)
	//{
	//	std::weak_ptr<CMonster>	Monster1 = CreateGameObject<CMonster>("Monster");

	//	std::shared_ptr<CMonster>	Monster = Monster1.lock();

	//	if (Monster)
	//	{
	//		Monster->SetWorldPos(-500.f + i * 30.f, 300.f);
	//		//Monster->SetWorldRotationZ(180.f);
	//	}
	//}

	// std::weak_ptr<CGameObject>	Wall = CreateGameObject<CGameObject>("Wall");
	// auto	WallObj = Wall.lock();
	// auto	WallBox =
	// 	WallObj->CreateComponent<CColliderBox2D>("Wall").lock();
	// WallBox->SetCollisionProfile("Static");
	// WallBox->SetBoxSize(500.f, 50.f);
	// WallBox->SetDebugDraw(true);
	// WallBox->SetInheritScale(false);
	// WallBox->SetWorldPos(0.f, -200.f);
	// WallBox->SetStatic(true);
	//WallBox->SetWorldRotationZ(45.f);

	/*std::weak_ptr<CMonster>	Monster1 = CreateGameObject<CMonster>("Monster");

	auto Monster = Monster1.lock();

	if (Monster)
	{
		Monster->SetWorldPos(400.f, 300.f);
		Monster->SetWorldRotationZ(180.f);
	}

	std::weak_ptr<CMonsterSpawnPoint>	SpawnPoint1 = CreateGameObject<CMonsterSpawnPoint>("SpawnPoint");

	std::shared_ptr<CMonsterSpawnPoint>	Point = SpawnPoint1.lock();

	if (Point)
	{
		Point->SetWorldPos(100.f, 0.f);
		Point->SetWorldRotationZ(20.f);
		Point->SetSpawnClass<CMonster>();
		Point->SetSpawnTime(5.f);
	}*/

	// if (!CRenderManager::GetInst()->CheckPostProcess("Hit"))
	// {
	// 	auto Hit = CRenderManager::GetInst()->CreatePostProcess<CPostProcessHit>("Hit", 3).lock();
	// 	Hit->SetEnable(false);
	// }

	return true;
}

void CMainWorld::LoadAnimation2D()
{
	mWorldAssetManager->CreateAnimation("PlayerIdle");
	mWorldAssetManager->SetAnimation2DTextureType(
		"PlayerIdle", EAnimation2DTextureType::Frame);

	std::vector<const TCHAR*>	TextureFileName;

	for (int i = 0; i < 7; ++i)
	{
		//TCHAR	FileName[MAX_PATH] = {};
		TCHAR* FileName = new TCHAR[MAX_PATH];
		memset(FileName, 0, sizeof(TCHAR) * MAX_PATH);
		wsprintf(FileName, 
			TEXT("Player/PlayerFrame/adventurer-get-up-0%d.png"), i);
		TextureFileName.push_back(FileName);
	}

	mWorldAssetManager->SetTexture("PlayerIdle", "PlayerIdle",
		TextureFileName);

	for (int i = 0; i < 7; ++i)
	{
		delete[] TextureFileName[i];
	}
	TextureFileName.clear();

	mWorldAssetManager->AddFrame("PlayerIdle", 7, 0.f, 0.f, 50.f, 37.f);

	mWorldAssetManager->CreateAnimation("PlayerWalk");
	mWorldAssetManager->SetAnimation2DTextureType("PlayerWalk",
		EAnimation2DTextureType::SpriteSheet);

	mWorldAssetManager->SetTexture("PlayerWalk", "PlayerSheet",
		TEXT("Player/Player.png"));

	mWorldAssetManager->AddFrame("PlayerWalk", 200.f, 222.f, 50.f, 37.f);
	mWorldAssetManager->AddFrame("PlayerWalk", 250.f, 222.f, 50.f, 37.f);
	mWorldAssetManager->AddFrame("PlayerWalk", 300.f, 222.f, 50.f, 37.f);
	mWorldAssetManager->AddFrame("PlayerWalk", 0.f, 259.f, 50.f, 37.f);
	mWorldAssetManager->AddFrame("PlayerWalk", 50.f, 259.f, 50.f, 37.f);
	mWorldAssetManager->AddFrame("PlayerWalk", 100.f, 259.f, 50.f, 37.f);

	mWorldAssetManager->CreateAnimation("PlayerAttack");
	mWorldAssetManager->SetAnimation2DTextureType("PlayerAttack",
		EAnimation2DTextureType::SpriteSheet);

	mWorldAssetManager->SetTexture("PlayerAttack", "PlayerSheet",
		TEXT("Player/Player.png"));

	mWorldAssetManager->AddFrame("PlayerAttack", 0.f, 0.f, 50.f, 37.f);
	mWorldAssetManager->AddFrame("PlayerAttack", 50.f, 0.f, 50.f, 37.f);
	mWorldAssetManager->AddFrame("PlayerAttack", 100.f, 0.f, 50.f, 37.f);
	mWorldAssetManager->AddFrame("PlayerAttack", 150.f, 0.f, 50.f, 37.f);

	mWorldAssetManager->CreateAnimation("MonsterIdle");
	mWorldAssetManager->SetAnimation2DTextureType(
		"MonsterIdle",
		EAnimation2DTextureType::SpriteSheet);

	mWorldAssetManager->SetTexture("MonsterIdle",
		"MonsterSheet",
		TEXT("Monster.png"));

	for (int i = 0; i < 14; ++i)
	{
		mWorldAssetManager->AddFrame("MonsterIdle", i * 45.f, 60.f,
			45.f, 60.f);
	}

	mWorldAssetManager->CreateAnimation("MonsterAttack");
	mWorldAssetManager->SetAnimation2DTextureType("MonsterAttack",
		EAnimation2DTextureType::SpriteSheet);

	mWorldAssetManager->SetTexture("MonsterAttack", "MonsterSheet",
		TEXT("Monster.png"));

	for (int i = 0; i < 21; ++i)
	{
		mWorldAssetManager->AddFrame("MonsterAttack", i * 45.f,
			180.f, 45.f, 60.f);
	}


	mWorldAssetManager->CreateAnimation("Explosion");
	mWorldAssetManager->SetAnimation2DTextureType("Explosion",
		EAnimation2DTextureType::Array);

	for (int i = 1; i <= 89; ++i)
	{
		//TCHAR	FileName[MAX_PATH] = {};
		TCHAR* FileName = new TCHAR[MAX_PATH];
		memset(FileName, 0, sizeof(TCHAR) * MAX_PATH);
		wsprintf(FileName,
			TEXT("Explosion/Explosion%d.png"), i);
		TextureFileName.push_back(FileName);
	}

	mWorldAssetManager->SetTextureArray("Explosion", 
		"Explosion", TextureFileName);

	for (int i = 0; i < 89; ++i)
	{
		delete[] TextureFileName[i];
	}
	TextureFileName.clear();

	mWorldAssetManager->AddFrame("Explosion", 89, 0.f, 0.f, 320.f, 240.f);
}

void CMainWorld::LoadSound()
{
	mWorldAssetManager->LoadSound("MainBGM", "BGM", true,
		"MainBgm.mp3");

	mWorldAssetManager->LoadSound("Fire", "Effect", false,
		"Fire1.wav");

	//mWorldAssetManager->SoundPlay("MainBGM");
}

void CMainWorld::CreateUI()
{
	std::weak_ptr<CMainWidget>	MainWidget =
		mUIManager->CreateWidget<CMainWidget>("MainWidget");
}
