#include "StartWorld.h"
#include "../UI/UILayoutApplier.h"
#include "../UI/UILayoutLoader.h"
#include "../UI/TropicoUiAssetCatalog.h"
#include "../UI/TropicoUiTheme.h"
#include "../UI/StartWidget.h"
#include "World/WorldUIManager.h"

CStartWorld::CStartWorld()
{
}

CStartWorld::~CStartWorld()
{
}

bool CStartWorld::Init()
{
	CWorld::Init();
    UILayoutLoader::RegisterRuntimeConfig();
    TropicoUiAssets::RegisterRuntimeConfig();
    TropicoUiTheme::RegisterRuntimeConfig();

	LoadAnimation2D();

	LoadSound();

	CreateUI();

	return true;
}

void CStartWorld::Update(float DeltaTime)
{
    UILayoutLoader::ReloadIfChanged(DeltaTime);
    TropicoUiAssets::ReloadIfChanged(DeltaTime);
    TropicoUiTheme::ReloadIfChanged(DeltaTime);
    CWorld::Update(DeltaTime);
}

void CStartWorld::OnUiManagerUpdated()
{
    UILayoutApplier::ApplyWidgetOverrides(GetUIManager().lock());
}

void CStartWorld::LoadAnimation2D()
{
}

void CStartWorld::LoadSound()
{
	mWorldAssetManager->LoadSound("MainBGM", "BGM", true,
		"MainBgm.mp3");

	mWorldAssetManager->SoundPlay("MainBGM");
}

void CStartWorld::CreateUI()
{
	std::weak_ptr<CStartWidget>	MainWidget =
		mUIManager->CreateWidget<CStartWidget>("StartWidget");
}

