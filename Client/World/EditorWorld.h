#pragma once

#include "World/World.h"

class CEditorWorld :
    public CWorld
{
public:
	CEditorWorld();
	virtual ~CEditorWorld();

private:
	std::weak_ptr<class CEditorPlayer>	mPlayer;
	std::weak_ptr<class CTileMapMain>	mTileMap;

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);

private:
	void LoadAnimation2D();
	void LoadSound();
	void CreateUI();
};

