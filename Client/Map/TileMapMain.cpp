#include "TileMapMain.h"
#include <cstdio>

CTileMapMain::CTileMapMain()
{
}

CTileMapMain::CTileMapMain(const CTileMapMain& ref) :
	CTileMapObject(ref)
{
}

CTileMapMain::CTileMapMain(CTileMapMain&& ref) noexcept :
	CTileMapObject(ref)
{
}

CTileMapMain::~CTileMapMain()
{
}

void CTileMapMain::Begin()
{
	CTileMapObject::Begin();
}

bool CTileMapMain::Init()
{
	CTileMapObject::Init();

	auto	Render = mTileMapRender.lock();

	if (Render)
	{
		Render->EnableTileAlphaBlend();

		Render->SetWorldScale(6400.f, 6400.f);

		// 이미지 없이 셰이더 색상만으로 타일을 채우기 위해
		// 더미 프레임 1개만 등록한다.
		Render->AddTileFrame(0.f, 0.f, 1.f, 1.f);
	}

	auto	TileMap = mTileMap.lock();

	if (TileMap)
	{
		const int CountX = 50;
		const int CountY = 100;

		TileMap->CreateTile(ETileShape::Isometric, CountX, CountY,
			FVector2(160.f, 80.f));
		TileMap->SetViewCulling(true);
		TileMap->SetTileTextureSize(1.f, 1.f);

		TileMap->SetTileFrameAll(0);
		TileMap->SetTileOutLineRender(true);

		const int TileCount = TileMap->GetTileCountX() *
			TileMap->GetTileCountY();

		for (int i = 0; i < TileCount; ++i)
		{
			auto Tile = TileMap->GetTile(i).lock();

			if (!Tile)
				continue;

			Tile->SetOutLineColor(1.f, 1.f, 1.f, 1.f);
		}

		// 논리적 좌표계로 변환하여 지그재그 타일맵에서도 대칭이 맞는 마름모 형태로 그린다.
		const int CenterX = CountX / 2;
		const int CenterY = CountY / 2;
		const float DiamondRadius = 1.0f;

		// 중심점의 논리적 위치 (홀수 행은 X축이 반 칸 시프트되어 있음)
		const float CenterLogicalX = CenterX + (CenterY % 2 == 0 ? 0.0f : 0.5f);
		const float CenterLogicalY = CenterY * 0.5f;

		for (int y = 0; y < CountY; ++y)
		{
			for (int x = 0; x < CountX; ++x)
			{
				// 현재 타일의 논리적 위치 계산
				float LogicalX = x + (y % 2 == 0 ? 0.0f : 0.5f);
				float LogicalY = y * 0.5f;

				// 지그재그 현상을 보정한 진정한 거리 계산
				float DistX = abs(LogicalX - CenterLogicalX);
				float DistY = abs(LogicalY - CenterLogicalY);

				if (DistX + DistY > DiamondRadius)
					continue;

				const int Index = y * CountX + x;
				auto Tile = TileMap->GetTile(Index).lock();

				if (!Tile)
					continue;

				Tile->SetOutLineColor(1.f, 0.f, 0.f, 1.f);
			}
		}
	}

	return true;
}

void CTileMapMain::PostUpdate(float DeltaTime)
{
	CTileMapObject::PostUpdate(DeltaTime);

#ifdef _DEBUG
	mRenderCountLogTime += DeltaTime;

	if (mRenderCountLogTime < 1.f)
		return;

	mRenderCountLogTime = 0.f;

	auto TileMap = mTileMap.lock();

	if (!TileMap)
		return;

	char	Text[160] = {};
	sprintf_s(Text, "[TileMap] RenderTile=%d, OutLine=%d\n",
		TileMap->GetInstancingCount(),
		TileMap->GetLineInstancingCount());
	OutputDebugStringA(Text);
#endif
}

void CTileMapMain::LoadTileMap(const TCHAR* FileName,
	const std::string& PathName)
{
	auto	TileMap = mTileMap.lock();

	TileMap->Load(FileName, PathName);
}
