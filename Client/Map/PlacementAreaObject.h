#pragma once

#include "Object/GameObject.h"
#include "../Building/BuildingCatalog.h"
#include "../Building/BuildingTypes.h"
#include "../Citizen/CitizenTypes.h"
#include "PlacementAreaComponents.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

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
    static void BeginTopologyBatchUpdate();
    static void EndTopologyBatchUpdate();

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
    int mActiveOperationModeIndex = 0;
    int mActiveRuntimeUpgradeIndex = -1;
    EWarehouseStoragePolicy mWarehouseStoragePolicy =
        EWarehouseStoragePolicy::Balanced;
    EResourceType mPreferredWarehouseResourceType =
        EResourceType::None;
    float mPowerSupplyRatio = 1.f;
    int mLocalPollutionExposure = 0;
    EPlacementBuildingKind mBuildingKind = EPlacementBuildingKind::Structure;
    FPlacementTemplate mTemplate;
    std::vector<int> mMarkerTileIndices;
    float mAccessibilityScore = 0.f;

    const FBuildingCatalogEntry* ResolveCatalogEntry() const
    {
        return FindBuildingCatalogEntry(mBuildingId);
    }

    int ResolveActiveOperationModeIndex(
        const FBuildingCatalogEntry* Entry) const
    {
        if (!Entry || Entry->OperationModeDefs.empty())
            return 0;

        const int ModeCount =
            static_cast<int>(Entry->OperationModeDefs.size());
        return (std::max)(0, (std::min)(ModeCount - 1, mActiveOperationModeIndex));
    }

    const FBuildingOperationModeDef* ResolveActiveOperationModeDef() const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();

        if (!Entry || Entry->OperationModeDefs.empty())
            return nullptr;

        return &Entry->OperationModeDefs[
            static_cast<size_t>(ResolveActiveOperationModeIndex(Entry))];
    }

    const FBuildingOperationModeEffect& ResolveActiveOperationModeEffect() const
    {
        static const FBuildingOperationModeEffect GDefaultEffect;
        const FBuildingOperationModeDef* const ModeDef =
            ResolveActiveOperationModeDef();
        return ModeDef ? ModeDef->Effect : GDefaultEffect;
    }

    int ResolveActiveRuntimeUpgradeIndex(
        const FBuildingCatalogEntry* Entry) const
    {
        if (!Entry || Entry->RuntimeUpgradeDefs.empty())
            return -1;

        const int UpgradeCount =
            static_cast<int>(Entry->RuntimeUpgradeDefs.size());
        return mActiveRuntimeUpgradeIndex >= 0 &&
            mActiveRuntimeUpgradeIndex < UpgradeCount ?
                mActiveRuntimeUpgradeIndex :
                -1;
    }

    const FBuildingRuntimeUpgradeDef* ResolveActiveRuntimeUpgradeDef() const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();
        const int UpgradeIndex = ResolveActiveRuntimeUpgradeIndex(Entry);

        if (!Entry || UpgradeIndex < 0)
            return nullptr;

        return &Entry->RuntimeUpgradeDefs[static_cast<size_t>(UpgradeIndex)];
    }

    const FBuildingOperationModeEffect& ResolveActiveRuntimeUpgradeEffect() const
    {
        static const FBuildingOperationModeEffect GDefaultEffect;
        const FBuildingRuntimeUpgradeDef* const UpgradeDef =
            ResolveActiveRuntimeUpgradeDef();
        return UpgradeDef ? UpgradeDef->Effect : GDefaultEffect;
    }

    FBuildingOperationModeEffect ResolveRuntimeEffect() const
    {
        FBuildingOperationModeEffect Result =
            ResolveActiveOperationModeEffect();
        const FBuildingOperationModeEffect& UpgradeEffect =
            ResolveActiveRuntimeUpgradeEffect();

        Result.ProductionMultiplier *= UpgradeEffect.ProductionMultiplier;
        Result.InputConsumptionMultiplier *=
            UpgradeEffect.InputConsumptionMultiplier;
        Result.ServiceThroughputMultiplier *=
            UpgradeEffect.ServiceThroughputMultiplier;
        Result.HarborProgressMultiplier *=
            UpgradeEffect.HarborProgressMultiplier;
        Result.ProducedPowerMultiplier *=
            UpgradeEffect.ProducedPowerMultiplier;
        Result.RequiredPowerMultiplier *=
            UpgradeEffect.RequiredPowerMultiplier;
        Result.WarehouseSlotCapacityMultiplier *=
            UpgradeEffect.WarehouseSlotCapacityMultiplier;
        Result.StorageLossMultiplier *=
            UpgradeEffect.StorageLossMultiplier;
        Result.PollutionMultiplier *= UpgradeEffect.PollutionMultiplier;
        Result.WageMultiplier *= UpgradeEffect.WageMultiplier;
        Result.UpkeepMultiplier *= UpgradeEffect.UpkeepMultiplier;
        Result.HousingQualityMultiplier *=
            UpgradeEffect.HousingQualityMultiplier;
        Result.JobQualityMultiplier *=
            UpgradeEffect.JobQualityMultiplier;
        Result.GenericServiceQualityMultiplier *=
            UpgradeEffect.GenericServiceQualityMultiplier;
        Result.ProducedPowerDeltaMW += UpgradeEffect.ProducedPowerDeltaMW;
        Result.RequiredPowerDeltaMW += UpgradeEffect.RequiredPowerDeltaMW;
        Result.WarehouseSlotCapacityDelta +=
            UpgradeEffect.WarehouseSlotCapacityDelta;
        Result.PollutionFlatDelta += UpgradeEffect.PollutionFlatDelta;
        Result.WageFlatDelta += UpgradeEffect.WageFlatDelta;
        Result.UpkeepFlatDelta += UpgradeEffect.UpkeepFlatDelta;
        Result.CapacityDelta += UpgradeEffect.CapacityDelta;
        Result.ServiceCapacityDelta += UpgradeEffect.ServiceCapacityDelta;
        Result.PerWorkerServiceCapacityDelta +=
            UpgradeEffect.PerWorkerServiceCapacityDelta;
        Result.HousingQualityDelta += UpgradeEffect.HousingQualityDelta;
        Result.JobQualityDelta += UpgradeEffect.JobQualityDelta;
        Result.GenericServiceQualityDelta +=
            UpgradeEffect.GenericServiceQualityDelta;
        return Result;
    }

    int ResolveEffectiveWarehouseSlotCapacity() const
    {
        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        const int ScaledCapacity = static_cast<int>(roundf(
            static_cast<float>(FBuildingOperationsState::WarehouseSlotCapacity) *
            (std::max)(0.f, Effect.WarehouseSlotCapacityMultiplier)));
        return (std::max)(
            1000,
            ScaledCapacity + Effect.WarehouseSlotCapacityDelta);
    }

    float ResolveWarehouseStorageLossMultiplier() const
    {
        float LossMultiplier = (std::max)(
            0.f,
            ResolveRuntimeEffect().StorageLossMultiplier);

        if (mWarehouseStoragePolicy == EWarehouseStoragePolicy::Dedicated)
            LossMultiplier *= 0.85f;

        return LossMultiplier;
    }

    void RefreshWarehouseStorageRuntime()
    {
        if (!IsWarehouse())
            return;

        mOperations.ConfigureWarehouseRuntime(
            ResolveEffectiveWarehouseSlotCapacity(),
            mWarehouseStoragePolicy,
            mPreferredWarehouseResourceType);
    }

    int ApplyOperationModeCapEffect(
        int BaseCap,
        float Multiplier,
        int Delta) const
    {
        const int ScaledCap = static_cast<int>(roundf(
            static_cast<float>(BaseCap) * (std::max)(0.f, Multiplier)));
        return (std::max)(0, (std::min)(100, ScaledCap + Delta));
    }

    int ApplyOperationModeEconomyEffect(
        int BaseValue,
        float Multiplier,
        int Delta) const
    {
        const int ScaledValue = static_cast<int>(roundf(
            static_cast<float>(BaseValue) * (std::max)(0.f, Multiplier)));
        return (std::max)(0, ScaledValue + Delta);
    }

    int ResolveOperationModeCapacityDelta() const
    {
        return ResolveRuntimeEffect().CapacityDelta;
    }

    int ResolveOperationModeServiceCapacityDelta() const
    {
        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        return Effect.ServiceCapacityDelta +
            Effect.PerWorkerServiceCapacityDelta * GetCapacity();
    }

    int ResolveEffectiveServiceCapacityDelta() const
    {
        return static_cast<int>(roundf(
            static_cast<float>(ResolveOperationModeServiceCapacityDelta()) *
            ResolvePowerOperationalMultiplier()));
    }

    float ResolveOperationModeProductionMultiplier() const
    {
        return ResolveRuntimeEffect().ProductionMultiplier;
    }

    float ResolveOperationModeInputConsumptionMultiplier() const
    {
        return ResolveRuntimeEffect().InputConsumptionMultiplier;
    }

    float ResolveOperationModeServiceThroughputMultiplier() const
    {
        return ResolveRuntimeEffect().ServiceThroughputMultiplier;
    }

    float ResolveOperationModeHarborProgressMultiplier() const
    {
        return ResolveRuntimeEffect().HarborProgressMultiplier;
    }

    float ResolveOperationModeProducedPowerMultiplier() const
    {
        return ResolveRuntimeEffect().ProducedPowerMultiplier;
    }

    float ResolveOperationModeRequiredPowerMultiplier() const
    {
        return ResolveRuntimeEffect().RequiredPowerMultiplier;
    }

    float ResolveOperationModePollutionMultiplier() const
    {
        return ResolveRuntimeEffect().PollutionMultiplier;
    }

    float ResolveOperationModePollutionMitigationMultiplier() const
    {
        return (std::max)(
            0.f,
            2.f - ResolveRuntimeEffect().PollutionMultiplier);
    }

    float ResolvePowerOperationalMultiplier() const
    {
        if (GetRequiredPowerMW() <= 0)
            return 1.f;

        if (mPowerSupplyRatio <= 0.05f)
            return 0.f;

        return (std::max)(0.f, (std::min)(1.f, mPowerSupplyRatio));
    }

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

    EPlacementBuildingKind GetBuildingKind() const
    {
        return mBuildingKind;
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
        bool HealthProvider = false,
        bool FaithProvider = false,
        int HousingSatisfactionCap = 100,
        int JobSatisfactionCap = 100,
        int FoodSatisfactionCap = 100,
        int FunSatisfactionCap = 100,
        int HealthSatisfactionCap = 100,
        int FaithSatisfactionCap = 100,
        int ServiceCapacity = 0,
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
        bool CanExportStoredResources,
        const std::array<EResourceType, GProductionInputSlotCount>&
            ProductionInputTypes =
            {
                EResourceType::None,
                EResourceType::None
            },
        const std::array<int, GProductionInputSlotCount>&
            ProductionInputAmounts = {})
    {
        mOperations.ConfigureResourceBehavior(
            ProducedResourceType,
            VisitConsumptionResourceType,
            SupportsTeamsterPickup,
            CanExportStoredResources,
            ProductionInputTypes,
            ProductionInputAmounts);
    }

    bool HasOperationModes() const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();
        return Entry && !Entry->OperationModeDefs.empty();
    }

    int GetOperationModeCount() const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();
        return Entry ?
            static_cast<int>(Entry->OperationModeDefs.size()) :
            0;
    }

    int GetActiveOperationModeIndex() const
    {
        return ResolveActiveOperationModeIndex(ResolveCatalogEntry());
    }

    std::wstring GetOperationModeDisplayName(int ModeIndex) const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();
        return Entry ?
            ::GetOperationModeDisplayName(*Entry, ModeIndex) :
            std::wstring();
    }

    std::wstring GetActiveOperationModeDisplayName() const
    {
        return GetOperationModeDisplayName(GetActiveOperationModeIndex());
    }

    std::wstring GetActiveOperationModeEffectSummary() const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();
        return Entry ?
            ::GetOperationModeEffectSummary(
                *Entry,
                GetActiveOperationModeIndex()) :
            std::wstring();
    }

    bool SetActiveOperationMode(int ModeIndex, std::wstring& OutMessage);
    bool CycleOperationMode(std::wstring& OutMessage);

    bool HasRuntimeUpgrades() const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();
        return Entry && !Entry->RuntimeUpgradeDefs.empty();
    }

    int GetRuntimeUpgradeCount() const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();
        return Entry ?
            static_cast<int>(Entry->RuntimeUpgradeDefs.size()) :
            0;
    }

    int GetActiveRuntimeUpgradeIndex() const
    {
        return ResolveActiveRuntimeUpgradeIndex(ResolveCatalogEntry());
    }

    std::wstring GetRuntimeUpgradeDisplayName(int UpgradeIndex) const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();
        return Entry ?
            ::GetRuntimeUpgradeDisplayName(*Entry, UpgradeIndex) :
            std::wstring();
    }

    std::wstring GetActiveRuntimeUpgradeDisplayName() const
    {
        return GetRuntimeUpgradeDisplayName(GetActiveRuntimeUpgradeIndex());
    }

    std::wstring GetRuntimeUpgradeEffectSummary(int UpgradeIndex) const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();
        return Entry ?
            ::GetRuntimeUpgradeEffectSummary(*Entry, UpgradeIndex) :
            std::wstring();
    }

    std::wstring GetActiveRuntimeUpgradeEffectSummary() const
    {
        return GetRuntimeUpgradeEffectSummary(
            GetActiveRuntimeUpgradeIndex());
    }

    bool CycleRuntimeUpgrade(std::wstring& OutMessage);

    bool HasWarehouseStorageControls() const
    {
        return IsWarehouse();
    }

    std::wstring GetWarehouseStoragePolicyDisplayName() const
    {
        switch (mWarehouseStoragePolicy)
        {
        case EWarehouseStoragePolicy::Dedicated:
            return L"전용 슬롯 유지";
        default:
            return L"균형 보관";
        }
    }

    std::wstring GetWarehousePriorityDisplayName() const
    {
        if (mPreferredWarehouseResourceType == EResourceType::None)
            return L"전체 허용";

        return std::wstring(
            GetResourceTypeDisplayName(mPreferredWarehouseResourceType)) +
            L" 우선";
    }

    bool CycleWarehouseStoragePolicy(std::wstring& OutMessage);
    bool CycleWarehousePriority(std::wstring& OutMessage);

    int GetProducedPowerMW() const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();

        if (!Entry)
            return 0;

        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        const int ScaledValue = static_cast<int>(roundf(
            static_cast<float>(Entry->BaseProducedPowerMW) *
            ResolveOperationModeProducedPowerMultiplier()));
        return (std::max)(0, ScaledValue + Effect.ProducedPowerDeltaMW);
    }

    int GetRequiredPowerMW() const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();

        if (!Entry)
            return 0;

        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        const int ScaledValue = static_cast<int>(roundf(
            static_cast<float>(Entry->BaseRequiredPowerMW) *
            ResolveOperationModeRequiredPowerMultiplier()));
        return (std::max)(0, ScaledValue + Effect.RequiredPowerDeltaMW);
    }

    float GetPowerSupplyRatio() const
    {
        return GetRequiredPowerMW() > 0 ?
            (std::max)(0.f, (std::min)(1.f, mPowerSupplyRatio)) :
            1.f;
    }

    void SetPowerSupplyRatio(float Ratio)
    {
        mPowerSupplyRatio = (std::max)(0.f, (std::min)(1.f, Ratio));
    }

    int GetPollutionOutput() const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();

        if (!Entry)
            return 0;

        const int BaseOutput = (std::max)(0, Entry->BasePollutionOutput);
        const int BaseMitigation =
            (std::max)(0, Entry->BasePollutionMitigation);

        if (BaseOutput <= 0)
            return 0;

        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        const bool PollutionDominant = BaseOutput >= BaseMitigation;
        const int ScaledValue = PollutionDominant ?
            static_cast<int>(roundf(
                static_cast<float>(BaseOutput) *
                ResolveOperationModePollutionMultiplier())) :
            BaseOutput;
        const int Delta = PollutionDominant ? Effect.PollutionFlatDelta : 0;
        return (std::max)(0, ScaledValue + Delta);
    }

    int GetPollutionMitigation() const
    {
        const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();

        if (!Entry)
            return 0;

        const int BaseOutput = (std::max)(0, Entry->BasePollutionOutput);
        const int BaseMitigation =
            (std::max)(0, Entry->BasePollutionMitigation);

        if (BaseMitigation <= 0)
            return 0;

        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        const bool MitigationDominant = BaseMitigation > BaseOutput;
        const int ScaledValue = MitigationDominant ?
            static_cast<int>(roundf(
                static_cast<float>(BaseMitigation) *
                ResolveOperationModePollutionMitigationMultiplier())) :
            BaseMitigation;
        const int Delta = MitigationDominant ? -Effect.PollutionFlatDelta : 0;
        return (std::max)(0, ScaledValue + Delta);
    }

    int GetLocalPollutionExposure() const
    {
        return mLocalPollutionExposure;
    }

    float GetLocalPollutionExposureNormalized() const
    {
        return (std::max)(
            0.f,
            (std::min)(
                1.f,
                static_cast<float>(mLocalPollutionExposure) / 100.f));
    }

    void SetLocalPollutionExposure(int Value)
    {
        mLocalPollutionExposure = (std::max)(0, Value);
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

    bool IsHealthProvider() const
    {
        return mServiceProfile.HealthProvider;
    }

    bool IsFaithProvider() const
    {
        return mServiceProfile.FaithProvider;
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
        return IsFoodResourceType(mOperations.ProducedResourceType) &&
            !IsTransportOffice() &&
            !IsHarbor();
    }

    bool CanGenerateWorkOutput() const
    {
        return mOperations.ProducedResourceType != EResourceType::None;
    }

    int GetCapacity() const
    {
        return (std::max)(
            0,
            mServiceProfile.Capacity + ResolveOperationModeCapacityDelta());
    }

    int GetHousingSatisfactionCap() const
    {
        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        return ApplyOperationModeCapEffect(
            mOperations.ApplyBudgetScale(
                mServiceProfile.HousingSatisfactionCap),
            Effect.HousingQualityMultiplier,
            Effect.HousingQualityDelta);
    }

    int GetJobSatisfactionCap() const
    {
        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        return ApplyOperationModeCapEffect(
            mOperations.ApplyBudgetScale(
                mServiceProfile.JobSatisfactionCap),
            Effect.JobQualityMultiplier,
            Effect.JobQualityDelta);
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
        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        return ApplyOperationModeCapEffect(
            mOperations.ApplyBudgetScale(
                mServiceProfile.FoodSatisfactionCap),
            Effect.GenericServiceQualityMultiplier,
            Effect.GenericServiceQualityDelta);
    }

    int GetFunSatisfactionCap() const
    {
        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        return ApplyOperationModeCapEffect(
            mOperations.ApplyBudgetScale(
                mServiceProfile.FunSatisfactionCap),
            Effect.GenericServiceQualityMultiplier,
            Effect.GenericServiceQualityDelta);
    }

    int GetHealthSatisfactionCap() const
    {
        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        return ApplyOperationModeCapEffect(
            mOperations.ApplyBudgetScale(
                mServiceProfile.HealthSatisfactionCap),
            Effect.GenericServiceQualityMultiplier,
            Effect.GenericServiceQualityDelta);
    }

    int GetFaithSatisfactionCap() const
    {
        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        return ApplyOperationModeCapEffect(
            mOperations.ApplyBudgetScale(
                mServiceProfile.FaithSatisfactionCap),
            Effect.GenericServiceQualityMultiplier,
            Effect.GenericServiceQualityDelta);
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

    int GetProductionInputCount() const
    {
        return mOperations.GetProductionInputCount();
    }

    EResourceType GetProductionInputType(int SlotIndex) const
    {
        return mOperations.GetProductionInputType(SlotIndex);
    }

    int GetProductionInputAmount(int SlotIndex) const
    {
        return mOperations.GetProductionInputAmount(SlotIndex);
    }

    bool UsesProductionInputResource(EResourceType Type) const
    {
        return mOperations.UsesProductionInputResource(Type);
    }

    float GetLastProductionEfficiency() const
    {
        return mOperations.GetLastProductionEfficiency();
    }

    int GetServiceVisitCapacity(EBuildingServiceType Type) const
    {
        return mOperations.GetServiceVisitCapacity(
            Type,
            ResolveOperationModeServiceThroughputMultiplier() *
                ResolvePowerOperationalMultiplier(),
            ResolveEffectiveServiceCapacityDelta());
    }

    int GetActiveServiceVisitorCount(EBuildingServiceType Type) const
    {
        return mOperations.GetActiveServiceVisitorCount(Type);
    }

    int GetMaxServiceVisitCapacity() const
    {
        int MaxCapacity = 0;

        for (int ServiceIndex = 0;
            ServiceIndex < GBuildingServiceTypeCount;
            ++ServiceIndex)
        {
            MaxCapacity = (std::max)(
                MaxCapacity,
                GetServiceVisitCapacity(
                    static_cast<EBuildingServiceType>(ServiceIndex)));
        }

        return MaxCapacity;
    }

    bool TryBeginServiceVisit(EBuildingServiceType Type)
    {
        return mOperations.TryBeginServiceVisit(
            Type,
            ResolveOperationModeServiceThroughputMultiplier() *
                ResolvePowerOperationalMultiplier(),
            ResolveEffectiveServiceCapacityDelta());
    }

    void EndServiceVisit(EBuildingServiceType Type)
    {
        mOperations.EndServiceVisit(Type);
    }

    bool TryConsumeServiceStock(
        EBuildingServiceType Type,
        int Amount = 1)
    {
        return mOperations.TryConsumeServiceStock(Type, Amount);
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
        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        return ApplyOperationModeEconomyEffect(
            mOperations.GetMonthlyWageCost(),
            Effect.WageMultiplier,
            Effect.WageFlatDelta);
    }

    int GetMonthlyUpkeepCost() const
    {
        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        return ApplyOperationModeEconomyEffect(
            mOperations.GetMonthlyUpkeepCost(),
            Effect.UpkeepMultiplier,
            Effect.UpkeepFlatDelta);
    }

    int GetDailyWageCost(int DaysInMonth) const
    {
        const int SafeDays = (std::max)(1, DaysInMonth);
        return (std::max)(
            0,
            static_cast<int>(roundf(
                static_cast<float>(GetMonthlyWageCost()) /
                static_cast<float>(SafeDays))));
    }

    int GetDailyUpkeepCost(int DaysInMonth) const
    {
        const int SafeDays = (std::max)(1, DaysInMonth);
        return (std::max)(
            0,
            static_cast<int>(roundf(
                static_cast<float>(GetMonthlyUpkeepCost()) /
                static_cast<float>(SafeDays))));
    }

    bool AdvanceHarborShipProgressAndCheckArrival(int DaysInMonth)
    {
        return mOperations.AdvanceHarborShipProgressAndCheckArrival(
            IsHarbor(),
            DaysInMonth,
            ResolveOperationModeHarborProgressMultiplier() *
                ResolvePowerOperationalMultiplier());
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
    int GetReservedResourcePickupAmount(EResourceType Type) const
    {
        return mOperations.GetReservedResourcePickupAmount(Type);
    }
    int GetReservedExportPickupAmount() const
    {
        return mOperations.GetReservedExportPickupAmount();
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
    int GetWarehouseSlotCapacityUnits() const
    {
        return IsWarehouse() ?
            mOperations.GetWarehouseSlotCapacityUnits() :
            0;
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
    int GetLastDailyWarehouseStorageLoss() const
    {
        return IsWarehouse() ?
            mOperations.GetLastDailyStorageLoss() :
            0;
    }
    int ApplyDailyWarehouseStorageLoss()
    {
        return IsWarehouse() ?
            mOperations.ApplyDailyWarehouseStorageLoss(
                ResolveWarehouseStorageLossMultiplier()) :
            0;
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

        mOperations.AddProduction(
            UnitsPerSec,
            DeltaTime,
            ResolveOperationModeProductionMultiplier() *
                ResolvePowerOperationalMultiplier(),
            ResolveOperationModeInputConsumptionMultiplier());
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
