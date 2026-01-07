#include "Mesh.h"
#include "../../Device.h"
#include "../Material/Material.h"

CMesh::CMesh()
{
	mAssetType = EAssetType::Mesh;
}

CMesh::~CMesh()
{
	/*size_t Size = mMeshSlot.size();

	for(size_t i = 0; i < Size; ++i)
	{
		SAFE_DELETE(mMeshSlot[i]);
	}*/
}

// 위치|색상   위치|UV
bool CMesh::CreateMesh(void* VertexData, int VertexSize, int VertexCount, D3D11_USAGE VertexUsage, D3D11_PRIMITIVE_TOPOLOGY Primitive,
	void* IndexData, int IndexSize, int IndexCount, DXGI_FORMAT Fmt, 
	D3D11_USAGE IndexUsage)
{
	// 버텍스 버퍼를 만든다.
	mVB.Size = VertexSize;
	mVB.Count = VertexCount;
	mPrimitive = Primitive;

	if (!CreateBuffer(&mVB.Buffer, D3D11_BIND_VERTEX_BUFFER, 
		VertexData,
		VertexSize, VertexCount, VertexUsage))
		return false;

	char* Vertices = (char*)VertexData;

	for (int i = 0; i < VertexCount; ++i)
	{
		FVector3	Pos = *((FVector3*)Vertices);

		if (mMin.x > Pos.x)
			mMin.x = Pos.x;

		if (mMin.y > Pos.y)
			mMin.y = Pos.y;

		if (mMin.z > Pos.z)
			mMin.z = Pos.z;

		if (mMax.x < Pos.x)
			mMax.x = Pos.x;

		if (mMax.y < Pos.y)
			mMax.y = Pos.y;

		if (mMax.z < Pos.z)
			mMax.z = Pos.z;

		Vertices += VertexSize;
	}

	mMeshSize = mMax - mMin;

	// 인덱스 데이터가 있다면 인덱스 버퍼도 만들어준다.
	if (IndexData)
	{
		std::shared_ptr<FMeshSlot>	Slot(new FMeshSlot);

		Slot->IndexBuffer.Size = IndexSize;
		Slot->IndexBuffer.Count = IndexCount;
		Slot->IndexBuffer.Fmt = Fmt;

		mMeshSlot.push_back(Slot);

		SetMaterial(0);

		if (!CreateBuffer(&Slot->IndexBuffer.Buffer, D3D11_BIND_INDEX_BUFFER,
			IndexData,
			IndexSize, IndexCount, IndexUsage))
			return false;
	}

	return true;
}

void CMesh::Render()
{
	UINT	Stride = mVB.Size;
	UINT	Offset = 0;

	// 출력할 도형 타입을 지정한다.
	CDevice::GetInst()->GetContext()->IASetPrimitiveTopology(mPrimitive);
	// 출력할 버텍스 버퍼를 지정한다.
	CDevice::GetInst()->GetContext()->IASetVertexBuffers(0, 1, &mVB.Buffer,
		&Stride, &Offset);

	if (mMeshSlot.empty())
	{
		CDevice::GetInst()->GetContext()->IASetIndexBuffer(
			nullptr, DXGI_FORMAT_UNKNOWN, 0);
		CDevice::GetInst()->GetContext()->Draw(mVB.Count, 0);
	}

	else
	{
		size_t	Size = mMeshSlot.size();

		for (size_t i = 0; i < Size; ++i)
		{
			// Material 정보를 Shader 상수버퍼로 남겨준다.
			if (mMeshSlot[i]->Material)
				mMeshSlot[i]->Material->UpdateConstantBuffer();

			// 출력에 사용할 인덱스 버퍼를 지정한다.
			CDevice::GetInst()->GetContext()->IASetIndexBuffer(
				mMeshSlot[i]->IndexBuffer.Buffer,
				mMeshSlot[i]->IndexBuffer.Fmt, 0);
			// 인덱스 버퍼의 인덱스를 참고하여 화면에 도형을 출력한다.
			CDevice::GetInst()->GetContext()->DrawIndexed(
				mMeshSlot[i]->IndexBuffer.Count, 0, 0);
		}
	}
}

void CMesh::Render(int SlotIndex)
{
	UINT	Stride = mVB.Size;
	UINT	Offset = 0;

	// 출력할 도형 타입을 지정한다.
	CDevice::GetInst()->GetContext()->IASetPrimitiveTopology(mPrimitive);
	// 출력할 버텍스 버퍼를 지정한다.
	CDevice::GetInst()->GetContext()->IASetVertexBuffers(0, 1, &mVB.Buffer,
		&Stride, &Offset);

	if (mMeshSlot.empty())
	{
		CDevice::GetInst()->GetContext()->IASetIndexBuffer(
			nullptr, DXGI_FORMAT_UNKNOWN, 0);
		CDevice::GetInst()->GetContext()->Draw(mVB.Count, 0);
	}

	else
	{
		// 출력에 사용할 인덱스 버퍼를 지정한다.
		CDevice::GetInst()->GetContext()->IASetIndexBuffer(
			mMeshSlot[SlotIndex]->IndexBuffer.Buffer,
			mMeshSlot[SlotIndex]->IndexBuffer.Fmt, 0);
		// 인덱스 버퍼의 인덱스를 참고하여 화면에 도형을 출력한다.
		CDevice::GetInst()->GetContext()->DrawIndexed(
			mMeshSlot[SlotIndex]->IndexBuffer.Count, 0, 0);
	}
}

void CMesh::SetMaterial(int SlotIndex)
{
	if (!mMeshSlot[SlotIndex]->Material)
	{
		mMeshSlot[SlotIndex]->Material.reset(new CMaterial);

		mMeshSlot[SlotIndex]->Material->Init();
	}
}

void CMesh::SetMaterialBaseColor(int SlotIndex, float r, float g, float b, float a)
{
	if (!mMeshSlot[SlotIndex]->Material)
	{
		mMeshSlot[SlotIndex]->Material.reset(new CMaterial);

		mMeshSlot[SlotIndex]->Material->Init();
	}

	mMeshSlot[SlotIndex]->Material->SetBaseColor(r, g, b, a);
}

void CMesh::SetMaterialBaseColor(int SlotIndex, int r, int g,
	int b, int a)
{
	if (!mMeshSlot[SlotIndex]->Material)
	{
		mMeshSlot[SlotIndex]->Material.reset(new CMaterial);

		mMeshSlot[SlotIndex]->Material->Init();
	}

	mMeshSlot[SlotIndex]->Material->SetBaseColor(r, g, b, a);
}

void CMesh::SetMaterialBaseColor(int SlotIndex, const FVector4& Color)
{
	if (!mMeshSlot[SlotIndex]->Material)
	{
		mMeshSlot[SlotIndex]->Material.reset(new CMaterial);

		mMeshSlot[SlotIndex]->Material->Init();
	}

	mMeshSlot[SlotIndex]->Material->SetBaseColor(Color);
}

void CMesh::SetMaterialOpacity(int SlotIndex, float Opacity)
{
	if (!mMeshSlot[SlotIndex]->Material)
	{
		mMeshSlot[SlotIndex]->Material.reset(new CMaterial);

		mMeshSlot[SlotIndex]->Material->Init();
	}

	mMeshSlot[SlotIndex]->Material->SetOpacity(Opacity);
}

bool CMesh::CreateBuffer(ID3D11Buffer** Buffer, D3D11_BIND_FLAG Flag,
	void* Data, int Size, int Count, D3D11_USAGE Usage)
{
	// 버퍼를 생성하기 위한 구조체
	D3D11_BUFFER_DESC	BufferDesc = {};

	// 버퍼의 전체 메모리 크기
	BufferDesc.ByteWidth = Size * Count;
	BufferDesc.Usage = Usage;
	BufferDesc.BindFlags = Flag;

	if (Usage == D3D11_USAGE_DYNAMIC)
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	else if(Usage == D3D11_USAGE_STAGING)
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;

	D3D11_SUBRESOURCE_DATA	BufferData = {};

	BufferData.pSysMem = Data;

	if (FAILED(CDevice::GetInst()->GetDevice()->CreateBuffer(&BufferDesc,
		&BufferData, Buffer)))
		return false;

	return true;
}
