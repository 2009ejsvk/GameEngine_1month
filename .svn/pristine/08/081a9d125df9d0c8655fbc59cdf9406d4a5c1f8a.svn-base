#pragma once

#include "Animation2D.h"

class CAnimation2DManager
{
	friend class CAssetManager;

private:
	CAnimation2DManager();

public:
	~CAnimation2DManager();

private:
	std::unordered_map<std::string, std::shared_ptr<CAnimation2D>>	  mAnimation2DMap;

public:
	bool Init();
	bool CreateAnimation(const std::string& Name);
	std::weak_ptr<CAnimation2D> FindAnimation(
		const std::string& Name);
	bool SetAnimation2DTextureType(const std::string& Name,
		EAnimation2DTextureType Type);
    bool SetTexture(const std::string& AnimationName,
        const std::weak_ptr<class CTexture>& Texture);
    /*bool SetTexture(const std::string& AnimationName,
        const std::string& Name);
    bool SetTexture(const std::string& AnimationName,
        const std::string& Name,
        const TCHAR* FileName,
        const std::string& PathName = "Texture");
    bool SetTextureFullPath(const std::string& AnimationName,
        const std::string& Name,
        const TCHAR* FullPath);
    bool SetTexture(const std::string& AnimationName,
        const std::string& Name,
        const std::vector<const TCHAR*>& FileName,
        const std::string& PathName = "Texture");
    bool SetTextureFullPath(const std::string& AnimationName,
        const std::string& Name,
        const std::vector<const TCHAR*>& FullPath);*/
    bool AddFrame(const std::string& AnimationName,
        const FVector2& Start, const FVector2& Size);
    bool AddFrame(const std::string& AnimationName,
        float StartX, float StartY, float SizeX,
        float SizeY);
    bool AddFrame(const std::string& AnimationName,
        int Count, const FVector2& Start, const FVector2& Size);
    bool AddFrame(const std::string& AnimationName,
        int Count, float StartX, float StartY, float SizeX,
        float SizeY);
    void ReleaseAsset(const std::string& Name);
};

