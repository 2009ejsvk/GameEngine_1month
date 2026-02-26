#pragma once

#include "../ThreadBase.h"
#include "Navigation.h"

namespace ENavigationHeader
{
	enum Type
	{
		FindPath,
		FindComplete,
		Exit
	};
}

class CThreadNavigation :
    public CThreadBase
{
	friend class CThreadManager;

protected:
	CThreadNavigation();

public:
	virtual ~CThreadNavigation();

private:
	std::weak_ptr<class CWorldNavigation>	mWorldNavigation;
	std::shared_ptr<CNavigation>	mNavigation;

public:
	void SetWorldNavigation(
	const std::weak_ptr<class CWorldNavigation>& Nav);
	void SetTileMap(const std::weak_ptr<class CTileMapComponent>& TileMap);

public:
	virtual void Exit();
	virtual void Run();
};

