
#include "Engine.h"
#include "resource.h"
#include "World/WorldManager.h"
#include "World/MainWorld.h"
#include "Player/Player.h"
#include "Player/Bullet.h"
#include "Player/Missile.h"
#include "Monster/Monster.h"
#include "Monster/MonsterSpawnPoint.h"
#include "Component/StateComponent.h"
#include "GlobalSetting.h"

#ifdef _DEBUG
// 라이브러리 링크를 걸어준다.
#pragma comment(lib, "GameEngine_Debug.lib")

#else

#pragma comment(lib, "GameEngine.lib")

#endif // _DEBUG


// HINSTANCE : 이 프로그램의 식별번호이다.
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    CEngine::GetInst()->CreateEngineSetting<CGlobalSetting>();

    if (!CEngine::GetInst()->Init(hInstance, TEXT("GameClient"), IDI_ICON1, IDI_ICON1,
        1280, 720, true))
    {
        CEngine::DestroyInst();
        return 0;
    }

    CEngine::CreateCDO<CPlayer>();
    CEngine::CreateCDO<CBullet>();
    CEngine::CreateCDO<CMissile>();
    CEngine::CreateCDO<CMonster>();
    CEngine::CreateCDO<CMonsterSpawnPoint>();
    CEngine::CreateCDO<CStateComponent>();

    // 시작 월드를 지정한다.
    CWorldManager::GetInst()->CreateWorld<CMainWorld>(false);

    int Ret = CEngine::GetInst()->Run();

    CEngine::DestroyInst();

    return Ret;
}
