#include "LoadingWorld.h"
#include "../UI/UILayoutConfig.h"
#include "../UI/LoadingWidget.h"
#include "World/WorldUIManager.h"
#include "ThreadManager.h"
#include "../Thread/LoadingThread.h"
#include "World/WorldManager.h"
#include "Render/RenderManager.h"

CLoadingWorld::CLoadingWorld()
{
}

CLoadingWorld::~CLoadingWorld()
{
	CThreadManager::GetInst()->DeleteThread("LoadingThread");
}

bool CLoadingWorld::Init()
{
	CWorld::Init();

	LoadAnimation2D();

	LoadSound();

	CreateUI();

	return true;
}

void CLoadingWorld::Update(float DeltaTime)
{
	CWorld::Update(DeltaTime);

	if (mThread->GetComplete())
	{
		CWorldManager::GetInst()->CompleteAsyncWorld();

		if (mLoadType == EWorldType::Editor)
		{
			CRenderManager::GetInst()->SetDebugTarget(false);
		}
	}
}

void CLoadingWorld::OnUiManagerUpdated()
{
    UIConfig::ApplyWidgetOverrides(GetUIManager().lock());
}

void CLoadingWorld::Load(EWorldType WorldType)
{
	mLoadType = WorldType;

	// 로딩 스레드를 생성한다.
	mThread =
		CThreadManager::GetInst()->Create<CLoadingThread>("LoadingThread", true);

	// 어떤 월드를 로딩할지를 지정한다.
	mThread->SetWorldType(WorldType);

	// 스레드를 시작한다.
	mThread->Resume();
}

void CLoadingWorld::LoadAnimation2D()
{
}

void CLoadingWorld::LoadSound()
{
	mWorldAssetManager->LoadSound("MainBGM", "BGM", true,
		"MainBgm.mp3");

	mWorldAssetManager->SoundPlay("MainBGM");
}

void CLoadingWorld::CreateUI()
{
	std::weak_ptr<CLoadingWidget>	Widget =
		mUIManager->CreateWidget<CLoadingWidget>("LoadingWidget");
}


