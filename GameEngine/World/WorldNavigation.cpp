#include "WorldNavigation.h"
#include "../Component/TileMapComponent.h"
#include "../ThreadManager.h"
#include "../World/NavAgent.h"

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

void CWorldNavigation::AddData(int Header, int Size,
	unsigned char* Data)
{
	mNavQueue->push(Header, Size, Data);
}

void CWorldNavigation::Begin()
{
	size_t	Size = mThreadList.size();

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
	unsigned char	Data[1024] = {};

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
}

void CWorldNavigation::CreateNavigationThread(int Count,
	const std::weak_ptr<CTileMapComponent>& TileMap)
{
	auto	World = mWorld.lock();

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
		Thread->SetTileMap(TileMap);

		mThreadList.push_back(Thread);
	}
}

void CWorldNavigation::FindPath(const FVector3& Start,
	const FVector3& End, std::weak_ptr<CComponent>* Agent)
{
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

	int	Header = ENavigationHeader::FindPath;

	unsigned char	Data[1024] = {};

	int	DataSize = 0;

	memcpy(Data, &Start, sizeof(FVector3));
	DataSize += sizeof(FVector3);

	memcpy(Data + DataSize, &End, sizeof(FVector3));
	DataSize += sizeof(FVector3);

	memcpy(Data + DataSize, &Agent,
		sizeof(std::weak_ptr<CComponent>*));
	DataSize += sizeof(std::weak_ptr<CComponent>*);

	mThreadList[Index]->AddData(Header, DataSize, Data);
}

void CWorldNavigation::NavigationComplete(int Size,
	unsigned char* Data)
{
	int	DataSize = 0;

	std::weak_ptr<CComponent>* Addr = nullptr;

	memcpy(&Addr, Data, sizeof(std::weak_ptr<CComponent>*));
	DataSize += sizeof(std::weak_ptr<CComponent>*);

	std::shared_ptr<CNavAgent>	Agent =
		std::dynamic_pointer_cast<CNavAgent>(Addr->lock());

	int	PathCount = 0;
	memcpy(&PathCount, Data + DataSize, sizeof(int));
	DataSize += sizeof(int);

	Agent->StartPathPoint();

	for (int i = 0; i < PathCount; ++i)
	{
		FVector3	Pos;
		memcpy(&Pos, Data + DataSize, sizeof(FVector3));
		DataSize += sizeof(FVector3);

		Agent->AddPathPoint(Pos);
	}

	Agent->StartPath();
}
