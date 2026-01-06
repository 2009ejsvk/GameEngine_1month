#include "AssetManager.h"
#include "Mesh/MeshManager.h"
#include "Shader/ShaderManager.h"
#include "Texture/TextureManager.h"
#include "Animation2D/Animation2DManager.h"
#include "PathManager.h"

CAssetManager* CAssetManager::mInst = nullptr;

CAssetManager::CAssetManager()
{
}

CAssetManager::~CAssetManager()
{
	CPathManager::Clear();
}

std::weak_ptr<CMeshManager> CAssetManager::GetMeshManager()
{
	return mMeshManager;
}

std::weak_ptr<class CShaderManager> CAssetManager::GetShaderManager()
{
	return mShaderManager;
}

std::weak_ptr<class CTextureManager> CAssetManager::GetTextureManager()
{
	return mTextureManager;
}

std::weak_ptr<class CAnimation2DManager> CAssetManager::GetAnimation2DManager()
{
	return mAnimation2DManager;
}

bool CAssetManager::Init()
{
	// 경로 관리자 초기화
	if (!CPathManager::Init())
		return false;

	mShaderManager.reset(new CShaderManager);

	if (!mShaderManager->Init())
		return false;

	mTextureManager.reset(new CTextureManager);

	if (!mTextureManager->Init())
		return false;

	mMeshManager.reset(new CMeshManager);

	if (!mMeshManager->Init())
		return false;

	mAnimation2DManager.reset(new CAnimation2DManager);

	if (!mAnimation2DManager->Init())
		return false;

	return true;
}

void CAssetManager::ReleaseAsset(
	const std::string& Name,
	EAssetType AssetType)
{
	switch (AssetType)
	{
	case EAssetType::Mesh:
		mMeshManager->ReleaseAsset(Name);
		break;
	case EAssetType::Shader:
		mShaderManager->ReleaseShader(Name);
		break;
	case EAssetType::ConstantBuffer:
		mShaderManager->ReleaseCBuffer(Name);
		break;
	case EAssetType::Material:
		break;
	case EAssetType::Texture:
		mTextureManager->ReleaseAsset(Name);
		break;
	case EAssetType::Animation2D:
		mAnimation2DManager->ReleaseAsset(Name);
		break;
	}
}
