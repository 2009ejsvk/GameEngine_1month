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
    bool HasDirectionalGap = true;

    int GetExpectedTileCount() const
    {
        const int Side = DiamondRadius * 2 + 1;
        return Side * Side - (HasDirectionalGap ? 1 : 0);
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
    int mPreviewDirection = 0;
    int mInitialCenterOffsetX = 0;
    int mInitialCenterOffsetY = 0;
    bool mAutoPlaceOnPrepare = true;
    std::string mBuildingId;
    std::string mBuildingDisplayName;
    std::string mBuildingCategoryName;
    std::string mBuildingSpriteTexturePath;
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
    int mResourceStock = 0;
    float mResourceProductionAccum = 0.f;
    int mBaseMonthlyWage = 0;
    int mBaseMonthlyUpkeep = 0;
    float mHarborShipProgressMonths = 0.f;
    static constexpr int GMaxResourceStock = 100000;

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
        int FunSatisfactionCap = 100,
        int BaseMonthlyWage = -1,
        int BaseMonthlyUpkeep = -1)
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

        const int SafeCapacity = (std::max)(0, mCapacity);

        if (BaseMonthlyWage < 0)
        {
            if (mResidential)
                mBaseMonthlyWage = 0;
            else
            {
                int DerivedWage = (std::max)(1, SafeCapacity) * 120;

                if (IsTransportOffice())
                    DerivedWage = (std::max)(DerivedWage, 800);

                if (IsHarbor())
                    DerivedWage = (std::max)(DerivedWage, 1000);

                mBaseMonthlyWage = DerivedWage;
            }
        }
        else
        {
            mBaseMonthlyWage = (std::max)(0, BaseMonthlyWage);
        }

        if (BaseMonthlyUpkeep < 0)
        {
            int DerivedUpkeep = mResidential ?
                (80 + SafeCapacity * 4) :
                (110 + SafeCapacity * 5);

            if (IsTransportOffice())
                DerivedUpkeep += 300;

            if (IsHarbor())
                DerivedUpkeep += 450;

            if (mEntertainmentProvider && !mFoodProvider)
                DerivedUpkeep += 120;

            if (mFoodProvider)
                DerivedUpkeep += 90;

            mBaseMonthlyUpkeep = (std::max)(0, DerivedUpkeep);
        }
        else
        {
            mBaseMonthlyUpkeep = (std::max)(0, BaseMonthlyUpkeep);
        }
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

    bool IsTransportOffice() const
    {
        return mBuildingId == "build_1_5" ||
            mBuildingId == "starter_teamster_office";
    }

    bool IsHarbor() const
    {
        return mBuildingId == "build_1_3" ||
            mBuildingId == "starter_harbor";
    }

    bool IsFoodProductionFacility() const
    {
        return mFoodProvider &&
            !mEntertainmentProvider &&
            !IsTransportOffice() &&
            !IsHarbor();
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

    int GetBaseMonthlyWage() const
    {
        return mBaseMonthlyWage;
    }

    int GetBaseMonthlyUpkeep() const
    {
        return mBaseMonthlyUpkeep;
    }

    int GetMonthlyWageCost() const
    {
        return ApplyEconomyScale(mBaseMonthlyWage);
    }

    int GetMonthlyUpkeepCost() const
    {
        return ApplyEconomyScale(mBaseMonthlyUpkeep);
    }

    int GetDailyWageCost(int DaysInMonth) const
    {
        const int SafeDays = (std::max)(1, DaysInMonth);
        const float Daily = static_cast<float>(GetMonthlyWageCost()) /
            static_cast<float>(SafeDays);
        return (std::max)(0, static_cast<int>(roundf(Daily)));
    }

    int GetDailyUpkeepCost(int DaysInMonth) const
    {
        const int SafeDays = (std::max)(1, DaysInMonth);
        const float Daily = static_cast<float>(GetMonthlyUpkeepCost()) /
            static_cast<float>(SafeDays);
        return (std::max)(0, static_cast<int>(roundf(Daily)));
    }

    bool AdvanceHarborShipProgressAndCheckArrival(int DaysInMonth)
    {
        if (!IsHarbor())
            return false;

        const int SafeDays = (std::max)(1, DaysInMonth);
        const float DailyProgress =
            GetBudgetSatisfactionScale() /
            static_cast<float>(SafeDays);
        mHarborShipProgressMonths += DailyProgress;

        constexpr float GBaseShipIntervalMonths = 3.f;

        if (mHarborShipProgressMonths < GBaseShipIntervalMonths)
            return false;

        while (mHarborShipProgressMonths >= GBaseShipIntervalMonths)
        {
            mHarborShipProgressMonths -= GBaseShipIntervalMonths;
        }

        return true;
    }

    float GetHarborShipProgressPercent() const
    {
        if (!IsHarbor())
            return 0.f;

        constexpr float GBaseShipIntervalMonths = 3.f;
        return Clamp<float>(
            mHarborShipProgressMonths / GBaseShipIntervalMonths, 0.f, 1.f);
    }

    int GetResourceStock() const { return mResourceStock; }
    int GetMaxResourceStock() const { return GMaxResourceStock; }

    void AddResourceStock(int Amount)
    {
        if (Amount <= 0)
            return;

        mResourceStock = (std::min)(
            GMaxResourceStock, mResourceStock + Amount);
    }

    // AtWork 시민이 매 프레임 호출 - float 누적 후 int 단위로 재고에 추가
    void AddProduction(float UnitsPerSec, float DeltaTime)
    {
        mResourceProductionAccum += UnitsPerSec * DeltaTime;
        const int Whole = static_cast<int>(mResourceProductionAccum);
        if (Whole > 0)
        {
            mResourceProductionAccum -= static_cast<float>(Whole);
            mResourceStock = (std::min)(GMaxResourceStock, mResourceStock + Whole);
        }
    }

    // AtFood 진입 시 호출 - 재고 1단위 소비 시도, 성공하면 true
    bool TryConsumeResource(int Amount = 1)
    {
        if (Amount <= 0)
            return true;

        if (mResourceStock >= Amount)
        {
            mResourceStock -= Amount;
            return true;
        }
        return false;
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
    int ApplyBudgetScale(int BaseCap) const
    {
        const float Scaled =
            BaseCap * GetBudgetSatisfactionScale();
        const int Rounded = static_cast<int>(roundf(Scaled));
        return (std::max)(0, (std::min)(100, Rounded));
    }

    int ApplyEconomyScale(int BaseCost) const
    {
        const float Scaled =
            static_cast<float>(BaseCost) * GetBudgetSatisfactionScale();
        return (std::max)(0, static_cast<int>(roundf(Scaled)));
    }
};
