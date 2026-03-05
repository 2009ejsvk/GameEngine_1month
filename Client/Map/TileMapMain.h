#pragma once

#include "Object/TileMapObject.h"

class CTileMapMain :
    public CTileMapObject
{
	friend class CWorld;
	friend class CObject;

protected:
	CTileMapMain();
	CTileMapMain(const CTileMapMain& ref);
	CTileMapMain(CTileMapMain&& ref)	noexcept;

public:
	virtual ~CTileMapMain();

public:
	static constexpr int BaseTileCountX = 50;
	static constexpr int BaseTileCountY = 100;
	static constexpr int SeaBorderX = 5;
	static constexpr int SeaBorderY = 10;
	static constexpr float TileTextureWidth = 132.f;
	static constexpr float TileTextureHeight = 99.f;

public:
	virtual void Begin();
	virtual bool Init();
	virtual void PostUpdate(float DeltaTime);

public:
	void LoadTileMap(const TCHAR* FileName,
		const std::string& PathName = "Root");

private:
	float	mRenderCountLogTime = 0.f;
};

