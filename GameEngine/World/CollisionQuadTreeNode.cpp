#include "CollisionQuadTreeNode.h"
#include "../Component/Collider.h"
#include "CollisionQuadTree.h"
#include "../Object/GameObject.h"
#include "../Component/Collision.h"
#include "../Asset/Shader/CBufferTransform.h"
#include "World.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Shader/CBufferTransform.h"

CCollisionQuadTreeNode::CCollisionQuadTreeNode()
{
#ifdef _DEBUG

	mTransformCBuffer.reset(new CCBufferTransform);

	mTransformCBuffer->Init();

#endif // _DEBUG
}

CCollisionQuadTreeNode::~CCollisionQuadTreeNode()
{
}

void CCollisionQuadTreeNode::AddCollider(
	const std::weak_ptr<class CCollider>& Collider, 
	std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool)
{
	// 충돌체가 현재 노드 안에 포함되었는지를 판단한다.
	if (!IsInCollider(Collider))
		return;

	if (!mChild[0])
	{
		mColliderList.push_back(Collider);

		auto	Tree = mTree.lock();

		// 2개 이상을 가지게 되는 노드는 실제 충돌을 수행해야 하는 노드
		// 이기 때문에 Tree에서 따로 노드를 가지고 있게 한다.
		if (mColliderList.size() == 2)
		{
			Tree->AddCollisionNodeList(mSelf);
		}

		else if (mColliderList.size() == 1)
		{
			Tree->AddMouseCollisionNodeList(mSelf);
		}

		// 충돌 목록에 추가된 개수가 노드를 분할해야할 개수 이상인지 판단.
		// 공간을 너무 많이 분할하면 오히려 느려질 수 있기 때문에
		// 최대 분할개수를 정해둔다.
		if (mColliderList.size() >= QUADTREE_DIVISION_COUNT &&
			mDepth < QUADTREE_DEPTH_MAX)
		{
			// 노드를 분할한다.
			CreateChild(NodePool);

			// 4개의 노드가 분할되었으므로 현재 노드가 가지고 있는
			// 충돌체들을 각 Child에 분배해준다.
			auto	iter = mColliderList.begin();
			auto	iterEnd = mColliderList.end();

			for (; iter != iterEnd; ++iter)
			{
				// 충돌체가 자식노드 4개 중 여러개에 겹쳐있을 수 있기
				// 때문에 노드와 충돌체가 겹쳐있는지 판단한다.
				for (int j = 0; j < 4; ++j)
				{
					if (mChild[j]->IsInCollider(*iter))
					{
						mChild[j]->AddCollider(*iter, NodePool);
					}
				}
			}

			// 자식노드에 분배가 끝났다면 목록을 비워준다.
			mColliderList.clear();

			// 현재노드는 충돌노드였다가 자식에 충돌체를 분배해주고
			// 비워졌기 때문에 더이상 충돌노드가 아니다.
			Tree->EraseCollisionNodeList(mSelf);

			Tree->EraseMouseCollisionNodeList(mSelf);
		}
	}

	// 자식노드가 이미 있을 경우
	else
	{
		for (int j = 0; j < 4; ++j)
		{
			mChild[j]->AddCollider(Collider, NodePool);
		}
	}
}

void CCollisionQuadTreeNode::Collision(float DeltaTime)
{
	auto	iter = mColliderList.begin();
	auto	iterEnd = mColliderList.end();
	--iterEnd;

	auto	iterEnd1 = mColliderList.end();

	auto	iter2 = mColliderList.end();
	--iter2;

	for (; iter != iterEnd;)
	{
		auto	SrcCollider = (*iter).lock();

		if (!SrcCollider->GetAlive())
		{
			iter = mColliderList.erase(iter);
			iterEnd = mColliderList.end();
			iterEnd1 = iterEnd;
			--iterEnd;
			continue;
		}

		else if (!SrcCollider->GetEnable())
		{
			++iter;
			continue;
		}

		FCollisionProfile* SrcProfile = SrcCollider->GetCollisionProfile();

		if (!SrcProfile->Enable)
		{
			++iter;
			continue;
		}

		auto	iter1 = iter;
		++iter1;

		for (; iter1 != iterEnd1;)
		{
			auto	DestCollider = (*iter1).lock();

			if (!DestCollider->GetAlive())
			{
				iter1 = mColliderList.erase(iter1);
				iterEnd = mColliderList.end();
				iterEnd1 = iterEnd;
				--iterEnd;
				continue;
			}

			else if (!DestCollider->GetEnable())
			{
				++iter1;
				continue;
			}

			// 두 물체의 충돌처리를 진행한다.
			// 두 물체의 충돌 프로파일을 비교한다.
			FCollisionProfile* DestProfile = DestCollider->GetCollisionProfile();

			if (!DestProfile->Enable)
			{
				++iter1;
				continue;
			}

			// 상대방 채널에 대해 충돌이 무시일 경우 충돌처리를 건너뛴다.
			else if (SrcProfile->Interaction[DestProfile->Channel->Channel] == ECollisionInteraction::Ignore ||
				DestProfile->Interaction[SrcProfile->Channel->Channel] == ECollisionInteraction::Ignore)
			{
				++iter1;
				continue;
			}

			// Overlap vs Block 처리 못함. 서로 같을 경우에만 처리.
			else if (SrcProfile->Interaction[DestProfile->Channel->Channel] !=
				DestProfile->Interaction[SrcProfile->Channel->Channel])
			{
				++iter1;
				continue;
			}

			// 실제 충돌처리를 진행한다.
			FVector3	HitPoint, BlockMove;

			// Collision함수의 반환값이 true일 경우 충돌 되었다는
			// 의미이다.
			if (SrcCollider->Collision(HitPoint, DestCollider))
			{
				// Hit 충돌의 경우 충돌 되었다가 떨어지는 것이 없기
				// 때문에 Hit일 경우를 체크하여 바로 진행한다.
				if (SrcProfile->Interaction[DestProfile->Channel->Channel] == ECollisionInteraction::Block)
				{
					// 둘중 이동하고 있던 물체를 찾는다.
					// RootComponent를 기준으로 찾는다.
					// 만약 둘 다 이동하고 있던 물체라면 두 물체는
					// 겹치는 구간의 반씩 반대방향으로 이동한다.
					auto SrcObj = SrcCollider->GetOwner().lock();
					auto DestObj = DestCollider->GetOwner().lock();

					//// 현재 프레임에 얼마나 움직였는지를 가지고 온다.
					FVector3	SrcVelocity = SrcObj->GetVelocity();
					FVector3	DestVelocity = DestObj->GetVelocity();

					FVector3	SrcPos = SrcObj->GetWorldPos();
					FVector3	DestPos = DestObj->GetWorldPos();
					
					FCollisionManifold	Manifold;
					
					// Manifold의 Normal은 Src가 밀려나가는 방향을 기준
					// 으로 만들어진다.
					if (SrcCollider->CollisionManifold(Manifold,
						DestCollider))
					{
						float	SrcInvMass = 0.f, DestInvMass = 0.f;

						if (!SrcCollider->GetStatic())
							SrcInvMass = 1.f;

						if (!DestCollider->GetStatic())
							DestInvMass = 1.f;

						if (CCollision::ResolveSlideStop2D(SrcPos,
							SrcVelocity, SrcInvMass,
							DestPos, DestVelocity,
							DestInvMass, Manifold))
						{
							SrcObj->ClearPhysics();
							DestObj->ClearPhysics();
						}

						SrcObj->SetWorldPos(SrcPos);
						DestObj->SetWorldPos(DestPos);
					}

					//HitPoint = Manifold.ContactPoint;

					SrcCollider->CallCollisionHit(HitPoint,
						*iter1);

					DestCollider->CallCollisionHit(HitPoint,
						*iter);
				}

				// Overlap일 경우
				else
				{
					if (!SrcCollider->CheckCollisionObject(
						DestCollider.get()))
					{
						SrcCollider->CallCollisionBegin(HitPoint,
							*iter1);

						DestCollider->CallCollisionBegin(HitPoint,
							*iter);
					}
				}
			}

			// 두 물체가 현재 프레임에는 부딪히지 않았지만 이전프레임 까지는
			// 부딪히고 있었을 경우는 이제 막 떨어지는 경우이다.
			else if (SrcCollider->CheckCollisionObject(
				DestCollider.get()))
			{
				if (SrcProfile->Interaction[DestProfile->Channel->Channel] == ECollisionInteraction::Overlap)
				{
					SrcCollider->CallCollisionEnd(DestCollider.get());

					DestCollider->CallCollisionEnd(SrcCollider.get());
				}

				else
				{
				}
			}

			++iter1;
		}

		iter2 = mColliderList.end();
		--iter2;
		if (iter == iter2)
		{
			continue;
		}

		++iter;
	}
}

bool CCollisionQuadTreeNode::CollisionMouse(
	std::weak_ptr<class CCollider>& Result, 
	float DeltaTime,
	const FVector2& MousePos)
{
	if (mColliderList.size() >= 2)
		mColliderList.sort(CCollisionQuadTreeNode::SortMouseCollision);

	auto	iter = mColliderList.begin();
	auto	iterEnd = mColliderList.end();

	for (; iter != iterEnd;)
	{
		if ((*iter).expired())
		{
			iter = mColliderList.erase(iter);
			iterEnd = mColliderList.end();
			continue;
		}

		auto	SrcCollider = (*iter).lock();

		if (!SrcCollider->GetAlive())
		{
			iter = mColliderList.erase(iter);
			iterEnd = mColliderList.end();
			continue;
		}

		else if (!SrcCollider->GetEnable())
		{
			++iter;
			continue;
		}

		if (SrcCollider->CollisionMouse(MousePos))
		{
			auto	ResultObj = Result.lock();

			if (ResultObj != SrcCollider)
			{
				FVector3	ConvertPos(MousePos.x, MousePos.y,
					0.f);

				if (ResultObj)
				{
					ResultObj->CallCollisionMouseEnd(ConvertPos);
				}

				Result = *iter;

				SrcCollider->CallCollisionMouseBegin(ConvertPos);
			}

			return true;
		}

		++iter;
	}

	return false;
}

void CCollisionQuadTreeNode::Render(
	const std::weak_ptr<class CMesh>& Mesh, 
	const std::weak_ptr<class CShader>& Shader)
{
#ifdef _DEBUG

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

	auto	RenderMesh = Mesh.lock();

	FMatrix	ScaleMatrix, TranslateMatrix, WorldMatrix;

	ScaleMatrix.Scaling(mSize);
	TranslateMatrix.Translation(mCenter);

	WorldMatrix = ScaleMatrix * TranslateMatrix;

	mTransformCBuffer->SetWorldMatrix(WorldMatrix);
	mTransformCBuffer->SetViewMatrix(ViewMat);
	mTransformCBuffer->SetProjMatrix(ProjMat);

	mTransformCBuffer->UpdateBuffer();

	auto	RenderShader = Shader.lock();

	RenderShader->SetShader();

	RenderMesh->Render();

	if (mChild[0])
	{
		for (int i = 0; i < 4; ++i)
		{
			mChild[i]->Render(Mesh, Shader);
		}
	}

#endif // _DEBUG
}

void CCollisionQuadTreeNode::ReturnNodePool(
	std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool)
{
	// 충돌 목록을 비워준다.
	mColliderList.clear();

	if (mChild[0])
	{
		for (int i = 0; i < 4; ++i)
		{
			NodePool.push_back(mChild[i]);

			mChild[i]->ReturnNodePool(NodePool);

			mChild[i].reset();
		}
	}
}

bool CCollisionQuadTreeNode::IsInCollider(
	const std::weak_ptr<class CCollider>& Collider)
{
	auto	Src = Collider.lock();

	FVector3	Min, Max;

	Min = Src->GetMin();
	Max = Src->GetMax();

	FVector3	NodeMin, NodeMax;

	NodeMin = mCenter - mSize * 0.5f;
	NodeMax = NodeMin + mSize;

	if (Min.x > NodeMax.x)
		return false;

	else if (Max.x < NodeMin.x)
		return false;

	else if (Min.y > NodeMax.y)
		return false;

	else if (Max.y < NodeMin.y)
		return false;

	return true;
}

bool CCollisionQuadTreeNode::IsInCollider(const FVector2& MousePos)
{
	FVector3	NodeMin, NodeMax;

	NodeMin = mCenter - mSize * 0.5f;
	NodeMax = NodeMin + mSize;

	if (MousePos.x < NodeMin.x)
		return false;

	else if (MousePos.x > NodeMax.x)
		return false;

	else if (MousePos.y < NodeMin.y)
		return false;

	else if (MousePos.y > NodeMax.y)
		return false;

	return true;
}

void CCollisionQuadTreeNode::CreateChild(
	std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool)
{
	if (NodePool.empty())
	{
		NodePool.resize(4);

		NodePool[0].reset(new CCollisionQuadTreeNode);
		NodePool[1].reset(new CCollisionQuadTreeNode);
		NodePool[2].reset(new CCollisionQuadTreeNode);
		NodePool[3].reset(new CCollisionQuadTreeNode);
	}

	for (int i = 0; i < 4; ++i)
	{
		mChild[i] = NodePool.back();

		NodePool.pop_back();

		mChild[i]->SetWorld(mWorld);
		mChild[i]->mSelf = mChild[i];
		mChild[i]->mTree = mTree;
		mChild[i]->mParent = mSelf;
		mChild[i]->mDepth = mDepth + 1;

		mChild[i]->mSize = mSize * 0.5f;

		if (i % 2 == 0)
			mChild[i]->mCenter.x = mCenter.x - mSize.x * 0.25f;

		else
			mChild[i]->mCenter.x = mCenter.x + mSize.x * 0.25f;

		if(i <= 1)
			mChild[i]->mCenter.y = mCenter.y + mSize.y * 0.25f;

		else
			mChild[i]->mCenter.y = mCenter.y - mSize.y * 0.25f;
	}
}

bool CCollisionQuadTreeNode::SortMouseCollision(
	const std::weak_ptr<class CCollider>& Src,
	const std::weak_ptr<class CCollider>& Dest)
{
	auto	_Src = Src.lock();
	auto	_Dest = Dest.lock();

	float	SrcY = _Src->GetWorldPos().y;
	float	DestY = _Dest->GetWorldPos().y;

	return SrcY < DestY;
}
