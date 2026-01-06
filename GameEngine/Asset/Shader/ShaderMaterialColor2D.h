#pragma once

#include "GraphicShader.h"

class CShaderMaterialColor2D :
    public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderMaterialColor2D();

public:
	virtual ~CShaderMaterialColor2D();

public:
	virtual bool Init(const std::string& PathName);
};

