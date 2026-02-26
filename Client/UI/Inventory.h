#pragma once

#include "UI/WidgetContainer.h"

class CInventory :
    public CWidgetContainer
{
    friend class CWorldUIManager;
    friend class CWidgetContainer;

protected:
    CInventory();

public:
    virtual ~CInventory();

private:
    std::vector<std::weak_ptr<class CSlot>> mSlotArray;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();

private:
    void CloseButtonClick();
};

