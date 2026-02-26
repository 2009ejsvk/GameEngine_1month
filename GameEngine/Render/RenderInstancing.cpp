#include "RenderInstancing.h"
#include "../Component/SceneComponent.h"
#include "RenderState.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Texture/Texture.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Shader/ShaderManager.h"
#include "../World/World.h"
#include "../World/CameraManager.h"
#include "../Component/Animation2DComponent.h"
#include "RenderManager.h"
#include "../Device.h"

#define	INSTANCING_START_COUNT	5

CRenderInstancing::CRenderInstancing()
{
}

CRenderInstancing::~CRenderInstancing()
{
}

bool CRenderInstancing::CheckMesh(
	const std::weak_ptr<class CMesh>& Mesh) const
{
	return mMesh.lock() == Mesh.lock();
}

bool CRenderInstancing::CheckTexture(
	const std::weak_ptr<class CTexture>& Texture) const
{
	return mTexture.lock() == Texture.lock();
}

bool CRenderInstancing::ComparisonAsset(
	const std::weak_ptr<class CMesh>& Mesh,
	const std::weak_ptr<class CTexture>& Texture)
{
	auto	SrcMesh = mMesh.lock();
	auto	SrcTex = mTexture.lock();

	auto	DestMesh = Mesh.lock();
	auto	DestTexture = Texture.lock();

	return SrcMesh == DestMesh && SrcTex == DestTexture;
}

void CRenderInstancing::SetMesh(
	const std::weak_ptr<class CMesh>& Mesh)
{
	mMesh = Mesh;
}

void CRenderInstancing::SetTexture(
	const std::weak_ptr<class CTexture>& Texture)
{
	mTexture = Texture;
}

void CRenderInstancing::AddRenderList(
	const std::weak_ptr<class CSceneComponent>& Obj)
{
	auto	_Obj = Obj.lock();

	if (!_Obj->GetEnable())
		return;
	
	if (mRenderList.empty())
	{
		mWorld = _Obj->GetWorld();
		mBlendState = _Obj->GetBlendState();
		auto Shader = _Obj->GetShader().lock();
		std::string	Name = Shader->GetName() +
			"Instancing";
		mShader = CAssetManager::GetInst()->GetShaderManager().lock()->FindShader(Name);
	}

	mRenderList.push_back(Obj);

	auto	iter = mRenderList.begin();
	auto	iterEnd = mRenderList.end();

	for (; iter != iterEnd;)
	{
		if ((*iter).expired())
		{
			iter = mRenderList.erase(iter);
			iterEnd = mRenderList.end();
			continue;
		}

		++iter;
	}

	if (!mRender)
	{
		if (mRenderList.size() >= INSTANCING_START_COUNT)
		{
			mRender = true;

			if (!mInstancingBuffer.Buffer)
			{
				CreateInstancingBuffer(
					sizeof(FInstancingData), 
					INSTANCING_START_COUNT);
			}

			iter = mRenderList.begin();
			iterEnd = mRenderList.end();

			for (; iter != iterEnd; ++iter)
			{
				auto	_Com = (*iter).lock();

				_Com->SetRenderOption(
					EComponentRenderOption::Instancing);
			}
		}
	}

	else if (mRenderList.size() < INSTANCING_START_COUNT)
	{
		mRender = false;

		iter = mRenderList.begin();
		iterEnd = mRenderList.end();

		for (; iter != iterEnd; ++iter)
		{
			auto	_Com = (*iter).lock();

			_Com->SetRenderOption(
				EComponentRenderOption::Normal);
		}
	}

	else
	{
		auto	_Com = Obj.lock();

		_Com->SetRenderOption(
			EComponentRenderOption::Instancing);
	}
}

void CRenderInstancing::Update(float DeltaTime)
{
	if (!mRender || mRenderList.empty())
		return;

	if (mRenderList.size() > 1)
		mRenderList.sort(CRenderManager::SortYRenderList);

	mInstancingData.resize(mRenderList.size());

	mInstancingCount = 0;

	auto	World = mWorld.lock();

	auto	CameraMgr = World->GetCameraManager().lock();

	auto	Mesh = mMesh.lock();
	auto	Texture = mTexture.lock();

	auto	iter = mRenderList.begin();
	auto	iterEnd = mRenderList.end();

	for (; iter != iterEnd; ++iter)
	{
		auto	Com = (*iter).lock();

		// 인스턴싱용 버퍼를 채워준다.
		FMatrix	ScaleMat, RotMat, TranslateMat, WorldMat;

		ScaleMat.Scaling(Com->GetWorldScale());
		RotMat.Rotation(Com->GetWorldRot());
		TranslateMat.Translation(Com->GetWorldPos());

		WorldMat = ScaleMat * RotMat * TranslateMat;

		FMatrix	ViewMat = CameraMgr->GetViewMatrix();
		FMatrix	ProjMat = CameraMgr->GetProjMatrix();

		FMatrix	WVPMat = WorldMat * ViewMat * ProjMat;

		mInstancingData[mInstancingCount].WVP0 = WVPMat[0];
		mInstancingData[mInstancingCount].WVP1 = WVPMat[1];
		mInstancingData[mInstancingCount].WVP2 = WVPMat[2];
		mInstancingData[mInstancingCount].WVP3 = WVPMat[3];

		mInstancingData[mInstancingCount].ArrayTextureEnable = 0;
		mInstancingData[mInstancingCount].AnimFrame = 0;

		auto	AnimCom = Com->GetAnimComponent().lock();

		if (AnimCom)
		{
			mInstancingData[mInstancingCount].LTUV =
				AnimCom->GetAnimLTUV();
			mInstancingData[mInstancingCount].RBUV =
				AnimCom->GetAnimRBUV();

			mInstancingData[mInstancingCount].AnimFrame =
				AnimCom->GetAnimationFrame();

			if (AnimCom->GetTextureType() == EAnimation2DTextureType::Array)
			{
				mInstancingData[mInstancingCount].ArrayTextureEnable = 1;
			}
		}

		else
		{
			if (Texture)
			{
				if (Texture->GetTextureType() ==
					ETextureType::Array)
				{
					mInstancingData[mInstancingCount].ArrayTextureEnable = 1;
				}
			}

			mInstancingData[mInstancingCount].LTUV =
				FVector2(0.f, 0.f);
			mInstancingData[mInstancingCount].RBUV =
				FVector2(1.f, 1.f);
		}

		FVector3	Pivot = Com->GetPivot();

		FVector3	PivotSize = Pivot * Mesh->GetMeshSize();

		mInstancingData[mInstancingCount].PivotSize =
			PivotSize;

		mInstancingData[mInstancingCount].BaseColor =
			Com->GetBaseColor();

		++mInstancingCount;
	}

	SetInstancingData(&mInstancingData[0],
		mInstancingCount);
}

void CRenderInstancing::Render()
{
	if (!mRender || mRenderList.empty())
		return;

	std::shared_ptr<CRenderState>	BlendState =
		mBlendState.lock();

	if (BlendState)
		BlendState->SetState();

	// 렌더링
	auto	Mesh = mMesh.lock();
	auto	Shader = mShader.lock();
	auto	Texture = mTexture.lock();

	auto	iter = mRenderList.begin();
	auto	iterEnd = mRenderList.end();

	for (; iter != iterEnd; ++iter)
	{
		auto	Com = (*iter).lock();

		Com->SetRenderOption(EComponentRenderOption::Normal);
	}

	if (Texture)
	{
		int	Register = 0;

		if (Texture->GetTextureType() ==
			ETextureType::Array)
			Register = 1;

		Texture->SetShader(Register,
			EShaderBufferType::Pixel, 0);
	}

	Shader->SetShader();

	Mesh->RenderInstancing(mInstancingBuffer,
		mInstancingCount);


	if (BlendState)
		BlendState->ResetState();
}

void CRenderInstancing::RenderClear()
{
	mRenderList.clear();
	mInstancingData.clear();
}

bool CRenderInstancing::CreateInstancingBuffer(int Size,
	int Count)
{
	SAFE_RELEASE(mInstancingBuffer.Buffer);

	mInstancingBuffer.Size = Size;
	mInstancingBuffer.Count = Count;

	// 버퍼를 생성하기 위한 구조체
	D3D11_BUFFER_DESC	BufferDesc = {};

	// 버퍼의 전체 메모리 크기
	BufferDesc.ByteWidth = Size * Count;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;


	if (FAILED(CDevice::GetInst()->GetDevice()->CreateBuffer(&BufferDesc, nullptr, &mInstancingBuffer.Buffer)))
		return false;

	return true;
}

bool CRenderInstancing::SetInstancingData(void* Data,
	int Count)
{
	if (!mInstancingBuffer.Buffer)
		return false;

	if (mInstancingBuffer.Count < Count)
	{
		if (!CreateInstancingBuffer(mInstancingBuffer.Size,
			Count * 2))
			return false;
	}

	ID3D11DeviceContext* Context =
		CDevice::GetInst()->GetContext();

	D3D11_MAPPED_SUBRESOURCE	MS = {};

	Context->Map(mInstancingBuffer.Buffer, 0,
		D3D11_MAP_WRITE_DISCARD, 0, &MS);

	memcpy(MS.pData, Data, mInstancingBuffer.Size * Count);

	Context->Unmap(mInstancingBuffer.Buffer, 0);

	return true;
}
