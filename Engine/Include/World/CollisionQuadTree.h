#pragma once

#include "CollisionQuadTreeNode.h"

class CCollisionQuadTree
{
	friend class CWorldCollision;

private:
	CCollisionQuadTree();

public:
	~CCollisionQuadTree();

private:
	std::weak_ptr<CCollisionQuadTree>	mSelf;
	std::weak_ptr<class CWorld>	mWorld;
	std::shared_ptr<CCollisionQuadTreeNode>	mRoot;
	std::vector<std::shared_ptr<CCollisionQuadTreeNode>>	mNodePool;
	std::vector<std::weak_ptr<CCollisionQuadTreeNode>>	mCollisionNodeList;
	std::vector<std::weak_ptr<CCollisionQuadTreeNode>>	mMouseCollisionNodeList;

#ifdef _DEBUG

	std::weak_ptr<class CShader>	mShader;
	std::weak_ptr<class CMesh>		mMesh;
	std::shared_ptr<class CCBufferCollider>	mColliderCBuffer;

#endif // _DEBUG

public:
	void SetWorld(const std::weak_ptr<class CWorld>& World);
	void AddCollisionNodeList(
		const std::weak_ptr<CCollisionQuadTreeNode>& Node);
	void EraseCollisionNodeList(
		const std::weak_ptr<CCollisionQuadTreeNode>& Node);
	void AddMouseCollisionNodeList(
		const std::weak_ptr<CCollisionQuadTreeNode>& Node);
	void EraseMouseCollisionNodeList(
		const std::weak_ptr<CCollisionQuadTreeNode>& Node);

public:
	bool Init();
	void AddCollider(const std::weak_ptr<class CCollider>& Collider);
	void Update(float DeltaTime);
	void Collision(float DeltaTime);
	void CollisionMouse(std::weak_ptr<class CCollider>& Result,
		float DeltaTime, const FVector2& MousePos);
	void Render();
	void ReturnNodePool();

private:
	void UpdateInfo();
};

