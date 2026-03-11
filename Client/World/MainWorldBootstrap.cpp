#include "MainWorld.h"
#include "BusRouteSystem.h"
#include "MainWorldConfig.h"
#include "RoadNetwork.h"
#include "../ObjectNames.h"
#include "Asset/AssetManager.h"
#include "../UI/TopHudWidget.h"
#include "../UI/BuildMenuWidget.h"
#include "../UI/AlmanacWidget.h"
#include "../UI/EdictWidget.h"
#include "../Building/BuildingCatalog.h"
#include "World/WorldUIManager.h"
#include "Render/RenderManager.h"
#include "Component/TileMapComponent.h"
#include "../Map/TileMapMain.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/PlacementBuildingVisual.h"
#include "../Player/MainCamera.h"
#include "../Map/PlacementController.h"
#include "../Politics/EdictSystem.h"
#include "../Politics/PoliticsSystem.h"
#include <Windows.h>
#include <cstring>
#include <string>

namespace
{
    int ResolveLayerOrder(
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
    }

    void ConfigureOverlayTileMap(
        const std::shared_ptr<CTileMapObject>& TileMapObj,
        const std::shared_ptr<CTileMapComponent>& BaseTileMap,
        const std::shared_ptr<CTileMapObject>& OverlayObj,
        const std::string& LayerName,
        int PreferredOrder,
        ERenderListSort SortType,
        const FVector4& InitialColor)
    {
        if (!TileMapObj || !BaseTileMap || !OverlayObj)
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

        const int TileCount =
            OverlayMap->GetTileCountX() * OverlayMap->GetTileCountY();

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
    }

    void ConfigureExpansionTileMap(
        const std::shared_ptr<CTileMapObject>& TileMapObj,
        const std::shared_ptr<CTileMapComponent>& BaseTileMap,
        const std::shared_ptr<CTileMapObject>& OverlayObj)
    {
        if (!TileMapObj || !BaseTileMap || !OverlayObj)
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
    }

    void CenterMainCameraOnTileMap(
        const std::shared_ptr<CMainCamera>& MainCameraObj,
        const std::shared_ptr<CTileMapObject>& TileMapObj)
    {
        if (!MainCameraObj || !TileMapObj)
            return;

        auto TileMapComp = TileMapObj->GetTileMap().lock();

        if (!TileMapComp)
            return;

        const int CountX = TileMapComp->GetTileCountX();
        const int CountY = TileMapComp->GetTileCountY();

        if (CountX <= 0 || CountY <= 0)
            return;

        const int CenterX = CountX / 2;
        const int CenterY = CountY / 2;
        const int CenterIndex = CenterY * CountX + CenterX;
        auto CenterTile = TileMapComp->GetTile(CenterIndex).lock();

        if (!CenterTile)
            return;

        const FVector2 Center = CenterTile->GetCenter();
        const FVector3 TileMapWorldPos = TileMapObj->GetWorldPos();
        const float CameraZ = MainCameraObj->GetWorldPos().z;

        MainCameraObj->SetWorldPos(
            Center.x + TileMapWorldPos.x,
            Center.y + TileMapWorldPos.y,
            CameraZ);
    }

    void CreateStarterBuilding(
        CMainWorld& World,
        const std::shared_ptr<CTileMapObject>& TileMapObj,
        const std::string& ObjectName,
        const std::string& BuildingId,
        int OffsetX,
        int OffsetY)
    {
        auto Building = World.CreateGameObject<CPlacementAreaObject>(ObjectName);
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
                CatalogEntry->Category,
                CatalogEntry->CategoryLocalIndex);

            if (!IconPath.empty())
                BuildingObj->SetBuildingSpriteTexturePath(IconPath);

            BuildingObj->ApplyCatalogEntry(*CatalogEntry);
        }
        else
        {
            BuildingObj->SetBuildingDisplayInfo(
                BuildingId, "기본", false, 0);
        }

        auto Visual = World.CreateGameObject<CBuildingVisual>(
            ObjectName + "_Visual");
        auto VisualObj = Visual.lock();

        if (VisualObj)
            VisualObj->SetBuilding(Building);
    }
}

CMainWorld::CMainWorld()
{
}

CMainWorld::~CMainWorld()
{
}

void CMainWorld::RebuildRoadNetwork()
{
    if (!mRoadNetwork)
        mRoadNetwork = std::make_shared<CRoadNetwork>();

    auto TileMapObj = FindObject<CTileMapObject>(GTileMapObjectName).lock();

    if (!TileMapObj)
    {
        mRoadNetwork->Rebuild(std::shared_ptr<CTileMapComponent>());
        return;
    }

    mRoadNetwork->Rebuild(TileMapObj->GetTileMap().lock());
    RebuildBusRoutes();
}

void CMainWorld::RebuildBusRoutes()
{
    if (!mBusRouteSystem)
        mBusRouteSystem = std::make_shared<CBusRouteSystem>();

    mBusRouteSystem->Rebuild(mSelf.lock(), mRoadNetwork.get());
}

void CMainWorld::ResetWorldState()
{
    mSpawnedNpcCount = 0;
    mNpcSpawnAccum = 0.f;
    mCitizenReassignAccum = 0.f;
    mNationalBudget = MainWorldConfig::GInitialNationalBudget;
    mLastDailyWageCost = 0;
    mLastDailyUpkeepCost = 0;
    mLastDailyExportIncome = 0;
    mLastDailyTaxIncome = 0;
    mLastDailyConsumptionTaxIncome = 0;
    mLastDailyIncomeTaxIncome = 0;
    mLastDailyPropertyTaxIncome = 0;
    mLastDailyEdictCost = 0;
    mLastDailyImportExpense = 0;
    mLastDailyNetChange = 0;
    mLastDailyTaxCollectionEfficiency = 0.0;
    mSimulationYear = MainWorldConfig::GSimulationStartYear;
    mSimulationMonth = MainWorldConfig::GSimulationStartMonth;
    mSimulationDay = MainWorldConfig::GSimulationStartDay;
    mDayProgressAccum = 0.f;
    mSecondsPerSimulationDay = MainWorldConfig::GSecondsPerSimulationDay;
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
}

bool CMainWorld::Init()
{
    CWorld::Init();

    CRenderManager::GetInst()->SetDebugTarget(false);

    LoadCitizenAnimation2D();
    CreateUI();

    auto MainCamera = CreateGameObject<CMainCamera>("MainCamera");
    CreateGameObject<CPlacementController>(GPlacementControllerName);

    auto TileMap = CreateGameObject<CTileMapMain>(GTileMapObjectName);
    auto TileMapObj = std::dynamic_pointer_cast<CTileMapObject>(
        TileMap.lock());
    auto FloorBlueTileMap =
        CreateGameObject<CTileMapObject>(GTileMapFloorBlueName);
    auto FloorYellowTileMap =
        CreateGameObject<CTileMapObject>(GTileMapFloorYellowName);
    auto ExpansionTileMap =
        CreateGameObject<CTileMapObject>(GTileMapExpansionName);
    auto MainCameraObj = MainCamera.lock();
    auto FloorBlueTileMapObj = FloorBlueTileMap.lock();
    auto FloorYellowTileMapObj = FloorYellowTileMap.lock();
    auto ExpansionTileMapObj = ExpansionTileMap.lock();

    if (TileMapObj &&
        FloorBlueTileMapObj &&
        FloorYellowTileMapObj &&
        ExpansionTileMapObj)
    {
        auto BaseTileMap = TileMapObj->GetTileMap().lock();

        if (BaseTileMap)
        {
            ResolveLayerOrder("MapExpansion", 1, ERenderListSort::None);
            ResolveLayerOrder("MapFloorBlue", 2, ERenderListSort::None);
            ResolveLayerOrder("BuildingVisual", 3, ERenderListSort::Y);

            ConfigureExpansionTileMap(
                TileMapObj,
                BaseTileMap,
                ExpansionTileMapObj);
            ConfigureOverlayTileMap(
                TileMapObj,
                BaseTileMap,
                FloorBlueTileMapObj,
                "MapFloorBlue",
                2,
                ERenderListSort::None,
                FVector4::Blue);
            ConfigureOverlayTileMap(
                TileMapObj,
                BaseTileMap,
                FloorYellowTileMapObj,
                "MapFloorBlue",
                2,
                ERenderListSort::None,
                FVector4(1.f, 1.f, 0.f, 1.f));
        }
    }

    CenterMainCameraOnTileMap(MainCameraObj, TileMapObj);

    CreateStarterBuilding(
        *this,
        TileMapObj,
        MainWorldConfig::GStarterTeamsterObjectName,
        "build_1_5",
        -14,
        8);
    CreateStarterBuilding(
        *this,
        TileMapObj,
        MainWorldConfig::GStarterRanchObjectName,
        "build_2_6",
        12,
        9);
    CreateStarterBuilding(
        *this,
        TileMapObj,
        MainWorldConfig::GStarterFarmObjectName,
        "build_2_4",
        -10,
        -10);
    CreateStarterBuilding(
        *this,
        TileMapObj,
        MainWorldConfig::GStarterDormitoryObjectName,
        "build_4_3",
        0,
        0);
    CreateStarterBuilding(
        *this,
        TileMapObj,
        MainWorldConfig::GStarterHarborObjectName,
        "build_1_3",
        15,
        -12);
    RebuildRoadNetwork();

    ResetWorldState();
    InitializeElectionSchedule();

    for (int i = 0; i < MainWorldConfig::GInitialNpcCount; ++i)
    {
        SpawnCitizenOrb();
    }

    ReassignCitizenNeeds();
    RefreshPoliticalSnapshot();
    RefreshEdictModifiers();

    return true;
}

void CMainWorld::LoadCitizenAnimation2D()
{
    auto CreateCitizenVariantAnimations = [&](const char* Prefix,
        float WalkTopY,
        float IdleTopY,
        float WalkReverseTopY)
    {
        for (int Direction = 0;
            Direction < MainWorldConfig::GCitizenDirectionCount;
            ++Direction)
        {
            const std::string IdleAnimationName =
                std::string(Prefix) + "Idle_Dir" +
                std::to_string(Direction);
            const std::string WalkAnimationName =
                std::string(Prefix) + "Walk_Dir" +
                std::to_string(Direction);
            const float RightBottomX =
                MainWorldConfig::GCitizenRightBottomStartX +
                static_cast<float>(Direction) *
                MainWorldConfig::GCitizenDirectionStepX;
            const float StartX =
                RightBottomX - MainWorldConfig::GCitizenFrameWidth;

            mWorldAssetManager->CreateAnimation(IdleAnimationName);
            mWorldAssetManager->SetAnimation2DTextureType(
                IdleAnimationName,
                EAnimation2DTextureType::SpriteSheet);
            mWorldAssetManager->SetTexture(
                IdleAnimationName,
                MainWorldConfig::GCitizenSheetTextureName,
                MainWorldConfig::GCitizenSheetTextureFile);
            mWorldAssetManager->AddFrame(
                IdleAnimationName,
                StartX,
                IdleTopY,
                MainWorldConfig::GCitizenFrameWidth,
                MainWorldConfig::GCitizenFrameHeight);

            mWorldAssetManager->CreateAnimation(WalkAnimationName);
            mWorldAssetManager->SetAnimation2DTextureType(
                WalkAnimationName,
                EAnimation2DTextureType::SpriteSheet);
            mWorldAssetManager->SetTexture(
                WalkAnimationName,
                MainWorldConfig::GCitizenSheetTextureName,
                MainWorldConfig::GCitizenSheetTextureFile);
            mWorldAssetManager->AddFrame(
                WalkAnimationName,
                StartX,
                WalkTopY,
                MainWorldConfig::GCitizenFrameWidth,
                MainWorldConfig::GCitizenFrameHeight);
            mWorldAssetManager->AddFrame(
                WalkAnimationName,
                StartX,
                WalkReverseTopY,
                MainWorldConfig::GCitizenFrameWidth,
                MainWorldConfig::GCitizenFrameHeight);
        }
    };

    CreateCitizenVariantAnimations(
        MainWorldConfig::GCitizenBlueAnimationPrefix,
        MainWorldConfig::GCitizenBlueWalkTopY,
        MainWorldConfig::GCitizenBlueIdleTopY,
        MainWorldConfig::GCitizenBlueWalkReverseTopY);
    CreateCitizenVariantAnimations(
        MainWorldConfig::GCitizenRedAnimationPrefix,
        MainWorldConfig::GCitizenRedWalkTopY,
        MainWorldConfig::GCitizenRedIdleTopY,
        MainWorldConfig::GCitizenRedWalkReverseTopY);
}

void CMainWorld::LoadAnimation2D()
{
    mWorldAssetManager->CreateAnimation("PlayerIdle");
    mWorldAssetManager->SetAnimation2DTextureType(
        "PlayerIdle", EAnimation2DTextureType::Frame);

    std::vector<const TCHAR*> TextureFileName;

    for (int i = 0; i < 7; ++i)
    {
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
        mWorldAssetManager->AddFrame("MonsterAttack", i * 90.f, 0.f,
            90.f, 60.f);
    }

    mWorldAssetManager->CreateAnimation("Explosion");
    mWorldAssetManager->SetAnimation2DTextureType("Explosion",
        EAnimation2DTextureType::Frame);

    for (int i = 0; i < 89; ++i)
    {
        TCHAR* FileName = new TCHAR[MAX_PATH];
        memset(FileName, 0, sizeof(TCHAR) * MAX_PATH);
        wsprintf(FileName,
            TEXT("Explosion/Explosion_%05d.png"), i + 1);
        TextureFileName.push_back(FileName);
    }

    mWorldAssetManager->SetTexture("Explosion",
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
