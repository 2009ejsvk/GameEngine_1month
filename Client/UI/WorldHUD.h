#pragma once

#include "UI/WidgetContainer.h"

class CWorldHUD :
    public CWidgetContainer
{
    friend class CWorldUIManager;
    friend class CWidgetComponent;
    friend class CWidgetContainer;

protected:
    CWorldHUD();

public:
    virtual ~CWorldHUD();

private:
    std::weak_ptr<class CTextBlock> mNameText;
    std::weak_ptr<class CProgressBar> mHPBar;

public:
    void SetPlayerName(const TCHAR* Name);
    void SetPlayerHP(float HP, float HPMax);

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
};

