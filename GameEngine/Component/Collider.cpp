#include "Collider.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Shader/CBufferCollider.h"
#include "../Asset/Shader/CBufferTransform.h"
#include "../Asset/Mesh/Mesh.h"
#include "../World/World.h"
#include "../World/CameraManager.h"
#include "../Render/RenderManager.h"
#include "../CollisionInfoManager.h"

CCollider::CCollider()
{
}

CCollider::CCollider(const CCollider& ref)	:
	CSceneComponent(ref)
{
}

CCollider::CCollider(CCollider&& ref) noexcept :
	CSceneComponent(std::move(ref))
{
}

CCollider::~CCollider()
{
}

void CCollider::SetDebugDraw(bool DebugDraw)
{
	mDebugDraw = DebugDraw;

	if (mDebugDraw)
	{
		mRenderType = EComponentRender::Render;

		auto	self = std::dynamic_pointer_cast<CSceneComponent>(mSelf.lock());

		CRenderManager::GetInst()->AddRenderLayer(self);

		mTransformCBuffer.reset(new CCBufferTransform);

		mTransformCBuffer->Init();
	}
}

void CCollider::SetCollisionProfile(const std::string& Name)
{
	mProfile = CCollisionInfoManager::GetInst()->FindProfile(Name);
}

bool CCollider::Init()
{
	CSceneComponent::Init();

	if (mDebugDraw)
	{
		mTransformCBuffer.reset(new CCBufferTransform);

		mTransformCBuffer->Init();
	}
	
	mProfile = CCollisionInfoManager::GetInst()->FindProfile("Static");

	return true;
}

void CCollider::Update(float DeltaTime)
{
	CSceneComponent::Update(DeltaTime);
}

void CCollider::PostUpdate(float DeltaTime)
{
	CSceneComponent::PostUpdate(DeltaTime);
}

void CCollider::Render()
{
	CSceneComponent::Render();

	if (mDebugDraw)
	{
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

		FMatrix	ScaleMatrix, RotMatrix, TranslateMatrix, WorldMatrix;

		ScaleMatrix.Scaling(mRenderScale);
		RotMatrix.Rotation(mWorldRot);
		TranslateMatrix.Translation(mWorldPos);

		WorldMatrix = ScaleMatrix * RotMatrix * TranslateMatrix;

		mTransformCBuffer->SetWorldMatrix(WorldMatrix);
		mTransformCBuffer->SetViewMatrix(ViewMat);
		mTransformCBuffer->SetProjMatrix(ProjMat);

		mTransformCBuffer->UpdateBuffer();

		if (mCollision)
			mColliderCBuffer->SetColor(FVector4::Red);

		else
			mColliderCBuffer->SetColor(FVector4::Green);

		mColliderCBuffer->UpdateBuffer();

		auto	Shader = mShader.lock();

		Shader->SetShader();

		auto	Mesh = mMesh.lock();

		Mesh->Render();
	}
}
