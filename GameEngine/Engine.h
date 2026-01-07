#pragma once

#include "EngineInfo.h"
#include "Object.h"
#include "EngineSetting.h"

class CEngine
{
private:
	CEngine();
	~CEngine();

private:
	HINSTANCE	mhInst;
	HWND		mhWnd;
	std::shared_ptr<CEngineSetting>	mSetting;

	static bool	mLoop;

public:
	HINSTANCE GetWindowInstance()	const
	{
		return mhInst;
	}

	HWND GetWindowHandle()	const
	{
		return mhWnd;
	}

public:
	bool Init(HINSTANCE hInst, const TCHAR* WindowName, 
		int IconID,
		int SmallIconID, int Width, int Height, 
		bool WindowMode = true);
	int Run();

private:
	void Logic();

	// 물체들의 정보를 갱신하는 함수
	void Update(float DeltaTime);

	// 갱신된 정보를 화면에 출력하는 함수
	void Render();

private:
	void InitCDO();

public:
	template <typename T>
	static void CreateCDO()
	{
		CObject::CreateCDO<T>();
	}

	// 윈도우 레지스터 클래스 등록
	void InitRegisterClass(const TCHAR* WindowName, int IconID, int SmallIconID);
	// 윈도우 창 생성
	bool Create(const TCHAR* WindowName, int Width, int Height);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	template <typename T>
	std::weak_ptr<T> CreateEngineSetting()
	{
		mSetting.reset(new T);

		return std::dynamic_pointer_cast<T>(mSetting);
	}

private:
	static CEngine* mInst;

public:
	static CEngine* GetInst()
	{
		if (!mInst)
			mInst = new CEngine;
		return mInst;
	}

	static void DestroyInst()
	{
		SAFE_DELETE(mInst);
	}
};

