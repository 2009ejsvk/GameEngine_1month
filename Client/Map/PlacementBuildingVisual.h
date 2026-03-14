#pragma once

#include "Object/GameObject.h"
#include <string>

class CBuildingVisual : public CGameObject
{
	friend class CObject;

public:
	CBuildingVisual();
	CBuildingVisual(const CBuildingVisual& ref);
	CBuildingVisual(CBuildingVisual&& ref) noexcept;

	virtual ~CBuildingVisual();

private:
	std::weak_ptr<class CPlacementAreaObject> mBuilding;
	std::weak_ptr<class CMeshComponent> mSprite;
	bool mVisible = false;
	std::string mLoadedBuildingId;
	std::string mLoadedTextureFile;
	unsigned long long mLoadedCatalogGeneration = 0;

public:
	void SetBuilding(const std::weak_ptr<class CPlacementAreaObject>& Building)
	{
		mBuilding = Building;
	}

	virtual bool Init() override;
	virtual void Update(float DeltaTime) override;

private:
	void SyncVisuals();
	bool BindSpriteTexture(const class CPlacementAreaObject& Building);
};
