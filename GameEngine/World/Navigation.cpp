#include "Navigation.h"
#include "../Component/TileMapComponent.h"

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

	auto	OriginMap = TileMap.lock();

	// 길찾기 정보를 생성한다.
	mShape = OriginMap->GetTileShape();
	mCountX = OriginMap->GetTileCountX();
	mCountY = OriginMap->GetTileCountY();
	mTileSize = OriginMap->GetTileSize();
	mDiagonalCost = mTileSize.Length();

	int	Count = mCountX * mCountY;

	mNodeList.resize((size_t)Count);
	mOpenList.reserve((size_t)Count);
	mUseList.reserve((size_t)Count);

	for (int i = 0; i < Count; ++i)
	{
		auto	Tile = OriginMap->GetTile(i).lock();

		mNodeList[i].Pos = Tile->GetPos();
		mNodeList[i].Size = mTileSize;
		mNodeList[i].Center = Tile->GetCenter();
		mNodeList[i].Index = i;
		mNodeList[i].IndexX = i % mCountX;
		mNodeList[i].IndexY = i / mCountX;
	}
}

bool CNavigation::FindPath(const FVector3& Start, const FVector3& End,
	std::list<FVector3>& PathList)
{
	auto	TileMap = mTileMap.lock();

	if (!TileMap)
	{
		PathList.clear();
		PathList.emplace_back(End);
		return false;
	}

	// 인덱스를 구한다.
	int	StartIndex = TileMap->GetTileIndex(Start);
	if (StartIndex == -1)
		return false;

	int	EndIndex = TileMap->GetTileIndex(End);
	if (EndIndex == -1)
		return false;

	// 만약 도착지점이 이동불가 타일이라면
	// 1. 탐색을 멈춘다.
	// 2. 주변 8방향을 검색하여 시작지점과 가까운 타일을 찾아서
	// 도착지점으로 사용한다.
	if (TileMap->GetTileType(EndIndex) == ETileType::UnableToMove)
	{
		// 멈출 경우
		//PathList.clear();
		//return false;

		// 주변 타일 찾을 경우
		int	StartX = StartIndex % mCountX;
		int	StartY = StartIndex / mCountX;

		int	EndX = EndIndex % mCountX;
		int	EndY = EndIndex / mCountX;

		int	FindIndex[8] =
		{
			(EndY + 1) * mCountX + EndX,
			(EndY + 1) * mCountX + (EndX + 1),
			EndY * mCountX + (EndX + 1),
			(EndY - 1) * mCountX + (EndX + 1),
			(EndY - 1) * mCountX + EndX,
			(EndY - 1) * mCountX + (EndX - 1),
			EndY * mCountX + (EndX - 1),
			(EndY + 1) * mCountX + (EndX - 1)
		};

		std::vector<int>	CheckIndex;

		for (int i = 0; i < 8; ++i)
		{
			if (TileMap->GetTileType(FindIndex[i]) ==
				ETileType::Normal)
			{
				CheckIndex.push_back(FindIndex[i]);
			}
		}

		float	Dist = FLT_MAX;

		size_t	Size = CheckIndex.size();

		for (size_t i = 0; i < Size; ++i)
		{
			float CheckDist = mNodeList[CheckIndex[i]].Center.Distance(mNodeList[StartIndex].Center);

			if (CheckDist < Dist)
			{
				EndIndex = CheckIndex[i];
				Dist = CheckDist;
			}
		}
	}

	PathList.clear();

	// 이전에 사용했던 노드는 모드 초기화 시켜준다.
	size_t	Size = mUseList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		mUseList[i]->NodeType = ENavNodeType::None;
		mUseList[i]->Dist = FLT_MAX;
		mUseList[i]->Huristic = FLT_MAX;
		mUseList[i]->Total = FLT_MAX;
		mUseList[i]->Parent = nullptr;
		mUseList[i]->SearchDirList.clear();
	}

	mUseList.clear();

	// 시작 노드와 도착 노드를 얻어온다.
	FNavNode* StartNode = &mNodeList[StartIndex];
	FNavNode* EndNode = &mNodeList[EndIndex];

	if (StartNode == EndNode)
	{
		PathList.emplace_back(End);
		return true;
	}

	StartNode->NodeType = ENavNodeType::Open;
	StartNode->Dist = 0.f;
	StartNode->Huristic = EndNode->Center.Distance(StartNode->Center);
	StartNode->Total = StartNode->Huristic;

	// 처음 시작 노드는 8방향을 모두 체크하며 시작한다.
	for (int i = 0; i < ESearchDir::End; ++i)
	{
		StartNode->SearchDirList.emplace_back((ESearchDir::Type)i);
	}

	mOpenList.emplace_back(StartNode);
	mUseList.emplace_back(StartNode);

	while (!mOpenList.empty())
	{
		// 검사할 노드를 가져온다.
		// 열린 목록은 비용이 제일 작은 노드가 가장 뒤에 갈 수 있게
		// 정렬한다. 즉, 내림차순 정렬을 해놓을 것이다.
		FNavNode* Node = mOpenList.back();
		// 열린목록에서 제거한다.
		mOpenList.pop_back();

		// 이 노드는 이제 닫힌 목록이 되어야 한다.
		Node->NodeType = ENavNodeType::Close;

		// SearchDir을 이용해 탐색 방향으로 탐색하여 코너를 가진 노드를
		// 찾아낸다. 이 함수에서는 도착점을 찾았다면 true를 반환한다.
		if (FindNode(Node, EndNode, End, PathList))
		{
			mOpenList.clear();
			return true;
		}

		// 열린목록을 정렬한다.
		if (mOpenList.size() >= 2)
		{
			std::sort(mOpenList.begin(), mOpenList.end(),
				CNavigation::SortOpenList);
		}
	}

	mOpenList.clear();

	return false;
}

bool CNavigation::FindNode(FNavNode* Node, FNavNode* EndNode,
	const FVector3& End, std::list<FVector3>& PathList)
{
	auto	iter = Node->SearchDirList.begin();
	auto	iterEnd = Node->SearchDirList.end();

	for (; iter != iterEnd; ++iter)
	{
		// 현재 방향으로 탐색해나가며 코너를 찾아온다.
		FNavNode* Corner = GetCorner(*iter, Node, EndNode);

		// 코너가 없을 경우 다음 방향을 검사하게 한다.
		if (!Corner)
			continue;

		else if (Corner == EndNode)
		{
			mUseList.emplace_back(Corner);

			// 도착점을 경로에 넣어준다.
			PathList.emplace_back(End);

			FNavNode* PathNode = Node;

			while (PathNode)
			{
				FVector3	Center;
				Center = PathNode->Center;

				PathList.emplace_front(Center);
				PathNode = PathNode->Parent;
			}

			// 시작노드의 위치는 필요없기 때문에 제거한다.
			PathList.pop_front();

			return true;
		}

		float	Dist = Node->Dist +
			Node->Center.Distance(Corner->Center);

		// 구해준 코너가 열린목록에 이미 들어가 있는 노드일 경우
		if (Corner->NodeType == ENavNodeType::Open)
		{
			if (Corner->Dist > Dist)
			{
				Corner->Dist = Dist;
				Corner->Total = Dist + Corner->Huristic;
				Corner->Parent = Node;

				AddDir(*iter, Corner);
			}
		}

		else
		{
			Corner->NodeType = ENavNodeType::Open;
			Corner->Dist = Dist;
			Corner->Huristic = EndNode->Center.Distance(Corner->Center);
			Corner->Total = Dist + Corner->Huristic;
			Corner->Parent = Node;

			mOpenList.emplace_back(Corner);
			mUseList.emplace_back(Corner);

			AddDir(*iter, Corner);
		}
	}

	return false;
}

FNavNode* CNavigation::GetCorner(ESearchDir::Type Dir, 
	FNavNode* Node, FNavNode* EndNode)
{
	switch (mShape)
	{
	case Rect:
		switch (Dir)
		{
		case ESearchDir::T:
			return GetCornerRectT(Node, EndNode);
		case ESearchDir::RT:
			return GetCornerRectRT(Node, EndNode);
		case ESearchDir::R:
			return GetCornerRectR(Node, EndNode);
		case ESearchDir::RB:
			return GetCornerRectRB(Node, EndNode);
		case ESearchDir::B:
			return GetCornerRectB(Node, EndNode);
		case ESearchDir::LB:
			return GetCornerRectLB(Node, EndNode);
		case ESearchDir::L:
			return GetCornerRectL(Node, EndNode);
		case ESearchDir::LT:
			return GetCornerRectLT(Node, EndNode);
		}
		break;
	case Isometric:
		switch (Dir)
		{
		case ESearchDir::T:
			break;
		case ESearchDir::RT:
			break;
		case ESearchDir::R:
			break;
		case ESearchDir::RB:
			break;
		case ESearchDir::B:
			break;
		case ESearchDir::LB:
			break;
		case ESearchDir::L:
			break;
		case ESearchDir::LT:
			break;
		}
		break;
	}

	return nullptr;
}

void CNavigation::AddDir(ESearchDir::Type Dir, FNavNode* Node)
{
	Node->SearchDirList.clear();

	switch (mShape)
	{
	case Rect:
		switch (Dir)
		{
		case ESearchDir::T:
			Node->SearchDirList.emplace_back(ESearchDir::T);
			Node->SearchDirList.emplace_back(ESearchDir::RT);
			Node->SearchDirList.emplace_back(ESearchDir::LT);
			break;
		case ESearchDir::RT:
			Node->SearchDirList.emplace_back(ESearchDir::RT);
			Node->SearchDirList.emplace_back(ESearchDir::T);
			Node->SearchDirList.emplace_back(ESearchDir::R);
			Node->SearchDirList.emplace_back(ESearchDir::LT);
			Node->SearchDirList.emplace_back(ESearchDir::RB);
			break;
		case ESearchDir::R:
			Node->SearchDirList.emplace_back(ESearchDir::R);
			Node->SearchDirList.emplace_back(ESearchDir::RT);
			Node->SearchDirList.emplace_back(ESearchDir::RB);
			break;
		case ESearchDir::RB:
			Node->SearchDirList.emplace_back(ESearchDir::RB);
			Node->SearchDirList.emplace_back(ESearchDir::B);
			Node->SearchDirList.emplace_back(ESearchDir::R);
			Node->SearchDirList.emplace_back(ESearchDir::RT);
			Node->SearchDirList.emplace_back(ESearchDir::LB);
			break;
		case ESearchDir::B:
			Node->SearchDirList.emplace_back(ESearchDir::B);
			Node->SearchDirList.emplace_back(ESearchDir::RB);
			Node->SearchDirList.emplace_back(ESearchDir::LB);
			break;
		case ESearchDir::LB:
			Node->SearchDirList.emplace_back(ESearchDir::LB);
			Node->SearchDirList.emplace_back(ESearchDir::B);
			Node->SearchDirList.emplace_back(ESearchDir::L);
			Node->SearchDirList.emplace_back(ESearchDir::LT);
			Node->SearchDirList.emplace_back(ESearchDir::RB);
			break;
		case ESearchDir::L:
			Node->SearchDirList.emplace_back(ESearchDir::L);
			Node->SearchDirList.emplace_back(ESearchDir::LT);
			Node->SearchDirList.emplace_back(ESearchDir::LB);
			break;
		case ESearchDir::LT:
			Node->SearchDirList.emplace_back(ESearchDir::LT);
			Node->SearchDirList.emplace_back(ESearchDir::T);
			Node->SearchDirList.emplace_back(ESearchDir::L);
			Node->SearchDirList.emplace_back(ESearchDir::LB);
			Node->SearchDirList.emplace_back(ESearchDir::RT);
			break;
		}
		break;
	case Isometric:
		break;
	}
}

FNavNode* CNavigation::GetCornerRectT(FNavNode* Node, 
	FNavNode* EndNode)
{
	auto	TileMap = mTileMap.lock();

	int	IndexX = Node->IndexX;
	int	IndexY = Node->IndexY;

	while (true)
	{
		++IndexY;

		if (IndexY >= mCountY)
			return nullptr;

		int	Index = IndexY * mCountX + IndexX;

		FNavNode* Corner = &mNodeList[Index];

		// 도착했으면 도착 노드를 바로 리턴한다.
		if (Corner == EndNode)
			return Corner;

		// 탐색하는 노드가 닫힌목록에 들어간 노드일 경우
		else if (Corner->NodeType == ENavNodeType::Close)
			return nullptr;

		// 이동 불가를 만났다면
		else if (TileMap->GetTileType(Index) == 
			ETileType::UnableToMove)
			return nullptr;

		// 오른쪽이 막혀있고 오른쪽 위가 뚫려있을 경우
		int	CornerX = IndexX + 1;
		int	CornerY = IndexY;
		int	CornerIndex = 0;

		if (CornerX < mCountX && CornerY + 1 < mCountY)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex + mCountX) ==
				ETileType::Normal)
				return Corner;
		}

		// 왼쪽이 막혀있고 왼쪽 위가 뚫려있을 경우
		CornerX = IndexX - 1;
		CornerY = IndexY;

		if (CornerX >= 0 && CornerY + 1 < mCountY)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex + mCountX) ==
				ETileType::Normal)
				return Corner;
		}
	}

	return nullptr;
}

FNavNode* CNavigation::GetCornerRectRT(FNavNode* Node, 
	FNavNode* EndNode)
{
	auto	TileMap = mTileMap.lock();

	int	IndexX = Node->IndexX;
	int	IndexY = Node->IndexY;

	while (true)
	{
		++IndexX;
		++IndexY;

		if (IndexX >= mCountX || IndexY >= mCountY)
			return nullptr;

		int	Index = IndexY * mCountX + IndexX;

		FNavNode* Corner = &mNodeList[Index];

		// 도착했으면 도착 노드를 바로 리턴한다.
		if (Corner == EndNode)
			return Corner;

		// 탐색하는 노드가 닫힌목록에 들어간 노드일 경우
		else if (Corner->NodeType == ENavNodeType::Close)
			return nullptr;

		// 이동 불가를 만났다면
		else if (TileMap->GetTileType(Index) ==
			ETileType::UnableToMove)
			return nullptr;

		// 왼쪽이 막혀있고 왼쪽 위가 뚫려있을 경우
		int	CornerX = IndexX - 1;
		int	CornerY = IndexY;
		int	CornerIndex = 0;

		if (CornerX >= 0 && CornerY + 1 < mCountY)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex + mCountX) ==
				ETileType::Normal)
				return Corner;
		}

		// 아래가 막혀있고 오른쪽 아래가 뚫려있을 경우
		CornerX = IndexX;
		CornerY = IndexY - 1;

		if (CornerX + 1 < mCountX && CornerY >= 0)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex + 1) ==
				ETileType::Normal)
				return Corner;
		}

		// 위의 2가지 경우가 아니라면 오른쪽, 위 쪽으로 코너가 있는지
		// 체크해야 한다.
		FNavNode* FindNode = GetCornerRectR(Corner, EndNode);
		
		if (FindNode)
			return Corner;

		FindNode = GetCornerRectT(Corner, EndNode);

		if (FindNode)
			return Corner;
	}

	return nullptr;
}

FNavNode* CNavigation::GetCornerRectR(FNavNode* Node, 
	FNavNode* EndNode)
{
	auto	TileMap = mTileMap.lock();

	int	IndexX = Node->IndexX;
	int	IndexY = Node->IndexY;

	while (true)
	{
		++IndexX;

		if (IndexX >= mCountX)
			return nullptr;

		int	Index = IndexY * mCountX + IndexX;

		FNavNode* Corner = &mNodeList[Index];

		// 도착했으면 도착 노드를 바로 리턴한다.
		if (Corner == EndNode)
			return Corner;

		// 탐색하는 노드가 닫힌목록에 들어간 노드일 경우
		else if (Corner->NodeType == ENavNodeType::Close)
			return nullptr;

		// 이동 불가를 만났다면
		else if (TileMap->GetTileType(Index) ==
			ETileType::UnableToMove)
			return nullptr;

		// 위가 막혀있고 오른쪽 위가 뚫려있을 경우
		int	CornerX = IndexX;
		int	CornerY = IndexY + 1;
		int	CornerIndex = 0;

		if (CornerX + 1 < mCountX && CornerY < mCountY)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex + 1) ==
				ETileType::Normal)
				return Corner;
		}

		// 아래가 막혀있고 오른쪽 아래가 뚫려있을 경우
		CornerX = IndexX;
		CornerY = IndexY - 1;

		if (CornerX + 1 < mCountX && CornerY >= 0)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex + 1) ==
				ETileType::Normal)
				return Corner;
		}
	}

	return nullptr;
}

FNavNode* CNavigation::GetCornerRectRB(FNavNode* Node,
	FNavNode* EndNode)
{
	auto	TileMap = mTileMap.lock();

	int	IndexX = Node->IndexX;
	int	IndexY = Node->IndexY;

	while (true)
	{
		++IndexX;
		--IndexY;

		if (IndexX >= mCountX || IndexY < 0)
			return nullptr;

		int	Index = IndexY * mCountX + IndexX;

		FNavNode* Corner = &mNodeList[Index];

		// 도착했으면 도착 노드를 바로 리턴한다.
		if (Corner == EndNode)
			return Corner;

		// 탐색하는 노드가 닫힌목록에 들어간 노드일 경우
		else if (Corner->NodeType == ENavNodeType::Close)
			return nullptr;

		// 이동 불가를 만났다면
		else if (TileMap->GetTileType(Index) ==
			ETileType::UnableToMove)
			return nullptr;

		// 위가 막혀있고 오른쪽 위가 뚫려있을 경우
		int	CornerX = IndexX;
		int	CornerY = IndexY + 1;
		int	CornerIndex = 0;

		if (CornerX + 1 < mCountX && CornerY < mCountY)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex + 1) ==
				ETileType::Normal)
				return Corner;
		}

		// 왼쪽이 막혀있고 왼쪽 아래가 뚫려있을 경우
		CornerX = IndexX - 1;
		CornerY = IndexY;

		if (CornerX >= 0 && CornerY - 1 >= 0)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex - mCountX) ==
				ETileType::Normal)
				return Corner;
		}

		// 위의 2가지 경우가 아니라면 오른쪽, 아래 쪽으로 코너가 있는지
		// 체크해야 한다.
		FNavNode* FindNode = GetCornerRectR(Corner, EndNode);

		if (FindNode)
			return Corner;

		FindNode = GetCornerRectB(Corner, EndNode);

		if (FindNode)
			return Corner;
	}

	return nullptr;
}

FNavNode* CNavigation::GetCornerRectB(FNavNode* Node,
	FNavNode* EndNode)
{
	auto	TileMap = mTileMap.lock();

	int	IndexX = Node->IndexX;
	int	IndexY = Node->IndexY;

	while (true)
	{
		--IndexY;

		if (IndexY < 0)
			return nullptr;

		int	Index = IndexY * mCountX + IndexX;

		FNavNode* Corner = &mNodeList[Index];

		// 도착했으면 도착 노드를 바로 리턴한다.
		if (Corner == EndNode)
			return Corner;

		// 탐색하는 노드가 닫힌목록에 들어간 노드일 경우
		else if (Corner->NodeType == ENavNodeType::Close)
			return nullptr;

		// 이동 불가를 만났다면
		else if (TileMap->GetTileType(Index) ==
			ETileType::UnableToMove)
			return nullptr;

		// 오른쪽이 막혀있고 오른쪽 아래가 뚫려있을 경우
		int	CornerX = IndexX + 1;
		int	CornerY = IndexY;
		int	CornerIndex = 0;

		if (CornerX < mCountX && CornerY - 1 >= 0)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex - mCountX) ==
				ETileType::Normal)
				return Corner;
		}

		// 왼쪽이 막혀있고 왼쪽 아래가 뚫려있을 경우
		CornerX = IndexX - 1;
		CornerY = IndexY;

		if (CornerX >= 0 && CornerY - 1 >= 0)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex - mCountX) ==
				ETileType::Normal)
				return Corner;
		}
	}

	return nullptr;
}

FNavNode* CNavigation::GetCornerRectLB(FNavNode* Node,
	FNavNode* EndNode)
{
	auto	TileMap = mTileMap.lock();

	int	IndexX = Node->IndexX;
	int	IndexY = Node->IndexY;

	while (true)
	{
		--IndexX;
		--IndexY;

		if (IndexX < 0 || IndexY < 0)
			return nullptr;

		int	Index = IndexY * mCountX + IndexX;

		FNavNode* Corner = &mNodeList[Index];

		// 도착했으면 도착 노드를 바로 리턴한다.
		if (Corner == EndNode)
			return Corner;

		// 탐색하는 노드가 닫힌목록에 들어간 노드일 경우
		else if (Corner->NodeType == ENavNodeType::Close)
			return nullptr;

		// 이동 불가를 만났다면
		else if (TileMap->GetTileType(Index) ==
			ETileType::UnableToMove)
			return nullptr;

		// 위가 막혀있고 왼쪽 위가 뚫려있을 경우
		int	CornerX = IndexX;
		int	CornerY = IndexY + 1;
		int	CornerIndex = 0;

		if (CornerX - 1 >= 0 && CornerY < mCountY)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex - 1) ==
				ETileType::Normal)
				return Corner;
		}

		// 오른쪽이 막혀있고 오른쪽 아래가 뚫려있을 경우
		CornerX = IndexX + 1;
		CornerY = IndexY;

		if (CornerX < mCountX && CornerY - 1 >= 0)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex - mCountX) ==
				ETileType::Normal)
				return Corner;
		}

		// 위의 2가지 경우가 아니라면 왼쪽, 아래 쪽으로 코너가 있는지
		// 체크해야 한다.
		FNavNode* FindNode = GetCornerRectL(Corner, EndNode);

		if (FindNode)
			return Corner;

		FindNode = GetCornerRectB(Corner, EndNode);

		if (FindNode)
			return Corner;
	}

	return nullptr;
}

FNavNode* CNavigation::GetCornerRectL(FNavNode* Node,
	FNavNode* EndNode)
{
	auto	TileMap = mTileMap.lock();

	int	IndexX = Node->IndexX;
	int	IndexY = Node->IndexY;

	while (true)
	{
		--IndexX;

		if (IndexX < 0)
			return nullptr;

		int	Index = IndexY * mCountX + IndexX;

		FNavNode* Corner = &mNodeList[Index];

		// 도착했으면 도착 노드를 바로 리턴한다.
		if (Corner == EndNode)
			return Corner;

		// 탐색하는 노드가 닫힌목록에 들어간 노드일 경우
		else if (Corner->NodeType == ENavNodeType::Close)
			return nullptr;

		// 이동 불가를 만났다면
		else if (TileMap->GetTileType(Index) ==
			ETileType::UnableToMove)
			return nullptr;

		// 위가 막혀있고 왼쪽 위가 뚫려있을 경우
		int	CornerX = IndexX;
		int	CornerY = IndexY + 1;
		int	CornerIndex = 0;

		if (CornerX - 1 >= 0 && CornerY < mCountY)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex - 1) ==
				ETileType::Normal)
				return Corner;
		}

		// 아래가 막혀있고 왼쪽 아래가 뚫려있을 경우
		CornerX = IndexX;
		CornerY = IndexY - 1;

		if (CornerX - 1 >= 0 && CornerY >= 0)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex - 1) ==
				ETileType::Normal)
				return Corner;
		}
	}

	return nullptr;
}

FNavNode* CNavigation::GetCornerRectLT(FNavNode* Node,
	FNavNode* EndNode)
{
	auto	TileMap = mTileMap.lock();

	int	IndexX = Node->IndexX;
	int	IndexY = Node->IndexY;

	while (true)
	{
		--IndexX;
		++IndexY;

		if (IndexX < 0 || IndexY >= mCountY)
			return nullptr;

		int	Index = IndexY * mCountX + IndexX;

		FNavNode* Corner = &mNodeList[Index];

		// 도착했으면 도착 노드를 바로 리턴한다.
		if (Corner == EndNode)
			return Corner;

		// 탐색하는 노드가 닫힌목록에 들어간 노드일 경우
		else if (Corner->NodeType == ENavNodeType::Close)
			return nullptr;

		// 이동 불가를 만났다면
		else if (TileMap->GetTileType(Index) ==
			ETileType::UnableToMove)
			return nullptr;

		// 아래가 막혀있고 왼쪽 아래가 뚫려있을 경우
		int	CornerX = IndexX;
		int	CornerY = IndexY - 1;
		int	CornerIndex = 0;

		if (CornerX - 1 >= 0 && CornerY >= 0)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex - 1) ==
				ETileType::Normal)
				return Corner;
		}

		// 오른쪽이 막혀있고 오른쪽 위가 뚫려있을 경우
		CornerX = IndexX + 1;
		CornerY = IndexY;

		if (CornerX < mCountX && CornerY + 1 < mCountY)
		{
			CornerIndex = CornerY * mCountX + CornerX;

			if (TileMap->GetTileType(CornerIndex) ==
				ETileType::UnableToMove &&
				TileMap->GetTileType(CornerIndex + mCountX) ==
				ETileType::Normal)
				return Corner;
		}

		// 위의 2가지 경우가 아니라면 왼쪽, 위 쪽으로 코너가 있는지
		// 체크해야 한다.
		FNavNode* FindNode = GetCornerRectL(Corner, EndNode);

		if (FindNode)
			return Corner;

		FindNode = GetCornerRectT(Corner, EndNode);

		if (FindNode)
			return Corner;
	}

	return nullptr;
}

bool CNavigation::SortOpenList(FNavNode* Src, FNavNode* Dest)
{
	return Src->Total > Dest->Total;
}
