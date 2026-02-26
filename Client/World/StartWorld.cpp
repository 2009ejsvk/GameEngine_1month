#include "StartWorld.h"
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

	LoadAnimation2D();

	LoadSound();

	CreateUI();

	return true;
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

