#pragma once

#include "../EngineInfo.h"

class CRenderState
{
	friend class CRenderManager;

protected:
	CRenderState();

public:
	virtual ~CRenderState();

protected:
	ID3D11DeviceChild* mState = nullptr;
	ID3D11DeviceChild* mPrevState = nullptr;

public:
	virtual void SetState() = 0;
	virtual void ResetState() = 0;
};

