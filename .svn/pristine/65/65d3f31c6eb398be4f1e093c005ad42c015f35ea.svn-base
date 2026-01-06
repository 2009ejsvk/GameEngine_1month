#pragma once

#include "../EngineInfo.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Mesh/MeshManager.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Material/Material.h"
#include "../Asset/Material/MaterialManager.h"
#include "../Asset/Texture/TextureManager.h"
#include "../Asset/Texture/Texture.h"
#include "../Asset/Animation2D/Animation2DManager.h"
#include "../Asset/Animation2D/Animation2D.h"

class CWorldAssetManager
{
	friend class CWorld;

private:
	CWorldAssetManager();

public:
	~CWorldAssetManager();

private:
	// AssetType_Name
	std::unordered_map<std::string, std::shared_ptr<class CAsset>>  mAssetMap;

public:
	bool Init();
	void Update(float DeltaTime);

#pragma region Mesh

public:
	bool CreateMesh(const std::string& Name, 
		void* VertexData, 
		int VertexSize, int VertexCount,
		D3D11_USAGE VertexUsage, 
		D3D11_PRIMITIVE_TOPOLOGY Primitive,
		void* IndexData = nullptr, int IndexSize = 0,
		int IndexCount = 0,
		DXGI_FORMAT Fmt = DXGI_FORMAT_UNKNOWN,
		D3D11_USAGE IndexUsage = D3D11_USAGE_DEFAULT);
	std::weak_ptr<class CMesh> FindMesh(const std::string& Name);

#pragma endregion Mesh

#pragma region Material
#pragma endregion Material

#pragma region Texture

public:
	bool LoadTexture(const std::string& Name, 
		const TCHAR* FileName,
		const std::string& PathName = "Texture");
	bool LoadTextureFullPath(const std::string& Name,
		const TCHAR* FullPath);
	bool LoadTexture(const std::string& Name,
		const std::vector<const TCHAR*>& FileName,
		const std::string& PathName = "Texture");
	bool LoadTextureFullPath(const std::string& Name,
		const std::vector<const TCHAR*> FullPath);
	std::weak_ptr<CTexture> FindTexture(
		const std::string& Name);

#pragma endregion Texture

#pragma region Animation2D

public:
	bool CreateAnimation(const std::string& Name);
	std::weak_ptr<CAnimation2D> FindAnimation(
		const std::string& Name);
	bool SetAnimation2DTextureType(const std::string& Name,
		EAnimation2DTextureType Type);
	bool SetTexture(const std::string& AnimationName,
		const std::weak_ptr<class CTexture>& Texture);
	bool SetTexture(const std::string& AnimationName,
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
		const std::vector<const TCHAR*>& FullPath);
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

#pragma endregion Animation2D
};

