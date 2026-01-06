#pragma once

#include "../Asset.h"
#include "CBufferContainer.h"

class CConstantBuffer :
    public CAsset
{
    friend class CShaderManager;

private:
    CConstantBuffer();

public:
    ~CConstantBuffer();

private:
    ID3D11Buffer* mBuffer = nullptr;
    int     mSize = 0;

    // 상수버퍼의 레지스터 번호
    int     mRegister = 0;

    // 어떤 Shader에서 이 상수버퍼를 사용하는지 판단하는 용도
    int     mShaderBuffer = 0;

public:
    bool Init(int Size, int Register, int ShaderBuffer);
    void Update(void* Data);
};

