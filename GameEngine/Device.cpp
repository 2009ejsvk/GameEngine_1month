#include "Device.h"

CDevice* CDevice::mInst = nullptr;

CDevice::CDevice()
{
}

CDevice::~CDevice()
{
	if (mTargetView)
		mTargetView->Release();

	if (mDepthView)
		mDepthView->Release();

	if (mSwapChain)
		mSwapChain->Release();

	// Context에 지정된 상태를 지워준다.
	if (mContext)
	{
		mContext->ClearState();

		mContext->Release();
	}

	if (mDevice)
	{
		mDevice->Release();
	}
}

bool CDevice::Init(HWND hWnd, int Width, int Height, bool WindowMode)
{
	mhWnd = hWnd;
	mRS.Width = Width;
	mRS.Height = Height;
	mWindowMode = WindowMode;

	UINT	Flag = 0;

#ifdef _DEBUG

	Flag = D3D11_CREATE_DEVICE_DEBUG;

#endif // _DEBUG

	D3D_FEATURE_LEVEL	FLevel = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL	FLevelResult;

	// FAILED매크로는 HRESULT로 반환된 결과에 대해 실패인지 체크하는 매크로이다.
	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, Flag, &FLevel,
		1, D3D11_SDK_VERSION, &mDevice, &FLevelResult, &mContext)))
	{
		return false;
	}

	UINT	SampleCount = 4;

	UINT	Check = 0;
	mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &Check);

	if (Check < 1)
		SampleCount = 1;

	SampleCount = 1;

	// SwapChain을 생성한다.
	DXGI_SWAP_CHAIN_DESC	SwapDesc = {};

	// 백버퍼의 가로, 세로 해상도를 지정한다.
	SwapDesc.BufferDesc.Width = Width;
	SwapDesc.BufferDesc.Height = Height;

	// 백버퍼의 픽셀 포맷을 지정한다.
	SwapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// 화면 주사율 지정. FLIP_DISCARD 사용시 주사율을 OS 맡겨둠.
	SwapDesc.BufferDesc.RefreshRate.Numerator = 1;
	SwapDesc.BufferDesc.RefreshRate.Denominator = 0;

	// 지정한 해상도와 모니터 해상도가 맞지 않을 경우 어떻게 스케일링 할지를 결정한다.
	// DXGI_MODE_SCALING_UNSPECIFIED   = 0, 드라이버/OS에 맡김.
	// DXGI_MODE_SCALING_CENTERED = 1, 작은 해상도를 쓸 때, 가운데 배치 + 주변은 검은 여백
	// DXGI_MODE_SCALING_STRETCHED = 2 해상도를 화면에 맞게 늘리거나 줄여서 사용.
	SwapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// 화면을 그릴 때 줄 순서를 어떻게 처리할지에 대한 옵션이다.
	// DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED = 0, 신경안씀.
	// DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE = 1, 순차 스캔.
	SwapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	// 백버퍼 수 지정. FLIP_DISCARD를 사용하려면 2개 이상 지정.
	SwapDesc.BufferCount = 2;

	// 백버퍼를 출력용으로 지정한다.
	SwapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// 출력할 윈도우 핸들을 지정한다.
	SwapDesc.OutputWindow = hWnd;

	// 멀티샘플링 퀄리티를 지정한다. FLIP_DISCARD는 MSAA(멀티샘플링) 지원 불가.
	SwapDesc.SampleDesc.Quality = 0;
	SwapDesc.SampleDesc.Count = 1;

	// 창모드인지 풀스크린인지 지정한다.
	SwapDesc.Windowed = WindowMode;

	/*
	스왑체인에서 백버퍼 -> 화면으로 넘길 때 백버퍼를 어떻게 처리할지를 지정하는 옵션이다.
	DXGI_SWAP_EFFECT_DISCARD	= 0, 프레임을 화면에 보낸 뒤, 버퍼 내용은 보장 안한다.
    DXGI_SWAP_EFFECT_SEQUENTIAL	= 1, 버퍼가 순서대로 화면에 처리된다.
    DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL	= 3, 순차 출력을 하며 플립 처리를 한다.
    DXGI_SWAP_EFFECT_FLIP_DISCARD	= 4 플립 하며 백버퍼의 내용은 버린다.

	DXGI_SWAP_EFFECT_FLIP_DISCARD 사용시 버퍼가 최소 2개 이상이어야 가능하다.
	*/
	SwapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// DXGIDevice를 얻어온다.
	IDXGIDevice* DXGIDevice = nullptr;

	// __uuidof : 컴 타입의 ID를 얻어온다.
	mDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&DXGIDevice);

	IDXGIAdapter* Adapter = nullptr;
	DXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&Adapter);

	IDXGIFactory* Factory = nullptr;
	Adapter->GetParent(__uuidof(IDXGIFactory), (void**)&Factory);

	// 위에서 지정한 정보를 이용하여 SwapChain을 생성해준다.
	if (FAILED(Factory->CreateSwapChain(mDevice, &SwapDesc, &mSwapChain)))
	{
		Factory->Release();
		Adapter->Release();
		DXGIDevice->Release();
		return false;
	}

	Factory->Release();
	Adapter->Release();
	DXGIDevice->Release();

	// SwapChain에 지정되어 있는 백버퍼를 이용해서 화면에 출력하기 위한 타겟을 생성한다.

	// SwapChain으로부터 백버퍼를 얻어온다.
	ID3D11Texture2D* BackBuffer = nullptr;

	// GetBuffer로 얻어오면 레퍼런스 카운트가 1 증가하기 때문에 다 썼다면
	// Release를 호출하여 카운트를 감소시켜주어야 한다.
	if (FAILED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer)))
		return false;

	// 백버퍼에 출력하기 위한 렌더타겟을 얻어온다.
	// 렌더타겟은 픽셀을 저장하는 버퍼에 픽셀을 출력하기 위한 용도이다.
	if (FAILED(mDevice->CreateRenderTargetView(BackBuffer, nullptr, &mTargetView)))
		return false;

	BackBuffer->Release();

	// 깊이 버퍼를 생성한다.
	D3D11_TEXTURE2D_DESC	DepthDesc = {};

	DepthDesc.Width = Width;
	DepthDesc.Height = Height;
	DepthDesc.ArraySize = 1;

	// D24 : 깊이를 저장하는 용으로 24비트(3바이트)를 사용한다.
	// S8 : 스텐실 값을 저장하는 용으로 8비트(1바이트)를 사용한다.
	// UNORM : 0 ~ 1사이의 실수값
	// UINT : 0 ~ 255 까지의 정수값
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 깊이, 스텐실 용으로 사용한다.
	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	DepthDesc.Usage = D3D11_USAGE_DEFAULT;

	DepthDesc.SampleDesc.Count = 1;
	DepthDesc.SampleDesc.Quality = 0;
	DepthDesc.MipLevels = 1;

	ID3D11Texture2D* DepthBuffer = nullptr;

	// 위에서 정의한 Texture 정보로 Texture를 생성한다.
	mDevice->CreateTexture2D(&DepthDesc, nullptr, &DepthBuffer);

	// 깊이 뷰를 만들어준다.
	if (FAILED(mDevice->CreateDepthStencilView(DepthBuffer, nullptr, &mDepthView)))
		return false;

	DepthBuffer->Release();

	// 뷰포트
	D3D11_VIEWPORT	VP = {};

	VP.Width = (float)Width;
	VP.Height = (float)Height;
	VP.MaxDepth = 1.f;

	// 뷰포트를 지정한다.
	mContext->RSSetViewports(1, &VP);

	return true;
}

void CDevice::ClearBackBuffer()
{
	// 지정된 색으로 백버퍼를 초기화한다.
	//float	Color[4] = { 1.f, 1.f, 1.f, 1.f };
	float	Color[4] = {};
	mContext->ClearRenderTargetView(mTargetView, Color);
}

void CDevice::ClearDepthStencil(float Depth, unsigned char Stencil)
{
	mContext->ClearDepthStencilView(mDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		Depth, Stencil);
}

void CDevice::SetTarget()
{
	// 출력하기 위한 타겟을 지정한다.
	mContext->OMSetRenderTargets(1, &mTargetView, mDepthView);
}

void CDevice::BeginRender()
{
	ClearBackBuffer();
	ClearDepthStencil();
	SetTarget();
}

void CDevice::EndRender()
{
	// 백버퍼를 화면에 보여준다.
	mSwapChain->Present(0, 0);
}
