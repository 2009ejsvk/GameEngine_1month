#pragma once

#include "UI/WidgetContainer.h"

class CMainWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CMainWidget();

public:
    virtual ~CMainWidget();

private:
    std::shared_ptr<class CPlayerStateWidget> mPlayerStateWidget;

public:
    std::shared_ptr<class CPlayerStateWidget> GetPlayerState()  const
    {
        return mPlayerStateWidget;
    }

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
};

