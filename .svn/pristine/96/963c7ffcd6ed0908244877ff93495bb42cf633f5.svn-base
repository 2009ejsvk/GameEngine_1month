#include "GraphicShader.h"
#include "../../Device.h"
#include "../PathManager.h"

CGraphicShader::CGraphicShader()
{
}

CGraphicShader::~CGraphicShader()
{
	SAFE_RELEASE(mInputLayout);
	SAFE_RELEASE(mVS);
	SAFE_RELEASE(mVSBlob);
	SAFE_RELEASE(mPS);
	SAFE_RELEASE(mPSBlob);
}

void CGraphicShader::SetShader()
{
	CDevice::GetInst()->GetContext()->VSSetShader(mVS, nullptr, 0);
	CDevice::GetInst()->GetContext()->PSSetShader(mPS, nullptr, 0);

	CDevice::GetInst()->GetContext()->IASetInputLayout(mInputLayout);
}

void CGraphicShader::AddInputDesc(const char* Semantic, UINT SemanticIndex,
	DXGI_FORMAT Fmt, UINT InputSlot, UINT Size,
	D3D11_INPUT_CLASSIFICATION InputSlotClass, UINT InstanceDataStepRate)
{
	D3D11_INPUT_ELEMENT_DESC	Desc = {};

	Desc.SemanticName = Semantic;
	Desc.SemanticIndex = SemanticIndex;
	Desc.Format = Fmt;
	Desc.InputSlot = InputSlot;
	Desc.InputSlotClass = InputSlotClass;
	Desc.InstanceDataStepRate = InstanceDataStepRate;
	Desc.AlignedByteOffset = mSize;

	mSize += Size;

	mInputDesc.push_back(Desc);
}

bool CGraphicShader::CreateInputLayout()
{
	// InputLayout은 정점이 어떤 구조로 구성되는지를 나타낸다.
	if (FAILED(CDevice::GetInst()->GetDevice()->CreateInputLayout(
		&mInputDesc[0],
		(UINT)mInputDesc.size(), mVSBlob->GetBufferPointer(),
		mVSBlob->GetBufferSize(), &mInputLayout)))
		return false;

	return true;
}

bool CGraphicShader::LoadVertexShader(const char* EntryName, 
	const TCHAR* FileName, const std::string& PathName)
{
	const TCHAR* BasePath = CPathManager::FindPath(PathName);

	TCHAR	FullPath[MAX_PATH] = {};

	lstrcpy(FullPath, BasePath);
	lstrcat(FullPath, FileName);

	UINT	Flag = 0;

#ifdef _DEBUG
	Flag = D3DCOMPILE_DEBUG;
#endif // _DEBUG

	// 컴파일 에러 메세지를 전달받기 위한 버퍼
	ID3DBlob* ErrorBlob = nullptr;

	// 이 경로의 Shader파일을 컴파일한다.
	// 1. Shader 파일이 있는 경로.
	// 2. Macro
	// 3. Include 어떻게 할것인지를 지정.
	// 4. 컴파일 하려는 함수의 이름
	// 5. Shader Version 지정
	// 6, 7. 추가 옵션 지정.
	// 8. 컴파일된 Shader의 바이트코드
	// 9. 에러가 있을 경우 에러 메세지를 얻어오기 위한 버퍼
	if (FAILED(D3DCompileFromFile(FullPath, nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryName, "vs_5_0", Flag,
		0, &mVSBlob, &ErrorBlob)))
	{
#ifdef _DEBUG
		char	ErrorText[1024] = {};
		strcpy_s(ErrorText, (const char*)ErrorBlob->GetBufferPointer());
		strcat_s(ErrorText, "\n");
		// Visual Studio의 출력창에 문자열을 출력한다.
		OutputDebugStringA(ErrorText);
#endif // _DEBUG

		return false;
	}

	// VertexShader 객체를 생성한다.
	// 1. 컴파일된 Shader 바이트 코드
	// 2. 바이트 코드의 크기
	// 3. 링크
	// 4. 생성할 Vertex Shader 객체
	if (FAILED(CDevice::GetInst()->GetDevice()->CreateVertexShader(
		mVSBlob->GetBufferPointer(), mVSBlob->GetBufferSize(),
		nullptr, &mVS)))
		return false;

	return true;
}

bool CGraphicShader::LoadPixelShader(const char* EntryName, 
	const TCHAR* FileName, const std::string& PathName)
{
	const TCHAR* BasePath = CPathManager::FindPath(PathName);

	TCHAR	FullPath[MAX_PATH] = {};

	lstrcpy(FullPath, BasePath);
	lstrcat(FullPath, FileName);

	UINT	Flag = 0;

#ifdef _DEBUG
	Flag = D3DCOMPILE_DEBUG;
#endif // _DEBUG

	// 컴파일 에러 메세지를 전달받기 위한 버퍼
	ID3DBlob* ErrorBlob = nullptr;

	// 이 경로의 Shader파일을 컴파일한다.
	// 1. Shader 파일이 있는 경로.
	// 2. Macro
	// 3. Include 어떻게 할것인지를 지정.
	// 4. 컴파일 하려는 함수의 이름
	// 5. Shader Version 지정
	// 6, 7. 추가 옵션 지정.
	// 8. 컴파일된 Shader의 바이트코드
	// 9. 에러가 있을 경우 에러 메세지를 얻어오기 위한 버퍼
	if (FAILED(D3DCompileFromFile(FullPath, nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryName, "ps_5_0", Flag,
		0, &mPSBlob, &ErrorBlob)))
	{
#ifdef _DEBUG
		char	ErrorText[1024] = {};
		strcpy_s(ErrorText, (const char*)ErrorBlob->GetBufferPointer());
		strcat_s(ErrorText, "\n");
		// Visual Studio의 출력창에 문자열을 출력한다.
		OutputDebugStringA(ErrorText);
#endif // _DEBUG

		return false;
	}

	// PixelShader 객체를 생성한다.
	// 1. 컴파일된 Shader 바이트 코드
	// 2. 바이트 코드의 크기
	// 3. 링크
	// 4. 생성할 Pixel Shader 객체
	if (FAILED(CDevice::GetInst()->GetDevice()->CreatePixelShader(
		mPSBlob->GetBufferPointer(), mPSBlob->GetBufferSize(),
		nullptr, &mPS)))
		return false;

	return true;
}
