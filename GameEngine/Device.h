#pragma once

#include "EngineInfo.h"

class CDevice
{
private:
	CDevice();
	~CDevice();

private:
	// 리소스 생성할 때 주로 사용한다.
	ID3D11Device* mDevice = nullptr;

	// 렌더링 명령 등에 주로 사용된다.
	ID3D11DeviceContext* mContext = nullptr;

	// 백버퍼를 가지고 있고 페이지 플리핑을 진행해주는 객체
	IDXGISwapChain* mSwapChain = nullptr;

	ID3D11RenderTargetView* mTargetView = nullptr;
	ID3D11DepthStencilView* mDepthView = nullptr;

	// 해상도
	FResolution		mRS;

	bool		mWindowMode = false;

	HWND		mhWnd;

public:
	ID3D11Device* GetDevice()	const
	{
		return mDevice;
	}

	ID3D11DeviceContext* GetContext()	const
	{
		return mContext;
	}

	const FResolution& GetResolution()	const
	{
		return mRS;
	}

	bool GetWindowMode()	const
	{
		return mWindowMode;
	}

public:
	bool Init(HWND hWnd, int Width, int Height, bool WindowMode);
	void ClearBackBuffer();
	void ClearDepthStencil(float Depth = 1.f, unsigned char Stencil = 0);
	void SetTarget();
	void BeginRender();
	void EndRender();

private:
	static CDevice* mInst;

public:
	static CDevice* GetInst()
	{
		if (!mInst)
			mInst = new CDevice;
		return mInst;
	}

	static void DestroyInst()
	{
		SAFE_DELETE(mInst);
	}
};

