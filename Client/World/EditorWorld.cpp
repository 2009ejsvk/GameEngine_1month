#include "EditorWorld.h"
#include "../ObjectNames.h"
#include "../UI/UILayoutApplier.h"
#include "../UI/TropicoUiAssetCatalog.h"
#include "../UI/TropicoUiTheme.h"
#include "World/WorldUIManager.h"
#include "World/WorldManager.h"
#include "../Map/TileMapMain.h"
#include "../Player/EditorPlayer.h"
#include "../UI/EditorWidget.h"

CEditorWorld::CEditorWorld()
{
}

CEditorWorld::~CEditorWorld()
{
}

bool CEditorWorld::Init()
{
    CWorld::Init();
    TropicoUiAssets::RegisterRuntimeConfig();
    TropicoUiTheme::RegisterRuntimeConfig();

	LoadAnimation2D();

	LoadSound();

	CreateUI();

	mPlayer = CreateGameObject<CEditorPlayer>("Player");

	mTileMap = CreateGameObject<CTileMapMain>(GTileMapObjectName);

	auto	TileMapObj = mTileMap.lock();

	auto	TileMap = TileMapObj->GetTileMap().lock();

	TileMap->SetTileOutLineRender(true);

	return true;
}

void CEditorWorld::Update(float DeltaTime)
{
    TropicoUiAssets::ReloadIfChanged(DeltaTime);
    TropicoUiTheme::ReloadIfChanged(DeltaTime);
	CWorld::Update(DeltaTime);
}

void CEditorWorld::OnUiManagerUpdated()
{
    UILayoutApplier::ApplyWidgetOverrides(GetUIManager().lock());
}

void CEditorWorld::LoadAnimation2D()
{
}

void CEditorWorld::LoadSound()
{
	/*mWorldAssetManager->LoadSound("MainBGM", "BGM", true,
		"MainBgm.mp3");

	mWorldAssetManager->SoundPlay("MainBGM");*/
}

void CEditorWorld::CreateUI()
{
	std::weak_ptr<CEditorWidget>	Widget =
		mUIManager->CreateWidget<CEditorWidget>("EditorWidget");
}


