#include "CollisionQuadTree.h"
#include "World.h"
#include "CameraManager.h"
#include "../Device.h"
#include "../Asset/Shader/ShaderManager.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Shader/CBufferCollider.h"
#include "../Asset/AssetManager.h"

CCollisionQuadTree::CCollisionQuadTree()
{
}

CCollisionQuadTree::~CCollisionQuadTree()
{
}

void CCollisionQuadTree::SetWorld(
	const std::weak_ptr<class CWorld>& World)
{
	mWorld = World;
}

void CCollisionQuadTree::AddCollisionNodeList(
	const std::weak_ptr<CCollisionQuadTreeNode>& Node)
{
	mCollisionNodeList.push_back(Node);
}

void CCollisionQuadTree::EraseCollisionNodeList(
	const std::weak_ptr<CCollisionQuadTreeNode>& Node)
{
	auto	iter = mCollisionNodeList.begin();
	auto	iterEnd = mCollisionNodeList.end();

	auto	EraseNode = Node.lock();

	for(; iter != iterEnd; ++iter)
	{
		auto	iterNode = (*iter).lock();

		if (iterNode == EraseNode)
		{
			mCollisionNodeList.erase(iter);
			break;
		}
	}
}

void CCollisionQuadTree::AddMouseCollisionNodeList(
	const std::weak_ptr<CCollisionQuadTreeNode>& Node)
{
	mMouseCollisionNodeList.push_back(Node);
}

void CCollisionQuadTree::EraseMouseCollisionNodeList(
	const std::weak_ptr<CCollisionQuadTreeNode>& Node)
{
	auto	iter = mMouseCollisionNodeList.begin();
	auto	iterEnd = mMouseCollisionNodeList.end();

	auto	EraseNode = Node.lock();

	for (; iter != iterEnd; ++iter)
	{
		auto	iterNode = (*iter).lock();

		if (iterNode == EraseNode)
		{
			mMouseCollisionNodeList.erase(iter);
			break;
		}
	}
}

bool CCollisionQuadTree::Init()
{
#ifdef _DEBUG

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

	mColliderCBuffer->SetColor(FVector4::Blue);

#endif // _DEBUG

	return true;
}

void CCollisionQuadTree::AddCollider(
	const std::weak_ptr<class CCollider>& Collider)
{
	if (!mRoot)
	{
		mRoot.reset(new CCollisionQuadTreeNode);

		mRoot->SetWorld(mWorld);
		mRoot->SetTree(mSelf);

		auto	World = mWorld.lock();

		auto	CameraMgr = World->GetCameraManager().lock();

		mRoot->mCenter = CameraMgr->GetMainCameraWorldPos();

		const FResolution& RS = CDevice::GetInst()->GetResolution();

		mRoot->mSize.x = RS.Width * 1.5f;
		mRoot->mSize.y = RS.Height * 1.5f;

		mRoot->mSelf = mRoot;
	}

	mRoot->AddCollider(Collider, mNodePool);
}

void CCollisionQuadTree::Update(float DeltaTime)
{
	if (mRoot)
	{
		auto	World = mWorld.lock();

		auto	CameraMgr = World->GetCameraManager().lock();

		mRoot->mCenter = CameraMgr->GetMainCameraWorldPos();
	}
}

void CCollisionQuadTree::Collision(float DeltaTime)
{
	auto	iter = mCollisionNodeList.begin();
	auto	iterEnd = mCollisionNodeList.end();

	for (; iter != iterEnd; ++iter)
	{
		auto	Node = (*iter).lock();

		Node->Collision(DeltaTime);
	}

	// 충돌이 모두 완료되고 나면 다음 프레임에도 검사해야 하기 때문에
	// 비워준다.
	mCollisionNodeList.clear();
}

void CCollisionQuadTree::CollisionMouse(
	std::weak_ptr<class CCollider>& Result, 
	float DeltaTime,
	const FVector2& MousePos)
{
	// 마우스 위치가 소속된 충돌 노드를 얻어온다.
	auto	iter = mMouseCollisionNodeList.begin();
	auto	iterEnd = mMouseCollisionNodeList.end();

	for (; iter != iterEnd; ++iter)
	{
		auto	Node = (*iter).lock();

		if (Node->IsInCollider(MousePos))
		{
			if (Node->CollisionMouse(Result, DeltaTime, MousePos))
			{
				mMouseCollisionNodeList.clear();
				return;
			}
		}
	}

	mMouseCollisionNodeList.clear();
}

void CCollisionQuadTree::Render()
{
#ifdef _DEBUG

	if (mRoot)
	{
		mColliderCBuffer->UpdateBuffer();

		mRoot->Render(mMesh, mShader);
	}

#endif // _DEBUG
}

void CCollisionQuadTree::ReturnNodePool()
{
	if (mRoot)
	{
		mRoot->ReturnNodePool(mNodePool);
	}
}

void CCollisionQuadTree::UpdateInfo()
{
}
