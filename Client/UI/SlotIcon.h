#pragma once

#include "UI/WidgetContainer.h"

class CSlotIcon :
    public CWidgetContainer
{
    friend class CWorldUIManager;
    friend class CInventory;
    friend class CWidgetContainer;

protected:
    CSlotIcon();
    CSlotIcon(const CSlotIcon& ref);

public:
    virtual ~CSlotIcon();

private:
    std::weak_ptr<class CImage> mImage;

public:
    void SetIconImage(const std::string& Name);
    void SetIconImage(const std::string& Name,
        const TCHAR* FileName, 
        const std::string& PathName = "Texture");

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
    virtual void MouseHovered();
    virtual void MouseUnHovered();
    virtual bool MouseDragStart(const FVector2& MousePos,
        std::shared_ptr<CWidget>& DragOperator);
    virtual bool MouseDrag(const FVector2& MousePos,
        const FVector2& MouseMove);
    virtual bool MouseDragEnd(const FVector2& MousePos);
    virtual CSlotIcon* Clone()	const;
};

