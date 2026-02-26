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
	virtual void Begin();
	virtual bool Init();

public:
	void LoadTileMap(const TCHAR* FileName,
		const std::string& PathName = "Root");
};

