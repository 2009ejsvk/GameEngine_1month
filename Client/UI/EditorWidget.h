#pragma once

#include "UI/WidgetContainer.h"
#include "../ClientInfo.h"

class CEditorWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CEditorWidget();

public:
    virtual ~CEditorWidget();

private:
    std::shared_ptr<class CTextBlock> mEditorMode;
    std::shared_ptr<class CTextBlock> mTileType;
    std::shared_ptr<class CTextBlock> mTileFrame;

public:
    void SetEditorMode(EEditorMode Mode);
    void SetTileFrame(int Frame);
    void SetTileType(ETileType TileType);

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
};

