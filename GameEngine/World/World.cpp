#include "World.h"
#include "../Component/TileMapComponent.h"

CWorld::CWorld()
{
}

CWorld::~CWorld()
{
}

void CWorld::InputActive()
{
	mInput->DeviceAcquire();
}

void CWorld::InputDeactive()
{
	mInput->DeviceUnAcquire();
}

bool CWorld::Init()
{
	mCameraManager.reset(new CCameraManager);

	if (!mCameraManager->Init())
		return false;

	mWorldAssetManager.reset(new CWorldAssetManager);

	if (!mWorldAssetManager->Init())
		return false;

	mInput.reset(new CInput);

	mInput->mWorld = mSelf;

	if (!mInput->Init())
		return false;

	mCollision.reset(new CWorldCollision);

	mCollision->SetWorld(mSelf);

	if (!mCollision->Init())
		return false;

	mUIManager.reset(new CWorldUIManager);

	mUIManager->mWorld = mSelf;
	mUIManager->mSelf = mUIManager;

	if (!mUIManager->Init())
		return false;

	mNavigation.reset(new CWorldNavigation);

	mNavigation->mWorld = mSelf;
	mNavigation->mSelf = mNavigation;

	if (!mNavigation->Init())
		return false;

	mObjList.reserve(10000);

	mStartObjList.reserve(200);

	return true;
}

void CWorld::Update(float DeltaTime)
{
	Begin();

	// 입력은 오브젝트 업데이트 전에 해야 한다.
	mInput->Update(DeltaTime);

	auto	iter = mObjList.begin();
	auto	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (iter->second.use_count() == 0)
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetEnable())
		{
			++iter;
			continue;
		}

		iter->second->Update(DeltaTime);
		++iter;
	}

	mCameraManager->Update(DeltaTime);

	mWorldAssetManager->Update(DeltaTime);

	mNavigation->Update(DeltaTime);

	mUIManager->Update(DeltaTime);
}

void CWorld::PostUpdate(float DeltaTime)
{
	Begin();

	auto	iter = mObjList.begin();
	auto	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (iter->second.use_count() == 0)
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetEnable())
		{
			++iter;
			continue;
		}

		iter->second->PostUpdate(DeltaTime);
		++iter;
	}

	// 모든 데이터 업데이트가 완료된 후에 충돌을 진행한다.
	mCollision->Update(DeltaTime);

	iter = mObjList.begin();
	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (iter->second.use_count() == 0)
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetEnable())
		{
			++iter;
			continue;
		}

		iter->second->UpdateTransform();
		++iter;
	}
}

void CWorld::Render()
{
	Begin();

#ifdef _DEBUG

	mCollision->Render();

#endif // _DEBUG

	mCollision->ReturnNodePool();
}

void CWorld::PostRender()
{
	auto	iter = mObjList.begin();
	auto	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (iter->second.use_count() == 0)
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetEnable())
		{
			++iter;
			continue;
		}

		iter->second->PostRender();
		++iter;
	}
}

void CWorld::RenderUI()
{
	mUIManager->Render();
}

void CWorld::ClearWorld()
{
	mWorldAssetManager->ClearAsset();
}

bool CWorld::BuildNavigationSnapshot(
	const std::shared_ptr<class CTileMapComponent>& TileMap,
	int EndTileIndex,
	const std::string& PreferredTargetObjectName,
	std::vector<unsigned char>& OutBlockedMask,
	std::vector<int>& OutGoalIndices,
	std::string& OutResolvedTargetObjectName)
{
	OutBlockedMask.clear();
	OutGoalIndices.clear();
	OutResolvedTargetObjectName.clear();

	if (!TileMap)
		return false;

	const int TileCount = TileMap->GetTileCountX() * TileMap->GetTileCountY();

	if (TileCount <= 0)
		return false;

	const int MaskByteCount = (TileCount + 7) / 8;
	OutBlockedMask.resize(MaskByteCount);

	auto SetBlocked = [&](int Index)
	{
		if (Index < 0 || Index >= TileCount)
			return;

		OutBlockedMask[Index / 8] |=
			(unsigned char)(1 << (Index % 8));
	};

	auto ClearBlocked = [&](int Index)
	{
		if (Index < 0 || Index >= TileCount)
			return;

		OutBlockedMask[Index / 8] &=
			(unsigned char)~(1 << (Index % 8));
	};

	auto IsBlocked = [&](int Index)
	{
		if (Index < 0 || Index >= TileCount)
			return true;

		return (OutBlockedMask[Index / 8] &
			(unsigned char)(1 << (Index % 8))) != 0;
	};

	std::vector<unsigned char>	SeenGoalMask;
	SeenGoalMask.resize((size_t)MaskByteCount);

	auto AddGoalUnique = [&](int Index, std::vector<int>& GoalList)
	{
		if (Index < 0 || Index >= TileCount)
			return;

		unsigned char& GoalByte = SeenGoalMask[Index / 8];
		const unsigned char GoalBit = (unsigned char)
			(1 << (Index % 8));

		if (GoalByte & GoalBit)
			return;

		GoalByte |= GoalBit;
		GoalList.emplace_back(Index);
	};

	auto CollectNeighborIndices = [&](int Index, std::vector<int>& OutNeighbors)
	{
		OutNeighbors.clear();

		if (Index < 0 || Index >= TileCount)
			return;

		auto Tile = TileMap->GetTile(Index).lock();

		if (!Tile)
			return;

		const int CountX = TileMap->GetTileCountX();
		const int CountY = TileMap->GetTileCountY();

		if (TileMap->GetTileShape() == ETileShape::Rect)
		{
			const int BaseX = Tile->GetIndexX();
			const int BaseY = Tile->GetIndexY();

			for (int dy = -1; dy <= 1; ++dy)
			{
				for (int dx = -1; dx <= 1; ++dx)
				{
					if (dx == 0 && dy == 0)
						continue;

					const int nx = BaseX + dx;
					const int ny = BaseY + dy;

					if (nx < 0 || nx >= CountX ||
						ny < 0 || ny >= CountY)
					{
						continue;
					}

					OutNeighbors.emplace_back(ny * CountX + nx);
				}
			}
		}
		else
		{
			// staggered isometric <-> skew grid 변환 기반 8방향 이웃.
			const int x = Tile->GetIndexX();
			const int y = Tile->GetIndexY();
			const int GridX = x + ((y + (y & 1)) / 2);
			const int GridY = x - (y / 2);
			const int DirX[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
			const int DirY[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

			for (int d = 0; d < 8; ++d)
			{
				const int NextGridX = GridX + DirX[d];
				const int NextGridY = GridY + DirY[d];
				const int NextY = NextGridX - NextGridY;

				if (NextY < 0 || NextY >= CountY)
					continue;

				const int NextX = NextGridY + (NextY / 2);

				if (NextX < 0 || NextX >= CountX)
					continue;

				OutNeighbors.emplace_back(NextY * CountX + NextX);
			}
		}
	};

	// 기본 이동 불가 타일을 먼저 반영한다.
	for (int i = 0; i < TileCount; ++i)
	{
		if (TileMap->GetTileType(i) == ETileType::UnableToMove)
		{
			SetBlocked(i);
		}
	}

	std::shared_ptr<CGameObject> PreferredTarget;

	if (!PreferredTargetObjectName.empty())
	{
		PreferredTarget =
			FindObject<CGameObject>(PreferredTargetObjectName).lock();
	}

	std::shared_ptr<CGameObject> ResolvedTarget;
	std::vector<int> TargetGoalIndices;
	std::vector<int> GlobalGoalIndices;

	auto iter = mObjList.begin();
	auto iterEnd = mObjList.end();

	for (; iter != iterEnd; ++iter)
	{
		auto Obj = iter->second;

		if (!Obj || !Obj->GetAlive() || !Obj->GetEnable())
			continue;

		if (!Obj->IsNavigationObstacle())
			continue;

		std::vector<int> BlockedTiles;
		Obj->GetNavigationBlockedTiles(BlockedTiles);

		for (size_t i = 0; i < BlockedTiles.size(); ++i)
		{
			SetBlocked(BlockedTiles[i]);
		}

		std::vector<int> GoalTiles;
		Obj->GetNavigationGoalTiles(GoalTiles);

		for (size_t i = 0; i < GoalTiles.size(); ++i)
		{
			AddGoalUnique(GoalTiles[i], GlobalGoalIndices);
		}

		bool IsTarget = false;

		if (PreferredTarget && Obj.get() == PreferredTarget.get())
		{
			IsTarget = true;
		}

		else if (!PreferredTarget && EndTileIndex >= 0)
		{
			bool ContainsEnd = false;

			for (size_t i = 0; i < BlockedTiles.size(); ++i)
			{
				if (BlockedTiles[i] == EndTileIndex)
				{
					ContainsEnd = true;
					break;
				}
			}

			if (!ContainsEnd)
			{
				for (size_t i = 0; i < GoalTiles.size(); ++i)
				{
					if (GoalTiles[i] == EndTileIndex)
					{
						ContainsEnd = true;
						break;
					}
				}
			}

			IsTarget = ContainsEnd;
		}

		if (IsTarget)
		{
			ResolvedTarget = Obj;
			TargetGoalIndices = GoalTiles;
		}
	}

	// 노란색 목표 타일은 기본적으로 통과 가능해야 한다.
	for (size_t i = 0; i < GlobalGoalIndices.size(); ++i)
	{
		ClearBlocked(GlobalGoalIndices[i]);
	}

	// [핵심] goal 타일이 사방이 막혀 고립되면
	// orb가 goal에는 도착해도 다음 타겟으로 출발하지 못한다.
	// 각 goal에 최소 1개 탈출 이웃을 보장한다.
	std::vector<int> NeighborIndices;

	for (size_t i = 0; i < GlobalGoalIndices.size(); ++i)
	{
		const int GoalIndex = GlobalGoalIndices[i];

		CollectNeighborIndices(GoalIndex, NeighborIndices);

		bool HasOpenNeighbor = false;

		for (size_t n = 0; n < NeighborIndices.size(); ++n)
		{
			if (!IsBlocked(NeighborIndices[n]))
			{
				HasOpenNeighbor = true;
				break;
			}
		}

		if (HasOpenNeighbor)
			continue;

		// 인접 타일 중 하나를 열어 탈출 경로를 만든다.
		if (!NeighborIndices.empty())
			ClearBlocked(NeighborIndices[0]);
	}

	SeenGoalMask.assign((size_t)MaskByteCount, 0);

	for (size_t i = 0; i < TargetGoalIndices.size(); ++i)
	{
		const int Index = TargetGoalIndices[i];

		if (Index < 0 || Index >= TileCount)
			continue;

		if (IsBlocked(Index))
			continue;

		AddGoalUnique(Index, OutGoalIndices);
	}

	if (ResolvedTarget)
	{
		OutResolvedTargetObjectName = ResolvedTarget->GetName();
	}

	return true;
}

void CWorld::Begin()
{
	if (!mStartObjList.empty())
	{
		size_t	Size = mStartObjList.size();

		for (size_t i = 0; i < Size; ++i)
		{
			auto	Obj = mStartObjList[i].lock();

			Obj->Begin();
		}

		mStartObjList.clear();
	}
}

void CWorld::BeginManager()
{
	mNavigation->Begin();
}
