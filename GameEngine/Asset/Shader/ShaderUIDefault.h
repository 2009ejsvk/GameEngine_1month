#pragma once

#include "GraphicShader.h"

class CShaderUIDefault :
    public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderUIDefault();

public:
	virtual ~CShaderUIDefault();

public:
	virtual bool Init(const std::string& PathName);
};

