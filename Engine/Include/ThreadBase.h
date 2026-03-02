#pragma once

#include "EngineInfo.h"
#include "ThreadQueue.h"
#include "Sync.h"
#include <thread>

class CThreadBase abstract
{
	friend class CThreadManager;

protected:
	CThreadBase();

public:
	virtual ~CThreadBase();

protected:
	std::string		mName;
	HANDLE			mThread = nullptr;
	std::shared_ptr<CThreadQueue>	mQueue;
	bool			mLoop = false;
	bool			mComplete = false;
	CRITICAL_SECTION	mCrt;

public:
	bool GetComplete()
	{
		CSync   sync(&mCrt);

		return mComplete;
	}

public:
	void SetName(const std::string& Name)
	{
		mName = Name;
	}

	const std::string& GetName()	const
	{
		return mName;
	}

public:
	bool Init(bool Pause = false);
	virtual void Exit();
	virtual void Run() = 0;

public:
	bool AddData(int Header, int Size, unsigned char* Data);
	void GetData(int& Header, int& Size, unsigned char* Data);
	int GetQueueSize();
	bool EmptyQueue();

public:
	void Suspend();
	void Resume();

private:
	static unsigned int __stdcall ThreadFunc(void* Arg);
};

