#pragma once

#include "../Asset.h"

struct FVertexBuffer
{
	// 버텍스를 저장하기 위한 버퍼
	ID3D11Buffer* Buffer = nullptr;
	// 버텍스 1개의 메모리 크기
	int		Size = 0;
	// 버택스 개수
	int		Count = 0;

	FVertexBuffer()
	{
	}

	~FVertexBuffer()
	{
		if (Buffer)
			Buffer->Release();
	}
};

struct FIndexBuffer
{
	// 인덱스를 저장하기 위한 버퍼
	ID3D11Buffer* Buffer = nullptr;
	// 인덱스 1개의 메모리 크기
	int		Size = 0;
	// 인덱스 개수
	int		Count = 0;
	// 인덱스를 이용해서 출력할 때 인덱스 타입을 지정하는 용도로 사용한다.
	DXGI_FORMAT	Fmt = DXGI_FORMAT_UNKNOWN;

	FIndexBuffer()
	{
	}

	~FIndexBuffer()
	{
		if (Buffer)
			Buffer->Release();
	}
};

struct FMeshSlot
{
	FIndexBuffer	IndexBuffer;
	std::shared_ptr<class CMaterial>	Material;
};

class CMesh :
    public CAsset
{
	friend class CMeshManager;

protected:
	CMesh();

public:
	virtual ~CMesh();

protected:
	FVertexBuffer	mVB;
	std::vector<std::shared_ptr<FMeshSlot>>	mMeshSlot;
	D3D11_PRIMITIVE_TOPOLOGY	mPrimitive =
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	FVector3	mMin = FVector3(FLT_MAX, FLT_MAX, FLT_MAX);
	FVector3	mMax = FVector3(FLT_MIN, FLT_MIN, FLT_MIN);
	FVector3	mMeshSize;

public:
	const FVector3& GetMin()	const
	{
		return mMin;
	}

	const FVector3& GetMax()	const
	{
		return mMax;
	}

	const FVector3& GetMeshSize()	const
	{
		return mMeshSize;
	}

	size_t GetMeshSlotCount()	const
	{
		return mMeshSlot.size();
	}

	std::shared_ptr<FMeshSlot> GetMeshSlot(int Index)	const
	{
		return mMeshSlot[Index];
	}

public:
	/*
	D3D11_USAGE : 버퍼나 텍스처 같은 리소스를 어떤 용도로 사용하는지 정의하는 타입이다.
	D3D11_USAGE_DEFAULT	= 0, : GPU 전용(읽기/쓰기 가능). CPU는 읽기/쓰기 모두 불가능하다. 
    D3D11_USAGE_IMMUTABLE	= 1, : GPU 읽기 가능. CPU 쓰기 불가능. 한번 넣고 정보
	를 안바꿀 경우 사용한다.
    D3D11_USAGE_DYNAMIC	= 2, : GPU 읽기 가능. CPU 쓰기 가능. 정보를 자주 업데이트
	해야 하는 곳에 주로 사용한다. 
    D3D11_USAGE_STAGING	= 3 : CPU 읽기/쓰기 모두 가능. GPU는 사용안한다.
	메모리를 확보하고 데이터를 저장해두는 용도로 주로 사용한다.
	*/
	bool CreateMesh(void* VertexData, int VertexSize, int VertexCount,
		D3D11_USAGE VertexUsage, D3D11_PRIMITIVE_TOPOLOGY Primitive,
		void* IndexData = nullptr, int IndexSize = 0, int IndexCount = 0,
		DXGI_FORMAT Fmt = DXGI_FORMAT_UNKNOWN,
		D3D11_USAGE IndexUsage = D3D11_USAGE_DEFAULT);
	void Render();
	void Render(int SlotIndex);
	void SetMaterial(int SlotIndex);
	void SetMaterialBaseColor(int SlotIndex,
		float r, float g, float b, float a);
	void SetMaterialBaseColor(int SlotIndex,
		int r, int g,
		int b, int a);
	void SetMaterialBaseColor(int SlotIndex,
		const FVector4& Color);
	void SetMaterialOpacity(int SlotIndex, float Opacity);

protected:
	bool CreateBuffer(ID3D11Buffer** Buffer, D3D11_BIND_FLAG Flag,
		void* Data, int Size, int Count, D3D11_USAGE Usage);
};

