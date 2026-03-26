#pragma once

#include "UI/WidgetContainer.h"

class CStartWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CStartWidget();

public:
    virtual ~CStartWidget();

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();

private:
    void SandboxClick();
    void ScenarioClick();
    void ExitClick();
};

