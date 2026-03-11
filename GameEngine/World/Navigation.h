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
	int			GridX = 0;
	int			GridY = 0;
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
	FVector2	mTileSize;
	std::vector<FNavNode*>	mOpenList;
	std::vector<FNavNode*>	mUseList;
	std::vector<unsigned char>	mBlockedMask;
	std::vector<unsigned char>	mGoalMask;
	std::vector<int>	mGoalIndices;
	int		mStartIndex = -1;
	bool	mUseJumpPointSearch = false;

public:
	void SetTileMap(const std::weak_ptr<class CTileMapComponent>& TileMap);
	bool FindPath(const FVector3& Start, const FVector3& End,
		std::list<FVector3>& PathList,
		const unsigned char* BlockMask = nullptr,
		int BlockMaskByteCount = 0,
		const int* GoalIndices = nullptr,
		int GoalCount = 0);

private:
	bool FindNode(FNavNode* Node, std::list<FVector3>& PathList);
	int Jump(FNavNode* Node, ESearchDir::Type Dir);
	void AddDir(ESearchDir::Type ParentDir, FNavNode* Node,
		bool HasParent);
	bool HasForcedNeighbor(int Index, ESearchDir::Type Dir) const;
	bool IsDiagonalDir(ESearchDir::Type Dir) const;
	bool GetMoveDelta(ESearchDir::Type Dir, int& OutDx, int& OutDy) const;
	ESearchDir::Type GetDirFromDelta(int dx, int dy) const;
	ESearchDir::Type GetParentDir(FNavNode* Node) const;
	int GetIndexByGrid(int GridX, int GridY) const;
	bool StepIndex(int FromIndex, ESearchDir::Type Dir,
		int& OutIndex, bool CheckCollision) const;
	int StepIndexRaw(int FromIndex, ESearchDir::Type Dir) const;
	bool IsBlockedIndex(int Index) const;
	bool IsWalkableGrid(int GridX, int GridY) const;
	bool IsGoalIndex(int Index) const;
	float ComputeHeuristic(int Index) const;
	float ComputeTraversalCost(int FromIndex, int ToIndex) const;
	float GetTileTraversalWeight(int Index) const;
	bool IsRoadTile(int Index) const;
	int FindFallbackGoalIndex(int StartIndex, int EndIndex) const;
	bool BuildPath(FNavNode* GoalNode, std::list<FVector3>& PathList);
	bool FindPathFallbackAStar(int StartIndex,
		std::list<FVector3>& PathList);

	static bool SortOpenList(FNavNode* Src, FNavNode* Dest);
};
