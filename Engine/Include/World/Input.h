#pragma once

#define	DIRECTINPUT_VERSION	0x0800

#define	DIK_MOUSELBUTTON	0xfc
#define	DIK_MOUSERBUTTON	0xfd
#define	DIK_MOUSEWHEELBUTTON	0xfe

#include "../EngineInfo.h"
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")

enum class EInputSystemType
{
	DInput,
	Window
};

namespace EInputType
{
	enum Type
	{
		Press,		// 누르는 순간
		Hold,		// 누르고 있을 때	
		Release,	// 키 해제시
		End
	};
}

namespace EMouseType
{
	enum Type
	{
		LButton,
		RButton,
		MButton,
		End
	};
}

struct FKeyState
{
	unsigned char	Key = 0;
	bool		Press = false;
	bool		Hold = false;
	bool		Release = false;
};

struct FBindKey
{
	// 어떤 키를 이용하는지.
	FKeyState* Key = nullptr;
	bool	Ctrl = false;
	bool	Alt = false;
	bool	Shift = false;
	std::function<void()>	Func[EInputType::End];
	bool	KeyHold = false;
};

class CInput
{
	friend class CWorld;

private:
	CInput();

public:
	~CInput();

private:
	EInputSystemType	mInputType = EInputSystemType::DInput;
	HINSTANCE	mhInst;
	HWND		mhWnd;
	IDirectInput8* mInput = nullptr;
	IDirectInputDevice8* mKeyboard = nullptr;
	IDirectInputDevice8* mMouse = nullptr;

	// DInput 키보드 상태를 저장하기 위한 변수.
	unsigned char	mKeyState[256] = {};
	DIMOUSESTATE	mMouseState = {};

private:
	std::unordered_map<unsigned char, FKeyState*>	mKeyStateMap;
	std::unordered_map<std::string, std::unique_ptr<FBindKey>>	mBindKeyMap;

	bool	mCtrlState[EInputType::End] = {};
	bool	mAltState[EInputType::End] = {};
	bool	mShiftState[EInputType::End] = {};

	bool	mMouseButton[EMouseType::End][EInputType::End] = {};

public:
	bool Init();
	void Update(float DeltaTime);

private:
	bool InitDevice();
	void UpdateKeyboard();
	void UpdateMouse();
	void UpdateInput(float DeltaTime);
	void UpdateDInput(float DeltaTime);
	void UpdateWindowInput(float DeltaTime);
	void UpdateBindKey(float DeltaTime);

public:
	void AddBindKey(const std::string& Name,
		unsigned char Key);
	void SetKeyCtrl(const std::string& Name, bool Ctrl);
	void SetKeyAlt(const std::string& Name, bool Alt);
	void SetKeyShift(const std::string& Name, bool Shift);

private:
	FKeyState* FindKeyState(unsigned char Key);
	FBindKey* FindBindKey(const std::string& Name);
	unsigned char ConvertKey(unsigned char Key);

public:
	template <typename T>
	void SetBindFunction(const std::string& Name,
		EInputType::Type Type,
		T* Object, void (T::*Func)())
	{
		FBindKey* Key = FindBindKey(Name);

		if (!Key)
			return;

		Key->Func[Type] = std::bind(Func, Object);
	}
};

