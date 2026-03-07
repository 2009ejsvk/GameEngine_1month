#include "MainWorld.h"
#include "../ObjectNames.h"
#include "Asset/AssetManager.h"
#include "Asset/Animation2D/Animation2DManager.h"
#include "Component/ColliderBox2D.h"
#include "../UI/TopHudWidget.h"
#include "../UI/BuildMenuWidget.h"
#include "../UI/EdictWidget.h"
#include "../Building/BuildingCatalog.h"
#include "World/WorldUIManager.h"
#include "Render/RenderManager.h"
#include "../Map/TileMapMain.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementBuildingVisual.h"
#include "../Player/MainCamera.h"
#include "../Map/PlacementController.h"
#include "../Politics/EdictSystem.h"
#include "../Politics/PoliticsSystem.h"
#include "../Citizen/CitizenSystem.h"
#include "../Economy/EconomySystem.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <random>
#include <string>
#include <vector>

namespace
{
	std::string WideToUtf8(const std::wstring& Text)
	{
		if (Text.empty())
			return std::string();

		const int RequiredBytes = WideCharToMultiByte(
			CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
			nullptr, 0, nullptr, nullptr);

		if (RequiredBytes <= 0)
			return std::string(Text.begin(), Text.end());

		std::string Utf8;
		Utf8.resize(RequiredBytes);
		WideCharToMultiByte(
			CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
			&Utf8[0], RequiredBytes, nullptr, nullptr);
		return Utf8;
	}

	constexpr int GInitialNpcCount = 10;
	constexpr int GMaxNpcCount = 2000;
	constexpr float GNpcSpawnInterval = 5.f;
	constexpr float GCitizenReassignInterval = 0.5f;
	constexpr long long GInitialNationalBudget = 500000;
	constexpr int GSimulationStartYear = 2000;
	constexpr int GSimulationStartMonth = 1;
	constexpr int GSimulationStartDay = 1;
	constexpr float GSecondsPerSimulationDay = 2.f;
	constexpr float GPoliticalSnapshotInterval = 1.f;
	constexpr int GCitizenDirectionCount = 8;
	constexpr float GCitizenFrameWidth = 16.f;
	constexpr float GCitizenFrameHeight = 19.f;
	constexpr float GCitizenRightBottomStartX = 15.f;
	constexpr float GCitizenDirectionStepX = 16.f;
	constexpr float GCitizenBlueWalkTopY = 125.f;
	constexpr float GCitizenBlueIdleTopY = 148.f;
	constexpr float GCitizenBlueWalkReverseTopY = 173.f;
	constexpr float GCitizenRedWalkTopY = 221.f;
	constexpr float GCitizenRedIdleTopY = 244.f;
	constexpr float GCitizenRedWalkReverseTopY = 269.f;
	constexpr const char* GCitizenBlueAnimationPrefix = "CitizenBlue";
	constexpr const char* GCitizenRedAnimationPrefix = "CitizenRed";
	constexpr const char* GCitizenSheetTextureName =
		"CitizenSmall8DirectionSheet";
	constexpr const TCHAR* GCitizenSheetTextureFile =
		TEXT("Small-8-Direction-Characters_by_AxulArt.png");
	constexpr const char* GStarterDormitoryObjectName = "StarterDormitory";
	constexpr const char* GStarterTeamsterObjectName = "StarterTeamsterOffice";
	constexpr const char* GStarterRanchObjectName = "StarterRanch";
	constexpr const char* GStarterFarmObjectName = "StarterLargeFarm";
	constexpr const char* GStarterHarborObjectName = "StarterHarbor";

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

	LoadCitizenAnimation2D();

	// LoadAnimation2D();

	// LoadSound();

	CreateUI();

	// MainWorld 화면 출력 요소는 우선 모두 주석 처리하고
	// 이동 가능한 카메라만 별도로 배치한다.
	auto MainCamera = CreateGameObject<CMainCamera>("MainCamera");
	CreateGameObject<CPlacementController>(GPlacementControllerName);

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

	auto TileMap = CreateGameObject<CTileMapMain>(GTileMapObjectName);
	auto TileMapObj = std::dynamic_pointer_cast<CTileMapObject>(
		TileMap.lock());
	auto FloorBlueTileMap =
		CreateGameObject<CTileMapObject>(GTileMapFloorBlueName);
	auto FloorBlueTileMapObj = FloorBlueTileMap.lock();
	auto FloorYellowTileMap =
		CreateGameObject<CTileMapObject>(GTileMapFloorYellowName);
	auto FloorYellowTileMapObj = FloorYellowTileMap.lock();
	auto ExpansionTileMap =
		CreateGameObject<CTileMapObject>(GTileMapExpansionName);
	auto ExpansionTileMapObj = ExpansionTileMap.lock();
	auto MainCameraObj = MainCamera.lock();

	if (TileMapObj &&
		FloorBlueTileMapObj &&
		FloorYellowTileMapObj &&
		ExpansionTileMapObj)
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

		auto ConfigureExpansionTileMap =
			[&](const std::shared_ptr<CTileMapObject>& OverlayObj)
		{
			if (!BaseTileMap || !OverlayObj)
				return;

			auto OverlayRender =
				OverlayObj->FindComponent<CTileMapRender>().lock();
			auto OverlayMap = OverlayObj->GetTileMap().lock();

			if (!OverlayRender || !OverlayMap)
				return;

			const int OverlayLayerOrder = ResolveLayerOrder(
				"MapExpansion", 1, ERenderListSort::None);

			if (OverlayLayerOrder >= 0)
				OverlayRender->SetRenderLayer("MapExpansion");

			OverlayRender->EnableTileAlphaBlend();
			OverlayRender->SetTexture(
				ETileTextureType::Tile,
				"LandscapeTile066Expansion",
				TEXT("landscapeTiles_066.png"));
			OverlayRender->AddTileFrame(
				0.f,
				0.f,
				CTileMapMain::TileTextureWidth,
				CTileMapMain::TileTextureHeight);

			OverlayMap->CreateTile(
				BaseTileMap->GetTileShape(),
				BaseTileMap->GetTileCountX(),
				BaseTileMap->GetTileCountY(),
				BaseTileMap->GetTileSize());
			OverlayMap->SetViewCulling(true);
			OverlayMap->SetTileTextureSize(
				CTileMapMain::TileTextureWidth,
				CTileMapMain::TileTextureHeight);
			OverlayMap->SetTileFrameAll(0);
			OverlayMap->SetTileOutLineRender(false);

			const int CountX = OverlayMap->GetTileCountX();
			const int CountY = OverlayMap->GetTileCountY();
			const int BorderX = CTileMapMain::SeaBorderX;
			const int BorderY = CTileMapMain::SeaBorderY;

			for (int y = 0; y < CountY; ++y)
			{
				for (int x = 0; x < CountX; ++x)
				{
					const int Index = y * CountX + x;
					auto Tile = OverlayMap->GetTile(Index).lock();

					if (!Tile)
						continue;

					const bool IsExpandedTile =
						x < BorderX ||
						x >= CountX - BorderX ||
						y < BorderY ||
						y >= CountY - BorderY;

					Tile->SetTileType(ETileType::Normal);
					Tile->SetOutLineColor(
						1.f, 1.f, 1.f, IsExpandedTile ? 1.f : 0.f);
				}
			}

			OverlayObj->SetWorldPos(TileMapObj->GetWorldPos());
		};

		if (BaseTileMap)
		{
			ResolveLayerOrder("MapExpansion",   1, ERenderListSort::None);
			ResolveLayerOrder("MapFloorBlue",    2, ERenderListSort::None);
			ResolveLayerOrder("BuildingVisual",  3, ERenderListSort::Y);

			ConfigureExpansionTileMap(ExpansionTileMapObj);
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
		int OffsetX,
		int OffsetY)
	{
		auto Building = CreateGameObject<CPlacementAreaObject>(ObjectName);
		auto BuildingObj = Building.lock();

		if (!BuildingObj)
			return;

		const FBuildingCatalogEntry* CatalogEntry =
			FindBuildingCatalogEntry(BuildingId);

		BuildingObj->SetTileMapObject(TileMapObj);
		BuildingObj->SetInitialCenterOffset(OffsetX, OffsetY);
		BuildingObj->SetBuildingId(BuildingId);

		if (CatalogEntry)
		{
			const std::string IconPath = GetCatalogEntryIconPathUtf8(
				CatalogEntry->Category, CatalogEntry->CategoryLocalIndex);

			if (!IconPath.empty())
				BuildingObj->SetBuildingSpriteTexturePath(IconPath);

			BuildingObj->SetBuildingKind(CatalogEntry->BuildingKind);
			BuildingObj->SetBuildingDisplayInfo(
				WideToUtf8(CatalogEntry->DisplayName),
				WideToUtf8(CatalogEntry->CategoryName),
				CatalogEntry->Residential,
				CatalogEntry->Capacity,
				CatalogEntry->FoodProvider,
				CatalogEntry->EntertainmentProvider,
				CatalogEntry->HousingSatisfactionCap,
				CatalogEntry->JobSatisfactionCap,
				CatalogEntry->FoodSatisfactionCap,
				CatalogEntry->FunSatisfactionCap);
			BuildingObj->SetPlacementTemplateType(CatalogEntry->TemplateType);
		}
		else
		{
			BuildingObj->SetBuildingDisplayInfo(
				BuildingId, "기본", false, 0);
		}

		auto Visual = CreateGameObject<CBuildingVisual>(
			ObjectName + "_Visual");
		auto VisualObj = Visual.lock();

		if (VisualObj)
			VisualObj->SetBuilding(Building);
	};

	CreateStarterBuilding(
		GStarterTeamsterObjectName,
		"build_1_5",
		-14,
		8);

	CreateStarterBuilding(
		GStarterRanchObjectName,
		"build_2_6",
		12,
		9);

	CreateStarterBuilding(
		GStarterFarmObjectName,
		"build_2_4",
		-10,
		-10);

	CreateStarterBuilding(
		GStarterDormitoryObjectName,
		"build_4_3",
		0,
		0);

	CreateStarterBuilding(
		GStarterHarborObjectName,
		"build_1_3",
		15,
		-12);

	mSpawnedNpcCount = 0;
	mNpcSpawnAccum = 0.f;
	mCitizenReassignAccum = 0.f;
	mNationalBudget = GInitialNationalBudget;
	mLastDailyWageCost = 0;
	mLastDailyUpkeepCost = 0;
	mLastDailyExportIncome = 0;
	mLastDailyEdictCost = 0;
	mLastDailyNetChange = 0;
	mSimulationYear = GSimulationStartYear;
	mSimulationMonth = GSimulationStartMonth;
	mSimulationDay = GSimulationStartDay;
	mDayProgressAccum = 0.f;
	mSecondsPerSimulationDay = GSecondsPerSimulationDay;
	mPoliticalSnapshotAccum = 0.f;
	PoliticsSystem::SetDefaultGovernmentProfile(mGovernmentProfile);
	mPoliticalSnapshot = FPoliticalWorldSnapshot();
	EdictSystem::InitializeGovernmentEdictStates(mGovernmentEdicts);
	mEdictModifiers = FGovernmentEdictModifiers();

	for (int i = 0; i < GInitialNpcCount; ++i)
	{
		SpawnCitizenOrb();
	}

	ReassignCitizenNeeds();
	RefreshPoliticalSnapshot();
	RefreshEdictModifiers();

	return true;
}

void CMainWorld::Update(float DeltaTime)
{
	CWorld::Update(DeltaTime);
	AdvanceSimulationDate(DeltaTime);

	mPoliticalSnapshotAccum += DeltaTime;

	if (mPoliticalSnapshotAccum >= GPoliticalSnapshotInterval)
	{
		mPoliticalSnapshotAccum = 0.f;
		RefreshPoliticalSnapshot();
	}

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

int CMainWorld::GetSimulationMonthDayCount() const
{
	return GetDaysInMonth(mSimulationYear, mSimulationMonth);
}

float CMainWorld::GetSimulationDayProgress() const
{
	if (mSecondsPerSimulationDay <= 0.f)
		return 0.f;

	return Clamp<float>(
		mDayProgressAccum / mSecondsPerSimulationDay, 0.f, 1.f);
}

float CMainWorld::GetSimulationMonthProgress() const
{
	const int MonthDays = GetDaysInMonth(mSimulationYear, mSimulationMonth);

	if (MonthDays <= 0)
		return 0.f;

	const float DayProgress = GetSimulationDayProgress();
	const float CompletedDays =
		static_cast<float>((std::max)(0, mSimulationDay - 1)) +
		DayProgress;

	return Clamp<float>(
		CompletedDays / static_cast<float>(MonthDays), 0.f, 1.f);
}

void CMainWorld::AdvanceSimulationDate(float DeltaTime)
{
	if (DeltaTime <= 0.f || mSecondsPerSimulationDay <= 0.f)
		return;

	mDayProgressAccum += DeltaTime;

	while (mDayProgressAccum >= mSecondsPerSimulationDay)
	{
		mDayProgressAccum -= mSecondsPerSimulationDay;
		AdvanceSimulationDay();
	}
}

void CMainWorld::AdvanceSimulationDay()
{
	ApplyDailyEconomySettlement();
	ApplyDailyEdictCitizenEffects();
	TickGovernmentEdicts();
	PoliticsSystem::TickGovernmentActions(mGovernmentProfile);
	RefreshEdictModifiers();
	RefreshPoliticalSnapshot();

	++mSimulationDay;
	const int CurrentMonthDays =
		GetDaysInMonth(mSimulationYear, mSimulationMonth);

	if (mSimulationDay <= CurrentMonthDays)
		return;

	mSimulationDay = 1;
	++mSimulationMonth;

	if (mSimulationMonth <= 12)
		return;

	mSimulationMonth = 1;
	++mSimulationYear;
}

int CMainWorld::GetDaysInMonth(int Year, int Month) const
{
	switch (Month)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		return 31;
	case 4:
	case 6:
	case 9:
	case 11:
		return 30;
	case 2:
	{
		const bool IsLeapYear =
			(Year % 400 == 0) || (Year % 4 == 0 && Year % 100 != 0);
		return IsLeapYear ? 29 : 28;
	}
	default:
		return 30;
	}
}

void CMainWorld::ApplyDailyEconomySettlement()
{
	const int DaysInMonth = GetDaysInMonth(mSimulationYear, mSimulationMonth);
	const auto Result = EconomySystem::ApplyDailySettlement(this, DaysInMonth);
	const long long DailyEdictUpkeep =
		EdictSystem::CalculateEdictDailyUpkeep(
			mGovernmentEdicts,
			DaysInMonth);
	const long long DailyEdictBudgetDelta =
		mEdictModifiers.DailyBudgetDelta;
	mLastDailyWageCost     = Result.WageCost;
	mLastDailyUpkeepCost   = Result.UpkeepCost;
	mLastDailyExportIncome = Result.ExportIncome;
	mLastDailyEdictCost    = DailyEdictUpkeep - DailyEdictBudgetDelta;
	mLastDailyNetChange    =
		Result.NetChange - DailyEdictUpkeep + DailyEdictBudgetDelta;
	mNationalBudget       += mLastDailyNetChange;
}

bool CMainWorld::TryApplyEdict(
	EGovernmentEdictType Type,
	std::wstring& OutMessage)
{
	const FGovernmentEdictDefinition* Definition =
		EdictSystem::FindGovernmentEdictDefinition(Type);

	if (!Definition)
	{
		OutMessage = L"정의되지 않은 칙령입니다.";
		return false;
	}

	FGovernmentEdictState* TargetState = nullptr;

	for (size_t i = 0; i < mGovernmentEdicts.size(); ++i)
	{
		if (mGovernmentEdicts[i].Type == Type)
		{
			TargetState = &mGovernmentEdicts[i];
			break;
		}
	}

	if (!TargetState)
	{
		OutMessage = L"칙령 상태를 찾을 수 없습니다.";
		return false;
	}

	const int ActiveCitizenCount =
		(std::max)(0, mPoliticalSnapshot.ActiveCitizenCount);

	if (Definition->Mode == EGovernmentEdictMode::Passive &&
		TargetState->Active)
	{
		TargetState->Active = false;
		TargetState->RemainingDays = 0;
		SyncGovernmentActionFromEdict(Type, false);
		RefreshEdictModifiers();
		RefreshPoliticalSnapshot();
		OutMessage = Definition->DisplayName + L" 해제";
		return true;
	}

	if (TargetState->Active)
	{
		OutMessage = Definition->DisplayName + L" 시행 중";
		return false;
	}

	if (TargetState->CooldownDays > 0)
	{
		OutMessage = Definition->DisplayName + L" 재사용 대기 중";
		return false;
	}

	const long long ActivationCost =
		EdictSystem::ResolveEdictActivationCost(
			*Definition,
			ActiveCitizenCount);

	if (ActivationCost > mNationalBudget)
	{
		OutMessage = L"예산이 부족합니다.";
		return false;
	}

	mNationalBudget -= ActivationCost;
	TargetState->Active = true;

	if (Definition->Mode == EGovernmentEdictMode::Active)
	{
		TargetState->RemainingDays = (std::max)(1, Definition->DurationDays);
		TargetState->CooldownDays = (std::max)(1, Definition->CooldownDays);
	}
	else
	{
		TargetState->RemainingDays = -1;
		TargetState->CooldownDays = 0;
	}

	SyncGovernmentActionFromEdict(Type, true);
	RefreshEdictModifiers();
	RefreshPoliticalSnapshot();
	OutMessage = Definition->DisplayName + L" 시행";
	return true;
}

const FGovernmentEdictState* CMainWorld::GetGovernmentEdictState(
	EGovernmentEdictType Type) const
{
	for (size_t i = 0; i < mGovernmentEdicts.size(); ++i)
	{
		if (mGovernmentEdicts[i].Type == Type)
			return &mGovernmentEdicts[i];
	}

	return nullptr;
}

void CMainWorld::TickGovernmentEdicts()
{
	bool ModifiersChanged = false;

	for (size_t i = 0; i < mGovernmentEdicts.size(); ++i)
	{
		FGovernmentEdictState& State = mGovernmentEdicts[i];
		const FGovernmentEdictDefinition* Definition =
			EdictSystem::FindGovernmentEdictDefinition(State.Type);

		if (!Definition)
			continue;

		if (State.Active &&
			Definition->Mode == EGovernmentEdictMode::Active &&
			State.RemainingDays > 0)
		{
			--State.RemainingDays;

			if (State.RemainingDays <= 0)
			{
				State.Active = false;
				State.RemainingDays = 0;
				SyncGovernmentActionFromEdict(State.Type, false);
				ModifiersChanged = true;
			}
		}

		if (!State.Active && State.CooldownDays > 0)
			--State.CooldownDays;
	}

	if (ModifiersChanged)
		RefreshEdictModifiers();
}

void CMainWorld::RefreshEdictModifiers()
{
	mEdictModifiers = EdictSystem::CalculateEdictModifiers(
		mGovernmentEdicts,
		mPoliticalSnapshot.ActiveCitizenCount);
}

void CMainWorld::ApplyDailyEdictCitizenEffects()
{
	const FGovernmentEdictModifiers& Modifiers = mEdictModifiers;

	if (Modifiers.DailyFoodDelta == 0.f &&
		Modifiers.DailyHousingDelta == 0.f &&
		Modifiers.DailyJobDelta == 0.f &&
		Modifiers.DailyFreedomDelta == 0.f &&
		Modifiers.DailySecurityDelta == 0.f)
	{
		return;
	}

	std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

	if (!FindObjectListByType<CBuildingMarkerOrb>(OrbList))
		return;

	for (size_t i = 0; i < OrbList.size(); ++i)
	{
		auto Orb = OrbList[i].lock();

		if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
			continue;

		Orb->ApplySatisfactionDelta(
			Modifiers.DailyFoodDelta,
			0.f,
			0.f,
			0.f,
			Modifiers.DailyHousingDelta,
			Modifiers.DailyJobDelta,
			Modifiers.DailyFreedomDelta,
			Modifiers.DailySecurityDelta);
	}
}

void CMainWorld::SyncGovernmentActionFromEdict(
	EGovernmentEdictType Type,
	bool Active)
{
	const FGovernmentEdictDefinition* Definition =
		EdictSystem::FindGovernmentEdictDefinition(Type);

	if (!Definition || Definition->ActionType == EPoliticalActionType::None)
		return;

	mGovernmentProfile.ActiveActions.erase(
		std::remove_if(
			mGovernmentProfile.ActiveActions.begin(),
			mGovernmentProfile.ActiveActions.end(),
			[&](const FGovernmentActionRecord& Action)
			{
				return Action.Type == Definition->ActionType;
			}),
		mGovernmentProfile.ActiveActions.end());

	if (!Active)
		return;

	mGovernmentProfile.ActiveActions.push_back(
		EdictSystem::MakeGovernmentActionFromEdict(*Definition));
}

void CMainWorld::RefreshPoliticalSnapshot()
{
	mPoliticalSnapshot = PoliticsSystem::EvaluateWorld(
		this,
		mGovernmentProfile);
}

void CMainWorld::SpawnCitizenOrb()
{
	CitizenSystem::SpawnCitizenOrb(this, mSpawnedNpcCount);
}

void CMainWorld::ReassignCitizenNeeds()
{
	CitizenSystem::ReassignCitizenNeeds(this);
}

void CMainWorld::LoadCitizenAnimation2D()
{
	auto CreateCitizenVariantAnimations = [&](const char* Prefix,
		float WalkTopY,
		float IdleTopY,
		float WalkReverseTopY)
	{
		for (int Direction = 0;
			Direction < GCitizenDirectionCount;
			++Direction)
		{
			const std::string IdleAnimationName =
				std::string(Prefix) + "Idle_Dir" +
				std::to_string(Direction);
			const std::string WalkAnimationName =
				std::string(Prefix) + "Walk_Dir" +
				std::to_string(Direction);
			const float RightBottomX = GCitizenRightBottomStartX +
				static_cast<float>(Direction) * GCitizenDirectionStepX;
			const float StartX = RightBottomX - GCitizenFrameWidth;

			mWorldAssetManager->CreateAnimation(IdleAnimationName);
			mWorldAssetManager->SetAnimation2DTextureType(
				IdleAnimationName,
				EAnimation2DTextureType::SpriteSheet);
			mWorldAssetManager->SetTexture(
				IdleAnimationName,
				GCitizenSheetTextureName,
				GCitizenSheetTextureFile);
			mWorldAssetManager->AddFrame(
				IdleAnimationName,
				StartX,
				IdleTopY,
				GCitizenFrameWidth,
				GCitizenFrameHeight);

			mWorldAssetManager->CreateAnimation(WalkAnimationName);
			mWorldAssetManager->SetAnimation2DTextureType(
				WalkAnimationName,
				EAnimation2DTextureType::SpriteSheet);
			mWorldAssetManager->SetTexture(
				WalkAnimationName,
				GCitizenSheetTextureName,
				GCitizenSheetTextureFile);
			mWorldAssetManager->AddFrame(
				WalkAnimationName,
				StartX,
				WalkTopY,
				GCitizenFrameWidth,
				GCitizenFrameHeight);
			mWorldAssetManager->AddFrame(
				WalkAnimationName,
				StartX,
				WalkReverseTopY,
				GCitizenFrameWidth,
				GCitizenFrameHeight);
		}
	};

	CreateCitizenVariantAnimations(
		GCitizenBlueAnimationPrefix,
		GCitizenBlueWalkTopY,
		GCitizenBlueIdleTopY,
		GCitizenBlueWalkReverseTopY);
	CreateCitizenVariantAnimations(
		GCitizenRedAnimationPrefix,
		GCitizenRedWalkTopY,
		GCitizenRedIdleTopY,
		GCitizenRedWalkReverseTopY);
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
	mUIManager->CreateWidget<CTopHudWidget>(
		GTopHudWidgetName, 250);
	mUIManager->CreateWidget<CEdictWidget>(
		GEdictWidgetName, 280);
	mUIManager->CreateWidget<CBuildMenuWidget>(
		GBuildMenuWidgetName, 300);
}
