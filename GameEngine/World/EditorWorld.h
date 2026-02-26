#pragma once

#include "World.h"

class CEditorWorld :
    public CWorld
{
public:
	CEditorWorld();
	virtual ~CEditorWorld();

public:
	virtual bool Init();

private:
	void LoadAnimation2D();
	void LoadSound();
	void CreateUI();
};

