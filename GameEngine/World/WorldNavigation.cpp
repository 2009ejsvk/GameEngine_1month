#include "WorldNavigation.h"
#include "../Component/TileMapComponent.h"
#include "../ThreadManager.h"
#include "../World/NavAgent.h"
#include "World.h"
#include <cstdarg>
#include <cstring>
#include <cstdio>

namespace
{
#ifdef _DEBUG
	void DebugNavLog(const char* Format, ...)
	{
		char Text[512] = {};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Text, Format, Args);
		va_end(Args);

		OutputDebugStringA(Text);
	}
#endif

	template <typename T>
	bool WriteValue(unsigned char* Data, int Capacity,
		int& Offset, const T& Value)
	{
		if (Offset + (int)sizeof(T) > Capacity)
			return false;

		memcpy(Data + Offset, &Value, sizeof(T));
		Offset += (int)sizeof(T);
		return true;
	}

	bool WriteRaw(unsigned char* Data, int Capacity,
		int& Offset, const void* Src, int Size)
	{
		if (Size < 0 || Offset + Size > Capacity)
			return false;

		if (Size > 0)
		{
			memcpy(Data + Offset, Src, Size);
			Offset += Size;
		}

		return true;
	}

	template <typename T>
	bool ReadValue(const unsigned char* Data, int Capacity,
		int& Offset, T& OutValue)
	{
		if (Offset + (int)sizeof(T) > Capacity)
			return false;

		memcpy(&OutValue, Data + Offset, sizeof(T));
		Offset += (int)sizeof(T);
		return true;
	}

	bool ReadRaw(const unsigned char* Data, int Capacity,
		int& Offset, void* Dest, int Size)
	{
		if (Size < 0 || Offset + Size > Capacity)
			return false;

		if (Size > 0)
		{
			memcpy(Dest, Data + Offset, Size);
			Offset += Size;
		}

		return true;
	}
}

CWorldNavigation::CWorldNavigation()
{
}

CWorldNavigation::~CWorldNavigation()
{
	size_t	Size = mThreadList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		std::string	Name = mThreadList[i]->GetName();

		// 종료 메세지를 스레드에 보내준다.
		mThreadList[i]->AddData(ENavigationHeader::Exit,
			0, nullptr);

		CThreadManager::GetInst()->DeleteThread(Name);
	}
}

bool CWorldNavigation::AddData(int Header, int Size,
	unsigned char* Data)
{
	return mNavQueue->push(Header, Size, Data);
}

void CWorldNavigation::Begin()
{
	mFrameCache.clear();

	size_t	Size = mThreadList.size();

#ifdef _DEBUG
	DebugNavLog("[Nav] Begin resume threads=%zu\n", Size);
#endif

	for (size_t i = 0; i < Size; ++i)
	{
		mThreadList[i]->Resume();
	}
}

bool CWorldNavigation::Init()
{
	mNavQueue.reset(new CThreadQueue);

	return true;
}

void CWorldNavigation::Update(float DeltaTime)
{
	int	QueueCount = mNavQueue->size();

	int	Header = 0, Size = 0;
	unsigned char	Data[THREAD_QUEUE_DATA_SIZE] = {};

	for (int i = 0; i < QueueCount; ++i)
	{
		mNavQueue->pop(Header, Size, Data);

		switch (Header)
		{
		case ENavigationHeader::FindComplete:
			NavigationComplete(Size, Data);
			break;
		}
	}

	// FindPath에서 사용하는 스냅샷 캐시는 프레임 단위 캐시다.
	// 다음 프레임에서 건물 이동/배치 변경을 반영하려면 매 프레임 비워야 한다.
	mFrameCache.clear();
}

void CWorldNavigation::CreateNavigationThread(int Count,
	const std::weak_ptr<CTileMapComponent>& TileMap)
{
	// 네비게이션 타일맵은 최초 등록된 맵을 기준으로 고정한다.
	// (오버레이 타일맵 생성 시 재등록되면 스냅샷 기준이 흔들린다.)
	if (mTileMap.expired())
	{
		mTileMap = TileMap;
	}

	// 스레드는 월드당 1회만 생성한다.
	if (!mThreadList.empty())
	{
#ifdef _DEBUG
		DebugNavLog(
			"[Nav] CreateNavigationThread skipped requested=%d existing=%zu\n",
			Count, mThreadList.size());
#endif
		return;
	}

	if (Count <= 0)
	{
		Count = 1;
	}

	auto	World = mWorld.lock();

	if (!World)
		return;

	unsigned __int64	Addr = (unsigned __int64)World.get();

	char	WorldAddr[64] = {};

	sprintf_s(WorldAddr, "%llu_Thread", Addr);

	for (int i = 0; i < Count; ++i)
	{
		char	ThreadName[128] = {};

		sprintf_s(ThreadName, "%s%d", WorldAddr, i);

		// 로딩 스레드를 생성한다.
		std::shared_ptr<CThreadNavigation> Thread =
			CThreadManager::GetInst()->Create<CThreadNavigation>(ThreadName, true);

		Thread->SetWorldNavigation(mSelf);
		Thread->SetTileMap(mTileMap);

		mThreadList.push_back(Thread);
	}

	// 스레드를 CREATE_SUSPENDED 모드로 생성했으므로 즉시 깨운다.
	// BeginManager()는 월드 초기화 시점에 1회만 호출되어
	// TileMap 로드 이후 생성된 스레드는 Resume되지 않는다.
	for (size_t i = 0; i < mThreadList.size(); ++i)
	{
		mThreadList[i]->Resume();
	}

#ifdef _DEBUG
	DebugNavLog("[Nav] CreateNavigationThread requested=%d total=%zu\n",
		Count, mThreadList.size());
#endif
}

bool CWorldNavigation::FindPath(const FVector3& Start,
	const FVector3& End, std::weak_ptr<CComponent>* Agent,
	const std::string& TargetObjectName, unsigned int RequestId)
{
	if (!Agent)
	{
#ifdef _DEBUG
		DebugNavLog("[Nav] FindPath ignored: null agent\n");
#endif
		return false;
	}

	if (mThreadList.empty())
	{
#ifdef _DEBUG
		DebugNavLog("[Nav] FindPath ignored: thread list empty target=%s req=%u\n",
			TargetObjectName.c_str(), RequestId);
#endif
		return false;
	}

	int	QueueCount = INT_MAX;
	int	Index = -1;

	size_t	Size = mThreadList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		int	Count = mThreadList[i]->GetQueueSize();

		if (Count < QueueCount)
		{
			Index = (int)i;

			if (Count == 0)
				break;

			QueueCount = Count;
		}
	}

	if (Index < 0)
	{
#ifdef _DEBUG
		DebugNavLog("[Nav] FindPath ignored: no thread selected target=%s req=%u\n",
			TargetObjectName.c_str(), RequestId);
#endif
		return false;
	}

	std::vector<unsigned char>	BlockedMask;
	std::vector<int> GoalIndices;
	std::string	ResolvedTargetObjectName = TargetObjectName;

	auto World = mWorld.lock();
	auto TileMap = mTileMap.lock();

	if (World && TileMap)
	{
		const int EndTileIndex = TileMap->GetTileIndex(End);

		// 같은 목적지에 대한 스냅샷은 프레임당 한 번만 계산한다.
		// 키: 이름이 있으면 건물 이름(최대 10개), 없으면 타일 인덱스(마커 위치 수).
		const std::string CacheKey = TargetObjectName.empty()
			? std::to_string(EndTileIndex)
			: TargetObjectName;

		auto CacheIt = mFrameCache.find(CacheKey);

		if (CacheIt != mFrameCache.end())
		{
			BlockedMask              = CacheIt->second.BlockedMask;
			GoalIndices              = CacheIt->second.GoalIndices;
			ResolvedTargetObjectName = CacheIt->second.ResolvedTargetObjectName;
		}
		else
		{
			World->BuildNavigationSnapshot(
				TileMap,
				EndTileIndex,
				TargetObjectName,
				BlockedMask,
				GoalIndices,
				ResolvedTargetObjectName);

			FNavCacheEntry& Entry        = mFrameCache[CacheKey];
			Entry.BlockedMask            = BlockedMask;
			Entry.GoalIndices            = GoalIndices;
			Entry.ResolvedTargetObjectName = ResolvedTargetObjectName;
		}
	}

	// 이름 기반 목표 요청인데 실제 건물을 찾지 못하면
	// 요청을 실패로 처리해 상위(FSM)가 즉시 타겟을 교체할 수 있게 한다.
	if (!TargetObjectName.empty() &&
		ResolvedTargetObjectName.empty())
	{
#ifdef _DEBUG
		DebugNavLog(
			"[Nav] FindPath invalid target target=%s req=%u\n",
			TargetObjectName.c_str(),
			RequestId);
#endif

		auto NavAgent =
			std::dynamic_pointer_cast<CNavAgent>(Agent->lock());

		if (NavAgent)
		{
			NavAgent->StartPathPoint();
			NavAgent->SetPathTargetObjectName("");
		}

		return false;
	}

	if (!ResolvedTargetObjectName.empty() && GoalIndices.empty())
	{
#ifdef _DEBUG
		DebugNavLog(
			"[Nav] FindPath no goal target=%s resolved=%s req=%u endTile=%d blockedBytes=%d\n",
			TargetObjectName.c_str(),
			ResolvedTargetObjectName.c_str(),
			RequestId,
			TileMap ? TileMap->GetTileIndex(End) : -1,
			(int)BlockedMask.size());
#endif

		auto NavAgent =
			std::dynamic_pointer_cast<CNavAgent>(Agent->lock());

		if (NavAgent)
		{
			NavAgent->StartPathPoint();
			NavAgent->SetPathTargetObjectName("");
		}

		return false;
	}

	int	Header = ENavigationHeader::FindPath;

	unsigned char	Data[THREAD_QUEUE_DATA_SIZE] = {};
	int	DataSize = 0;

	if (!WriteValue(Data, THREAD_QUEUE_DATA_SIZE, DataSize, Start) ||
		!WriteValue(Data, THREAD_QUEUE_DATA_SIZE, DataSize, End) ||
		!WriteValue(Data, THREAD_QUEUE_DATA_SIZE, DataSize, Agent) ||
		!WriteValue(Data, THREAD_QUEUE_DATA_SIZE, DataSize, RequestId))
	{
		return false;
	}

	const int BlockMaskByteCount = (int)BlockedMask.size();

	if (!WriteValue(Data, THREAD_QUEUE_DATA_SIZE, DataSize,
		BlockMaskByteCount))
	{
		return false;
	}

	if (!WriteRaw(Data, THREAD_QUEUE_DATA_SIZE, DataSize,
		BlockedMask.data(), BlockMaskByteCount))
	{
		return false;
	}

	const int GoalCount = (int)GoalIndices.size();

	if (!WriteValue(Data, THREAD_QUEUE_DATA_SIZE, DataSize, GoalCount))
	{
		return false;
	}

	if (!WriteRaw(Data, THREAD_QUEUE_DATA_SIZE, DataSize,
		GoalIndices.data(), GoalCount * (int)sizeof(int)))
	{
		return false;
	}

	const int TargetNameLength = (int)ResolvedTargetObjectName.size();

	if (!WriteValue(Data, THREAD_QUEUE_DATA_SIZE, DataSize,
		TargetNameLength))
	{
		return false;
	}

	if (!WriteRaw(Data, THREAD_QUEUE_DATA_SIZE, DataSize,
		ResolvedTargetObjectName.data(), TargetNameLength))
	{
		return false;
	}

	const bool Enqueued =
		mThreadList[Index]->AddData(Header, DataSize, Data);

	if (!Enqueued)
	{
#ifdef _DEBUG
		DebugNavLog(
			"[Nav] Enqueue failed req=%u target=%s resolved=%s goals=%d blockedBytes=%d thread=%d q=%d\n",
			RequestId,
			TargetObjectName.c_str(),
			ResolvedTargetObjectName.c_str(),
			GoalCount,
			BlockMaskByteCount,
			Index,
			mThreadList[Index]->GetQueueSize());
#endif
		return false;
	}

#ifdef _DEBUG
	DebugNavLog(
		"[Nav] Enqueue req=%u target=%s resolved=%s goals=%d blockedBytes=%d thread=%d\n",
		RequestId,
		TargetObjectName.c_str(),
		ResolvedTargetObjectName.c_str(),
		GoalCount,
		BlockMaskByteCount,
		Index);
#endif

	return true;
}

void CWorldNavigation::NavigationComplete(int Size,
	unsigned char* Data)
{
	int	DataSize = 0;

	std::weak_ptr<CComponent>* Addr = nullptr;

	if (!ReadValue(Data, Size, DataSize, Addr))
		return;

	if (!Addr)
		return;

	std::shared_ptr<CNavAgent>	Agent =
		std::dynamic_pointer_cast<CNavAgent>(Addr->lock());

	if (!Agent)
		return;

	unsigned int RequestId = 0;

	if (!ReadValue(Data, Size, DataSize, RequestId))
		return;

	if (!Agent->IsExpectedPathRequestId(RequestId))
	{
#ifdef _DEBUG
		DebugNavLog("[Nav] Drop stale path req=%u\n", RequestId);
#endif
		return;
	}

	int	PathCount = 0;

	if (!ReadValue(Data, Size, DataSize, PathCount))
		return;

	Agent->StartPathPoint();

	for (int i = 0; i < PathCount; ++i)
	{
		FVector3	Pos;

		if (!ReadValue(Data, Size, DataSize, Pos))
			return;

		Agent->AddPathPoint(Pos);
	}

	int TargetNameLength = 0;

	if (!ReadValue(Data, Size, DataSize, TargetNameLength))
	{
		Agent->SetPathTargetObjectName("");
		return;
	}

	std::string TargetObjectName;

	if (TargetNameLength > 0)
	{
		TargetObjectName.resize((size_t)TargetNameLength);

		if (!ReadRaw(Data, Size, DataSize,
			&TargetObjectName[0], TargetNameLength))
		{
			TargetObjectName.clear();
		}
	}

	Agent->SetPathTargetObjectName(TargetObjectName);

#ifdef _DEBUG
	DebugNavLog("[Nav] Complete req=%u pathCount=%d target=%s\n",
		RequestId, PathCount,
		TargetObjectName.empty() ? "<none>" : TargetObjectName.c_str());
#endif

	if (PathCount > 0)
	{
		Agent->StartPath();
	}
}
