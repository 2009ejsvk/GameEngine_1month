#include "Animation2D.h"
#include "../Texture/Texture.h"
#include "../Texture/TextureManager.h"
#include "../AssetManager.h"

CAnimation2D::CAnimation2D()
{
	mAssetType = EAssetType::Animation2D;
}

CAnimation2D::~CAnimation2D()
{
}

void CAnimation2D::SetTexture(const std::weak_ptr<CTexture>& Texture)
{
	mTexture = Texture;
}

void CAnimation2D::SetTexture(const std::string& Name)
{
	auto	TextureManager = CAssetManager::GetInst()->GetTextureManager().lock();

	mTexture = TextureManager->FindTexture(Name);
}

void CAnimation2D::SetTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName)
{
	auto	TextureManager = CAssetManager::GetInst()->GetTextureManager().lock();

	TextureManager->LoadTexture(Name, FileName, PathName);

	mTexture = TextureManager->FindTexture(Name);
}

void CAnimation2D::SetTextureFullPath(const std::string& Name, const TCHAR* FullPath)
{
	auto	TextureManager = CAssetManager::GetInst()->GetTextureManager().lock();

	TextureManager->LoadTextureFullPath(Name, FullPath);

	mTexture = TextureManager->FindTexture(Name);
}

void CAnimation2D::SetTexture(const std::string& Name, const std::vector<const TCHAR*>& FileName, const std::string& PathName)
{
	auto	TextureManager = CAssetManager::GetInst()->GetTextureManager().lock();

	TextureManager->LoadTexture(Name, FileName, PathName);

	mTexture = TextureManager->FindTexture(Name);
}

void CAnimation2D::SetTextureFullPath(const std::string& Name, const std::vector<const TCHAR*>& FullPath)
{
	auto	TextureManager = CAssetManager::GetInst()->GetTextureManager().lock();

	TextureManager->LoadTextureFullPath(Name, FullPath);

	mTexture = TextureManager->FindTexture(Name);
}

void CAnimation2D::AddFrame(const FVector2& Start, const FVector2& Size)
{
	FTextureFrame	Frame;

	Frame.Start = Start;
	Frame.Size = Size;

	mFrameArray.push_back(Frame);
}

void CAnimation2D::AddFrame(float StartX, float StartY, float SizeX, float SizeY)
{
	FTextureFrame	Frame;

	Frame.Start = FVector2(StartX, StartY);
	Frame.Size = FVector2(SizeX, SizeY);

	mFrameArray.push_back(Frame);
}

void CAnimation2D::AddFrame(int Count, const FVector2& Start, const FVector2& Size)
{
	for (int i = 0; i < Count; ++i)
	{
		FTextureFrame	Frame;

		Frame.Start = Start;
		Frame.Size = Size;

		mFrameArray.push_back(Frame);
	}
}

void CAnimation2D::AddFrame(int Count, float StartX, float StartY, float SizeX, float SizeY)
{
	for (int i = 0; i < Count; ++i)
	{
		FTextureFrame	Frame;

		Frame.Start = FVector2(StartX, StartY);
		Frame.Size = FVector2(SizeX, SizeY);

		mFrameArray.push_back(Frame);
	}
}
