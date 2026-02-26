#pragma once

#include "GraphicShader.h"

class CShaderTexture2DInstancing :
    public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderTexture2DInstancing();

public:
	virtual ~CShaderTexture2DInstancing();

public:
	virtual bool Init(const std::string& PathName);
};

