#pragma once

#include "Object/GameObject.h"
#include <string>

class CBuildingVisual : public CGameObject
{
	friend class CWorld;
	friend class CObject;

protected:
	CBuildingVisual();
	CBuildingVisual(const CBuildingVisual& ref);
	CBuildingVisual(CBuildingVisual&& ref) noexcept;

public:
	virtual ~CBuildingVisual();

private:
	std::weak_ptr<class CPlacementAreaObject> mBuilding;
	std::weak_ptr<class CMeshComponent> mSprite;
	bool mVisible = false;
	std::string mLoadedBuildingId;
	std::string mLoadedTextureFile;

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
