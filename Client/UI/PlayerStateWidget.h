#pragma once

#include "UI/WidgetContainer.h"

class CPlayerStateWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;
    friend class CWidgetContainer;

protected:
    CPlayerStateWidget();

public:
    virtual ~CPlayerStateWidget();

private:
    std::weak_ptr<class CTextBlock> mNameText;
    std::weak_ptr<class CProgressBar> mHPBar;
    std::weak_ptr<class CProgressBar> mMPBar;

public:
    void SetPlayerName(const TCHAR* Name);
    void SetPlayerHP(float HP, float HPMax);
    void SetPlayerMP(float MP, float MPMax);

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
};

