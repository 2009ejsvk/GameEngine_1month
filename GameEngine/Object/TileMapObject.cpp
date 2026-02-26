#include "TileMapObject.h"

CTileMapObject::CTileMapObject()
{
}

CTileMapObject::CTileMapObject(const CTileMapObject& ref)
{
}

CTileMapObject::CTileMapObject(CTileMapObject&& ref) noexcept
{
}

CTileMapObject::~CTileMapObject()
{
}

void CTileMapObject::Begin()
{
	CGameObject::Begin();
}

bool CTileMapObject::Init()
{
	CGameObject::Init();

	mTileMapRender = CreateComponent<CTileMapRender>("Render");
	mTileMap = CreateComponent<CTileMapComponent>("TileMap");

	return true;
}

void CTileMapObject::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);
}

void CTileMapObject::PostUpdate(float DeltaTime)
{
	CGameObject::PostUpdate(DeltaTime);
}

void CTileMapObject::Render()
{
	CGameObject::Render();
}

void CTileMapObject::Destroy()
{
	CGameObject::Destroy();
}
