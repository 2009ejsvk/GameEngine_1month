#pragma once

#include "World/World.h"
#include "../ClientInfo.h"

class CLoadingWorld :
    public CWorld
{
public:
	CLoadingWorld();
	virtual ~CLoadingWorld();

private:
	std::shared_ptr<class CLoadingThread>	mThread;
	EWorldType	mLoadType;

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	void Load(EWorldType WorldType);

private:
	void LoadAnimation2D();
	void LoadSound();
	void CreateUI();
};

