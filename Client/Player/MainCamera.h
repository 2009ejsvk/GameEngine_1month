#pragma once

#include "Object/GameObject.h"
#include "../Map/PlacementAreaObject.h"
#include <string>
#include <vector>

class CMainCamera :
    public CGameObject
{
    friend class CWorld;
    friend class CObject;

protected:
    CMainCamera();
    CMainCamera(const CMainCamera& ref);
    CMainCamera(CMainCamera&& ref) noexcept;

public:
    virtual ~CMainCamera();

private:
    std::weak_ptr<class CCameraComponent> mCameraComponent;
    std::weak_ptr<class CObjectMovementComponent> mMovement;
    std::vector<std::weak_ptr<class CPlacementAreaObject>>
        mPlacementObjects;
    std::weak_ptr<class CPlacementAreaObject> mActivePlacementObject;
    std::weak_ptr<class CPlacementAreaObject> mDemolitionHoverObject;
    std::weak_ptr<class CCitizenInfoWidget> mCitizenInfoWidget;
    float mViewDistance = 1000.f;
    float mZoomWidth = 0.f;
    float mZoomHeight = 0.f;
    float mZoomAspect = 16.f / 9.f;
    float mMinZoomWidth = 200.f;
    float mMaxZoomWidth = 10000.f;
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
        const std::string& BuildingId,
        const std::string& BuildingDisplayName,
        const std::string& CategoryName,
        bool Residential,
        int Capacity,
        bool FoodProvider,
        bool EntertainmentProvider,
        int HousingSatisfactionCap,
        int JobSatisfactionCap,
        int FoodSatisfactionCap,
        int FunSatisfactionCap,
        EPlacementTemplateType TemplateType,
        EPlacementBuildingKind BuildingKind);

private:
    void MoveUp();
    void MoveDown();
    void MoveLeft();
    void MoveRight();
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
