#include "ThreadNavigation.h"
#include "../Component/TileMapComponent.h"
#include "WorldNavigation.h"
#include "NavAgent.h"

CThreadNavigation::CThreadNavigation()
{
}

CThreadNavigation::~CThreadNavigation()
{
}

void CThreadNavigation::SetWorldNavigation(
	const std::weak_ptr<class CWorldNavigation>& Nav)
{
	mWorldNavigation = Nav;
}

void CThreadNavigation::SetTileMap(
	const std::weak_ptr<class CTileMapComponent>& TileMap)
{
	mNavigation.reset(new CNavigation);

	mNavigation->SetTileMap(TileMap);
}

void CThreadNavigation::Exit()
{
	CThreadBase::Exit();
}

void CThreadNavigation::Run()
{
	mLoop = true;

	while (mLoop)
	{
		if (!mQueue->empty())
		{
			int	Header = 0;
			int	Size = 0;
			unsigned char	Data[1024] = {};

			mQueue->pop(Header, Size, Data);

			switch (Header)
			{
			case ENavigationHeader::FindPath:
			{
				FVector3	Start, End;

				int	DataSize = 0;

				memcpy(&Start, Data, sizeof(FVector3));
				DataSize += sizeof(FVector3);
				memcpy(&End, Data + DataSize, sizeof(FVector3));
				DataSize += sizeof(FVector3);

				std::weak_ptr<CComponent>* Addr = nullptr;
				memcpy(&Addr, Data + DataSize,
					sizeof(std::weak_ptr<CComponent>*));

				std::shared_ptr<CNavAgent>	Agent =
					std::dynamic_pointer_cast<CNavAgent>(Addr->lock());

				// 길찾기를 수행한다.
				std::list<FVector3>	PathList;

				if (mNavigation->FindPath(Start, End, PathList))
				{
					// 길을 찾았을 경우 경로를 넣어준다.
					memset(Data, 0, 1024);
					DataSize = 0;

					memcpy(Data, &Addr, sizeof(std::weak_ptr<CComponent>*));
					DataSize += sizeof(std::weak_ptr<CComponent>*);

					int	Count = (int)PathList.size();
					memcpy(Data + DataSize, &Count, sizeof(int));
					DataSize += sizeof(int);

					auto	iter = PathList.begin();
					auto	iterEnd = PathList.end();

					for (; iter != iterEnd; ++iter)
					{
						FVector3	Pos = *iter;
						memcpy(Data + DataSize, &Pos,
							sizeof(FVector3));
						DataSize += sizeof(FVector3);
					}

					auto	WorldNav = mWorldNavigation.lock();

					WorldNav->AddData(ENavigationHeader::FindComplete,
						DataSize, Data);
				}
			}
				break;
			case ENavigationHeader::Exit:
				mLoop = false;
				return;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
