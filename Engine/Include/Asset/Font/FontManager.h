#pragma once

#include "Font.h"
#include "FontCollection.h"

class CFontManager
{
	friend class CAssetManager;

private:
	CFontManager();

public:
	~CFontManager();

private:
	IDWriteFactory5* mWriteFactory = nullptr;

	std::unordered_map<std::string, std::shared_ptr<CFont>>	mFontMap;
	std::unordered_map<std::string, std::shared_ptr<CFontCollection>>	mFontCollectionMap;
	std::unordered_map<unsigned int, ID2D1SolidColorBrush*>	mFontColorMap;

public:
	bool Init();
	bool LoadFont(const std::string& Name, const TCHAR* FontName,
		int Weight, float FontSize, const TCHAR* LocalName,
		int Stretch = DWRITE_FONT_STRETCH_NORMAL);
	bool LoadFontCollection(const std::string& Name,
		const TCHAR* FileName, const std::string& PathName = "Font");
	const TCHAR* GetFontFaceName(const std::string& Name);
	bool CreateFontColor(unsigned char r, unsigned char g,
		unsigned char b, unsigned char a);
	bool CreateFontColor(const FVector4& Color);

	std::weak_ptr<CFont> FindFont(const std::string& Name);
	std::weak_ptr<CFontCollection> FindFontCollection(const std::string& Name);
	ID2D1SolidColorBrush* FindFontColor(unsigned char r,
		unsigned char g,
		unsigned char b, unsigned char a);
	ID2D1SolidColorBrush* FindFontColor(const FVector4& Color);

public:
	void ReleaseAsset(const std::string& Name);
	void ReleaseCollectionAsset(const std::string& Name);
};

