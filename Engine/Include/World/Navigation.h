#pragma once

#include "../EngineInfo.h"

namespace ESearchDir
{
	enum Type
	{
		T,
		RT,
		R,
		RB,
		B,
		LB,
		L,
		LT,
		End
	};
}

enum class ENavNodeType
{
	None,
	Open,
	Close
};

struct FNavNode
{
	FNavNode* Parent = nullptr;
	ENavNodeType	NodeType = ENavNodeType::None;
	FVector2	Pos;
	FVector2	Size;
	FVector2	Center;
	int			IndexX = 0;
	int			IndexY = 0;
	int			Index = 0;
	float		Dist = FLT_MAX;
	float		Huristic = FLT_MAX;
	float		Total = FLT_MAX;
	std::list<ESearchDir::Type>	SearchDirList;
};

class CNavigation
{
	friend class CWorld;
	friend class CThreadNavigation;

private:
	CNavigation();

public:
	~CNavigation();

private:
	std::weak_ptr<class CTileMapComponent>	mTileMap;
	std::vector<FNavNode>	mNodeList;
	ETileShape	mShape = ETileShape::Rect;
	int		mCountX = 0;
	int		mCountY = 0;
	float	mDiagonalCost = 0.f;
	FVector2	mTileSize;
	std::vector<FNavNode*>	mOpenList;
	std::vector<FNavNode*>	mUseList;

public:
	void SetTileMap(const std::weak_ptr<class CTileMapComponent>& TileMap);
	bool FindPath(const FVector3& Start, const FVector3& End,
		std::list<FVector3>& PathList);

private:
	bool FindNode(FNavNode* Node, FNavNode* EndNode,
		const FVector3& End, std::list<FVector3>& PathList);
	FNavNode* GetCorner(ESearchDir::Type Dir, FNavNode* Node,
		FNavNode* EndNode);
	void AddDir(ESearchDir::Type Dir, FNavNode* Node);
	FNavNode* GetCornerRectT(FNavNode* Node, FNavNode* EndNode);
	FNavNode* GetCornerRectRT(FNavNode* Node, FNavNode* EndNode);
	FNavNode* GetCornerRectR(FNavNode* Node, FNavNode* EndNode);
	FNavNode* GetCornerRectRB(FNavNode* Node, FNavNode* EndNode);
	FNavNode* GetCornerRectB(FNavNode* Node, FNavNode* EndNode);
	FNavNode* GetCornerRectLB(FNavNode* Node, FNavNode* EndNode);
	FNavNode* GetCornerRectL(FNavNode* Node, FNavNode* EndNode);
	FNavNode* GetCornerRectLT(FNavNode* Node, FNavNode* EndNode);

	static bool SortOpenList(FNavNode* Src, FNavNode* Dest);
};

