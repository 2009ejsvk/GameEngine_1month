#include "TileMapComponent.h"
#include "../World/World.h"
#include "../World/WorldNavigation.h"
#include "../World/WorldAssetManager.h"
#include "../World/CameraManager.h"
#include "../World/WorldManager.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Shader/ShaderManager.h"
#include "../Asset/Shader/CBufferTransform.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Mesh/MeshManager.h"
#include "../Asset/Texture/Texture.h"
#include "../Asset/Texture/TextureManager.h"
#include "../Object/GameObject.h"
#include "TileMapRender.h"
#include "../Device.h"
#include "../Asset/Shader/CBufferTileMap.h"
#include "../Asset/PathManager.h"

CTileMapComponent::CTileMapComponent()
{
	SetClassType<CTileMapComponent>();
}

CTileMapComponent::CTileMapComponent(
	const CTileMapComponent& ref) :
	CObjectComponent(ref)
{
}

CTileMapComponent::CTileMapComponent(
	CTileMapComponent&& ref) noexcept :
	CObjectComponent(std::move(ref))
{
}

CTileMapComponent::~CTileMapComponent()
{
}

ETileType CTileMapComponent::GetTileType(int Index)
{
	return mTileList[Index]->GetType();
}

int CTileMapComponent::GetTileIndex(const FVector2& Pos) const
{
	auto	Owner = mOwner.lock();

	FVector3	OwnerPos = Owner->GetWorldPos();
	FVector2	ConvertPos;

	ConvertPos.x = Pos.x - OwnerPos.x;
	ConvertPos.y = Pos.y - OwnerPos.y;

	int	IndexX = GetTileIndexX(ConvertPos);

	if (IndexX == -1)
		return -1;

	int	IndexY = GetTileIndexY(ConvertPos);

	if (IndexY == -1)
		return -1;

	return IndexY * mCountX + IndexX;
}

int CTileMapComponent::GetTileIndex(const FVector3& Pos) const
{
	return GetTileIndex(FVector2(Pos.x, Pos.y));
}

int CTileMapComponent::GetTileIndex(float x, float y) const
{
	return GetTileIndex(FVector2(x, y));
}

int CTileMapComponent::GetTileIndexX(const FVector2& Pos) const
{
	switch (mShape)
	{
	case Rect:
	{
		// 타일맵 시작점으로부터 상대적인 위치를 구한다.
		float Convert = Pos.x - mOwner.lock()->GetWorldPos().x;

		float Value = Convert / mTileSize.x;

		int Index = (int)Value;

		if (Index < 0 || Index >= mCountX)
			return -1;

		return Index;
	}
	case Isometric:
	{
		FVector2	ConvertPos;
		ConvertPos.x = Pos.x - mOwner.lock()->GetWorldPos().x;
		ConvertPos.y = Pos.y - mOwner.lock()->GetWorldPos().y;

		// y 인덱스가 홀수인지 짝수인지에 따라 다르다.
		int	IndexY = GetTileIndexY(ConvertPos);

		int	IndexX = -1;

		float	Value = 0.f;

		// 짝수일 경우
		if (IndexY % 2 == 0)
		{
			Value = ConvertPos.x / mTileSize.x;
		}

		// 홀수일 경우
		else
		{
			Value = (ConvertPos.x - mTileSize.x * 0.5f) /
				mTileSize.x;
		}

		if (Value < 0.f)
			return -1;

		IndexX = (int)Value;

		if (IndexX >= mCountX)
			return -1;

		return IndexX;
	}
	}

	return -1;
}

int CTileMapComponent::GetTileIndexY(const FVector2& Pos) const
{
	switch (mShape)
	{
	case Rect:
	{
		// 타일맵 시작점으로부터 상대적인 위치를 구한다.
		float Convert = Pos.y - mOwner.lock()->GetWorldPos().y;

		float Value = Convert / mTileSize.y;

		int Index = (int)Value;

		if (Index < 0 || Index >= mCountY)
			return -1;

		return Index;
	}
	case Isometric:
	{
		FVector2	ConvertPos;
		ConvertPos.x = Pos.x - mOwner.lock()->GetWorldPos().x;
		ConvertPos.y = Pos.y - mOwner.lock()->GetWorldPos().y;

		if (ConvertPos.x < 0.f || ConvertPos.x > mMapSize.x ||
			ConvertPos.y < 0.f || ConvertPos.y > mMapSize.y)
			return -1;

		float RatioX = ConvertPos.x / mTileSize.x;
		float RatioY = ConvertPos.y / mTileSize.y;

		// 인덱스로 변환한다.
		int	IndexX = (int)RatioX;
		int	IndexY = (int)RatioY;

		// 정수부분만 뺀 소수점 부분만 남긴다.
		RatioX -= (int)RatioX;
		RatioY -= (int)RatioY;

		// 사각 영역의 하단에 존재할 경우
		if (RatioY < 0.5f)
		{
			// 사각 영역 기준으로 왼쪽 하단 사각형의
			// 왼쪽 하단 삼각형일 경우
			if (RatioY < 0.5f - RatioX)
			{
				// 가장 아래쪽일 경우
				if (IndexY == 0)
					return -1;

				// 가장 왼쪽일 경우
				else if (IndexX == 0)
				{
					// Y인덱스가 짝수일 경우만 없다.
					// 하지만 여기서는 그려야 하기 때문에 범위를
					// 벗어난 예외만 처리한다.
					if (IndexY < 0)
						return -1;

					else if (IndexX == 0)
						return -1;

					else if (IndexY >= mCountY / 2 + 1)
						return -1;
				}

				return IndexY * 2 - 1;
			}

			// 사각 영역 기준으로 오른쪽 하단 사각형의
			// 오른쪽 하단 삼각형일 경우
			else if (RatioY < RatioX - 0.5f)
			{
				if (IndexY == 0)
					return -1;

				else if (IndexY >= mCountY / 2 + 1)
					return -1;

				return IndexY * 2 - 1;
			}

			else if (IndexY >= mCountY / 2)
				return -1;

			return IndexY * 2;
		}

		// 사각 영역의 상단에 존재할 경우
		else if (RatioY > 0.5f)
		{
			// 사각 영역 기준으로 왼쪽 상단 사각형의
			// 왼쪽 상단 삼각형일 경우
			if (RatioY - 0.5f > RatioX)
			{
				// 왼쪽은 없다.
				if (IndexX == 0)
					return -1;

				else if (IndexY * 2 + 1 >= mCountY)
					return -1;

				else if (IndexY >= mCountY / 2)
					return -1;

				return IndexY * 2 + 1;
			}

			// 사각 영역 기준으로 오른쪽 상단 사각형의
			// 오른쪽 상단 삼각형일 경우
			else if (RatioY - 0.5f > 1.f - RatioX)
			{
				if (IndexX >= mCountX)
					return -1;

				else if (IndexY * 2 + 1 >= mCountY)
					return -1;

				else if (IndexY >= mCountY / 2)
					return -1;

				return IndexY * 2 + 1;
			}

			else if (IndexY >= mCountY / 2)
				return -1;

			return IndexY * 2;
		}

		// 가운데에 존재할 경우
		else
		{
			if (IndexY >= mCountY / 2)
				return -1;

			return IndexY * 2;
		}
	}
	}

	return -1;
}

std::weak_ptr<CTile> CTileMapComponent::GetTile(
	const FVector2& Pos) const
{
	return GetTile(GetTileIndex(Pos));
}

std::weak_ptr<CTile> CTileMapComponent::GetTile(
	const FVector3& Pos) const
{
	return GetTile(GetTileIndex(Pos));
}

std::weak_ptr<CTile> CTileMapComponent::GetTile(
	float x, float y) const
{
	return GetTile(GetTileIndex(x, y));
}

void CTileMapComponent::SetTileFrame(int Index, int Frame)
{
	mTileList[Index]->SetFrame(mTileFrame[Frame].Start,
		mTileFrame[Frame].End);
}

void CTileMapComponent::SetTileOutLineRender(
	bool Render)
{
	size_t	Size = mTileList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		mTileList[i]->SetOutLineRender(Render);
	}

	//mTileOutLineRender = Render;

	//if (mTileOutLineRender)
	//{
	//	// OutLine 출력용 Shader와 Mesh를 얻어온다.
	//}

	//else
	//{
	//	mOutLineMesh.reset();
	//	mOutLineShader.reset();
	//}
}

void CTileMapComponent::SetTileOutLineRender(
	bool Render, int Index)
{
	mTileList[Index]->SetOutLineRender(Render);
}

void CTileMapComponent::SetOutLineMesh(const std::string& Name)
{
	auto	World = mWorld.lock();

	if (World)
	{
		auto	AssetMgr = World->GetWorldAssetManager().lock();
		mOutLineMesh = AssetMgr->FindMesh(Name);
	}

	else
	{
		auto	MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();
		std::string	MeshName = "Mesh_" + Name;
		mOutLineMesh = MeshMgr->FindMesh(MeshName);
	}
}

void CTileMapComponent::SetOutLineShader(
	const std::string& Name)
{
	auto	ShaderMgr = CAssetManager::GetInst()->GetShaderManager().lock();

	mOutLineShader = ShaderMgr->FindShader(Name);
}

void CTileMapComponent::SetTileMesh(const std::string& Name)
{
	auto	World = mWorld.lock();

	if (World)
	{
		auto	AssetMgr = World->GetWorldAssetManager().lock();

		mTileMesh = AssetMgr->FindMesh(Name);
	}

	else
	{
		auto	MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();

		std::string	MeshName = "Mesh_" + Name;

		mTileMesh = MeshMgr->FindMesh(MeshName);
	}
}

void CTileMapComponent::SetTileShader(const std::string& Name)
{
	auto	ShaderMgr = CAssetManager::GetInst()->GetShaderManager().lock();

	mTileShader = ShaderMgr->FindShader(Name);
}

bool CTileMapComponent::SetTileTexture(
	ETileTextureType::Type Type,
	const std::weak_ptr<CTexture>& Texture)
{
	auto	Owner = mOwner.lock();

	std::weak_ptr<CTileMapRender>	Renderer =
		Owner->FindComponent<CTileMapRender>();

	if (Renderer.expired())
		return false;

	return Renderer.lock()->SetTexture(Type, Texture);
}

bool CTileMapComponent::SetTileTexture(
	ETileTextureType::Type Type,
	const std::string& Name)
{
	auto	Owner = mOwner.lock();

	std::weak_ptr<CTileMapRender>	Renderer =
		Owner->FindComponent<CTileMapRender>();

	if (Renderer.expired())
		return false;

	return Renderer.lock()->SetTexture(Type, Name);
}

bool CTileMapComponent::SetTileTexture(
	ETileTextureType::Type Type,
	const std::string& Name, const TCHAR* FileName,
	const std::string& PathName)
{
	auto	Owner = mOwner.lock();

	std::weak_ptr<CTileMapRender>	Renderer =
		Owner->FindComponent<CTileMapRender>();

	if (Renderer.expired())
		return false;

	return Renderer.lock()->SetTexture(Type, Name, FileName,
		PathName);
}

void CTileMapComponent::AddTileFrame(const FVector2& Start,
	const FVector2& End)
{
	FTileFrame	Frame;
	Frame.Start = Start;
	Frame.End = End;

	mTileFrame.push_back(Frame);
}

void CTileMapComponent::AddTileFrame(float StartX, float StartY,
	float EndX, float EndY)
{
	FTileFrame	Frame;
	Frame.Start = FVector2(StartX, StartY);
	Frame.End = FVector2(EndX, EndY);

	mTileFrame.push_back(Frame);
}

void CTileMapComponent::SetTileFrameAll(int FrameIndex)
{
	auto	iter = mTileList.begin();
	auto	iterEnd = mTileList.end();

	FTileFrame	Frame = mTileFrame[FrameIndex];

	for (; iter != iterEnd; ++iter)
	{
		(*iter)->SetFrame(Frame.Start, Frame.End);
	}
}

bool CTileMapComponent::Init()
{
	CObjectComponent::Init();

	auto	Owner = mOwner.lock();

	std::shared_ptr<CTileMapRender>	Renderer =
		Owner->FindComponent<CTileMapRender>().lock();

	if (Renderer)
	{
		Renderer->SetTileMapComponent(std::dynamic_pointer_cast<CTileMapComponent>(mSelf.lock()));
	}


	mTransform.reset(new CCBufferTransform);

	mTransform->Init();

	mCBufferTileMap.reset(new CCBufferTileMap);

	mCBufferTileMap->Init();

	auto	World = mWorld.lock();

	if (World)
	{
		auto	AssetMgr = World->GetWorldAssetManager().lock();

		mTileMesh = AssetMgr->FindMesh("RectTex");
	}

	else
	{
		auto	MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();

		mTileMesh = MeshMgr->FindMesh("Mesh_RectTex");
	}

	auto	ShaderMgr = CAssetManager::GetInst()->GetShaderManager().lock();

	//mTileShader = ShaderMgr->FindShader("TileMap");
	mTileShader = ShaderMgr->FindShader("TileMapInstancing");
	mOutLineShader = ShaderMgr->FindShader("TileMapLineInstancing");

	return true;
}

bool CTileMapComponent::Init(const char* FileName)
{
	return true;
}

void CTileMapComponent::Update(float DeltaTime)
{
}

void CTileMapComponent::PostUpdate(float DeltaTime)
{
	if (mCountX <= 0 || mCountY <= 0)
		return;

	FVector3	Center;

	auto	World = mWorld.lock();

	if (!World)
	{
		World = CWorldManager::GetInst()->GetWorld().lock();
	}

	Center = World->GetCameraManager().lock()->GetMainCameraWorldPos();

	// 타일맵 시작점으로부터 상대적인 위치를 구한다.
	Center -= mOwner.lock()->GetWorldPos();

	FResolution	RS = CDevice::GetInst()->GetResolution();

	if (mUseViewCulling)
	{
		switch (mShape)
		{
		case Rect:
			mViewStartX = (int)((Center.x - RS.Width * 0.5f) /
				mTileSize.x);
			mViewStartY = (int)((Center.y - RS.Height * 0.5f) /
				mTileSize.y);

			mViewEndX = (int)((Center.x + RS.Width * 0.5f) /
				mTileSize.x);
			mViewEndY = (int)((Center.y + RS.Height * 0.5f) /
				mTileSize.y);
			break;
		case Isometric:
		{
			FVector2	LB, RT;

			LB.x = Center.x - RS.Width * 0.5f;
			LB.y = Center.y - RS.Height * 0.5f;

			RT.x = Center.x + RS.Width * 0.5f;
			RT.y = Center.y + RS.Height * 0.5f;

			mViewStartX = GetTileRenderIndexX(LB);
			mViewStartY = GetTileRenderIndexY(LB);

			mViewEndX = GetTileRenderIndexX(RT);
			mViewEndY = GetTileRenderIndexY(RT);

			mViewStartX -= 2;
			mViewEndX += 2;

			mViewStartY -= 2;
			mViewEndY += 2;
		}
			break;
		}
	}

	else
	{
		mViewStartX = 0;
		mViewStartY = 0;
		mViewEndX = mCountX - 1;
		mViewEndY = mCountY - 1;
	}

	mViewStartX = Clamp<int>(mViewStartX, 0, mCountX - 1);
	mViewEndX = Clamp<int>(mViewEndX, 0, mCountX - 1);

	mViewStartY = Clamp<int>(mViewStartY, 0, mCountY - 1);
	mViewEndY = Clamp<int>(mViewEndY, 0, mCountY - 1);

	int	DataCountX = mViewEndX - mViewStartX + 1;
	int	DataCountY = mViewEndY - mViewStartY + 1;

	mInstancingCount = DataCountX * DataCountY;

	if (mTileIstData.size() < mInstancingCount)
	{
		mTileIstData.clear();
		mTileIstData.resize(mInstancingCount);
	}

	if (mTileLineIstData.size() < mInstancingCount)
	{
		mTileLineIstData.clear();
		mTileLineIstData.resize(mInstancingCount);
	}

	auto	Owner = mOwner.lock();
	auto	CameraMgr = World->GetCameraManager().lock();

	mInstancingCount = 0;
	mLineInstancingCount = 0;

	// 인스턴싱 정보를 만든다.
	for (int i = mViewStartY; i <= mViewEndY; ++i)
	{
		for (int j = mViewStartX; j <= mViewEndX; ++j)
		{
			int	Index = i * mCountX + j;

			if (mTileList[Index]->GetRender())
			{
				FMatrix	ScaleMat, TranslateMat, WorldMat;

				ScaleMat.Scaling(mTileSize);

				FVector2	Pos = mTileList[Index]->GetPos();

				Pos.x += Owner->GetWorldPos().x;
				Pos.y += Owner->GetWorldPos().y;

				TranslateMat.Translation(Pos);

				//FMatrix	RotMat;

				//RotMat.RotationZ(230.f);
				//RotMat.Rotation(FVector3(0.f, 0.f, 238.f));

				//WorldMat = ScaleMat * RotMat * TranslateMat;
				WorldMat = ScaleMat * TranslateMat;

				FMatrix	ViewMat = CameraMgr->GetViewMatrix();
				FMatrix	ProjMat = CameraMgr->GetProjMatrix();

				FMatrix	WVPMat = WorldMat * ViewMat * ProjMat;

				mTileIstData[mInstancingCount].WVP0 = WVPMat[0];
				mTileIstData[mInstancingCount].WVP1 = WVPMat[1];
				mTileIstData[mInstancingCount].WVP2 = WVPMat[2];
				mTileIstData[mInstancingCount].WVP3 = WVPMat[3];

				mTileIstData[mInstancingCount].LTUV =
					mTileList[Index]->GetFrameStart() / mTileTextureSize;
				mTileIstData[mInstancingCount].RBUV =
					mTileList[Index]->GetFrameEnd() / mTileTextureSize;

				mTileIstData[mInstancingCount].Color =
					mTileList[Index]->GetOutLineColor();

				++mInstancingCount;
			}

			if (mTileList[Index]->GetOutLineRender())
			{
				FMatrix	ScaleMat, TranslateMat, WorldMat;

				ScaleMat.Scaling(mTileSize);

				FVector2	Pos = mTileList[Index]->GetPos();

				Pos.x += Owner->GetWorldPos().x;
				Pos.y += Owner->GetWorldPos().y;

				TranslateMat.Translation(Pos);

				//FMatrix	RotMat;

				//RotMat.RotationZ(230.f);
				//RotMat.Rotation(FVector3(0.f, 0.f, 238.f));

				//WorldMat = ScaleMat * RotMat * TranslateMat;
				WorldMat = ScaleMat * TranslateMat;

				FMatrix	ViewMat = CameraMgr->GetViewMatrix();
				FMatrix	ProjMat = CameraMgr->GetProjMatrix();

				FMatrix	WVPMat = WorldMat * ViewMat * ProjMat;

				mTileLineIstData[mLineInstancingCount].WVP0 = WVPMat[0];
				mTileLineIstData[mLineInstancingCount].WVP1 = WVPMat[1];
				mTileLineIstData[mLineInstancingCount].WVP2 = WVPMat[2];
				mTileLineIstData[mLineInstancingCount].WVP3 = WVPMat[3];

				mTileLineIstData[mLineInstancingCount].Color = mTileList[Index]->GetOutLineColor();

				++mLineInstancingCount;
			}
		}
	}

	SetInstancingData(&mTileIstData[0],
		mInstancingCount);
	SetLineInstancingData(&mTileLineIstData[0],
		mLineInstancingCount);
}

void CTileMapComponent::PostRender()
{
	CObjectComponent::PostRender();
}

void CTileMapComponent::Destroy()
{
	CObjectComponent::Destroy();
}

CTileMapComponent* CTileMapComponent::Clone()	const
{
	return new CTileMapComponent(*this);
}

void CTileMapComponent::RenderTile()
{
	auto	Shader = mTileShader.lock();
	auto	Mesh = mTileMesh.lock();

	Shader->SetShader();

	Mesh->RenderInstancing(mInstancingBuffer,
		mInstancingCount);

	/*
	auto	Owner = mOwner.lock();
	auto	World = mWorld.lock();

	if (!World)
		World = CWorldManager::GetInst()->GetWorld().lock();

	auto	CameraMgr = World->GetCameraManager().lock();
	
	for (int i = mViewStartY; i <= mViewEndY; ++i)
	{
		for (int j = mViewStartX; j <= mViewEndX; ++j)
		{
			int	Index = i * mCountX + j;

			if (!mTileList[Index]->GetRender())
				continue;

			FMatrix	ScaleMat, TranslateMat, WorldMat;

			ScaleMat.Scaling(mTileSize);

			FVector2	Pos = mTileList[Index]->GetPos();

			Pos.x += Owner->GetWorldPos().x;
			Pos.y += Owner->GetWorldPos().y;

			TranslateMat.Translation(Pos);

			WorldMat = ScaleMat * TranslateMat;

			mTransform->SetWorldMatrix(WorldMat);

			mTransform->SetViewMatrix(CameraMgr->GetViewMatrix());
			mTransform->SetProjMatrix(CameraMgr->GetProjMatrix());

			mTransform->UpdateBuffer();

			FVector2	LTUV = mTileList[Index]->GetFrameStart() / mTileTextureSize;
			FVector2	RBUV = mTileList[Index]->GetFrameEnd() / mTileTextureSize;

			mCBufferTileMap->SetUV(LTUV, RBUV);

			mCBufferTileMap->UpdateBuffer();

			Shader->SetShader();

			Mesh->Render();
		}
	}*/
}

void CTileMapComponent::RenderTileOutLine()
{
	auto	Shader = mOutLineShader.lock();
	auto	Mesh = mOutLineMesh.lock();

	Shader->SetShader();

	Mesh->RenderInstancing(mLineInstancingBuffer,
		mLineInstancingCount);
}

void CTileMapComponent::CreateTile(ETileShape Shape,
	int CountX, int CountY, const FVector2& TileSize,
	int TileTextureFrame,
	bool OutLineRender)
{
	mShape = Shape;
	mCountX = CountX;
	mCountY = CountY;
	mTileSize = TileSize;

	auto	World = mWorld.lock();

	switch (mShape)
	{
	case Rect:
		mMapSize = mTileSize *
			FVector2((float)mCountX, (float)mCountY);

		if (World)
		{
			auto	AssetMgr = World->GetWorldAssetManager().lock();

			mOutLineMesh = AssetMgr->FindMesh("LBFrameRect");
		}

		else
		{
			auto	MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();

			mOutLineMesh = MeshMgr->FindMesh("Mesh_LBFrameRect");
		}
		break;
	case Isometric:
		mMapSize.x = mTileSize.x * mCountX +
			mTileSize.x * 0.5f;
		mMapSize.y = mTileSize.y +
			mTileSize.y * 0.5f * (mCountY - 1);

		if (World)
		{
			auto	AssetMgr = World->GetWorldAssetManager().lock();

			mOutLineMesh = AssetMgr->FindMesh("IstFrameRect");
		}

		else
		{
			auto	MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();

			mOutLineMesh = MeshMgr->FindMesh("Mesh_IstFrameRect");
		}
		break;
	}

	auto	Owner = mOwner.lock();

	std::shared_ptr<CTileMapRender>	Renderer =
		Owner->FindComponent<CTileMapRender>().lock();

	if (Renderer)
	{
		Renderer->SetWorldScale(mMapSize);
	}

	mTileList.clear();

	mTileList.resize(mCountX * mCountY);

	for (int i = 0; i < mCountY; ++i)
	{
		for (int j = 0; j < mCountX; ++j)
		{
			int	Index = i * mCountX + j;

			mTileList[Index].reset(new CTile);

			mTileList[Index]->SetIndex(j, i, Index);

			switch (mShape)
			{
			case Rect:
				mTileList[Index]->SetPos(j * mTileSize.x,
					i * mTileSize.y);
				break;
			case Isometric:
				if (i % 2 == 0)
				{
					mTileList[Index]->SetPos(j * mTileSize.x,
						i * mTileSize.y * 0.5f);
				}

				else
				{
					mTileList[Index]->SetPos(
						j * mTileSize.x + mTileSize.x * 0.5f,
						i * mTileSize.y * 0.5f);
				}
				break;
			}

			mTileList[Index]->SetSize(mTileSize);
			mTileList[Index]->SetCenter(mTileList[Index]->GetPos() + mTileSize * 0.5f);
			mTileList[Index]->SetTextureFrame(TileTextureFrame);
			mTileList[Index]->SetOutLineRender(OutLineRender);
		}
	}

	// 화면에 보일 수 있는 최대 타일 개수를 정한다.
	FResolution	RS = CDevice::GetInst()->GetResolution();

	int	ViewCountX = (int)(RS.Width / mTileSize.x + 5);
	int	ViewCountY = (int)(RS.Height / mTileSize.y + 3);

	if (mShape == Isometric)
		ViewCountY = (ViewCountY * 2) + 2;

	CreateInstancingBuffer(
		sizeof(FTileMapInstancingBuffer),
		ViewCountX * ViewCountY);

	CreateLineInstancingBuffer(
		sizeof(FTileMapInstancingBuffer),
		ViewCountX * ViewCountY);

	mTileIstData.resize(ViewCountX * ViewCountY);
	mTileLineIstData.resize(ViewCountX * ViewCountY);

	// 내비게이션 스레드를 생성한다.
	auto	Nav = World->GetNavigation().lock();

	SYSTEM_INFO	SysInfo = {};

	GetSystemInfo(&SysInfo);

	Nav->CreateNavigationThread((int)SysInfo.dwNumberOfProcessors,
		std::dynamic_pointer_cast<CTileMapComponent>(mSelf.lock()));
}

bool CTileMapComponent::CreateInstancingBuffer(int Size,
	int Count)
{
	SAFE_RELEASE(mInstancingBuffer.Buffer);

	mInstancingBuffer.Size = Size;
	mInstancingBuffer.Count = Count;

	// 버퍼를 생성하기 위한 구조체
	D3D11_BUFFER_DESC	BufferDesc = {};

	// 버퍼의 전체 메모리 크기
	BufferDesc.ByteWidth = Size * Count;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;


	if (FAILED(CDevice::GetInst()->GetDevice()->CreateBuffer(&BufferDesc, nullptr, &mInstancingBuffer.Buffer)))
		return false;

	return true;
}

bool CTileMapComponent::SetInstancingData(void* Data,
	int Count)
{
	if (!mInstancingBuffer.Buffer)
		return false;

	if (mInstancingBuffer.Count < Count)
	{
		if (!CreateInstancingBuffer(mInstancingBuffer.Size,
			Count * 2))
			return false;
	}

	ID3D11DeviceContext* Context =
		CDevice::GetInst()->GetContext();

	D3D11_MAPPED_SUBRESOURCE	MS = {};

	Context->Map(mInstancingBuffer.Buffer, 0,
		D3D11_MAP_WRITE_DISCARD, 0, &MS);

	memcpy(MS.pData, Data, mInstancingBuffer.Size * Count);

	Context->Unmap(mInstancingBuffer.Buffer, 0);

	mInstancingCount = Count;

	return true;
}

bool CTileMapComponent::CreateLineInstancingBuffer(int Size,
	int Count)
{
	SAFE_RELEASE(mLineInstancingBuffer.Buffer);

	mLineInstancingBuffer.Size = Size;
	mLineInstancingBuffer.Count = Count;

	// 버퍼를 생성하기 위한 구조체
	D3D11_BUFFER_DESC	BufferDesc = {};

	// 버퍼의 전체 메모리 크기
	BufferDesc.ByteWidth = Size * Count;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;


	if (FAILED(CDevice::GetInst()->GetDevice()->CreateBuffer(&BufferDesc, nullptr, &mLineInstancingBuffer.Buffer)))
		return false;

	return true;
}

bool CTileMapComponent::SetLineInstancingData(void* Data,
	int Count)
{
	if (!mLineInstancingBuffer.Buffer)
		return false;

	if (mLineInstancingBuffer.Count < Count)
	{
		if (!CreateLineInstancingBuffer(mLineInstancingBuffer.Size,
			Count * 2))
			return false;
	}

	ID3D11DeviceContext* Context =
		CDevice::GetInst()->GetContext();

	D3D11_MAPPED_SUBRESOURCE	MS = {};

	Context->Map(mLineInstancingBuffer.Buffer, 0,
		D3D11_MAP_WRITE_DISCARD, 0, &MS);

	memcpy(MS.pData, Data, mLineInstancingBuffer.Size * Count);

	Context->Unmap(mLineInstancingBuffer.Buffer, 0);

	mLineInstancingCount = Count;

	return true;
}

void CTileMapComponent::Save(const TCHAR* FileName,
	const std::string& PathName)
{
	TCHAR	FullPath[MAX_PATH] = {};

	lstrcpy(FullPath, CPathManager::FindPath(PathName));
	lstrcat(FullPath, FileName);

	SaveFullPath(FullPath);
}

void CTileMapComponent::SaveFullPath(const TCHAR* FullPath)
{
	char	FullPathMultibyte[MAX_PATH] = {};

	int Length = WideCharToMultiByte(CP_ACP, 0, FullPath,
		-1, nullptr, 0, nullptr, nullptr);
	WideCharToMultiByte(CP_ACP, 0, FullPath, -1, 
		FullPathMultibyte, Length, nullptr, nullptr);

	FILE* File = nullptr;

	fopen_s(&File, FullPathMultibyte, "wb");

	if (!File)
		return;

	// 타일맵 렌더 정보를 저장한다.
	auto	Render = mTileMapRender.lock();

	Render->Save(File);

	// 타일맵 정보를 저장한다.
	size_t	Size = mTileList.size();

	fwrite(&Size, sizeof(size_t), 1, File);

	for (size_t i = 0; i < Size; ++i)
	{
		mTileList[i]->Save(File);
	}

	fwrite(&mShape, sizeof(ETileShape), 1, File);
	fwrite(&mTileSize, sizeof(FVector2), 1, File);
	fwrite(&mMapSize, sizeof(FVector2), 1, File);

	fwrite(&mCountX, sizeof(int), 1, File);
	fwrite(&mCountY, sizeof(int), 1, File);

	fwrite(&mTileOutLineRender, sizeof(bool), 1, File);

	auto	OutLineMesh = mOutLineMesh.lock();

	bool Enable = false;

	if (OutLineMesh)
		Enable = true;

	fwrite(&Enable, sizeof(bool), 1, File);

	if (OutLineMesh)
	{
		std::string	Name = OutLineMesh->GetSplitName();

		size_t	Count = Name.length();

		fwrite(&Count, sizeof(size_t), 1, File);

		fwrite(Name.c_str(), 1, Name.length(), File);
	}

	auto	OutLineShader = mOutLineShader.lock();

	Enable = false;

	if (OutLineShader)
		Enable = true;

	fwrite(&Enable, sizeof(bool), 1, File);

	if (OutLineShader)
	{
		std::string	Name = OutLineShader->GetName();

		size_t	Count = Name.length();

		fwrite(&Count, sizeof(size_t), 1, File);

		fwrite(Name.c_str(), 1, Name.length(), File);
	}

	auto	TileMesh = mTileMesh.lock();

	Enable = false;

	if (TileMesh)
		Enable = true;

	fwrite(&Enable, sizeof(bool), 1, File);

	if (TileMesh)
	{
		std::string	Name = TileMesh->GetSplitName();

		size_t	Count = Name.length();

		fwrite(&Count, sizeof(size_t), 1, File);

		fwrite(Name.c_str(), 1, Name.length(), File);
	}

	auto	TileShader = mTileShader.lock();

	Enable = false;

	if (TileShader)
		Enable = true;

	fwrite(&Enable, sizeof(bool), 1, File);

	if (TileShader)
	{
		std::string	Name = TileShader->GetName();

		size_t	Count = Name.length();

		fwrite(&Count, sizeof(size_t), 1, File);

		fwrite(Name.c_str(), 1, Name.length(), File);
	}

	fwrite(&mTileTextureSize, sizeof(FVector2), 1, File);

	Size = mTileFrame.size();

	fwrite(&Size, sizeof(size_t), 1, File);

	fwrite(&mTileFrame[0], sizeof(FTileFrame), Size, File);

	fclose(File);
}

void CTileMapComponent::Load(const TCHAR* FileName,
	const std::string& PathName)
{
	TCHAR	FullPath[MAX_PATH] = {};

	lstrcpy(FullPath, CPathManager::FindPath(PathName));
	lstrcat(FullPath, FileName);

	LoadFullPath(FullPath);
}

void CTileMapComponent::LoadFullPath(const TCHAR* FullPath)
{
	char	FullPathMultibyte[MAX_PATH] = {};

	int Length = WideCharToMultiByte(CP_ACP, 0, FullPath,
		-1, nullptr, 0, nullptr, nullptr);
	WideCharToMultiByte(CP_ACP, 0, FullPath, -1,
		FullPathMultibyte, Length, nullptr, nullptr);

	FILE* File = nullptr;

	fopen_s(&File, FullPathMultibyte, "rb");

	if (!File)
		return;

	// 타일맵 렌더 정보를 읽어온다.
	auto	Render = mTileMapRender.lock();

	Render->Load(File);

	// 타일맵 정보를 읽어온다.
	size_t	Size = 0;

	fread(&Size, sizeof(size_t), 1, File);

	mTileList.clear();
	mTileList.resize(Size);

	for (size_t i = 0; i < Size; ++i)
	{
		mTileList[i].reset(new CTile);

		mTileList[i]->Load(File);
	}

	FResolution	RS = CDevice::GetInst()->GetResolution();

	int	ViewCountX = (int)(RS.Width / mTileSize.x + 3);
	int	ViewCountY = (int)(RS.Height / mTileSize.y + 3);

	if (mShape == Isometric)
		ViewCountY = (ViewCountY * 2) + 2;

	CreateInstancingBuffer(
		sizeof(FTileMapInstancingBuffer),
		ViewCountX * ViewCountY);

	CreateLineInstancingBuffer(
		sizeof(FTileMapInstancingBuffer),
		ViewCountX * ViewCountY);

	mTileIstData.clear();
	mTileLineIstData.clear();

	mTileIstData.resize(ViewCountX * ViewCountY);
	mTileLineIstData.resize(ViewCountX * ViewCountY);

	fread(&mShape, sizeof(ETileShape), 1, File);
	fread(&mTileSize, sizeof(FVector2), 1, File);
	fread(&mMapSize, sizeof(FVector2), 1, File);

	fread(&mCountX, sizeof(int), 1, File);
	fread(&mCountY, sizeof(int), 1, File);

	fread(&mTileOutLineRender, sizeof(bool), 1, File);

	bool	Enable = false;

	fread(&Enable, sizeof(bool), 1, File);

	if (Enable)
	{
		char	Name[256] = {};

		size_t	Count = 0;

		fread(&Count, sizeof(size_t), 1, File);

		fread(Name, 1, Count, File);

		SetOutLineMesh(Name);
	}

	Enable = false;

	fread(&Enable, sizeof(bool), 1, File);

	if (Enable)
	{
		char	Name[256] = {};

		size_t	Count = 0;

		fread(&Count, sizeof(size_t), 1, File);

		fread(Name, 1, Count, File);

		SetOutLineShader(Name);
	}

	Enable = false;

	fread(&Enable, sizeof(bool), 1, File);

	if (Enable)
	{
		char	Name[256] = {};

		size_t	Count = 0;

		fread(&Count, sizeof(size_t), 1, File);

		fread(Name, 1, Count, File);

		SetTileMesh(Name);
	}

	Enable = false;

	fread(&Enable, sizeof(bool), 1, File);

	if (Enable)
	{
		char	Name[256] = {};

		size_t	Count = 0;

		fread(&Count, sizeof(size_t), 1, File);

		fread(Name, 1, Count, File);

		SetTileShader(Name);
	}

	fread(&mTileTextureSize, sizeof(FVector2), 1, File);

	Size = 0;

	fread(&Size, sizeof(size_t), 1, File);

	mTileFrame.clear();
	mTileFrame.resize(Size);

	fread(&mTileFrame[0], sizeof(FTileFrame), Size, File);

	fclose(File);

	auto	World = mWorld.lock();

	// 내비게이션 스레드를 생성한다.
	auto	Nav = World->GetNavigation().lock();

	SYSTEM_INFO	SysInfo = {};

	GetSystemInfo(&SysInfo);

	Nav->CreateNavigationThread((int)SysInfo.dwNumberOfProcessors,
		std::dynamic_pointer_cast<CTileMapComponent>(mSelf.lock()));
}

int CTileMapComponent::GetTileRenderIndexX(
	const FVector2& Pos) const
{
	switch (mShape)
	{
	case Rect:
	{
		// 타일맵 시작점으로부터 상대적인 위치를 구한다.
		float Convert = Pos.x - mOwner.lock()->GetWorldPos().x;

		float Value = Convert / mTileSize.x;

		int Index = (int)Value;

		if (Index < 0)
			return 0;

		else if (Index >= mCountX)
			return mCountX - 1;

		return Index;
	}
	case Isometric:
	{
		FVector2	ConvertPos;
		ConvertPos.x = Pos.x - mOwner.lock()->GetWorldPos().x;
		ConvertPos.y = Pos.y - mOwner.lock()->GetWorldPos().y;

		// y 인덱스가 홀수인지 짝수인지에 따라 다르다.
		int	IndexY = GetTileRenderIndexY(ConvertPos);

		int	IndexX = -1;

		float	Value = 0.f;

		// 짝수일 경우
		if (IndexY % 2 == 0)
		{
			Value = ConvertPos.x / mTileSize.x;
		}

		// 홀수일 경우
		else
		{
			Value = (ConvertPos.x - mTileSize.x * 0.5f) /
				mTileSize.x;
		}

		if (Value < 0.f)
			return 0;

		IndexX = (int)Value;

		if (IndexX >= mCountX)
			return mCountX - 1;

		return IndexX;
	}
	}

	return -1;
}

int CTileMapComponent::GetTileRenderIndexY(
	const FVector2& Pos) const
{
	switch (mShape)
	{
	case Rect:
	{
		// 타일맵 시작점으로부터 상대적인 위치를 구한다.
		float Convert = Pos.y - mOwner.lock()->GetWorldPos().y;

		float Value = Convert / mTileSize.y;

		int Index = (int)Value;

		if (Index < 0)
			return 0;

		else if (Index >= mCountY)
			return mCountY - 1;

		return Index;
	}
	case Isometric:
	{
		FVector2	ConvertPos;
		ConvertPos.x = Pos.x - mOwner.lock()->GetWorldPos().x;
		ConvertPos.y = Pos.y - mOwner.lock()->GetWorldPos().y;

		float RatioX = ConvertPos.x / mTileSize.x;
		float RatioY = ConvertPos.y / mTileSize.y;

		// 인덱스로 변환한다.
		int	IndexX = (int)RatioX;
		int	IndexY = (int)RatioY;

		if (IndexX < 0)
			IndexX = 0;

		else if (IndexX >= mCountX)
			IndexX = mCountX - 1;

		if (IndexY < 0)
			IndexY = 0;

		else if (IndexY >= mCountY)
			IndexY = mCountY - 1;

		// 정수부분만 뺀 소수점 부분만 남긴다.
		RatioX -= (int)RatioX;
		RatioY -= (int)RatioY;

		// 사각 영역의 하단에 존재할 경우
		if (RatioY < 0.5f)
		{
			// 사각 영역 기준으로 왼쪽 하단 사각형의
			// 왼쪽 하단 삼각형일 경우
			if (RatioY < 0.5f - RatioX)
			{
				// 가장 아래쪽일 경우
				if (IndexY == 0)
					return 0;

				// 가장 왼쪽일 경우
				else if (IndexX == 0)
				{
					// Y인덱스가 짝수일 경우만 없다.
					// 하지만 여기서는 그려야 하기 때문에 범위를
					// 벗어난 예외만 처리한다.
					if (IndexY < 0)
						return 0;

					else if (IndexY >= mCountY)
						return mCountY - 1;
				}

				return IndexY * 2 - 1;
			}

			// 사각 영역 기준으로 오른쪽 하단 사각형의
			// 오른쪽 하단 삼각형일 경우
			else if (RatioY < RatioX - 0.5f)
			{
				if (IndexY == 0)
					return 0;

				return IndexY * 2 - 1;
			}

			return IndexY * 2;
		}

		// 사각 영역의 상단에 존재할 경우
		else if (RatioY > 0.5f)
		{
			// 사각 영역 기준으로 왼쪽 상단 사각형의
			// 왼쪽 상단 삼각형일 경우
			if (RatioY - 0.5f > RatioX)
			{
				if (IndexX == 0)
				{
					if (IndexY < 0)
						return 0;

					else if (IndexY >= mCountY)
						return mCountY - 1;
				}

				else if (IndexY * 2 + 1 >= mCountY)
					return mCountY - 1;

				return IndexY * 2 + 1;
			}

			// 사각 영역 기준으로 오른쪽 상단 사각형의
			// 오른쪽 상단 삼각형일 경우
			else if (RatioY - 0.5f > 1.f - RatioX)
			{
				if (IndexX >= mCountX)
				{
					if (IndexY < 0)
						return 0;

					else if (IndexY >= mCountY)
						return mCountY - 1;
				}

				else if (IndexY * 2 + 1 >= mCountY)
					return mCountY - 1;

				return IndexY * 2 + 1;
			}

			return IndexY * 2;
		}

		// 가운데에 존재할 경우
		else
		{
			return IndexY * 2;
		}
	}
	}

	return -1;
}
