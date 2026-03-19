#pragma once

#include "Object/GameObject.h"
#include "PlacementAreaRuntimeState.h"
#include "../Building/BuildingCatalogQuery.h"
#include "../Building/BuildingTypes.h"
#include "../Citizen/CitizenTypes.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

class CWorld;

namespace PlacementAreaObjectInternal
{
    void FlushPlacementTopologyUpdates(const std::shared_ptr<CWorld>& World);
}

class CPlacementAreaObject :
    public CGameObject
{
    friend class CObject;
    friend void PlacementAreaObjectInternal::FlushPlacementTopologyUpdates(
        const std::shared_ptr<CWorld>& World);

public:
    CPlacementAreaObject();
    CPlacementAreaObject(const CPlacementAreaObject& ref);
    CPlacementAreaObject(CPlacementAreaObject&& ref) noexcept;

    virtual ~CPlacementAreaObject();

    class FScopedTopologyBatch
    {
    public:
        FScopedTopologyBatch() = default;
        ~FScopedTopologyBatch();

        FScopedTopologyBatch(const FScopedTopologyBatch&) = delete;
        FScopedTopologyBatch& operator=(const FScopedTopologyBatch&) = delete;

    private:
        friend class CPlacementAreaObject;

        void MarkDirty(const std::shared_ptr<class CWorld>& World);
        void Flush();

        bool mPending = false;
        std::weak_ptr<class CWorld> mWorld;
    };

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
    FPlacementAreaRuntimeState mRuntime;
    EPlacementBuildingKind mBuildingKind = EPlacementBuildingKind::Structure;
    FPlacementTemplate mTemplate;
    std::vector<int> mMarkerTileIndices;
    std::vector<FPoliticalSignalDef> mPoliticalSignals;

    const FBuildingCatalogRuntimeData* ResolveCatalogRuntimeData() const
    {
        return FindBuildingCatalogRuntimeData(mBuildingId);
    }

    const FBuildingCatalogEntry* ResolveCatalogEntry() const
    {
        return FindBuildingCatalogEntry(mBuildingId);
    }

    template <typename TVisitor>
    void ForEachRuntimeVisitConsumptionAcceptedResourceType(
        EResourceType VisitConsumptionType,
        const TVisitor& Visitor) const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();
        const FBuildingOperationModeEffect Effect = ResolveRuntimeEffect();
        const std::vector<EResourceType>* AcceptedTypes = nullptr;

        if (Effect.HasVisitConsumptionAcceptedTypesOverride)
        {
            AcceptedTypes = &Effect.VisitConsumptionAcceptedTypesOverride;
        }
        else if (Entry)
        {
            AcceptedTypes = &Entry->VisitConsumptionAcceptedResourceTypes;
        }

        const EResourceType EffectiveVisitConsumptionType =
            Effect.HasVisitConsumptionTypeOverride ?
                Effect.VisitConsumptionTypeOverride :
                VisitConsumptionType;

        if (AcceptedTypes && !AcceptedTypes->empty())
        {
            if (ShouldVisitConsumptionUseSelfProducedFoodOnly(GetBuildingId()))
            {
                const EResourceType ProducedType = GetProducedResourceType();

                if (!IsEdibleResourceType(ProducedType))
                    return;

                for (size_t Index = 0;
                    Index < AcceptedTypes->size();
                    ++Index)
                {
                    if ((*AcceptedTypes)[Index] == ProducedType)
                    {
                        Visitor(ProducedType);
                        break;
                    }
                }

                return;
            }

            for (size_t Index = 0;
                Index < AcceptedTypes->size();
                ++Index)
            {
                const EResourceType AcceptedType = (*AcceptedTypes)[Index];

                if (AcceptedType == EResourceType::None ||
                    AcceptedType == EResourceType::Count)
                {
                    continue;
                }

                Visitor(AcceptedType);
            }
            return;
        }

        ForEachFoodVisitCompatibleResourceType(
            GetBuildingId(),
            GetProducedResourceType(),
            EffectiveVisitConsumptionType,
            Visitor);
    }

    std::vector<EResourceType> BuildRuntimeVisitConsumptionAcceptedResourceTypes()
        const
    {
        std::vector<EResourceType> Result;

        ForEachRuntimeVisitConsumptionAcceptedResourceType(
            GetVisitConsumptionResourceType(),
            [&](EResourceType AcceptedType)
            {
                if (std::find(Result.begin(), Result.end(), AcceptedType) ==
                    Result.end())
                {
                    Result.push_back(AcceptedType);
                }
            });

        return Result;
    }

    int ResolveActiveOperationModeIndex(
        const FBuildingCatalogRuntimeData* Entry) const
    {
        if (!Entry || Entry->OperationModeDefs.empty())
            return 0;

        const int ModeCount =
            static_cast<int>(Entry->OperationModeDefs.size());
        return (std::max)(
            0,
            (std::min)(ModeCount - 1, mRuntime.ActiveOperationModeIndex));
    }

    const FBuildingOperationModeDef* ResolveActiveOperationModeDef() const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();

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
        const FBuildingCatalogRuntimeData* Entry) const
    {
        if (!Entry || Entry->RuntimeUpgradeDefs.empty())
            return -1;

        const int UpgradeCount =
            static_cast<int>(Entry->RuntimeUpgradeDefs.size());
        return mRuntime.ActiveRuntimeUpgradeIndex >= 0 &&
            mRuntime.ActiveRuntimeUpgradeIndex < UpgradeCount ?
                mRuntime.ActiveRuntimeUpgradeIndex :
                -1;
    }

    const FBuildingRuntimeUpgradeDef* ResolveActiveRuntimeUpgradeDef() const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();
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

        if (UpgradeEffect.HasProducedResourceTypeOverride)
        {
            Result.HasProducedResourceTypeOverride = true;
            Result.ProducedResourceTypeOverride =
                UpgradeEffect.ProducedResourceTypeOverride;
        }

        if (UpgradeEffect.HasProductionInputTypesOverride)
        {
            Result.HasProductionInputTypesOverride = true;
            Result.ProductionInputTypesOverride =
                UpgradeEffect.ProductionInputTypesOverride;
            Result.ProductionInputAmountsOverride =
                UpgradeEffect.ProductionInputAmountsOverride;
        }

        if (UpgradeEffect.HasVisitConsumptionTypeOverride)
        {
            Result.HasVisitConsumptionTypeOverride = true;
            Result.VisitConsumptionTypeOverride =
                UpgradeEffect.VisitConsumptionTypeOverride;
        }

        if (UpgradeEffect.HasVisitConsumptionAcceptedTypesOverride)
        {
            Result.HasVisitConsumptionAcceptedTypesOverride = true;
            Result.VisitConsumptionAcceptedTypesOverride =
                UpgradeEffect.VisitConsumptionAcceptedTypesOverride;
        }

        Result.ProductionMultiplier *= UpgradeEffect.ProductionMultiplier;
        Result.InputConsumptionMultiplier *=
            UpgradeEffect.InputConsumptionMultiplier;
        Result.ServiceThroughputMultiplier *=
            UpgradeEffect.ServiceThroughputMultiplier;
        Result.HarborProgressMultiplier *=
            UpgradeEffect.HarborProgressMultiplier;
        Result.TeamsterTransferMultiplier *=
            UpgradeEffect.TeamsterTransferMultiplier;
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
        Result.TeamsterCargoLossPercent = (std::max)(
            Result.TeamsterCargoLossPercent,
            UpgradeEffect.TeamsterCargoLossPercent);
        Result.ExportTradeRoutePriceDeltaPercent +=
            UpgradeEffect.ExportTradeRoutePriceDeltaPercent;
        Result.ImportTradeRoutePriceDeltaPercent +=
            UpgradeEffect.ImportTradeRoutePriceDeltaPercent;
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

        if (mRuntime.WarehouseStoragePolicy ==
            EWarehouseStoragePolicy::Dedicated)
            LossMultiplier *= 0.85f;

        return LossMultiplier;
    }

    void RefreshWarehouseStorageRuntime()
    {
        if (!IsWarehouse())
            return;

        mOperations.ConfigureWarehouseRuntime(
            ResolveEffectiveWarehouseSlotCapacity(),
            mRuntime.WarehouseStoragePolicy,
            mRuntime.PreferredWarehouseResourceType);
    }

    void ApplyRuntimeResourceBehavior()
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();

        if (!Entry)
            return;

        EResourceType ProducedResourceType = Entry->ProducedResourceType;
        const FBuildingOperationModeEffect Effect = ResolveRuntimeEffect();

        if (Effect.HasProducedResourceTypeOverride)
            ProducedResourceType = Effect.ProducedResourceTypeOverride;

        EResourceType VisitConsumptionResourceType =
            Entry->VisitConsumptionResourceType;

        if (Effect.HasVisitConsumptionTypeOverride)
        {
            VisitConsumptionResourceType =
                Effect.VisitConsumptionTypeOverride;
        }

        // Input-only operation modes are first-class runtime recipes: keep the
        // current output if needed, but swap the active input slots exactly.
        const std::array<EResourceType, GProductionInputSlotCount>&
            ProductionInputTypes =
                Effect.HasProductionInputTypesOverride ?
                    Effect.ProductionInputTypesOverride :
                    Entry->ProductionInputTypes;
        const std::array<int, GProductionInputSlotCount>&
            ProductionInputAmounts =
                Effect.HasProductionInputTypesOverride ?
                    Effect.ProductionInputAmountsOverride :
                    Entry->ProductionInputAmounts;

        SetResourceBehavior(
            ProducedResourceType,
            VisitConsumptionResourceType,
            Entry->SupportsTeamsterPickup,
            Entry->CanExportStoredResources,
            ProductionInputTypes,
            ProductionInputAmounts);
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
            Effect.PerWorkerServiceCapacityDelta * GetCurrentWorkerOccupancy();
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

    float ResolveOperationModeTeamsterTransferMultiplier() const
    {
        return ResolveRuntimeEffect().TeamsterTransferMultiplier;
    }

    int ResolveOperationModeTeamsterCargoLossPercent() const
    {
        return (std::max)(
            0,
            ResolveRuntimeEffect().TeamsterCargoLossPercent);
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

    float ResolveDamageOperationalMultiplier() const
    {
        return GetDamageEfficiencyMultiplier();
    }

    float ResolvePowerOperationalMultiplier() const
    {
        if (GetRequiredPowerMW() <= 0)
            return 1.f;

        if (mRuntime.PowerSupplyRatio <= 0.05f)
            return 0.f;

        return (std::max)(
            0.f,
            (std::min)(1.f, mRuntime.PowerSupplyRatio));
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

    void SetHouseholdCapacity(int Capacity)
    {
        mServiceProfile.HouseholdCapacity = (std::max)(0, Capacity);
    }

    void SetLeisureClass(EBuildingLeisureClass LeisureClass)
    {
        mServiceProfile.LeisureClass = LeisureClass;
    }

    void SetPrimaryTouristPreference(ETouristPreference Preference)
    {
        mServiceProfile.PrimaryTouristPreference = Preference;
    }

    void SetPoliticalSignals(const std::vector<FPoliticalSignalDef>& Signals)
    {
        mPoliticalSignals = Signals;
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

    void SetAllowedWealthMask(unsigned int Mask)
    {
        mServiceProfile.AllowedWealthMask =
            Mask == GBuildingWealthMaskNone ?
                GBuildingWealthMaskAll :
                Mask;
    }

    void SetResourceBehavior(
        EResourceType ProducedResourceType,
        EResourceType VisitConsumptionResourceType,
        bool SupportsTeamsterPickup,
        bool CanExportStoredResources,
        const std::array<EResourceType, GProductionInputSlotCount>&
            ProductionInputTypes = {},
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
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();
        return Entry && !Entry->OperationModeDefs.empty();
    }

    int GetOperationModeCount() const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();
        return Entry ?
            static_cast<int>(Entry->OperationModeDefs.size()) :
            0;
    }

    int GetActiveOperationModeIndex() const
    {
        return ResolveActiveOperationModeIndex(ResolveCatalogRuntimeData());
    }

    std::wstring GetOperationModeDisplayName(int ModeIndex) const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();
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
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();
        return Entry ?
            ::GetOperationModeEffectSummary(
                *Entry,
                GetActiveOperationModeIndex()) :
            std::wstring();
    }

    bool IsOperationModeResearchLocked(int ModeIndex) const;
    bool IsOperationModeResearchUnlocked(int ModeIndex) const;
    int GetOperationModeResearchCost(int ModeIndex) const;
    std::wstring GetOperationModeResearchLabel(int ModeIndex) const;
    bool SetActiveOperationMode(int ModeIndex, std::wstring& OutMessage);
    bool CycleOperationMode(std::wstring& OutMessage);

    bool HasRuntimeUpgrades() const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();
        return Entry && !Entry->RuntimeUpgradeDefs.empty();
    }

    int GetRuntimeUpgradeCount() const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();
        return Entry ?
            static_cast<int>(Entry->RuntimeUpgradeDefs.size()) :
            0;
    }

    int GetActiveRuntimeUpgradeIndex() const
    {
        return ResolveActiveRuntimeUpgradeIndex(ResolveCatalogRuntimeData());
    }

    std::wstring GetRuntimeUpgradeDisplayName(int UpgradeIndex) const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();
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
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();
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
        switch (mRuntime.WarehouseStoragePolicy)
        {
        case EWarehouseStoragePolicy::Dedicated:
            return L"전용 슬롯 유지";
        default:
            return L"균형 보관";
        }
    }

    std::wstring GetWarehousePriorityDisplayName() const
    {
        if (mRuntime.PreferredWarehouseResourceType == EResourceType::None)
            return L"전체 허용";

        return std::wstring(
            GetResourceTypeDisplayName(
                mRuntime.PreferredWarehouseResourceType)) +
            L" 우선";
    }

    bool CycleWarehouseStoragePolicy(std::wstring& OutMessage);
    bool CycleWarehousePriority(std::wstring& OutMessage);

    int GetProducedPowerMW() const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();

        if (!Entry)
            return 0;

        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        const int ScaledValue = static_cast<int>(roundf(
            static_cast<float>(Entry->BaseProducedPowerMW) *
            ResolveOperationModeProducedPowerMultiplier()));
        return static_cast<int>(roundf(
            static_cast<float>((std::max)(
                0,
                ScaledValue + Effect.ProducedPowerDeltaMW)) *
            ResolveDamageOperationalMultiplier()));
    }

    int GetRequiredPowerMW() const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();

        if (!Entry)
            return 0;

        const FBuildingOperationModeEffect& Effect =
            ResolveRuntimeEffect();
        const int ScaledValue = static_cast<int>(roundf(
            static_cast<float>(Entry->BaseRequiredPowerMW) *
            ResolveOperationModeRequiredPowerMultiplier()));
        return static_cast<int>(roundf(
            static_cast<float>((std::max)(
                0,
                ScaledValue + Effect.RequiredPowerDeltaMW)) *
            ResolveDamageOperationalMultiplier()));
    }

    float GetPowerSupplyRatio() const
    {
        return GetRequiredPowerMW() > 0 ?
            (std::max)(0.f, (std::min)(1.f, mRuntime.PowerSupplyRatio)) :
            1.f;
    }

    void SetPowerSupplyRatio(float Ratio)
    {
        mRuntime.PowerSupplyRatio =
            (std::max)(0.f, (std::min)(1.f, Ratio));
    }

    EBuildingDamageLevel GetDamageLevel() const
    {
        return mRuntime.DamageLevel;
    }

    void SetDamageLevel(EBuildingDamageLevel Level)
    {
        mRuntime.DamageLevel = Level;
    }

    bool HasBuildingDamage() const
    {
        return mRuntime.DamageLevel != EBuildingDamageLevel::None;
    }

    float GetDamageEfficiencyMultiplier() const
    {
        switch (mRuntime.DamageLevel)
        {
        case EBuildingDamageLevel::Damaged:
            return 0.5f;
        case EBuildingDamageLevel::Critical:
            return 0.f;
        case EBuildingDamageLevel::None:
        default:
            return 1.f;
        }
    }

    int GetRepairCost() const
    {
        return (std::max)(0, mRuntime.RepairCost);
    }

    void SetRepairCost(int Cost)
    {
        mRuntime.RepairCost = (std::max)(0, Cost);
    }

    int GetPollutionOutput() const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();

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
        return static_cast<int>(roundf(
            static_cast<float>((std::max)(0, ScaledValue + Delta)) *
            ResolveDamageOperationalMultiplier()));
    }

    int GetPollutionMitigation() const
    {
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();

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
        return static_cast<int>(roundf(
            static_cast<float>((std::max)(0, ScaledValue + Delta)) *
            ResolveDamageOperationalMultiplier()));
    }

    int GetLocalPollutionExposure() const
    {
        return mRuntime.LocalPollutionExposure;
    }

    float GetLocalPollutionExposureNormalized() const
    {
        return (std::max)(
            0.f,
            (std::min)(
                1.f,
                static_cast<float>(mRuntime.LocalPollutionExposure) / 100.f));
    }

    void SetLocalPollutionExposure(int Value)
    {
        mRuntime.LocalPollutionExposure = (std::max)(0, Value);
    }

    int GetLocalFreedomSupport() const
    {
        return mRuntime.LocalFreedomSupport;
    }

    float GetLocalFreedomSupportNormalized() const
    {
        return (std::max)(
            0.f,
            (std::min)(
                1.f,
                0.5f +
                    static_cast<float>(mRuntime.LocalFreedomSupport) / 200.f));
    }

    int GetLocalFreedomScore() const
    {
        return (std::max)(
            0,
            (std::min)(
                100,
                static_cast<int>(roundf(
                    GetLocalFreedomSupportNormalized() * 100.f))));
    }

    void SetLocalFreedomSupport(int Value)
    {
        mRuntime.LocalFreedomSupport =
            (std::max)(-100, (std::min)(100, Value));
    }

    int GetLocalSecuritySupport() const
    {
        return mRuntime.LocalSecuritySupport;
    }

    float GetLocalSecuritySupportNormalized() const
    {
        return (std::max)(
            0.f,
            (std::min)(
                1.f,
                0.5f +
                    static_cast<float>(mRuntime.LocalSecuritySupport) / 200.f));
    }

    int GetLocalSecurityScore() const
    {
        return (std::max)(
            0,
            (std::min)(
                100,
                static_cast<int>(roundf(
                    GetLocalSecuritySupportNormalized() * 100.f))));
    }

    void SetLocalSecuritySupport(int Value)
    {
        mRuntime.LocalSecuritySupport =
            (std::max)(-100, (std::min)(100, Value));
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

    int GetHouseholdCapacity() const
    {
        return mServiceProfile.HouseholdCapacity;
    }

    EBuildingLeisureClass GetLeisureClass() const
    {
        return mServiceProfile.LeisureClass;
    }

    ETouristPreference GetPrimaryTouristPreference() const
    {
        return mServiceProfile.PrimaryTouristPreference;
    }

    const std::vector<FPoliticalSignalDef>& GetPoliticalSignals() const
    {
        return mPoliticalSignals;
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
        return mRuntime.Roles.Warehouse;
    }

    bool IsBusGarage() const
    {
        return mRuntime.Roles.BusGarage;
    }

    bool IsBusStop() const
    {
        return mRuntime.Roles.BusStop;
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

    int GetCurrentWorkerOccupancy() const
    {
        return (std::min)(
            GetCapacity(),
            mOperations.GetCurrentWorkerOccupancy());
    }

    void SetCurrentWorkerOccupancy(int Occupancy)
    {
        mOperations.SetCurrentWorkerOccupancy(Occupancy);
    }

    int GetWorkingNowOccupancy() const
    {
        return (std::min)(
            GetCapacity(),
            mOperations.GetWorkingNowOccupancy());
    }

    void SetWorkingNowOccupancy(int Occupancy)
    {
        mOperations.SetWorkingNowOccupancy(Occupancy);
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
            (std::max)(0.f, (std::min)(1.f, mRuntime.AccessibilityScore));
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

    unsigned int GetAllowedWealthMask() const
    {
        return mServiceProfile.AllowedWealthMask;
    }

    EResourceType GetProducedResourceType() const
    {
        return mOperations.ProducedResourceType;
    }

    EResourceType GetVisitConsumptionResourceType() const
    {
        return mOperations.VisitConsumptionResourceType;
    }

    const std::vector<EResourceType>&
    GetVisitConsumptionAcceptedResourceTypes() const
    {
        static const std::vector<EResourceType> GEmptyAcceptedTypes;
        const FBuildingCatalogRuntimeData* const Entry =
            ResolveCatalogRuntimeData();
        return Entry ?
            Entry->VisitConsumptionAcceptedResourceTypes :
            GEmptyAcceptedTypes;
    }

    std::vector<EResourceType>
    GetRuntimeVisitConsumptionAcceptedResourceTypes() const
    {
        return BuildRuntimeVisitConsumptionAcceptedResourceTypes();
    }

    int GetProductionInputCount() const
    {
        return mOperations.GetProductionInputCount();
    }

    EResourceType GetProductionInputType(int SlotIndex) const
    {
        return mOperations.GetProductionInputType(SlotIndex);
    }

    int GetProductionInputCompatibleResourceStock(
        EResourceType DemandType) const
    {
        return mOperations.GetProductionInputCompatibleResourceStock(
            DemandType);
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
                ResolvePowerOperationalMultiplier() *
                ResolveDamageOperationalMultiplier(),
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
                ResolvePowerOperationalMultiplier() *
                ResolveDamageOperationalMultiplier(),
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
        const int SafeCapacity = GetCapacity();
        float OccupancyRatio = 0.f;

        if (SafeCapacity > 0)
        {
            OccupancyRatio = (std::max)(
                0.f,
                (std::min)(
                    1.f,
                    static_cast<float>(GetCurrentWorkerOccupancy()) /
                        static_cast<float>(SafeCapacity)));
        }

        const int OccupancyScaledWage = static_cast<int>(roundf(
            static_cast<float>(mOperations.GetMonthlyWageCost()) *
            OccupancyRatio));
        return ApplyOperationModeEconomyEffect(
            OccupancyScaledWage,
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

    int GetTeamsterTransferUnits() const
    {
        return (std::max)(
            0,
            static_cast<int>(roundf(
                static_cast<float>(GameConstants::Orb::TeamsterTransferUnit) *
                (std::max)(0.f, ResolveOperationModeTeamsterTransferMultiplier()) *
                ResolveDamageOperationalMultiplier())));
    }

    int GetTeamsterCargoLossPercent() const
    {
        return ResolveOperationModeTeamsterCargoLossPercent();
    }

    int GetTradeRouteExportPriceModifierPercent() const
    {
        return ResolveRuntimeEffect().ExportTradeRoutePriceDeltaPercent;
    }

    int GetTradeRouteImportPriceModifierPercent() const
    {
        return ResolveRuntimeEffect().ImportTradeRoutePriceDeltaPercent;
    }

    bool AdvanceHarborShipProgressAndCheckArrival(int DaysInMonth)
    {
        return mOperations.AdvanceHarborShipProgressAndCheckArrival(
            CanExportStoredResources(),
            DaysInMonth,
            ResolveOperationModeHarborProgressMultiplier() *
                ResolvePowerOperationalMultiplier() *
                ResolveDamageOperationalMultiplier());
    }

    float GetHarborShipProgressPercent() const
    {
        return mOperations.GetHarborShipProgressPercent(
            CanExportStoredResources());
    }

    int GetResourceStock() const { return mOperations.GetResourceStock(); }
    int GetResourceStock(EResourceType Type) const
    {
        return mOperations.GetResourceStock(Type);
    }
    int GetVisitConsumptionCompatibleResourceStock(
        EResourceType DemandType) const
    {
        if (DemandType == EResourceType::None ||
            DemandType == EResourceType::Count)
        {
            return 0;
        }

        if (DemandType != GetVisitConsumptionResourceType() ||
            !IsSummaryResourceType(DemandType))
        {
            return mOperations.GetResourceStock(DemandType);
        }

        int Total = 0;
        ForEachRuntimeVisitConsumptionAcceptedResourceType(
            DemandType,
            [&](EResourceType SupplyType)
            {
                Total += mOperations.GetResourceStock(SupplyType);
            });
        return Total;
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
    int GetProductionInputCompatibleReservedIncomingResourceAmount(
        EResourceType DemandType) const
    {
        if (DemandType == EResourceType::None ||
            DemandType == EResourceType::Count)
        {
            return 0;
        }

        if (DemandType != EResourceType::FeedCrops)
            return mOperations.GetReservedIncomingResourceAmount(DemandType);

        return mOperations.GetReservedIncomingResourceAmount(
                   EResourceType::Corn) +
            mOperations.GetReservedIncomingResourceAmount(
                EResourceType::Sugar);
    }
    int GetVisitConsumptionCompatibleReservedIncomingResourceAmount(
        EResourceType DemandType) const
    {
        if (DemandType == EResourceType::None ||
            DemandType == EResourceType::Count)
        {
            return 0;
        }

        if (DemandType != GetVisitConsumptionResourceType() ||
            !IsSummaryResourceType(DemandType))
        {
            return mOperations.GetReservedIncomingResourceAmount(DemandType);
        }

        int Total = 0;
        ForEachRuntimeVisitConsumptionAcceptedResourceType(
            DemandType,
            [&](EResourceType SupplyType)
            {
                Total += mOperations.GetReservedIncomingResourceAmount(
                    SupplyType);
            });
        return Total;
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

    // 건물 단위로 매 프레임 호출 - float 누적 후 int 단위로 재고에 추가
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
            GetCurrentWorkerOccupancy(),
            GetCapacity(),
            ResolveOperationModeProductionMultiplier() *
                GetBudgetSatisfactionScale() *
                ResolvePowerOperationalMultiplier() *
                ResolveDamageOperationalMultiplier(),
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

    bool TryConsumeVisitConsumptionCompatibleResource(
        EResourceType DemandType,
        int Amount = 1)
    {
        if (Amount <= 0)
            return true;

        if (DemandType == EResourceType::None ||
            DemandType == EResourceType::Count)
        {
            return false;
        }

        if (DemandType != GetVisitConsumptionResourceType() ||
            !IsSummaryResourceType(DemandType))
        {
            return TryConsumeResource(DemandType, Amount);
        }

        EResourceType BestSupplyType = EResourceType::None;
        int BestStock = 0;

        ForEachRuntimeVisitConsumptionAcceptedResourceType(
            DemandType,
            [&](EResourceType SupplyType)
            {
                const int Stock = mOperations.GetResourceStock(SupplyType);

                if (Stock <= 0)
                    return;

                if (BestSupplyType == EResourceType::None ||
                    Stock > BestStock)
                {
                    BestSupplyType = SupplyType;
                    BestStock = Stock;
                }
            });

        return BestSupplyType != EResourceType::None &&
            BestStock >= Amount &&
            TryConsumeResource(BestSupplyType, Amount);
    }

    bool TryConsumeVisitConsumptionResource(int Amount = 1)
    {
        const EResourceType VisitType = GetVisitConsumptionResourceType();
        return VisitType != EResourceType::None ?
            TryConsumeVisitConsumptionCompatibleResource(VisitType, Amount) :
            TryConsumeResource(Amount);
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
        return mRuntime.AccessibilityScore;
    }

    void SetPlacementTemplateType(EPlacementTemplateType Type);
    void SetPlacementTemplate(const FPlacementTemplate& Template);

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Destroy() override;
    void Destroy(FScopedTopologyBatch* TopologyBatch);
    virtual bool IsNavigationObstacle() const override;
    virtual void GetNavigationBlockedTiles(
        std::vector<int>& OutIndices) override;
    virtual void GetNavigationGoalTiles(
        std::vector<int>& OutIndices) override;

public:
    void StartMovePreview(const FVector2& MouseWorldPos);
    void ConfirmPlacement(FScopedTopologyBatch* TopologyBatch = nullptr);
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
    void NotifyPlacementTopologyChanged(
        FScopedTopologyBatch* TopologyBatch = nullptr);
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
