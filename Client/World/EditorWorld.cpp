#include "EditorWorld.h"
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

	LoadAnimation2D();

	LoadSound();

	CreateUI();

	mPlayer = CreateGameObject<CEditorPlayer>("Player");

	mTileMap = CreateGameObject<CTileMapMain>("TileMap");

	auto	TileMapObj = mTileMap.lock();

	auto	TileMap = TileMapObj->GetTileMap().lock();

	TileMap->SetTileOutLineRender(true);

	return true;
}

void CEditorWorld::Update(float DeltaTime)
{
	CWorld::Update(DeltaTime);
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


