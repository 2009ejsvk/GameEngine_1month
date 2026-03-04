#include "MainWorld.h"
#include "../Player/Player.h"
#include "../Monster/Monster.h"
#include "../Monster/MonsterSpawnPoint.h"
#include "Asset/AssetManager.h"
#include "Asset/Animation2D/Animation2DManager.h"
#include "Component/ColliderBox2D.h"
#include "../UI/BuildMenuWidget.h"
#include "World/WorldUIManager.h"
#include "Render/RenderManager.h"
#include "../PostProcess/PostProcessHit.h"
#include "../Map/TileMapMain.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementBuildingVisual.h"
#include "../Player/MainCamera.h"
#include <algorithm>
#include <numeric>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
	constexpr int GInitialNpcCount = 10;
	constexpr int GMaxNpcCount = 2000;
	constexpr float GNpcSpawnInterval = 5.f;
	constexpr float GCitizenReassignInterval = 0.5f;
	constexpr float GNpcSpeedBase = 140.f;
	constexpr float GNpcSpeedVariance = 21.f;
	constexpr const char* GStarterHousingObjectName = "StarterHousing";
	constexpr const char* GStarterWorkObjectName = "StarterWork";
	constexpr const char* GStarterFoodObjectName = "StarterFood";
	constexpr const char* GStarterFunObjectName = "StarterFun";

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

	std::string PickRandomBuildingName(
		const std::vector<std::string>& Names)
	{
		if (Names.empty())
			return std::string();

		return Names[rand() % Names.size()];
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

	CreateUI();

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
			ResolveLayerOrder("MapFloorBlue",    2, ERenderListSort::None);
			ResolveLayerOrder("BuildingVisual",  3, ERenderListSort::Y);

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

	auto CreateStarterBuilding = [&](const std::string& ObjectName,
		const std::string& BuildingId,
		const std::string& DisplayName,
		const std::string& CategoryName,
		bool Residential,
		int Capacity,
		bool FoodProvider,
		bool EntertainmentProvider,
		int HousingSatisfactionCap,
		int JobSatisfactionCap,
		int FoodSatisfactionCap,
		int FunSatisfactionCap,
		EPlacementBuildingKind Kind,
		EPlacementTemplateType TemplateType,
		int OffsetX,
		int OffsetY)
	{
		auto Building = CreateGameObject<CPlacementAreaObject>(ObjectName);
		auto BuildingObj = Building.lock();

		if (!BuildingObj)
			return;

		BuildingObj->SetTileMapObject(TileMapObj);
		BuildingObj->SetInitialCenterOffset(OffsetX, OffsetY);
		BuildingObj->SetBuildingId(BuildingId);
		BuildingObj->SetBuildingDisplayInfo(
			DisplayName,
			CategoryName,
			Residential,
			Capacity,
			FoodProvider,
			EntertainmentProvider,
			HousingSatisfactionCap,
			JobSatisfactionCap,
			FoodSatisfactionCap,
			FunSatisfactionCap);
		BuildingObj->SetBuildingKind(Kind);
		BuildingObj->SetPlacementTemplateType(TemplateType);

		auto Visual = CreateGameObject<CBuildingVisual>(
			ObjectName + "_Visual");
		auto VisualObj = Visual.lock();

		if (VisualObj)
			VisualObj->SetBuilding(Building);
	};

	CreateStarterBuilding(
		GStarterHousingObjectName,
		"starter_housing",
		"판잣집",
		"주거지",
		true,
		30,
		false,
		false,
		10,
		100,
		100,
		100,
		EPlacementBuildingKind::BuildingA,
		EPlacementTemplateType::Diamond5x5TwoMarker,
		0,
		0);

	CreateStarterBuilding(
		GStarterWorkObjectName,
		"starter_office",
		"건설 사무소",
		"교통 및 기반시설",
		false,
		25,
		false,
		false,
		100,
		55,
		100,
		100,
		EPlacementBuildingKind::BuildingB,
		EPlacementTemplateType::Diamond5x5TwoMarker,
		-12,
		8);

	CreateStarterBuilding(
		GStarterFoodObjectName,
		"starter_food",
		"패스트푸드 체인점",
		"유흥",
		false,
		25,
		true,
		true,
		100,
		100,
		55,
		52,
		EPlacementBuildingKind::BuildingA,
		EPlacementTemplateType::Diamond5x5FourMarker,
		12,
		8);

	CreateStarterBuilding(
		GStarterFunObjectName,
		"starter_fun",
		"주점",
		"유흥",
		false,
		20,
		false,
		true,
		100,
		100,
		100,
		45,
		EPlacementBuildingKind::BuildingB,
		EPlacementTemplateType::Diamond3x3SingleMarker,
		0,
		-12);

	mSpawnedNpcCount = 0;
	mNpcSpawnAccum = 0.f;
	mCitizenReassignAccum = 0.f;

	for (int i = 0; i < GInitialNpcCount; ++i)
	{
		SpawnCitizenOrb();
	}

	ReassignCitizenNeeds();
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

void CMainWorld::Update(float DeltaTime)
{
	CWorld::Update(DeltaTime);

	mCitizenReassignAccum += DeltaTime;

	while (mCitizenReassignAccum >= GCitizenReassignInterval)
	{
		mCitizenReassignAccum -= GCitizenReassignInterval;
		ReassignCitizenNeeds();
	}

	if (mSpawnedNpcCount < GMaxNpcCount)
	{
		mNpcSpawnAccum += DeltaTime;

		while (mNpcSpawnAccum >= GNpcSpawnInterval &&
			mSpawnedNpcCount < GMaxNpcCount)
		{
			mNpcSpawnAccum -= GNpcSpawnInterval;
			SpawnCitizenOrb();
		}
	}
}

void CMainWorld::SpawnCitizenOrb()
{
	if (mSpawnedNpcCount >= GMaxNpcCount)
		return;

	const int OrbIndex = mSpawnedNpcCount;
	const std::string OrbName = (OrbIndex == 0) ?
		"BuildingMarkerOrb" :
		"BuildingMarkerOrb" + std::to_string(OrbIndex + 1);

	auto MarkerOrb = CreateGameObject<CBuildingMarkerOrb>(OrbName);
	auto MarkerOrbObj = MarkerOrb.lock();

	if (!MarkerOrbObj)
		return;

	std::vector<std::string> AllNames;
	std::vector<std::string> HomeNames;
	std::vector<std::string> WorkNames;
	std::vector<std::string> FoodNames;
	std::vector<std::string> FunNames;
	CollectCurrentBuildingNames(AllNames);
	CollectHomeBuildingNames(HomeNames);
	CollectWorkBuildingNames(WorkNames);
	CollectFoodBuildingNames(FoodNames);
	CollectEntertainmentBuildingNames(FunNames);

	MarkerOrbObj->SetRandomTargetNames(AllNames);

	if (!HomeNames.empty() &&
		!WorkNames.empty() &&
		!FoodNames.empty())
	{
		MarkerOrbObj->SetHomeBuilding(PickRandomBuildingName(HomeNames));
		MarkerOrbObj->SetWorkBuilding(PickRandomBuildingName(WorkNames));
		MarkerOrbObj->SetFoodBuilding(PickRandomBuildingName(FoodNames));
	}

	if (!FunNames.empty())
		MarkerOrbObj->SetFunBuilding(PickRandomBuildingName(FunNames));

	const float Speed = GNpcSpeedBase +
		((float)(rand() % 1001) / 500.f - 1.f) * GNpcSpeedVariance;
	MarkerOrbObj->SetMoveSpeed(Speed);
	++mSpawnedNpcCount;
}

void CMainWorld::ReassignCitizenNeeds()
{
	struct FFoodBuildingInfo
	{
		std::string Name;
		int FoodCap = 0;
		int Assigned = 0;
	};

	struct FWorkBuildingInfo
	{
		std::string Name;
		int Capacity = 0;
		int JobCap = 0;
		int Occupied = 0;
		int MinRequired = 0;
		bool IsFoodProvider = false;
	};

	std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

	if (!FindObjectListByType<CBuildingMarkerOrb>(OrbList))
		return;

	std::vector<std::string> AllNames;
	std::vector<std::string> HomeNames;
	std::vector<std::string> WorkNames;
	std::vector<std::string> FoodNames;
	std::vector<std::string> FunNames;
	CollectCurrentBuildingNames(AllNames);
	CollectHomeBuildingNames(HomeNames);
	CollectWorkBuildingNames(WorkNames);
	CollectFoodBuildingNames(FoodNames);
	CollectEntertainmentBuildingNames(FunNames);

	std::vector<FWorkBuildingInfo> WorkInfos;
	WorkInfos.reserve(WorkNames.size());

	for (size_t i = 0; i < WorkNames.size(); ++i)
	{
		auto WorkBuilding =
			FindObject<CPlacementAreaObject>(WorkNames[i]).lock();

		if (!WorkBuilding ||
			!WorkBuilding->GetAlive() ||
			!WorkBuilding->GetEnable() ||
			!WorkBuilding->HasPlacedArea())
		{
			continue;
		}

		FWorkBuildingInfo Info;
		Info.Name = WorkNames[i];
		Info.Capacity = (std::max)(0, WorkBuilding->GetCapacity());
		Info.JobCap = WorkBuilding->GetJobSatisfactionCap();
		Info.IsFoodProvider = WorkBuilding->IsFoodProvider();
		WorkInfos.push_back(std::move(Info));
	}

	std::sort(WorkInfos.begin(), WorkInfos.end(),
		[](const FWorkBuildingInfo& A, const FWorkBuildingInfo& B)
		{
			if (A.JobCap != B.JobCap)
				return A.JobCap > B.JobCap;

			if (A.Capacity != B.Capacity)
				return A.Capacity > B.Capacity;

			return A.Name < B.Name;
		});

	std::unordered_map<std::string, size_t> WorkIndexByName;
	WorkIndexByName.reserve(WorkInfos.size());

	for (size_t i = 0; i < WorkInfos.size(); ++i)
	{
		WorkIndexByName.emplace(WorkInfos[i].Name, i);
	}

	std::vector<std::shared_ptr<CBuildingMarkerOrb>> ActiveOrbs;
	ActiveOrbs.reserve(OrbList.size());

	for (size_t i = 0; i < OrbList.size(); ++i)
	{
		auto Orb = OrbList[i].lock();

		if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
			continue;

		ActiveOrbs.push_back(Orb);

		for (size_t NameIndex = 0; NameIndex < AllNames.size(); ++NameIndex)
		{
			Orb->AddTargetBuildingName(AllNames[NameIndex]);
		}

		const ECitizenState OrbState = Orb->GetCitizenState();
		const bool IsInFoodVisit =
			OrbState == ECitizenState::GoingToFood ||
			OrbState == ECitizenState::AtFood;

		if (Orb->GetHomeBuilding().empty() && !HomeNames.empty())
			Orb->SetHomeBuilding(PickRandomBuildingName(HomeNames));

		if (!IsInFoodVisit &&
			Orb->GetFoodBuilding().empty() &&
			!FoodNames.empty())
		{
			Orb->SetFoodBuilding(PickRandomBuildingName(FoodNames));
		}

		if (Orb->GetFunBuilding().empty() && !FunNames.empty())
			Orb->SetFunBuilding(PickRandomBuildingName(FunNames));
	}

	if (ActiveOrbs.empty())
		return;

	std::vector<FFoodBuildingInfo> FoodInfos;
	FoodInfos.reserve(FoodNames.size());

	for (size_t i = 0; i < FoodNames.size(); ++i)
	{
		auto FoodBuilding =
			FindObject<CPlacementAreaObject>(FoodNames[i]).lock();

		if (!FoodBuilding ||
			!FoodBuilding->GetAlive() ||
			!FoodBuilding->GetEnable() ||
			!FoodBuilding->HasPlacedArea())
		{
			continue;
		}

		FFoodBuildingInfo Info;
		Info.Name = FoodNames[i];
		Info.FoodCap = FoodBuilding->GetFoodSatisfactionCap();
		FoodInfos.push_back(std::move(Info));
	}

	std::sort(FoodInfos.begin(), FoodInfos.end(),
		[](const FFoodBuildingInfo& A, const FFoodBuildingInfo& B)
		{
			if (A.FoodCap != B.FoodCap)
				return A.FoodCap > B.FoodCap;

			return A.Name < B.Name;
		});

	std::unordered_map<std::string, size_t> FoodIndexByName;
	FoodIndexByName.reserve(FoodInfos.size());

	for (size_t i = 0; i < FoodInfos.size(); ++i)
	{
		FoodIndexByName.emplace(FoodInfos[i].Name, i);
	}

	std::vector<int> OrbFoodIndex(ActiveOrbs.size(), -1);
	auto IsFoodAssignmentLocked = [&](int OrbIndex) -> bool
	{
		if (OrbIndex < 0 ||
			OrbIndex >= static_cast<int>(ActiveOrbs.size()))
		{
			return false;
		}

		auto Orb = ActiveOrbs[OrbIndex];

		if (!Orb)
			return false;

		const ECitizenState State = Orb->GetCitizenState();
		return State == ECitizenState::GoingToFood ||
			State == ECitizenState::AtFood;
	};

	auto AssignOrbToFood = [&](int OrbIndex, int FoodIndex) -> bool
	{
		if (OrbIndex < 0 || FoodIndex < 0)
			return false;

		if (OrbIndex >= static_cast<int>(ActiveOrbs.size()) ||
			FoodIndex >= static_cast<int>(FoodInfos.size()))
		{
			return false;
		}

		auto Orb = ActiveOrbs[OrbIndex];

		if (!Orb)
			return false;

		const int PrevFoodIndex = OrbFoodIndex[OrbIndex];

		if (PrevFoodIndex == FoodIndex)
			return true;

		if (IsFoodAssignmentLocked(OrbIndex))
			return false;

		if (PrevFoodIndex >= 0 &&
			PrevFoodIndex < static_cast<int>(FoodInfos.size()) &&
			FoodInfos[PrevFoodIndex].Assigned > 0)
		{
			--FoodInfos[PrevFoodIndex].Assigned;
		}

		auto& TargetInfo = FoodInfos[FoodIndex];

		if (Orb->GetFoodBuilding() != TargetInfo.Name)
			Orb->SetFoodBuilding(TargetInfo.Name);

		++TargetInfo.Assigned;
		OrbFoodIndex[OrbIndex] = FoodIndex;
		return true;
	};

	auto FindLeastLoadedFood = [&]() -> int
	{
		if (FoodInfos.empty())
			return -1;

		int BestIndex = -1;

		for (size_t i = 0; i < FoodInfos.size(); ++i)
		{
			if (BestIndex < 0 ||
				FoodInfos[i].Assigned < FoodInfos[BestIndex].Assigned ||
				(FoodInfos[i].Assigned == FoodInfos[BestIndex].Assigned &&
					FoodInfos[i].FoodCap > FoodInfos[BestIndex].FoodCap))
			{
				BestIndex = static_cast<int>(i);
			}
		}

		return BestIndex;
	};

	auto FindDonorFood = [&]() -> int
	{
		int DonorIndex = -1;

		for (size_t i = 0; i < FoodInfos.size(); ++i)
		{
			if (FoodInfos[i].Assigned <= 1)
				continue;

			if (DonorIndex < 0 ||
				FoodInfos[i].Assigned > FoodInfos[DonorIndex].Assigned)
			{
				DonorIndex = static_cast<int>(i);
			}
		}

		return DonorIndex;
	};

	auto FindOrbAssignedToFood = [&](int FoodIndex) -> int
	{
		for (size_t i = 0; i < OrbFoodIndex.size(); ++i)
		{
			if (OrbFoodIndex[i] == FoodIndex &&
				!IsFoodAssignmentLocked(static_cast<int>(i)))
			{
				return static_cast<int>(i);
			}
		}

		return -1;
	};

	if (!FoodInfos.empty())
	{
		for (size_t i = 0; i < ActiveOrbs.size(); ++i)
		{
			auto Orb = ActiveOrbs[i];

			if (!Orb)
				continue;

			const std::string& CurrentFood = Orb->GetFoodBuilding();

			if (CurrentFood.empty())
				continue;

			auto FoodIt = FoodIndexByName.find(CurrentFood);

			if (FoodIt == FoodIndexByName.end())
			{
				Orb->SetFoodBuilding("");
				continue;
			}

			const int FoodIndex = static_cast<int>(FoodIt->second);
			OrbFoodIndex[i] = FoodIndex;
			++FoodInfos[FoodIndex].Assigned;
		}

		for (size_t i = 0; i < ActiveOrbs.size(); ++i)
		{
			if (OrbFoodIndex[i] >= 0)
				continue;

			if (IsFoodAssignmentLocked(static_cast<int>(i)))
				continue;

			const int BestFoodIndex = FindLeastLoadedFood();

			if (BestFoodIndex < 0)
				break;

			AssignOrbToFood(static_cast<int>(i), BestFoodIndex);
		}

		const bool CanCoverAllFoodBuildings =
			ActiveOrbs.size() >= FoodInfos.size();

		if (CanCoverAllFoodBuildings)
		{
			for (size_t FoodIdx = 0; FoodIdx < FoodInfos.size(); ++FoodIdx)
			{
				if (FoodInfos[FoodIdx].Assigned > 0)
					continue;

				const int DonorFoodIndex = FindDonorFood();

				if (DonorFoodIndex < 0)
					break;

				const int DonorOrbIndex =
					FindOrbAssignedToFood(DonorFoodIndex);

				if (DonorOrbIndex < 0)
					break;

				AssignOrbToFood(DonorOrbIndex, static_cast<int>(FoodIdx));
			}
		}
	}

	if (WorkInfos.empty())
		return;

	std::vector<int> OrbWorkIndex(ActiveOrbs.size(), -1);
	std::vector<int> OrbWorkCap(ActiveOrbs.size(), -1);
	std::vector<int> UnemployedOrbIndices;
	UnemployedOrbIndices.reserve(ActiveOrbs.size());

	std::unordered_map<std::string, int> FoodDemandByBuilding;
	FoodDemandByBuilding.reserve(FoodNames.size());

	for (size_t i = 0; i < ActiveOrbs.size(); ++i)
	{
		auto Orb = ActiveOrbs[i];

		if (!Orb)
			continue;

		const std::string& FoodName = Orb->GetFoodBuilding();

		if (!FoodName.empty())
			++FoodDemandByBuilding[FoodName];
	}

	for (size_t i = 0; i < WorkInfos.size(); ++i)
	{
		auto& Info = WorkInfos[i];

		if (!Info.IsFoodProvider || Info.Capacity <= 0)
			continue;

		auto FoodDemandIt = FoodDemandByBuilding.find(Info.Name);

		if (FoodDemandIt != FoodDemandByBuilding.end() &&
			FoodDemandIt->second > 0)
		{
			Info.MinRequired = 1;
		}
	}

	auto AssignOrbToWork = [&](int OrbIndex, int WorkIndex) -> bool
	{
		if (OrbIndex < 0 || WorkIndex < 0)
			return false;

		if (OrbIndex >= static_cast<int>(ActiveOrbs.size()) ||
			WorkIndex >= static_cast<int>(WorkInfos.size()))
		{
			return false;
		}

		auto Orb = ActiveOrbs[OrbIndex];

		if (!Orb)
			return false;

		const int PrevWorkIndex = OrbWorkIndex[OrbIndex];

		if (PrevWorkIndex == WorkIndex)
			return true;

		if (PrevWorkIndex >= 0 &&
			PrevWorkIndex < static_cast<int>(WorkInfos.size()))
		{
			auto& PrevInfo = WorkInfos[PrevWorkIndex];

			if (PrevInfo.Occupied <= PrevInfo.MinRequired)
				return false;

			if (PrevInfo.Occupied > 0)
				--PrevInfo.Occupied;
		}

		auto& TargetInfo = WorkInfos[WorkIndex];

		if (TargetInfo.Capacity <= 0 ||
			TargetInfo.Occupied >= TargetInfo.Capacity)
		{
			return false;
		}

		const std::string& TargetName = TargetInfo.Name;

		if (Orb->GetWorkBuilding() != TargetName)
			Orb->SetWorkBuilding(TargetName);

		++TargetInfo.Occupied;
		OrbWorkIndex[OrbIndex] = WorkIndex;
		OrbWorkCap[OrbIndex] = TargetInfo.JobCap;
		return true;
	};

	auto FindBestVacancyWork = [&](int MinJobCapExclusive) -> int
	{
		for (size_t i = 0; i < WorkInfos.size(); ++i)
		{
			const FWorkBuildingInfo& Info = WorkInfos[i];

			if (Info.JobCap <= MinJobCapExclusive)
				break;

			if (Info.Capacity <= 0)
				continue;

			if (Info.Occupied < Info.Capacity)
				return static_cast<int>(i);
		}

		return -1;
	};

	auto FindFoodDeficitWork = [&]() -> int
	{
		for (size_t i = 0; i < WorkInfos.size(); ++i)
		{
			const FWorkBuildingInfo& Info = WorkInfos[i];

			if (!Info.IsFoodProvider || Info.MinRequired <= 0)
				continue;

			if (Info.Capacity <= 0)
				continue;

			if (Info.Occupied < Info.MinRequired &&
				Info.Occupied < Info.Capacity)
			{
				return static_cast<int>(i);
			}
		}

		return -1;
	};

	auto FindDonorWork = [&](int ExcludeWorkIndex) -> int
	{
		for (int i = static_cast<int>(WorkInfos.size()) - 1; i >= 0; --i)
		{
			if (i == ExcludeWorkIndex)
				continue;

			const FWorkBuildingInfo& Info = WorkInfos[i];

			if (Info.Occupied > Info.MinRequired)
				return i;
		}

		return -1;
	};

	auto FindOrbAssignedToWork = [&](int WorkIndex) -> int
	{
		for (size_t i = 0; i < OrbWorkIndex.size(); ++i)
		{
			if (OrbWorkIndex[i] == WorkIndex)
				return static_cast<int>(i);
		}

		return -1;
	};

	for (size_t i = 0; i < ActiveOrbs.size(); ++i)
	{
		auto Orb = ActiveOrbs[i];

		if (!Orb)
			continue;

		const std::string& CurrentWork = Orb->GetWorkBuilding();

		if (CurrentWork.empty())
		{
			UnemployedOrbIndices.push_back(static_cast<int>(i));
			continue;
		}

		auto WorkIt = WorkIndexByName.find(CurrentWork);

		if (WorkIt == WorkIndexByName.end())
		{
			Orb->SetWorkBuilding("");
			UnemployedOrbIndices.push_back(static_cast<int>(i));
			continue;
		}

		const int WorkIndex = static_cast<int>(WorkIt->second);
		OrbWorkIndex[i] = WorkIndex;
		OrbWorkCap[i] = WorkInfos[WorkIndex].JobCap;
		++WorkInfos[WorkIndex].Occupied;
	}

	for (size_t WorkIdx = 0; WorkIdx < WorkInfos.size(); ++WorkIdx)
	{
		FWorkBuildingInfo& Info = WorkInfos[WorkIdx];

		if (Info.Capacity < 0)
			Info.Capacity = 0;

		if (Info.Occupied <= Info.Capacity)
			continue;

		int Overflow = Info.Occupied - Info.Capacity;

		for (size_t OrbIdx = 0;
			OrbIdx < ActiveOrbs.size() && Overflow > 0;
			++OrbIdx)
		{
			if (OrbWorkIndex[OrbIdx] != static_cast<int>(WorkIdx))
				continue;

			auto Orb = ActiveOrbs[OrbIdx];

			if (!Orb)
				continue;

			Orb->SetWorkBuilding("");
			OrbWorkIndex[OrbIdx] = -1;
			OrbWorkCap[OrbIdx] = -1;
			if (Info.Occupied > 0)
				--Info.Occupied;
			--Overflow;
			UnemployedOrbIndices.push_back(static_cast<int>(OrbIdx));
		}
	}

	while (true)
	{
		const int DeficitWorkIndex = FindFoodDeficitWork();

		if (DeficitWorkIndex < 0)
			break;

		bool Assigned = false;

		for (size_t i = 0; i < UnemployedOrbIndices.size(); ++i)
		{
			const int OrbIndex = UnemployedOrbIndices[i];

			if (OrbIndex < 0 ||
				OrbIndex >= static_cast<int>(OrbWorkIndex.size()))
			{
				continue;
			}

			if (OrbWorkIndex[OrbIndex] >= 0)
				continue;

			if (AssignOrbToWork(OrbIndex, DeficitWorkIndex))
			{
				Assigned = true;
				break;
			}
		}

		if (Assigned)
			continue;

		const int DonorWorkIndex = FindDonorWork(DeficitWorkIndex);

		if (DonorWorkIndex < 0)
			break;

		const int DonorOrbIndex = FindOrbAssignedToWork(DonorWorkIndex);

		if (DonorOrbIndex < 0)
			break;

		if (!AssignOrbToWork(DonorOrbIndex, DeficitWorkIndex))
			break;
	}

	for (size_t i = 0; i < UnemployedOrbIndices.size(); ++i)
	{
		const int OrbIndex = UnemployedOrbIndices[i];

		if (OrbIndex < 0 ||
			OrbIndex >= static_cast<int>(OrbWorkIndex.size()))
		{
			continue;
		}

		if (OrbWorkIndex[OrbIndex] >= 0)
			continue;

		const int BestWorkIndex = FindBestVacancyWork(-1);

		if (BestWorkIndex < 0)
			break;

		AssignOrbToWork(OrbIndex, BestWorkIndex);
	}

	for (size_t i = 0; i < ActiveOrbs.size(); ++i)
	{
		if (OrbWorkIndex[i] < 0)
			continue;

		const int CurrentWorkIndex = OrbWorkIndex[i];

		if (CurrentWorkIndex < 0 ||
			CurrentWorkIndex >= static_cast<int>(WorkInfos.size()))
		{
			continue;
		}

		if (WorkInfos[CurrentWorkIndex].Occupied <=
			WorkInfos[CurrentWorkIndex].MinRequired)
		{
			continue;
		}

		const int BetterWorkIndex = FindBestVacancyWork(OrbWorkCap[i]);

		if (BetterWorkIndex < 0)
			continue;

		AssignOrbToWork(static_cast<int>(i), BetterWorkIndex);
	}
}

void CMainWorld::CollectCurrentBuildingNames(
	std::vector<std::string>& OutBuildingNames)
{
	OutBuildingNames.clear();

	std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

	if (!FindObjectListByType<CPlacementAreaObject>(BuildingList))
		return;

	for (size_t i = 0; i < BuildingList.size(); ++i)
	{
		auto Building = BuildingList[i].lock();

		if (!Building || !Building->GetAlive() ||
			!Building->HasPlacedArea())
			continue;

		const std::string& Name = Building->GetName();

		if (Name.empty())
			continue;

		if (std::find(OutBuildingNames.begin(),
			OutBuildingNames.end(), Name) == OutBuildingNames.end())
		{
			OutBuildingNames.push_back(Name);
		}
	}
}

void CMainWorld::CollectHomeBuildingNames(std::vector<std::string>& Out)
{
	Out.clear();
	std::vector<std::weak_ptr<CPlacementAreaObject>> List;
	if (!FindObjectListByType<CPlacementAreaObject>(List))
		return;

	for (auto& W : List)
	{
		auto B = W.lock();

		if (!B || !B->GetAlive() || !B->HasPlacedArea())
			continue;

		if (B->IsResidential())
			Out.push_back(B->GetName());
	}
}

void CMainWorld::CollectWorkBuildingNames(std::vector<std::string>& Out)
{
	Out.clear();
	std::vector<std::weak_ptr<CPlacementAreaObject>> List;
	if (!FindObjectListByType<CPlacementAreaObject>(List))
		return;

	for (auto& W : List)
	{
		auto B = W.lock();

		if (!B || !B->GetAlive() || !B->HasPlacedArea())
			continue;

		// FoodProvider 건물은 직장 겸 음식 생산지로 포함
		// EntertainmentProvider 전용 건물(주점 등)은 제외
		if (!B->IsResidential() &&
			(!B->IsEntertainmentProvider() || B->IsFoodProvider()))
		{
			Out.push_back(B->GetName());
		}
	}
}

void CMainWorld::CollectFoodBuildingNames(std::vector<std::string>& Out)
{
	Out.clear();
	std::vector<std::weak_ptr<CPlacementAreaObject>> List;
	if (!FindObjectListByType<CPlacementAreaObject>(List))
		return;

	for (auto& W : List)
	{
		auto B = W.lock();

		if (!B || !B->GetAlive() || !B->HasPlacedArea())
			continue;

		if (B->IsFoodProvider())
			Out.push_back(B->GetName());
	}
}

void CMainWorld::CollectEntertainmentBuildingNames(
	std::vector<std::string>& Out)
{
	Out.clear();
	std::vector<std::weak_ptr<CPlacementAreaObject>> List;
	if (!FindObjectListByType<CPlacementAreaObject>(List))
		return;

	for (auto& W : List)
	{
		auto B = W.lock();

		if (!B || !B->GetAlive() || !B->HasPlacedArea())
			continue;

		if (B->IsEntertainmentProvider())
			Out.push_back(B->GetName());
	}
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
	mUIManager->CreateWidget<CBuildMenuWidget>(
		"BuildMenuWidget", 300);
}
