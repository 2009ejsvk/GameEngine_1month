#pragma once

#include "Shader.h"

class CComputeShader :
    public CShader
{
	friend class CShaderManager;

protected:
	CComputeShader();

public:
	virtual ~CComputeShader() = 0;

public:
	virtual bool Init(const std::string& PathName) = 0;
	virtual void SetShader() final;
};

