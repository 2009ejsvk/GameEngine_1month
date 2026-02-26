#pragma once

#include "GameObject.h"
#include "../Component/TileMapRender.h"
#include "../Component/TileMapComponent.h"

class CTileMapObject :
    public CGameObject
{
	friend class CWorld;
	friend class CObject;

protected:
	CTileMapObject();
	CTileMapObject(const CTileMapObject& ref);
	CTileMapObject(CTileMapObject&& ref)	noexcept;

public:
	virtual ~CTileMapObject();

protected:
	std::weak_ptr<CTileMapRender>		mTileMapRender;
	std::weak_ptr<CTileMapComponent>	mTileMap;

public:
	std::weak_ptr<CTileMapComponent> GetTileMap()	const
	{
		return mTileMap;
	}

public:
	virtual void Begin();
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void Destroy();
};

