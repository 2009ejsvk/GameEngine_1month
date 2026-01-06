#include "Animation2DManager.h"
#include "../Texture/Texture.h"

CAnimation2DManager::CAnimation2DManager()
{
}

CAnimation2DManager::~CAnimation2DManager()
{
}

bool CAnimation2DManager::Init()
{
	return true;
}

bool CAnimation2DManager::CreateAnimation(
	const std::string& Name)
{
	if (!FindAnimation(Name).expired())
		return true;

	std::shared_ptr<CAnimation2D>	Animation(new CAnimation2D);

	Animation->SetName(Name);

	mAnimation2DMap.insert(std::make_pair(Name, Animation));

	return true;
}

std::weak_ptr<CAnimation2D> CAnimation2DManager::FindAnimation(
	const std::string& Name)
{
	auto	iter = mAnimation2DMap.find(Name);

	if (iter == mAnimation2DMap.end())
		return std::weak_ptr<CAnimation2D>();

	return iter->second;
}

bool CAnimation2DManager::SetAnimation2DTextureType(const std::string& Name, EAnimation2DTextureType Type)
{
	auto	iter = mAnimation2DMap.find(Name);

	if (iter == mAnimation2DMap.end())
		return false;

	iter->second->SetAnimation2DTextureType(Type);

	return true;
}

bool CAnimation2DManager::SetTexture(const std::string& AnimationName, const std::weak_ptr<CTexture>& Texture)
{
	auto	iter = mAnimation2DMap.find(AnimationName);

	if (iter == mAnimation2DMap.end())
		return false;

	iter->second->SetTexture(Texture);

	return true;
}
//
//bool CAnimation2DManager::SetTexture(const std::string& AnimationName, const std::string& Name)
//{
//	auto	iter = mAnimation2DMap.find(AnimationName);
//
//	if (iter == mAnimation2DMap.end())
//		return false;
//
//	iter->second->SetTexture(Name);
//
//	return true;
//}
//
//bool CAnimation2DManager::SetTexture(const std::string& AnimationName, const std::string& Name, const TCHAR* FileName, const std::string& PathName)
//{
//	auto	iter = mAnimation2DMap.find(AnimationName);
//
//	if (iter == mAnimation2DMap.end())
//		return false;
//
//	iter->second->SetTexture(Name, FileName, PathName);
//
//	return true;
//}
//
//bool CAnimation2DManager::SetTextureFullPath(const std::string& AnimationName, const std::string& Name, const TCHAR* FullPath)
//{
//	auto	iter = mAnimation2DMap.find(AnimationName);
//
//	if (iter == mAnimation2DMap.end())
//		return false;
//
//	iter->second->SetTextureFullPath(Name, FullPath);
//
//	return true;
//}
//
//bool CAnimation2DManager::SetTexture(const std::string& AnimationName, const std::string& Name, const std::vector<const TCHAR*>& FileName, const std::string& PathName)
//{
//	auto	iter = mAnimation2DMap.find(AnimationName);
//
//	if (iter == mAnimation2DMap.end())
//		return false;
//
//	iter->second->SetTexture(Name, FileName, PathName);
//
//	return true;
//}
//
//bool CAnimation2DManager::SetTextureFullPath(const std::string& AnimationName, const std::string& Name, const std::vector<const TCHAR*>& FullPath)
//{
//	auto	iter = mAnimation2DMap.find(AnimationName);
//
//	if (iter == mAnimation2DMap.end())
//		return false;
//
//	iter->second->SetTextureFullPath(Name, FullPath);
//
//	return true;
//}

bool CAnimation2DManager::AddFrame(const std::string& AnimationName, const FVector2& Start, const FVector2& Size)
{
	auto	iter = mAnimation2DMap.find(AnimationName);

	if (iter == mAnimation2DMap.end())
		return false;

	iter->second->AddFrame(Start, Size);

	return true;
}

bool CAnimation2DManager::AddFrame(const std::string& AnimationName, float StartX, float StartY, float SizeX, float SizeY)
{
	auto	iter = mAnimation2DMap.find(AnimationName);

	if (iter == mAnimation2DMap.end())
		return false;

	iter->second->AddFrame(StartX, StartY, SizeX, SizeY);

	return true;
}

bool CAnimation2DManager::AddFrame(const std::string& AnimationName, int Count, const FVector2& Start, const FVector2& Size)
{
	auto	iter = mAnimation2DMap.find(AnimationName);

	if (iter == mAnimation2DMap.end())
		return false;

	iter->second->AddFrame(Count, Start, Size);

	return true;
}

bool CAnimation2DManager::AddFrame(const std::string& AnimationName, int Count, float StartX, float StartY, float SizeX, float SizeY)
{
	auto	iter = mAnimation2DMap.find(AnimationName);

	if (iter == mAnimation2DMap.end())
		return false;

	iter->second->AddFrame(Count, StartX, StartY, SizeX, SizeY);

	return true;
}

void CAnimation2DManager::ReleaseAsset(
	const std::string& Name)
{
	auto	iter = mAnimation2DMap.find(Name);

	if (iter != mAnimation2DMap.end())
	{
		// 다른 월드에서 더이상 들고 있지 않을 경우
		if (iter->second.use_count() == 1)
		{
			mAnimation2DMap.erase(iter);
		}
	}
}
