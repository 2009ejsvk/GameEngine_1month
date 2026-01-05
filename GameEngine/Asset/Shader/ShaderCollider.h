#pragma once

#include "GraphicShader.h"

class CShaderCollider :
    public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderCollider();

public:
	virtual ~CShaderCollider();

public:
	virtual bool Init(const std::string& PathName);
};

