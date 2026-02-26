#pragma once

#include "GraphicShader.h"

class CShaderDefaultTex :
    public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderDefaultTex();

public:
	virtual ~CShaderDefaultTex();

public:
	virtual bool Init(const std::string& PathName);
};

