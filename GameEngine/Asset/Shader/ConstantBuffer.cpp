#include "ConstantBuffer.h"
#include "../../Device.h"

CConstantBuffer::CConstantBuffer()
{
	mAssetType = EAssetType::ConstantBuffer;
}

CConstantBuffer::~CConstantBuffer()
{
	SAFE_RELEASE(mBuffer);
}

bool CConstantBuffer::Init(int Size, int Register, int ShaderBuffer)
{
	mSize = Size;
	mRegister = Register;
	mShaderBuffer = ShaderBuffer;

	D3D11_BUFFER_DESC	Desc = {};

	// 상수버퍼는 데이터의 갱신이 빈번하게 일어나게 때문에
	// Dynamic으로 생성한다.
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.ByteWidth = mSize;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(CDevice::GetInst()->GetDevice()->CreateBuffer(&Desc,
		nullptr, &mBuffer)))
		return false;

	return true;
}

void CConstantBuffer::Update(void* Data)
{
	// 인자로 들어온 데이터로 버퍼를 갱신한다.
	D3D11_MAPPED_SUBRESOURCE	Map = {};
	
	// Map 안의 pData 변수에 버퍼 배열의 시작 주소를 얻어온다.
	CDevice::GetInst()->GetContext()->Map(mBuffer, 0,
		D3D11_MAP_WRITE_DISCARD, 0, &Map);

	memcpy(Map.pData, Data, mSize);

	CDevice::GetInst()->GetContext()->Unmap(mBuffer, 0);

	// Shader에 상수버퍼를 보내준다.
	if (mShaderBuffer & EShaderBufferType::Vertex)
	{
		CDevice::GetInst()->GetContext()->VSSetConstantBuffers(
			mRegister, 1, &mBuffer);
	}

	// PixelShader
	if (mShaderBuffer & EShaderBufferType::Pixel)
	{
		CDevice::GetInst()->GetContext()->PSSetConstantBuffers(
			mRegister, 1, &mBuffer);
	}

	// DomainShader
	if (mShaderBuffer & EShaderBufferType::Domain)
	{
		CDevice::GetInst()->GetContext()->DSSetConstantBuffers(
			mRegister, 1, &mBuffer);
	}

	// HullShader
	if (mShaderBuffer & EShaderBufferType::Hull)
	{
		CDevice::GetInst()->GetContext()->HSSetConstantBuffers(
			mRegister, 1, &mBuffer);
	}

	// GeometryShader
	if (mShaderBuffer & EShaderBufferType::Geometry)
	{
		CDevice::GetInst()->GetContext()->GSSetConstantBuffers(
			mRegister, 1, &mBuffer);
	}

	// ComputeShader
	if (mShaderBuffer & EShaderBufferType::Compute)
	{
		CDevice::GetInst()->GetContext()->CSSetConstantBuffers(
			mRegister, 1, &mBuffer);
	}
}
