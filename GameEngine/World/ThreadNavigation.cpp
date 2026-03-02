#include "ThreadNavigation.h"
#include "../Component/TileMapComponent.h"
#include "WorldNavigation.h"
#include "NavAgent.h"
#include <cstdarg>
#include <cstring>
#include <cstdio>

namespace
{
#ifdef _DEBUG
	void DebugThreadNavLog(const char* Format, ...)
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
}

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
			unsigned char	Data[THREAD_QUEUE_DATA_SIZE] = {};

			mQueue->pop(Header, Size, Data);

			switch (Header)
			{
			case ENavigationHeader::FindPath:
			{
				FVector3	Start, End;
				std::weak_ptr<CComponent>* Addr = nullptr;
				unsigned int RequestId = 0;
				int DataSize = 0;

				if (!ReadValue(Data, Size, DataSize, Start) ||
					!ReadValue(Data, Size, DataSize, End) ||
					!ReadValue(Data, Size, DataSize, Addr) ||
					!ReadValue(Data, Size, DataSize, RequestId))
				{
					break;
				}

				if (!Addr)
					break;

				int BlockMaskByteCount = 0;

				if (!ReadValue(Data, Size, DataSize, BlockMaskByteCount))
					break;

				std::vector<unsigned char> BlockMask;

				if (BlockMaskByteCount > 0)
				{
					BlockMask.resize((size_t)BlockMaskByteCount);

					if (!ReadRaw(Data, Size, DataSize,
						&BlockMask[0], BlockMaskByteCount))
					{
						break;
					}
				}

				int GoalCount = 0;

				if (!ReadValue(Data, Size, DataSize, GoalCount))
					break;

				std::vector<int> GoalIndices;

				if (GoalCount > 0)
				{
					GoalIndices.resize((size_t)GoalCount);

					if (!ReadRaw(Data, Size, DataSize,
						&GoalIndices[0],
						GoalCount * (int)sizeof(int)))
					{
						break;
					}
				}

				int TargetNameLength = 0;

				if (!ReadValue(Data, Size, DataSize, TargetNameLength))
					break;

				std::string TargetObjectName;

				if (TargetNameLength > 0)
				{
					TargetObjectName.resize((size_t)TargetNameLength);

					if (!ReadRaw(Data, Size, DataSize,
						&TargetObjectName[0], TargetNameLength))
					{
						break;
					}
				}

				std::shared_ptr<CNavAgent>	Agent =
					std::dynamic_pointer_cast<CNavAgent>(Addr->lock());

				if (!Agent)
					break;

				// 길찾기를 수행한다.
				std::list<FVector3>	PathList;

				if (mNavigation->FindPath(Start, End, PathList,
					BlockMask.empty() ? nullptr : &BlockMask[0],
					(int)BlockMask.size(),
					GoalIndices.empty() ? nullptr : &GoalIndices[0],
					(int)GoalIndices.size()))
				{
					// 길을 찾았을 경우 경로를 넣어준다.
					memset(Data, 0, THREAD_QUEUE_DATA_SIZE);
					DataSize = 0;

					if (!WriteValue(Data, THREAD_QUEUE_DATA_SIZE,
						DataSize, Addr))
					{
						break;
					}

					if (!WriteValue(Data, THREAD_QUEUE_DATA_SIZE,
						DataSize, RequestId))
					{
						break;
					}

					const int TargetNameWriteLength =
						(int)TargetObjectName.size();

					const int HeaderSize =
						(int)sizeof(std::weak_ptr<CComponent>*) +
						(int)sizeof(unsigned int) +
						(int)sizeof(int) +
						(int)sizeof(int) +
						TargetNameWriteLength;

					int MaxPointCount =
						(THREAD_QUEUE_DATA_SIZE - HeaderSize) /
						(int)sizeof(FVector3);

					if (MaxPointCount < 0)
						MaxPointCount = 0;

					int	Count = (int)PathList.size();

					if (Count > MaxPointCount)
						Count = MaxPointCount;

					if (!WriteValue(Data, THREAD_QUEUE_DATA_SIZE,
						DataSize, Count))
					{
						break;
					}

					auto	iter = PathList.begin();
					auto	iterEnd = PathList.end();

					int WrittenCount = 0;

					for (; iter != iterEnd && WrittenCount < Count;
						++iter, ++WrittenCount)
					{
						FVector3	Pos = *iter;

						if (!WriteValue(Data, THREAD_QUEUE_DATA_SIZE,
							DataSize, Pos))
						{
							break;
						}
					}

					if (!WriteValue(Data, THREAD_QUEUE_DATA_SIZE,
						DataSize, TargetNameWriteLength))
					{
						break;
					}

					if (!WriteRaw(Data, THREAD_QUEUE_DATA_SIZE,
						DataSize,
						TargetObjectName.data(),
						TargetNameWriteLength))
					{
						break;
					}

					auto	WorldNav = mWorldNavigation.lock();

					if (WorldNav)
					{
						bool Submitted = false;

						for (int Retry = 0; Retry < 8; ++Retry)
						{
							if (WorldNav->AddData(
								ENavigationHeader::FindComplete,
								DataSize, Data))
							{
								Submitted = true;
								break;
							}

							std::this_thread::sleep_for(
								std::chrono::milliseconds(1));
						}

#ifdef _DEBUG
						if (!Submitted)
						{
							DebugThreadNavLog(
								"[ThreadNav] Drop completion req=%u target=%s (world queue full)\n",
								RequestId,
								TargetObjectName.empty() ? "<none>" : TargetObjectName.c_str());
						}
#endif
					}
				}

#ifdef _DEBUG
				else
				{
					DebugThreadNavLog(
						"[ThreadNav] FindPath failed req=%u target=%s start=(%.1f, %.1f) end=(%.1f, %.1f) goals=%d blockMaskBytes=%d\n",
						RequestId,
						TargetObjectName.empty() ? "<none>" : TargetObjectName.c_str(),
						Start.x, Start.y,
						End.x, End.y,
						(int)GoalIndices.size(),
						(int)BlockMask.size());
				}
#endif
			}
				break;
			case ENavigationHeader::Exit:
				mLoop = false;
				return;
			}
		}
		else
		{
			// 큐가 빌 때만 sleep — 아이템이 있을 땐 즉시 다음 처리
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}
