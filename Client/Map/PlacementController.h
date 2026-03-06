#pragma once

#include "Object/GameObject.h"
#include "../Map/PlacementAreaObject.h"
#include "../Building/BuildingCatalog.h"
#include <string>
#include <vector>

class CPlacementController :
    public CGameObject
{
    friend class CWorld;
    friend class CObject;

protected:
    CPlacementController();
    CPlacementController(const CPlacementController& ref);
    CPlacementController(CPlacementController&& ref) noexcept;

public:
    virtual ~CPlacementController();

private:
    std::vector<std::weak_ptr<class CPlacementAreaObject>>
        mPlacementObjects;
    std::weak_ptr<class CPlacementAreaObject> mActivePlacementObject;
    std::weak_ptr<class CPlacementAreaObject> mDemolitionHoverObject;
    std::weak_ptr<class CCitizenInfoWidget> mCitizenInfoWidget;
    bool mDemolitionMode = false;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    void SetDemolitionMode(bool Enable);
    bool IsDemolitionMode() const
    {
        return mDemolitionMode;
    }
    bool BeginBuildPlacement(
        const FBuildingCatalogEntry& Entry,
        const std::string& SpriteTexturePath);

private:
    void RotateCurrentAreaCCW();
    void RotateCurrentAreaCW();
    void MoveCurrentArea();
    void PlaceCurrentArea();
    void UpdateDemolitionHoverPreview();
    void ClearDemolitionHoverPreview();
    void RefreshPlacementObjects();
    std::shared_ptr<class CPlacementAreaObject> PickPlacementObject(
        const FVector2& MouseWorldPos);
    std::shared_ptr<class CBuildingMarkerOrb> PickCitizenOrb(
        const FVector2& MouseWorldPos);
    void EnsureCitizenInfoWidget();
    void DemolishPlacementObject(
        const std::shared_ptr<class CPlacementAreaObject>& PlacementObject);
    void RegisterBuildingToOrbs(
        const std::string& BuildingObjectName);
    void UnregisterBuildingFromOrbs(
        const std::string& BuildingObjectName);
};
