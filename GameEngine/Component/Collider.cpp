#include "Collider.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Shader/CBufferCollider.h"
#include "../Asset/Shader/CBufferTransform.h"
#include "../Asset/Mesh/Mesh.h"
#include "../World/World.h"
#include "../World/WorldCollision.h"
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
	auto	iter = mCollisionObjectMap.begin();
	auto	iterEnd = mCollisionObjectMap.end();

	for (; iter != iterEnd; ++iter)
	{
		if (iter->second.expired())
			continue;

		auto	Dest = iter->second.lock();

		Dest->EraseCollisionObject(this);
		//CallCollisionEnd(Dest.get());
		//Dest->CallCollisionEnd(this);
	}
}

bool CCollider::CheckCollisionObject(CCollider* Collider)
{
	auto iter = mCollisionObjectMap.find(Collider);

	if (iter == mCollisionObjectMap.end())
		return false;

	return true;
}

void CCollider::EraseCollisionObject(CCollider* Collider)
{
	auto	iter = mCollisionObjectMap.find(Collider);

	if (iter == mCollisionObjectMap.end())
		return;

	mCollisionObjectMap.erase(Collider);

	if (mCollisionObjectMap.empty())
		mCollision = false;
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

	auto	World = mWorld.lock();

	if (World)
	{
		auto	CollisionMgr = World->GetCollision().lock();

		if (CollisionMgr)
		{
			CollisionMgr->AddCollider(std::dynamic_pointer_cast<CCollider>(mSelf.lock()));
		}
	}

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

		auto	Mesh = mMesh.lock();

		FMatrix	ScaleMatrix, RotMatrix, TranslateMatrix, WorldMatrix;

		FVector3	RenderPos = mWorldPos;

		ScaleMatrix.Scaling(mRenderScale);
		RotMatrix.Rotation(mWorldRot);
		TranslateMatrix.Translation(RenderPos);

		WorldMatrix = ScaleMatrix * RotMatrix * TranslateMatrix;

		mTransformCBuffer->SetWorldMatrix(WorldMatrix);
		mTransformCBuffer->SetViewMatrix(ViewMat);
		mTransformCBuffer->SetProjMatrix(ProjMat);

		FVector3	PivotSize = mPivot * Mesh->GetMeshSize();

		mTransformCBuffer->SetPivotSize(PivotSize);

		mTransformCBuffer->UpdateBuffer();

		if (mCollision)
			mColliderCBuffer->SetColor(FVector4::Red);

		else
			mColliderCBuffer->SetColor(FVector4::Green);

		mColliderCBuffer->UpdateBuffer();

		auto	Shader = mShader.lock();

		Shader->SetShader();

		Mesh->Render();
	}
}

void CCollider::CallCollisionBegin(const FVector3& HitPoint, 
	const std::weak_ptr<CCollider>& Dest)
{
	auto	_Dest = Dest.lock();

	mCollisionObjectMap.insert(std::make_pair(_Dest.get(), Dest));

	mCollision = true;

	if (mCollisionBeginFunc)
		mCollisionBeginFunc(HitPoint, _Dest.get());
}

void CCollider::CallCollisionEnd(CCollider* Dest)
{
	mCollisionObjectMap.erase(Dest);

	if (mCollisionObjectMap.empty())
		mCollision = false;

	if (mCollisionEndFunc)
		mCollisionEndFunc(Dest);
}
