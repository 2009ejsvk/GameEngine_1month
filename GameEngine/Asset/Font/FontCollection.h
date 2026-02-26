#pragma once

#include "../Asset.h"

class CFontCollection :
    public CAsset
{
    friend class CFontManager;

protected:
    CFontCollection();

public:
    virtual ~CFontCollection();

private:
    IDWriteFontCollection1* mCollection = nullptr;
    TCHAR                   mFontFaceName[128] = {};

public:
    const TCHAR* GetFontFaceName()  const
    {
        return mFontFaceName;
    }

public:
    bool LoadFontCollection(IDWriteFactory5* Factory,
        const TCHAR* FileName, const std::string& PathName = "Font");
};

