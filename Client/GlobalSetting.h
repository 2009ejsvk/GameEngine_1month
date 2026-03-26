#pragma once

#include "EngineSetting.h"
#include "ClientInfo.h"

namespace GameSession
{
    EGameMode& CurrentMode();
}

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

