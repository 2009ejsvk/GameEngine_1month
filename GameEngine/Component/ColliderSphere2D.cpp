#include "ColliderSphere2D.h"
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

CColliderSphere2D::CColliderSphere2D()
{
	mColliderType = EColliderType::Sphere2D;
}

CColliderSphere2D::CColliderSphere2D(const CColliderSphere2D& ref) :
	CCollider(ref)
{
	mColliderType = EColliderType::Sphere2D;
}

CColliderSphere2D::CColliderSphere2D(CColliderSphere2D&& ref) noexcept :
	CCollider(std::move(ref))
{
	mColliderType = EColliderType::Sphere2D;
}

CColliderSphere2D::~CColliderSphere2D()
{
}

void CColliderSphere2D::SetDebugDraw(bool DebugDraw)
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

			mMesh = AssetMgr->FindMesh("FrameSphere2D");
		}

		else
		{
			std::weak_ptr<CMeshManager> Weak_MeshMgr =
				CAssetManager::GetInst()->GetMeshManager();

			std::shared_ptr<CMeshManager>   MeshMgr = Weak_MeshMgr.lock();

			mMesh = MeshMgr->FindMesh("Mesh_FrameSphere2D");
		}

		mColliderCBuffer.reset(new CCBufferCollider);

		mColliderCBuffer->Init();
	}
}

bool CColliderSphere2D::Init()
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

			mMesh = AssetMgr->FindMesh("FrameSphere2D");
		}

		else
		{
			std::weak_ptr<CMeshManager> Weak_MeshMgr =
				CAssetManager::GetInst()->GetMeshManager();

			std::shared_ptr<CMeshManager>   MeshMgr = Weak_MeshMgr.lock();

			mMesh = MeshMgr->FindMesh("Mesh_FrameSphere2D");
		}

		mColliderCBuffer.reset(new CCBufferCollider);

		mColliderCBuffer->Init();
	}

	return true;
}

void CColliderSphere2D::Update(float DeltaTime)
{
	CCollider::Update(DeltaTime);
}

void CColliderSphere2D::PostUpdate(float DeltaTime)
{
	CCollider::PostUpdate(DeltaTime);

	mInfo.Center = mWorldPos + mOffset;

	mRenderScale.x = mWorldScale.x * mInfo.Radius;
	mRenderScale.y = mWorldScale.y * mInfo.Radius;
	mRenderScale.z = 1.f;
}

CColliderSphere2D* CColliderSphere2D::Clone()	const
{
	return new CColliderSphere2D(*this);
}

bool CColliderSphere2D::Collision(FVector3& HitPoint,
	std::shared_ptr<CCollider> Dest)
{
	// 상대방의 충돌체 모양이 무엇이냐에 따라 충돌 알고리즘이 달라진다.
	switch (Dest->GetColliderType())
	{
	case EColliderType::Box2D:
		// 둘다 회전이 0일 경우 AABB, 아니면 OBB 충돌을 진행한다.
		/*return CCollision::CollisionBox2DToBox2D(HitPoint, this,
			dynamic_cast<CColliderSphere2D*>(Dest.get()));*/
		break;
	case EColliderType::Sphere2D:
		break;
	}

	return false;
}
