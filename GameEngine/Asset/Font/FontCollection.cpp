#include "FontCollection.h"
#include "../PathManager.h"

CFontCollection::CFontCollection()
{
    mAssetType = EAssetType::FontCollection;
}

CFontCollection::~CFontCollection()
{
    SAFE_RELEASE(mCollection);
}

bool CFontCollection::LoadFontCollection(IDWriteFactory5* Factory,
    const TCHAR* FileName, const std::string& PathName)
{
    TCHAR    FullPath[MAX_PATH] = {};

    const TCHAR* Path = CPathManager::FindPath(PathName);

    if (Path)
        lstrcpy(FullPath, Path);

    lstrcat(FullPath, FileName);

    IDWriteFontFile* FontFile = nullptr;

    if (FAILED(Factory->CreateFontFileReference(FullPath, nullptr,
        &FontFile)))
        return false;

    IDWriteFontSetBuilder1* FontBuilder = nullptr;

    if (FAILED(Factory->CreateFontSetBuilder(&FontBuilder)))
        return false;

    FontBuilder->AddFontFile(FontFile);

    IDWriteFontSet* FontSet = nullptr;

    if (FAILED(FontBuilder->CreateFontSet(&FontSet)))
        return false;

    if (FAILED(Factory->CreateFontCollectionFromFontSet(FontSet,
        &mCollection)))
        return false;

    SAFE_RELEASE(FontSet);
    SAFE_RELEASE(FontBuilder);
    SAFE_RELEASE(FontFile);

    UINT32 Count = mCollection->GetFontFamilyCount();

    IDWriteFontFamily* Family = nullptr;

    if (FAILED(mCollection->GetFontFamily(0, &Family)))
        return false;

    IDWriteLocalizedStrings* LocalizedName = nullptr;

    if (FAILED(Family->GetFamilyNames(&LocalizedName)))
        return false;

    if (FAILED(LocalizedName->GetString(0, mFontFaceName, 128)))
        return false;

    SAFE_RELEASE(LocalizedName);
    SAFE_RELEASE(Family);

    return true;
}
