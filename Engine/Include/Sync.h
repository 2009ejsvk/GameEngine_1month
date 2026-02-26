#pragma once

#include "EngineInfo.h"

class CSync
{
public:
	CSync(CRITICAL_SECTION* Crt)
	{
		mCrt = Crt;

		EnterCriticalSection(mCrt);
	}

	~CSync()
	{
		if (mCrt)
			LeaveCriticalSection(mCrt);
	}

private:
	CRITICAL_SECTION* mCrt = nullptr;
};

