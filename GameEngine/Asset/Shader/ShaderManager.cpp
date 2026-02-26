#include "ShaderManager.h"
#include "ShaderColor2D.h"
#include "ShaderMaterialColor2D.h"
#include "ShaderTexture2D.h"
#include "ShaderDefaultTex.h"
#include "ShaderCollider.h"
#include "ShaderUIDefault.h"
#include "ShaderNullBuffer.h"
#include "ShaderTileMap.h"
#include "ShaderTileMapInstancing.h"
#include "ShaderTileMapLineInstancing.h"
#include "ShaderBlur.h"
#include "ConstantBuffer.h"
#include "ShaderTexture2DInstancing.h"
#include "ShaderMaterialColor2DInstancing.h"

CShaderManager::CShaderManager()
{
}

CShaderManager::~CShaderManager()
{
}

bool CShaderManager::Init()
{
	// ======== 상수버퍼 =========
	if (!CreateCBuffer("Transform", sizeof(FCBufferTransformData),
		0, EShaderBufferType::Graphic))
		return false;

	if (!CreateCBuffer("Material", sizeof(FCBufferMaterialData),
		1, EShaderBufferType::Pixel))
		return false;

	if (!CreateCBuffer("Animation2D", sizeof(FCBufferAnimation2DData),
		2, EShaderBufferType::VP))
		return false;

	if (!CreateCBuffer("Collider", sizeof(FCBufferColliderData),
		10, EShaderBufferType::Pixel))
		return false;

	if (!CreateCBuffer("UIDefault", sizeof(FCBufferUIDefaultData),
		10, EShaderBufferType::VP))
		return false;

	if (!CreateCBuffer("Blur", sizeof(FCBufferBlur),
		10, EShaderBufferType::Pixel))
		return false;

	if (!CreateCBuffer("TileMap", sizeof(FCBufferTileMapData),
		10, EShaderBufferType::VP))
		return false;


	// ======== Shader =========
	if (!CreateShader<CShaderColor2D>("Color2D", "EngineShader"))
		return false;

	if (!CreateShader<CShaderMaterialColor2D>("MaterialColor2D",
		"EngineShader"))
		return false;

	if (!CreateShader<CShaderMaterialColor2DInstancing>("MaterialColor2DInstancing",
		"EngineShader"))
		return false;

	if (!CreateShader<CShaderTexture2D>("DefaultTexture2D",
		"EngineShader"))
		return false;

	if (!CreateShader<CShaderTexture2DInstancing>(
		"DefaultTexture2DInstancing",
		"EngineShader"))
		return false;

	if (!CreateShader<CShaderDefaultTex>("DefaultTex",
		"EngineShader"))
		return false;

	if (!CreateShader<CShaderCollider>("Collider",
		"EngineShader"))
		return false;

	if (!CreateShader<CShaderUIDefault>("UIDefault",
		"EngineShader"))
		return false;

	if (!CreateShader<CShaderNullBuffer>("NullBuffer",
		"EngineShader"))
		return false;

	if (!CreateShader<CShaderBlur>("Blur",
		"EngineShader"))
		return false;

	if (!CreateShader<CShaderTileMap>("TileMap",
		"EngineShader"))
		return false;

	if (!CreateShader<CShaderTileMapInstancing>(
		"TileMapInstancing",
		"EngineShader"))
		return false;

	if (!CreateShader<CShaderTileMapLineInstancing>(
		"TileMapLineInstancing",
		"EngineShader"))
		return false;

	return true;
}

bool CShaderManager::CreateCBuffer(const std::string& Name, 
	int Size, int Register, int ShaderBuffer)
{
	std::weak_ptr<CConstantBuffer>	Check = FindCBuffer(Name);

	// 있을 경우
	if (!Check.expired())
		return true;

	CConstantBuffer* Origin = new CConstantBuffer;

	std::shared_ptr<CConstantBuffer> CBuffer;

	CBuffer.reset(Origin);

	if (!CBuffer->Init(Size, Register, ShaderBuffer))
		return false;

	CBuffer->SetName(Name);

	mCBufferMap.insert(std::make_pair(Name, CBuffer));

	return true;
}

std::weak_ptr<CConstantBuffer> CShaderManager::FindCBuffer(
	const std::string& Name)
{
	auto	iter = mCBufferMap.find(Name);

	if (iter == mCBufferMap.end())
		return std::weak_ptr<CConstantBuffer>();

	return iter->second;
}

std::weak_ptr<CShader> CShaderManager::FindShader(
	const std::string& Name)
{
	auto	iter = mShaderMap.find(Name);

	if (iter == mShaderMap.end())
		return std::weak_ptr<CShader>();

	return iter->second;
}

void CShaderManager::ReleaseShader(const std::string& Name)
{
	auto	iter = mShaderMap.find(Name);

	if (iter != mShaderMap.end())
	{
		// 다른 월드에서 더이상 들고 있지 않을 경우
		if (iter->second.use_count() == 1)
		{
			mShaderMap.erase(iter);
		}
	}
}

void CShaderManager::ReleaseCBuffer(const std::string& Name)
{
	auto	iter = mCBufferMap.find(Name);

	if (iter != mCBufferMap.end())
	{
		// 다른 월드에서 더이상 들고 있지 않을 경우
		if (iter->second.use_count() == 1)
		{
			mCBufferMap.erase(iter);
		}
	}
}
