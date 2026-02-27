#include "TileMapRender.h"
#include "../Render/RenderManager.h"
#include "../Render/RenderState.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Texture/TextureManager.h"
#include "../Asset/Texture/Texture.h"
#include "../World/World.h"
#include "../World/WorldAssetManager.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Mesh/MeshManager.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Shader/ShaderManager.h"
#include "../Asset/Shader/CBufferTransform.h"
#include "TileMapComponent.h"
#include "../World/WorldManager.h"
#include "../World/CameraManager.h"

CTileMapRender::CTileMapRender()
{
	SetClassType<CTileMapRender>();
	mRenderType = EComponentRender::Render;
}

CTileMapRender::CTileMapRender(const CTileMapRender& ref)	:
	CSceneComponent(ref)
{
	mRenderType = EComponentRender::Render;
}

CTileMapRender::CTileMapRender(CTileMapRender&& ref) noexcept :
	CSceneComponent(ref)
{
	mRenderType = EComponentRender::Render;
}

CTileMapRender::~CTileMapRender()
{
}

void CTileMapRender::SetTileMapComponent(
	const std::weak_ptr<class CTileMapComponent>& TileMap)
{
	mTileMap = TileMap;

	auto	TileMapCom = mTileMap.lock();

	if (TileMapCom)
	{
		TileMapCom->SetTileMapRender(			std::dynamic_pointer_cast<CTileMapRender>(mSelf.lock()));

		SetWorldScale(TileMapCom->GetMapSize());
	}
}

void CTileMapRender::EnableAlphaBlend()
{
	mAlphaBlend = CRenderManager::GetInst()->FindRenderState("AlphaBlend");
}

void CTileMapRender::EnableTileAlphaBlend()
{
	mTileAlphaBlend = CRenderManager::GetInst()->FindRenderState("AlphaBlend");
}

bool CTileMapRender::SetTexture(ETileTextureType::Type Type,
	const std::weak_ptr<class CTexture>& Texture)
{
	mTexture[Type] = Texture;

	if (Type == ETileTextureType::Tile)
	{
		auto	Texture = mTexture[Type].lock();

		auto	TileMap = mTileMap.lock();

		if (TileMap)
		{
			TileMap->SetTileTextureSize((float)Texture->GetTexture()->Width, (float)Texture->GetTexture()->Height);
		}
	}

	return true;
}

bool CTileMapRender::SetTexture(ETileTextureType::Type Type,
	const std::string& Name)
{
	auto	World = mWorld.lock();

	if (World)
	{
		auto	AssetMgr = World->GetWorldAssetManager().lock();

		mTexture[Type] = AssetMgr->FindTexture(Name);
	}

	else
	{
		auto	TexMgr = CAssetManager::GetInst()->GetTextureManager().lock();

		mTexture[Type] = TexMgr->FindTexture("Texture_" + Name);
	}

	if (Type == ETileTextureType::Tile)
	{
		auto	Texture = mTexture[Type].lock();

		auto	TileMap = mTileMap.lock();

		if (TileMap)
		{
			TileMap->SetTileTextureSize((float)Texture->GetTexture()->Width, (float)Texture->GetTexture()->Height);
		}
	}

	return true;
}

bool CTileMapRender::SetTexture(ETileTextureType::Type Type,
	const std::string& Name, const TCHAR* FileName,
	const std::string& PathName)
{
	auto	World = mWorld.lock();

	if (World)
	{
		auto	AssetMgr = World->GetWorldAssetManager().lock();

		if (!AssetMgr->LoadTexture(Name, FileName, PathName))
			return false;

		mTexture[Type] = AssetMgr->FindTexture(Name);
	}

	else
	{
		auto	TexMgr = CAssetManager::GetInst()->GetTextureManager().lock();

		if (!TexMgr->LoadTexture("Texture_" + Name, FileName, PathName))
			return false;

		mTexture[Type] = TexMgr->FindTexture("Texture_" + Name);
	}

	if (Type == ETileTextureType::Tile)
	{
		auto	Texture = mTexture[Type].lock();

		auto	TileMap = mTileMap.lock();

		if (TileMap)
		{
			TileMap->SetTileTextureSize((float)Texture->GetTexture()->Width, (float)Texture->GetTexture()->Height);
		}
	}

	return true;
}

bool CTileMapRender::SetTextureFullPath(
	ETileTextureType::Type Type, const std::string& Name,
	const TCHAR* FullPath)
{
	auto	World = mWorld.lock();

	if (World)
	{
		auto	AssetMgr = World->GetWorldAssetManager().lock();

		if (!AssetMgr->LoadTextureFullPath(Name, FullPath))
			return false;

		mTexture[Type] = AssetMgr->FindTexture(Name);
	}

	else
	{
		auto	TexMgr = CAssetManager::GetInst()->GetTextureManager().lock();

		if (!TexMgr->LoadTextureFullPath("Texture_" + Name,
			FullPath))
			return false;

		mTexture[Type] = TexMgr->FindTexture("Texture_" + Name);
	}

	if (Type == ETileTextureType::Tile)
	{
		auto	Texture = mTexture[Type].lock();

		auto	TileMap = mTileMap.lock();

		if (TileMap)
		{
			TileMap->SetTileTextureSize((float)Texture->GetTexture()->Width, (float)Texture->GetTexture()->Height);
		}
	}

	return true;
}

bool CTileMapRender::SetTextureFullPath(
	ETileTextureType::Type Type, const std::string& Name,
	const std::vector<const TCHAR*>& FullPath)
{
	auto	World = mWorld.lock();

	if (World)
	{
		auto	AssetMgr = World->GetWorldAssetManager().lock();

		if (!AssetMgr->LoadTextureFullPath(Name, FullPath))
			return false;

		mTexture[Type] = AssetMgr->FindTexture(Name);
	}

	else
	{
		auto	TexMgr = CAssetManager::GetInst()->GetTextureManager().lock();

		if (!TexMgr->LoadTextureFullPath("Texture_" + Name,
			FullPath))
			return false;

		mTexture[Type] = TexMgr->FindTexture("Texture_" + Name);
	}

	if (Type == ETileTextureType::Tile)
	{
		auto	Texture = mTexture[Type].lock();

		auto	TileMap = mTileMap.lock();

		if (TileMap)
		{
			TileMap->SetTileTextureSize((float)Texture->GetTexture()->Width, (float)Texture->GetTexture()->Height);
		}
	}

	return true;
}

void CTileMapRender::SetBackMesh(
	const std::weak_ptr<class CMesh>& Mesh)
{
	mBackMesh = Mesh;
}

void CTileMapRender::SetBackMesh(const std::string& Name)
{
	auto	World = mWorld.lock();

	if (World)
	{
		auto	AssetMgr = World->GetWorldAssetManager().lock();

		mBackMesh = AssetMgr->FindMesh(Name);
	}

	else
	{
		auto	MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();

		mBackMesh = MeshMgr->FindMesh("Mesh_" + Name);
	}
}

void CTileMapRender::SetBackShader(
	const std::weak_ptr<class CShader>& Shader)
{
	mBackShader = Shader;
}

void CTileMapRender::SetBackShader(const std::string& Name)
{
	auto	ShaderMgr = CAssetManager::GetInst()->GetShaderManager().lock();

	mBackShader = ShaderMgr->FindShader(Name);
}

void CTileMapRender::SetTileFrameSize(float x, float y)
{
	auto	TileMap = mTileMap.lock();

	if (TileMap)
	{
	}
}

void CTileMapRender::AddTileFrame(const FVector2& Start,
	const FVector2& End)
{
	FTileFrame	Frame;
	Frame.Start = Start;
	Frame.End = End;

	mTileFrame.push_back(Frame);

	auto	TileMap = mTileMap.lock();

	if (TileMap)
	{
		TileMap->AddTileFrame(Start, End);
	}
}

void CTileMapRender::AddTileFrame(float StartX, float StartY,
	float EndX, float EndY)
{
	FTileFrame	Frame;
	Frame.Start = FVector2(StartX, StartY);
	Frame.End = FVector2(EndX, EndY);

	mTileFrame.push_back(Frame);

	auto	TileMap = mTileMap.lock();

	if (TileMap)
	{
		TileMap->AddTileFrame(Frame.Start, Frame.End);
	}
}

bool CTileMapRender::Init()
{
	SetRenderLayer("Map");

	CSceneComponent::Init();

	if(mTileMap.expired())
	{
		auto	Owner = mOwner.lock();

		mTileMap = Owner->FindComponent<CTileMapComponent>();
	}

	auto	TileMap = mTileMap.lock();

	if (TileMap)
	{
		SetWorldScale(TileMap->GetMapSize());
	}

	mTransformCBuffer.reset(new CCBufferTransform);

	mTransformCBuffer->Init();

	SetBackShader("DefaultTex");
	SetBackMesh("RectTex");

	return true;
}

void CTileMapRender::Render()
{
	CSceneComponent::Render();

	if (!mTexture[ETileTextureType::Back].expired())
	{
		FMatrix	ScaleMat, TranslateMat, WorldMat;

		ScaleMat.Scaling(mWorldScale);
		TranslateMat.Translation(mWorldPos);

		WorldMat = ScaleMat * TranslateMat;

		mTransformCBuffer->SetWorldMatrix(WorldMat);

		auto	World = mWorld.lock();

		if (!World)
		{
			World = CWorldManager::GetInst()->GetWorld().lock();
		}

		auto	CameraMgr = World->GetCameraManager().lock();

		mTransformCBuffer->SetViewMatrix(CameraMgr->GetViewMatrix());
		mTransformCBuffer->SetProjMatrix(CameraMgr->GetProjMatrix());

		std::shared_ptr<CShader>    Shader = mBackShader.lock();
		std::shared_ptr<CMesh>    Mesh = mBackMesh.lock();

		FVector3	PivotSize = mPivot * Mesh->GetMeshSize();

		mTransformCBuffer->SetPivotSize(PivotSize);

		mTransformCBuffer->UpdateBuffer();

		auto	Texture = mTexture[ETileTextureType::Back].lock();

		Texture->SetShader(0, EShaderBufferType::Pixel, 0);

		auto	State = mAlphaBlend.lock();

		if (State)
			State->SetState();

		Shader->SetShader();

		Mesh->Render();

		if (State)
			State->ResetState();
	}

	// 타일 출력
	if (!mTileMap.expired())
	{
		auto	TileMap = mTileMap.lock();

		if (!mTexture[ETileTextureType::Tile].expired())
		{
			auto	Texture = mTexture[ETileTextureType::Tile].lock();

			Texture->SetShader(1, EShaderBufferType::Pixel,
				0);
		}

		auto	State = mTileAlphaBlend.lock();

		if (State)
			State->SetState();

		TileMap->RenderTile();

		if (State)
			State->ResetState();

		TileMap->RenderTileOutLine();
	}
}

CTileMapRender* CTileMapRender::Clone()	const
{
	return new CTileMapRender(*this);
}

void CTileMapRender::Save(FILE* File)
{
	auto	BackShader = mBackShader.lock();

	bool	Enable = false;

	if (BackShader)
		Enable = true;

	fwrite(&Enable, sizeof(bool), 1, File);

	if (BackShader)
	{
		std::string	Name = BackShader->GetName();

		size_t	Count = Name.length();

		fwrite(&Count, sizeof(size_t), 1, File);

		fwrite(Name.c_str(), 1, Name.length(), File);
	}

	auto	BackMesh = mBackMesh.lock();

	Enable = false;

	if (BackMesh)
		Enable = true;

	fwrite(&Enable, sizeof(bool), 1, File);

	if (BackMesh)
	{
		std::string	Name = BackMesh->GetSplitName();

		size_t	Count = Name.length();

		fwrite(&Count, sizeof(size_t), 1, File);

		fwrite(Name.c_str(), 1, Name.length(), File);
	}

	for (int i = 0; i < ETileTextureType::End; ++i)
	{
		auto	Texture = mTexture[i].lock();

		Enable = false;

		if (Texture)
			Enable = true;

		fwrite(&Enable, sizeof(bool), 1, File);

		if (Texture)
		{
			std::string	Name = Texture->GetSplitName();

			size_t	Count = Name.length();

			fwrite(&Count, sizeof(size_t), 1, File);

			fwrite(Name.c_str(), 1, Name.length(), File);

			Texture->Save(File);
		}
	}

	auto	Blend = mAlphaBlend.lock();

	Enable = false;

	if (Blend)
		Enable = true;

	fwrite(&Enable, sizeof(bool), 1, File);

	auto	TileBlend = mTileAlphaBlend.lock();

	Enable = false;

	if (TileBlend)
		Enable = true;

	fwrite(&Enable, sizeof(bool), 1, File);

	size_t	Size = mTileFrame.size();

	fwrite(&Size, sizeof(size_t), 1, File);

	fwrite(&mTileFrame[0], sizeof(FTileFrame), Size, File);

	// 위치, 크기, 회전정보도 저장해둔다.
	fwrite(&mWorldScale, sizeof(FVector3), 1, File);
	fwrite(&mWorldRot, sizeof(FVector3), 1, File);
	fwrite(&mWorldPos, sizeof(FVector3), 1, File);
	fwrite(mWorldAxis, sizeof(FVector3), EAxis::End, File);
}

void CTileMapRender::Load(FILE* File)
{
	bool	Enable = false;

	fread(&Enable, sizeof(bool), 1, File);

	if (Enable)
	{
		char	Name[256] = {};

		size_t	Count = 0;

		fread(&Count, sizeof(size_t), 1, File);

		fread(Name, 1, Count, File);

		SetBackShader(Name);
	}

	Enable = false;

	fread(&Enable, sizeof(bool), 1, File);

	if (Enable)
	{
		char	Name[256] = {};

		size_t	Count = 0;

		fread(&Count, sizeof(size_t), 1, File);

		fread(Name, 1, Count, File);

		SetBackMesh(Name);
	}

	for (int i = 0; i < ETileTextureType::End; ++i)
	{
		Enable = false;

		fread(&Enable, sizeof(bool), 1, File);

		if (Enable)
		{
			char	Name[256] = {};

			size_t	Count = 0;

			fread(&Count, sizeof(size_t), 1, File);

			fread(Name, 1, Count, File);

			// 이미지 전체 경로를 얻어온다.
			size_t	TexCount = 0;
			fread(&TexCount, sizeof(size_t), 1, File);

			if (TexCount > 1)
			{
				std::vector<const TCHAR*>	FullPathArray;

				for (size_t j = 0; j < TexCount; ++j)
				{
					size_t	PathCount = 0;

					TCHAR* FullPath = new TCHAR[MAX_PATH];
					memset(FullPath, 0, sizeof(TCHAR) * MAX_PATH);

					fread(&PathCount, sizeof(size_t), 1, File);
					fread(FullPath,
						sizeof(wchar_t), PathCount, File);

					FullPathArray.push_back(FullPath);
				}

				SetTextureFullPath((ETileTextureType::Type)i,
					Name, FullPathArray);

				for (size_t j = 0; j < TexCount; ++j)
				{
					SAFE_DELETE_ARRAY(FullPathArray[j]);
				}
			}

			else
			{
				size_t	PathCount = 0;
				TCHAR	FullPath[MAX_PATH] = {};

				fread(&PathCount, sizeof(size_t), 1, File);
				fread(FullPath,
					sizeof(wchar_t), PathCount, File);

				SetTextureFullPath((ETileTextureType::Type)i,
					Name, FullPath);
			}
		}
	}

	Enable = false;

	fread(&Enable, sizeof(bool), 1, File);

	if (Enable)
	{
		EnableAlphaBlend();
	}

	Enable = false;

	fread(&Enable, sizeof(bool), 1, File);

	if (Enable)
	{
		EnableTileAlphaBlend();
	}

	size_t	Size = 0;

	fread(&Size, sizeof(size_t), 1, File);

	mTileFrame.clear();

	mTileFrame.resize(Size);

	fread(&mTileFrame[0], sizeof(FTileFrame), Size, File);

	// 위치, 크기, 회전정보도 저장해둔다.
	fread(&mWorldScale, sizeof(FVector3), 1, File);
	fread(&mWorldRot, sizeof(FVector3), 1, File);
	fread(&mWorldPos, sizeof(FVector3), 1, File);
	fread(mWorldAxis, sizeof(FVector3), EAxis::End, File);

	SetWorldPos(mWorldPos);
	SetWorldScale(mWorldScale);
	SetWorldRotation(mWorldRot);
}
