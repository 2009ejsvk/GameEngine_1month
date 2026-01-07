#pragma once

#include "EngineSetting.h"

class CGlobalSetting :
    public CEngineSetting
{
    friend class CEngine;

private:
    CGlobalSetting();

public:
    ~CGlobalSetting();

public:
    virtual bool Init();
};

