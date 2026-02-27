#pragma once

#include "Sync.h"

constexpr int THREAD_QUEUE_DATA_SIZE = 8192;

struct FThreadQueueData
{
	int		Header = 0;
	int		Size = 0;
	unsigned char	Data[THREAD_QUEUE_DATA_SIZE] = {};
};

class CThreadQueue
{
	friend class CThreadBase;

public:
	CThreadQueue();
	~CThreadQueue();

private:
	FThreadQueueData	mData[200];
	int		mPush = 0;
	int		mPop = 0;
	int		mSize = 0;
	CRITICAL_SECTION	mCrt;

public:
	void push(int Header, int Size, unsigned char* Data);
	void pop(int& Header, int& Size, unsigned char* Data);
	int size();
	bool full();
	bool empty();
};

