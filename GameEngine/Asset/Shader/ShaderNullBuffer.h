#pragma once

#include "GraphicShader.h"

class CShaderNullBuffer :
    public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderNullBuffer();

public:
	virtual ~CShaderNullBuffer();

public:
	virtual bool Init(const std::string& PathName);
};

