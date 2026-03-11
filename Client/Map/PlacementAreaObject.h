#pragma once

#include "Object/GameObject.h"
#include "../Building/BuildingTypes.h"
#include "../Citizen/CitizenTypes.h"
#include "PlacementAreaComponents.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

struct FBuildingCatalogEntry;

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
    int mPreviewDirection = 0;
    int mInitialCenterOffsetX = 0;
    int mInitialCenterOffsetY = 0;
    bool mAutoPlaceOnPrepare = true;
    std::string mBuildingId;
    std::string mBuildingDisplayName;
    std::string mBuildingCategoryName;
    std::string mBuildingSpriteTexturePath;
    FBuildingServiceProfile mServiceProfile;
    FBuildingOperationsState mOperations;
    EPlacementBuildingKind mBuildingKind = EPlacementBuildingKind::BuildingB;
    FPlacementTemplate mTemplate;
    std::vector<int> mMarkerTileIndices;
    float mAccessibilityScore = 0.f;

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

    void SetBuildingCategory(EBuildingCategory Category)
    {
        mServiceProfile.Category = Category;
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
        int FunSatisfactionCap = 100,
        int BaseMonthlyWage = -1,
        int BaseMonthlyUpkeep = -1);
    void ApplyCatalogEntry(const FBuildingCatalogEntry& Entry);

    void SetRequiredEducationLevel(ECitizenEducationLevel Level)
    {
        mServiceProfile.RequiredEducationLevel = Level;
    }

    void SetResourceBehavior(
        EResourceType ProducedResourceType,
        EResourceType VisitConsumptionResourceType,
        bool SupportsTeamsterPickup,
        bool CanExportStoredResources)
    {
        mOperations.ConfigureResourceBehavior(
            ProducedResourceType,
            VisitConsumptionResourceType,
            SupportsTeamsterPickup,
            CanExportStoredResources);
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

    EBuildingCategory GetBuildingCategory() const
    {
        return mServiceProfile.Category;
    }

    void SetBuildingSpriteTexturePath(const std::string& TexturePath)
    {
        mBuildingSpriteTexturePath = TexturePath;
    }

    const std::string& GetBuildingSpriteTexturePath() const
    {
        return mBuildingSpriteTexturePath;
    }

    bool IsResidential() const
    {
        return mServiceProfile.Residential;
    }

    bool IsFoodProvider() const
    {
        return mServiceProfile.FoodProvider;
    }

    bool IsEntertainmentProvider() const
    {
        return mServiceProfile.EntertainmentProvider;
    }

    bool IsTransportOffice() const
    {
        return mBuildingKind == EPlacementBuildingKind::TransportOffice;
    }

    bool IsRoad() const
    {
        return mBuildingKind == EPlacementBuildingKind::Road;
    }

    bool IsHarbor() const
    {
        return mBuildingKind == EPlacementBuildingKind::Harbor;
    }

    bool IsWarehouse() const
    {
        return mBuildingId == "build_1_7" ||
            mBuildingId == "build_1_8";
    }

    bool IsBusGarage() const
    {
        return mBuildingId == "build_1_11";
    }

    bool IsBusStop() const
    {
        return mBuildingId == "build_1_12";
    }

    bool IsFoodProductionFacility() const
    {
        return mOperations.ProducedResourceType == EResourceType::Food &&
            !IsTransportOffice() &&
            !IsHarbor();
    }

    bool CanGenerateWorkOutput() const
    {
        return mOperations.ProducedResourceType != EResourceType::None;
    }

    int GetCapacity() const
    {
        return mServiceProfile.Capacity;
    }

    int GetHousingSatisfactionCap() const
    {
        return mOperations.ApplyBudgetScale(
            mServiceProfile.HousingSatisfactionCap);
    }

    int GetJobSatisfactionCap() const
    {
        return mOperations.ApplyBudgetScale(
            mServiceProfile.JobSatisfactionCap);
    }

    int GetEffectiveJobSatisfactionCap() const
    {
        if (IsRoad())
            return 0;

        const float EffectiveCap =
            static_cast<float>(GetJobSatisfactionCap()) *
            (std::max)(0.f, (std::min)(1.f, mAccessibilityScore));
        return (std::max)(
            0,
            (std::min)(100, static_cast<int>(roundf(EffectiveCap))));
    }

    int GetFoodSatisfactionCap() const
    {
        return mOperations.ApplyBudgetScale(
            mServiceProfile.FoodSatisfactionCap);
    }

    int GetFunSatisfactionCap() const
    {
        return mOperations.ApplyBudgetScale(
            mServiceProfile.FunSatisfactionCap);
    }

    int GetBudgetLevel() const
    {
        return mOperations.BudgetLevel;
    }

    void SetBudgetLevel(int Level)
    {
        mOperations.SetBudgetLevel(Level);
    }

    float GetBudgetSatisfactionScale() const
    {
        return mOperations.GetBudgetSatisfactionScale();
    }

    int GetBaseMonthlyWage() const
    {
        return mOperations.BaseMonthlyWage;
    }

    int GetBaseMonthlyUpkeep() const
    {
        return mOperations.BaseMonthlyUpkeep;
    }

    ECitizenEducationLevel GetRequiredEducationLevel() const
    {
        return mServiceProfile.RequiredEducationLevel;
    }

    EResourceType GetProducedResourceType() const
    {
        return mOperations.ProducedResourceType;
    }

    EResourceType GetVisitConsumptionResourceType() const
    {
        return mOperations.VisitConsumptionResourceType;
    }

    bool SupportsTeamsterPickup() const
    {
        return mOperations.SupportsTeamsterPickup;
    }

    bool CanExportStoredResources() const
    {
        return mOperations.CanExportStoredResources;
    }

    int GetMonthlyWageCost() const
    {
        return mOperations.GetMonthlyWageCost();
    }

    int GetMonthlyUpkeepCost() const
    {
        return mOperations.GetMonthlyUpkeepCost();
    }

    int GetDailyWageCost(int DaysInMonth) const
    {
        return mOperations.GetDailyWageCost(DaysInMonth);
    }

    int GetDailyUpkeepCost(int DaysInMonth) const
    {
        return mOperations.GetDailyUpkeepCost(DaysInMonth);
    }

    bool AdvanceHarborShipProgressAndCheckArrival(int DaysInMonth)
    {
        return mOperations.AdvanceHarborShipProgressAndCheckArrival(
            IsHarbor(),
            DaysInMonth);
    }

    float GetHarborShipProgressPercent() const
    {
        return mOperations.GetHarborShipProgressPercent(IsHarbor());
    }

    int GetResourceStock() const { return mOperations.GetResourceStock(); }
    int GetResourceStock(EResourceType Type) const
    {
        return mOperations.GetResourceStock(Type);
    }
    int GetExportableResourceStock() const
    {
        return mOperations.GetExportableResourceStock();
    }
    int GetAvailableExportableResourceStock() const
    {
        return mOperations.GetAvailableExportableResourceStock();
    }
    int GetAvailableResourceStock(EResourceType Type) const
    {
        return mOperations.GetAvailableResourceStock(Type);
    }
    int GetAvailableIncomingCapacity(EResourceType Type) const
    {
        return mOperations.GetAvailableIncomingCapacity(Type);
    }
    int GetReservedIncomingResourceAmount(EResourceType Type) const
    {
        return mOperations.GetReservedIncomingResourceAmount(Type);
    }
    int GetWarehouseSlotCount() const
    {
        return IsWarehouse() ?
            FBuildingOperationsState::WarehouseSlotCount :
            0;
    }
    EResourceType GetWarehouseSlotType(int SlotIndex) const
    {
        return static_cast<EResourceType>(
            mOperations.GetWarehouseSlotType(SlotIndex));
    }
    int GetResourceTypeCapacity(EResourceType Type) const
    {
        return mOperations.GetResourceTypeCapacity(Type);
    }
    bool CanStoreResourceType(EResourceType Type) const
    {
        return mOperations.CanStoreResourceType(Type);
    }
    int GetMaxResourceStock() const
    {
        return mOperations.GetMaxTotalResourceStock();
    }

    void AddResourceStock(int Amount)
    {
        AddResourceStock(
            ResolvePrimaryResourceTypeForLegacy(),
            Amount);
    }

    void AddResourceStock(EResourceType Type, int Amount)
    {
        mOperations.AddResourceStock(Type, Amount);
    }

    bool TryAddResourceStock(EResourceType Type, int Amount)
    {
        return mOperations.TryAddResourceStock(Type, Amount);
    }

    // AtWork 시민이 매 프레임 호출 - float 누적 후 int 단위로 재고에 추가
    void AddProduction(float UnitsPerSec, float DeltaTime)
    {
        if (UnitsPerSec <= 0.f ||
            mOperations.ProducedResourceType == EResourceType::None)
        {
            return;
        }

        mOperations.AddProduction(UnitsPerSec, DeltaTime);
    }

    // AtFood 진입 시 호출 - 재고 1단위 소비 시도, 성공하면 true
    bool TryConsumeResource(int Amount = 1)
    {
        return TryConsumeResource(
            ResolvePrimaryResourceTypeForLegacy(),
            Amount);
    }

    bool TryConsumeResource(EResourceType Type, int Amount = 1)
    {
        return mOperations.TryConsumeResource(Type, Amount);
    }

    bool TryConsumeAnyExportableResource(
        int Amount,
        EResourceType& OutType)
    {
        return mOperations.TryConsumeAnyExportableResource(Amount, OutType);
    }

    bool TryConsumeExportableResources(int Amount)
    {
        return mOperations.TryConsumeExportableResources(Amount);
    }

    bool TryGetExportableResourceTypeForAmount(
        int Amount,
        EResourceType& OutType) const
    {
        return mOperations.TryGetExportableResourceTypeForAmount(
            Amount,
            OutType);
    }

    bool ReserveTeamsterExportPickup(int Amount)
    {
        return mOperations.ReserveExportPickupAmount(Amount);
    }

    void ReleaseTeamsterExportPickup(int Amount)
    {
        mOperations.ReleaseExportPickupAmount(Amount);
    }

    bool ReserveTeamsterPickup(EResourceType Type, int Amount)
    {
        return mOperations.ReserveResourcePickupAmount(Type, Amount);
    }

    void ReleaseTeamsterPickup(EResourceType Type, int Amount)
    {
        mOperations.ReleaseResourcePickupAmount(Type, Amount);
    }

    bool ReserveIncomingResource(EResourceType Type, int Amount)
    {
        return mOperations.ReserveIncomingResourceAmount(Type, Amount);
    }

    void ReleaseIncomingResource(EResourceType Type, int Amount)
    {
        mOperations.ReleaseIncomingResourceAmount(Type, Amount);
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

    float GetAccessibilityScore() const
    {
        return mAccessibilityScore;
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
    void RotatePreviewCW(const FVector2& MouseWorldPos);
    void RotatePreviewCCW(const FVector2& MouseWorldPos);
    int GetPlacementDirection() const
    {
        return (mPreviewDirection % 4 + 4) % 4;
    }
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
    bool GetPlacedCenterGridCoords(int& OutGridX, int& OutGridY) const;

private:
    EResourceType ResolvePrimaryResourceTypeForLegacy() const
    {
        return mOperations.ResolvePrimaryResourceTypeForLegacy();
    }

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
    void ApplyPlacementStateToTileMap(
        const std::shared_ptr<class CTileMapComponent>& TileMap,
        const std::vector<int>& Indices,
        bool Apply);
    void RefreshAccessibilityScore();
    void NotifyPlacementTopologyChanged();
    bool BuildDiamondAreaIndices(
        const std::shared_ptr<class CTileMapComponent>& TileMap,
        int CenterIndex, std::vector<int>& OutIndices) const;
    int FindPreviewOpenTileIndex(
        const std::shared_ptr<class CTileMapComponent>& TileMap,
        int CenterIndex,
        const std::vector<int>& CandidateIndices) const;
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
};
