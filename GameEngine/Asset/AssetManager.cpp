#include "AssetManager.h"
#include "Mesh/MeshManager.h"
#include "Shader/ShaderManager.h"
#include "Texture/TextureManager.h"
#include "Animation2D/Animation2DManager.h"
#include "Sound/SoundManager.h"
#include "Font/FontManager.h"
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

std::weak_ptr<CShaderManager> CAssetManager::GetShaderManager()
{
	return mShaderManager;
}

std::weak_ptr<CTextureManager> CAssetManager::GetTextureManager()
{
	return mTextureManager;
}

std::weak_ptr<CAnimation2DManager> CAssetManager::GetAnimation2DManager()
{
	return mAnimation2DManager;
}

std::weak_ptr<CSoundManager> CAssetManager::GetSoundManager()
{
	return mSoundManager;
}

std::weak_ptr<class CFontManager> CAssetManager::GetFontManager()
{
	return mFontManager;
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

	mSoundManager.reset(new CSoundManager);

	if (!mSoundManager->Init())
		return false;

	mFontManager.reset(new CFontManager);

	if (!mFontManager->Init())
		return false;

	return true;
}

void CAssetManager::Update()
{
	mSoundManager->Update();
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
	case EAssetType::Sound:
		mSoundManager->ReleaseAsset(Name);
		break;
	case EAssetType::Font:
		mFontManager->ReleaseAsset(Name);
		break;
	case EAssetType::FontCollection:
		mFontManager->ReleaseCollectionAsset(Name);
		break;
	}
}
