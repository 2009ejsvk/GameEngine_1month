#pragma once

#include "Object/GameObject.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

enum class EPlacementBuildingKind
{
    BuildingA,
    BuildingB
};

enum class EPlacementTemplateType
{
    Diamond3x3SingleMarker,
    Diamond5x5TwoMarker,
    Diamond5x5FourMarker,
    Diamond7x7ThreeMarker
};

struct FPlacementMarkerAnchor
{
    float LogicalOffsetX = 0.f;
    float LogicalOffsetY = 0.f;
};

struct FPlacementTemplate
{
    EPlacementTemplateType Type =
        EPlacementTemplateType::Diamond3x3SingleMarker;
    int DiamondRadius = 1;
    std::vector<FPlacementMarkerAnchor> MarkerAnchors;
    FVector4 AreaColor = FVector4::Blue;

    int GetExpectedTileCount() const
    {
        const int Side = DiamondRadius * 2 + 1;
        return Side * Side;
    }
};

class CPlacementAreaObject :
    public CGameObject
{
    friend class CWorld;
    friend class CObject;

protected:
    CPlacementAreaObject();
    CPlacementAreaObject(const CPlacementAreaObject& ref);
    CPlacementAreaObject(CPlacementAreaObject&& ref) noexcept;

public:
    virtual ~CPlacementAreaObject();

private:
    std::weak_ptr<class CTileMapObject> mTileMapObject;
    std::weak_ptr<class CTileMapObject> mBlueOverlayTileMapObject;
    std::weak_ptr<class CTileMapObject> mYellowOverlayTileMapObject;
    std::vector<int> mPrimaryPlacedIndices;
    std::vector<int> mAppliedPrimaryOverlayIndices;
    std::vector<int> mAppliedMarkerOverlayIndices;
    std::vector<int> mPreviewIndices;
    bool mPreviewCanPlace = false;
    bool mDemolitionHoverActive = false;
    bool mTileMapPrepared = false;
    bool mMovePreviewActive = false;
    int mPlacedCenterIndex = -1;
    int mPreviewCenterIndex = -1;
    int mInitialCenterOffsetX = 0;
    int mInitialCenterOffsetY = 0;
    bool mAutoPlaceOnPrepare = true;
    std::string mBuildingId;
    std::string mBuildingDisplayName;
    std::string mBuildingCategoryName;
    bool mResidential = false;
    bool mFoodProvider = false;
    bool mEntertainmentProvider = false;
    int mHousingSatisfactionCap = 100;
    int mJobSatisfactionCap = 100;
    int mFoodSatisfactionCap = 100;
    int mFunSatisfactionCap = 100;
    int mBudgetLevel = 3;
    int mCapacity = 0;
    EPlacementBuildingKind mBuildingKind = EPlacementBuildingKind::BuildingB;
    FPlacementTemplate mTemplate;
    std::vector<int> mMarkerTileIndices;

public:
    void SetTileMapObject(
        const std::weak_ptr<class CTileMapObject>& TileMapObject)
    {
        mTileMapObject = TileMapObject;
    }

    void SetInitialCenterOffset(int OffsetX, int OffsetY)
    {
        mInitialCenterOffsetX = OffsetX;
        mInitialCenterOffsetY = OffsetY;
    }

    void SetBuildingKind(EPlacementBuildingKind Kind)
    {
        mBuildingKind = Kind;
    }

    void SetBuildingId(const std::string& Id)
    {
        mBuildingId = Id;

        if (mBuildingDisplayName.empty())
            mBuildingDisplayName = Id;
    }

    const std::string& GetBuildingId() const
    {
        return mBuildingId;
    }

    void SetAutoPlaceOnPrepare(bool AutoPlace)
    {
        mAutoPlaceOnPrepare = AutoPlace;
    }

    void SetBuildingDisplayInfo(
        const std::string& DisplayName,
        const std::string& CategoryName,
        bool Residential,
        int Capacity,
        bool FoodProvider = false,
        bool EntertainmentProvider = false,
        int HousingSatisfactionCap = 100,
        int JobSatisfactionCap = 100,
        int FoodSatisfactionCap = 100,
        int FunSatisfactionCap = 100)
    {
        auto ClampTo100 = [](int Value)
        {
            return (std::max)(0, (std::min)(100, Value));
        };

        mBuildingDisplayName = DisplayName;
        mBuildingCategoryName = CategoryName;
        mResidential = Residential;
        mCapacity = Capacity;
        mFoodProvider = FoodProvider;
        mEntertainmentProvider = EntertainmentProvider;
        mHousingSatisfactionCap = ClampTo100(HousingSatisfactionCap);
        mJobSatisfactionCap = ClampTo100(JobSatisfactionCap);
        mFoodSatisfactionCap = ClampTo100(FoodSatisfactionCap);
        mFunSatisfactionCap = ClampTo100(FunSatisfactionCap);
        mBudgetLevel = 3;
    }

    const std::string& GetBuildingDisplayName() const
    {
        return mBuildingDisplayName.empty() ?
            mBuildingId :
            mBuildingDisplayName;
    }

    const std::string& GetBuildingCategoryName() const
    {
        return mBuildingCategoryName;
    }

    bool IsResidential() const
    {
        return mResidential;
    }

    bool IsFoodProvider() const
    {
        return mFoodProvider;
    }

    bool IsEntertainmentProvider() const
    {
        return mEntertainmentProvider;
    }

    int GetCapacity() const
    {
        return mCapacity;
    }

    int GetHousingSatisfactionCap() const
    {
        return ApplyBudgetScale(mHousingSatisfactionCap);
    }

    int GetJobSatisfactionCap() const
    {
        return ApplyBudgetScale(mJobSatisfactionCap);
    }

    int GetFoodSatisfactionCap() const
    {
        return ApplyBudgetScale(mFoodSatisfactionCap);
    }

    int GetFunSatisfactionCap() const
    {
        return ApplyBudgetScale(mFunSatisfactionCap);
    }

    int GetBudgetLevel() const
    {
        return mBudgetLevel;
    }

    void SetBudgetLevel(int Level)
    {
        mBudgetLevel = (std::max)(1, (std::min)(5, Level));
    }

    float GetBudgetSatisfactionScale() const
    {
        switch (mBudgetLevel)
        {
        case 1: return 0.70f;
        case 2: return 0.85f;
        case 4: return 1.15f;
        case 5: return 1.30f;
        default: return 1.00f;
        }
    }

    bool HasPlacedArea() const
    {
        return mPlacedCenterIndex >= 0 &&
            !mPrimaryPlacedIndices.empty();
    }

    int GetDiamondRadius() const
    {
        return mTemplate.DiamondRadius;
    }

    void SetPlacementTemplateType(EPlacementTemplateType Type);
    void SetPlacementTemplate(const FPlacementTemplate& Template);

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Destroy() override;
    virtual bool IsNavigationObstacle() const override;
    virtual void GetNavigationBlockedTiles(
        std::vector<int>& OutIndices) override;
    virtual void GetNavigationGoalTiles(
        std::vector<int>& OutIndices) override;

public:
    void StartMovePreview(const FVector2& MouseWorldPos);
    void ConfirmPlacement();
    void CancelMovePreview();
    bool IsMovePreviewActive() const
    {
        return mMovePreviewActive;
    }
    void SetDemolitionHoverActive(bool Active)
    {
        mDemolitionHoverActive = Active;
    }
    bool ContainsPlacedTile(const FVector2& MouseWorldPos);
    float GetCenterDistanceSq(const FVector2& MouseWorldPos) const;
    bool GetMarkerWorldPos(FVector3& OutWorldPos);
    bool GetMarkerWorldPositions(std::vector<FVector3>& OutWorldPosList);
    bool GetClosestMarkerWorldPos(
        const FVector3& RefWorldPos, FVector3& OutWorldPos);
    bool GetTileSize(FVector2& OutTileSize);

private:
    static FPlacementTemplate CreateTemplateByType(
        EPlacementTemplateType Type);
    void EnsureTemplateValidity();
    void ResetPlacementState();
    void EnsurePlacementObject();
    void UpdatePlacementPreviewFromMouse(const FVector2& MouseWorldPos);
    void ClearPreview();
    bool AcquireTileMap(std::shared_ptr<class CTileMapComponent>& OutTileMap);
    bool AcquireBlueOverlayTileMap(
        std::shared_ptr<class CTileMapComponent>& OutTileMap);
    bool AcquireYellowOverlayTileMap(
        std::shared_ptr<class CTileMapComponent>& OutTileMap);
    void UpdatePrimaryOverlayTiles(const std::vector<int>& NextIndices);
    void UpdateMarkerOverlayTiles(const std::vector<int>& NextIndices);
    bool BuildDiamondAreaIndices(
        const std::shared_ptr<class CTileMapComponent>& TileMap,
        int CenterIndex, std::vector<int>& OutIndices) const;
    bool IsAreaPlaceable(
        const std::shared_ptr<class CTileMapComponent>& TileMap,
        const std::vector<int>& Indices) const;
    void SetAreaColor(
        const std::shared_ptr<class CTileMapComponent>& TileMap,
        const std::vector<int>& Indices, const FVector4& Color);
    bool IsPlacedIndex(int Index) const;
    int FindMarkerTileIndexByLogicalOffset(
        const std::shared_ptr<class CTileMapComponent>& TileMap,
        int CenterIndex, const std::vector<int>& Indices,
        float TargetOffsetX, float TargetOffsetY) const;
    void ApplyPlacedAreaColor(
        const std::shared_ptr<class CTileMapComponent>& TileMap);
    void RestoreTileColor(
        const std::shared_ptr<class CTileMapComponent>& TileMap, int Index);
    void SyncWorldPosFromCenter(
        const std::shared_ptr<class CTileMapComponent>& TileMap,
        int CenterIndex);
    int GetIsoNeighborIndexByDir(
        const std::shared_ptr<class CTileMapComponent>& TileMap,
        int TileIndex, int DirIndex) const;
    int ApplyBudgetScale(int BaseCap) const
    {
        const float Scaled =
            BaseCap * GetBudgetSatisfactionScale();
        const int Rounded = static_cast<int>(roundf(Scaled));
        return (std::max)(0, (std::min)(100, Rounded));
    }
};
