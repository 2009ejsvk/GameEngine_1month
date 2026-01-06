#pragma once

#include "Shader.h"

class CGraphicShader :
    public CShader
{
	friend class CShaderManager;

protected:
	CGraphicShader();

public:
	virtual ~CGraphicShader() = 0;

protected:
	// VertexShader, PixelShader, DomainShader, HullShader, GeometryShader
	ID3D11InputLayout* mInputLayout = nullptr;

	// D3D11_INPUT_ELEMENT_DESC : 입력레지스터 요소 1개를 이 구조체 1개로 지정.
	// 예를 들어 float3 : POSITION0, float4 : COLOR0 일 경우
	// 이 구조체 2개가 있어야 지정이 가능하다.
	std::vector<D3D11_INPUT_ELEMENT_DESC>	mInputDesc;

	UINT		mSize = 0;

	// VertexShader 객체
	ID3D11VertexShader* mVS = nullptr;

	// VertexShader 코드를 컴파일하고 바이트 코드를 저장하기 위한 버퍼
	ID3DBlob* mVSBlob = nullptr;

	// PixelShader 객체
	ID3D11PixelShader* mPS = nullptr;

	// PixelShader 코드를 컴파일하고 바이트 코드를 저장하기 위한 버퍼
	ID3DBlob* mPSBlob = nullptr;

public:
	virtual bool Init(const std::string& PathName) = 0;
	virtual void SetShader() final;

public:
	/*
	LPCSTR SemanticName;	레지스터 이름
    UINT SemanticIndex;		레지스터 번호
    DXGI_FORMAT Format;		몇 바이트인지를 포맷으로 지정.
    UINT InputSlot;			버텍스 버퍼 번호
    UINT AlignedByteOffset;	여기서 지정하는 요소가 구조체의 몇바이트 째에 있는지
    D3D11_INPUT_CLASSIFICATION InputSlotClass;	정점데이터인지 인스턴싱인지
    UINT InstanceDataStepRate;	인스턴싱일 경우에만 사용.
	*/
	void AddInputDesc(const char* Semantic, UINT SemanticIndex,
		DXGI_FORMAT Fmt, UINT InputSlot, UINT Size,
		D3D11_INPUT_CLASSIFICATION InputSlotClass,
		UINT InstanceDataStepRate);
	bool CreateInputLayout();
	// EntryName : 함수 이름
	bool LoadVertexShader(const char* EntryName, const TCHAR* FileName, const std::string& PathName = "Shader");
	bool LoadPixelShader(const char* EntryName, const TCHAR* FileName, const std::string& PathName = "Shader");
};

