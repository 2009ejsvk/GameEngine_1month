#pragma once

#include "Asset/Shader/GraphicShader.h"

class CShaderPostProcessHit :
    public CGraphicShader
{
	friend class CShaderManager;

protected:
	CShaderPostProcessHit();

public:
	virtual ~CShaderPostProcessHit();

public:
	virtual bool Init(const std::string& PathName);
};

