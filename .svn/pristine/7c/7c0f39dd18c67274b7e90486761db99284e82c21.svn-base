#pragma once

#include "EngineInfo.h"

class CStateInterface	abstract
{
public:
	CStateInterface();
	virtual ~CStateInterface();

protected:
	int		mStateHP = 10;

public:
	void AddHP(int HP)
	{
		mStateHP += HP;

		char	Test[256] = {};
		sprintf_s(Test, "HP : %d\n", mStateHP);
		OutputDebugStringA(Test);
	}
};

