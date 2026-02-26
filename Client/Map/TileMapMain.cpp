#include "TileMapMain.h"

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

		Render->SetTexture(ETileTextureType::Back,
			"MainMap", TEXT("MapBackGround.png"));

		Render->SetWorldScale(6400.f, 6400.f);

		/*Render->SetTexture(ETileTextureType::Tile,
			"MainTile", 
			TEXT("Diablos_Lair_Floor_TRS/Diablos_Lair_Floor.png"));

		for (int i = 0; i < 76; ++i)
		{
			for (int j = 0; j < 5; ++j)
			{
				Render->AddTileFrame(j * 160.f, i * 80.f,
					(j + 1) * 160.f, (i + 1) * 80.f);
			}
		}*/

		Render->SetTexture(ETileTextureType::Tile,
			"MainTile",
			TEXT("Floors.png"));

		for (int i = 0; i < 5; ++i)
		{
			Render->AddTileFrame(0.f, i * 64.f, 
				64.f, 64.f + i * 64.f);
		}
	}

	auto	TileMap = mTileMap.lock();

	if (TileMap)
	{
		/*TileMap->CreateTile(ETileShape::Isometric, 100, 100,
			FVector2(160.f, 80.f));

		TileMap->SetTileFrameAll(3);*/

		TileMap->CreateTile(ETileShape::Rect, 100, 100,
			FVector2(64.f, 64.f));

		TileMap->SetTileFrameAll(0);
	}

	return true;
}

void CTileMapMain::LoadTileMap(const TCHAR* FileName,
	const std::string& PathName)
{
	auto	TileMap = mTileMap.lock();

	TileMap->Load(FileName, PathName);
}
