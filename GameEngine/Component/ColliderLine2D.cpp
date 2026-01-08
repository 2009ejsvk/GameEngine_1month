#include "ColliderLine2D.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Shader/ShaderManager.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Shader/CBufferCollider.h"
#include "../Asset/Mesh/MeshManager.h"
#include "../Asset/Mesh/Mesh.h"
#include "../World/World.h"
#include "../World/WorldAssetManager.h"
#include "Collision.h"
#include "ColliderBox2D.h"
#include "ColliderSphere2D.h"

CColliderLine2D::CColliderLine2D()
{
	mColliderType = EColliderType::Line2D;
}

CColliderLine2D::CColliderLine2D(const CColliderLine2D& ref) :
	CCollider(ref)
{
	mColliderType = EColliderType::Line2D;
}

CColliderLine2D::CColliderLine2D(CColliderLine2D&& ref) noexcept :
	CCollider(std::move(ref))
{
	mColliderType = EColliderType::Line2D;
}

CColliderLine2D::~CColliderLine2D()
{
}

void CColliderLine2D::SetDebugDraw(bool DebugDraw)
{
	CCollider::SetDebugDraw(DebugDraw);

	if (DebugDraw && mShader.expired())
	{
		std::shared_ptr<CShaderManager>   ShaderMgr =
			CAssetManager::GetInst()->GetShaderManager().lock();

		mShader = ShaderMgr->FindShader("Collider");

		auto	World = mWorld.lock();

		std::weak_ptr<CMesh>	Mesh;

		if (World)
		{
			auto	AssetMgr = World->GetWorldAssetManager().lock();

			mMesh = AssetMgr->FindMesh("LineUP2D");
		}

		else
		{
			std::weak_ptr<CMeshManager> Weak_MeshMgr =
				CAssetManager::GetInst()->GetMeshManager();

			std::shared_ptr<CMeshManager>   MeshMgr = Weak_MeshMgr.lock();

			mMesh = MeshMgr->FindMesh("Mesh_LineUP2D");
		}

		mColliderCBuffer.reset(new CCBufferCollider);

		mColliderCBuffer->Init();
	}
}

bool CColliderLine2D::Init()
{
	CCollider::Init();

	if (mDebugDraw)
	{
		std::shared_ptr<CShaderManager>   ShaderMgr =
			CAssetManager::GetInst()->GetShaderManager().lock();

		mShader = ShaderMgr->FindShader("Collider");

		auto	World = mWorld.lock();

		std::weak_ptr<CMesh>	Mesh;

		if (World)
		{
			auto	AssetMgr = World->GetWorldAssetManager().lock();

			mMesh = AssetMgr->FindMesh("LineUP2D");
		}

		else
		{
			std::weak_ptr<CMeshManager> Weak_MeshMgr =
				CAssetManager::GetInst()->GetMeshManager();

			std::shared_ptr<CMeshManager>   MeshMgr = Weak_MeshMgr.lock();

			mMesh = MeshMgr->FindMesh("Mesh_LineUP2D");
		}

		mColliderCBuffer.reset(new CCBufferCollider);

		mColliderCBuffer->Init();
	}

	return true;
}

void CColliderLine2D::Update(float DeltaTime)
{
	CCollider::Update(DeltaTime);
}

void CColliderLine2D::PostUpdate(float DeltaTime)
{
	CCollider::PostUpdate(DeltaTime);

	mInfo.Start = mWorldPos;

	// 회전된 선의 방향을 구한다.
	FVector3	Dir;

	Dir = mLineDir.TransformNormal(mRotMatrix);
	Dir.Normalize();

	mInfo.End = mInfo.Start + Dir * mDistance;

	mMin.x = mInfo.Start.x < mInfo.End.x ? mInfo.Start.x :
		mInfo.End.x;
	mMin.y = mInfo.Start.y < mInfo.End.y ? mInfo.Start.y :
		mInfo.End.y;

	mMax.x = mInfo.Start.x > mInfo.End.x ? mInfo.Start.x :
		mInfo.End.x;
	mMax.y = mInfo.Start.y > mInfo.End.y ? mInfo.Start.y :
		mInfo.End.y;

	mRenderScale.x = 1.f;
	mRenderScale.y = mDistance;
	mRenderScale.z = 1.f;
}

CColliderLine2D* CColliderLine2D::Clone()	const
{
	return new CColliderLine2D(*this);
}

bool CColliderLine2D::Collision(FVector3& HitPoint,
	std::shared_ptr<CCollider> Dest)
{
	// 상대방의 충돌체 모양이 무엇이냐에 따라 충돌 알고리즘이 달라진다.
	switch (Dest->GetColliderType())
	{
	case EColliderType::Box2D:
		return CCollision::CollisionBox2DToLine2D(HitPoint, 
			dynamic_cast<CColliderBox2D*>(Dest.get()), this);
		break;
	case EColliderType::Sphere2D:
		return CCollision::CollisionSphere2DToLine2D(HitPoint,
			dynamic_cast<CColliderSphere2D*>(Dest.get()), this);
	case EColliderType::Line2D:
		return CCollision::CollisionLine2DToLine2D(HitPoint,
			dynamic_cast<CColliderLine2D*>(Dest.get()), this);
	}

	return false;
}

