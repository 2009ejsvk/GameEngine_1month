#pragma once

#include "GraphicShader.h"

class CShaderTileMapLineInstancing :
    public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderTileMapLineInstancing();

public:
	virtual ~CShaderTileMapLineInstancing();

public:
	virtual bool Init(const std::string& PathName);
};

