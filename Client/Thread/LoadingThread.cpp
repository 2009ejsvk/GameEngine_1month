#include "LoadingThread.h"
#include "World/WorldManager.h"
#include "../World/MainWorld.h"
#include "../World/StartWorld.h"
#include "../World/EditorWorld.h"

CLoadingThread::CLoadingThread()
{
}

CLoadingThread::~CLoadingThread()
{
}

void CLoadingThread::Exit()
{
	CThreadBase::Exit();
}

void CLoadingThread::Run()
{
	switch (mWorldType)
	{
	case EWorldType::Start:
		CWorldManager::GetInst()->CreateAsyncWorld<CStartWorld>();
		break;
	case EWorldType::Editor:
		CWorldManager::GetInst()->CreateAsyncWorld<CEditorWorld>();
		break;
	case EWorldType::Main:
		CWorldManager::GetInst()->CreateAsyncWorld<CMainWorld>();
		break;
	}

	/*mLoop = true;

	while (mLoop)
	{
	}*/
}
