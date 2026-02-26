#pragma once

#include "../EngineInfo.h"

#define	QUADTREE_DIVISION_COUNT		5
#define	QUADTREE_DEPTH_MAX			4

class CCollisionQuadTreeNode
{
	friend class CCollisionQuadTree;

private:
	CCollisionQuadTreeNode();

public:
	~CCollisionQuadTreeNode();

private:
	std::weak_ptr<class CWorld>	mWorld;
	std::weak_ptr<class CCollisionQuadTreeNode>	mSelf;
	std::weak_ptr<class CCollisionQuadTreeNode>	mParent;
	std::weak_ptr<class CCollisionQuadTree> mTree;
	std::shared_ptr<CCollisionQuadTreeNode> mChild[4];
	FVector3	mCenter;
	FVector3	mSize;
	int			mDepth = 0;
	std::list<std::weak_ptr<class CCollider>>	mColliderList;

#ifdef _DEBUG

	std::shared_ptr<class CCBufferTransform>	mTransformCBuffer;

#endif // _DEBUG

public:
	void SetWorld(const std::weak_ptr<class CWorld>& World)
	{
		mWorld = World;
	}

	void SetTree(const std::weak_ptr<class CCollisionQuadTree>& Tree)
	{
		mTree = Tree;
	}

public:
	void AddCollider(const std::weak_ptr<class CCollider>& Collider,
		std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool);
	void Collision(float DeltaTime);
	bool CollisionMouse(std::weak_ptr<class CCollider>& Result,
		float DeltaTime, const FVector2& MousePos);
	void Render(const std::weak_ptr<class CMesh>& Mesh,
		const std::weak_ptr<class CShader>& Shader);
	void ReturnNodePool(
		std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool);

private:
	bool IsInCollider(const std::weak_ptr<class CCollider>& Collider);
	bool IsInCollider(const FVector2& MousePos);
	void CreateChild(
		std::vector<std::shared_ptr<CCollisionQuadTreeNode>>& NodePool);
	static bool SortMouseCollision(
		const std::weak_ptr<class CCollider>& Src,
		const std::weak_ptr<class CCollider>& Dest);
};

