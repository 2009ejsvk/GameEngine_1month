#pragma once

#include "../EngineInfo.h"

class CAssetManager
{
private:
	CAssetManager();
	~CAssetManager();

private:
	std::shared_ptr<class CMeshManager>		mMeshManager;
	std::shared_ptr<class CShaderManager>	mShaderManager;
	std::shared_ptr<class CTextureManager>	mTextureManager;
	std::shared_ptr<class CAnimation2DManager>	mAnimation2DManager;

public:
	std::weak_ptr<class CMeshManager> GetMeshManager();
	std::weak_ptr<class CShaderManager> GetShaderManager();
	std::weak_ptr<class CTextureManager> GetTextureManager();
	std::weak_ptr<class CAnimation2DManager> GetAnimation2DManager();

public:
	bool Init();
	void ReleaseAsset(const std::string& Name,
		EAssetType AssetType);

private:
	static CAssetManager* mInst;

public:
	static CAssetManager* GetInst()
	{
		if (!mInst)
			mInst = new CAssetManager;
		return mInst;
	}

	static void DestroyInst()
	{
		SAFE_DELETE(mInst);
	}
};

