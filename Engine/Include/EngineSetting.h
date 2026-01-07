#pragma once

#include "EngineInfo.h"

class CEngineSetting
{
	friend class CEngine;

protected:
	CEngineSetting();

public:
	~CEngineSetting();

public:
	virtual bool Init();
};
