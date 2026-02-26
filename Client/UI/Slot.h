#pragma once

#include "UI/WidgetContainer.h"

class CSlot :
    public CWidgetContainer
{
    friend class CWorldUIManager;
    friend class CInventory;
    friend class CWidgetContainer;

protected:
    CSlot();

public:
    virtual ~CSlot();

private:
    std::shared_ptr<class CSlotIcon>    mIcon;

public:
    void EnableIcon(bool Enable);
    void SetIconImage(const std::string& Name);
    void SetIconImage(const std::string& Name,
        const TCHAR* FileName,
        const std::string& PathName = "Texture");

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
};

