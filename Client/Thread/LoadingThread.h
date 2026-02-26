#pragma once

#include "ThreadBase.h"
#include "../ClientInfo.h"

class CLoadingThread :
    public CThreadBase
{
	friend class CThreadManager;

protected:
	CLoadingThread();

public:
	virtual ~CLoadingThread();

private:
	EWorldType		mWorldType = EWorldType::None;

public:
	void SetWorldType(EWorldType Type)
	{
		mWorldType = Type;
	}

public:
	virtual void Exit();
	virtual void Run();
};

