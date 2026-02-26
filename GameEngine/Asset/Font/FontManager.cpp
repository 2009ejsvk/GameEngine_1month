#include "FontManager.h"
#include "../../Device.h"

CFontManager::CFontManager()
{
}

CFontManager::~CFontManager()
{
	mFontMap.clear();
	mFontCollectionMap.clear();

	auto	iter = mFontColorMap.begin();
	auto	iterEnd = mFontColorMap.end();

	for (; iter != iterEnd; ++iter)
	{
		SAFE_RELEASE(iter->second);
	}

	SAFE_RELEASE(mWriteFactory);
}

bool CFontManager::Init()
{
	if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(mWriteFactory), (IUnknown**)&mWriteFactory)))
		return false;

	LoadFontCollection("EngineDefault",
		TEXT("NotoSansKR-Regular.otf"), "EngineFont");

	LoadFont("EngineDefault", GetFontFaceName("EngineDefault"),
		600, 20.f, TEXT("ko"));

	CreateFontColor(FVector4::Black);
	CreateFontColor(FVector4::White);
	CreateFontColor(FVector4::Red);
	CreateFontColor(FVector4::Blue);
	CreateFontColor(FVector4::Green);
	CreateFontColor(255, 255, 0, 255);

	return true;
}

bool CFontManager::LoadFont(const std::string& Name,
	const TCHAR* FontName, int Weight, float FontSize,
	const TCHAR* LocalName, int Stretch)
{
	if (!FindFont(Name).expired())
		return true;

	std::shared_ptr<CFont>	Font;
	Font.reset(new CFont);

	Font->SetName(Name);

	if (!Font->LoadFont(mWriteFactory, FontName, Weight, FontSize,
		LocalName, Stretch))
		return false;

	mFontMap.insert(std::make_pair(Name, Font));

	return true;
}

bool CFontManager::LoadFontCollection(const std::string& Name,
	const TCHAR* FileName, const std::string& PathName)
{
	if (!FindFontCollection(Name).expired())
		return true;

	std::shared_ptr<CFontCollection>	Collectoin;
	Collectoin.reset(new CFontCollection);

	Collectoin->mName = Name;

	if (!Collectoin->LoadFontCollection(mWriteFactory, FileName,
		PathName))
		return false;

	mFontCollectionMap.insert(std::make_pair(Name, Collectoin));

	return true;
}

const TCHAR* CFontManager::GetFontFaceName(const std::string& Name)
{
	auto	iter = mFontCollectionMap.find(Name);

	if (iter == mFontCollectionMap.end())
		return nullptr;

	return iter->second->GetFontFaceName();
}

bool CFontManager::CreateFontColor(unsigned char r, unsigned char g,
	unsigned char b, unsigned char a)
{
	ID2D1SolidColorBrush* Brush = FindFontColor(r, g, b, a);

	if (Brush)
		return true;

	unsigned int	Key = a;
	Key <<= 8;

	Key |= r;
	Key <<= 8;

	Key |= g;
	Key <<= 8;

	Key |= b;

	if (FAILED(CDevice::GetInst()->Get2DTarget()->CreateSolidColorBrush(
		D2D1::ColorF(r / 255.f, g / 255.f, b / 255.f, a / 255.f),
		&Brush)))
		return false;

	mFontColorMap.insert(std::make_pair(Key, Brush));

	return true;
}

bool CFontManager::CreateFontColor(const FVector4& Color)
{
	ID2D1SolidColorBrush* Brush = FindFontColor(Color);

	if (Brush)
		return true;

	unsigned char	r, g, b, a;
	r = (unsigned char)(Color.x * 255);
	g = (unsigned char)(Color.y * 255);
	b = (unsigned char)(Color.z * 255);
	a = (unsigned char)(Color.w * 255);

	unsigned int	Key = a;
	Key <<= 8;

	Key |= r;
	Key <<= 8;

	Key |= g;
	Key <<= 8;

	Key |= b;

	if (FAILED(CDevice::GetInst()->Get2DTarget()->CreateSolidColorBrush(
		D2D1::ColorF(Color.x, Color.y, Color.z, Color.w),
		&Brush)))
		return false;

	mFontColorMap.insert(std::make_pair(Key, Brush));

	return true;
}

std::weak_ptr<CFont> CFontManager::FindFont(const std::string& Name)
{
	auto	iter = mFontMap.find(Name);

	if (iter == mFontMap.end())
		return std::weak_ptr<CFont>();

	return iter->second;
}

std::weak_ptr<CFontCollection> CFontManager::FindFontCollection(
	const std::string& Name)
{
	auto	iter = mFontCollectionMap.find(Name);

	if (iter == mFontCollectionMap.end())
		return std::weak_ptr<CFontCollection>();

	return iter->second;
}

ID2D1SolidColorBrush* CFontManager::FindFontColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	unsigned int	Key = a;
	Key <<= 8;

	Key |= r;
	Key <<= 8;

	Key |= g;
	Key <<= 8;

	Key |= b;

	auto	iter = mFontColorMap.find(Key);

	if (iter == mFontColorMap.end())
		return nullptr;

	return iter->second;
}

ID2D1SolidColorBrush* CFontManager::FindFontColor(
	const FVector4& Color)
{
	unsigned char	r, g, b, a;
	r = (unsigned char)(Color.x * 255);
	g = (unsigned char)(Color.y * 255);
	b = (unsigned char)(Color.z * 255);
	a = (unsigned char)(Color.w * 255);

	unsigned int	Key = a;
	Key <<= 8;

	Key |= r;
	Key <<= 8;

	Key |= g;
	Key <<= 8;

	Key |= b;

	auto	iter = mFontColorMap.find(Key);

	if (iter == mFontColorMap.end())
		return nullptr;

	return iter->second;
}

void CFontManager::ReleaseAsset(const std::string& Name)
{
	auto	iter = mFontMap.find(Name);

	if (iter != mFontMap.end())
	{
		// 다른 월드에서 더이상 들고 있지 않을 경우
		if (iter->second.use_count() == 1)
		{
			mFontMap.erase(iter);
		}
	}
}

void CFontManager::ReleaseCollectionAsset(const std::string& Name)
{
	auto	iter = mFontCollectionMap.find(Name);

	if (iter != mFontCollectionMap.end())
	{
		// 다른 월드에서 더이상 들고 있지 않을 경우
		if (iter->second.use_count() == 1)
		{
			mFontCollectionMap.erase(iter);
		}
	}
}
