#pragma once

#include "GraphicShader.h"

class CShaderMaterialColor2DInstancing :
    public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderMaterialColor2DInstancing();

public:
	virtual ~CShaderMaterialColor2DInstancing();

public:
	virtual bool Init(const std::string& PathName);
};

