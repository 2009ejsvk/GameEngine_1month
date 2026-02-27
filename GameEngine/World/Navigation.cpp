#include "Navigation.h"
#include "../Component/TileMapComponent.h"
#include "../Object/GameObject.h"
#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <limits>

namespace
{
#ifdef _DEBUG
	void DebugPathLog(const char* Format, ...)
	{
		char Text[512] = {};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Text, Format, Args);
		va_end(Args);

		OutputDebugStringA(Text);
	}
#endif

	constexpr int DIR_DX[ESearchDir::End] =
	{
		0, 1, 1, 1, 0, -1, -1, -1
	};

	constexpr int DIR_DY[ESearchDir::End] =
	{
		1, 1, 0, -1, -1, -1, 0, 1
	};
}

CNavigation::CNavigation()
{
}

CNavigation::~CNavigation()
{
}

void CNavigation::SetTileMap(
	const std::weak_ptr<CTileMapComponent>& TileMap)
{
	mTileMap = TileMap;

	auto OriginMap = TileMap.lock();

	if (!OriginMap)
		return;

	// 길찾기 정보를 생성한다.
	mShape = OriginMap->GetTileShape();
	mCountX = OriginMap->GetTileCountX();
	mCountY = OriginMap->GetTileCountY();
	mTileSize = OriginMap->GetTileSize();

	int	Count = mCountX * mCountY;

	mNodeList.resize((size_t)Count);
	mOpenList.reserve((size_t)Count);
	mUseList.reserve((size_t)Count);

	for (int i = 0; i < Count; ++i)
	{
		auto Tile = OriginMap->GetTile(i).lock();

		if (!Tile)
			continue;

		FNavNode& Node = mNodeList[i];
		Node.Pos = Tile->GetPos();
		Node.Size = mTileSize;
		Node.Center = Tile->GetCenter();

		auto Owner = OriginMap->GetOwner().lock();

		if (Owner)
		{
			const FVector3 OwnerWorldPos = Owner->GetWorldPos();
			Node.Center.x += OwnerWorldPos.x;
			Node.Center.y += OwnerWorldPos.y;
		}
		Node.Index = i;
		Node.IndexX = Tile->GetIndexX();
		Node.IndexY = Tile->GetIndexY();

		if (mShape == ETileShape::Rect)
		{
			Node.GridX = Node.IndexX;
			Node.GridY = Node.IndexY;
		}

		else
		{
			// staggered isometric(odd-row shift) -> integer skew grid
			Node.GridX = Node.IndexX +
				((Node.IndexY + (Node.IndexY & 1)) / 2);
			Node.GridY = Node.IndexX -
				(Node.IndexY / 2);
		}
	}
}

bool CNavigation::FindPath(const FVector3& Start, const FVector3& End,
	std::list<FVector3>& PathList,
	const unsigned char* BlockMask,
	int BlockMaskByteCount,
	const int* GoalIndices,
	int GoalCount)
{
	auto TileMap = mTileMap.lock();

	if (!TileMap)
	{
		PathList.clear();
#ifdef _DEBUG
		DebugPathLog("[Path] Fail: tile map missing\n");
#endif
		return false;
	}

	const int TileCount = mCountX * mCountY;

	if (TileCount <= 0)
	{
		PathList.clear();
#ifdef _DEBUG
		DebugPathLog("[Path] Fail: invalid tile count=%d\n", TileCount);
#endif
		return false;
	}

	const int MaskByteCount = (TileCount + 7) / 8;

	mBlockedMask.assign((size_t)MaskByteCount, 0);

	if (BlockMask && BlockMaskByteCount > 0)
	{
		const int CopySize = MaskByteCount < BlockMaskByteCount ?
			MaskByteCount : BlockMaskByteCount;

		if (CopySize > 0)
		{
			memcpy(&mBlockedMask[0], BlockMask, (size_t)CopySize);
		}
	}

	else
	{
		for (int i = 0; i < TileCount; ++i)
		{
			if (TileMap->GetTileType(i) != ETileType::UnableToMove)
				continue;

			mBlockedMask[i / 8] |= (unsigned char)(1 << (i % 8));
		}
	}

	// 인덱스를 구한다.
	int StartIndex = TileMap->GetTileIndex(Start);

	if (StartIndex < 0 || StartIndex >= TileCount)
	{
#ifdef _DEBUG
		DebugPathLog("[Path] Fail: invalid start index=%d start=(%.1f, %.1f)\n",
			StartIndex, Start.x, Start.y);
#endif
		return false;
	}

	int EndIndex = TileMap->GetTileIndex(End);

	mStartIndex = StartIndex;
	mGoalMask.assign((size_t)MaskByteCount, 0);
	mGoalIndices.clear();

	auto SetGoal = [&](int Index)
	{
		if (Index < 0 || Index >= TileCount)
			return;

		const unsigned char Bit = (unsigned char)(1 << (Index % 8));
		unsigned char& Byte = mGoalMask[Index / 8];

		if (Byte & Bit)
			return;

		Byte |= Bit;
		mGoalIndices.emplace_back(Index);

		// 목표 타일은 탐색 불가 마스크에서 제외한다.
		mBlockedMask[Index / 8] &= (unsigned char)~Bit;
	};

	if (GoalIndices && GoalCount > 0)
	{
		for (int i = 0; i < GoalCount; ++i)
		{
			SetGoal(GoalIndices[i]);
		}
	}

	else
	{
		EndIndex = FindFallbackGoalIndex(StartIndex, EndIndex);

		if (EndIndex < 0)
		{
#ifdef _DEBUG
			DebugPathLog(
				"[Path] Fail: fallback goal missing start=%d end=%d\n",
				StartIndex, TileMap->GetTileIndex(End));
#endif
			return false;
		}

		SetGoal(EndIndex);
	}

	if (mGoalIndices.empty())
	{
#ifdef _DEBUG
		DebugPathLog(
			"[Path] Fail: goal list empty start=%d end=%d inputGoalCount=%d blockMaskBytes=%d\n",
			StartIndex,
			TileMap->GetTileIndex(End),
			GoalCount,
			BlockMaskByteCount);
#endif
		return false;
	}

	PathList.clear();

#ifdef _DEBUG
	DebugPathLog(
		"[Path] Start start=%d end=%d goals=%zu blockMaskBytes=%d\n",
		StartIndex,
		TileMap->GetTileIndex(End),
		mGoalIndices.size(),
		(int)mBlockedMask.size());
#endif

	// 이전에 사용했던 노드는 모두 초기화한다.
	size_t	UseSize = mUseList.size();

	for (size_t i = 0; i < UseSize; ++i)
	{
		FNavNode* Used = mUseList[i];
		Used->NodeType = ENavNodeType::None;
		Used->Dist = FLT_MAX;
		Used->Huristic = FLT_MAX;
		Used->Total = FLT_MAX;
		Used->Parent = nullptr;
		Used->SearchDirList.clear();
	}

	mUseList.clear();
	mOpenList.clear();

	// 시작 노드 설정
	FNavNode* StartNode = &mNodeList[StartIndex];

	if (IsGoalIndex(StartIndex))
	{
#ifdef _DEBUG
		DebugPathLog("[Path] Start already goal index=%d\n", StartIndex);
#endif
		return true;
	}

	StartNode->NodeType = ENavNodeType::Open;
	StartNode->Dist = 0.f;
	StartNode->Huristic = ComputeHeuristic(StartIndex);
	StartNode->Total = StartNode->Huristic;

	for (int i = 0; i < ESearchDir::End; ++i)
	{
		StartNode->SearchDirList.emplace_back((ESearchDir::Type)i);
	}

	mOpenList.emplace_back(StartNode);
	mUseList.emplace_back(StartNode);

	while (!mOpenList.empty())
	{
		// 열린 목록은 비용이 제일 작은 노드가 가장 뒤에 오도록 정렬한다.
		FNavNode* Node = mOpenList.back();
		mOpenList.pop_back();

		if (Node->NodeType == ENavNodeType::Close)
			continue;

		Node->NodeType = ENavNodeType::Close;

		if (IsGoalIndex(Node->Index))
		{
			mOpenList.clear();
			const bool Built = BuildPath(Node, PathList);
#ifdef _DEBUG
			DebugPathLog(
				"[Path] Goal reached index=%d built=%d pathPoints=%zu\n",
				Node->Index, Built ? 1 : 0, PathList.size());
#endif
			return Built;
		}

		FindNode(Node, PathList);

		if (mOpenList.size() >= 2)
		{
			std::sort(mOpenList.begin(), mOpenList.end(),
				CNavigation::SortOpenList);
		}
	}

	mOpenList.clear();
#ifdef _DEBUG
	DebugPathLog("[Path] Fail: open list exhausted start=%d goals=%zu\n",
		StartIndex, mGoalIndices.size());
#endif

	if (FindPathFallbackAStar(StartIndex, PathList))
	{
#ifdef _DEBUG
		DebugPathLog("[Path] Fallback A* success pathPoints=%zu\n",
			PathList.size());
#endif
		return true;
	}

#ifdef _DEBUG
	DebugPathLog("[Path] Fallback A* failed\n");
#endif

	return false;
}

bool CNavigation::FindNode(FNavNode* Node, std::list<FVector3>& PathList)
{
	const bool HasParent = Node->Parent != nullptr;
	const ESearchDir::Type ParentDir = GetParentDir(Node);

	AddDir(ParentDir, Node, HasParent);

	auto iter = Node->SearchDirList.begin();
	auto iterEnd = Node->SearchDirList.end();

	for (; iter != iterEnd; ++iter)
	{
		const int CornerIndex = Jump(Node, *iter);

		if (CornerIndex < 0)
			continue;

		FNavNode* Corner = &mNodeList[CornerIndex];

		if (Corner->NodeType == ENavNodeType::Close)
			continue;

		const float Dist = Node->Dist +
			Node->Center.Distance(Corner->Center);

		if (Corner->NodeType == ENavNodeType::Open)
		{
			if (Corner->Dist > Dist)
			{
				Corner->Dist = Dist;
				Corner->Total = Dist + Corner->Huristic;
				Corner->Parent = Node;
			}
		}

		else
		{
			Corner->NodeType = ENavNodeType::Open;
			Corner->Dist = Dist;
			Corner->Huristic = ComputeHeuristic(CornerIndex);
			Corner->Total = Dist + Corner->Huristic;
			Corner->Parent = Node;

			mOpenList.emplace_back(Corner);
			mUseList.emplace_back(Corner);
		}
	}

	return false;
}

int CNavigation::Jump(FNavNode* Node, ESearchDir::Type Dir)
{
	int NextIndex = -1;

	if (!StepIndex(Node->Index, Dir, NextIndex, true))
		return -1;

	if (IsGoalIndex(NextIndex))
		return NextIndex;

	if (HasForcedNeighbor(NextIndex, Dir))
		return NextIndex;

	if (IsDiagonalDir(Dir))
	{
		int dx = 0;
		int dy = 0;

		if (!GetMoveDelta(Dir, dx, dy))
			return -1;

		const ESearchDir::Type Horizontal = GetDirFromDelta(dx, 0);
		const ESearchDir::Type Vertical = GetDirFromDelta(0, dy);

		if (Horizontal != ESearchDir::End &&
			Jump(&mNodeList[NextIndex], Horizontal) != -1)
		{
			return NextIndex;
		}

		if (Vertical != ESearchDir::End &&
			Jump(&mNodeList[NextIndex], Vertical) != -1)
		{
			return NextIndex;
		}
	}

	return Jump(&mNodeList[NextIndex], Dir);
}

void CNavigation::AddDir(ESearchDir::Type ParentDir, FNavNode* Node,
	bool HasParent)
{
	Node->SearchDirList.clear();

	bool DirAdded[ESearchDir::End] = {};

	auto AddUnique = [&](ESearchDir::Type Dir)
	{
		if (Dir == ESearchDir::End)
			return;

		if (DirAdded[Dir])
			return;

		DirAdded[Dir] = true;
		Node->SearchDirList.emplace_back(Dir);
	};

	if (!HasParent)
	{
		for (int i = 0; i < ESearchDir::End; ++i)
		{
			AddUnique((ESearchDir::Type)i);
		}

		return;
	}

	int dx = 0;
	int dy = 0;

	if (!GetMoveDelta(ParentDir, dx, dy))
		return;

	const int gx = Node->GridX;
	const int gy = Node->GridY;

	if (dx != 0 && dy != 0)
	{
		// natural neighbors
		AddUnique(ParentDir);
		AddUnique(GetDirFromDelta(dx, 0));
		AddUnique(GetDirFromDelta(0, dy));

		// forced neighbors
		if (!IsWalkableGrid(gx - dx, gy) &&
			IsWalkableGrid(gx - dx, gy + dy))
		{
			AddUnique(GetDirFromDelta(-dx, dy));
		}

		if (!IsWalkableGrid(gx, gy - dy) &&
			IsWalkableGrid(gx + dx, gy - dy))
		{
			AddUnique(GetDirFromDelta(dx, -dy));
		}
	}

	else if (dx != 0)
	{
		AddUnique(ParentDir);

		if (!IsWalkableGrid(gx, gy + 1) &&
			IsWalkableGrid(gx + dx, gy + 1))
		{
			AddUnique(GetDirFromDelta(dx, 1));
		}

		if (!IsWalkableGrid(gx, gy - 1) &&
			IsWalkableGrid(gx + dx, gy - 1))
		{
			AddUnique(GetDirFromDelta(dx, -1));
		}
	}

	else if (dy != 0)
	{
		AddUnique(ParentDir);

		if (!IsWalkableGrid(gx + 1, gy) &&
			IsWalkableGrid(gx + 1, gy + dy))
		{
			AddUnique(GetDirFromDelta(1, dy));
		}

		if (!IsWalkableGrid(gx - 1, gy) &&
			IsWalkableGrid(gx - 1, gy + dy))
		{
			AddUnique(GetDirFromDelta(-1, dy));
		}
	}
}

bool CNavigation::HasForcedNeighbor(int Index, ESearchDir::Type Dir) const
{
	int dx = 0;
	int dy = 0;

	if (!GetMoveDelta(Dir, dx, dy))
		return false;

	const FNavNode& Node = mNodeList[Index];
	const int gx = Node.GridX;
	const int gy = Node.GridY;

	if (dx != 0 && dy != 0)
	{
		if (!IsWalkableGrid(gx - dx, gy) &&
			IsWalkableGrid(gx - dx, gy + dy))
		{
			return true;
		}

		if (!IsWalkableGrid(gx, gy - dy) &&
			IsWalkableGrid(gx + dx, gy - dy))
		{
			return true;
		}
	}

	else if (dx != 0)
	{
		if (!IsWalkableGrid(gx, gy + 1) &&
			IsWalkableGrid(gx + dx, gy + 1))
		{
			return true;
		}

		if (!IsWalkableGrid(gx, gy - 1) &&
			IsWalkableGrid(gx + dx, gy - 1))
		{
			return true;
		}
	}

	else if (dy != 0)
	{
		if (!IsWalkableGrid(gx + 1, gy) &&
			IsWalkableGrid(gx + 1, gy + dy))
		{
			return true;
		}

		if (!IsWalkableGrid(gx - 1, gy) &&
			IsWalkableGrid(gx - 1, gy + dy))
		{
			return true;
		}
	}

	return false;
}

bool CNavigation::IsDiagonalDir(ESearchDir::Type Dir) const
{
	int dx = 0;
	int dy = 0;

	if (!GetMoveDelta(Dir, dx, dy))
		return false;

	return dx != 0 && dy != 0;
}

bool CNavigation::GetMoveDelta(ESearchDir::Type Dir, int& OutDx, int& OutDy) const
{
	if (Dir < 0 || Dir >= ESearchDir::End)
		return false;

	OutDx = DIR_DX[Dir];
	OutDy = DIR_DY[Dir];
	return true;
}

ESearchDir::Type CNavigation::GetDirFromDelta(int dx, int dy) const
{
	dx = (dx > 0) - (dx < 0);
	dy = (dy > 0) - (dy < 0);

	for (int i = 0; i < ESearchDir::End; ++i)
	{
		if (DIR_DX[i] == dx && DIR_DY[i] == dy)
			return (ESearchDir::Type)i;
	}

	return ESearchDir::End;
}

ESearchDir::Type CNavigation::GetParentDir(FNavNode* Node) const
{
	if (!Node || !Node->Parent)
		return ESearchDir::End;

	const int dx = Node->GridX - Node->Parent->GridX;
	const int dy = Node->GridY - Node->Parent->GridY;

	return GetDirFromDelta(dx, dy);
}

int CNavigation::GetIndexByGrid(int GridX, int GridY) const
{
	if (mShape == ETileShape::Rect)
	{
		if (GridX < 0 || GridX >= mCountX ||
			GridY < 0 || GridY >= mCountY)
		{
			return -1;
		}

		return GridY * mCountX + GridX;
	}

	// isometric skew grid -> tile index
	const int y = GridX - GridY;

	if (y < 0 || y >= mCountY)
		return -1;

	const int x = GridY + (y / 2);

	if (x < 0 || x >= mCountX)
		return -1;

	return y * mCountX + x;
}

bool CNavigation::StepIndex(int FromIndex, ESearchDir::Type Dir,
	int& OutIndex, bool CheckCollision) const
{
	int dx = 0;
	int dy = 0;

	if (!GetMoveDelta(Dir, dx, dy))
		return false;

	const FNavNode& FromNode = mNodeList[FromIndex];

	const int NextGridX = FromNode.GridX + dx;
	const int NextGridY = FromNode.GridY + dy;

	const int NextIndex = GetIndexByGrid(NextGridX, NextGridY);

	if (NextIndex < 0)
		return false;

	if (CheckCollision)
	{
		if (IsBlockedIndex(NextIndex))
			return false;

		// 대각 이동 시 코너 비집고 통과 금지.
		if (dx != 0 && dy != 0)
		{
			const int Side1 = GetIndexByGrid(FromNode.GridX + dx,
				FromNode.GridY);
			const int Side2 = GetIndexByGrid(FromNode.GridX,
				FromNode.GridY + dy);

			if (Side1 < 0 || Side2 < 0)
				return false;

			if (IsBlockedIndex(Side1) || IsBlockedIndex(Side2))
				return false;
		}
	}

	OutIndex = NextIndex;
	return true;
}

int CNavigation::StepIndexRaw(int FromIndex, ESearchDir::Type Dir) const
{
	int Index = -1;

	if (!StepIndex(FromIndex, Dir, Index, false))
		return -1;

	return Index;
}

bool CNavigation::IsBlockedIndex(int Index) const
{
	if (Index < 0 || Index >= (int)mNodeList.size())
		return true;

	if (Index == mStartIndex)
		return false;

	if (!mBlockedMask.empty())
	{
		return (mBlockedMask[Index / 8] &
			(unsigned char)(1 << (Index % 8))) != 0;
	}

	auto TileMap = mTileMap.lock();

	if (!TileMap)
		return true;

	return TileMap->GetTileType(Index) == ETileType::UnableToMove;
}

bool CNavigation::IsWalkableGrid(int GridX, int GridY) const
{
	const int Index = GetIndexByGrid(GridX, GridY);

	if (Index < 0)
		return false;

	return !IsBlockedIndex(Index);
}

bool CNavigation::IsGoalIndex(int Index) const
{
	if (Index < 0 || Index >= (int)mNodeList.size())
		return false;

	if (mGoalMask.empty())
		return false;

	return (mGoalMask[Index / 8] &
		(unsigned char)(1 << (Index % 8))) != 0;
}

float CNavigation::ComputeHeuristic(int Index) const
{
	if (mGoalIndices.empty())
		return 0.f;

	float Best = FLT_MAX;

	const FVector2& Center = mNodeList[Index].Center;

	for (size_t i = 0; i < mGoalIndices.size(); ++i)
	{
		const int GoalIndex = mGoalIndices[i];
		const float Dist = Center.Distance(mNodeList[GoalIndex].Center);

		if (Dist < Best)
			Best = Dist;
	}

	return Best;
}

int CNavigation::FindFallbackGoalIndex(int StartIndex, int EndIndex) const
{
	if (EndIndex < 0 || EndIndex >= (int)mNodeList.size())
		return -1;

	if (!IsBlockedIndex(EndIndex))
		return EndIndex;

	const FVector2& StartCenter = mNodeList[StartIndex].Center;

	float BestDist = FLT_MAX;
	int BestIndex = -1;

	for (int i = 0; i < ESearchDir::End; ++i)
	{
		const int Neighbor = StepIndexRaw(EndIndex, (ESearchDir::Type)i);

		if (Neighbor < 0)
			continue;

		if (IsBlockedIndex(Neighbor))
			continue;

		const float Dist =
			StartCenter.Distance(mNodeList[Neighbor].Center);

		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestIndex = Neighbor;
		}
	}

	return BestIndex;
}

bool CNavigation::BuildPath(FNavNode* GoalNode,
	std::list<FVector3>& PathList)
{
	PathList.clear();

	if (!GoalNode)
		return false;

	std::vector<int> ChainIndices;

	FNavNode* CurrentNode = GoalNode;

	while (CurrentNode)
	{
		ChainIndices.emplace_back(CurrentNode->Index);
		CurrentNode = CurrentNode->Parent;
	}

	if (ChainIndices.empty())
		return false;

	std::reverse(ChainIndices.begin(), ChainIndices.end());

	if (ChainIndices.size() <= 1)
		return true;

	for (size_t i = 0; i + 1 < ChainIndices.size(); ++i)
	{
		const int FromIndex = ChainIndices[i];
		const int ToIndex = ChainIndices[i + 1];

		const int dx = mNodeList[ToIndex].GridX -
			mNodeList[FromIndex].GridX;
		const int dy = mNodeList[ToIndex].GridY -
			mNodeList[FromIndex].GridY;

		const ESearchDir::Type Dir = GetDirFromDelta(dx, dy);

		if (Dir == ESearchDir::End)
			return false;

		int CurrentIndex = FromIndex;

		while (CurrentIndex != ToIndex)
		{
			CurrentIndex = StepIndexRaw(CurrentIndex, Dir);

			if (CurrentIndex < 0)
				return false;

			const FVector2& Center = mNodeList[CurrentIndex].Center;
			PathList.emplace_back(Center.x, Center.y, 0.f);
		}
	}

	return true;
}

bool CNavigation::FindPathFallbackAStar(int StartIndex,
	std::list<FVector3>& PathList)
{
	const int NodeCount = (int)mNodeList.size();

	if (StartIndex < 0 || StartIndex >= NodeCount)
		return false;

	if (mGoalIndices.empty())
		return false;

	const float Inf = (std::numeric_limits<float>::max)();

	std::vector<float> GScore((size_t)NodeCount, Inf);
	std::vector<float> FScore((size_t)NodeCount, Inf);
	std::vector<int> Parent((size_t)NodeCount, -1);
	std::vector<unsigned char> OpenMask((size_t)NodeCount, 0);
	std::vector<unsigned char> ClosedMask((size_t)NodeCount, 0);

	GScore[(size_t)StartIndex] = 0.f;
	FScore[(size_t)StartIndex] = ComputeHeuristic(StartIndex);
	OpenMask[(size_t)StartIndex] = 1;

	int GoalIndex = -1;

	while (true)
	{
		int Current = -1;
		float BestF = Inf;

		for (int i = 0; i < NodeCount; ++i)
		{
			if (!OpenMask[(size_t)i])
				continue;

			if (FScore[(size_t)i] < BestF)
			{
				BestF = FScore[(size_t)i];
				Current = i;
			}
		}

		if (Current < 0)
			break;

		if (IsGoalIndex(Current))
		{
			GoalIndex = Current;
			break;
		}

		OpenMask[(size_t)Current] = 0;
		ClosedMask[(size_t)Current] = 1;

		for (int Dir = 0; Dir < ESearchDir::End; ++Dir)
		{
			int Next = -1;

			if (!StepIndex(Current, (ESearchDir::Type)Dir, Next, true))
				continue;

			if (Next < 0 || Next >= NodeCount)
				continue;

			if (ClosedMask[(size_t)Next])
				continue;

			const float StepCost = mNodeList[Current].Center.Distance(
				mNodeList[Next].Center);
			const float NewG = GScore[(size_t)Current] + StepCost;

			if (!OpenMask[(size_t)Next] || NewG < GScore[(size_t)Next])
			{
				Parent[(size_t)Next] = Current;
				GScore[(size_t)Next] = NewG;
				FScore[(size_t)Next] = NewG + ComputeHeuristic(Next);
				OpenMask[(size_t)Next] = 1;
			}
		}
	}

	if (GoalIndex < 0)
		return false;

	std::vector<int> Chain;
	Chain.reserve((size_t)NodeCount);

	int Current = GoalIndex;

	while (Current >= 0 && Current < NodeCount)
	{
		Chain.emplace_back(Current);

		if (Current == StartIndex)
			break;

		Current = Parent[(size_t)Current];
	}

	if (Chain.empty() || Chain.back() != StartIndex)
		return false;

	std::reverse(Chain.begin(), Chain.end());

	for (int i = 0; i < NodeCount; ++i)
	{
		mNodeList[(size_t)i].Parent = nullptr;
	}

	for (size_t i = 1; i < Chain.size(); ++i)
	{
		const int Child = Chain[i];
		const int ParentIndex = Chain[i - 1];
		mNodeList[(size_t)Child].Parent = &mNodeList[(size_t)ParentIndex];
	}

	return BuildPath(&mNodeList[(size_t)GoalIndex], PathList);
}

bool CNavigation::SortOpenList(FNavNode* Src, FNavNode* Dest)
{
	return Src->Total > Dest->Total;
}
