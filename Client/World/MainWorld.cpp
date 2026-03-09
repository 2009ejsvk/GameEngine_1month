#include "MainWorld.h"
#include "../ObjectNames.h"
#include "Asset/AssetManager.h"
#include "Asset/Animation2D/Animation2DManager.h"
#include "Component/ColliderBox2D.h"
#include "../UI/TopHudWidget.h"
#include "../UI/BuildMenuWidget.h"
#include "../UI/AlmanacWidget.h"
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
		{
			std::string Fallback;
			Fallback.reserve(Text.size());

			for (size_t Index = 0; Index < Text.size(); ++Index)
			{
				const wchar_t Character = Text[Index];
				Fallback.push_back(
					Character >= 0 && Character <= 0x7f ?
					static_cast<char>(Character) :
					'?');
			}

			return Fallback;
		}

		std::string Utf8;
		Utf8.resize(RequiredBytes);
		WideCharToMultiByte(
			CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
			&Utf8[0], RequiredBytes, nullptr, nullptr);
		return Utf8;
	}

	int* ResolveTaxRatePercent(
		FTaxPolicy& TaxPolicy,
		ETaxPolicyType Type)
	{
		switch (Type)
		{
		case ETaxPolicyType::Consumption:
			return &TaxPolicy.ConsumptionRatePercent;
		case ETaxPolicyType::Income:
			return &TaxPolicy.IncomeRatePercent;
		case ETaxPolicyType::Property:
			return &TaxPolicy.PropertyRatePercent;
		default:
			return nullptr;
		}
	}

	ETaxPolicyEventType GetRequiredTaxPolicyEventForEdict(
		EGovernmentEdictType Type)
	{
		switch (Type)
		{
		case EGovernmentEdictType::LaborTaxRelief:
			return ETaxPolicyEventType::WorkerTaxStrike;
		case EGovernmentEdictType::PropertyTaxRelief:
			return ETaxPolicyEventType::PropertyTaxBacklash;
		case EGovernmentEdictType::EmergencyAusterity:
			return ETaxPolicyEventType::BudgetCrisis;
		default:
			return ETaxPolicyEventType::None;
		}
	}

	int ApplyTaxPolicyRateDelta(
		FTaxPolicy& TaxPolicy,
		ETaxPolicyType Type,
		int DeltaPercent)
	{
		int* TargetRatePercent = ResolveTaxRatePercent(TaxPolicy, Type);

		if (!TargetRatePercent)
			return 0;

		const int PreviousRatePercent = *TargetRatePercent;
		const int NewRatePercent = (std::max)(
			GetTaxPolicyMinPercent(Type),
			(std::min)(
				GetTaxPolicyMaxPercent(Type),
				PreviousRatePercent + DeltaPercent));
		*TargetRatePercent = NewRatePercent;
		return NewRatePercent - PreviousRatePercent;
	}

	const wchar_t* GetTaxPolicyEventTitle(ETaxPolicyEventType Type)
	{
		switch (Type)
		{
		case ETaxPolicyEventType::WorkerTaxStrike:
			return L"자본주의자·지식인 조세 시위";
		case ETaxPolicyEventType::PropertyTaxBacklash:
			return L"보수주의자·자본주의자 재산권 반발";
		case ETaxPolicyEventType::BudgetCrisis:
			return L"보수주의자·공산주의자 재정 압박";
		default:
			return L"정치 경고";
		}
	}

	std::wstring BuildTaxPolicyEventWarningSummary(
		ETaxPolicyEventType Type,
		int DaysActive)
	{
		const bool Escalated = DaysActive >= 4;

		switch (Type)
		{
		case ETaxPolicyEventType::WorkerTaxStrike:
			return Escalated ?
				L"자본주의자와 지식인이 근로세 경감을 최후통첩합니다." :
				L"자본주의자와 지식인이 근로세 경감을 요구합니다.";
		case ETaxPolicyEventType::PropertyTaxBacklash:
			return Escalated ?
				L"보수주의자와 자본주의자가 재산세 유예를 강하게 압박합니다." :
				L"보수주의자와 자본주의자가 재산세 유예를 요구합니다.";
		case ETaxPolicyEventType::BudgetCrisis:
			return Escalated ?
				L"보수주의자와 공산주의자가 재정 안정 대책을 최후통첩합니다." :
				L"보수주의자와 공산주의자가 재정 안정 대책을 요구합니다.";
		default:
			return L"정치 경고가 감지되었습니다.";
		}
	}

	std::wstring BuildTaxPolicyEventResolvedSummary(
		ETaxPolicyEventType Type,
		bool Success)
	{
		switch (Type)
		{
		case ETaxPolicyEventType::WorkerTaxStrike:
			return Success ?
				L"자본주의자와 지식인이 한발 물러섰습니다." :
				L"자본주의자와 지식인의 조세 시위가 소강 상태로 돌아섰습니다.";
		case ETaxPolicyEventType::PropertyTaxBacklash:
			return Success ?
				L"보수주의자와 자본주의자의 재산권 압박이 진정되었습니다." :
				L"보수주의자와 자본주의자의 반발이 소강 상태에 들어갔습니다.";
		case ETaxPolicyEventType::BudgetCrisis:
			return Success ?
				L"보수주의자와 공산주의자의 재정 압박이 진정되었습니다." :
				L"재정 압박 연대가 일단락되었지만 경계는 남아 있습니다.";
		default:
			return Success ?
				L"정치 경고가 진정되었습니다." :
				L"정치 경고가 일단락되었습니다.";
		}
	}

	int GetTaxPolicyEventDurationDays(ETaxPolicyEventType Type)
	{
		switch (Type)
		{
		case ETaxPolicyEventType::WorkerTaxStrike:
			return 8;
		case ETaxPolicyEventType::PropertyTaxBacklash:
			return 7;
		case ETaxPolicyEventType::BudgetCrisis:
			return 9;
		default:
			return 0;
		}
	}

	int GetTaxPolicyEventCooldownDays(bool Success)
	{
		return Success ? 18 : 24;
	}

	constexpr int GInitialNpcCount = 10;
	constexpr int GMaxNpcCount = 2000;
	constexpr float GNpcSpawnInterval = 5.f;
	constexpr float GCitizenReassignInterval = 0.5f;
	constexpr long long GInitialNationalBudget = 500000;
	constexpr int GInitialElectionLeadYears = 2;
	constexpr int GElectionIntervalYears = 4;
	constexpr int GElectionMonth = 1;
	constexpr int GElectionDay = 1;
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
	mLastDailyTaxIncome = 0;
	mLastDailyConsumptionTaxIncome = 0;
	mLastDailyIncomeTaxIncome = 0;
	mLastDailyPropertyTaxIncome = 0;
	mLastDailyEdictCost = 0;
	mLastDailyNetChange = 0;
	mLastDailyTaxCollectionEfficiency = 0.0;
	mSimulationYear = GSimulationStartYear;
	mSimulationMonth = GSimulationStartMonth;
	mSimulationDay = GSimulationStartDay;
	mDayProgressAccum = 0.f;
	mSecondsPerSimulationDay = GSecondsPerSimulationDay;
	mPoliticalSnapshotAccum = 0.f;
	mWorkerTaxPressureDays = 0;
	mPropertyTaxPressureDays = 0;
	mBudgetCrisisPressureDays = 0;
	PoliticsSystem::SetDefaultGovernmentProfile(mGovernmentProfile);
	mPoliticalSnapshot = FPoliticalWorldSnapshot();
	mElectionStatus = FElectionStatus();
	mTaxEventStatus = FTaxPolicyEventStatus();
	EdictSystem::InitializeGovernmentEdictStates(mGovernmentEdicts);
	mEdictModifiers = FGovernmentEdictModifiers();
	InitializeElectionSchedule();

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

	if (mElectionStatus.GameLost)
		return;

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
	ApplyDailyTaxPolicyEventEffects();
	TickGovernmentEdicts();
	PoliticsSystem::TickGovernmentActions(mGovernmentProfile);
	RefreshEdictModifiers();
	RefreshPoliticalSnapshot();
	TickTaxPolicyEvents();

	++mSimulationDay;
	const int CurrentMonthDays =
		GetDaysInMonth(mSimulationYear, mSimulationMonth);

	if (mSimulationDay > CurrentMonthDays)
	{
		mSimulationDay = 1;
		++mSimulationMonth;

		if (mSimulationMonth > 12)
		{
			mSimulationMonth = 1;
			++mSimulationYear;
		}
	}

	ResolveScheduledElection();
}

void CMainWorld::InitializeElectionSchedule()
{
	mElectionStatus = FElectionStatus();
	ScheduleNextElection(GInitialElectionLeadYears);
}

void CMainWorld::ScheduleNextElection(int YearsUntilElection)
{
	const int SafeYearsUntilElection = (std::max)(1, YearsUntilElection);
	mElectionStatus.NextElectionYear = mSimulationYear + SafeYearsUntilElection;
	mElectionStatus.NextElectionMonth = GElectionMonth;
	mElectionStatus.NextElectionDay = GElectionDay;
}

void CMainWorld::ResolveScheduledElection()
{
	if (mElectionStatus.GameLost)
		return;

	if (mSimulationYear != mElectionStatus.NextElectionYear ||
		mSimulationMonth != mElectionStatus.NextElectionMonth ||
		mSimulationDay != mElectionStatus.NextElectionDay)
	{
		return;
	}

	RefreshPoliticalSnapshot();

	const int ActiveCitizenCount =
		(std::max)(0, mPoliticalSnapshot.ActiveCitizenCount);
	const int IncumbentVotes =
		(std::max)(0, mPoliticalSnapshot.IncumbentCount);
	const int OppositionVotes =
		(std::max)(0, mPoliticalSnapshot.OppositionCount);
	const int AbstainVotes =
		(std::max)(0, mPoliticalSnapshot.AbstainCount);
	const int CastVotes = IncumbentVotes + OppositionVotes;
	const bool IncumbentWon =
		CastVotes > 0 && IncumbentVotes >= OppositionVotes;

	mElectionStatus.HasRecordedElection = true;
	mElectionStatus.IncumbentWonLastElection = IncumbentWon;
	mElectionStatus.LastElectionYear = mSimulationYear;
	mElectionStatus.LastElectionMonth = mSimulationMonth;
	mElectionStatus.LastElectionDay = mSimulationDay;
	mElectionStatus.LastIncumbentVotes = IncumbentVotes;
	mElectionStatus.LastOppositionVotes = OppositionVotes;
	mElectionStatus.LastAbstainVotes = AbstainVotes;
	mElectionStatus.LastVoteShare =
		CastVotes > 0 ?
		static_cast<double>(IncumbentVotes) /
			static_cast<double>(CastVotes) * 100.0 :
		0.0;
	mElectionStatus.LastTurnoutPercent =
		ActiveCitizenCount > 0 ?
		static_cast<double>(CastVotes) /
			static_cast<double>(ActiveCitizenCount) * 100.0 :
		0.0;

	if (IncumbentWon)
	{
		++mElectionStatus.ElectionsWon;
		ScheduleNextElection(GElectionIntervalYears);
		return;
	}

	mElectionStatus.GameLost = true;
	mElectionStatus.NextElectionYear = 0;
	mElectionStatus.NextElectionMonth = 0;
	mElectionStatus.NextElectionDay = 0;
}

int CMainWorld::GetDaysUntilNextElection() const
{
	if (mElectionStatus.GameLost ||
		mElectionStatus.NextElectionYear <= 0 ||
		mElectionStatus.NextElectionMonth <= 0 ||
		mElectionStatus.NextElectionDay <= 0)
	{
		return -1;
	}

	if (mSimulationYear > mElectionStatus.NextElectionYear ||
		(mSimulationYear == mElectionStatus.NextElectionYear &&
			mSimulationMonth > mElectionStatus.NextElectionMonth) ||
		(mSimulationYear == mElectionStatus.NextElectionYear &&
			mSimulationMonth == mElectionStatus.NextElectionMonth &&
			mSimulationDay >= mElectionStatus.NextElectionDay))
	{
		return 0;
	}

	int DaysUntilElection = 0;
	int Year = mSimulationYear;
	int Month = mSimulationMonth;
	int Day = mSimulationDay;

	while (Year < mElectionStatus.NextElectionYear ||
		(Year == mElectionStatus.NextElectionYear &&
			Month < mElectionStatus.NextElectionMonth) ||
		(Year == mElectionStatus.NextElectionYear &&
			Month == mElectionStatus.NextElectionMonth &&
			Day < mElectionStatus.NextElectionDay))
	{
		++DaysUntilElection;
		++Day;

		if (Day > GetDaysInMonth(Year, Month))
		{
			Day = 1;
			++Month;

			if (Month > 12)
			{
				Month = 1;
				++Year;
			}
		}

		if (DaysUntilElection > 3660)
			break;
	}

	return DaysUntilElection;
}

double CMainWorld::GetElectionWarningScore() const
{
	if (mElectionStatus.GameLost)
		return 1.0;

	const int DaysUntilElection = GetDaysUntilNextElection();

	if (DaysUntilElection < 0)
		return 0.0;

	double ProximityPressure = 0.0;

	if (DaysUntilElection <= 30)
	{
		ProximityPressure = 1.0;
	}
	else if (DaysUntilElection <= 90)
	{
		ProximityPressure =
			0.75 +
			static_cast<double>(90 - DaysUntilElection) / 60.0 * 0.25;
	}
	else if (DaysUntilElection <= 180)
	{
		ProximityPressure =
			0.45 +
			static_cast<double>(180 - DaysUntilElection) / 90.0 * 0.30;
	}
	else if (DaysUntilElection <= 365)
	{
		ProximityPressure =
			0.15 +
			static_cast<double>(365 - DaysUntilElection) / 185.0 * 0.30;
	}

	const int ActiveCitizenCount = (std::max)(1, mPoliticalSnapshot.ActiveCitizenCount);
	const double SupportPercent =
		static_cast<double>(mPoliticalSnapshot.IncumbentCount) /
		static_cast<double>(ActiveCitizenCount) * 100.0;
	const double OppositionPercent =
		static_cast<double>(mPoliticalSnapshot.OppositionCount) /
		static_cast<double>(ActiveCitizenCount) * 100.0;
	const double AbstainPercent =
		static_cast<double>(mPoliticalSnapshot.AbstainCount) /
		static_cast<double>(ActiveCitizenCount) * 100.0;
	const double SupportRisk =
		Clamp<double>((52.0 - SupportPercent) / 22.0, 0.0, 1.0);
	const double OppositionRisk =
		Clamp<double>((OppositionPercent - 34.0) / 24.0, 0.0, 1.0);
	const double AbstainRisk =
		Clamp<double>((AbstainPercent - 18.0) / 25.0, 0.0, 1.0);

	double EventPressure = 0.0;

	if (mTaxEventStatus.Active)
	{
		EventPressure =
			0.40 +
			Clamp<double>(
				static_cast<double>(mTaxEventStatus.DaysActive + 1) / 6.0,
				0.0,
				1.0) * 0.35;

		if (mTaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis)
			EventPressure += 0.10;
	}
	else if (mTaxEventStatus.NotificationDays > 0 &&
		!mTaxEventStatus.Summary.empty())
	{
		EventPressure = 0.18;
	}

	const double Score =
		0.45 * ProximityPressure +
		0.25 * SupportRisk +
		0.12 * OppositionRisk +
		0.08 * AbstainRisk +
		0.20 * (ProximityPressure * Clamp<double>(EventPressure, 0.0, 1.0));

	return Clamp<double>(Score, 0.0, 1.0);
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
	const auto Result = EconomySystem::ApplyDailySettlement(
		this,
		DaysInMonth,
		mGovernmentProfile);
	const long long AdjustedTaxIncome = static_cast<long long>(std::llround(
		static_cast<double>(Result.TaxIncome) *
		static_cast<double>(mEdictModifiers.TaxRevenueMultiplier)));
	long long AdjustedConsumptionTaxIncome = 0;
	long long AdjustedIncomeTaxIncome = 0;
	long long AdjustedPropertyTaxIncome = 0;

	if (Result.TaxIncome > 0 && AdjustedTaxIncome > 0)
	{
		const double BaseTaxIncome = static_cast<double>(Result.TaxIncome);
		AdjustedConsumptionTaxIncome = static_cast<long long>(std::llround(
			static_cast<double>(AdjustedTaxIncome) *
			static_cast<double>(Result.ConsumptionTaxIncome) /
			BaseTaxIncome));
		AdjustedIncomeTaxIncome = static_cast<long long>(std::llround(
			static_cast<double>(AdjustedTaxIncome) *
			static_cast<double>(Result.IncomeTaxIncome) /
			BaseTaxIncome));
		AdjustedPropertyTaxIncome =
			AdjustedTaxIncome -
			AdjustedConsumptionTaxIncome -
			AdjustedIncomeTaxIncome;
	}
	const long long TaxRevenueDelta = AdjustedTaxIncome - Result.TaxIncome;
	const long long DailyEdictUpkeep =
		EdictSystem::CalculateEdictDailyUpkeep(
			mGovernmentEdicts,
			DaysInMonth);
	const long long DailyEdictBudgetDelta =
		mEdictModifiers.DailyBudgetDelta;
	mLastDailyWageCost     = Result.WageCost;
	mLastDailyUpkeepCost   = Result.UpkeepCost;
	mLastDailyExportIncome = Result.ExportIncome;
	mLastDailyTaxIncome    = AdjustedTaxIncome;
	mLastDailyConsumptionTaxIncome = AdjustedConsumptionTaxIncome;
	mLastDailyIncomeTaxIncome = AdjustedIncomeTaxIncome;
	mLastDailyPropertyTaxIncome = AdjustedPropertyTaxIncome;
	mLastDailyEdictCost    = DailyEdictUpkeep - DailyEdictBudgetDelta;
	mLastDailyTaxCollectionEfficiency = Result.TaxCollectionEfficiency;
	mLastDailyNetChange    =
		Result.NetChange + TaxRevenueDelta -
		DailyEdictUpkeep + DailyEdictBudgetDelta;
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
	const ETaxPolicyEventType RequiredTaxEvent =
		GetRequiredTaxPolicyEventForEdict(Type);

	if (RequiredTaxEvent != ETaxPolicyEventType::None)
	{
		if (!mTaxEventStatus.Active || mTaxEventStatus.Type != RequiredTaxEvent)
		{
			OutMessage =
				Definition->DisplayName +
				L"은(는) " +
				GetTaxPolicyEventTitle(RequiredTaxEvent) +
				L" 발생 중에만 시행할 수 있습니다.";
			return false;
		}
	}

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

	std::wstring ResponseMessage;

	switch (Type)
	{
	case EGovernmentEdictType::LaborTaxRelief:
	{
		const int RateDelta = ApplyTaxPolicyRateDelta(
			mGovernmentProfile.TaxPolicy,
			ETaxPolicyType::Income,
			-4);
		ResolveTaxPolicyEvent(
			L"근로세 경감과 함께 근로층 세금 파업이 진정되었습니다.",
			true);
		ResponseMessage =
			L"소득세 " +
			std::to_wstring((std::max)(0, -RateDelta)) +
			L"%p 인하";
		break;
	}
	case EGovernmentEdictType::PropertyTaxRelief:
	{
		const int RateDelta = ApplyTaxPolicyRateDelta(
			mGovernmentProfile.TaxPolicy,
			ETaxPolicyType::Property,
			-10);
		ResolveTaxPolicyEvent(
			L"재산세 유예와 함께 주거층 반발이 진정되었습니다.",
			true);
		ResponseMessage =
			L"재산세 " +
			std::to_wstring((std::max)(0, -RateDelta)) +
			L"%p 인하";
		break;
	}
	case EGovernmentEdictType::EmergencyAusterity:
	{
		const long long EmergencyFunds = 12000;
		mNationalBudget += EmergencyFunds;
		mLastDailyNetChange += EmergencyFunds;
		ResolveTaxPolicyEvent(
			L"긴축 예산이 발동되어 국고 위기 경보가 진정되었습니다.",
			true);
		ResponseMessage = L"긴급 자금 $12,000 투입";
		break;
	}
	default:
		break;
	}

	SyncGovernmentActionFromEdict(Type, true);
	RefreshEdictModifiers();
	RefreshPoliticalSnapshot();

	OutMessage = Definition->DisplayName + L" 시행";

	if (!ResponseMessage.empty())
	{
		OutMessage += L" / ";
		OutMessage += ResponseMessage;
	}

	return true;
}

bool CMainWorld::AdjustTaxPolicy(
	ETaxPolicyType Type,
	int DeltaPercent,
	std::wstring& OutMessage)
{
	int* TargetRatePercent =
		ResolveTaxRatePercent(mGovernmentProfile.TaxPolicy, Type);

	if (!TargetRatePercent)
	{
		OutMessage = L"정의되지 않은 세율 정책입니다.";
		return false;
	}

	const int PreviousRatePercent = *TargetRatePercent;
	const int NewRatePercent = (std::max)(
		GetTaxPolicyMinPercent(Type),
		(std::min)(
			GetTaxPolicyMaxPercent(Type),
			PreviousRatePercent + DeltaPercent));

	if (PreviousRatePercent == NewRatePercent)
	{
		OutMessage =
			std::wstring(GetTaxPolicyDisplayName(Type)) +
			(DeltaPercent < 0 ?
				L"는 이미 최저 세율입니다." :
				L"는 이미 최고 세율입니다.");
		return false;
	}

	*TargetRatePercent = NewRatePercent;
	OutMessage =
		std::wstring(GetTaxPolicyDisplayName(Type)) +
		L" " +
		std::to_wstring(PreviousRatePercent) +
		L"% -> " +
		std::to_wstring(NewRatePercent) +
		L"%";
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

void CMainWorld::ApplyDailyTaxPolicyEventEffects()
{
	if (!mTaxEventStatus.Active ||
		mTaxEventStatus.Type == ETaxPolicyEventType::None)
	{
		return;
	}

	const float Escalation =
		1.0f +
		(std::min)(1.35f,
			static_cast<float>(mTaxEventStatus.DaysActive) / 4.0f);
	const float ControlBreakdown =
		1.0f +
		(std::min)(0.80f,
			static_cast<float>(mTaxEventStatus.DaysActive) / 6.0f);
	float FoodDelta = 0.f;
	float HealthDelta = 0.f;
	float FunDelta = 0.f;
	float FaithDelta = 0.f;
	float HousingDelta = 0.f;
	float JobDelta = 0.f;
	float FreedomDelta = 0.f;
	float SecurityDelta = 0.f;

	switch (mTaxEventStatus.Type)
	{
	case ETaxPolicyEventType::WorkerTaxStrike:
		JobDelta = -0.80f * Escalation;
		FreedomDelta = -0.55f * ControlBreakdown;
		SecurityDelta = -0.25f * ControlBreakdown;
		FunDelta = -0.16f * Escalation;
		break;
	case ETaxPolicyEventType::PropertyTaxBacklash:
		HousingDelta = -0.85f * Escalation;
		FreedomDelta = -0.45f * ControlBreakdown;
		SecurityDelta = -0.20f * ControlBreakdown;
		HealthDelta = -0.10f * Escalation;
		break;
	case ETaxPolicyEventType::BudgetCrisis:
		FoodDelta = -0.70f * Escalation;
		JobDelta = -0.55f * Escalation;
		SecurityDelta = -0.40f * ControlBreakdown;
		HealthDelta = -0.18f * Escalation;
		FunDelta = -0.18f * Escalation;
		break;
	default:
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
			FoodDelta,
			HealthDelta,
			FunDelta,
			FaithDelta,
			HousingDelta,
			JobDelta,
			FreedomDelta,
			SecurityDelta);
	}
}

void CMainWorld::TickTaxPolicyEvents()
{
	if (mTaxEventStatus.NotificationDays > 0)
		--mTaxEventStatus.NotificationDays;

	if (!mTaxEventStatus.Active && mTaxEventStatus.CooldownDays > 0)
		--mTaxEventStatus.CooldownDays;

	const int ActiveCitizenCount =
		(std::max)(0, mPoliticalSnapshot.ActiveCitizenCount);
	const double SupportPercent =
		ActiveCitizenCount > 0 ?
		static_cast<double>(mPoliticalSnapshot.IncumbentCount) /
			static_cast<double>(ActiveCitizenCount) * 100.0 :
		50.0;
	const float WorkerTaxBurden = GetCitizenTaxBurdenNormalized(
		mGovernmentProfile.TaxPolicy,
		true,
		false);
	const float ResidentTaxBurden = GetCitizenTaxBurdenNormalized(
		mGovernmentProfile.TaxPolicy,
		false,
		true);
	const float OverallTaxBurden = GetCitizenTaxBurdenNormalized(
		mGovernmentProfile.TaxPolicy,
		true,
		true);
	const bool WorkerPressure =
		WorkerTaxBurden >= 0.45f ||
		(WorkerTaxBurden >= 0.30f && SupportPercent <= 48.0);
	const bool PropertyPressure =
		ResidentTaxBurden >= 0.50f ||
		(ResidentTaxBurden >= 0.35f && SupportPercent <= 50.0);
	const bool BudgetPressure =
		OverallTaxBurden <= -0.35f &&
		mLastDailyNetChange <= -5000;

	if (WorkerPressure)
		++mWorkerTaxPressureDays;
	else
		mWorkerTaxPressureDays = (std::max)(0, mWorkerTaxPressureDays - 2);

	if (PropertyPressure)
		++mPropertyTaxPressureDays;
	else
		mPropertyTaxPressureDays =
			(std::max)(0, mPropertyTaxPressureDays - 2);

	if (BudgetPressure)
		++mBudgetCrisisPressureDays;
	else
		mBudgetCrisisPressureDays =
			(std::max)(0, mBudgetCrisisPressureDays - 2);

	if (mTaxEventStatus.Active)
	{
		++mTaxEventStatus.DaysActive;
		mTaxEventStatus.Summary = BuildTaxPolicyEventWarningSummary(
			mTaxEventStatus.Type,
			mTaxEventStatus.DaysActive);

		if (mTaxEventStatus.RemainingDays > 0)
			--mTaxEventStatus.RemainingDays;

		const bool CanResolveEarly = mTaxEventStatus.DaysActive >= 3;

		switch (mTaxEventStatus.Type)
		{
		case ETaxPolicyEventType::WorkerTaxStrike:
			if (CanResolveEarly && !WorkerPressure)
			{
				ResolveTaxPolicyEvent(
					L"근로층 세금 파업이 진정되었습니다.",
					true);
				return;
			}
			break;
		case ETaxPolicyEventType::PropertyTaxBacklash:
			if (CanResolveEarly && !PropertyPressure)
			{
				ResolveTaxPolicyEvent(
					L"재산세 반발이 잦아들었습니다.",
					true);
				return;
			}
			break;
		case ETaxPolicyEventType::BudgetCrisis:
			if (CanResolveEarly && !BudgetPressure)
			{
				ResolveTaxPolicyEvent(
					L"국고 위기 경보가 완화되었습니다.",
					true);
				return;
			}
			break;
		default:
			break;
		}

		if (mTaxEventStatus.RemainingDays <= 0)
		{
			switch (mTaxEventStatus.Type)
			{
			case ETaxPolicyEventType::WorkerTaxStrike:
				ResolveTaxPolicyEvent(
					L"근로층 세금 파업이 소강 상태에 접어들었습니다.",
					false);
				return;
			case ETaxPolicyEventType::PropertyTaxBacklash:
				ResolveTaxPolicyEvent(
					L"재산세 반발이 소강 상태에 접어들었습니다.",
					false);
				return;
			case ETaxPolicyEventType::BudgetCrisis:
				ResolveTaxPolicyEvent(
					L"국고 위기 경보가 일단락되었습니다.",
					false);
				return;
			default:
				break;
			}
		}

		long long DailyEventPenalty = 0;
		const float Escalation =
			1.0f +
			(std::min)(1.50f,
				static_cast<float>(mTaxEventStatus.DaysActive) / 4.0f);

		switch (mTaxEventStatus.Type)
		{
		case ETaxPolicyEventType::WorkerTaxStrike:
			DailyEventPenalty = static_cast<long long>(std::llround(
				900.0 + 450.0 * static_cast<double>(Escalation)));
			break;
		case ETaxPolicyEventType::PropertyTaxBacklash:
			DailyEventPenalty = static_cast<long long>(std::llround(
				750.0 + 360.0 * static_cast<double>(Escalation)));
			break;
		case ETaxPolicyEventType::BudgetCrisis:
			DailyEventPenalty = static_cast<long long>(std::llround(
				1400.0 + 650.0 * static_cast<double>(Escalation)));
			break;
		default:
			break;
		}

		if (DailyEventPenalty > 0)
		{
			mNationalBudget -= DailyEventPenalty;
			mLastDailyNetChange -= DailyEventPenalty;
		}

		return;
	}

	if (mTaxEventStatus.CooldownDays > 0)
		return;

	if (mBudgetCrisisPressureDays >= 4)
	{
		StartTaxPolicyEvent(
			ETaxPolicyEventType::BudgetCrisis,
			L"감세 장기화와 적자가 겹쳐 국고 위기 경보가 발령되었습니다.",
			-6000);
		mBudgetCrisisPressureDays = 0;
		return;
	}

	if (mWorkerTaxPressureDays >= 5 &&
		mWorkerTaxPressureDays >= mPropertyTaxPressureDays)
	{
		StartTaxPolicyEvent(
			ETaxPolicyEventType::WorkerTaxStrike,
			L"근로층 과세 압박이 누적되어 작업장 불만이 확산되고 있습니다.",
			-4000);
		mWorkerTaxPressureDays = 0;
		return;
	}

	if (mPropertyTaxPressureDays >= 5)
	{
		StartTaxPolicyEvent(
			ETaxPolicyEventType::PropertyTaxBacklash,
			L"주거층의 재산세 반발이 커지며 정권 불만이 높아지고 있습니다.",
			-3500);
		mPropertyTaxPressureDays = 0;
	}
}

void CMainWorld::StartTaxPolicyEvent(
	ETaxPolicyEventType Type,
	const std::wstring& Summary,
	long long ImmediateBudgetDelta)
{
	(void)Summary;

	if (Type == ETaxPolicyEventType::None || mTaxEventStatus.Active)
		return;

	mTaxEventStatus = FTaxPolicyEventStatus();
	mTaxEventStatus.Type = Type;
	mTaxEventStatus.Active = true;
	mTaxEventStatus.RemainingDays = GetTaxPolicyEventDurationDays(Type);
	mTaxEventStatus.NotificationDays = 6;
	mTaxEventStatus.DaysActive = 0;
	mTaxEventStatus.TriggerYear = mSimulationYear;
	mTaxEventStatus.TriggerMonth = mSimulationMonth;
	mTaxEventStatus.TriggerDay = mSimulationDay;
	mTaxEventStatus.Title = GetTaxPolicyEventTitle(Type);
	mTaxEventStatus.Summary = BuildTaxPolicyEventWarningSummary(Type, 0);
	mWorkerTaxPressureDays = 0;
	mPropertyTaxPressureDays = 0;
	mBudgetCrisisPressureDays = 0;

	if (ImmediateBudgetDelta != 0)
	{
		mNationalBudget += ImmediateBudgetDelta;
		mLastDailyNetChange += ImmediateBudgetDelta;
	}
}

void CMainWorld::ResolveTaxPolicyEvent(
	const std::wstring& Summary,
	bool Success)
{
	(void)Summary;

	if (mTaxEventStatus.Type == ETaxPolicyEventType::None)
		return;

	mTaxEventStatus.Active = false;
	mTaxEventStatus.RemainingDays = 0;
	mTaxEventStatus.CooldownDays = GetTaxPolicyEventCooldownDays(Success);
	mTaxEventStatus.NotificationDays = 8;
	mTaxEventStatus.Summary = BuildTaxPolicyEventResolvedSummary(
		mTaxEventStatus.Type,
		Success);
	mTaxEventStatus.DaysActive = 0;
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
	mUIManager->CreateWidget<CAlmanacWidget>(
		GAlmanacWidgetName, 320);
}
