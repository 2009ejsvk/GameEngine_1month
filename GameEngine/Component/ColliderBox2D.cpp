#include "ColliderBox2D.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Shader/ShaderManager.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Shader/CBufferCollider.h"
#include "../Asset/Mesh/MeshManager.h"
#include "../Asset/Mesh/Mesh.h"
#include "../World/World.h"
#include "../World/WorldAssetManager.h"
#include "Collision.h"
#include "ColliderSphere2D.h"
#include "ColliderLine2D.h"

CColliderBox2D::CColliderBox2D()
{
	mColliderType = EColliderType::Box2D;
}

CColliderBox2D::CColliderBox2D(const CColliderBox2D& ref) :
	CCollider(ref)
{
	mColliderType = EColliderType::Box2D;
}

CColliderBox2D::CColliderBox2D(CColliderBox2D&& ref) noexcept :
	CCollider(std::move(ref))
{
	mColliderType = EColliderType::Box2D;
}

CColliderBox2D::~CColliderBox2D()
{
}

void CColliderBox2D::SetDebugDraw(bool DebugDraw)
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

			mMesh = AssetMgr->FindMesh("CenterFrameRect");
		}

		else
		{
			std::weak_ptr<CMeshManager> Weak_MeshMgr =
				CAssetManager::GetInst()->GetMeshManager();

			std::shared_ptr<CMeshManager>   MeshMgr = Weak_MeshMgr.lock();

			mMesh = MeshMgr->FindMesh("Mesh_CenterFrameRect");
		}

		mColliderCBuffer.reset(new CCBufferCollider);

		mColliderCBuffer->Init();
	}
}

bool CColliderBox2D::Init()
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

			mMesh = AssetMgr->FindMesh("Mesh_CenterFrameRect");
		}

		else
		{
			std::weak_ptr<CMeshManager> Weak_MeshMgr =
				CAssetManager::GetInst()->GetMeshManager();

			std::shared_ptr<CMeshManager>   MeshMgr = Weak_MeshMgr.lock();

			mMesh = MeshMgr->FindMesh("Mesh_CenterFrameRect");
		}

		mColliderCBuffer.reset(new CCBufferCollider);

		mColliderCBuffer->Init();
	}

	return true;
}

void CColliderBox2D::Update(float DeltaTime)
{
	CCollider::Update(DeltaTime);
}

void CColliderBox2D::PostUpdate(float DeltaTime)
{
	CCollider::PostUpdate(DeltaTime);

	mInfo.Center = mWorldPos;

	mInfo.Axis[EAxis::X] = mWorldAxis[EAxis::X];
	mInfo.Axis[EAxis::Y] = mWorldAxis[EAxis::Y];

	// 사각형을 구성하는 4개의 꼭지점을 구한다.
	FVector3	Pos[4];

	Pos[0] = mInfo.Center - mInfo.Axis[EAxis::X] * mInfo.HalfSize.x -
		mInfo.Axis[EAxis::Y] * mInfo.HalfSize.y;
	Pos[1] = mInfo.Center - mInfo.Axis[EAxis::X] * mInfo.HalfSize.x +
		mInfo.Axis[EAxis::Y] * mInfo.HalfSize.y;
	Pos[2] = mInfo.Center + mInfo.Axis[EAxis::X] * mInfo.HalfSize.x -
		mInfo.Axis[EAxis::Y] * mInfo.HalfSize.y;
	Pos[3] = mInfo.Center + mInfo.Axis[EAxis::X] * mInfo.HalfSize.x +
		mInfo.Axis[EAxis::Y] * mInfo.HalfSize.y;

	mMin = Pos[0];
	mMax = Pos[0];

	for (int i = 1; i < 4; ++i)
	{
		// 최소값으로 지정된 x가 점의 x보다 클 경우 교체한다.
		mMin.x = mMin.x > Pos[i].x ? Pos[i].x : mMin.x;
		mMin.y = mMin.y > Pos[i].y ? Pos[i].y : mMin.y;

		mMax.x = mMax.x < Pos[i].x ? Pos[i].x : mMax.x;
		mMax.y = mMax.y < Pos[i].y ? Pos[i].y : mMax.y;
	}

	mRenderScale.x = mWorldScale.x * mInfo.HalfSize.x * 2.f;
	mRenderScale.y = mWorldScale.y * mInfo.HalfSize.y * 2.f;
	mRenderScale.z = 1.f;
}

CColliderBox2D* CColliderBox2D::Clone()	const
{
	return new CColliderBox2D(*this);
}

bool CColliderBox2D::Collision(FVector3& HitPoint,
	std::shared_ptr<CCollider> Dest)
{
	// 상대방의 충돌체 모양이 무엇이냐에 따라 충돌 알고리즘이 달라진다.
	switch (Dest->GetColliderType())
	{
	case EColliderType::Box2D:
		// 둘다 회전이 0일 경우 AABB, 아니면 OBB 충돌을 진행한다.
		return CCollision::CollisionBox2DToBox2D(HitPoint, this,
			dynamic_cast<CColliderBox2D*>(Dest.get()));
	case EColliderType::Sphere2D:
		return CCollision::CollisionBox2DToSphere2D(HitPoint,
			this, dynamic_cast<CColliderSphere2D*>(Dest.get()));
	case EColliderType::Line2D:
		return CCollision::CollisionBox2DToLine2D(HitPoint, this,
			dynamic_cast<CColliderLine2D*>(Dest.get()));
	}

	return false;
}
