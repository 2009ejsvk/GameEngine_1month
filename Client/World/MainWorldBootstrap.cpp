#include "MainWorld.h"
#include "BusRouteSystem.h"
#include "../GlobalSetting.h"
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
#include "../UI/TaskWidget.h"
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
    mInfrastructure->RebuildRoadNetwork();
}

void CMainWorld::ResetWorldState()
{
    mPopulation->Reset();
    mEconomy->Reset();
    mSimulation->Reset();
    mScenario->Reset();
    mPolitics->Reset();
    mEraState->Reset();
    mEdictState->Reset();
    mCrisis->Reset();
    mKnowledgeState->Reset();
    mTrade->Reset();
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

    mPopulation->LoadCitizenAnimation2D();
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
    mPolitics->InitializeElectionSchedule(
        mEraState->EraProgress.CurrentEra,
        mSimulation->Year);

    for (int i = 0; i < MainWorldConfig::GInitialNpcCount; ++i)
    {
        mPopulation->SpawnCitizenOrb();
    }

    mInfrastructure->RefreshPowerGridCoverage();
    mPopulation->ReassignCitizenNeeds();
    mInfrastructure->RefreshBuildingPollutionExposure();
    mEraState->RefreshEraProgress();
    mPolitics->RefreshSnapshot(
        {
            &mEconomy->TaxEventStatus,
            &mKnowledgeState->ConstitutionState.ActiveEffects
        });
    mEdictState->RefreshEdictModifiers();
    mTrade->OnDayAdvanced(
        {
            mPolitics->GovernmentProfile,
            mEconomy->TaxEventStatus,
            mEdictState->GovernmentEdicts,
            mEraState->EraProgress.CurrentEra,
            mSimulation->Year,
            mSimulation->Month,
            mSimulation->Day,
            false
        });
    mEconomy->RefreshWorldMarketPrices(
        {
            mPolitics->GovernmentProfile,
            mEdictState->GovernmentEdicts,
            mCrisis->WorldCrisisService->GetStatus(),
            mTrade->State.ForeignPowerStates,
            mSimulation->Year,
            mSimulation->Month,
            mSimulation->Day
        });
    mScenario->InitializeResultTracking();

    if (GameSession::CurrentMode() == EGameMode::Scenario)
    {
        // 즉시 SmugglersOffer로 진입 — 수요 주입 + TaskWidget 열기까지 완료
        mScenario->TickPhase();
    }

    return true;
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
    mUIManager->CreateWidget<CTaskWidget>(
        GTaskWidgetName, 305);
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
        mKnowledgeState->ConstitutionState;
    const FPoliticalWorldSnapshot OriginalPoliticalSnapshot =
        mPolitics->PoliticalSnapshot;
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

        mKnowledgeState->ConstitutionState = ValidationState;
        mPolitics->RefreshSnapshot(
            {
                &mEconomy->TaxEventStatus,
                &mKnowledgeState->ConstitutionState.ActiveEffects
            });
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

        ValidationState = mKnowledgeState->ConstitutionState;
    }

    Log(AllStepsPassed ? "SUMMARY PASS" : "SUMMARY FAIL");

    mKnowledgeState->ConstitutionState = OriginalConstitutionState;
    mPolitics->PoliticalSnapshot = OriginalPoliticalSnapshot;
    TopHud->Update(0.f);

    WriteConstitutionValidationLog(ValidationLines);
}
#endif

