#pragma once

#include "UI/WidgetContainer.h"

class CLoadingWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CLoadingWidget();

public:
    virtual ~CLoadingWidget();

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
};

