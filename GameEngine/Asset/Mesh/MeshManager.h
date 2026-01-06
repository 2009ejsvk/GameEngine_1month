#pragma once

#include "../../EngineInfo.h"

class CMeshManager
{
	friend class CAssetManager;

private:
	CMeshManager();

public:
	~CMeshManager();

private:
	std::unordered_map<std::string, std::shared_ptr<class CMesh>>	mMeshMap;

public:
	bool Init();
	bool CreateMesh(const std::string& Name, void* VertexData, int VertexSize,
		int VertexCount,
		D3D11_USAGE VertexUsage, D3D11_PRIMITIVE_TOPOLOGY Primitive,
		void* IndexData = nullptr, int IndexSize = 0, int IndexCount = 0,
		DXGI_FORMAT Fmt = DXGI_FORMAT_UNKNOWN,
		D3D11_USAGE IndexUsage = D3D11_USAGE_DEFAULT);
	std::weak_ptr<class CMesh> FindMesh(const std::string& Name);
	void ReleaseAsset(const std::string& Name);
};

