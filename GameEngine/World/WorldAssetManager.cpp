#include "WorldAssetManager.h"

CWorldAssetManager::CWorldAssetManager()
{
}

CWorldAssetManager::~CWorldAssetManager()
{
	auto	iter = mAssetMap.begin();

	for (; iter != mAssetMap.end();)
	{
		EAssetType	Type = iter->second->GetAssetType();

		std::string	Name = iter->second->GetName();

		// 카운트 1 감소.
		iter = mAssetMap.erase(iter);

		CAssetManager::GetInst()->ReleaseAsset(Name, Type);
	}
}

bool CWorldAssetManager::Init()
{
	// ColorMesh 사각형 생성
	FVertexColor	CenterRectColor[4] =
	{
		FVertexColor(-0.5f, 0.5f, 0.f, 1.f, 0.f, 0.f, 1.f),
		FVertexColor(0.5f, 0.5f, 0.f, 0.f, 1.f, 0.f, 1.f),
		FVertexColor(-0.5f, -0.5f, 0.f, 0.f, 0.f, 1.f, 1.f),
		FVertexColor(0.5f, -0.5f, 0.f, 1.f, 1.f, 0.f, 1.f)
	};

	unsigned short	CenterRectColorIdx[6] = { 0, 1, 3, 0, 3, 2 };

	if (!CreateMesh("CenterRectColor", CenterRectColor, sizeof(FVertexColor),
		4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		CenterRectColorIdx, 2, 6, DXGI_FORMAT_R16_UINT,
		D3D11_USAGE_IMMUTABLE))
		return false;

	return true;
}

void CWorldAssetManager::Update(float DeltaTime)
{
}

#pragma region Mesh

bool CWorldAssetManager::CreateMesh(const std::string& Name,
	void* VertexData,
	int VertexSize, int VertexCount,
	D3D11_USAGE VertexUsage,
	D3D11_PRIMITIVE_TOPOLOGY Primitive,
	void* IndexData, int IndexSize,
	int IndexCount,
	DXGI_FORMAT Fmt,
	D3D11_USAGE IndexUsage)
{
	auto MeshMgr = 
		CAssetManager::GetInst()->GetMeshManager().lock();

	if (MeshMgr)
	{
		std::string	Key = "Mesh_" + Name;

		// 메시를 새로 만들거나 같은 메시가 이미 있을 경우
		// 통과.
		if (!MeshMgr->CreateMesh(Key, VertexData, VertexSize,
			VertexCount, VertexUsage, Primitive, IndexData,
			IndexSize, IndexCount, Fmt, IndexUsage))
			return false;

		auto	iter = mAssetMap.find(Key);

		// 현재 월드의 AssetMap에 추가가 안되어 있을 경우
		if (iter == mAssetMap.end())
		{
			// MeshManager와 mAssetMap에 들어가 있는 Mesh는
			// 공용이다. 그러므로 서로 shared_ptr이므로 같은
			// 객체를 공유하며 카운트가 둘다 1씩 증가하여
			// 2가 되어 있을 것이다.
			mAssetMap.insert(std::make_pair(Key,
				MeshMgr->FindMesh(Key)));
		}

		return true;
	}

	return false;
}

std::weak_ptr<CMesh> CWorldAssetManager::FindMesh(
	const std::string& Name)
{
	std::string	Key = "Mesh_" + Name;

	auto	iter = mAssetMap.find(Key);

	// 현재 월드의 AssetMap에 추가가 안되어 있을 경우
	if (iter == mAssetMap.end())
	{
		auto MeshMgr =
			CAssetManager::GetInst()->GetMeshManager().lock();

		auto	Mesh = MeshMgr->FindMesh(Key);

		if (!Mesh.expired())
		{
			mAssetMap.insert(std::make_pair(Key, Mesh));
		}

		iter = mAssetMap.find(Key);
	}

	return std::dynamic_pointer_cast<CMesh>(iter->second);
}

#pragma endregion Mesh

#pragma region Material
#pragma endregion Material

#pragma region Texture

bool CWorldAssetManager::LoadTexture(const std::string& Name,
	const TCHAR* FileName,
	const std::string& PathName)
{
	auto TextureMgr =
		CAssetManager::GetInst()->GetTextureManager().lock();

	if (TextureMgr)
	{
		std::string	Key = "Texture_" + Name;

		if (!TextureMgr->LoadTexture(Key, FileName, PathName))
			return false;

		auto	iter = mAssetMap.find(Key);

		// 현재 월드의 AssetMap에 추가가 안되어 있을 경우
		if (iter == mAssetMap.end())
		{
			// MeshManager와 mAssetMap에 들어가 있는 Mesh는
			// 공용이다. 그러므로 서로 shared_ptr이므로 같은
			// 객체를 공유하며 카운트가 둘다 1씩 증가하여
			// 2가 되어 있을 것이다.
			mAssetMap.insert(std::make_pair(Key,
				TextureMgr->FindTexture(Key)));
		}

		return true;
	}

	return false;
}

bool CWorldAssetManager::LoadTextureFullPath(
	const std::string& Name,
	const TCHAR* FullPath)
{
	auto TextureMgr =
		CAssetManager::GetInst()->GetTextureManager().lock();

	if (TextureMgr)
	{
		std::string	Key = "Texture_" + Name;

		if (!TextureMgr->LoadTextureFullPath(Key, FullPath))
			return false;

		auto	iter = mAssetMap.find(Key);

		// 현재 월드의 AssetMap에 추가가 안되어 있을 경우
		if (iter == mAssetMap.end())
		{
			// MeshManager와 mAssetMap에 들어가 있는 Mesh는
			// 공용이다. 그러므로 서로 shared_ptr이므로 같은
			// 객체를 공유하며 카운트가 둘다 1씩 증가하여
			// 2가 되어 있을 것이다.
			mAssetMap.insert(std::make_pair(Key,
				TextureMgr->FindTexture(Key)));
		}

		return true;
	}

	return false;
}

bool CWorldAssetManager::LoadTexture(const std::string& Name,
	const std::vector<const TCHAR*>& FileName,
	const std::string& PathName)
{
	auto TextureMgr =
		CAssetManager::GetInst()->GetTextureManager().lock();

	if (TextureMgr)
	{
		std::string	Key = "Texture_" + Name;

		if (!TextureMgr->LoadTexture(Key, FileName, PathName))
			return false;

		auto	iter = mAssetMap.find(Key);

		// 현재 월드의 AssetMap에 추가가 안되어 있을 경우
		if (iter == mAssetMap.end())
		{
			// MeshManager와 mAssetMap에 들어가 있는 Mesh는
			// 공용이다. 그러므로 서로 shared_ptr이므로 같은
			// 객체를 공유하며 카운트가 둘다 1씩 증가하여
			// 2가 되어 있을 것이다.
			mAssetMap.insert(std::make_pair(Key,
				TextureMgr->FindTexture(Key)));
		}

		return true;
	}

	return false;
}

bool CWorldAssetManager::LoadTextureFullPath(
	const std::string& Name,
	const std::vector<const TCHAR*> FullPath)
{
	auto TextureMgr =
		CAssetManager::GetInst()->GetTextureManager().lock();

	if (TextureMgr)
	{
		std::string	Key = "Texture_" + Name;

		if (!TextureMgr->LoadTextureFullPath(Key, FullPath))
			return false;

		auto	iter = mAssetMap.find(Key);

		// 현재 월드의 AssetMap에 추가가 안되어 있을 경우
		if (iter == mAssetMap.end())
		{
			// MeshManager와 mAssetMap에 들어가 있는 Mesh는
			// 공용이다. 그러므로 서로 shared_ptr이므로 같은
			// 객체를 공유하며 카운트가 둘다 1씩 증가하여
			// 2가 되어 있을 것이다.
			mAssetMap.insert(std::make_pair(Key,
				TextureMgr->FindTexture(Key)));
		}

		return true;
	}

	return false;
}

std::weak_ptr<CTexture> CWorldAssetManager::FindTexture(
	const std::string& Name)
{
	std::string	Key = "Texture_" + Name;

	auto	iter = mAssetMap.find(Key);

	// 현재 월드의 AssetMap에 추가가 안되어 있을 경우
	if (iter == mAssetMap.end())
	{
		auto TextureMgr =
			CAssetManager::GetInst()->GetTextureManager().lock();

		auto	Texture = TextureMgr->FindTexture(Key);

		if (!Texture.expired())
		{
			mAssetMap.insert(std::make_pair(Key, Texture));
		}

		iter = mAssetMap.find(Key);
	}

	return std::dynamic_pointer_cast<CTexture>(iter->second);
}


#pragma endregion Texture

#pragma region Animation2D

bool CWorldAssetManager::CreateAnimation(
	const std::string& Name)
{
	auto Animation2DMgr =
		CAssetManager::GetInst()->GetAnimation2DManager().lock();

	if (Animation2DMgr)
	{
		std::string	Key = "Animation2D_" + Name;

		if (!Animation2DMgr->CreateAnimation(Key))
			return false;

		auto	iter = mAssetMap.find(Key);

		// 현재 월드의 AssetMap에 추가가 안되어 있을 경우
		if (iter == mAssetMap.end())
		{
			// MeshManager와 mAssetMap에 들어가 있는 Mesh는
			// 공용이다. 그러므로 서로 shared_ptr이므로 같은
			// 객체를 공유하며 카운트가 둘다 1씩 증가하여
			// 2가 되어 있을 것이다.
			mAssetMap.insert(std::make_pair(Key,
				Animation2DMgr->FindAnimation(Key)));
		}

		return true;
	}

	return false;
}

std::weak_ptr<CAnimation2D> CWorldAssetManager::FindAnimation(
	const std::string& Name)
{
	std::string	Key = "Animation2D_" + Name;

	auto	iter = mAssetMap.find(Key);

	// 현재 월드의 AssetMap에 추가가 안되어 있을 경우
	if (iter == mAssetMap.end())
	{
		auto Animation2DMgr =
			CAssetManager::GetInst()->GetAnimation2DManager().lock();

		auto	Animation2D = Animation2DMgr->FindAnimation(Key);

		if (!Animation2D.expired())
		{
			mAssetMap.insert(std::make_pair(Key, Animation2D));
		}

		iter = mAssetMap.find(Key);
	}

	return std::dynamic_pointer_cast<CAnimation2D>(iter->second);
}

bool CWorldAssetManager::SetAnimation2DTextureType(
	const std::string& Name,
	EAnimation2DTextureType Type)
{
	std::string	Key = "Animation2D_" + Name;

	auto	iter = mAssetMap.find(Key);

	if (iter == mAssetMap.end())
		return false;

	std::dynamic_pointer_cast<CAnimation2D>(iter->second)->SetAnimation2DTextureType(Type);

	return true;
}

bool CWorldAssetManager::SetTexture(
	const std::string& AnimationName,
	const std::weak_ptr<CTexture>& Texture)
{
	std::string	Key = "Animation2D_" + AnimationName;

	auto	iter = mAssetMap.find(Key);

	if (iter == mAssetMap.end())
		return false;

	std::dynamic_pointer_cast<CAnimation2D>(iter->second)->SetTexture(Texture);

	return true;
}

bool CWorldAssetManager::SetTexture(
	const std::string& AnimationName,
	const std::string& Name)
{
	std::string	Key = "Animation2D_" + AnimationName;

	auto	iter = mAssetMap.find(Key);

	if (iter == mAssetMap.end())
		return false;

	std::weak_ptr<CTexture> Texture = FindTexture(Name);

	if (Texture.expired())
		return false;

	std::dynamic_pointer_cast<CAnimation2D>(iter->second)->SetTexture(Texture);

	return true;
}

bool CWorldAssetManager::SetTexture(
	const std::string& AnimationName,
	const std::string& Name,
	const TCHAR* FileName,
	const std::string& PathName)
{
	std::string	Key = "Animation2D_" + AnimationName;

	auto	iter = mAssetMap.find(Key);

	if (iter == mAssetMap.end())
		return false;

	if (!LoadTexture(Name, FileName, PathName))
		return false;

	std::weak_ptr<CTexture> Texture = FindTexture(Name);

	std::dynamic_pointer_cast<CAnimation2D>(iter->second)->SetTexture(Texture);

	return true;
}

bool CWorldAssetManager::SetTextureFullPath(
	const std::string& AnimationName,
	const std::string& Name,
	const TCHAR* FullPath)
{
	std::string	Key = "Animation2D_" + AnimationName;

	auto	iter = mAssetMap.find(Key);

	if (iter == mAssetMap.end())
		return false;

	if (!LoadTextureFullPath(Name, FullPath))
		return false;

	std::weak_ptr<CTexture> Texture = FindTexture(Name);

	std::dynamic_pointer_cast<CAnimation2D>(iter->second)->SetTexture(Texture);

	return true;
}

bool CWorldAssetManager::SetTexture(
	const std::string& AnimationName,
	const std::string& Name,
	const std::vector<const TCHAR*>& FileName,
	const std::string& PathName)
{
	std::string	Key = "Animation2D_" + AnimationName;

	auto	iter = mAssetMap.find(Key);

	if (iter == mAssetMap.end())
		return false;

	if (!LoadTexture(Name, FileName, PathName))
		return false;

	std::weak_ptr<CTexture> Texture = FindTexture(Name);

	std::dynamic_pointer_cast<CAnimation2D>(iter->second)->SetTexture(Texture);

	return true;
}

bool CWorldAssetManager::SetTextureFullPath(
	const std::string& AnimationName,
	const std::string& Name,
	const std::vector<const TCHAR*>& FullPath)
{
	std::string	Key = "Animation2D_" + AnimationName;

	auto	iter = mAssetMap.find(Key);

	if (iter == mAssetMap.end())
		return false;

	if (!LoadTextureFullPath(Name, FullPath))
		return false;

	std::weak_ptr<CTexture> Texture = FindTexture(Name);

	std::dynamic_pointer_cast<CAnimation2D>(iter->second)->SetTexture(Texture);

	return true;
}

bool CWorldAssetManager::AddFrame(
	const std::string& AnimationName,
	const FVector2& Start, const FVector2& Size)
{
	std::string	Key = "Animation2D_" + AnimationName;

	auto	iter = mAssetMap.find(Key);

	if (iter == mAssetMap.end())
		return false;

	std::dynamic_pointer_cast<CAnimation2D>(iter->second)->AddFrame(Start, Size);

	return true;
}

bool CWorldAssetManager::AddFrame(
	const std::string& AnimationName,
	float StartX, float StartY, float SizeX,
	float SizeY)
{
	std::string	Key = "Animation2D_" + AnimationName;

	auto	iter = mAssetMap.find(Key);

	if (iter == mAssetMap.end())
		return false;

	std::dynamic_pointer_cast<CAnimation2D>(iter->second)->AddFrame(StartX, StartY, SizeX, SizeY);

	return true;
}

bool CWorldAssetManager::AddFrame(
	const std::string& AnimationName,
	int Count, const FVector2& Start, const FVector2& Size)
{
	std::string	Key = "Animation2D_" + AnimationName;

	auto	iter = mAssetMap.find(Key);

	if (iter == mAssetMap.end())
		return false;

	std::dynamic_pointer_cast<CAnimation2D>(iter->second)->AddFrame(Count, Start, Size);

	return true;
}

bool CWorldAssetManager::AddFrame(
	const std::string& AnimationName,
	int Count, float StartX, float StartY, float SizeX,
	float SizeY)
{
	std::string	Key = "Animation2D_" + AnimationName;

	auto	iter = mAssetMap.find(Key);

	if (iter == mAssetMap.end())
		return false;

	std::dynamic_pointer_cast<CAnimation2D>(iter->second)->AddFrame(Count, StartX, StartY, SizeX, SizeY);

	return true;
}


#pragma endregion Animation2D