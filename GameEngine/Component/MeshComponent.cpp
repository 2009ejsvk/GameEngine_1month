#include "MeshComponent.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Mesh/MeshManager.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Shader/ShaderManager.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Shader/CBufferTransform.h"
#include "CameraComponent.h"
#include "../World/World.h"
#include "../World/CameraManager.h"
#include "../Asset/Material/Material.h"
#include "Animation2DComponent.h"
#include "../Asset/Shader/CBufferAnimation2D.h"

std::shared_ptr<class CCBufferAnimation2D> CMeshComponent::mEmptyAnimCBuffer;

CMeshComponent::CMeshComponent()
{
	SetClassType<CMeshComponent>();
	mRenderType = EComponentRender::Render;
}

CMeshComponent::CMeshComponent(const CMeshComponent& ref)	:
	CSceneComponent(ref)
{
	mRenderType = EComponentRender::Render;
	mMesh = ref.mMesh;
	mShader = ref.mShader;
	mTransformCBuffer.reset(ref.mTransformCBuffer->Clone());
}

CMeshComponent::CMeshComponent(CMeshComponent&& ref)	noexcept :
	CSceneComponent(std::move(ref))
{
	mRenderType = EComponentRender::Render;
	mMesh = ref.mMesh;
	mShader = ref.mShader;
	mTransformCBuffer.reset(ref.mTransformCBuffer->Clone());
}

CMeshComponent::~CMeshComponent()
{
}

void CMeshComponent::CreateEmptyAnimCBuffer()
{
	mEmptyAnimCBuffer.reset(new CCBufferAnimation2D);

	mEmptyAnimCBuffer->Init();

	mEmptyAnimCBuffer->SetAnimation2DEnable(false);
}

void CMeshComponent::ClearEmptyAnimCBuffer()
{
	mEmptyAnimCBuffer.reset();
}

void CMeshComponent::SetMesh(const std::weak_ptr<CMesh>& Mesh)
{
	mMesh = Mesh;

	std::shared_ptr<CMesh>	_Mesh = mMesh.lock();

	if (_Mesh)
	{
		mMaterialSlot.clear();

		size_t	SlotCount = _Mesh->GetMeshSlotCount();

		for (size_t i = 0; i < SlotCount; ++i)
		{
			std::shared_ptr<FMeshSlot>	Slot = _Mesh->GetMeshSlot((int)i);

			CMaterial* Material = Slot->Material->Clone();

			std::shared_ptr<CMaterial>	Mtrl(Material);

			mMaterialSlot.push_back(Mtrl);
		}
	}
}

void CMeshComponent::SetMesh(const std::string& Name)
{
	auto	World = mWorld.lock();

	std::weak_ptr<CMesh>	Mesh;

	if (World)
	{
		auto	AssetMgr = World->GetWorldAssetManager().lock();

		Mesh = AssetMgr->FindMesh(Name);

		if (Mesh.expired())
			return;

		SetMesh(Mesh);
	}

	else
	{
		std::weak_ptr<CMeshManager> Weak_MeshMgr =
			CAssetManager::GetInst()->GetMeshManager();

		std::shared_ptr<CMeshManager>   MeshMgr = Weak_MeshMgr.lock();

		SetMesh(MeshMgr->FindMesh(Name));
	}
}

void CMeshComponent::SetShader(
	const std::weak_ptr<class CShader>& Shader)
{
	mShader = Shader;
}

void CMeshComponent::SetShader(const std::string& Name)
{
	std::shared_ptr<CShaderManager>   ShaderMgr =
		CAssetManager::GetInst()->GetShaderManager().lock();

	mShader = ShaderMgr->FindShader(Name);
}

void CMeshComponent::SetBlendState(int SlotIndex, 
	const std::string& Name)
{
	mMaterialSlot[SlotIndex]->SetBlendState(Name);
}

void CMeshComponent::SetBlendState(int SlotIndex,
	const std::weak_ptr<class CRenderState>& State)
{
	mMaterialSlot[SlotIndex]->SetBlendState(State);
}

void CMeshComponent::AddTexture(int SlotIndex, const std::weak_ptr<class CTexture>& Texture, int Register, int ShaderBufferType, int Index)
{
	mMaterialSlot[SlotIndex]->AddTexture(Texture, Register,
		ShaderBufferType, Index);
}

void CMeshComponent::AddTexture(int SlotIndex, const std::string& Name, int Register, int ShaderBufferType, int Index)
{
	auto	World = mWorld.lock();

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	std::weak_ptr<CTexture>	Texture = 
		AssetMgr->FindTexture(Name);

	mMaterialSlot[SlotIndex]->AddTexture(Texture, Register,
		ShaderBufferType, Index);
}

void CMeshComponent::AddTexture(int SlotIndex, const std::string& Name, const TCHAR* FileName, const std::string& PathName, int Register, int ShaderBufferType, int Index)
{
	auto	World = mWorld.lock();

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr->LoadTexture(Name, FileName, PathName))
		return;

	std::weak_ptr<CTexture>	Texture = 
		AssetMgr->FindTexture(Name);

	mMaterialSlot[SlotIndex]->AddTexture(Texture,
		Register, ShaderBufferType, Index);
}

void CMeshComponent::AddTextureFullPath(int SlotIndex, const std::string& Name, const TCHAR* FullPath, int Register, int ShaderBufferType, int Index)
{
	auto	World = mWorld.lock();

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr->LoadTextureFullPath(Name, FullPath))
		return;

	std::weak_ptr<CTexture>	Texture =
		AssetMgr->FindTexture(Name);

	mMaterialSlot[SlotIndex]->AddTexture(Texture,
		Register, ShaderBufferType, Index);
}

void CMeshComponent::AddTexture(int SlotIndex, const std::string& Name, const std::vector<const TCHAR*>& FileName, const std::string& PathName, int Register, int ShaderBufferType, int Index)
{
	auto	World = mWorld.lock();

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr->LoadTexture(Name, FileName, PathName))
		return;

	std::weak_ptr<CTexture>	Texture =
		AssetMgr->FindTexture(Name);

	mMaterialSlot[SlotIndex]->AddTexture(Texture,
		Register, ShaderBufferType, Index);
}

void CMeshComponent::AddTextureFullPath(int SlotIndex, const std::string& Name, const std::vector<const TCHAR*>& FullPath, int Register, int ShaderBufferType, int Index)
{
	auto	World = mWorld.lock();

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr->LoadTextureFullPath(Name, FullPath))
		return;

	std::weak_ptr<CTexture>	Texture =
		AssetMgr->FindTexture(Name);

	mMaterialSlot[SlotIndex]->AddTexture(Texture,
		Register, ShaderBufferType, Index);
}

bool CMeshComponent::SetTexture(int SlotIndex, int TextureIndex, const std::weak_ptr<class CTexture>& Texture)
{
	if ((int)mMaterialSlot.size() <= SlotIndex)
		return false;

	return mMaterialSlot[SlotIndex]->SetTexture(TextureIndex,
		Texture);
}

bool CMeshComponent::SetTextureIndex(int SlotIndex, int TextureIndex)
{
	if ((int)mMaterialSlot.size() <= SlotIndex)
		return false;

	//mMaterialSlot[SlotIndex]->

	return false;
}


void CMeshComponent::SetMaterialBaseColor(int SlotIndex,
	float r, float g, float b, float a)
{
	mMaterialSlot[SlotIndex]->SetBaseColor(r, g, b, a);
}

void CMeshComponent::SetMaterialBaseColor(int SlotIndex,
	int r, int g,
	int b, int a)
{
	mMaterialSlot[SlotIndex]->SetBaseColor(r, g, b, a);
}

void CMeshComponent::SetMaterialBaseColor(int SlotIndex,
	const FVector4& Color)
{
	mMaterialSlot[SlotIndex]->SetBaseColor(Color);
}

void CMeshComponent::SetMaterialOpacity(int SlotIndex, float Opacity)
{
	mMaterialSlot[SlotIndex]->SetOpacity(Opacity);
}

bool CMeshComponent::Init()
{
	CSceneComponent::Init();

	mTransformCBuffer.reset(new CCBufferTransform);

	mTransformCBuffer->Init();

	return true;
}

void CMeshComponent::Render()
{
	if (mShader.expired() || mMesh.expired())
		return;

	std::shared_ptr<CShader>    Shader = mShader.lock();
	std::shared_ptr<CMesh>    Mesh = mMesh.lock();

	FMatrix ViewMat;
	FMatrix ProjMat;

	auto	World = mWorld.lock();

	if (World)
	{
		auto	CameraMgr = World->GetCameraManager().lock();

		if (CameraMgr)
		{
			ViewMat = CameraMgr->GetViewMatrix();
			ProjMat = CameraMgr->GetProjMatrix();
		}
	}

	mTransformCBuffer->SetWorldMatrix(mWorldMatrix);
	mTransformCBuffer->SetViewMatrix(ViewMat);
	mTransformCBuffer->SetProjMatrix(ProjMat);

	FVector3	PivotSize = mPivot * Mesh->GetMeshSize();

	mTransformCBuffer->SetPivotSize(PivotSize);

	mTransformCBuffer->UpdateBuffer();

	Shader->SetShader();

	auto	Anim = mAnimComponent.lock();

	if (Anim)
		Anim->SetShader();

	else
	{
		mEmptyAnimCBuffer->UpdateBuffer();
	}

	// 슬롯 수만큼 반복하며 각각의 슬롯을 출력한다.
	size_t	Size = mMaterialSlot.size();

	for (size_t i = 0; i < Size; ++i)
	{
		// 애니메이션이 있을 경우
		if (!mAnimComponent.expired())
		{
			auto	Anim = mAnimComponent.lock();
						
			// Sprite Texture를 사용할 경우
			if (Anim->GetTextureType() == EAnimation2DTextureType::SpriteSheet)
			{
				if (mMaterialSlot[i])
					mMaterialSlot[i]->UpdateConstantBuffer();
			}

			// 낱장단위 Texture를 사용할 경우
			else if (Anim->GetTextureType() == EAnimation2DTextureType::Frame)
			{
				if (mMaterialSlot[i])
					mMaterialSlot[i]->UpdateConstantBuffer(Anim->GetAnimationFrame());
			}
		}

		else
		{
			if (mMaterialSlot[i])
				mMaterialSlot[i]->UpdateConstantBuffer();
		}

		Mesh->Render((int)i);

		if (mMaterialSlot[i])
			mMaterialSlot[i]->Reset();
	}

	CSceneComponent::Render();
}

CMeshComponent* CMeshComponent::Clone()	const
{
	return new CMeshComponent(*this);
}
