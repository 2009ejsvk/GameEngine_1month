#include "TileMapComponent.h"
#include "../World/World.h"
#include "../World/WorldNavigation.h"
#include "../World/WorldAssetManager.h"
#include "../World/CameraManager.h"
#include "../World/WorldManager.h"
#include "CameraComponent.h"
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

/*
 * [GetTileIndex]
 * 특정 월드 좌표(Pos)가 어떤 타일 위에 있는지 '인덱스(순서)'를 찾아주는 함수입니다.
 * 예: 10x10 맵에서 (0,0) 위치라면 0번 인덱스를 반환합니다.
 */
int CTileMapComponent::GetTileIndex(const FVector2& Pos) const
{
	// 이 컴포넌트를 소유하고 있는 게임 오브젝트(TileMapObject 등)를 가져옵니다.
	auto	Owner = mOwner.lock();

	// 타일맵 자체가 월드상에서 어디에 있는지(기준점) 위치를 가져옵니다.
	FVector3	OwnerPos = Owner->GetWorldPos();
	FVector2	ConvertPos;

	// 입력받은 위치(Pos)에서 타일맵의 기준점(OwnerPos)을 빼서,
	// 타일맵 내부에서의 상대적인 위치(내부 좌표)로 변환합니다.
	ConvertPos.x = Pos.x - OwnerPos.x;
	ConvertPos.y = Pos.y - OwnerPos.y;

	// X축 방향으로 몇 번째 타일 칸에 있는지 찾습니다.
	int	IndexX = GetTileIndexX(ConvertPos);

	// 만약 맵 범위를 벗어났다면 -1(없음)을 반환합니다.
	if (IndexX == -1)
		return -1;

	// Y축 방향으로 몇 번째 타일 칸에 있는지 찾습니다.
	int	IndexY = GetTileIndexY(ConvertPos);

	if (IndexY == -1)
		return -1;

	// 2차원 좌표(X, Y)를 1차원 배열 인덱스로 변환하여 반환합니다.
	// 공식: (Y번호 * 한 줄당 타일 개수) + X번호
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

/*
 * [GetTileIndexX]
 * 상대 좌표(Pos)를 받아서 가로(X) 방향으로 몇 번째 타일인지 계산합니다.
 */
int CTileMapComponent::GetTileIndexX(const FVector2& Pos) const
{
	switch (mShape)
	{
	case Rect: // 일반 사각형 타일맵인 경우
	{
		// 마우스 위치에서 타일맵 시작점을 뺀 거리를 타일 한 칸의 너비로 나눕니다.
		float Convert = Pos.x - mOwner.lock()->GetWorldPos().x;
		float Value = Convert / mTileSize.x;

		int Index = (int)Value; // 소수점을 버리면 그게 바로 타일의 번호가 됩니다.

		// 맵의 왼쪽이나 오른쪽을 넘어갔는지 체크합니다.
		if (Index < 0 || Index >= mCountX)
			return -1;

		return Index;
	}
	case Isometric: // 마름모 모양(아이소매트릭) 타일맵인 경우
	{
		FVector2	ConvertPos;
		ConvertPos.x = Pos.x - mOwner.lock()->GetWorldPos().x;
		ConvertPos.y = Pos.y - mOwner.lock()->GetWorldPos().y;

		// 아이소매트릭은 행(Y)이 홀수냐 짝수냐에 따라 X 위치가 반 칸씩 어긋나 있습니다 (Zig-Zag).
		// 그래서 먼저 Y 인덱스를 알아내야 정확한 X 인덱스를 구할 수 있습니다.
		int	IndexY = GetTileIndexY(ConvertPos);
		int	IndexX = -1;
		float	Value = 0.f;

		// 짝수 줄(0, 2, 4...)은 정직하게 너비로 나누면 됩니다.
		if (IndexY % 2 == 0)
		{
			Value = ConvertPos.x / mTileSize.x;
		}
		// 홀수 줄(1, 3, 5...)은 타일들이 오른쪽으로 반 칸(mTileSize.x * 0.5f) 밀려 있습니다.
		// 그래서 반 칸을 뒤로 당겨서 계산해야 합니다.
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

/*
 * [GetTileIndexY]
 * 세로(Y) 방향으로 몇 번째 타일인지 계산합니다. 
 * 아이소매트릭 방식에서는 마름모 경계선 판정이 들어가는 가장 복잡한 부분입니다.
 */
int CTileMapComponent::GetTileIndexY(const FVector2& Pos) const
{
	switch (mShape)
	{
	case Rect: // 사각형은 간단하게 높이로 나눕니다.
	{
		float Convert = Pos.y - mOwner.lock()->GetWorldPos().y;
		float Value = Convert / mTileSize.y;
		int Index = (int)Value;

		if (Index < 0 || Index >= mCountY)
			return -1;

		return Index;
	}
	case Isometric: // 마름모 판정 로직
	{
		FVector2	ConvertPos;
		ConvertPos.x = Pos.x - mOwner.lock()->GetWorldPos().x;
		ConvertPos.y = Pos.y - mOwner.lock()->GetWorldPos().y;

		// 맵 전체 영역을 벗어났는지 먼저 확인합니다.
		if (ConvertPos.x < 0.f || ConvertPos.x > mMapSize.x ||
			ConvertPos.y < 0.f || ConvertPos.y > mMapSize.y)
			return -1;

		// 타일의 가로/세로 비율(Ratio)을 구합니다. (0.0 ~ 1.0 사이 값)
		float RatioX = ConvertPos.x / mTileSize.x;
		float RatioY = ConvertPos.y / mTileSize.y;

		// 일단 정수 인덱스를 구합니다.
		int	IndexX = (int)RatioX;
		int	IndexY = (int)RatioY;

		// 정수 부분을 빼면 타일 한 칸 안에서의 0.0 ~ 1.0 사이 위치가 남습니다.
		RatioX -= (int)RatioX;
		RatioY -= (int)RatioY;

		// [중요] 아이소매트릭 타일은 사각형 안에 마름모가 들어있는 형태입니다.
		// 사각형의 네 귀퉁이(삼각형 부분)를 클릭했을 때 인접한 다른 타일로 인덱스를 보정해주어야 합니다.
		
		// 사격 영역의 하단(절반 아래)에 클릭이 발생했을 때
		if (RatioY < 0.5f)
		{
			// 왼쪽 하단 모서리 삼각형 영역인 경우
			if (RatioY < 0.5f - RatioX)
			{
				if (IndexY == 0) return -1;
				return IndexY * 2 - 1; // 대각선 위의 타일로 보정
			}

			// 오른쪽 하단 모서리 삼각형 영역인 경우
			else if (RatioY < RatioX - 0.5f)
			{
				if (IndexY == 0) return -1;
				return IndexY * 2 - 1;
			}

			// 삼각형 영역이 아니라면 현재 행의 짝수 인덱스
			return IndexY * 2;
		}

		// 사각 영역의 상단(절반 위)에 클릭이 발생했을 때
		else if (RatioY > 0.5f)
		{
			// 왼쪽 상단 모서리 삼각형 영역
			if (RatioY - 0.5f > RatioX)
			{
				if (IndexX == 0) return -1;
				return IndexY * 2 + 1;
			}

			// 오른쪽 상단 모서리 삼각형 영역
			else if (RatioY - 0.5f > 1.f - RatioX)
			{
				if (IndexX >= mCountX) return -1;
				return IndexY * 2 + 1;
			}

			return IndexY * 2;
		}

		// 정확히 가운데 가로선상에 있을 때
		else
		{
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

/*
 * [Init] - 컴포넌트 초기화
 * 타일맵이 처음 생성될 때 필요한 셰이더(Shader), 메쉬(Mesh), 상수 버퍼 등을 준비합니다.
 */
bool CTileMapComponent::Init()
{
	CObjectComponent::Init();

	auto	Owner = mOwner.lock();

	// 렌더러(Renderer) 컴포넌트를 찾아서 서로 연결해줍니다.
	// 그래야 타일맵 데이터가 바뀔 때 화면에 바로 그려질 수 있습니다.
	std::shared_ptr<CTileMapRender>	Renderer =
		Owner->FindComponent<CTileMapRender>().lock();

	if (Renderer)
	{
		Renderer->SetTileMapComponent(std::dynamic_pointer_cast<CTileMapComponent>(mSelf.lock()));
	}

	// [상수 버퍼 준비]
	// CPU에 있는 행렬 데이터를 GPU로 전달하기 위한 통로(Buffer)를 만듭니다.
	mTransform.reset(new CCBufferTransform);
	mTransform->Init();

	mCBufferTileMap.reset(new CCBufferTileMap);
	mCBufferTileMap->Init();

	auto	World = mWorld.lock();

	// 타일로 사용할 기본 사각형 메쉬(Mesh)를 엔진에서 가져옵니다.
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

	// [셰이더(Shader) 설정]
	// 타일을 화면에 그릴 때 사용할 '그리기 공식(Shader)'을 선택합니다.
	// 낱개로 그리는 방식이 아닌, 인스턴싱(Instancing) 방식용 셰이더를 사용합니다.
	auto	ShaderMgr = CAssetManager::GetInst()->GetShaderManager().lock();

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

/*
 * [PostUpdate] - 업데이트 단계의 마지막 처리
 * 1. 현재 카메라가 어디를 보고 있는지 확인하여 화면에 보이는 타일만 렌더링하도록 거르는 '뷰 컬링'을 수행합니다.
 * 2. 수천 개의 타일을 각각 그리면 느리므로, 한 번에 묶어서 그리기 위한 '인스턴싱 데이터'를 만듭니다.
 */
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

	// 현재 메인 카메라의 월드 위치를 가져옵니다.
	Center = World->GetCameraManager().lock()->GetMainCameraWorldPos();

	// 타일맵 기준점에서 카메라까지의 거리를 구합니다.
	Center -= mOwner.lock()->GetWorldPos();

	// 현재 화면(창)의 해상도 가로/세로 크기를 가져옵니다.
	FResolution	RS = CDevice::GetInst()->GetResolution();
	float ViewWidth = (float)RS.Width;
	float ViewHeight = (float)RS.Height;
	auto CameraMgr = World->GetCameraManager().lock();

	// 카메라의 줌 정도에 따라 실제로 보이는 영역의 크기를 보정합니다.
	if (CameraMgr)
	{
		auto MainCamera = CameraMgr->GetMainCamera().lock();

		if (MainCamera &&
			MainCamera->GetProjectionType() ==
			ECameraProjectionType::Ortho)
		{
			ViewWidth = MainCamera->GetViewWidth();
			ViewHeight = MainCamera->GetViewHeight();
		}
	}

	// [뷰 컬링(View Culling)] : 화면 밖 타일은 계산에서 제외
	if (mUseViewCulling)
	{
		switch (mShape)
		{
		case Rect: // 사각형은 단순하게 현재 위치에서 화면 절반 너비를 빼고 더해서 범위를 구합니다.
			mViewStartX = (int)((Center.x - ViewWidth * 0.5f) /
				mTileSize.x);
			mViewStartY = (int)((Center.y - ViewHeight * 0.5f) /
				mTileSize.y);

			mViewEndX = (int)((Center.x + ViewWidth * 0.5f) /
				mTileSize.x);
			mViewEndY = (int)((Center.y + ViewHeight * 0.5f) /
				mTileSize.y);
			break;

		case Isometric: // 아이소매트릭은 대각선 형태라 보정값이 더 들어갑니다.
		{
			FVector2	LB, RT; // 화면의 왼쪽아래(LB), 오른쪽위(RT) 지점

			LB.x = Center.x - ViewWidth * 0.5f;
			LB.y = Center.y - ViewHeight * 0.5f;

			RT.x = Center.x + ViewWidth * 0.5f;
			RT.y = Center.y + ViewHeight * 0.5f;

			// 각 지점이 가리키는 타일 인덱스를 구합니다.
			mViewStartX = GetTileRenderIndexX(LB);
			mViewStartY = GetTileRenderIndexY(LB);

			mViewEndX = GetTileRenderIndexX(RT);
			mViewEndY = GetTileRenderIndexY(RT);

			// 경계 타일이 깜빡거리는 걸 방지하기 위해 상하좌우로 2칸씩 마진을 줍니다.
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
		// 컬링을 안 쓰면 전체 맵을 다 그리도록 설정합니다.
		mViewStartX = 0;
		mViewStartY = 0;
		mViewEndX = mCountX - 1;
		mViewEndY = mCountY - 1;
	}

	// 맵 배열의 유효 범위를 벗어나지 않도록 값을 고정(Clamp)해줍니다.
	mViewStartX = Clamp<int>(mViewStartX, 0, mCountX - 1);
	mViewEndX = Clamp<int>(mViewEndX, 0, mCountX - 1);
	mViewStartY = Clamp<int>(mViewStartY, 0, mCountY - 1);
	mViewEndY = Clamp<int>(mViewEndY, 0, mCountY - 1);

	// 이번 프레임에 실제로 그려야 할 타일의 총 개수를 계산합니다.
	int	DataCountX = mViewEndX - mViewStartX + 1;
	int	DataCountY = mViewEndY - mViewStartY + 1;
	mInstancingCount = DataCountX * DataCountY;

	// 인스턴싱 데이터를 담을 메모리(버퍼)를 준비합니다.
	if (mTileIstData.size() < (size_t)mInstancingCount)
	{
		mTileIstData.clear();
		mTileIstData.resize(mInstancingCount);
	}

	if (mTileLineIstData.size() < (size_t)mInstancingCount)
	{
		mTileLineIstData.clear();
		mTileLineIstData.resize(mInstancingCount);
	}

	auto	Owner = mOwner.lock();
	mInstancingCount = 0;
	mLineInstancingCount = 0;

	// [인스턴싱 정보 생성 루프]
	// 화면에 보이는 영역(ViewStartX~ViewEndX 등)만큼만 반복문을 돌립니다.
	for (int i = mViewStartY; i <= mViewEndY; ++i)
	{
		for (int j = mViewStartX; j <= mViewEndX; ++j)
		{
			int	Index = i * mCountX + j;

			if (mTileList[Index]->GetRender())
			{
				// 타일 하나하나의 크기(Scale)와 위치(Translate) 행렬을 만듭니다.
				FMatrix	ScaleMat, TranslateMat, WorldMat;
				ScaleMat.Scaling(mTileSize);

				FVector2	Pos = mTileList[Index]->GetPos();
				Pos.x += Owner->GetWorldPos().x;
				Pos.y += Owner->GetWorldPos().y;
				TranslateMat.Translation(Pos);

				// 크기 * 이동 = 월드 행렬 (타일이 월드 어디에 어떻게 서 있는지)
				WorldMat = ScaleMat * TranslateMat;

				// 3D 공간의 위치를 2D 화면 좌표로 바꿔주는 WVP(World * View * Projection) 행렬을 계산합니다.
				FMatrix	ViewMat = CameraMgr->GetViewMatrix();
				FMatrix	ProjMat = CameraMgr->GetProjMatrix();
				FMatrix	WVPMat = WorldMat * ViewMat * ProjMat;

				// GPU에 넘겨줄 인스턴싱 데이터 시트(Sheet)에 정보를 기입합니다.
				mTileIstData[mInstancingCount].WVP0 = WVPMat[0];
				mTileIstData[mInstancingCount].WVP1 = WVPMat[1];
				mTileIstData[mInstancingCount].WVP2 = WVPMat[2];
				mTileIstData[mInstancingCount].WVP3 = WVPMat[3];

				// 타일 이미지 시트에서 어느 부분을 잘라다 쓸지 UV 좌표로 알려줍니다.
				mTileIstData[mInstancingCount].LTUV =
					mTileList[Index]->GetFrameStart() / mTileTextureSize;
				mTileIstData[mInstancingCount].RBUV =
					mTileList[Index]->GetFrameEnd() / mTileTextureSize;

				mTileIstData[mInstancingCount].Color =
					mTileList[Index]->GetOutLineColor();

				++mInstancingCount;
			}

			// 타일 외곽선(그리드)을 그려야 한다면 관련 정보도 따로 모읍니다.
			if (mTileList[Index]->GetOutLineRender())
			{
				FMatrix	ScaleMat, TranslateMat, WorldMat;
				ScaleMat.Scaling(mTileSize);

				FVector2	Pos = mTileList[Index]->GetPos();
				Pos.x += Owner->GetWorldPos().x;
				Pos.y += Owner->GetWorldPos().y;
				TranslateMat.Translation(Pos);

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

	// 작성된 수백~수천 개의 타일 데이터를 GPU 비디오 메모리로 한 번에 쏴줍니다.
	SetInstancingData(&mTileIstData[0], mInstancingCount);
	SetLineInstancingData(&mTileLineIstData[0], mLineInstancingCount);
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

/*
 * [CreateTile] - 실제 타일맵의 뼈대를 만드는 함수
 * 가로/세로 개수만큼 타일(CTile) 객체들을 생성하고 메모리에 배치하며, 필요한 외곽선 메쉬 등을 설정합니다.
 */
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

	// 타일 모양(Shape)에 따라 전체 맵의 가로/세로 크기(mMapSize)를 계산하고,
	// 격자(외곽선)를 그릴 때 사용할 전용 메쉬를 가져옵니다.
	switch (mShape)
	{
	case Rect: // 사각 타일맵
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

	case Isometric: // 아이소매트릭 타일맵
		// 아이소매트릭은 타일이 겹치기 때문에 계산식이 조금 다릅니다.
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

	// 렌더러에게 맵의 전체 스케일 정보를 알려줍니다.
	std::shared_ptr<CTileMapRender>	Renderer =
		Owner->FindComponent<CTileMapRender>().lock();

	if (Renderer)
	{
		Renderer->SetWorldScale(mMapSize);
	}

	// 기존 타일 목록을 지우고 새로 생성할 준비를 합니다.
	mTileList.clear();
	mTileList.resize(mCountX * mCountY);

	// 이중 반복문을 돌며 타일을 하나씩 생성하고 위치를 잡아줍니다.
	for (int i = 0; i < mCountY; ++i)
	{
		for (int j = 0; j < mCountX; ++j)
		{
			int	Index = i * mCountX + j;

			mTileList[Index].reset(new CTile);
			mTileList[Index]->SetIndex(j, i, Index);

			switch (mShape)
			{
			case Rect: // 사각형 배치
				mTileList[Index]->SetPos(j * mTileSize.x,
					i * mTileSize.y);
				break;
			case Isometric: // 마름모(지그재그) 배치
				if (i % 2 == 0) // 짝수 줄
				{
					mTileList[Index]->SetPos(j * mTileSize.x,
						i * mTileSize.y * 0.5f);
				}
				else // 홀수 줄 (반 칸 밀기)
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

	// [화면 컬링용 버퍼 크기 결정]
	// 화면에 동시에 보일 수 있는 최대 타일 개수를 예측하여 GPU 메모리를 예약합니다.
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

	// 내비게이션(경로 탐색)을 위한 전용 스레드를 생성합니다.
	auto	Nav = World->GetNavigation().lock();
	SYSTEM_INFO	SysInfo = {};
	GetSystemInfo(&SysInfo);

	Nav->CreateNavigationThread((int)SysInfo.dwNumberOfProcessors,
		std::dynamic_pointer_cast<CTileMapComponent>(mSelf.lock()));
}

/*
 * [CreateInstancingBuffer] - GPU 전용 메모리 공간 건설
 * 수천 개의 타일 정보를 GPU가 한 번에 읽어갈 수 있도록 전용 버퍼(ID3D11Buffer)를 생성합니다.
 */
bool CTileMapComponent::CreateInstancingBuffer(int Size,
	int Count)
{
	SAFE_RELEASE(mInstancingBuffer.Buffer);

	mInstancingBuffer.Size = Size;
	mInstancingBuffer.Count = Count;

	// 버퍼를 생성하기 위한 설정값(Description)을 채웁니다.
	D3D11_BUFFER_DESC	BufferDesc = {};

	// 버퍼의 전체 메모리 크기 = (데이터 하나당 크기 * 개수)
	BufferDesc.ByteWidth = Size * Count;
	// 수시로 내용을 바꿔야 하므로 DYNAMIC(동적) 속성을 줍니다.
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	// 이 버퍼는 정점 데이터(Vertex Buffer) 용도임을 명시합니다.
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// CPU에서 데이터를 써야 하므로 쓰기 권한을 줍니다 (WRITE).
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

	if (Size > 0)
	{
		fwrite(&mTileFrame[0], sizeof(FTileFrame), Size, File);
	}

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

	fread(&mShape, sizeof(ETileShape), 1, File);
	fread(&mTileSize, sizeof(FVector2), 1, File);
	fread(&mMapSize, sizeof(FVector2), 1, File);

	fread(&mCountX, sizeof(int), 1, File);
	fread(&mCountY, sizeof(int), 1, File);

	fread(&mTileOutLineRender, sizeof(bool), 1, File);

	FResolution	RS = CDevice::GetInst()->GetResolution();

	const float SafeTileWidth = mTileSize.x > 0.f ? mTileSize.x : 1.f;
	const float SafeTileHeight = mTileSize.y > 0.f ? mTileSize.y : 1.f;

	int	ViewCountX = (int)(RS.Width / SafeTileWidth + 3);
	int	ViewCountY = (int)(RS.Height / SafeTileHeight + 3);

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

	if (Size > 0)
	{
		fread(&mTileFrame[0], sizeof(FTileFrame), Size, File);
	}

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
