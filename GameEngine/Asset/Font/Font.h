#pragma once

#include "../Asset.h"

class CFont :
    public CAsset
{
    friend class CFontManager;

protected:
    CFont();

public:
    virtual ~CFont();

private:
    IDWriteFactory5* mFactory = nullptr;
    IDWriteTextFormat* mFormat = nullptr;

public:
    bool LoadFont(IDWriteFactory5* Factory, const TCHAR* FontName,
        int Weight, float FontSize, const TCHAR* LocalName,
        int Stretch = DWRITE_FONT_STRETCH_NORMAL);
    IDWriteTextLayout* CreateLayout(const TCHAR* Text, int Length,
        float Width, float Height);
};

