#include "Engine.h"
#include "Timer.h"
#include "Device.h"
#include "World/WorldManager.h"
#include "Asset/AssetManager.h"
#include "Asset/Mesh/MeshManager.h"
#include "Asset/Mesh/Mesh.h"
#include "Asset/Shader/ShaderManager.h"
#include "Asset/Shader/Shader.h"
#include "Asset/Shader/CBufferTransform.h"
#include "Object.h"
#include "Component/CameraComponent.h"
#include "Component/MeshComponent.h"
#include "Object/GameObject.h"
#include "Render/RenderManager.h"
#include "CollisionInfoManager.h"

CEngine* CEngine::mInst = nullptr;
bool CEngine::mLoop = true;

CEngine::CEngine()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(151);

    mSetting.reset(new CEngineSetting);
}

CEngine::~CEngine()
{
    CMeshComponent::ClearEmptyAnimCBuffer();

    CWorldManager::DestroyInst();

    CObject::DestroyCDO();

    CRenderManager::DestroyInst();

    CCollisionInfoManager::DestroyInst();

    CAssetManager::DestroyInst();

    CDevice::DestroyInst();
}

bool CEngine::Init(HINSTANCE hInst, const TCHAR* WindowName, int IconID,
    int SmallIconID, int Width, int Height, bool WindowMode)
{
    srand((unsigned int)GetTickCount());
    rand();

    mhInst = hInst;

    // 윈도우 레지스터 클래스를 등록한다.
    InitRegisterClass(WindowName, IconID, SmallIconID);

    // 윈도우 창을 생성한다.
    Create(WindowName, Width, Height);

    // 디바이스 초기화
    if (!CDevice::GetInst()->Init(mhWnd, Width, Height, WindowMode))
        return false;

    // 렌더링 관리자 초기화
    if (!CRenderManager::GetInst()->Init())
        return false;

    // 충돌 정보 관리자 초기화
    if (!CCollisionInfoManager::GetInst()->Init())
        return false;

    // 애셋 관리자 초기화
    if (!CAssetManager::GetInst()->Init())
        return false;

    CMeshComponent::CreateEmptyAnimCBuffer();

    mSetting->Init();

    // 월드 관리자 초기화
    if (!CWorldManager::GetInst()->Init())
        return false;

    // 타이머 초기화
    CTimer::Init();

    return true;
}

int CEngine::Run()
{
    MSG msg = {};

    // 기본 메시지 루프입니다:
    // GetMessage 함수를 이용해 메세지 큐에 있는 메세지를 얻어온다.
    // 단, 메세지가 없을 경우 이 함수 내부에서 대기하게 된다.
    while (mLoop)
    {
        // GetMessage(&msg, nullptr, 0, 0)
        // PeekMessage도 윈도우 메세지 큐에서 메세지를 꺼내온다.
        // 단, 메세지가 없으면 바로 빠져나온다.
        // PM_REMOVE 는 메세지 큐에서 메세지를 제거하고 가져오라는 의미이다.
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            // 키보드 입력은 WM_KEYDOWN이 발생하는데, 문자 입력을 했다면 아래 함수에서
            // WM_CHAR 메세지를 만들어준다. 키보드 관련이 아니면 건너뛴다.
            TranslateMessage(&msg);

            // 이 메세지를 등록된 윈도우 프로시저 함수에 전달해준다.
            // WndProc 함수를 호출하는 것이다.
            DispatchMessage(&msg);
        }

        // 메세지가 없을 경우
        else
        {
            Logic();
        }
    }

    return (int)msg.wParam;
}

void CEngine::Logic()
{
    float DeltaTime = CTimer::Update();

    Update(DeltaTime);

    Render();
}

void CEngine::Update(float DeltaTime)
{
    CWorldManager::GetInst()->Update(DeltaTime);

    CWorldManager::GetInst()->PostUpdate(DeltaTime);
}

void CEngine::Render()
{
    CDevice::GetInst()->BeginRender();

    // 출력할 코드들.
    //CWorldManager::GetInst()->Render();
    CRenderManager::GetInst()->Render();

    CDevice::GetInst()->EndRender();
}

void CEngine::InitCDO()
{
    CObject::CreateCDO<CGameObject>();
    CObject::CreateCDO<CSceneComponent>();
    CObject::CreateCDO<CMeshComponent>();
    CObject::CreateCDO<CCameraComponent>();
}

void CEngine::InitRegisterClass(const TCHAR* WindowName, int IconID, int SmallIconID)
{
    // 운영체제에 등록할 창 정보를 지정하여 레지스터에 등록한다.
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    // 창의 가로, 세로 크기가 바뀔때 윈도우 전체를 다시 그리게 하는 옵션.
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    // 메세지 발생시 호출할 함수의 포인터를 등록한다.
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;

    // 이 프로그램의 인스턴스(식별번호)를 넣어준다.
    wcex.hInstance = mhInst;
    // 실행파일 아이콘
    wcex.hIcon = LoadIcon(mhInst, MAKEINTRESOURCE(IconID));
    // 마우스 커서 모양.
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    // 배경 색상.
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    // 등록할 메뉴를 지정한다.
    wcex.lpszMenuName = nullptr;// MAKEINTRESOURCEW(IDC_WINTEST);
    // 레지스터에 등록할 윈도우 클래스 이름을 지정한다.
    // TEXT : 유니코드 문자열을 만들어줄 때 사용한다.
    // L"테스트";
    wcex.lpszClassName = WindowName;
    // 창 좌측 상단의 아이콘
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(SmallIconID));

    // 생성한 구조체 정보를 이용해서 레지스터에 창 클래스를 등록한다.
    RegisterClassExW(&wcex);
}

bool CEngine::Create(const TCHAR* WindowName, int Width, int Height)
{
    // 창을 만들어준다. 만들어준 창을 컨트롤 할 수 있는 핸들도 함께 반환된다.
    // 1번 : 윈도우 클래스 이름
    // 2번 : 윈도우 타이틀바에 출력할 이름
    // 3번 : 윈도우 창 스타일을 지정한다.
    // 4번 : 윈도우 창 시작 X좌표를 지정한다.
    // 5번 : 윈도우 창 시작 Y좌표를 지정한다.
    // 6번 : 윈도우 창 가로 크기
    // 7번 : 윈도우 창 세로 크기
    // 8번 : 부모 윈도우 핸들
    // 9번 : 메뉴 핸들
    // 10번 : 식별번호
    mhWnd = CreateWindowW(WindowName, WindowName, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, Width, Height, nullptr, nullptr, mhInst, nullptr);

    if (!mhWnd)
    {
        return false;
    }

    // 클라이언트 영역의 해상도를 RECT로 만든다.
    RECT    WindowRC = { 0, 0, Width, Height };

    // 클라이언트 영역이 해상도 크기로 생성되기 위해 ThickFrame이나 기타 요소들을 포함한 
    // 실제 크기를 얻어온다.
    AdjustWindowRect(&WindowRC, WS_OVERLAPPEDWINDOW, FALSE);

    // 윈도우 크기를 변경한다.
    SetWindowPos(mhWnd, HWND_TOPMOST, 0, 0, WindowRC.right - WindowRC.left,
        WindowRC.bottom - WindowRC.top, SWP_NOMOVE | SWP_NOZORDER);

    // 창을 보여줄지를 결정한다.
    ShowWindow(mhWnd, SW_SHOW);

    // 이 윈도우를 즉시 다시 그리게 만들어준다.
    // WM_PAINT 메세지를 발생시켜준다.
    UpdateWindow(mhWnd);

    return true;
}

LRESULT CEngine::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
        EndPaint(hWnd, &ps);
    }
    break;
    // 창이 파괴될 때 들어온다.
    case WM_DESTROY:
        mLoop = false;
        PostQuitMessage(0);
        break;
    default:
        // 위에서 지정한 메세지 외 다른 메세지가 발생할 경우 윈도우의 기본 동작으로
        // 처리하게 만들어준다.
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
