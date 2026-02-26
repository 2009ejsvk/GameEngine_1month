#pragma once

#include "GraphicShader.h"

class CShaderTileMapInstancing :
    public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderTileMapInstancing();

public:
	virtual ~CShaderTileMapInstancing();

public:
	virtual bool Init(const std::string& PathName);
};

