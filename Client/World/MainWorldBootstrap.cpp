#include "MainWorld.h"
#include "BusRouteSystem.h"
#include "MainWorldConfig.h"
#include "RoadNetwork.h"
#include "RuntimeConfigRegistry.h"
#include "../GameConstants.h"
#include "../GameBalanceTuning.h"
#include "../ObjectNames.h"
#include "Asset/AssetManager.h"
#include "../UI/UILayoutLoader.h"
#include "../UI/AlmanacPageData.h"
#include "../UI/TropicoUiAssetCatalog.h"
#include "../UI/TropicoUiTheme.h"
#include "../UI/EventWidget.h"
#include "../UI/ResultWidget.h"
#include "../UI/TopHudWidget.h"
#include "../UI/TradeWidget.h"
#include "../UI/BuildMenuWidget.h"
#include "../UI/AlmanacWidget.h"
#include "../UI/EdictWidget.h"
#include "WorldStatsSnapshot.h"
#include "../Building/BuildingCatalog.h"
#include "../Economy/ResourceTradePricing.h"
#include "World/WorldUIManager.h"
#include "Render/RenderManager.h"
#include "Component/TileMapComponent.h"
#include "../Map/TileMapMain.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/PlacementBuildingVisual.h"
#include "../Player/MainCamera.h"
#include "../Map/PlacementController.h"
#include "../Politics/ConstitutionSystem.h"
#include "../Politics/EdictSystem.h"
#include "../Politics/PoliticsSystem.h"
#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <sstream>
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
            const std::string SpriteTexturePath =
                GetCatalogEntrySpriteTexturePathUtf8(*CatalogEntry);

            if (!SpriteTexturePath.empty())
                BuildingObj->SetBuildingSpriteTexturePath(
                    SpriteTexturePath);

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

#ifdef _DEBUG
    std::wstring GetConstitutionValidationPath(const wchar_t* FileName)
    {
        wchar_t ModulePath[MAX_PATH] = {};
        const DWORD Length = GetModuleFileNameW(
            nullptr,
            ModulePath,
            MAX_PATH);

        std::wstring Result =
            Length > 0 ? std::wstring(ModulePath, Length) : std::wstring();
        const size_t LastSlash = Result.find_last_of(L"\\/");

        if (LastSlash != std::wstring::npos)
            Result.erase(LastSlash + 1);
        else
            Result.clear();

        Result += FileName ? FileName : L"";
        return Result;
    }

    bool IsConstitutionValidationRequested()
    {
        wchar_t Buffer[8] = {};
        const DWORD Count = GetEnvironmentVariableW(
            L"TROPICO_VALIDATE_CONSTITUTION_UI",
            Buffer,
            static_cast<DWORD>(std::size(Buffer)));

        if (Count > 0 && Buffer[0] != L'0')
            return true;

        const std::wstring RequestPath =
            GetConstitutionValidationPath(
                L"ConstitutionValidation.request");
        const DWORD Attributes = GetFileAttributesW(RequestPath.c_str());
        return Attributes != INVALID_FILE_ATTRIBUTES &&
            (Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    void OutputConstitutionValidationLine(const std::string& Line)
    {
        OutputDebugStringA("[ConstitutionValidation] ");
        OutputDebugStringA(Line.c_str());
        OutputDebugStringA("\n");
    }

    void WriteConstitutionValidationLog(
        const std::vector<std::string>& Lines)
    {
        FILE* File = nullptr;
        const std::wstring LogPath =
            GetConstitutionValidationPath(L"ConstitutionValidation.log");

        if (_wfopen_s(&File, LogPath.c_str(), L"wb") != 0 ||
            !File)
        {
            return;
        }

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::string& Line = Lines[Index];
            fwrite(Line.c_str(), 1, Line.size(), File);
            fwrite("\n", 1, 1, File);
        }

        fclose(File);
    }
#endif
}

void CMainWorld::RebuildRoadNetwork()
{
    if (!mInfrastructure.RoadNetwork)
        mInfrastructure.RoadNetwork = std::make_shared<CRoadNetwork>();

    auto TileMapObj = FindObject<CTileMapObject>(GTileMapObjectName).lock();

    if (!TileMapObj)
    {
        mInfrastructure.RoadNetwork->Rebuild(std::shared_ptr<CTileMapComponent>());
        return;
    }

    mInfrastructure.RoadNetwork->Rebuild(TileMapObj->GetTileMap().lock());
    RebuildBusRoutes();
}

void CMainWorld::RebuildBusRoutes()
{
    if (!mInfrastructure.BusRouteSystem)
        mInfrastructure.BusRouteSystem = std::make_shared<CBusRouteSystem>();

    mInfrastructure.BusRouteSystem->Rebuild(mSelf.lock(), mInfrastructure.RoadNetwork.get());
}

void CMainWorld::ResetWorldState()
{
    mPopulation.SpawnedNpcCount = 0;
    mPopulation.NpcSpawnAccum = 0.f;
    mPopulation.CitizenReassignAccum = 0.f;
    mBudget.NationalBudget = MainWorldConfig::GInitialNationalBudget;
    mBudget.LastDailyWageCost = 0;
    mBudget.LastDailyUpkeepCost = 0;
    mBudget.LastDailyExportIncome = 0;
    mBudget.LastDailyTaxIncome = 0;
    mBudget.LastDailyConsumptionTaxIncome = 0;
    mBudget.LastDailyIncomeTaxIncome = 0;
    mBudget.LastDailyPropertyTaxIncome = 0;
    mBudget.LastDailyEdictCost = 0;
    mBudget.LastDailyImportExpense = 0;
    mBudget.LastDailyNetChange = 0;
    mBudget.LastDailyTaxCollectionEfficiency = 0.0;
    mSimulation.Year = MainWorldConfig::GSimulationStartYear;
    mSimulation.Month = MainWorldConfig::GSimulationStartMonth;
    mSimulation.Day = MainWorldConfig::GSimulationStartDay;
    mSimulation.DayProgressAccum = 0.f;
    mSimulation.SecondsPerSimulationDay = MainWorldConfig::GSecondsPerSimulationDay;
    mSimulation.Paused = false;
    mSimulation.SpeedMultiplier = 1;
    mSimulation.PoliticalSnapshotAccum = 0.f;
    mResultRuntime.TermStartYear = mSimulation.Year;
    mResultRuntime.TermStartMonth = mSimulation.Month;
    mResultRuntime.TermStartDay = mSimulation.Day;
    mResultRuntime.InitialBuildingCount = 0;
    mResultRuntime.PeakSupportPercent = 0.0;
    mResultRuntime.ResultShown = false;
    mPolicy.WorkerTaxPressureDays = 0;
    mPolicy.PropertyTaxPressureDays = 0;
    mPolicy.BudgetCrisisPressureDays = 0;
    PoliticsSystem::SetDefaultGovernmentProfile(mPolicy.GovernmentProfile);
    mPolicy.PoliticalSnapshot = FPoliticalWorldSnapshot();
    if (!mServices.ElectionService)
        mServices.ElectionService.reset(new CMainWorldElectionService());
    mServices.ElectionService->Reset();
    mPolicy.TaxEventStatus = FTaxPolicyEventStatus();
    mPolicy.EraProgress = FEraProgressState();
    mPolicy.EraTransition = FEraTransitionState();
    mPolicy.KnowledgeState.Reset();
    EdictSystem::InitializeGovernmentEdictStates(mPolicy.GovernmentEdicts);
    mPolicy.EdictModifiers = FGovernmentEdictModifiers();
    mTradeDiplomacyState.ActiveTradeRoutes.clear();
    mTradeDiplomacyState.CompletedTradeRoutes.clear();
    mTradeDiplomacyState.ForeignPowerStandingStates = {};
    mTradeDiplomacyState.ForeignPowerStates = {};
    if (!mServices.PoliticalDemandService)
        mServices.PoliticalDemandService.reset(new CMainWorldPoliticalDemandService());
    mServices.PoliticalDemandService->Reset();
    if (!mServices.WorldCrisisService)
        mServices.WorldCrisisService.reset(new CMainWorldWorldCrisisService());
    mServices.WorldCrisisService->Reset();
    mTradeDiplomacyState.NextTradeRouteId = 1;
    mTradeDiplomacyState.NextTradeRouteCompletionRecordId = 1;
    mTradeDiplomacyState.TradeRouteCompletionNotificationVersion = 0;
    mScenarioElectionPromptPending = false;
    ResourceTradePricing::ResetWorldMarketPriceState();
}

bool CMainWorld::Init()
{
    CWorld::Init();

    CRenderManager::GetInst()->SetDebugTarget(false);
    RuntimeConfigRegistry::Reset();
    GameConstants::RegisterRuntimeConfig();
    GameBalanceTuning::RegisterRuntimeConfig();
    RegisterRuntimeConfig();
    EdictSystem::RegisterRuntimeConfig();
    UILayoutLoader::RegisterRuntimeConfig();
    TropicoUiAssets::RegisterRuntimeConfig();
    TropicoUiTheme::RegisterRuntimeConfig();
    AlmanacPageData::RegisterRuntimeConfig();
    mLastGameConstantsGeneration =
        GameConstants::GetRuntimeConfigGeneration();
    mLastEdictConfigGeneration =
        EdictSystem::GetRuntimeConfigGeneration();

    LoadCitizenAnimation2D();
    CreateUI();
#ifdef _DEBUG
    RunDebugConstitutionValidationIfRequested();
#endif

    auto MainCamera = CreateGameObject<CMainCamera>("MainCamera");
    CreateGameObject<CPlacementController>(GPlacementControllerName);

    auto TileMap = CreateGameObject<CTileMapMain>(GTileMapObjectName);
    auto TileMapObj = std::dynamic_pointer_cast<CTileMapObject>(
        TileMap.lock());
    auto FloorBlueTileMap =
        CreateGameObject<CTileMapObject>(GTileMapFloorBlueName);
    auto FloorYellowTileMap =
        CreateGameObject<CTileMapObject>(GTileMapFloorYellowName);
    auto RoadPreviewTileMap =
        CreateGameObject<CTileMapObject>(GTileMapRoadPreviewName);
    auto ExpansionTileMap =
        CreateGameObject<CTileMapObject>(GTileMapExpansionName);
    auto MainCameraObj = MainCamera.lock();
    auto FloorBlueTileMapObj = FloorBlueTileMap.lock();
    auto FloorYellowTileMapObj = FloorYellowTileMap.lock();
    auto RoadPreviewTileMapObj = RoadPreviewTileMap.lock();
    auto ExpansionTileMapObj = ExpansionTileMap.lock();

    if (TileMapObj &&
        FloorBlueTileMapObj &&
        FloorYellowTileMapObj &&
        RoadPreviewTileMapObj &&
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
            ConfigureOverlayTileMap(
                TileMapObj,
                BaseTileMap,
                RoadPreviewTileMapObj,
                "MapFloorBlue",
                2,
                ERenderListSort::None,
                FVector4(1.f, 0.72f, 0.20f, 1.f));
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
        MainWorldConfig::GStarterAlfalfaFarmObjectName,
        "build_2_4",
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
        MainWorldConfig::GStarterTenementAObjectName,
        "build_4_8",
        -4,
        0);
    CreateStarterBuilding(
        *this,
        TileMapObj,
        MainWorldConfig::GStarterTenementBObjectName,
        "build_4_8",
        4,
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

    RefreshPowerGridCoverage();
    ReassignCitizenNeeds();
    RefreshBuildingPollutionExposure();
    RefreshEraProgress();
    RefreshPoliticalSnapshot();
    RefreshEdictModifiers();
    RefreshForeignTradeDiplomacy(false);
    RefreshWorldMarketPrices();
    InitializeResultTracking();
    mScenarioRunner.Init();

    return true;
}

void CMainWorld::InitializeResultTracking()
{
    mResultRuntime.TermStartYear = mSimulation.Year;
    mResultRuntime.TermStartMonth = mSimulation.Month;
    mResultRuntime.TermStartDay = mSimulation.Day;
    mResultRuntime.ResultShown = false;
    mResultRuntime.PeakSupportPercent =
        (std::max)(0.0, mPolicy.PoliticalSnapshot.AverageSupportScore);

    const std::shared_ptr<CWorld> World = mSelf.lock();

    if (!World)
    {
        mResultRuntime.InitialBuildingCount = 0;
        return;
    }

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(World);
    mResultRuntime.InitialBuildingCount =
        (std::max)(0, Snapshot.TotalBuildingCount);
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
    mUIManager->CreateWidget<CEventWidget>(
        GEventWidgetName, 400);
    mUIManager->CreateWidget<CResultWidget>(
        GResultWidgetName, 450);
    mUIManager->CreateWidget<CEdictWidget>(
        GEdictWidgetName, 280);
    mUIManager->CreateWidget<CBuildMenuWidget>(
        GBuildMenuWidgetName, 300);
    mUIManager->CreateWidget<CTradeWidget>(
        GTradeWidgetName, 310);
    mUIManager->CreateWidget<CAlmanacWidget>(
        GAlmanacWidgetName, 320);
}

#ifdef _DEBUG
void CMainWorld::RunDebugConstitutionValidationIfRequested()
{
    if (!IsConstitutionValidationRequested())
        return;

    std::vector<std::string> ValidationLines;
    auto Log =
        [&](const std::string& Line)
        {
            ValidationLines.push_back(Line);
            OutputConstitutionValidationLine(Line);
        };

    auto TopHud =
        mUIManager ?
        mUIManager->FindWidget<CTopHudWidget>(GTopHudWidgetName).lock() :
        nullptr;

    if (!TopHud)
    {
        Log("FAIL top_hud_widget_not_found");
        WriteConstitutionValidationLog(ValidationLines);
        return;
    }

    std::vector<ConstitutionSystem::FDebugValidationStep> Steps;

    if (!ConstitutionSystem::BuildDebugRightOptionValidationSteps(Steps))
    {
        Log("FAIL validation_step_build_failed");
        WriteConstitutionValidationLog(ValidationLines);
        return;
    }

    const FConstitutionState OriginalConstitutionState =
        mPolicy.ConstitutionState;
    const FPoliticalWorldSnapshot OriginalPoliticalSnapshot =
        mPolicy.PoliticalSnapshot;
    FConstitutionState ValidationState;
    EBuildingEra AppliedEra = EBuildingEra::Colonial;
    bool AppliedAnyEra = false;
    bool AllStepsPassed = true;

    for (size_t Index = 0; Index < Steps.size(); ++Index)
    {
        const ConstitutionSystem::FDebugValidationStep& Step = Steps[Index];

        if (!AppliedAnyEra || AppliedEra != Step.TriggerEra)
        {
            ConstitutionSystem::OnEraTransitioned(
                ValidationState,
                Step.TriggerEra);
            AppliedEra = Step.TriggerEra;
            AppliedAnyEra = true;
        }

        mPolicy.ConstitutionState = ValidationState;
        RefreshPoliticalSnapshot();
        TopHud->Update(0.f);

        std::string StepMessage;
        const bool StepPassed =
            TopHud->DebugValidateCurrentConstitutionRightButton(
                Step,
                StepMessage);

        std::ostringstream Stream;
        Stream << (StepPassed ? "PASS" : "FAIL")
            << " step=" << (Index + 1)
            << "/" << Steps.size()
            << " " << StepMessage;
        Log(Stream.str());

        if (!StepPassed)
            AllStepsPassed = false;

        ValidationState = mPolicy.ConstitutionState;
    }

    Log(AllStepsPassed ? "SUMMARY PASS" : "SUMMARY FAIL");

    mPolicy.ConstitutionState = OriginalConstitutionState;
    mPolicy.PoliticalSnapshot = OriginalPoliticalSnapshot;
    TopHud->Update(0.f);

    WriteConstitutionValidationLog(ValidationLines);
}
#endif

