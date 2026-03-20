#pragma once

#include "Object/GameObject.h"
#include "Vector2.h"
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

	std::shared_ptr<class CPlacementAreaObject> GetBuilding() const
	{
		return mBuilding.lock();
	}

	virtual bool Init() override;
	virtual void Update(float DeltaTime) override;
	bool TryGetProjectedScreenBounds(
		FVector2& OutMin,
		FVector2& OutMax,
		FVector2& OutCenter) const;

private:
	void SyncVisuals();
	bool BindSpriteTexture(const class CPlacementAreaObject& Building);
};
