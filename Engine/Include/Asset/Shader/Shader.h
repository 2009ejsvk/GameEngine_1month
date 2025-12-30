#pragma once

#include "../Asset.h"

class CShader :
    public CAsset
{
	friend class CShaderManager;

protected:
	CShader();

public:
	virtual ~CShader() = 0;

public:
	virtual bool Init(const std::string& PathName) = 0;
	virtual void SetShader() = 0;
};

