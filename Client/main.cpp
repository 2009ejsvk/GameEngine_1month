
#include "Engine.h"
#include "resource.h"
#include "World/WorldManager.h"
#include "World/MainWorld.h"
#include "World/StartWorld.h"
#include "Player/MainCamera.h"
#include "GlobalSetting.h"
#include <Windows.h>
#include <string>

#ifdef _DEBUG
namespace
{
    bool ShouldLaunchMainWorldValidation()
    {
        wchar_t ModulePath[MAX_PATH] = {};
        const DWORD Length = GetModuleFileNameW(
            nullptr,
            ModulePath,
            MAX_PATH);

        std::wstring RequestPath =
            Length > 0 ? std::wstring(ModulePath, Length) : std::wstring();
        const size_t LastSlash = RequestPath.find_last_of(L"\\/");

        if (LastSlash != std::wstring::npos)
            RequestPath.erase(LastSlash + 1);
        else
            RequestPath.clear();

        RequestPath += L"ConstitutionValidation.request";
        const DWORD Attributes = GetFileAttributesW(RequestPath.c_str());
        return Attributes != INVALID_FILE_ATTRIBUTES &&
            (Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }
}
#endif

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

    CEngine::CreateCDO<CMainCamera>();

    // 시작 월드를 지정한다.
#ifdef _DEBUG
    if (ShouldLaunchMainWorldValidation())
        CWorldManager::GetInst()->CreateWorld<CMainWorld>(false);
    else
        CWorldManager::GetInst()->CreateWorld<CStartWorld>(false);
#else
    CWorldManager::GetInst()->CreateWorld<CStartWorld>(false);
#endif

    int Ret = CEngine::GetInst()->Run();

    CEngine::DestroyInst();

    return Ret;
}
