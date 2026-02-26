#pragma once

#include "GraphicShader.h"

class CShaderTileMap :
    public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderTileMap();

public:
	virtual ~CShaderTileMap();

public:
	virtual bool Init(const std::string& PathName);
};

