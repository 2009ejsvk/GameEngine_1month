#pragma once

#include "Object/GameObject.h"
#include "../Map/PlacementAreaObject.h"
#include "../Building/BuildingCatalogEntry.h"
#include <string>
#include <vector>

class CPlacementController :
    public CGameObject
{
    friend class CObject;

public:
    CPlacementController();
    CPlacementController(const CPlacementController& ref);
    CPlacementController(CPlacementController&& ref) noexcept;

    virtual ~CPlacementController();

private:
    std::vector<std::weak_ptr<class CPlacementAreaObject>>
        mPlacementObjects;
    std::weak_ptr<class CPlacementAreaObject> mActivePlacementObject;
    std::weak_ptr<class CPlacementAreaObject> mDemolitionHoverObject;
    std::weak_ptr<class CCitizenInfoWidget> mCitizenInfoWidget;
    bool mDemolitionMode = false;
    bool mRoadBrushMode = false;
    FBuildingCatalogEntry mRoadBrushEntry;
    std::string mRoadBrushSpriteTexturePath;
    int mLastRoadBrushTileIndex = -1;
    int mRoadPathStartTileIndex = -1;
    int mRoadPreviewEndTileIndex = -1;
    std::vector<int> mRoadPreviewTileIndices;
    std::weak_ptr<class CTileMapObject> mRoadPreviewTileMapObject;

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
    bool BeginMoveExistingBuilding(const std::string& BuildingObjectName);
    bool DemolishBuildingByName(const std::string& BuildingObjectName);

private:
    void RotateCurrentAreaCCW();
    void RotateCurrentAreaCW();
    void MoveCurrentArea();
    void PlaceCurrentArea();
    void PaintRoadHold();
    void FinishRoadBrushStroke();
    void CancelPlacementMode();
    void CancelActivePlacementSession();
    void UpdateDemolitionHoverPreview();
    void ClearDemolitionHoverPreview();
    void RefreshPlacementObjects();
    bool IsPointerOverActiveUi(const FVector2& MouseScreenPos) const;
    bool IsPlacementInputBlocked(const FVector2& MouseScreenPos) const;
    bool TryCommitActivePlacement();
    bool RestartRoadBrushPlacement();
    void UpdateRoadPathPreview();
    bool TryGetTileMap(
        std::shared_ptr<class CTileMapComponent>& OutTileMap) const;
    bool TryGetRoadPreviewTileMap(
        std::shared_ptr<class CTileMapComponent>& OutTileMap);
    bool TryGetMouseTileIndex(
        const FVector2& MouseWorldPos, int& OutTileIndex) const;
    bool TryGetTileWorldPos(
        int TileIndex, FVector2& OutWorldPos) const;
    bool ResolveRoadPathToTile(
        int EndTileIndex,
        std::vector<int>& OutPathIndices) const;
    bool IsRoadPathTileTraversable(
        const std::shared_ptr<class CTileMapComponent>& TileMap,
        int TileIndex) const;
    void ApplyRoadPreviewTiles(const std::vector<int>& TileIndices);
    bool EnsureRoadPlacementObject();
    bool TryPlaceRoadAtTile(
        int TileIndex,
        CPlacementAreaObject::FScopedTopologyBatch* TopologyBatch = nullptr);
    bool CommitRoadPathToTile(int EndTileIndex);
    void ClearRoadBrushMode();
    std::shared_ptr<class CPlacementAreaObject> PickPlacementObjectByVisual(
        const FVector2& MouseScreenPos);
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
