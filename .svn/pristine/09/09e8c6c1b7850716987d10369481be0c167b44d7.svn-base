#include "Input.h"
#include "../Engine.h"
#include "../Device.h"

CInput::CInput()
{
}

CInput::~CInput()
{
    auto    iter = mKeyStateMap.begin();
    auto    iterEnd = mKeyStateMap.end();

    for (; iter != iterEnd; ++iter)
    {
        SAFE_DELETE(iter->second);
    }
}

bool CInput::Init()
{
    mhInst = CEngine::GetInst()->GetWindowInstance();
    mhWnd = CEngine::GetInst()->GetWindowHandle();

    // DirectInput 객체 생성
    if (FAILED(DirectInput8Create(mhInst, DIRECTINPUT_VERSION,
        IID_IDirectInput8, (void**)&mInput, nullptr)))
        mInputType = EInputSystemType::Window;

    else
    {
        if (!InitDevice())
            return false;
    }

    return true;
}

void CInput::Update(float DeltaTime)
{
    if (mInputType == EInputSystemType::DInput)
    {
        UpdateKeyboard();
        UpdateMouse();
    }

	// 입력 상태를 갱신한다.
	UpdateInput(DeltaTime);

	// 키에 따른 함수 호출을 진행한다.
	UpdateBindKey(DeltaTime);
}

bool CInput::InitDevice()
{
    // 키보드 장치 생성
    if (FAILED(mInput->CreateDevice(GUID_SysKeyboard,
        &mKeyboard, nullptr)))
        return false;

    // 키보드 데이터 형식 설정
    mKeyboard->SetDataFormat(&c_dfDIKeyboard);

    // 창모드인지 풀스크린 모드인지 체크.
    if (CDevice::GetInst()->GetWindowMode())
    {
        // 입력장치를 윈도우와 어떤 규칙으로 공유 혹은 독점해서
        // 키를 받을지 정하는 함수이다.
        /*
        DISCL_EXCLUSIVE : 입력을 독점으로 한다.
        DISCL_NONEXCLUSIVE : 다른 앱과 입력을 공유한다.
        DISCL_FOREGROUND : 내 윈도우가 활성화(포커스)일 때만 입력
        을 받는다.
        DISCL_BACKGROUND : 내 윈도우가 비활성이어도 입력을 받는
        다.
        DISCL_NOWINKEY : 윈도우 키 같은 시스템 키를 막아준다.
        */
        mKeyboard->SetCooperativeLevel(mhWnd,
            DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    }

    else
    {
        mKeyboard->SetCooperativeLevel(mhWnd,
            DISCL_EXCLUSIVE | DISCL_FOREGROUND);
    }

    // 입력 장치를 실제로 읽을 수 있게 해준다.
    // DIERR_INPUTLOST : 포커스를 잃었거나 장치가 잠깐 풀릴 때
    // 다시 Acquire 함수로 얻어온다.
    // DIERR_NOTACQUIRED : 아직 획득이 안된 상태. Acquire 호출로
    // 해결
    if (FAILED(mKeyboard->Acquire()))
        return false;




    // 마우스 장치 생성
    if (FAILED(mInput->CreateDevice(GUID_SysMouse,
        &mMouse, nullptr)))
        return false;

    // 마우스 데이터 형식 설정
    mMouse->SetDataFormat(&c_dfDIMouse);

    // 창모드인지 풀스크린 모드인지 체크.
    if (CDevice::GetInst()->GetWindowMode())
    {
        // 입력장치를 윈도우와 어떤 규칙으로 공유 혹은 독점해서
        // 키를 받을지 정하는 함수이다.
        /*
        DISCL_EXCLUSIVE : 입력을 독점으로 한다.
        DISCL_NONEXCLUSIVE : 다른 앱과 입력을 공유한다.
        DISCL_FOREGROUND : 내 윈도우가 활성화(포커스)일 때만 입력
        을 받는다.
        DISCL_BACKGROUND : 내 윈도우가 비활성이어도 입력을 받는
        다.
        DISCL_NOWINKEY : 윈도우 키 같은 시스템 키를 막아준다.
        */
        mMouse->SetCooperativeLevel(mhWnd,
            DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    }

    else
    {
        mMouse->SetCooperativeLevel(mhWnd,
            DISCL_EXCLUSIVE | DISCL_FOREGROUND);
    }

    if (FAILED(mMouse->Acquire()))
        return false;

    return true;
}

void CInput::UpdateKeyboard()
{
    if (!mKeyboard)
        return;

    HRESULT Result = mKeyboard->GetDeviceState(256,
        (LPVOID)&mKeyState);

    if (FAILED(Result))
    {
        // 입력 장치를 실제로 읽을 수 있게 해준다.
        // DIERR_INPUTLOST : 포커스를 잃었거나 장치가 잠깐 
        // 풀릴 때
        // 다시 Acquire 함수로 얻어온다.
        // DIERR_NOTACQUIRED : 아직 획득이 안된 상태. Acquire 
        // 호출로 해결
        if (Result == DIERR_INPUTLOST ||
            Result == DIERR_NOTACQUIRED)
        {
            if (SUCCEEDED(mKeyboard->Acquire()))
            {
                mKeyboard->GetDeviceState(256,
                    (LPVOID)&mKeyState);
            }
        }
    }
}

void CInput::UpdateMouse()
{
    if (!mMouse)
        return;

    HRESULT Result = mMouse->GetDeviceState(
        sizeof(mMouseState), (LPVOID)&mMouseState);

    if (FAILED(Result))
    {
        // 입력 장치를 실제로 읽을 수 있게 해준다.
        // DIERR_INPUTLOST : 포커스를 잃었거나 장치가 잠깐 
        // 풀릴 때
        // 다시 Acquire 함수로 얻어온다.
        // DIERR_NOTACQUIRED : 아직 획득이 안된 상태. Acquire 
        // 호출로 해결
        if (Result == DIERR_INPUTLOST ||
            Result == DIERR_NOTACQUIRED)
        {
            if (SUCCEEDED(mMouse->Acquire()))
            {
                mMouse->GetDeviceState(
                    sizeof(mMouseState), 
                    (LPVOID)&mMouseState);
            }
        }
    }
}

void CInput::UpdateInput(float DeltaTime)
{
	switch (mInputType)
	{
	case EInputSystemType::DInput:
		UpdateDInput(DeltaTime);
		break;
	case EInputSystemType::Window:
		UpdateWindowInput(DeltaTime);
		break;
	}
}

void CInput::UpdateDInput(float DeltaTime)
{
	// 왼쪽 컨트롤키가 눌러졌는지 판단한다.
	if (mKeyState[DIK_LCONTROL] & 0x80)
	{
		mCtrlState[EInputType::Release] = false;

		// 이제 막 누르기 시작한 경우
		if (!mCtrlState[EInputType::Press] &&
			!mCtrlState[EInputType::Hold])
		{
			mCtrlState[EInputType::Press] = true;
			mCtrlState[EInputType::Hold] = true;
		}

		// 이전 프레임에도 눌러져있고 현재도 누르고 있는 경우
		else
			mCtrlState[EInputType::Press] = false;
	}

	// 왼쪽 컨트롤을 누르지 않고 이전에는 눌러져 있던 경우
	// 지금 막 키를 해제한 경우이다.
	else if (mCtrlState[EInputType::Hold])
	{
		mCtrlState[EInputType::Press] = false;
		mCtrlState[EInputType::Hold] = false;
		mCtrlState[EInputType::Release] = true;
	}

	else if (mCtrlState[EInputType::Release])
	{
		mCtrlState[EInputType::Release] = false;
	}

	// 왼쪽 알트키가 눌러졌는지 판단한다.
	if (mKeyState[DIK_LALT] & 0x80)
	{
		mAltState[EInputType::Release] = false;

		// 이제 막 누르기 시작한 경우
		if (!mAltState[EInputType::Press] &&
			!mAltState[EInputType::Hold])
		{
			mAltState[EInputType::Press] = true;
			mAltState[EInputType::Hold] = true;
		}

		// 이전 프레임에도 눌러져있고 현재도 누르고 있는 경우
		else
			mAltState[EInputType::Press] = false;
	}

	// 왼쪽 알트를 누르지 않고 이전에는 눌러져 있던 경우
	// 지금 막 키를 해제한 경우이다.
	else if (mAltState[EInputType::Hold])
	{
		mAltState[EInputType::Press] = false;
		mAltState[EInputType::Hold] = false;
		mAltState[EInputType::Release] = true;
	}

	else if (mAltState[EInputType::Release])
	{
		mAltState[EInputType::Release] = false;
	}

	// 왼쪽 쉬프트키가 눌러졌는지 판단한다.
	if (mKeyState[DIK_LSHIFT] & 0x80)
	{
		mShiftState[EInputType::Release] = false;

		// 이제 막 누르기 시작한 경우
		if (!mShiftState[EInputType::Press] &&
			!mShiftState[EInputType::Hold])
		{
			mShiftState[EInputType::Press] = true;
			mShiftState[EInputType::Hold] = true;
		}

		// 이전 프레임에도 눌러져있고 현재도 누르고 있는 경우
		else
			mShiftState[EInputType::Press] = false;
	}

	// 왼쪽 쉬프트를 누르지 않고 이전에는 눌러져 있던 경우
	// 지금 막 키를 해제한 경우이다.
	else if (mShiftState[EInputType::Hold])
	{
		mShiftState[EInputType::Press] = false;
		mShiftState[EInputType::Hold] = false;
		mShiftState[EInputType::Release] = true;
	}

	else if (mShiftState[EInputType::Release])
	{
		mShiftState[EInputType::Release] = false;
	}

	for (int i = 0; i < EMouseType::End; ++i)
	{
		if (mMouseState.rgbButtons[i] & 0x80)
		{
			mMouseButton[i][EInputType::Release] = false;

			// 이제 막 누르기 시작한 경우
			if (!mMouseButton[i][EInputType::Press] &&
				!mMouseButton[i][EInputType::Hold])
			{
				mMouseButton[i][EInputType::Press] = true;
				mMouseButton[i][EInputType::Hold] = true;
			}

			// 이전 프레임에도 눌러져있고 현재도 누르고 있는 경우
			else
				mMouseButton[i][EInputType::Press] = false;
		}

		else if (mMouseButton[i][EInputType::Hold])
		{
			mMouseButton[i][EInputType::Press] = false;
			mMouseButton[i][EInputType::Hold] = false;
			mMouseButton[i][EInputType::Release] = true;
		}

		else if (mMouseButton[i][EInputType::Release])
		{
			mMouseButton[i][EInputType::Release] = false;
		}
	}

	auto	iter = mKeyStateMap.begin();
	auto	iterEnd = mKeyStateMap.end();

	for (; iter != iterEnd; ++iter)
	{
		switch (iter->second->Key)
		{
		case DIK_MOUSELBUTTON:
			iter->second->Press = mMouseButton[EMouseType::LButton][EInputType::Press];
			iter->second->Hold = mMouseButton[EMouseType::LButton][EInputType::Hold];
			iter->second->Release = mMouseButton[EMouseType::LButton][EInputType::Release];
			break;
		case DIK_MOUSERBUTTON:
			iter->second->Press = mMouseButton[EMouseType::RButton][EInputType::Press];
			iter->second->Hold = mMouseButton[EMouseType::RButton][EInputType::Hold];
			iter->second->Release = mMouseButton[EMouseType::RButton][EInputType::Release];
			break;
		case DIK_MOUSEWHEELBUTTON:
			iter->second->Press = mMouseButton[EMouseType::MButton][EInputType::Press];
			iter->second->Hold = mMouseButton[EMouseType::MButton][EInputType::Hold];
			iter->second->Release = mMouseButton[EMouseType::MButton][EInputType::Release];
			break;
		default:
			if (mKeyState[iter->second->Key] & 0x80)
			{
				iter->second->Release = false;

				// 이제 막 누르기 시작한 경우
				if (!iter->second->Press &&
					!iter->second->Hold)
				{
					iter->second->Press = true;
					iter->second->Hold = true;
				}

				// 이전 프레임에도 눌러져있고 현재도 누르고 있는 경우
				else
					iter->second->Press = false;
			}

			else if (iter->second->Hold)
			{
				iter->second->Press = false;
				iter->second->Hold = false;
				iter->second->Release = true;
			}

			else if (iter->second->Release)
			{
				iter->second->Release = false;
			}
			break;
		}
	}
}

void CInput::UpdateWindowInput(float DeltaTime)
{
	// 왼쪽 컨트롤키가 눌러졌는지 판단한다.
	if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
	{
		mCtrlState[EInputType::Release] = false;

		// 이제 막 누르기 시작한 경우
		if (!mCtrlState[EInputType::Press] &&
			!mCtrlState[EInputType::Hold])
		{
			mCtrlState[EInputType::Press] = true;
			mCtrlState[EInputType::Hold] = true;
		}

		// 이전 프레임에도 눌러져있고 현재도 누르고 있는 경우
		else
			mCtrlState[EInputType::Press] = false;
	}

	// 왼쪽 컨트롤을 누르지 않고 이전에는 눌러져 있던 경우
	// 지금 막 키를 해제한 경우이다.
	else if (mCtrlState[EInputType::Hold])
	{
		mCtrlState[EInputType::Press] = false;
		mCtrlState[EInputType::Hold] = false;
		mCtrlState[EInputType::Release] = true;
	}

	else if (mCtrlState[EInputType::Release])
	{
		mCtrlState[EInputType::Release] = false;
	}

	// 왼쪽 알트키가 눌러졌는지 판단한다.
	if (GetAsyncKeyState(VK_LMENU) & 0x8000)
	{
		mAltState[EInputType::Release] = false;

		// 이제 막 누르기 시작한 경우
		if (!mAltState[EInputType::Press] &&
			!mAltState[EInputType::Hold])
		{
			mAltState[EInputType::Press] = true;
			mAltState[EInputType::Hold] = true;
		}

		// 이전 프레임에도 눌러져있고 현재도 누르고 있는 경우
		else
			mAltState[EInputType::Press] = false;
	}

	// 왼쪽 알트를 누르지 않고 이전에는 눌러져 있던 경우
	// 지금 막 키를 해제한 경우이다.
	else if (mAltState[EInputType::Hold])
	{
		mAltState[EInputType::Press] = false;
		mAltState[EInputType::Hold] = false;
		mAltState[EInputType::Release] = true;
	}

	else if (mAltState[EInputType::Release])
	{
		mAltState[EInputType::Release] = false;
	}

	// 왼쪽 쉬프트키가 눌러졌는지 판단한다.
	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
	{
		mShiftState[EInputType::Release] = false;

		// 이제 막 누르기 시작한 경우
		if (!mShiftState[EInputType::Press] &&
			!mShiftState[EInputType::Hold])
		{
			mShiftState[EInputType::Press] = true;
			mShiftState[EInputType::Hold] = true;
		}

		// 이전 프레임에도 눌러져있고 현재도 누르고 있는 경우
		else
			mShiftState[EInputType::Press] = false;
	}

	// 왼쪽 쉬프트를 누르지 않고 이전에는 눌러져 있던 경우
	// 지금 막 키를 해제한 경우이다.
	else if (mShiftState[EInputType::Hold])
	{
		mShiftState[EInputType::Press] = false;
		mShiftState[EInputType::Hold] = false;
		mShiftState[EInputType::Release] = true;
	}

	else if (mShiftState[EInputType::Release])
	{
		mShiftState[EInputType::Release] = false;
	}

	int	MouseKey[EMouseType::End] =
	{
		VK_LBUTTON,
		VK_RBUTTON,
		VK_MBUTTON
	};

	for (int i = 0; i < EMouseType::End; ++i)
	{
		if (GetAsyncKeyState(MouseKey[i]) & 0x8000)
		{
			mMouseButton[i][EInputType::Release] = false;

			// 이제 막 누르기 시작한 경우
			if (!mMouseButton[i][EInputType::Press] &&
				!mMouseButton[i][EInputType::Hold])
			{
				mMouseButton[i][EInputType::Press] = true;
				mMouseButton[i][EInputType::Hold] = true;
			}

			// 이전 프레임에도 눌러져있고 현재도 누르고 있는 경우
			else
				mMouseButton[i][EInputType::Press] = false;
		}

		else if (mMouseButton[i][EInputType::Hold])
		{
			mMouseButton[i][EInputType::Press] = false;
			mMouseButton[i][EInputType::Hold] = false;
			mMouseButton[i][EInputType::Release] = true;
		}

		else if (mMouseButton[i][EInputType::Release])
		{
			mMouseButton[i][EInputType::Release] = false;
		}
	}

	auto	iter = mKeyStateMap.begin();
	auto	iterEnd = mKeyStateMap.end();

	for (; iter != iterEnd; ++iter)
	{
		if (GetAsyncKeyState(iter->second->Key) & 0x8000)
		{
			iter->second->Release = false;

			// 이제 막 누르기 시작한 경우
			if (!iter->second->Press &&
				!iter->second->Hold)
			{
				iter->second->Press = true;
				iter->second->Hold = true;
			}

			// 이전 프레임에도 눌러져있고 현재도 누르고 있는 경우
			else
				iter->second->Press = false;
		}

		else if (iter->second->Hold)
		{
			iter->second->Press = false;
			iter->second->Hold = false;
			iter->second->Release = true;
		}

		else if (iter->second->Release)
		{
			iter->second->Release = false;
		}
	}
}

void CInput::UpdateBindKey(float DeltaTime)
{
	auto	iter = mBindKeyMap.begin();
	auto	iterEnd = mBindKeyMap.end();

	for (; iter != iterEnd; ++iter)
	{
		// Key : 1, Ctrl : true shift : false alt : falst
		// Ctrl == CtrlState 와 비교하면 Ctrl 키가 눌러졌을 경우
		// CtrlState는 true이므로 Ctrl키가 눌러졌을때만 통과할 수
		// 있다.
		if (iter->second->Key->Press &&
			iter->second->Ctrl == mCtrlState[EInputType::Hold] &&
			iter->second->Shift == mShiftState[EInputType::Hold] &&
			iter->second->Alt == mAltState[EInputType::Hold])
		{
			if (iter->second->Func[EInputType::Press])
				iter->second->Func[EInputType::Press]();
		}

		if (iter->second->Key->Hold &&
			iter->second->Ctrl == mCtrlState[EInputType::Hold] &&
			iter->second->Shift == mShiftState[EInputType::Hold] &&
			iter->second->Alt == mAltState[EInputType::Hold])
		{
			iter->second->KeyHold = true;

			if (iter->second->Func[EInputType::Hold])
				iter->second->Func[EInputType::Hold]();
		}

		// Release 상태를 만드는 경우에는
		// Ctrl + 1 키일 경우 Ctrl + 1을 누르고 있다가 둘중
		// 하나라도 떼는 경우 발생한다.
		// Ctrl + 1 을 누르고 있다가 Shift나 Alt를 누르는 경우
		// 발생한다.
		bool	Verification = false;

		// Ctrl키를 누른 상태의 조합키일 경우
		if (iter->second->Ctrl)
		{
			if (mCtrlState[EInputType::Release])
				Verification = true;
		}

		else
		{
			if (mCtrlState[EInputType::Hold])
				Verification = true;
		}

		if (iter->second->Alt)
		{
			if (mAltState[EInputType::Release])
				Verification = true;
		}

		else
		{
			if (mAltState[EInputType::Hold])
				Verification = true;
		}

		if (iter->second->Shift)
		{
			if (mShiftState[EInputType::Release])
				Verification = true;
		}

		else
		{
			if (mShiftState[EInputType::Hold])
				Verification = true;
		}

		if ((iter->second->Key->Release || Verification) &&
			iter->second->KeyHold)
		{
			iter->second->KeyHold = false;

			if (iter->second->Func[EInputType::Release])
				iter->second->Func[EInputType::Release]();
		}
	}
}

void CInput::AddBindKey(const std::string& Name, 
    unsigned char Key)
{
    auto    iter = mBindKeyMap.find(Name);

    if (iter != mBindKeyMap.end())
        return;

    std::unique_ptr<FBindKey>   NewKey =
        std::make_unique<FBindKey>();

    FKeyState* State = FindKeyState(Key);

    // 키가 없을 경우 새로 생성해서 등록한다.
    if (!State)
    {
        State = new FKeyState;

        State->Key = ConvertKey(Key);

		mKeyStateMap.insert(std::make_pair(Key, State));
    }

	NewKey->Key = State;

	// std::move로 소유권을 map으로 이동시킨다.
	mBindKeyMap.insert(std::make_pair(Name, 
		std::move(NewKey)));
}

void CInput::SetKeyCtrl(const std::string& Name, bool Ctrl)
{
	FBindKey*	Key = FindBindKey(Name);

	if (!Key)
		return;

	Key->Ctrl = Ctrl;
}

void CInput::SetKeyAlt(const std::string& Name, bool Alt)
{
	FBindKey* Key = FindBindKey(Name);

	if (!Key)
		return;

	Key->Alt = Alt;
}

void CInput::SetKeyShift(const std::string& Name, bool Shift)
{
	FBindKey* Key = FindBindKey(Name);

	if (!Key)
		return;

	Key->Shift = Shift;
}

FKeyState* CInput::FindKeyState(unsigned char Key)
{
    auto    iter = mKeyStateMap.find(Key);

    if (iter == mKeyStateMap.end())
        return nullptr;

    return iter->second;
}

FBindKey* CInput::FindBindKey(const std::string& Name)
{
    auto    iter = mBindKeyMap.find(Name);

    if (iter == mBindKeyMap.end())
        return nullptr;

    return iter->second.get();
}

unsigned char CInput::ConvertKey(unsigned char Key)
{
    if (mInputType == EInputSystemType::DInput)
    {
		switch (Key)
		{
		case VK_LBUTTON:
			return DIK_MOUSELBUTTON;
		case VK_RBUTTON:
			return DIK_MOUSERBUTTON;
		case VK_MBUTTON:
			return DIK_MOUSEWHEELBUTTON;
		case VK_BACK:
			return DIK_BACK;
		case VK_TAB:
			return DIK_TAB;
		case VK_RETURN:
			return DIK_RETURN;
		case VK_LCONTROL:
			return DIK_LCONTROL;
		case VK_RCONTROL:
			return DIK_RCONTROL;
		case VK_LMENU:
			return DIK_LALT;
		case VK_RMENU:
			return DIK_RALT;
		case VK_LSHIFT:
			return DIK_LSHIFT;
		case VK_RSHIFT:
			return DIK_RSHIFT;
		case VK_PAUSE:
			return DIK_PAUSE;
		case VK_CAPITAL:
			return DIK_CAPSLOCK;
		case VK_ESCAPE:
			return DIK_ESCAPE;
		case VK_SPACE:
			return DIK_SPACE;
		case VK_NEXT:
			return DIK_PGDN;
		case VK_PRIOR:
			return DIK_PGUP;
		case VK_END:
			return DIK_END;
		case VK_HOME:
			return DIK_HOME;
		case VK_LEFT:
			return DIK_LEFT;
		case VK_UP:
			return DIK_UP;
		case VK_RIGHT:
			return DIK_RIGHT;
		case VK_DOWN:
			return DIK_DOWN;
		case VK_PRINT:
			return DIK_SYSRQ;
		case VK_INSERT:
			return DIK_INSERT;
		case VK_DELETE:
			return DIK_DELETE;
		case VK_HELP:
			return 0;
		case '0':
			return DIK_0;
		case '1':
			return DIK_1;
		case '2':
			return DIK_2;
		case '3':
			return DIK_3;
		case '4':
			return DIK_4;
		case '5':
			return DIK_5;
		case '6':
			return DIK_6;
		case '7':
			return DIK_7;
		case '8':
			return DIK_8;
		case '9':
			return DIK_9;
		case 'A':
			return DIK_A;
		case 'B':
			return DIK_B;
		case 'C':
			return DIK_C;
		case 'D':
			return DIK_D;
		case 'E':
			return DIK_E;
		case 'F':
			return DIK_F;
		case 'G':
			return DIK_G;
		case 'H':
			return DIK_H;
		case 'I':
			return DIK_I;
		case 'J':
			return DIK_J;
		case 'K':
			return DIK_K;
		case 'L':
			return DIK_L;
		case 'M':
			return DIK_M;
		case 'N':
			return DIK_N;
		case 'O':
			return DIK_O;
		case 'P':
			return DIK_P;
		case 'Q':
			return DIK_Q;
		case 'R':
			return DIK_R;
		case 'S':
			return DIK_S;
		case 'T':
			return DIK_T;
		case 'U':
			return DIK_U;
		case 'V':
			return DIK_V;
		case 'W':
			return DIK_W;
		case 'X':
			return DIK_X;
		case 'Y':
			return DIK_Y;
		case 'Z':
			return DIK_Z;
		case VK_OEM_3:
			return DIK_GRAVE;
		case VK_OEM_MINUS:
			return DIK_MINUS;
		case VK_OEM_NEC_EQUAL:
			return DIK_EQUALS;
		case VK_OEM_4:
			return DIK_LBRACKET;
		case VK_OEM_6:
			return DIK_RBRACKET;
		case VK_OEM_5:
			return DIK_BACKSLASH;
		case VK_OEM_1:
			return DIK_SEMICOLON;
		case VK_OEM_7:
			return DIK_APOSTROPHE;
		case VK_OEM_COMMA:
			return DIK_COMMA;
		case VK_OEM_PERIOD:
			return DIK_PERIOD;
		case VK_OEM_2:
			return DIK_SLASH;
		case VK_NUMPAD0:
			return DIK_NUMPAD0;
		case VK_NUMPAD1:
			return DIK_NUMPAD1;
		case VK_NUMPAD2:
			return DIK_NUMPAD2;
		case VK_NUMPAD3:
			return DIK_NUMPAD3;
		case VK_NUMPAD4:
			return DIK_NUMPAD4;
		case VK_NUMPAD5:
			return DIK_NUMPAD5;
		case VK_NUMPAD6:
			return DIK_NUMPAD6;
		case VK_NUMPAD7:
			return DIK_NUMPAD7;
		case VK_NUMPAD8:
			return DIK_NUMPAD8;
		case VK_NUMPAD9:
			return DIK_NUMPAD9;
		case VK_MULTIPLY:
			return DIK_MULTIPLY;
		case VK_ADD:
			return DIK_ADD;
		case VK_SEPARATOR:
			return DIK_NUMPADCOMMA;
		case VK_SUBTRACT:
			return DIK_SUBTRACT;
		case VK_DECIMAL:
			return DIK_DECIMAL;
		case VK_DIVIDE:
			return DIK_DIVIDE;
			//case VK_RETURN:		
			//return DIK_NUMPADENTER;
		case VK_F1:
			return DIK_F1;
		case VK_F2:
			return DIK_F2;
		case VK_F3:
			return DIK_F3;
		case VK_F4:
			return DIK_F4;
		case VK_F5:
			return DIK_F5;
		case VK_F6:
			return DIK_F6;
		case VK_F7:
			return DIK_F7;
		case VK_F8:
			return DIK_F8;
		case VK_F9:
			return DIK_F9;
		case VK_F10:
			return DIK_F10;
		case VK_F11:
			return DIK_F11;
		case VK_F12:
			return DIK_F12;
		case VK_F13:
			return DIK_F13;
		case VK_F14:
			return DIK_F14;
		case VK_F15:
			return DIK_F15;
		case VK_F16:
		case VK_F17:
		case VK_F18:
		case VK_F19:
		case VK_F20:
		case VK_F21:
		case VK_F22:
		case VK_F23:
		case VK_F24:
			return 0;
		case VK_NUMLOCK:
			return DIK_NUMLOCK;
		case VK_SCROLL:
			return DIK_SCROLL;
		case VK_LWIN:
			return DIK_LWIN;
		case VK_RWIN:
			return DIK_RWIN;
		case VK_APPS:
			return DIK_APPS;
		case VK_OEM_102:
			return DIK_OEM_102;
		}

		return 0xff;
    }

    return Key;
}
