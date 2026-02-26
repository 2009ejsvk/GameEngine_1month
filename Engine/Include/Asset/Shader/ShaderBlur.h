#pragma once

#include "GraphicShader.h"

class CShaderBlur :
	public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderBlur();

public:
	virtual ~CShaderBlur();

public:
	virtual bool Init(const std::string& PathName);
};

